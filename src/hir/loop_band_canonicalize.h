// 文件职责：规范化循环 band，使后续 interchange/blocking/recurrence 能稳定消费。本文件负责声明接口和数据结构。
#pragma once

#include "../core_ir/ir/core_ir.h"

namespace analysis { class AnalysisManager; }

namespace hir {

// 标量清理后为所有自然循环重新建立专用前置块，
// 让后续循环带处理看到稳定的“单入口”结构。
bool applyLoopBandCanonicalization(core_ir::Module& mod);
bool applyLoopBandCanonicalization(core_ir::Module& mod,
                                   analysis::AnalysisManager& analysisManager);

} // namespace hir
