// 文件职责：提供保守的 i32 常量范围事实；无法排除回绕时返回 Unknown。
#pragma once

#include "../core_ir/ir/core_ir.h"
#include "proof_result.h"

#include <cstdint>

namespace analysis {

struct IntegerFacts {
  ProofResult proof = ProofResult::unknown("integer-facts-unknown");
  bool rangeKnown = false;
  int32_t minimum = 0;
  int32_t maximum = 0;
  uint32_t knownZero = 0;
  uint32_t knownOne = 0;
  bool nonNegative = false;
  bool nonZero = false;
};

IntegerFacts evaluateIntegerFacts(core_ir::Value* value);

} // namespace analysis
