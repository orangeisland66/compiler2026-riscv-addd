#include "research_switch.h"
// 文件职责：实现 MIR 线性扫描寄存器分配和 spill 处理。本文件负责实现具体逻辑。
#include "linear_scan.h"

#include "mir_liveness.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <limits>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mir {

namespace {

// 可分配寄存器池，数组前面的寄存器优先使用。短区间优先调用者保存寄存器，
// 不足时回退到被调用者保存寄存器。
constexpr int kCallerGPR[] = {
  gpr::t0, gpr::t1, gpr::t2, gpr::t3,
  gpr::a2, gpr::a3, gpr::a4, gpr::a5, gpr::a6, gpr::a7,
  gpr::t4,
};
constexpr int kExtraCallerGPR[] = {gpr::a0, gpr::a1};
constexpr int kCalleeGPR[] = {
  gpr::s1, gpr::s2, gpr::s3, gpr::s4, gpr::s5, gpr::s6,
  gpr::s7, gpr::s8, gpr::s9, gpr::s10, gpr::s11,
};
constexpr int kCallerFPR[] = {
  fpr::ft0, fpr::ft1, fpr::ft2, fpr::ft3, fpr::ft4,
  fpr::ft5, fpr::ft6, fpr::ft7, fpr::ft8,
  fpr::fa2, fpr::fa3, fpr::fa4, fpr::fa5, fpr::fa6, fpr::fa7,
};
constexpr int kExtraCallerFPR[] = {fpr::fa0, fpr::fa1};
constexpr int kCalleeFPR[] = {
  fpr::fs0, fpr::fs1, fpr::fs2, fpr::fs3, fpr::fs4, fpr::fs5,
  fpr::fs6, fpr::fs7, fpr::fs8, fpr::fs9, fpr::fs10, fpr::fs11,
};

// 回存和重载使用的临时寄存器不能与分配池冲突。整数指令最多有两个寄存器
// 源操作数，单个目标可以在读取源操作数后复用一个源临时寄存器。只保留
// t5/t6 作为专用临时寄存器，可以让高寄存器压力路径使用 t4。
constexpr int kGPRScratch[2] = {gpr::t5, gpr::t6};
constexpr int kFPRScratch[3] = {fpr::ft10, fpr::ft11, fpr::ft9};

bool isAllocatableGPR(int r) {
  for (int p : kCallerGPR) if (p == r) return true;
  for (int p : kExtraCallerGPR) if (p == r) return true;
  for (int p : kCalleeGPR) if (p == r) return true;
  return false;
}
bool isAllocatableFPR(int r) {
  for (int p : kCallerFPR) if (p == r) return true;
  for (int p : kExtraCallerFPR) if (p == r) return true;
  for (int p : kCalleeFPR) if (p == r) return true;
  return false;
}

bool enableExtraArgRegFallback() {
  return sycEnvironmentValue("SYC_SKIP_MIR_EXTRA_ARG_REG_FALLBACK") == nullptr;
}

// ============================================================
// 分配器状态。
// ============================================================
struct AllocResult {
  // 每个虚拟寄存器的最终分配结果。physReg 和 spillSlot 中只有一个有效：
  // physReg >= 0 表示分配到寄存器，spillSlot >= 0 表示回存到栈槽。
  std::unordered_map<int, int> physReg;     // 虚拟寄存器 -> 物理寄存器
  std::unordered_map<int, int> spillSlot;   // 虚拟寄存器 -> 栈帧索引
  std::unordered_set<int> rematNoSlot;      // 可以按需重新物化的虚拟寄存器
};

struct RematRecipe {
  enum class Kind {
    LI,
    LA,
    FRAME_ADDR,
    ADDI,
  };

  Kind kind = Kind::LI;
  RegClass cls = RegClass::GPR;
  int64_t imm = 0;
  std::string sym;
  int frameIdx = -1;
  int baseReg = -1; // ADDI 使用的虚拟或物理寄存器
};

using CopyHintMap = std::unordered_map<int, std::unordered_map<int, int>>;

std::unordered_set<int> collectPhiRelatedVRegs(MachineFunction& mf) {
  std::unordered_set<int> out;
  for (auto& bb : mf.blocks()) {
    for (auto& mi : bb->insts()) {
      if (mi->opcode() != MIOp::PHI && mi->opcode() != MIOp::PHI_COPY)
        continue;
      for (const auto& opnd : mi->operands()) {
        if (opnd.isReg() && isVirtualReg(opnd.reg))
          out.insert(opnd.reg);
      }
    }
  }
  return out;
}

std::unordered_map<int, RematRecipe> collectRematRecipes(MachineFunction& mf) {
  std::unordered_map<int, RematRecipe> recipes;
  for (auto& bb : mf.blocks()) {
    for (auto& miPtr : bb->insts()) {
      MachineInst* mi = miPtr.get();
      if (!mi || mi->numOperands() == 0 || !mi->operand(0).isReg() ||
          !mi->operand(0).isDef) {
        continue;
      }
      int vreg = mi->operand(0).reg;
      if (!isVirtualReg(vreg) || !mf.vregInfo(vreg).rematerializable)
        continue;

      RematRecipe recipe;
      recipe.cls = mi->operand(0).regClass;
      if (mi->opcode() == MIOp::LI && mi->numOperands() == 2 &&
          mi->operand(1).isImm()) {
        recipe.kind = RematRecipe::Kind::LI;
        recipe.imm = mi->operand(1).imm;
      } else if (mi->opcode() == MIOp::LA && mi->numOperands() == 2 &&
                 mi->operand(1).isSym()) {
        recipe.kind = RematRecipe::Kind::LA;
        recipe.sym = mi->operand(1).sym;
      } else if (mi->opcode() == MIOp::ADDI && mi->numOperands() == 3 &&
                 mi->operand(1).isFrame() && mi->operand(2).isImm() &&
                 mi->operand(2).imm == 0) {
        recipe.kind = RematRecipe::Kind::FRAME_ADDR;
        recipe.frameIdx = mi->operand(1).frameIdx;
      } else if (mi->opcode() == MIOp::ADDI && mi->numOperands() == 3 &&
                 mi->operand(1).isReg() && mi->operand(2).isImm()) {
        recipe.kind = RematRecipe::Kind::ADDI;
        recipe.baseReg = mi->operand(1).reg;
        recipe.imm = mi->operand(2).imm;
      } else {
        continue;
      }
      recipes[vreg] = std::move(recipe);
    }
  }
  return recipes;
}

bool canRematWithoutSlot(int vreg,
                         const std::unordered_map<int, RematRecipe>& recipes,
                         const std::unordered_set<int>& phiRelated,
                         const std::unordered_set<int>& blockedVRegs,
                         std::unordered_map<int, bool>& memo,
                         std::unordered_set<int>& active) {
  auto memoIt = memo.find(vreg);
  if (memoIt != memo.end())
    return memoIt->second;
  if (phiRelated.count(vreg) || blockedVRegs.count(vreg))
    return memo[vreg] = false;
  auto it = recipes.find(vreg);
  if (it == recipes.end())
    return memo[vreg] = false;
  if (!active.insert(vreg).second)
    return memo[vreg] = false;

  bool ok = true;
  if (it->second.kind == RematRecipe::Kind::ADDI) {
    if (isPhysReg(it->second.baseReg)) {
      ok = true;
    } else if (isVirtualReg(it->second.baseReg)) {
      ok = canRematWithoutSlot(it->second.baseReg, recipes, phiRelated,
                               blockedVRegs,
                               memo, active);
    } else {
      ok = false;
    }
  }

  active.erase(vreg);
  memo[vreg] = ok;
  return ok;
}

bool sameRegClass(const MOperand& a, const MOperand& b) {
  return a.isReg() && b.isReg() && a.regClass == b.regClass;
}

CopyHintMap collectCopyHints(MachineFunction& mf) {
  CopyHintMap hints;
  auto addHint = [&](int dst, int src, int weight) {
    if (!isVirtualReg(dst) || !isVirtualReg(src) || dst == src)
      return;
    hints[dst][src] += weight;
  };

  for (auto& bb : mf.blocks()) {
    for (auto& mi : bb->insts()) {
      if (mi->opcode() == MIOp::PHI && mi->numOperands() >= 3 &&
          mi->operand(0).isReg() && mi->operand(0).isDef) {
        int dst = mi->operand(0).reg;
        for (size_t i = 1; i + 1 < mi->numOperands(); i += 2) {
          const MOperand& src = mi->operand(i);
          if (!sameRegClass(mi->operand(0), src))
            continue;
          addHint(dst, src.reg, 2);
        }
        continue;
      }

      if ((mi->opcode() == MIOp::MV || mi->opcode() == MIOp::FMV_S) &&
          mi->numOperands() >= 2) {
        const MOperand& dst = mi->operand(0);
        const MOperand& src = mi->operand(1);
        if (dst.isReg() && dst.isDef && sameRegClass(dst, src)) {
          addHint(dst.reg, src.reg, 4);
          addHint(src.reg, dst.reg, 1);
        }
      }
    }
  }
  return hints;
}

class LinearScan {
public:
  LinearScan(MachineFunction& mf,
             const std::unordered_set<int>& rematNoSlotCandidates,
             std::vector<LiveInterval> intervals,
             CopyHintMap copyHints)
      : mf_(mf),
        rematNoSlotCandidates_(rematNoSlotCandidates),
        intervals_(std::move(intervals)),
        copyHints_(std::move(copyHints)) {}

  AllocResult allocate() {
    for (size_t i = 0; i < mf_.numVRegs(); ++i) {
      auto& info = mf_.vregInfo(FIRST_VREG + static_cast<int>(i));
      info.physReg = -1;
      info.spillSlot = -1;
      info.crossesCall = false;
      info.loopDepth = 0;
      info.hotLoopWeight = 0;
      info.preferCallerSaved = false;
    }
    for (const auto& iv : intervals_) {
      auto& info = mf_.vregInfo(iv.vreg);
      info.crossesCall = iv.crossesCall;
      info.loopDepth = iv.loopDepth;
      info.hotLoopWeight = iv.hotLoopWeight;
      info.preferCallerSaved = iv.preferCallerSaved;
    }

      // 按区间起始位置排序。
    std::sort(intervals_.begin(), intervals_.end(),
              [](const LiveInterval& a, const LiveInterval& b) {
                if (a.start != b.start) return a.start < b.start;
                return a.end < b.end;
              });

    AllocResult result;
    assignedGPR_.clear();
    assignedFPR_.clear();
    lastAssignedPhysByVReg_.clear();

    for (auto& iv : intervals_) {
      tryAllocateOrSpill(iv, result);
    }
    return result;
  }

private:
  MachineFunction& mf_;
  const std::unordered_set<int>& rematNoSlotCandidates_;
  std::vector<LiveInterval> intervals_;
  CopyHintMap copyHints_;

  // 记录每个物理寄存器上的区间。分段区间在各段不重叠时可以共享寄存器，
  // 即使它们的线性包络发生重叠。
  using AssignedMap =
      std::unordered_map<int, std::vector<LiveInterval>>;
  AssignedMap assignedGPR_;
  AssignedMap assignedFPR_;
  std::unordered_map<int, int> lastAssignedPhysByVReg_;

  AssignedMap& assignedFor(RegClass cls) {
    return cls == RegClass::FPR ? assignedFPR_ : assignedGPR_;
  }

  static bool intervalsOverlap(const LiveInterval& lhs,
                               const LiveInterval& rhs) {
    size_t lhsIndex = 0;
    size_t rhsIndex = 0;
    while (lhsIndex < lhs.segments.size() &&
           rhsIndex < rhs.segments.size()) {
      const LiveSegment& lhsSegment = lhs.segments[lhsIndex];
      const LiveSegment& rhsSegment = rhs.segments[rhsIndex];
      if (lhsSegment.end < rhsSegment.start) {
        ++lhsIndex;
      } else if (rhsSegment.end < lhsSegment.start) {
        ++rhsIndex;
      } else {
        return true;
      }
    }
    return false;
  }

  bool regConflicts(int physReg, const LiveInterval& interval) {
    auto& assigned = assignedFor(interval.cls);
    auto it = assigned.find(physReg);
    if (it == assigned.end())
      return false;
    for (const LiveInterval& other : it->second) {
      if (intervalsOverlap(other, interval))
        return true;
    }
    return false;
  }

  std::vector<const LiveInterval*> conflictingIntervals(
      int physReg, const LiveInterval& interval) {
    std::vector<const LiveInterval*> conflicts;
    auto& assigned = assignedFor(interval.cls);
    auto it = assigned.find(physReg);
    if (it == assigned.end())
      return conflicts;
    for (const LiveInterval& other : it->second) {
      if (intervalsOverlap(other, interval))
        conflicts.push_back(&other);
    }
    return conflicts;
  }

  void removeAssignedInterval(RegClass cls, int physReg, int vreg) {
    auto& assigned = assignedFor(cls);
    auto it = assigned.find(physReg);
    if (it == assigned.end())
      return;
    auto& intervals = it->second;
    intervals.erase(
        std::remove_if(intervals.begin(), intervals.end(),
                       [&](const LiveInterval& interval) {
                         return interval.vreg == vreg;
                       }),
        intervals.end());
    if (intervals.empty())
      assigned.erase(it);
  }

  bool isCalleeReg(int physReg, RegClass cls) const {
    if (cls == RegClass::FPR) {
      for (int p : kCalleeFPR) if (p == physReg) return true;
    } else {
      for (int p : kCalleeGPR) if (p == physReg) return true;
    }
    return false;
  }

  bool isCallerReg(int physReg, RegClass cls) const {
    if (cls == RegClass::FPR) {
      for (int p : kExtraCallerFPR) if (p == physReg) return true;
      for (int p : kCallerFPR) if (p == physReg) return true;
    } else {
      for (int p : kExtraCallerGPR) if (p == physReg) return true;
      for (int p : kCallerGPR) if (p == physReg) return true;
    }
    return false;
  }

  bool shouldUseCalleeFallback(const LiveInterval& iv) const {
    if (iv.crossesCall)
      return true;
    int span = std::max(1, iv.end - iv.start + 1);
    if (iv.preferCallerSaved) {
      // 热点叶循环临时值仍优先使用调用者保存寄存器，
      // 但循环中的长密集区间如果频繁回存，代价可能高于一次函数级别的
      // 被调用者保存寄存器保存与恢复。
      if (iv.loopDepth > 0 && iv.useCount >= 4 && span >= 18)
        return true;
      if (iv.hotLoopWeight >= 64 && span >= 12)
        return true;
      return false;
    }
    if (iv.useCount >= 4 && span >= 12)
      return true;
    if (span >= 48)
      return true;
    return false;
  }

  bool shouldPreferCalleeFirst(const LiveInterval& iv) const {
    if (iv.crossesCall)
      return true;
    int span = std::max(1, iv.end - iv.start + 1);
    return iv.loopDepth > 0 && iv.useCount >= 2 && span >= 6;
  }

  bool regAllowedForInterval(int physReg, const LiveInterval& iv) {
    if (iv.cls == RegClass::FPR) {
      if (!isAllocatableFPR(physReg))
        return false;
    } else {
      if (!isAllocatableGPR(physReg))
        return false;
    }
    if (iv.crossesCall)
      return isCalleeReg(physReg, iv.cls);
    return isCallerReg(physReg, iv.cls) || isCalleeReg(physReg, iv.cls);
  }

  int nextUseAfter(const LiveInterval& iv, int pos) const {
    auto it = std::lower_bound(iv.usePositions.begin(), iv.usePositions.end(), pos);
    if (it == iv.usePositions.end())
      return std::numeric_limits<int>::max();
    return *it;
  }

  // 尝试分配对应类别的物理寄存器；没有可用寄存器时返回 -1。
  //
  // 跨调用区间放入被调用者保存寄存器（s*/fs*），避免被调用指令覆盖。
  // 不跨调用的区间优先使用调用者保存寄存器，不足时使用被调用者保存寄存器，
  // 由序言和尾声负责保存与恢复。
  int tryFindFreeReg(const LiveInterval& iv) {
    auto hintIt = copyHints_.find(iv.vreg);
    if (hintIt != copyHints_.end()) {
      int bestHintReg = -1;
      int bestWeight = -1;
      for (const auto& peer : hintIt->second) {
        auto regIt = lastAssignedPhysByVReg_.find(peer.first);
        if (regIt == lastAssignedPhysByVReg_.end())
          continue;
        int physReg = regIt->second;
        if (regConflicts(physReg, iv) ||
            !regAllowedForInterval(physReg, iv))
          continue;
        if (peer.second > bestWeight) {
          bestWeight = peer.second;
          bestHintReg = physReg;
        }
      }
      if (bestHintReg >= 0)
        return bestHintReg;
    }

    auto tryPool = [&](const int* pool, int n) {
      for (int i = 0; i < n; ++i)
        if (!regConflicts(pool[i], iv)) return pool[i];
      return -1;
    };
    if (iv.cls == RegClass::FPR) {
      if (iv.crossesCall) {
        // 只使用被调用者保存寄存器。
        return tryPool(kCalleeFPR, sizeof(kCalleeFPR)/sizeof(int));
      }
      if (shouldPreferCalleeFirst(iv)) {
        int r = tryPool(kCalleeFPR, sizeof(kCalleeFPR)/sizeof(int));
        if (r >= 0) return r;
      }
      int r = tryPool(kCallerFPR, sizeof(kCallerFPR)/sizeof(int));
      if (r >= 0) return r;
      if (shouldUseCalleeFallback(iv)) {
        r = tryPool(kCalleeFPR, sizeof(kCalleeFPR)/sizeof(int));
        if (r >= 0) return r;
      }
      if (enableExtraArgRegFallback())
        return tryPool(kExtraCallerFPR, sizeof(kExtraCallerFPR)/sizeof(int));
      return -1;
    } else {
      if (iv.crossesCall) {
        return tryPool(kCalleeGPR, sizeof(kCalleeGPR)/sizeof(int));
      }
      if (shouldPreferCalleeFirst(iv)) {
        int r = tryPool(kCalleeGPR, sizeof(kCalleeGPR)/sizeof(int));
        if (r >= 0) return r;
      }
      int r = tryPool(kCallerGPR, sizeof(kCallerGPR)/sizeof(int));
      if (r >= 0) return r;
      if (shouldUseCalleeFallback(iv)) {
        r = tryPool(kCalleeGPR, sizeof(kCalleeGPR)/sizeof(int));
        if (r >= 0) return r;
      }
      if (enableExtraArgRegFallback())
        return tryPool(kExtraCallerGPR, sizeof(kExtraCallerGPR)/sizeof(int));
      return -1;
    }
  }

  // 区间回存代价：数值越小越适合回存。热区间、密集使用区间和深层循环区间
  // 提高代价；可以重新物化的值适当降低代价。
  double spillCost(const LiveInterval& iv, int pos) const {
    int liveSpan = 0;
    for (const LiveSegment& segment : iv.segments)
      liveSpan += std::max(1, segment.end - segment.start + 1);
    liveSpan = std::max(1, liveSpan);
    double useDensity = static_cast<double>(iv.useCount) / static_cast<double>(liveSpan);
    double hotDensity = static_cast<double>(iv.hotUseCount) / static_cast<double>(liveSpan);
    int nextUse = nextUseAfter(iv, pos);
    double nextUseFactor = 0.85;
    if (nextUse != std::numeric_limits<int>::max()) {
      int distance = std::max(1, nextUse - pos);
      nextUseFactor = 1.0 + 32.0 / static_cast<double>(distance);
    }
    double densityFactor = 1.0 + 10.0 * useDensity;
    double hotFactor = 1.0 + 1.5 * hotDensity;
    double loopFactor = 1.0 + 6.0 * iv.loopDepth;
    double callerPrefFactor = iv.preferCallerSaved ? 1.15 : 1.0;
    double crossCallFactor = iv.crossesCall ? 1.15 : 1.0;
    double rematFactor = iv.rematerializable ? 0.60 : 1.0;
    return densityFactor * hotFactor * loopFactor * callerPrefFactor *
           crossCallFactor * nextUseFactor * rematFactor;
  }

  void assignPhysReg(const LiveInterval& iv, int reg, AllocResult& result) {
    result.physReg[iv.vreg] = reg;
    auto& info = mf_.vregInfo(iv.vreg);
    info.physReg = reg;
    info.spillSlot = -1;
    assignedFor(iv.cls)[reg].push_back(iv);
    lastAssignedPhysByVReg_[iv.vreg] = reg;
  }

  void tryAllocateOrSpill(const LiveInterval& iv, AllocResult& result) {
    int reg = tryFindFreeReg(iv);
    if (reg >= 0) {
      assignPhysReg(iv, reg, result);
      return;
    }
    // 回存时选择代价最低的活动区间，使热内层循环中的值尽量留在寄存器中，
    // 同时结合使用密度而不是只看区间结束位置。跨调用区间只从被调用者保存
    // 寄存器中选择替换对象。
    int worstReg = -1;
    int worstVReg = -1;
    double worstCost = spillCost(iv, iv.start);
    int worstNextUse = nextUseAfter(iv, iv.start);
    for (const auto& entry : assignedFor(iv.cls)) {
      int candidateReg = entry.first;
      if (!regAllowedForInterval(candidateReg, iv))
        continue;
      auto conflicts = conflictingIntervals(candidateReg, iv);
      // 只有当前寄存器没有被其他已分配区间重叠占用时，替换一个旧区间
      // 才能真正释放该寄存器。
      if (conflicts.size() != 1)
        continue;
      const LiveInterval& candidate = *conflicts.front();
      double c = spillCost(candidate, iv.start);
      int candidateNextUse = nextUseAfter(candidate, iv.start);
      if (c < worstCost ||
          (c == worstCost && candidateNextUse > worstNextUse)) {
        worstCost = c;
        worstNextUse = candidateNextUse;
        worstReg = candidateReg;
        worstVReg = candidate.vreg;
      }
    }
    // 找到可替换的牺牲区间：将其逐出寄存器并分配 spill 槽
    // （可重物化者免槽），把寄存器让给当前区间。
    if (worstReg >= 0) {
      int victimVReg = worstVReg;
      removeAssignedInterval(iv.cls, worstReg, victimVReg);
      result.physReg.erase(victimVReg);
      lastAssignedPhysByVReg_.erase(victimVReg);
      auto& info = mf_.vregInfo(victimVReg);
      info.physReg = -1;
      if (rematNoSlotCandidates_.count(victimVReg)) {
        result.rematNoSlot.insert(victimVReg);
        info.spillSlot = -1;
      } else {
        int size = info.cls == RegClass::FPR ? 4 : 8;
        int align = info.cls == RegClass::FPR ? 4 : 8;
        int fi = mf_.newFrameObject(size, align, /*isSpill=*/true,
                                    "spill.v" + std::to_string(victimVReg - FIRST_VREG));
        result.spillSlot[victimVReg] = fi;
        info.spillSlot = fi;
      }
      assignPhysReg(iv, worstReg, result);
    } else {
      // 无可替换牺牲者：当前区间自身回存到栈上 spill 槽。
      auto& info = mf_.vregInfo(iv.vreg);
      info.physReg = -1;
      if (rematNoSlotCandidates_.count(iv.vreg)) {
        result.rematNoSlot.insert(iv.vreg);
        info.spillSlot = -1;
      } else {
        int size = info.cls == RegClass::FPR ? 4 : 8;
        int align = info.cls == RegClass::FPR ? 4 : 8;
        int fi = mf_.newFrameObject(size, align, /*isSpill=*/true,
                                    "spill.v" + std::to_string(iv.vreg - FIRST_VREG));
        result.spillSlot[iv.vreg] = fi;
        info.spillSlot = fi;
      }
    }
  }
};

// ============================================================
// 重写操作数，并插入回存与重载指令。
// ============================================================
MIOp loadOpFor(RegClass cls) { return cls == RegClass::FPR ? MIOp::FLW : MIOp::LD; }
MIOp storeOpFor(RegClass cls) { return cls == RegClass::FPR ? MIOp::FSW : MIOp::SD; }

std::unique_ptr<MachineInst> makeReload(int physReg, RegClass cls, int frameIdx) {
  auto mi = std::make_unique<MachineInst>(loadOpFor(cls));
  mi->addOperand(MOperand::Def(physReg, cls));
  mi->addOperand(MOperand::Frame(frameIdx));
  mi->addOperand(MOperand::Use(gpr::sp, RegClass::GPR));
  mi->comment = "reload";
  return mi;
}
std::unique_ptr<MachineInst> makeSpill(int physReg, RegClass cls, int frameIdx) {
  auto mi = std::make_unique<MachineInst>(storeOpFor(cls));
  mi->addOperand(MOperand::Use(physReg, cls));
  mi->addOperand(MOperand::Frame(frameIdx));
  mi->addOperand(MOperand::Use(gpr::sp, RegClass::GPR));
  mi->comment = "spill";
  return mi;
}

void appendRematInst(std::vector<std::unique_ptr<MachineInst>>& outBuf,
                     MachineBasicBlock& bb,
                     std::unique_ptr<MachineInst> inst) {
  inst->setParent(&bb);
  outBuf.push_back(std::move(inst));
}

// 按 RematRecipe 把已回存 vreg 的值重物化到 dstReg：LI/LA/帧地址直接生成，
// ADDI 的基址若为虚拟寄存器则先递归重物化；active 集防止依赖成环死递归。
bool emitRematForVReg(int vreg,
                      int dstReg,
                      RegClass dstCls,
                      MachineBasicBlock& bb,
                      std::vector<std::unique_ptr<MachineInst>>& outBuf,
                      const std::unordered_map<int, RematRecipe>& recipes,
                      std::unordered_set<int>& active) {
  auto it = recipes.find(vreg);
  if (it == recipes.end())
    return false;
  if (!active.insert(vreg).second)
    return false;

  const RematRecipe& recipe = it->second;
  bool ok = true;
  switch (recipe.kind) {
    case RematRecipe::Kind::LI: {
      auto mi = std::make_unique<MachineInst>(MIOp::LI);
      mi->addOperand(MOperand::Def(dstReg, dstCls));
      mi->addOperand(MOperand::Imm(recipe.imm));
      mi->comment = "remat";
      appendRematInst(outBuf, bb, std::move(mi));
      break;
    }
    case RematRecipe::Kind::LA: {
      auto mi = std::make_unique<MachineInst>(MIOp::LA);
      mi->addOperand(MOperand::Def(dstReg, dstCls));
      mi->addOperand(MOperand::Sym(recipe.sym));
      mi->comment = "remat";
      appendRematInst(outBuf, bb, std::move(mi));
      break;
    }
    case RematRecipe::Kind::FRAME_ADDR: {
      auto mi = std::make_unique<MachineInst>(MIOp::ADDI);
      mi->addOperand(MOperand::Def(dstReg, dstCls));
      mi->addOperand(MOperand::Frame(recipe.frameIdx));
      mi->addOperand(MOperand::Imm(0));
      mi->comment = "remat";
      appendRematInst(outBuf, bb, std::move(mi));
      break;
    }
    case RematRecipe::Kind::ADDI: {
      if (isPhysReg(recipe.baseReg)) {
        auto mi = std::make_unique<MachineInst>(MIOp::ADDI);
        mi->addOperand(MOperand::Def(dstReg, dstCls));
        mi->addOperand(MOperand::Use(recipe.baseReg, RegClass::GPR));
        mi->addOperand(MOperand::Imm(recipe.imm));
        mi->comment = "remat";
        appendRematInst(outBuf, bb, std::move(mi));
      } else if (isVirtualReg(recipe.baseReg) &&
                 emitRematForVReg(recipe.baseReg, dstReg, RegClass::GPR,
                                  bb, outBuf, recipes, active)) {
        // 基址已重物化到 dstReg，再追加 ADDI 叠加立即数偏移
        auto mi = std::make_unique<MachineInst>(MIOp::ADDI);
        mi->addOperand(MOperand::Def(dstReg, dstCls));
        mi->addOperand(MOperand::Use(dstReg, RegClass::GPR));
        mi->addOperand(MOperand::Imm(recipe.imm));
        mi->comment = "remat";
        appendRematInst(outBuf, bb, std::move(mi));
      } else {
        ok = false;
      }
      break;
    }
  }

  active.erase(vreg);
  return ok;
}

// 使用分配结果重写基本块指令。已分配的虚拟寄存器直接替换为物理寄存器；
// 已回存的虚拟寄存器在指令前后通过临时寄存器插入重载和回存。
void rewriteBlock(MachineFunction& mf, MachineBasicBlock& bb,
                  const AllocResult& alloc,
                  const std::unordered_map<int, RematRecipe>& rematRecipes) {
  auto isPureRematOpcode = [](MIOp op) {
    return op == MIOp::LI || op == MIOp::LA || op == MIOp::ADDI;
  };
  std::vector<std::unique_ptr<MachineInst>> outBuf;
  outBuf.reserve(bb.insts().size() * 2);

  for (auto& miPtr : bb.insts()) {
    MachineInst* mi = miPtr.get();
    MIOp op = mi->opcode();

    if (op == MIOp::PHI) {
      // 丢弃，稍后由 PHI 消解阶段处理。
      continue;
    }

    // 记录当前指令的临时寄存器使用情况，用于容纳无法直接分配的回存虚拟寄存器。
    int gprScratchIdx = 0, fprScratchIdx = 0;
    std::unordered_map<int, int> vregToScratch; // 虚拟寄存器 -> 当前指令使用的临时物理寄存器
    std::vector<int> gprUseScratches;
    std::vector<int> fprUseScratches;
    std::vector<std::pair<int, int>> reloads;   // （临时寄存器，栈帧槽位）
    std::vector<std::pair<int, int>> spills;
    std::vector<std::pair<int, int>> remats;    // （虚拟寄存器，临时寄存器）
    std::vector<RegClass> reloadCls, spillCls;

    // 第一遍：处理已回存虚拟寄存器的使用。
    for (auto& opnd : mi->operands()) {
      if (!opnd.isReg() || !isVirtualReg(opnd.reg)) continue;
      if (opnd.isDef) continue;
      int v = opnd.reg;
      // 已经分配到物理寄存器的虚拟寄存器由后续步骤处理。
      auto pit = alloc.physReg.find(v);
      if (pit != alloc.physReg.end()) continue; // 第二遍处理
      if (vregToScratch.count(v)) continue;
      int scratch;
      if (opnd.regClass == RegClass::FPR) {
        if (fprScratchIdx >= 3) throw std::runtime_error("linear_scan: too many FPR uses to scratch");
        scratch = kFPRScratch[fprScratchIdx++];
        fprUseScratches.push_back(scratch);
      } else {
        if (gprScratchIdx >= 2) throw std::runtime_error("linear_scan: too many GPR uses to scratch");
        scratch = kGPRScratch[gprScratchIdx++];
        gprUseScratches.push_back(scratch);
      }
      vregToScratch[v] = scratch;
      bool noSlotRemat = alloc.rematNoSlot.count(v) != 0;
      if (noSlotRemat) {
        remats.push_back({v, scratch});
      } else {
        auto sit = alloc.spillSlot.find(v);
        if (sit == alloc.spillSlot.end()) continue;
        reloads.push_back({scratch, sit->second});
        reloadCls.push_back(opnd.regClass);
      }
    }

    int virtualDefCount = 0;
    for (const auto& opnd : mi->operands()) {
      if (opnd.isReg() && opnd.isDef && isVirtualReg(opnd.reg))
        ++virtualDefCount;
    }

    // 单个目标可以复用源临时寄存器：RISC-V 会先读取所有源操作数再写入 rd。
    // 多目标伪指令继续使用不同临时寄存器，以保留各结果。
    for (auto& opnd : mi->operands()) {
      if (!opnd.isReg() || !isVirtualReg(opnd.reg)) continue;
      if (!opnd.isDef) continue;
      int v = opnd.reg;
      if (alloc.physReg.count(v)) continue;
      auto sit = alloc.spillSlot.find(v);
      bool noSlotRemat = alloc.rematNoSlot.count(v) != 0;
      if (sit == alloc.spillSlot.end() && !noSlotRemat) continue;
      int scratch = -1;
      auto existing = vregToScratch.find(v);
      if (existing != vregToScratch.end()) {
        scratch = existing->second;
      } else if (virtualDefCount == 1 && opnd.regClass == RegClass::FPR &&
                 !fprUseScratches.empty()) {
        scratch = fprUseScratches.front();
      } else if (virtualDefCount == 1 && opnd.regClass == RegClass::GPR &&
                 !gprUseScratches.empty()) {
        scratch = gprUseScratches.front();
      } else if (opnd.regClass == RegClass::FPR) {
        if (fprScratchIdx >= 3) throw std::runtime_error("linear_scan: too many FPR defs to scratch");
        scratch = kFPRScratch[fprScratchIdx++];
      } else {
        if (gprScratchIdx >= 2) throw std::runtime_error("linear_scan: too many GPR defs to scratch");
        scratch = kGPRScratch[gprScratchIdx++];
      }
      vregToScratch[v] = scratch;
      if (!noSlotRemat && sit != alloc.spillSlot.end()) {
        spills.push_back({scratch, sit->second});
        spillCls.push_back(opnd.regClass);
      }
    }

    // 在原指令前插入重载。
    for (size_t i = 0; i < reloads.size(); ++i) {
      outBuf.push_back(makeReload(reloads[i].first, reloadCls[i], reloads[i].second));
      outBuf.back()->setParent(&bb);
    }
    for (const auto& remat : remats) {
      std::unordered_set<int> active;
      if (!emitRematForVReg(remat.first, remat.second,
                            mf.vregInfo(remat.first).cls,
                            bb, outBuf, rematRecipes, active)) {
        auto sit = alloc.spillSlot.find(remat.first);
        if (sit == alloc.spillSlot.end()) {
          throw std::runtime_error("linear_scan: missing remat fallback");
        }
        outBuf.push_back(makeReload(remat.second,
                                    mf.vregInfo(remat.first).cls,
                                    sit->second));
        outBuf.back()->setParent(&bb);
      }
    }

    // 重写操作数中的寄存器。
    for (auto& opnd : mi->operands()) {
      if (!opnd.isReg() || !isVirtualReg(opnd.reg)) continue;
      auto pit = alloc.physReg.find(opnd.reg);
      if (pit != alloc.physReg.end()) {
        opnd.reg = pit->second;
        continue;
      }
      auto sit = vregToScratch.find(opnd.reg);
      if (sit != vregToScratch.end()) {
        opnd.reg = sit->second;
      }
    }

    bool dropPureRematDef = false;
    if (isPureRematOpcode(mi->opcode())) {
      std::vector<int> defs;
      mi->collectDefs(defs);
      if (!defs.empty()) {
        dropPureRematDef = true;
        for (int def : defs) {
          if (!isVirtualReg(def) || alloc.physReg.count(def) ||
              !alloc.rematNoSlot.count(def)) {
            dropPureRematDef = false;
            break;
          }
        }
      }
    }

    if (!dropPureRematDef) {
      outBuf.push_back(std::move(miPtr));
    }

    // 在原指令后插入回存。
    for (size_t i = 0; i < spills.size(); ++i) {
      outBuf.push_back(makeSpill(spills[i].first, spillCls[i], spills[i].second));
      outBuf.back()->setParent(&bb);
    }
  }
  bb.insts() = std::move(outBuf);
}

struct CopyLoc {
  enum class Kind { Reg, Slot };
  Kind kind = Kind::Reg;
  int value = -1;
};

struct ParallelCopy {
  CopyLoc src;
  CopyLoc dst;
  RegClass cls = RegClass::GPR;
  bool hasOverride = false;
  CopyLoc overrideSrc;
};

bool sameLoc(const CopyLoc& a, const CopyLoc& b) {
  return a.kind == b.kind && a.value == b.value;
}

void appendMove(std::vector<std::unique_ptr<MachineInst>>& seq,
                int dst,
                int src,
                RegClass cls,
                const char* comment) {
  auto mv = std::make_unique<MachineInst>(cls == RegClass::FPR ? MIOp::FMV_S : MIOp::MV);
  mv->addOperand(MOperand::Def(dst, cls));
  mv->addOperand(MOperand::Use(src, cls));
  mv->comment = comment;
  seq.push_back(std::move(mv));
}

void appendCopyInst(std::vector<std::unique_ptr<MachineInst>>& seq,
                    const CopyLoc& src,
                    const CopyLoc& dst,
                    RegClass cls) {
  if (sameLoc(src, dst)) return;
  if (dst.kind == CopyLoc::Kind::Reg && src.kind == CopyLoc::Kind::Reg) {
    appendMove(seq, dst.value, src.value, cls, "phi-copy");
    return;
  }
  if (dst.kind == CopyLoc::Kind::Reg && src.kind == CopyLoc::Kind::Slot) {
    seq.push_back(makeReload(dst.value, cls, src.value));
    seq.back()->comment = "phi-reload";
    return;
  }
  if (dst.kind == CopyLoc::Kind::Slot && src.kind == CopyLoc::Kind::Reg) {
    seq.push_back(makeSpill(src.value, cls, dst.value));
    seq.back()->comment = "phi-store";
    return;
  }

  int scratch = cls == RegClass::FPR ? kFPRScratch[0] : kGPRScratch[0];
  seq.push_back(makeReload(scratch, cls, src.value));
  seq.back()->comment = "phi-reload";
  seq.push_back(makeSpill(scratch, cls, dst.value));
  seq.back()->comment = "phi-store";
}

void resolveParallelCopies(std::vector<ParallelCopy> pending,
                           RegClass cls,
                           std::vector<std::unique_ptr<MachineInst>>& seq) {
  auto currentSrc = [](const ParallelCopy& copy) -> CopyLoc {
    return copy.hasOverride ? copy.overrideSrc : copy.src;
  };

  while (!pending.empty()) {
    bool progressed = false;
    for (size_t i = 0; i < pending.size(); ++i) {
      CopyLoc dst = pending[i].dst;
      bool dstUsedAsSource = false;
      for (size_t j = 0; j < pending.size(); ++j) {
        if (i == j) continue;
        if (sameLoc(dst, currentSrc(pending[j]))) {
          dstUsedAsSource = true;
          break;
        }
      }
      if (!dstUsedAsSource) {
        appendCopyInst(seq, currentSrc(pending[i]), pending[i].dst, cls);
        pending.erase(pending.begin() + static_cast<long>(i));
        progressed = true;
        break;
      }
    }
    if (progressed) continue;

    int scratch = cls == RegClass::FPR ? kFPRScratch[0] : kGPRScratch[0];
    for (auto& copy : pending) {
      CopyLoc src = currentSrc(copy);
      if (sameLoc(src, copy.dst)) continue;
      if (src.kind == CopyLoc::Kind::Reg) {
        appendMove(seq, scratch, src.value, cls, "phi-cycle-break");
      } else {
        seq.push_back(makeReload(scratch, cls, src.value));
        seq.back()->comment = "phi-cycle-break";
      }
      copy.hasOverride = true;
      copy.overrideSrc = {CopyLoc::Kind::Reg, scratch};
      progressed = true;
      break;
    }
    if (!progressed) break;
  }
}

void resolveCallArgCopies(MachineFunction& mf, const AllocResult& alloc) {
  auto locForVReg = [&](int vreg) -> CopyLoc {
    auto pit = alloc.physReg.find(vreg);
    if (pit != alloc.physReg.end())
      return {CopyLoc::Kind::Reg, pit->second};
    auto sit = alloc.spillSlot.find(vreg);
    if (sit != alloc.spillSlot.end())
      return {CopyLoc::Kind::Slot, sit->second};
    throw std::runtime_error("call argument copy: vreg missing allocation");
  };

  for (auto& bbPtr : mf.blocks()) {
    MachineBasicBlock& bb = *bbPtr;
    std::vector<std::unique_ptr<MachineInst>> rewritten;
    rewritten.reserve(bb.insts().size());
    for (auto& miPtr : bb.insts()) {
      MachineInst* mi = miPtr.get();
      if (!mi || mi->opcode() != MIOp::PHI_COPY ||
          mi->comment != "call-args") {
        rewritten.push_back(std::move(miPtr));
        continue;
      }

      std::vector<ParallelCopy> gprCopies;
      std::vector<ParallelCopy> fprCopies;
      for (size_t i = 0; i + 1 < mi->numOperands(); i += 2) {
        const MOperand& dst = mi->operand(i);
        const MOperand& src = mi->operand(i + 1);
        if (!dst.isReg() || !dst.isDef || !isPhysReg(dst.reg) ||
            !src.isReg() || src.isDef || !isVirtualReg(src.reg) ||
            dst.regClass != src.regClass) {
          throw std::runtime_error("malformed call argument copy");
        }
        ParallelCopy copy;
        copy.src = locForVReg(src.reg);
        copy.dst = {CopyLoc::Kind::Reg, dst.reg};
        copy.cls = dst.regClass;
        auto& bundle =
            copy.cls == RegClass::FPR ? fprCopies : gprCopies;
        bundle.push_back(copy);
      }

      std::vector<std::unique_ptr<MachineInst>> seq;
      resolveParallelCopies(std::move(gprCopies), RegClass::GPR, seq);
      resolveParallelCopies(std::move(fprCopies), RegClass::FPR, seq);
      for (auto& copy : seq) {
        copy->comment = "call-arg-copy";
        copy->setParent(&bb);
        rewritten.push_back(std::move(copy));
      }
    }
    bb.insts() = std::move(rewritten);
  }
}

// 为每条前驱边插入一组并行复制来消解 phi 伪指令。
// 按寄存器类别分别处理复制关系，使寄存器与栈槽之间的各种组合都能
// 参与同一套环路拆分逻辑。
void resolveBranchPhis(MachineFunction& mf,
                       const AllocResult& alloc,
                       std::unordered_map<MachineBasicBlock*,
                                          std::vector<std::unique_ptr<MachineInst>>>& savedPhis) {
  for (auto& bbPtr : mf.blocks()) {
    auto it = savedPhis.find(bbPtr.get());
    if (it == savedPhis.end()) continue;
    std::unordered_map<MachineBasicBlock*, std::vector<ParallelCopy>> gprBundles;
    std::unordered_map<MachineBasicBlock*, std::vector<ParallelCopy>> fprBundles;

    auto locForVReg = [&](int vreg) -> CopyLoc {
      auto pit = alloc.physReg.find(vreg);
      if (pit != alloc.physReg.end()) return {CopyLoc::Kind::Reg, pit->second};
      auto sit = alloc.spillSlot.find(vreg);
      if (sit != alloc.spillSlot.end()) return {CopyLoc::Kind::Slot, sit->second};
      throw std::runtime_error("phi resolution: vreg missing allocation");
    };

    for (auto& phi : it->second) {
      if (phi->numOperands() == 0 || !phi->operand(0).isReg()) continue;
      int destV = phi->operand(0).reg;
      RegClass cls = phi->operand(0).regClass;
      CopyLoc dst = locForVReg(destV);

      for (size_t i = 1; i + 1 < phi->numOperands(); i += 2) {
        MOperand& srcOp = phi->operand(i);
        MOperand& bbOp = phi->operand(i + 1);
        if (!srcOp.isReg() || !bbOp.isMBB() || !bbOp.mbb) continue;
        ParallelCopy copy;
        copy.src = locForVReg(srcOp.reg);
        copy.dst = dst;
        copy.cls = cls;
        if (sameLoc(copy.src, copy.dst)) continue;
        auto& bundleMap = cls == RegClass::FPR ? fprBundles : gprBundles;
        bundleMap[bbOp.mbb].push_back(copy);
      }
    }

    auto flushBundles = [&](std::unordered_map<MachineBasicBlock*, std::vector<ParallelCopy>>& bundles,
                            RegClass cls) {
      for (auto& kv : bundles) {
        MachineBasicBlock* pred = kv.first;
        std::vector<std::unique_ptr<MachineInst>> seq;
        resolveParallelCopies(kv.second, cls, seq);
        MachineInst* term = pred->terminator();
        for (auto& inst : seq) {
          if (term) pred->insertBefore(term, std::move(inst));
          else pred->append(std::move(inst));
        }
      }
    };

    flushBundles(gprBundles, RegClass::GPR);
    flushBundles(fprBundles, RegClass::FPR);
  }
}

} // namespace

void runLinearScanRA(MachineFunction& mf) {
  if (mf.blocks().empty()) return;
  std::unordered_set<int> rematNoSlotCandidates;
  std::unordered_map<int, RematRecipe> rematRecipes = collectRematRecipes(mf);
  auto live = analyzeLiveness(mf);
  CopyHintMap copyHints = collectCopyHints(mf);
  std::unordered_set<int> phiRelated = collectPhiRelatedVRegs(mf);
  std::unordered_set<int> blockedRemat;
  for (const auto& iv : live.intervals) {
    if (iv.crossesCall)
      blockedRemat.insert(iv.vreg);
  }
  std::unordered_map<int, bool> memo;
  std::unordered_set<int> activeRemat;
  for (const auto& iv : live.intervals) {
    if (!iv.rematerializable)
      continue;
    if (canRematWithoutSlot(iv.vreg, rematRecipes, phiRelated, blockedRemat,
                            memo, activeRemat)) {
      rematNoSlotCandidates.insert(iv.vreg);
    }
  }

  // 保留 PHI 运行线性扫描：活跃性分析需要它们把 phi 来源虚拟寄存器的
  // 区间延伸到每个前驱块末尾，边复制就会插入在那里。
  AllocResult alloc;
  {
    LinearScan ls(mf, rematNoSlotCandidates, live.intervals, std::move(copyHints));
    alloc = ls.allocate();
  }

  resolveCallArgCopies(mf, alloc);

  // 取出 PHI。rewriteBlock 会丢弃它们，之后在操作数改写完成后
  // 再通过逐边复制完成消解。
  std::unordered_map<MachineBasicBlock*,
                     std::vector<std::unique_ptr<MachineInst>>> savedPhis;
  for (auto& bbPtr : mf.blocks()) {
    std::vector<std::unique_ptr<MachineInst>> phis;
    std::vector<std::unique_ptr<MachineInst>> rest;
    for (auto& mi : bbPtr->insts()) {
      if (mi->opcode() == MIOp::PHI) phis.push_back(std::move(mi));
      else rest.push_back(std::move(mi));
    }
    bbPtr->insts() = std::move(rest);
    if (!phis.empty()) savedPhis[bbPtr.get()] = std::move(phis);
  }

  // 改写操作数，并为回存虚拟寄存器插入 reload/spill 指令。
  for (auto& bb : mf.blocks()) rewriteBlock(mf, *bb, alloc, rematRecipes);

  // 在入边上消解 PHI；此时块内其余指令已经改写完，末尾保留真实终结指令。
  resolveBranchPhis(mf, alloc, savedPhis);

  mf.usedCalleeSavedGPR.clear();
  mf.usedCalleeSavedFPR.clear();
  // prologue.cpp 中的 scanCalleeSavedUsage 会重新统计实际使用的寄存器。
}

void runLinearScanRA(MachineModule& mm) {
  for (auto& fn : mm.functions()) runLinearScanRA(*fn);
}

} // namespace mir
