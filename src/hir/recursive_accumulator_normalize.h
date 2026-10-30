// 文件职责：把递归 accumulator 形态归一化，便于后续尾递归和 helper 优化。本文件负责声明接口和数据结构。
#pragma once

#include <string>
#include <unordered_set>

struct CompUnit;

namespace hir {

bool normalizeRecursiveAccumulators(
    CompUnit& cu,
    const std::unordered_set<std::string>* blockedFunctions);
bool normalizeRecursiveAccumulators(CompUnit& cu);

} // namespace hir
