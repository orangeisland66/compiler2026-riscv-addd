// 文件职责：把只观察奇偶性的表达式改写为直接位测试。本文件负责声明接口和数据结构。
#pragma once

#include "../core_ir/ir/core_ir.h"

namespace rewrite {

// 奇偶性规范化，对应 InstCombine / strength reduction。
//
// 如果最终用途只观察 even/odd 语义，就把 `% 2`、乘积奇偶等表达式改成
// 直接位测试，避免保留有符号除余的归一化链。触发条件必须来自代数等价和
// use-chain 证明。
bool applyParityCanonicalization(core_ir::Module& mod);

} // namespace rewrite
