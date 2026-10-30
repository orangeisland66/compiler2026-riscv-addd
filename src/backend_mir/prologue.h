// 文件职责：生成函数序言/尾声和栈帧布局，遵守 RISC-V ABI。本文件负责声明接口和数据结构。
#pragma once

#include "machine_ir.h"

namespace mir {

// 为 mf 完成栈帧布局：
//   - 根据 FrameObject 和被调用者保存寄存器计算总栈帧大小；
//   - 为每个 FrameObject 分配相对 sp 的偏移；
//   - 把适用的 FrameIdx 操作数替换为立即数和 sp 基址；
//   - 在函数入口插入序言，在每个 RET 位置插入尾声。
//
// 前置条件：寄存器分配已经完成，不再存在虚拟寄存器。
void lowerFrame(MachineFunction& mf);
void lowerFrame(MachineModule& mm);

} // namespace mir
