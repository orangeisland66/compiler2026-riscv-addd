// 文件职责：收集函数级结构信息，辅助内联、递归和副作用判断。本文件负责实现具体逻辑。
#include "function_analysis.h"
#include "analysis_manager.h"
#include "induction_var.h"
#include "cfg_utils.h"
#include "../hir/canonical_loop.h"

#include <algorithm>

namespace analysis {

namespace {

using core_ir::BasicBlock;
using core_ir::ConstInt;
using core_ir::Function;
using core_ir::GlobalVar;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Value;

Value* runtimeBoundMemoryRoot(Value* addr) {
  Value* cur = addr;
  while (auto* inst = dynamic_cast<Instruction*>(cur)) {
    if ((inst->opcode() == Opcode::GEP || inst->opcode() == Opcode::BitCast) &&
        inst->numOperands() >= 1) {
      cur = inst->operand(0);
      continue;
    }
    break;
  }
  return cur;
}

bool isScalarI32Global(const Value* value) {
  auto* gv = dynamic_cast<const GlobalVar*>(value);
  return gv && gv->sizeBytes() == 4 && gv->dims().empty() &&
         gv->elemTy().isI32();
}

bool runtimeBoundAllowedOpcode(Opcode op) {
  switch (op) {
  case Opcode::Phi:
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Mul:
  case Opcode::SDiv:
  case Opcode::SRem:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
  case Opcode::Shl:
  case Opcode::Shr:
  case Opcode::ICmp:
  case Opcode::Mov:
  case Opcode::Neg:
  case Opcode::BitCast:
    return true;
  default:
    return false;
  }
}

bool containsScalarGlobalLoadLoose(Value* value,
                                   std::unordered_set<int>& visiting,
                                   int depth = 0) {
  if (!value || depth > 12)
    return false;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst)
    return false;
  if (!visiting.insert(inst->id()).second)
    return false;
  if ((inst->opcode() == Opcode::Load || inst->opcode() == Opcode::VLoad) &&
      inst->numOperands() == 1 &&
      isScalarI32Global(runtimeBoundMemoryRoot(inst->operand(0)))) {
    visiting.erase(inst->id());
    return true;
  }
  for (Value* operand : inst->operands()) {
    if (containsScalarGlobalLoadLoose(operand, visiting, depth + 1)) {
      visiting.erase(inst->id());
      return true;
    }
  }
  visiting.erase(inst->id());
  return false;
}

struct RuntimeScalarGlobalTrace {
  bool depends = false;
  bool unsupported = false;
  bool hasInLoopLoad = false;
  std::vector<Instruction*> directLoads;
  std::vector<GlobalVar*> globals;
  std::unordered_set<int> directLoadIds;
  std::unordered_set<int> globalIds;
};

void recordRuntimeScalarGlobalLoad(RuntimeScalarGlobalTrace& trace,
                                   Instruction* load,
                                   GlobalVar* gv,
                                   const NaturalLoop& loop) {
  trace.depends = true;
  if (load && trace.directLoadIds.insert(load->id()).second)
    trace.directLoads.push_back(load);
  if (gv && trace.globalIds.insert(gv->id()).second)
    trace.globals.push_back(gv);
  if (load && load->parent() && loop.blocks.count(load->parent()))
    trace.hasInLoopLoad = true;
}

bool traceRuntimeScalarGlobalBound(Value* value, const NaturalLoop& loop,
                                   RuntimeScalarGlobalTrace& trace,
                                   std::unordered_set<int>& visiting,
                                   int depth = 0) {
  if (!value || depth > 12)
    return false;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst)
    return false;
  if (!visiting.insert(inst->id()).second)
    return false;

  if ((inst->opcode() == Opcode::Load || inst->opcode() == Opcode::VLoad) &&
      inst->numOperands() == 1) {
    auto* gv = dynamic_cast<GlobalVar*>(
        runtimeBoundMemoryRoot(inst->operand(0)));
    if (isScalarI32Global(gv)) {
      if (inst->opcode() == Opcode::Load && inst->type().isI32() &&
          inst->operand(0) == gv) {
        recordRuntimeScalarGlobalLoad(trace, inst, gv, loop);
      } else {
        trace.depends = true;
        trace.unsupported = true;
        if (inst->parent() && loop.blocks.count(inst->parent()))
          trace.hasInLoopLoad = true;
        if (gv && trace.globalIds.insert(gv->id()).second)
          trace.globals.push_back(gv);
      }
      visiting.erase(inst->id());
      return true;
    }
    visiting.erase(inst->id());
    return false;
  }

  if (!runtimeBoundAllowedOpcode(inst->opcode())) {
    std::unordered_set<int> looseVisiting;
    if (containsScalarGlobalLoadLoose(inst, looseVisiting)) {
      trace.depends = true;
      trace.unsupported = true;
      visiting.erase(inst->id());
      return true;
    }
    visiting.erase(inst->id());
    return false;
  }

  bool found = false;
  for (Value* operand : inst->operands()) {
    if (traceRuntimeScalarGlobalBound(operand, loop, trace, visiting,
                                      depth + 1)) {
      found = true;
    }
  }
  visiting.erase(inst->id());
  return found;
}

bool valueDependsOnDirectRuntimeScalarLoads(
    Value* value, const std::unordered_set<int>& directLoadIds,
    std::unordered_map<int, bool>& memo) {
  if (!value)
    return false;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst)
    return false;
  if (directLoadIds.count(inst->id()))
    return true;
  auto it = memo.find(inst->id());
  if (it != memo.end())
    return it->second;
  memo[inst->id()] = false;
  for (Value* operand : inst->operands()) {
    if (valueDependsOnDirectRuntimeScalarLoads(operand, directLoadIds, memo)) {
      memo[inst->id()] = true;
      break;
    }
  }
  return memo[inst->id()];
}

bool canCloneRuntimeScalarBoundValue(
    Value* value, const NaturalLoop& loop,
    const std::unordered_set<int>& directLoadIds,
    std::unordered_map<int, bool>& memo) {
  if (!value)
    return false;
  if (value->isConstant() || dynamic_cast<core_ir::Argument*>(value) ||
      dynamic_cast<core_ir::GlobalVar*>(value) ||
      dynamic_cast<BasicBlock*>(value)) {
    return true;
  }

  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst)
    return false;
  if (directLoadIds.count(inst->id()))
    return true;

  auto it = memo.find(inst->id());
  if (it != memo.end())
    return it->second;

  if (!inst->parent() || !loop.blocks.count(inst->parent())) {
    memo[inst->id()] = true;
    return true;
  }
  if (inst->opcode() == Opcode::Phi ||
      !runtimeBoundAllowedOpcode(inst->opcode())) {
    memo[inst->id()] = false;
    return false;
  }

  memo[inst->id()] = false;
  for (Value* operand : inst->operands()) {
    if (!canCloneRuntimeScalarBoundValue(operand, loop, directLoadIds, memo))
      return false;
  }
  memo[inst->id()] = true;
  return true;
}

bool runtimeCallMayClobberTrackedMemory(const Instruction* inst,
                                        const Module& mod,
                                        const FunctionEffectMap* effects) {
  if (!inst || inst->opcode() != Opcode::Call)
    return false;
  if (effects && analysis::isReadonlyCall(inst, mod, *effects))
    return false;
  if (mod.isBuiltin(inst->name())) {
    for (int i = 0; i < inst->numOperands(); ++i) {
      Value* operand = inst->operand(i);
      if (operand && operand->type().isPtr())
        return true;
    }
    return false;
  }
  return inst->name() != "_sysy_starttime" &&
         inst->name() != "_sysy_stoptime";
}

const Instruction* loopHeaderCompare(const NaturalLoop& loop) {
  if (!loop.header)
    return nullptr;
  Instruction* term = loop.header->terminator();
  if (!term || term->opcode() != Opcode::CondBr || term->numOperands() < 1)
    return nullptr;
  auto* cmp = dynamic_cast<Instruction*>(term->operand(0));
  if (!cmp || cmp->opcode() != Opcode::ICmp)
    return nullptr;
  const std::string cond = hir::canonicalLoopCmpCond(cmp);
  if (cond == "eq" || cond == "ne")
    return nullptr;
  return cmp;
}

} // namespace

FunctionAnalysisCache::FunctionAnalysisCache(core_ir::Function* func,
                                             AnalysisManager* manager)
    : func_(func), manager_(manager) {}

void FunctionAnalysisCache::invalidateCFG() {
  blocksReady_ = false;
  domsReady_ = false;
  loopsReady_ = false;
  loopForestReady_ = false;
  affineGuardsReady_ = false;
  loopBandsReady_ = false;
  affineRegionsReady_ = false;
  instructionCountReady_ = false;
  blocks_.clear();
  reachable_.clear();
  doms_.clear();
  loops_.clear();
  loopForest_ = LoopForest();
  inductionVarsByHeader_.clear();
  loopTripsByHeader_.clear();
  affineRecurrencesByHeader_.clear();
  unsafeRuntimeScalarBoundByHeader_.clear();
  emptyInductionVars_.clear();
  emptyAffineRecurrences_.clear();
  affineGuards_.clear();
  loopBands_.clear();
  affineRegions_.clear();
  instructionCount_ = 0;
}

const std::vector<core_ir::BasicBlock*>& FunctionAnalysisCache::blocks() {
  ensureBlocksAndReachable();
  return blocks_;
}

const std::unordered_set<core_ir::BasicBlock*>&
FunctionAnalysisCache::reachable() {
  ensureBlocksAndReachable();
  return reachable_;
}

const DomSet& FunctionAnalysisCache::dominators() {
  ensureDominators();
  return doms_;
}

const std::vector<NaturalLoop>& FunctionAnalysisCache::loops() {
  ensureLoops();
  return loops_;
}

const LoopForest& FunctionAnalysisCache::loopForest() {
  ensureLoopForest();
  return loopForest_;
}

const std::vector<InductionVar>&
FunctionAnalysisCache::inductionVars(const NaturalLoop& loop) {
  if (!func_ || !loop.header)
    return emptyInductionVars_;
  const int headerId = loop.header->id();
  auto it = inductionVarsByHeader_.find(headerId);
  if (it != inductionVarsByHeader_.end())
    return it->second;
  // 归纳变量是 loop-memory 多个 pass 的共同前置事实。
  // 以 header id 为键缓存，CFG 失效时统一清空，避免同一轮流水线反复扫描函数。
  auto inserted = inductionVarsByHeader_.emplace(
      headerId, findInductionVars(func_, loop));
  return inserted.first->second;
}

const LoopTripRecipe& FunctionAnalysisCache::loopTrip(
    const NaturalLoop& loop) {
  if (!func_ || !loop.header) return emptyLoopTrip_;
  const int headerId = loop.header->id();
  auto found = loopTripsByHeader_.find(headerId);
  if (found != loopTripsByHeader_.end()) return found->second;
  return loopTripsByHeader_
      .emplace(headerId, analyzeLoopTrip(func_, loop))
      .first->second;
}

const std::vector<AffineRecurrencePlan>&
FunctionAnalysisCache::affineRecurrences(const NaturalLoop& loop) {
  if (!func_ || !loop.header)
    return emptyAffineRecurrences_;
  const int headerId = loop.header->id();
  auto it = affineRecurrencesByHeader_.find(headerId);
  if (it != affineRecurrencesByHeader_.end())
    return it->second;
  const auto& ivs = inductionVars(loop);
  if (ivs.empty())
    return emptyAffineRecurrences_;
  // affine recurrence 是 scalar rewrite / pointer bump 的共享
  // 前置摘要。缓存它可以让 loop-memory 流水线复用同一组地址族判断；
  // 任何 CFG 或访存改写都会通过 invalidateCFG 清空这里的结果。
  auto inserted = affineRecurrencesByHeader_.emplace(
      headerId, collectAffineRecurrencePlans(func_, loop, ivs));
  return inserted.first->second;
}

const std::vector<AffineGuardSummary>& FunctionAnalysisCache::affineGuards() {
  ensureAffineGuards();
  return affineGuards_;
}

const std::vector<LoopBandPlan>& FunctionAnalysisCache::loopBands() {
  ensureLoopBands();
  return loopBands_;
}

const std::vector<AffineRegionPlan>& FunctionAnalysisCache::affineRegions() {
  ensureAffineRegions();
  return affineRegions_;
}

bool loopTouchesAggregateMemory(const NaturalLoop& loop) {
  for (auto* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      Value* addr = nullptr;
      if ((inst->opcode() == Opcode::Load || inst->opcode() == Opcode::VLoad) &&
          inst->numOperands() == 1) {
        addr = inst->operand(0);
      } else if ((inst->opcode() == Opcode::Store ||
                  inst->opcode() == Opcode::VStore) &&
                 inst->numOperands() == 2) {
        addr = inst->operand(1);
      }
      Value* root = runtimeBoundMemoryRoot(addr);
      if (dynamic_cast<core_ir::Argument*>(root))
        return true;
      auto* gv = dynamic_cast<GlobalVar*>(root);
      if (gv && (gv->sizeBytes() > 4 || !gv->dims().empty()))
        return true;
      auto* rootInst = dynamic_cast<Instruction*>(root);
      if (rootInst && rootInst->opcode() == Opcode::Alloca &&
          (rootInst->isArrayObject() || rootInst->memorySizeBytes() > 4)) {
        return true;
      }
    }
  }
  return false;
}

std::vector<RuntimeScalarGlobalBoundInfo> collectRuntimeScalarGlobalBounds(
    Function* func, const NaturalLoop& loop, const std::vector<InductionVar>& ivs,
    const FunctionEffectMap* effects) {
  std::vector<RuntimeScalarGlobalBoundInfo> infos;
  if (!func || !func->parent() || !loop.header || !loopTouchesAggregateMemory(loop))
    return infos;

  const Module& mod = *func->parent();
  std::vector<Instruction*> compares;
  if (const auto* headerCmp = loopHeaderCompare(loop))
    compares.push_back(const_cast<Instruction*>(headerCmp));
  auto guards = collectAffineGuards(func, loop, ivs);
  for (const auto& guard : guards) {
    if (!guard.valid || !guard.compare)
      continue;
    if (std::find(compares.begin(), compares.end(), guard.compare) ==
        compares.end()) {
      compares.push_back(guard.compare);
    }
  }
  if (compares.empty())
    return infos;

  std::unordered_set<int> directStoreGlobalIds;
  std::unordered_set<int> indirectGlobalAccessIds;
  bool unknownCall = false;
  for (auto* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      if (inst->opcode() == Opcode::Call &&
          runtimeCallMayClobberTrackedMemory(inst, mod, effects)) {
        unknownCall = true;
      }
      Value* addr = nullptr;
      if ((inst->opcode() == Opcode::Load || inst->opcode() == Opcode::VLoad) &&
          inst->numOperands() == 1) {
        addr = inst->operand(0);
      } else if ((inst->opcode() == Opcode::Store ||
                  inst->opcode() == Opcode::VStore) &&
                 inst->numOperands() == 2) {
        addr = inst->operand(1);
      }
      auto* gv = dynamic_cast<GlobalVar*>(runtimeBoundMemoryRoot(addr));
      if (!isScalarI32Global(gv))
        continue;
      if (addr != gv)
        indirectGlobalAccessIds.insert(gv->id());
      if (inst->opcode() == Opcode::Store || inst->opcode() == Opcode::VStore)
        directStoreGlobalIds.insert(gv->id());
    }
  }

  for (Instruction* compare : compares) {
    RuntimeScalarGlobalTrace trace;
    std::unordered_set<int> visiting;
    for (Value* operand : compare->operands()) {
      traceRuntimeScalarGlobalBound(operand, loop, trace, visiting);
    }
    if (!trace.depends)
      continue;

    RuntimeScalarGlobalBoundInfo info;
    info.compare = compare;
    info.directLoads = trace.directLoads;
    info.globals = trace.globals;
    info.hasInLoopLoad = trace.hasInLoopLoad;

    bool mayClobber = unknownCall;
    bool indirectAlias = false;
    for (auto* gv : trace.globals) {
      if (!gv)
        continue;
      mayClobber = mayClobber || directStoreGlobalIds.count(gv->id());
      indirectAlias = indirectAlias || indirectGlobalAccessIds.count(gv->id());
    }

    bool localizable = trace.hasInLoopLoad && !mayClobber && !indirectAlias &&
                       !trace.unsupported;
    if (localizable) {
      std::unordered_map<int, bool> depMemo;
      std::unordered_map<int, bool> cloneMemo;
      for (Value* operand : compare->operands()) {
        if (!valueDependsOnDirectRuntimeScalarLoads(
                operand, trace.directLoadIds, depMemo)) {
          continue;
        }
        if (!canCloneRuntimeScalarBoundValue(
                operand, loop, trace.directLoadIds, cloneMemo)) {
          localizable = false;
          break;
        }
      }
    }

    const bool alreadySafe = !trace.hasInLoopLoad && !mayClobber;
    info.localizable = localizable;
    info.unsafe = !(alreadySafe || localizable);
    infos.push_back(std::move(info));
  }
  return infos;
}

bool loopHasUnsafeRuntimeScalarGlobalBound(Function* func, const NaturalLoop& loop,
                                           const std::vector<InductionVar>& ivs,
                                           const FunctionEffectMap* effects) {
  auto infos = collectRuntimeScalarGlobalBounds(func, loop, ivs, effects);
  for (const auto& info : infos) {
    if (info.unsafe)
      return true;
  }
  return false;
}

bool FunctionAnalysisCache::hasUnsafeRuntimeScalarGlobalBound(
    const NaturalLoop& loop) {
  if (!func_ || !loop.header)
    return false;
  const int headerId = loop.header->id();
  auto it = unsafeRuntimeScalarBoundByHeader_.find(headerId);
  if (it != unsafeRuntimeScalarBoundByHeader_.end())
    return it->second;
  const FunctionEffectMap* effects = nullptr;
  if (manager_)
    effects = &manager_->functionEffects();
  bool unsafe = loopHasUnsafeRuntimeScalarGlobalBound(
      func_, loop, inductionVars(loop), effects);
  unsafeRuntimeScalarBoundByHeader_[headerId] = unsafe;
  return unsafe;
}

int FunctionAnalysisCache::instructionCount() {
  if (!instructionCountReady_) {
    ensureBlocksAndReachable();
    instructionCount_ = 0;
    for (auto* bb : blocks_) {
      instructionCount_ += static_cast<int>(bb->instructions().size());
    }
    instructionCountReady_ = true;
  }
  return instructionCount_;
}

void FunctionAnalysisCache::ensureBlocksAndReachable() {
  if (blocksReady_) {
    return;
  }
  blocks_.clear();
  if (func_) {
    for (auto& bb : func_->blocks()) {
      blocks_.push_back(bb.get());
    }
    reachable_ = collectReachableBlocks(func_);
  } else {
    reachable_.clear();
  }
  blocksReady_ = true;
}

void FunctionAnalysisCache::ensureDominators() {
  if (domsReady_) {
    return;
  }
  ensureBlocksAndReachable();
  doms_ = computeDominators(func_, blocks_, reachable_);
  domsReady_ = true;
}

void FunctionAnalysisCache::ensureLoops() {
  if (loopsReady_) {
    return;
  }
  ensureDominators();
  loops_ = collectNaturalLoops(blocks_, doms_, reachable_);
  loopsReady_ = true;
}

void FunctionAnalysisCache::ensureLoopForest() {
  if (loopForestReady_) {
    return;
  }
  ensureLoops();
  loopForest_ = buildLoopForest(loops_);
  loopForestReady_ = true;
}

void FunctionAnalysisCache::ensureAffineGuards() {
  if (affineGuardsReady_) {
    return;
  }
  ensureLoops();
  affineGuards_.clear();
  if (func_) {
    for (const auto& loop : loops_) {
      if (hasUnsafeRuntimeScalarGlobalBound(loop))
        continue;
      const auto& ivs = inductionVars(loop);
      auto guards = collectAffineGuards(func_, loop, ivs);
      affineGuards_.insert(affineGuards_.end(), guards.begin(), guards.end());
    }
  }
  affineGuardsReady_ = true;
}

void FunctionAnalysisCache::ensureLoopBands() {
  if (loopBandsReady_) {
    return;
  }
  ensureLoopForest();
  loopBands_ = buildLoopBandPlans(func_, *this);
  loopBandsReady_ = true;
}

void FunctionAnalysisCache::ensureAffineRegions() {
  if (affineRegionsReady_) {
    return;
  }
  ensureAffineGuards();
  affineRegions_ = buildAffineRegionPlans(*this);
  affineRegionsReady_ = true;
}

} // namespace analysis

#define ANALYSIS_FUNCTION_EFFECTS_EMBED
#include "function_effects.cpp"
