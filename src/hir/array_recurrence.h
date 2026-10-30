// 文件职责：声明默认循环流水线使用的仿射数组地址递推接口。
#pragma once
#include "../core_ir/ir/core_ir.h"

namespace hir {

// 把循环里的 GEP + load/store 地址链改成指针递推。
//
// 若某个地址可证明等价于 `base + iv * stride`，就把它改写成：
// 1. 循环前初始化 `ptr = base`；
// 2. 循环回边更新 `ptr = ptr + stride`；
// 3. 循环体内直接使用 `ptr` 而不是重新做 GEP 计算。
//
// 该改写由默认流水线在循环整理、展开前后按成本条件调用，并保留独立
// 跳过开关，便于比较递推地址与原始 GEP 地址链。
bool applyArrayAddressRecurrence(core_ir::Module& mod);

} // namespace hir
