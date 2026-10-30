#include "research_switch.h"
// 文件职责：执行保守 peephole 优化，删除局部冗余机器指令。本文件负责实现具体逻辑。
#include "peephole.h"

#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mir {

namespace {

// 判断是否为目标是 nextBlock 的 `j MBB`，这种跳转可以删除。
bool isFallthroughJ(const MachineInst& mi, const MachineBasicBlock* nextBlock) {
  if (mi.opcode() != MIOp::J) return false;
  if (mi.numOperands() != 1 || !mi.operand(0).isMBB()) return false;
  return mi.operand(0).mbb == nextBlock;
}

bool isRedundantMove(const MachineInst& mi) {
  MIOp op = mi.opcode();
  if (op == MIOp::MV || op == MIOp::FMV_S) {
    if (mi.numOperands() == 2 &&
        mi.operand(0).isReg() && mi.operand(1).isReg() &&
        mi.operand(0).reg == mi.operand(1).reg) {
      return true;
    }
  }
  if (op == MIOp::ADDI || op == MIOp::ADDIW) {
    // rd == rs 时，addi rd, rs, 0 没有实际效果。
    if (mi.numOperands() == 3 &&
        mi.operand(0).isReg() && mi.operand(1).isReg() &&
        mi.operand(2).isImm() &&
        mi.operand(0).reg == mi.operand(1).reg &&
        mi.operand(2).imm == 0) {
      return true;
    }
  }
  return false;
}

bool matchAdjacentStackReload(const MachineInst& store,
                              const MachineInst& load,
                              MIOp& moveOp,
                              int& destination,
                              int& source,
                              RegClass& regClass) {
  const bool integerPair =
      store.opcode() == MIOp::SD && load.opcode() == MIOp::LD;
  const bool floatPair =
      store.opcode() == MIOp::FSW && load.opcode() == MIOp::FLW;
  if ((!integerPair && !floatPair) || store.numOperands() != 3 ||
      load.numOperands() != 3 || !store.operand(0).isReg() ||
      !load.operand(0).isReg() || !store.operand(2).isReg() ||
      !load.operand(2).isReg() ||
      store.operand(2).reg != gpr::sp || load.operand(2).reg != gpr::sp) {
    return false;
  }
  const bool sameFrame =
      store.operand(1).isFrame() && load.operand(1).isFrame() &&
      store.operand(1).frameIdx == load.operand(1).frameIdx;
  const bool sameOffset =
      store.operand(1).isImm() && load.operand(1).isImm() &&
      store.operand(1).imm == load.operand(1).imm;
  if (!sameFrame && !sameOffset)
    return false;
  regClass = integerPair ? RegClass::GPR : RegClass::FPR;
  if ((integerPair && (!isGPR(store.operand(0).reg) ||
                       !isGPR(load.operand(0).reg))) ||
      (floatPair && (!isFPR(store.operand(0).reg) ||
                     !isFPR(load.operand(0).reg)))) {
    return false;
  }
  moveOp = integerPair ? MIOp::MV : MIOp::FMV_S;
  destination = load.operand(0).reg;
  source = store.operand(0).reg;
  return true;
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

// ============================================================
// 比较与分支合并
//
// 识别以下两条指令形状：
//   slt/sltu  rd, rs1, rs2
//   bnez/beqz rd, target
// 并合并为一条直接分支：
//   blt/bltu/bge/bgeu  rs1, rs2, target
//
// 每次比较可以减少一条指令和一个临时寄存器使用，对比较密集的循环尤其有帮助。
//
// rd 不能与 rs1、rs2 相同，否则 slt 会在分支读取前覆盖源值；rd 在当前块中
// 也不能再被后续指令使用。
// ============================================================
struct CompareBranchFusion {
  MIOp fusedOp = MIOp::COUNT;
  int rs1 = -1;
  int rs2 = -1;
  MachineBasicBlock* target = nullptr;
};

// 尝试把 insts[i]（slt/sltu）与 insts[i+1]（bnez/beqz）合并，成功时填写 out。
bool matchCompareBranch(const MachineInst& cmp, const MachineInst& br,
                        CompareBranchFusion& out) {
  MIOp cmpOp = cmp.opcode();
  MIOp brOp = br.opcode();

  // cmp 必须是带有 rd、rs1、rs2 三个寄存器操作数的 slt 或 sltu。
  if (cmpOp != MIOp::SLT && cmpOp != MIOp::SLTU) return false;
  if (cmp.numOperands() != 3) return false;
  if (!cmp.operand(0).isReg() || !cmp.operand(1).isReg() || !cmp.operand(2).isReg())
    return false;

  int rd  = cmp.operand(0).reg;
  int rs1 = cmp.operand(1).reg;
  int rs2 = cmp.operand(2).reg;

  // rd 不能与任一源寄存器别名，否则 slt 会覆盖源值。
  if (rd == rs1 || rd == rs2) return false;

  // br 必须是带有 reg、MBB 两个操作数的 bnez 或 beqz。
  if (brOp != MIOp::BNEZ && brOp != MIOp::BEQZ) return false;
  if (br.numOperands() != 2) return false;
  if (!br.operand(0).isReg() || !br.operand(1).isMBB()) return false;

  // 分支检查的寄存器必须是 slt 定义的同一个寄存器。
  if (br.operand(0).reg != rd) return false;

  // 确定融合后的分支操作码。
  if (cmpOp == MIOp::SLT) {
    out.fusedOp = (brOp == MIOp::BNEZ) ? MIOp::BLT : MIOp::BGE;
  } else { // SLTU
    out.fusedOp = (brOp == MIOp::BNEZ) ? MIOp::BLTU : MIOp::BGEU;
  }
  out.rs1 = rs1;
  out.rs2 = rs2;
  out.target = br.operand(1).mbb;
  return true;
}

// 检查寄存器 r 是否没有被 [begin, end) 区间内的指令使用。
bool regUsedInRange(int r, std::vector<std::unique_ptr<MachineInst>>::const_iterator begin,
                    std::vector<std::unique_ptr<MachineInst>>::const_iterator end) {
  for (auto it = begin; it != end; ++it) {
    const MachineInst& mi = **it;
    for (size_t k = 0; k < mi.numOperands(); ++k) {
      const MOperand& op = mi.operand(k);
      if (op.isReg() && !op.isDef && op.reg == r)
        return true;
    }
  }
  return false;
}

// 检查寄存器 r 在基本块 bb 中是否先使用后定义。
// 如果在首次定义前使用，则返回 true，表示该寄存器是块入口需要的值。
bool regUsedBeforeRedefinedInBlock(int r, const MachineBasicBlock* bb) {
  for (auto& miPtr : bb->insts()) {
    const MachineInst& mi = *miPtr;
    // 先检查使用，因为一条指令可能同时使用和定义同一寄存器。
    for (size_t k = 0; k < mi.numOperands(); ++k) {
      const MOperand& op = mi.operand(k);
      if (op.isReg() && !op.isDef && op.reg == r)
        return true; // 在重新定义前使用
    }
    // 检查定义。
    for (size_t k = 0; k < mi.numOperands(); ++k) {
      const MOperand& op = mi.operand(k);
      if (op.isReg() && op.isDef && op.reg == r)
        return false; // 先重新定义再使用
    }
  }
  return false; // 到达块尾仍没有使用或定义，当前块不需要该值
}

// 传递活跃性检查：从 bb 的后继开始沿 CFG 前向遍历，判断寄存器 rd 是否活跃出块：
// - 某块在重新定义前使用 rd，表示沿该路径仍需要 rd；
// - 某块在使用前重新定义 rd，表示该路径上的值已被覆盖；
// - 某块既不使用也不定义 rd，则继续遍历其后继。
// 使用带访问集合的工作列表处理循环。
bool regIsLiveOut(int rd, const MachineBasicBlock* bb) {
  std::vector<const MachineBasicBlock*> worklist;
  std::unordered_set<const MachineBasicBlock*> visited;
  for (const MachineBasicBlock* succ : bb->succs()) {
    if (succ) worklist.push_back(succ);
  }
  while (!worklist.empty()) {
    const MachineBasicBlock* cur = worklist.back();
    worklist.pop_back();
    if (!visited.insert(cur).second) continue;
    // 扫描 cur 中对 rd 的使用和定义。
    bool foundUse = false, foundDef = false;
    for (auto& miPtr : cur->insts()) {
      if (!miPtr) continue; // 防御性跳过暂时为空的移动槽位
      const MachineInst& mi = *miPtr;
      for (size_t k = 0; k < mi.numOperands(); ++k) {
        const MOperand& op = mi.operand(k);
        if (!op.isReg() || op.reg != rd) continue;
        if (op.isDef) { foundDef = true; break; }
        else { foundUse = true; break; }
      }
      if (foundUse || foundDef) break;
    }
    if (foundUse) return true;   // 该路径仍需要 rd
    if (foundDef) continue;      // 该路径已覆盖 rd，在此停止
    // 既没有使用也没有定义，继续检查后继。
    for (const MachineBasicBlock* succ : cur->succs()) {
      if (succ && !visited.count(succ))
        worklist.push_back(succ);
    }
  }
  return false; // rd 在所有路径上都不是 live-out
}

// 如果 bb 在前置清理后只包含一条无条件 `j target`，则它是跳板块。
// 可以把所有指向 bb 的分支改为指向 target，从而绕过该块；成功时返回 target。
MachineBasicBlock* trampolineTarget(MachineBasicBlock* bb) {
  if (!bb) return nullptr;
  // 允许前序清理留下的说明项或已被删除的伪指令存在。
  MachineInst* sole = nullptr;
  int realCount = 0;
  for (auto& mi : bb->insts()) {
    MIOp op = mi->opcode();
    // 跳过不会发射为真实指令的残留伪指令。
    if (op == MIOp::PHI || op == MIOp::PHI_COPY) continue;
    sole = mi.get();
    if (++realCount > 1) return nullptr;
  }
  if (!sole || sole->opcode() != MIOp::J) return nullptr;
  if (sole->numOperands() != 1 || !sole->operand(0).isMBB()) return nullptr;
  MachineBasicBlock* tgt = sole->operand(0).mbb;
  if (tgt == bb) return nullptr; // 自循环保留不动
  return tgt;
}

// 遍历指令操作数，按照 redirect 映射传递重定向其中的 MBB 目标。
void redirectBranchOperands(MachineInst& mi,
                            const std::unordered_map<MachineBasicBlock*, MachineBasicBlock*>& redirect) {
  for (auto& op : mi.operands()) {
    if (!op.isMBB()) continue;
    MachineBasicBlock* cur = op.mbb;
    // 沿映射链遍历，并防止意外形成环。
    std::unordered_set<MachineBasicBlock*> seen;
    while (cur) {
      auto it = redirect.find(cur);
      if (it == redirect.end()) break;
      if (!seen.insert(cur).second) break;
      cur = it->second;
    }
    op.mbb = cur;
  }
}

} // namespace

void runPeephole(MachineFunction& mf) {
  // A spill-everything allocation may define a value, preserve its stable
  // stack home, and immediately reload the same full-width value for an ABI
  // argument. Keep the home store, but forward the still-live physical source
  // into the reload destination. The pair may still use FrameIdx operands or
  // already lowered equal imm(sp) offsets. Narrow integer stores are excluded
  // because a move would not reproduce truncation/sign-extension behavior.
  for (auto& bb : mf.blocks()) {
    auto& insts = bb->insts();
    for (size_t index = 0; index + 1 < insts.size(); ++index) {
      MIOp moveOp = MIOp::COUNT;
      int destination = -1;
      int source = -1;
      RegClass regClass = RegClass::GPR;
      if (!matchAdjacentStackReload(*insts[index], *insts[index + 1], moveOp,
                                    destination, source, regClass)) {
        continue;
      }
      if (destination == source) {
        insts.erase(insts.begin() + index + 1);
      } else {
        auto move = std::make_unique<MachineInst>(moveOp);
        move->addOperand(MOperand::Def(destination, regClass));
        move->addOperand(MOperand::Use(source, regClass));
        move->comment = "forward adjacent stack reload";
        move->setParent(bb.get());
        insts[index + 1] = std::move(move);
      }
    }
  }

  // 第 1 遍：删除冗余移动，使后续跳板识别看到最简单的块形状。
  for (auto& bb : mf.blocks()) {
    std::vector<std::unique_ptr<MachineInst>> out;
    out.reserve(bb->insts().size());
    for (auto& mi : bb->insts()) {
      if (isRedundantMove(*mi)) continue;
      out.push_back(std::move(mi));
    }
    bb->insts() = std::move(out);
  }

  // 第 1.5 遍：合并比较与分支。
  // 将 `slt/sltu rd, rs1, rs2; bnez/beqz rd, target` 合并为
  // `blt/bge/bltu/bgeu rs1, rs2, target`，减少比较密集代码中的指令数量和寄存器压力。
  if (!sycEnvironmentValue("MIR_NO_CMP_BRANCH_FUSION")) {
    for (auto& bb : mf.blocks()) {
      auto& insts = bb->insts();
      if (insts.size() < 2) continue;
      // 先在 insts 完整时决定所有合并。regIsLiveOut 会沿 CFG 前向遍历，并可能
      // 通过循环回边再次访问当前块；如果边判断边移动 insts 中的指令，遍历会看到
      // 空槽位，既可能崩溃也可能误判活跃性。提前决定可以保持所有可达块的一致状态。
      std::vector<CompareBranchFusion> fusionAt(insts.size());
      std::vector<char> doFuse(insts.size(), 0);
      for (size_t i = 0; i + 1 < insts.size(); ++i) {
        CompareBranchFusion fusion;
        if (!matchCompareBranch(*insts[i], *insts[i + 1], fusion)) continue;
        // 检查 rd 在当前块的分支之后没有使用，并且不在任何后继块中活跃出块。
        int rd = insts[i]->operand(0).reg;
        bool rdUsedLater = regUsedInRange(rd, insts.begin() + i + 2, insts.end());
        bool rdLiveOut = regIsLiveOut(rd, bb.get());
        if (!rdUsedLater && !rdLiveOut) {
          fusionAt[i] = fusion;
          doFuse[i] = 1;
        }
      }
      // 按记录的融合结果重建基本块。
      std::vector<std::unique_ptr<MachineInst>> out;
      out.reserve(insts.size());
      size_t i = 0;
      while (i < insts.size()) {
        if (i + 1 < insts.size() && doFuse[i]) {
          // 发射合并后的分支：blt/bge/bltu/bgeu rs1, rs2, target。
          auto fused = std::make_unique<MachineInst>(fusionAt[i].fusedOp);
          fused->addOperand(MOperand::Use(fusionAt[i].rs1, RegClass::GPR));
          fused->addOperand(MOperand::Use(fusionAt[i].rs2, RegClass::GPR));
          fused->addOperand(MOperand::BB(fusionAt[i].target));
          out.push_back(std::move(fused));
          i += 2; // 跳过原来的两条指令
          continue;
        }
        out.push_back(std::move(insts[i]));
        ++i;
      }
      bb->insts() = std::move(out);
    }
  }

  // 第 2 遍：分支穿线。只有一条 `j target` 的基本块可以被绕过，所有引用都重定向。
  // 先计算完整的传递映射，再统一应用。
  std::unordered_map<MachineBasicBlock*, MachineBasicBlock*> directTarget;
  for (auto& bb : mf.blocks()) {
    if (MachineBasicBlock* tgt = trampolineTarget(bb.get())) {
      directTarget[bb.get()] = tgt;
    }
  }
  if (!directTarget.empty()) {
    // 不重定向入口块，因为链接器或调用者会从函数外部到达它；同时避免 main 的
    // 单跳入口被删除后函数失去入口。
    if (!mf.blocks().empty()) directTarget.erase(mf.blocks().front().get());
    for (auto& bb : mf.blocks()) {
      for (auto& mi : bb->insts()) redirectBranchOperands(*mi, directTarget);
    }
  }

  // 第 3 遍：如果 `condbr target_next` 后紧跟 `j elsewhere`，则反转条件，使下
  // 一个块成为顺序落下路径并删除显式跳转。
  std::vector<MachineBasicBlock*> layout2;
  for (auto& bb : mf.blocks()) layout2.push_back(bb.get());
  for (size_t i = 0; i < layout2.size(); ++i) {
    MachineBasicBlock* bb = layout2[i];
    MachineBasicBlock* next = (i + 1 < layout2.size()) ? layout2[i + 1] : nullptr;
    auto& insts = bb->insts();
    if (!next || insts.size() < 2) continue;
    MachineInst* br = insts[insts.size() - 2].get();
    MachineInst* j = insts.back().get();
    if (!mioIsBranch(br->opcode()) || j->opcode() != MIOp::J) continue;
    if (br->numOperands() == 0 || !br->operand(br->numOperands() - 1).isMBB()) continue;
    if (j->numOperands() != 1 || !j->operand(0).isMBB()) continue;
    if (br->operand(br->numOperands() - 1).mbb != next) continue;
    MIOp inv = invertBranchOpcode(br->opcode());
    if (inv == MIOp::COUNT) continue;
    br->setOpcode(inv);
    br->operand(br->numOperands() - 1).mbb = j->operand(0).mbb;
    insts.pop_back();
  }

  // 第 4 遍：删除落入下一块的 `j`。前面的重定向和分支反转可能再次暴露这种跳转。
  std::vector<MachineBasicBlock*> layout;
  for (auto& bb : mf.blocks()) layout.push_back(bb.get());
  for (size_t i = 0; i < layout.size(); ++i) {
    MachineBasicBlock* bb = layout[i];
    MachineBasicBlock* next = (i + 1 < layout.size()) ? layout[i + 1] : nullptr;
    std::vector<std::unique_ptr<MachineInst>> out;
    out.reserve(bb->insts().size());
    for (auto& mi : bb->insts()) {
      if (isFallthroughJ(*mi, next)) continue;
      out.push_back(std::move(mi));
    }
    bb->insts() = std::move(out);
  }
}

void runPeephole(MachineModule& mm) {
  for (auto& fn : mm.functions()) runPeephole(*fn);
}

} // namespace mir
