// 文件职责：汇总可优化的 affine 区域，连接循环分析与实际变换。本文件负责声明接口和数据结构。
#pragma once

#include "affine_guard.h"

#include <cstdint>
#include <vector>

namespace analysis {

class FunctionAnalysisCache;
struct LoopBandPlan;

struct LoopMemoryVersioningShape {
  core_ir::BasicBlock* versionGuardBlock = nullptr;
  core_ir::BasicBlock* interiorPreheader = nullptr;
  core_ir::BasicBlock* remainderPreheader = nullptr;
  core_ir::BasicBlock* versionPreheader = nullptr;
  int phiCount = 0;
  int outsidePredCount = 0;
  int outsideInstCount = 0;
  int outsidePhiCount = 0;
  int outsideCondBranchCount = 0;
  int outsideUncondBranchCount = 0;
  bool valid = false;
  bool hasGuardBranch = false;
  bool hasInteriorBridge = false;
  bool hasRemainderBridge = false;
  bool hasMergePhi = false;
};

struct LoopMemoryProfitSummary {
  int estimatedRegisterPressure = 0;
  int estimatedScalarRegisterPressure = 0;
  int hotLoopInstCount = 0;
  int hotLoopCondBranchCount = 0;
  int hotLoopBodyCondBranchCount = 0;
  int64_t estimatedWorkingSetDrop = 0;
  int64_t localityProfitScore = 0;
  int64_t localityRiskScore = 0;
  int64_t postSplitCfgDeltaScore = 0;
  int64_t postSplitMemoryDeltaScore = 0;
  int64_t postSplitProfitScore = 0;
  bool physicalVersioned = false;
  bool postSplitMetricsImproved = false;
  bool postSplitProfitable = false;
  bool interiorFastPathReady = false;
  bool remainderPathReady = false;
  bool tileLocalScalarReady = false;
  bool multiLevelBlockingReady = false;
};

struct AffineRegionPlan {
  bool valid = false;
  int loopNode = -1;
  int bandRootNode = -1;
  int bandMemberIndex = -1;
  const NaturalLoop* loop = nullptr;
  const AffineGuardSummary* guard = nullptr;
  int memoryOps = 0;
  int affineMemoryOps = 0;
  int dynamicStrideMemoryOps = 0;
  int64_t strideCost = 0;
  int preferredTileFactor = 1;
  LoopMemoryVersioningShape versioning;
  LoopMemoryProfitSummary profit;
  int estimatedBenefit = 0;
  bool hotInterior = false;
  bool supportsBlocking = false;
  bool supportsPermutation = false;
  bool supportsScalarExpansion = false;
  bool needsVersioning = false;
  bool interiorRemainderReady = false;
};

std::vector<AffineRegionPlan> buildAffineRegionPlans(FunctionAnalysisCache& cache);
int64_t affineRegionNetProfitScore(const AffineRegionPlan& region);
const AffineRegionPlan* bestAffineRegionForLoopBand(
    FunctionAnalysisCache& cache,
    const LoopBandPlan& band);

} // namespace analysis
