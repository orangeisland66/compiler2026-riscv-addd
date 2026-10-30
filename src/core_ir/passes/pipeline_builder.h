// 文件职责：把公开 CLI 模式映射成内部优化流水线配置。本文件负责声明接口和数据结构。
#pragma once

#include "passes.h"

#include <string>
#include <unordered_set>

namespace analysis { class AnalysisManager; }

namespace core_ir {

enum class PipelineMode {
  Functional,
  Performance,
};

struct HIRTransformSummary {
  // 若 HIR 层仍保留可消费的 loop-memory 结构，则 CoreIR 可继续
  // 跑完整 affine/loop-memory 主线；否则只跑保守清理。
  bool preservesLoopMemoryStructure = true;
  // 某些高层重写会破坏 band 形状，但仍值得在 CoreIR 末尾补做
  // 低风险 loop cleanup，这里只表达能力，不绑定具体 pass 名。
  bool needsStandaloneLoopCleanup = false;
  // 是否发生过递归结构性改写，例如 worklist lowering、
  // helper normalize 或尾递归包装。
  bool rewroteRecursiveStructure = false;
  // 这些函数已经在 HIR 层完成递归结构改写，后续 CoreIR 递归 pass
  // 只能把它们当普通函数看待，不能再次尝试结构性改写。
  std::unordered_set<std::string> blockedPostHIRRecursiveFunctions;
};

struct PipelineBuildOptions {
  PipelineMode mode = PipelineMode::Functional;
  bool coreIROptEnabled = true;
  HIRTransformSummary hirSummary;
  analysis::AnalysisManager* analysisManager = nullptr;
};

struct PipelinePlan {
  PipelineMode mode = PipelineMode::Functional;
  // 功能入口直接走旧 codegen；性能入口进入 CoreIR SSA 流水线。
  bool runCoreIR = false;
  PipelineOptions coreIR;
};

PipelinePlan buildPipelinePlan(const PipelineBuildOptions& options);

} // namespace core_ir
