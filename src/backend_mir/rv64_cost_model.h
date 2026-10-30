// 文件职责：描述 RV64GC 能力和通用机器成本，不包含 benchmark/符号特判。
#pragma once

#include "machine_ir.h"

namespace mir {

enum class RV64TargetProfileKind {
  GenericRV64GC,
  BoomRV64GC,
};

struct RV64TargetInfo {
  const char* profileName = "generic-rv64gc-conservative";
  const char* modelRevision = "2026-08-13-v1";
  bool hasI = true;
  bool hasM = true;
  bool hasA = true;
  bool hasF = true;
  bool hasD = true;
  bool hasC = true;
  bool hasV = false;
  // Must match the production linear-scan pools. Scratch registers are not
  // included: 11 regular caller-saved + a0/a1 + 11 callee-saved GPRs, and
  // 15 regular caller-saved + fa0/fa1 + 12 callee-saved FPRs.
  int allocatableGPRs = 24;
  int allocatableFPRs = 29;
  int stackAlignment = 16;
  int issueWidth = 1;
  // Zero means that no authoritative target value is available.  Callers
  // must not invent a latency from another microarchitecture.
  int loadUseLatency = 0;
  int maxPreferredFrameBytes = 1 << 20;
  bool costsCalibratedForSpecificCPU = false;
};

struct RV64InstructionCost {
  int latency = 1;
  int reciprocalThroughput = 1;
  int codeBytes = 4;
};

const RV64TargetInfo& defaultRV64GCTarget();
const RV64TargetInfo& rv64TargetProfile(RV64TargetProfileKind kind);
RV64InstructionCost rv64InstructionCost(MIOp opcode);

} // namespace mir
