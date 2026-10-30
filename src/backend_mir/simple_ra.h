// 文件职责：实现简化寄存器分配路径，用于小函数或 fallback 场景。本文件负责声明接口和数据结构。
#pragma once

#include "machine_ir.h"

namespace mir {

// 简单的栈溢出寄存器分配器。每个虚拟寄存器都分配一个栈帧槽位；每次使用前
// 重新加载到临时寄存器（GPR 使用 t5/t6，FPR 使用 ft10/ft11），定义后立即
// 将临时寄存器的值写回栈帧槽位。
//
// PHI 伪指令通过在每条输入边上插入并行存储来处理：把 src 写入 dst 的栈帧槽位，
// 插入位置在前驱终结指令之前。每个 phi 目标都有独立槽位，因此这一阶段不会因
// 共享寄存器形成环，不需要额外的环处理。
//
// 本 pass 完成后：
//   - 不再保留虚拟寄存器操作数，全部替换为物理寄存器；
//   - PHI 和 PHI_COPY 伪指令被消除；
//   - PROLOGUE/EPILOGUE 标记仍保留，稍后由其他 pass 降低；
//   - 栈帧大小确定，每个 FrameObject 都有相对 sp 的偏移。
//
// 该寄存器分配路径有意采用保守策略，对应当前后端“临时值全部溢出”的行为。
// 它用于验证 MIR 降低、窥孔整理和汇编输出流程，也作为优化路径的调试与二分回退。
void runSimpleRA(MachineFunction& mf);

void runSimpleRA(MachineModule& mm);

} // namespace mir
