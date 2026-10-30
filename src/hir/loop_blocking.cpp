#include "research_switch.h"
// 文件职责：执行保守循环分块，改善 affine 访问的工作集局部性。本文件负责实现具体逻辑。
#include "loop_blocking.h"
#include "canonical_loop.h"

#include "../analysis/analysis_manager.h"
#include "../analysis/affine_region.h"
#include "../analysis/cfg_utils.h"
#include "../analysis/dominators.h"
#include "../analysis/function_analysis.h"
#include "../analysis/loop_band.h"
#include "../analysis/loop_info.h"
#include "affine_loop_locality_plan.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hir {

namespace {

using analysis::LoopBandPlan;
using analysis::LoopBandMemberPlan;
using analysis::NaturalLoop;
using core_ir::BasicBlock;
using core_ir::ConstInt;
using core_ir::Function;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Type;
using core_ir::Value;

struct BlockingRegionHints {
  bool available = false;
  int estimatedRegisterPressure = 0;
  int outsideCondBranchCount = 0;
  int outsidePhiCount = 0;
  int64_t estimatedWorkingSetDrop = 0;
  int64_t postSplitMemoryDeltaScore = 0;
  bool postSplitProfitable = false;
  bool postSplitMetricsImproved = false;
  bool multiLevelBlockingReady = false;
  bool tileLocalScalarReady = false;
};

bool loopBlockingTraceEnabled() {
  return sycEnvironmentValue("LOOP_BLOCKING_TRACE") != nullptr;
}

void traceBlockingDecision(const char* stage, Function* func,
                           const LoopBandPlan& plan,
                           const char* reason,
                           int64_t score = 0) {
  if (!loopBlockingTraceEnabled())
    return;
  std::fprintf(stderr,
               "[loop-blocking] %s func=%s root=%d members=%zu factor=%d "
               "mem=%d affine=%d indexed=%d dynamic=%d score=%lld reason=%s\n",
               stage, func ? func->name().c_str() : "<null>", plan.rootNode,
               plan.members.size(), plan.preferredTileFactor,
               plan.totalMemoryOps, plan.totalAffineMemoryOps,
               plan.totalIndexedAffineMemoryOps,
               plan.totalDynamicStrideMemoryOps,
               static_cast<long long>(score), reason ? reason : "-");
}

void traceMultiLevelGate(Function* func, const LoopBandPlan& plan,
                         const analysis::LoopMemoryProfitSummary& profit,
                         const analysis::AffineRegionPlan& region,
                         bool versionedFastPathReady,
                         bool versionFreeFastPathReady,
                         bool denseVersionFreeMultiReady,
                         int64_t previousNetScore) {
  if (!loopBlockingTraceEnabled())
    return;
  std::fprintf(stderr,
               "[loop-blocking] gate func=%s root=%d members=%zu factor=%d "
               "versioned=%d versionfree=%d densefree=%d interior=%d remainder=%d "
               "metrics=%d profitable=%d cfg=%lld mem=%lld wsdrop=%lld reg=%d "
               "net=%lld dyn=%d bodycond=%d\n",
               func ? func->name().c_str() : "<null>", plan.rootNode,
               plan.members.size(), plan.preferredTileFactor,
               versionedFastPathReady ? 1 : 0,
               versionFreeFastPathReady ? 1 : 0,
               denseVersionFreeMultiReady ? 1 : 0,
               profit.interiorFastPathReady ? 1 : 0,
               profit.remainderPathReady ? 1 : 0,
               profit.postSplitMetricsImproved ? 1 : 0,
               profit.postSplitProfitable ? 1 : 0,
               static_cast<long long>(profit.postSplitCfgDeltaScore),
               static_cast<long long>(profit.postSplitMemoryDeltaScore),
               static_cast<long long>(profit.estimatedWorkingSetDrop),
               profit.estimatedRegisterPressure,
               static_cast<long long>(previousNetScore),
               plan.totalDynamicStrideMemoryOps,
               profit.hotLoopBodyCondBranchCount);
}

BlockingRegionHints collectBlockingRegionHints(
    const analysis::AffineRegionPlan* region) {
  BlockingRegionHints hints;
  if (!region)
    return hints;
  hints.available = true;
  hints.estimatedRegisterPressure = region->profit.estimatedRegisterPressure;
  hints.outsideCondBranchCount = region->versioning.outsideCondBranchCount;
  hints.outsidePhiCount = region->versioning.outsidePhiCount;
  hints.estimatedWorkingSetDrop = region->profit.estimatedWorkingSetDrop;
  hints.postSplitMemoryDeltaScore = region->profit.postSplitMemoryDeltaScore;
  hints.postSplitProfitable = region->profit.postSplitProfitable;
  hints.postSplitMetricsImproved = region->profit.postSplitMetricsImproved;
  hints.multiLevelBlockingReady = region->profit.multiLevelBlockingReady;
  hints.tileLocalScalarReady = region->profit.tileLocalScalarReady;
  return hints;
}

int64_t blockingPlanPriority(Function* func,
                             analysis::FunctionAnalysisCache& cache,
                             const LoopBandPlan& plan) {
  auto cand = buildAffineLoopLocalityCandidate(func, plan, &cache);
  if (!plan.legality.supportsBlocking || !cand.supportsBlocking)
    return std::numeric_limits<int64_t>::min();
  if (cand.regionNetProfitScore < 64 &&
      cand.profitScore < 128 &&
      cand.estimatedWorkingSetDrop < 64) {
    // blocking 执行端再补一道共享净收益门槛，防止线性 sweep
    // 仅凭“可分块”就反复进入 strip-mining fixpoint。
    return std::numeric_limits<int64_t>::min();
  }
  if (cand.primaryAction == AffineLoopLocalityAction::VersionedBlocking &&
      cand.needsVersioning) {
    // 当前 blocking 只能落普通 strip-mining；即使共享候选更偏向
    // VersionedBlocking，也要在这里温和降权，避免把尚未完全兑现的动作
    // 提到纯 blocking 路径最前面。
    return affineLoopLocalityCandidatePriority(cand) - 160;
  }
  // blocking 计划排序默认直接复用共享 locality candidate
  // priority，仅保留极少量 pass 私有修正，避免重新拼一整套 region 权重。
  int64_t priority = affineLoopLocalityCandidatePriority(cand);
  if (plan.legality.hasMultiLevelBand && !cand.multiLevelBlockingReady)
    priority -= 192;
  if (plan.legality.needsVersioning && !cand.postSplitProfitable)
    priority -= 256;
  return priority;
}

int64_t memberLocalityPriority(const LoopBandMemberPlan& member) {
  return member.hotnessScore * 4 +
         static_cast<int64_t>(member.suggestedTileFactor) * 64 +
         static_cast<int64_t>(member.stridedMemoryOps) * 32 +
         static_cast<int64_t>(member.dynamicStrideMemoryOps) * 16 -
         static_cast<int64_t>(member.blockCount) * 8;
}

int cappedTileFactorForRegion(int preferred,
                              const BlockingRegionHints& hints,
                              int64_t netScoreHint) {
  if (preferred <= 1 || !hints.available)
    return preferred;
  int capped = preferred;
  if (netScoreHint < 128)
    capped = std::min(capped, 4);
  else if (netScoreHint < 384)
    capped = std::min(capped, 8);
  else if (netScoreHint < 1280)
    // strip-mining 只增加固定的块边界，不会按因子复制循环体。32 次迭代
    // 能把每块约 8 至 10 条边界和地址准备指令摊薄到每迭代约三分之一条，
    // 同时保留比 64 更小的局部工作集；只有更高净收益才继续采用 64。
    capped = std::min(capped, 32);
  // 寄存器压力由标量替换、多层分块和展开各自的入口控制；单层
  // strip-mining 的因子不会增加标量状态数量，因此这里不重复压低因子。
  if (hints.outsideCondBranchCount > 0 && netScoreHint < 1024)
    capped = std::min(capped, 8);
  return capped;
}

int64_t memberBlockingPriority(const LoopBandPlan& plan,
                               const LoopBandMemberPlan& member, int memberIdx,
                               const BlockingRegionHints& hints,
                               int64_t netScoreHint) {
  int preferredFactor =
      cappedTileFactorForRegion(member.suggestedTileFactor, hints, netScoreHint);
  int64_t score = member.hotnessScore * 4 +
                  static_cast<int64_t>(preferredFactor) * 96 +
                  static_cast<int64_t>(member.affineMemoryOps) * 32 +
                  static_cast<int64_t>(member.stridedMemoryOps) * 24 -
                  static_cast<int64_t>(member.dynamicStrideMemoryOps) * 48 -
                  static_cast<int64_t>(member.blockCount) * 8;
  if (plan.preferredTileMember == memberIdx)
    score += 192;
  if (!hints.available)
    return score;
  score += netScoreHint / 8;
  score += hints.postSplitMemoryDeltaScore / 4;
  score += hints.estimatedWorkingSetDrop / 8;
  if (hints.outsideCondBranchCount > 0 && member.loopDepth > 2)
    score -= 96;
  return score;
}

int64_t factorBlockingPriority(const LoopBandMemberPlan& member,
                               int preferredFactor, int factor,
                               const BlockingRegionHints& hints,
                               int64_t netScoreHint) {
  if (factor <= 1)
    return std::numeric_limits<int64_t>::min();
  int distance =
      preferredFactor > factor ? preferredFactor - factor : factor - preferredFactor;
  int64_t score = static_cast<int64_t>(factor) * 128 -
                  static_cast<int64_t>(distance) * 48 +
                  member.hotnessScore * 2 +
                  static_cast<int64_t>(member.affineMemoryOps) * 32 +
                  static_cast<int64_t>(member.stridedMemoryOps) * 24 -
                  static_cast<int64_t>(member.dynamicStrideMemoryOps) * 64;
  if (!hints.available)
    return score;

  // factor 选择不再机械地“大块优先”，而是把 split 后热路径
  // 是否真的更干净、CFG 外围成本是否可接受、以及 tile 是否更利于标量替换
  // 一并纳入排序。这样 blocking 和 scalar replacement 才共享一套净收益依据。
  if (hints.postSplitProfitable)
    score += static_cast<int64_t>(factor) * 48;
  if (hints.postSplitMetricsImproved)
    score += static_cast<int64_t>(factor) * 32;
  if (hints.multiLevelBlockingReady)
    score += static_cast<int64_t>(factor) * 24;
  if (hints.tileLocalScalarReady) {
    if (factor <= 8)
      score += 192;
    else
      score -= static_cast<int64_t>(factor - 8) * 24;
  }
  score += hints.estimatedWorkingSetDrop / 16;
  score += hints.postSplitMemoryDeltaScore / 2;
  score += netScoreHint / 8;
  if (netScoreHint < 512)
    score -= static_cast<int64_t>(factor) * 96;
  else if (netScoreHint < 1024)
    score -= static_cast<int64_t>(factor) * 32;
  score -= static_cast<int64_t>(factor) *
           std::max(0, hints.estimatedRegisterPressure - 8) * 12;
  score -= static_cast<int64_t>(factor) *
           hints.outsideCondBranchCount * 64;
  score -= static_cast<int64_t>(factor) * hints.outsidePhiCount * 12;
  return score;
}

std::vector<int> tileFactorCandidates(int preferred,
                                      const LoopBandMemberPlan& member,
                                      const BlockingRegionHints& hints,
                                      int64_t netScoreHint) {
  std::vector<int> factors;
  // 分块因子采用通用 power-of-two 回退序列。收益模型先给 preferred，若
  // CFG 或边界条件不满足，则尝试更小 tile，避免因一个过激因子放弃整个 band。
  for (int factor = preferred; factor >= 2; factor /= 2) {
    if (factors.empty() || factors.back() != factor)
      factors.push_back(factor);
  }
  std::stable_sort(factors.begin(), factors.end(),
                   [&](int lhs, int rhs) {
                     int64_t lhsScore = factorBlockingPriority(
                         member, preferred, lhs, hints, netScoreHint);
                     int64_t rhsScore = factorBlockingPriority(
                         member, preferred, rhs, hints, netScoreHint);
                     if (lhsScore != rhsScore)
                       return lhsScore > rhsScore;
                     return lhs > rhs;
                   });
  return factors;
}

bool loopCompatibleForStripMine(const NaturalLoop& loop, const CanonicalLoop& cl) {
  for (auto* bb : loop.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      auto* inst = instPtr.get();
      if (!inst || inst->isTerminator())
        continue;
      if (inst->opcode() == Opcode::Call || inst->opcode() == Opcode::Alloca)
        return false;
    }

    Instruction* term = bb->terminator();
    if (!term)
      return false;
    if (bb == cl.header) {
      if (term->opcode() != Opcode::CondBr)
        return false;
      continue;
    }
    if (bb == cl.latch) {
      if (term->opcode() != Opcode::Br ||
          dynamic_cast<BasicBlock*>(term->operand(0)) != cl.header) {
        return false;
      }
      continue;
    }

    if (term->opcode() != Opcode::Br && term->opcode() != Opcode::CondBr)
      return false;
    for (auto* succ : bb->succs()) {
      if (!succ || !loop.blocks.count(succ))
        return false;
    }
  }
  return true;
}

bool valueDefinedInsideLoop(Value* value, const NaturalLoop& loop) {
  auto* inst = dynamic_cast<Instruction*>(value);
  return inst && inst->parent() && loop.blocks.count(inst->parent());
}

bool loopHasFloatingArithmetic(const NaturalLoop& loop) {
  for (BasicBlock* block : loop.blocks) {
    if (!block)
      continue;
    for (const auto& instPtr : block->instructions()) {
      if (!instPtr)
        continue;
      switch (instPtr->opcode()) {
      case Opcode::FAdd:
      case Opcode::FSub:
      case Opcode::FMul:
      case Opcode::FDiv:
        return true;
      default:
        break;
      }
    }
  }
  return false;
}

bool blockIsGeneratedTileScaffold(BasicBlock* bb) {
  return bb && bb->name().find("tile.") != std::string::npos;
}

bool valueDefinedInGeneratedTileScaffold(Value* value) {
  auto* inst = dynamic_cast<Instruction*>(value);
  return inst && blockIsGeneratedTileScaffold(inst->parent());
}

bool valueDefinedInEnclosingLoop(Value* value,
                                  const analysis::LoopForest& forest,
                                  int nodeIdx) {
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !inst->parent())
    return false;
  const auto* node = forest.node(nodeIdx);
  for (int parentIdx = node ? node->parent : -1; parentIdx >= 0;) {
    const auto* parent = forest.node(parentIdx);
    if (!parent)
      break;
    if (parent->loop && parent->loop->blocks.count(inst->parent()))
      return true;
    parentIdx = parent->parent;
  }
  return false;
}

bool valueTransitivelyUses(Value* value, Value* needle,
                           std::unordered_set<int>& active) {
  if (!value || !needle)
    return false;
  if (value == needle)
    return true;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst)
    return false;
  if (!active.insert(inst->id()).second)
    return false;
  for (int i = 0; i < inst->numOperands(); ++i) {
    if (valueTransitivelyUses(inst->operand(i), needle, active)) {
      active.erase(inst->id());
      return true;
    }
  }
  active.erase(inst->id());
  return false;
}

bool valueTransitivelyUses(Value* value, Value* needle) {
  std::unordered_set<int> active;
  return valueTransitivelyUses(value, needle, active);
}

bool nestedLoopLimitDependsOnCurrentIv(const analysis::LoopForest& forest,
                                       int nodeIdx,
                                       const CanonicalLoop& cl) {
  const auto* node = forest.node(nodeIdx);
  if (!node || !node->loop || !cl.ivPhi)
    return false;
  for (int childIdx : node->children) {
    const auto* child = forest.node(childIdx);
    if (!child || !child->loop)
      continue;
    BasicBlock* childPreheader =
        child->preheader ? child->preheader
                         : analysis::dedicatedPreheader(*child->loop);
    if (!childPreheader)
      continue;
    CanonicalLoop childCl;
    if (!recognizeCanonicalLoop(*child->loop, childPreheader, childCl))
      continue;
    if (valueDefinedInGeneratedTileScaffold(childCl.boundVal) ||
        valueDefinedInGeneratedTileScaffold(childCl.initVal))
      continue;
    if (valueTransitivelyUses(childCl.boundVal, cl.ivPhi) ||
        valueTransitivelyUses(childCl.initVal, cl.ivPhi))
      return true;
  }
  return false;
}

bool hasNonCanonicalDirectChildLoop(const analysis::LoopForest& forest,
                                    int nodeIdx) {
  const auto* node = forest.node(nodeIdx);
  if (!node || !node->loop)
    return false;
  for (int childIdx : node->children) {
    const auto* child = forest.node(childIdx);
    if (!child || !child->loop)
      return true;
    BasicBlock* childPreheader =
        child->preheader ? child->preheader
                         : analysis::dedicatedPreheader(*child->loop);
    CanonicalLoop childCl;
    if (!childPreheader ||
        !recognizeCanonicalLoop(*child->loop, childPreheader, childCl)) {
      return true;
    }
  }
  return false;
}

bool addressRootIsLocalArray(Value* value,
                             std::unordered_set<int>& active) {
  if (!value)
    return false;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst)
    return false;
  if (inst->opcode() == Opcode::Alloca)
    return inst->isArrayObject() && inst->memorySizeBytes() >= 64;
  if (!active.insert(inst->id()).second)
    return false;
  bool result = false;
  if (inst->opcode() == Opcode::GEP && inst->numOperands() >= 1) {
    result = addressRootIsLocalArray(inst->operand(0), active);
  } else if (inst->opcode() == Opcode::Phi) {
    for (int i = 0; i + 1 < inst->numOperands(); i += 2) {
      if (addressRootIsLocalArray(inst->operand(i), active)) {
        result = true;
        break;
      }
    }
  }
  active.erase(inst->id());
  return result;
}

bool loopTouchesLocalArrayBuffer(const NaturalLoop& loop) {
  for (auto* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      Value* addr = nullptr;
      if ((inst->opcode() == Opcode::Load || inst->opcode() == Opcode::VLoad) &&
          inst->numOperands() >= 1) {
        addr = inst->operand(0);
      } else if ((inst->opcode() == Opcode::Store ||
                  inst->opcode() == Opcode::VStore) &&
                 inst->numOperands() >= 2) {
        addr = inst->operand(1);
      }
      if (!addr)
        continue;
      std::unordered_set<int> active;
      if (addressRootIsLocalArray(addr, active))
        return true;
    }
  }
  return false;
}

bool exitPhiCompatible(const NaturalLoop& loop,
                       const CanonicalLoop& cl,
                       const std::vector<Instruction*>& headerPhis) {
  std::unordered_set<int> headerPhiIds;
  for (auto* phi : headerPhis)
    headerPhiIds.insert(phi->id());

  for (auto* phi : analysis::collectLeadingPhi(cl.exit)) {
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
      if (pred != cl.header)
        continue;
      Value* incoming = phi->operand(i);
      if (auto* inst = dynamic_cast<Instruction*>(incoming)) {
        if (headerPhiIds.count(inst->id()))
          continue;
      }
      if (valueDefinedInsideLoop(incoming, loop))
        return false;
    }
  }
  return true;
}

bool outsideUsesCompatible(const NaturalLoop& loop,
                           const std::vector<Instruction*>& headerPhis,
                           const char** rejectReason = nullptr) {
  auto reject = [&](const char* reason) {
    if (rejectReason)
      *rejectReason = reason;
    return false;
  };

  // 计算出口块：循环内基本块指向循环外的后继。
  std::unordered_set<BasicBlock*> exitBlocks;
  for (auto* bb : loop.blocks) {
    if (!bb) continue;
    for (auto* succ : bb->succs()) {
      if (succ && !loop.blocks.count(succ))
        exitBlocks.insert(succ);
    }
  }

  for (auto* phi : headerPhis) {
    std::vector<Instruction*> users = phi->uses();
    for (auto* user : users) {
      if (!user)
        continue;
      BasicBlock* parent = user->parent();
      if (parent && loop.blocks.count(parent))
        continue;
      for (int i = 0; i < user->numOperands(); ++i) {
        if (user->operand(i) != phi)
          continue;
        if (user->opcode() == Opcode::Phi) {
          if (i + 1 >= user->numOperands())
            return reject("outside-phi-missing-pred");
          auto* pred = dynamic_cast<BasicBlock*>(user->operand(i + 1));
          // 外部 phi 的 incoming 边可以来自 header/latch 等任意
          // loop 内退出块。真正重写时会把这些边统一改到 outerHeader；
          // 只有来自 loop 外的 incoming 才不能安全替换成 outer phi。
          // 放宽条件：如果 incoming pred 是 loop 的 exit block，则允许，
          // 因为 strip-mine 后 exit 会被重定向到 outerHeader。
          if (!pred)
            return reject("outside-phi-null-pred");
          if (!loop.blocks.count(pred)) {
            // 检查 pred 是否为循环出口块，是时允许该路径，因为 strip-mine 会把
            // 出口边重定向到 outerHeader。
            if (!exitBlocks.count(pred))
              return reject("outside-phi-outer-pred");
          }
        }
      }
    }
  }
  return true;
}

int headerPhiIndex(const std::vector<Instruction*>& headerPhis, Value* value) {
  for (size_t i = 0; i < headerPhis.size(); ++i) {
    if (headerPhis[i] == value)
      return static_cast<int>(i);
  }
  return -1;
}

bool rewriteLoopExitUses(const NaturalLoop& loop,
                         const CanonicalLoop& cl,
                         const std::vector<Instruction*>& headerPhis,
                         const std::vector<Instruction*>& outerPhis,
                         BasicBlock* outerHeader) {
  for (auto* phi : analysis::collectLeadingPhi(cl.exit)) {
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
      if (pred != cl.header)
        continue;
      int idx = headerPhiIndex(headerPhis, phi->operand(i));
      if (idx >= 0)
        phi->setOperand(i, outerPhis[static_cast<size_t>(idx)]);
      phi->setOperand(i + 1, outerHeader);
    }
  }

  for (size_t phiIdx = 0; phiIdx < headerPhis.size(); ++phiIdx) {
    auto* oldPhi = headerPhis[phiIdx];
    auto* newPhi = outerPhis[phiIdx];
    std::vector<Instruction*> users = oldPhi->uses();
    for (auto* user : users) {
      if (!user)
        continue;
      if (user == newPhi)
        continue;
      BasicBlock* parent = user->parent();
      if (parent && loop.blocks.count(parent))
        continue;
      for (int opIdx = 0; opIdx < user->numOperands(); ++opIdx) {
        if (user->operand(opIdx) != oldPhi)
          continue;
        user->setOperand(opIdx, newPhi);
        if (user->opcode() == Opcode::Phi && opIdx + 1 < user->numOperands()) {
          auto* pred = dynamic_cast<BasicBlock*>(user->operand(opIdx + 1));
          if (pred && loop.blocks.count(pred))
            user->setOperand(opIdx + 1, outerHeader);
        }
      }
    }
  }
  return true;
}

bool applyStripMine(Function* func,
                    const NaturalLoop& loop,
                    const CanonicalLoop& cl,
                    int factor,
                    bool allowTileLocalBound = false,
                    const char** rejectReason = nullptr) {
  auto reject = [&](const char* reason) {
    if (rejectReason)
      *rejectReason = reason;
    return false;
  };
  if (factor <= 1)
    return reject("factor-too-small");
  // 普通单层分块仍要求常量起点；multi-level 的内层分块允许
  // tile-local 动态起点，这是标准 strip-mining 的 `outer + factor` 形式。
  if (!dynamic_cast<ConstInt*>(cl.initVal) &&
      (!allowTileLocalBound ||
       !valueDefinedInGeneratedTileScaffold(cl.initVal)))
    return reject("dynamic-init");
  if (auto* initConst = dynamic_cast<ConstInt*>(cl.initVal)) {
    if (auto* boundConst = dynamic_cast<ConstInt*>(cl.boundVal)) {
      int trips = boundConst->value() - initConst->value();
      if (trips <= factor)
        return reject("trip-too-small");
    }
  }
  if (!cl.ivPhi->type().isI32() || !cl.boundVal || !cl.boundVal->type().isI32())
    return reject("non-i32-bound");
  if (auto* boundInst = dynamic_cast<Instruction*>(cl.boundVal)) {
    if (boundInst->opcode() == Opcode::Phi && boundInst->parent() &&
        !loop.blocks.count(boundInst->parent())) {
      // 普通单层 fallback 遇到外部 tile.bound phi 时继续保守拒绝，
      // 避免在下一轮 fixpoint 里重复分块同一个循环。multi-level 执行器已经
      // 用 alreadyAppliedNode 避免重分同一层，因此允许不同成员在 tile-local
      // bound 下继续分块，形成真正的多层 blocking。
      if (!allowTileLocalBound ||
          !valueDefinedInGeneratedTileScaffold(boundInst))
        return reject("tile-bound");
    }
  }
  if (!cl.ivIsCmpLhs || canonicalLoopCmpCond(cl.cmp) != "lt")
    return reject("cmp-shape");
  if (valueDefinedInsideLoop(cl.boundVal, loop))
    return reject("bound-inside-loop");
  if (!loopCompatibleForStripMine(loop, cl))
    return reject("loop-incompatible");

  std::vector<Instruction*> headerPhis;
  std::vector<Value*> initVals;
  if (!collectCanonicalLoopPhis(cl, headerPhis, initVals))
    return reject("phi-shape");
  if (!exitPhiCompatible(loop, cl, headerPhis))
    return reject("exit-phi");
  const char* outsideRejectReason = nullptr;
  if (!outsideUsesCompatible(loop, headerPhis, &outsideRejectReason))
    return reject(outsideRejectReason ? outsideRejectReason : "outside-use");

  int ivIndex = headerPhiIndex(headerPhis, cl.ivPhi);
  if (ivIndex < 0)
    return reject("iv-phi-missing");

  auto* outerHeader = func->newBlock("tile.header");
  auto* tileCheck = func->newBlock("tile.check");
  auto* tileUseBound = func->newBlock("tile.use_bound");
  auto* tileUseCandidate = func->newBlock("tile.use_candidate");
  auto* tileEntry = func->newBlock("tile.entry");
  auto* outerLatch = func->newBlock("tile.latch");

  std::vector<Instruction*> outerPhis;
  outerPhis.reserve(headerPhis.size());
  for (size_t i = 0; i < headerPhis.size(); ++i) {
    auto* outerPhi = new Instruction(Opcode::Phi, headerPhis[i]->type(),
                                     headerPhis[i]->name().empty()
                                         ? "tile.phi"
                                         : headerPhis[i]->name() + ".tile");
    outerPhi->addOperand(initVals[i]);
    outerPhi->addOperand(cl.preheader);
    outerHeader->pushBack(outerPhi);
    outerPhis.push_back(outerPhi);
  }

  auto* tileCandidate = new Instruction(Opcode::Add, Type::i32(), "tile.bound.candidate");
  tileCandidate->addOperand(outerPhis[static_cast<size_t>(ivIndex)]);
  tileCandidate->addOperand(func->parent()->getConstInt(factor));
  outerHeader->pushBack(tileCandidate);

  auto* outerCmp = new Instruction(Opcode::ICmp, Type::i32(), cl.cmp->name());
  outerCmp->addOperand(outerPhis[static_cast<size_t>(ivIndex)]);
  outerCmp->addOperand(cl.boundVal);
  outerHeader->pushBack(outerCmp);

  auto* outerBr = new Instruction(Opcode::CondBr, Type::voidTy());
  outerBr->addOperand(outerCmp);
  outerBr->addOperand(tileCheck);
  outerBr->addOperand(cl.exit);
  outerHeader->pushBack(outerBr);

  auto* tileClampCmp =
      new Instruction(Opcode::ICmp, Type::i32(), cl.cmp->name());
  tileClampCmp->addOperand(tileCandidate);
  tileClampCmp->addOperand(cl.boundVal);
  tileCheck->pushBack(tileClampCmp);
  auto* tileClampBr = new Instruction(Opcode::CondBr, Type::voidTy());
  tileClampBr->addOperand(tileClampCmp);
  tileClampBr->addOperand(tileUseCandidate);
  tileClampBr->addOperand(tileUseBound);
  tileCheck->pushBack(tileClampBr);

  auto* useCandidateBr = new Instruction(Opcode::Br, Type::voidTy());
  useCandidateBr->addOperand(tileEntry);
  tileUseCandidate->pushBack(useCandidateBr);

  auto* useBoundBr = new Instruction(Opcode::Br, Type::voidTy());
  useBoundBr->addOperand(tileEntry);
  tileUseBound->pushBack(useBoundBr);

  auto* tileBound = new Instruction(Opcode::Phi, Type::i32(), "tile.bound");
  tileBound->addOperand(tileCandidate);
  tileBound->addOperand(tileUseCandidate);
  tileBound->addOperand(cl.boundVal);
  tileBound->addOperand(tileUseBound);
  tileEntry->pushBack(tileBound);
  auto* entryBr = new Instruction(Opcode::Br, Type::voidTy());
  entryBr->addOperand(cl.header);
  tileEntry->pushBack(entryBr);

  auto* latchBr = new Instruction(Opcode::Br, Type::voidTy());
  latchBr->addOperand(outerHeader);
  outerLatch->pushBack(latchBr);

  for (size_t i = 0; i < outerPhis.size(); ++i) {
    outerPhis[i]->addOperand(headerPhis[i]);
    outerPhis[i]->addOperand(outerLatch);
  }

  if (!analysis::redirectSuccessor(cl.preheader, cl.header, outerHeader))
    return reject("redirect-failed");

  for (size_t i = 0; i < headerPhis.size(); ++i) {
    auto* phi = headerPhis[i];
    for (int opIdx = 0; opIdx + 1 < phi->numOperands(); opIdx += 2) {
      auto* pred = dynamic_cast<BasicBlock*>(phi->operand(opIdx + 1));
      if (pred != cl.preheader)
        continue;
      phi->setOperand(opIdx, outerPhis[i]);
      phi->setOperand(opIdx + 1, tileEntry);
      break;
    }
  }

  if (cl.ivIsCmpLhs)
    cl.cmp->setOperand(1, tileBound);
  else
    cl.cmp->setOperand(0, tileBound);

  if (dynamic_cast<BasicBlock*>(cl.condBr->operand(1)) == cl.exit)
    cl.condBr->setOperand(1, outerLatch);
  if (dynamic_cast<BasicBlock*>(cl.condBr->operand(2)) == cl.exit)
    cl.condBr->setOperand(2, outerLatch);

  rewriteLoopExitUses(loop, cl, headerPhis, outerPhis, outerHeader);
  func->recomputeCFG();
  return true;
}


bool loopHasBitStateCompute(const NaturalLoop& loop) {
  int computeOps = 0;
  int loadOps = 0;
  int storeOps = 0;
  for (BasicBlock* block : loop.blocks) {
    if (!block)
      continue;
    for (const auto& instPtr : block->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      switch (inst->opcode()) {
      case Opcode::Shl:
      case Opcode::Shr:
      case Opcode::And:
      case Opcode::Or:
      case Opcode::Xor:
      case Opcode::Add:
        ++computeOps;
        break;
      case Opcode::Load:
        ++loadOps;
        break;
      case Opcode::Store:
        ++storeOps;
        break;
      default:
        break;
      }
    }
  }
  return computeOps >= 4 && loadOps <= 1 && storeOps <= 1;
}

bool tryPlan(Function* func,
             const LoopBandPlan& plan,
             analysis::FunctionAnalysisCache& cache,
             const analysis::AffineRegionPlan* region = nullptr,
             int alreadyAppliedNode = -1,
             int64_t netScoreHint = std::numeric_limits<int64_t>::min(),
             bool allowTileLocalBound = false,
             int* appliedNodeOut = nullptr) {
  if (!func || !plan.valid || plan.nodeIndices.empty() || plan.members.empty()) {
    return false;
  }
  if (!region)
    region = analysis::bestAffineRegionForLoopBand(cache, plan);
  if (region && netScoreHint == std::numeric_limits<int64_t>::min())
    netScoreHint = analysis::affineRegionNetProfitScore(*region);
  if (netScoreHint != std::numeric_limits<int64_t>::min() && netScoreHint < 32 &&
      plan.bandProfitScore < 128 && plan.estimatedWorkingSetDrop < 64) {
    return false;
  }
  BlockingRegionHints hints = collectBlockingRegionHints(region);
  bool hasDeepDenseMember = false;
  for (const auto& member : plan.members) {
    if (member.loopDepth >= 3 && member.memoryOps >= 7 &&
        member.indexedAffineMemoryOps >= 7 &&
        member.dynamicStrideMemoryOps == 0) {
      hasDeepDenseMember = true;
      break;
    }
  }
  if (hints.available && plan.totalIndexedAffineMemoryOps >= 7 &&
      plan.totalDynamicStrideMemoryOps == 0 &&
      (hints.estimatedRegisterPressure >= 16 || hasDeepDenseMember)) {
    // 密集 affine 内核已经包含较多地址和值的活跃区间。
    // 对这些循环带做分块会增加分块控制递推，并可能让后续用于隐藏延迟的
    // 部分展开器看不到内层算术循环。
    traceBlockingDecision("reject", func, plan,
                          "high-pressure-dense-band", netScoreHint);
    return false;
  }
  if (hints.available && !allowTileLocalBound &&
      netScoreHint != std::numeric_limits<int64_t>::min() && netScoreHint <= 0) {
    // region summary 已经把 split 后 memory/CFG/remainder 成本折成
    // 净收益。若净收益为负，普通 strip-mining 不应再靠旧 band hotness 放行；
    // tile-local bound 有单独的收益判断，这里不拦截。
    traceBlockingDecision("reject", func, plan, "negative-region-profit",
                          netScoreHint);
    return false;
  }
  if (hints.available && !allowTileLocalBound &&
      netScoreHint != std::numeric_limits<int64_t>::min() &&
      netScoreHint < 256 && !hints.postSplitProfitable) {
    // 普通 fallback 不能只凭旧 band hotness 执行低净收益分块。
    // 这类候选的 tile CFG/phi 成本通常已经超过 locality 收益；真正的
    // multi-level 路径仍由 allowTileLocalBound 下的专用 gate 负责。
    traceBlockingDecision("reject", func, plan, "low-region-profit",
                          netScoreHint);
    return false;
  }
  if (hints.available && !allowTileLocalBound &&
      plan.preferredTileFactor == 16 &&
      plan.totalAffineMemoryOps <= 4 &&
      plan.totalIndexedAffineMemoryOps <= 4 &&
      hints.estimatedRegisterPressure >= 8 &&
      hints.estimatedWorkingSetDrop <= 64 &&
      hints.postSplitMemoryDeltaScore <= 96) {
    // 只有一个缓存行工作集下降的窄仿射带会增加 tile CFG/PHI 开销，却没有
    // 足够的局部性收益来摊平这些开销。
    traceBlockingDecision("reject", func, plan, "thin-band-locality",
                          netScoreHint);
    return false;
  }
  if (hints.available && !allowTileLocalBound &&
      plan.preferredTileMember >= 0 &&
      plan.preferredTileMember < static_cast<int>(plan.members.size())) {
    const auto& preferred =
        plan.members[static_cast<size_t>(plan.preferredTileMember)];
    if (preferred.loop && loopHasFloatingArithmetic(*preferred.loop) &&
        preferred.memoryOps <= 3 && preferred.rootReuseScore <= 2 &&
        preferred.dynamicStrideMemoryOps == 0 &&
        hints.estimatedWorkingSetDrop <= 96) {
      traceBlockingDecision("reject", func, plan, "streaming-stripmine",
                            netScoreHint);
      return false;
    }

  // 拒绝对小型 bit-state 循环做 strip-mining
  if (hints.available && !allowTileLocalBound &&
      plan.preferredTileMember >= 0 &&
      plan.preferredTileMember < static_cast<int>(plan.members.size())) {
    const auto& preferred =
        plan.members[static_cast<size_t>(plan.preferredTileMember)];
    if (preferred.loop && !loopHasFloatingArithmetic(*preferred.loop) &&
        preferred.memoryOps <= 3 &&
        preferred.dynamicStrideMemoryOps == 0 &&
        hints.estimatedWorkingSetDrop <= 96) {
      int cappedTile = cappedTileFactorForRegion(
          preferred.suggestedTileFactor, hints, netScoreHint);
      if (cappedTile <= 8 &&
          (loopTouchesLocalArrayBuffer(*preferred.loop) ||
           loopHasBitStateCompute(*preferred.loop))) {
        traceBlockingDecision("reject", func, plan,
                              "small-workload-blocking", netScoreHint);
        return false;
      }
    }
  }

  }

  std::vector<int> order;
  order.reserve(plan.members.size());
  for (int i = 0; i < static_cast<int>(plan.members.size()); ++i)
    order.push_back(i);
  std::sort(order.begin(), order.end(),
            [&](int lhs, int rhs) {
              const auto& a = plan.members[static_cast<size_t>(lhs)];
              const auto& b = plan.members[static_cast<size_t>(rhs)];
              int cappedA =
                  cappedTileFactorForRegion(a.suggestedTileFactor, hints,
                                            netScoreHint);
              int cappedB =
                  cappedTileFactorForRegion(b.suggestedTileFactor, hints,
                                            netScoreHint);
              if (cappedA != cappedB)
                return cappedA > cappedB;
              int64_t aPriority =
                  memberBlockingPriority(plan, a, lhs, hints, netScoreHint);
              int64_t bPriority =
                  memberBlockingPriority(plan, b, rhs, hints, netScoreHint);
              if (aPriority != bPriority)
                return aPriority > bPriority;
              if (a.hotnessScore != b.hotnessScore)
                return a.hotnessScore > b.hotnessScore;
              if (a.loopDepth != b.loopDepth)
                return a.loopDepth > b.loopDepth;
              return lhs < rhs;
            });

  if (!hints.available && plan.preferredTileMember >= 0 &&
      plan.preferredTileMember < static_cast<int>(plan.members.size())) {
    // 没有 region/profit summary 时才回退到 loop-band 自己的
    // preferred member；一旦 AffineRegionPlan 可用，实际执行顺序应由共享
    // 共享的收益和边界信息决定顺序，避免旧启发式覆盖 post-split 指标。
    auto it = std::find(order.begin(), order.end(), plan.preferredTileMember);
    if (it != order.end() && it != order.begin())
      std::rotate(order.begin(), it, it + 1);
  }

  // 按排序后的顺序逐个 member 尝试 strip-mining：先由收益 hint
  // 限制 tile factor，再依次检查循环节点、专用 preheader 和
  // 局部数组缓冲区等合法性条件，任一不满足即跳到下一 member。
  for (int memberIdx : order) {
    const auto& member = plan.members[static_cast<size_t>(memberIdx)];
    int preferredFactor =
        cappedTileFactorForRegion(member.suggestedTileFactor, hints,
                                  netScoreHint);
    if (preferredFactor <= 1)
      continue;
    if (hints.available && !allowTileLocalBound &&
        hints.estimatedWorkingSetDrop < 32 &&
        hints.postSplitMemoryDeltaScore < 32) {
      if (loopBlockingTraceEnabled()) {
        std::fprintf(stderr,
                     "[loop-blocking] member-reject func=%s root=%d "
                     "member=%d reason=low-stripmine-profit wsdrop=%lld "
                     "memdelta=%lld\n",
                     func ? func->name().c_str() : "<null>", plan.rootNode,
                     memberIdx,
                     static_cast<long long>(hints.estimatedWorkingSetDrop),
                     static_cast<long long>(hints.postSplitMemoryDeltaScore));
      }
      continue;
    }
    int nodeIdx = plan.nodeIndices[static_cast<size_t>(memberIdx)];
    if (nodeIdx == alreadyAppliedNode)
      continue;
    const auto* node = cache.loopForest().node(nodeIdx);
    if (!node || !node->loop) {
      if (loopBlockingTraceEnabled()) {
        std::fprintf(stderr,
                     "[loop-blocking] member-reject func=%s root=%d node=%d "
                     "member=%d reason=no-loop-node\n",
                     func ? func->name().c_str() : "<null>", plan.rootNode,
                     nodeIdx, memberIdx);
      }
      continue;
    }
    if (!allowTileLocalBound &&
        hasNonCanonicalDirectChildLoop(cache.loopForest(), nodeIdx)) {
      // 链式搜索等不规则子循环的访存不代表外层顺序流具有分块局部性；
      // 此时切分外层循环只会增加 tile 控制，保留原有遍历结构。
      if (loopBlockingTraceEnabled()) {
        std::fprintf(stderr,
                     "[loop-blocking] member-reject func=%s root=%d node=%d "
                     "member=%d reason=nested-irregular-loop\n",
                     func ? func->name().c_str() : "<null>", plan.rootNode,
                     nodeIdx, memberIdx);
      }
      continue;
    }
    BasicBlock* preheader = analysis::dedicatedPreheader(*node->loop);
    if (!preheader) {
      if (loopBlockingTraceEnabled()) {
        std::fprintf(stderr,
                     "[loop-blocking] member-reject func=%s root=%d node=%d "
                     "member=%d reason=no-dedicated-preheader\n",
                     func ? func->name().c_str() : "<null>", plan.rootNode,
                     nodeIdx, memberIdx);
      }
      continue;
    }
    if (loopTouchesLocalArrayBuffer(*node->loop)) {
      if (loopBlockingTraceEnabled()) {
        std::fprintf(stderr,
                     "[loop-blocking] member-reject func=%s root=%d node=%d "
                     "member=%d reason=local-array-buffer\n",
                     func ? func->name().c_str() : "<null>", plan.rootNode,
                     nodeIdx, memberIdx);
      }
      continue;
    }

    // 通过合法性后要求可识别为 canonical loop，且初值/上界不依赖
    // 外层 IV（tile 脚手架定义除外），最后按候选 factor 逐个试 strip-mine。
    CanonicalLoop cl;
    if (!recognizeCanonicalLoop(*node->loop, preheader, cl)) {
      if (loopBlockingTraceEnabled()) {
        std::fprintf(stderr,
                     "[loop-blocking] member-reject func=%s root=%d node=%d "
                     "member=%d reason=not-canonical-loop\n",
                     func ? func->name().c_str() : "<null>", plan.rootNode,
                     nodeIdx, memberIdx);
      }
      continue;
    }
    if (nestedLoopLimitDependsOnCurrentIv(cache.loopForest(), nodeIdx, cl)) {
      if (loopBlockingTraceEnabled()) {
        std::fprintf(stderr,
                     "[loop-blocking] member-reject func=%s root=%d node=%d "
                     "member=%d reason=nested-iv-limit\n",
                     func ? func->name().c_str() : "<null>", plan.rootNode,
                     nodeIdx, memberIdx);
      }
      continue;
    }
    if ((valueDefinedInEnclosingLoop(cl.boundVal, cache.loopForest(), nodeIdx) ||
         valueDefinedInEnclosingLoop(cl.initVal, cache.loopForest(), nodeIdx)) &&
        !(allowTileLocalBound &&
          (valueDefinedInGeneratedTileScaffold(cl.boundVal) ||
           valueDefinedInGeneratedTileScaffold(cl.initVal)))) {
      if (loopBlockingTraceEnabled()) {
        std::fprintf(stderr,
                     "[loop-blocking] member-reject func=%s root=%d node=%d "
                     "member=%d reason=enclosing-loop-limit\n",
                     func ? func->name().c_str() : "<null>", plan.rootNode,
                     nodeIdx, memberIdx);
      }
      continue;
    }
    for (int factor :
         tileFactorCandidates(preferredFactor, member, hints, netScoreHint)) {
      const char* stripRejectReason = nullptr;
      if (applyStripMine(func, *node->loop, cl, factor, allowTileLocalBound,
                         &stripRejectReason)) {
        if (appliedNodeOut)
          *appliedNodeOut = nodeIdx;
        traceBlockingDecision("accept", func, plan, "strip-mine",
                              netScoreHint);
        return true;
      }
      if (loopBlockingTraceEnabled()) {
        std::fprintf(stderr,
                     "[loop-blocking] factor-reject func=%s root=%d node=%d "
                     "member=%d factor=%d reason=%s\n",
                     func ? func->name().c_str() : "<null>", plan.rootNode,
                     nodeIdx, memberIdx, factor,
                     stripRejectReason ? stripRejectReason : "-");
      }
    }
  }
  traceBlockingDecision("reject", func, plan, "strip-mine-failed",
                        netScoreHint);
  return false;
}

// 多层 blocking 入口：要求 band 合法且最优 affine region 的
// 快路径/寄存器压力/净收益等指标达标后才执行，否则维持单层 strip-mining。
bool tryMultiLevelPlan(Function* func,
                       const LoopBandPlan& plan,
                       analysis::AnalysisManager& analysisManager) {
  if (!func || !plan.valid || !plan.legality.hasMultiLevelBand ||
      !plan.legality.supportsBlocking) {
    return false;
  }
  analysis::FunctionAnalysisCache& initialCache = analysisManager.function(func);
  const auto* region = analysis::bestAffineRegionForLoopBand(initialCache, plan);
  if (!region)
    return false;
  const auto& profit = region->profit;
  const auto& versioning = region->versioning;
  const bool versionedFastPathReady =
      profit.physicalVersioned && versioning.hasGuardBranch &&
      versioning.hasInteriorBridge && versioning.hasRemainderBridge &&
      versioning.hasMergePhi;
  const bool versionFreeFastPathReady =
      !region->needsVersioning && profit.interiorFastPathReady &&
      profit.remainderPathReady;
  int64_t previousNetScore = analysis::affineRegionNetProfitScore(*region);
  // 对无动态 stride 且收益充分的 dense band，放宽为 version-free
  // 多层准入（denseVersionFreeMultiReady），允许直接复用 interior fast path。
  const bool denseVersionFreeMultiReady =
      versionFreeFastPathReady &&
      plan.totalDynamicStrideMemoryOps == 0 &&
      profit.estimatedRegisterPressure <= 18 &&
      previousNetScore >= 256 &&
      profit.estimatedWorkingSetDrop >= 128 &&
      profit.postSplitMemoryDeltaScore > 0 &&
      profit.postSplitCfgDeltaScore >= -128 &&
      profit.hotLoopBodyCondBranchCount <= 1;
  if ((!profit.multiLevelBlockingReady && !denseVersionFreeMultiReady) ||
      !profit.interiorFastPathReady ||
      (!profit.postSplitMetricsImproved && !denseVersionFreeMultiReady) ||
      (!profit.postSplitProfitable && !denseVersionFreeMultiReady) ||
      (!versionedFastPathReady && !versionFreeFastPathReady) ||
      plan.totalDynamicStrideMemoryOps != 0 ||
      (!denseVersionFreeMultiReady && profit.estimatedRegisterPressure > 12) ||
      previousNetScore < 128 ||
      profit.estimatedWorkingSetDrop < 128) {
    traceMultiLevelGate(func, plan, profit, *region, versionedFastPathReady,
                        versionFreeFastPathReady, denseVersionFreeMultiReady,
                        previousNetScore);
    // 真正的 multi-level blocking 只在 region/band 已经具备
    // 可消费的快路径收益、无动态 stride 且寄存器压力可控时执行。
    // 对需要 versioning 的 loop，仍要求 guard/merge 物理形状已经成型；
    // 对本来就不需要 versioning 的干净 affine band，则允许直接复用
    // version-free interior fast path，不再被“没有 split 外壳”误拦。
    // 其它情况仍保留单层 strip-mining，避免为了多层分块扩大 CFG 和 live range。
    traceBlockingDecision("reject-multi", func, plan, "multi-gate",
                          previousNetScore);
    return false;
  }

  bool changed = false;
  int lastAppliedNode = -1;
  // round7 暂时将 multi-level blocking 限制为最多 2 层。
  // 三层 blocking 在深度矩阵类等深度嵌套核上会导致 tile loop 嵌套过深，
  // 配合 unroller peel cascade 产生明显回退。等 unroller tile-loop 感知
  // 和寄存器压力 guard 稳定后再考虑重新放回 3 层。
  int maxLevels = 2;
  maxLevels = std::min<int>(static_cast<int>(plan.members.size()), maxLevels);
  for (int level = 0; level < maxLevels; ++level) {
    func->recomputeCFG();
    analysis::FunctionAnalysisCache& freshCache = analysisManager.function(func);
    auto freshPlans = freshCache.loopBands();

    std::sort(freshPlans.begin(), freshPlans.end(),
              [&](const LoopBandPlan& lhs, const LoopBandPlan& rhs) {
                int64_t lhsPriority = blockingPlanPriority(func, freshCache, lhs);
                int64_t rhsPriority = blockingPlanPriority(func, freshCache, rhs);
                if (lhsPriority != rhsPriority)
                  return lhsPriority > rhsPriority;
                return lhs.members.size() > rhs.members.size();
              });
    const LoopBandPlan* appliedPlan = nullptr;
    int64_t appliedNetScore = std::numeric_limits<int64_t>::min();
    for (const auto& freshPlan : freshPlans) {
      if (!freshPlan.legality.supportsBlocking)
        continue;
      // multi-level 执行器不能在第一层退化去挑单层高分 plan，
      // 否则会把真正的 band 变换入口让给无关 singleton，导致后续层永远
      // 没有机会执行。单层 fallback 仍留给外层 tryPlan。
      if (!freshPlan.legality.hasMultiLevelBand)
        continue;
      const auto* freshRegion =
          analysis::bestAffineRegionForLoopBand(freshCache, freshPlan);
      if (!freshRegion)
        continue;
      int64_t netScore = analysis::affineRegionNetProfitScore(*freshRegion);
      // 第二层及之后必须至少维持住上一层的大部分净收益，
      // 否则宁可停在当前层，避免为了“更多层”而引入额外 CFG/寄存器成本。
      if (level > 0 && netScore + 192 < previousNetScore)
        continue;
      if (netScore < 256)
        continue;
      const auto* matchingRegion =
          analysis::bestAffineRegionForLoopBand(freshCache, freshPlan);
      int appliedNode = -1;
      if (tryPlan(func, freshPlan, freshCache, matchingRegion, lastAppliedNode,
                  netScore, true, &appliedNode)) {
        appliedPlan = &freshPlan;
        appliedNetScore = netScore;
        lastAppliedNode = appliedNode;
        break;
      }
      traceBlockingDecision("reject-multi", func, freshPlan,
                            "fresh-candidate-failed", netScore);
    }
    if (!appliedPlan) {
      traceBlockingDecision("reject-multi", func, plan, "no-fresh-match",
                            previousNetScore);
      break;
    }

    changed = true;
    previousNetScore = appliedNetScore;
    // multi-level blocking 每成功一层都重新分析 loop forest，
    // 不复用旧 node 指针，避免 tile CFG 插入后继续按旧 band 变换。
    analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
  }
  if (changed)
    traceBlockingDecision("accept-multi", func, plan, "multi-level",
                          previousNetScore);
  return changed;
}

bool processFunction(Function* func, analysis::AnalysisManager& analysisManager) {
  if (!func || !func->entryBlock())
    return false;

  bool changed = false;
  for (int iter = 0; iter < 16; ++iter) {
    func->recomputeCFG();
    analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
    auto plans = cache.loopBands();
    std::sort(plans.begin(), plans.end(),
              [&](const LoopBandPlan& lhs, const LoopBandPlan& rhs) {
                int64_t lhsPriority = blockingPlanPriority(func, cache, lhs);
                int64_t rhsPriority = blockingPlanPriority(func, cache, rhs);
                if (lhsPriority != rhsPriority)
                  return lhsPriority > rhsPriority;
                if (lhs.members.size() != rhs.members.size())
                  return lhs.members.size() > rhs.members.size();
                return lhs.preferredTileFactor > rhs.preferredTileFactor;
              });
    bool iterChanged = false;
    for (const auto& plan : plans) {
      bool planChanged = false;
      if (plan.legality.hasMultiLevelBand)
        planChanged = tryMultiLevelPlan(func, plan, analysisManager);
      if (!planChanged) {
        const auto* region = analysis::bestAffineRegionForLoopBand(cache, plan);
        planChanged = tryPlan(func, plan, cache, region);
      }
      if (!planChanged)
        continue;
      iterChanged = true;
      changed = true;
      analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
      break;
    }
    if (!iterChanged)
      break;
    // 限制每个函数最多创建 6 个 tile.header。
    // 过多 tile loop（如深度分解类内核有 12+ 个）导致
    // blocking 自身开销超过收益，造成明显回退。
    {
      int tileHeaders = 0;
      for (auto& bb : func->blocks())
        if (bb->name().find("tile.header") != std::string::npos)
          tileHeaders++;
      if (tileHeaders >= 4) {
        traceBlockingDecision("cap-tile-headers", func, analysis::LoopBandPlan{},
                              "tile-header-limit", tileHeaders);
        break;
      }
    }
  }
  return changed;
}

} // namespace

bool applyLoopBlocking(Module& mod) {
  analysis::AnalysisManager analysisManager(mod);
  return applyLoopBlocking(mod, analysisManager);
}

bool applyLoopBlocking(Module& mod, analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions())
    changed |= processFunction(funcPtr.get(), analysisManager);
  return changed;
}

} // namespace hir
