// 文件职责：实现保守 RAW/WAR/WAW 依赖识别。
#include "loop_dependence.h"

#include "alias_analysis.h"

namespace analysis {

LoopDependenceSummary analyzeLoopDependences(
    const NaturalLoop& loop, const std::vector<InductionVar>& ivs) {
  LoopDependenceSummary summary;
  summary.accesses = collectLoopAccesses(loop, ivs);
  bool complete = true;
  for (std::size_t i = 0; i < summary.accesses.size(); ++i) {
    for (std::size_t j = i + 1; j < summary.accesses.size(); ++j) {
      auto& left = summary.accesses[i];
      auto& right = summary.accesses[j];
      if (!left.writes && !right.writes) continue;
      auto alias = queryAlias(left.address, left.accessBytes,
                              right.address, right.accessBytes);
      if (alias.kind == AliasKind::NoAlias) continue;
      LoopDependence dependence;
      dependence.source = &left;
      dependence.sink = &right;
      if (left.writes && right.writes)
        dependence.kind = DependenceKind::Output;
      else if (left.writes)
        dependence.kind = DependenceKind::Flow;
      else
        dependence.kind = DependenceKind::Anti;
      if (alias.kind == AliasKind::MustAlias)
        dependence.proof = ProofResult::success("must-alias-dependence");
      else {
        dependence.kind = DependenceKind::Unknown;
        dependence.proof = ProofResult::unknown("may-alias-dependence");
        complete = false;
      }
      summary.dependences.push_back(std::move(dependence));
    }
  }
  for (const auto& access : summary.accesses)
    complete = complete && access.proof.proved();
  summary.completeness = complete
      ? ProofResult::success("dependence-inventory-complete")
      : ProofResult::unknown("dependence-inventory-incomplete");
  return summary;
}

} // namespace analysis
