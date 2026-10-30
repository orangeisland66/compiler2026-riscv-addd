// 文件职责：实现 MIR 线性扫描寄存器分配和 spill 处理。本文件负责声明接口和数据结构。
#pragma once

#include "machine_ir.h"

namespace mir {

// 在 MIR 上执行线性扫描寄存器分配。
//
// 流程约定：
//   - 输入：包含虚拟寄存器的 MachineFunction；除了函数入口的 ABI 预着色 MV、
//     调用和返回指令外，不含物理寄存器虚拟值。
//   - 输出：每个虚拟寄存器操作数被替换为物理寄存器，或放入栈帧槽位，并在
//     使用前重载、定义后回存；PHI / PHI_COPY 伪指令在前驱终结指令前按边
//     展开为移动。
//
// 寄存器池：
//   GPR 可分配：t0-t3、s1-s11（15 个寄存器）
//   FPR 可分配：ft0-ft8、fs0-fs11（21 个寄存器）
//   排除：x0/sp/gp/tp/ra/s0（特殊寄存器）、t4-t6（整数回存临时寄存器）、
//         ft9-ft11（浮点回存临时寄存器）、a0-a7 和 fa0-fa7（仅用于 ABI
//         传参和返回，避免与预着色操作冲突）。
//
// 分配策略：
//   - 活跃区间跨越调用时优先使用被调用者保存寄存器（s*/fs*）。
//   - 其他区间优先使用调用者保存寄存器（t*/ft*），必要时回退到被调用者保存
//     寄存器。
//   - 需要回存时，根据区间使用密度、热度和后续使用位置选择目标区间。
//
// mir_backend.cpp 优化 MIR 路径的默认寄存器分配器。
void runLinearScanRA(MachineFunction& mf);
void runLinearScanRA(MachineModule& mm);

} // namespace mir
