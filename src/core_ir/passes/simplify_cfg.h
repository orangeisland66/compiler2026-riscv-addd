// 文件职责：声明 CoreIR 控制流图简化接口。
#pragma once
#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"

namespace core_ir {

bool simplifyCFG(Module& mod);
bool mergeLinearBlocks(Module& mod);

} // namespace core_ir
