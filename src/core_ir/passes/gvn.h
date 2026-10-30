// GVN：全局值编号。
// 为每个值分配全局编号，消除冗余计算。
#pragma once

#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"

namespace core_ir {

// 主入口：在模块上运行全局值编号
bool globalValueNumbering(Module& mod, analysis::AnalysisManager& am);

} // namespace core_ir
