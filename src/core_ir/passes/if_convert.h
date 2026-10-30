// 文件职责：声明 CoreIR 条件分支改写接口。
#pragma once
#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"

namespace core_ir {

/// Small diamond if-conversion: 将分支改成 branchless select
bool ifConvertSmallDiamonds(Module& mod);

} // namespace core_ir
