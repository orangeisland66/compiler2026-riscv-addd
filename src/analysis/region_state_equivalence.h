// 文件职责：比较 baseline/candidate ordered effects，证明外部可观察内存事件一致。
#pragma once

#include "ordered_region_effect.h"

#include <unordered_map>

namespace analysis {

struct RegionStateEquivalence {
  ProofResult proof;
  std::size_t comparedEvents = 0;
};

RegionStateEquivalence compareRegionStateEffects(
    const OrderedRegionEffectSummary& baseline,
    const OrderedRegionEffectSummary& candidate,
    const std::unordered_map<const core_ir::Value*, const core_ir::Value*>&
        objectMap = {});

} // namespace analysis
