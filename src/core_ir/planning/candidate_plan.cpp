// 文件职责：实现候选失败不污染原模块的事务式评估与提交。
#include "candidate_plan.h"

#include "../passes/verify.h"
#include "../../research_switch.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <ostream>
#include <sstream>
#include <utility>

namespace core_ir::planning {

namespace {

std::string snapshot(const Module& module) {
  std::ostringstream output;
  printModule(output, module);
  return output.str();
}

CandidateStatistics& mutableCandidateStatistics() {
  static CandidateStatistics statistics;
  return statistics;
}

enum class RejectionClass { Proof, Transform, Verify, Machine };

void recordEvaluation(const std::string& passName,
                      const CandidateEvaluation& result,
                      RejectionClass rejectionClass) {
  auto& statistics = mutableCandidateStatistics()[
      passName.empty() ? std::string("unnamed-candidate") : passName];
  ++statistics.candidates;
  statistics.evaluationMicros += result.evaluationMicros;
  if (result.admitted) {
    ++statistics.admitted;
    statistics.admittedIRGrowthBytes += result.irGrowthBytes;
    return;
  }
  switch (rejectionClass) {
  case RejectionClass::Proof:
    ++statistics.proofRejected;
    break;
  case RejectionClass::Transform:
    ++statistics.transformRejected;
    break;
  case RejectionClass::Verify:
    ++statistics.verifyRejected;
    break;
  case RejectionClass::Machine:
    ++statistics.machineRejected;
    break;
  }
}

} // namespace

const CandidateStatistics& candidateStatistics() {
  return mutableCandidateStatistics();
}

void resetCandidateStatistics() { mutableCandidateStatistics().clear(); }

void dumpCandidateStatistics(std::ostream& output) {
  std::vector<std::string> names;
  names.reserve(mutableCandidateStatistics().size());
  for (const auto& entry : mutableCandidateStatistics())
    names.push_back(entry.first);
  std::sort(names.begin(), names.end());
  for (const auto& name : names) {
    const auto& item = mutableCandidateStatistics().at(name);
    output << "candidate-summary pass=" << name
           << " candidates=" << item.candidates
           << " proof-rejected=" << item.proofRejected
           << " transform-rejected=" << item.transformRejected
           << " verify-rejected=" << item.verifyRejected
           << " machine-rejected=" << item.machineRejected
           << " admitted=" << item.admitted
           << " committed=" << item.committed
           << " evaluation-us=" << item.evaluationMicros
           << " admitted-ir-growth=" << item.admittedIRGrowthBytes << "\n";
  }
}

CandidateTransaction::CandidateTransaction(Module& baseline, CandidatePlan plan)
    : baseline_(baseline), plan_(std::move(plan)),
      baselineSnapshot_(snapshot(baseline)) {}

CandidateEvaluation CandidateTransaction::evaluate() {
  const auto started = std::chrono::steady_clock::now();
  result_ = {};
  result_.discovered = true;
  result_.baselineIRBytes = baselineSnapshot_.size();
  candidate_.reset();
  auto reject = [&](RejectionClass rejectionClass) {
    result_.evaluationMicros = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - started)
            .count());
    recordEvaluation(plan_.passName, result_, rejectionClass);
    return result_;
  };
  if (!plan_.legality.proved()) {
    result_.reasonCode = plan_.legality.reasonCode.empty()
                             ? "legality-not-proved"
                             : plan_.legality.reasonCode;
    result_.detail = plan_.legality.detail;
    return reject(RejectionClass::Proof);
  }
  for (const auto& fact : plan_.proofFacts) {
    if (fact.proved()) continue;
    result_.reasonCode = fact.reasonCode.empty() ? "proof-fact-not-proved"
                                                 : fact.reasonCode;
    result_.detail = fact.detail;
    return reject(RejectionClass::Proof);
  }
  result_.proved = true;
  if (!plan_.transform) {
    result_.reasonCode = "missing-transform";
    return reject(RejectionClass::Transform);
  }
  if (snapshot(baseline_) != baselineSnapshot_) {
    result_.reasonCode = "baseline-mutated-before-evaluation";
    return reject(RejectionClass::Transform);
  }
  try {
    candidate_ = cloneModuleForPlanning(baseline_);
  } catch (const std::exception& error) {
    result_.reasonCode = "planning-clone-failed";
    result_.detail = error.what();
    return reject(RejectionClass::Transform);
  }
  std::string transformReason;
  if (!plan_.transform(*candidate_, transformReason)) {
    candidate_.reset();
    result_.reasonCode = "candidate-transform-rejected";
    result_.detail = std::move(transformReason);
    return reject(RejectionClass::Transform);
  }
  result_.transformed = true;
  const std::string candidateSnapshot = snapshot(candidate_->module());
  result_.candidateIRBytes = candidateSnapshot.size();
  result_.irGrowthBytes = static_cast<std::int64_t>(result_.candidateIRBytes) -
                          static_cast<std::int64_t>(result_.baselineIRBytes);
  auto verification = verifyModuleChecked(candidate_->module());
  if (!verification.ok) {
    if (sycEnvironmentValue("SYC_DUMP_REJECTED_CANDIDATE")) {
      std::cerr << "[planning] rejected-candidate pass=" << plan_.passName
                << " verifier=" << verification.reasonCode << "\n";
      printModule(std::cerr, candidate_->module());
    }
    candidate_.reset();
    result_.reasonCode = "candidate-ir-verifier-failed";
    result_.detail = verification.reasonCode + ": " + verification.detail;
    return reject(RejectionClass::Verify);
  }
  result_.verified = true;
  auto baselineMachine = mir::runMachinePipeline(
      mir::lowerForPlanning(baseline_), mir::MachinePipelineMode::Planning);
  auto candidateMachine = mir::runMachinePipeline(
      mir::lowerForPlanning(candidate_->module()),
      mir::MachinePipelineMode::Planning);
  if (!baselineMachine.verification.ok || !baselineMachine.module) {
    candidate_.reset();
    result_.reasonCode = "baseline-machine-verifier-failed";
    result_.detail = baselineMachine.verification.reasonCode + ": " +
                     baselineMachine.verification.detail;
    return reject(RejectionClass::Machine);
  }
  if (!candidateMachine.verification.ok || !candidateMachine.module) {
    candidate_.reset();
    result_.reasonCode = "candidate-machine-verifier-failed";
    result_.detail = candidateMachine.verification.reasonCode + ": " +
                     candidateMachine.verification.detail;
    return reject(RejectionClass::Machine);
  }
  auto baselineEmission = mir::emitChecked(*baselineMachine.module, baseline_);
  if (!baselineEmission.ok) {
    candidate_.reset();
    result_.reasonCode = "baseline-assembly-emission-failed";
    result_.detail = baselineEmission.reasonCode + ": " +
                     baselineEmission.detail;
    return reject(RejectionClass::Machine);
  }
  auto candidateEmission =
      mir::emitChecked(*candidateMachine.module, candidate_->module());
  if (!candidateEmission.ok) {
    candidate_.reset();
    result_.reasonCode = "candidate-assembly-emission-failed";
    result_.detail = candidateEmission.reasonCode + ": " +
                     candidateEmission.detail;
    return reject(RejectionClass::Machine);
  }
  baselineMachine.metrics.emittedAssemblyBytes =
      baselineEmission.assembly.size();
  candidateMachine.metrics.emittedAssemblyBytes =
      candidateEmission.assembly.size();
  if (snapshot(baseline_) != baselineSnapshot_) {
    candidate_.reset();
    result_.reasonCode = "baseline-mutated-during-evaluation";
    return reject(RejectionClass::Machine);
  }
  result_.baselineMetrics = baselineMachine.metrics;
  result_.candidateMetrics = candidateMachine.metrics;
  result_.admission = mir::admitRV64Candidate(
      baselineMachine, candidateMachine, plan_.admissionPolicy);
  if (!result_.admission.accepted) {
    candidate_.reset();
    result_.reasonCode = result_.admission.reasonCode;
    return reject(RejectionClass::Machine);
  }
  result_.admitted = true;
  result_.accepted = true;
  result_.reasonCode = "accepted";
  result_.evaluationMicros = static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::steady_clock::now() - started)
          .count());
  recordEvaluation(plan_.passName, result_, RejectionClass::Machine);
  return result_;
}

bool CandidateTransaction::commit() {
  if (!result_.accepted || !candidate_ || result_.committed) return false;
  auto replacement = candidate_->release();
  if (!replacement) return false;
  baseline_.replaceWith(std::move(*replacement));
  // 当前 PlanningClone 的 commit 粒度仍是整个 Module。即使候选只改 Values，
  // replaceWith 也会销毁所有旧 Function/Block/Instruction 地址，因此必须清空
  // 全部指针键缓存；只有未来实现真正的 function/region 局部替换后，才能按
  // plan_.invalidation 使用细粒度失效。
  if (plan_.analysisManager)
    plan_.analysisManager->invalidateModule(
        analysis::AnalysisInvalidation::Module);
  result_.committed = true;
  auto& statistics = mutableCandidateStatistics()[
      plan_.passName.empty() ? std::string("unnamed-candidate")
                             : plan_.passName];
  ++statistics.committed;
  candidate_.reset();
  return true;
}

} // namespace core_ir::planning
