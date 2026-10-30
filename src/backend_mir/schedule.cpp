// 文件职责：做机器指令调度和块布局，改善热路径执行形状。本文件负责实现具体逻辑。
#include "schedule.h"

#include <algorithm>
#include <climits>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mir {

namespace {

std::vector<MachineBasicBlock*> reversePostOrder(MachineFunction& mf) {
  std::vector<MachineBasicBlock*> order;
  std::unordered_set<MachineBasicBlock*> visited;
  std::vector<std::pair<MachineBasicBlock*, size_t>> stack;
  MachineBasicBlock* entry = mf.entryBlock();
  if (!entry)
    return order;
  stack.push_back({entry, 0});
  visited.insert(entry);

  std::vector<MachineBasicBlock*> postOrder;
  while (!stack.empty()) {
    auto& top = stack.back();
    MachineBasicBlock* bb = top.first;
    if (top.second < bb->succs().size()) {
      MachineBasicBlock* succ = bb->succs()[top.second++];
      if (succ && visited.insert(succ).second)
        stack.push_back({succ, 0});
    } else {
      postOrder.push_back(bb);
      stack.pop_back();
    }
  }
  order.assign(postOrder.rbegin(), postOrder.rend());
  return order;
}

std::unordered_set<MachineBasicBlock*> intersectSets(
    const std::unordered_set<MachineBasicBlock*>& a,
    const std::unordered_set<MachineBasicBlock*>& b) {
  const auto* small = &a;
  const auto* large = &b;
  if (small->size() > large->size())
    std::swap(small, large);
  std::unordered_set<MachineBasicBlock*> out;
  for (auto* bb : *small) {
    if (large->count(bb))
      out.insert(bb);
  }
  return out;
}

std::unordered_map<MachineBasicBlock*, std::unordered_set<MachineBasicBlock*>>
computeDominators(MachineFunction& mf,
                  const std::vector<MachineBasicBlock*>& rpo) {
  std::unordered_map<MachineBasicBlock*, std::unordered_set<MachineBasicBlock*>> doms;
  if (rpo.empty())
    return doms;

  std::unordered_set<MachineBasicBlock*> reachable(rpo.begin(), rpo.end());
  for (auto* bb : rpo)
    doms[bb] = reachable;
  doms[rpo.front()] = {rpo.front()};

  bool changed = true;
  while (changed) {
    changed = false;
    for (size_t i = 1; i < rpo.size(); ++i) {
      MachineBasicBlock* bb = rpo[i];
      bool seeded = false;
      std::unordered_set<MachineBasicBlock*> newDom;
      for (auto* pred : bb->preds()) {
        if (!pred || !reachable.count(pred))
          continue;
        if (!seeded) {
          newDom = doms[pred];
          seeded = true;
        } else {
          newDom = intersectSets(newDom, doms[pred]);
        }
      }
      if (!seeded)
        newDom.clear();
      newDom.insert(bb);
      if (newDom != doms[bb]) {
        doms[bb] = std::move(newDom);
        changed = true;
      }
    }
  }
  return doms;
}

std::unordered_map<MachineBasicBlock*, int> computeLoopDepth(MachineFunction& mf) {
  std::unordered_map<MachineBasicBlock*, int> depth;
  auto rpo = reversePostOrder(mf);
  auto doms = computeDominators(mf, rpo);
  std::unordered_set<MachineBasicBlock*> reachable(rpo.begin(), rpo.end());

  for (auto* tail : rpo) {
    for (auto* header : tail->succs()) {
      if (!header || !reachable.count(header))
        continue;
      auto it = doms.find(tail);
      if (it == doms.end() || !it->second.count(header))
        continue;

      std::unordered_set<MachineBasicBlock*> loopBlocks;
      loopBlocks.insert(header);
      std::vector<MachineBasicBlock*> stack = {tail};
      while (!stack.empty()) {
        MachineBasicBlock* bb = stack.back();
        stack.pop_back();
        if (!loopBlocks.insert(bb).second)
          continue;
        for (auto* pred : bb->preds()) {
          if (pred && reachable.count(pred) && pred != header)
            stack.push_back(pred);
        }
      }
      for (auto* bb : loopBlocks)
        depth[bb]++;
    }
  }
  return depth;
}

int latencyOf(MIOp op) {
  switch (op) {
    case MIOp::MUL: case MIOp::MULW:
      return 3;
    case MIOp::DIV: case MIOp::DIVW: case MIOp::DIVU: case MIOp::DIVUW:
    case MIOp::REM: case MIOp::REMW: case MIOp::REMU: case MIOp::REMUW:
      return 20;
    case MIOp::LB: case MIOp::LH: case MIOp::LW: case MIOp::LD:
    case MIOp::LBU: case MIOp::LHU: case MIOp::LWU: case MIOp::FLW:
      return 3;
    case MIOp::FADD_S: case MIOp::FSUB_S: case MIOp::FMUL_S:
    case MIOp::FMADD_S: case MIOp::FMSUB_S: case MIOp::FNMADD_S: case MIOp::FNMSUB_S:
      return 4;
    case MIOp::FDIV_S: case MIOp::FSQRT_S:
      return 10;
    default:
      return 1;
  }
}

bool isMemOp(MIOp op) { return mioMayLoad(op) || mioMayStore(op); }

bool isLongLatencyOp(MIOp op) {
  return mioMayLoad(op) ||
         op == MIOp::DIV || op == MIOp::DIVW || op == MIOp::DIVU || op == MIOp::DIVUW ||
         op == MIOp::REM || op == MIOp::REMW || op == MIOp::REMU || op == MIOp::REMUW ||
         op == MIOp::FDIV_S || op == MIOp::FSQRT_S;
}

bool isDelayFillerOp(MIOp op) {
  switch (op) {
    case MIOp::ADD:
    case MIOp::ADDW:
    case MIOp::ADDI:
    case MIOp::ADDIW:
    case MIOp::SUB:
    case MIOp::SUBW:
    case MIOp::SLL:
    case MIOp::SLLW:
    case MIOp::SLLI:
    case MIOp::SLLIW:
    case MIOp::SRL:
    case MIOp::SRLW:
    case MIOp::SRLI:
    case MIOp::SRLIW:
    case MIOp::SRA:
    case MIOp::SRAW:
    case MIOp::SRAI:
    case MIOp::SRAIW:
    case MIOp::SLT:
    case MIOp::SLTU:
    case MIOp::SLTI:
    case MIOp::SLTIU:
    case MIOp::AND_:
    case MIOp::ANDI:
    case MIOp::OR_:
    case MIOp::ORI:
    case MIOp::XOR_:
    case MIOp::XORI:
    case MIOp::MV:
    case MIOp::FMV_S:
      return true;
    default:
      return false;
  }
}

MIOp invertBranchOpcode(MIOp op) {
  switch (op) {
    case MIOp::BEQ: return MIOp::BNE;
    case MIOp::BNE: return MIOp::BEQ;
    case MIOp::BLT: return MIOp::BGE;
    case MIOp::BGE: return MIOp::BLT;
    case MIOp::BLTU: return MIOp::BGEU;
    case MIOp::BGEU: return MIOp::BLTU;
    case MIOp::BEQZ: return MIOp::BNEZ;
    case MIOp::BNEZ: return MIOp::BEQZ;
    case MIOp::BLTZ: return MIOp::BGEZ;
    case MIOp::BGEZ: return MIOp::BLTZ;
    case MIOp::BLEZ: return MIOp::BGTZ;
    case MIOp::BGTZ: return MIOp::BLEZ;
    default:
      return MIOp::COUNT;
  }
}

bool isPinnedPhiCopy(const MachineInst* inst) {
  return inst && inst->comment.rfind("phi-", 0) == 0;
}

bool isFPOp(const MachineInst* inst) {
  for (const auto& opnd : inst->operands()) {
    if (opnd.isReg() && opnd.regClass == RegClass::FPR) return true;
  }
  return mioWritesFPR(inst->opcode());
}

int issueKind(const MachineInst* inst) {
  if (isMemOp(inst->opcode())) return 0;
  if (isFPOp(inst)) return 2;
  return 1;
}

bool isABIArgPhysReg(int reg) {
  switch (reg) {
    case gpr::a0: case gpr::a1: case gpr::a2: case gpr::a3:
    case gpr::a4: case gpr::a5: case gpr::a6: case gpr::a7:
    case fpr::fa0: case fpr::fa1: case fpr::fa2: case fpr::fa3:
    case fpr::fa4: case fpr::fa5: case fpr::fa6: case fpr::fa7:
      return true;
    default:
      return false;
  }
}

bool isABIBoundaryBlock(const MachineBasicBlock& bb) {
  const MachineFunction* mf = bb.parent();
  if (mf && mf->entryBlock() == &bb &&
      (mf->incomingIntArgs > 0 || mf->incomingFloatArgs > 0)) {
    return true;
  }
  for (const auto& inst : bb.insts()) {
    if (inst->opcode() == MIOp::RET)
      return true;
  }
  return false;
}

struct Node {
  MachineInst* inst = nullptr;
  int idx = 0;
  int latency = 1;
  int height = 0;          // 从该节点开始的最长关键路径（周期数）
  int readyCycle = 0;      // 该节点最早可以发射的周期
  int unmetDeps = 0;
  std::vector<int> succs;
};

struct MemAccess {
  bool valid = false;
  int baseReg = -1;
  int64_t offset = 0;
  int size = 0;
};

int memAccessSize(MIOp op) {
  switch (op) {
    case MIOp::LB:
    case MIOp::LBU:
    case MIOp::SB:
      return 1;
    case MIOp::LH:
    case MIOp::LHU:
    case MIOp::SH:
      return 2;
    case MIOp::LW:
    case MIOp::LWU:
    case MIOp::SW:
    case MIOp::FLW:
    case MIOp::FSW:
      return 4;
    case MIOp::LD:
    case MIOp::SD:
      return 8;
    default:
      return 0;
  }
}

MemAccess analyzeMemAccess(const MachineInst* inst) {
  MemAccess access;
  if (!inst || (!mioMayLoad(inst->opcode()) && !mioMayStore(inst->opcode())))
    return access;
  if (inst->numOperands() < 3)
    return access;
  // 加载和存储使用 [值或目的寄存器、立即数偏移、基址寄存器] 的操作数布局。
  const auto& off = inst->operand(1);
  const auto& base = inst->operand(2);
  if (!base.isReg() || !off.isImm())
    return access;
  access.valid = true;
  access.baseReg = base.reg;
  access.offset = off.imm;
  access.size = memAccessSize(inst->opcode());
  if (access.size <= 0)
    access.valid = false;
  return access;
}

bool mayAlias(const MemAccess& a, const MemAccess& b) {
  if (!a.valid || !b.valid)
    return true;
  if (a.baseReg != b.baseReg)
    return true;
  // 即使两个有符号偏移覆盖整个 int64_t 范围，无符号减法也能得到准确的
  // 非负距离。
  if (a.offset <= b.offset) {
    const std::uint64_t distance = static_cast<std::uint64_t>(b.offset) -
                                   static_cast<std::uint64_t>(a.offset);
    return distance < static_cast<std::uint64_t>(a.size);
  }
  const std::uint64_t distance = static_cast<std::uint64_t>(a.offset) -
                                 static_cast<std::uint64_t>(b.offset);
  return distance < static_cast<std::uint64_t>(b.size);
}

void addDep(std::vector<Node>& nodes, int from, int to) {
  if (from == to) return;
  // 避免重复边；这里的 O(d) 检查成本很低，实际节点度数通常很小。
  for (int s : nodes[from].succs) if (s == to) return;
  nodes[from].succs.push_back(to);
  nodes[to].unmetDeps++;
}

void scheduleBlock(MachineBasicBlock& bb) {
  auto& insts = bb.insts();
  int N = static_cast<int>(insts.size());
  if (N <= 1) return;

  // 找到终结指令开始的位置；终结指令始终保留在块尾。
  int termStart = N;
  for (int i = N - 1; i >= 0; --i) {
    if (mioIsTerminator(insts[i]->opcode()) || isPinnedPhiCopy(insts[i].get())) {
      termStart = i;
    }
    else break;
  }
  if (termStart < 2) return; // 没有可调度的有效指令

  int M = termStart;
  // 很大的展开块已经带来较高的寄存器和指令缓存压力。局部延迟启发式容易
  // 分散重复 lane，依赖构造还是二次复杂度，因此保留产生者的局部顺序。
  constexpr int kMaxScheduledInsts = 192;
  if (M > kMaxScheduledInsts)
    return;
  const bool protectABIBoundary = isABIBoundaryBlock(bb);
  for (int i = 0; i < M; ++i) {
    if (insts[i]->comment == "spill" || insts[i]->comment == "reload") {
      // 让分配器生成的栈流量紧邻其对应值。Frame-index 访问按可能别名处理，
      // 分散它们会削弱调度后的回存前递清理。
      return;
    }
    if (mioIsCall(insts[i]->opcode())) {
      // 当前调度器对寄存器/内存依赖是保守建模，但 call 前的
      // ABI 实参准备仍然包含大量 pre-colored 物理寄存器与 outgoing stack
      // slot 协同。为了先保证通用 correctness，这里直接跳过含 call 的块，
      // 避免把实参 setup 重排到错误位置；纯计算热块仍可继续调度。
      return;
    }
    if (!protectABIBoundary)
      continue;
    for (const auto& opnd : insts[i]->operands()) {
      if (!opnd.isReg() || !isPhysReg(opnd.reg))
        continue;
      if (!isABIArgPhysReg(opnd.reg))
        continue;
      // 只有入口和返回块携带无调用的 ABI 传输组。其他 a/fa 寄存器使用可能是
      // 普通分配后的寄存器，不应因此关闭整个热点块的调度。
      return;
    }
  }

  std::vector<Node> nodes(M);
  for (int i = 0; i < M; ++i) {
    nodes[i].inst = insts[i].get();
    nodes[i].idx = i;
    nodes[i].latency = latencyOf(nodes[i].inst->opcode());
  }

  // 构造依赖边。
  std::unordered_map<int, int> lastDef;                    // physReg -> idx
  std::unordered_map<int, std::vector<int>> usesSinceDef;  // physReg -> idxs
  int lastCallIdx = -1;
  std::vector<MemAccess> memAccesses(M);
  std::vector<int> priorLoads;
  std::vector<int> priorStores;

  for (int i = 0; i < M; ++i) {
    MachineInst* mi = nodes[i].inst;
    MIOp op = mi->opcode();
    bool isLoad = mioMayLoad(op);
    bool isStore = mioMayStore(op);
    bool isCall = mioIsCall(op);
    if (isLoad || isStore)
      memAccesses[i] = analyzeMemAccess(mi);

    // 调用之后的每条指令都依赖该调用，因为调用会覆盖调用者保存寄存器并可能
    // 修改内存。
    if (lastCallIdx >= 0) addDep(nodes, lastCallIdx, i);

  // RAW：每次使用都必须位于该寄存器最近一次定义之后。
    for (auto& opnd : mi->operands()) {
      if (!opnd.isReg() || opnd.isDef) continue;
      // 跳过常量零寄存器 x0，它没有定义链。
      if (opnd.reg == gpr::x0) continue;
      auto it = lastDef.find(opnd.reg);
      if (it != lastDef.end()) addDep(nodes, it->second, i);
    }

    // 内存访问顺序。
    if (isLoad) {
      for (int s : priorStores) {
        if (mayAlias(memAccesses[s], memAccesses[i]))
          addDep(nodes, s, i);
      }
      priorLoads.push_back(i);
    }
    if (isStore) {
      for (int s : priorStores) {
        if (mayAlias(memAccesses[s], memAccesses[i]))
          addDep(nodes, s, i);
      }
      for (int l : priorLoads) {
        if (mayAlias(memAccesses[l], memAccesses[i]))
          addDep(nodes, l, i);
      }
      priorStores.push_back(i);
    }

    // 调用依赖之前已经调度的工作，包括 outgoing 参数准备和内存状态；块较小，
    // 这种依赖关系的开销可以接受。
    if (isCall) {
      for (int j = 0; j < i; ++j) addDep(nodes, j, i);
    }

    // WAW / WAR：定义必须位于同一寄存器此前的定义或使用之后。
    for (auto& opnd : mi->operands()) {
      if (!opnd.isReg() || !opnd.isDef) continue;
      if (opnd.reg == gpr::x0) continue;
      auto itD = lastDef.find(opnd.reg);
      if (itD != lastDef.end()) addDep(nodes, itD->second, i);
      auto itU = usesSinceDef.find(opnd.reg);
      if (itU != usesSinceDef.end()) {
        for (int u : itU->second) addDep(nodes, u, i);
        itU->second.clear();
      }
      lastDef[opnd.reg] = i;
    }
    // 记录使用位置，供后续 WAR 分析使用。
    for (auto& opnd : mi->operands()) {
      if (!opnd.isReg() || opnd.isDef) continue;
      if (opnd.reg == gpr::x0) continue;
      usesSinceDef[opnd.reg].push_back(i);
    }

    if (isStore) {
    }
    if (isCall) {
      lastCallIdx = i;
      priorLoads.clear();
      priorStores.clear();
    }
  }

  // 按逆拓扑序计算节点高度。
  std::vector<int> indegree(M, 0);
  for (int i = 0; i < M; ++i) indegree[i] = nodes[i].unmetDeps;
  std::vector<int> topo;
  topo.reserve(M);
  std::vector<int> stack;
  for (int i = 0; i < M; ++i) if (indegree[i] == 0) stack.push_back(i);
  while (!stack.empty()) {
    int v = stack.back(); stack.pop_back();
    topo.push_back(v);
    for (int s : nodes[v].succs) {
      if (--indegree[s] == 0) stack.push_back(s);
    }
  }
  if (static_cast<int>(topo.size()) != M) {
    // 检测到环，退出并保持基本块原状。
    return;
  }
  for (auto it = topo.rbegin(); it != topo.rend(); ++it) {
    int v = *it;
    int h = nodes[v].latency;
    for (int s : nodes[v].succs) {
      h = std::max(h, nodes[v].latency + nodes[s].height);
    }
    nodes[v].height = h;
  }

  // 执行列表调度。
  // 重新初始化未满足的依赖，开始新一轮遍历。
  for (int i = 0; i < M; ++i) nodes[i].unmetDeps = 0;
  for (int i = 0; i < M; ++i)
    for (int s : nodes[i].succs)
      nodes[s].unmetDeps++;

  std::vector<int> ready;
  std::vector<char> enqueued(M, 0);
  for (int i = 0; i < M; ++i)
    if (nodes[i].unmetDeps == 0) {
      ready.push_back(i);
      enqueued[i] = 1;
    }

  std::vector<int> scheduled;
  scheduled.reserve(M);
  int cycle = 0;
  while (!ready.empty()) {
    auto betterReady = [&](int a, int b, int firstIssued) {
      bool ar = nodes[a].readyCycle <= cycle;
      bool br = nodes[b].readyCycle <= cycle;
      if (ar != br) return ar > br;

      int aPair = 0;
      int bPair = 0;
      if (firstIssued >= 0) {
        bool firstIsLoad = mioMayLoad(nodes[firstIssued].inst->opcode());
        if (firstIsLoad && !isMemOp(nodes[a].inst->opcode())) aPair += 4;
        if (firstIsLoad && !isMemOp(nodes[b].inst->opcode())) bPair += 4;
        bool firstIsLong = isLongLatencyOp(nodes[firstIssued].inst->opcode());
        if (firstIsLong && !isMemOp(nodes[a].inst->opcode())) aPair += 2;
        if (firstIsLong && !isMemOp(nodes[b].inst->opcode())) bPair += 2;
        if (firstIsLong && isDelayFillerOp(nodes[a].inst->opcode())) aPair += 3;
        if (firstIsLong && isDelayFillerOp(nodes[b].inst->opcode())) bPair += 3;
        if (issueKind(nodes[a].inst) != issueKind(nodes[firstIssued].inst)) aPair += 2;
        if (issueKind(nodes[b].inst) != issueKind(nodes[firstIssued].inst)) bPair += 2;
      }
      if (aPair != bPair) return aPair > bPair;

      bool aLoad = mioMayLoad(nodes[a].inst->opcode());
      bool bLoad = mioMayLoad(nodes[b].inst->opcode());
      if (aLoad != bLoad) return aLoad > bLoad;
      bool aLong = isLongLatencyOp(nodes[a].inst->opcode());
      bool bLong = isLongLatencyOp(nodes[b].inst->opcode());
      if (aLong != bLong) return aLong > bLong;
      if (nodes[a].height != nodes[b].height) return nodes[a].height > nodes[b].height;
      if (nodes[a].latency != nodes[b].latency) return nodes[a].latency > nodes[b].latency;
      return nodes[a].idx < nodes[b].idx;
    };

    int issued = 0, memIssued = 0;
    std::vector<char> pickedNow(M, 0);
    int firstIssued = -1;
    while (issued < 2) {
      int bestPos = -1;
      for (size_t k = 0; k < ready.size(); ++k) {
        int v = ready[k];
        if (pickedNow[v]) continue;
        if (nodes[v].readyCycle > cycle) continue;
        bool memOp = isMemOp(nodes[v].inst->opcode());
        if (memOp && memIssued >= 1) continue;
        if (bestPos < 0 || betterReady(v, ready[bestPos], firstIssued)) {
          bestPos = static_cast<int>(k);
        }
      }
      if (bestPos < 0) break;
      int v = ready[bestPos];
      pickedNow[v] = 1;
      scheduled.push_back(v);
      ++issued;
      if (isMemOp(nodes[v].inst->opcode())) ++memIssued;
      if (firstIssued < 0) firstIssued = v;
      for (int s : nodes[v].succs) {
        nodes[s].readyCycle = std::max(nodes[s].readyCycle, cycle + nodes[v].latency);
        if (--nodes[s].unmetDeps == 0 && !enqueued[s]) {
          ready.push_back(s);
          enqueued[s] = 1;
        }
      }
    }
    ready.erase(
        std::remove_if(ready.begin(), ready.end(),
                       [&](int v) { return pickedNow[v] != 0; }),
        ready.end());
    if (issued == 0) {
      // 当前周期没有指令可以发射，推进到最早的 readyCycle。
      int nextC = INT_MAX;
      for (int v : ready) nextC = std::min(nextC, nodes[v].readyCycle);
      cycle = (nextC == INT_MAX) ? cycle + 1 : nextC;
    } else {
      cycle++;
    }
  }

  if (static_cast<int>(scheduled.size()) != M) return; // 没有完成全部指令的调度
  std::vector<char> seen(M, 0);
  for (int idx : scheduled) {
    if (idx < 0 || idx >= M || seen[idx]) return;
    seen[idx] = 1;
  }

  // 按调度顺序重建 bb.insts()，并把原终结指令保留在末尾。
  std::vector<std::unique_ptr<MachineInst>> tmp(insts.size());
  for (size_t i = 0; i < insts.size(); ++i) tmp[i] = std::move(insts[i]);
  insts.clear();
  insts.reserve(tmp.size());
  for (int idx : scheduled) insts.push_back(std::move(tmp[idx]));
  for (int i = termStart; i < N; ++i) insts.push_back(std::move(tmp[i]));
}

bool blockHasFPOps(const MachineBasicBlock& bb) {
  for (const auto& instPtr : bb.insts()) {
    const MachineInst* inst = instPtr.get();
    if (inst && isFPOp(inst))
      return true;
  }
  return false;
}

} // namespace

void runListSchedule(MachineFunction& mf) {
  for (auto& bb : mf.blocks()) scheduleBlock(*bb);
}

void runListSchedule(MachineModule& mm) {
  for (auto& fn : mm.functions()) runListSchedule(*fn);
}

void runListScheduleFPHot(MachineFunction& mf) {
  for (auto& bb : mf.blocks()) {
    if (blockHasFPOps(*bb))
      scheduleBlock(*bb);
  }
}

void runListScheduleFPHot(MachineModule& mm) {
  for (auto& fn : mm.functions()) runListScheduleFPHot(*fn);
}

void runListScheduleNonFP(MachineFunction& mf) {
  for (auto& bb : mf.blocks()) {
    if (!blockHasFPOps(*bb))
      scheduleBlock(*bb);
  }
}

void runListScheduleNonFP(MachineModule& mm) {
  for (auto& fn : mm.functions()) runListScheduleNonFP(*fn);
}

namespace {

int loopBias(MachineBasicBlock* bb,
             const std::unordered_map<MachineBasicBlock*, int>& loopDepth);

MachineBasicBlock* branchTarget(const MachineInst* inst) {
  if (!inst || inst->numOperands() == 0) return nullptr;
  const auto& op = inst->operand(inst->numOperands() - 1);
  return op.isMBB() ? op.mbb : nullptr;
}

int successorHeatScore(MachineBasicBlock* succ,
                       const std::unordered_map<MachineBasicBlock*, int>& loopDepth) {
  if (!succ)
    return INT_MIN;
  int score = loopBias(succ, loopDepth) * 8;
  score += static_cast<int>(succ->preds().size()) * 2;
  score += static_cast<int>(succ->succs().size());
  return score;
}

MachineBasicBlock* preferredSuccessor(
    MachineBasicBlock* bb,
    const std::unordered_map<MachineBasicBlock*, int>& loopDepth) {
  if (!bb || bb->insts().empty()) return nullptr;
  auto& insts = bb->insts();
  if (insts.size() >= 2) {
    MachineInst* prev = insts[insts.size() - 2].get();
    MachineInst* last = insts.back().get();
    if (mioIsBranch(prev->opcode()) && last->opcode() == MIOp::J) {
      MachineBasicBlock* branchSucc = branchTarget(prev);
      MachineBasicBlock* jumpSucc = branchTarget(last);
      if (!branchSucc) return jumpSucc;
      if (!jumpSucc) return branchSucc;
      return successorHeatScore(branchSucc, loopDepth) >=
                     successorHeatScore(jumpSucc, loopDepth)
                 ? branchSucc
                 : jumpSucc;
    }
  }
  MachineInst* last = insts.back().get();
  if (last->opcode() == MIOp::J) return branchTarget(last);
  if (mioIsBranch(last->opcode())) {
    // 条件分支的显式标签不一定是热点路径：降低阶段常把冷循环出口作为跳转
    // 目标，让控制流落入循环体。按热度在两个 CFG 后继中选择，使布局保留
    // 热点直落路径，并在后续移除多余跳转。
    MachineBasicBlock* best = branchTarget(last);
    int bestScore = successorHeatScore(best, loopDepth);
    for (MachineBasicBlock* succ : bb->succs()) {
      int score = successorHeatScore(succ, loopDepth);
      if (score > bestScore) {
        best = succ;
        bestScore = score;
      }
    }
    return best;
  }
  for (auto* succ : bb->succs()) {
    if (succ) return succ;
  }
  return nullptr;
}

int loopBias(MachineBasicBlock* bb,
             const std::unordered_map<MachineBasicBlock*, int>& loopDepth) {
  if (!bb)
    return 0;
  int bias = 0;
  auto it = loopDepth.find(bb);
  if (it != loopDepth.end())
    bias += it->second * 4;
  for (auto* succ : bb->succs()) {
    if (!succ)
      continue;
    auto succIt = loopDepth.find(succ);
    int succDepth = succIt == loopDepth.end() ? 0 : succIt->second;
    if (succDepth >= (it == loopDepth.end() ? 0 : it->second))
      bias += 1;
  }
  for (auto* pred : bb->preds()) {
    if (pred) {
      bias += 1;
    }
  }
  return bias;
}

MachineBasicBlock* chooseNextLayoutBlock(
    const std::vector<std::unique_ptr<MachineBasicBlock>>& blocks,
    const std::vector<bool>& placed,
    MachineBasicBlock* current,
    const std::unordered_map<MachineBasicBlock*, int>& loopDepth) {
  auto isAvailable = [&](MachineBasicBlock* bb) {
    return bb && bb->id() >= 0 &&
           bb->id() < static_cast<int>(placed.size()) &&
           !placed[bb->id()];
  };

  if (current) {
    if (MachineBasicBlock* pref = preferredSuccessor(current, loopDepth);
        isAvailable(pref)) {
      return pref;
    }
    for (auto* succ : current->succs()) {
      if (isAvailable(succ)) return succ;
    }
  }

  int bestIdx = -1;
  int bestScore = INT_MIN;
  MachineBasicBlock* pref =
      current ? preferredSuccessor(current, loopDepth) : nullptr;
  for (size_t i = 0; i < blocks.size(); ++i) {
    if (placed[i]) continue;
    MachineBasicBlock* candidate = blocks[i].get();
    int score = loopBias(candidate, loopDepth) * 2;
    if (current) {
      if (candidate == pref) score += 16;
      for (auto* succ : current->succs()) {
        if (succ == candidate) {
          score += 8;
          break;
        }
      }
    }
    for (auto* pred : blocks[i]->preds()) {
      if (pred && pred->id() >= 0 && pred->id() < static_cast<int>(placed.size()) &&
          placed[pred->id()]) {
        score += 4 + loopBias(pred, loopDepth);
      }
    }
    if (score > bestScore) {
      bestScore = score;
      bestIdx = static_cast<int>(i);
    }
  }
  return bestIdx >= 0 ? blocks[bestIdx].get() : nullptr;
}

} // namespace

void runBlockLayout(MachineFunction& mf) {
  auto& blocks = mf.blocks();
  if (blocks.size() <= 1) return;
  auto loopDepth = computeLoopDepth(mf);

  std::vector<bool> placed(blocks.size(), false);
  std::vector<int> order;
  order.reserve(blocks.size());
  order.push_back(0);
  placed[0] = true;
  MachineBasicBlock* current = blocks[0].get();

  while (order.size() < blocks.size()) {
    MachineBasicBlock* next =
        chooseNextLayoutBlock(blocks, placed, current, loopDepth);
    if (!next) break;
    placed[next->id()] = true;
    order.push_back(next->id());
    current = next;
  }
  for (size_t i = 0; i < blocks.size(); ++i) {
    if (!placed[i]) order.push_back(static_cast<int>(i));
  }

  std::vector<std::unique_ptr<MachineBasicBlock>> oldBlocks;
  oldBlocks.swap(blocks);
  blocks.reserve(oldBlocks.size());
  for (int idx : order) {
    blocks.push_back(std::move(oldBlocks[idx]));
  }
  for (size_t i = 0; i < blocks.size(); ++i) {
    blocks[i]->setId(static_cast<int>(i));
  }

  // 重排会改变物理直落路径。先修复按旧布局降低的条件分支，
  // 保持 CFG 语义，再让窥孔阶段删除多余跳转。
  for (size_t i = 0; i < blocks.size(); ++i) {
    MachineBasicBlock* bb = blocks[i].get();
    MachineBasicBlock* next = (i + 1 < blocks.size()) ? blocks[i + 1].get() : nullptr;
    auto& insts = bb->insts();
    if (insts.empty())
      continue;
    MachineInst* br = insts.back().get();
    if (!mioIsBranch(br->opcode()) || br->numOperands() == 0)
      continue;
    MOperand& targetOp = br->operand(br->numOperands() - 1);
    if (!targetOp.isMBB())
      continue;

    MachineBasicBlock* branchTarget = targetOp.mbb;
    MachineBasicBlock* fallthroughTarget = nullptr;
    for (auto* succ : bb->succs()) {
      if (succ && succ != branchTarget) {
        fallthroughTarget = succ;
        break;
      }
    }
    if (!fallthroughTarget)
      continue;
    if (next == fallthroughTarget)
      continue;

    if (next == branchTarget) {
      MIOp inv = invertBranchOpcode(br->opcode());
      if (inv != MIOp::COUNT) {
        br->setOpcode(inv);
        targetOp.mbb = fallthroughTarget;
        continue;
      }
    }

    auto j = std::make_unique<MachineInst>(MIOp::J);
    j->addOperand(MOperand::BB(fallthroughTarget));
    bb->append(std::move(j));
  }
}

void runBlockLayout(MachineModule& mm) {
  for (auto& fn : mm.functions()) runBlockLayout(*fn);
}

} // namespace mir
