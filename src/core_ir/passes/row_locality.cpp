#include "research_switch.h"
// 文件职责：识别连续行访问并整理行级局部性相关的中间表示。
#include "row_locality.h"
#include "cse.h"
#include "passes_utils.h"
#include "scalar_recurrence.h"

#include "../../analysis/cfg_utils.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace core_ir {

// 行局部性改写会在函数入口物化一行 i32 暂存区。将单行暂存控制在
// 16 KiB 内，可避免该变换单独制造过大的栈帧；这是后端栈空间成本预算，
// 与源程序的数组名称或测试规模无关。
constexpr int kRowLocalityElementBytes = 4;
constexpr int kRowLocalityScratchBudgetBytes = 16 * 1024;

static bool rowFitsLocalityScratchBudget(int rowElements) {
  return rowElements > 0 &&
         rowElements <=
             kRowLocalityScratchBudgetBytes / kRowLocalityElementBytes;
}

struct InPlaceRowLocalityPlan {
  Function* func = nullptr;
  analysis::NaturalLoop outerLoop;
  hir::CanonicalLoop outer;
  hir::CanonicalLoop middle;
  hir::CanonicalLoop inner;
  GlobalVar* dst = nullptr;
  GlobalVar* lhs = nullptr;
  int rowElems = 0;
  int rowBytes = 0;
  Value* bound = nullptr;
  bool hasConstantSuffixRows = false;
  Value* constantSuffixStart = nullptr;
  bool constantSuffixStartExclusive = false;
  int32_t constantSuffixValue = 0;
};

struct ConstantRowSuffixFill {
  bool valid = false;
  Value* start = nullptr;
  bool startExclusive = false;
  BasicBlock* exit = nullptr;
  int32_t value = 0;
};

bool isZeroInt(Value* value) {
  auto* ci = dynamic_cast<ConstInt*>(value);
  return ci && ci->value() == 0;
}

bool sameValue(Value* lhs, Value* rhs) {
  return lhs == rhs;
}

BasicBlock* singleBrTarget(BasicBlock* bb) {
  if (!bb)
    return nullptr;
  Instruction* term = bb->terminator();
  if (!term || term->opcode() != Opcode::Br || term->numOperands() != 1)
    return nullptr;
  return dynamic_cast<BasicBlock*>(term->operand(0));
}

const analysis::NaturalLoop*
findLoopByHeader(const std::vector<analysis::NaturalLoop>& loops,
                 BasicBlock* header) {
  if (!header)
    return nullptr;
  for (const auto& loop : loops) {
    if (loop.header == header)
      return &loop;
  }
  return nullptr;
}

int64_t abs64(int64_t value) {
  return value < 0 ? -value : value;
}

int64_t linearCoeffFor(const LinearOffsetKey& key, Value* value) {
  std::string wanted = valueKey(value);
  for (const auto& term : key.terms) {
    if (term.first == wanted)
      return term.second;
  }
  return 0;
}

bool linearKeyHasOnlyTerms(const LinearOffsetKey& key,
                           const std::vector<Value*>& allowed) {
  std::unordered_set<std::string> allowedKeys;
  for (auto* value : allowed)
    allowedKeys.insert(valueKey(value));
  for (const auto& term : key.terms) {
    if (!allowedKeys.count(term.first))
      return false;
  }
  return true;
}

bool gepBasePlusConst(Value* value, Value* base, int64_t delta) {
  auto* gep = dynamic_cast<Instruction*>(value);
  if (!gep || gep->opcode() != Opcode::GEP || gep->numOperands() != 2 ||
      gep->operand(0) != base) {
    return false;
  }
  auto* ci = dynamic_cast<ConstInt*>(gep->operand(1));
  return ci && ci->value() == delta;
}

bool matchPointerPhi(Instruction* phi,
                     Value* initPred,
                     Value* nextPred,
                     GlobalVar* expectedRoot,
                     const std::vector<std::pair<Value*, int64_t>>& coeffs,
                     int64_t stepBytes) {
  if (!phi || phi->opcode() != Opcode::Phi || phi->numOperands() != 4) {
    return false;
  }

  Value* init = nullptr;
  Value* next = nullptr;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    if (phi->operand(i + 1) == initPred)
      init = phi->operand(i);
    else if (phi->operand(i + 1) == nextPred)
      next = phi->operand(i);
  }
  auto* initGep = dynamic_cast<Instruction*>(init);
  if (!initGep || initGep->opcode() != Opcode::GEP)
    return false;
  LinearOffsetKey key = buildGEPLinearOffsetKey(initGep);
  if (!key.valid || key.root != expectedRoot || key.constant != 0)
    return false;
  std::vector<Value*> allowed;
  for (const auto& [value, coeff] : coeffs) {
    if (linearCoeffFor(key, value) != coeff)
      return false;
    allowed.push_back(value);
  }
  if (!linearKeyHasOnlyTerms(key, allowed))
    return false;
  return gepBasePlusConst(next, phi, stepBytes);
}

bool matchMatrixAddress(Value* addr,
                        GlobalVar* root,
                        Value* row,
                        Value* col,
                        int rowBytes) {
  auto* gep = dynamic_cast<Instruction*>(addr);
  if (!gep || gep->opcode() != Opcode::GEP)
    return false;
  LinearOffsetKey key = buildGEPLinearOffsetKey(gep);
  if (!key.valid || key.root != root || key.constant != 0)
    return false;
  if (linearCoeffFor(key, row) != rowBytes ||
      linearCoeffFor(key, col) != 4) {
    return false;
  }
  return linearKeyHasOnlyTerms(key, {row, col});
}

bool isMulOf(Value* value, Value* lhs, Value* rhs) {
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->opcode() != Opcode::Mul || inst->numOperands() != 2)
    return false;
  return (inst->operand(0) == lhs && inst->operand(1) == rhs) ||
         (inst->operand(0) == rhs && inst->operand(1) == lhs);
}

bool isAddOf(Value* value, Value* lhs, Value* rhs) {
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->opcode() != Opcode::Add || inst->numOperands() != 2)
    return false;
  return (inst->operand(0) == lhs && inst->operand(1) == rhs) ||
         (inst->operand(0) == rhs && inst->operand(1) == lhs);
}

bool loopValuesAreInternal(const analysis::NaturalLoop& loop) {
  for (auto* bb : loop.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst || inst->type().isVoid())
        continue;
      for (auto* user : inst->uses()) {
        if (!user || !user->parent())
          return false;
        if (!loop.blocks.count(user->parent()))
          return false;
      }
    }
  }
  return true;
}

bool loopWritesOnlyRoot(const analysis::NaturalLoop& loop, GlobalVar* dst) {
  for (auto* bb : loop.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      switch (inst->opcode()) {
      case Opcode::Call:
      case Opcode::Alloca:
      case Opcode::Ret:
      case Opcode::VStore:
        return false;
      case Opcode::Store:
        if (inst->numOperands() != 2 || memoryRoot(inst->operand(1)) != dst)
          return false;
        break;
      default:
        break;
      }
    }
  }
  return true;
}

// 物化过程会延后目标数组写入，并引入栈上的行累加器。它要求匹配的循环只访问
// 一个只读源矩阵和一个目标矩阵，不能包含调用、未知指针或向量内存操作。这里
// 会检查完整的外层循环区域，而不是只根据模式匹配器发现的两个加载来判断。
bool loopHasClosedMatrixMemory(const analysis::NaturalLoop& loop,
                               GlobalVar* lhs, GlobalVar* dst) {
  if (!lhs || !dst || lhs == dst || dst->isConst())
    return false;
  for (auto* bb : loop.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      switch (inst->opcode()) {
      case Opcode::Call:
      case Opcode::Alloca:
      case Opcode::VLoad:
      case Opcode::VStore:
        return false;
      case Opcode::Load: {
        if (inst->numOperands() != 1)
          return false;
        Value* root = memoryRoot(inst->operand(0));
        if (root != lhs && root != dst)
          return false;
        break;
      }
      case Opcode::Store:
        if (inst->numOperands() != 2 ||
            memoryRoot(inst->operand(1)) != dst)
          return false;
        break;
      default:
        break;
      }
      // 操作码白名单之外，再拒绝所有其余带副作用指令（Store 与控制流已豁免），
      // 防止隐藏的可观察行为混入待物化循环。
      if (inst->hasSideEffects() && inst->opcode() != Opcode::Store &&
          inst->opcode() != Opcode::Br &&
          inst->opcode() != Opcode::CondBr &&
          inst->opcode() != Opcode::Ret)
        return false;
    }
  }
  return true;
}

bool blockStoresToRoot(BasicBlock* bb, Value* root) {
  if (!bb || !root)
    return false;
  for (auto& instPtr : bb->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst || inst->opcode() != Opcode::Store || inst->numOperands() != 2)
      continue;
    if (memoryRoot(inst->operand(1)) == root)
      return true;
  }
  return false;
}

bool pathToTargetHasRootStore(BasicBlock* start,
                              BasicBlock* target,
                              Value* root) {
  if (!start || !target || !root)
    return true;
  std::queue<BasicBlock*> work;
  std::unordered_set<BasicBlock*> visited;
  work.push(start);
  visited.insert(start);
  while (!work.empty()) {
    BasicBlock* bb = work.front();
    work.pop();
    if (bb == target)
      continue;
    if (blockStoresToRoot(bb, root))
      return true;
    for (auto* succ : bb->succs()) {
      if (!succ)
        return true;
      if (visited.insert(succ).second)
        work.push(succ);
    }
  }
  return false;
}

bool matchConstantRowFillStore(Instruction* store,
                               GlobalVar* dst,
                               Value* row,
                               Value* col,
                               int rowBytes,
                               Instruction* rowPtrPhi,
                               int32_t& valueOut) {
  if (!store || store->opcode() != Opcode::Store || store->numOperands() != 2)
    return false;
  auto* ci = dynamic_cast<ConstInt*>(store->operand(0));
  if (!ci)
    return false;
  if (store->operand(1) != rowPtrPhi &&
      !matchMatrixAddress(store->operand(1), dst, row, col, rowBytes)) {
    return false;
  }
  valueOut = ci->value();
  return true;
}

bool matchSuffixFillGuard(Instruction* cmp,
                          Value* row,
                          Value*& suffixStart,
                          bool& fillOnTrue,
                          bool& startExclusive) {
  if (!cmp || cmp->opcode() != Opcode::ICmp || cmp->numOperands() != 2 ||
      !row) {
    return false;
  }
  const std::string cond = hir::canonicalLoopCmpCond(cmp);
  Value* lhs = cmp->operand(0);
  Value* rhs = cmp->operand(1);
  if (lhs == row && (cond == "ge" || cond == "gt")) {
    suffixStart = rhs;
    fillOnTrue = true;
    startExclusive = cond == "gt";
    return true;
  }
  if (lhs == row && (cond == "lt" || cond == "le")) {
    suffixStart = rhs;
    fillOnTrue = false;
    startExclusive = cond == "le";
    return true;
  }
  if (rhs == row && (cond == "le" || cond == "lt")) {
    suffixStart = lhs;
    fillOnTrue = true;
    startExclusive = cond == "lt";
    return true;
  }
  if (rhs == row && (cond == "gt" || cond == "ge")) {
    suffixStart = lhs;
    fillOnTrue = false;
    startExclusive = cond == "ge";
    return true;
  }
  return false;
}

bool collectHeaderLoopPhis(const hir::CanonicalLoop& loop,
                           std::vector<Instruction*>& phis,
                           std::vector<Value*>& initVals,
                           std::vector<Value*>* nextVals);

// 检测“嵌套循环以同一常量整行填充 dst”：先在内层 header 找行指针 phi
// （外层每轮推进 rowBytes），再扫外层循环区域，仅容忍匹配常量填充模式
// 且填充值一致的 Store。
bool loopStoresConstantRowFill(const analysis::NaturalLoop& outerLoop,
                               const hir::CanonicalLoop& outer,
                               const analysis::NaturalLoop& innerLoop,
                               const hir::CanonicalLoop& inner,
                               GlobalVar* dst,
                               int rowBytes,
                               int32_t& fillValueOut) {
  bool sawStore = false;
  int32_t fillValue = 0;
  Instruction* rowPtrPhi = nullptr;
  std::vector<Instruction*> innerPhis;
  std::vector<Value*> innerInits;
  std::vector<Value*> innerNexts;
  if (collectHeaderLoopPhis(inner, innerPhis, innerInits, &innerNexts)) {
    for (auto* phi : innerPhis) {
      if (phi != inner.ivPhi &&
          matchPointerPhi(phi, inner.preheader, inner.latch, dst,
                          {{outer.ivPhi, rowBytes}}, 4)) {
        rowPtrPhi = phi;
        break;
      }
    }
  }
  for (auto* bb : outerLoop.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      switch (inst->opcode()) {
      case Opcode::Call:
      case Opcode::Alloca:
      case Opcode::Ret:
      case Opcode::Load:
      case Opcode::VLoad:
      case Opcode::VStore:
        return false;
      case Opcode::Store: {
        int32_t thisValue = 0;
        if (!matchConstantRowFillStore(inst, dst, outer.ivPhi, inner.ivPhi,
                                       rowBytes, rowPtrPhi, thisValue)) {
          return false;
        }
        if (sawStore && thisValue != fillValue)
          return false;
        fillValue = thisValue;
        sawStore = true;
        break;
      }
      default:
        break;
      }
    }
  }
  if (!sawStore)
    return false;
  fillValueOut = fillValue;
  (void)innerLoop;
  return true;
}

ConstantRowSuffixFill findConstantRowSuffixFill(
    Function* func,
    const std::vector<analysis::NaturalLoop>& loops,
    const DomSet& doms,
    const analysis::NaturalLoop& consumerLoop,
    BasicBlock* consumerPreheader,
    GlobalVar* dst,
    Value* bound,
    int rowBytes) {
  ConstantRowSuffixFill best;
  if (!func || !consumerPreheader || !dst || !bound || rowBytes <= 0)
    return best;

  for (const auto& fillOuterLoop : loops) {
    if (!fillOuterLoop.header || fillOuterLoop.header == consumerLoop.header ||
        fillOuterLoop.blocks.count(consumerLoop.header)) {
      continue;
    }

    BasicBlock* fillPreheader = analysis::dedicatedPreheader(fillOuterLoop);
    hir::CanonicalLoop fillOuter;
    if (!fillPreheader ||
        !hir::recognizeCanonicalLoop(fillOuterLoop, fillPreheader, fillOuter) ||
        !fillOuter.ivIsCmpLhs ||
        hir::canonicalLoopCmpCond(fillOuter.cmp) != "lt" ||
        !sameValue(fillOuter.boundVal, bound) ||
        !loopHasSingleExit(fillOuterLoop, fillOuter.exit)) {
      continue;
    }
    if (!dominates(doms, fillOuter.exit, consumerPreheader))
      continue;

    auto acceptNestedFill = [&](BasicBlock* nestedPreheader,
                                BasicBlock* afterNested,
                                Value* suffixStart,
                                bool startExclusive) {
      if (!nestedPreheader || !afterNested || !suffixStart)
        return false;
      if (auto* startInst = dynamic_cast<Instruction*>(suffixStart)) {
        BasicBlock* startBlock = startInst->parent();
        if (!startBlock || fillOuterLoop.blocks.count(startBlock) ||
            !dominates(doms, startBlock, consumerPreheader)) {
          return false;
        }
      }
      BasicBlock* innerHeader = singleBrTarget(nestedPreheader);
      const auto* innerLoop = findLoopByHeader(loops, innerHeader);
      if (!innerLoop)
        return false;
      BasicBlock* innerPreheader = analysis::dedicatedPreheader(*innerLoop);
      hir::CanonicalLoop inner;
      if (innerPreheader != nestedPreheader ||
          !hir::recognizeCanonicalLoop(*innerLoop, innerPreheader, inner) ||
          !inner.ivIsCmpLhs || hir::canonicalLoopCmpCond(inner.cmp) != "lt" ||
          !isZeroInt(inner.initVal) ||
          !sameValue(inner.boundVal, bound)) {
        return false;
      }
      if (inner.exit != afterNested && singleBrTarget(inner.exit) != afterNested)
        return false;

      int32_t fillValue = 0;
      if (!loopStoresConstantRowFill(fillOuterLoop, fillOuter, *innerLoop,
                                     inner, dst, rowBytes, fillValue)) {
        return false;
      }
      if (pathToTargetHasRootStore(fillOuter.exit, consumerPreheader, dst))
        return false;

      // 优先选择最近的支配填充点。后面的填充出口只有在直线结构 CFG 中才能支配
      // 前面的填充点，而这里的事实只依赖这种结构。
      if (best.valid && best.exit &&
          dominates(doms, fillOuter.exit, best.exit)) {
        return true;
      }
      best.valid = true;
      best.start = suffixStart;
      best.startExclusive = startExclusive;
      best.exit = fillOuter.exit;
      best.value = fillValue;
      return true;
    };

    if (acceptNestedFill(fillOuter.body, fillOuter.latch, fillOuter.initVal,
                         false))
      continue;

    Instruction* guardTerm = fillOuter.body ? fillOuter.body->terminator() : nullptr;
    if (!guardTerm || guardTerm->opcode() != Opcode::CondBr ||
        guardTerm->numOperands() != 3) {
      continue;
    }
    auto* guardCmp = dynamic_cast<Instruction*>(guardTerm->operand(0));
    Value* suffixStart = nullptr;
    bool fillOnTrue = false;
    bool startExclusive = false;
    if (!matchSuffixFillGuard(guardCmp, fillOuter.ivPhi, suffixStart,
                              fillOnTrue, startExclusive))
      continue;
    auto* trueBB = dynamic_cast<BasicBlock*>(guardTerm->operand(1));
    auto* falseBB = dynamic_cast<BasicBlock*>(guardTerm->operand(2));
    BasicBlock* fillEntry = fillOnTrue ? trueBB : falseBB;
    if (acceptNestedFill(fillEntry, fillOuter.latch, suffixStart,
                         startExclusive))
      continue;
  }
  return best;
}

bool collectHeaderLoopPhis(const hir::CanonicalLoop& loop,
                           std::vector<Instruction*>& phis,
                           std::vector<Value*>& initVals,
                           std::vector<Value*>* nextVals) {
  if (!loop.header || !loop.preheader || !loop.latch)
    return false;
  for (auto& instPtr : loop.header->instructions()) {
    Instruction* phi = instPtr.get();
    if (!phi || phi->opcode() != Opcode::Phi)
      continue;
    if (phi->numOperands() != 4)
      return false;
    Value* init = nullptr;
    Value* next = nullptr;
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
      if (pred == loop.preheader) {
        init = phi->operand(i);
      } else if (pred == loop.latch) {
        next = phi->operand(i);
      } else {
        return false;
      }
    }
    if (!init || (nextVals && !next))
      return false;
    phis.push_back(phi);
    initVals.push_back(init);
    if (nextVals)
      nextVals->push_back(next);
  }
  return !phis.empty();
}

bool matchInPlaceRowLocalityLoop(
    Function* func,
    const analysis::NaturalLoop& outerLoop,
    const std::vector<analysis::NaturalLoop>& loops,
    const DomSet& doms,
    InPlaceRowLocalityPlan& plan) {
  auto reject = [&](const char* reason) {
    if (sycEnvironmentValue("ROW_LOCALITY_TRACE")) {
      std::cerr << "[row-locality] reject header="
                << (outerLoop.header ? outerLoop.header->name() : std::string("<null>"))
                << " reason=" << (reason ? reason : "-") << "\n";
    }
    return false;
  };
  if (!func || !func->parent() || !outerLoop.header)
    return reject("missing-func");
  BasicBlock* outerPreheader = analysis::dedicatedPreheader(outerLoop);
  hir::CanonicalLoop outer;
  if (!outerPreheader ||
      !hir::recognizeCanonicalLoop(outerLoop, outerPreheader, outer) ||
      !outer.ivIsCmpLhs || hir::canonicalLoopCmpCond(outer.cmp) != "lt" ||
      !isZeroInt(outer.initVal) || !loopHasSingleExit(outerLoop, outer.exit) ||
      !analysis::collectLeadingPhi(outer.exit).empty()) {
    return reject("outer-shape");
  }

  BasicBlock* middleHeader = singleBrTarget(outer.body);
  const auto* middleLoop = findLoopByHeader(loops, middleHeader);
  if (!middleLoop)
    return reject("no-middle-loop");
  BasicBlock* middlePreheader = analysis::dedicatedPreheader(*middleLoop);
  hir::CanonicalLoop middle;
  if (middlePreheader != outer.body ||
      !hir::recognizeCanonicalLoop(*middleLoop, middlePreheader, middle) ||
      middle.exit != outer.latch || !middle.ivIsCmpLhs ||
      hir::canonicalLoopCmpCond(middle.cmp) != "lt" ||
      !isZeroInt(middle.initVal) ||
      !sameValue(middle.boundVal, outer.boundVal)) {
    return reject("middle-shape");
  }

  BasicBlock* innerHeader = singleBrTarget(middle.body);
  const auto* innerLoop = findLoopByHeader(loops, innerHeader);
  if (!innerLoop)
    return reject("no-inner-loop");
  BasicBlock* innerPreheader = analysis::dedicatedPreheader(*innerLoop);
  hir::CanonicalLoop inner;
  if (innerPreheader != middle.body ||
      !hir::recognizeCanonicalLoop(*innerLoop, innerPreheader, inner) ||
      inner.exit != middle.latch || !inner.ivIsCmpLhs ||
      hir::canonicalLoopCmpCond(inner.cmp) != "lt" ||
      !isZeroInt(inner.initVal) ||
      !sameValue(inner.boundVal, outer.boundVal)) {
    return reject("inner-shape");
  }

  std::vector<Instruction*> middlePhis;
  std::vector<Value*> middleInits;
  std::vector<Value*> middleNexts;
  if (!collectHeaderLoopPhis(middle, middlePhis, middleInits, &middleNexts)) {
    return reject("middle-phis");
  }
  std::vector<Instruction*> innerPhis;
  std::vector<Value*> innerInits;
  std::vector<Value*> innerNexts;
  if (!collectHeaderLoopPhis(inner, innerPhis, innerInits, &innerNexts)) {
    return reject("inner-phis");
  }

  Instruction* store = nullptr;
  for (auto& instPtr : middle.latch->instructions()) {
    Instruction* inst = instPtr.get();
    if (inst && inst->opcode() == Opcode::Store) {
      if (store)
        return reject("multi-store");
      store = inst;
    }
  }
  if (!store || store->numOperands() != 2)
    return reject("no-store");

  GlobalVar* dst = nullptr;
  int rowElems = 0;
  int rowBytes = 0;
  bool storeAddrMatchesDst = false;
  if (auto* storeAddrGep = dynamic_cast<Instruction*>(store->operand(1))) {
    LinearOffsetKey storeKey = buildGEPLinearOffsetKey(storeAddrGep);
    auto* candidateDst = dynamic_cast<GlobalVar*>(storeKey.root);
    if (storeKey.valid && candidateDst && candidateDst->elemTy() == Type::i32() &&
        candidateDst->dims().size() >= 2 &&
        rowFitsLocalityScratchBudget(candidateDst->dims()[1])) {
      int candidateRowElems = candidateDst->dims()[1];
      int candidateRowBytes = candidateRowElems * kRowLocalityElementBytes;
      if (matchMatrixAddress(store->operand(1), candidateDst, outer.ivPhi,
                             middle.ivPhi, candidateRowBytes)) {
        dst = candidateDst;
        rowElems = candidateRowElems;
        rowBytes = candidateRowBytes;
        storeAddrMatchesDst = true;
      }
    }
  }

  // store 地址未直接匹配 dst GEP 时的回退路径：在 middle 层 header 找行指针 phi
  // （初值为 dst 的 GEP、外层每轮推进一行字节数），将其作为目标地址。
  Instruction* dstPtrPhi = nullptr;
  if (!storeAddrMatchesDst) {
    for (auto* phi : middlePhis) {
      if (phi == middle.ivPhi)
        continue;
      Value* init = nullptr;
      for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
        if (phi->operand(i + 1) == middle.preheader) {
          init = phi->operand(i);
          break;
        }
      }
      auto* initGep = dynamic_cast<Instruction*>(init);
      if (!initGep || initGep->opcode() != Opcode::GEP)
        continue;
      LinearOffsetKey initKey = buildGEPLinearOffsetKey(initGep);
      auto* candidateDst = dynamic_cast<GlobalVar*>(initKey.root);
      if (!candidateDst || candidateDst->elemTy() != Type::i32() ||
          candidateDst->dims().size() < 2 ||
          !rowFitsLocalityScratchBudget(candidateDst->dims()[1])) {
        continue;
      }
      int candidateRowElems = candidateDst->dims()[1];
      int candidateRowBytes = candidateRowElems * kRowLocalityElementBytes;
      if (matchPointerPhi(phi, middle.preheader, middle.latch, candidateDst,
                          {{outer.ivPhi, candidateRowBytes}}, 4)) {
        dst = candidateDst;
        rowElems = candidateRowElems;
        rowBytes = candidateRowBytes;
        dstPtrPhi = phi;
        break;
      }
    }
  }
  // 目标地址两路均未匹配则拒绝；ROW_LOCALITY_TRACE 开启时先打印 middle 层各 phi
  // 的初值/回边细节便于排查。
  if ((!storeAddrMatchesDst && (!dstPtrPhi || store->operand(1) != dstPtrPhi)) ||
      !dst || rowBytes <= 0) {
    if (sycEnvironmentValue("ROW_LOCALITY_TRACE")) {
      std::cerr << "[row-locality] dst-phi-detail header="
                << (outerLoop.header ? outerLoop.header->name() : std::string("<null>"))
                << " storeAddr="
                << (store->operand(1) ? store->operand(1)->name() : std::string("<null>"))
                << " middlePre="
                << (middle.preheader ? middle.preheader->name() : std::string("<null>"))
                << " middleLatch="
                << (middle.latch ? middle.latch->name() : std::string("<null>"))
                << "\n";
      for (auto* phi : middlePhis) {
        if (!phi)
          continue;
        std::cerr << "  phi=" << phi->name() << " ops=" << phi->numOperands();
        for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
          auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
          std::cerr << " [" << (phi->operand(i) ? phi->operand(i)->name() : std::string("<null>"))
                    << ","
                    << (pred ? pred->name() : std::string("<null>")) << "]";
        }
        std::cerr << "\n";
      }
    }
    return reject("dst-ptr-phi");
  }

  // 给内层 header phi 划分角色：每轮步进 4 字节的指针是右侧元素指针，
  // 0 初值的 i32 是行和累加器；存储值必须正是该累加器。
  Instruction* rightPtrPhi = nullptr;
  Instruction* sumPhi = nullptr;
  Value* sumNext = nullptr;
  for (size_t i = 0; i < innerPhis.size(); ++i) {
    Instruction* phi = innerPhis[i];
    if (phi == inner.ivPhi)
      continue;
    if (matchPointerPhi(phi, inner.preheader, inner.latch, dst,
                        {{middle.ivPhi, 4}}, rowBytes)) {
      rightPtrPhi = phi;
      continue;
    }
    if (phi->type().isI32() && isZeroInt(innerInits[i])) {
      sumPhi = phi;
      sumNext = innerNexts[i];
    }
  }
  if (!sumPhi || store->operand(0) != sumPhi) {
    if (sycEnvironmentValue("ROW_LOCALITY_TRACE")) {
      std::cerr << "[row-locality] inner-phi-detail header="
                << (outerLoop.header ? outerLoop.header->name() : std::string("<null>"))
                << " right=" << (rightPtrPhi ? rightPtrPhi->name() : std::string("<null>"))
                << " sum=" << (sumPhi ? sumPhi->name() : std::string("<null>"))
                << " storeVal="
                << (store->operand(0) ? store->operand(0)->name() : std::string("<null>"))
                << "\n";
      for (size_t i = 0; i < innerPhis.size(); ++i) {
        Instruction* phi = innerPhis[i];
        if (!phi)
          continue;
        bool ptrRole =
            phi != inner.ivPhi &&
            matchPointerPhi(phi, inner.preheader, inner.latch, dst,
                            {{middle.ivPhi, 4}}, rowBytes);
        bool sumRole = phi != inner.ivPhi && phi->type().isI32() &&
                       isZeroInt(innerInits[i]);
        std::cerr << "  phi=" << phi->name()
                  << " type=" << typeName(phi->type())
                  << " init="
                  << (innerInits[i] ? innerInits[i]->name() : std::string("<null>"))
                  << " next="
                  << (innerNexts[i] ? innerNexts[i]->name() : std::string("<null>"))
                  << " ptrRole=" << (ptrRole ? 1 : 0)
                  << " sumRole=" << (sumRole ? 1 : 0) << "\n";
      }
    }
    return reject("inner-phi-roles");
  }

  Instruction* leftLoad = nullptr;
  Instruction* rightLoad = nullptr;
  for (auto& instPtr : inner.body->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst || inst->opcode() != Opcode::Load || inst->numOperands() != 1)
      continue;
    bool isRightLoad =
        (rightPtrPhi && inst->operand(0) == rightPtrPhi) ||
        matchMatrixAddress(inst->operand(0), dst, inner.ivPhi, middle.ivPhi,
                           rowBytes);
    if (isRightLoad) {
      if (rightLoad)
        return reject("multi-right-load");
      rightLoad = inst;
      continue;
    }
    auto* root = dynamic_cast<GlobalVar*>(memoryRoot(inst->operand(0)));
    if (root && root != dst) {
      if (leftLoad)
        return reject("multi-left-load");
      leftLoad = inst;
    }
  }
  if (!leftLoad || !rightLoad)
    return reject("loads");
  auto* lhs = dynamic_cast<GlobalVar*>(memoryRoot(leftLoad->operand(0)));
  if (!lhs || lhs == dst || lhs->elemTy() != Type::i32() ||
      lhs->dims().size() < 2 || lhs->dims()[1] != rowElems) {
    return reject("bad-lhs");
  }
  if (!matchMatrixAddress(leftLoad->operand(0), lhs, outer.ivPhi,
                          inner.ivPhi, rowBytes)) {
    return reject("lhs-address");
  }

  if (!loopHasClosedMatrixMemory(outerLoop, lhs, dst))
    return reject("closed-memory");

  auto* add = dynamic_cast<Instruction*>(sumNext);
  if (!add || add->opcode() != Opcode::Add || add->numOperands() != 2)
    return reject("sum-next");
  Value* product = add->operand(0) == sumPhi ? add->operand(1)
                                             : (add->operand(1) == sumPhi
                                                    ? add->operand(0)
                                                    : nullptr);
  if (!product || !isMulOf(product, leftLoad, rightLoad))
    return reject("product");

  if (!loopWritesOnlyRoot(outerLoop, dst) || !loopValuesAreInternal(outerLoop))
    return reject("side-effects-or-uses");

  plan.func = func;
  plan.outerLoop = outerLoop;
  plan.outer = outer;
  plan.middle = middle;
  plan.inner = inner;
  plan.dst = dst;
  plan.lhs = lhs;
  plan.rowElems = rowElems;
  plan.rowBytes = rowBytes;
  plan.bound = outer.boundVal;
  ConstantRowSuffixFill suffix = findConstantRowSuffixFill(
      func, loops, doms, outerLoop, outer.preheader, dst, outer.boundVal,
      rowBytes);
  if (suffix.valid && suffix.start) {
    plan.hasConstantSuffixRows = true;
    plan.constantSuffixStart = suffix.start;
    plan.constantSuffixStartExclusive = suffix.startExclusive;
    plan.constantSuffixValue = suffix.value;
  }
  return true;
}

bool isProfitableInPlaceRowLocality(const InPlaceRowLocalityPlan& plan) {
  // 该物化过程把一行目标数据保存在栈上临时值中，在所有 k 次更新完成后再写回
  // 全局行。这里的预算用于控制栈空间和局部性收益。
  constexpr int kMaxFullRowTempElems = 2048;
  return plan.rowElems > 0 && plan.rowElems <= kMaxFullRowTempElems;
}

Instruction* appendInst(BasicBlock* bb, Opcode op, Type ty,
                        const std::string& name,
                        std::initializer_list<Value*> operands = {}) {
  auto* inst = new Instruction(op, ty, name);
  for (auto* operand : operands)
    inst->addOperand(operand);
  bb->pushBack(inst);
  return inst;
}

Instruction* appendICmp(BasicBlock* bb, const std::string& cond,
                        Value* lhs, Value* rhs,
                        const std::string& name) {
  auto* inst = appendInst(bb, Opcode::ICmp, Type::i32(), cond + "_" + name,
                         {lhs, rhs});
  return inst;
}

Value* materializeClampedSuffixStart(const InPlaceRowLocalityPlan& plan,
                                     Module* mod) {
  BasicBlock* preheader = plan.outer.preheader;
  if (!mod || !preheader || !preheader->terminator() ||
      !plan.constantSuffixStart || !plan.bound) {
    return nullptr;
  }

  Instruction* terminator = preheader->terminator();
  auto insert = [&](Opcode opcode, Type type, const std::string& name,
                    std::initializer_list<Value*> operands) -> Instruction* {
    auto* inst = new Instruction(opcode, type, name);
    for (auto* operand : operands)
      inst->addOperand(operand);
    preheader->insertBefore(terminator, inst);
    return inst;
  };

  ConstInt* zero = mod->getConstInt(0);
  auto* boundNegative = insert(Opcode::ICmp, Type::i32(),
                               "lt_rowloc.suffix.bound.negative",
                               {plan.bound, zero});
  Value* nonnegativeBound = insert(
      Opcode::Select, Type::i32(), "rowloc.suffix.bound.nonnegative",
      {boundNegative, zero, plan.bound});

  Value* rawStart = plan.constantSuffixStart;
  auto* startNegative = insert(Opcode::ICmp, Type::i32(),
                               "lt_rowloc.suffix.start.negative",
                               {rawStart, zero});
  auto* startAtOrAboveBound = insert(
      Opcode::ICmp, Type::i32(), "ge_rowloc.suffix.start.upper",
      {rawStart, nonnegativeBound});

  Value* interiorStart = rawStart;
  if (plan.constantSuffixStartExclusive) {
    // 在两条截断路径上都给 Add 提供无害的哨兵值。即使 rawStart 为 INT_MIN 或
    // INT_MAX，也能让边界调整本身保持在整数范围内；外层 Select 仍会产生
    // (rawStart, +inf) 与 [0, nonnegativeBound) 的精确交集。
    Value* upperSafeBase = insert(
        Opcode::Select, Type::i32(), "rowloc.suffix.start.upper.safe",
        {startAtOrAboveBound, zero, rawStart});
    Value* safeBase = insert(
        Opcode::Select, Type::i32(), "rowloc.suffix.start.lower.safe",
        {startNegative, mod->getConstInt(-1), upperSafeBase});
    interiorStart = insert(Opcode::Add, Type::i32(),
                           "rowloc.suffix.start.exclusive",
                           {safeBase, mod->getConstInt(1)});
  }
  Value* upperClamped = insert(
      Opcode::Select, Type::i32(), "rowloc.suffix.start.upper.clamped",
      {startAtOrAboveBound, nonnegativeBound, interiorStart});
  return insert(Opcode::Select, Type::i32(),
                "rowloc.suffix.start.clamped",
                {startNegative, zero, upperClamped});
}

Instruction* appendPhiStart(BasicBlock* bb, Type ty, const std::string& name,
                            Value* init, BasicBlock* initPred) {
  auto* phi = new Instruction(Opcode::Phi, ty, name);
  phi->addOperand(init);
  phi->addOperand(initPred);
  bb->pushBack(phi);
  return phi;
}

void addPhiIncoming(Instruction* phi, Value* value, BasicBlock* pred) {
  phi->addOperand(value);
  phi->addOperand(pred);
}

Instruction* createEntryArrayAlloca(Function* func,
                                    Type elemTy,
                                    int bytes,
                                    int elems,
                                    const std::string& name) {
  if (!func || !func->entryBlock())
    return nullptr;
  auto* alloca = new Instruction(Opcode::Alloca, Type::ptr(), name);
  alloca->setMemoryInfo(elemTy, bytes, true, {elems});
  func->addAlloca(alloca);
  Instruction* firstNonAlloca = nullptr;
  for (auto& instPtr : func->entryBlock()->instructions()) {
    if (instPtr->opcode() != Opcode::Alloca) {
      firstNonAlloca = instPtr.get();
      break;
    }
  }
  if (firstNonAlloca)
    func->entryBlock()->insertBefore(firstNonAlloca, alloca);
  else
    func->entryBlock()->pushBack(alloca);
  return alloca;
}

bool isPowerOfTwoPositive(int value, int& shift) {
  if (value <= 0 || (value & (value - 1)) != 0)
    return false;
  shift = 0;
  while ((1 << shift) != value)
    ++shift;
  return true;
}

Value* buildScaledIndex(BasicBlock* bb, Module* mod, Value* value, int scale,
                        const std::string& name) {
  if (scale == 1)
    return value;
  int shift = 0;
  if (isPowerOfTwoPositive(scale, shift)) {
    return appendInst(bb, Opcode::Shl, Type::i32(), name + ".shl",
                      {value, mod->getConstInt(shift)});
  }
  return appendInst(bb, Opcode::Mul, Type::i32(), name + ".mul",
                    {value, mod->getConstInt(scale)});
}

Value* buildMatrixAddress(BasicBlock* bb, Module* mod, Value* base,
                          Value* row, Value* col, int rowBytes,
                          const std::string& name) {
  Value* rowOff = buildScaledIndex(bb, mod, row, rowBytes, name + ".row");
  Value* colOff = buildScaledIndex(bb, mod, col, 4, name + ".col");
  auto* off = appendInst(bb, Opcode::Add, Type::i32(), name + ".off",
                         {rowOff, colOff});
  return appendInst(bb, Opcode::GEP, Type::ptr(), name, {base, off});
}

Value* buildVectorAddress(BasicBlock* bb, Module* mod, Value* base,
                          Value* index, const std::string& name) {
  Value* off = buildScaledIndex(bb, mod, index, 4, name + ".idx");
  return appendInst(bb, Opcode::GEP, Type::ptr(), name, {base, off});
}

bool materializeInPlaceRowLocalityUntiled(const InPlaceRowLocalityPlan& plan) {
  Function* func = plan.func;
  if (!func || !func->parent() || !plan.outer.preheader || !plan.outer.header ||
      !plan.outer.exit) {
    return false;
  }
  Module* mod = func->parent();
  auto* acc = createEntryArrayAlloca(func, Type::i32(), plan.rowBytes,
                                     plan.rowElems, "rowloc.acc");
  if (!acc)
    return false;
  const bool useSuffixRows =
      plan.hasConstantSuffixRows && plan.constantSuffixStart;
  Value* suffixStart =
      useSuffixRows ? materializeClampedSuffixStart(plan, mod) : nullptr;
  if (useSuffixRows && !suffixStart)
    return false;

  BasicBlock* iHeader = func->newBlock("rowloc.i.header");
  BasicBlock* initHeader = func->newBlock("rowloc.init.header");
  BasicBlock* initBody = func->newBlock("rowloc.init.body");
  BasicBlock* preKHeader = func->newBlock("rowloc.prek.header");
  BasicBlock* preKBody = func->newBlock("rowloc.prek.body");
  BasicBlock* preJHeader = func->newBlock("rowloc.prej.header");
  BasicBlock* preJBody = func->newBlock("rowloc.prej.body");
  BasicBlock* preKLatch = func->newBlock("rowloc.prek.latch");
  BasicBlock* selfPrep = func->newBlock("rowloc.self.prep");
  BasicBlock* selfJHeader = func->newBlock("rowloc.selfj.header");
  BasicBlock* selfJBody = func->newBlock("rowloc.selfj.body");
  BasicBlock* postKInitBB = func->newBlock("rowloc.postk.init");
  BasicBlock* postKHeader = func->newBlock("rowloc.postk.header");
  BasicBlock* postKBody = func->newBlock("rowloc.postk.body");
  BasicBlock* postJHeader = func->newBlock("rowloc.postj.header");
  BasicBlock* postJBody = func->newBlock("rowloc.postj.body");
  BasicBlock* postKLatch = func->newBlock("rowloc.postk.latch");
  BasicBlock* suffixPrep =
      useSuffixRows ? func->newBlock("rowloc.suffix.prep") : nullptr;
  BasicBlock* suffixTailHeader =
      (useSuffixRows && plan.constantSuffixValue != 0)
          ? func->newBlock("rowloc.suffix.tail.header")
          : nullptr;
  BasicBlock* suffixTailBody =
      (useSuffixRows && plan.constantSuffixValue != 0)
          ? func->newBlock("rowloc.suffix.tail.body")
          : nullptr;
  BasicBlock* suffixApplyHeader =
      (useSuffixRows && plan.constantSuffixValue != 0)
          ? func->newBlock("rowloc.suffix.apply.header")
          : nullptr;
  BasicBlock* suffixApplyBody =
      (useSuffixRows && plan.constantSuffixValue != 0)
          ? func->newBlock("rowloc.suffix.apply.body")
          : nullptr;
  BasicBlock* storeHeader = func->newBlock("rowloc.store.header");
  BasicBlock* storeBody = func->newBlock("rowloc.store.body");
  BasicBlock* iLatch = func->newBlock("rowloc.i.latch");

  ConstInt* zero = mod->getConstInt(0);
  ConstInt* one = mod->getConstInt(1);

  if (!analysis::redirectSuccessor(plan.outer.preheader, plan.outer.header, iHeader))
    return false;

  auto* iPhi = appendPhiStart(iHeader, Type::i32(), "rowloc.i", zero,
                              plan.outer.preheader);
  auto* iCmp = appendICmp(iHeader, "lt", iPhi, plan.bound, "rowloc.i");
  appendInst(iHeader, Opcode::CondBr, Type::voidTy(), "",
             {iCmp, initHeader, plan.outer.exit});

  auto* initJPhi = appendPhiStart(initHeader, Type::i32(), "rowloc.init.j",
                                  zero, iHeader);
  auto* initCmp = appendICmp(initHeader, "lt", initJPhi, plan.bound,
                             "rowloc.init");
  appendInst(initHeader, Opcode::CondBr, Type::voidTy(), "",
             {initCmp, initBody, preKHeader});

  Value* initAccAddr = buildVectorAddress(initBody, mod, acc, initJPhi,
                                          "rowloc.init.acc");
  appendInst(initBody, Opcode::Store, Type::voidTy(), "",
             {zero, initAccAddr});
  auto* initJNext = appendInst(initBody, Opcode::Add, Type::i32(),
                               "rowloc.init.j.next", {initJPhi, one});
  appendInst(initBody, Opcode::Br, Type::voidTy(), "", {initHeader});
  addPhiIncoming(initJPhi, initJNext, initBody);

  auto* preKPhi = appendPhiStart(preKHeader, Type::i32(), "rowloc.prek",
                                 zero, initHeader);
  auto* preKCmp = appendICmp(preKHeader, "lt", preKPhi, iPhi, "rowloc.prek");
  appendInst(preKHeader, Opcode::CondBr, Type::voidTy(), "",
             {preKCmp, preKBody, selfPrep});

  Value* preCoeffAddr = buildMatrixAddress(preKBody, mod, plan.lhs, iPhi,
                                           preKPhi, plan.rowBytes,
                                           "rowloc.prek.coeff.addr");
  auto* preCoeff = appendInst(preKBody, Opcode::Load, Type::i32(),
                              "rowloc.prek.coeff", {preCoeffAddr});
  appendInst(preKBody, Opcode::Br, Type::voidTy(), "", {preJHeader});

  auto* preJPhi = appendPhiStart(preJHeader, Type::i32(), "rowloc.prej",
                                 zero, preKBody);
  auto* preJCmp = appendICmp(preJHeader, "lt", preJPhi, plan.bound,
                             "rowloc.prej");
  appendInst(preJHeader, Opcode::CondBr, Type::voidTy(), "",
             {preJCmp, preJBody, preKLatch});

  Value* preAccAddr = buildVectorAddress(preJBody, mod, acc, preJPhi,
                                         "rowloc.prej.acc");
  auto* preAcc = appendInst(preJBody, Opcode::Load, Type::i32(),
                            "rowloc.prej.acc.val", {preAccAddr});
  Value* preRhsAddr = buildMatrixAddress(preJBody, mod, plan.dst, preKPhi,
                                         preJPhi, plan.rowBytes,
                                         "rowloc.prej.rhs");
  auto* preRhs = appendInst(preJBody, Opcode::Load, Type::i32(),
                            "rowloc.prej.rhs.val", {preRhsAddr});
  auto* preProd = appendInst(preJBody, Opcode::Mul, Type::i32(),
                             "rowloc.prej.prod", {preCoeff, preRhs});
  auto* preSum = appendInst(preJBody, Opcode::Add, Type::i32(),
                            "rowloc.prej.sum", {preAcc, preProd});
  appendInst(preJBody, Opcode::Store, Type::voidTy(), "", {preSum, preAccAddr});
  auto* preJNext = appendInst(preJBody, Opcode::Add, Type::i32(),
                              "rowloc.prej.next", {preJPhi, one});
  appendInst(preJBody, Opcode::Br, Type::voidTy(), "", {preJHeader});
  addPhiIncoming(preJPhi, preJNext, preJBody);

  auto* preKNext = appendInst(preKLatch, Opcode::Add, Type::i32(),
                              "rowloc.prek.next", {preKPhi, one});
  appendInst(preKLatch, Opcode::Br, Type::voidTy(), "", {preKHeader});
  addPhiIncoming(preKPhi, preKNext, preKLatch);

  Value* selfCoeffAddr = buildMatrixAddress(selfPrep, mod, plan.lhs, iPhi,
                                            iPhi, plan.rowBytes,
                                            "rowloc.self.coeff.addr");
  auto* selfCoeff = appendInst(selfPrep, Opcode::Load, Type::i32(),
                               "rowloc.self.coeff", {selfCoeffAddr});
  appendInst(selfPrep, Opcode::Br, Type::voidTy(), "", {selfJHeader});

  auto* selfJPhi = appendPhiStart(selfJHeader, Type::i32(), "rowloc.selfj",
                                  zero, selfPrep);
  auto* selfJCmp = appendICmp(selfJHeader, "lt", selfJPhi, plan.bound,
                              "rowloc.selfj");
  appendInst(selfJHeader, Opcode::CondBr, Type::voidTy(), "",
             {selfJCmp, selfJBody, postKInitBB});

  Value* selfAccAddr = buildVectorAddress(selfJBody, mod, acc, selfJPhi,
                                          "rowloc.selfj.acc");
  auto* selfAcc = appendInst(selfJBody, Opcode::Load, Type::i32(),
                             "rowloc.selfj.acc.val", {selfAccAddr});
  Value* selfRhsAddr = buildMatrixAddress(selfJBody, mod, plan.dst, iPhi,
                                          selfJPhi, plan.rowBytes,
                                          "rowloc.selfj.rhs");
  auto* selfRhs = appendInst(selfJBody, Opcode::Load, Type::i32(),
                             "rowloc.selfj.rhs.val", {selfRhsAddr});
  auto* selfProd = appendInst(selfJBody, Opcode::Mul, Type::i32(),
                              "rowloc.selfj.prod", {selfCoeff, selfRhs});
  auto* selfSum = appendInst(selfJBody, Opcode::Add, Type::i32(),
                             "rowloc.selfj.sum", {selfAcc, selfProd});
  appendInst(selfJBody, Opcode::Store, Type::voidTy(), "",
             {selfSum, selfAccAddr});
  auto* selfJNext = appendInst(selfJBody, Opcode::Add, Type::i32(),
                               "rowloc.selfj.next", {selfJPhi, one});
  appendInst(selfJBody, Opcode::Br, Type::voidTy(), "", {selfJHeader});
  addPhiIncoming(selfJPhi, selfJNext, selfJBody);

  auto* postKInit = appendInst(postKInitBB, Opcode::Add, Type::i32(),
                               "rowloc.postk.init", {iPhi, one});
  if (useSuffixRows) {
    auto* needsPrefix = appendICmp(postKInitBB, "lt", iPhi,
                                   suffixStart,
                                   "rowloc.suffix.prefix");
    appendInst(postKInitBB, Opcode::CondBr, Type::voidTy(), "",
               {needsPrefix, postKHeader, suffixPrep});
  } else {
    appendInst(postKInitBB, Opcode::Br, Type::voidTy(), "", {postKHeader});
  }

  auto* postKPhi = appendPhiStart(postKHeader, Type::i32(), "rowloc.postk",
                                  postKInit, postKInitBB);
  Value* postKBound = useSuffixRows ? suffixStart : plan.bound;
  auto* postKCmp = appendICmp(postKHeader, "lt", postKPhi, postKBound,
                              "rowloc.postk");
  appendInst(postKHeader, Opcode::CondBr, Type::voidTy(), "",
             {postKCmp, postKBody, useSuffixRows ? suffixPrep : storeHeader});

  Value* postCoeffAddr = buildMatrixAddress(postKBody, mod, plan.lhs, iPhi,
                                            postKPhi, plan.rowBytes,
                                            "rowloc.postk.coeff.addr");
  auto* postCoeff = appendInst(postKBody, Opcode::Load, Type::i32(),
                               "rowloc.postk.coeff", {postCoeffAddr});
  appendInst(postKBody, Opcode::Br, Type::voidTy(), "", {postJHeader});

  auto* postJPhi = appendPhiStart(postJHeader, Type::i32(), "rowloc.postj",
                                  zero, postKBody);
  auto* postJCmp = appendICmp(postJHeader, "lt", postJPhi, plan.bound,
                              "rowloc.postj");
  appendInst(postJHeader, Opcode::CondBr, Type::voidTy(), "",
             {postJCmp, postJBody, postKLatch});

  Value* postAccAddr = buildVectorAddress(postJBody, mod, acc, postJPhi,
                                          "rowloc.postj.acc");
  auto* postAcc = appendInst(postJBody, Opcode::Load, Type::i32(),
                             "rowloc.postj.acc.val", {postAccAddr});
  Value* postRhsAddr = buildMatrixAddress(postJBody, mod, plan.dst, postKPhi,
                                          postJPhi, plan.rowBytes,
                                          "rowloc.postj.rhs");
  auto* postRhs = appendInst(postJBody, Opcode::Load, Type::i32(),
                             "rowloc.postj.rhs.val", {postRhsAddr});
  auto* postProd = appendInst(postJBody, Opcode::Mul, Type::i32(),
                              "rowloc.postj.prod", {postCoeff, postRhs});
  auto* postSum = appendInst(postJBody, Opcode::Add, Type::i32(),
                             "rowloc.postj.sum", {postAcc, postProd});
  appendInst(postJBody, Opcode::Store, Type::voidTy(), "",
             {postSum, postAccAddr});
  auto* postJNext = appendInst(postJBody, Opcode::Add, Type::i32(),
                               "rowloc.postj.next", {postJPhi, one});
  appendInst(postJBody, Opcode::Br, Type::voidTy(), "", {postJHeader});
  addPhiIncoming(postJPhi, postJNext, postJBody);

  auto* postKNext = appendInst(postKLatch, Opcode::Add, Type::i32(),
                               "rowloc.postk.next", {postKPhi, one});
  appendInst(postKLatch, Opcode::Br, Type::voidTy(), "", {postKHeader});
  addPhiIncoming(postKPhi, postKNext, postKLatch);

  BasicBlock* storeEntryPred = postKHeader;
  if (useSuffixRows) {
    auto* suffixStartPhi =
        appendPhiStart(suffixPrep, Type::i32(), "rowloc.suffix.start",
                       postKInit, postKInitBB);
    addPhiIncoming(suffixStartPhi, postKPhi, postKHeader);
    if (plan.constantSuffixValue == 0) {
      appendInst(suffixPrep, Opcode::Br, Type::voidTy(), "", {storeHeader});
      storeEntryPred = suffixPrep;
    } else {
      appendInst(suffixPrep, Opcode::Br, Type::voidTy(), "",
                 {suffixTailHeader});

      auto* tailKPhi =
          appendPhiStart(suffixTailHeader, Type::i32(), "rowloc.suffix.k",
                         suffixStartPhi, suffixPrep);
      auto* tailSumPhi =
          appendPhiStart(suffixTailHeader, Type::i32(), "rowloc.suffix.sum",
                         zero, suffixPrep);
      auto* tailCmp = appendICmp(suffixTailHeader, "lt", tailKPhi, plan.bound,
                                 "rowloc.suffix.k");
      appendInst(suffixTailHeader, Opcode::CondBr, Type::voidTy(), "",
                 {tailCmp, suffixTailBody, suffixApplyHeader});

      Value* tailLhsAddr = buildMatrixAddress(
          suffixTailBody, mod, plan.lhs, iPhi, tailKPhi, plan.rowBytes,
          "rowloc.suffix.lhs");
      auto* tailLhs = appendInst(suffixTailBody, Opcode::Load, Type::i32(),
                                 "rowloc.suffix.lhs.val", {tailLhsAddr});
      auto* tailSumNext = appendInst(suffixTailBody, Opcode::Add, Type::i32(),
                                     "rowloc.suffix.sum.next",
                                     {tailSumPhi, tailLhs});
      auto* tailKNext = appendInst(suffixTailBody, Opcode::Add, Type::i32(),
                                   "rowloc.suffix.k.next", {tailKPhi, one});
      appendInst(suffixTailBody, Opcode::Br, Type::voidTy(), "",
                 {suffixTailHeader});
      addPhiIncoming(tailKPhi, tailKNext, suffixTailBody);
      addPhiIncoming(tailSumPhi, tailSumNext, suffixTailBody);

      auto* suffixJPhi =
          appendPhiStart(suffixApplyHeader, Type::i32(), "rowloc.suffix.j",
                         zero, suffixTailHeader);
      auto* suffixJCmp = appendICmp(suffixApplyHeader, "lt", suffixJPhi,
                                    plan.bound, "rowloc.suffix.j");
      appendInst(suffixApplyHeader, Opcode::CondBr, Type::voidTy(), "",
                 {suffixJCmp, suffixApplyBody, storeHeader});

      Value* suffixAccAddr = buildVectorAddress(
          suffixApplyBody, mod, acc, suffixJPhi, "rowloc.suffix.acc");
      auto* suffixAcc = appendInst(suffixApplyBody, Opcode::Load, Type::i32(),
                                   "rowloc.suffix.acc.val", {suffixAccAddr});
      Value* suffixContribution = tailSumPhi;
      if (plan.constantSuffixValue != 1) {
        suffixContribution =
            appendInst(suffixApplyBody, Opcode::Mul, Type::i32(),
                       "rowloc.suffix.contrib",
                       {mod->getConstInt(plan.constantSuffixValue),
                        tailSumPhi});
      }
      auto* suffixNewAcc =
          appendInst(suffixApplyBody, Opcode::Add, Type::i32(),
                     "rowloc.suffix.acc.next",
                     {suffixAcc, suffixContribution});
      appendInst(suffixApplyBody, Opcode::Store, Type::voidTy(), "",
                 {suffixNewAcc, suffixAccAddr});
      auto* suffixJNext =
          appendInst(suffixApplyBody, Opcode::Add, Type::i32(),
                     "rowloc.suffix.j.next", {suffixJPhi, one});
      appendInst(suffixApplyBody, Opcode::Br, Type::voidTy(), "",
                 {suffixApplyHeader});
      addPhiIncoming(suffixJPhi, suffixJNext, suffixApplyBody);
      storeEntryPred = suffixApplyHeader;
    }
  }

  auto* storeJPhi = appendPhiStart(storeHeader, Type::i32(), "rowloc.storej",
                                   zero, storeEntryPred);
  auto* storeJCmp = appendICmp(storeHeader, "lt", storeJPhi, plan.bound,
                               "rowloc.storej");
  appendInst(storeHeader, Opcode::CondBr, Type::voidTy(), "",
             {storeJCmp, storeBody, iLatch});

  Value* storeAccAddr = buildVectorAddress(storeBody, mod, acc, storeJPhi,
                                           "rowloc.store.acc");
  auto* storeVal = appendInst(storeBody, Opcode::Load, Type::i32(),
                              "rowloc.store.val", {storeAccAddr});
  Value* storeDstAddr = buildMatrixAddress(storeBody, mod, plan.dst, iPhi,
                                           storeJPhi, plan.rowBytes,
                                           "rowloc.store.dst");
  appendInst(storeBody, Opcode::Store, Type::voidTy(), "",
             {storeVal, storeDstAddr});
  auto* storeJNext = appendInst(storeBody, Opcode::Add, Type::i32(),
                                "rowloc.storej.next", {storeJPhi, one});
  appendInst(storeBody, Opcode::Br, Type::voidTy(), "", {storeHeader});
  addPhiIncoming(storeJPhi, storeJNext, storeBody);

  auto* iNext = appendInst(iLatch, Opcode::Add, Type::i32(), "rowloc.i.next",
                           {iPhi, one});
  appendInst(iLatch, Opcode::Br, Type::voidTy(), "", {iHeader});
  addPhiIncoming(iPhi, iNext, iLatch);

  func->recomputeCFG();
  return true;
}

  // 对归约维度进行分块，同时保留连续的 j 扫描。相邻的源行可以一起保持在
  // 热数据中，系数在完整目标行扫描期间重复使用，一个栈累加器的加载和存储
  // 可以服务多次乘加。这样既减少原物化过程每次乘加的行缓冲流量，也避免在
  // 带步长的 k 扫描中长期保留多列目标数据造成的缓存浪费和寄存器压力。
bool materializeInPlaceRowLocalityKBlocked(
    const InPlaceRowLocalityPlan& plan) {
  Function* func = plan.func;
  if (!func || !func->parent() || !plan.outer.preheader ||
      !plan.outer.header || !plan.outer.exit) {
    return false;
  }

  constexpr int kReductionBlock = 6;
  Module* mod = func->parent();
  auto* rowAcc = createEntryArrayAlloca(func, Type::i32(), plan.rowBytes,
                                        plan.rowElems, "rowloc.acc");
  if (!rowAcc)
    return false;

  const bool useSuffixRows =
      plan.hasConstantSuffixRows && plan.constantSuffixStart;
  Value* suffixStart =
      useSuffixRows ? materializeClampedSuffixStart(plan, mod) : nullptr;
  if (useSuffixRows && !suffixStart)
    return false;
  const bool applySuffix = useSuffixRows && plan.constantSuffixValue != 0;

  BasicBlock* iHeader = func->newBlock("rowloc.i.header");
  BasicBlock* iPrep = func->newBlock("rowloc.i.prep");
  BasicBlock* initHeader = func->newBlock("rowloc.init.header");
  BasicBlock* initBody = func->newBlock("rowloc.init.body");
  BasicBlock* blockKHeader = func->newBlock("rowloc.block.k.header");
  BasicBlock* blockKSetup = func->newBlock("rowloc.block.k.setup");
  BasicBlock* blockJHeader = func->newBlock("rowloc.block.j.header");
  BasicBlock* blockJBody = func->newBlock("rowloc.block.j.body");
  BasicBlock* blockKLatch = func->newBlock("rowloc.block.k.latch");
  BasicBlock* tailKHeader = func->newBlock("rowloc.ktail.header");
  BasicBlock* tailKSetup = func->newBlock("rowloc.ktail.setup");
  BasicBlock* tailJHeader = func->newBlock("rowloc.ktail.j.header");
  BasicBlock* tailJBody = func->newBlock("rowloc.ktail.j.body");
  BasicBlock* tailKLatch = func->newBlock("rowloc.ktail.latch");
  BasicBlock* suffixHeader =
      applySuffix ? func->newBlock("rowloc.suffix.k.header") : nullptr;
  BasicBlock* suffixBody =
      applySuffix ? func->newBlock("rowloc.suffix.k.body") : nullptr;
  BasicBlock* suffixDone =
      applySuffix ? func->newBlock("rowloc.suffix.k.done") : nullptr;
  BasicBlock* suffixApplyHeader =
      applySuffix ? func->newBlock("rowloc.suffix.j.header") : nullptr;
  BasicBlock* suffixApplyBody =
      applySuffix ? func->newBlock("rowloc.suffix.j.body") : nullptr;
  BasicBlock* storeHeader = func->newBlock("rowloc.store.header");
  BasicBlock* storeBody = func->newBlock("rowloc.store.body");
  BasicBlock* iLatch = func->newBlock("rowloc.i.latch");

  ConstInt* zero = mod->getConstInt(0);
  ConstInt* one = mod->getConstInt(1);

  if (!analysis::redirectSuccessor(plan.outer.preheader, plan.outer.header,
                                   iHeader)) {
    return false;
  }

  auto* iPhi = appendPhiStart(iHeader, Type::i32(), "rowloc.i", zero,
                              plan.outer.preheader);
  auto* iCmp = appendICmp(iHeader, "lt", iPhi, plan.bound, "rowloc.i");
  appendInst(iHeader, Opcode::CondBr, Type::voidTy(), "",
             {iCmp, iPrep, plan.outer.exit});

  auto* iPlusOne = appendInst(iPrep, Opcode::Add, Type::i32(),
                              "rowloc.i.plus.one", {iPhi, one});
  Value* nonconstantKLimit = plan.bound;
  if (useSuffixRows) {
    auto* beforeSuffix = appendICmp(iPrep, "lt", iPhi,
                                    suffixStart,
                                    "rowloc.suffix.prefix");
    nonconstantKLimit = appendInst(
        iPrep, Opcode::Select, Type::i32(), "rowloc.k.limit",
        {beforeSuffix, suffixStart, iPlusOne});
  }
  appendInst(iPrep, Opcode::Br, Type::voidTy(), "", {initHeader});

  auto* initJ = appendPhiStart(initHeader, Type::i32(), "rowloc.init.j", zero,
                               iPrep);
  auto* initCmp = appendICmp(initHeader, "lt", initJ, plan.bound,
                             "rowloc.init.j");
  appendInst(initHeader, Opcode::CondBr, Type::voidTy(), "",
             {initCmp, initBody, blockKHeader});
  Value* initAddr = buildVectorAddress(initBody, mod, rowAcc, initJ,
                                       "rowloc.init.acc");
  appendInst(initBody, Opcode::Store, Type::voidTy(), "", {zero, initAddr});
  auto* initJNext = appendInst(initBody, Opcode::Add, Type::i32(),
                               "rowloc.init.j.next", {initJ, one});
  appendInst(initBody, Opcode::Br, Type::voidTy(), "", {initHeader});
  addPhiIncoming(initJ, initJNext, initBody);

  auto* blockK = appendPhiStart(blockKHeader, Type::i32(), "rowloc.block.k",
                                zero, initHeader);
  auto* blockKNext = appendInst(blockKHeader, Opcode::Add, Type::i32(),
                                "rowloc.block.k.next",
                                {blockK, mod->getConstInt(kReductionBlock)});
  auto* hasFullBlock = appendICmp(blockKHeader, "le", blockKNext,
                                  nonconstantKLimit,
                                  "rowloc.block.k.full");
  appendInst(blockKHeader, Opcode::CondBr, Type::voidTy(), "",
             {hasFullBlock, blockKSetup, tailKHeader});

  std::vector<Value*> blockRows;
  std::vector<Value*> blockCoeffs;
  blockRows.reserve(kReductionBlock);
  blockCoeffs.reserve(kReductionBlock);
  for (int lane = 0; lane < kReductionBlock; ++lane) {
    Value* kValue = blockK;
    if (lane != 0) {
      kValue = appendInst(blockKSetup, Opcode::Add, Type::i32(),
                          "rowloc.block.k." + std::to_string(lane),
                          {blockK, mod->getConstInt(lane)});
    }
    blockRows.push_back(kValue);
    Value* coeffAddr = buildMatrixAddress(
        blockKSetup, mod, plan.lhs, iPhi, kValue, plan.rowBytes,
        "rowloc.block.coeff.addr." + std::to_string(lane));
    blockCoeffs.push_back(appendInst(
        blockKSetup, Opcode::Load, Type::i32(),
        "rowloc.block.coeff." + std::to_string(lane), {coeffAddr}));
  }
  appendInst(blockKSetup, Opcode::Br, Type::voidTy(), "", {blockJHeader});

  auto* blockJ = appendPhiStart(blockJHeader, Type::i32(), "rowloc.block.j",
                                zero, blockKSetup);
  auto* blockJCmp = appendICmp(blockJHeader, "lt", blockJ, plan.bound,
                               "rowloc.block.j");
  appendInst(blockJHeader, Opcode::CondBr, Type::voidTy(), "",
             {blockJCmp, blockJBody, blockKLatch});

  Value* blockAccAddr = buildVectorAddress(blockJBody, mod, rowAcc, blockJ,
                                           "rowloc.block.acc");
  Value* blockSum = appendInst(blockJBody, Opcode::Load, Type::i32(),
                               "rowloc.block.acc.val", {blockAccAddr});
  for (int lane = 0; lane < kReductionBlock; ++lane) {
    const std::string laneName = std::to_string(lane);
    Value* rhsAddr = buildMatrixAddress(
        blockJBody, mod, plan.dst, blockRows[lane], blockJ, plan.rowBytes,
        "rowloc.block.rhs." + laneName);
    auto* rhs = appendInst(blockJBody, Opcode::Load, Type::i32(),
                           "rowloc.block.rhs.val." + laneName, {rhsAddr});
    auto* product = appendInst(blockJBody, Opcode::Mul, Type::i32(),
                               "rowloc.block.prod." + laneName,
                               {blockCoeffs[lane], rhs});
    blockSum = appendInst(blockJBody, Opcode::Add, Type::i32(),
                          "rowloc.block.sum." + laneName,
                          {blockSum, product});
  }
  appendInst(blockJBody, Opcode::Store, Type::voidTy(), "",
             {blockSum, blockAccAddr});
  auto* blockJNext = appendInst(blockJBody, Opcode::Add, Type::i32(),
                                "rowloc.block.j.next", {blockJ, one});
  appendInst(blockJBody, Opcode::Br, Type::voidTy(), "", {blockJHeader});
  addPhiIncoming(blockJ, blockJNext, blockJBody);

  appendInst(blockKLatch, Opcode::Br, Type::voidTy(), "", {blockKHeader});
  addPhiIncoming(blockK, blockKNext, blockKLatch);

  auto* tailK = appendPhiStart(tailKHeader, Type::i32(), "rowloc.ktail.k",
                               blockK, blockKHeader);
  auto* tailKCmp = appendICmp(tailKHeader, "lt", tailK,
                              nonconstantKLimit, "rowloc.ktail.k");
  BasicBlock* afterTailK = applySuffix ? suffixHeader : storeHeader;
  appendInst(tailKHeader, Opcode::CondBr, Type::voidTy(), "",
             {tailKCmp, tailKSetup, afterTailK});

  Value* tailCoeffAddr = buildMatrixAddress(
      tailKSetup, mod, plan.lhs, iPhi, tailK, plan.rowBytes,
      "rowloc.ktail.coeff.addr");
  auto* tailCoeff = appendInst(tailKSetup, Opcode::Load, Type::i32(),
                               "rowloc.ktail.coeff", {tailCoeffAddr});
  appendInst(tailKSetup, Opcode::Br, Type::voidTy(), "", {tailJHeader});

  auto* tailJ = appendPhiStart(tailJHeader, Type::i32(), "rowloc.ktail.j",
                               zero, tailKSetup);
  auto* tailJCmp = appendICmp(tailJHeader, "lt", tailJ, plan.bound,
                              "rowloc.ktail.j");
  appendInst(tailJHeader, Opcode::CondBr, Type::voidTy(), "",
             {tailJCmp, tailJBody, tailKLatch});

  Value* tailAccAddr = buildVectorAddress(tailJBody, mod, rowAcc, tailJ,
                                          "rowloc.ktail.acc");
  auto* tailAcc = appendInst(tailJBody, Opcode::Load, Type::i32(),
                             "rowloc.ktail.acc.val", {tailAccAddr});
  Value* tailRhsAddr = buildMatrixAddress(tailJBody, mod, plan.dst, tailK,
                                          tailJ, plan.rowBytes,
                                          "rowloc.ktail.rhs");
  auto* tailRhs = appendInst(tailJBody, Opcode::Load, Type::i32(),
                             "rowloc.ktail.rhs.val", {tailRhsAddr});
  auto* tailProduct = appendInst(tailJBody, Opcode::Mul, Type::i32(),
                                 "rowloc.ktail.prod", {tailCoeff, tailRhs});
  auto* tailSum = appendInst(tailJBody, Opcode::Add, Type::i32(),
                             "rowloc.ktail.sum", {tailAcc, tailProduct});
  appendInst(tailJBody, Opcode::Store, Type::voidTy(), "",
             {tailSum, tailAccAddr});
  auto* tailJNext = appendInst(tailJBody, Opcode::Add, Type::i32(),
                               "rowloc.ktail.j.next", {tailJ, one});
  appendInst(tailJBody, Opcode::Br, Type::voidTy(), "", {tailJHeader});
  addPhiIncoming(tailJ, tailJNext, tailJBody);

  auto* tailKNext = appendInst(tailKLatch, Opcode::Add, Type::i32(),
                               "rowloc.ktail.k.next", {tailK, one});
  appendInst(tailKLatch, Opcode::Br, Type::voidTy(), "", {tailKHeader});
  addPhiIncoming(tailK, tailKNext, tailKLatch);

  BasicBlock* storeEntryPred = tailKHeader;
  if (applySuffix) {
    auto* suffixK = appendPhiStart(suffixHeader, Type::i32(),
                                   "rowloc.suffix.k", nonconstantKLimit,
                                   tailKHeader);
    auto* suffixSum = appendPhiStart(suffixHeader, Type::i32(),
                                     "rowloc.suffix.sum", zero, tailKHeader);
    auto* suffixCmp = appendICmp(suffixHeader, "lt", suffixK, plan.bound,
                                 "rowloc.suffix.k");
    appendInst(suffixHeader, Opcode::CondBr, Type::voidTy(), "",
               {suffixCmp, suffixBody, suffixDone});

    Value* suffixLhsAddr = buildMatrixAddress(
        suffixBody, mod, plan.lhs, iPhi, suffixK, plan.rowBytes,
        "rowloc.suffix.lhs");
    auto* suffixLhs = appendInst(suffixBody, Opcode::Load, Type::i32(),
                                 "rowloc.suffix.lhs.val", {suffixLhsAddr});
    auto* suffixSumNext = appendInst(suffixBody, Opcode::Add, Type::i32(),
                                     "rowloc.suffix.sum.next",
                                     {suffixSum, suffixLhs});
    auto* suffixKNext = appendInst(suffixBody, Opcode::Add, Type::i32(),
                                   "rowloc.suffix.k.next", {suffixK, one});
    appendInst(suffixBody, Opcode::Br, Type::voidTy(), "", {suffixHeader});
    addPhiIncoming(suffixK, suffixKNext, suffixBody);
    addPhiIncoming(suffixSum, suffixSumNext, suffixBody);

    Value* suffixContribution = suffixSum;
    if (plan.constantSuffixValue != 1) {
      suffixContribution = appendInst(
          suffixDone, Opcode::Mul, Type::i32(), "rowloc.suffix.contrib",
          {mod->getConstInt(plan.constantSuffixValue), suffixSum});
    }
    appendInst(suffixDone, Opcode::Br, Type::voidTy(), "",
               {suffixApplyHeader});

    auto* suffixJ = appendPhiStart(suffixApplyHeader, Type::i32(),
                                   "rowloc.suffix.j", zero, suffixDone);
    auto* suffixJCmp = appendICmp(suffixApplyHeader, "lt", suffixJ,
                                  plan.bound, "rowloc.suffix.j");
    appendInst(suffixApplyHeader, Opcode::CondBr, Type::voidTy(), "",
               {suffixJCmp, suffixApplyBody, storeHeader});
    Value* suffixAccAddr = buildVectorAddress(
        suffixApplyBody, mod, rowAcc, suffixJ, "rowloc.suffix.acc");
    auto* suffixAcc = appendInst(suffixApplyBody, Opcode::Load, Type::i32(),
                                 "rowloc.suffix.acc.val", {suffixAccAddr});
    auto* suffixAccNext = appendInst(
        suffixApplyBody, Opcode::Add, Type::i32(),
        "rowloc.suffix.acc.next", {suffixAcc, suffixContribution});
    appendInst(suffixApplyBody, Opcode::Store, Type::voidTy(), "",
               {suffixAccNext, suffixAccAddr});
    auto* suffixJNext = appendInst(suffixApplyBody, Opcode::Add, Type::i32(),
                                   "rowloc.suffix.j.next", {suffixJ, one});
    appendInst(suffixApplyBody, Opcode::Br, Type::voidTy(), "",
               {suffixApplyHeader});
    addPhiIncoming(suffixJ, suffixJNext, suffixApplyBody);
    storeEntryPred = suffixApplyHeader;
  }

  auto* storeJ = appendPhiStart(storeHeader, Type::i32(), "rowloc.storej",
                                zero, storeEntryPred);
  auto* storeCmp = appendICmp(storeHeader, "lt", storeJ, plan.bound,
                              "rowloc.storej");
  appendInst(storeHeader, Opcode::CondBr, Type::voidTy(), "",
             {storeCmp, storeBody, iLatch});
  Value* storeAccAddr = buildVectorAddress(storeBody, mod, rowAcc, storeJ,
                                           "rowloc.store.acc");
  auto* storeValue = appendInst(storeBody, Opcode::Load, Type::i32(),
                                "rowloc.store.val", {storeAccAddr});
  Value* storeDstAddr = buildMatrixAddress(storeBody, mod, plan.dst, iPhi,
                                           storeJ, plan.rowBytes,
                                           "rowloc.store.dst");
  appendInst(storeBody, Opcode::Store, Type::voidTy(), "",
             {storeValue, storeDstAddr});
  auto* storeJNext = appendInst(storeBody, Opcode::Add, Type::i32(),
                                "rowloc.storej.next", {storeJ, one});
  appendInst(storeBody, Opcode::Br, Type::voidTy(), "", {storeHeader});
  addPhiIncoming(storeJ, storeJNext, storeBody);

  auto* iNext = appendInst(iLatch, Opcode::Add, Type::i32(), "rowloc.i.next",
                           {iPhi, one});
  appendInst(iLatch, Opcode::Br, Type::voidTy(), "", {iHeader});
  addPhiIncoming(iPhi, iNext, iLatch);

  func->recomputeCFG();
  return true;
}

// 物化分派入口：默认沿归约维度分块，让一次行缓冲读写完成多次乘加。
// skip/disable 开关保留未分块实现，便于定向比较两种通用物化策略。
bool materializeInPlaceRowLocality(const InPlaceRowLocalityPlan& plan) {
  if (sycEnvironmentValue("SYC_SKIP_COREIR_ROW_LOCALITY_K_BLOCKING") ||
      sycEnvironmentValue("ROW_LOCALITY_DISABLE_K_BLOCKING")) {
    return materializeInPlaceRowLocalityUntiled(plan);
  }
  return materializeInPlaceRowLocalityKBlocked(plan);
}

// 函数级入口：指令数预算检查后，按块数从大到小（外层优先）逐个循环尝试匹配，
// 首个收益达标的形态即物化并立即返回，每个函数至多改写一处。
bool applyInPlaceRowLocality(Function* func,
                             analysis::AnalysisManager& analysisManager) {
  if (!func || !func->entryBlock())
    return false;
  analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
  if (cache.instructionCount() > 100000) {
    if (sycEnvironmentValue("ROW_LOCALITY_TRACE"))
      std::cerr << "[row-locality] reject func=" << func->name()
                << " reason=instruction-budget count="
                << cache.instructionCount() << "\n";
    return false;
  }
  auto loops = cache.loops();
  const DomSet& doms = cache.dominators();
  std::sort(loops.begin(), loops.end(),
            [](const analysis::NaturalLoop& lhs,
               const analysis::NaturalLoop& rhs) {
              if (lhs.blocks.size() != rhs.blocks.size())
                return lhs.blocks.size() > rhs.blocks.size();
              return lhs.header && rhs.header
                         ? lhs.header->id() < rhs.header->id()
                         : lhs.header < rhs.header;
            });
  const bool trace = sycEnvironmentValue("ROW_LOCALITY_TRACE") != nullptr;
  for (const auto& loop : loops) {
    InPlaceRowLocalityPlan plan;
    if (!matchInPlaceRowLocalityLoop(func, loop, loops, doms, plan))
      continue;
    if (!isProfitableInPlaceRowLocality(plan)) {
      if (trace) {
        std::cerr << "[row-locality] reject header="
                  << (loop.header ? loop.header->name() : std::string("<null>"))
                  << " reason=profit rowElems=" << plan.rowElems << "\n";
      }
      continue;
    }
    if (materializeInPlaceRowLocality(plan)) {
      if (trace) {
        std::cerr << "[row-locality] func=" << func->name()
                  << " header=" << (loop.header ? loop.header->name() : "-")
                  << " dst=" << plan.dst->name()
                  << " lhs=" << plan.lhs->name()
                  << " rowBytes=" << plan.rowBytes << "\n";
      }
      return true;
    }
  }
  return false;
}

bool applyInPlaceRowLocality(Module& mod,
                             analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func)
      continue;
    if (applyInPlaceRowLocality(func, analysisManager)) {
      changed = true;
      analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
    }
  }
  return changed;
}


} // namespace core_ir
