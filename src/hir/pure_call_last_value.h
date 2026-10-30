// 文件职责：识别"循环携带值每轮被整体覆盖、体内只有只读计算"的计数循环，
// 将整个循环塌缩为末轮一次求值。本文件负责声明接口。
#pragma once

#include "../core_ir/ir/core_ir.h"

namespace analysis { class AnalysisManager; }

namespace hir {

// 针对 `while (cond(iv)) { x = f(iv); iv += step; }` 形态——体内
// 无任何写副作用（store/IO/写内存调用均排除）、除归纳变量外所有 header phi
// 的 latch 值不构成真递推（不读上一轮的自己）——出口活值只取决于末轮，
// 因此保留 header 守卫后可用末轮 IV 单次执行循环体替换整个循环。
bool applyPureCallLastValueCollapse(core_ir::Module& mod,
                                    analysis::AnalysisManager& analysisManager);

} // namespace hir
