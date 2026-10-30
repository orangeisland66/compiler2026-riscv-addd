// 文件职责：声明循环 continue 边界分析和处理接口。
#pragma once

#include "../ir/core_ir.h"

namespace analysis {
class AnalysisManager;
}

namespace core_ir {

// 处理循环体为无副作用 continue 保护、形式为 outer_iv < inner_iv 的规范内层
// 循环。只有两条 latch 路径携带相同循环 PHI 时才进入该处理。
bool applyLoopContinueBound(Module& mod, analysis::AnalysisManager& am);

} // namespace core_ir
