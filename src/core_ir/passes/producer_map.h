// 文件职责：声明生产者映射分析和变换接口。
#pragma once
#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"

namespace core_ir {

bool applyGuardedStencilInteriorFastPath(
    Module& mod,
    analysis::AnalysisManager& analysisManager);

} // namespace core_ir
