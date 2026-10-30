#include "research_switch.h"
// 文件职责：生成函数序言/尾声和栈帧布局，遵守 RISC-V ABI。本文件负责实现具体逻辑。
#include "prologue.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <stdexcept>
#include <numeric>
#include <unordered_set>
#include <vector>

namespace mir {

namespace {

int alignUp(int x, int a) {
  if (a <= 1) return x;
  int r = x % a;
  return r == 0 ? x : x + (a - r);
}

bool fitsSigned12(int64_t v) { return v >= -2048 && v <= 2047; }

int frameLayoutBucket(const FrameObject& fo) {
  // 栈帧布局优先把高频小对象放到低偏移区，尽量保住 imm12。
  // 这属于通用 frame-layout 优化：spill、小标量槽位和短生命周期临时对象
  // 通常访问更频繁；大私有数组虽然也可能在热路径里，但它们更适合先取一次
  // base，再在局部用小偏移/GEP 递推，因此整体应后置，避免把大量标量槽位
  // 一起挤出 12-bit 偏移范围。
  if (fo.isSpill)
    return 0;
  if (fo.size <= 16)
    return 1;
  if (fo.size <= 64)
    return 2;
  if (fo.size <= 256)
    return 3;
  return 4;
}

// 大偏移 frame 访存需要先物化地址临时寄存器。如果临时寄存器
// 恰好与 store 的源寄存器或 base 寄存器重名，就会先把待存值/基址覆盖掉。
// 这里统一从后端保留的 scratch 集合里挑一个不冲突的寄存器。
int selectLargeOffsetScratchReg(int baseReg, int dataReg, bool isLoad) {
  const int candidates[] = {gpr::t6, gpr::t5, gpr::t4};
  for (int scratch : candidates) {
    if (scratch == baseReg)
      continue;
    if (!isLoad && scratch == dataReg)
      continue;
    return scratch;
  }
  throw std::runtime_error("no scratch register for large-offset memory op");
}

// 按 RISC-V 调用约定列出被调用者保存的 GPR，顺序与保存布局一致。
const int kCalleeSavedGPR[] = {
  gpr::s1, gpr::s2, gpr::s3, gpr::s4, gpr::s5, gpr::s6,
  gpr::s7, gpr::s8, gpr::s9, gpr::s10, gpr::s11,
};
const int kCalleeSavedFPR[] = {
  fpr::fs0, fpr::fs1, fpr::fs2, fpr::fs3, fpr::fs4, fpr::fs5,
  fpr::fs6, fpr::fs7, fpr::fs8, fpr::fs9, fpr::fs10, fpr::fs11,
};

struct EarlyReturnShrinkWrap {
  MachineBasicBlock* prologueBlock = nullptr;
  MachineBasicBlock* fastReturn = nullptr;
  std::unordered_set<MachineBasicBlock*> framedBlocks;
};

bool isSavedPhysicalRegister(int reg) {
  if (reg == gpr::ra || reg == gpr::s0 || reg == gpr::sp)
    return true;
  for (int saved : kCalleeSavedGPR) {
    if (reg == saved)
      return true;
  }
  for (int saved : kCalleeSavedFPR) {
    if (reg == saved)
      return true;
  }
  return false;
}

bool blockNeedsEstablishedFrame(const MachineBasicBlock* block) {
  if (!block)
    return true;
  for (const auto& miPtr : block->insts()) {
    if (!miPtr || mioIsCall(miPtr->opcode()))
      return true;
    for (const MOperand& operand : miPtr->operands()) {
      if (operand.isFrame())
        return true;
      if (operand.isReg() && isPhysReg(operand.reg) &&
          isSavedPhysicalRegister(operand.reg) &&
          !(miPtr->opcode() == MIOp::RET && operand.reg == gpr::ra))
        return true;
    }
  }
  return false;
}

std::optional<EarlyReturnShrinkWrap>
findEarlyReturnShrinkWrap(MachineFunction& mf) {
  MachineBasicBlock* entry = mf.entryBlock();
  if (!entry || mf.incomingStackArgBytes != 0 ||
      entry->succs().size() != 2 || blockNeedsEstablishedFrame(entry))
    return std::nullopt;

  auto isFastReturn = [entry](MachineBasicBlock* block) {
    return block && block->preds().size() == 1 &&
           block->preds().front() == entry && block->succs().empty() &&
           block->insts().size() <= 4 && block->terminator() &&
           block->terminator()->opcode() == MIOp::RET &&
           !blockNeedsEstablishedFrame(block);
  };
  MachineBasicBlock* first = entry->succs()[0];
  MachineBasicBlock* second = entry->succs()[1];
  const bool firstReturns = isFastReturn(first);
  const bool secondReturns = isFastReturn(second);
  if (firstReturns == secondReturns)
    return std::nullopt;

  MachineBasicBlock* fastReturn = firstReturns ? first : second;
  MachineBasicBlock* prologueBlock = firstReturns ? second : first;
  if (!prologueBlock || prologueBlock->preds().size() != 1 ||
      prologueBlock->preds().front() != entry)
    return std::nullopt;

  std::unordered_set<MachineBasicBlock*> framed;
  std::queue<MachineBasicBlock*> worklist;
  framed.insert(prologueBlock);
  worklist.push(prologueBlock);
  while (!worklist.empty()) {
    MachineBasicBlock* block = worklist.front();
    worklist.pop();
    for (MachineBasicBlock* succ : block->succs()) {
      if (succ != entry && succ != fastReturn && framed.insert(succ).second)
        worklist.push(succ);
    }
  }
  if (framed.count(fastReturn))
    return std::nullopt;

  for (const auto& bbPtr : mf.blocks()) {
    MachineBasicBlock* block = bbPtr.get();
    if (block == entry || block == fastReturn || framed.count(block))
      continue;
    // 不要围绕无关或不可达的栈帧使用点收缩序言范围。
    if (blockNeedsEstablishedFrame(block))
      return std::nullopt;
  }
  return EarlyReturnShrinkWrap{prologueBlock, fastReturn, std::move(framed)};
}

// 扫描函数体，返回其中使用的被调用者保存寄存器。
void scanCalleeSavedUsage(MachineFunction& mf,
                          std::vector<int>& outGPR,
                          std::vector<int>& outFPR) {
  std::unordered_set<int> usedGPR, usedFPR;
  for (auto& bb : mf.blocks()) {
    for (auto& mi : bb->insts()) {
      for (auto& op : mi->operands()) {
        if (!op.isReg() || !isPhysReg(op.reg)) continue;
        if (op.regClass == RegClass::GPR && isGPR(op.reg)) {
          usedGPR.insert(op.reg);
        } else if (op.regClass == RegClass::FPR && isFPR(op.reg)) {
          usedFPR.insert(op.reg);
        }
      }
    }
  }
  for (int r : kCalleeSavedGPR) if (usedGPR.count(r)) outGPR.push_back(r);
  for (int r : kCalleeSavedFPR) if (usedFPR.count(r)) outFPR.push_back(r);
}

// 构造一条 SD/LD/FSW/FLW 指令，并处理可能较大的立即数。偏移放不进有符号
// 12 位立即数时，先用 li+add 将地址放入 t6，再使用偏移 0。
void emitMemoryOp(std::vector<std::unique_ptr<MachineInst>>& out,
                  MIOp memOp, int dataReg, RegClass dataCls,
                  int baseReg, int64_t offset) {
  bool isLoad = (memOp == MIOp::LB || memOp == MIOp::LH || memOp == MIOp::LW ||
                 memOp == MIOp::LD || memOp == MIOp::LBU || memOp == MIOp::LHU ||
                 memOp == MIOp::LWU || memOp == MIOp::FLW);
  if (fitsSigned12(offset)) {
    auto mi = std::make_unique<MachineInst>(memOp);
    mi->addOperand(MOperand::Reg(dataReg, dataCls,
                                 /*isDef=*/isLoad));
    mi->addOperand(MOperand::Imm(offset));
    mi->addOperand(MOperand::Use(baseReg, RegClass::GPR));
    out.push_back(std::move(mi));
    return;
  }
  // 大偏移地址不一定能直接用 t6。store 源寄存器也可能正好就是
  // t6/t5/t4，所以要先挑一个不与 base/source 冲突的 scratch。
  {
    int scratchReg = selectLargeOffsetScratchReg(baseReg, dataReg, isLoad);
    auto li = std::make_unique<MachineInst>(MIOp::LI);
    li->addOperand(MOperand::Def(scratchReg, RegClass::GPR));
    li->addOperand(MOperand::Imm(offset));
    out.push_back(std::move(li));
    auto add = std::make_unique<MachineInst>(MIOp::ADD);
    add->addOperand(MOperand::Def(scratchReg, RegClass::GPR));
    add->addOperand(MOperand::Use(baseReg, RegClass::GPR));
    add->addOperand(MOperand::Use(scratchReg, RegClass::GPR));
    out.push_back(std::move(add));
    auto mi = std::make_unique<MachineInst>(memOp);
    mi->addOperand(MOperand::Reg(dataReg, dataCls, isLoad));
    mi->addOperand(MOperand::Imm(0));
    mi->addOperand(MOperand::Use(scratchReg, RegClass::GPR));
    out.push_back(std::move(mi));
  }
}

// 生成 `addi rd, sp, offset`；偏移较大时通过 t6 计算。
void emitSpRel(std::vector<std::unique_ptr<MachineInst>>& out,
               int dstReg, int64_t offset) {
  if (fitsSigned12(offset)) {
    auto mi = std::make_unique<MachineInst>(MIOp::ADDI);
    mi->addOperand(MOperand::Def(dstReg, RegClass::GPR));
    mi->addOperand(MOperand::Use(gpr::sp, RegClass::GPR));
    mi->addOperand(MOperand::Imm(offset));
    out.push_back(std::move(mi));
    return;
  }
  auto li = std::make_unique<MachineInst>(MIOp::LI);
  li->addOperand(MOperand::Def(dstReg, RegClass::GPR));
  li->addOperand(MOperand::Imm(offset));
  out.push_back(std::move(li));
  auto add = std::make_unique<MachineInst>(MIOp::ADD);
  add->addOperand(MOperand::Def(dstReg, RegClass::GPR));
  add->addOperand(MOperand::Use(gpr::sp, RegClass::GPR));
  add->addOperand(MOperand::Use(dstReg, RegClass::GPR));
  out.push_back(std::move(add));
}

// 生成 `addi sp, sp, imm`；立即数较大时通过 t6 计算。
void emitSpAdjust(std::vector<std::unique_ptr<MachineInst>>& out, int64_t delta) {
  if (delta == 0) return;
  if (fitsSigned12(delta)) {
    auto mi = std::make_unique<MachineInst>(MIOp::ADDI);
    mi->addOperand(MOperand::Def(gpr::sp, RegClass::GPR));
    mi->addOperand(MOperand::Use(gpr::sp, RegClass::GPR));
    mi->addOperand(MOperand::Imm(delta));
    out.push_back(std::move(mi));
    return;
  }
  auto li = std::make_unique<MachineInst>(MIOp::LI);
  li->addOperand(MOperand::Def(gpr::t6, RegClass::GPR));
  li->addOperand(MOperand::Imm(delta));
  out.push_back(std::move(li));
  auto add = std::make_unique<MachineInst>(MIOp::ADD);
  add->addOperand(MOperand::Def(gpr::sp, RegClass::GPR));
  add->addOperand(MOperand::Use(gpr::sp, RegClass::GPR));
  add->addOperand(MOperand::Use(gpr::t6, RegClass::GPR));
  out.push_back(std::move(add));
}

} // namespace

void lowerFrame(MachineFunction& mf) {
  if (mf.blocks().empty()) return;

  // 1. 统计实际使用的被调用者保存寄存器，只保存必要集合。
  scanCalleeSavedUsage(mf, mf.usedCalleeSavedGPR, mf.usedCalleeSavedFPR);

  // 叶子函数快速路径：没有调用、栈槽、被调用者保存寄存器或栈上传参时，
  // 不生成序言/尾声。触发条件只来自 ABI 和 frame 形状，不来自函数名。
  bool leafFast = !mf.hasCalls
      && mf.frame().empty()
      && mf.usedCalleeSavedGPR.empty()
      && mf.usedCalleeSavedFPR.empty()
      && mf.outgoingArgBytes == 0
      && mf.incomingStackArgBytes == 0;
  if (leafFast) {
    mf.frameSize = 0;
    // frame 为空时不会存在 FrameIdx，RET 伪指令会直接打印为 ret。
    return;
  }

  // 2. 计算栈帧布局。
  //    按 sp 从低地址到高地址排列：
  //      [0, outgoingArgBytes)：调用方栈参数区；simple-RA 回退时也作暂存区
  //      [outgoingArgBytes, userBytes)：FrameObject（alloca 和 spill）
  //      [userBytes, ...)：ABI 要求保存的寄存器
  //    总大小向上取整到 16 字节。
  //
  // 旧实现总是保存 ra/s0 并建立 frame pointer。实际上本后端的
  // FrameIdx 都在 lowerFrame 中改成 sp-relative，只有“入栈实参”需要 s0
  // 指回调用者进入本函数时的 sp；ra 也只在本函数会发起调用时才会被覆盖。
  // 这里按 ABI 事实按需保存（通用 prologue/epilogue 收缩）。
  int userBytes = alignUp(mf.outgoingArgBytes, 8);
  std::vector<int> frameOrder(mf.frame().size());
  std::iota(frameOrder.begin(), frameOrder.end(), 0);
  std::stable_sort(frameOrder.begin(), frameOrder.end(),
                   [&](int lhs, int rhs) {
                     const auto& a = mf.frame()[lhs];
                     const auto& b = mf.frame()[rhs];
                     int bucketA = frameLayoutBucket(a);
                     int bucketB = frameLayoutBucket(b);
                     if (bucketA != bucketB)
                       return bucketA < bucketB;
                     if (a.align != b.align)
                       return a.align > b.align;
                     if (a.size != b.size)
                       return a.size < b.size;
                     return lhs < rhs;
                   });
  for (int frameIdx : frameOrder) {
    auto& fo = mf.frame()[static_cast<size_t>(frameIdx)];
    userBytes = alignUp(userBytes, fo.align);
    fo.offset = userBytes;
    userBytes += fo.size;
  }
  userBytes = alignUp(userBytes, 8);

  const bool saveRA = mf.hasCalls;
  const bool saveS0 = mf.incomingStackArgBytes > 0;

  int savedRegsBytes = 0;
  if (saveS0)
    savedRegsBytes += 8;
  if (saveRA)
    savedRegsBytes += 8;
  savedRegsBytes += 8 * mf.usedCalleeSavedGPR.size();
  savedRegsBytes += 4 * mf.usedCalleeSavedFPR.size();
  savedRegsBytes = alignUp(savedRegsBytes, 8);

  int frameSize = alignUp(userBytes + savedRegsBytes, 16);
  if (frameSize < 16) frameSize = 16;
  mf.frameSize = frameSize;

  // 保存寄存器相对于 sp 的偏移：
  int s0Off = -1;
  int raOff = -1;
  std::vector<std::pair<int,int>> savedGprOff;
  std::vector<std::pair<int,int>> savedFprOff;
  int cursor = userBytes;
  if (saveS0) {
    s0Off = cursor;
    cursor += 8;
  }
  if (saveRA) {
    raOff = cursor;
    cursor += 8;
  }
  for (int r : mf.usedCalleeSavedGPR) {
    savedGprOff.emplace_back(r, cursor);
    cursor += 8;
  }
  for (int r : mf.usedCalleeSavedFPR) {
    savedFprOff.emplace_back(r, cursor);
    cursor += 4;
  }

  // 3. 遍历每条指令，替换 FrameIdx 操作数，并把回存/重载形式改写为真实的
  //    sp 相对偏移。常见引用 FrameIdx 的指令包括：
  //      - ADDI dst, FrameIdx, 0  (alloca address)
  //      - LW/LD/FLW dst, FrameIdx, sp   (reload)
  //      - SW/SD/FSW src, FrameIdx, sp   (spill)
  for (auto& bb : mf.blocks()) {
    std::vector<std::unique_ptr<MachineInst>> out;
    out.reserve(bb->insts().size());

    for (auto& mi : bb->insts()) {
        // 检查位置 1 的 FrameIdx 操作数，这是常见的回存、重载和 alloca 形式。
      MachineInst* inst = mi.get();
      MIOp op = inst->opcode();

        // 情况 A：ADDI dst, FrameIdx, 0，生成 ADDI dst, sp, off。
      if (op == MIOp::ADDI && inst->numOperands() == 3 &&
          inst->operand(1).isFrame()) {
        int dst = inst->operand(0).reg;
        int fi = inst->operand(1).frameIdx;
        int off = mf.frame()[fi].offset;
        // 生成新的 ADDI 或 li+add 序列，并丢弃原指令。
        emitSpRel(out, dst, off);
        continue;
      }

      // 情况 B：内存操作在槽位 1 使用 FrameIdx，第三个操作数为 sp。
      auto isFrameMem = [&]() {
        return inst->numOperands() == 3 &&
               (op == MIOp::LW || op == MIOp::LD || op == MIOp::LB ||
                op == MIOp::LH || op == MIOp::LBU || op == MIOp::LHU ||
                op == MIOp::LWU || op == MIOp::FLW ||
                op == MIOp::SW || op == MIOp::SD || op == MIOp::SB ||
                op == MIOp::SH || op == MIOp::FSW) &&
               inst->operand(1).isFrame();
      };
      if (isFrameMem()) {
        int dataReg = inst->operand(0).reg;
        RegClass cls = inst->operand(0).regClass;
        int fi = inst->operand(1).frameIdx;
        int off = mf.frame()[fi].offset;
        emitMemoryOp(out, op, dataReg, cls, gpr::sp, off);
        continue;
      }

      // 默认保留原指令。
      out.push_back(std::move(mi));
    }
    bb->insts() = std::move(out);
  }

  // 4. 在入口块开头构造函数序言。
  std::vector<std::unique_ptr<MachineInst>> prologue;
  // sp -= frameSize
  emitSpAdjust(prologue, -frameSize);
  if (saveS0)
    emitMemoryOp(prologue, MIOp::SD, gpr::s0, RegClass::GPR, gpr::sp, s0Off);
  if (saveRA)
    emitMemoryOp(prologue, MIOp::SD, gpr::ra, RegClass::GPR, gpr::sp, raOff);
  for (auto& sg : savedGprOff) {
    emitMemoryOp(prologue, MIOp::SD, sg.first, RegClass::GPR, gpr::sp, sg.second);
  }
  for (auto& sf : savedFprOff) {
    emitMemoryOp(prologue, MIOp::FSW, sf.first, RegClass::FPR, gpr::sp, sf.second);
  }
  if (saveS0)
    emitSpRel(prologue, gpr::s0, frameSize);

  const std::optional<EarlyReturnShrinkWrap> shrinkWrap =
      sycEnvironmentValue("SYC_SKIP_MIR_EARLY_RETURN_SHRINKWRAP")
          ? std::nullopt
          : findEarlyReturnShrinkWrap(mf);
  // 常规情况仍然在函数入口插入序言。简单的提前返回不修改 sp 和保存寄存器，
  // 因此只有它的慢速后继需要栈帧。
  auto* entry = mf.entryBlock();
  MachineBasicBlock* prologueBlock =
      shrinkWrap ? shrinkWrap->prologueBlock : entry;
  auto& einsts = prologueBlock->insts();
  std::vector<std::unique_ptr<MachineInst>> newEntry;
  newEntry.reserve(prologue.size() + einsts.size());
  for (auto& pi : prologue) {
    pi->setParent(prologueBlock);
    newEntry.push_back(std::move(pi));
  }
  for (auto& pi : einsts) newEntry.push_back(std::move(pi));
  einsts = std::move(newEntry);

  // 5. 在每个 RET 位置插入函数尾声，把 `ret` 替换为：
  //      恢复保存寄存器；addi sp, sp, frameSize；ret
  for (auto& bb : mf.blocks()) {
    std::vector<std::unique_ptr<MachineInst>> out;
    out.reserve(bb->insts().size() + 8);
    for (auto& mi : bb->insts()) {
      const bool needsEpilogue =
          !shrinkWrap || shrinkWrap->framedBlocks.count(bb.get()) != 0;
      if (mi->opcode() == MIOp::RET && needsEpilogue) {
        // 在 ret 前生成函数尾声。
        std::vector<std::unique_ptr<MachineInst>> epi;
        // 按相反顺序恢复寄存器。
        for (auto it = savedFprOff.rbegin(); it != savedFprOff.rend(); ++it) {
          emitMemoryOp(epi, MIOp::FLW, it->first, RegClass::FPR, gpr::sp, it->second);
        }
        for (auto it = savedGprOff.rbegin(); it != savedGprOff.rend(); ++it) {
          emitMemoryOp(epi, MIOp::LD, it->first, RegClass::GPR, gpr::sp, it->second);
        }
        if (saveRA)
          emitMemoryOp(epi, MIOp::LD, gpr::ra, RegClass::GPR, gpr::sp, raOff);
        if (saveS0)
          emitMemoryOp(epi, MIOp::LD, gpr::s0, RegClass::GPR, gpr::sp, s0Off);
        emitSpAdjust(epi, frameSize);
        for (auto& pi : epi) {
          pi->setParent(bb.get());
          out.push_back(std::move(pi));
        }
        out.push_back(std::move(mi));
      } else {
        out.push_back(std::move(mi));
      }
    }
    bb->insts() = std::move(out);
  }
}

void lowerFrame(MachineModule& mm) {
  for (auto& fn : mm.functions()) lowerFrame(*fn);
}

} // namespace mir
