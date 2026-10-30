// 文件职责：提供基础 AST/IR 重写入口和通用重写辅助函数。本文件负责声明接口和数据结构。
#pragma once
#include "../core_ir/ir/core_ir.h"

namespace rewrite {

// 执行 CoreIR 层的代数规范化与强度削减。
//
// 典型规则包括：
// 1. 常量乘法改写为移位加减链。
// 2. 常量除法/取模改写为倒数乘法或等价位运算。
// 3. 合并线性组合，例如 `x * N + x -> x * (N + 1)`。
// 4. 压平可证明安全的移位/加法嵌套。
// 5. 消除双重取负等明显冗余。
bool applyIRAlgebraicRewrite(core_ir::Module& mod);

} // namespace rewrite
