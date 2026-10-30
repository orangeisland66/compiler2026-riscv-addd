// 文件职责：提升可证明稳定的 load，减少循环或热路径中的重复访存。本文件负责声明接口和数据结构。
#pragma once
#include "../core_ir/ir/core_ir.h"

namespace analysis { class AnalysisManager; }

namespace rewrite {

// 循环不变 load 提升，来源对应 LICM / MemorySSA 风格的保守
// load hoist。触发条件只看 dedicated preheader、readonly call、同根 store
// clobber 和地址不变性。
bool applyLoopInvariantLoadHoist(core_ir::Module& mod);
bool applyLoopInvariantLoadHoist(core_ir::Module& mod,
                                 analysis::AnalysisManager& analysisManager);

bool applyLoopInvariantStoreHoist(core_ir::Module& mod,
                                  analysis::AnalysisManager& analysisManager);

} // namespace rewrite
