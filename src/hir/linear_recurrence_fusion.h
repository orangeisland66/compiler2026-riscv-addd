// 文件职责：声明 CoreIR 降低前的四状态线性递推合并接口。
#pragma once

struct CompUnit;

namespace hir {

bool applyLinearRecurrenceFusion(CompUnit& cu);

} // namespace hir
