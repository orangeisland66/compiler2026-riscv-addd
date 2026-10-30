// 文件职责：把循环 access dependence 转为使用稳定索引的 statement relation。
#pragma once

#include "loop_dependence.h"

namespace analysis {

enum class LoopCarriedKind { No, Yes, Unknown };

struct StatementDependenceEdge {
  std::size_t sourceAccess = 0;
  std::size_t sinkAccess = 0;
  DependenceKind kind = DependenceKind::Unknown;
  LoopCarriedKind loopCarried = LoopCarriedKind::Unknown;
  ProofResult proof;
};

struct StatementDependenceRelation {
  ProofResult proof;
  std::vector<LoopAccess> accesses;
  std::vector<StatementDependenceEdge> edges;
  bool permutationSafe = false;
};

StatementDependenceRelation buildStatementDependenceRelation(
    const NaturalLoop& loop, const std::vector<InductionVar>& ivs);

} // namespace analysis
