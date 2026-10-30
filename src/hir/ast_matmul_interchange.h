// 文件职责：声明 AST 层矩阵乘法循环的访问顺序整理接口。
// 通过调整循环顺序，把连续的行访问暴露给后续代码生成。
#pragma once
#include "../frontend/ast.h"

namespace hir {

// 对矩阵乘法归约形状执行 AST 层循环整理，返回是否发生变换。
bool applyASTMatmulInterchange(CompUnit& cu);

} // namespace hir
