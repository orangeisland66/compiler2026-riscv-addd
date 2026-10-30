// 文件职责：声明相邻兼容循环的合并接口。
#pragma once
#include "../core_ir/ir/core_ir.h"

namespace analysis { class AnalysisManager; }

namespace hir {

// 合并迭代空间相同且数据访问关系兼容的规范化循环，减少重复的循环控制和访存。
bool applyLoopFusion(core_ir::Module& mod);
bool applyLoopFusion(core_ir::Module& mod,
                     analysis::AnalysisManager& analysisManager);

} // namespace hir
