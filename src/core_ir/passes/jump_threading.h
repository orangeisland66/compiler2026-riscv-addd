// 文件职责：实现 LLVM 风格的 JumpThreading 模块级 pass 入口。
#pragma once

#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"

namespace core_ir {

bool applyJumpThreading(Module& mod,
                        analysis::AnalysisManager& analysisManager);

} // namespace core_ir
