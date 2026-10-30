// 文件职责：识别循环内标量累加器，把 load/store 形态提升为 SSA carried scalar。本文件负责声明接口和数据结构。
#pragma once

#include "../core_ir/ir/core_ir.h"

namespace analysis { class AnalysisManager; }

namespace hir {

// 把可证明的循环局部标量累加器提升成 phi carried SSA，来源
// 对应 LLVM mem2reg/LCSSA 后的 reduction promotion 思路；触发条件只看
// alloca/load/store 和支配/循环结构。
bool applyLoopScalarAccumulatorPromotion(core_ir::Module& mod);
bool applyLoopScalarAccumulatorPromotion(core_ir::Module& mod,
                                         analysis::AnalysisManager& analysisManager);
bool applyLoopMemoryAccumulatorPromotion(core_ir::Module& mod,
                                         analysis::AnalysisManager& analysisManager);

} // namespace hir
