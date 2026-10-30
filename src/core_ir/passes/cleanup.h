// 文件职责：声明 CoreIR 清理 pass 的实现接口。
#pragma once
#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"

namespace core_ir {

// 死代码消除
bool dce(Module& mod, analysis::AnalysisManager& am);

bool eliminateUnreachableFunctions(Module& mod);

// Phi 降低辅助函数。
bool lowerPhiToEdgeStores(Module& mod);

// CFG 简化：常量条件分支折叠
bool simplifyCFG(Module& mod);

// 合并线性基本块
bool mergeLinearBlocks(Module& mod);

// Phi 边归一化
bool normalizePhiEdges(Module& mod);

// 小 diamond if-conversion
bool ifConvertSmallDiamonds(Module& mod);

} // namespace core_ir
