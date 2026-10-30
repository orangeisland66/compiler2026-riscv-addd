// 文件职责：计算 MIR 层活跃信息，为寄存器分配和 cleanup 提供依据。本文件负责实现具体逻辑。
#include "mir_liveness.h"

#include <algorithm>
#include <climits>
#include <functional>
#include <queue>
#include <stack>
#include <unordered_set>

namespace mir {

namespace {

// 按逆后序遍历 CFG，入口块在前、支配块先于被支配块；返回 RPO 中的块编号。
std::vector<MachineBasicBlock*> reversePostOrder(MachineFunction& mf) {
  std::vector<MachineBasicBlock*> order;
  std::unordered_set<MachineBasicBlock*> visited;
  std::stack<std::pair<MachineBasicBlock*, size_t>> stk;
  MachineBasicBlock* entry = mf.entryBlock();
  if (!entry) return order;
  stk.push({entry, 0});
  visited.insert(entry);
  // 迭代计算后序。
  std::vector<MachineBasicBlock*> postOrder;
  while (!stk.empty()) {
    auto& top = stk.top();
    auto* bb = top.first;
    if (top.second < bb->succs().size()) {
      auto* s = bb->succs()[top.second++];
      if (!visited.count(s)) {
        visited.insert(s);
        stk.push({s, 0});
      }
    } else {
      postOrder.push_back(bb);
      stk.pop();
    }
  }
  order.assign(postOrder.rbegin(), postOrder.rend());
  return order;
}

std::unordered_set<MachineBasicBlock*> computeReachable(MachineFunction& mf) {
  std::unordered_set<MachineBasicBlock*> reachable;
  auto* entry = mf.entryBlock();
  if (!entry) return reachable;
  std::vector<MachineBasicBlock*> stack = {entry};
  reachable.insert(entry);
  while (!stack.empty()) {
    MachineBasicBlock* bb = stack.back();
    stack.pop_back();
    for (auto* succ : bb->succs()) {
      if (succ && reachable.insert(succ).second) stack.push_back(succ);
    }
  }
  return reachable;
}

std::unordered_set<MachineBasicBlock*> intersectSets(
    const std::unordered_set<MachineBasicBlock*>& a,
    const std::unordered_set<MachineBasicBlock*>& b) {
  const auto* small = &a;
  const auto* large = &b;
  if (small->size() > large->size()) std::swap(small, large);
  std::unordered_set<MachineBasicBlock*> out;
  for (auto* bb : *small) {
    if (large->count(bb)) out.insert(bb);
  }
  return out;
}

std::unordered_map<MachineBasicBlock*, std::unordered_set<MachineBasicBlock*>>
computeDominators(MachineFunction& mf,
                  const std::vector<MachineBasicBlock*>& rpo) {
  std::unordered_map<MachineBasicBlock*, std::unordered_set<MachineBasicBlock*>> doms;
  if (rpo.empty()) return doms;

  std::unordered_set<MachineBasicBlock*> reachable(rpo.begin(), rpo.end());
  for (auto* bb : rpo) doms[bb] = reachable;
  doms[rpo.front()] = {rpo.front()};

  bool changed = true;
  while (changed) {
    changed = false;
    for (size_t i = 1; i < rpo.size(); ++i) {
      MachineBasicBlock* bb = rpo[i];
      bool seeded = false;
      std::unordered_set<MachineBasicBlock*> newDom;
      for (auto* pred : bb->preds()) {
        if (!pred || !reachable.count(pred)) continue;
        if (!seeded) {
          newDom = doms[pred];
          seeded = true;
        } else {
          newDom = intersectSets(newDom, doms[pred]);
        }
      }
      if (!seeded) newDom.clear();
      newDom.insert(bb);
      if (newDom != doms[bb]) {
        doms[bb] = std::move(newDom);
        changed = true;
      }
    }
  }
  return doms;
}

// 使用支配关系和回边近似计算自然循环嵌套深度。对每条 tail -> header 边，
// 当 header 支配 tail 时，沿反向 CFG 从 tail 回溯到 header 收集自然循环。
std::unordered_map<MachineBasicBlock*, int> computeLoopDepth(MachineFunction& mf) {
  std::unordered_map<MachineBasicBlock*, int> depth;
  auto rpo = reversePostOrder(mf);
  auto doms = computeDominators(mf, rpo);
  std::unordered_set<MachineBasicBlock*> reachable(rpo.begin(), rpo.end());

  for (auto* tail : rpo) {
    for (auto* header : tail->succs()) {
      if (!header || !reachable.count(header)) continue;
      auto tailDomIt = doms.find(tail);
      if (tailDomIt == doms.end() || !tailDomIt->second.count(header)) continue;

      std::unordered_set<MachineBasicBlock*> loopBlocks;
      loopBlocks.insert(header);
      std::vector<MachineBasicBlock*> stack = {tail};
      while (!stack.empty()) {
        MachineBasicBlock* bb = stack.back();
        stack.pop_back();
        if (!loopBlocks.insert(bb).second) continue;
        for (auto* pred : bb->preds()) {
          if (pred && reachable.count(pred) && pred != header) stack.push_back(pred);
        }
      }
      for (auto* bb : loopBlocks) depth[bb]++;
    }
  }
  return depth;
}

std::unordered_set<int> collectCrossCallVRegs(
    MachineFunction& mf,
    const std::unordered_map<MachineBasicBlock*, BlockLiveness>& liveness) {
  std::unordered_set<int> out;
  for (auto& bb : mf.blocks()) {
    auto it = liveness.find(bb.get());
    if (it == liveness.end())
      continue;

    std::unordered_set<int> live = it->second.liveOut;
    for (auto instIt = bb->insts().rbegin(); instIt != bb->insts().rend(); ++instIt) {
      MachineInst* inst = instIt->get();
      if (!inst || inst->opcode() == MIOp::PHI)
        continue;

      std::vector<int> defs;
      std::vector<int> uses;
      for (auto& op : inst->operands()) {
        if (!op.isReg() || !isVirtualReg(op.reg))
          continue;
        if (op.isDef)
          defs.push_back(op.reg);
        else
          uses.push_back(op.reg);
      }

      if (mioIsCall(inst->opcode())) {
        for (int v : live) {
          if (isVirtualReg(v))
            out.insert(v);
        }
      }

      for (int def : defs)
        live.erase(def);
      for (int use : uses)
        live.insert(use);
    }
  }
  return out;
}

} // namespace

void numberInstructions(MachineFunction& mf) {
  int pos = 0;
  // 按逆后序遍历，使位置反映稳定的线性顺序。
  auto rpo = reversePostOrder(mf);
  for (auto* bb : rpo) {
    for (auto& inst : bb->insts()) {
      // 指令之间保留一个位置间隔；使用发生在 pos，定义发生在 pos + 1，
      // 这样结果可以复用同一条 RISC-V 指令已消耗的源寄存器。
      inst->pos = pos;
      pos += 2;
    }
  }
  // 不在 RPO 中的块视为不可达并放到较后位置；正常输入不会出现这种情况，
  // 这里仍保留防御性处理。
  for (auto& bb : mf.blocks()) {
    for (auto& inst : bb->insts()) {
      if (inst->pos < 0) {
        inst->pos = pos;
        pos += 2;
      }
    }
  }
}

std::unordered_map<MachineBasicBlock*, BlockLiveness> computeLiveness(
    MachineFunction& mf) {
  std::unordered_map<MachineBasicBlock*, BlockLiveness> bl;

  // 正向扫描指令，计算每个基本块的 use/def 集合。
  for (auto& bb : mf.blocks()) {
    BlockLiveness L;
    std::unordered_set<int> definedHere;
    for (auto& inst : bb->insts()) {
      // PHI 需要特殊处理：它的操作数属于前驱块的 live-out，
      // 不作为当前块内的普通使用。
      if (inst->opcode() == MIOp::PHI) {
        // operand(0) 是定义，后续操作数按（使用值、基本块）成对出现。
        if (inst->numOperands() > 0 && inst->operand(0).isReg()) {
          int v = inst->operand(0).reg;
          if (isVirtualReg(v)) {
            definedHere.insert(v);
            L.def.insert(v);
          }
        }
        continue;
      }
      // 先处理使用值。
      for (auto& op : inst->operands()) {
        if (op.isReg() && !op.isDef && isVirtualReg(op.reg)) {
          if (!definedHere.count(op.reg)) {
            L.use.insert(op.reg);
          }
        }
      }
      // 再处理定义值。
      for (auto& op : inst->operands()) {
        if (op.isReg() && op.isDef && isVirtualReg(op.reg)) {
          definedHere.insert(op.reg);
          L.def.insert(op.reg);
        }
      }
    }
    bl[bb.get()] = std::move(L);
  }

  // PHI 操作数需要传递到前驱：传入寄存器属于对应前驱块的 live-out。
  // 这里先把这些寄存器写入每个前驱的 liveOut 集合作为初始值。
  std::unordered_map<MachineBasicBlock*, std::unordered_set<int>> phiLeak;
  for (auto& bb : mf.blocks()) {
    for (auto& inst : bb->insts()) {
      // 调度可能在多个 PHI 之间放置非 PHI 指令，因此扫描整个基本块，让靠后的
      // PHI 仍然贡献对应边上的使用信息。
      if (inst->opcode() != MIOp::PHI) continue;
      // 操作数形式为：定义值，随后是（寄存器、基本块）对。
      for (size_t i = 1; i + 1 < inst->numOperands(); i += 2) {
        const MOperand& regOp = inst->operand(i);
        const MOperand& bbOp  = inst->operand(i + 1);
        if (!regOp.isReg() || !bbOp.isMBB()) continue;
        if (isVirtualReg(regOp.reg) && bbOp.mbb)
          phiLeak[bbOp.mbb].insert(regOp.reg);
      }
    }
  }

  // 按 reverse-RPO 迭代到结果稳定。活跃性是反向分析，
  // 因此先访问后继块再回到当前块。
  auto rpo = reversePostOrder(mf);
  std::vector<MachineBasicBlock*> reverseRPO(rpo.rbegin(), rpo.rend());
  bool changed = true;
  while (changed) {
    changed = false;
    for (auto* bb : reverseRPO) {
      auto& L = bl[bb];
      std::unordered_set<int> newOut;
      // 从 PHI 传出的寄存器用于初始化后继块的 PHI 使用值。
      auto pit = phiLeak.find(bb);
      if (pit != phiLeak.end()) {
        for (int v : pit->second) newOut.insert(v);
      }
      for (auto* s : bb->succs()) {
        for (int v : bl[s].liveIn) newOut.insert(v);
      }
      // liveIn = use ∪（liveOut - def）。
      std::unordered_set<int> newIn = L.use;
      for (int v : newOut) {
        if (!L.def.count(v)) newIn.insert(v);
      }
      if (newOut != L.liveOut || newIn != L.liveIn) {
        L.liveOut = std::move(newOut);
        L.liveIn = std::move(newIn);
        changed = true;
      }
    }
  }
  return bl;
}

std::vector<LiveInterval> buildLiveIntervals(
    MachineFunction& mf,
    const std::unordered_map<MachineBasicBlock*, BlockLiveness>& liveness) {
  numberInstructions(mf);
  auto crossCallVRegs = collectCrossCallVRegs(mf, liveness);

  // 计算每个基本块的循环深度；直线代码为 0，循环内至少为 1。
  auto blockDepth = computeLoopDepth(mf);

  // 对每个虚拟寄存器累计（start、end、crossesCall、useCount）。
  std::unordered_map<int, LiveInterval> intervals;
  auto touch = [&](int vreg, int pos, RegClass cls) {
    auto it = intervals.find(vreg);
    if (it == intervals.end()) {
      LiveInterval iv;
      iv.vreg = vreg;
      iv.cls = cls;
      iv.start = pos;
      iv.end = pos;
      iv.crossesCall = false;
      iv.loopDepth = 0;
      iv.useCount = 0;
      iv.rematerializable = mf.vregInfo(vreg).rematerializable;
      intervals[vreg] = iv;
    } else {
      it->second.start = std::min(it->second.start, pos);
      it->second.end   = std::max(it->second.end, pos);
    }
  };

  // 辅助函数：取得基本块的起止位置。
  std::unordered_map<MachineBasicBlock*, std::pair<int,int>> blockRange;
  std::unordered_map<MachineBasicBlock*, int> blockHeat;
  for (auto& bb : mf.blocks()) {
    int lo = INT_MAX, hi = -1;
    for (auto& inst : bb->insts()) {
      lo = std::min(lo, inst->pos);
      // liveOut 位于最后一条指令的定义之后。
      hi = std::max(hi, inst->pos + 1);
    }
    if (hi < 0) { lo = 0; hi = 0; }
    blockRange[bb.get()] = {lo, hi};
    int depth = 0;
    auto dIt = blockDepth.find(bb.get());
    if (dIt != blockDepth.end()) depth = dIt->second;
    blockHeat[bb.get()] = 1 + depth * 4;
  }

  using VRegRangeMap = std::unordered_map<int, std::pair<int, int>>;
  std::unordered_map<MachineBasicBlock*, VRegRangeMap> localRanges;
  auto noteLocalPosition = [&](MachineBasicBlock* bb, int vreg, int pos) {
    auto& ranges = localRanges[bb];
    auto it = ranges.find(vreg);
    if (it == ranges.end()) {
      ranges.emplace(vreg, std::make_pair(pos, pos));
    } else {
      it->second.first = std::min(it->second.first, pos);
      it->second.second = std::max(it->second.second, pos);
    }
  };

  // 对每个基本块：liveIn 中的虚拟寄存器从块起点开始，liveOut 中的虚拟寄存器
  // 延伸到块终点；指令 i 的定义从 i 开始，使用把区间终点延伸到 i。
  for (auto& bb : mf.blocks()) {
    auto rng = blockRange[bb.get()];
    int heat = blockHeat[bb.get()];
    int bbDepth = 0;
    auto dIt = blockDepth.find(bb.get());
    if (dIt != blockDepth.end()) bbDepth = dIt->second;
    auto it = liveness.find(bb.get());
    if (it == liveness.end()) continue;
    const auto& L = it->second;
    for (int v : L.liveIn) {
      // 无法确定时先使用 GPR 作为默认类别，后面再细化。
      RegClass cls = mf.vregInfo(v).cls;
      touch(v, rng.first, cls);
      noteLocalPosition(bb.get(), v, rng.first);
      auto& iv = intervals[v];
      if (bbDepth > iv.loopDepth) iv.loopDepth = bbDepth;
    }
    for (int v : L.liveOut) {
      RegClass cls = mf.vregInfo(v).cls;
      touch(v, rng.second, cls);
      noteLocalPosition(bb.get(), v, rng.second);
      auto& iv = intervals[v];
      if (bbDepth > iv.loopDepth) iv.loopDepth = bbDepth;
    }
    for (auto& inst : bb->insts()) {
      const int usePos = inst->pos;
      const int defPos = inst->pos + 1;

      // PHI 输入是边上的使用，不是后继块内的使用。
      // 如果把它们记在循环头，会让回边值贯穿整个线性化循环，
      // 从而产生不必要的寄存器冲突。
      if (inst->opcode() == MIOp::PHI) {
        if (inst->numOperands() > 0) {
          const MOperand& def = inst->operand(0);
          if (def.isReg() && def.isDef && isVirtualReg(def.reg)) {
            touch(def.reg, defPos, def.regClass);
            noteLocalPosition(bb.get(), def.reg, defPos);
            auto& iv = intervals[def.reg];
            if (bbDepth > iv.loopDepth) iv.loopDepth = bbDepth;
          }
        }
        for (size_t index = 1; index + 1 < inst->numOperands(); index += 2) {
          const MOperand& source = inst->operand(index);
          const MOperand& predecessor = inst->operand(index + 1);
          if (!source.isReg() || !isVirtualReg(source.reg) ||
              !predecessor.isMBB() || !predecessor.mbb)
            continue;
          auto rangeIt = blockRange.find(predecessor.mbb);
          if (rangeIt == blockRange.end())
            continue;
          const int edgePos = rangeIt->second.second;
          touch(source.reg, edgePos, source.regClass);
          noteLocalPosition(predecessor.mbb, source.reg, edgePos);
          auto& iv = intervals[source.reg];
          int predecessorDepth = 0;
          auto depthIt = blockDepth.find(predecessor.mbb);
          if (depthIt != blockDepth.end())
            predecessorDepth = depthIt->second;
          if (predecessorDepth > iv.loopDepth)
            iv.loopDepth = predecessorDepth;
          const int edgeHeat = blockHeat[predecessor.mbb];
          iv.useCount++;
          iv.hotUseCount += edgeHeat;
          iv.hotLoopWeight +=
              edgeHeat * std::max(1, predecessorDepth + 1);
          iv.usePositions.push_back(edgePos);
        }
        continue;
      }

      // 使用发生在定义之前；分割位置让线性扫描可以安全地为指令结果复用
      // 已经结束活跃期的源寄存器。
      for (auto& op : inst->operands()) {
        if (!op.isReg() || op.isDef || !isVirtualReg(op.reg))
          continue;
        touch(op.reg, usePos, op.regClass);
        noteLocalPosition(bb.get(), op.reg, usePos);
        auto& iv = intervals[op.reg];
        if (bbDepth > iv.loopDepth) iv.loopDepth = bbDepth;
        iv.useCount++;
        iv.hotUseCount += heat;
        iv.hotLoopWeight += heat * std::max(1, bbDepth + 1);
        iv.usePositions.push_back(usePos);
      }
      for (auto& op : inst->operands()) {
        if (!op.isReg() || !op.isDef || !isVirtualReg(op.reg))
          continue;
        touch(op.reg, defPos, op.regClass);
        noteLocalPosition(bb.get(), op.reg, defPos);
        auto& iv = intervals[op.reg];
        if (bbDepth > iv.loopDepth) iv.loopDepth = bbDepth;
      }
    }
  }

  // 每个基本块保留一个覆盖区间。贯穿整个块的值记录两个边界位置，局部临时值
  // 只覆盖第一次和最后一次定义或使用；分块区间可以保留 CFG 中的真实空洞。
  for (auto& blockEntry : localRanges) {
    for (const auto& valueEntry : blockEntry.second) {
      auto intervalIt = intervals.find(valueEntry.first);
      if (intervalIt == intervals.end())
        continue;
      intervalIt->second.segments.push_back(
          {valueEntry.second.first, valueEntry.second.second});
    }
  }

  for (auto& kv : intervals) {
    auto& iv = kv.second;
    std::sort(iv.segments.begin(), iv.segments.end(),
              [](const LiveSegment& lhs, const LiveSegment& rhs) {
                if (lhs.start != rhs.start) return lhs.start < rhs.start;
                return lhs.end < rhs.end;
              });
    std::vector<LiveSegment> merged;
    for (const LiveSegment& segment : iv.segments) {
      if (merged.empty() || segment.start > merged.back().end) {
        merged.push_back(segment);
      } else {
        merged.back().end = std::max(merged.back().end, segment.end);
      }
    }
    iv.segments = std::move(merged);
    if (iv.segments.empty())
      iv.segments.push_back({iv.start, iv.end});
    iv.start = iv.segments.front().start;
    iv.end = iv.segments.back().end;
    iv.crossesCall = crossCallVRegs.count(iv.vreg) != 0;
    iv.preferCallerSaved =
        !iv.crossesCall &&
        (iv.hotLoopWeight >= 12 || (iv.loopDepth > 0 && iv.useCount >= 2));
    std::sort(iv.usePositions.begin(), iv.usePositions.end());
  }

  std::vector<LiveInterval> out;
  out.reserve(intervals.size());
  for (auto& kv : intervals) out.push_back(kv.second);
  std::sort(out.begin(), out.end(),
            [](const LiveInterval& a, const LiveInterval& b) { return a.start < b.start; });
  return out;
}

LivenessResult analyzeLiveness(MachineFunction& mf) {
  LivenessResult r;
  r.blockLiveness = computeLiveness(mf);
  r.intervals = buildLiveIntervals(mf, r.blockLiveness);
  return r;
}

} // namespace mir
