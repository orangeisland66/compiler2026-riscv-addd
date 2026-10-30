#include "research_switch.h"
// 文件职责：条件累积 if-conversion（使用 Select 指令）。
// 将 if(cond) { acc = acc + val } 转换为 acc = acc + select(cond, val, 0)。
// Select 语义正确处理重复前驱问题：结果在 condBB 中计算，merge Phi 自然折叠。

#include "if_convert_accumulation.h"
#include "../analysis/analysis_manager.h"
#include "../analysis/dominators.h"
#include "../analysis/loop_info.h"
#include "canonical_loop.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hir {
namespace {

using core_ir::BasicBlock;
using core_ir::ConstInt;
using core_ir::Function;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Type;
using core_ir::Value;

bool traceEnabled() { return sycEnvironmentValue("SYC_TRACE_IF_CONVERT") != nullptr; }

void trace(const char* msg, BasicBlock* bb = nullptr, const char* extra = "") {
  if (!traceEnabled()) return;
  std::cerr << "[if-convert] " << msg
            << (bb ? " blk=" + bb->name() : std::string())
            << " " << extra << "\n";
}

bool isBooleanValue(Value* value, int depth = 0) {
  if (!value || depth > 12)
    return false;
  if (auto* constant = dynamic_cast<ConstInt*>(value))
    return constant->value() == 0 || constant->value() == 1;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !inst->type().isI32())
    return false;
  if (inst->opcode() == Opcode::ICmp)
    return inst->numOperands() == 2;
  if (inst->opcode() != Opcode::And && inst->opcode() != Opcode::Or &&
      inst->opcode() != Opcode::Xor)
    return false;
  return inst->numOperands() == 2 &&
         isBooleanValue(inst->operand(0), depth + 1) &&
         isBooleanValue(inst->operand(1), depth + 1);
}

bool isPureSpeculatableValue(Value* value, int depth = 0) {
  if (!value || depth > 16)
    return false;
  if (dynamic_cast<ConstInt*>(value) ||
      dynamic_cast<core_ir::GlobalVar*>(value) ||
      dynamic_cast<core_ir::Argument*>(value) ||
      dynamic_cast<BasicBlock*>(value))
    return true;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst) return false;
  // GEP 地址计算允许使用指针类型。
  if (!inst->type().isI32() && !inst->type().isPtr())
    return false;
  switch (inst->opcode()) {
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Mul:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
  case Opcode::Shl:
  case Opcode::Shr:
  case Opcode::ICmp:
  case Opcode::Neg:
  case Opcode::Load:   // 全局数组加载可以提前计算
  case Opcode::GEP:    // 地址计算本身没有副作用
  case Opcode::Phi:    // 循环携带值在被支配的基本块中可用
    if (inst->opcode() == Opcode::Phi)
      return true;
    for (Value* operand : inst->operands()) {
      if (!isPureSpeculatableValue(operand, depth + 1)) {
        auto* opInst = dynamic_cast<Instruction*>(operand);
        if (traceEnabled())
          std::cerr << "[if-convert] specul-fail: parent=" << (int)inst->opcode()
                    << " op=" << (opInst ? std::to_string((int)opInst->opcode()) : "n/a")
                    << " depth=" << depth
                    << std::endl;
        return false;
      }
    }
    return true;
  default:
    return false;
  }
}

bool valueAvailableAt(Value* value, BasicBlock* block,
                      const analysis::DomSet& doms, int depth = 0) {
  if (!value || !block || depth > 16)
    return false;
  if (dynamic_cast<ConstInt*>(value) ||
      dynamic_cast<core_ir::GlobalVar*>(value) ||
      dynamic_cast<core_ir::Argument*>(value) ||
      dynamic_cast<BasicBlock*>(value))
    return true;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !inst->parent() ||
      (inst->parent() != block &&
       !analysis::dominates(doms, inst->parent(), block)))
    return false;
  if (inst->opcode() == Opcode::Phi)
    return true;
  for (Value* operand : inst->operands()) {
    if (!valueAvailableAt(operand, block, doms, depth + 1))
      return false;
  }
  return true;
}

bool isSpeculatableOpcode(Instruction* inst) {
  if (!inst || (!inst->type().isI32() && !inst->type().isPtr()))
    return false;
  switch (inst->opcode()) {
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Mul:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
  case Opcode::Shl:
  case Opcode::Shr:
  case Opcode::ICmp:
  case Opcode::Neg:
  case Opcode::Load:
  case Opcode::GEP:
  case Opcode::Select:
    return true;
  default:
    return false;
  }
}

struct ProvenI32Range {
  bool valid = false;
  int64_t minimum = 0;
  int64_t maximum = 0;
};

bool rangeFitsI32(int64_t minimum, int64_t maximum) {
  return minimum >= std::numeric_limits<int32_t>::min() &&
         maximum <= std::numeric_limits<int32_t>::max();
}

ProvenI32Range evaluateProvenRange(
    Value* value,
    const std::unordered_map<Value*, ProvenI32Range>& known,
    std::unordered_set<Value*>& active,
    int depth = 0) {
  if (!value || depth > 32)
    return {};
  auto knownIt = known.find(value);
  if (knownIt != known.end())
    return knownIt->second;
  if (auto* constant = dynamic_cast<ConstInt*>(value))
    return {true, constant->value(), constant->value()};
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !active.insert(value).second)
    return {};

  ProvenI32Range result;
  if ((inst->opcode() == Opcode::Add || inst->opcode() == Opcode::Sub ||
       inst->opcode() == Opcode::Mul) &&
      inst->numOperands() == 2) {
    ProvenI32Range lhs =
        evaluateProvenRange(inst->operand(0), known, active, depth + 1);
    ProvenI32Range rhs =
        evaluateProvenRange(inst->operand(1), known, active, depth + 1);
    if (lhs.valid && rhs.valid) {
      int64_t minimum = 0;
      int64_t maximum = 0;
      if (inst->opcode() == Opcode::Add) {
        minimum = lhs.minimum + rhs.minimum;
        maximum = lhs.maximum + rhs.maximum;
      } else if (inst->opcode() == Opcode::Sub) {
        minimum = lhs.minimum - rhs.maximum;
        maximum = lhs.maximum - rhs.minimum;
      } else {
        const int64_t candidates[] = {
            lhs.minimum * rhs.minimum, lhs.minimum * rhs.maximum,
            lhs.maximum * rhs.minimum, lhs.maximum * rhs.maximum};
        minimum = *std::min_element(std::begin(candidates),
                                    std::end(candidates));
        maximum = *std::max_element(std::begin(candidates),
                                    std::end(candidates));
      }
      if (rangeFitsI32(minimum, maximum))
        result = {true, minimum, maximum};
    }
  } else if (inst->opcode() == Opcode::Shl && inst->numOperands() == 2) {
    auto* shift = dynamic_cast<ConstInt*>(inst->operand(1));
    ProvenI32Range input =
        evaluateProvenRange(inst->operand(0), known, active, depth + 1);
    if (shift && shift->value() >= 0 && shift->value() <= 30 && input.valid) {
      const int64_t factor = int64_t{1} << shift->value();
      const int64_t minimum = input.minimum * factor;
      const int64_t maximum = input.maximum * factor;
      if (rangeFitsI32(minimum, maximum))
        result = {true, minimum, maximum};
    }
  } else if (inst->opcode() == Opcode::Neg && inst->numOperands() == 1) {
    ProvenI32Range input =
        evaluateProvenRange(inst->operand(0), known, active, depth + 1);
    if (input.valid && input.minimum != std::numeric_limits<int32_t>::min())
      result = {true, -input.maximum, -input.minimum};
  }
  active.erase(value);
  return result;
}

bool loadIsProvedInBounds(
    Instruction* load,
    const std::unordered_map<Value*, ProvenI32Range>& knownRanges) {
  if (!load || load->opcode() != Opcode::Load || load->numOperands() != 1 ||
      !load->type().isI32()) {
    return false;
  }
  Value* pointer = load->operand(0);
  int64_t minimumOffset = 0;
  int64_t maximumOffset = 0;
  std::unordered_set<Value*> visitedPointers;
  while (auto* gep = dynamic_cast<Instruction*>(pointer)) {
    if (!visitedPointers.insert(pointer).second || gep->opcode() != Opcode::GEP ||
        gep->numOperands() < 2) {
      return false;
    }
    for (int index = 1; index < gep->numOperands(); ++index) {
      std::unordered_set<Value*> active;
      ProvenI32Range offset = evaluateProvenRange(
          gep->operand(index), knownRanges, active);
      if (!offset.valid)
        return false;
      minimumOffset += offset.minimum;
      maximumOffset += offset.maximum;
    }
    pointer = gep->operand(0);
  }
  auto* root = dynamic_cast<core_ir::GlobalVar*>(pointer);
  return root && minimumOffset >= 0 && maximumOffset >= minimumOffset &&
         maximumOffset + 4 <= root->sizeBytes();
}

// 在条件分支前准备纯值。then 块中的值会随着该块移除而失去使用位置，
// 因此先按拓扑顺序复制完整的依赖关系；已经支配目标块的值（包括循环归纳
// 变量 PHI）直接复用。依赖中出现未处理的操作时停止这条转换路径。
Value* materializeSpeculatableValue(
    Value* value, BasicBlock* target, Instruction* insertBefore,
    const analysis::DomSet& doms,
    std::unordered_map<Instruction*, Value*>& clones,
    std::unordered_set<Instruction*>& active, int depth = 0) {
  if (!value || !target || !insertBefore || depth > 32)
    return nullptr;
  if (valueAvailableAt(value, target, doms))
    return value;

  auto* inst = dynamic_cast<Instruction*>(value);
  if (!isSpeculatableOpcode(inst)) {
    if (traceEnabled()) {
      std::cerr << "[if-convert] materialize-reject op="
                << (inst ? static_cast<int>(inst->opcode()) : -1)
                << " name=" << (inst ? inst->name() : "null") << "\n";
    }
    return nullptr;
  }
  auto cached = clones.find(inst);
  if (cached != clones.end())
    return cached->second;
  if (!active.insert(inst).second)
    return nullptr;

  std::vector<Value*> operands;
  operands.reserve(inst->numOperands());
  for (Value* operand : inst->operands()) {
    Value* materialized = materializeSpeculatableValue(
        operand, target, insertBefore, doms, clones, active, depth + 1);
    if (!materialized) {
      if (traceEnabled()) {
        std::cerr << "[if-convert] materialize-operand-reject parent="
                  << inst->name() << "\n";
      }
      active.erase(inst);
      return nullptr;
    }
    operands.push_back(materialized);
  }

  auto* clone = new Instruction(inst->opcode(), inst->type(),
                                inst->name() + ".ifc.spec");
  for (Value* operand : operands)
    clone->addOperand(operand);
  target->insertBefore(insertBefore, clone);
  clones.emplace(inst, clone);
  active.erase(inst);
  return clone;
}

bool strictSpeculationCostAllowed(
    Value* value, BasicBlock* target, const analysis::DomSet& doms,
    const std::unordered_map<Value*, ProvenI32Range>& knownRanges,
    std::unordered_set<Instruction*>& active, int depth = 0) {
  if (!value || !target || depth > 32)
    return false;
  if (valueAvailableAt(value, target, doms))
    return true;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !isSpeculatableOpcode(inst) || !active.insert(inst).second)
    return false;
  // 只允许把已证明落在 direct global object 内的 i32 load 提前到条件前。
  // 这保留了合法程序的 trap 行为，也不依赖运行时输入分布；参数指针、动态
  // 上界、可能回绕的地址表达式一律保守拒绝。
  if (inst->opcode() == Opcode::Load) {
    const bool proved = loadIsProvedInBounds(inst, knownRanges);
    active.erase(inst);
    return proved;
  }
  for (Value* operand : inst->operands()) {
    if (!strictSpeculationCostAllowed(operand, target, doms, knownRanges, active,
                                      depth + 1)) {
      active.erase(inst);
      return false;
    }
  }
  active.erase(inst);
  return true;
}

bool blockHasNoExternalUses(BasicBlock* block, Instruction* exempt) {
  if (!block)
    return false;
  for (const auto& ip : block->instructions()) {
    auto* inst = ip.get();
    if (!inst || inst == exempt)
      continue;
    for (Instruction* user : inst->uses()) {
      if (!user || user->parent() != block)
        return false;
    }
  }
  return true;
}

bool thenBlockHasNoSideEffects(BasicBlock* block) {
  if (!block)
    return false;
  for (const auto& ip : block->instructions()) {
    auto* inst = ip.get();
    if (!inst || inst->isTerminator())
      continue;
    if (inst->hasSideEffects())
      return false;
  }
  return true;
}

void detachBlockInstructions(BasicBlock* block) {
  if (!block)
    return;
  std::vector<Instruction*> instructions;
  instructions.reserve(block->instructions().size());
  for (const auto& ip : block->instructions())
    if (ip)
      instructions.push_back(ip.get());
  for (Instruction* inst : instructions)
    inst->dropAllOperands();
}

bool updateUsesStayInLoop(Instruction* update,
                          const analysis::NaturalLoop& natural) {
  if (!update)
    return false;
  for (Instruction* user : update->uses()) {
    if (!user || !user->parent() ||
        !natural.blocks.count(user->parent()) ||
        user->opcode() != Opcode::Phi)
      return false;
  }
  return true;
}

bool phiContainsValue(Instruction* phi, Value* value) {
  if (!phi || phi->opcode() != Opcode::Phi || !value)
    return false;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2)
    if (phi->operand(i) == value)
      return true;
  return false;
}

bool phiHasOnlyUser(Instruction* phi, Instruction* allowed) {
  if (!phi || !allowed)
    return false;
  for (Instruction* user : phi->uses())
    if (user != allowed)
      return false;
  return true;
}

bool findAccumulatorIncoming(Instruction* accPhi, Instruction* accAdd,
                             const analysis::NaturalLoop& natural,
                             int& incomingIndex) {
  if (!accPhi || !accAdd)
    return false;
  for (int i = 0; i + 1 < accPhi->numOperands(); i += 2) {
    if (accPhi->operand(i) == accAdd) {
      incomingIndex = i;
      return true;
    }
    auto* mergePhi = dynamic_cast<Instruction*>(accPhi->operand(i));
    if (!mergePhi || mergePhi->opcode() != Opcode::Phi ||
        !phiContainsValue(mergePhi, accAdd) ||
        !natural.blocks.count(mergePhi->parent()) ||
        !phiHasOnlyUser(mergePhi, accPhi))
      continue;
    // 旧累加器通过合流 PHI 传递。只有该合流 PHI 只服务于 accPhi 时，才把
    // 它并入新的边。
    incomingIndex = i;
    return true;
  }
  return false;
}

bool processLoop(Function* func, const analysis::NaturalLoop& natural,
                 const std::vector<const analysis::NaturalLoop*>& enclosingLoops,
                 bool strictMode) {
  trace("processLoop", natural.header);
  BasicBlock* preheader = analysis::dedicatedPreheader(natural);
  if (!preheader) {
    trace("skip: no preheader", natural.header);
    return false;
  }

  std::vector<BasicBlock*> blocks;
  std::unordered_set<BasicBlock*> reachable;
  for (auto& block : func->blocks()) {
    blocks.push_back(block.get());
    reachable.insert(block.get());
  }
  const analysis::DomSet doms =
      analysis::computeDominators(func, blocks, reachable);

  std::unordered_map<Value*, ProvenI32Range> knownRanges;
  for (const analysis::NaturalLoop* enclosing : enclosingLoops) {
    if (!enclosing)
      continue;
    BasicBlock* enclosingPreheader = analysis::dedicatedPreheader(*enclosing);
    CanonicalLoop canonical;
    if (!enclosingPreheader ||
        !recognizeCanonicalLoop(*enclosing, enclosingPreheader, canonical) ||
        !canonical.ivIsCmpLhs || canonicalLoopCmpCond(canonical.cmp) != "lt") {
      continue;
    }
    auto* initial = dynamic_cast<ConstInt*>(canonical.initVal);
    auto* bound = dynamic_cast<ConstInt*>(canonical.boundVal);
    if (!initial || !bound || initial->value() < 0 ||
        bound->value() <= initial->value()) {
      continue;
    }
    knownRanges[canonical.ivPhi] =
        {true, initial->value(), static_cast<int64_t>(bound->value()) - 1};
  }

  // 查找从循环前置块以 0 初始化的累加器 PHI。
  Instruction* accPhi = nullptr;
  for (auto& ip : natural.header->instructions()) {
    auto* inst = ip.get();
    if (!inst || inst->opcode() != Opcode::Phi) break;
    for (int i = 0; i + 1 < inst->numOperands(); i += 2) {
      auto* pred = dynamic_cast<BasicBlock*>(inst->operand(i + 1));
      if (pred != preheader) continue;
      auto* initConst = dynamic_cast<ConstInt*>(inst->operand(i));
      if (initConst && initConst->value() == 0) {
        accPhi = inst;
        break;
      }
    }
    if (accPhi) break;
  }
  if (!accPhi) {
    trace("skip: no accPhi", natural.header);
    return false;
  }
  trace("accPhi-ok", natural.header);

  // 只处理具有规范归纳变量、偶数常量上界且上界至少为 32 的循环，
  // 让这条路径与 reduction_split 的处理范围保持一致。
  {
    CanonicalLoop cl;
    if (!recognizeCanonicalLoop(natural, preheader, cl)) {
      trace("skip: no canonical loop", natural.header);
      return false;
    }
    auto* kb = dynamic_cast<ConstInt*>(cl.boundVal);
    if (!kb || kb->value() < 32 || kb->value() % 2 != 0) {
      if (traceEnabled())
        std::cerr << "[if-convert] skip: bound="
                  << (kb ? kb->value() : -1) << std::endl;
      return false;
    }
  }
  trace("bound-ok, scanning blocks", natural.header);

  // 扫描条件累加模式。
  for (auto* bb : natural.blocks) {
    if (bb == natural.header) continue;
    auto preds = bb->preds();
    if (preds.size() != 1) continue;
    BasicBlock* condBB = preds[0];
    if (condBB == natural.header) continue;
    if (!natural.blocks.count(condBB)) continue;
    Instruction* condTerm = condBB->terminator();
    if (!condTerm || condTerm->opcode() != Opcode::CondBr) continue;
    BasicBlock* thenBB = dynamic_cast<BasicBlock*>(condTerm->operand(1));
    if (thenBB != bb) continue;
    BasicBlock* elseBB = dynamic_cast<BasicBlock*>(condTerm->operand(2));
    Value* cond = condTerm->operand(0);
    if (traceEnabled())
      std::cerr << "[if-convert] candidate: " << bb->name()
                << " in " << condBB->name()
                << " else=" << (elseBB ? elseBB->name() : "null")
                << " inLoop=" << (elseBB ? natural.blocks.count(elseBB) : 0)
                << " bool=" << isBooleanValue(cond)
                << std::endl;
    if (!elseBB || !natural.blocks.count(elseBB) ||
        !isBooleanValue(cond))
      continue;

    // 在 then 块中查找 accNext = accPhi + val，同时处理直接引用和合流
    // PHI 间接引用两种形式。
    Instruction* accAdd = nullptr;
    Value* addVal = nullptr;
    for (auto& ip : bb->instructions()) {
      auto* inst = ip.get();
      if (!inst || inst->opcode() != Opcode::Add) continue;
      for (int side = 0; side < 2; ++side) {
        Value* op = inst->operand(side);
        if (op == accPhi) {
          accAdd = inst;
          addVal = inst->operand(1 - side);
          break;
        }
        // 通过合流处的 Phi 传递结果。
        auto* opPhi = dynamic_cast<Instruction*>(op);
        if (opPhi && opPhi->opcode() == Opcode::Phi) {
          for (int pi = 0; pi + 1 < opPhi->numOperands(); pi += 2) {
            if (opPhi->operand(pi) == accPhi) {
              accAdd = inst;
              addVal = inst->operand(1 - side);
              break;
            }
          }
        }
        if (accAdd) break;
      }
      if (accAdd) break;
    }
    if (!accAdd || !addVal || !accAdd->type().isI32() ||
        !addVal->type().isI32())
      continue;

    // 确认 accAdd 会回流到 accPhi，也处理一层合流 PHI 的形式。合流 PHI
    // 还有其他使用者时不合并。
    int accPhiIncoming = -1;
    if (!findAccumulatorIncoming(accPhi, accAdd, natural,
                                 accPhiIncoming)) {
      trace("skip: accAdd doesn't feed accPhi", bb);
      continue;
    }

    if (!updateUsesStayInLoop(accAdd, natural)) {
      trace("skip: accAdd has non-Phi or external use", bb);
      continue;
    }
    if (!blockHasNoExternalUses(bb, accAdd)) {
      trace("skip: then block has external uses", bb);
      continue;
    }
    if (!thenBlockHasNoSideEffects(bb)) {
      trace("skip: then block has side effects", bb);
      continue;
    }
    if (!thenBlockHasNoSideEffects(elseBB)) {
      trace("skip: else block has side effects", elseBB);
      continue;
    }
    if (strictMode) {
      std::unordered_set<Instruction*> active;
      if (!strictSpeculationCostAllowed(addVal, condBB, doms, knownRanges,
                                        active)) {
        trace("skip: strict cost gate", bb);
        continue;
      }
    }
    std::unordered_map<Instruction*, Value*> materialized;
    std::unordered_set<Instruction*> active;
    Value* selectVal = materializeSpeculatableValue(
        addVal, condBB, condTerm, doms, materialized, active);
    if (!selectVal) {
      trace("skip: add value cannot be materialized", bb);
      continue;
    }

    trace("found", bb);

    // 第一步：在 condBB 的分支前构造 select(cond, materializedAddVal, 0)。
    // 物化后的加数不再依赖下面将要移除的代码块。
    auto* zero = func->parent()->getConstInt(0);
    auto* selInst = new Instruction(Opcode::Select, selectVal->type(), "ifc.sel");
    selInst->addOperand(cond);
    selInst->addOperand(selectVal);
    selInst->addOperand(zero);
    condBB->insertBefore(condTerm, selInst);

    // 第二步：在 condBB 中构造 accNextNew = accPhi + selectVal。
    auto* accNextNew = new Instruction(Opcode::Add, accPhi->type(), "ifc.acc");
    accNextNew->addOperand(accPhi);
    accNextNew->addOperand(selInst);
    condBB->insertBefore(condTerm, accNextNew);

    // 第三步：更新 accPhi 的回边输入。旧值若经过合流 PHI 传递，就直接替换
    // 循环头部的边；第四步会处理待清理合流节点的输入。
    accPhi->setOperand(accPhiIncoming, accNextNew);
    accPhi->setOperand(accPhiIncoming + 1, condBB);

    // 第四步：更新所有引用旧 accAdd 的 PHI，同时替换值和对应的前驱块。
    for (auto* loopBB : natural.blocks) {
      for (auto& ip : loopBB->instructions()) {
        auto* inst = ip.get();
        if (!inst || inst->opcode() != Opcode::Phi) continue;
        for (int i = 0; i + 1 < inst->numOperands(); i += 2) {
          if (inst->operand(i) == accAdd) {
            inst->setOperand(i, accNextNew);
            inst->setOperand(i + 1, condBB);  // 同时更新前驱块
          }
        }
      }
    }

    // 第五步：从 then 块移除 accAdd。
    bb->remove(accAdd);

    // 第六步：将条件分支改为指向 elseBB 的无条件分支。
    auto* uncondBr = new Instruction(Opcode::Br, Type::voidTy());
    uncondBr->addOperand(elseBB);
    condBB->remove(condTerm);
    condBB->pushBack(uncondBr);

    // 第七步：合并 condBB 和 elseBB，移动指令前先处理 PHI。
    // 7a：先移除 then 块，避免 replaceAllUsesWith 触碰待删除指令；先清理
    //     操作数的使用关系，Function::removeBlock 负责最终删除节点。
    detachBlockInstructions(bb);
    func->removeBlock(bb);
    // 7b：折叠 elseBB 中的合流 PHI，此时各输入都来自 condBB 的 accNextNew。
    {
      std::vector<Instruction*> elsePhis;
      for (auto& ip : elseBB->instructions()) {
        auto* phi = ip.get();
        if (!phi || phi->opcode() != Opcode::Phi) break;
        elsePhis.push_back(phi);
      }
      for (auto* phi : elsePhis) {
        // accPhi 已直接改用 accNextNew 后，原 diamond 的累加合流
        // Phi 可能变成无使用。若将它随其他指令移入 condBB，会形成
        // non-Phi 之后的非法 Phi；在所有 use 已闭合时直接删除。
        if (phi->uses().empty()) {
          elseBB->remove(phi);
          continue;
        }
        Value* common = phi->operand(0);
        bool allSame = true;
        for (int i = 2; i + 1 < phi->numOperands(); i += 2) {
          if (phi->operand(i) != common) { allSame = false; break; }
        }
        if (allSame && common) {
          phi->replaceAllUsesWith(common);
          elseBB->remove(phi);
        }
      }
    }
    // 7c：移除 condBB 中的无条件分支。
    condBB->remove(uncondBr);
    // 7d：将 elseBB 中剩余指令移动到 condBB，此时其中已没有 PHI。
    {
      auto& elseInsts = elseBB->instructions();
      for (auto it = elseInsts.begin(); it != elseInsts.end();) {
        auto moved = std::move(*it);
        it = elseInsts.erase(it);
        condBB->pushBack(moved.release());
      }
    }
    // 7e：更新循环头 PHI，把 elseBB 前驱替换为 condBB。
    for (auto& ip : natural.header->instructions()) {
      auto* phi = ip.get();
      if (!phi || phi->opcode() != Opcode::Phi) break;
      for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
        auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
        if (pred == elseBB)
          phi->setOperand(i + 1, condBB);
      }
    }
    // 7f：删除空的 elseBB。
    func->removeBlock(elseBB);

    // 多个条件累加依次合并时，后一次可能复制出与前一次
    // 同址的 load，而 use 已被共享值替换。只删除已由同一边界证明
    // 为 direct-global in-bounds 的无使用 load，不吞掉可能 trap。
    std::vector<Instruction*> deadLoads;
    for (const auto& instPtr : condBB->instructions()) {
      Instruction* candidate = instPtr.get();
      if (candidate && candidate->opcode() == Opcode::Load &&
          candidate->uses().empty() &&
          loadIsProvedInBounds(candidate, knownRanges)) {
        deadLoads.push_back(candidate);
      }
    }
    for (Instruction* deadLoad : deadLoads)
      condBB->remove(deadLoad);

    func->recomputeCFG();
    trace("converted+merged", condBB);
    return true;
  }
  return false;
}

bool processFunction(Function* func, analysis::AnalysisManager& am,
                     bool strictMode) {
  if (!func || func->blocks().empty()) return false;
  func->recomputeCFG();
  analysis::FunctionAnalysisCache& cache = am.function(func);
  const auto& forest = cache.loopForest();

  // 只处理三层嵌套循环（深度至少为 2），以匹配矩阵乘法形式，
  // 避免在不适合 reduction_split 的循环上增加额外工作。
  for (size_t ii = 0; ii < forest.nodes().size(); ++ii) {
    auto* node = forest.node(static_cast<int>(ii));
    if (!node || !node->loop) continue;
    if (!node->children.empty()) continue;  // 只处理最内层循环
    // 检查父循环和祖父循环是否存在，以确认嵌套层数。
    if (node->parent < 0) continue;
    auto* parentNode = forest.node(node->parent);
    if (!parentNode || parentNode->parent < 0) continue;
    std::vector<const analysis::NaturalLoop*> enclosingLoops;
    for (int cursor = static_cast<int>(ii); cursor >= 0;) {
      auto* enclosingNode = forest.node(cursor);
      if (!enclosingNode || !enclosingNode->loop)
        break;
      enclosingLoops.push_back(enclosingNode->loop);
      cursor = enclosingNode->parent;
    }
    if (processLoop(func, *node->loop, enclosingLoops, strictMode))
      return true;
  }
  return false;
}

} // namespace

bool applyIfConvertAccumulation(Module& mod, analysis::AnalysisManager& am,
                                bool strictMode) {
  bool changed = false;
  for (int iter = 0; iter < 16; ++iter) {
    bool iterChanged = false;
    for (auto& fp : mod.functions()) {
      if (processFunction(fp.get(), am, strictMode)) {
        iterChanged = true;
        // 变换删除基本块后，清除过期的循环和 CFG 信息。
        am.invalidateFunction(fp.get(), analysis::AnalysisInvalidation::CFG);
        break;  // 使用新的分析结果重新开始遍历
      }
    }
    if (!iterChanged) break;
    changed = true;
  }
  return changed;
}

} // namespace hir
