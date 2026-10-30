// 文件职责：建立函数副作用摘要，区分只读、递归、稳定状态和 helper 属性。本文件负责声明接口和数据结构。
#pragma once

#include "../core_ir/ir/core_ir.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace analysis {

// 单个函数的副作用摘要：纯度、内存读写范围、递归性与规模统计。
struct FunctionEffectSummary {
  bool pure = false;
  bool readonly = false;
  bool argMemOnly = false;
  bool noRecurse = false;
  bool smallHelper = false;
  bool selfRecursive = false;
  bool readsArgMemory = false;
  bool readsMutableGlobalMemory = false;
  bool readsUnknownMemory = false;
  bool writesArgMemory = false;
  bool writesMutableGlobalMemory = false;
  bool writesUnknownMemory = false;
  // 按形参位置记录通过该指针发生的内存读写。这些是
  // IR/alias 事实，不依赖参数名称或运行时取值。
  std::unordered_set<std::size_t> readArgumentIndices;
  std::unordered_set<std::size_t> writtenArgumentIndices;
  std::unordered_set<std::size_t> capturedArgumentIndices;
  std::unordered_set<std::size_t> returnedArgumentIndices;
  std::unordered_set<const core_ir::GlobalVar*> writtenGlobals;
  bool stableGlobalRead = false;
  bool stableInvocationState = false;
  bool memoizableRecursion = false;
  int selfRecursiveCallCount = 0;
  int instructionCount = 0;
  int loopCount = 0;
};

using FunctionEffectMap =
    std::unordered_map<const core_ir::Function*, FunctionEffectSummary>;

// 递归优化模式：无优化 / 记忆化 / 快速路径。
enum class RecursiveOptimizationMode {
  None,
  Memoized,
  FastPath,
};

// 记忆化存储布局：无 / 一维稠密表 / 二维稠密表 / 哈希回退。
enum class RecursiveMemoLayout {
  None,
  Dense1D,
  Dense2D,
  HashFallback,
};

// 递归状态（参数/全局标量）的变化方向：未知 / 单调不增 / 稳定 / 单调不减。
enum class RecursiveStateDirection {
  Unknown = 0,
  NonIncreasing = -1,
  Stable = 1,
  NonDecreasing = 2,
};

// 递归函数单个参数的取值范围与步进特征，用于稠密记忆化表的尺寸估算。
struct RecursiveArgumentRange {
  bool valid = false;
  bool nonNegative = false;
  bool hasKnownStep = false;
  int32_t minValue = 0;
  int32_t maxValue = 0;
  int32_t step = 0;
  RecursiveStateDirection direction = RecursiveStateDirection::Unknown;
};

// 自递归函数的结构汇总：可记忆化性、状态稳定性、参数范围与选定的优化模式。
struct RecursiveFunctionSummary {
  bool selfRecursive = false;
  bool finiteStateI32Tuple = false;
  bool readonly = false;
  bool stableInvocationState = false;
  bool repeatedSubproblem = false;
  bool persistentMemoState = false;
  bool stableStateAcrossCalls = false;
  bool tailRecursiveOnly = false;
  int recursiveBranchCount = 0;
  int selfRecursiveCallCount = 0;
  bool monotonicStateProgress = false;
  bool returnValuePure = false;
  std::vector<const core_ir::GlobalVar*> stableScalarGlobals;
  std::vector<RecursiveArgumentRange> argumentRanges;
  std::vector<int32_t> denseLowerBounds;
  std::vector<int32_t> denseExtents;
  RecursiveMemoLayout memoLayout = RecursiveMemoLayout::None;
  size_t denseCellEstimate = 0;
  size_t denseByteEstimate = 0;
  bool needsStateFingerprint = false;
  RecursiveOptimizationMode mode = RecursiveOptimizationMode::None;
};

using RecursiveFunctionMap =
    std::unordered_map<const core_ir::Function*, RecursiveFunctionSummary>;

// 汇总模块内所有函数的副作用信息（纯/只读/内存读写/递归性等）。
FunctionEffectMap summarizeModuleFunctionEffects(core_ir::Module& mod);

// 在副作用汇总基础上，进一步汇总自递归函数的结构特征（供记忆化/快速路径优化决策）。
RecursiveFunctionMap summarizeModuleRecursiveFunctions(
    core_ir::Module& mod,
    const FunctionEffectMap& effects);

// 按函数指针查询副作用汇总；未找到返回 nullptr。
const FunctionEffectSummary* lookupFunctionEffect(
    const FunctionEffectMap& effects,
    const core_ir::Function* func);

// 按被调函数名在模块中查找并返回副作用汇总；未找到返回 nullptr。
const FunctionEffectSummary* lookupFunctionEffect(
    const FunctionEffectMap& effects,
    const core_ir::Module& mod,
    const std::string& calleeName);

// 按函数指针查询递归函数结构汇总；未找到返回 nullptr。
const RecursiveFunctionSummary* lookupRecursiveSummary(
    const RecursiveFunctionMap& summaries,
    const core_ir::Function* func);

// 返回递归优化模式的名称字符串（调试/日志用）。
const char* recursiveOptimizationModeName(RecursiveOptimizationMode mode);
// 返回递归记忆化存储布局的名称字符串（调试/日志用）。
const char* recursiveMemoLayoutName(RecursiveMemoLayout layout);

// 判断一条 call 指令的被调函数是否为纯函数（无副作用、可自由重排/消除）。
bool isPureCall(const core_ir::Instruction* inst,
                const core_ir::Module& mod,
                const FunctionEffectMap& effects);

// 判断一条 call 指令的被调函数是否只读（不改内存，可提升/重排但不能任意消除）。
bool isReadonlyCall(const core_ir::Instruction* inst,
                    const core_ir::Module& mod,
                    const FunctionEffectMap& effects);

// 判断一条 call 指令的被调函数是否为轻量小函数（内联代价判断用）。
bool isSmallHelperCall(const core_ir::Instruction* inst,
                       const core_ir::Module& mod,
                       const FunctionEffectMap& effects);

// 基于递归结构汇总判断函数是否适合记忆化优化。
bool isMemoizableRecursiveFunction(const RecursiveFunctionSummary* summary,
                                   const core_ir::Function* func);

// 基于副作用汇总判断函数是否适合记忆化优化（轻量版判定）。
bool isMemoizableRecursiveFunction(const FunctionEffectSummary* summary,
                                   const core_ir::Function* func);

} // namespace analysis
