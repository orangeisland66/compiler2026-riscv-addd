// 文件职责：声明公共子表达式消除及相关表达式分析接口。
#pragma once
#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"
#include "../../analysis/dominators.h"
#include "../../analysis/function_effects.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace core_ir {

using analysis::DomSet;

struct LinearOffsetKey {
  bool valid = false;
  Value* root = nullptr;
  int64_t constant = 0;
  std::vector<std::pair<std::string, int64_t>> terms;
};

enum class MemoryRootKind {
  Unknown,
  Global,
  Argument,
  Alloca,
};

struct MemoryAliasInfo {
  bool valid = false;
  bool precise = false;
  bool invariant = false;
  MemoryRootKind rootKind = MemoryRootKind::Unknown;
  Value* root = nullptr;
  std::string preciseKey;
};

struct MemoryVersionState {
  // 排序后的到达定义标记。使用集合而不是路径局部计数器，
  // 使 CFG 合流和循环回边之后的版本仍可比较。
  std::vector<int> globalBarriers;
  std::vector<int> allStoreVersions;
  std::vector<int> externalStoreVersions;
  std::vector<int> externalBarriers;
  std::unordered_map<std::string, std::vector<int>> preciseVersions;
  struct ExactStoredValue {
    Value* value = nullptr;
    std::string preciseRootKey;
  };
  std::unordered_map<std::string, ExactStoredValue> exactStoredValues;
};

std::string valueKey(Value* v);
bool isCSECandidate(Instruction* inst);
LinearOffsetKey buildGEPLinearOffsetKey(Instruction* gep);
std::string cseKey(Instruction* inst);
Value* memoryRoot(Value* addr);
MemoryAliasInfo classifyMemory(Value* addr);
bool memoryMayAlias(const MemoryAliasInfo& lhs, const MemoryAliasInfo& rhs);
bool callMayWriteMemory(Instruction* inst, const MemoryAliasInfo& target,
                        const Module& mod,
                        const analysis::FunctionEffectMap& effects);
Value* forwardedExactStoredValue(Instruction* inst,
                                 const MemoryVersionState& state);
void recordExactStoredValue(MemoryVersionState& state,
                            Value* addr,
                            Value* storedValue);
void recordMemoryStore(MemoryVersionState& state, const MemoryAliasInfo& info,
                       int definitionToken);
void recordMemoryCall(MemoryVersionState& state, int definitionToken);
void recordMemoryCallEffects(MemoryVersionState& state, Instruction* inst,
                             const Module& mod,
                             const analysis::FunctionEffectMap& effects,
                             int definitionToken);
std::string loadCSEKey(Instruction* inst, const MemoryVersionState& state);
std::string callCSEKey(Instruction* inst, const Module& mod,
                       const analysis::FunctionEffectMap& effects,
                       const MemoryVersionState& state);
void clearLoadCSEEntries(std::unordered_map<std::string, Value*>& exprs);
bool callPreservesTrackedMemory(Instruction* inst,
                                const Module& mod,
                                const analysis::FunctionEffectMap& effects);
bool dominates(const DomSet& doms, BasicBlock* a, BasicBlock* b);
std::vector<BasicBlock*> reachableCachedBlocks(
    analysis::FunctionAnalysisCache& cache);

// 局部加载转发。
bool localLoadForwarding(Module& mod, analysis::AnalysisManager& am);

// 支配树上的公共子表达式消除。
bool dominatorTreeCSE(Module& mod, analysis::AnalysisManager& am);

// 基本块内公共子表达式消除。
bool basicBlockCSE(Module& mod, analysis::AnalysisManager& am);

// 常量全局 load 转发
bool forwardConstGlobalLoads(Module& mod);

// 单赋值标量全局转发
bool forwardSingleAssignmentScalarGlobals(Module& mod, analysis::AnalysisManager& am);

// 常量小全局数组 load 转发
bool forwardConstantSmallGlobalArrayLoads(Module& mod, analysis::AnalysisManager& am);
bool forwardConstantSmallGlobalArrayLoads(
    Module& mod, const analysis::FunctionEffectMap& effects);

bool forwardSingleAssignmentScalarGlobals(
    Function* func, Module& mod, const analysis::FunctionEffectMap& effects);

// 标量全局提升为 entry local
bool promoteDirectScalarGlobalsToEntryLocals(Module& mod, analysis::AnalysisManager& am);

bool deadPureCallElimination(Module& mod, analysis::AnalysisManager& am);
bool removeUnobservedScalarGlobalStores(Module& mod);
bool removeRedundantScalarGlobalReloadStores(
    Module& mod, analysis::AnalysisManager& am);
bool localizeRuntimeScalarGlobalBounds(Module& mod,
                                       analysis::AnalysisManager& am);

// GEP 链 rebase
bool rebaseLocalGEPChains(Module& mod);

} // namespace core_ir
