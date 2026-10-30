// 文件职责：声明循环零系数处理接口。
#pragma once

#include "../ir/core_ir.h"

namespace analysis {
class AnalysisManager;
}

namespace core_ir {

bool applyLoopZeroCoefficientSpecialization(
    Module& mod, analysis::AnalysisManager& analysisManager);

} // namespace core_ir
