// 文件职责：执行保守循环分块，改善 affine 访问的工作集局部性。本文件负责声明接口和数据结构。
#pragma once

#include "../core_ir/ir/core_ir.h"

namespace analysis { class AnalysisManager; }

namespace hir {

// 对 canonical affine loop band 做保守的 strip-mining / blocking。
//
// 当前实现只在收益模型能稳定选择 tile 因子、并且不会引入复杂 remainder
// 处理时触发，目标是先让工作集缩小，再把更适合的 tile 形状交给后续
// 标量替换和 lowering。
bool applyLoopBlocking(core_ir::Module& mod);
bool applyLoopBlocking(core_ir::Module& mod,
                       analysis::AnalysisManager& analysisManager);

} // namespace hir
