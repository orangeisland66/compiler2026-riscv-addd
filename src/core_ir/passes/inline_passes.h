// 文件职责：声明 CoreIR 函数内联 pass 的接口。
#pragma once
#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"

namespace core_ir {

// 为 void 函数开头的提前返回 call 添加守卫，使其可被安全内联。
bool applyVoidEarlyReturnCallGuards(Module& mod,
                                    analysis::AnalysisManager& analysisManager);
// 内联体积较小的函数（按指令数代价模型判断）。
bool inlineSmallFunctions(Module& mod,
                          analysis::AnalysisManager& analysisManager);
// 内联极小的纯辅助函数（无副作用，可自由复制）。
bool inlineTinyPureHelpers(Module& mod,
                           analysis::AnalysisManager& analysisManager);
// 内联仅被循环过程调用的单一调用者函数。
bool inlineSingleCallerLoopProcedures(Module& mod,
                                      analysis::AnalysisManager& analysisManager);

} // namespace core_ir
