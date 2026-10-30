// 文件职责：定义机器函数、机器块、机器指令和操作数等 MIR 数据结构。本文件负责声明接口和数据结构。
#pragma once

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace core_ir { class Module; }

namespace mir {

// ============================================================
// 物理寄存器编码
//
// 物理寄存器和虚拟寄存器共用一个有符号整数编号空间。寄存器分配后所有操作数
// 都解析为物理寄存器，寄存器分配前则由虚拟寄存器承载值。
//
// 布局：
//   0..31    -> 物理 GPR x0..x31
//   32..63   -> 物理 FPR f0..f31
//   >= 64    -> 虚拟寄存器编号
//   -1       -> 尚未分配或无效
// ============================================================
namespace gpr {
constexpr int x0  = 0,  ra  = 1,  sp  = 2,  gp  = 3,  tp  = 4;
constexpr int t0  = 5,  t1  = 6,  t2  = 7;
constexpr int s0  = 8,  s1  = 9;
constexpr int a0  = 10, a1  = 11, a2  = 12, a3  = 13, a4  = 14, a5  = 15, a6  = 16, a7  = 17;
constexpr int s2  = 18, s3  = 19, s4  = 20, s5  = 21, s6  = 22, s7  = 23, s8  = 24, s9  = 25, s10 = 26, s11 = 27;
constexpr int t3  = 28, t4  = 29, t5  = 30, t6  = 31;
} // namespace gpr

namespace fpr {
constexpr int ft0 = 32, ft1 = 33, ft2 = 34, ft3 = 35, ft4 = 36, ft5 = 37, ft6 = 38, ft7 = 39;
constexpr int fs0 = 40, fs1 = 41;
constexpr int fa0 = 42, fa1 = 43, fa2 = 44, fa3 = 45, fa4 = 46, fa5 = 47, fa6 = 48, fa7 = 49;
constexpr int fs2 = 50, fs3 = 51, fs4 = 52, fs5 = 53, fs6 = 54, fs7 = 55, fs8 = 56, fs9 = 57, fs10 = 58, fs11 = 59;
constexpr int ft8 = 60, ft9 = 61, ft10 = 62, ft11 = 63;
} // namespace fpr

constexpr int FIRST_VREG = 64;

inline bool isVirtualReg(int r) { return r >= FIRST_VREG; }
inline bool isPhysReg(int r) { return r >= 0 && r < FIRST_VREG; }
inline bool isGPR(int r) { return r >= 0 && r < 32; }
inline bool isFPR(int r) { return r >= 32 && r < 64; }

const char* gprName(int r);
const char* fprName(int r);
const char* physRegName(int r); // 根据寄存器类别分派到 gprName/fprName

enum class RegClass { GPR, FPR };

// ============================================================
// 操作码。伪指令在寄存器分配和调度前存在，之后再展开。
// ============================================================
enum class MIOp {
  // 伪指令（最终发射前处理）
  PHI,           // SSA phi，寄存器分配前降低为输入边上的 PHI_COPY
  PHI_COPY,      // CFG 边上的并行复制：dst = src
  MV,            // 伪移动指令，支持任意寄存器类别
  LI,            // 加载立即数
  LA,            // 加载全局符号地址
  SPILL,         // 将虚拟寄存器溢出到栈槽
  RELOAD,        // 从栈槽重新加载虚拟寄存器
  PROLOGUE,      // 函数序言标记，寄存器分配确定保存集合后降低
  EPILOGUE,      // 函数尾声标记

  // ---- 整数 R 型 ----
  ADD, ADDW, SUB, SUBW,
  MUL, MULW, MULH, DIV, DIVW, DIVU, DIVUW, REM, REMW, REMU, REMUW,
  AND_, OR_, XOR_,
  SLL, SLLW, SRL, SRLW, SRA, SRAW,
  SLT, SLTU,

  // ---- 整数 I 型 ----
  ADDI, ADDIW, ANDI, ORI, XORI,
  SLLI, SLLIW, SRLI, SRLIW, SRAI, SRAIW,
  SLTI, SLTIU,

  // ---- 加载和存储 ----
  LB, LH, LW, LD, LBU, LHU, LWU,
  SB, SH, SW, SD,

  // ---- 浮点（单精度，SysY 不包含 f64）----
  FLW, FSW,
  FADD_S, FSUB_S, FMUL_S, FDIV_S,
  FSQRT_S, FNEG_S, FABS_S,
  FMADD_S, FMSUB_S, FNMADD_S, FNMSUB_S, // 融合乘加（a*b ± c）
  FEQ_S, FLT_S, FLE_S,
  FCVT_S_W, FCVT_W_S, // 有符号整数与浮点互转
  FMV_W_X,            // 将 GPR 中的原始 32 位值移动到 FPR
  FMV_X_W,            // 将 FPR 中的原始 32 位值移动到 GPR
  FMV_S,              // 浮点寄存器之间移动

  // ---- 分支和跳转 ----
  BEQ, BNE, BLT, BGE, BLTU, BGEU,
  BEQZ, BNEZ, BLTZ, BGEZ, BLEZ, BGTZ, // 伪指令，与 x0 比较
  J,            // 伪指令：jal x0, label
  JAL,          // jal ra, label，直接调用
  JALR,         // jalr ra, 0(rs)，间接调用
  RET,          // 伪指令：jalr x0, 0(ra)

  // ---- 系统指令 ----
  ECALL,
  NOP,

  // 枚举结束标记。
  COUNT,
};

const char* mioName(MIOp op);
bool mioIsTerminator(MIOp op);
bool mioMayLoad(MIOp op);
bool mioMayStore(MIOp op);
bool mioIsCall(MIOp op);
bool mioIsBranch(MIOp op);           // 条件分支（例如 BEQ）
bool mioWritesGPR(MIOp op);          // 结果寄存器类别
bool mioWritesFPR(MIOp op);

// ============================================================
// 操作数。MachineInst 持有少量 MOperand。
// ============================================================
struct MOperand {
  enum class Kind {
    None,
    Reg,        // 虚拟寄存器或物理寄存器
    Imm,        // 64 位有符号立即数
    FImm,       // 浮点立即数，用于值物化伪指令
    MBB,        // 基本块标签目标
    Symbol,     // 全局符号或函数名
    FrameIdx,   // 栈槽索引，在序言和尾声阶段解析
  };

  Kind kind = Kind::None;
  // 寄存器操作数：保存虚拟寄存器编号或寄存器分配后的物理寄存器编号。
  int reg = -1;
  bool isDef = false;
  bool isKill = false;       // 最后一次使用，由活跃性分析设置
  RegClass regClass = RegClass::GPR;

  // 立即数操作数。
  int64_t imm = 0;
  // 浮点立即数，用于值物化，稍后降低为内存池引用。
  float fimm = 0.0f;

  // 标签目标。
  class MachineBasicBlock* mbb = nullptr;
  // 符号或全局名称。
  std::string sym;
  // 栈帧索引，最终根据 sp 解析为正负偏移。
  int frameIdx = 0;

  static MOperand Reg(int r, RegClass cls, bool def);
  static MOperand Use(int r, RegClass cls);
  static MOperand Def(int r, RegClass cls);
  static MOperand Imm(int64_t v);
  static MOperand FImm(float v);
  static MOperand BB(MachineBasicBlock* b);
  static MOperand Sym(const std::string& s);
  static MOperand Frame(int idx);

  bool isReg() const { return kind == Kind::Reg; }
  bool isImm() const { return kind == Kind::Imm; }
  bool isMBB() const { return kind == Kind::MBB; }
  bool isSym() const { return kind == Kind::Symbol; }
  bool isFrame() const { return kind == Kind::FrameIdx; }
  bool isFImm() const { return kind == Kind::FImm; }
};

class MachineFunction;

// ============================================================
// 机器指令。
// ============================================================
class MachineInst {
public:
  MachineInst(MIOp op) : op_(op) {}

  MIOp opcode() const { return op_; }
  void setOpcode(MIOp op) { op_ = op; }

  std::vector<MOperand>& operands() { return ops_; }
  const std::vector<MOperand>& operands() const { return ops_; }
  MOperand& operand(size_t i) { return ops_[i]; }
  const MOperand& operand(size_t i) const { return ops_[i]; }
  size_t numOperands() const { return ops_.size(); }
  void addOperand(MOperand o) { ops_.push_back(std::move(o)); }

  // 便捷接口：收集所有寄存器定义和使用。
  void collectDefs(std::vector<int>& out) const;
  void collectUses(std::vector<int>& out) const;
  // PHI 伪指令使用（输入寄存器虚拟编号、输入 MBB）成对保存。
  // operand(0) 是定义虚拟寄存器，之后每个前驱对应一组（reg，mbb）。

  MachineBasicBlock* parent() const { return parent_; }
  void setParent(MachineBasicBlock* b) { parent_ = b; }

  // 基本块内位置，由线性化阶段为活跃性和调度分配。
  int pos = -1;

  // 发射指令时追加到指令末尾的自由格式说明。
  std::string comment;

private:
  MIOp op_;
  std::vector<MOperand> ops_;
  MachineBasicBlock* parent_ = nullptr;
};

// ============================================================
// 机器基本块。
// ============================================================
class MachineBasicBlock {
public:
  MachineBasicBlock(MachineFunction* func, std::string label);

  MachineFunction* parent() const { return parent_; }
  const std::string& label() const { return label_; }
  int id() const { return id_; }
  void setId(int i) { id_ = i; }

  std::vector<std::unique_ptr<MachineInst>>& insts() { return insts_; }
  const std::vector<std::unique_ptr<MachineInst>>& insts() const { return insts_; }
  bool empty() const { return insts_.empty(); }
  MachineInst* terminator() const;

  MachineInst* append(std::unique_ptr<MachineInst> inst);
  MachineInst* prepend(std::unique_ptr<MachineInst> inst);
  void insertBefore(MachineInst* pos, std::unique_ptr<MachineInst> inst);

  // CFG 关系。
  std::vector<MachineBasicBlock*>& preds() { return preds_; }
  std::vector<MachineBasicBlock*>& succs() { return succs_; }
  const std::vector<MachineBasicBlock*>& preds() const { return preds_; }
  const std::vector<MachineBasicBlock*>& succs() const { return succs_; }

private:
  MachineFunction* parent_;
  std::string label_;
  int id_ = -1;
  std::vector<std::unique_ptr<MachineInst>> insts_;
  std::vector<MachineBasicBlock*> preds_;
  std::vector<MachineBasicBlock*> succs_;
};

// ============================================================
// 栈帧描述。在降低和寄存器分配阶段逐步构建，在序言和尾声降低前确定最终布局。
// ============================================================
struct FrameObject {
  int index;       // MOperand 引用的 FrameIdx
  int size;        // 字节数
  int align;       // 对齐字节数
  int offset = 0;  // 相对 sp 的偏移，在最终布局阶段填写
  bool isSpill = false;
  bool isArg = false;       // 输入参数溢出槽
  std::string name;
};

// ============================================================
// 虚拟寄存器信息。
// ============================================================
struct VRegInfo {
  int id;
  RegClass cls;
  int physReg = -1;       // 由寄存器分配设置，溢出或分配前为 -1
  int spillSlot = -1;     // 溢出时对应的 FrameIdx
  bool crossesCall = false;
  int loopDepth = 0;
  int hotLoopWeight = 0;
  bool preferCallerSaved = false;
  bool rematerializable = false;
};

// ============================================================
// 机器函数。
// ============================================================
class MachineFunction {
public:
  explicit MachineFunction(std::string name);

  const std::string& name() const { return name_; }
  void setName(std::string n) { name_ = std::move(n); }

  // 基本块。
  MachineBasicBlock* newBlock(const std::string& label = "");
  MachineBasicBlock* entryBlock() const { return blocks_.empty() ? nullptr : blocks_.front().get(); }
  std::vector<std::unique_ptr<MachineBasicBlock>>& blocks() { return blocks_; }
  const std::vector<std::unique_ptr<MachineBasicBlock>>& blocks() const { return blocks_; }

  // 虚拟寄存器。
  int newVReg(RegClass cls);
  VRegInfo& vregInfo(int v);
  const VRegInfo& vregInfo(int v) const;
  size_t numVRegs() const { return vregs_.size(); }
  const std::vector<VRegInfo>& vregs() const { return vregs_; }

  // 栈帧对象。
  int newFrameObject(int size, int align, bool isSpill = false, const std::string& name = "");
  std::vector<FrameObject>& frame() { return frame_; }
  const std::vector<FrameObject>& frame() const { return frame_; }

  // 调用约定信息，在降低结束或寄存器分配开始时计算。
  bool hasCalls = false;
  int incomingIntArgs = 0;     // 函数接收的整数参数数量（寄存器最多 8 个）
  int incomingFloatArgs = 0;   // 浮点参数数量（寄存器最多 8 个）
  int incomingStackArgBytes = 0;
  int outgoingArgBytes = 0;    // 超过 8 个参数时为调用方参数预留的字节数

  // 寄存器分配后设置。
  std::vector<int> usedCalleeSavedGPR; // gpr 命名空间中的物理寄存器编号
  std::vector<int> usedCalleeSavedFPR;
  int frameSize = 16; // 字节数，在栈帧降低阶段最终确定

private:
  std::string name_;
  std::vector<std::unique_ptr<MachineBasicBlock>> blocks_;
  std::vector<VRegInfo> vregs_;       // 索引 0 不使用（FIRST_VREG=64，保存时减去 64）
  std::vector<FrameObject> frame_;
  int nextLabel_ = 0;
};

// ============================================================
// 机器模块。对应 core_ir::Module 的函数集合和全局信息。
// 这里不拥有全局变量，打印器从 CoreIR 模块读取它们。
// ============================================================
class MachineModule {
public:
  MachineFunction* newFunction(const std::string& name);
  std::vector<std::unique_ptr<MachineFunction>>& functions() { return funcs_; }
  const std::vector<std::unique_ptr<MachineFunction>>& functions() const { return funcs_; }

  // 需要放入 .rodata 常量池的浮点常量。降低阶段收集它们，打印器发射 .word 项。
  int internFloatConst(float v);
  const std::vector<float>& floatConsts() const { return floatConsts_; }

private:
  std::vector<std::unique_ptr<MachineFunction>> funcs_;
  std::vector<float> floatConsts_;
  std::unordered_map<uint32_t, int> floatConstMap_;
};

// ============================================================
// 汇编打印器。输出完整的 .s 文件，包括文件头、全局变量、函数和浮点常量池。
// coreModule 用于提供全局变量定义，MIR 层不拥有全局变量。
// ============================================================
void printAsm(std::ostream& out,
              const MachineModule& mm,
              const core_ir::Module& coreModule);

} // namespace mir
