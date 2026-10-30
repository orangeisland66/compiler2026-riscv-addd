// 文件职责：转发 affine copy/view 的只读 consumer，避免不必要的物化数组。本文件负责声明接口和数据结构。
#pragma once

#include "../core_ir/ir/core_ir.h"

namespace analysis { class AnalysisManager; }

namespace hir {

// affine copy/view 转发，对应 copy propagation / scalar replacement /
// Polly 风格 affine 视图变换。
//
// 当 producer 是可证明全覆盖的 affine copy 或转置视图，且后续 consumer
// 只读这个视图、中间没有写入/逃逸/别名破坏时，将 consumer load 直接转发
// 到原始 source view。该优化只能由 affine copy region 和 dominance/path
// 证明触发。
bool applyTransposeViewForwarding(core_ir::Module& mod);
bool applyTransposeViewForwarding(core_ir::Module& mod,
                                  analysis::AnalysisManager& analysisManager);

} // namespace hir
