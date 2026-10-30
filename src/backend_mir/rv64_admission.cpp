// 文件职责：实现 fail-closed 的 RV64 机器候选准入策略。
#include "rv64_admission.h"

#include <algorithm>

namespace mir {

long long estimateRV64MachineCost(const MachineModule& module) {
  long long cost = 0;
  for (const auto& function : module.functions()) {
    for (const auto& block : function->blocks()) {
      for (const auto& instruction : block->insts()) {
        const auto item = rv64InstructionCost(instruction->opcode());
        cost += std::max(item.latency, item.reciprocalThroughput);
      }
    }
  }
  return cost;
}

RV64AdmissionResult admitRV64Candidate(
    const MachinePipelineResult& baseline,
    const MachinePipelineResult& candidate,
    const RV64AdmissionPolicy& policy) {
  RV64AdmissionResult out;
  if (!baseline.module || !baseline.verification.ok) {
    out.reasonCode = "baseline-invalid";
    return out;
  }
  if (!candidate.module || !candidate.verification.ok) {
    out.reasonCode = "candidate-verifier-failed";
    return out;
  }
  out.baselineCost = estimateRV64MachineCost(*baseline.module);
  out.candidateCost = estimateRV64MachineCost(*candidate.module);
  const auto& b = baseline.metrics;
  const auto& c = candidate.metrics;
  if (c.frameBytes > policy.maxFrameBytes) {
    out.reasonCode = "frame-limit-exceeded";
    return out;
  }
  if (c.spillSlots > b.spillSlots + policy.maxSpillGrowth ||
      c.spillInstructions + c.reloadInstructions >
          b.spillInstructions + b.reloadInstructions + policy.maxSpillGrowth) {
    out.reasonCode = "spill-growth";
    return out;
  }
  if (c.frameBytes > b.frameBytes + policy.maxFrameGrowthBytes) {
    out.reasonCode = "frame-growth";
    return out;
  }
  const auto& target = defaultRV64GCTarget();
  // A higher pre-RA peak is not itself a regression while it still fits the
  // allocator's physical pool. The spill/frame checks above consume the
  // post-RA result and remain authoritative when pressure exceeds capacity.
  const bool gprPressureOverflow =
      c.peakGPRPressure > b.peakGPRPressure + policy.maxPressureGrowth &&
      c.peakGPRPressure > target.allocatableGPRs;
  const bool fprPressureOverflow =
      c.peakFPRPressure > b.peakFPRPressure + policy.maxPressureGrowth &&
      c.peakFPRPressure > target.allocatableFPRs;
  if (gprPressureOverflow || fprPressureOverflow) {
    out.reasonCode = "register-pressure-growth";
    return out;
  }
  if (b.instructions != 0 &&
      c.instructions * 100 >
          b.instructions * (100 + policy.maxCodeGrowthPercent)) {
    out.reasonCode = "code-growth";
    return out;
  }
  if (b.estimatedCriticalPath != 0 &&
      c.estimatedCriticalPath * 100 >
          b.estimatedCriticalPath *
              (100 + policy.maxCriticalPathGrowthPercent)) {
    out.reasonCode = "critical-path-growth";
    return out;
  }
  if (c.cyclicCFGEdges > b.cyclicCFGEdges) {
    out.reasonCode = "cyclic-cfg-growth";
    return out;
  }
  // Assembly size captures directive/data effects that instruction counting
  // misses.  A zero value means the caller deliberately skipped emission, so
  // keep the MIR-only API useful for diagnostics and unit-level callers.
  if (b.emittedAssemblyBytes != 0 &&
      c.emittedAssemblyBytes * 100 >
          b.emittedAssemblyBytes * (100 + policy.maxCodeGrowthPercent)) {
    out.reasonCode = "assembly-size-growth";
    return out;
  }
  if (out.baselineCost <= 0 ||
      out.candidateCost * 100 >
          out.baselineCost *
              (100 - policy.minimumWeightedCostReductionPercent)) {
    out.reasonCode = "no-machine-benefit";
    return out;
  }
  out.accepted = true;
  out.reasonCode = "accepted";
  return out;
}

} // namespace mir
