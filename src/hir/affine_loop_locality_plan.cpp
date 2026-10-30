// 文件职责：承接 affine loop locality 优化入口，逐步把 AST 过渡规则迁移到通用 HIR/CoreIR 计划。本文件负责实现具体逻辑。
#include "affine_loop_locality_plan.h"

#include "../analysis/analysis_manager.h"
#include "../analysis/affine_region.h"
#include "../core_ir/lowering/ast_to_ir.h"
#include "../analysis/loop_band.h"

#include <algorithm>

namespace hir {

namespace {

int64_t localityRiskScore(const analysis::LoopBandPlan& band) {
  // risk 只描述通用结构风险。
  // dynamic stride 和寄存器压力越高，越应该先走 versioning/保守路径。
  return static_cast<int64_t>(band.totalDynamicStrideMemoryOps) * 48 +
         static_cast<int64_t>(band.estimatedRegisterPressure) * 16 +
         static_cast<int64_t>(band.members.size()) * 8;
}

int localityActionPriorityImpl(AffineLoopLocalityAction action) {
  switch (action) {
  case AffineLoopLocalityAction::VersionedBlocking:
    return 5;
  case AffineLoopLocalityAction::Blocking:
    return 4;
  case AffineLoopLocalityAction::Permutation:
    return 3;
  case AffineLoopLocalityAction::ScalarExpansion:
    return 2;
  case AffineLoopLocalityAction::None:
    return 0;
  }
  return 0;
}

// primary action 先看共享 region 净收益与 split readiness，
// 再决定更像 blocking、scalar expansion 还是 permutation；这样不同 pass
// 不会各自按 band 局部画像做出相互矛盾的第一选择。
AffineLoopLocalityAction
choosePrimaryAction(const AffineLoopLocalityCandidate& cand) {
  if (cand.preferredTileFactor <= 1 || cand.totalAffineMemoryOps < 2)
    return AffineLoopLocalityAction::None;
  if (cand.needsVersioning && cand.postSplitProfitable &&
      cand.interiorFastPathReady && cand.regionNetProfitScore > cand.riskScore / 2) {
    return AffineLoopLocalityAction::VersionedBlocking;
  }
  if (cand.supportsScalarExpansion && cand.tileLocalScalarReady &&
      cand.estimatedRegisterPressure <= 16 && cand.totalAffineMemoryOps >= 3 &&
      cand.regionNetProfitScore + cand.estimatedWorkingSetDrop >= cand.riskScore / 2) {
    return AffineLoopLocalityAction::ScalarExpansion;
  }
  if (cand.supportsPermutation && cand.preferPermutation &&
      cand.totalDynamicStrideMemoryOps == 0 &&
      cand.regionNetProfitScore + cand.estimatedWorkingSetDrop > cand.riskScore) {
    return AffineLoopLocalityAction::Permutation;
  }
  if (cand.supportsBlocking &&
      (cand.postSplitProfitable || cand.regionNetProfitScore > 0)) {
    return AffineLoopLocalityAction::Blocking;
  }
  if (cand.supportsScalarExpansion && cand.estimatedRegisterPressure <= 16 &&
      cand.totalAffineMemoryOps >= 3) {
    return AffineLoopLocalityAction::ScalarExpansion;
  }
  if (cand.supportsPermutation && cand.preferPermutation)
    return AffineLoopLocalityAction::Permutation;
  if (cand.supportsBlocking)
    return AffineLoopLocalityAction::Blocking;
  return AffineLoopLocalityAction::None;
}

// locality candidate 的排序要把 region/band 的共享净收益、
// CFG 版本化状态和动作偏好一起考虑，避免 collect 阶段和具体变换阶段
// 使用两套不同优先级。
int64_t localityCandidatePriorityImpl(
    const AffineLoopLocalityCandidate& cand) {
  int64_t score = cand.regionNetProfitScore * 4 + cand.profitScore * 2 +
                  cand.estimatedWorkingSetDrop - cand.riskScore;
  score +=
      static_cast<int64_t>(localityActionPriorityImpl(cand.primaryAction)) *
      128;
  if (cand.postSplitProfitable)
    score += 192;
  if (cand.postSplitMetricsImproved)
    score += 160;
  if (cand.interiorFastPathReady)
    score += 128;
  if (cand.tileLocalScalarReady)
    score += 160;
  if (cand.multiLevelBlockingReady)
    score += 160;
  if (cand.hasVersionGuardBranch)
    score += 96;
  if (cand.hasVersionMergePhi)
    score += static_cast<int64_t>(cand.versionMergePhiCount) * 16;
  if (cand.needsVersioning && !cand.postSplitProfitable)
    score -= 256;
  score += cand.postSplitCfgDeltaScore;
  score += cand.postSplitMemoryDeltaScore;
  score += cand.regionLocalityProfitScore / 32;
  score -= cand.regionLocalityRiskScore / 8;
  if (cand.preferPermutation && cand.primaryAction == AffineLoopLocalityAction::Permutation)
    score += 96;
  return score;
}

} // namespace

int affineLoopLocalityActionPriority(AffineLoopLocalityAction action) {
  return localityActionPriorityImpl(action);
}

int64_t affineLoopLocalityCandidatePriority(
    const AffineLoopLocalityCandidate& cand) {
  return localityCandidatePriorityImpl(cand);
}

bool applyAffineLoopLocalityPlan(
    CompUnit& cu,
    const AffineLoopLocalityPlanOptions& options) {
  (void)cu;
  (void)options;
  return false;
}

AffineLoopLocalityCandidate buildAffineLoopLocalityCandidate(
    core_ir::Function* func,
    const analysis::LoopBandPlan& band,
    analysis::FunctionAnalysisCache* cache) {
  AffineLoopLocalityCandidate cand;
  cand.function = func;
  cand.rootNode = band.rootNode;
  cand.preferredTileMember = band.preferredTileMember;
  cand.preferredTileFactor = band.preferredTileFactor;
  cand.totalMemoryOps = band.totalMemoryOps;
  cand.totalAffineMemoryOps = band.totalAffineMemoryOps;
  cand.totalUnitStrideMemoryOps = band.totalUnitStrideMemoryOps;
  cand.totalStridedMemoryOps = band.totalStridedMemoryOps;
  cand.totalDynamicStrideMemoryOps = band.totalDynamicStrideMemoryOps;
  cand.estimatedWorkingSetDrop = band.estimatedWorkingSetDrop;
  cand.estimatedRegisterPressure = band.estimatedRegisterPressure;
  cand.profitScore = band.bandProfitScore;
  cand.riskScore = localityRiskScore(band);
  cand.maxStrideCost = band.maxStrideCost;
  cand.preferPermutation = band.preferPermutation;
  cand.supportsPermutation = band.legality.supportsPermutation;
  // 这里先生成“可执行计划的摘要”，真正 transform 仍需要
  // dependence/alias/versioning/CFG 形状检查。
  cand.supportsBlocking = band.legality.supportsBlocking;
  cand.supportsScalarExpansion = band.legality.supportsScalarExpansion;
  cand.needsVersioning = band.legality.needsVersioning;
  if (cache) {
    if (const auto* region = analysis::bestAffineRegionForLoopBand(*cache, band)) {
      cand.regionNetProfitScore = analysis::affineRegionNetProfitScore(*region);
      cand.postSplitCfgDeltaScore = region->profit.postSplitCfgDeltaScore;
      cand.postSplitMemoryDeltaScore = region->profit.postSplitMemoryDeltaScore;
      cand.regionLocalityProfitScore = region->profit.localityProfitScore;
      cand.regionLocalityRiskScore = region->profit.localityRiskScore;
      cand.interiorFastPathReady = region->profit.interiorFastPathReady;
      cand.postSplitProfitable = region->profit.postSplitProfitable;
      cand.postSplitMetricsImproved = region->profit.postSplitMetricsImproved;
      cand.tileLocalScalarReady = region->profit.tileLocalScalarReady;
      cand.multiLevelBlockingReady = region->profit.multiLevelBlockingReady;
      cand.hasVersionGuardBranch = region->versioning.hasGuardBranch;
      cand.hasVersionMergePhi = region->versioning.hasMergePhi;
      cand.versionMergePhiCount = region->versioning.phiCount;
    }
  }
  cand.primaryAction = choosePrimaryAction(cand);
  return cand;
}

std::vector<AffineLoopLocalityCandidate> collectAffineLoopLocalityPlans(
    core_ir::Module& mod,
    analysis::AnalysisManager& analysisManager) {
  std::vector<AffineLoopLocalityCandidate> candidates;
  for (auto& funcPtr : mod.functions()) {
    core_ir::Function* func = funcPtr.get();
    if (!func || !func->entryBlock())
      continue;

    analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
    for (const auto& band : cache.loopBands()) {
      if (!band.valid || band.totalAffineMemoryOps < 2) {
        continue;
      }

      auto cand = buildAffineLoopLocalityCandidate(func, band, &cache);
      if (cand.profitScore <= 0 && cand.regionNetProfitScore <= 0)
        continue;
      candidates.push_back(cand);
    }
  }

  std::sort(candidates.begin(), candidates.end(),
            [](const AffineLoopLocalityCandidate& lhs,
               const AffineLoopLocalityCandidate& rhs) {
              int64_t lhsPriority = affineLoopLocalityCandidatePriority(lhs);
              int64_t rhsPriority = affineLoopLocalityCandidatePriority(rhs);
              if (lhsPriority != rhsPriority)
                return lhsPriority > rhsPriority;
              if (lhs.regionNetProfitScore != rhs.regionNetProfitScore) {
                return lhs.regionNetProfitScore > rhs.regionNetProfitScore;
              }
              if (lhs.profitScore != rhs.profitScore)
                return lhs.profitScore > rhs.profitScore;
              if (lhs.estimatedWorkingSetDrop != rhs.estimatedWorkingSetDrop)
                return lhs.estimatedWorkingSetDrop > rhs.estimatedWorkingSetDrop;
              if (lhs.riskScore != rhs.riskScore)
                return lhs.riskScore < rhs.riskScore;
              return lhs.totalAffineMemoryOps > rhs.totalAffineMemoryOps;
            });
  return candidates;
}

} // namespace hir
