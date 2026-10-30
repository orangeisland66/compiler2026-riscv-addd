// 文件职责：实现简化寄存器分配路径，用于小函数或 fallback 场景。本文件负责实现具体逻辑。
#include "simple_ra.h"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace mir {

namespace {

// 临时寄存器池用于保存溢出值。对于最多有 3 个 GPR/FPR 操作数槽位的指令，
// 每个临时寄存器只在一条指令内保持活跃，因此不会互相覆盖。
constexpr int kGprScratch[3] = {gpr::t5, gpr::t6, gpr::t4};
constexpr int kFprScratch[3] = {fpr::ft10, fpr::ft11, fpr::ft9};

bool isCallInst(MIOp op) { return mioIsCall(op); }

// 如果虚拟寄存器还没有栈槽，则预先分配一个栈帧槽位。
int slotFor(MachineFunction& mf, int vreg) {
  auto& info = mf.vregInfo(vreg);
  if (info.spillSlot >= 0) return info.spillSlot;
  int size = info.cls == RegClass::FPR ? 4 : 8; // 整数为 32 位，指针使用 64 位槽位
  int align = info.cls == RegClass::FPR ? 4 : 8;
  int fi = mf.newFrameObject(size, align, /*isSpill=*/true,
                              "spill.v" + std::to_string(vreg - FIRST_VREG));
  info.spillSlot = fi;
  return fi;
}

MIOp loadOpFor(RegClass cls) {
  return cls == RegClass::FPR ? MIOp::FLW : MIOp::LD;
}
MIOp storeOpFor(RegClass cls) {
  return cls == RegClass::FPR ? MIOp::FSW : MIOp::SD;
}

// 发射 reload：从溢出槽加载到物理寄存器。
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

// 降低单个基本块中的指令。
void rewriteBlock(MachineFunction& mf, MachineBasicBlock& bb) {
  std::vector<std::unique_ptr<MachineInst>> outBuf;
  outBuf.reserve(bb.insts().size() * 3);

  for (auto& miPtr : bb.insts()) {
    MachineInst* mi = miPtr.get();
    MIOp op = mi->opcode();

    // PHI 伪指令在这里完全移除；输入边上的复制指令会在寄存器分配完成后由
    // 独立步骤为每个基本块补出。
    if (op == MIOp::PHI) {
      continue; // 丢弃
    }

    // 每条指令独立维护临时寄存器计数。
    int gprUseIdx = 0, fprUseIdx = 0;
    int gprDefIdx = 0, fprDefIdx = 0;

    // 先为虚拟寄存器使用规划 reload。
    struct Reload { int physReg; RegClass cls; int frameIdx; };
    std::vector<Reload> reloads;
    // 再为虚拟寄存器定义规划 spill。
    struct Spill { int physReg; RegClass cls; int frameIdx; };
    std::vector<Spill> spills;

    // 遍历操作数：使用分配临时寄存器并排入 reload，定义分配临时寄存器并排入
    // spill。同一个虚拟寄存器在一条指令中重复出现时（例如 `add v, v, v`），
    // 后续使用复用第一次分配的临时寄存器。
    std::unordered_map<int, int> vregToScratch; // 虚拟寄存器 -> 当前指令使用的物理寄存器

    for (auto& opnd : mi->operands()) {
      if (!opnd.isReg() || !isVirtualReg(opnd.reg)) continue;
      int vreg = opnd.reg;
      RegClass cls = opnd.regClass;

      if (opnd.isDef) {
        // 定义稍后分配，先完成所有使用的临时寄存器安排。
        continue;
      }

      // 当前指令是否已经为该虚拟寄存器分配临时寄存器？
      auto it = vregToScratch.find(vreg);
      if (it != vregToScratch.end()) continue;

      // 选择临时寄存器并安排 reload。
      int scratch;
      if (cls == RegClass::FPR) {
        if (fprUseIdx >= 3)
          throw std::runtime_error("simple_ra: too many FPR uses in one inst");
        scratch = kFprScratch[fprUseIdx++];
      } else {
        if (gprUseIdx >= 3)
          throw std::runtime_error("simple_ra: too many GPR uses in one inst");
        scratch = kGprScratch[gprUseIdx++];
      }
      vregToScratch[vreg] = scratch;
      reloads.push_back({scratch, cls, slotFor(mf, vreg)});
    }

    // 现在为定义分配临时寄存器。
    for (auto& opnd : mi->operands()) {
      if (!opnd.isReg() || !isVirtualReg(opnd.reg)) continue;
      if (!opnd.isDef) continue;
      int vreg = opnd.reg;
      RegClass cls = opnd.regClass;
      int scratch;
      // 定义使用同一寄存器池的第一个临时寄存器。使用独立计数器，避免与使用的
      // 编号冲突，因为定义是在指令读取输入之后才覆盖寄存器。
      if (cls == RegClass::FPR) {
        if (fprDefIdx >= 3)
          throw std::runtime_error("simple_ra: too many FPR defs in one inst");
        // 复用第一个临时寄存器。定义写入时使用已经读取完成，因此覆盖是安全的。
        scratch = kFprScratch[fprDefIdx++];
      } else {
        if (gprDefIdx >= 3)
          throw std::runtime_error("simple_ra: too many GPR defs in one inst");
        scratch = kGprScratch[gprDefIdx++];
      }
      vregToScratch[vreg] = scratch;
      spills.push_back({scratch, cls, slotFor(mf, vreg)});
    }

    // 发射 reload。
    for (auto& r : reloads) {
      outBuf.push_back(makeReload(r.physReg, r.cls, r.frameIdx));
      outBuf.back()->setParent(&bb);
    }

    // 重写操作数中的寄存器。
    for (auto& opnd : mi->operands()) {
      if (!opnd.isReg() || !isVirtualReg(opnd.reg)) continue;
      auto it = vregToScratch.find(opnd.reg);
      if (it != vregToScratch.end()) opnd.reg = it->second;
    }

    // 把原指令移动到输出缓冲区。
    outBuf.push_back(std::move(miPtr));

    // 在指令之后发射 spill。
    for (auto& s : spills) {
      outBuf.push_back(makeSpill(s.physReg, s.cls, s.frameIdx));
      outBuf.back()->setParent(&bb);
    }
  }

  bb.insts() = std::move(outBuf);
}

// 基本块重写移除 PHI 伪指令后，在每条输入边插入“加载 src，再写入 phi 目标槽位”
// 来还原其效果。每个 phi 目标都有独立栈槽，因此这种顺序处理不会产生冲突。
void resolvePhis(MachineFunction& mf,
                 std::unordered_map<MachineBasicBlock*,
                                    std::vector<std::unique_ptr<MachineInst>>>& savedPhis) {
  for (auto& bbPtr : mf.blocks()) {
    MachineBasicBlock* bb = bbPtr.get();
    auto it = savedPhis.find(bb);
    if (it == savedPhis.end()) continue;
    for (auto& phi : it->second) {
      // 操作数顺序为 def，之后是若干组（输入虚拟寄存器、输入 MIR 基本块）。
      if (phi->numOperands() == 0 || !phi->operand(0).isReg()) continue;
      int destVReg = phi->operand(0).reg;
      RegClass cls = phi->operand(0).regClass;
      int destSlot = slotFor(mf, destVReg);

      for (size_t i = 1; i + 1 < phi->numOperands(); i += 2) {
        MOperand& srcOp = phi->operand(i);
        MOperand& bbOp = phi->operand(i + 1);
        if (!srcOp.isReg() || !bbOp.isMBB() || !bbOp.mbb) continue;
        int srcVReg = srcOp.reg;
        MachineBasicBlock* pred = bbOp.mbb;

        // 选择临时寄存器。前驱终结指令的 reload 已经发射，这里把新的加载和存储
        // 插入到终结指令之前，避免覆盖仍在使用的值。
        int scratch = cls == RegClass::FPR ? fpr::ft10 : gpr::t5;
        int srcSlot = slotFor(mf, srcVReg);

        // 构造 reload 和写入 phi 槽位的指令对。
        auto reload = makeReload(scratch, cls, srcSlot);
        auto store = makeSpill(scratch, cls, destSlot);

        // 找到前驱的终结指令并在其之前插入。
        MachineInst* term = pred->terminator();
        if (term) {
          pred->insertBefore(term, std::move(reload));
          pred->insertBefore(term, std::move(store));
        } else {
          pred->append(std::move(reload));
          pred->append(std::move(store));
        }
      }
    }
  }
}

} // namespace

void runSimpleRA(MachineFunction& mf) {
  if (mf.blocks().empty()) return;

  // 在 rewriteBlock 删除 PHI 之前保存它们。
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

  // 重写指令：在每次使用和定义周围插入 reload/spill。
  for (auto& bb : mf.blocks()) {
    rewriteBlock(mf, *bb);
  }

  // 通过在输入边上经由栈槽插入复制来处理 phi。
  resolvePhis(mf, savedPhis);

  // 暂时将所有被调用者保存寄存器标记为空；prologue pass 会根据函数体实际使用
  // 的物理寄存器重新计算集合。
  mf.usedCalleeSavedGPR.clear();
  mf.usedCalleeSavedFPR.clear();
  (void)isCallInst;
}

void runSimpleRA(MachineModule& mm) {
  for (auto& fn : mm.functions()) runSimpleRA(*fn);
}

} // namespace mir
