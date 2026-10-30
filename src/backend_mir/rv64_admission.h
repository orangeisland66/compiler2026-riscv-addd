// 文件职责：比较 baseline/candidate 的真实 RV64 MIR 指标并稳定给出准入原因。
#pragma once

#include "mir_backend.h"
#include "rv64_cost_model.h"

#include <string>

namespace mir {

struct RV64AdmissionPolicy {
  std::size_t maxFrameGrowthBytes = 0;
  std::size_t maxSpillGrowth = 0;
  int maxPressureGrowth = 0;
  int maxCodeGrowthPercent = 10;
  int maxCriticalPathGrowthPercent = 10;
  int minimumWeightedCostReductionPercent = 1;
  std::size_t maxFrameBytes = 1 << 20;
};

struct RV64AdmissionResult {
  bool accepted = false;
  std::string reasonCode;
  long long baselineCost = 0;
  long long candidateCost = 0;
};

long long estimateRV64MachineCost(const MachineModule& module);
RV64AdmissionResult admitRV64Candidate(
    const MachinePipelineResult& baseline,
    const MachinePipelineResult& candidate,
    const RV64AdmissionPolicy& policy = {});

} // namespace mir
