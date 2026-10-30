// 文件职责：实现小型聚合对象标量替换，配合 Mem2Reg 消除局部数组访存。本文件负责声明接口和数据结构。
#pragma once
#include "../core_ir/ir/core_ir.h"

namespace rewrite {

// 小型聚合对象标量替换，对应 LLVM SROA / Mem2Reg。
//
// 只处理不逃逸的小数组 alloca：所有地址用户都必须是常量偏移，并且最终
// 只流向 load/store 或可证明的零初始化。满足条件时，每个元素可以拆成
// 独立标量槽，再交给现有 Mem2Reg 提升为 SSA。
//
// 常数阈值来自通用收益模型：避免过大对象导致代码体积、栈槽数量和寄存器
// 压力失控。任意非常量地址、逃逸
// 或非 load/store 用户都会让整个 alloca 保守放弃。
bool applyArraySROA(core_ir::Module& mod);

} // namespace rewrite
