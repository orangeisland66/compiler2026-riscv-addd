// 文件职责：计算 MIR 层活跃信息，为寄存器分配和 cleanup 提供依据。本文件负责声明接口和数据结构。
#pragma once

#include "machine_ir.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mir {

// 按虚拟寄存器编号记录的基本块活跃集合。
struct BlockLiveness {
  std::unordered_set<int> use;     // 在本块定义前使用
  std::unordered_set<int> def;     // 在本块中定义过
  std::unordered_set<int> liveIn;  // 进入本块时活跃
  std::unordered_set<int> liveOut; // 离开本块时活跃
};

struct LiveSegment {
  int start;
  int end;
};

// 每个虚拟寄存器对应一个活跃区间。start/end 是供回存启发式使用的覆盖范围，
// segments 保留 CFG 形成的空洞，使不重叠的值可以共享寄存器。
struct LiveInterval {
  int vreg;
  RegClass cls;
  int start;        // 虚拟寄存器开始活跃的第一条指令位置
  int end;          // 虚拟寄存器最后活跃的指令位置
  bool crossesCall; // [start, end] 中是否包含调用指令
  int loopDepth;    // 活跃位置上的最大循环深度（由调用者填写）
  int useCount;     // 总使用次数（供回存启发式使用）
  int hotUseCount = 0;
  int hotLoopWeight = 0;
  bool preferCallerSaved = false;
  bool rematerializable = false;
  std::vector<int> usePositions;
  std::vector<LiveSegment> segments;
};

// 将指令线性化：每条 MachineInst 获得唯一的非负 pos。基本块按逆后序访问，
// 使支配块通常先于被支配块，活跃区间也按起点到终点排列。
void numberInstructions(MachineFunction& mf);

// 使用固定点数据流计算每个基本块的活跃信息。
std::unordered_map<MachineBasicBlock*, BlockLiveness> computeLiveness(
    MachineFunction& mf);

// 根据基本块活跃信息和指令编号构造分段活跃区间。
std::vector<LiveInterval> buildLiveIntervals(
    MachineFunction& mf,
    const std::unordered_map<MachineBasicBlock*, BlockLiveness>& liveness);

// 便捷接口：依次执行上述步骤，并返回活跃区间和活跃信息。
struct LivenessResult {
  std::unordered_map<MachineBasicBlock*, BlockLiveness> blockLiveness;
  std::vector<LiveInterval> intervals;
};
LivenessResult analyzeLiveness(MachineFunction& mf);

} // namespace mir
