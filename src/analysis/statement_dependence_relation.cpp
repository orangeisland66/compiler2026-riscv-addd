// 文件职责：实现 fail-closed statement relation；无法求距离时不授权循环置换。
#include "statement_dependence_relation.h"

namespace analysis {

StatementDependenceRelation buildStatementDependenceRelation(
    const NaturalLoop& loop, const std::vector<InductionVar>& ivs) {
  StatementDependenceRelation result;
  auto summary = analyzeLoopDependences(loop, ivs);
  const auto* accessBase = summary.accesses.data();
  bool complete = summary.completeness.proved();
  for (const auto& dependence : summary.dependences) {
    StatementDependenceEdge edge;
    edge.kind = dependence.kind;
    edge.proof = dependence.proof;
    if (!dependence.source || !dependence.sink) {
      edge.proof = ProofResult::unknown("dependence-endpoint-missing");
      complete = false;
      result.edges.push_back(std::move(edge));
      continue;
    }
    edge.sourceAccess = static_cast<std::size_t>(
        dependence.source - accessBase);
    edge.sinkAccess = static_cast<std::size_t>(
        dependence.sink - accessBase);
    const auto& source = summary.accesses[edge.sourceAccess];
    const auto& sink = summary.accesses[edge.sinkAccess];
    if (!source.affine.valid || !sink.affine.valid) {
      edge.loopCarried = LoopCarriedKind::Unknown;
      edge.proof = ProofResult::unknown("dependence-distance-unknown");
      complete = false;
    } else if (source.affine.offset.ivTerms.empty() &&
               source.affine.offset.scaledIvTerms.empty() &&
               sink.affine.offset.ivTerms.empty() &&
               sink.affine.offset.scaledIvTerms.empty()) {
      // The same loop-invariant location is observed in multiple iterations.
      edge.loopCarried = LoopCarriedKind::Yes;
      edge.proof = ProofResult::success("loop-invariant-carried-dependence");
    } else {
      // A full integer distance solver is required before claiming No.
      edge.loopCarried = LoopCarriedKind::Unknown;
      edge.proof = ProofResult::unknown("affine-distance-not-solved");
      complete = false;
    }
    result.edges.push_back(std::move(edge));
  }
  result.permutationSafe = result.edges.empty() && complete;
  result.accesses = std::move(summary.accesses);
  result.proof = complete
                     ? ProofResult::success("statement-dependence-complete")
                     : ProofResult::unknown(
                           "statement-dependence-distance-incomplete");
  return result;
}

} // namespace analysis
