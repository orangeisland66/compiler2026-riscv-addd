// 文件职责：定义机器函数、机器块、机器指令和操作数等 MIR 数据结构。本文件负责实现具体逻辑。
#include "machine_ir.h"

#include "core_ir.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <unordered_set>

namespace mir {

// ============================================================
// 寄存器名称。
// ============================================================
static const char* kGprNames[32] = {
  "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0",   "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6",   "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8",   "s9", "s10","s11","t3", "t4", "t5", "t6",
};
static const char* kFprNames[32] = {
  "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7",
  "fs0", "fs1", "fa0", "fa1", "fa2", "fa3", "fa4", "fa5",
  "fa6", "fa7", "fs2", "fs3", "fs4", "fs5", "fs6", "fs7",
  "fs8", "fs9", "fs10","fs11","ft8", "ft9", "ft10","ft11",
};

const char* gprName(int r) {
  if (r < 0 || r >= 32) return "<bad-gpr>";
  return kGprNames[r];
}
const char* fprName(int r) {
  if (r < 32 || r >= 64) return "<bad-fpr>";
  return kFprNames[r - 32];
}
const char* physRegName(int r) {
  if (isGPR(r)) return gprName(r);
  if (isFPR(r)) return fprName(r);
  return "<vreg>";
}

// ============================================================
// 操作码名称和属性标记。
// ============================================================
struct OpInfo {
  const char* mnemonic;
  uint32_t flags;
};
namespace flag {
constexpr uint32_t TERM   = 1u << 0;  // 终结指令
constexpr uint32_t LOAD   = 1u << 1;  // 可能读取内存
constexpr uint32_t STORE  = 1u << 2;  // 可能写入内存
constexpr uint32_t CALL   = 1u << 3;  // 调用，会破坏调用者保存寄存器
constexpr uint32_t BRANCH = 1u << 4;  // 条件分支
constexpr uint32_t DEFGP  = 1u << 5;  // 写入通用寄存器
constexpr uint32_t DEFFP  = 1u << 6;  // 写入浮点寄存器
} // namespace flag

#define G flag::DEFGP
#define F flag::DEFFP
#define T flag::TERM
#define L flag::LOAD
#define S flag::STORE
#define C flag::CALL
#define B flag::BRANCH

// 顺序必须与 MIOp 枚举一致。
static const OpInfo kOps[] = {
  // 伪指令。
  {"<phi>",       0},
  {"<phi-copy>",  0},
  {"mv",          0},          // 由操作数类别选择通用或浮点寄存器
  {"li",          G},
  {"la",          G},
  {"<spill>",     S},
  {"<reload>",    L},
  {"<prologue>",  0},
  {"<epilogue>",  T},

  // R 型整数指令。
  {"add",  G}, {"addw", G}, {"sub",  G}, {"subw", G},
  {"mul",  G}, {"mulw", G}, {"mulh", G}, {"div",  G}, {"divw", G},
  {"divu", G}, {"divuw",G}, {"rem",  G}, {"remw", G},
  {"remu", G}, {"remuw",G},
  {"and",  G}, {"or",   G}, {"xor",  G},
  {"sll",  G}, {"sllw", G}, {"srl",  G}, {"srlw", G},
  {"sra",  G}, {"sraw", G},
  {"slt",  G}, {"sltu", G},

  // I 型整数指令。
  {"addi", G}, {"addiw",G}, {"andi", G}, {"ori",  G}, {"xori", G},
  {"slli", G}, {"slliw",G}, {"srli", G}, {"srliw",G}, {"srai", G}, {"sraiw",G},
  {"slti", G}, {"sltiu",G},

  // 加载和存储指令。
  {"lb",  G | L}, {"lh",  G | L}, {"lw",  G | L}, {"ld",  G | L},
  {"lbu", G | L}, {"lhu", G | L}, {"lwu", G | L},
  {"sb",  S}, {"sh",  S}, {"sw",  S}, {"sd",  S},

  // 浮点指令。
  {"flw",     F | L}, {"fsw",     S},
  {"fadd.s",  F}, {"fsub.s",  F}, {"fmul.s",  F}, {"fdiv.s",  F},
  {"fsqrt.s", F}, {"fneg.s",  F}, {"fabs.s",  F},
  {"fmadd.s", F}, {"fmsub.s", F}, {"fnmadd.s",F}, {"fnmsub.s",F},
  {"feq.s",   G}, {"flt.s",   G}, {"fle.s",   G},
  {"fcvt.s.w",F}, {"fcvt.w.s",G},
  {"fmv.w.x", F}, {"fmv.x.w", G},
  {"fmv.s",   F},

  // 分支和跳转指令。
  {"beq",  T | B}, {"bne",  T | B}, {"blt",  T | B}, {"bge",  T | B},
  {"bltu", T | B}, {"bgeu", T | B},
  {"beqz", T | B}, {"bnez", T | B}, {"bltz", T | B}, {"bgez", T | B},
  {"blez", T | B}, {"bgtz", T | B},
  {"j",    T},
  {"call", C},          // 输出为 `call symbol`
  {"jalr", C},
  {"ret",  T},

  // 系统指令。
  {"ecall", C},
  {"nop",   0},
};
static_assert(sizeof(kOps) / sizeof(kOps[0]) == static_cast<size_t>(MIOp::COUNT),
              "kOps size must equal MIOp::COUNT");

#undef G
#undef F
#undef T
#undef L
#undef S
#undef C
#undef B

const char* mioName(MIOp op) { return kOps[static_cast<int>(op)].mnemonic; }
static uint32_t opFlags(MIOp op) { return kOps[static_cast<int>(op)].flags; }

bool mioIsTerminator(MIOp op) { return opFlags(op) & flag::TERM; }
bool mioMayLoad(MIOp op)       { return opFlags(op) & flag::LOAD; }
bool mioMayStore(MIOp op)      { return opFlags(op) & flag::STORE; }
bool mioIsCall(MIOp op)        { return opFlags(op) & flag::CALL; }
bool mioIsBranch(MIOp op)      { return opFlags(op) & flag::BRANCH; }
bool mioWritesGPR(MIOp op)     { return opFlags(op) & flag::DEFGP; }
bool mioWritesFPR(MIOp op)     { return opFlags(op) & flag::DEFFP; }

// ============================================================
// MOperand 构造辅助函数。
// ============================================================
MOperand MOperand::Reg(int r, RegClass cls, bool def) {
  MOperand o;
  o.kind = Kind::Reg;
  o.reg = r;
  o.isDef = def;
  o.regClass = cls;
  return o;
}
MOperand MOperand::Use(int r, RegClass cls) { return Reg(r, cls, false); }
MOperand MOperand::Def(int r, RegClass cls) { return Reg(r, cls, true); }
MOperand MOperand::Imm(int64_t v) { MOperand o; o.kind = Kind::Imm; o.imm = v; return o; }
MOperand MOperand::FImm(float v)  { MOperand o; o.kind = Kind::FImm; o.fimm = v; return o; }
MOperand MOperand::BB(MachineBasicBlock* b) { MOperand o; o.kind = Kind::MBB; o.mbb = b; return o; }
MOperand MOperand::Sym(const std::string& s) { MOperand o; o.kind = Kind::Symbol; o.sym = s; return o; }
MOperand MOperand::Frame(int idx) { MOperand o; o.kind = Kind::FrameIdx; o.frameIdx = idx; return o; }

// ============================================================
// MachineInst 辅助函数。
// ============================================================
void MachineInst::collectDefs(std::vector<int>& out) const {
  for (const auto& o : ops_) {
    if (o.isReg() && o.isDef) out.push_back(o.reg);
  }
}
void MachineInst::collectUses(std::vector<int>& out) const {
  for (const auto& o : ops_) {
    if (o.isReg() && !o.isDef) out.push_back(o.reg);
  }
}

// ============================================================
// MachineBasicBlock
// ============================================================
MachineBasicBlock::MachineBasicBlock(MachineFunction* func, std::string label)
    : parent_(func), label_(std::move(label)) {}

MachineInst* MachineBasicBlock::terminator() const {
  for (auto it = insts_.rbegin(); it != insts_.rend(); ++it) {
    if (mioIsTerminator((*it)->opcode())) return it->get();
  }
  return nullptr;
}

MachineInst* MachineBasicBlock::append(std::unique_ptr<MachineInst> inst) {
  inst->setParent(this);
  auto* p = inst.get();
  insts_.push_back(std::move(inst));
  return p;
}
MachineInst* MachineBasicBlock::prepend(std::unique_ptr<MachineInst> inst) {
  inst->setParent(this);
  auto* p = inst.get();
  insts_.insert(insts_.begin(), std::move(inst));
  return p;
}
void MachineBasicBlock::insertBefore(MachineInst* pos, std::unique_ptr<MachineInst> inst) {
  inst->setParent(this);
  for (auto it = insts_.begin(); it != insts_.end(); ++it) {
    if (it->get() == pos) {
      insts_.insert(it, std::move(inst));
      return;
    }
  }
  // 没找到插入位置时追加到块尾，正常流程不会走到这里。
  insts_.push_back(std::move(inst));
}

// ============================================================
// MachineFunction
// ============================================================
MachineFunction::MachineFunction(std::string name) : name_(std::move(name)) {}

MachineBasicBlock* MachineFunction::newBlock(const std::string& label) {
  std::string lbl = label.empty()
      ? ("." + name_ + ".bb" + std::to_string(nextLabel_++))
      : label;
  auto bb = std::make_unique<MachineBasicBlock>(this, std::move(lbl));
  bb->setId(static_cast<int>(blocks_.size()));
  auto* p = bb.get();
  blocks_.push_back(std::move(bb));
  return p;
}

int MachineFunction::newVReg(RegClass cls) {
  int id = FIRST_VREG + static_cast<int>(vregs_.size());
  VRegInfo info;
  info.id = id;
  info.cls = cls;
  vregs_.push_back(info);
  return id;
}

VRegInfo& MachineFunction::vregInfo(int v) {
  assert(v >= FIRST_VREG);
  return vregs_[v - FIRST_VREG];
}
const VRegInfo& MachineFunction::vregInfo(int v) const {
  assert(v >= FIRST_VREG);
  return vregs_[v - FIRST_VREG];
}

int MachineFunction::newFrameObject(int size, int align, bool isSpill, const std::string& name) {
  FrameObject fo;
  fo.index = static_cast<int>(frame_.size());
  fo.size = size;
  fo.align = align ? align : 4;
  fo.isSpill = isSpill;
  fo.name = name;
  frame_.push_back(fo);
  return fo.index;
}

// ============================================================
// MachineModule
// ============================================================
MachineFunction* MachineModule::newFunction(const std::string& name) {
  auto fn = std::make_unique<MachineFunction>(name);
  auto* p = fn.get();
  funcs_.push_back(std::move(fn));
  return p;
}

int MachineModule::internFloatConst(float v) {
  uint32_t bits;
  std::memcpy(&bits, &v, sizeof(bits));
  auto it = floatConstMap_.find(bits);
  if (it != floatConstMap_.end()) return it->second;
  int idx = static_cast<int>(floatConsts_.size());
  floatConsts_.push_back(v);
  floatConstMap_.emplace(bits, idx);
  return idx;
}

// ============================================================
// 打印器。
// ============================================================
namespace {

// 输出一个寄存器操作数，并根据寄存器类别选择名称。
std::string regNameOf(const MOperand& op) {
  if (!op.isReg()) return "<not-reg>";
  if (isVirtualReg(op.reg)) {
    return "%v" + std::to_string(op.reg - FIRST_VREG)
        + (op.regClass == RegClass::FPR ? "f" : "");
  }
  return op.regClass == RegClass::FPR ? fprName(op.reg) : gprName(op.reg);
}

void emitOperand(std::ostream& out, const MOperand& op) {
  switch (op.kind) {
    case MOperand::Kind::Reg:
      out << regNameOf(op);
      break;
    case MOperand::Kind::Imm:
      out << op.imm;
      break;
    case MOperand::Kind::FImm:
      out << op.fimm;
      break;
    case MOperand::Kind::MBB:
      out << (op.mbb ? op.mbb->label() : "<null-mbb>");
      break;
    case MOperand::Kind::Symbol:
      out << op.sym;
      break;
    case MOperand::Kind::FrameIdx:
      out << "<fi" << op.frameIdx << ">";
      break;
    case MOperand::Kind::None:
      out << "<none>";
      break;
  }
}

bool opUsesMemoryOffset(MIOp op) {
  // IR 中的加载和存储输出为 `MNEMONIC dst, imm(base)`。
  switch (op) {
    case MIOp::LB: case MIOp::LH: case MIOp::LW: case MIOp::LD:
    case MIOp::LBU: case MIOp::LHU: case MIOp::LWU:
    case MIOp::SB: case MIOp::SH: case MIOp::SW: case MIOp::SD:
    case MIOp::FLW: case MIOp::FSW:
      return true;
    default:
      return false;
  }
}

bool isZeroInitialized(const core_ir::GlobalVar& gv) {
  if (!gv.hasInit() || gv.initData.empty())
    return true;
  for (int32_t word : gv.initData) {
    if (word != 0)
      return false;
  }
  return true;
}

void emitInstruction(std::ostream& out, const MachineInst& inst) {
  MIOp op = inst.opcode();

  // 跳过发射前应已降低的虚拟伪指令（PHI / PHI_COPY / PROLOGUE / EPILOGUE）。
  // 如果它们仍然存在，则以注释形式输出，便于调试。
  if (op == MIOp::PHI || op == MIOp::PHI_COPY ||
      op == MIOp::PROLOGUE || op == MIOp::EPILOGUE ||
      op == MIOp::SPILL || op == MIOp::RELOAD)
    return;

  out << '\t' << mioName(op);
  if (op == MIOp::RET) {
    if (!inst.comment.empty()) out << "\t# " << inst.comment;
    out << '\n';
    return;
  }
  if (inst.numOperands() == 0) {
    out << '\n';
    return;
  }
  out << '\t';

  if (op == MIOp::JAL && inst.numOperands() >= 1) {
    emitOperand(out, inst.operand(0));
    if (!inst.comment.empty()) out << "\t# " << inst.comment;
    out << '\n';
    return;
  }

  if (opUsesMemoryOffset(op) && inst.numOperands() == 3) {
    // load:  dst, imm(base)
    // store: src, imm(base)
    emitOperand(out, inst.operand(0));
    out << ", ";
    emitOperand(out, inst.operand(1));
    out << "(";
    emitOperand(out, inst.operand(2));
    out << ")";
  } else {
    for (size_t i = 0; i < inst.numOperands(); ++i) {
      if (i) out << ", ";
      emitOperand(out, inst.operand(i));
    }
    // 浮点转整数按 SysY 语义使用向零舍入，不能使用默认的就近舍入模式。
    if (op == MIOp::FCVT_W_S) out << ", rtz";
  }

  if (!inst.comment.empty()) out << "\t# " << inst.comment;
  out << '\n';
}

void emitGlobal(std::ostream& out, const core_ir::GlobalVar& gv) {
  bool useBss = !gv.hasInit() || gv.initData.empty() || isZeroInitialized(gv);
  out << "\n";
  if (useBss) {
    out << "  .bss\n";
  } else if (gv.isConst()) {
    out << "  .section .rodata\n";
  } else {
    out << "  .data\n";
  }
  out << "  .globl " << gv.name() << "\n";
  out << "  .align 2\n";
  out << "  .type  " << gv.name() << ", @object\n";
  out << "  .size  " << gv.name() << ", " << gv.sizeBytes() << "\n";
  out << gv.name() << ":\n";
  if (useBss) {
    out << "  .zero " << gv.sizeBytes() << "\n";
    return;
  }
  int words = gv.sizeBytes() / 4;
  int emitted = 0;
  for (int i = 0; i < words; ++i) {
    int32_t word = i < static_cast<int>(gv.initData.size()) ? gv.initData[i] : 0;
    out << "  .word " << word << "\n";
    ++emitted;
  }
  int remaining = gv.sizeBytes() - emitted * 4;
  if (remaining > 0)
    out << "  .zero " << remaining << "\n";
}

void emitFloatPool(std::ostream& out, const MachineModule& mm) {
  if (mm.floatConsts().empty()) return;
  out << "\n  .section .rodata\n";
  out << "  .align 2\n";
  int idx = 0;
  for (float v : mm.floatConsts()) {
    uint32_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    out << ".LFC" << idx << ":\n  .word " << bits << "\n";
    ++idx;
  }
}

void emitFunction(std::ostream& out, const MachineFunction& fn) {
  out << "\n  .globl " << fn.name() << "\n";
  out << "  .type  " << fn.name() << ", @function\n";
  out << fn.name() << ":\n";
  for (const auto& bb : fn.blocks()) {
    out << bb->label() << ":\n";
    for (const auto& inst : bb->insts()) {
      emitInstruction(out, *inst);
    }
  }
  out << "\t.size " << fn.name() << ", .-" << fn.name() << "\n";
}

} // namespace

void printAsm(std::ostream& out,
              const MachineModule& mm,
              const core_ir::Module& coreModule) {
  // 不发射硬编码的 .file "program.sy"，避免把固定源文件元数据写入汇编。

  // 输出全局变量。
  for (const auto& g : coreModule.globals()) {
    emitGlobal(out, *g);
  }

  // 输出函数。
  out << "\n  .text\n";
  for (const auto& fn : mm.functions()) {
    emitFunction(out, *fn);
  }

  // 输出浮点常量池。
  emitFloatPool(out, mm);
}

} // namespace mir
