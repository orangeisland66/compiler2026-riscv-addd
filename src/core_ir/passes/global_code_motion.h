// 文件职责：声明 CoreIR 全局代码移动 pass 的接口。
#pragma once

#include "../ir/core_ir.h"

namespace analysis {
class AnalysisManager;
}

namespace core_ir {

// 将纯 SSA 表达式尽量下沉到使用位置附近；不提前执行指令，也不把指令移入
// 更深层循环。
bool globalCodeMotion(Module& mod, analysis::AnalysisManager& am);

} // namespace core_ir
