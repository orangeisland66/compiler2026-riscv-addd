// 文件职责：声明 AST 简化接口。
#pragma once

struct CompUnit;

namespace hir {

bool simplifyAST(CompUnit& compUnit);

} // namespace hir
