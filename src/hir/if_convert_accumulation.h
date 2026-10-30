// 文件职责：条件累积 if-conversion pass 声明。
// 使用 Select 指令将 if(cond) { acc = acc + val } 转换为
// acc = acc + select(cond, val, 0)，消除条件分支。
#pragma once

#include "../analysis/analysis_manager.h"
#include "../core_ir/ir/core_ir.h"

namespace hir {

bool applyIfConvertAccumulation(core_ir::Module& mod,
                                analysis::AnalysisManager& am,
                                bool strictMode = false);

} // namespace hir
