// 文件职责：声明循环不变除数处理接口。
#pragma once

#include "../ir/core_ir.h"

namespace analysis {
class AnalysisManager;
}

namespace core_ir {

bool applyLoopInvariantDivisorSpecialization(
    Module& mod, analysis::AnalysisManager& analysisManager);

} // namespace core_ir
