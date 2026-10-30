// 文件职责：规约循环寄存器分块（reduction splitting）。
// 将内层归约循环的中层维度步长改为 2，使内层循环体同时计算两个独立累加器
// 并共享其中一个数组装载（src1），从而减少访存压力并提高指令级并行度。
// 仅对偶数常量中层边界生效（v1）；整数加法结合律安全。
// 调试：SYC_TRACE_REDUCTION_SPLIT=1。

#include "reduction_split.h"

#include "../analysis/analysis_manager.h"
#include "../analysis/cfg_utils.h"
#include "../analysis/loop_info.h"
#include "../research_switch.h"
#include "canonical_loop.h"
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hir {
namespace {

using core_ir::BasicBlock;
using core_ir::ConstInt;
using core_ir::Function;
using core_ir::GlobalVar;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Type;
using core_ir::Value;

bool traceEnabled() { return sycEnvironmentValue("SYC_TRACE_REDUCTION_SPLIT") != nullptr; }

void trace(const char* msg, BasicBlock* hdr = nullptr, const char* extra = "") {
  if (!traceEnabled()) return;
  std::cerr << "[reduction-split] " << msg
            << (hdr ? " header=" + hdr->name() : std::string())
            << " " << extra << "\n";
}

// 识别 GEP(root, index) 地址并检查 index 是否引用指定的 jphi。
struct GepInfo {
  GlobalVar* root = nullptr;
  Value* index = nullptr;
  bool valid = false;
};

GepInfo parseGep(Value* addr) {
  GepInfo g;
  auto* inst = dynamic_cast<Instruction*>(addr);
  if (!inst || inst->opcode() != Opcode::GEP || inst->numOperands() != 2)
    return g;
  g.root = dynamic_cast<GlobalVar*>(inst->operand(0));
  g.index = inst->operand(1);
  g.valid = g.root != nullptr && g.root->elemTy() == Type::i32();
  return g;
}

bool valueUsesPhi(Value* v, Instruction* phi, int depth = 0) {
  if (!v || depth > 20) return false;
  if (v == phi) return true;
  auto* inst = dynamic_cast<Instruction*>(v);
  if (!inst) return false;
  for (int k = 0; k < inst->numOperands(); ++k)
    if (valueUsesPhi(inst->operand(k), phi, depth + 1))
      return true;
  return false;
}

struct TargetCostModel {
  int issueWidth = 2;
  int setupIssueSlots = 4;
  int savedLoadIssueSlots = 1;
  int safetyFactor = 4;

  int64_t minimumTripCount() const {
    const int64_t numerator = static_cast<int64_t>(setupIssueSlots) *
                              issueWidth * safetyFactor;
    return std::max<int64_t>(2, (numerator + savedLoadIssueSlots - 1) /
                                    savedLoadIssueSlots);
  }

  bool profitable(int64_t tripCount) const {
    return tripCount > 0 && (tripCount & 1) == 0 &&
           tripCount >= minimumTripCount();
  }
};

const TargetCostModel& targetCostModel() {
  static const TargetCostModel model;
  return model;
}

Instruction* cloneInstructionShell(Instruction* source) {
  if (!source)
    return nullptr;
  std::string name = source->name();
  if (source->opcode() != Opcode::Call)
    name = name.empty() ? "reduction.version" : name + ".reduction.version";
  auto* clone = new Instruction(source->opcode(), source->type(), name);
  clone->setMemoryInfo(source->memoryElemTy(), source->memorySizeBytes(),
                       source->isArrayObject(), source->dims());
  return clone;
}

Value* remapClonedValue(
    Value* value, const std::unordered_map<Value*, Value*>& valueMap,
    const std::unordered_map<BasicBlock*, BasicBlock*>& blockMap) {
  if (auto* block = dynamic_cast<BasicBlock*>(value)) {
    auto blockIt = blockMap.find(block);
    if (blockIt != blockMap.end())
      return blockIt->second;
  }
  auto valueIt = valueMap.find(value);
  return valueIt == valueMap.end() ? value : valueIt->second;
}

Value* phiIncomingFrom(Instruction* phi, BasicBlock* predecessor) {
  if (!phi || !predecessor)
    return nullptr;
  for (int index = 0; index + 1 < phi->numOperands(); index += 2) {
    if (phi->operand(index + 1) == predecessor)
      return phi->operand(index);
  }
  return nullptr;
}

bool loopCanBeCloned(const analysis::NaturalLoop& loop) {
  for (auto* block : loop.blocks) {
    if (!block)
      return false;
    for (auto& instruction : block->instructions()) {
      if (!instruction)
        return false;
      switch (instruction->opcode()) {
      case Opcode::Alloca:
      case Opcode::Call:
      case Opcode::Ret:
        return false;
      default:
        break;
      }
    }
  }
  return true;
}

bool cloneLoopBlocks(
    Function* function, const analysis::NaturalLoop& loop,
    std::unordered_map<BasicBlock*, BasicBlock*>& blockMap,
    std::unordered_map<Value*, Value*>& valueMap) {
  if (!function || !loop.header || !loopCanBeCloned(loop))
    return false;

  std::vector<BasicBlock*> orderedBlocks;
  orderedBlocks.reserve(loop.blocks.size());
  for (auto& block : function->blocks()) {
    if (loop.blocks.count(block.get()))
      orderedBlocks.push_back(block.get());
  }
  if (orderedBlocks.size() != loop.blocks.size())
    return false;

  for (auto* source : orderedBlocks)
    blockMap[source] = function->newBlock("reduction.version");

  for (auto* source : orderedBlocks) {
    BasicBlock* destination = blockMap.at(source);
    for (auto& instruction : source->instructions()) {
      Instruction* clone = cloneInstructionShell(instruction.get());
      if (!clone)
        return false;
      destination->pushBack(clone);
      valueMap[instruction.get()] = clone;
    }
  }

  for (auto* source : orderedBlocks) {
    BasicBlock* destination = blockMap.at(source);
    for (size_t index = 0; index < source->instructions().size(); ++index) {
      Instruction* sourceInstruction = source->instructions()[index].get();
      Instruction* destinationInstruction =
          destination->instructions()[index].get();
      for (int operandIndex = 0;
           operandIndex < sourceInstruction->numOperands(); ++operandIndex) {
        destinationInstruction->addOperand(remapClonedValue(
            sourceInstruction->operand(operandIndex), valueMap, blockMap));
      }
    }
  }
  return true;
}

bool addClonedExitPhiIncoming(
    const analysis::NaturalLoop& loop,
    const std::unordered_map<BasicBlock*, BasicBlock*>& blockMap,
    const std::unordered_map<Value*, Value*>& valueMap) {
  for (auto* source : loop.blocks) {
    auto sourceIt = blockMap.find(source);
    if (!source || sourceIt == blockMap.end())
      return false;
    for (auto* successor : source->succs()) {
      if (!successor || loop.blocks.count(successor))
        continue;
      for (auto* phi : analysis::collectLeadingPhi(successor)) {
        Value* incoming = phiIncomingFrom(phi, source);
        if (!incoming)
          return false;
        phi->addOperand(remapClonedValue(incoming, valueMap, blockMap));
        phi->addOperand(sourceIt->second);
      }
    }
  }
  return true;
}

analysis::NaturalLoop mapClonedLoop(
    const analysis::NaturalLoop& loop,
    const std::unordered_map<BasicBlock*, BasicBlock*>& blockMap) {
  analysis::NaturalLoop cloned;
  auto headerIt = blockMap.find(loop.header);
  if (headerIt == blockMap.end())
    return cloned;
  cloned.header = headerIt->second;
  for (auto* block : loop.blocks) {
    auto blockIt = blockMap.find(block);
    if (blockIt == blockMap.end())
      return {};
    cloned.blocks.insert(blockIt->second);
  }
  for (auto* latch : loop.latches) {
    auto latchIt = blockMap.find(latch);
    if (latchIt == blockMap.end())
      return {};
    cloned.latches.push_back(latchIt->second);
  }
  return cloned;
}

BasicBlock* createForwardingPreheader(Function* function, BasicBlock* target) {
  if (!function || !target)
    return nullptr;
  BasicBlock* preheader = function->newBlock("reduction.version.preheader");
  auto* branch = new Instruction(Opcode::Br, Type::voidTy());
  branch->addOperand(target);
  preheader->pushBack(branch);
  return preheader;
}

bool isAlreadyVersionedPreheader(BasicBlock* preheader) {
  if (!preheader || preheader->preds().size() != 1)
    return false;
  BasicBlock* dispatch = preheader->preds().front();
  Instruction* term = dispatch ? dispatch->terminator() : nullptr;
  if (!term || term->opcode() != Opcode::CondBr || term->numOperands() != 3 ||
      term->name() != "reduction.version.dispatch")
    return false;
  auto* first = dynamic_cast<BasicBlock*>(term->operand(1));
  auto* second = dynamic_cast<BasicBlock*>(term->operand(2));
  BasicBlock* sibling = first == preheader ? second :
                        (second == preheader ? first : nullptr);
  Instruction* siblingTerm = sibling ? sibling->terminator() : nullptr;
  return siblingTerm && siblingTerm->opcode() == Opcode::Br &&
         siblingTerm->numOperands() == 1;
}

bool installRuntimeBoundDispatch(Module& module, BasicBlock* preheader,
                                 Value* bound, BasicBlock* fastPreheader,
                                 BasicBlock* slowPreheader) {
  if (!preheader || !bound || !bound->type().isI32() || !fastPreheader ||
      !slowPreheader)
    return false;
  Instruction* term = preheader->terminator();
  if (!term || term->opcode() != Opcode::Br || term->numOperands() != 1)
    return false;

  auto* parity = new Instruction(Opcode::And, Type::i32(), "reduction.parity");
  parity->addOperand(bound);
  parity->addOperand(module.getConstInt(1));
  preheader->insertBefore(term, parity);
  auto* even = new Instruction(Opcode::ICmp, Type::i32(), "reduction.even");
  even->addOperand(parity);
  even->addOperand(module.getConstInt(0));
  preheader->insertBefore(term, even);
  auto* large = new Instruction(Opcode::ICmp, Type::i32(), "reduction.large");
  large->addOperand(bound);
  large->addOperand(module.getConstInt(
      static_cast<int32_t>(targetCostModel().minimumTripCount())));
  preheader->insertBefore(term, large);
  auto* fast = new Instruction(Opcode::And, Type::i32(), "reduction.fast");
  fast->addOperand(even);
  fast->addOperand(large);
  preheader->insertBefore(term, fast);
  preheader->remove(term);
  auto* dispatch = new Instruction(Opcode::CondBr, Type::voidTy(),
                                   "reduction.version.dispatch");
  dispatch->addOperand(fast);
  dispatch->addOperand(fastPreheader);
  dispatch->addOperand(slowPreheader);
  preheader->pushBack(dispatch);
  return true;
}

struct KLoopMatch {
  CanonicalLoop kLoop;
  BasicBlock* kHeader = nullptr;
  BasicBlock* kLatch = nullptr;
  Instruction* kIv = nullptr;
  Instruction* accPhi = nullptr;
  Instruction* accNext = nullptr;  // 锁存块中的 accPhi + expr
  Instruction* aLoad = nullptr;    // 不依赖 j 的 load（共享）
  Value* bAddr = nullptr;          // b-load 的地址（依赖 j，需 per-lane）
  Instruction* bLoad = nullptr;
  int64_t kBoundVal = 0;
};

KLoopMatch matchKLoop(const analysis::NaturalLoop& natural,
                      CanonicalLoop jLoop,
                      Value* provenRuntimeBound = nullptr) {
  KLoopMatch km;
  trace("matchKLoop-enter", natural.header);
  BasicBlock* preheader = analysis::dedicatedPreheader(natural);
  if (!preheader) {
    trace("no-preheader", natural.header);
    return km;
  }
  CanonicalLoop kLoop;
  if (!recognizeCanonicalLoop(natural, preheader, kLoop)) {
    trace("no-canonical", natural.header);
    return km;
  }
  if (!kLoop.latch) {
    trace("no-latch", kLoop.header);
    return km;
  }
  trace("canonical-ok", kLoop.header);
  // k 循环体包含多个基本块，当前先保持原结构。
  if (kLoop.body && kLoop.body != kLoop.latch) {
    trace("multi-block-body", kLoop.header);
    return km;
  }
  auto* kb = dynamic_cast<ConstInt*>(kLoop.boundVal);
  const bool hasProvenRuntimeBound =
      provenRuntimeBound && kLoop.boundVal == provenRuntimeBound;
  if (!kb && !hasProvenRuntimeBound) {
    trace("bound-not-const", kLoop.header);
    return km;
  }
  if (kb && (kb->value() <= 0 || kb->value() % 2 != 0)) {
    trace("bound-not-even", kLoop.header);
    return km;
  }
  // 成本门控：仅对 N ≥ 32 的大幅面循环启用（寄存器分块摊销门槛，
  // 防止对 verify suite 小循环产生退化）。
  if (kb && !targetCostModel().profitable(kb->value())) {
    trace("bound-too-small", kLoop.header, std::to_string(kb->value()).c_str());
    return KLoopMatch();
  }
  trace("bound-ok", kLoop.header,
        kb ? std::to_string(kb->value()).c_str() : "runtime-proven");
  km.kBoundVal = kb ? kb->value() : 0;
  km.kLoop = kLoop;
  km.kHeader = kLoop.header;
  km.kLatch = kLoop.latch;
  km.kIv = kLoop.ivPhi;

  // 循环头 PHI：一个 kIv 和一个累加器 PHI。
  trace("check-header-phis", kLoop.header);
  Instruction* otherPhi = nullptr;
  for (auto& ip : kLoop.header->instructions()) {
    auto* inst = ip.get();
    if (!inst) continue;
    if (inst->opcode() != Opcode::Phi) break;
    if (inst == kLoop.ivPhi) continue;
    if (otherPhi) {
      trace("too-many-phis", kLoop.header);
      return km; // PHI 数量超过预期
    }
    otherPhi = inst;
  }
  if (!otherPhi) {
    trace("no-accPhi", kLoop.header);
    return km;
  }
  trace("check-acc-init", kLoop.header);
  // 初始值可以是任意值，不限定为 0。拆分后 lane0 使用原始初值，lane1
  // 从 0 开始，分别保存两条累加结果。
  Value* accInitVal = otherPhi->operand(0);
  if (!accInitVal) {
    trace("acc-init-null", kLoop.header);
    return km;
  }
  km.accPhi = otherPhi;
  trace("accPhi-ok", kLoop.header);

  // 在循环体中查找 loadA（地址不依赖 jLoop.ivPhi）、loadB 和 accNext，
  // 并扫描自然循环的所有块以覆盖多块循环体。
  trace("check-body-loads", kLoop.header);
  Instruction* loadA = nullptr;
  Instruction* loadB = nullptr;
  Value* bAddr = nullptr;
  int loadCount = 0;
  for (auto* bb : natural.blocks) {
    if (bb == kLoop.header) continue;  // 跳过循环头部 PHI。
  for (auto& ip : bb->instructions()) {
    auto* inst = ip.get();
    if (!inst) continue;
    Opcode op = inst->opcode();
    if (op == Opcode::Br || op == Opcode::Phi) continue;
    if (op == Opcode::Load && inst->numOperands() >= 1) {
      Value* addr = inst->operand(0);
      if (!addr) continue;
      loadCount++;
      trace("load-found", kLoop.header,
            (std::string("addr=") + (addr ? "ok" : "null") + " #" + std::to_string(loadCount)).c_str());
      bool usesJphi = valueUsesPhi(addr, jLoop.ivPhi);
      trace("load-addr", kLoop.header, (std::to_string((uintptr_t)addr) + " usesJphi=" + (usesJphi?"Y":"N")).c_str());
      if (!loadA && !usesJphi) {
        loadA = inst;
        trace("aLoad-found", kLoop.header);
      } else if (!loadB && usesJphi) {
        loadB = inst;
        bAddr = addr;
        trace("bLoad-found", kLoop.header);
      }
    }
  }
  }  // 完成每个基本块的处理
  if (!loadA || !loadB || !bAddr) {
    trace("loads-matched", kLoop.header,
          (std::string("a=") + (loadA?"Y":"N") + " b=" + (loadB?"Y":"N")).c_str());
    return km;
  }
  km.aLoad = loadA;
  km.bLoad = loadB;
  km.bAddr = bAddr;
  trace("loads-ok", kLoop.header);

  // accNext: accPhi phi 的 latch incoming value，即每迭代的累加结果。
  // 不限 opcode（Add/Xor-And select 等均允许），只需 accPhi 的 latch
  // incoming 存在即可。
  trace("check-accNext", kLoop.header);
  Instruction* accNextCandidate = nullptr;
  for (int i = 0; i + 1 < otherPhi->numOperands(); i += 2) {
    auto* pred = dynamic_cast<BasicBlock*>(otherPhi->operand(i + 1));
    if (!pred) continue;
    bool isLatchPred = (pred == kLoop.latch);
    if (!isLatchPred) {
      for (auto* p : kLoop.header->preds())
        if (p == pred) { isLatchPred = true; break; }
    }
    if (pred == preheader || !isLatchPred) continue;
    Value* incoming = otherPhi->operand(i);
    auto* incomingInst = dynamic_cast<Instruction*>(incoming);
    if (incomingInst && natural.blocks.count(incomingInst->parent()) &&
        valueUsesPhi(incomingInst, otherPhi)) {
      accNextCandidate = incomingInst;
      trace("accNext-found", kLoop.header,
            (std::string("block=") + incomingInst->parent()->name()).c_str());
      break;
    }
  }
  if (!accNextCandidate) {
    trace("no-accNext", kLoop.header);
    return KLoopMatch();
  }
  // lane1 从加法单位元 0 开始，因此当前只拆分一个操作数为累加器的普通
  // 整数加法。这里不把其他运算或多个使用者带入双路改写。
  if (accNextCandidate->opcode() != Opcode::Add ||
      accNextCandidate->numOperands() != 2) {
    trace("acc-next-not-add", kLoop.header);
    return KLoopMatch();
  }
  const bool accIsLhs = accNextCandidate->operand(0) == otherPhi;
  const bool accIsRhs = accNextCandidate->operand(1) == otherPhi;
  if (accIsLhs == accIsRhs) {
    trace("acc-next-acc-use-shape", kLoop.header);
    return KLoopMatch();
  }
  Value* addend = accIsLhs ? accNextCandidate->operand(1)
                           : accNextCandidate->operand(0);
  if (!addend || valueUsesPhi(addend, otherPhi)) {
    trace("acc-next-addend-uses-acc", kLoop.header);
    return KLoopMatch();
  }
  trace("acc-next-add-ok", kLoop.header);
  km.accNext = accNextCandidate;

  // 纯 SSA 算术验证：只拒绝有副作用的指令（Call/Store/Alloca/Ret/向量访存），
  // 其余所有算术/比较/位运算/分支等纯 SSA 指令均允许。
  trace("check-side-effects", kLoop.header);
  for (auto& ip : kLoop.body->instructions()) {
    auto* inst = ip.get();
    if (!inst) {
      trace("null-inst", kLoop.header);
      return KLoopMatch();
    }
    Opcode op = inst->opcode();
    if (op == Opcode::Call || op == Opcode::Store || op == Opcode::Alloca ||
        op == Opcode::Ret  || op == Opcode::VLoad  || op == Opcode::VStore) {
      trace("side-effect-op", kLoop.header,
            (inst->name() + ":" + std::to_string(static_cast<int>(op))).c_str());
      return KLoopMatch();
    }
  }
  trace("side-effects-ok", kLoop.header);

  return km;
}

// lane1 的副本位于 k 循环内，但依赖 j 的地址表达式通常在 j 循环体中、
// 即 k 循环前置块中准备。只替换 jLoop.ivPhi 会让副本继续使用 lane0 地址。
// 这个辅助函数只复制无副作用的 SSA 表达式；加载、存储、调用、PHI 和
// 终结指令留在原位置处理。
bool isLaneCloneableOpcode(Opcode opcode) {
  switch (opcode) {
  case Opcode::Br:
  case Opcode::CondBr:
  case Opcode::Ret:
  case Opcode::Alloca:
  case Opcode::Load:
  case Opcode::Store:
  case Opcode::Call:
  case Opcode::VLoad:
  case Opcode::VStore:
  case Opcode::Phi:
    return false;
  default:
    return true;
  }
}

bool canPrepareLaneValue(
    Value* value, Instruction* jIvPhi, const analysis::NaturalLoop& kNatural,
    std::unordered_set<Value*>& active) {
  if (!value || value == jIvPhi)
    return true;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !valueUsesPhi(value, jIvPhi))
    return true;

  // k 循环指令由 transformNest 负责复制，这里先遍历其操作数，准备 j 相关
  // 的外部表达式，再修改 IR。
  if (inst->parent() && kNatural.blocks.count(inst->parent())) {
    // 循环携带的 PHI 构成 SSA 环，停止沿环继续遍历；实际的 lane 副本会
    // 单独重映射该 PHI。
    if (!active.insert(value).second)
      return true;
    for (auto* operand : inst->operands())
      if (!canPrepareLaneValue(operand, jIvPhi, kNatural, active)) {
        active.erase(value);
        return false;
      }
    active.erase(value);
    return true;
  }

  if (!isLaneCloneableOpcode(inst->opcode()) || inst->isTerminator())
    return false;
  if (!active.insert(value).second)
    return false;
  for (auto* operand : inst->operands()) {
    if (!canPrepareLaneValue(operand, jIvPhi, kNatural, active)) {
      active.erase(value);
      return false;
    }
  }
  active.erase(value);
  return true;
}

Value* cloneLaneValue(
    Value* value, Instruction* jIvPhi, const analysis::NaturalLoop& kNatural,
    BasicBlock* insertBlock, Instruction* insertPos,
    std::unordered_map<Value*, Value*>& laneMap,
    std::unordered_set<Value*>& active) {
  if (!value)
    return nullptr;
  auto mapped = laneMap.find(value);
  if (mapped != laneMap.end())
    return mapped->second;
  if (value == jIvPhi)
    return value;

  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !valueUsesPhi(value, jIvPhi))
    return value;

  // k 循环指令会在下面的常规指令遍历中复制。这里递归准备它依赖的 j 相关
  // 操作数，但不把指令本身复制到前置块。
  if (inst->parent() && kNatural.blocks.count(inst->parent())) {
    // 循环携带的 PHI 可能经由 latch 指回自身，重复访问时不再复制外部的
    // lane 表达式。
    if (!active.insert(value).second)
      return value;
    for (auto* operand : inst->operands()) {
      if (!cloneLaneValue(operand, jIvPhi, kNatural, insertBlock, insertPos,
                          laneMap, active)) {
        active.erase(value);
        return nullptr;
      }
    }
    active.erase(value);
    return value;
  }

  if (!insertBlock || !insertPos || !isLaneCloneableOpcode(inst->opcode()) ||
      inst->isTerminator() || !active.insert(value).second)
    return nullptr;

  std::vector<Value*> clonedOperands;
  clonedOperands.reserve(inst->numOperands());
  for (auto* operand : inst->operands()) {
    Value* cloned = cloneLaneValue(operand, jIvPhi, kNatural, insertBlock,
                                   insertPos, laneMap, active);
    if (!cloned) {
      active.erase(value);
      return nullptr;
    }
    clonedOperands.push_back(cloned);
  }

  auto* clone = cloneInstructionShell(inst);
  if (!clone) {
    active.erase(value);
    return nullptr;
  }
  for (auto* operand : clonedOperands)
    clone->addOperand(operand);
  insertBlock->insertBefore(insertPos, clone);
  laneMap[value] = clone;
  active.erase(value);
  return clone;
}

Instruction* findCStoreInJLoop(const analysis::NaturalLoop& jNatural,
                               Instruction* accPhi, GlobalVar*& cRoot) {
  // 搜索 j 循环所有块（body、latch、exit 后的后续块均可）中使用 accPhi 的 store。
  // 对 tiled 结构，C-store 可能在 tile.exit/j-loop 后续块里，不在 jBody 内。
  for (auto* bb : jNatural.blocks) {
    for (auto& ip : bb->instructions()) {
      auto* inst = ip.get();
      if (!inst || inst->opcode() != Opcode::Store) continue;
      Value* storeVal = inst->operand(0);
      if (storeVal != accPhi) continue;
      auto gep = parseGep(inst->operand(1));
      if (!gep.valid) continue;
      cRoot = gep.root;
      return inst;
    }
  }
  return nullptr;
}

bool transformNest(Function* func,
                   analysis::AnalysisManager& am,
                   const analysis::NaturalLoop& jNatural,
                   CanonicalLoop jLoop,
                   const analysis::NaturalLoop& kNatural,
                   const KLoopMatch& km) {
  BasicBlock* jBody = jLoop.body;
  BasicBlock* kPre = analysis::dedicatedPreheader(kNatural);
  if (!kPre) { trace("no-kPre", jBody); return false; }
  Instruction* kIvNext = km.kLoop.ivNext;
  BasicBlock* kHeader = km.kHeader;
  BasicBlock* kLatch = km.kLatch;

  GlobalVar* cRoot = nullptr;
  Instruction* cStore = findCStoreInJLoop(jNatural, km.accPhi, cRoot);
  if (!cStore) { trace("no-cStore", jBody); return false; }
  BasicBlock* cStoreBlock = cStore->parent();
  if (!cStoreBlock || !jNatural.blocks.count(cStoreBlock)) {
    trace("bad-cStore-block", jBody);
    return false;
  }
  // 记录 c 与 a、b 的数组角色，并取得 aLoad 的地址 GEP。
  auto aGep = parseGep(km.aLoad->operand(0));
  auto bGep = parseGep(km.bAddr);
  if (!aGep.valid || !bGep.valid) { trace("a/b-gep-invalid", jBody); return false; }
  if (cRoot == aGep.root || cRoot == bGep.root) { trace("cRoot-conflict", jBody); return false; }

  // 不要只复制归约而遗漏 j 循环中的其他副作用。这里仅复制归约和结果存储，
  // 所在 j 循环的其他工作只作为纯算术准备。
  for (auto* block : jNatural.blocks) {
    if (!block || kNatural.blocks.count(block))
      continue;
    for (auto& instruction : block->instructions()) {
      auto* inst = instruction.get();
      if (!inst || inst->isTerminator() || inst->opcode() == Opcode::Phi ||
          inst == cStore)
        continue;
      switch (inst->opcode()) {
      case Opcode::Alloca:
      case Opcode::Load:
      case Opcode::Store:
      case Opcode::Call:
      case Opcode::VLoad:
      case Opcode::VStore:
        trace("j-loop-side-effect", jBody,
              inst->name().c_str());
        return false;
      default:
        break;
      }
    }
  }

  // 确认 lane1 的 k 循环副本使用的每个 j 相关操作数都能重映射，尤其处理
  // jBody 中、k 循环前构造的常见地址表达式。
  // 只复制 accNext 的反向值切片。安全 if-conversion 可能在块内
  // 留下已无使用的 in-bounds load；机械地复制整个块会让它们
  // 每车道每迭代重复执行，且不是第二车道语义的一部分。
  std::unordered_set<Instruction*> laneNeeded;
  std::unordered_set<Value*> laneActive;
  std::function<void(Value*)> markLaneDependency = [&](Value* value) {
    auto* instruction = dynamic_cast<Instruction*>(value);
    if (!instruction || instruction == km.aLoad ||
        instruction == kIvNext || instruction->opcode() == Opcode::Phi ||
        instruction->parent() != kLatch ||
        !laneActive.insert(value).second) {
      return;
    }
    if (laneNeeded.insert(instruction).second)
      for (Value* operand : instruction->operands())
        markLaneDependency(operand);
    laneActive.erase(value);
  };
  markLaneDependency(km.accNext);

  std::vector<Instruction*> laneSources;
  for (auto& instruction : kLatch->instructions()) {
    auto* inst = instruction.get();
    if (!inst || !laneNeeded.count(inst))
      continue;
    laneSources.push_back(inst);
  }
  std::unordered_set<Value*> proofActive;
  for (auto* inst : laneSources) {
    for (auto* operand : inst->operands()) {
      if (!canPrepareLaneValue(operand, jLoop.ivPhi, kNatural,
                               proofActive)) {
        trace("lane-address-reject", km.kHeader);
        return false;
      }
    }
  }
  // lane1 的结果地址也必须由原地址表达式按 j -> j+1 重建。不能在已经
  // 线性化的指针上再做 GEP(ptr, 1)：该 IR 的裸 ptr 不携带元素宽度，后端会
  // 把它解释为 1 字节偏移，既可能产生未对齐 store，也会写错相邻元素。
  if (!canPrepareLaneValue(cStore->operand(1), jLoop.ivPhi, kNatural,
                           proofActive)) {
    trace("lane-result-address-reject", km.kHeader);
    return false;
  }

  // ---- 第一步：构造 jphi1 = jphi + 1，放在 j 循环体的 k 循环前置块之前 ----
  Instruction* jIvPhi = jLoop.ivPhi;
  Instruction* jPreTerm = jBody->terminator();
  auto* jphi1 = new Instruction(Opcode::Add, Type::i32(), "jlane1");
  jphi1->addOperand(jIvPhi);
  jphi1->addOperand(func->parent()->getConstInt(1));
  jBody->insertBefore(jPreTerm, jphi1);

  // ---- 第二步：构造 b_row_base1 = 原 b 行指针初值 + 4 ----
  // b 行指针初值来自 k 头部 PHI 在 k 前置块上的输入，先找到驱动 b 加载的
  // 指针 PHI。对于 GEP 地址，复制地址计算链并把 jphi 替换为 jphi1；
  // 对于指针 PHI 地址，在循环体中建立对应的 lane1 输入。

  // ---- 第三步：在 k 循环头部构造 accPhi1 ----
  auto* accPhi1 = new Instruction(Opcode::Phi, Type::i32(), "acc1");
  accPhi1->addOperand(func->parent()->getConstInt(0));
  accPhi1->addOperand(kPre);
  // accNext1 稍后加入 kLatch。
  km.kHeader->pushFront(accPhi1);

  // ---- 第四步：复制 lane1 的 k 循环体和 latch 指令 ----
  // body 与 latch 为同一块，当前匹配器按单块 k 循环处理。
  std::unordered_map<Value*, Value*> remap;
  remap[jIvPhi] = jphi1;
  remap[km.accPhi] = accPhi1;
  std::unordered_set<Value*> cloneActive;
  for (auto* inst : laneSources) {
    for (auto* operand : inst->operands()) {
      if (!cloneLaneValue(operand, jIvPhi, kNatural, jBody, jPreTerm,
                          remap, cloneActive)) {
        trace("lane-address-clone-failed", kHeader);
        return false;
      }
    }
  }
  Instruction* accNext1 = nullptr;

  // 先收集要复制的指令，避免 insertBefore 修改当前遍历列表。
  std::vector<Instruction*> toClone;
  for (auto& ip : kLatch->instructions()) {
    auto* inst = ip.get();
    if (!inst || !laneNeeded.count(inst))
      continue;
    toClone.push_back(inst);
  }
  for (auto* inst : toClone) {
    auto* clone = new Instruction(inst->opcode(), inst->type(),
                                  inst->name() + ".lane1");
    for (int k = 0; k < inst->numOperands(); ++k) {
      Value* op = inst->operand(k);
      auto it = remap.find(op);
      clone->addOperand(it != remap.end() ? it->second : op);
    }
    kLatch->insertBefore(kIvNext, clone);
    remap[inst] = clone;
    if (inst == km.accNext) accNext1 = clone;
  }

  if (!accNext1) {
    trace("no-accNext1", kHeader);
    return false;
  }

  // 将 accPhi1 的 latch 输入连接到 accNext1。
  accPhi1->addOperand(accNext1);
  accPhi1->addOperand(km.kLatch);

  // ---- 第五步：在 j 循环体中复制 lane1 的 c 存储 ----
  // 从对象根重新复制完整地址表达式，并把 j 映射为 j+1。这样元素宽度来自
  // 原始 GEP 链，不依赖裸指针上的隐式步长。
  Value* cAddr1 = cloneLaneValue(cStore->operand(1), jIvPhi, kNatural,
                                 cStoreBlock, cStore, remap, cloneActive);
  if (!cAddr1) {
    trace("lane-result-address-clone-failed", km.kHeader);
    return false;
  }
  auto* store1 = new Instruction(Opcode::Store, Type::voidTy(), "");
  store1->addOperand(accPhi1);  // value = accPhi1（第 1 路结果）
  store1->addOperand(cAddr1);   // address = c[i][j+1]
  cStoreBlock->insertBefore(cStore, store1);

  // ---- 第六步：把 j 归纳变量步长从 1 改为 2 ----
  // 查找 jLoop.ivNext = jphi + 1，并将常量操作数替换为 2。
  if (jLoop.ivNext && jLoop.ivNext->opcode() == Opcode::Add) {
    for (int k = 0; k < 2; ++k) {
      auto* c = dynamic_cast<ConstInt*>(jLoop.ivNext->operand(k));
      if (c && c->value() == 1) {
        jLoop.ivNext->setOperand(k, func->parent()->getConstInt(2));
        break;
      }
    }
  }

  func->recomputeCFG();
  am.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
  trace("accept", km.kHeader, "dual-lane applied");
  return true;
}

bool versionAndTransformRuntimeNest(
    Function* function, analysis::AnalysisManager& analysisManager,
    const analysis::NaturalLoop& jNatural, CanonicalLoop jLoop,
    const analysis::NaturalLoop& kNatural) {
  BasicBlock* originalPreheader = analysis::dedicatedPreheader(jNatural);
  Value* bound = jLoop.boundVal;
  if (!function) {
    trace("runtime-version-reject", jNatural.header, "null-function");
    return false;
  }
  if (!originalPreheader) {
    trace("runtime-version-reject", jNatural.header, "no-preheader");
    return false;
  }
  if (!bound || !bound->type().isI32()) {
    trace("runtime-version-reject", jNatural.header, "invalid-bound");
    return false;
  }
  if (isAlreadyVersionedPreheader(originalPreheader)) {
    trace("runtime-version-reject", jNatural.header, "already-versioned");
    return false;
  }

  std::unordered_map<BasicBlock*, BasicBlock*> blockMap;
  std::unordered_map<Value*, Value*> valueMap;
  if (!cloneLoopBlocks(function, jNatural, blockMap, valueMap) ||
      !addClonedExitPhiIncoming(jNatural, blockMap, valueMap)) {
    trace("runtime-version-reject", jNatural.header, "clone");
    return false;
  }

  analysis::NaturalLoop fastJNatural = mapClonedLoop(jNatural, blockMap);
  analysis::NaturalLoop fastKNatural = mapClonedLoop(kNatural, blockMap);
  if (!fastJNatural.header || !fastKNatural.header) {
    trace("runtime-version-reject", jNatural.header, "loop-map");
    return false;
  }
  BasicBlock* fastPreheader =
      createForwardingPreheader(function, fastJNatural.header);
  BasicBlock* slowPreheader =
      createForwardingPreheader(function, jNatural.header);
  if (!fastPreheader || !slowPreheader) {
    trace("runtime-version-reject", jNatural.header, "preheader-create");
    return false;
  }

  function->recomputeCFG();

  CanonicalLoop fastJLoop;
  if (!recognizeCanonicalLoop(fastJNatural, fastPreheader, fastJLoop)) {
    trace("runtime-version-reject", fastJNatural.header, "j-canonical");
    return false;
  }
  KLoopMatch fastMatch =
      matchKLoop(fastKNatural, fastJLoop, fastJLoop.boundVal);
  if (!fastMatch.accPhi || !fastMatch.aLoad) {
    trace("runtime-version-reject", fastJNatural.header, "k-match");
    return false;
  }
  if (!transformNest(function, analysisManager, fastJNatural, fastJLoop,
                     fastKNatural, fastMatch)) {
    trace("runtime-version-reject", fastJNatural.header, "transform");
    return false;
  }
  // 只有 transformNest 成功后才更新 PHI 前驱，避免中途失败时留下半成品 IR。
  analysis::replacePhiIncomingPred(jNatural.header, originalPreheader,
                                   slowPreheader);
  analysis::replacePhiIncomingPred(fastJNatural.header, originalPreheader,
                                   fastPreheader);
  if (!installRuntimeBoundDispatch(*function->parent(), originalPreheader,
                                   bound, fastPreheader, slowPreheader)) {
    trace("runtime-version-reject", fastJNatural.header, "dispatch");
    return false;
  }

  function->recomputeCFG();
  analysisManager.invalidateFunction(function,
                                     analysis::AnalysisInvalidation::CFG);
  trace("runtime-versioned", fastJNatural.header, "dual-lane applied");
  return true;
}

bool processFunction(Function* func, analysis::AnalysisManager& am) {
  if (!func || func->blocks().empty()) return false;
  func->recomputeCFG();
  am.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
  analysis::FunctionAnalysisCache& cache = am.function(func);
  const auto& forest = cache.loopForest();
  if (traceEnabled()) {
    std::cerr << "[reduction-split] func=" << func->name()
              << " forest_nodes=" << forest.nodes().size() << "\n";
  }
  bool changed = false;

  for (size_t ii = 0; ii < forest.nodes().size(); ++ii) {
    auto* iNode = forest.node(static_cast<int>(ii));
    if (!iNode || !iNode->loop || iNode->depth != 0) continue;
    if (traceEnabled()) {
      std::cerr << "[reduction-split] depth0 header="
                << (iNode->loop->header ? iNode->loop->header->name() : "?")
                << " children=" << iNode->children.size() << "\n";
    }
    CanonicalLoop iLoop;
    if (!recognizeCanonicalLoop(*iNode->loop, iNode->preheader, iLoop))
      continue;

    for (int ji : iNode->children) {
      auto* jNode = forest.node(ji);
      if (!jNode || !jNode->loop) continue;
      if (traceEnabled()) {
        std::cerr << "[reduction-split] depth1 header="
                  << (jNode->loop->header ? jNode->loop->header->name() : "?")
                  << " children=" << jNode->children.size() << "\n";
      }
      CanonicalLoop jLoop;
      if (!recognizeCanonicalLoop(*jNode->loop, jNode->preheader, jLoop))
        continue;
      auto* jBoundConst = dynamic_cast<ConstInt*>(jLoop.boundVal);
      const bool staticJBound =
          jBoundConst && jBoundConst->value() > 0 &&
          jBoundConst->value() % 2 == 0;
      if (jBoundConst && !staticJBound)
        continue;
      const bool enableRuntimeVersioning =
          true;  // 已找到 lane1 的地址表达式。
      if (!staticJBound && !enableRuntimeVersioning)
        continue;

      for (int ki : jNode->children) {
        auto* kNode = forest.node(ki);
        if (!kNode || !kNode->loop) continue;
        if (traceEnabled()) {
          std::cerr << "[reduction-split] depth2 header="
                    << (kNode->loop->header ? kNode->loop->header->name() : "?")
                    << " preheader="
                    << (kNode->preheader ? kNode->preheader->name() : "null") << "\n";
        }
        KLoopMatch km = matchKLoop(*kNode->loop, jLoop,
                                    staticJBound ? nullptr : jLoop.boundVal);
        if (!km.accPhi || !km.aLoad) continue;
        // k-loop 的指令数量、加载数量和 spill 数量受到共同限制。
        {
          int kBodyOps = 0;
          int kBodyLoads = 0;
          for (auto* bb : kNode->loop->blocks) {
            if (!bb) continue;
            for (auto& ins : bb->instructions()) {
              if (!ins || ins->isTerminator()) continue;
              switch (ins->opcode()) {
              case Opcode::Load:
              case Opcode::VLoad:
                ++kBodyLoads;
                ++kBodyOps;
                break;
              case Opcode::Store:
              case Opcode::VStore:
              case Opcode::Mul:
              case Opcode::Add:
              case Opcode::Sub:
                ++kBodyOps;
                break;
              default:
                break;
              }
            }
          }
          if (kBodyOps > 16 || kBodyLoads > 10) {
            trace("register-pressure-reject", km.kHeader);
            continue;
          }
        }
        trace("match-ok", km.kHeader);
        if (staticJBound &&
            transformNest(func, am, *jNode->loop, jLoop, *kNode->loop, km)) {
          changed = true;
          return changed; // 每轮只执行一次变换，然后清理分析结果并重新扫描
        }
        if (!staticJBound && enableRuntimeVersioning &&
            versionAndTransformRuntimeNest(
                                 func, am, *jNode->loop, jLoop,
                                 *kNode->loop)) {
          changed = true;
          return changed;
        }
      }
    }
  }
  return changed;
}

} // namespace

bool applyReductionSplit(Module& mod, analysis::AnalysisManager& am) {
  bool changed = false;
  for (int iter = 0; iter < 16; ++iter) {
    bool iterChanged = false;
    for (auto& fp : mod.functions())
      iterChanged |= processFunction(fp.get(), am);
    if (!iterChanged) break;
    changed = true;
  }
  return changed;
}

} // namespace hir
