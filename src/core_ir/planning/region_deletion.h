// 文件职责：把已证明的 region dead-value 子图包装为事务式 RV64 候选。
#pragma once

#include "candidate_plan.h"
#include "analysis/dead_local_materialization.h"
#include "analysis/observer_chain_plan.h"

namespace core_ir::planning {

CandidatePlan makeRegionValueDeletionCandidate(
    Function* function,
    const analysis::RegionValueDeletionProof& deletion,
    analysis::AnalysisManager* analysisManager = nullptr);

CandidatePlan makeDeadLocalMaterializationCandidate(
    const std::vector<analysis::DeadLocalMaterializationProof>& proofs,
    analysis::AnalysisManager* analysisManager = nullptr);

CandidatePlan makeLocalMaterializationSlicingCandidate(
    const std::vector<analysis::DeadLocalMaterializationProof>& deadProofs,
    const std::vector<analysis::PartialLocalMaterializationProof>&
        partialProofs,
    analysis::AnalysisManager* analysisManager = nullptr);

bool runDeadLocalMaterializationCandidate(
    Module& module, analysis::AnalysisManager& analysisManager,
    CandidateEvaluation* evaluation = nullptr);

} // namespace core_ir::planning
