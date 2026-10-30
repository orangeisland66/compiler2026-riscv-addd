// 文件职责：分析 affine 地址递推族，帮助后续生成 pointer bump 和标量替换计划。本文件负责声明接口和数据结构。
#pragma once

#include "affine_access.h"
#include "induction_var.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace analysis {

enum class AffineRecurrenceRole {
  Generic,
  Stream,
  UpdateStream,
  Broadcast,
  Reduction,
  UpdateSliceAnchor,
  UpdateSliceCompanion,
};

struct AffineRecurrencePlan {
  bool valid = false;
  const NaturalLoop* loop = nullptr;
  InductionVar induction{};
  core_ir::Value* currentIV = nullptr;
  AffineAccessSummary access;
  int64_t rawCoeff = 0;
  std::vector<std::pair<core_ir::Value*, int64_t>> scaledTerms;
  std::vector<core_ir::Instruction*> memOps;
  std::string familyKey;
  std::string updateSliceAnchorKey;
  int64_t estimatedBenefit = 0;
  int64_t strideCost = 0;
  int estimatedRegisterPressure = 0;
  AffineRecurrenceRole role = AffineRecurrenceRole::Generic;
  bool pointerBumpCandidate = false;
  bool tileLocalScalarCandidate = false;
  bool sameAddressEveryIteration = false;
  bool unitStride = false;
  bool dynamicStride = false;
  bool updateSliceAnchor = false;
  bool updateSliceCompanion = false;
  int updateSliceCompanionCount = 0;
  bool hasStore = false;
  bool hasLoad = false;
};

std::vector<AffineRecurrencePlan>
collectAffineRecurrencePlans(core_ir::Function* func,
                             const NaturalLoop& loop,
                             const std::vector<InductionVar>& ivs);

} // namespace analysis
