// 文件职责：收集函数级结构信息，辅助内联、递归和副作用判断。本文件负责声明接口和数据结构。
#pragma once

#include "../core_ir/ir/core_ir.h"
#include "affine_guard.h"
#include "affine_recurrence.h"
#include "affine_region.h"
#include "dominators.h"
#include "function_effects.h"
#include "induction_var.h"
#include "loop_band.h"
#include "loop_forest.h"
#include "loop_info.h"
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace analysis {

class AnalysisManager;

// 运行时标量全局变量边界的信息汇总：比较指令、直接 load、涉及的全局变量及安全性判定。
struct RuntimeScalarGlobalBoundInfo {
  core_ir::Instruction* compare = nullptr;          // 作为循环边界的比较指令
  std::vector<core_ir::Instruction*> directLoads;   // 边界值来源的直接 load 指令
  std::vector<core_ir::GlobalVar*> globals;         // 涉及的全局变量集合
  bool hasInLoopLoad = false;                       // 边界 load 是否位于循环内部（循环内读取视为不可靠）
  bool unsafe = false;                              // 是否被判定为不安全边界
  bool localizable = false;                         // 该边界 load 是否可提升为循环不变量
};

// 判断循环体内是否存在聚合内存（数组/指针）访问。
bool loopTouchesAggregateMemory(const NaturalLoop& loop);

// 收集循环中"以全局标量值为运行时边界"的比较，供后续循环变换做安全性判定。
std::vector<RuntimeScalarGlobalBoundInfo> collectRuntimeScalarGlobalBounds(
    core_ir::Function* func, const NaturalLoop& loop,
    const std::vector<InductionVar>& ivs,
    const FunctionEffectMap* effects = nullptr);

// 判断循环是否存在不安全的全局标量运行时边界（存在则禁止依赖该边界的变换）。
bool loopHasUnsafeRuntimeScalarGlobalBound(
    core_ir::Function* func, const NaturalLoop& loop,
    const std::vector<InductionVar>& ivs,
    const FunctionEffectMap* effects = nullptr);

// 函数级分析的惰性缓存：各分析结果按需计算并缓存，供多个 pass 共享，避免重复计算。
class FunctionAnalysisCache {
public:
  // 构造时绑定目标函数；manager 可选，用于复用模块级分析结果。
  explicit FunctionAnalysisCache(core_ir::Function* func,
                                 AnalysisManager* manager = nullptr);

  // 返回缓存所绑定的函数。
  core_ir::Function* function() const { return func_; }

  // CFG 被改动后调用：使基本块/支配树/循环等依赖 CFG 的缓存全部失效。
  void invalidateCFG();

  // 按线性化顺序返回全部基本块。
  const std::vector<core_ir::BasicBlock*>& blocks();
  // 返回从入口块可达的基本块集合。
  const std::unordered_set<core_ir::BasicBlock*>& reachable();
  // 返回支配树（支配块集合）。
  const DomSet& dominators();
  // 返回函数内识别出的全部自然循环。
  const std::vector<NaturalLoop>& loops();
  // 返回循环嵌套森林（循环间的父子关系）。
  const LoopForest& loopForest();
  // 返回指定循环的归纳变量列表。
  const std::vector<InductionVar>& inductionVars(const NaturalLoop& loop);
  const LoopTripRecipe& loopTrip(const NaturalLoop& loop);
  // 返回指定循环的仿射递推（归纳变量线性递推）求解结果。
  const std::vector<AffineRecurrencePlan>& affineRecurrences(
      const NaturalLoop& loop);
  // 返回函数内仿射守卫（形如 if(i < n) 的循环保护条件）的汇总。
  const std::vector<AffineGuardSummary>& affineGuards();
  // 返回可整体调度的循环带（连续嵌套仿射循环组）规划。
  const std::vector<LoopBandPlan>& loopBands();
  // 返回仿射区域（可整体变换的仿射代码区）的划分规划。
  const std::vector<AffineRegionPlan>& affineRegions();
  // 判断指定循环是否存在不安全的全局标量运行时边界。
  bool hasUnsafeRuntimeScalarGlobalBound(const NaturalLoop& loop);
  // 返回函数内指令总数（用于内联代价等判断）。
  int instructionCount();

private:
  void ensureBlocksAndReachable();
  void ensureDominators();
  void ensureLoops();
  void ensureLoopForest();
  void ensureAffineGuards();
  void ensureLoopBands();
  void ensureAffineRegions();

  core_ir::Function* func_ = nullptr;
  AnalysisManager* manager_ = nullptr;

  bool blocksReady_ = false;
  bool domsReady_ = false;
  bool loopsReady_ = false;
  bool loopForestReady_ = false;
  bool affineGuardsReady_ = false;
  bool loopBandsReady_ = false;
  bool affineRegionsReady_ = false;
  bool instructionCountReady_ = false;

  std::vector<core_ir::BasicBlock*> blocks_;
  std::unordered_set<core_ir::BasicBlock*> reachable_;
  DomSet doms_;
  std::vector<NaturalLoop> loops_;
  LoopForest loopForest_;
  std::unordered_map<int, std::vector<InductionVar>> inductionVarsByHeader_;
  std::unordered_map<int, LoopTripRecipe> loopTripsByHeader_;
  std::unordered_map<int, std::vector<AffineRecurrencePlan>>
      affineRecurrencesByHeader_;
  std::unordered_map<int, bool> unsafeRuntimeScalarBoundByHeader_;
  std::vector<InductionVar> emptyInductionVars_;
  LoopTripRecipe emptyLoopTrip_;
  std::vector<AffineRecurrencePlan> emptyAffineRecurrences_;
  std::vector<AffineGuardSummary> affineGuards_;
  std::vector<LoopBandPlan> loopBands_;
  std::vector<AffineRegionPlan> affineRegions_;
  int instructionCount_ = 0;
};

} // namespace analysis
