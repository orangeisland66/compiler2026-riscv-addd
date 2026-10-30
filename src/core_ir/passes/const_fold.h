// 文件职责：声明 CoreIR 常量折叠 pass 的实现接口。
#pragma once
#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"

namespace core_ir {

// 常量折叠：对模块中所有指令进行常量折叠
bool constFold(Module& mod);

} // namespace core_ir
