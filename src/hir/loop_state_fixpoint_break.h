// 文件职责：识别"确定性循环体反复重放、跨轮状态仅为标量集合"的计数循环，
// 插入运行期状态快照比较，收敛后提前退出。本文件负责声明接口。
#pragma once

#include "../core_ir/ir/core_ir.h"

namespace analysis { class AnalysisManager; }

namespace hir {

// 针对 `while (iv > 0) { 重置; f(); iv -= 1 }` 形态——体（含调用
// 闭包）无 IO、读集 = 只读数组 + 标量状态、写集 = 标量状态 + 体内不读的
// 数组——每轮行为仅由标量状态决定。运行期在每轮末比较标量状态快照与上一
// 轮，相等即 break（后续迭代观察等价），最坏情况仅增加常数比较开销。
bool applyLoopStateFixpointBreak(core_ir::Module& mod,
                                 analysis::AnalysisManager& analysisManager);

} // namespace hir
