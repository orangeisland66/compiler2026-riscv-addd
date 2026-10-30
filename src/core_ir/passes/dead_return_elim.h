// 无用返回值消除接口。
#pragma once
#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"

namespace core_ir {
bool deadReturnValueElimination(Module& mod, analysis::AnalysisManager& am);
} // namespace core_ir
