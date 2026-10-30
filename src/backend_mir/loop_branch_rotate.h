// 文件职责：声明 MIR 条件分支布局调整接口。
#pragma once

#include "machine_ir.h"

namespace mir {

// 将无副作用的循环头条件复制到寄存器分配后的回边上。
// 原循环头部继续作为零迭代保护，后续迭代可以直接从锁存块分支回循环体。
void runLoopBranchRotate(MachineFunction& mf);
void runLoopBranchRotate(MachineModule& mm);

} // namespace mir
