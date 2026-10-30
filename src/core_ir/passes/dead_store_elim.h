// DSE：消除无效存储。
// 消除已被后续覆盖且中间未被读取的存储指令。
#pragma once

#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"

namespace core_ir {

// 主入口：消除死存储
bool deadStoreElimination(Module& mod, analysis::AnalysisManager& am);

} // namespace core_ir
