// 文件职责：规划多层 loop band 的交换、分块和收益模型。本文件负责实现具体逻辑。
#include "loop_band.h"

#include "affine_access.h"
#include "function_analysis.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <unordered_map>
#include <unordered_set>

namespace analysis {

namespace {

using core_ir::Function;
using core_ir::Instruction;
using core_ir::Opcode;
using core_ir::Value;

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

bool isScalarLike(Opcode op) {
  switch (op) {
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Mul:
  case Opcode::SDiv:
  case Opcode::SRem:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
  case Opcode::Shl:
  case Opcode::Shr:
  case Opcode::FAdd:
  case Opcode::FSub:
  case Opcode::FMul:
  case Opcode::FDiv:
  case Opcode::ICmp:
  case Opcode::FCmp:
  case Opcode::SIToFP:
  case Opcode::FPToSI:
  case Opcode::GEP:
  case Opcode::Phi:
  case Opcode::Neg:
  case Opcode::FNeg:
  case Opcode::BitCast:
  case Opcode::Mov:
  case Opcode::Load:
  case Opcode::Store:
  case Opcode::VLoad:
  case Opcode::VStore:
    return true;
  default:
    return false;
  }
}

bool loopBodyIsBlockingFriendly(const NaturalLoop& loop) {
  for (auto* bb : loop.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      auto* inst = instPtr.get();
      if (!inst || inst->isTerminator())
        continue;
      if (!isScalarLike(inst->opcode()))
        return false;
      if (inst->opcode() == Opcode::Call || inst->opcode() == Opcode::Alloca)
        return false;
    }
  }
  return true;
}

int64_t memberHotnessScore(const LoopBandMemberPlan& member) {
  return member.affineStrideScore * 2 +
         static_cast<int64_t>(member.indexedAffineMemoryOps) * 40 +
         static_cast<int64_t>(member.rootReuseScore) * 20 +
         static_cast<int64_t>(member.affineMemoryOps) * 16 +
         static_cast<int64_t>(member.memoryOps) * 8 +
         static_cast<int64_t>(member.loopDepth) * 32 -
         static_cast<int64_t>(member.blockCount) * 4;
}

int chooseTileFactor(const LoopBandMemberPlan& member) {
  if (member.indexedAffineMemoryOps < 2)
    return 1;
  if (member.blockCount > 16)
    return 1;

  int64_t localityScore =
      member.affineStrideScore * 2 +
      static_cast<int64_t>(member.indexedAffineMemoryOps) * 40 +
      static_cast<int64_t>(member.rootReuseScore) * 24 +
      static_cast<int64_t>(member.affineMemoryOps) * 16 +
      static_cast<int64_t>(member.memoryOps) * 8 +
      static_cast<int64_t>(member.loopDepth) * 32;
  // 只让真正由循环索引驱动的 affine 访存参与 tile 因子选择。
  // 单纯标量槽位或单数组顺扫不该把 blocking 因子抬高。
  if (member.affineStrideScore < 4 && member.indexedAffineMemoryOps < 3 &&
      member.rootReuseScore < 2)
    return 1;
  if (member.blockCount > 8 && localityScore < 192)
    return 1;
  if (member.blockCount > 4 && localityScore < 128)
    return 1;
  if (member.distinctIndexedRoots <= 1 && member.stridedMemoryOps == 0 &&
      member.dynamicStrideMemoryOps == 0)
    return 1;
  if (localityScore >= 960 && member.blockCount <= 4 &&
      member.indexedAffineMemoryOps >= 4)
    return 64;
  if (localityScore >= 640 && member.blockCount <= 4)
    return 32;
  if (localityScore >= 320)
    return 16;
  if (localityScore >= 128)
    return 8;
  if (localityScore >= 48)
    return 4;
  return 2;
}

bool hasDynamicScaleForBasis(const AffineAccessSummary& access, Value* basis) {
  if (!access.valid || !access.offset.valid || !basis)
    return false;
  for (const auto& term : access.offset.scaledIvTerms) {
    if (term.basis == basis)
      return true;
  }
  return false;
}

bool accessUsesLoopIndex(const AffineAccessSummary& access) {
  if (!access.valid || !access.offset.valid)
    return false;
  return !access.offset.ivTerms.empty() || !access.offset.scaledIvTerms.empty();
}

void recordAccessLocality(LoopBandMemberPlan& member,
                          const AffineAccessSummary& access) {
  // 这里提取通用 locality 特征，类似 Polly/LoopAccessAnalysis 为收益模型提供
  // stride 分布；它只用于排序和 tile 选择，不参与等价关系判断。
  bool hasUnitStride = false;
  bool hasNonUnitStride = false;
  bool hasDynamicStride = false;
  int64_t maxCostForAccess = 0;

  for (const auto& iv : member.ivs) {
    Value* basis = iv.phi ? static_cast<Value*>(iv.phi)
                          : static_cast<Value*>(iv.storage);
    if (!basis)
      continue;
    int64_t cost = affineStrideCostForProfit(access, basis);
    if (cost <= 0)
      continue;
    maxCostForAccess = std::max(maxCostForAccess, cost);
    if (cost == 1)
      hasUnitStride = true;
    else
      hasNonUnitStride = true;
    if (hasDynamicScaleForBasis(access, basis))
      hasDynamicStride = true;
  }

  member.maxStrideCost = std::max(member.maxStrideCost, maxCostForAccess);
  if (hasUnitStride)
    ++member.unitStrideMemoryOps;
  if (hasNonUnitStride)
    ++member.stridedMemoryOps;
  if (hasDynamicStride)
    ++member.dynamicStrideMemoryOps;
}

void finalizeBandProfit(LoopBandPlan& plan) {
  plan.totalMemoryOps = 0;
  plan.totalAffineMemoryOps = 0;
  plan.totalIndexedAffineMemoryOps = 0;
  plan.totalUnitStrideMemoryOps = 0;
  plan.totalStridedMemoryOps = 0;
  plan.totalDynamicStrideMemoryOps = 0;
  plan.maxDistinctIndexedRoots = 0;
  plan.totalRootReuseScore = 0;
  plan.totalStrideScore = 0;
  plan.maxStrideCost = 0;
  plan.estimatedWorkingSetDrop = 0;
  plan.estimatedRegisterPressure = 0;
  plan.estimatedScalarRegisterPressure = 0;
  plan.bandProfitScore = 0;

  int maxFactor = 1;
  int deepest = 0;
  for (const auto& member : plan.members) {
    plan.totalMemoryOps += member.memoryOps;
    plan.totalAffineMemoryOps += member.affineMemoryOps;
    plan.totalIndexedAffineMemoryOps += member.indexedAffineMemoryOps;
    plan.totalUnitStrideMemoryOps += member.unitStrideMemoryOps;
    plan.totalStridedMemoryOps += member.stridedMemoryOps;
    plan.totalDynamicStrideMemoryOps += member.dynamicStrideMemoryOps;
    plan.maxDistinctIndexedRoots =
        std::max(plan.maxDistinctIndexedRoots, member.distinctIndexedRoots);
    plan.totalRootReuseScore += member.rootReuseScore;
    plan.totalStrideScore += member.affineStrideScore;
    plan.maxStrideCost = std::max(plan.maxStrideCost, member.maxStrideCost);
    maxFactor = std::max(maxFactor, member.suggestedTileFactor);
    deepest = std::max(deepest, member.loopDepth);
    // 寄存器压力估计只看通用结构成本。
    plan.estimatedRegisterPressure += member.affineMemoryOps + member.blockCount / 2;
    // tile-local scalar replacement 更关心热内层同时活跃的
    // 地址流/索引族，而不是整条 band 的 memory-op 数量；这里单独记录一份
    // 更贴近局部标量化的压力画像，避免把长流式核仅因为访存条目多就整体判死。
    int scalarPressure = 2;
    scalarPressure += std::min(member.distinctIndexedRoots * 2, 8);
    scalarPressure += std::min(member.indexedAffineMemoryOps, 4);
    scalarPressure += std::min(member.dynamicStrideMemoryOps * 2, 4);
    if (member.loopDepth > 1)
      scalarPressure += 1;
    if (member.blockCount > 8)
      scalarPressure += 1;
    plan.estimatedScalarRegisterPressure =
        std::max(plan.estimatedScalarRegisterPressure, scalarPressure);
  }

  if (plan.members.size() > 1)
    plan.estimatedScalarRegisterPressure +=
        std::min<int>(6, (static_cast<int>(plan.members.size()) - 1) * 2);
  if (plan.totalDynamicStrideMemoryOps > 0)
    plan.estimatedScalarRegisterPressure +=
        std::min<int>(6, plan.totalDynamicStrideMemoryOps * 2);

  if (maxFactor > 1) {
    plan.estimatedWorkingSetDrop =
        static_cast<int64_t>(plan.totalAffineMemoryOps) *
        static_cast<int64_t>(maxFactor);
  }
  int effectiveBandPressure = plan.estimatedRegisterPressure;
  if (!plan.preferPermutation && plan.totalDynamicStrideMemoryOps == 0 &&
      plan.maxDistinctIndexedRoots <= 2) {
    // 对简单流式 band，band-profit 更该反映“并发地址流”的真实压力，
    // 而不是把整条 band 的 memory-op 数量线性叠进惩罚。这里仅在不需要
    // permutation 且无 dynamic stride 时，温和地把收益排序扣分改成更贴近
    // scalar-replacement 的压力画像；blocking 的硬门槛仍继续看保守压力。
    effectiveBandPressure =
        std::min(effectiveBandPressure,
                 plan.estimatedScalarRegisterPressure +
                     std::min<int>(4, static_cast<int>(plan.members.size())));
  }

  plan.bandProfitScore =
      plan.totalStrideScore * 3 +
      static_cast<int64_t>(plan.totalIndexedAffineMemoryOps) * 48 +
      static_cast<int64_t>(plan.totalRootReuseScore) * 32 +
      static_cast<int64_t>(plan.totalStridedMemoryOps) * 36 +
      static_cast<int64_t>(plan.totalDynamicStrideMemoryOps) * 64 +
      static_cast<int64_t>(plan.totalAffineMemoryOps) * 24 +
      static_cast<int64_t>(plan.totalMemoryOps) * 12 +
      plan.estimatedWorkingSetDrop * 2 +
      static_cast<int64_t>(deepest) * 32 -
      static_cast<int64_t>(effectiveBandPressure) * 10;

  // 这里给 Phase 2 loop-memory pass 提供共享摘要。摘要只来自循环结构、副作用
  // 和 affine/profile 信息；具体变换继续检查
  // CFG、依赖和边界。
  plan.legality.sideEffectFree = true;
  plan.legality.hasMultiLevelBand = plan.members.size() >= 2;
  const bool hasIndexedReuse =
      plan.totalIndexedAffineMemoryOps >= 3 &&
      (plan.totalRootReuseScore >= 2 || plan.maxDistinctIndexedRoots >= 2);
  const bool hasStrongStridePain =
      plan.totalDynamicStrideMemoryOps > 0 ||
      plan.totalStridedMemoryOps >= 2 || plan.maxStrideCost >= 64;
  plan.legality.supportsBlocking =
      plan.preferredTileFactor > 1 && plan.totalIndexedAffineMemoryOps >= 2 &&
      (hasIndexedReuse || hasStrongStridePain);
  plan.legality.supportsPermutation =
      plan.legality.hasMultiLevelBand && plan.preferPermutation &&
      plan.totalIndexedAffineMemoryOps >= 2;
  plan.legality.supportsScalarExpansion =
      plan.totalIndexedAffineMemoryOps >= 3 &&
      plan.estimatedScalarRegisterPressure <= 16;
  plan.legality.needsVersioning =
      plan.totalDynamicStrideMemoryOps > plan.totalUnitStrideMemoryOps;
  plan.legality.interiorRemainderReady =
      plan.legality.supportsBlocking && !plan.legality.needsVersioning;
}

LoopBandMemberPlan analyzeMember(Function* func,
                                 FunctionAnalysisCache& cache,
                                 const LoopTreeNode& node) {
  LoopBandMemberPlan member;
  member.loop = node.loop;
  member.node = &node;
  if (!func || !node.loop)
    return member;

  member.loopDepth = node.depth + 1;
  member.blockCount = static_cast<int>(node.loop->blocks.size());
  member.ivs = cache.inductionVars(*node.loop);
  std::unordered_set<int> seenMemIds;
  std::unordered_map<int, int> indexedRootFreq;
  for (auto* bb : node.loop->blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      auto* inst = instPtr.get();
      if (!isMemoryOp(inst) || !seenMemIds.insert(inst->id()).second)
        continue;
      ++member.memoryOps;
      Value* addr = memoryAddress(inst);
      if (!addr || member.ivs.empty())
        continue;
      auto access = summarizeAffineAccess(addr, *node.loop, member.ivs);
      if (!access.valid || !access.offset.valid)
        continue;
      ++member.affineMemoryOps;
      if (accessUsesLoopIndex(access)) {
        ++member.indexedAffineMemoryOps;
        if (access.root)
          ++indexedRootFreq[access.root->id()];
      }
      recordAccessLocality(member, access);
      for (const auto& iv : member.ivs) {
        Value* basis = iv.phi ? static_cast<Value*>(iv.phi)
                              : static_cast<Value*>(iv.storage);
        if (!basis)
          continue;
        // 现阶段分块/交换仍使用旧的 affine 系数权重，
        // 新的 stride cost 只记录在 locality/profile 字段中。这样可以
        // 先维持已有性能，再逐步把更强的收益模型接入统一决策流程。
        int64_t weight = affineWeightedCoeffFor(access, basis);
        if (weight < 0)
          weight = -weight;
        member.affineStrideScore += weight;
      }
    }
  }
  member.distinctIndexedRoots = static_cast<int>(indexedRootFreq.size());
  for (const auto& [_, freq] : indexedRootFreq) {
    if (freq >= 2)
      member.rootReuseScore += freq - 1;
  }
  member.hotnessScore = memberHotnessScore(member);
  member.suggestedTileFactor = chooseTileFactor(member);
  return member;
}

} // namespace

// 以循环森林的每个根循环为起点，沿“适合分块且含访存”的最热子循环
// 逐级向下延伸，构造多层循环带（band）计划，供后续分块变换使用。
std::vector<LoopBandPlan> buildLoopBandPlans(Function* func,
                                             FunctionAnalysisCache& cache) {
  std::vector<LoopBandPlan> plans;
  const auto& forest = cache.loopForest();
  const auto& nodes = forest.nodes();

  for (int idx = 0; idx < static_cast<int>(nodes.size()); ++idx) {
    const auto* root = forest.node(idx);
    if (!root || !root->loop)
      continue;
    if (cache.hasUnsafeRuntimeScalarGlobalBound(*root->loop))
      continue;

    LoopBandPlan plan;
    plan.rootNode = idx;
    int cur = idx;
    while (cur >= 0) {
      const auto* node = forest.node(cur);
      if (!node || !node->loop)
        break;
      if (cache.hasUnsafeRuntimeScalarGlobalBound(*node->loop))
        break;
      if (!loopBodyIsBlockingFriendly(*node->loop))
        break;
      auto member = analyzeMember(func, cache, *node);
      if (member.memoryOps == 0)
        break;
      plan.nodeIndices.push_back(cur);
      plan.members.push_back(std::move(member));
      if (node->children.empty())
        break;
      // 多个子循环时只沿热度最高且适合分块的一支继续向下延伸。
      int bestChild = -1;
      int64_t bestChildScore = std::numeric_limits<int64_t>::min();
      for (int childIdx : node->children) {
        const auto* child = forest.node(childIdx);
        if (!child || !child->loop)
          continue;
        if (cache.hasUnsafeRuntimeScalarGlobalBound(*child->loop))
          continue;
        if (!loopBodyIsBlockingFriendly(*child->loop))
          continue;
        auto childMember = analyzeMember(func, cache, *child);
        if (childMember.memoryOps == 0)
          continue;
        int64_t score = childMember.hotnessScore;
        if (score > bestChildScore) {
          bestChildScore = score;
          bestChild = childIdx;
        }
      }
      if (bestChild < 0)
        break;
      cur = bestChild;
    }

    if (plan.members.empty())
      continue;

    int bestMember = -1;
    int bestFactor = 1;
    int64_t bestScore = std::numeric_limits<int64_t>::min();
    for (int i = 0; i < static_cast<int>(plan.members.size()); ++i) {
      const auto& member = plan.members[i];
      int factor = member.suggestedTileFactor;
      int64_t score = memberHotnessScore(member) +
                      static_cast<int64_t>(member.indexedAffineMemoryOps) * 12 +
                      static_cast<int64_t>(member.rootReuseScore) * 16 +
                      static_cast<int64_t>(member.memoryOps) * 2 -
                      static_cast<int64_t>(member.blockCount) * 4 +
                      static_cast<int64_t>(factor) * 6;
      if (factor <= 1)
        score = std::numeric_limits<int64_t>::min();
      if (score > bestScore) {
        bestScore = score;
        bestMember = i;
        bestFactor = factor;
      }
    }

    if (bestMember < 0 || bestFactor <= 1)
      continue;

    plan.valid = true;
    plan.preferredTileMember = bestMember;
    plan.preferredTileFactor = bestFactor;
    if (plan.members.size() > 1) {
      const auto& outer = plan.members.front();
      const auto& inner = plan.members.back();
      // stride 分布目前只作为诊断信息保留。实际交换仍沿用
      // 原先的 affine stride 分数，避免 dynamic-stride 统计在未完成
      // dependence/profit 闭环前把分块顺序带偏。
      plan.preferPermutation = inner.affineStrideScore > outer.affineStrideScore;
    }
    finalizeBandProfit(plan);
    plans.push_back(std::move(plan));
  }

  std::sort(plans.begin(), plans.end(),
            [](const LoopBandPlan& lhs, const LoopBandPlan& rhs) {
              if (lhs.members.size() != rhs.members.size())
                return lhs.members.size() > rhs.members.size();
              if (lhs.preferredTileFactor != rhs.preferredTileFactor)
                return lhs.preferredTileFactor > rhs.preferredTileFactor;
              int lhsDepth = 0;
              int rhsDepth = 0;
              if (lhs.preferredTileMember >= 0 &&
                  lhs.preferredTileMember < static_cast<int>(lhs.members.size())) {
                lhsDepth = lhs.members[static_cast<size_t>(lhs.preferredTileMember)].loopDepth;
              }
              if (rhs.preferredTileMember >= 0 &&
                  rhs.preferredTileMember < static_cast<int>(rhs.members.size())) {
                rhsDepth = rhs.members[static_cast<size_t>(rhs.preferredTileMember)].loopDepth;
              }
              if (lhsDepth != rhsDepth)
                return lhsDepth > rhsDepth;
              return lhs.rootNode < rhs.rootNode;
            });
  return plans;
}

} // namespace analysis
