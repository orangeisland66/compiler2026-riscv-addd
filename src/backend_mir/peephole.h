// 文件职责：执行保守 peephole 优化，删除局部冗余机器指令。本文件负责声明接口和数据结构。
#pragma once

#include "machine_ir.h"

namespace mir {

// 窥孔整理在寄存器分配之后、栈帧降低和汇编发射之前执行。
//
// 当前处理：
//   - 将紧邻同一完整 spill home 的 reload 转发为寄存器 move；
//   - 删除 rd == rs 时的冗余 `mv rd, rs` / `fmv.s fd, fs`；
//   - 删除 `addi rd, rd, 0` / `addiw rd, rd, 0`；
//   - 删除目标是紧邻后继块的 `j label`；
//   - 条件分支后接落下跳转的整理由后续规则处理。
//
// 逐个函数处理，打印器只读取每个基本块中的指令列表。
void runPeephole(MachineFunction& mf);
void runPeephole(MachineModule& mm);

} // namespace mir
