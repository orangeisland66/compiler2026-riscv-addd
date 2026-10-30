// 文件职责：未读全局数组死存储消除——全模块从未被读且指针从不逃逸的
// 全局数组，其全部存储均为死写，可整体删除。本文件负责声明接口。
#pragma once

#include "../core_ir/ir/core_ir.h"

namespace analysis { class AnalysisManager; }

namespace hir {

// 对每个非 const 全局数组 G，从 G 出发沿 GEP/BitCast 做前向
// 派生闭包；闭包内出现 Load/VLoad（读）、作为 Call 实参（IO/传参逃逸）、
// 作为 Store 的值操作数（指针被写入内存）或流入其他指令（Phi/Select 等）
// 即放弃。全部用途仅为 Store/VStore 的地址时，这些存储不可能被观察，
// 全部删除；残留的死地址计算交由既有清理回合消除。
bool applyDeadGlobalArrayStoreElim(core_ir::Module& mod,
                                   analysis::AnalysisManager& analysisManager);

} // namespace hir
