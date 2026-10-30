// 文件职责：组织 CoreIR 优化流水线，包括清理、内联、CSE、循环优化等。本文件负责实现具体逻辑。
// 以下聚合本流水线所需的全部头文件：core_ir pass、分析层、HIR 变换与重写层。
#include "passes.h"
#include "pass_manager.h"
#include "pass_registry.h"
#include "passes_utils.h"
#include "verify.h"
#include "const_fold.h"
#include "phi_utils.h"
#include "simplify_cfg.h"
#include "if_convert.h"
#include "mem2reg.h"
#include "cse.h"
#include "loop_basic.h"
#include "scalar_recurrence.h"
#include "row_locality.h"
#include "stage_fusion.h"
#ifdef SYC_ENABLE_RESEARCH_PASSES
#include "producer_map.h"
#endif
#include "inline_passes.h"
#include "cleanup.h"
#include "correlated_value_propagation.h"
#include "function_specialization.h"
#include "dead_return_elim.h"
#include "inst_combine.h"
#include "gvn.h"
#include "partial_redundancy_elimination.h"
#include "global_code_motion.h"
#include "dead_store_elim.h"
#include "sccp.h"
#ifdef SYC_ENABLE_RESEARCH_PASSES
#include "jump_threading.h"
#endif
#include "loop_invariant_divisor_specialization.h"
#include "loop_unswitch.h"
#ifdef SYC_ENABLE_RESEARCH_PASSES
#include "loop_zero_coefficient_specialization.h"
#endif
#ifdef SYC_ENABLE_RESEARCH_PASSES
#include "loop_continue_bound.h"
#endif
#include "../../analysis/analysis_manager.h"
#include "../../analysis/dominators.h"
#include "../../analysis/function_effects.h"
#include "../../analysis/loop_info.h"
#include "../../analysis/cfg_utils.h"
#include "../../research_switch.h"
#include "../../analysis/induction_var.h"
#include "../../hir/array_recurrence.h"
#include "../../hir/canonical_loop.h"
#include "../../hir/loop_band_canonicalize.h"
#include "../../hir/loop_blocking.h"
#include "../../hir/affine_loop_tightening.h"
#include "../../hir/affine_scalar_rewrite.h"
#ifdef SYC_ENABLE_RESEARCH_PASSES
#include "../../hir/loop_scalar_accumulator.h"
#endif
#include "../../hir/pure_call_last_value.h"
#include "../../hir/loop_state_fixpoint_break.h"
#include "../../hir/dead_global_array_store_elim.h"
#include "../../hir/recursion_memoization.h"
#include "../../hir/tail_recursion.h"
#include "../../rewrite/ir_rewrite.h"
#include "../../rewrite/bit_patterns.h"
#include "../../rewrite/parity_canonicalize.h"
#include "../../rewrite/strength_reduce.h"
#include "../../rewrite/array_sroa.h"
#include "../../rewrite/load_hoist.h"
#include "../planning/region_deletion.h"
#ifdef SYC_ENABLE_RESEARCH_PASSES
#include "../planning/candidate_plan.h"
#endif
#include "../../hir/if_convert_accumulation.h"
#ifdef SYC_ENABLE_RESEARCH_PASSES
#include "../../hir/loop_fusion.h"
#include "../../hir/loop_interchange.h"
#endif
#include "../../hir/loop_unroll.h"
#include "../../hir/local_store_load_forward.h"
#include "../../hir/reduction_split.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <queue>
#include <stdexcept>
#include <string>
#include <sstream>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace core_ir {

namespace {

#ifdef SYC_ENABLE_RESEARCH_PASSES
// 首个端到端 planning pilot。强度削减本身只接受局部、精确的整数恒等式；
// 这里再用事务 clone 和真实 RV64 后端筛掉代码尺寸、寄存器压力或成本恶化。
// 该入口保持 research-only，便于继续标定 admission 阈值而不改变 strict。
bool runPlannedConstantStrengthReduction(
    Module& mod, analysis::AnalysisManager& analysisManager) {
  planning::CandidatePlan plan;
  plan.passName = "constant-strength-reduction";
  plan.analysisManager = &analysisManager;
  plan.legality = analysis::ProofResult::success(
      "local-exact-integer-identities");
  plan.transform = [](planning::PlanningClone& clone, std::string& reason) {
    if (rewrite::applyConstantStrengthReduction(clone.module()))
      return true;
    reason = "no-strength-reduction-candidate";
    return false;
  };
  planning::CandidateTransaction transaction(mod, std::move(plan));
  const auto evaluation = transaction.evaluate();
  if (sycEnvironmentValue("SYC_TRACE_PLANNED_STRENGTH_REDUCTION"))
    std::cerr << "[planning] pass=constant-strength-reduction reason="
              << evaluation.reasonCode << "\n";
  return evaluation.accepted && transaction.commit();
}

bool runPlannedLocalEquivalences(
    Module& mod, analysis::AnalysisManager& analysisManager,
    bool includeLocalLoadForwarding) {
  planning::CandidatePlan plan;
  plan.passName = "local-equivalence-bundle";
  plan.analysisManager = &analysisManager;
  plan.legality = analysis::ProofResult::success("local-equivalence-contracts");
  plan.proofFacts = {
      analysis::ProofResult::success("constant-fold-exact"),
      analysis::ProofResult::success("algebraic-identity-exact"),
      analysis::ProofResult::success("same-object-forwarding-checked"),
  };
  plan.targetCostSummary =
      "real-rv64-mir-pressure-spill-frame-critical-path-assembly";
  plan.transform = [includeLocalLoadForwarding](
                       planning::PlanningClone& clone, std::string& reason) {
    analysis::AnalysisManager candidateAnalysis(clone.module());
    bool changed = false;
    if (includeLocalLoadForwarding)
      changed |= localLoadForwarding(clone.module(), candidateAnalysis);
    changed |= constFold(clone.module());
    changed |= rewrite::applyIRAlgebraicRewrite(clone.module());
    changed |= rewrite::applyConstantStrengthReduction(clone.module());
    changed |= rewrite::applyParityCanonicalization(clone.module());
    changed |= basicBlockCSE(clone.module(), candidateAnalysis);
    if (changed) return true;
    reason = "no-local-equivalence-candidate";
    return false;
  };
  planning::CandidateTransaction transaction(mod, std::move(plan));
  const auto evaluation = transaction.evaluate();
  if (sycEnvironmentValue("SYC_TRACE_PLANNED_LOCAL_EQUIVALENCES"))
    std::cerr << "[planning] pass=local-equivalence-bundle reason="
              << evaluation.reasonCode << "\n";
  return evaluation.accepted && transaction.commit();
}

ModulePassSpec transactionalResearchPass(ModulePassSpec spec,
                                         const char* proofReason) {
  spec.transactional = true;
  spec.legality = analysis::ProofResult::success(proofReason);
  return spec;
}
#endif

// 多轮迭代清理：每轮依次做局部加载转发、单块 mem2reg、DCE、常量折叠、
// IR 代数重写、常量强度削减、奇偶规范化、块级 CSE、支配树 CSE、
// GEP 重基、线性块合并、CFG 简化、DCE、DSE；某一轮无任何变化即提前收敛退出。
bool runCleanupRounds(Module& mod,
                      const PipelineOptions& options,
                      analysis::AnalysisManager& analysisManager,
                      int maxRounds,
                      bool includeLocalLoadForwarding,
                      bool includeSingleBlockMem2Reg,
                      bool includeDomCSE) {
  bool changed = false;
  for (int iter = 0; iter < maxRounds; ++iter) {
    bool iterChanged = false;
#ifdef SYC_ENABLE_RESEARCH_PASSES
    const bool plannedLocalEquivalences = sycResearchSwitchEnabled(
        "SYC_ENABLE_PLANNED_LOCAL_EQUIVALENCES");
#else
    const bool plannedLocalEquivalences = false;
#endif
    verifyLiveOperandsForDebug(mod, "cleanup-round-entry");
    if (includeLocalLoadForwarding && !plannedLocalEquivalences) {
      iterChanged |= localLoadForwarding(mod, analysisManager);
      verifyLiveOperandsForDebug(mod, "local-load-forwarding");
    }
    if (includeSingleBlockMem2Reg) {
      iterChanged |= singleBlockScalarMem2Reg(mod);
      verifyLiveOperandsForDebug(mod, "single-block-mem2reg");
    }
    iterChanged |= dce(mod, analysisManager);
    verifyLiveOperandsForDebug(mod, "early-dce-after-scalar-promotion");
#ifdef SYC_ENABLE_RESEARCH_PASSES
    if (plannedLocalEquivalences)
      iterChanged |= runPlannedLocalEquivalences(
          mod, analysisManager, includeLocalLoadForwarding);
#endif
    if (!plannedLocalEquivalences)
      iterChanged |= constFold(mod);
    verifyLiveOperandsForDebug(mod, "const-fold");
    if (!plannedLocalEquivalences &&
        !sycEnvironmentValue("SYC_SKIP_IR_ALGEBRAIC"))
      iterChanged |= rewrite::applyIRAlgebraicRewrite(mod);
    verifyLiveOperandsForDebug(mod, "ir-algebraic-rewrite");
    if (options.enableConstStrengthReduce && !plannedLocalEquivalences) {
#ifdef SYC_ENABLE_RESEARCH_PASSES
      if (sycResearchSwitchEnabled(
              "SYC_ENABLE_PLANNED_STRENGTH_REDUCTION"))
        iterChanged |=
            runPlannedConstantStrengthReduction(mod, analysisManager);
      else
#endif
        iterChanged |= rewrite::applyConstantStrengthReduction(mod);
    }
    verifyLiveOperandsForDebug(mod, "constant-strength-reduction");
    if (!plannedLocalEquivalences &&
        !sycEnvironmentValue("SYC_SKIP_PARITY_CANONICALIZE"))
      iterChanged |= rewrite::applyParityCanonicalization(mod);
    verifyLiveOperandsForDebug(mod, "parity-canonicalization");
    if (!plannedLocalEquivalences)
      iterChanged |= basicBlockCSE(mod, analysisManager);
    verifyLiveOperandsForDebug(mod, "basic-block-cse");
    if (includeDomCSE)
      iterChanged |= dominatorTreeCSE(mod, analysisManager);
    verifyLiveOperandsForDebug(mod, "dominator-tree-cse");
    iterChanged |= rebaseLocalGEPChains(mod);
    verifyLiveOperandsForDebug(mod, "rebase-local-gep-chains");
    iterChanged |= mergeLinearBlocks(mod);
    verifyLiveOperandsForDebug(mod, "merge-linear-blocks");
    iterChanged |= simplifyCFG(mod);
    verifyLiveOperandsForDebug(mod, "simplify-cfg");
    iterChanged |= dce(mod, analysisManager);
    verifyLiveOperandsForDebug(mod, "dce");
    if (!sycEnvironmentValue("SYC_SKIP_DSE"))
      iterChanged |= deadStoreElimination(mod, analysisManager);
    verifyLiveOperandsForDebug(mod, "dead-store-elim");
    if (iterChanged)
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::Effects);
    changed |= iterChanged;
    if (!iterChanged)
      break;
  }
  return changed;
}

bool runMem2RegFixpoint(Module& mod,
                        const PipelineOptions& options,
                        analysis::AnalysisManager& analysisManager,
                        int maxRounds,
                        bool includeLocalLoadForwarding,
                        bool includeSingleBlockMem2Reg,
                        bool includeDomCSE) {
  bool changed = false;
  for (int iter = 0; iter < maxRounds; ++iter) {
    bool iterChanged = false;
    iterChanged |= runCleanupRounds(mod, options, analysisManager, 1,
                                    includeLocalLoadForwarding,
                                    includeSingleBlockMem2Reg,
                                    includeDomCSE);
    bool mem2regChanged = globalScalarMem2Reg(mod, analysisManager);
    iterChanged |= mem2regChanged;
    if (mem2regChanged) {
      iterChanged |= runCleanupRounds(mod, options, analysisManager, 1,
                                      includeLocalLoadForwarding,
                                      /*includeSingleBlockMem2Reg=*/false,
                                      /*includeDomCSE=*/true);
    }
    changed |= iterChanged;
    if (!iterChanged)
      break;
  }
  return changed;
}

} // namespace

bool splitSharedBranchComparisons(Module& mod) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func)
      continue;

    std::vector<Instruction*> compares;
    for (auto& bbPtr : func->blocks()) {
      for (auto& instPtr : bbPtr->instructions()) {
        Instruction* inst = instPtr.get();
        if (inst && (inst->opcode() == Opcode::ICmp ||
                     inst->opcode() == Opcode::FCmp))
          compares.push_back(inst);
      }
    }

    for (Instruction* compare : compares) {
      if (!compare || compare->uses().size() <= 1)
        continue;

      std::vector<Instruction*> branchUsers;
      bool allUsesAreBranches = true;
      for (Instruction* user : compare->uses()) {
        if (!user || user->opcode() != Opcode::CondBr ||
            user->numOperands() < 1 || user->operand(0) != compare) {
          allUsesAreBranches = false;
          break;
        }
        branchUsers.push_back(user);
      }
      if (!allUsesAreBranches || branchUsers.size() <= 1)
        continue;

      for (size_t index = 1; index < branchUsers.size(); ++index) {
        Instruction* branch = branchUsers[index];
        BasicBlock* block = branch ? branch->parent() : nullptr;
        if (!block || block->terminator() != branch)
          continue;

        auto* clone = new Instruction(compare->opcode(), compare->type(),
                                      compare->name() + ".branch");
        for (Value* operand : compare->operands())
          clone->addOperand(operand);
        block->insertBefore(branch, clone);
        branch->setOperand(0, clone);
        changed = true;
      }
    }
  }
  return changed;
}

bool runCoreIROptimizations(Module& mod, const PipelineOptions& options) {
  verifyModule(mod);
  // 统一分析管理器先作为 Phase 1 的架构入口接入。旧 pass 仍可直接调用
  // 现有分析函数，后续再逐步替换成 manager 缓存，避免一次性改变行为。
  analysis::AnalysisManager localAnalysisManager(mod);
  analysis::AnalysisManager& analysisManager =
      options.analysisManager ? *options.analysisManager : localAnalysisManager;
  (void)analysisManager.functionEffects();
  (void)analysisManager.recursiveFunctions();
  bool changed = false;
  auto envFlag = [](const char* name) {
    // 影响代码生成的开关只在 research 构建且 SYC_RESEARCH=1 时响应；
    // strict 构建保持环境无关，TRACE/DUMP/VERIFY 等观测开关除外。
    if (std::strncmp(name, "SYC_ENABLE_", 11) == 0)
      return sycResearchSwitchEnabled(name);
    return sycEnvironmentValue(name) != nullptr;
  };
  auto runRowLocality = [&]() {
    ModulePassSpec spec{
        "loop-memory", "row-locality", true,
        analysis::AnalysisInvalidation::CFG,
        [](Module& m, analysis::AnalysisManager& am) {
          return applyInPlaceRowLocality(m, am);
        }};
#ifdef SYC_ENABLE_RESEARCH_PASSES
    // research opt-in 用统一事务验证该 pass 的完整候选；strict 仍沿用已经
    // 登记的保守子集，不让研究开关改变提交构建。
    if (envFlag("SYC_ENABLE_COREIR_ROW_LOCALITY"))
      spec = transactionalResearchPass(
          std::move(spec), "row-locality-affine-dependence-proved");
#endif
    return runModulePass(spec, mod, analysisManager);
  };
#ifdef SYC_ENABLE_RESEARCH_PASSES
  auto runTransactionalStencil = [&]() {
    return runModulePass(
        transactionalResearchPass(
            {"stencil", "stencil-interior", true,
             analysis::AnalysisInvalidation::CFG,
             [](Module& m, analysis::AnalysisManager& am) {
               return applyGuardedStencilInteriorFastPath(m, am);
             }},
            "stencil-interior-guard-and-observer-proved"),
        mod, analysisManager);
  };
  auto runTransactionalStageFusion = [&]() {
    return runModulePass(
        transactionalResearchPass(
            {"region-state", "stage-fusion", true,
             analysis::AnalysisInvalidation::Module,
             [](Module& m, analysis::AnalysisManager& am) {
               bool stageChanged =
                   applyMapRowReduceChecksumFusion(m, am);
               stageChanged |=
                   applyRowBroadcastTransposeChecksumFusion(m, am);
               return stageChanged;
             }},
            "stage-effect-observer-and-liveout-proved"),
        mod, analysisManager);
  };
#endif
  auto runRecursiveMemoization = [&]() {
#ifdef SYC_ENABLE_RESEARCH_PASSES
    if (envFlag("SYC_ENABLE_COREIR_RECURSIVE_MEMOIZATION")) {
      return runModulePass(
          transactionalResearchPass(
              {"recursive-state", "recursive-memoization", true,
               analysis::AnalysisInvalidation::Module,
               [](Module& m, analysis::AnalysisManager& am) {
                 return hir::applyRecursiveMemoization(m, am);
               }},
              "recursive-state-effect-and-observer-proved"),
          mod, analysisManager);
    }
#endif
    return hir::applyRecursiveMemoization(mod, analysisManager);
  };
  // 该开关只增加早期运行窗口；收尾阶段的值域传播属于默认整理路径。
  const bool enableExperimentalEarlyCoreIRCVP =
      envFlag("SYC_ENABLE_EXPERIMENTAL_COREIR_CVP");
  // 阶段 2.5：数组 SROA。把较小数组 alloca 拆成逐元素的标量 alloca，
  // 供后续 Mem2Reg 提升。该阶段在清理循环前执行一次，
  // 清理循环中的 Mem2Reg 会继续处理新产生的标量。
  if (options.enableArraySROA && !envFlag("SYC_SKIP_COREIR_EARLY") &&
      !envFlag("SYC_SKIP_ARRAY_SROA"))
    changed |= rewrite::applyArraySROA(mod);
  changed |= forwardConstGlobalLoads(mod);
  if (!envFlag("SYC_SKIP_COREIR_EARLY")) {
    if (!envFlag("SYC_SKIP_EARLY_MEM2REG"))
      changed |= runMem2RegFixpoint(mod, options, analysisManager, 4,
                                    /*includeLocalLoadForwarding=*/true,
                                    /*includeSingleBlockMem2Reg=*/true,
                                    /*includeDomCSE=*/true);
    if (!envFlag("SYC_SKIP_COREIR_BIT_ACCUMULATOR")) {
      bool digitChanged =
          rewrite::applyPow2DigitHelperRecognition(mod, analysisManager);
      changed |= digitChanged;
      if (digitChanged)
        analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
    }
    bool scalarChanged = false;
    bool inlineChanged = false;
    // early inline 当前会和紧随其后的 remem2reg / cleanup 形成
    // 错误联动，复杂过程在被提前折平后容易被过度清理。为先保证 -O1
    // 正确性，这里暂时停掉 early inline，只保留 late inline 作为更靠后的
    // 保守折叠入口。
    if (inlineChanged && !envFlag("SYC_SKIP_EARLY_FORWARD_SCALARS")) {
      const analysis::FunctionEffectMap& effects = analysisManager.functionEffects();
      for (auto& funcPtr : mod.functions())
        scalarChanged |= forwardSingleAssignmentScalarGlobals(
            funcPtr.get(), mod, effects);
    }
    if (!envFlag("SYC_SKIP_EARLY_PROMOTE_GLOBALS"))
      scalarChanged |=
          promoteDirectScalarGlobalsToEntryLocals(mod, analysisManager);
    bool loopCanonChanged = false;
    if (!envFlag("SYC_SKIP_EARLY_LOOP_CANON"))
      loopCanonChanged = loopCanonicalize(mod, analysisManager);
    scalarChanged |= loopCanonChanged;
    if (loopCanonChanged)
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
    if (!envFlag("SYC_SKIP_EARLY_DOM_CSE"))
      scalarChanged |= dominatorTreeCSE(mod, analysisManager);
    if (!envFlag("SYC_SKIP_EARLY_GVN"))
      scalarChanged |= globalValueNumbering(mod, analysisManager);
    if (!envFlag("SYC_SKIP_EARLY_PRE"))
      scalarChanged |= partialRedundancyElimination(mod, analysisManager);
    if (!envFlag("SYC_SKIP_EARLY_GCM"))
      scalarChanged |= globalCodeMotion(mod, analysisManager);
    if (!envFlag("SYC_SKIP_EARLY_SCCP"))
      scalarChanged |= sparseConditionalConstantPropagation(mod, analysisManager);
    if (!envFlag("SYC_SKIP_EARLY_LVN"))
      scalarChanged |= loopLocalValueNumbering(mod, analysisManager);
    if (!envFlag("SYC_SKIP_EARLY_LICM"))
      scalarChanged |= loopInvariantCodeMotion(mod, analysisManager);
    // Phase 2.6：循环不变量加载提升，补充前面的 LICM 未处理的加载；必须在
    // loopCanonicalize 之后运行，使前置块识别器能够找到专用前置块。
    if (options.enableLoadHoist && !envFlag("SYC_SKIP_EARLY_LOAD_HOIST")) {
      scalarChanged |=
          rewrite::applyLoopInvariantLoadHoist(mod, analysisManager);
    }
    if (!envFlag("SYC_SKIP_COREIR_MOD_SCALAR_RECURRENCE")) {
      bool modRecChanged = applyModularScalarRecurrences(mod, analysisManager);
      scalarChanged |= modRecChanged;
      if (modRecChanged)
        analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
    }
    if (!envFlag("SYC_SKIP_COREIR_REPEATED_REDUCTION")) {
      bool repeatedChanged = applyRepeatedReductionLoops(mod, analysisManager);
      scalarChanged |= repeatedChanged;
      if (repeatedChanged)
        analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
    }
    // 线性递推快路径只基于 CoreIR SSA/CFG/effect 事实，并带有大 trip-count
    // 成本门槛；默认启用以覆盖纯标量大递推，仍保留环境变量用于回归定位。
    if (!envFlag("SYC_SKIP_COREIR_LINEAR_RECURRENCE")) {
      bool linRecChanged =
          applySmallLinearRecurrenceLoops(mod, analysisManager);
      scalarChanged |= linRecChanged;
      if (linRecChanged)
        analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
    }
    if (!envFlag("SYC_SKIP_COREIR_ROW_LOCALITY")) {
      bool rowLocalChanged = runRowLocality();
      scalarChanged |= rowLocalChanged;
      if (rowLocalChanged)
        analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
    }
    changed |= scalarChanged;
    if (scalarChanged) {
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::Effects);
      if (!envFlag("SYC_SKIP_EARLY_REMEM2REG"))
        changed |= runMem2RegFixpoint(mod, options, analysisManager, 2,
                                      /*includeLocalLoadForwarding=*/true,
                                      /*includeSingleBlockMem2Reg=*/true,
                                      /*includeDomCSE=*/true);
    }
  }
  if (!envFlag("SYC_SKIP_COREIR_EARLY") &&
      !envFlag("SYC_SKIP_COREIR_ROW_LOCALITY") &&
      !envFlag("SYC_SKIP_COREIR_ROW_LOCALITY_POST_CLEANUP")) {
    // row-locality 依赖 canonical loop + mem2reg 暴露出的
    // affine pointer phi。第一次 early 标量窗口可能仍是原始 GEP/store
    // 形态，因此在 remem2reg/cleanup 后补一次同一通用 matcher。
    bool rowLocalChanged = runRowLocality();
    changed |= rowLocalChanged;
    if (rowLocalChanged) {
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::Effects);
      changed |= runCleanupRounds(mod, options, analysisManager, 1,
                                  /*includeLocalLoadForwarding=*/true,
                                  /*includeSingleBlockMem2Reg=*/false,
                                  /*includeDomCSE=*/true);
    }
  }
#ifdef SYC_ENABLE_RESEARCH_PASSES
  if (envFlag("SYC_ENABLE_COREIR_STAGE_FUSION") &&
      !envFlag("SYC_SKIP_COREIR_STAGE_FUSION")) {
    const bool stageChanged = runTransactionalStageFusion();
    changed |= stageChanged;
    if (stageChanged)
      changed |= runCleanupRounds(mod, options, analysisManager, 1,
                                  /*includeLocalLoadForwarding=*/true,
                                  /*includeSingleBlockMem2Reg=*/false,
                                  /*includeDomCSE=*/true);
  }
#endif
  if (!envFlag("SYC_SKIP_COREIR_EARLY") &&
      !envFlag("SYC_SKIP_COREIR_BIT_ACCUMULATOR")) {
    bool earlyBitPatternChanged = rewrite::applyBitAccumulatorRecognition(
        mod, analysisManager);
    changed |= earlyBitPatternChanged;
    if (earlyBitPatternChanged) {
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::Effects);
    }
  }
  // 位模式识别：子匹配器均为结构角色匹配
  // （pow2 位助手 / 位累加 / 位循环 / 通用化 modmul），默认开启 + SKIP 旁路。
  if (!envFlag("SYC_SKIP_COREIR_EARLY") &&
      !envFlag("SYC_SKIP_COREIR_BIT_PATTERNS")) {
    bool earlyBitPatternChanged =
        rewrite::applyBitPatternRecognition(mod, analysisManager);
    changed |= earlyBitPatternChanged;
    if (earlyBitPatternChanged) {
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::Effects);
    }
  }
  if (!envFlag("SYC_SKIP_COREIR_EARLY") &&
      !envFlag("SYC_SKIP_COREIR_FUNCTION_SPECIALIZATION")) {
    const bool specializationChanged = runModulePass(
        ModulePassSpec{
            "coreir-early",
            "function-specialization",
            true,
            analysis::AnalysisInvalidation::Module,
            [](Module& module, analysis::AnalysisManager& manager) {
              return applyFunctionSpecialization(module, manager);
            }},
        mod, analysisManager);
    changed |= specializationChanged;
    if (specializationChanged) {
      changed |= runCleanupRounds(mod, options, analysisManager, 1,
                                  /*includeLocalLoadForwarding=*/true,
                                  /*includeSingleBlockMem2Reg=*/false,
                                  /*includeDomCSE=*/true);
    }
  }
  if (!envFlag("SYC_SKIP_COREIR_EARLY") &&
      !envFlag("SYC_SKIP_COREIR_DEAD_RETURN")) {
    // 无用返回值消除：全部调用点都忽略返回值的函数，其返回值计算链
    // 对行为不可观测；替为零常量后由紧跟的清理轮 DCE 收掉死链。
    if (options.enableDeadReturnElim) {
      const bool deadReturnChanged = runModulePass(
          ModulePassSpec{
              "coreir-early",
              "dead-return-elim",
              true,
              analysis::AnalysisInvalidation::Module,
              [](Module& module, analysis::AnalysisManager& manager) {
                return deadReturnValueElimination(module, manager);
              }},
          mod, analysisManager);
      changed |= deadReturnChanged;
      if (deadReturnChanged) {
        changed |= runCleanupRounds(mod, options, analysisManager, 2,
                                    /*includeLocalLoadForwarding=*/true,
                                    /*includeSingleBlockMem2Reg=*/false,
                                    /*includeDomCSE=*/true);
      }
    }
  }
  if (!envFlag("SYC_SKIP_COREIR_EARLY") &&
      !envFlag("SYC_SKIP_COREIR_INST_COMBINE")) {
    const bool instCombineChanged = runModulePass(
        ModulePassSpec{
            "coreir-early",
            "inst-combine",
            true,
            analysis::AnalysisInvalidation::CFG,
            [](Module& module, analysis::AnalysisManager& manager) {
              return applyInstCombine(module, manager);
            }},
        mod, analysisManager);
    changed |= instCombineChanged;
    if (instCombineChanged) {
      changed |= runCleanupRounds(mod, options, analysisManager, 1,
                                  /*includeLocalLoadForwarding=*/true,
                                  /*includeSingleBlockMem2Reg=*/false,
                                  /*includeDomCSE=*/true);
    }
  }
  if (!envFlag("SYC_SKIP_COREIR_EARLY") &&
      !envFlag("SYC_SKIP_COREIR_CORRELATED_VALUE_PROPAGATION") &&
      enableExperimentalEarlyCoreIRCVP) {
    const bool cvpChanged = runModulePass(
        ModulePassSpec{
            "coreir-early",
            "correlated-value-propagation",
            true,
            analysis::AnalysisInvalidation::CFG,
            [](Module& module, analysis::AnalysisManager& manager) {
              return applyCorrelatedValuePropagation(module, manager);
            }},
        mod, analysisManager);
    changed |= cvpChanged;
    if (cvpChanged) {
      changed |= runCleanupRounds(mod, options, analysisManager, 1,
                                  /*includeLocalLoadForwarding=*/true,
                                  /*includeSingleBlockMem2Reg=*/false,
                                  /*includeDomCSE=*/true);
    }
  }
  // 未读全局数组死存储消除在主要 HIR/late 变换完成后执行；
  // 具体调用位于下方统一清理窗口，避免提前改变 loop-memory 的输入形状。
  if (!envFlag("SYC_SKIP_COREIR_EARLY") &&
      !envFlag("SYC_SKIP_COREIR_PROCEDURE_INLINE")) {
    // 只在 loop-memory 之前内联单调用者、arg-memory-only、
    // 无内部调用/栈对象且调用点位于循环内的有限过程。它是通用 IPO
    // profit hook，用来暴露后续 affine/RA 机会。
    bool procedureInlineChanged = false;
    bool tinyInlineChanged = false;
    if (!envFlag("SYC_SKIP_COREIR_EARLY_TINY_PURE_INLINE"))
      tinyInlineChanged = inlineTinyPureHelpers(mod, analysisManager);
    procedureInlineChanged |= tinyInlineChanged;
    changed |= tinyInlineChanged;
    if (tinyInlineChanged) {
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::Effects);
      bool promoteChanged =
          promoteDirectScalarGlobalsToEntryLocals(mod, analysisManager);
      changed |= promoteChanged;
      if (promoteChanged) {
        changed |= runMem2RegFixpoint(mod, options, analysisManager, 2,
                                      /*includeLocalLoadForwarding=*/true,
                                      /*includeSingleBlockMem2Reg=*/true,
                                      /*includeDomCSE=*/true);
      }
      changed |= runCleanupRounds(mod, options, analysisManager, 1,
                                  /*includeLocalLoadForwarding=*/true,
                                  /*includeSingleBlockMem2Reg=*/false,
                                  /*includeDomCSE=*/true);
    }
    const bool singleCallerInlineChanged =
        inlineSingleCallerLoopProcedures(mod, analysisManager);
    procedureInlineChanged |= singleCallerInlineChanged;
    changed |= singleCallerInlineChanged;
    if (singleCallerInlineChanged) {
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::Effects);
      bool loopCanonChanged = loopCanonicalize(mod, analysisManager);
      changed |= loopCanonChanged;
      if (loopCanonChanged)
        analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
      changed |= runCleanupRounds(mod, options, analysisManager, 1,
                                  /*includeLocalLoadForwarding=*/true,
                                  /*includeSingleBlockMem2Reg=*/false,
                                  /*includeDomCSE=*/true);
      if (!envFlag("SYC_SKIP_COREIR_OVERWRITE_LOOP_COLLAPSE") &&
          !envFlag("SYC_SKIP_COREIR_OVERWRITE_LOOP_COLLAPSE")) {
        bool overwriteChanged =
            applyOverwriteRepeatLoopCollapse(mod, analysisManager);
        changed |= overwriteChanged;
        if (overwriteChanged) {
          analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
          changed |= runCleanupRounds(mod, options, analysisManager, 1,
                                      /*includeLocalLoadForwarding=*/true,
                                      /*includeSingleBlockMem2Reg=*/false,
                                      /*includeDomCSE=*/true);
        }
      }
    }
    if (procedureInlineChanged && !envFlag("SYC_SKIP_COREIR_INST_COMBINE")) {
      const bool postInlineInstCombineChanged = runModulePass(
          ModulePassSpec{
              "coreir-early-post-inline",
              "inst-combine",
              true,
              analysis::AnalysisInvalidation::CFG,
              [](Module& module, analysis::AnalysisManager& manager) {
                return applyPostInlineInstCombine(module, manager);
              }},
          mod, analysisManager);
      changed |= postInlineInstCombineChanged;
      if (postInlineInstCombineChanged) {
        changed |= runCleanupRounds(mod, options, analysisManager, 1,
                                    /*includeLocalLoadForwarding=*/true,
                                    /*includeSingleBlockMem2Reg=*/false,
                                    /*includeDomCSE=*/true);
      }
    }
  }
  if (!envFlag("SYC_SKIP_COREIR_EARLY") &&
      !envFlag("SYC_SKIP_COREIR_CORRELATED_VALUE_PROPAGATION") &&
      enableExperimentalEarlyCoreIRCVP) {
    const bool postInlineCVPChanged = runModulePass(
        ModulePassSpec{
            "coreir-early-post-inline",
            "correlated-value-propagation",
            true,
            analysis::AnalysisInvalidation::CFG,
            [](Module& module, analysis::AnalysisManager& manager) {
              return applyCorrelatedValuePropagation(module, manager);
            }},
        mod, analysisManager);
    changed |= postInlineCVPChanged;
    if (postInlineCVPChanged) {
      changed |= runCleanupRounds(mod, options, analysisManager, 1,
                                  /*includeLocalLoadForwarding=*/true,
                                  /*includeSingleBlockMem2Reg=*/false,
                                  /*includeDomCSE=*/true);
    }
  }
  if (!envFlag("SYC_SKIP_COREIR_EARLY") &&
      !envFlag("SYC_SKIP_COREIR_LOOP_UNSWITCH")) {
    const bool loopUnswitchChanged = runModulePass(
        ModulePassSpec{
            "coreir-early-post-inline",
            "loop-unswitch",
            true,
            analysis::AnalysisInvalidation::Module,
            [](Module& module, analysis::AnalysisManager& manager) {
              return applyLoopUnswitch(module, manager);
            }},
        mod, analysisManager);
    changed |= loopUnswitchChanged;
    if (loopUnswitchChanged) {
      changed |= runCleanupRounds(mod, options, analysisManager, 1,
                                  /*includeLocalLoadForwarding=*/true,
                                  /*includeSingleBlockMem2Reg=*/false,
                                  /*includeDomCSE=*/true);
    }
  }
#ifdef SYC_ENABLE_RESEARCH_PASSES
  if (!envFlag("SYC_SKIP_COREIR_EARLY") &&
      envFlag("SYC_ENABLE_COREIR_ZERO_COEFF_SPECIALIZATION") &&
      !envFlag("SYC_SKIP_COREIR_ZERO_COEFF_SPECIALIZATION")) {
    const bool zeroCoeffSpecializationChanged = runModulePass(
        transactionalResearchPass(ModulePassSpec{
            "coreir-early-post-inline",
            "loop-zero-coefficient-specialization",
            true,
            analysis::AnalysisInvalidation::Module,
            [](Module& module, analysis::AnalysisManager& manager) {
              return applyLoopZeroCoefficientSpecialization(module, manager);
            }}, "zero-coefficient-guards-proved"),
        mod, analysisManager);
    changed |= zeroCoeffSpecializationChanged;
    if (zeroCoeffSpecializationChanged) {
      changed |= runCleanupRounds(mod, options, analysisManager, 1,
                                  /*includeLocalLoadForwarding=*/true,
                                  /*includeSingleBlockMem2Reg=*/false,
                                  /*includeDomCSE=*/true);
    }
  }
#endif
  if (!envFlag("SYC_SKIP_COREIR_EARLY") &&
      !envFlag("SYC_SKIP_COREIR_DIVISOR_SPECIALIZATION")) {
    const bool divisorSpecializationChanged = runModulePass(
        ModulePassSpec{
            "coreir-early-post-inline",
            "loop-invariant-divisor-specialization",
            true,
            analysis::AnalysisInvalidation::Module,
            [](Module& module, analysis::AnalysisManager& manager) {
              return applyLoopInvariantDivisorSpecialization(module, manager);
            }},
        mod, analysisManager);
    changed |= divisorSpecializationChanged;
    if (divisorSpecializationChanged) {
      changed |= runCleanupRounds(mod, options, analysisManager, 1,
                                  /*includeLocalLoadForwarding=*/true,
                                  /*includeSingleBlockMem2Reg=*/false,
                                  /*includeDomCSE=*/true);
    }
  }
#ifdef SYC_ENABLE_RESEARCH_PASSES
  if (!envFlag("SYC_SKIP_COREIR_EARLY") &&
      envFlag("SYC_ENABLE_EXPERIMENTAL_JUMP_THREADING") &&
      !envFlag("SYC_SKIP_COREIR_JUMP_THREADING")) {
    const bool jumpThreadingChanged = runModulePass(
        transactionalResearchPass(ModulePassSpec{
            "coreir-early-post-inline",
            "jump-threading",
            true,
            analysis::AnalysisInvalidation::CFG,
            [](Module& module, analysis::AnalysisManager& manager) {
              return applyJumpThreading(module, manager);
            }}, "threaded-edge-condition-proved"),
        mod, analysisManager);
    changed |= jumpThreadingChanged;
    if (jumpThreadingChanged) {
      changed |= runCleanupRounds(mod, options, analysisManager, 1,
                                  /*includeLocalLoadForwarding=*/true,
                                  /*includeSingleBlockMem2Reg=*/false,
                                  /*includeDomCSE=*/true);
    }
  }
#endif
  // 标量清理之后执行高层循环和数组变换。
  bool hirChanged = false;
  bool loopMemoryChanged = false;
  if (!envFlag("SYC_SKIP_COREIR_RUNTIME_BOUND_LOCALIZE")) {
    bool runtimeBoundChanged =
        localizeRuntimeScalarGlobalBounds(mod, analysisManager);
    changed |= runtimeBoundChanged;
  }
  const bool enableHIRLoopMemory =
      options.enableHIRLoopMemory &&
      !envFlag("SYC_SKIP_COREIR_LOOP_MEMORY") &&
      hasHIRLoopMemoryCandidates(mod, analysisManager);
  if (enableHIRLoopMemory) {
    for (int iter = 0; iter < 3; ++iter) {
      bool iterChanged = false;
      auto runHIRPass = [&](ModulePassSpec spec) {
        // loop-memory 阶段的 pass 会连续消费 LoopForest、
        // AffineAccess、effect 等缓存。只要 pass 改了 CFG/访存形状，就必须
        // 按它声明的粒度失效；否则 early cleanup 生成的新 IR 会被后续
        // loop-memory pass 用旧 loop band 或旧支配信息解释，出现错误重写。
        const bool passChanged = runModulePass(spec, mod, analysisManager);
        iterChanged |= passChanged;
        return passChanged;
      };

#ifdef SYC_ENABLE_RESEARCH_PASSES
      // 该变换目前只识别固定双回边 continue 骨架，尚无独立正向迁移证据；
      // research 构建需显式开启，strict 提交构建不编译也不注册该实现。
      runHIRPass(transactionalResearchPass(
                 {"hir-loop-memory", "loop-continue-bound",
                  envFlag("SYC_ENABLE_HIR_LOOP_CONTINUE_BOUND") &&
                      !envFlag("SYC_SKIP_COREIR_LOOP_CONTINUE_BOUND"),
                  analysis::AnalysisInvalidation::CFG,
                  [](Module& m, analysis::AnalysisManager& am) {
                    return applyLoopContinueBound(m, am);
                  }}, "continue-bound-domain-proved"));
#endif
      runHIRPass({"hir-loop-memory", "loop-band-canonicalize",
                  !envFlag("SYC_SKIP_HIR_LOOP_BAND_CANONICALIZE"),
                  analysis::AnalysisInvalidation::CFG,
                  [](Module& m, analysis::AnalysisManager& am) {
                    return hir::applyLoopBandCanonicalization(m, am);
                  }});
#ifdef SYC_ENABLE_RESEARCH_PASSES
      runHIRPass(transactionalResearchPass(
                 {"hir-loop-memory", "loop-fusion",
                  options.enableLoopFusion &&
                      envFlag("SYC_ENABLE_HIR_LOOP_FUSION") &&
                      !envFlag("SYC_SKIP_HIR_LOOP_FUSION"),
                  analysis::AnalysisInvalidation::CFG,
                  [](Module& m, analysis::AnalysisManager& am) {
                    return hir::applyLoopFusion(m, am);
                  }}, "affine-fusion-dependence-proved"));
      runHIRPass(transactionalResearchPass(
                 {"hir-loop-memory", "loop-interchange",
                  options.enableLoopInterchange &&
                      envFlag("SYC_ENABLE_HIR_LOOP_INTERCHANGE") &&
                      !envFlag("SYC_SKIP_HIR_LOOP_INTERCHANGE"),
                  analysis::AnalysisInvalidation::CFG,
                  [](Module& m, analysis::AnalysisManager& am) {
                    return hir::applyLoopInterchange(m, am);
                  }}, "affine-permutation-dependence-proved"));
#endif
      // 条件累积 if-conversion 只保留不提前执行 load 的严格子集。宽模式会把
      // then 块中的 load 链推测到条件之前，尚缺完整的内存/边界 refinement
      // 证明，因此 research enable 也不得放宽该门槛。
      runHIRPass({"hir-loop-memory", "if-convert-accumulation",
                  !envFlag("SYC_SKIP_IF_CONVERT_ACCUMULATION"),
                  analysis::AnalysisInvalidation::CFG,
                  [](Module& m, analysis::AnalysisManager& am) {
                    return hir::applyIfConvertAccumulation(m, am, true);
                  }});
      // 先在单结果 producer 仍完整时证明并事务式尝试 row-min/broadcast/
      // transpose/checksum 融合。Reduction split 会把相邻两列拆成两个 store，
      // 从而丢失 producer 的单定义事实；融合失败或机器准入拒绝时，原模块不变，
      // 后续仍可正常尝试 reduction split。
      ModulePassSpec rowBroadcastFusion{
          "hir-loop-memory", "row-broadcast-transpose-checksum-fusion",
          !envFlag("SYC_SKIP_COREIR_ROW_BROADCAST_FUSION"),
          analysis::AnalysisInvalidation::Module,
          [](Module& m, analysis::AnalysisManager& am) {
            return applyRowBroadcastTransposeChecksumFusion(m, am);
          }};
      rowBroadcastFusion.transactional = true;
      rowBroadcastFusion.legality = analysis::ProofResult::success(
          "row-broadcast-observer-effect-liveout-proved");
      runHIRPass(std::move(rowBroadcastFusion));
      // 规约循环寄存器分块：中层步长 2、内层双车道共享 src1 装载。
      // 仅对偶数常量中层边界、k 循环体指令数 ≤8、N≥32 的简单模式生效。
      runHIRPass({"hir-loop-memory", "reduction-split",
                  !envFlag("SYC_SKIP_COREIR_REDUCTION_SPLIT"),
                  analysis::AnalysisInvalidation::CFG,
                  [](Module& m, analysis::AnalysisManager& am) {
                    return hir::applyReductionSplit(m, am);
                  }});
#ifdef SYC_ENABLE_RESEARCH_PASSES
      runHIRPass(transactionalResearchPass(
                 {"hir-loop-memory", "stencil-interior-pre-blocking",
                  envFlag("SYC_ENABLE_COREIR_STENCIL_INTERIOR") &&
                      !envFlag("SYC_SKIP_COREIR_STENCIL_INTERIOR") &&
                      !envFlag("SYC_SKIP_COREIR_STENCIL_INTERIOR_PRE_BLOCKING"),
                  analysis::AnalysisInvalidation::CFG,
                  [](Module& m, analysis::AnalysisManager& am) {
                    return applyGuardedStencilInteriorFastPath(m, am);
                  }}, "stencil-interior-guard-proved"));
#endif
      // 先把真正的小常量 trip 内层循环展开，再让 blocking
      // 处理剩余动态 loop band。否则 blocking 可能把常量 5 之类的核循环
      // 切成 tile/remainder CFG，后续只能复制更大的动态外层结构。
      const bool preBlockingUnrollChanged =
          runHIRPass({"hir-loop-memory", "loop-unroll-pre-blocking",
                      options.enableLoopUnroll &&
                          envFlag("SYC_ENABLE_HIR_LOOP_UNROLL_PRE_BLOCKING") &&
                          !envFlag("SYC_SKIP_HIR_LOOP_UNROLL"),
                      analysis::AnalysisInvalidation::CFG,
                      [](Module& m, analysis::AnalysisManager& am) {
                        return hir::applyLoopUnroll(m, am);
                      }});
      if (preBlockingUnrollChanged) {
        runHIRPass({"hir-loop-memory", "affine-scalar-rewrite-pre-blocking",
                    !envFlag("SYC_SKIP_HIR_AFFINE_SCALAR_REWRITE"),
                    analysis::AnalysisInvalidation::CFG,
                    [](Module& m, analysis::AnalysisManager& am) {
                      return hir::applyAffineScalarRewrite(m, am);
                    }});
      }
      // 展开/模板预处理完成后，对剩余的动态 loop band 做分块。
      runHIRPass({"hir-loop-memory", "loop-blocking",
                  options.enableLoopBlocking &&
                      !envFlag("SYC_SKIP_HIR_LOOP_BLOCKING"),
                  analysis::AnalysisInvalidation::CFG,
                  [](Module& m, analysis::AnalysisManager& am) {
                    return hir::applyLoopBlocking(m, am);
                  }});
      // blocking 之后只收紧仿射区域边界（RegionOnly 模式）；若收紧带来变化，
      // 再补一轮仿射标量重写与数组地址递推以暴露新机会。
      const bool preUnrollTighteningChanged =
          runHIRPass({"hir-loop-memory", "affine-loop-tightening-pre-unroll",
                      !envFlag("SYC_SKIP_HIR_AFFINE_LOOP_TIGHTENING") &&
                          envFlag("SYC_ENABLE_HIR_AFFINE_LOOP_TIGHTENING_PRE_UNROLL") &&
                          !envFlag("SYC_SKIP_HIR_AFFINE_LOOP_TIGHTENING_PRE_UNROLL"),
                      analysis::AnalysisInvalidation::CFG,
                      [](Module& m, analysis::AnalysisManager& am) {
                        return hir::applyAffineLoopTightening(
                            m, am, hir::AffineLoopTighteningMode::RegionOnly);
                      }});
      if (preUnrollTighteningChanged) {
        runHIRPass({"hir-loop-memory", "affine-scalar-rewrite-pre-unroll",
                    !envFlag("SYC_SKIP_HIR_AFFINE_SCALAR_REWRITE"),
                    analysis::AnalysisInvalidation::CFG,
                    [](Module& m, analysis::AnalysisManager& am) {
                      return hir::applyAffineScalarRewrite(m, am);
                    }});
        runHIRPass({"hir-loop-memory", "array-address-recurrence-pre-unroll",
                    options.enableArrayRecurrence &&
                        !envFlag("SYC_SKIP_HIR_ARRAY_RECURRENCE_PRE_UNROLL"),
                    analysis::AnalysisInvalidation::CFG,
                    [](Module& m, analysis::AnalysisManager&) {
                      return hir::applyArrayAddressRecurrence(m);
                    }});
      }
      // 先展开真正的小常量 trip 内层循环，再做 affine
      // interior/remainder tightening。否则版本化可能把原本的常量核
      // 改成动态 remainder bound，后续只能展开外层 CFG，容易复制仍含
      // 动态回边的内层循环。
      const bool preTightenUnrollChanged =
          runHIRPass({"hir-loop-memory", "loop-unroll-pre-tightening",
                      options.enableLoopUnroll &&
                          !envFlag("SYC_SKIP_HIR_LOOP_UNROLL"),
                      analysis::AnalysisInvalidation::CFG,
                      [](Module& m, analysis::AnalysisManager& am) {
                        return hir::applyLoopUnroll(m, am);
                      }});
      if (preTightenUnrollChanged) {
        runHIRPass({"hir-loop-memory", "affine-scalar-rewrite-pre-tightening",
                    !envFlag("SYC_SKIP_HIR_AFFINE_SCALAR_REWRITE"),
                    analysis::AnalysisInvalidation::CFG,
                    [](Module& m, analysis::AnalysisManager& am) {
                      return hir::applyAffineScalarRewrite(m, am);
                    }});
      }
      runHIRPass({"hir-loop-memory", "affine-loop-tightening",
                  !envFlag("SYC_SKIP_HIR_AFFINE_LOOP_TIGHTENING"),
                  analysis::AnalysisInvalidation::CFG,
                  [](Module& m, analysis::AnalysisManager& am) {
                    return hir::applyAffineLoopTightening(m, am);
                  }});
#ifdef SYC_ENABLE_RESEARCH_PASSES
      runHIRPass({"hir-loop-memory", "loop-scalar-accumulator",
                  options.enableLoopScalarAccumulator &&
                      envFlag("SYC_ENABLE_HIR_LOOP_SCALAR_ACCUMULATOR") &&
                      !envFlag("SYC_SKIP_HIR_LOOP_SCALAR_ACCUMULATOR"),
                  analysis::AnalysisInvalidation::CFG,
                  [](Module& m, analysis::AnalysisManager& am) {
                    return hir::applyLoopScalarAccumulatorPromotion(m, am);
                  }});
#endif
      runHIRPass({"hir-loop-memory", "affine-scalar-rewrite",
                  !envFlag("SYC_SKIP_HIR_AFFINE_SCALAR_REWRITE"),
                  analysis::AnalysisInvalidation::CFG,
                  [](Module& m, analysis::AnalysisManager& am) {
                    return hir::applyAffineScalarRewrite(m, am);
                  }});
      // 阶段 2.2：循环展开，仅处理较小的常量迭代循环。
      // 在数组地址递推之前执行，使后者看到更简单的 IR。
      const bool loopUnrollChanged =
          runHIRPass({"hir-loop-memory", "loop-unroll",
                      options.enableLoopUnroll &&
                          !envFlag("SYC_SKIP_HIR_LOOP_UNROLL"),
                      analysis::AnalysisInvalidation::CFG,
                      [](Module& m, analysis::AnalysisManager& am) {
                        return hir::applyLoopUnroll(m, am);
                      }});
      if (loopUnrollChanged) {
        // 展开后既会暴露新的地址递推族，也常把原本的
        // load/store 累加器重新摊平成栈槽形状。因此这里固定补两轮：
        // 先重新提升 loop scalar accumulator，再立刻喂给
        // affine-scalar-rewrite，保证 dense-affine 热内层在同一轮里
        // 收敛回 SSA carried scalar + pointer bump。
#ifdef SYC_ENABLE_RESEARCH_PASSES
        runHIRPass({"hir-loop-memory", "loop-scalar-accumulator-post-unroll",
                    options.enableLoopScalarAccumulator &&
                        envFlag("SYC_ENABLE_HIR_LOOP_SCALAR_ACCUMULATOR") &&
                        !envFlag("SYC_SKIP_HIR_LOOP_SCALAR_ACCUMULATOR"),
                    analysis::AnalysisInvalidation::CFG,
                    [](Module& m, analysis::AnalysisManager& am) {
                      return hir::applyLoopScalarAccumulatorPromotion(m, am);
                    }});
#endif
        runHIRPass({"hir-loop-memory", "affine-scalar-rewrite-post-unroll",
                    !envFlag("SYC_SKIP_HIR_AFFINE_SCALAR_REWRITE"),
                    analysis::AnalysisInvalidation::CFG,
                    [](Module& m, analysis::AnalysisManager& am) {
                      return hir::applyAffineScalarRewrite(m, am);
                    }});
      }
      runHIRPass({"hir-loop-memory", "local-store-load-forward",
                  !envFlag("SYC_SKIP_HIR_LOCAL_STORE_LOAD_FORWARD"),
                  analysis::AnalysisInvalidation::CFG,
                  [](Module& m, analysis::AnalysisManager& am) {
                    return hir::applyLocalStoreLoadForward(m, am);
                  }});

      bool iterCleanupChanged = false;
      if (iterChanged) {
        if (options.enableLoadHoist)
          runHIRPass({"hir-loop-memory", "loop-invariant-load-hoist", true,
                      analysis::AnalysisInvalidation::CFG,
                      [](Module& m, analysis::AnalysisManager& am) {
                        return rewrite::applyLoopInvariantLoadHoist(m, am);
                      }});
        iterCleanupChanged |= runCleanupRounds(mod, options, analysisManager, 2,
                                              /*includeLocalLoadForwarding=*/true,
                                              /*includeSingleBlockMem2Reg=*/false,
                                              /*includeDomCSE=*/true);
        iterCleanupChanged |= runMem2RegFixpoint(mod, options, analysisManager, 1,
                                                 /*includeLocalLoadForwarding=*/false,
                                                 /*includeSingleBlockMem2Reg=*/false,
                                                 /*includeDomCSE=*/true);
      }

      loopMemoryChanged |= iterChanged || iterCleanupChanged;
      if (!iterChanged && !iterCleanupChanged)
        break;
    }
  }
  hirChanged |= loopMemoryChanged;
  if (!enableHIRLoopMemory) {
    bool standaloneChanged = false;
    if (options.enableStandaloneLoopUnroll)
      standaloneChanged |= hir::applyLoopUnroll(mod, analysisManager);
    if (options.enableStandaloneAffineScalarRewrite)
      standaloneChanged |= hir::applyAffineScalarRewrite(mod, analysisManager);
#ifdef SYC_ENABLE_RESEARCH_PASSES
    if (options.enableLoopScalarAccumulator &&
        envFlag("SYC_ENABLE_HIR_LOOP_MEMORY_ACCUMULATOR") &&
        !envFlag("SYC_SKIP_HIR_LOOP_SCALAR_ACCUMULATOR") &&
        !envFlag("SYC_SKIP_HIR_LOOP_MEMORY_ACCUMULATOR"))
      standaloneChanged |=
          hir::applyLoopMemoryAccumulatorPromotion(mod, analysisManager);
#endif
    if (standaloneChanged) {
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
      standaloneChanged |= runCleanupRounds(mod, options, analysisManager, 1,
                                            /*includeLocalLoadForwarding=*/true,
                                            /*includeSingleBlockMem2Reg=*/false,
                                            /*includeDomCSE=*/true);
      standaloneChanged |= runMem2RegFixpoint(mod, options, analysisManager, 1,
                                              /*includeLocalLoadForwarding=*/false,
                                              /*includeSingleBlockMem2Reg=*/false,
                                              /*includeDomCSE=*/true);
    }
    hirChanged |= standaloneChanged;
  }
  bool lateChanged = false;
  bool deadGlobalStoreChanged = false;
  bool deadGlobalStoreRan = false;
  auto runDeadGlobalStoreElim = [&]() {
    if (deadGlobalStoreRan)
      return false;
    deadGlobalStoreRan = true;
    if (envFlag("SYC_SKIP_HIR_DEAD_GLOBAL_ARRAY_STORE_ELIM"))
      return false;
    bool passChanged =
        hir::applyDeadGlobalArrayStoreElim(mod, analysisManager);
    if (passChanged)
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::Effects);
    deadGlobalStoreChanged |= passChanged;
    return passChanged;
  };
  // 主 loop-memory 阶段已经完成后再删除全局数组死写；这样不改变主循环
  // 的展开/阻塞决策，同时要早于 post-inline loop-memory 的通用清理，
  // 保留该 pass 自身可复核的证明和 trace。
  lateChanged |= runDeadGlobalStoreElim();

  const std::unordered_set<std::string>* blockedRecursiveFunctions =
      options.blockedPostHIRRecursiveFunctions.empty()
          ? nullptr
          : &options.blockedPostHIRRecursiveFunctions;
  if (!envFlag("SYC_SKIP_COREIR_LATE")) {
    if (!envFlag("SYC_SKIP_COREIR_RECURSIVE_MEMOIZATION"))
      hirChanged |= runRecursiveMemoization();
    hirChanged |= hir::eliminateTailRecursion(mod, blockedRecursiveFunctions);
    if (options.enableArrayRecurrence &&
        !envFlag("SYC_SKIP_HIR_ARRAY_RECURRENCE"))
      hirChanged |= hir::applyArrayAddressRecurrence(mod);
  }
  changed |= hirChanged;
  if (hirChanged)
    analysisManager.invalidateModule(analysis::AnalysisInvalidation::Effects);
  auto runPostInlineLoopMemory = [&]() {
    bool postChanged = false;
    if (!options.enableHIRLoopMemory ||
        envFlag("SYC_SKIP_COREIR_LOOP_MEMORY") ||
        envFlag("SYC_SKIP_COREIR_POST_INLINE_LOOP_MEMORY")) {
      return postChanged;
    }
    if (!envFlag("SYC_SKIP_COREIR_RUNTIME_BOUND_LOCALIZE")) {
      postChanged |= localizeRuntimeScalarGlobalBounds(mod, analysisManager);
    }
    if (!hasHIRLoopMemoryCandidates(mod, analysisManager))
      return postChanged;
    for (int iter = 0; iter < 2; ++iter) {
      bool iterChanged = false;
      auto runPostHIRPass = [&](ModulePassSpec spec) {
        const bool passChanged = runModulePass(spec, mod, analysisManager);
        iterChanged |= passChanged;
        return passChanged;
      };

      // post-inline 窗口依次尝试：循环带规范化、转置视图转发、循环交换、
      // 分块、边界收紧、标量累加提升、仿射标量重写、数组地址递推。
      runPostHIRPass({"post-inline-loop-memory", "loop-band-canonicalize",
                      !envFlag("SYC_SKIP_HIR_LOOP_BAND_CANONICALIZE"),
                      analysis::AnalysisInvalidation::CFG,
                      [](Module& m, analysis::AnalysisManager& am) {
                        return hir::applyLoopBandCanonicalization(m, am);
                      }});
#ifdef SYC_ENABLE_RESEARCH_PASSES
      runPostHIRPass(transactionalResearchPass(
                     {"post-inline-loop-memory", "loop-interchange",
                      options.enableLoopInterchange &&
                          envFlag("SYC_ENABLE_HIR_LOOP_INTERCHANGE") &&
                          !envFlag("SYC_SKIP_HIR_LOOP_INTERCHANGE"),
                      analysis::AnalysisInvalidation::CFG,
                      [](Module& m, analysis::AnalysisManager& am) {
                        return hir::applyLoopInterchange(m, am);
                      }}, "affine-permutation-dependence-proved"));
#endif
      runPostHIRPass({"post-inline-loop-memory", "loop-blocking",
                      options.enableLoopBlocking &&
                          !envFlag("SYC_SKIP_HIR_LOOP_BLOCKING"),
                      analysis::AnalysisInvalidation::CFG,
                      [](Module& m, analysis::AnalysisManager& am) {
                        return hir::applyLoopBlocking(m, am);
                      }});
      runPostHIRPass({"post-inline-loop-memory", "affine-loop-tightening",
                      !envFlag("SYC_SKIP_HIR_AFFINE_LOOP_TIGHTENING"),
                      analysis::AnalysisInvalidation::CFG,
                      [](Module& m, analysis::AnalysisManager& am) {
                        return hir::applyAffineLoopTightening(m, am);
                      }});
#ifdef SYC_ENABLE_RESEARCH_PASSES
      runPostHIRPass(transactionalResearchPass(
                     {"post-inline-loop-memory", "loop-scalar-accumulator",
                      options.enableLoopScalarAccumulator &&
                          envFlag("SYC_ENABLE_HIR_LOOP_SCALAR_ACCUMULATOR") &&
                          !envFlag("SYC_SKIP_HIR_LOOP_SCALAR_ACCUMULATOR"),
                      analysis::AnalysisInvalidation::CFG,
                      [](Module& m, analysis::AnalysisManager& am) {
                        return hir::applyLoopScalarAccumulatorPromotion(m, am);
                      }}, "scalar-accumulator-memory-closure-proved"));
#endif
      runPostHIRPass({"post-inline-loop-memory", "affine-scalar-rewrite",
                      !envFlag("SYC_SKIP_HIR_AFFINE_SCALAR_REWRITE"),
                      analysis::AnalysisInvalidation::CFG,
                      [](Module& m, analysis::AnalysisManager& am) {
                        return hir::applyAffineScalarRewrite(m, am);
                      }});
      runPostHIRPass({"post-inline-loop-memory", "array-address-recurrence",
                      options.enableArrayRecurrence &&
                          !envFlag("SYC_SKIP_HIR_ARRAY_RECURRENCE"),
                      analysis::AnalysisInvalidation::CFG,
                      [](Module& m, analysis::AnalysisManager&) {
                        return hir::applyArrayAddressRecurrence(m);
                      }});

      if (!iterChanged)
        break;
      postChanged = true;
      iterChanged |= runCleanupRounds(mod, options, analysisManager, 1,
                                      /*includeLocalLoadForwarding=*/true,
                                      /*includeSingleBlockMem2Reg=*/false,
                                      /*includeDomCSE=*/true);
      iterChanged |= runMem2RegFixpoint(mod, options, analysisManager, 1,
                                        /*includeLocalLoadForwarding=*/false,
                                        /*includeSingleBlockMem2Reg=*/false,
                                        /*includeDomCSE=*/true);
      postChanged |= iterChanged;
    }
    return postChanged;
  };
  if (!envFlag("SYC_SKIP_COREIR_LATE")) {
    if (options.enableArraySROA && !envFlag("SYC_SKIP_ARRAY_SROA"))
      lateChanged |= rewrite::applyArraySROA(mod);
    lateChanged |= forwardConstGlobalLoads(mod);
    if (!envFlag("SYC_SKIP_COREIR_BIT_ACCUMULATOR"))
      lateChanged |=
          rewrite::applyBitAccumulatorRecognition(mod, analysisManager);
    if (!envFlag("SYC_SKIP_COREIR_BIT_ACCUMULATOR"))
      lateChanged |=
          rewrite::applyPow2DigitHelperRecognition(mod, analysisManager);
    // 递归 modmul 识别依赖通用 SSA/CFG 递推角色和闭合纯度证明；固定
    // 块数约束已移除，默认开启并保留 SYC_SKIP 旁路。
    if (!envFlag("SYC_SKIP_COREIR_RECURSIVE_MODMUL"))
      lateChanged |=
          rewrite::applyRecursiveModMulRecognition(mod, analysisManager);
    if (!envFlag("SYC_SKIP_COREIR_BIT_PATTERNS"))
      lateChanged |= rewrite::applyBitPatternRecognition(mod, analysisManager);
    bool inlineChanged = false;
    if (!sycEnvironmentValue("SYC_SKIP_COREIR_INLINE")) {
      // late inline 是通用 IPO 入口，不能依赖前序 late pass
      // 是否已经改动 IR。很多 helper 在 early cleanup 后才具备稳定的
      // effect/cost 摘要；若这里被 lateChanged 门控住，循环内 helper 调用
      // 会一直留到后端，后续 loop-memory 也无法跨过程消费地址和状态。
      inlineChanged = inlineSmallFunctions(mod, analysisManager);
      lateChanged |= inlineChanged;
    }
    if (!envFlag("SYC_SKIP_COREIR_EARLY_RETURN_CALL_GUARD")) {
      bool guardChanged = applyVoidEarlyReturnCallGuards(mod, analysisManager);
      lateChanged |= guardChanged;
      inlineChanged |= guardChanged;
    }
    if (inlineChanged) {
      // IPO 折平后会暴露新的常量 trip-count 小循环和 affine 地址递推。原先的
      // loop-unroll 阶段可能因为调用尚未内联而跳过，因此这里补一个 post-IPO
      // 小循环窗口，只使用通用 loop/cost 条件。
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
      lateChanged |= loopCanonicalize(mod, analysisManager);
      if (!envFlag("SYC_SKIP_COREIR_MOD_SCALAR_RECURRENCE")) {
        bool modRecChanged = applyModularScalarRecurrences(mod, analysisManager);
        lateChanged |= modRecChanged;
        if (modRecChanged)
          analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
      }
      if (!envFlag("SYC_SKIP_COREIR_REPEATED_REDUCTION")) {
        bool repeatedChanged = applyRepeatedReductionLoops(mod, analysisManager);
        lateChanged |= repeatedChanged;
        if (repeatedChanged)
          analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
      }
      // 与 early 阶段相同，只由通用 loop/cost 条件决定是否物化快路径。
      if (!envFlag("SYC_SKIP_COREIR_LINEAR_RECURRENCE")) {
        bool linRecChanged =
            applySmallLinearRecurrenceLoops(mod, analysisManager);
        lateChanged |= linRecChanged;
        if (linRecChanged)
          analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
      }
      if (!envFlag("SYC_SKIP_COREIR_ROW_LOCALITY")) {
        bool rowLocalChanged = runRowLocality();
        lateChanged |= rowLocalChanged;
        if (rowLocalChanged)
          analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
      }
      if (!envFlag("SYC_SKIP_COREIR_OVERWRITE_LOOP_COLLAPSE") &&
          !envFlag("SYC_SKIP_COREIR_OVERWRITE_LOOP_COLLAPSE")) {
        bool overwriteChanged =
            applyOverwriteRepeatLoopCollapse(mod, analysisManager);
        lateChanged |= overwriteChanged;
        if (overwriteChanged)
          analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
      }
      // 很多性能热循环要等 late inline 折平 helper/过程边界后，
      // 才能暴露出稳定的 affine band、别名和 recurrence 事实。因此这里补
      // 一个保守的 post-inline loop-memory 窗口，仍完全由现有
      // LoopForest/AffineAccess/effect/profit 决策，不回落到 AST 形状入口。
      lateChanged |= runPostInlineLoopMemory();
      if (options.enableLoopUnroll && !envFlag("SYC_SKIP_HIR_LOOP_UNROLL"))
        lateChanged |= hir::applyLoopUnroll(mod, analysisManager);
#ifdef SYC_ENABLE_RESEARCH_PASSES
      if (options.enableLoopScalarAccumulator &&
          envFlag("SYC_ENABLE_HIR_LOOP_SCALAR_ACCUMULATOR") &&
          !envFlag("SYC_SKIP_HIR_LOOP_SCALAR_ACCUMULATOR"))
        lateChanged |= hir::applyLoopScalarAccumulatorPromotion(mod,
                                                                analysisManager);
#endif
      if (!envFlag("SYC_SKIP_HIR_AFFINE_SCALAR_REWRITE"))
        lateChanged |= hir::applyAffineScalarRewrite(mod, analysisManager);
    }
    // 后期变换有改动时：先失效函数副作用分析，再做单赋值全局标量转发
    // 与常量小全局数组加载转发，并提升直接可局部化的全局变量。
    if (lateChanged) {
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::Effects);
      if (lateChanged) {
        const analysis::FunctionEffectMap& effects = analysisManager.functionEffects();
        for (auto& funcPtr : mod.functions())
          lateChanged |= forwardSingleAssignmentScalarGlobals(
              funcPtr.get(), mod, effects);
        lateChanged |= forwardConstantSmallGlobalArrayLoads(mod, effects);
      }
      lateChanged |=
          promoteDirectScalarGlobalsToEntryLocals(mod, analysisManager);
      lateChanged |= removeUnobservedScalarGlobalStores(mod);
      lateChanged |= basicBlockCSE(mod, analysisManager);
      lateChanged |= dominatorTreeCSE(mod, analysisManager);
      lateChanged |= loopLocalValueNumbering(mod, analysisManager);
      if (options.enableArraySROA && !envFlag("SYC_SKIP_ARRAY_SROA"))
        lateChanged |= rewrite::applyArraySROA(mod);
    }
  }
  if (!deadGlobalStoreRan)
    lateChanged |= runDeadGlobalStoreElim();
  changed |= lateChanged;
  // 收尾窗口：先做循环不变量 load/store 外提与 LICM，再重做全局转发/提升
  // 与块级、支配树 CSE，最后用两轮清理迭代收敛。
  bool cleanupChanged = false;
  if ((hirChanged || lateChanged || deadGlobalStoreChanged) &&
      !envFlag("SYC_SKIP_COREIR_LATE")) {
    if (options.enableLoadHoist) {
      cleanupChanged |=
          rewrite::applyLoopInvariantLoadHoist(mod, analysisManager);
      cleanupChanged |=
          rewrite::applyLoopInvariantStoreHoist(mod, analysisManager);
    }
    if (!envFlag("SYC_SKIP_COREIR_LATE_LICM"))
      cleanupChanged |= loopInvariantCodeMotion(mod, analysisManager);
    if (hirChanged || lateChanged) {
      const analysis::FunctionEffectMap& effects = analysisManager.functionEffects();
      for (auto& funcPtr : mod.functions())
        cleanupChanged |= forwardSingleAssignmentScalarGlobals(
            funcPtr.get(), mod, effects);
      cleanupChanged |= forwardConstantSmallGlobalArrayLoads(mod, effects);
    }
    cleanupChanged |=
        promoteDirectScalarGlobalsToEntryLocals(mod, analysisManager);
    cleanupChanged |= removeUnobservedScalarGlobalStores(mod);
    cleanupChanged |= runCleanupRounds(mod, options, analysisManager, 2,
                                       /*includeLocalLoadForwarding=*/true,
                                       /*includeSingleBlockMem2Reg=*/false,
                                       /*includeDomCSE=*/true);
    if (options.enableArraySROA && !envFlag("SYC_SKIP_ARRAY_SROA"))
      cleanupChanged |= rewrite::applyArraySROA(mod);
    cleanupChanged |= forwardConstGlobalLoads(mod);
    cleanupChanged |= runMem2RegFixpoint(mod, options, analysisManager, 2,
                                         /*includeLocalLoadForwarding=*/false,
                                         /*includeSingleBlockMem2Reg=*/false,
                                         /*includeDomCSE=*/true);
    if (!envFlag("SYC_SKIP_COREIR_RECURSIVE_MEMOIZATION"))
      cleanupChanged |= runRecursiveMemoization();
    cleanupChanged |= hir::eliminateTailRecursion(
        mod, blockedRecursiveFunctions);
    changed |= cleanupChanged;
  }
  if ((hirChanged || lateChanged || deadGlobalStoreChanged || cleanupChanged ||
       (options.enableArraySROA && !envFlag("SYC_SKIP_ARRAY_SROA"))) &&
      !envFlag("SYC_SKIP_COREIR_LATE")) {
    analysisManager.invalidateModule(analysis::AnalysisInvalidation::Effects);
    changed |= runMem2RegFixpoint(mod, options, analysisManager, 3,
                                  /*includeLocalLoadForwarding=*/true,
                                  /*includeSingleBlockMem2Reg=*/true,
                                  /*includeDomCSE=*/true);
  }
  if (!envFlag("SYC_SKIP_COREIR_CORRELATED_VALUE_PROPAGATION")) {
    const bool rangeChanged =
        applyCorrelatedValuePropagation(mod, analysisManager);
    if (rangeChanged) {
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
      changed = true;
      changed |= runCleanupRounds(mod, options, analysisManager, 1,
                                  /*includeLocalLoadForwarding=*/true,
                                  /*includeSingleBlockMem2Reg=*/false,
                                  /*includeDomCSE=*/true);
    }
  }
  if (!envFlag("SYC_SKIP_COREIR_BOUNDED_SIGNED_REM")) {
    bool boundedRemChanged = applyBoundedSignedModuloAdd(mod);
    if (boundedRemChanged) {
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::Effects);
      changed = true;
      changed |= constFold(mod);
      changed |= dce(mod, analysisManager);
      changed |= basicBlockCSE(mod, analysisManager);
      changed |= dominatorTreeCSE(mod, analysisManager);
      changed |= dce(mod, analysisManager);
    }
  }
  bool ifConversionChanged = !envFlag("SYC_SKIP_COREIR_IFCONV") &&
                             ifConvertSmallDiamonds(mod);
  if (ifConversionChanged) {
    // 小 diamond 的 if-conversion 会删除分支块并生成新的
    // 选择链，必须马上做 CFG/DCE 清理，避免死块干扰后端。
    analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
    changed = true;
    changed |= constFold(mod);
    changed |= simplifyCFG(mod);
    changed |= mergeLinearBlocks(mod);
    changed |= dce(mod, analysisManager);
    changed |= basicBlockCSE(mod, analysisManager);
    changed |= dominatorTreeCSE(mod, analysisManager);
    changed |= dce(mod, analysisManager);
  }
  // stencil 内部快路径：循环规范形 + tap 角色
  // 匹配 + 运行时 interiorCond 值守回原路径，默认开启 + SKIP 旁路。
#ifdef SYC_ENABLE_RESEARCH_PASSES
  if (envFlag("SYC_ENABLE_COREIR_STENCIL_INTERIOR") &&
      !envFlag("SYC_SKIP_COREIR_STENCIL_INTERIOR")) {
    bool stencilChanged = runTransactionalStencil();
    if (stencilChanged) {
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
      changed = true;
      // stencil 构造器分别生成每个 tap。物化快路径后运行一次普通清理，使相邻
      // tap 产生的重复行偏移和行基址乘积在寄存器分配前共享。
      changed |= runCleanupRounds(mod, options, analysisManager, 1,
                                  /*includeLocalLoadForwarding=*/true,
                                  /*includeSingleBlockMem2Reg=*/false,
                                  /*includeDomCSE=*/true);
    }
  }
#endif
  // 位模拟闭式表达式先参与完整的 IPO 与循环优化，最后再物化非负输入
  // 快路径；这样既保留负数慢路径语义，也不让早期分支阻断内联和循环分析。
  if (options.enableLoopUnroll && !envFlag("SYC_SKIP_HIR_LOOP_UNROLL"))
    changed |= hir::applySingleTripLoopUnroll(mod, analysisManager);
  // 删除完全不逃逸、无任何读取者且所有写范围均位于对象内的局部
  // materialization。候选在 clone 中执行并经过真实 RV64 admission；未知
  // pointer use、动态范围或机器无收益均保持 baseline。
  changed |= planning::runDeadLocalMaterializationCandidate(
      mod, analysisManager);
  changed |= constFold(mod);
  changed |= dce(mod, analysisManager);
  if (!envFlag("SYC_SKIP_COREIR_BIT_ACCUMULATOR") &&
      rewrite::lowerSignedBitClosureFastPaths(mod)) {
    analysisManager.invalidateModule(analysis::AnalysisInvalidation::CFG);
    changed = true;
    changed |= dce(mod, analysisManager);
  }
  if (!envFlag("SYC_SKIP_COREIR_BRANCH_COMPARE_SPLIT"))
    changed |= splitSharedBranchComparisons(mod);
  // 为后端保留 SSA 形式的 PHI，同时规范化输入边，使边上的复制消解不必处理
  // 临界边。
  changed |= normalizePhiEdges(mod);
  // 在降低前移除重复的标量全局重载存储，使寄存器分配面对更小的分配前数据流。
  // 下面的分配后阶段继续处理只有获得物理寄存器和地址来源后才能发现的情况。
  analysisManager.invalidateModule(analysis::AnalysisInvalidation::Module);
  changed |= removeRedundantScalarGlobalReloadStores(mod, analysisManager);
  verifyModule(mod);
  if (!envFlag("SYC_SKIP_COREIR_DEAD_FUNCTION_ELIMINATION") &&
      eliminateUnreachableFunctions(mod)) {
    analysisManager.invalidateModule(analysis::AnalysisInvalidation::Module);
    changed = true;
    verifyModule(mod);
  }
#ifdef SYC_ENABLE_RESEARCH_PASSES
  if (sycEnvironmentValue("SYC_REPORT_CANDIDATE_SUMMARY"))
    planning::dumpCandidateStatistics(std::cerr);
#endif
  return changed;
}

} // namespace core_ir
