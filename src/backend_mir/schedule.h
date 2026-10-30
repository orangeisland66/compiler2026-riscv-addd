// 文件职责：做机器指令调度和块布局，改善热路径执行形状。本文件负责声明接口和数据结构。
#pragma once

#include "machine_ir.h"

namespace mir {

// 面向 BOOM v3 的基本块列表调度器。
//
// 模型：
//   - 每周期 2 个发射槽（双发射）。
//   - 1 个内存端口（每周期最多发射一条加载或存储）。
//   - 粗略延迟表：ALU=1，分支=1，乘法=3，除法/取余约为 20，
//     加载=3，浮点加/减/乘=4，浮点除/平方根=10。
//
// 依赖模型：
//   - 每个寄存器操作数都建立 RAW/WAR/WAW 依赖；寄存器分配后主要是物理寄存器。
//   - 加载跟随前一条存储；存储依赖前一条存储以及该存储之后的所有加载。
//   - 调用是完整屏障：此前所有指令先于调用，调用先于之后所有指令。
//   - 终结指令保留在块尾，不参与重排。
//
// 在寄存器分配后运行，主要处理物理寄存器。遇到 DAG 成环、块中全是调用等
// 特殊情况时保持基本块不变。
void runListSchedule(MachineFunction& mf);
void runListSchedule(MachineModule& mm);
void runListScheduleFPHot(MachineFunction& mf);
void runListScheduleFPHot(MachineModule& mm);
void runListScheduleNonFP(MachineFunction& mf);
void runListScheduleNonFP(MachineModule& mm);

// 轻量级基本块布局：保留入口块，再沿 CFG 更偏好的后继排列基本块，
// 尽量增加自然直落路径，最后由窥孔阶段删除多余的 `j`。
void runBlockLayout(MachineFunction& mf);
void runBlockLayout(MachineModule& mm);

} // namespace mir
