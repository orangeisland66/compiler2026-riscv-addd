// 文件职责：汇总可优化的 affine 区域，连接循环分析与实际变换。本文件负责实现具体逻辑。
#include "affine_region.h"

#include "affine_access.h"
#include "function_analysis.h"
#include "induction_var.h"
#include "loop_band.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_set>

namespace analysis {

namespace {

using core_ir::Instruction;
using core_ir::Opcode;
using core_ir::Value;
using core_ir::BasicBlock;

bool blockNameContains(const core_ir::BasicBlock* bb, const char* needle) {
  return bb && needle && bb->name().find(needle) != std::string::npos;
}

bool isOutsideLoopBlock(const NaturalLoop& loop, BasicBlock* bb) {
  return bb && !loop.blocks.count(bb);
}

bool isBranchOnlyBlockTo(BasicBlock* bb, BasicBlock* succ) {
  if (!bb || !succ)
    return false;
  Instruction* term = bb->terminator();
  if (!term || term->opcode() != Opcode::Br || term->numOperands() != 1 ||
      dynamic_cast<BasicBlock*>(term->operand(0)) != succ) {
    return false;
  }
  for (auto& instPtr : bb->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst || inst->isTerminator())
      continue;
    if (inst->opcode() != Opcode::Phi)
      return false;
  }
  return true;
}

int countLeadingPhi(BasicBlock* bb) {
  if (!bb)
    return 0;
  int count = 0;
  for (auto& instPtr : bb->instructions()) {
    if (instPtr->opcode() != Opcode::Phi)
      break;
    ++count;
  }
  return count;
}

struct BlockCostProfile {
  int instCount = 0;
  int phiCount = 0;
  int condBranchCount = 0;
  int uncondBranchCount = 0;
};

BlockCostProfile summarizeBlockCost(BasicBlock* bb) {
  BlockCostProfile profile;
  if (!bb)
    return profile;
  for (auto& instPtr : bb->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst)
      continue;
    ++profile.instCount;
    if (inst->opcode() == Opcode::Phi)
      ++profile.phiCount;
    if (inst->opcode() == Opcode::CondBr)
      ++profile.condBranchCount;
    if (inst->opcode() == Opcode::Br)
      ++profile.uncondBranchCount;
  }
  return profile;
}

void accumulateBlockCost(LoopMemoryVersioningShape& shape, BasicBlock* bb,
                         std::unordered_set<int>& seenIds) {
  if (!bb || !seenIds.insert(bb->id()).second)
    return;
  BlockCostProfile profile = summarizeBlockCost(bb);
  shape.outsideInstCount += profile.instCount;
  shape.outsidePhiCount += profile.phiCount;
  shape.outsideCondBranchCount += profile.condBranchCount;
  shape.outsideUncondBranchCount += profile.uncondBranchCount;
}

LoopMemoryVersioningShape
analyzeVersioningShape(const NaturalLoop& loop, BasicBlock* versionPreheader) {
  LoopMemoryVersioningShape shape;
  shape.versionPreheader = versionPreheader;
  if (!versionPreheader)
    return shape;

  Instruction* term = versionPreheader->terminator();
  if (!term || term->opcode() != Opcode::Br || term->numOperands() != 1 ||
      dynamic_cast<BasicBlock*>(term->operand(0)) != loop.header) {
    return shape;
  }

  shape.phiCount = countLeadingPhi(versionPreheader);
  shape.hasMergePhi = shape.phiCount > 0;

  std::vector<BasicBlock*> outsidePreds;
  std::unordered_set<int> seenPredIds;
  for (auto* pred : versionPreheader->preds()) {
    if (!pred || !isOutsideLoopBlock(loop, pred) || !seenPredIds.insert(pred->id()).second)
      continue;
    if (isBranchOnlyBlockTo(pred, versionPreheader))
      outsidePreds.push_back(pred);
  }
  shape.outsidePredCount = static_cast<int>(outsidePreds.size());
  if (outsidePreds.size() != 2 || !shape.hasMergePhi)
    return shape;

  auto classifyChoice = [](BasicBlock* bb) {
    if (!bb)
      return 0;
    if (blockNameContains(bb, "affine.interior."))
      return 1;
    if (blockNameContains(bb, "affine.remainder."))
      return -1;
    return 0;
  };

  int firstKind = classifyChoice(outsidePreds[0]);
  int secondKind = classifyChoice(outsidePreds[1]);
  if (firstKind == 1 || secondKind == -1) {
    shape.interiorPreheader = outsidePreds[0];
    shape.remainderPreheader = outsidePreds[1];
  } else if (firstKind == -1 || secondKind == 1) {
    shape.interiorPreheader = outsidePreds[1];
    shape.remainderPreheader = outsidePreds[0];
  } else {
    shape.interiorPreheader = outsidePreds[0];
    shape.remainderPreheader = outsidePreds[1];
  }
  shape.hasInteriorBridge = shape.interiorPreheader != nullptr;
  shape.hasRemainderBridge = shape.remainderPreheader != nullptr;

  std::unordered_set<int> commonPredIds;
  for (auto* pred : outsidePreds[0]->preds()) {
    if (pred && isOutsideLoopBlock(loop, pred))
      commonPredIds.insert(pred->id());
  }
  for (auto* pred : outsidePreds[1]->preds()) {
    if (!pred || !isOutsideLoopBlock(loop, pred) || !commonPredIds.count(pred->id()))
      continue;
    Instruction* predTerm = pred->terminator();
    if (!predTerm || predTerm->opcode() != Opcode::CondBr || predTerm->numOperands() != 3)
      continue;
    auto* succTrue = dynamic_cast<BasicBlock*>(predTerm->operand(1));
    auto* succFalse = dynamic_cast<BasicBlock*>(predTerm->operand(2));
    if ((succTrue == outsidePreds[0] && succFalse == outsidePreds[1]) ||
        (succTrue == outsidePreds[1] && succFalse == outsidePreds[0])) {
      shape.versionGuardBlock = pred;
      shape.hasGuardBranch = true;
      break;
    }
  }

  shape.valid = shape.hasGuardBranch && shape.hasInteriorBridge &&
                shape.hasRemainderBridge && shape.hasMergePhi;
  if (shape.valid) {
    std::unordered_set<int> seenCostIds;
    accumulateBlockCost(shape, shape.versionGuardBlock, seenCostIds);
    accumulateBlockCost(shape, shape.interiorPreheader, seenCostIds);
    accumulateBlockCost(shape, shape.remainderPreheader, seenCostIds);
    accumulateBlockCost(shape, shape.versionPreheader, seenCostIds);
  }
  return shape;
}

bool isMemoryOp(Instruction* inst) {
  return inst && (inst->opcode() == Opcode::Load || inst->opcode() == Opcode::Store ||
                  inst->opcode() == Opcode::VLoad || inst->opcode() == Opcode::VStore);
}

Value* memoryAddress(Instruction* inst) {
  if (!inst)
    return nullptr;
  if ((inst->opcode() == Opcode::Load || inst->opcode() == Opcode::VLoad) &&
      inst->numOperands() == 1)
    return inst->operand(0);
  if ((inst->opcode() == Opcode::Store || inst->opcode() == Opcode::VStore) &&
      inst->numOperands() == 2)
    return inst->operand(1);
  return nullptr;
}

bool hasDynamicScale(const AffineAccessSummary& access, Value* basis) {
  if (!access.valid || !access.offset.valid || !basis)
    return false;
  for (const auto& term : access.offset.scaledIvTerms) {
    if (term.basis == basis)
      return true;
  }
  return false;
}

void fillLoopMemoryProfile(FunctionAnalysisCache& cache,
                           core_ir::Function* func,
                           const NaturalLoop& loop,
                           AffineRegionPlan& plan) {
  if (!func)
    return;
  const auto& ivs = cache.inductionVars(loop);
  std::unordered_set<int> seenMemIds;
  for (auto* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!isMemoryOp(inst) || !seenMemIds.insert(inst->id()).second)
        continue;
      ++plan.memoryOps;
      Value* addr = memoryAddress(inst);
      if (!addr || ivs.empty())
        continue;
      auto access = summarizeAffineAccess(addr, loop, ivs);
      if (!access.valid || !access.offset.valid)
        continue;
      ++plan.affineMemoryOps;
      bool dynamic = false;
      for (const auto& iv : ivs) {
        Value* basis = iv.phi ? static_cast<Value*>(iv.phi)
                              : static_cast<Value*>(iv.storage);
        if (!basis)
          continue;
        plan.strideCost += affineStrideCostForProfit(access, basis);
        dynamic = dynamic || hasDynamicScale(access, basis);
      }
      if (dynamic)
        ++plan.dynamicStrideMemoryOps;
    }
  }
}

void fillLoopHotPathProfile(const NaturalLoop& loop, AffineRegionPlan& plan) {
  for (auto* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      ++plan.profit.hotLoopInstCount;
      if (!inst->isTerminator())
        continue;
      if (inst->opcode() != Opcode::CondBr)
        continue;
      ++plan.profit.hotLoopCondBranchCount;
      if (bb != loop.header)
        ++plan.profit.hotLoopBodyCondBranchCount;
    }
  }
}

int64_t regionLocalityRisk(const LoopBandPlan& band) {
  // region 风险沿用 loop-band 的通用画像，避免每个 pass
  // 自己发明阈值。dynamic stride 需要 versioning，寄存器压力影响 scalar expansion。
  return static_cast<int64_t>(band.totalDynamicStrideMemoryOps) * 48 +
         static_cast<int64_t>(band.estimatedRegisterPressure) * 16 +
         static_cast<int64_t>(band.members.size()) * 8;
}

int64_t computePostSplitProfit(const AffineRegionPlan& plan,
                               const LoopMemoryProfitSummary& profit) {
  const bool versionFreeFastPath =
      !plan.needsVersioning && plan.interiorRemainderReady &&
      plan.affineMemoryOps >= 2 && plan.dynamicStrideMemoryOps == 0;
  if (!profit.physicalVersioned && !versionFreeFastPath)
    return 0;
  const int affineRatio =
      plan.memoryOps == 0 ? 0 : (plan.affineMemoryOps * 100) / plan.memoryOps;
  // post-split profit 只使用通用事实：guard 已移出热循环、
  // affine 访存占比、工作集下降、动态 stride 和 CFG/寄存器压力成本。
  // 对于本来就不需要 versioning 的 band，这组字段也承载“version-free
  // interior fast path”的共享收益判断，避免干净的 dense-affine 内核被
  // 误判成“没有 physical split 就完全不能继续”的空白区。
  int64_t score = profit.physicalVersioned ? 128 : 96;
  score += static_cast<int64_t>(affineRatio) * 2;
  score += profit.estimatedWorkingSetDrop / 8;
  score += profit.localityProfitScore / 64;
  score += profit.postSplitCfgDeltaScore;
  score += profit.postSplitMemoryDeltaScore;
  score -= static_cast<int64_t>(plan.dynamicStrideMemoryOps) * 96;
  score -= profit.localityRiskScore / 4;
  score -= static_cast<int64_t>(profit.estimatedRegisterPressure) * 8;
  return score;
}

void computePostSplitDeltaMetrics(AffineRegionPlan& plan) {
  auto& profit = plan.profit;
  profit.postSplitCfgDeltaScore = 0;
  profit.postSplitMemoryDeltaScore = 0;
  profit.postSplitMetricsImproved = false;
  const bool versionFreeFastPath =
      !plan.needsVersioning && plan.interiorRemainderReady &&
      plan.affineMemoryOps >= 2 && plan.dynamicStrideMemoryOps == 0;
  if (!profit.physicalVersioned && !versionFreeFastPath)
    return;

  // 这里看 split 之后当前热循环
  // 是否已经呈现出“guard 外提、loop 内分支减少、动态访存风险下降”的
  // 通用结构收益。只有这些事实同时成立，才允许后续 pass 抬高优先级。
  int64_t cfgBenefit = 0;
  if (!plan.guard && profit.hotLoopBodyCondBranchCount == 0)
    cfgBenefit += profit.physicalVersioned ? 160 : 128;
  else if (profit.hotLoopBodyCondBranchCount == 0)
    cfgBenefit += 96;
  if (profit.hotLoopCondBranchCount <= 1)
    cfgBenefit += 48;
  if (plan.versioning.hasInteriorBridge || versionFreeFastPath)
    cfgBenefit += 24;

  int64_t cfgPenalty = 0;
  if (profit.physicalVersioned) {
    cfgPenalty += static_cast<int64_t>(plan.versioning.outsideInstCount) * 10;
    cfgPenalty += static_cast<int64_t>(plan.versioning.outsidePhiCount) * 16;
    cfgPenalty +=
        static_cast<int64_t>(plan.versioning.outsideCondBranchCount) * 96;
    cfgPenalty +=
        static_cast<int64_t>(plan.versioning.outsideUncondBranchCount) * 24;
    cfgPenalty += std::max(0, plan.versioning.outsidePredCount - 2) * 32;
  }
  cfgPenalty += static_cast<int64_t>(profit.hotLoopBodyCondBranchCount) * 80;
  cfgPenalty +=
      std::max(0, profit.hotLoopCondBranchCount - 1) * static_cast<int64_t>(24);
  int64_t cfgScore = cfgBenefit - cfgPenalty;

  const int affineRatio =
      plan.memoryOps == 0 ? 0 : (plan.affineMemoryOps * 100) / plan.memoryOps;
  int64_t memoryScore = 0;
  memoryScore += plan.affineMemoryOps * 16;
  memoryScore += affineRatio / 2;
  memoryScore += profit.estimatedWorkingSetDrop / 16;
  memoryScore -= static_cast<int64_t>(plan.dynamicStrideMemoryOps) * 96;
  memoryScore -= static_cast<int64_t>(profit.estimatedRegisterPressure) * 4;

  profit.postSplitCfgDeltaScore = cfgScore;
  profit.postSplitMemoryDeltaScore = memoryScore;
  profit.postSplitMetricsImproved =
      cfgScore > 0 && memoryScore > 0 && profit.hotLoopBodyCondBranchCount == 0 &&
      plan.dynamicStrideMemoryOps == 0;
}

void attachBandLegality(FunctionAnalysisCache& cache,
                        int loopNode,
                        AffineRegionPlan& plan) {
  const auto& bands = cache.loopBands();
  const LoopBandPlan* bestBand = nullptr;
  int bestMemberIndex = -1;
  int64_t bestScore = -1;
  for (const auto& band : bands) {
    for (int i = 0; i < static_cast<int>(band.nodeIndices.size()); ++i) {
      if (band.nodeIndices[static_cast<size_t>(i)] != loopNode)
        continue;
      int64_t score = band.bandProfitScore;
      if (i >= 0 && i < static_cast<int>(band.members.size()))
        score += band.members[static_cast<size_t>(i)].hotnessScore;
      if (score <= bestScore)
        continue;
      bestScore = score;
      bestBand = &band;
      bestMemberIndex = i;
    }
  }

  if (!bestBand)
    return;

  plan.bandRootNode = bestBand->rootNode;
  plan.bandMemberIndex = bestMemberIndex;
  plan.preferredTileFactor = bestBand->preferredTileFactor;
  plan.profit.estimatedRegisterPressure = bestBand->estimatedRegisterPressure;
  plan.profit.estimatedScalarRegisterPressure =
      bestBand->estimatedScalarRegisterPressure;
  plan.profit.estimatedWorkingSetDrop = bestBand->estimatedWorkingSetDrop;
  plan.profit.localityProfitScore = bestBand->bandProfitScore;
  plan.profit.localityRiskScore = regionLocalityRisk(*bestBand);
  plan.supportsBlocking = bestBand->legality.supportsBlocking;
  plan.supportsPermutation = bestBand->legality.supportsPermutation;
  plan.supportsScalarExpansion = bestBand->legality.supportsScalarExpansion;
  plan.needsVersioning = bestBand->legality.needsVersioning;
  const auto* node = cache.loopForest().node(loopNode);
  core_ir::BasicBlock* preheader = node ? node->preheader : nullptr;
  plan.versioning = analyzeVersioningShape(*plan.loop, preheader);
  if (!plan.guard && plan.versioning.valid) {
    // physical split 完成后，原来的 guard 已经被外提到
    // version guard block；此时 region plan 本身可能不再带 guard summary，
    // 但它仍然代表可供 blocking/scalar replacement 消费的 interior 快路径。
    plan.hotInterior = true;
  }
  if (!plan.guard && bestBand->legality.interiorRemainderReady) {
    // 若 loop-band 已经证明该 band 不需要额外 versioning 就能
    // 直接进入 interior/remainder 主线，那么即使当前 CFG 还没有显式 split，
    // 这个 region 也应被下游视为可消费的稳定快路径。
    plan.hotInterior = true;
  }
  plan.interiorRemainderReady =
      bestBand->legality.interiorRemainderReady && plan.hotInterior;
  plan.profit.physicalVersioned = plan.versioning.valid;
  computePostSplitDeltaMetrics(plan);
  plan.profit.postSplitProfitScore = computePostSplitProfit(plan, plan.profit);
  const bool versionedInteriorReady =
      plan.versioning.valid && plan.versioning.hasGuardBranch &&
      plan.versioning.hasInteriorBridge;
  const bool versionFreeInteriorReady =
      !bestBand->legality.needsVersioning && plan.interiorRemainderReady &&
      plan.affineMemoryOps >= 2 && plan.dynamicStrideMemoryOps == 0;
  const bool interiorReady = versionFreeInteriorReady || versionedInteriorReady;
  plan.profit.postSplitProfitable =
      (plan.profit.physicalVersioned || interiorReady) &&
      plan.profit.postSplitMetricsImproved &&
      plan.profit.postSplitProfitScore >= 160 &&
      plan.affineMemoryOps >= 2 && plan.dynamicStrideMemoryOps == 0;
  plan.profit.interiorFastPathReady = interiorReady;
  plan.profit.remainderPathReady =
      plan.versioning.valid ? plan.versioning.hasRemainderBridge
                            : interiorReady;
  const bool transformFastPathReady =
      plan.profit.interiorFastPathReady &&
      plan.profit.postSplitMetricsImproved &&
      plan.profit.postSplitProfitable &&
      (!plan.needsVersioning ||
       (plan.profit.physicalVersioned && plan.versioning.hasGuardBranch &&
        plan.versioning.hasInteriorBridge &&
        plan.versioning.hasRemainderBridge && plan.versioning.hasMergePhi));
  // 从这轮开始，AffineRegionPlan 不再只是排序辅助；它必须先给出
  // 可消费的 physical split，blocking/scalar replacement 才能提升优先级。
  // 没有 interior/remainder 物理分离时，普通 pointer bump 仍可继续走，
  // 但激进的 tile-local scalar replacement 和 multi-level blocking 只在
  // interior fast path 已经成立、且 split 指标确实改善时才继续放行。
  // 例外是“本来就不需要 versioning 的干净 fast path”：这类 band 没有
  // 额外 remainder/guard 成本，允许直接复用同一套 shared profit 入口。
  plan.profit.tileLocalScalarReady =
      bestBand->legality.supportsScalarExpansion &&
      transformFastPathReady && plan.affineMemoryOps >= 2 &&
      plan.profit.estimatedScalarRegisterPressure <= 16;
  plan.profit.multiLevelBlockingReady =
      bestBand->legality.hasMultiLevelBand && bestBand->legality.supportsBlocking &&
      transformFastPathReady;
}

bool canMaterializeBandBackedRegion(const AffineRegionPlan& plan) {
  if (!plan.valid || !plan.loop)
    return false;
  if (plan.versioning.valid)
    return true;
  if (plan.needsVersioning)
    return false;
  if (plan.affineMemoryOps < 2 || plan.dynamicStrideMemoryOps != 0)
    return false;

  // 没有 physical split 的 fallback region 只处理共享 loop-band 摘要显示有
  // 局部性收益的循环，避免把普通顺扫或偶然 affine 形状都放入 region 池。
  if (plan.supportsBlocking && plan.preferredTileFactor > 1 &&
      (plan.profit.estimatedWorkingSetDrop >= 64 ||
       plan.profit.localityProfitScore >= 192)) {
    return true;
  }
  if (plan.profit.tileLocalScalarReady)
    return true;
  if (plan.supportsPermutation &&
      plan.profit.localityProfitScore > plan.profit.localityRiskScore) {
    return true;
  }
  return false;
}

bool buildRegionPlanForLoop(FunctionAnalysisCache& cache, core_ir::Function* func,
                            int nodeIdx, const NaturalLoop& loop,
                            const AffineGuardSummary* guard,
                            AffineRegionPlan& plan) {
  if (nodeIdx < 0)
    return false;
  plan = AffineRegionPlan();
  plan.valid = true;
  plan.loopNode = nodeIdx;
  plan.loop = &loop;
  plan.guard = guard;
  if (guard) {
    plan.hotInterior = (guard->relation == "lt" || guard->relation == "le" ||
                        guard->relation == "gt" || guard->relation == "ge");
  }
  fillLoopMemoryProfile(cache, func, loop, plan);
  fillLoopHotPathProfile(loop, plan);
  attachBandLegality(cache, nodeIdx, plan);
  return plan.valid && plan.loop != nullptr;
}

void finalizeRegionEstimatedBenefit(FunctionAnalysisCache& cache,
                                    AffineRegionPlan& plan) {
  const auto& forest = cache.loopForest();
  const auto* guard = plan.guard;
  plan.estimatedBenefit =
      forest.node(plan.loopNode) ? forest.node(plan.loopNode)->depth * 16 : 0;
  if (guard) {
    plan.estimatedBenefit +=
        static_cast<int>(guard->boundExpr.invariantTerms.size()) * 4;
    plan.estimatedBenefit +=
        static_cast<int>(guard->boundExpr.scaledIvTerms.size()) * 8;
  }
  // region plan 是 Phase 2 的桥接层：guard 描述 interior/remainder 形状，
  // loop-band 描述 locality 和收益。这里仅汇总通用事实。
  if (plan.hotInterior)
    plan.estimatedBenefit += 16;
  if (plan.interiorRemainderReady)
    plan.estimatedBenefit += 24;
  if (plan.profit.postSplitProfitable)
    plan.estimatedBenefit += 32;
  if (plan.profit.remainderPathReady && plan.profit.postSplitProfitable)
    plan.estimatedBenefit += 12;
  plan.estimatedBenefit += static_cast<int>(plan.profit.postSplitCfgDeltaScore / 8);
  plan.estimatedBenefit += static_cast<int>(plan.profit.postSplitMemoryDeltaScore / 16);
  if (plan.profit.tileLocalScalarReady)
    plan.estimatedBenefit += 18;
  if (plan.profit.multiLevelBlockingReady)
    plan.estimatedBenefit += 20;
  plan.estimatedBenefit += static_cast<int>(plan.profit.localityProfitScore / 64);
  plan.estimatedBenefit -= static_cast<int>(plan.profit.localityRiskScore / 32);
}

} // namespace

std::vector<AffineRegionPlan> buildAffineRegionPlans(FunctionAnalysisCache& cache) {
  std::vector<AffineRegionPlan> plans;
  const auto& guards = cache.affineGuards();
  const auto& forest = cache.loopForest();
  core_ir::Function* func = cache.function();
  std::unordered_set<int> coveredLoopNodes;
  for (const auto& guard : guards) {
    if (!guard.valid || !guard.loop || !guard.header)
      continue;
    int nodeIdx = forest.findByHeader(guard.header);
    if (nodeIdx < 0)
      continue;
    AffineRegionPlan plan;
    if (!buildRegionPlanForLoop(cache, func, nodeIdx, *guard.loop, &guard, plan))
      continue;
    finalizeRegionEstimatedBenefit(cache, plan);
    plans.push_back(plan);
    coveredLoopNodes.insert(nodeIdx);
  }

  // 对未被 guard 覆盖的循环做兜底：仅当能物化为 band 支撑的区域时才
  // 生成计划。
  for (const auto& node : forest.nodes()) {
    if (!node.loop || !node.header)
      continue;
    int nodeIdx = forest.findByHeader(node.header);
    if (nodeIdx < 0 || coveredLoopNodes.count(nodeIdx))
      continue;
    AffineRegionPlan plan;
    if (!buildRegionPlanForLoop(cache, func, nodeIdx, *node.loop, nullptr, plan))
      continue;
    if (!canMaterializeBandBackedRegion(plan))
      continue;
    finalizeRegionEstimatedBenefit(cache, plan);
    plans.push_back(plan);
  }
  // 按可优化程度降序排序：优先 remainder 就绪、拆分后有利可图的计划，
  // 同分时以预估收益、最后以节点编号保证确定性。
  std::sort(plans.begin(), plans.end(),
            [](const AffineRegionPlan& lhs, const AffineRegionPlan& rhs) {
              if (lhs.interiorRemainderReady != rhs.interiorRemainderReady)
                return lhs.interiorRemainderReady > rhs.interiorRemainderReady;
              if (lhs.profit.postSplitProfitable != rhs.profit.postSplitProfitable)
                return lhs.profit.postSplitProfitable >
                       rhs.profit.postSplitProfitable;
              if (lhs.profit.multiLevelBlockingReady !=
                  rhs.profit.multiLevelBlockingReady) {
                return lhs.profit.multiLevelBlockingReady >
                       rhs.profit.multiLevelBlockingReady;
              }
              if (lhs.profit.postSplitProfitScore !=
                  rhs.profit.postSplitProfitScore) {
                return lhs.profit.postSplitProfitScore >
                       rhs.profit.postSplitProfitScore;
              }
              if (lhs.profit.tileLocalScalarReady !=
                  rhs.profit.tileLocalScalarReady) {
                return lhs.profit.tileLocalScalarReady >
                       rhs.profit.tileLocalScalarReady;
              }
              if (lhs.estimatedBenefit != rhs.estimatedBenefit)
                return lhs.estimatedBenefit > rhs.estimatedBenefit;
              return lhs.loopNode < rhs.loopNode;
            });
  return plans;
}

int64_t affineRegionNetProfitScore(const AffineRegionPlan& region) {
  const auto& profit = region.profit;
  const auto& versioning = region.versioning;
  int64_t score = region.estimatedBenefit;
  if (profit.multiLevelBlockingReady)
    score += 192;
  if (profit.tileLocalScalarReady)
    score += 160;
  if (profit.postSplitProfitable)
    score += 192;
  if (profit.postSplitMetricsImproved)
    score += 160;
  if (profit.interiorFastPathReady)
    score += 96;
  if (profit.remainderPathReady)
    score += 24;
  score += profit.postSplitProfitScore / 2;
  score += profit.postSplitCfgDeltaScore * 2;
  score += profit.postSplitMemoryDeltaScore * 2;
  score += profit.estimatedWorkingSetDrop / 4;
  score += profit.localityProfitScore / 64;
  score -= profit.localityRiskScore / 16;
  // 共享净收益用于 blocking、scalar replacement 等多条
  // loop-memory 变换主线。这里显式扣回 split 外围 CFG 成本，保证“物理
  // 分离后热路径真的更值”才会抬高后续变换优先级。
  score -= static_cast<int64_t>(versioning.outsideInstCount) * 4;
  score -= static_cast<int64_t>(versioning.outsidePhiCount) * 8;
  score -= static_cast<int64_t>(versioning.outsideCondBranchCount) * 48;
  score -= static_cast<int64_t>(versioning.outsideUncondBranchCount) * 12;
  if (!versioning.valid && region.needsVersioning)
    score -= 512;
  if (!region.valid)
    score -= 1024;
  return score;
}

const AffineRegionPlan* bestAffineRegionForLoopBand(
    FunctionAnalysisCache& cache,
    const LoopBandPlan& band) {
  std::unordered_set<int> bandNodes(band.nodeIndices.begin(), band.nodeIndices.end());
  const AffineRegionPlan* bestRegion = nullptr;
  int64_t bestScore = std::numeric_limits<int64_t>::min();
  for (const auto& region : cache.affineRegions()) {
    if (!region.valid || !bandNodes.count(region.loopNode))
      continue;
    int64_t score = affineRegionNetProfitScore(region);
    if (region.profit.tileLocalScalarReady)
      score += 64;
    if (region.profit.multiLevelBlockingReady)
      score += 48;
    // band->region 选择统一下沉到 analysis 层，后续 blocking、
    // locality plan 和其它 loop-memory 变换共享同一个“最佳 region”入口，
    // 避免不同 pass 各自维护近似但不完全一致的选择规则。
    if (score <= bestScore)
      continue;
    bestScore = score;
    bestRegion = &region;
  }
  return bestRegion;
}

} // namespace analysis
