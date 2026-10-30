// 文件职责：统一管理模块级和函数级分析缓存，为 O3 级流水线提供分析复用入口。本文件负责实现具体逻辑。
#include "analysis_manager.h"

#include "loop_access.h"
#include "loop_dependence.h"

#include <ostream>

namespace analysis {

namespace {

void clearEffectCaches(bool& effectsReady,
                       bool& recursiveReady,
                       FunctionEffectMap& effects,
                       RecursiveFunctionMap& recursive) {
  effectsReady = false;
  recursiveReady = false;
  effects.clear();
  recursive.clear();
}

} // namespace

AnalysisManager::AnalysisManager(core_ir::Module& mod) : mod_(mod) {}

std::uint64_t AnalysisManager::functionEpoch(core_ir::Function* func) const {
  auto found = functionEpochs_.find(func);
  return found == functionEpochs_.end() ? 0 : found->second;
}

FunctionAnalysisCache& AnalysisManager::function(core_ir::Function* func) {
  auto& slot = functions_[func];
  if (!slot)
    slot = std::make_unique<FunctionAnalysisCache>(func, this);
  return *slot;
}

const FunctionEffectMap& AnalysisManager::functionEffects() {
  if (!effectsReady_) {
    effects_ = summarizeModuleFunctionEffects(mod_);
    effectsReady_ = true;
    recursiveReady_ = false;
    recursive_.clear();
  }
  return effects_;
}

const RecursiveFunctionMap& AnalysisManager::recursiveFunctions() {
  if (!recursiveReady_) {
    recursive_ = summarizeModuleRecursiveFunctions(mod_, functionEffects());
    recursiveReady_ = true;
  }
  return recursive_;
}

const MemorySSALite& AnalysisManager::memorySSA(core_ir::Function* func) {
  auto found = memorySSA_.find(func);
  if (found == memorySSA_.end()) {
    const auto& effects = functionEffects();
    found = memorySSA_.emplace(func, buildMemorySSALite(func, &effects)).first;
  }
  return found->second;
}

const FunctionSemanticSummary& AnalysisManager::functionSemantics(
    core_ir::Function* func) {
  if (!semanticsReady_) {
    termination_ = proveModuleFunctionTermination(mod_, *this);
    const auto& effects = functionEffects();
    semantics_ = summarizeModuleFunctionSemantics(mod_, effects,
                                                  termination_);
    semanticsReady_ = true;
  }
  auto found = semantics_.find(func);
  if (found == semantics_.end()) {
    static const FunctionSemanticSummary missing = [] {
      FunctionSemanticSummary summary;
      summary.proof =
          ProofResult::unknown("function-semantic-summary-missing");
      return summary;
    }();
    return missing;
  }
  return found->second;
}

const IntegerFacts& AnalysisManager::integerFacts(core_ir::Value* value) {
  auto found = integerFacts_.find(value);
  if (found == integerFacts_.end())
    found = integerFacts_.emplace(value, evaluateIntegerFacts(value)).first;
  return found->second;
}

void AnalysisManager::dump(std::ostream& output) {
  const auto& effects = functionEffects();
  output << "analysis module-epoch=" << moduleEpoch_ << " functions="
         << mod_.functions().size() << "\n";
  for (const auto& ownedFunction : mod_.functions()) {
    auto* func = ownedFunction.get();
    auto& cache = function(func);
    const auto* effect = lookupFunctionEffect(effects, func);
    const auto& memory = memorySSA(func);
    const auto& semantic = functionSemantics(func);
    output << "function id=" << func->name() << " epoch="
           << functionEpoch(func) << " blocks=" << cache.blocks().size()
           << " instructions=" << cache.instructionCount()
           << " loops=" << cache.loops().size()
           << " memory-nodes=" << memory.nodes.size();
    if (effect)
      output << " pure=" << effect->pure << " readonly=" << effect->readonly
             << " reads-unknown=" << effect->readsUnknownMemory
             << " writes-unknown=" << effect->writesUnknownMemory;
    output << " memory-proof=" << memory.proof.reasonCode
           << " semantic-proof=" << semantic.proof.reasonCode
           << " total=" << semantic.total
           << " deterministic=" << semantic.deterministic << "\n";
    for (std::size_t loopIndex = 0; loopIndex < cache.loops().size();
         ++loopIndex) {
      const auto& loop = cache.loops()[loopIndex];
      const auto& ivs = cache.inductionVars(loop);
      const auto& trip = cache.loopTrip(loop);
      auto dependence = analyzeLoopDependences(loop, ivs);
      output << " loop index=" << loopIndex
             << " blocks=" << loop.blocks.size()
             << " latches=" << loop.latches.size()
             << " ivs=" << ivs.size()
             << " accesses=" << dependence.accesses.size()
             << " dependences=" << dependence.dependences.size()
             << " proof=" << dependence.completeness.reasonCode
             << " trip-proof=" << trip.proof.reasonCode
             << " trip=" << trip.constantTripCount << "\n";
    }
  }
}

void AnalysisManager::invalidateFunction(core_ir::Function* func,
                                         AnalysisInvalidation kind) {
  if (kind == AnalysisInvalidation::None)
    return;
  semanticsReady_ = false;
  semantics_.clear();
  termination_.clear();
  ++functionEpochs_[func];
  // pass 可能仍持有 FunctionAnalysisCache 引用；函数级失效只能清空缓存内容，
  // 不能在 pass 返回前销毁缓存对象。
  auto cachedFunction = functions_.find(func);
  if (cachedFunction != functions_.end() && cachedFunction->second)
    cachedFunction->second->invalidateCFG();
  if (kind != AnalysisInvalidation::Values)
    memorySSA_.erase(func);
  integerFacts_.clear();
  if (kind == AnalysisInvalidation::Module ||
      kind == AnalysisInvalidation::ModuleGlobals) {
    clearEffectCaches(effectsReady_, recursiveReady_, effects_, recursive_);
    ++moduleEpoch_;
    return;
  }
  if (kind == AnalysisInvalidation::Memory ||
      kind == AnalysisInvalidation::Calls ||
      kind == AnalysisInvalidation::Effects) {
    clearEffectCaches(effectsReady_, recursiveReady_, effects_, recursive_);
    ++moduleEpoch_;
    if (kind == AnalysisInvalidation::Memory ||
        kind == AnalysisInvalidation::Calls ||
        kind == AnalysisInvalidation::Effects)
      memorySSA_.clear();
  }
}

void AnalysisManager::invalidateModule(AnalysisInvalidation kind) {
  if (kind == AnalysisInvalidation::None)
    return;
  semanticsReady_ = false;
  semantics_.clear();
  termination_.clear();
  ++moduleEpoch_;
  if (kind == AnalysisInvalidation::Module ||
      kind == AnalysisInvalidation::ModuleGlobals) {
    for (auto& entry : functions_)
      ++functionEpochs_[entry.first];
    functions_.clear();
    memorySSA_.clear();
    integerFacts_.clear();
    clearEffectCaches(effectsReady_, recursiveReady_, effects_, recursive_);
    return;
  }
  for (auto& entry : functions_) {
    ++functionEpochs_[entry.first];
    if (entry.second) entry.second->invalidateCFG();
  }
  if (kind != AnalysisInvalidation::Values)
    memorySSA_.clear();
  integerFacts_.clear();
  if (kind == AnalysisInvalidation::Memory ||
      kind == AnalysisInvalidation::Calls ||
      kind == AnalysisInvalidation::Effects) {
    clearEffectCaches(effectsReady_, recursiveReady_, effects_, recursive_);
  }
}

} // namespace analysis
