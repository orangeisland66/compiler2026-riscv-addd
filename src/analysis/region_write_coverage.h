// 文件职责：用已证明的常量 byte range 判定 region 对单一对象的完整覆盖。
#pragma once

#include "ordered_region_effect.h"

#include <cstdint>

namespace analysis {

enum class RegionCoverageKind { Complete, Partial, Unknown };

struct RegionObjectCoverage {
  core_ir::Value* object = nullptr;
  RegionCoverageKind kind = RegionCoverageKind::Unknown;
  int64_t objectBytes = 0;
  int64_t coveredBytes = 0;
  bool read = false;
  bool written = false;
  bool readBeforeCompleteWrite = false;
  ProofResult proof;
};

struct RegionWriteCoverage {
  ProofResult proof;
  std::vector<RegionObjectCoverage> objects;
  bool hasUnknownEffect = false;
};

RegionWriteCoverage analyzeRegionWriteCoverage(
    const OrderedRegionEffectSummary& effects);
const char* regionCoverageKindName(RegionCoverageKind kind);

} // namespace analysis
