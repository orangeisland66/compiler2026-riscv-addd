// 文件职责：分析 CoreIR 值活跃区间，供寄存器分配和优化收益判断使用。本文件负责实现具体逻辑。
#include "liveness.h"
#include "cfg_utils.h"
#include <algorithm>

namespace analysis {

std::unordered_map<core_ir::Instruction*, int> numberInstructions(core_ir::Function* func) {
  std::unordered_map<core_ir::Instruction*, int> index;
  int cur = 0;
  for (auto& bb : func->blocks()) {
    for (auto& instPtr : bb->instructions()) {
      index[instPtr.get()] = cur++;
    }
  }
  return index;
}

std::unordered_map<core_ir::BasicBlock*, BlockLiveness> computeLiveness(
    core_ir::Function* func) {
  std::unordered_map<core_ir::BasicBlock*, BlockLiveness> result;
  if (!func) return result;

  auto reachable = collectReachableBlocks(func);

  // 收集所有基本块。
  std::vector<core_ir::BasicBlock*> blocks;
  for (auto& bb : func->blocks()) {
    if (reachable.count(bb.get())) {
      blocks.push_back(bb.get());
      result[bb.get()] = {};
    }
  }
  if (blocks.empty()) return result;

  // 计算每个基本块的局部 use/def。
  for (auto* bb : blocks) {
    auto& info = result[bb];
    for (auto it = bb->instructions().rbegin(); it != bb->instructions().rend(); ++it) {
      core_ir::Instruction* inst = it->get();
      int instId = inst->id();

  // use：尚未在本块定义的操作数。
      for (int i = 0; i < inst->numOperands(); ++i) {
        core_ir::Value* op = inst->operand(i);
        if (!op || op->isConstant()) continue;
        if (dynamic_cast<core_ir::BasicBlock*>(op)) continue;
        if (dynamic_cast<core_ir::GlobalVar*>(op)) continue;
        if (dynamic_cast<core_ir::Argument*>(op)) continue;
        int opId = op->id();
        if (!info.def.count(opId))
          info.use.insert(opId);
      }

  // def：当前指令定义的值。
      if (!inst->type().isVoid())
        info.def.insert(instId);
    }
  }

  // 通过反向数据流计算 liveIn / liveOut。
  bool changed = true;
  while (changed) {
    changed = false;
    for (auto* bb : blocks) {
      auto& info = result[bb];

  // liveOut = 所有后继的 liveIn 并集。
      std::unordered_set<int> newLiveOut;
      for (auto* succ : bb->succs()) {
        if (!succ || !reachable.count(succ)) continue;
        auto& succInfo = result[succ];
        newLiveOut.insert(succInfo.liveIn.begin(), succInfo.liveIn.end());
      }

      if (newLiveOut != info.liveOut) {
        info.liveOut = std::move(newLiveOut);
        changed = true;
      }

  // liveIn = use ∪ (liveOut - def)。
      std::unordered_set<int> newLiveIn = info.use;
      for (int id : info.liveOut) {
        if (!info.def.count(id))
          newLiveIn.insert(id);
      }

      if (newLiveIn != info.liveIn) {
        info.liveIn = std::move(newLiveIn);
        changed = true;
      }
    }
  }

  return result;
}

std::vector<LiveInterval> buildLiveIntervals(
    core_ir::Function* func,
    const std::unordered_map<core_ir::BasicBlock*, BlockLiveness>& liveness,
    const std::unordered_map<core_ir::Instruction*, int>& instIndex) {
  std::vector<LiveInterval> intervals;

  // 对每个值（指令结果）查找活跃范围，从定义位置延伸到最后一次使用。
  for (auto& bb : func->blocks()) {
    for (auto& instPtr : bb->instructions()) {
      core_ir::Instruction* inst = instPtr.get();
      if (inst->type().isVoid()) continue;

      int defIdx = -1;
      auto defIt = instIndex.find(inst);
      if (defIt != instIndex.end())
        defIdx = defIt->second;

  // 扫描所有指令，查找最后一次使用。
      int lastUse = defIdx;
      for (auto& bb2 : func->blocks()) {
        for (auto& instPtr2 : bb2->instructions()) {
          core_ir::Instruction* user = instPtr2.get();
          for (int i = 0; i < user->numOperands(); ++i) {
            if (user->operand(i) == inst) {
              auto useIt = instIndex.find(user);
              if (useIt != instIndex.end() && useIt->second > lastUse)
                lastUse = useIt->second;
            }
          }
        }
      }

      if (defIdx >= 0) {
        LiveInterval iv;
        iv.vregId = inst->id();
        iv.start = defIdx;
        iv.end = lastUse;
        iv.isFloat = inst->type().isF32();
        iv.isPtr = inst->type().isPtr();
        intervals.push_back(iv);
      }
    }
  }

  // 按起始位置排序，供线性扫描使用。
  std::sort(intervals.begin(), intervals.end(),
            [](const LiveInterval& a, const LiveInterval& b) {
              return a.start < b.start;
            });

  return intervals;
}

} // namespace analysis
