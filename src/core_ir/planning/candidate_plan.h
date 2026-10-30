// 文件职责：统一 discover/prove/clone/verify/machine-admit/commit 事务协议。
#pragma once

#include "planning_clone.h"
#include "analysis/analysis_manager.h"
#include "analysis/proof_result.h"
#include "backend_mir/rv64_admission.h"

#include <functional>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace core_ir::planning {

struct CandidatePlan {
  std::string passName;
  Function* inputFunction = nullptr;
  std::vector<BasicBlock*> inputRegion;
  std::vector<Instruction*> observers;
  analysis::AnalysisManager* analysisManager = nullptr;
  analysis::ProofResult legality;
  std::vector<analysis::ProofResult> proofFacts;
  analysis::AnalysisInvalidation invalidation =
      analysis::AnalysisInvalidation::Module;
  std::string targetCostSummary;
  std::function<bool(PlanningClone&, std::string&)> transform;
  mir::RV64AdmissionPolicy admissionPolicy;
};

struct CandidateEvaluation {
  // 阶段位只表示候选确实完成了该阶段；拒绝时后续阶段保持 false，避免把
  // legality、transform、verifier 和 machine admission 混在一个布尔值中。
  bool discovered = false;
  bool proved = false;
  bool transformed = false;
  bool verified = false;
  bool admitted = false;
  bool accepted = false;
  bool committed = false;
  std::string reasonCode;
  std::string detail;
  std::size_t baselineIRBytes = 0;
  std::size_t candidateIRBytes = 0;
  std::int64_t irGrowthBytes = 0;
  std::uint64_t evaluationMicros = 0;
  mir::MachineMetrics baselineMetrics;
  mir::MachineMetrics candidateMetrics;
  mir::RV64AdmissionResult admission;
};

struct CandidatePassStatistics {
  std::uint64_t candidates = 0;
  std::uint64_t proofRejected = 0;
  std::uint64_t transformRejected = 0;
  std::uint64_t verifyRejected = 0;
  std::uint64_t machineRejected = 0;
  std::uint64_t admitted = 0;
  std::uint64_t committed = 0;
  std::uint64_t evaluationMicros = 0;
  std::int64_t admittedIRGrowthBytes = 0;
};

using CandidateStatistics =
    std::unordered_map<std::string, CandidatePassStatistics>;

const CandidateStatistics& candidateStatistics();
void resetCandidateStatistics();
void dumpCandidateStatistics(std::ostream& output);

class CandidateTransaction {
public:
  CandidateTransaction(Module& baseline, CandidatePlan plan);
  CandidateEvaluation evaluate();
  bool commit();
  const CandidateEvaluation& result() const { return result_; }

private:
  Module& baseline_;
  CandidatePlan plan_;
  CandidateEvaluation result_;
  std::unique_ptr<PlanningClone> candidate_;
  std::string baselineSnapshot_;
};

} // namespace core_ir::planning
