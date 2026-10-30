// 文件职责：基于纯度和调用期稳定状态为可 memoize 自递归生成缓存 wrapper。本文件负责声明接口和数据结构。
#pragma once

#include "../core_ir/ir/core_ir.h"
#include "../analysis/function_effects.h"

namespace analysis { class AnalysisManager; }

namespace hir {

// 对调用树内纯且状态稳定的 i32 自递归函数插入每次顶层调用独立
// 的 memo 表；带 AnalysisManager 的重载复用 effect/recursive summary 缓存。
bool applyRecursiveMemoization(core_ir::Module& mod);
bool applyRecursiveMemoization(core_ir::Module& mod,
                               analysis::AnalysisManager& analysisManager);

analysis::RecursiveOptimizationMode classifyRecursiveOptimizationMode(
    core_ir::Module& mod,
    core_ir::Function* func);
analysis::RecursiveOptimizationMode classifyRecursiveOptimizationMode(
    core_ir::Module& mod,
    core_ir::Function* func,
    analysis::AnalysisManager& analysisManager);

} // namespace hir
