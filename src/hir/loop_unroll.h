// 文件职责：按收益模型展开小循环或 tile-local 内层循环，控制代码体积。本文件负责声明接口和数据结构。
#pragma once
#include "../core_ir/ir/core_ir.h"

namespace analysis { class AnalysisManager; }

namespace hir {

// 通用循环展开，来源对应 LLVM LoopUnroll。这里只依赖规范化
// 循环形状、trip count、代码体积和副作用预算。
// 完全展开用于小常量 trip-count，部分展开用于 tile-local 小循环。
bool applyLoopUnroll(core_ir::Module& mod);
bool applyLoopUnroll(core_ir::Module& mod,
                     analysis::AnalysisManager& analysisManager);

// 仅完整展开静态迭代次数为 1 的循环，供所有过程间变换结束后的收尾阶段
// 消除单次循环 PHI 与回边，不触发普通 partial-unroll 成本策略。
bool applySingleTripLoopUnroll(
    core_ir::Module& mod, analysis::AnalysisManager& analysisManager);

} // namespace hir
