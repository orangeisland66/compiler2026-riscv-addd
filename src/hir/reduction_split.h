// 文件职责：规约循环寄存器分块（reduction splitting）。
// 将内层归约循环（acc += load_src1 * load_src2 形式）的中层维度步长改为 2，
// 使内层循环体同时计算两个独立累加器并共享其中一个装载，从而减少访存压力
// 并提高指令级并行度。仅对偶数常量中层边界生效（v1）；整数加法结合律安全。
#pragma once
#include "../core_ir/ir/core_ir.h"
namespace analysis { class AnalysisManager; }
namespace hir {
bool applyReductionSplit(core_ir::Module& mod,
                         analysis::AnalysisManager& am);
} // namespace hir
