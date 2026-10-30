// 文件职责：规划多层 loop band 的交换、分块和收益模型。本文件负责声明接口和数据结构。
#pragma once

#include "affine_access.h"
#include "induction_var.h"
#include "loop_forest.h"

#include <cstdint>
#include <vector>

namespace analysis {

class FunctionAnalysisCache;

struct LoopBandMemberPlan {
  const NaturalLoop* loop = nullptr;
  const LoopTreeNode* node = nullptr;
  std::vector<InductionVar> ivs;
  int loopDepth = 0;
  int blockCount = 0;
  int memoryOps = 0;
  int affineMemoryOps = 0;
  int indexedAffineMemoryOps = 0;
  int unitStrideMemoryOps = 0;
  int stridedMemoryOps = 0;
  int dynamicStrideMemoryOps = 0;
  int distinctIndexedRoots = 0;
  int rootReuseScore = 0;
  int64_t affineStrideScore = 0;
  int64_t maxStrideCost = 0;
  int64_t hotnessScore = 0;
  int suggestedTileFactor = 1;
};

struct LoopBandTransformLegality {
  bool sideEffectFree = false;
  bool hasMultiLevelBand = false;
  bool supportsBlocking = false;
  bool supportsPermutation = false;
  bool supportsScalarExpansion = false;
  bool needsVersioning = false;
  bool interiorRemainderReady = false;
};

struct LoopBandPlan {
  bool valid = false;
  int rootNode = -1;
  std::vector<int> nodeIndices;
  std::vector<LoopBandMemberPlan> members;
  LoopBandTransformLegality legality;
  int preferredTileMember = -1;
  int preferredTileFactor = 1;
  bool preferPermutation = false;
  int totalMemoryOps = 0;
  int totalAffineMemoryOps = 0;
  int totalIndexedAffineMemoryOps = 0;
  int totalUnitStrideMemoryOps = 0;
  int totalStridedMemoryOps = 0;
  int totalDynamicStrideMemoryOps = 0;
  int maxDistinctIndexedRoots = 0;
  int totalRootReuseScore = 0;
  int64_t totalStrideScore = 0;
  int64_t maxStrideCost = 0;
  int64_t estimatedWorkingSetDrop = 0;
  int estimatedRegisterPressure = 0;
  int estimatedScalarRegisterPressure = 0;
  int64_t bandProfitScore = 0;
};

std::vector<LoopBandPlan> buildLoopBandPlans(core_ir::Function* func,
                                             FunctionAnalysisCache& cache);

} // namespace analysis
