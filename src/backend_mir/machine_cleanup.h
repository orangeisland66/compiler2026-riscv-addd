// 文件职责：清理机器指令中的死定义、冗余 move 和可安全消除的跳转。本文件负责声明接口和数据结构。
#pragma once

#include "machine_ir.h"

namespace mir {

// 这是调度、布局和窥孔处理之后、栈帧降低之前的轻量级机器级清理。
// 它删除无用寄存器定义、重复的重新物化指令，以及只有在中端循环变换
// 完成后才容易识别的局部无效地址更新。
void runMachineCleanup(MachineFunction& mf);
void runMachineCleanup(MachineModule& mm);
// 在专用慢路径处分割入口定义的虚拟寄存器活跃区间，
// 让栈帧降低阶段可以在寄存器分配后为简单的提前返回收缩保存范围。
void prepareEarlyReturnShrinkWrapping(MachineModule& mm);
bool removeRedundantPostRAStores(MachineModule& mm);

} // namespace mir
