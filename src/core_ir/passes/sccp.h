// SCCP：稀疏条件常量传播。
// 结合常量传播和死代码消除的迭代算法。
#pragma once

#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"

namespace core_ir {

// 主入口：稀疏条件常量传播
bool sparseConditionalConstantPropagation(Module& mod, analysis::AnalysisManager& am);

} // namespace core_ir
