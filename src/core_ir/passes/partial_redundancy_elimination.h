// 文件职责：声明部分冗余消除 pass 的接口。
#pragma once

#include "../ir/core_ir.h"

namespace analysis {
class AnalysisManager;
}

namespace core_ir {

// 对只有部分控制流入边已经计算出的表达式补齐缺失路径，
// 并用 phi 合并各条入边的值。
bool partialRedundancyElimination(Module& mod, analysis::AnalysisManager& am);

} // namespace core_ir
