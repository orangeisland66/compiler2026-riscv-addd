// 文件职责：收紧 affine 循环边界，生成更适合 interior fast path 的形状。本文件负责声明接口和数据结构。
#pragma once

#include "../core_ir/ir/core_ir.h"

namespace analysis { class AnalysisManager; }

namespace hir {

enum class AffineLoopTighteningMode {
  Full,
  RegionOnly,
};

// 当单调 affine guard 只控制 continue 边时，收紧 canonical 循环
// 的上下界，把三角形/带 guard 的内层循环变成更规则的单 latch 区间。
//
// 这样后续的 address recurrence、scalar replacement 和 blocking 就能直接
// 消费更干净的 interior fast path 形状。
bool applyAffineLoopTightening(core_ir::Module& mod);
bool applyAffineLoopTightening(core_ir::Module& mod,
                               analysis::AnalysisManager& analysisManager);
bool applyAffineLoopTightening(core_ir::Module& mod,
                               analysis::AnalysisManager& analysisManager,
                               AffineLoopTighteningMode mode);

} // namespace hir
