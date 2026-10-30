// 文件职责：在 planning clone 内删除已证明的纯值子图，失败时不修改原模块。
#include "region_deletion.h"

#include <algorithm>
#include <unordered_set>

namespace core_ir::planning {

CandidatePlan makeRegionValueDeletionCandidate(
    Function* function,
    const analysis::RegionValueDeletionProof& deletion,
    analysis::AnalysisManager* analysisManager) {
  CandidatePlan candidate;
  candidate.passName = "region-value-demand-deletion";
  candidate.inputFunction = function;
  candidate.analysisManager = analysisManager;
  candidate.invalidation = analysis::AnalysisInvalidation::Module;
  candidate.legality = deletion.proof;
  const auto sourceInstructions = deletion.instructions;
  candidate.transform =
      [sourceInstructions](PlanningClone& clone, std::string& reason) {
        std::vector<Instruction*> mapped;
        std::unordered_set<Instruction*> mappedSet;
        for (auto* source : sourceInstructions) {
          auto* instruction = dynamic_cast<Instruction*>(clone.mapped(source));
          if (!instruction || !instruction->parent()) {
            reason = "region-deletion-mapping-missing";
            return false;
          }
          mapped.push_back(instruction);
          mappedSet.insert(instruction);
        }
        for (auto* instruction : mapped) {
          for (auto* user : instruction->uses()) {
            if (!user || !mappedSet.count(user)) {
              reason = "region-deletion-use-closure-changed";
              return false;
            }
          }
        }
        // 先断开整个子图的 use-def，再交由各基本块释放所有权；这样 Phi/cycle
        // 也不会依赖偶然的遍历顺序。
        for (auto* instruction : mapped)
          instruction->dropAllOperands();
        for (auto* instruction : mapped)
          instruction->parent()->remove(instruction);
        return !mapped.empty();
      };
  return candidate;
}

CandidatePlan makeDeadLocalMaterializationCandidate(
    const std::vector<analysis::DeadLocalMaterializationProof>& proofs,
    analysis::AnalysisManager* analysisManager) {
  return makeLocalMaterializationSlicingCandidate(proofs, {},
                                                   analysisManager);
}

CandidatePlan makeLocalMaterializationSlicingCandidate(
    const std::vector<analysis::DeadLocalMaterializationProof>& deadProofs,
    const std::vector<analysis::PartialLocalMaterializationProof>&
        partialProofs,
    analysis::AnalysisManager* analysisManager) {
  CandidatePlan candidate;
  candidate.passName = "local-materialization-slicing";
  candidate.analysisManager = analysisManager;
  candidate.invalidation = analysis::AnalysisInvalidation::Module;
  candidate.legality = deadProofs.empty() && partialProofs.empty()
                           ? analysis::ProofResult::disproved(
                                 "dead-local-no-candidate")
                           : analysis::ProofResult::success(
                                 "dead-local-candidates-proved");
  std::vector<Instruction*> sourceInstructions;
  std::unordered_set<Instruction*> unique;
  for (const auto& proof : deadProofs) {
    candidate.proofFacts.push_back(proof.proof);
    for (auto* instruction : proof.instructions)
      if (instruction && unique.insert(instruction).second)
        sourceInstructions.push_back(instruction);
  }
  for (const auto& proof : partialProofs) {
    candidate.proofFacts.push_back(proof.proof);
    for (auto* instruction : proof.storesToDelete)
      if (instruction && unique.insert(instruction).second)
        sourceInstructions.push_back(instruction);
  }
  candidate.transform =
      [sourceInstructions](PlanningClone& clone, std::string& reason) {
        std::vector<Instruction*> mapped;
        mapped.reserve(sourceInstructions.size());
        for (auto* source : sourceInstructions) {
          auto* instruction = dynamic_cast<Instruction*>(clone.mapped(source));
          if (!instruction || !instruction->parent()) {
            reason = "dead-local-mapping-missing";
            return false;
          }
          mapped.push_back(instruction);
        }
        for (auto* instruction : mapped)
          instruction->dropAllOperands();
        // 先删除 store/地址派生，再删除 alloca；排序不参与合法性，只保证日志和
        // Value 生命周期在不同进程中确定。
        std::sort(mapped.begin(), mapped.end(), [](const auto* lhs,
                                                   const auto* rhs) {
          if ((lhs->opcode() == Opcode::Alloca) !=
              (rhs->opcode() == Opcode::Alloca))
            return lhs->opcode() != Opcode::Alloca;
          return lhs->id() < rhs->id();
        });
        for (auto* instruction : mapped)
          instruction->parent()->remove(instruction);
        return !mapped.empty();
      };
  return candidate;
}

bool runDeadLocalMaterializationCandidate(
    Module& module, analysis::AnalysisManager& analysisManager,
    CandidateEvaluation* evaluation) {
  auto proofs = analysis::findDeadLocalMaterializations(module);
  auto partialProofs = analysis::findPartialLocalMaterializations(module);
  if (proofs.empty() && partialProofs.empty()) {
    if (evaluation) {
      *evaluation = {};
      evaluation->discovered = false;
      evaluation->reasonCode = "dead-local-no-candidate";
    }
    return false;
  }
  CandidateTransaction transaction(
      module,
      makeLocalMaterializationSlicingCandidate(
          proofs, partialProofs, &analysisManager));
  const auto result = transaction.evaluate();
  if (evaluation)
    *evaluation = result;
  return result.accepted && transaction.commit();
}

} // namespace core_ir::planning
