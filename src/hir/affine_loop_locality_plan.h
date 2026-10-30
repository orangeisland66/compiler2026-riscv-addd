// 文件职责：承接 affine loop locality 优化入口，逐步把 AST 过渡规则迁移到通用 HIR/CoreIR 计划。本文件负责声明接口和数据结构。
#pragma once

#include "../frontend/ast.h"
#include "../core_ir/ir/core_ir.h"

#include <cstdint>
#include <vector>

namespace analysis {
class AnalysisManager;
class FunctionAnalysisCache;
struct LoopBandPlan;
} // namespace analysis

namespace hir {

struct AffineLoopLocalityPlanOptions {
  bool allowLegacyAstFallback = false;
};

enum class AffineLoopLocalityAction {
  None,
  Blocking,
  Permutation,
  ScalarExpansion,
  VersionedBlocking,
};

struct AffineLoopLocalityCandidate {
  core_ir::Function* function = nullptr;
  int rootNode = -1;
  int preferredTileMember = -1;
  int preferredTileFactor = 1;
  int totalMemoryOps = 0;
  int totalAffineMemoryOps = 0;
  int totalUnitStrideMemoryOps = 0;
  int totalStridedMemoryOps = 0;
  int totalDynamicStrideMemoryOps = 0;
  int64_t estimatedWorkingSetDrop = 0;
  int estimatedRegisterPressure = 0;
  int64_t profitScore = 0;
  int64_t riskScore = 0;
  int64_t regionNetProfitScore = 0;
  int64_t postSplitCfgDeltaScore = 0;
  int64_t postSplitMemoryDeltaScore = 0;
  int64_t regionLocalityProfitScore = 0;
  int64_t regionLocalityRiskScore = 0;
  int64_t maxStrideCost = 0;
  AffineLoopLocalityAction primaryAction = AffineLoopLocalityAction::None;
  bool preferPermutation = false;
  bool supportsPermutation = false;
  bool supportsScalarExpansion = false;
  bool supportsBlocking = false;
  bool needsVersioning = false;
  bool interiorFastPathReady = false;
  bool postSplitProfitable = false;
  bool postSplitMetricsImproved = false;
  bool tileLocalScalarReady = false;
  bool multiLevelBlockingReady = false;
  bool hasVersionGuardBranch = false;
  bool hasVersionMergePhi = false;
  int versionMergePhiCount = 0;
};

// 通用 locality 计划入口。该入口只负责分析驱动的 plan 汇总，不再回落到
  // AST 回退路径。
bool applyAffineLoopLocalityPlan(
    CompUnit& cu,
    const AffineLoopLocalityPlanOptions& options = {});

std::vector<AffineLoopLocalityCandidate> collectAffineLoopLocalityPlans(
    core_ir::Module& mod,
    analysis::AnalysisManager& analysisManager);

int affineLoopLocalityActionPriority(AffineLoopLocalityAction action);
int64_t affineLoopLocalityCandidatePriority(
    const AffineLoopLocalityCandidate& cand);

// 把 LoopBandPlan 规范化成 locality 候选。多个 transform pass
// 可以共享这层 plan/risk/action，而不是各自重新写一套收益排序。
AffineLoopLocalityCandidate buildAffineLoopLocalityCandidate(
    core_ir::Function* func,
    const analysis::LoopBandPlan& band,
    analysis::FunctionAnalysisCache* cache = nullptr);

} // namespace hir
