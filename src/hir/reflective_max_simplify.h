// 文件职责：声明 CoreIR 降低前的反射式 max 链简化接口。
#pragma once

struct CompUnit;

namespace hir {

bool simplifyReflectiveMaxLadders(CompUnit& cu);

} // namespace hir
