// 文件职责：用 alias/affine facts 建立保守循环依赖清单。
#pragma once

#include "loop_access.h"

namespace analysis {

enum class DependenceKind { Flow, Anti, Output, Unknown };

struct LoopDependence {
  const LoopAccess* source = nullptr;
  const LoopAccess* sink = nullptr;
  DependenceKind kind = DependenceKind::Unknown;
  ProofResult proof;
};

struct LoopDependenceSummary {
  std::vector<LoopAccess> accesses;
  std::vector<LoopDependence> dependences;
  ProofResult completeness;
};

LoopDependenceSummary analyzeLoopDependences(
    const NaturalLoop& loop, const std::vector<InductionVar>& ivs);

} // namespace analysis
