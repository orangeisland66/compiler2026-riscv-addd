// 文件职责：实现严格事件序列等价；缺少 object 映射或 range 证明时返回 Unknown。
#include "region_state_equivalence.h"

namespace analysis {

RegionStateEquivalence compareRegionStateEffects(
    const OrderedRegionEffectSummary& baseline,
    const OrderedRegionEffectSummary& candidate,
    const std::unordered_map<const core_ir::Value*, const core_ir::Value*>&
        objectMap) {
  RegionStateEquivalence result;
  if (!baseline.proof.proved() || !candidate.proof.proved()) {
    result.proof = ProofResult::unknown("region-effect-summary-incomplete");
    return result;
  }
  if (baseline.hasSideExit != candidate.hasSideExit ||
      baseline.hasObservableCall != candidate.hasObservableCall ||
      baseline.events.size() != candidate.events.size()) {
    result.proof = ProofResult::disproved("region-effect-shape-mismatch");
    return result;
  }
  for (std::size_t index = 0; index < baseline.events.size(); ++index) {
    const auto& left = baseline.events[index];
    const auto& right = candidate.events[index];
    if (left.kind != right.kind) {
      result.proof = ProofResult::disproved("region-effect-order-mismatch");
      return result;
    }
    if (left.object || right.object) {
      const core_ir::Value* mapped = left.object;
      auto found = objectMap.find(left.object);
      if (found != objectMap.end()) mapped = found->second;
      if (mapped != right.object) {
        result.proof = ProofResult::unknown("region-object-map-missing");
        return result;
      }
      if (!left.range.proof.proved() || !right.range.proof.proved()) {
        result.proof = ProofResult::unknown("region-range-unproved");
        return result;
      }
      if (left.range.beginByte != right.range.beginByte ||
          left.range.endByte != right.range.endByte) {
        result.proof = ProofResult::disproved("region-range-mismatch");
        return result;
      }
    }
    ++result.comparedEvents;
  }
  result.proof = ProofResult::success("region-state-effects-equivalent");
  return result;
}

} // namespace analysis
