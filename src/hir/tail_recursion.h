// 文件职责：识别并消除安全的尾递归，把尾调用改写为循环。本文件负责声明接口和数据结构。
#pragma once
#include "../core_ir/ir/core_ir.h"

#include <string>
#include <unordered_set>

struct CompUnit;

namespace hir {

// 把安全的尾递归改写成循环。
//
// 适用形态是“函数最后一步是一次自递归调用，且返回值不再参与后续计算”。
// 这类递归可以等价改写成：
//
// 1. 先把下一轮递归参数写回当前参数槽。
// 2. 再跳回函数入口继续执行。
//
// 这样可以消除大量小递归调用带来的栈帧构造、参数搬运和返回开销。
bool eliminateTailRecursion(core_ir::Module& mod);
bool eliminateTailRecursion(
    core_ir::Module& mod,
    const std::unordered_set<std::string>* blockedFunctions);
bool eliminateTailRecursion(CompUnit& cu);
bool eliminateTailRecursion(
    CompUnit& cu,
    const std::unordered_set<std::string>* blockedFunctions);
bool pruneTrivialSelfRecursion(CompUnit& cu);
bool pruneTrivialSelfRecursion(
    CompUnit& cu,
    const std::unordered_set<std::string>* blockedFunctions);

} // namespace hir
