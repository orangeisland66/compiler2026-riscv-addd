// 文件职责：根据 affine 访问关系执行循环交换，优先改善内层访问 stride。本文件负责声明接口和数据结构。
#pragma once
#include "../core_ir/ir/core_ir.h"

namespace analysis { class AnalysisManager; }

namespace hir {

// 在 loop forest 上做通用的相邻循环交换。
//
// 该 pass 仍然要求循环是 canonical 形状、局部嵌套可证明安全，且内层主体
// 没有会因为迭代顺序改变而破坏语义的副作用；但它不再把“两层循环”或“三层
// 循环”写死在实现里，而是遍历父子相邻的局部完美嵌套并逐步交换。
bool applyLoopInterchange(core_ir::Module& mod);
bool applyLoopInterchange(core_ir::Module& mod,
                          analysis::AnalysisManager& analysisManager);

} // namespace hir
