// 文件职责：分析 CoreIR 值活跃区间，供寄存器分配和优化收益判断使用。本文件负责声明接口和数据结构。
#pragma once
#include "../core_ir/ir/core_ir.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace analysis {

struct LiveInterval {
  int vregId;       // Value::id()
  int start;        // 最早指令位置
  int end;          // 最晚指令位置
  bool isFloat;     // 浮点或整数/指针
  bool isPtr;       // 指针类型（需要整数寄存器）
};

// 基本块活跃信息。
struct BlockLiveness {
  std::unordered_set<int> use;   // 在本块定义前使用
  std::unordered_set<int> def;   // 在本块中定义
  std::unordered_set<int> liveIn;
  std::unordered_set<int> liveOut;
};

// 计算每个基本块的活跃信息（use/def/liveIn/liveOut），并为函数中的每条指令
// 分配线性位置。
std::unordered_map<core_ir::BasicBlock*, BlockLiveness> computeLiveness(
    core_ir::Function* func);

// 根据基本块活跃信息和指令顺序构造活跃区间。
std::vector<LiveInterval> buildLiveIntervals(
    core_ir::Function* func,
    const std::unordered_map<core_ir::BasicBlock*, BlockLiveness>& liveness,
    const std::unordered_map<core_ir::Instruction*, int>& instIndex);

// 为每条指令分配连续位置，供活跃区间构造使用。
std::unordered_map<core_ir::Instruction*, int> numberInstructions(core_ir::Function* func);

} // namespace analysis
