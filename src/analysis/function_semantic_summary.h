// 文件职责：把 effect、终止、trap 和 pointer escape 汇总成候选证明可消费的函数合同。
#pragma once

#include "function_effects.h"
#include "proof_result.h"
#include "termination.h"

#include <unordered_map>
#include <unordered_set>

namespace analysis {

struct FunctionSemanticSummary {
  ProofResult proof;
  bool pure = false;
  bool readonly = false;
  bool total = false;
  bool deterministic = false;
  bool nonTrapping = false;
  bool hasObservableCall = false;
  bool pointerArgumentEscapes = false;
  bool returnsPointer = false;
  std::unordered_set<std::size_t> readArgumentIndices;
  std::unordered_set<std::size_t> writtenArgumentIndices;
  std::unordered_set<std::size_t> capturedArgumentIndices;
  std::unordered_set<std::size_t> returnedArgumentIndices;
  std::unordered_set<const core_ir::GlobalVar*> writtenGlobals;
  int memoryReads = 0;
  int memoryWrites = 0;
  int calls = 0;
};

using FunctionSemanticSummaryMap =
    std::unordered_map<const core_ir::Function*, FunctionSemanticSummary>;

FunctionSemanticSummary summarizeFunctionSemantics(
    core_ir::Function* function, const FunctionEffectMap& effects,
    const FunctionTerminationMap& termination,
    const FunctionSemanticSummaryMap* known = nullptr);
FunctionSemanticSummaryMap summarizeModuleFunctionSemantics(
    core_ir::Module& module, const FunctionEffectMap& effects,
    const FunctionTerminationMap& termination);

} // namespace analysis
