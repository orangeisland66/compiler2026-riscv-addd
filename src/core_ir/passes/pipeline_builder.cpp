// 文件职责：根据功能/性能两种公开入口生成内部优化配置。本文件负责实现具体逻辑。
#include "pipeline_builder.h"

namespace core_ir {

PipelinePlan buildPipelinePlan(const PipelineBuildOptions& options) {
  PipelinePlan plan;
  plan.mode = options.mode;
  plan.runCoreIR = options.coreIROptEnabled &&
                   options.mode == PipelineMode::Performance;

  if (options.mode != PipelineMode::Performance)
    return plan;

  // 性能入口公开仍叫 -O1，但内部走 O3 级预算。这里先保持旧性能路径的
  // 开关组合不变，只把策略从 main.cpp 搬到统一 PipelineBuilder。
  plan.coreIR.enableConstStrengthReduce = true;
  plan.coreIR.enableArraySROA = true;
  plan.coreIR.enableLoadHoist = true;
  plan.coreIR.enableArrayRecurrence = true;
#ifdef SYC_ENABLE_RESEARCH_PASSES
  // 两项循环重排仍在完善候选发现与变换闭环，只向 research 配置暴露；
  // 实际运行还需要各自的 SYC_ENABLE_* 开关。
  plan.coreIR.enableLoopFusion = true;
  plan.coreIR.enableLoopInterchange = true;
#else
  plan.coreIR.enableLoopFusion = false;
  plan.coreIR.enableLoopInterchange = false;
#endif
  plan.coreIR.enableLoopUnroll = true;
  plan.coreIR.enableLoopBlocking = true;
  // 无用返回值消除默认启用，可用 SYC_SKIP_COREIR_DEAD_RETURN 旁路。
  plan.coreIR.enableDeadReturnElim = true;
#ifdef SYC_ENABLE_RESEARCH_PASSES
  // 标量累加槽提升尚未在默认用例中形成独立正例，只向 research 配置暴露；
  // 实际运行仍需要 SYC_ENABLE_HIR_LOOP_SCALAR_ACCUMULATOR。
  plan.coreIR.enableLoopScalarAccumulator = true;
#else
  plan.coreIR.enableLoopScalarAccumulator = false;
#endif

  // PipelineBuilder 只消费 HIR 变换后的通用能力摘要，不关心
  // 摘要来自哪个具体 pass。后续如果某个 HIR pass 破坏了可消费的 loop
  // memory 结构，就通过 preservesLoopMemoryStructure=false 表达；如果
  // 它需要保守的 standalone cleanup，则通过 needsStandaloneLoopCleanup
  // 表达。
  plan.coreIR.enableHIRLoopMemory =
      options.hirSummary.preservesLoopMemoryStructure;
  plan.coreIR.enableStandaloneLoopUnroll =
      options.hirSummary.needsStandaloneLoopCleanup;
  plan.coreIR.enableStandaloneAffineScalarRewrite =
      options.hirSummary.needsStandaloneLoopCleanup;
  plan.coreIR.rewroteRecursiveStructure =
      options.hirSummary.rewroteRecursiveStructure;
  plan.coreIR.blockedPostHIRRecursiveFunctions =
      options.hirSummary.blockedPostHIRRecursiveFunctions;
  plan.coreIR.analysisManager = options.analysisManager;
  return plan;
}

} // namespace core_ir
