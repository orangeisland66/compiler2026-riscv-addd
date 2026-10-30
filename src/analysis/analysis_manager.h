// 文件职责：统一管理模块级和函数级分析缓存，为 O3 级流水线提供分析复用入口。本文件负责声明接口和数据结构。
#pragma once

#include "../core_ir/ir/core_ir.h"
#include "function_analysis.h"
#include "function_effects.h"
#include "function_semantic_summary.h"
#include "integer_facts.h"
#include "memory_ssa_lite.h"

#include <memory>
#include <cstdint>
#include <iosfwd>
#include <unordered_map>

namespace analysis {

enum class AnalysisInvalidation {
  None,
  Values,
  Memory,
  CFG,
  Calls,
  ModuleGlobals,
  // 兼容现有 pass 声明；新代码优先使用 Calls/ModuleGlobals。
  Effects,
  Module,
};

class AnalysisManager {
public:
  explicit AnalysisManager(core_ir::Module& mod);

  core_ir::Module& module() { return mod_; }
  std::uint64_t moduleEpoch() const { return moduleEpoch_; }
  std::uint64_t functionEpoch(core_ir::Function* func) const;

  // 函数级缓存入口。各 pass 可以复用支配、循环、归纳变量等分析，
  // 并在 CFG 改动后通过 invalidateFunction 失效。
  FunctionAnalysisCache& function(core_ir::Function* func);
  // 模块级 effect 摘要。当前用于纯调用判断、只读 helper 复用、
  // 递归摘要派生等场景。
  const FunctionEffectMap& functionEffects();
  // 统一递归摘要入口。它基于函数 effect 结果派生，因此失效逻辑
  // 与 functionEffects 绑定。
  const RecursiveFunctionMap& recursiveFunctions();
  const MemorySSALite& memorySSA(core_ir::Function* func);
  const FunctionSemanticSummary& functionSemantics(
      core_ir::Function* func);
  const IntegerFacts& integerFacts(core_ir::Value* value);
  // 确定性、只读的分析快照；用于 proof 调试和回归，不参与优化决策。
  void dump(std::ostream& output);

  void invalidateFunction(core_ir::Function* func, AnalysisInvalidation kind);
  void invalidateModule(AnalysisInvalidation kind = AnalysisInvalidation::Module);

private:
  core_ir::Module& mod_;
  bool effectsReady_ = false;
  bool recursiveReady_ = false;
  FunctionEffectMap effects_;
  RecursiveFunctionMap recursive_;
  bool semanticsReady_ = false;
  FunctionTerminationMap termination_;
  FunctionSemanticSummaryMap semantics_;
  std::unordered_map<core_ir::Function*, std::unique_ptr<FunctionAnalysisCache>>
      functions_;
  std::unordered_map<core_ir::Function*, MemorySSALite> memorySSA_;
  std::unordered_map<core_ir::Value*, IntegerFacts> integerFacts_;
  std::uint64_t moduleEpoch_ = 0;
  std::unordered_map<core_ir::Function*, std::uint64_t> functionEpochs_;
};

} // namespace analysis
