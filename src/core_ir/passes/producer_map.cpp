#include "research_switch.h"
// 文件职责：整理循环生产者与消费者之间的值和内存访问关系。
#include "producer_map.h"
#include "cse.h"
#include "passes_utils.h"
#include "row_locality.h"
#include "scalar_recurrence.h"
#include "stage_fusion.h"

#include "../../hir/canonical_loop.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace core_ir {

struct GuardedStencilTap {
  GlobalVar* root = nullptr;
  int32_t rowOffset = 0;
  int32_t colOffset = 0;
  int32_t coeff = 0;
};

struct StencilLoadKey {
  GlobalVar* root = nullptr;
  int32_t rowOffset = 0;
  int32_t colOffset = 0;

  bool operator==(const StencilLoadKey& other) const {
    return root == other.root && rowOffset == other.rowOffset &&
           colOffset == other.colOffset;
  }
};

struct StencilLoadKeyHash {
  size_t operator()(const StencilLoadKey& key) const {
    size_t hash = std::hash<GlobalVar*>{}(key.root);
    hash ^= std::hash<int32_t>{}(key.rowOffset) + 0x9e3779b9u +
            (hash << 6) + (hash >> 2);
    hash ^= std::hash<int32_t>{}(key.colOffset) + 0x9e3779b9u +
            (hash << 6) + (hash >> 2);
    return hash;
  }
};

struct ProducerMapChecksumPlan {
  bool valid = false;
  hir::CanonicalLoop rowLoop;
  hir::CanonicalLoop repeatLoop;
  hir::CanonicalLoop mapLoop;
  Instruction* mapLoad = nullptr;
  Value* mapExpr = nullptr;
  BasicBlock* mapHeader = nullptr;
  BasicBlock* checksumExit = nullptr;
  Instruction* checksumValuePhi = nullptr;
  Instruction* checksumBoundPhi = nullptr;
  Value* mapBound = nullptr;
  Value* repeatBound = nullptr;
};

bool loopLiveOutsAreClosed(
    const analysis::NaturalLoop& loop,
    const std::unordered_set<Instruction*>& preservedValues,
    const std::unordered_set<BasicBlock*>& allowedExternalBlocks) {
  std::unordered_set<BasicBlock*> regionBlocks;
  for (BasicBlock* block : loop.blocks) {
    if (!block)
      return false;
    regionBlocks.insert(block);
  }
  for (BasicBlock* block : loop.blocks) {
    for (const auto& instPtr : block->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst || inst->isTerminator() || preservedValues.count(inst))
        continue;
      for (Instruction* user : inst->uses()) {
        if (!user || !user->parent() ||
            (!regionBlocks.count(user->parent()) &&
             !allowedExternalBlocks.count(user->parent()))) {
          return false;
        }
      }
    }
  }
  return true;
}

// 匹配 base + 常量偏移形式的值：沿 Add/Sub 链递归下降，
// 要求另一侧可求值为 int32 范围内的常量；限制递归深度防爆。
bool matchValuePlusConst(Value* value, Value* base, int32_t& offset,
                         int depth = 0) {
  if (!value || !base || depth > 8)
    return false;
  if (value == base) {
    offset = 0;
    return true;
  }
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->numOperands() != 2)
    return false;
  if (inst->opcode() == Opcode::Add) {
    int32_t sub = 0;
    int64_t constant = 0;
    if (matchValuePlusConst(inst->operand(0), base, sub, depth + 1) &&
        evaluateConstIntExpression(inst->operand(1), constant) &&
        constant >= std::numeric_limits<int32_t>::min() &&
        constant <= std::numeric_limits<int32_t>::max()) {
      offset = sub + static_cast<int32_t>(constant);
      return true;
    }
    if (matchValuePlusConst(inst->operand(1), base, sub, depth + 1) &&
        evaluateConstIntExpression(inst->operand(0), constant) &&
        constant >= std::numeric_limits<int32_t>::min() &&
        constant <= std::numeric_limits<int32_t>::max()) {
      offset = sub + static_cast<int32_t>(constant);
      return true;
    }
  }
  if (inst->opcode() == Opcode::Sub) {
    int32_t sub = 0;
    int64_t constant = 0;
    if (matchValuePlusConst(inst->operand(0), base, sub, depth + 1) &&
        evaluateConstIntExpression(inst->operand(1), constant) &&
        constant >= std::numeric_limits<int32_t>::min() &&
        constant <= std::numeric_limits<int32_t>::max()) {
      offset = sub - static_cast<int32_t>(constant);
      return true;
    }
  }
  return false;
}

bool stripScale4(Value* value, Value*& unscaled) {
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->numOperands() != 2)
    return false;
  if (inst->opcode() == Opcode::Shl) {
    auto* shift = dynamic_cast<ConstInt*>(inst->operand(1));
    if (shift && shift->value() == 2) {
      unscaled = inst->operand(0);
      return true;
    }
  }
  if (inst->opcode() == Opcode::Mul) {
    auto* lhsConst = dynamic_cast<ConstInt*>(inst->operand(0));
    auto* rhsConst = dynamic_cast<ConstInt*>(inst->operand(1));
    if (lhsConst && lhsConst->value() == 4) {
      unscaled = inst->operand(1);
      return true;
    }
    if (rhsConst && rhsConst->value() == 4) {
      unscaled = inst->operand(0);
      return true;
    }
  }
  return false;
}

bool matchProductByBound(Value* value, Value* bound, Value*& scaled) {
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->opcode() != Opcode::Mul || inst->numOperands() != 2)
    return false;
  if (inst->operand(0) == bound) {
    scaled = inst->operand(1);
    return true;
  }
  if (inst->operand(1) == bound) {
    scaled = inst->operand(0);
    return true;
  }
  return false;
}

bool matchMatrixIndexWithOffsets(Value* value,
                                 Value* row,
                                 Value* col,
                                 Value* bound,
                                 int32_t& rowOffset,
                                 int32_t& colOffset) {
  if (!value || !row || !col || !bound)
    return false;
  auto tryPair = [&](Value* productValue, Value* colValue) -> bool {
    Value* rowExpr = nullptr;
    int32_t dr = 0;
    int32_t dc = 0;
    if (!matchProductByBound(productValue, bound, rowExpr))
      return false;
    if (!matchValuePlusConst(rowExpr, row, dr))
      return false;
    if (!matchValuePlusConst(colValue, col, dc))
      return false;
    rowOffset = dr;
    colOffset = dc;
    return true;
  };

  if (tryPair(value, col))
    return true;

  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->numOperands() != 2)
    return false;
  if (inst->opcode() == Opcode::Add) {
    return tryPair(inst->operand(0), inst->operand(1)) ||
           tryPair(inst->operand(1), inst->operand(0));
  }
  if (inst->opcode() == Opcode::Sub && isZeroInt(inst->operand(1)))
    return matchMatrixIndexWithOffsets(inst->operand(0), row, col, bound,
                                       rowOffset, colOffset);
  return false;
}

bool matchMatrixGEPWithOffsets(Value* value,
                               Value* row,
                               Value* col,
                               Value* bound,
                               GlobalVar*& root,
                               int32_t& rowOffset,
                               int32_t& colOffset) {
  auto* gep = dynamic_cast<Instruction*>(value);
  if (!gep || gep->opcode() != Opcode::GEP || gep->numOperands() != 2)
    return false;
  auto* gv = dynamic_cast<GlobalVar*>(gep->operand(0));
  if (!gv || gv->elemTy() != Type::i32())
    return false;
  Value* unscaled = nullptr;
  if (!stripScale4(gep->operand(1), unscaled))
    return false;
  if (!matchMatrixIndexWithOffsets(unscaled, row, col, bound,
                                   rowOffset, colOffset))
    return false;
  root = gv;
  return true;
}

bool matchRowBasePointerInit(Value* value,
                             Value* bound,
                             GlobalVar*& root,
                             Value*& row) {
  auto* gep = dynamic_cast<Instruction*>(value);
  if (!gep || gep->opcode() != Opcode::GEP || gep->numOperands() != 2)
    return false;
  auto* gv = dynamic_cast<GlobalVar*>(gep->operand(0));
  if (!gv || gv->elemTy() != Type::i32())
    return false;
  Value* unscaled = nullptr;
  if (!stripScale4(gep->operand(1), unscaled))
    return false;
  Value* rowExpr = nullptr;
  if (!matchProductByBound(unscaled, bound, rowExpr))
    return false;
  root = gv;
  row = rowExpr;
  return true;
}

int32_t coefficientFromLoadUse(Instruction* load) {
  if (!load || load->uses().size() != 1)
    return 0;
  Instruction* user = load->uses().front();
  if (!user)
    return 0;
  if (user->opcode() == Opcode::Neg)
    return -1;
  if (user->opcode() == Opcode::Add)
    return 1;
  if (user->opcode() == Opcode::Sub) {
    if (user->numOperands() == 2 && user->operand(0) == load)
      return 1;
    if (user->numOperands() == 2 && user->operand(1) == load)
      return -1;
  }
  if (user->opcode() == Opcode::Mul && user->numOperands() == 2) {
    auto* lhsConst = dynamic_cast<ConstInt*>(user->operand(0));
    auto* rhsConst = dynamic_cast<ConstInt*>(user->operand(1));
    if (lhsConst && user->operand(1) == load)
      return lhsConst->value();
    if (rhsConst && user->operand(0) == load)
      return rhsConst->value();
  }
  return 0;
}

void mergeStencilTap(std::vector<GuardedStencilTap>& taps,
                     const GuardedStencilTap& tap) {
  if (!tap.root || tap.coeff == 0)
    return;
  for (auto& existing : taps) {
    if (existing.root == tap.root &&
        existing.rowOffset == tap.rowOffset &&
        existing.colOffset == tap.colOffset) {
      existing.coeff += tap.coeff;
      return;
    }
  }
  taps.push_back(tap);
}

Value* buildValuePlusConst(IRBuilder& builder, Value* base, int32_t offset) {
  if (offset == 0)
    return base;
  if (offset > 0)
    return builder.createAdd(base, builder.ci(offset), "stencil.idx");
  return builder.createSub(base, builder.ci(-offset), "stencil.idx");
}

Value* appendAnd(IRBuilder& builder, Value* lhs, Value* rhs) {
  if (!lhs)
    return rhs;
  return builder.createAnd(lhs, rhs, "stencil.interior.and");
}

Value* buildInteriorCondition(IRBuilder& builder,
                              Value* row,
                              Value* col,
                              Value* bound,
                              int32_t minRowOffset,
                              int32_t maxRowOffset,
                              int32_t minColOffset,
                              int32_t maxColOffset) {
  Value* zero = builder.ci(0);
  Value* cond = nullptr;
  cond = appendAnd(builder, cond,
                   builder.createICmp("ge",
                                      buildValuePlusConst(builder, row,
                                                          minRowOffset),
                                      zero,
                                      "stencil.row.lo"));
  cond = appendAnd(builder, cond,
                   builder.createICmp("lt",
                                      buildValuePlusConst(builder, row,
                                                          maxRowOffset),
                                      bound,
                                      "stencil.row.hi"));
  cond = appendAnd(builder, cond,
                   builder.createICmp("ge",
                                      buildValuePlusConst(builder, col,
                                                          minColOffset),
                                      zero,
                                      "stencil.col.lo"));
  cond = appendAnd(builder, cond,
                   builder.createICmp("lt",
                                      buildValuePlusConst(builder, col,
                                                          maxColOffset),
                                      bound,
                                      "stencil.col.hi"));
  return cond;
}

Value* buildStencilRawLoad(IRBuilder& builder,
                           const StencilLoadKey& key,
                           Value* row,
                           Value* col,
                           Value* bound) {
  Value* tapRow = buildValuePlusConst(builder, row, key.rowOffset);
  Value* rowBase = builder.createMul(tapRow, bound, "stencil.row.base");
  Value* tapCol = buildValuePlusConst(builder, col, key.colOffset);
  Value* index = builder.createAdd(rowBase, tapCol, "stencil.index");
  Value* bytes = builder.createShl(index, builder.ci(2), "stencil.bytes");
  Value* addr = builder.createGEP(key.root, {bytes}, "stencil.addr");
  return builder.createLoad(addr, Type::i32(), "stencil.load");
}

Value* applyStencilCoefficient(IRBuilder& builder,
                               Value* loaded,
                               int32_t coefficient) {
  if (coefficient == 1)
    return loaded;
  if (coefficient == -1)
    return builder.createNeg(loaded);
  return builder.createMul(loaded, builder.ci(coefficient), "stencil.coeff");
}

Value* buildStencilTapLoad(IRBuilder& builder,
                           const GuardedStencilTap& tap,
                           Value* row,
                           Value* col,
                           Value* bound) {
  StencilLoadKey key{tap.root, tap.rowOffset, tap.colOffset};
  Value* loaded = buildStencilRawLoad(builder, key, row, col, bound);
  return applyStencilCoefficient(builder, loaded, tap.coeff);
}

bool mapExpressionCanBeRemapped(
    Value* value,
    Instruction* mapLoad,
    std::unordered_map<Value*, bool>& memo,
    std::unordered_set<Value*>& active) {
  if (!value || !mapLoad)
    return false;
  if (value == mapLoad || value->isConstant() || dynamic_cast<GlobalVar*>(value) ||
      dynamic_cast<Argument*>(value) || dynamic_cast<BasicBlock*>(value)) {
    return true;
  }
  auto memoIt = memo.find(value);
  if (memoIt != memo.end())
    return memoIt->second;
  if (!active.insert(value).second)
    return false;
  auto* inst = dynamic_cast<Instruction*>(value);
  bool valid = inst && inst->opcode() != Opcode::Phi && !inst->isTerminator() &&
               inst->opcode() != Opcode::Load &&
               inst->opcode() != Opcode::Store &&
               inst->opcode() != Opcode::Call &&
               inst->opcode() != Opcode::Alloca &&
               inst->opcode() != Opcode::VLoad &&
               inst->opcode() != Opcode::VStore;
  if (valid) {
    for (Value* operand : inst->operands()) {
      if (!mapExpressionCanBeRemapped(operand, mapLoad, memo, active)) {
        valid = false;
        break;
      }
    }
  }
  active.erase(value);
  memo[value] = valid;
  return valid;
}

bool mapExpressionCanBeRemapped(Value* value, Instruction* mapLoad) {
  std::unordered_map<Value*, bool> memo;
  std::unordered_set<Value*> active;
  return mapExpressionCanBeRemapped(value, mapLoad, memo, active);
}

// 递归克隆 map 表达式的定义链并插入到指定指令之前：mapLoad 替换为
// replacement；常量/全局/实参直接复用；Phi、访存与调用指令不可克隆，
// 立即失败；memo 防止重复克隆。
Value* cloneMapExpressionBefore(Value* value,
                                Instruction* mapLoad,
                                Value* replacement,
                                BasicBlock* bb,
                                Instruction* before,
                                std::unordered_map<Value*, Value*>& memo) {
  if (!value || !mapLoad || !replacement || !bb || !before)
    return nullptr;
  if (value == mapLoad)
    return replacement;
  if (value->isConstant() || dynamic_cast<GlobalVar*>(value) ||
      dynamic_cast<Argument*>(value) || dynamic_cast<BasicBlock*>(value)) {
    return value;
  }
  auto memoIt = memo.find(value);
  if (memoIt != memo.end())
    return memoIt->second;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->opcode() == Opcode::Phi || inst->isTerminator() ||
      inst->opcode() == Opcode::Load || inst->opcode() == Opcode::Store ||
      inst->opcode() == Opcode::Call || inst->opcode() == Opcode::Alloca ||
      inst->opcode() == Opcode::VLoad || inst->opcode() == Opcode::VStore) {
    return nullptr;
  }
  auto* clone = new Instruction(inst->opcode(), inst->type(), "producer.map");
  for (Value* operand : inst->operands()) {
    Value* clonedOperand =
        cloneMapExpressionBefore(operand, mapLoad, replacement, bb, before,
                                 memo);
    if (!clonedOperand) {
      clone->dropAllOperands();
      delete clone;
      return nullptr;
    }
    clone->addOperand(clonedOperand);
  }
  bb->insertBefore(before, clone);
  memo[value] = clone;
  return clone;
}

Value* cloneMapExpressionBefore(Value* value,
                                Instruction* mapLoad,
                                Value* replacement,
                                BasicBlock* bb,
                                Instruction* before) {
  std::unordered_map<Value*, Value*> memo;
  return cloneMapExpressionBefore(value, mapLoad, replacement, bb, before,
                                  memo);
}

Instruction* insertBinaryBefore(BasicBlock* bb,
                                Instruction* before,
                                Opcode op,
                                Type type,
                                const std::string& name,
                                Value* lhs,
                                Value* rhs) {
  auto* inst = new Instruction(op, type, name);
  inst->addOperand(lhs);
  inst->addOperand(rhs);
  bb->insertBefore(before, inst);
  return inst;
}

const analysis::NaturalLoop* findSmallestContainingLoop(
    const std::vector<analysis::NaturalLoop>& loops,
    BasicBlock* block,
    const analysis::NaturalLoop* exclude) {
  const analysis::NaturalLoop* best = nullptr;
  for (const auto& loop : loops) {
    if (&loop == exclude || !block || !loop.blocks.count(block))
      continue;
    if (!best || loop.blocks.size() < best->blocks.size())
      best = &loop;
  }
  return best;
}

bool matchMapSumRecurrence(Value* value,
                           const hir::CanonicalLoop& mapLoop,
                           Value* mappedValue) {
  auto* phi = dynamic_cast<Instruction*>(value);
  if (!phi || phi->opcode() != Opcode::Phi || phi->type() != Type::i32() ||
      phi->parent() != mapLoop.header || !mappedValue) {
    return false;
  }
  Value* init = nullptr;
  Value* back = nullptr;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
    if (pred == mapLoop.preheader)
      init = phi->operand(i);
    else if (pred == mapLoop.latch)
      back = phi->operand(i);
  }
  auto* add = dynamic_cast<Instruction*>(back);
  return isZeroInt(init) && add && add->opcode() == Opcode::Add &&
         add->numOperands() == 2 && add->parent() == mapLoop.latch &&
         ((add->operand(0) == phi && add->operand(1) == mappedValue) ||
          (add->operand(1) == phi && add->operand(0) == mappedValue));
}

bool matchOneMinusBound(Value* value, Value* bound) {
  auto* sub = dynamic_cast<Instruction*>(value);
  auto* one = sub && sub->numOperands() == 2
                  ? dynamic_cast<ConstInt*>(sub->operand(0))
                  : nullptr;
  return sub && sub->opcode() == Opcode::Sub && sub->numOperands() == 2 &&
         one && one->value() == 1 && sub->operand(1) == bound;
}

bool matchStageChecksumFormula(Value* value,
                               Value* bound,
                               const hir::CanonicalLoop& mapLoop,
                               Value* mappedValue) {
  auto* mul = dynamic_cast<Instruction*>(value);
  if (!mul || mul->opcode() != Opcode::Mul || mul->numOperands() != 2)
    return false;
  if (matchOneMinusBound(mul->operand(0), bound))
    return matchMapSumRecurrence(mul->operand(1), mapLoop, mappedValue);
  if (matchOneMinusBound(mul->operand(1), bound))
    return matchMapSumRecurrence(mul->operand(0), mapLoop, mappedValue);
  return false;
}

bool recognizeStencilForwardLoop(const analysis::NaturalLoop& natural,
                                 BasicBlock* preheader,
                                 hir::CanonicalLoop& out);

// 判断 map 循环能否被生产者旁路：要求规范的前进 lt 循环（初值 0、
// 体块即闩块、单出口），边界为方阵维度，并找到根植于 outRoot 的
// 顺序指针 phi。
bool matchMapLoopForProducerBypass(const std::vector<analysis::NaturalLoop>& loops,
                                   BasicBlock* mapHeader,
                                   GlobalVar* outRoot,
                                   ProducerMapChecksumPlan& plan) {
  auto trace = [&](const char* reason) {
    if (sycEnvironmentValue("SYC_TRACE_COREIR_STENCIL")) {
      std::cerr << "[producer-map] reject header="
                << (mapHeader ? mapHeader->name() : std::string("<null>"))
                << " reason=" << (reason ? reason : "-") << "\n";
    }
  };
  const analysis::NaturalLoop* mapNatural = loopForHeader(loops, mapHeader);
  BasicBlock* mapPreheader = mapNatural ? analysis::dedicatedPreheader(*mapNatural)
                                        : nullptr;
  hir::CanonicalLoop mapLoop;
  if (!mapNatural || !mapPreheader ||
      !hir::recognizeCanonicalLoop(*mapNatural, mapPreheader, mapLoop) ||
      !canonicalLoopIsForwardLt(mapLoop) || !isZeroInt(mapLoop.initVal) ||
      mapLoop.body != mapLoop.latch || !loopHasSingleExit(*mapNatural, mapLoop.exit)) {
    trace("map-shape");
    return false;
  }
  Value* mapBound = squareMatrixLoopDimension(mapLoop.boundVal);
  if (!mapBound) {
    trace("map-bound");
    return false;
  }
  Instruction* mapPtrPhi = nullptr;
  std::vector<Instruction*> mapPhis;
  std::vector<Value*> mapInits;
  std::vector<Value*> mapNexts;
  if (collectHeaderLoopPhis(mapLoop, mapPhis, mapInits, &mapNexts)) {
    for (Instruction* phi : mapPhis) {
      if (!phi || !phi->type().isPtr())
        continue;
      Value* init = nullptr;
      if (matchSequentialPointerPhi(phi, mapLoop.preheader, mapLoop.latch,
                                    init) &&
          matchRootZeroGEP(init, outRoot)) {
        mapPtrPhi = phi;
        break;
      }
    }
  }

  // 扫描循环体：只允许恰好一个匹配 map 地址的 load 和一个 store，
  // 拒绝调用与向量访存；最后要求 store 写回 load 的同一地址，
  // 且存入表达式可重映射。
  Instruction* load = nullptr;
  Instruction* store = nullptr;
  auto matchesMapAddress = [&](Value* address) {
    if (mapPtrPhi && address == mapPtrPhi)
      return true;
    GlobalVar* directRoot = nullptr;
    return matchRootVectorIndexGEP(address, directRoot, mapLoop.ivPhi) &&
           directRoot == outRoot;
  };
  for (auto& instPtr : mapLoop.body->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst) {
      trace("map-inst");
      return false;
    }
    if (inst->opcode() == Opcode::Load) {
      if (inst->numOperands() != 1)
        return false;
      if (matchesMapAddress(inst->operand(0))) {
        if (load)
          return false;
        load = inst;
      } else if (memoryRoot(inst->operand(0)) == outRoot) {
        return false;
      }
      continue;
    }
    if (inst->opcode() == Opcode::Store) {
      if (inst->numOperands() != 2 || !matchesMapAddress(inst->operand(1)) ||
          store) {
        return false;
      }
      store = inst;
      continue;
    }
    if (inst->opcode() == Opcode::Call || inst->opcode() == Opcode::Ret ||
        inst->opcode() == Opcode::Alloca || inst->opcode() == Opcode::VLoad ||
        inst->opcode() == Opcode::VStore) {
      return false;
    }
  }
  if (!load || !store || store->operand(1) != load->operand(0) ||
      !mapExpressionCanBeRemapped(store->operand(0), load)) {
    trace("map-load-store");
    return false;
  }

  std::unordered_set<Instruction*> noPreservedValues;
  std::unordered_set<BasicBlock*> mapBypassBlocks = {mapLoop.exit};
  if (!loopLiveOutsAreClosed(*mapNatural, noPreservedValues,
                             mapBypassBlocks)) {
    trace("map-live-out");
    return false;
  }

  Instruction* mapExitTerm = mapLoop.exit ? mapLoop.exit->terminator() : nullptr;
  if (!mapExitTerm || mapExitTerm->opcode() != Opcode::CondBr ||
      mapExitTerm->numOperands() != 3) {
    trace("map-exit");
    return false;
  }
  auto* bypassTarget = dynamic_cast<BasicBlock*>(mapExitTerm->operand(1));
  BasicBlock* bypassPredecessor = mapLoop.exit;
  BasicBlock* checksumExit = singleBrTarget(bypassTarget);
  if (checksumExit)
    bypassPredecessor = bypassTarget;
  else
    checksumExit = bypassTarget;
  if (!checksumExit) {
    trace("checksum-exit");
    return false;
  }

  Instruction* checksumValuePhi = nullptr;
  Instruction* checksumBoundPhi = nullptr;
  for (auto& instPtr : checksumExit->instructions()) {
    Instruction* phi = instPtr.get();
    if (!phi || phi->opcode() != Opcode::Phi)
      break;
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
      if (pred != bypassPredecessor)
        continue;
      if (phi->type().isI32() &&
          matchStageChecksumFormula(phi->operand(i), mapBound, mapLoop,
                                    store->operand(0))) {
        if (checksumValuePhi)
          return false;
        checksumValuePhi = phi;
      } else if (phi->type().isI32() && phi->operand(i) == mapBound) {
        if (checksumBoundPhi)
          return false;
        checksumBoundPhi = phi;
      }
    }
  }
  if (!checksumValuePhi) {
    trace("checksum-phi");
    return false;
  }

  plan.valid = true;
  plan.mapLoop = mapLoop;
  plan.mapLoad = load;
  plan.mapExpr = store->operand(0);
  plan.mapHeader = mapHeader;
  plan.checksumExit = checksumExit;
  plan.checksumValuePhi = checksumValuePhi;
  plan.checksumBoundPhi = checksumBoundPhi;
  plan.mapBound = mapBound;
  return true;
}

bool findProducerMapChecksumPlan(const std::vector<analysis::NaturalLoop>& loops,
                                 const analysis::NaturalLoop& innerNatural,
                                 const hir::CanonicalLoop& innerLoop,
                                 GlobalVar* outRoot,
                                 Value* producerRow,
                                 Instruction* producerPtrPhi,
                                 ProducerMapChecksumPlan& plan) {
  auto trace = [&](const char* reason) {
    if (sycEnvironmentValue("SYC_TRACE_COREIR_STENCIL")) {
      std::cerr << "[producer-map] reject inner="
                << (innerLoop.header ? innerLoop.header->name() : std::string("<null>"))
                << " reason=" << (reason ? reason : "-") << "\n";
    }
  };
  const analysis::NaturalLoop* rowNatural =
      findSmallestContainingLoop(loops, innerLoop.header, &innerNatural);
  if (!rowNatural) {
    trace("no-row");
    return false;
  }
  BasicBlock* rowPreheader = analysis::dedicatedPreheader(*rowNatural);
  hir::CanonicalLoop rowLoop;
  if (!rowPreheader ||
      !recognizeStencilForwardLoop(*rowNatural, rowPreheader, rowLoop) ||
      rowLoop.body != innerLoop.preheader || rowLoop.latch != innerLoop.exit) {
    trace("row-shape");
    return false;
  }

  const analysis::NaturalLoop* repeatNatural =
      findSmallestContainingLoop(loops, rowLoop.header, rowNatural);
  if (!repeatNatural) {
    trace("no-repeat");
    return false;
  }
  BasicBlock* repeatPreheader = analysis::dedicatedPreheader(*repeatNatural);
  hir::CanonicalLoop repeatLoop;
  if (!repeatPreheader ||
      !recognizeStencilForwardLoop(*repeatNatural, repeatPreheader, repeatLoop) ||
      rowLoop.preheader != repeatLoop.body || repeatLoop.latch != rowLoop.exit) {
    trace("repeat-shape");
    return false;
  }

  BasicBlock* mapHeader = singleBrTarget(repeatLoop.exit);
  if (!matchMapLoopForProducerBypass(loops, mapHeader, outRoot, plan)) {
    trace("map");
    return false;
  }
  Value* producerPtrInit = nullptr;
  GlobalVar* producerRoot = nullptr;
  Value* producerPtrRow = nullptr;
  if (!isZeroInt(innerLoop.initVal) || !isZeroInt(rowLoop.initVal) ||
      innerLoop.boundVal != plan.mapBound ||
      rowLoop.boundVal != plan.mapBound || producerRow != rowLoop.ivPhi ||
      !matchSequentialPointerPhi(producerPtrPhi, innerLoop.preheader,
                                 innerLoop.latch, producerPtrInit) ||
      !matchRowBasePointerInit(producerPtrInit, plan.mapBound, producerRoot,
                               producerPtrRow) ||
      producerRoot != outRoot || producerPtrRow != rowLoop.ivPhi) {
    trace("producer-coverage");
    return false;
  }
  plan.rowLoop = rowLoop;
  plan.repeatLoop = repeatLoop;
  plan.repeatBound = repeatLoop.boundVal;
  return true;
}

bool producerEffectsAreProven(const analysis::NaturalLoop& loop,
                              Instruction* expectedStore) {
  if (!expectedStore)
    return false;
  for (BasicBlock* block : loop.blocks) {
    if (!block)
      return false;
    for (auto& instPtr : block->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      if (inst->opcode() == Opcode::Call || inst->opcode() == Opcode::Ret ||
          inst->opcode() == Opcode::Alloca || inst->opcode() == Opcode::VLoad ||
          inst->opcode() == Opcode::VStore) {
        return false;
      }
      if (inst->opcode() == Opcode::Store && inst != expectedStore)
        return false;
    }
  }
  return true;
}

Instruction* addAccumulatorPhi(BasicBlock* header,
                               Value* init,
                               BasicBlock* initPred,
                               const std::string& name) {
  auto* phi = new Instruction(Opcode::Phi, Type::i32(), name);
  phi->addOperand(init);
  phi->addOperand(initPred);
  insertBeforeFirstNonPhi(header, phi);
  return phi;
}

bool materializeProducerMapAccumulator(Function* func,
                                       const hir::CanonicalLoop& innerLoop,
                                       Instruction* ptrPhi,
                                       Instruction* store,
                                       const ProducerMapChecksumPlan& mapPlan,
                                       Instruction*& innerAccPhiOut,
                                       Value*& repeatAccPhiOut) {
  if (!func || !func->parent() || !ptrPhi || !store || !mapPlan.valid ||
      !mapPlan.repeatLoop.header || !mapPlan.rowLoop.header ||
      !innerLoop.header || !innerLoop.latch) {
    return false;
  }
  Instruction* latchTerm = innerLoop.latch->terminator();
  if (!latchTerm || latchTerm->opcode() != Opcode::Br ||
      !mapExpressionCanBeRemapped(mapPlan.mapExpr, mapPlan.mapLoad)) {
    return false;
  }
  Module* mod = func->parent();
  Instruction* repeatAcc = addAccumulatorPhi(
      mapPlan.repeatLoop.header, mod->getConstInt(0),
      mapPlan.repeatLoop.preheader, "producer.map.repeat.sum");
  Instruction* rowAcc = addAccumulatorPhi(
      mapPlan.rowLoop.header, mod->getConstInt(0),
      mapPlan.rowLoop.preheader, "producer.map.row.sum");
  Instruction* innerAcc = addAccumulatorPhi(
      innerLoop.header, rowAcc, innerLoop.preheader,
      "producer.map.inner.sum");

  rowAcc->addOperand(innerAcc);
  rowAcc->addOperand(mapPlan.rowLoop.latch);
  repeatAcc->addOperand(rowAcc);
  repeatAcc->addOperand(mapPlan.repeatLoop.latch);

  Value* mapped = cloneMapExpressionBefore(mapPlan.mapExpr, mapPlan.mapLoad,
                                           store->operand(0), innerLoop.latch,
                                           latchTerm);
  if (!mapped)
    return false;
  Instruction* accNext = insertBinaryBefore(innerLoop.latch, latchTerm,
                                            Opcode::Add, Type::i32(),
                                            "producer.map.inner.next",
                                            innerAcc, mapped);
  innerAcc->addOperand(accNext);
  innerAcc->addOperand(innerLoop.latch);

  innerAccPhiOut = innerAcc;
  repeatAccPhiOut = repeatAcc;
  return true;
}

bool producerMapBypassCanBeMaterialized(
    Function* func, const ProducerMapChecksumPlan& mapPlan) {
  if (!func || !func->parent() || !mapPlan.valid ||
      !mapPlan.repeatLoop.exit || !mapPlan.mapHeader ||
      !mapPlan.checksumExit || !mapPlan.checksumValuePhi ||
      !mapPlan.mapBound || !mapPlan.repeatBound ||
      !mapPlan.repeatLoop.initVal) {
    return false;
  }
  BasicBlock* exit = mapPlan.repeatLoop.exit;
  Instruction* term = exit->terminator();
  return term && term->opcode() == Opcode::Br && term->numOperands() == 1 &&
         term->operand(0) == mapPlan.mapHeader;
}

bool materializeProducerMapBypass(Function* func,
                                  const ProducerMapChecksumPlan& mapPlan,
                                  Value* repeatAccPhi) {
  if (!repeatAccPhi ||
      !producerMapBypassCanBeMaterialized(func, mapPlan)) {
    return false;
  }
  Module* mod = func->parent();
  BasicBlock* exit = mapPlan.repeatLoop.exit;
  Instruction* term = exit->terminator();

  Instruction* hasRepeat = insertModRecICmpBefore(
      exit, term, "gt", "producer.map.repeat.has.trip",
      mapPlan.repeatBound, mapPlan.repeatLoop.initVal);
  Instruction* nNonNegative = insertModRecICmpBefore(
      exit, term, "ge", "producer.map.n.nonnegative",
      mapPlan.mapBound, mod->getConstInt(0));
  Instruction* canBypass = insertModRecInstBefore(
      exit, term, Opcode::And, Type::i32(), "producer.map.can.bypass",
      {hasRepeat, nNonNegative});
  Instruction* oneMinusN = insertModRecInstBefore(
      exit, term, Opcode::Sub, Type::i32(), "producer.map.one.minus.n",
      {mod->getConstInt(1), mapPlan.mapBound});
  Instruction* fusedChecksum = insertModRecInstBefore(
      exit, term, Opcode::Mul, Type::i32(), "producer.map.fused.checksum",
      {oneMinusN, repeatAccPhi});

  mapPlan.checksumValuePhi->addOperand(fusedChecksum);
  mapPlan.checksumValuePhi->addOperand(exit);
  if (mapPlan.checksumBoundPhi) {
    mapPlan.checksumBoundPhi->addOperand(mapPlan.mapBound);
    mapPlan.checksumBoundPhi->addOperand(exit);
  }

  exit->remove(term);
  auto* condBr = new Instruction(Opcode::CondBr, Type::voidTy());
  condBr->addOperand(canBypass);
  condBr->addOperand(mapPlan.checksumExit);
  condBr->addOperand(mapPlan.mapHeader);
  exit->pushBack(condBr);
  func->recomputeCFG();
  return true;
}

// 在创建 producer 累加 PHI 和改写旁路之前，先确认 stencil 快路径能够
// 接管当前循环状态。该检查只读 IR，避免后续物化失败时留下半完成的 PHI、
// 地址递推或 CFG 改写。
bool stencilFastPathStateCanBeExtended(const hir::CanonicalLoop& loop,
                                       Instruction* ptrPhi,
                                       BasicBlock* originalBody) {
  if (!loop.header || !loop.latch || !loop.condBr || !ptrPhi ||
      !originalBody) {
    return false;
  }

  std::vector<Instruction*> headerPhis;
  std::vector<Value*> headerInitVals;
  std::vector<Value*> slowNextVals;
  if (!collectHeaderLoopPhis(loop, headerPhis, headerInitVals,
                             &slowNextVals)) {
    return false;
  }
  for (Instruction* phi : headerPhis) {
    if (phi != loop.ivPhi && phi != ptrPhi)
      return false;
    bool hasLatchIncoming = false;
    for (int opIndex = 1; opIndex < phi->numOperands(); opIndex += 2) {
      if (phi->operand(opIndex) == loop.latch) {
        hasLatchIncoming = true;
        break;
      }
    }
    if (!hasLatchIncoming)
      return false;
  }

  auto* trueTarget = dynamic_cast<BasicBlock*>(loop.condBr->operand(1));
  if (trueTarget != originalBody || originalBody->preds().size() != 1 ||
      originalBody->preds().front() != loop.header) {
    return false;
  }
  Instruction* latchTerm = loop.latch->terminator();
  return latchTerm && latchTerm->opcode() == Opcode::Br &&
         latchTerm->numOperands() == 1 &&
         latchTerm->operand(0) == loop.header;
}

bool appendGuardedStencilInteriorFastPath(Function* func,
                                          Module& mod,
                                          const hir::CanonicalLoop& loop,
                                          Instruction* ptrPhi,
                                          BasicBlock* originalBody,
                                          const std::vector<GuardedStencilTap>& taps,
                                          Value* row,
                                          Value* col,
                                          Value* bound,
                                          Instruction* innerAccPhi,
                                          Instruction* mapLoad,
                                          Value* mapExpr,
                                          int32_t minRowOffset,
                                          int32_t maxRowOffset,
                                          int32_t minColOffset,
                                          int32_t maxColOffset) {
  auto traceEarlyReject = [&](const char* reason) {
    if (sycEnvironmentValue("SYC_TRACE_COREIR_STENCIL")) {
      std::cerr << "[stencil-interior] early-reject func="
                << (func ? func->name() : std::string("<null>"))
                << " header="
                << (loop.header ? loop.header->name() : std::string("<null>"))
                << " reason=" << (reason ? reason : "-") << "\n";
    }
  };
  if (!func || !loop.header || !loop.latch || !loop.condBr || !ptrPhi ||
      !originalBody || taps.empty() || !row || !col || !bound) {
    traceEarlyReject("input");
    return false;
  }

  std::vector<Instruction*> headerPhis;
  std::vector<Value*> headerInitVals;
  std::vector<Value*> slowNextVals;
  if (!collectHeaderLoopPhis(loop, headerPhis, headerInitVals,
                             &slowNextVals)) {
    traceEarlyReject("header-phis");
    return false;
  }
  std::vector<int> latchPredOperandIndices;
  for (Instruction* phi : headerPhis) {
    if (phi != loop.ivPhi && phi != ptrPhi && phi != innerAccPhi) {
      traceEarlyReject("phi-role");
      return false;
    }
    int latchPredIndex = -1;
    for (int opIndex = 1; opIndex < phi->numOperands(); opIndex += 2) {
      if (phi->operand(opIndex) == loop.latch) {
        latchPredIndex = opIndex;
        break;
      }
    }
    if (latchPredIndex < 0) {
      traceEarlyReject("phi-latch");
      return false;
    }
    latchPredOperandIndices.push_back(latchPredIndex);
  }
  auto* trueTarget = dynamic_cast<BasicBlock*>(loop.condBr->operand(1));
  if (trueTarget != originalBody || originalBody->preds().size() != 1 ||
      originalBody->preds().front() != loop.header) {
    traceEarlyReject("body-edge");
    return false;
  }
  Instruction* slowLatchTerm = loop.latch->terminator();
  if (!slowLatchTerm || slowLatchTerm->opcode() != Opcode::Br ||
      slowLatchTerm->numOperands() != 1 ||
      slowLatchTerm->operand(0) != loop.header ||
      (innerAccPhi && (!mapLoad || !mapExpr))) {
    traceEarlyReject("latch-state");
    return false;
  }

  std::unordered_map<StencilLoadKey, int32_t, StencilLoadKeyHash> lane0Coeffs;
  std::unordered_map<StencilLoadKey, int32_t, StencilLoadKeyHash> lane1Coeffs;
  std::unordered_set<StencilLoadKey, StencilLoadKeyHash> seenWideKeys;
  std::vector<StencilLoadKey> wideKeys;
  bool wideOffsetsSafe = true;
  for (const auto& tap : taps) {
    StencilLoadKey lane0{tap.root, tap.rowOffset, tap.colOffset};
    lane0Coeffs[lane0] += tap.coeff;
    if (seenWideKeys.insert(lane0).second)
      wideKeys.push_back(lane0);
    if (tap.colOffset == std::numeric_limits<int32_t>::max()) {
      wideOffsetsSafe = false;
      continue;
    }
    StencilLoadKey lane1{tap.root, tap.rowOffset, tap.colOffset + 1};
    lane1Coeffs[lane1] += tap.coeff;
    if (seenWideKeys.insert(lane1).second)
      wideKeys.push_back(lane1);
  }
  const size_t naiveWideLoads = taps.size() * 2;
  auto hasNonZeroCoefficient = [](const auto& coefficients) {
    return std::any_of(coefficients.begin(), coefficients.end(),
                       [](const auto& entry) { return entry.second != 0; });
  };
  const bool enableWide =
      wideOffsetsSafe && naiveWideLoads > 0 &&
      hasNonZeroCoefficient(lane0Coeffs) &&
      hasNonZeroCoefficient(lane1Coeffs) &&
      (naiveWideLoads - wideKeys.size()) * 4 >= naiveWideLoads;

  BasicBlock* dispatch = func->newBlock("stencil.interior.dispatch");
  BasicBlock* scalarFast = func->newBlock("stencil.interior.fast");
  BasicBlock* wideDispatch =
      enableWide ? func->newBlock("stencil.wide.dispatch") : nullptr;
  BasicBlock* wideFast =
      enableWide ? func->newBlock("stencil.wide.fast") : nullptr;
  BasicBlock* mergeLatch = func->newBlock("stencil.latch.merge");
  auto traceLateReject = [&](const char* reason) {
    if (sycEnvironmentValue("SYC_TRACE_COREIR_STENCIL")) {
      std::cerr << "[stencil-interior] late-reject func=" << func->name()
                << " header=" << loop.header->name()
                << " reason=" << (reason ? reason : "-") << "\n";
    }
  };

  IRBuilder dispatchBuilder(mod, func);
  dispatchBuilder.setInsertPoint(dispatch);
  Value* interiorCond = buildInteriorCondition(
      dispatchBuilder, row, col, bound, minRowOffset, maxRowOffset,
      minColOffset, maxColOffset);
  dispatchBuilder.createCondBr(interiorCond,
                               enableWide ? wideDispatch : scalarFast,
                               originalBody);

  if (enableWide) {
    IRBuilder wideDispatchBuilder(mod, func);
    wideDispatchBuilder.setInsertPoint(wideDispatch);
    Value* nextCol = wideDispatchBuilder.createAdd(
        col, wideDispatchBuilder.ci(1), "stencil.wide.col");
    Value* nextInLoop = wideDispatchBuilder.createICmp(
        "lt", nextCol, bound, "stencil.wide.inloop");
    Value* nextInterior = buildInteriorCondition(
        wideDispatchBuilder, row, nextCol, bound, minRowOffset,
        maxRowOffset, minColOffset, maxColOffset);
    Value* wideGuard = wideDispatchBuilder.createAnd(
        nextInLoop, nextInterior, "stencil.wide.guard");
    wideDispatchBuilder.createCondBr(wideGuard, wideFast, scalarFast);
  }

  // 标量快路径构造：逐 tap 加载求和；若存在内层累加 phi，则克隆 map
  // 表达式并叠加得到下一累加值；最后补齐指针/归纳变量/累加 phi 的
  // 下一值并跳回循环头。
  IRBuilder fastBuilder(mod, func);
  fastBuilder.setInsertPoint(scalarFast);
  Value* sum = nullptr;
  for (const auto& tap : taps) {
    if (tap.coeff == 0)
      continue;
    Value* term = buildStencilTapLoad(fastBuilder, tap, row, col, bound);
    sum = sum ? fastBuilder.createAdd(sum, term, "stencil.sum") : term;
  }
  if (!sum) {
    traceLateReject("scalar-sum");
    return false;
  }
  Value* accNext = nullptr;
  if (innerAccPhi && mapLoad && mapExpr) {
    auto* marker = new Instruction(Opcode::Br, Type::voidTy());
    marker->addOperand(loop.header);
    scalarFast->pushBack(marker);
    Value* mapped =
        cloneMapExpressionBefore(mapExpr, mapLoad, sum, scalarFast, marker);
    if (!mapped) {
      traceLateReject("scalar-map");
      return false;
    }
    accNext = insertBinaryBefore(scalarFast, marker, Opcode::Add, Type::i32(),
                                 "producer.map.inner.next",
                                 innerAccPhi, mapped);
    scalarFast->remove(marker);
  }
  if (!innerAccPhi)
    fastBuilder.createStore(sum, ptrPhi);
  Value* ptrNext = fastBuilder.createGEP(ptrPhi, {fastBuilder.ci(4)},
                                         "stencil.out.next");
  Value* ivNext = fastBuilder.createAdd(col, fastBuilder.ci(1),
                                        "stencil.iv.next");
  fastBuilder.createBr(mergeLatch);

  Value* widePtrNext = nullptr;
  Value* wideIvNext = nullptr;
  Value* wideAccNext = nullptr;
  if (enableWide) {
    IRBuilder wideBuilder(mod, func);
    wideBuilder.setInsertPoint(wideFast);
    Value* wideSum0 = nullptr;
    Value* wideSum1 = nullptr;
    for (const StencilLoadKey& key : wideKeys) {
      Value* loaded = buildStencilRawLoad(wideBuilder, key, row, col, bound);
      auto lane0 = lane0Coeffs.find(key);
      if (lane0 != lane0Coeffs.end() && lane0->second != 0) {
        Value* term =
            applyStencilCoefficient(wideBuilder, loaded, lane0->second);
        wideSum0 = wideSum0
                       ? wideBuilder.createAdd(wideSum0, term, "stencil.sum0")
                       : term;
      }
      auto lane1 = lane1Coeffs.find(key);
      if (lane1 != lane1Coeffs.end() && lane1->second != 0) {
        Value* term =
            applyStencilCoefficient(wideBuilder, loaded, lane1->second);
        wideSum1 = wideSum1
                       ? wideBuilder.createAdd(wideSum1, term, "stencil.sum1")
                       : term;
      }
    }
    if (!wideSum0 || !wideSum1) {
      traceLateReject("wide-sum");
      return false;
    }

    if (innerAccPhi && mapLoad && mapExpr) {
      auto* marker = new Instruction(Opcode::Br, Type::voidTy());
      marker->addOperand(loop.header);
      wideFast->pushBack(marker);
      Value* mapped0 = cloneMapExpressionBefore(
          mapExpr, mapLoad, wideSum0, wideFast, marker);
      Value* mapped1 = cloneMapExpressionBefore(
          mapExpr, mapLoad, wideSum1, wideFast, marker);
      if (!mapped0 || !mapped1) {
        traceLateReject("wide-map");
        return false;
      }
      Value* after0 = insertBinaryBefore(
          wideFast, marker, Opcode::Add, Type::i32(),
          "producer.map.inner.wide0", innerAccPhi, mapped0);
      wideAccNext = insertBinaryBefore(
          wideFast, marker, Opcode::Add, Type::i32(),
          "producer.map.inner.wide1", after0, mapped1);
      wideFast->remove(marker);
    }
    if (!innerAccPhi) {
      wideBuilder.createStore(wideSum0, ptrPhi);
      Value* secondPtr = wideBuilder.createGEP(
          ptrPhi, {wideBuilder.ci(4)}, "stencil.wide.second");
      wideBuilder.createStore(wideSum1, secondPtr);
    }
    widePtrNext = wideBuilder.createGEP(
        ptrPhi, {wideBuilder.ci(8)}, "stencil.wide.out.next");
    wideIvNext = wideBuilder.createAdd(
        col, wideBuilder.ci(2), "stencil.wide.iv.next");
    wideBuilder.createBr(mergeLatch);
  }

  // 三条执行路径只携带同一组循环状态。先在公共 latch 中合并下一状态，
  // 再唯一回到循环头，使后续归纳变量和地址递推分析看到标准循环形状。
  for (size_t phiIndex = 0; phiIndex < headerPhis.size(); ++phiIndex) {
    Instruction* phi = headerPhis[phiIndex];
    Value* scalarNext = phi == ptrPhi ? ptrNext
                        : phi == loop.ivPhi ? ivNext
                                           : accNext;
    Value* wideNext = phi == ptrPhi ? widePtrNext
                      : phi == loop.ivPhi ? wideIvNext
                                         : wideAccNext;
    auto* merged = new Instruction(Opcode::Phi, phi->type(),
                                   "stencil.latch.state");
    merged->addOperand(slowNextVals[phiIndex]);
    merged->addOperand(loop.latch);
    merged->addOperand(scalarNext);
    merged->addOperand(scalarFast);
    if (enableWide) {
      merged->addOperand(wideNext);
      merged->addOperand(wideFast);
    }
    mergeLatch->pushBack(merged);

    const int latchPredIndex = latchPredOperandIndices[phiIndex];
    phi->setOperand(latchPredIndex - 1, merged);
    phi->setOperand(latchPredIndex, mergeLatch);
  }
  slowLatchTerm->setOperand(0, mergeLatch);
  auto* mergeBack = new Instruction(Opcode::Br, Type::voidTy());
  mergeBack->addOperand(loop.header);
  mergeLatch->pushBack(mergeBack);

  loop.condBr->setOperand(1, dispatch);
  func->recomputeCFG();
  return true;
}

bool recognizeStencilForwardLoop(const analysis::NaturalLoop& natural,
                                 BasicBlock* preheader,
                                 hir::CanonicalLoop& out) {
  if (!natural.header || !preheader)
    return false;
  std::vector<BasicBlock*> latches;
  std::unordered_set<int> seenLatchIds;
  for (auto* latch : natural.latches) {
    if (latch && seenLatchIds.insert(latch->id()).second)
      latches.push_back(latch);
  }
  if (latches.size() != 1)
    return false;
  BasicBlock* latch = latches.front();
  Instruction* latchTerm = latch->terminator();
  if (!latchTerm || latchTerm->opcode() != Opcode::Br ||
      latchTerm->numOperands() != 1 || latchTerm->operand(0) != natural.header) {
    return false;
  }

  Instruction* term = natural.header->terminator();
  if (!term || term->opcode() != Opcode::CondBr || term->numOperands() != 3)
    return false;
  auto* cmp = dynamic_cast<Instruction*>(term->operand(0));
  if (!cmp || cmp->opcode() != Opcode::ICmp || cmp->numOperands() != 2)
    return false;
  auto* trueBB = dynamic_cast<BasicBlock*>(term->operand(1));
  auto* falseBB = dynamic_cast<BasicBlock*>(term->operand(2));
  if (!trueBB || !falseBB)
    return false;
  BasicBlock* body = natural.blocks.count(trueBB) ? trueBB : falseBB;
  BasicBlock* exit = natural.blocks.count(trueBB) ? falseBB : trueBB;
  if (!natural.blocks.count(body) || natural.blocks.count(exit))
    return false;

  Value* lhs = cmp->operand(0);
  Value* rhs = cmp->operand(1);
  auto* phi = dynamic_cast<Instruction*>(lhs);
  bool ivIsLhs = phi && phi->opcode() == Opcode::Phi &&
                 phi->parent() == natural.header;
  if (!ivIsLhs) {
    phi = dynamic_cast<Instruction*>(rhs);
    if (!phi || phi->opcode() != Opcode::Phi ||
        phi->parent() != natural.header) {
      return false;
    }
  }
  if (phi->numOperands() != 4)
    return false;

  Value* init = nullptr;
  Value* nextValue = nullptr;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
    if (pred == preheader)
      init = phi->operand(i);
    else if (pred == latch)
      nextValue = phi->operand(i);
  }
  auto* next = dynamic_cast<Instruction*>(nextValue);
  if (!init || !next || next->opcode() != Opcode::Add ||
      !next->parent() || !natural.blocks.count(next->parent())) {
    return false;
  }
  auto* lhsConst = dynamic_cast<ConstInt*>(next->operand(0));
  auto* rhsConst = dynamic_cast<ConstInt*>(next->operand(1));
  bool stepOne = (lhsConst && lhsConst->value() == 1 &&
                  next->operand(1) == phi) ||
                 (rhsConst && rhsConst->value() == 1 &&
                  next->operand(0) == phi);
  if (!stepOne)
    return false;

  out.preheader = preheader;
  out.header = natural.header;
  out.body = body;
  out.latch = latch;
  out.exit = exit;
  out.ivPhi = phi;
  out.ivNext = next;
  out.cmp = cmp;
  out.condBr = term;
  out.initVal = init;
  out.boundVal = ivIsLhs ? rhs : lhs;
  out.ivIsCmpLhs = ivIsLhs;
  return canonicalLoopIsForwardLt(out);
}

bool applyGuardedStencilInteriorFastPath(Function* func,
                                         analysis::AnalysisManager& analysisManager) {
  if (!func)
    return false;
  auto traceReject = [&](const analysis::NaturalLoop& loop,
                         const char* reason) {
    if (sycEnvironmentValue("SYC_TRACE_COREIR_STENCIL")) {
      std::cerr << "[stencil-interior] reject func=" << func->name()
                << " header="
                << (loop.header ? loop.header->name() : std::string("<null>"))
                << " reason=" << (reason ? reason : "-") << "\n";
    }
  };
  func->recomputeCFG();
  analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
  auto loops = cache.loops();
  std::sort(loops.begin(), loops.end(),
            [](const analysis::NaturalLoop& lhs,
               const analysis::NaturalLoop& rhs) {
              return lhs.blocks.size() < rhs.blocks.size();
            });

  for (const auto& natural : loops) {
    BasicBlock* preheader = analysis::dedicatedPreheader(natural);
    hir::CanonicalLoop loop;
    bool recognized = preheader &&
        (hir::recognizeCanonicalLoop(natural, preheader, loop) ||
         recognizeStencilForwardLoop(natural, preheader, loop));
    bool forwardLt = recognized && canonicalLoopIsForwardLt(loop);
    bool singleExit = recognized && loopHasSingleExit(natural, loop.exit);
    bool bodyLatchOk = recognized && loop.body && loop.latch;
    if (!recognized || !forwardLt || !singleExit || !bodyLatchOk) {
      if (sycEnvironmentValue("SYC_TRACE_COREIR_STENCIL")) {
        std::cerr << "[stencil-interior] loop-detail func=" << func->name()
                  << " header="
                  << (natural.header ? natural.header->name() : std::string("<null>"))
                  << " preheader=" << (preheader ? 1 : 0)
                  << " recognized=" << (recognized ? 1 : 0)
                  << " forward=" << (forwardLt ? 1 : 0)
                  << " singleExit=" << (singleExit ? 1 : 0)
                  << " bodyLatch=" << (bodyLatchOk ? 1 : 0)
                  << "\n";
      }
      traceReject(natural, "loop-shape");
      continue;
    }

    Instruction* store = nullptr;
    int storeCount = 0;
    for (auto& instPtr : loop.latch->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst || inst->opcode() != Opcode::Store)
        continue;
      store = inst;
      ++storeCount;
    }
    if (!store || storeCount != 1 || store->numOperands() != 2) {
      traceReject(natural, "latch-store");
      continue;
    }
    auto* ptrPhi = dynamic_cast<Instruction*>(store->operand(1));
    if (!ptrPhi || ptrPhi->opcode() != Opcode::Phi ||
        ptrPhi->parent() != loop.header) {
      traceReject(natural, "store-phi");
      continue;
    }

    std::vector<Instruction*> phis;
    std::vector<Value*> initVals;
    std::vector<Value*> nextVals;
    if (!collectHeaderLoopPhis(loop, phis, initVals, &nextVals))
      continue;
    Value* ptrInit = nullptr;
    for (size_t i = 0; i < phis.size(); ++i) {
      if (phis[i] == ptrPhi) {
        ptrInit = initVals[i];
        break;
      }
    }
    GlobalVar* outRoot = nullptr;
    Value* row = nullptr;
    if (!ptrInit || !matchRowBasePointerInit(ptrInit, loop.boundVal,
                                             outRoot, row)) {
      traceReject(natural, "row-base");
      continue;
    }

    // 扫描循环全部块确认副作用安全并收集模板采样点：调用/额外 store/
    // latch 内 load 等直接拒绝；body 内的 load 必须匹配带 (dr, dc) 偏移的
    // 矩阵 GEP 且不读输出数组自身，提取系数后合并进 taps。
    bool sideEffectSafe = true;
    std::vector<GuardedStencilTap> taps;
    for (auto* bb : natural.blocks) {
      if (!bb) {
        sideEffectSafe = false;
        break;
      }
      for (auto& instPtr : bb->instructions()) {
        Instruction* inst = instPtr.get();
        if (!inst) {
          sideEffectSafe = false;
          break;
        }
        if (inst->opcode() == Opcode::Call || inst->opcode() == Opcode::Ret ||
            inst->opcode() == Opcode::Alloca || inst->opcode() == Opcode::VLoad ||
            inst->opcode() == Opcode::VStore ||
            (inst->opcode() == Opcode::Store && inst != store)) {
          sideEffectSafe = false;
          traceReject(natural, "side-effect");
          break;
        }
        if (bb == loop.header)
          continue;
        if (bb == loop.latch) {
          if (inst->opcode() == Opcode::Load) {
            sideEffectSafe = false;
            traceReject(natural, "latch-load");
            break;
          }
          continue;
        }
        if (inst->opcode() != Opcode::Load || inst->numOperands() != 1)
          continue;
        GlobalVar* loadRoot = nullptr;
        int32_t dr = 0;
        int32_t dc = 0;
        if (!matchMatrixGEPWithOffsets(inst->operand(0), row, loop.ivPhi,
                                       loop.boundVal, loadRoot, dr, dc) ||
            loadRoot == outRoot) {
          sideEffectSafe = false;
          traceReject(natural, "load-shape");
          break;
        }
        int32_t coeff = coefficientFromLoadUse(inst);
        if (coeff == 0) {
          sideEffectSafe = false;
          traceReject(natural, "load-coeff");
          break;
        }
        mergeStencilTap(taps, {loadRoot, dr, dc, coeff});
      }
      if (!sideEffectSafe)
        break;
    }
    if (sideEffectSafe) {
      std::unordered_set<Instruction*> preservedLoopState = {loop.ivPhi,
                                                             ptrPhi};
      std::unordered_set<BasicBlock*> noAllowedExternalBlocks;
      if (!loopLiveOutsAreClosed(natural, preservedLoopState,
                                 noAllowedExternalBlocks)) {
        sideEffectSafe = false;
        traceReject(natural, "loop-live-out");
      }
    }
    if (!sideEffectSafe || taps.empty()) {
      if (sideEffectSafe)
        traceReject(natural, "no-taps");
      continue;
    }
    // 去除零系数 tap 并按（根数组、行偏移、列偏移）排序，保证后续
    // 代码生成顺序稳定。
    taps.erase(std::remove_if(taps.begin(), taps.end(),
                              [](const GuardedStencilTap& tap) {
                                return tap.coeff == 0;
                              }),
               taps.end());
    if (taps.empty()) {
      traceReject(natural, "zero-taps");
      continue;
    }
    std::sort(taps.begin(), taps.end(),
              [](const GuardedStencilTap& lhs, const GuardedStencilTap& rhs) {
                if (lhs.root != rhs.root)
                  return lhs.root < rhs.root;
                if (lhs.rowOffset != rhs.rowOffset)
                  return lhs.rowOffset < rhs.rowOffset;
                return lhs.colOffset < rhs.colOffset;
              });

    int32_t minRow = taps.front().rowOffset;
    int32_t maxRow = taps.front().rowOffset;
    int32_t minCol = taps.front().colOffset;
    int32_t maxCol = taps.front().colOffset;
    for (const auto& tap : taps) {
      minRow = std::min(minRow, tap.rowOffset);
      maxRow = std::max(maxRow, tap.rowOffset);
      minCol = std::min(minCol, tap.colOffset);
      maxCol = std::max(maxCol, tap.colOffset);
    }
    if (minRow == 0 && maxRow == 0 && minCol == 0 && maxCol == 0) {
      traceReject(natural, "point-only");
      continue;
    }

    if (!stencilFastPathStateCanBeExtended(loop, ptrPhi, loop.body)) {
      traceReject(natural, "fastpath-state");
      continue;
    }

    ProducerMapChecksumPlan mapPlan;
    Instruction* innerAccPhi = nullptr;
    Value* repeatAccPhi = nullptr;
    // producer-map 与 stencil 必须作为一个整体落地。先完成全部只读检查，
    // 确认 checksum 旁路的目标和 CFG 形状都可改写，再创建累加 PHI；这样
    // 不会在旁路失败时留下只有前半段的状态改写。
    bool producerPlanReady =
        !sycEnvironmentValue("SYC_SKIP_COREIR_PRODUCER_MAP") &&
        producerEffectsAreProven(natural, store) &&
        findProducerMapChecksumPlan(loops, natural, loop, outRoot, row, ptrPhi,
                                    mapPlan) &&
        producerMapBypassCanBeMaterialized(func, mapPlan);
    bool producerBypassReady = false;
    if (producerPlanReady) {
      producerBypassReady = materializeProducerMapAccumulator(
                                func, loop, ptrPhi, store, mapPlan,
                                innerAccPhi, repeatAccPhi) &&
                            materializeProducerMapBypass(
                                func, mapPlan, repeatAccPhi);
    }

    if (appendGuardedStencilInteriorFastPath(
            func, *func->parent(), loop, ptrPhi, loop.body, taps, row,
            loop.ivPhi, loop.boundVal,
            producerBypassReady ? innerAccPhi : nullptr,
            producerBypassReady ? mapPlan.mapLoad : nullptr,
            producerBypassReady ? mapPlan.mapExpr : nullptr,
            minRow, maxRow, minCol, maxCol)) {
      if (producerBypassReady && store->parent())
        store->parent()->remove(store);
      if (sycEnvironmentValue("SYC_TRACE_COREIR_STENCIL")) {
        std::cerr << "[stencil-interior] func=" << func->name()
                  << " header=" << loop.header->name()
                  << " taps=" << taps.size()
                  << " producerMap=" << (producerBypassReady ? 1 : 0)
                  << " latch=merged"
                  << "\n";
      }
      analysisManager.invalidateFunction(func,
                                         analysis::AnalysisInvalidation::CFG);
      return true;
    }
  }
  return false;
}

bool applyGuardedStencilInteriorFastPath(
    Module& mod,
    analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    if (applyGuardedStencilInteriorFastPath(funcPtr.get(), analysisManager))
      changed = true;
  }
  return changed;
}


} // namespace core_ir
