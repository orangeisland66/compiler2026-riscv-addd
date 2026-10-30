// 文件职责：将 CoreIR 指令降低为机器级 MIR 指令。本文件负责声明接口和数据结构。
#pragma once

#include "machine_ir.h"

namespace core_ir {
class Module;
class Function;
class BasicBlock;
class Instruction;
class Value;
} // namespace core_ir

namespace mir {

// 使用虚拟寄存器将 CoreIR module 降低到 MachineModule。
// lowering 后满足：
//   - 每个非 void 类型的 core_ir::Value 都有对应 vreg；参数和调用返回值会先从 ABI
//     物理寄存器通过 MV 接入。
//   - Phi 节点表示为 block 顶部的 PHI pseudo；lowering 阶段暂不做边上的 parallel-copy
//     resolution，这由 Phase 1.5 负责。
//   - 栈分配 Alloca 转成 MachineFunction 上的 FrameObject。
//   - 每个 MachineFunction 都会填充 hasCalls、incomingIntArgs、incomingFloatArgs，
//     供后续 pass 判断叶子函数快捷路径。
//   - 浮点常量会 intern 到 MachineModule 的常量池。
//
// lowering 刻意保持直接，生成容易由中端强度削减、RA、peephole 和 scheduler 继续改进的
// MIR。memset-zero 展开、trivial-select helper inline 等模式放在中端或 peephole 处理。
std::unique_ptr<MachineModule> lowerToMIR(const core_ir::Module& mod);

} // namespace mir
