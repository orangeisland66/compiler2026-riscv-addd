// 文件职责：实现模块级 pass 执行器，并在 pass 修改 IR 后统一失效分析缓存。
#include "pass_manager.h"

#include "../planning/candidate_plan.h"
#include "../../research_switch.h"

#include <iostream>
#include <stdexcept>
#include <utility>

namespace core_ir {

void ModulePassManager::add(ModulePassSpec spec) {
  passes_.push_back(std::move(spec));
}

bool ModulePassManager::run(Module& mod,
                            analysis::AnalysisManager& analysisManager) const {
  bool changed = false;
  for (const auto& pass : passes_) {
    changed |= runModulePass(pass, mod, analysisManager);
  }
  return changed;
}

bool runModulePass(const ModulePassSpec& spec,
                   Module& mod,
                   analysis::AnalysisManager& analysisManager) {
  if (!spec.enabled)
    return false;
  if (!spec.run)
    throw std::runtime_error("empty module pass: " + spec.name);
  if (spec.transactional) {
    planning::CandidatePlan plan;
    plan.passName = spec.name;
    plan.analysisManager = &analysisManager;
    plan.legality = spec.legality;
    plan.invalidation = spec.invalidation;
    plan.transform = [&](planning::PlanningClone& clone,
                         std::string& reason) {
      analysis::AnalysisManager candidateAnalysis(clone.module());
      if (spec.run(clone.module(), candidateAnalysis)) return true;
      reason = "pass-produced-no-candidate";
      return false;
    };
    planning::CandidateTransaction transaction(mod, std::move(plan));
    const auto evaluation = transaction.evaluate();
    if (sycEnvironmentValue("SYC_TRACE_TRANSACTIONAL_PASSES")) {
      std::cerr << "[planning] stage=" << spec.stage
                << " pass=" << spec.name
                << " reason=" << evaluation.reasonCode
                << " discovered=" << evaluation.discovered
                << " proved=" << evaluation.proved
                << " transformed=" << evaluation.transformed
                << " verified=" << evaluation.verified
                << " admitted=" << evaluation.admitted;
      if (!evaluation.detail.empty())
        std::cerr << " detail=" << evaluation.detail;
      if (evaluation.transformed) {
        const auto& baseline = evaluation.baselineMetrics;
        const auto& candidate = evaluation.candidateMetrics;
        std::cerr << " gpr=" << baseline.peakGPRPressure << "->"
                  << candidate.peakGPRPressure
                  << " fpr=" << baseline.peakFPRPressure << "->"
                  << candidate.peakFPRPressure
                  << " spill=" << baseline.spillInstructions +
                                        baseline.reloadInstructions
                  << "->" << candidate.spillInstructions +
                                  candidate.reloadInstructions
                  << " frame=" << baseline.frameBytes << "->"
                  << candidate.frameBytes
                  << " inst=" << baseline.instructions << "->"
                  << candidate.instructions
                  << " hot-inst=" << baseline.hotLoopInstructions << "->"
                  << candidate.hotLoopInstructions
                  << " hot-load=" << baseline.hotLoopLoads << "->"
                  << candidate.hotLoopLoads
                  << " hot-store=" << baseline.hotLoopStores << "->"
                  << candidate.hotLoopStores
                  << " hot-branch=" << baseline.hotLoopBranches << "->"
                  << candidate.hotLoopBranches;
      }
      std::cerr << "\n";
    }
    return evaluation.accepted && transaction.commit();
  }
  const bool changed = spec.run(mod, analysisManager);
  if (changed)
    analysisManager.invalidateModule(spec.invalidation);
  return changed;
}

} // namespace core_ir
