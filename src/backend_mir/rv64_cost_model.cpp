// 文件职责：实现保守、可解释的 RV64GC 指令成本表。
#include "rv64_cost_model.h"

namespace mir {

const RV64TargetInfo& defaultRV64GCTarget() {
  return rv64TargetProfile(RV64TargetProfileKind::GenericRV64GC);
}

const RV64TargetInfo& rv64TargetProfile(RV64TargetProfileKind kind) {
  static const RV64TargetInfo generic;
  static const RV64TargetInfo boom = [] {
    RV64TargetInfo target;
    target.profileName = "boom-rv64gc-uncalibrated";
    target.modelRevision = "2026-08-14-v1";
    // BOOM is an out-of-order, multiple-issue family.  Width is useful for
    // diagnostics, but latency/throughput remain unknown until the actual
    // configured core is identified and measured.
    target.issueWidth = 2;
    target.loadUseLatency = 0;
    target.costsCalibratedForSpecificCPU = false;
    return target;
  }();
  return kind == RV64TargetProfileKind::BoomRV64GC ? boom : generic;
}

RV64InstructionCost rv64InstructionCost(MIOp opcode) {
  RV64InstructionCost cost;
  if (mioMayLoad(opcode)) {
    cost.latency = 3;
    cost.reciprocalThroughput = 1;
  }
  if (mioMayStore(opcode)) {
    cost.latency = 1;
    cost.reciprocalThroughput = 1;
  }
  if (mioIsBranch(opcode)) {
    cost.latency = 2;
    cost.reciprocalThroughput = 1;
  }
  if (mioIsCall(opcode)) {
    cost.latency = 8;
    cost.reciprocalThroughput = 4;
  }
  switch (opcode) {
  case MIOp::MUL:
  case MIOp::MULW:
  case MIOp::MULH:
    cost.latency = 3;
    break;
  case MIOp::DIV:
  case MIOp::DIVW:
  case MIOp::DIVU:
  case MIOp::DIVUW:
  case MIOp::REM:
  case MIOp::REMW:
  case MIOp::REMU:
  case MIOp::REMUW:
    cost.latency = 20;
    cost.reciprocalThroughput = 8;
    break;
  case MIOp::FADD_S:
  case MIOp::FSUB_S:
  case MIOp::FMUL_S:
  case MIOp::FMADD_S:
  case MIOp::FMSUB_S:
  case MIOp::FNMADD_S:
  case MIOp::FNMSUB_S:
    cost.latency = 4;
    break;
  case MIOp::FDIV_S:
  case MIOp::FSQRT_S:
    cost.latency = 16;
    cost.reciprocalThroughput = 8;
    break;
  default:
    break;
  }
  return cost;
}

} // namespace mir
