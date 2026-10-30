// 文件职责：声明 CoreIR 局部指令组合和表达式简化接口。
#pragma once

#include "../../analysis/analysis_manager.h"
#include "../ir/core_ir.h"

namespace core_ir {

bool applyInstCombine(Module& mod, analysis::AnalysisManager& analysisManager);
bool applyPostInlineInstCombine(Module& mod,
                                analysis::AnalysisManager& analysisManager);

} // namespace core_ir
