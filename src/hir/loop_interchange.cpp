#include "research_switch.h"
// 文件职责：根据 affine 访问关系执行循环交换，优先改善内层访问 stride。本文件负责实现具体逻辑。
#include "loop_interchange.h"
#include "canonical_loop.h"

#include "../analysis/affine_access.h"
#include "../analysis/analysis_manager.h"
#include "../analysis/cfg_utils.h"
#include "../analysis/dominators.h"
#include "../analysis/function_analysis.h"
#include "../analysis/loop_forest.h"
#include "../analysis/induction_var.h"
#include "../analysis/loop_info.h"
#include "../analysis/loop_value_utils.h"
#include "affine_loop_locality_plan.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdio>
#include <cstdlib>

namespace hir {

namespace {

using analysis::NaturalLoop;
using analysis::LoopBandPlan;
using core_ir::BasicBlock;
using core_ir::Function;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Value;

// 检查循环体是否只包含纯计算：除头部控制流和 latch 归纳变量更新外，指令
// 只能是 Load、ALU、PHI 或写回标量 alloca 的归约存储；数组/全局写入、调用
// 和 alloca 不进入当前处理范围。
bool bodyIsPure(const NaturalLoop& loop) {
  for (auto* bb : loop.blocks) {
    for (auto& instPtr : bb->instructions()) {
      Opcode op = instPtr->opcode();
      if (op == Opcode::Call || op == Opcode::Alloca)
        return false;
      if (op == Opcode::Store) {
        if (instPtr->numOperands() != 2 ||
            !analysis::isScalarSlot(instPtr->operand(1)))
          return false;
      }
    }
  }
  return true;
}

bool sameLoopValue(Value* lhs, Value* rhs) {
  if (lhs == rhs)
    return true;
  auto* lhsConst = dynamic_cast<core_ir::ConstInt*>(lhs);
  auto* rhsConst = dynamic_cast<core_ir::ConstInt*>(rhs);
  return lhsConst && rhsConst && lhsConst->value() == rhsConst->value();
}

bool isAdjacentPerfectNest(const CanonicalLoop& outer,
                           const NaturalLoop& innerLoop,
                           CanonicalLoop& inner) {
  if (!outer.body)
    return false;
  Instruction* outerBodyTerm = outer.body->terminator();
  if (!outerBodyTerm || outerBodyTerm->opcode() != Opcode::Br)
    return false;
  // 保留现有的完美嵌套形状要求：当前交换只调整循环边界，不重映射外层循环体
  // 中的归纳变量使用。若外层循环体还包含其他准备语句，矩形循环交换就不适用。
  if (outer.body->instructions().size() != 1)
    return false;
  auto* innerHdr = dynamic_cast<BasicBlock*>(outerBodyTerm->operand(0));
  if (!innerHdr || innerHdr != innerLoop.header)
    return false;

  BasicBlock* innerPH = outer.body;
  if (analysis::dedicatedPreheader(innerLoop) != innerPH)
    return false;
  if (!recognizeCanonicalLoop(innerLoop, innerPH, inner))
    return false;
  return inner.exit == outer.latch;
}

int profitabilityScore(const NaturalLoop& innerLoop,
                       Instruction* outerPhi,
                       Instruction* innerPhi) {
  analysis::InductionVar outerIV;
  outerIV.phi = outerPhi;
  outerIV.storage = nullptr;
  outerIV.init = nullptr;
  outerIV.step = 1;
  outerIV.stepVal = nullptr;
  outerIV.isCanonical = true;
  analysis::InductionVar innerIV;
  innerIV.phi = innerPhi;
  innerIV.storage = nullptr;
  innerIV.init = nullptr;
  innerIV.step = 1;
  innerIV.stepVal = nullptr;
  innerIV.isCanonical = true;
  std::vector<analysis::InductionVar> ivs = {outerIV, innerIV};

  int score = 0;
  for (auto* bb : innerLoop.blocks) {
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      Value* addr = nullptr;
      if (inst->opcode() == Opcode::Load && inst->numOperands() == 1) {
        addr = inst->operand(0);
      } else if (inst->opcode() == Opcode::Store && inst->numOperands() == 2) {
        addr = inst->operand(1);
      } else if (inst->opcode() == Opcode::GEP) {
        addr = inst;
      }
      if (!addr)
        continue;

      analysis::AffineAccessSummary access =
          analysis::summarizeAffineAccess(addr, innerLoop, ivs);
      if (!access.valid)
        continue;

      int64_t outerCoeff =
          analysis::affineStrideCostForProfit(access, outerPhi);
      int64_t innerCoeff =
          analysis::affineStrideCostForProfit(access, innerPhi);
      if (outerCoeff <= innerCoeff)
        continue;
      int opWeight = inst->opcode() == Opcode::Store ? 12 : 8;
      if (innerCoeff == 0)
        opWeight += 4;
      score += static_cast<int>((outerCoeff - innerCoeff) * opWeight);
    }
  }
  return score;
}

int64_t bandPermutationBonus(Function* func,
                             analysis::FunctionAnalysisCache& cache,
                             const LoopBandPlan& band,
                             int outerIdx,
                             int innerIdx) {
  bool containsOuter = false;
  bool containsInner = false;
  for (int nodeIdx : band.nodeIndices) {
    containsOuter = containsOuter || nodeIdx == outerIdx;
    containsInner = containsInner || nodeIdx == innerIdx;
  }
  if (!containsOuter || !containsInner)
    return 0;
  if (!band.legality.supportsPermutation)
    return 0;

  auto cand = buildAffineLoopLocalityCandidate(func, band, &cache);
  if (!cand.supportsPermutation &&
      cand.primaryAction != AffineLoopLocalityAction::Permutation) {
    return 0;
  }
  // interchange 只消费 plan 的收益/风险排序，不绕过 perfect-nest、body purity
  // 和 affine stride 检查。
  return affineLoopLocalityCandidatePriority(cand) / 16;
}

// 执行实际交换。交换后外层遍历 [innerInit, innerBound)，内层遍历
// [outerInit, outerBound)。通过交换两个规范循环的 init/bound 值实现，CFG
// 结构保持不变；调用者会先检查内层循环体不包含依赖迭代顺序的可观察操作。
bool doSwap(CanonicalLoop& outer, CanonicalLoop& inner) {
  if (sameLoopValue(outer.initVal, inner.initVal) &&
      sameLoopValue(outer.boundVal, inner.boundVal))
    return false;
  // 交换每个 phi 的初始值，即来自前置块的入值。
  auto swapPhiInit = [](CanonicalLoop& L, Value* newInit) {
    // ivPhi 的操作数按（值、基本块）交错排列；与前置块配对的槽位就是初值槽。
    for (int i = 0; i + 1 < L.ivPhi->numOperands(); i += 2) {
      auto* paired = dynamic_cast<BasicBlock*>(L.ivPhi->operand(i + 1));
      if (paired == L.preheader) {
        L.ivPhi->setOperand(i, newInit);
        return;
      }
    }
  };
  Value* outerInit = outer.initVal;
  Value* innerInit = inner.initVal;
  swapPhiInit(outer, innerInit);
  swapPhiInit(inner, outerInit);

  // 交换 icmp 中使用的循环边界。
  auto setBound = [](CanonicalLoop& L, Value* newBound) {
    if (L.ivIsCmpLhs) L.cmp->setOperand(1, newBound);
    else L.cmp->setOperand(0, newBound);
  };
  Value* outerBound = outer.boundVal;
  Value* innerBound = inner.boundVal;
  setBound(outer, innerBound);
  setBound(inner, outerBound);
  return true;
}

bool processFunction(Function* func, analysis::AnalysisManager& analysisManager) {
  if (!func || func->blocks().empty())
    return false;

  bool changed = false;
  for (int iter = 0; iter < 12; ++iter) {
    analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
    const auto& forest = cache.loopForest();
    const auto& bands = cache.loopBands();

    int bestOuterIdx = -1;
    int bestInnerIdx = -1;
    int64_t bestScore = 0;
    for (int outerIdx = 0; outerIdx < static_cast<int>(forest.nodes().size());
         ++outerIdx) {
      const analysis::LoopTreeNode* outerNode = forest.node(outerIdx);
      if (!outerNode || !outerNode->loop)
        continue;

      CanonicalLoop outer;
      if (!recognizeCanonicalLoop(*outerNode->loop, outerNode->preheader, outer))
        continue;

      for (int innerIdx : outerNode->children) {
        const analysis::LoopTreeNode* innerNode = forest.node(innerIdx);
        if (!innerNode || !innerNode->loop)
          continue;

        CanonicalLoop inner;
        if (!isAdjacentPerfectNest(outer, *innerNode->loop, inner))
          continue;
        if (!bodyIsPure(*innerNode->loop))
          continue;

         // `doSwap` 会交换边界，但不会改写两个归纳变量的所有使用点。
         // 因此只有两个迭代域完全相同时才保持原有语义；这种情况本身没有收益，
         // 会直接跳过。矩形嵌套循环不进入当前交换路径。
        if (!sameLoopValue(outer.initVal, inner.initVal) ||
            !sameLoopValue(outer.boundVal, inner.boundVal))
          continue;

        int64_t score = profitabilityScore(*innerNode->loop, outer.ivPhi, inner.ivPhi);
        if (score <= 0)
          continue;
        score += outerNode->depth * 8 + innerNode->depth * 12;
        for (const auto& band : bands)
          score += bandPermutationBonus(func, cache, band, outerIdx, innerIdx);
        if (score > bestScore) {
          bestScore = score;
          bestOuterIdx = outerIdx;
          bestInnerIdx = innerIdx;
        }
      }
    }
    if (bestOuterIdx < 0 || bestInnerIdx < 0)
      break;

    const analysis::LoopTreeNode* outerNode = forest.node(bestOuterIdx);
    const analysis::LoopTreeNode* innerNode = forest.node(bestInnerIdx);
    if (!outerNode || !innerNode || !outerNode->loop || !innerNode->loop)
      break;

    CanonicalLoop outer;
    CanonicalLoop inner;
    if (!recognizeCanonicalLoop(*outerNode->loop, outerNode->preheader, outer) ||
        !isAdjacentPerfectNest(outer, *innerNode->loop, inner) ||
        !bodyIsPure(*innerNode->loop)) {
      break;
    }

    if (!doSwap(outer, inner))
      break;
    if (sycEnvironmentValue("SYC_TRACE_HIR_LOOP_INTERCHANGE")) {
      std::fprintf(stderr,
                   "[loop-interchange] function=%s outer=%s inner=%s score=%lld\n",
                   func->name().c_str(), outer.header->name().c_str(),
                   inner.header->name().c_str(),
                   static_cast<long long>(bestScore));
    }
    changed = true;
    analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
  }
  return changed;
}

} // namespace

bool applyLoopInterchange(Module& mod) {
  analysis::AnalysisManager analysisManager(mod);
  return applyLoopInterchange(mod, analysisManager);
}

bool applyLoopInterchange(Module& mod, analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    if (processFunction(funcPtr.get(), analysisManager)) changed = true;
  }
  return changed;
}

} // namespace hir
