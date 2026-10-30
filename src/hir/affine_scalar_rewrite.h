// 文件职责：把 affine 地址和重复标量访问改写成递推、SSA 累加和局部替换。本文件负责声明接口和数据结构。
#pragma once

#include "../core_ir/ir/core_ir.h"

namespace analysis { class AnalysisManager; }

namespace hir {

// 把可证明 affine 的循环地址族改写成 pointer recurrence，并把同一
// 地址族的重复标量 load/store 做局部转发。来源对应 LoopStrengthReduce、
// GVN/load forwarding 和 scalar replacement，不依赖具体矩阵维度或函数名。
bool applyAffineScalarRewrite(core_ir::Module& mod);
bool applyAffineScalarRewrite(core_ir::Module& mod,
                              analysis::AnalysisManager& analysisManager);

} // namespace hir
