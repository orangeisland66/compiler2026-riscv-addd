// 文件职责：将 CoreIR 指令降低为机器级 MIR 指令。本文件负责实现具体逻辑。
#include "lower.h"

#include "core_ir.h"
#include "../research_switch.h"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mir {

namespace {

using core_ir::BasicBlock;
using core_ir::ConstFloat;
using core_ir::ConstInt;
using core_ir::Function;
using core_ir::GlobalVar;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Type;
using core_ir::Value;

constexpr int kAReg[8] = {gpr::a0, gpr::a1, gpr::a2, gpr::a3, gpr::a4, gpr::a5, gpr::a6, gpr::a7};
constexpr int kFaReg[8] = {fpr::fa0, fpr::fa1, fpr::fa2, fpr::fa3, fpr::fa4, fpr::fa5, fpr::fa6, fpr::fa7};
constexpr int kCallerSavedGPR[] = {
    gpr::ra, gpr::t0, gpr::t1, gpr::t2, gpr::a0, gpr::a1, gpr::a2, gpr::a3,
    gpr::a4, gpr::a5, gpr::a6, gpr::a7, gpr::t3, gpr::t4, gpr::t5, gpr::t6,
};
constexpr int kCallerSavedFPR[] = {
    fpr::ft0, fpr::ft1, fpr::ft2, fpr::ft3, fpr::ft4, fpr::ft5, fpr::ft6, fpr::ft7,
    fpr::fa0, fpr::fa1, fpr::fa2, fpr::fa3, fpr::fa4, fpr::fa5, fpr::fa6, fpr::fa7,
    fpr::ft8, fpr::ft9, fpr::ft10, fpr::ft11,
};

bool fitsSigned12(int64_t v) { return v >= -2048 && v <= 2047; }

int32_t wrapI32(int64_t value) {
  return static_cast<int32_t>(static_cast<uint32_t>(value));
}

struct SignedDivMagicInfo {
  int32_t magic = 0;
  int shift = 0;
  bool valid = false;
};

SignedDivMagicInfo computeSignedDivMagic(int32_t divisor) {
  SignedDivMagicInfo info;
  if (divisor == 0 || divisor == 1 || divisor == -1)
    return info;

  constexpr int bits = 32;
  const int64_t signedMin = 1LL << (bits - 1);
  const int64_t absDivisor =
      divisor < 0 ? -static_cast<int64_t>(divisor)
                  : static_cast<int64_t>(divisor);
  const int64_t t = signedMin + (divisor < 0 ? 1 : 0);
  const int64_t anc = t - 1 - (t % absDivisor);

  int p = bits - 1;
  int64_t q1 = signedMin / anc;
  int64_t r1 = signedMin - q1 * anc;
  int64_t q2 = signedMin / absDivisor;
  int64_t r2 = signedMin - q2 * absDivisor;

  while (true) {
    ++p;
    q1 <<= 1;
    r1 <<= 1;
    if (r1 >= anc) {
      ++q1;
      r1 -= anc;
    }

    q2 <<= 1;
    r2 <<= 1;
    if (r2 >= absDivisor) {
      ++q2;
      r2 -= absDivisor;
    }

    const int64_t delta = absDivisor - r2;
    if (!(q1 < delta || (q1 == delta && r1 == 0)))
      break;
  }

  int64_t magic = q2 + 1;
  if (divisor < 0)
    magic = -magic;
  info.magic = wrapI32(magic);
  info.shift = p - bits;
  info.valid = true;
  return info;
}

uint32_t floatBits(float value) {
  uint32_t bits = 0;
  std::memcpy(&bits, &value, sizeof(bits));
  return bits;
}

std::string condFromName(const Instruction* inst) {
  const std::string& n = inst->name();
  auto pos = n.find('_');
  return pos == std::string::npos ? n : n.substr(0, pos);
}

bool isKnownI32Boolean(Value* value, int depth = 0) {
  if (!value || depth > 12)
    return false;
  if (auto* constant = dynamic_cast<ConstInt*>(value))
    return constant->value() == 0 || constant->value() == 1;
  auto* instruction = dynamic_cast<Instruction*>(value);
  if (!instruction || !instruction->type().isI32())
    return false;
  if (instruction->opcode() == Opcode::ICmp)
    return instruction->numOperands() == 2;
  if (instruction->opcode() == Opcode::And && instruction->numOperands() == 2) {
    auto* lhs = dynamic_cast<ConstInt*>(instruction->operand(0));
    auto* rhs = dynamic_cast<ConstInt*>(instruction->operand(1));
    if ((lhs && lhs->value() == 1) || (rhs && rhs->value() == 1))
      return true;
  }
  if ((instruction->opcode() != Opcode::And &&
       instruction->opcode() != Opcode::Or &&
       instruction->opcode() != Opcode::Xor) ||
      instruction->numOperands() != 2) {
    return false;
  }
  return isKnownI32Boolean(instruction->operand(0), depth + 1) &&
         isKnownI32Boolean(instruction->operand(1), depth + 1);
}

// 识别同一整数值的非负下界和正数常量上界。
bool matchNonNegativeUpperBound(core_ir::Value* first,
                                core_ir::Value* second,
                                core_ir::Value*& valueOut,
                                core_ir::Value*& boundOut) {
  auto* nonNegative = dynamic_cast<Instruction*>(first);
  auto* belowBound = dynamic_cast<Instruction*>(second);
  if (!nonNegative || !belowBound ||
      nonNegative->opcode() != Opcode::ICmp ||
      belowBound->opcode() != Opcode::ICmp ||
      nonNegative->numOperands() != 2 || belowBound->numOperands() != 2) {
    return false;
  }

  Value* value = nullptr;
  const std::string nonNegativeCond = condFromName(nonNegative);
  if (nonNegativeCond == "ge") {
    auto* zero = dynamic_cast<ConstInt*>(nonNegative->operand(1));
    if (zero && zero->value() == 0)
      value = nonNegative->operand(0);
  } else if (nonNegativeCond == "le") {
    auto* zero = dynamic_cast<ConstInt*>(nonNegative->operand(0));
    if (zero && zero->value() == 0)
      value = nonNegative->operand(1);
  }
  if (!value || !value->type().isI32())
    return false;

  Value* bound = nullptr;
  const std::string upperCond = condFromName(belowBound);
  if (upperCond == "lt" && belowBound->operand(0) == value) {
    bound = belowBound->operand(1);
  } else if (upperCond == "gt" && belowBound->operand(1) == value) {
    bound = belowBound->operand(0);
  }
  auto* boundConst = dynamic_cast<ConstInt*>(bound);
  if (!boundConst || boundConst->value() <= 0)
    return false;

  valueOut = value;
  boundOut = bound;
  return true;
}

bool matchNonNegativeUpperBoundAnd(Instruction* inst,
                                   core_ir::Value*& valueOut,
                                   core_ir::Value*& boundOut) {
  if (!inst || inst->opcode() != Opcode::And || inst->numOperands() != 2)
    return false;
  return matchNonNegativeUpperBound(inst->operand(0), inst->operand(1),
                                    valueOut, boundOut) ||
         matchNonNegativeUpperBound(inst->operand(1), inst->operand(0),
                                    valueOut, boundOut);
}

RegClass classFor(Type ty) {
  return ty.isF32() ? RegClass::FPR : RegClass::GPR;
}

bool isDirectCondBrCompare(const Instruction* inst) {
  if (!inst ||
      (inst->opcode() != Opcode::ICmp && inst->opcode() != Opcode::FCmp) ||
      inst->uses().size() != 1) {
    return false;
  }
  const Instruction* user = inst->uses().front();
  return user &&
         user->opcode() == Opcode::CondBr &&
         user->numOperands() >= 1 &&
         user->operand(0) == inst;
}

bool splitConstantPtrOffset(Value* val, Value*& base, int& offset) {
  int64_t totalOffset = 0;
  Value* cur = val;
  bool changed = false;
  while (auto* inst = dynamic_cast<Instruction*>(cur)) {
    if (inst->opcode() == Opcode::BitCast && inst->numOperands() == 1) {
      cur = inst->operand(0);
      changed = true;
      continue;
    }
    if (inst->opcode() != Opcode::GEP || inst->numOperands() < 1) break;

    int64_t localOffset = 0;
    bool allConst = true;
    bool sawConst = false;
    for (int i = 1; i < inst->numOperands(); ++i) {
      auto* ci = dynamic_cast<ConstInt*>(inst->operand(i));
      if (!ci) {
        allConst = false;
        continue;
      }
      sawConst = true;
      localOffset += ci->value();
    }
    if (!sawConst) break;
    totalOffset += localOffset;
    changed = true;
    if (!allConst) {
      base = cur;
      if (totalOffset < std::numeric_limits<int>::min() ||
          totalOffset > std::numeric_limits<int>::max()) {
        return false;
      }
      offset = static_cast<int>(totalOffset);
      return true;
    }
    cur = inst->operand(0);
  }

  if (totalOffset < std::numeric_limits<int>::min() ||
      totalOffset > std::numeric_limits<int>::max()) {
    return false;
  }
  base = cur;
  offset = static_cast<int>(totalOffset);
  return changed || base == val;
}

enum class ABIArgLocKind { FloatReg, IntReg, Stack };

struct ABIArgLoc {
  ABIArgLocKind kind = ABIArgLocKind::Stack;
  int index = -1;
  bool isFloat = false;
};

struct ABIArgLayout {
  std::vector<ABIArgLoc> locs;
  int intRegsUsed = 0;
  int floatRegsUsed = 0;
  int stackSlotsUsed = 0;
};

ABIArgLayout assignABIArgLayout(const std::vector<Type>& argTypes) {
  ABIArgLayout layout;
  layout.locs.resize(argTypes.size());

  int intReg = 0;
  int floatReg = 0;
  int stackSlot = 0;
  for (size_t i = 0; i < argTypes.size(); ++i) {
    ABIArgLoc loc;
    loc.isFloat = argTypes[i].isF32();
    if (loc.isFloat) {
      if (floatReg < 8) {
        loc.kind = ABIArgLocKind::FloatReg;
        loc.index = floatReg++;
      } else if (intReg < 8) {
        loc.kind = ABIArgLocKind::IntReg;
        loc.index = intReg++;
      } else {
        loc.kind = ABIArgLocKind::Stack;
        loc.index = stackSlot++;
      }
    } else {
      if (intReg < 8) {
        loc.kind = ABIArgLocKind::IntReg;
        loc.index = intReg++;
      } else {
        loc.kind = ABIArgLocKind::Stack;
        loc.index = stackSlot++;
      }
    }
    layout.locs[i] = loc;
  }

  layout.intRegsUsed = intReg;
  layout.floatRegsUsed = floatReg;
  layout.stackSlotsUsed = stackSlot;
  return layout;
}

void addCallArgUses(MachineInst& call, const ABIArgLayout& layout) {
  for (const ABIArgLoc& loc : layout.locs) {
    if (loc.kind == ABIArgLocKind::FloatReg) {
      call.addOperand(MOperand::Use(kFaReg[loc.index], RegClass::FPR));
      continue;
    }
    if (loc.kind == ABIArgLocKind::IntReg) {
      call.addOperand(MOperand::Use(kAReg[loc.index], RegClass::GPR));
    }
  }
}

void addCallClobberDefs(MachineInst& call) {
  for (int reg : kCallerSavedGPR) {
    call.addOperand(MOperand::Def(reg, RegClass::GPR));
  }
  for (int reg : kCallerSavedFPR) {
    call.addOperand(MOperand::Def(reg, RegClass::FPR));
  }
}

class Lowering {
public:
  Lowering(const Module& mod, MachineModule& mm) : mod_(mod), mm_(mm) {}

  void run() {
    // 第一遍先声明所有函数，使调用可以解析前向引用。
    for (const auto& fn : mod_.functions()) {
      mfMap_[fn.get()] = mm_.newFunction(fn->name());
    }
    for (const auto& fn : mod_.functions()) {
      lowerFunction(fn.get());
    }
  }

private:
  // --------------------------------------------------------------
  // 状态
  // --------------------------------------------------------------
  const Module& mod_;
  MachineModule& mm_;
  std::unordered_map<const Function*, MachineFunction*> mfMap_;

  // 当前函数状态。
  Function* curFunc_ = nullptr;
  MachineFunction* curMF_ = nullptr;
  const BasicBlock* curCoreBB_ = nullptr;
  std::unordered_map<const BasicBlock*, MachineBasicBlock*> bbMap_;
  std::unordered_map<const Value*, int> valueVReg_; // Value* -> 虚拟寄存器编号
  std::unordered_map<const Instruction*, int> allocaFrame_; // Alloca -> 栈帧索引
  MachineBasicBlock* curMBB_ = nullptr;
  std::unordered_map<const Value*, int> blockAddrVReg_;
  std::unordered_map<const Value*, int> hoistedAddrVReg_;
  std::unordered_map<uint32_t, int> floatConstVReg_;
  std::unordered_map<int32_t, int> invariantIntConstVReg_;

  bool isInLoop(const BasicBlock* block) const {
    if (!block)
      return false;
    std::vector<const BasicBlock*> work(block->succs().begin(),
                                        block->succs().end());
    std::unordered_set<const BasicBlock*> seen;
    while (!work.empty()) {
      const BasicBlock* current = work.back();
      work.pop_back();
      if (!current || !seen.insert(current).second)
        continue;
      if (current == block)
        return true;
      for (const BasicBlock* successor : current->succs())
        work.push_back(successor);
    }
    return false;
  }

  int newTempVReg(RegClass cls, bool rematerializable = false) {
    int vreg = curMF_->newVReg(cls);
    curMF_->vregInfo(vreg).rematerializable = rematerializable;
    return vreg;
  }

  BasicBlock* nextBlockInLayout(const BasicBlock* bb) const {
    if (!bb) return nullptr;
    const auto& blocks = curFunc_->blocks();
    for (size_t i = 0; i < blocks.size(); ++i) {
      if (blocks[i].get() != bb) continue;
      return i + 1 < blocks.size() ? blocks[i + 1].get() : nullptr;
    }
    return nullptr;
  }

  static MIOp loadOpForType(Type ty) {
    if (ty.isF32()) return MIOp::FLW;
    if (ty.isPtr()) return MIOp::LD;
    return MIOp::LW;
  }

  static MIOp storeOpForType(Type ty) {
    if (ty.isF32()) return MIOp::FSW;
    if (ty.isPtr()) return MIOp::SD;
    return MIOp::SW;
  }

  static std::string invertICmpCond(const std::string& cond) {
    if (cond == "eq") return "ne";
    if (cond == "ne") return "eq";
    if (cond == "lt") return "ge";
    if (cond == "le") return "gt";
    if (cond == "gt") return "le";
    if (cond == "ge") return "lt";
    throw std::runtime_error("unknown icmp condition '" + cond + "'");
  }

  void appendLoadImmToGPR(int dstReg, int64_t imm) {
    auto mi = std::make_unique<MachineInst>(MIOp::LI);
    mi->addOperand(MOperand::Def(dstReg, RegClass::GPR));
    mi->addOperand(MOperand::Imm(imm));
    curMBB_->append(std::move(mi));
  }

  void appendLoadGlobalAddrToGPR(int dstReg, const std::string& sym) {
    auto mi = std::make_unique<MachineInst>(MIOp::LA);
    mi->addOperand(MOperand::Def(dstReg, RegClass::GPR));
    mi->addOperand(MOperand::Sym(sym));
    curMBB_->append(std::move(mi));
  }

  void appendAddrAdd(int dstReg, int baseReg, int64_t imm) {
    if (imm == 0 && dstReg == baseReg) return;
    if (fitsSigned12(imm)) {
      auto mi = std::make_unique<MachineInst>(MIOp::ADDI);
      mi->addOperand(MOperand::Def(dstReg, RegClass::GPR));
      mi->addOperand(MOperand::Use(baseReg, RegClass::GPR));
      mi->addOperand(MOperand::Imm(imm));
      curMBB_->append(std::move(mi));
      return;
    }
    if (imm >= -4096 && imm <= 4094) {
      const int64_t firstImm = imm > 2047 ? imm - 2047 : -2048;
      const int64_t secondImm = imm - firstImm;
      auto appendAddi = [&](int dst, int base, int64_t amount) {
        auto mi = std::make_unique<MachineInst>(MIOp::ADDI);
        mi->addOperand(MOperand::Def(dst, RegClass::GPR));
        mi->addOperand(MOperand::Use(base, RegClass::GPR));
        mi->addOperand(MOperand::Imm(amount));
        curMBB_->append(std::move(mi));
      };
      appendAddi(dstReg, baseReg, firstImm);
      appendAddi(dstReg, dstReg, secondImm);
      return;
    }
    int immReg = newTempVReg(RegClass::GPR, /*rematerializable=*/true);
    auto loadImm = std::make_unique<MachineInst>(MIOp::LI);
    loadImm->addOperand(MOperand::Def(immReg, RegClass::GPR));
    loadImm->addOperand(MOperand::Imm(imm));
    curMBB_->append(std::move(loadImm));
    auto mi = std::make_unique<MachineInst>(MIOp::ADD);
    mi->addOperand(MOperand::Def(dstReg, RegClass::GPR));
    mi->addOperand(MOperand::Use(baseReg, RegClass::GPR));
    mi->addOperand(MOperand::Use(immReg, RegClass::GPR));
    curMBB_->append(std::move(mi));
  }

  int materializeIntConstant(int64_t value) {
    int reg = newTempVReg(RegClass::GPR, /*rematerializable=*/true);
    appendLoadImmToGPR(reg, value);
    return reg;
  }

  int appendBinaryRR(MIOp op, int lhsReg, int rhsReg,
                     const char* comment = nullptr) {
    int dstReg = newTempVReg(RegClass::GPR);
    auto mi = std::make_unique<MachineInst>(op);
    mi->addOperand(MOperand::Def(dstReg, RegClass::GPR));
    mi->addOperand(MOperand::Use(lhsReg, RegClass::GPR));
    mi->addOperand(MOperand::Use(rhsReg, RegClass::GPR));
    if (comment)
      mi->comment = comment;
    curMBB_->append(std::move(mi));
    return dstReg;
  }

  int appendBinaryRI(MIOp op, int lhsReg, int64_t imm,
                     const char* comment = nullptr) {
    int dstReg = newTempVReg(RegClass::GPR);
    auto mi = std::make_unique<MachineInst>(op);
    mi->addOperand(MOperand::Def(dstReg, RegClass::GPR));
    mi->addOperand(MOperand::Use(lhsReg, RegClass::GPR));
    mi->addOperand(MOperand::Imm(imm));
    if (comment)
      mi->comment = comment;
    curMBB_->append(std::move(mi));
    return dstReg;
  }

  int appendArithmeticShift64By32(int srcReg,
                                  const char* comment = nullptr) {
    int dstReg = newTempVReg(RegClass::GPR);
    auto mi = std::make_unique<MachineInst>(MIOp::SRAI);
    mi->addOperand(MOperand::Def(dstReg, RegClass::GPR));
    mi->addOperand(MOperand::Use(srcReg, RegClass::GPR));
    mi->addOperand(MOperand::Imm(32));
    if (comment)
      mi->comment = comment;
    curMBB_->append(std::move(mi));
    return dstReg;
  }

  bool tryLowerSignedDivRemConst(Instruction* inst, Value* lhs, int32_t divisor,
                                 int dstReg) {
    if (divisor == 0)
      return false;
    if (divisor == 1) {
      int lhsReg = materializeValue(lhs);
      appendRegMove(dstReg, RegClass::GPR, lhsReg, RegClass::GPR);
      return true;
    }
    if (divisor == -1) {
      int lhsReg = materializeValue(lhs);
      int zeroReg = materializeIntConstant(0);
      int negReg = appendBinaryRR(MIOp::SUBW, zeroReg, lhsReg,
                                  "const-div.neg");
      appendRegMove(dstReg, RegClass::GPR, negReg, RegClass::GPR);
      return true;
    }

    const uint32_t unsignedDivisor = static_cast<uint32_t>(divisor);
    if (!sycEnvironmentValue("SYC_SKIP_MIR_POWER2_DIVREM") && divisor > 1 &&
        (unsignedDivisor & (unsignedDivisor - 1)) == 0) {
      int shift = 0;
      uint32_t remaining = unsignedDivisor;
      while (remaining > 1) {
        ++shift;
        remaining >>= 1;
      }

      int lhsReg = materializeValue(lhs);
      int signMaskReg = appendBinaryRI(MIOp::SRAIW, lhsReg, 31,
                                       "const-div.pow2.sign");
      int32_t biasMask = divisor - 1;
      int biasReg = 0;
      if (fitsSigned12(biasMask)) {
        biasReg = appendBinaryRI(MIOp::ANDI, signMaskReg, biasMask,
                                 "const-div.pow2.bias");
      } else {
        int maskReg = materializeIntConstant(biasMask);
        biasReg = appendBinaryRR(MIOp::AND_, signMaskReg, maskReg,
                                 "const-div.pow2.bias");
      }
      int adjustedReg = appendBinaryRR(MIOp::ADDW, lhsReg, biasReg,
                                       "const-div.pow2.adjust");
      int quotientReg = appendBinaryRI(MIOp::SRAIW, adjustedReg, shift,
                                       "const-div.pow2.shift");
      if (inst->opcode() == Opcode::SDiv) {
        appendRegMove(dstReg, RegClass::GPR, quotientReg, RegClass::GPR);
        return true;
      }

      int productReg = appendBinaryRI(MIOp::SLLIW, quotientReg, shift,
                                      "const-rem.pow2.product");
      int remainderReg = appendBinaryRR(MIOp::SUBW, lhsReg, productReg,
                                        "const-rem.pow2.sub");
      appendRegMove(dstReg, RegClass::GPR, remainderReg, RegClass::GPR);
      return true;
    }

    SignedDivMagicInfo magic = computeSignedDivMagic(divisor);
    if (!magic.valid)
      return false;

    // 在当前目标模型中，REMW 的延迟明显高于整数乘法和移位序列。这里比较依赖
    // 路径延迟而不是伪指令数量，避免表面更短的 REMW 拖慢热点常量取模循环。
    const bool needsFixup =
        (divisor > 0 && magic.magic < 0) ||
        (divisor < 0 && magic.magic > 0);
    constexpr int kMulLatency = 3;
    constexpr int kRemLatency = 20;
    const int magicRemLatency =
        kMulLatency + 1 + (needsFixup ? 1 : 0) +
        (magic.shift > 0 ? 1 : 0) + 1 + 1 + kMulLatency + 1;
    auto materializeDivConst = [&](int32_t value) {
      // 整个函数范围的提升会让两个 magic 常量跨越嵌套循环，并可能增加被调用者
      // 保存寄存器的使用。等降低阶段具备循环前置块放置和压力数据后再扩大范围。
      if (sycResearchSwitchEnabled("SYC_ENABLE_MIR_INVARIANT_INT_CONST") &&
          curCoreBB_ && curCoreBB_ != curFunc_->entryBlock() &&
          isInLoop(curCoreBB_))
        return materializeCachedInvariantIntConst(value);
      return materializeIntConstant(value);
    };
    if (inst->opcode() == Opcode::SRem &&
        magicRemLatency >= kRemLatency) {
      int lhsReg = materializeValue(lhs);
      int divisorReg = materializeDivConst(divisor);
      int remReg = appendBinaryRR(MIOp::REMW, lhsReg, divisorReg,
                                  "const-rem.hardware");
      appendRegMove(dstReg, RegClass::GPR, remReg, RegClass::GPR);
      return true;
    }

    int lhsReg = materializeValue(lhs);
    int magicReg = materializeDivConst(magic.magic);
    int fullMulReg = appendBinaryRR(MIOp::MUL, lhsReg, magicReg,
                                    "const-div.fullmul");
    int qReg = appendArithmeticShift64By32(fullMulReg, "const-div.mulhi");
    if (divisor > 0 && magic.magic < 0) {
      qReg = appendBinaryRR(MIOp::ADDW, qReg, lhsReg, "const-div.fixup");
    } else if (divisor < 0 && magic.magic > 0) {
      qReg = appendBinaryRR(MIOp::SUBW, qReg, lhsReg, "const-div.fixup");
    }
    if (magic.shift > 0) {
      qReg = appendBinaryRI(MIOp::SRAIW, qReg, magic.shift,
                            "const-div.shift");
    }
    int signReg = appendBinaryRI(MIOp::SRLIW, qReg, 31, "const-div.sign");
    qReg = appendBinaryRR(MIOp::ADDW, qReg, signReg, "const-div.round");

    if (inst->opcode() == Opcode::SDiv) {
      appendRegMove(dstReg, RegClass::GPR, qReg, RegClass::GPR);
      return true;
    }

    int divReg = materializeDivConst(divisor);
    int prodReg = appendBinaryRR(MIOp::MULW, qReg, divReg, "const-rem.mul");
    int remReg = appendBinaryRR(MIOp::SUBW, lhsReg, prodReg, "const-rem.sub");
    appendRegMove(dstReg, RegClass::GPR, remReg, RegClass::GPR);
    return true;
  }

  bool tryLowerDirectConstAddr(Value* baseValue, int64_t extraOffset, int dstReg) {
    Value* rootBase = nullptr;
    int rootOffset = 0;
    if (!splitConstantPtrOffset(baseValue, rootBase, rootOffset)) {
      rootBase = baseValue;
      rootOffset = 0;
    }
    int64_t totalOffset = extraOffset + static_cast<int64_t>(rootOffset);

    if (auto* allocaInst = dynamic_cast<Instruction*>(rootBase)) {
      auto fiIt = allocaFrame_.find(allocaInst);
      if (fiIt != allocaFrame_.end()) {
        curMF_->vregInfo(dstReg).rematerializable = true;
        if (totalOffset == 0) {
          auto mi = std::make_unique<MachineInst>(MIOp::ADDI);
          mi->addOperand(MOperand::Def(dstReg, RegClass::GPR));
          mi->addOperand(MOperand::Frame(fiIt->second));
          mi->addOperand(MOperand::Imm(0));
          mi->comment = "gep.frame";
          curMBB_->append(std::move(mi));
          return true;
        }
        int baseReg = newTempVReg(RegClass::GPR, /*rematerializable=*/true);
        auto base = std::make_unique<MachineInst>(MIOp::ADDI);
        base->addOperand(MOperand::Def(baseReg, RegClass::GPR));
        base->addOperand(MOperand::Frame(fiIt->second));
        base->addOperand(MOperand::Imm(0));
        base->comment = "gep.frame.base";
        curMBB_->append(std::move(base));
        appendAddrAdd(dstReg, baseReg, totalOffset);
        return true;
      }
    }

    if (auto* gv = dynamic_cast<GlobalVar*>(rootBase)) {
      curMF_->vregInfo(dstReg).rematerializable = true;
      if (totalOffset == 0) {
        appendLoadGlobalAddrToGPR(dstReg, gv->name());
        return true;
      }
      int baseReg = newTempVReg(RegClass::GPR, /*rematerializable=*/true);
      appendLoadGlobalAddrToGPR(baseReg, gv->name());
      appendAddrAdd(dstReg, baseReg, totalOffset);
      return true;
    }

    return false;
  }

  void appendRegMove(int dstReg, RegClass dstCls, int srcReg, RegClass srcCls) {
    if (dstReg == srcReg && dstCls == srcCls)
      return;
    MIOp op;
    if (dstCls == RegClass::GPR && srcCls == RegClass::GPR) op = MIOp::MV;
    else if (dstCls == RegClass::FPR && srcCls == RegClass::FPR) op = MIOp::FMV_S;
    else if (dstCls == RegClass::FPR && srcCls == RegClass::GPR) op = MIOp::FMV_W_X;
    else if (dstCls == RegClass::GPR && srcCls == RegClass::FPR) op = MIOp::FMV_X_W;
    else throw std::runtime_error("unsupported move between register classes");
    auto mi = std::make_unique<MachineInst>(op);
    mi->addOperand(MOperand::Def(dstReg, dstCls));
    mi->addOperand(MOperand::Use(srcReg, srcCls));
    curMBB_->append(std::move(mi));
  }

  void appendFloatConstToFPR(int dstReg, float value) {
    int bitsReg = newTempVReg(RegClass::GPR, /*rematerializable=*/true);
    appendLoadImmToGPR(bitsReg, static_cast<int32_t>(floatBits(value)));
    auto mv = std::make_unique<MachineInst>(MIOp::FMV_W_X);
    mv->addOperand(MOperand::Def(dstReg, RegClass::FPR));
    mv->addOperand(MOperand::Use(bitsReg, RegClass::GPR));
    curMBB_->append(std::move(mv));
  }

  void appendStoreToStack(int srcReg, RegClass srcCls, MIOp op, int off) {
    if (fitsSigned12(off)) {
      auto st = std::make_unique<MachineInst>(op);
      st->addOperand(MOperand::Use(srcReg, srcCls));
      st->addOperand(MOperand::Imm(off));
      st->addOperand(MOperand::Use(gpr::sp, RegClass::GPR));
      curMBB_->append(std::move(st));
      return;
    }
    appendLoadImmToGPR(gpr::t6, off);
    auto add = std::make_unique<MachineInst>(MIOp::ADD);
    add->addOperand(MOperand::Def(gpr::t6, RegClass::GPR));
    add->addOperand(MOperand::Use(gpr::sp, RegClass::GPR));
    add->addOperand(MOperand::Use(gpr::t6, RegClass::GPR));
    curMBB_->append(std::move(add));
    auto st = std::make_unique<MachineInst>(op);
    st->addOperand(MOperand::Use(srcReg, srcCls));
    st->addOperand(MOperand::Imm(0));
    st->addOperand(MOperand::Use(gpr::t6, RegClass::GPR));
    curMBB_->append(std::move(st));
  }

  void appendLoadFromStack(int dstReg, RegClass dstCls, MIOp op, int off) {
    if (fitsSigned12(off)) {
      auto ld = std::make_unique<MachineInst>(op);
      ld->addOperand(MOperand::Def(dstReg, dstCls));
      ld->addOperand(MOperand::Imm(off));
      ld->addOperand(MOperand::Use(gpr::sp, RegClass::GPR));
      curMBB_->append(std::move(ld));
      return;
    }
    appendLoadImmToGPR(gpr::t6, off);
    auto add = std::make_unique<MachineInst>(MIOp::ADD);
    add->addOperand(MOperand::Def(gpr::t6, RegClass::GPR));
    add->addOperand(MOperand::Use(gpr::sp, RegClass::GPR));
    add->addOperand(MOperand::Use(gpr::t6, RegClass::GPR));
    curMBB_->append(std::move(add));
    auto ld = std::make_unique<MachineInst>(op);
    ld->addOperand(MOperand::Def(dstReg, dstCls));
    ld->addOperand(MOperand::Imm(0));
    ld->addOperand(MOperand::Use(gpr::t6, RegClass::GPR));
    curMBB_->append(std::move(ld));
  }

  void appendStoreAtOffset(int srcReg, RegClass srcCls, MIOp op,
                           int baseReg, int off, const char* comment = nullptr) {
    int addrReg = baseReg;
    int localOff = off;
    if (!fitsSigned12(localOff)) {
      int foldedBase = newTempVReg(RegClass::GPR, /*rematerializable=*/true);
      appendAddrAdd(foldedBase, baseReg, localOff);
      addrReg = foldedBase;
      localOff = 0;
    }
    auto st = std::make_unique<MachineInst>(op);
    st->addOperand(MOperand::Use(srcReg, srcCls));
    st->addOperand(MOperand::Imm(localOff));
    st->addOperand(MOperand::Use(addrReg, RegClass::GPR));
    if (comment)
      st->comment = comment;
    curMBB_->append(std::move(st));
  }

  bool tryLowerMemsetZeroCall(Instruction* inst) {
    if (!inst || inst->name() != "memset" || inst->numOperands() != 3)
      return false;
    auto* fill = dynamic_cast<ConstInt*>(inst->operand(1));
    auto* size = dynamic_cast<ConstInt*>(inst->operand(2));
    if (!fill || !size || fill->value() != 0)
      return false;
    int bytes = size->value();
    if (bytes <= 0 || bytes > 512)
      return false;

    Value* rootBase = nullptr;
    int rootOffset = 0;
    if (!splitConstantPtrOffset(inst->operand(0), rootBase, rootOffset))
      return false;
    auto* allocaInst = dynamic_cast<Instruction*>(rootBase);
    if (!allocaInst || allocaInst->opcode() != Opcode::Alloca)
      return false;
    if (rootOffset < 0 || rootOffset + bytes > allocaInst->memorySizeBytes())
      return false;

    int baseReg = -1;
    auto fiIt = allocaFrame_.find(allocaInst);
    if (fiIt != allocaFrame_.end()) {
      baseReg = gpr::t5;
      auto base = std::make_unique<MachineInst>(MIOp::ADDI);
      base->addOperand(MOperand::Def(baseReg, RegClass::GPR));
      base->addOperand(MOperand::Frame(fiIt->second));
      base->addOperand(MOperand::Imm(0));
      base->comment = "memset-base";
      curMBB_->append(std::move(base));
    } else {
      baseReg = materializeValue(rootBase);
    }

    int off = rootOffset;
    int remaining = bytes;
    bool preferWideStores = allocaInst->memorySizeBytes() >= 8;
    if (preferWideStores && (off & 7) != 0 && remaining >= 4) {
      appendStoreAtOffset(gpr::x0, RegClass::GPR, MIOp::SW,
                          baseReg, off, "memset-zero");
      off += 4;
      remaining -= 4;
    }
    while (preferWideStores && remaining >= 8 && (off & 7) == 0) {
      appendStoreAtOffset(gpr::x0, RegClass::GPR, MIOp::SD,
                          baseReg, off, "memset-zero");
      off += 8;
      remaining -= 8;
    }
    while (remaining >= 4) {
      appendStoreAtOffset(gpr::x0, RegClass::GPR, MIOp::SW,
                          baseReg, off, "memset-zero");
      off += 4;
      remaining -= 4;
    }
    return true;
  }

  bool tryLowerInternalModMulCall(Instruction* inst) {
    if (!inst || inst->name() != "__syc_modmul_i32" ||
        inst->numOperands() != 3 || !inst->type().isI32() ||
        mod_.getFunction("__syc_modmul_i32"))
      return false;

    // 只有模块内不存在同名用户函数时，这个名称才表示编译器内部 helper。
    // 前端只在 `0 <= a < mod`、`b >= 0` 且模数足够小时生成这个内部
    // helper。此时宽乘积为非负 64 位整数；常量模数可用倒数高位乘法求商，
    // 再用一次符号修正得到余数，避开循环热点里的硬件除法延迟。
    int lhsReg = materializeValue(inst->operand(0));
    int rhsReg = materializeValue(inst->operand(1));
    int productReg = appendBinaryRR(MIOp::MUL, lhsReg, rhsReg,
                                    "modmul.wide.mul");
    int remReg = 0;
    auto* modConst = dynamic_cast<ConstInt*>(inst->operand(2));
    if (modConst && modConst->value() >= 3 &&
        modConst->value() <= std::numeric_limits<int32_t>::max() / 2) {
      const uint64_t modulus = static_cast<uint32_t>(modConst->value());
      const uint64_t reciprocal =
          std::numeric_limits<uint64_t>::max() / modulus + 1;
      int reciprocalReg =
          materializeIntConstant(static_cast<int64_t>(reciprocal));
      int quotientReg = appendBinaryRR(MIOp::MULH, productReg, reciprocalReg,
                                       "modmul.recip.quotient");
      int modReg = materializeValue(inst->operand(2));
      int reducedReg = appendBinaryRR(MIOp::MUL, quotientReg, modReg,
                                      "modmul.recip.product");
      int candidateReg = appendBinaryRR(MIOp::SUB, productReg, reducedReg,
                                        "modmul.recip.candidate");
      int signReg = appendBinaryRI(MIOp::SRAI, candidateReg, 63,
                                   "modmul.recip.sign");
      int correctionReg = appendBinaryRR(MIOp::AND_, signReg, modReg,
                                         "modmul.recip.correction");
      remReg = appendBinaryRR(MIOp::ADD, candidateReg, correctionReg,
                              "modmul.recip.remainder");
    } else {
      int modReg = materializeValue(inst->operand(2));
      remReg = appendBinaryRR(MIOp::REM, productReg, modReg,
                              "modmul.wide.rem");
    }
    int dst = ensureVReg(inst);
    int zeroReg = materializeIntConstant(0);
    int narrowed = appendBinaryRR(MIOp::ADDW, remReg, zeroReg,
                                  "modmul.narrow");
    appendRegMove(dst, RegClass::GPR, narrowed, RegClass::GPR);
    return true;
  }

  void materializeValueToGPR(Value* v, int dstReg) {
    if (auto* ci = dynamic_cast<ConstInt*>(v)) {
      appendLoadImmToGPR(dstReg, ci->value());
      return;
    }
    Value* rootBase = nullptr;
    int rootOffset = 0;
    if (splitConstantPtrOffset(v, rootBase, rootOffset)) {
      if (auto* allocaInst = dynamic_cast<Instruction*>(rootBase)) {
        auto fiIt = allocaFrame_.find(allocaInst);
        if (fiIt != allocaFrame_.end()) {
          auto mi = std::make_unique<MachineInst>(MIOp::ADDI);
          mi->addOperand(MOperand::Def(dstReg, RegClass::GPR));
          mi->addOperand(MOperand::Frame(fiIt->second));
          mi->addOperand(MOperand::Imm(0));
          mi->comment = "direct-arg.frame";
          curMBB_->append(std::move(mi));
          if (rootOffset != 0)
            appendAddrAdd(dstReg, dstReg, rootOffset);
          return;
        }
      }
      if (auto* gv = dynamic_cast<GlobalVar*>(rootBase)) {
        appendLoadGlobalAddrToGPR(dstReg, gv->name());
        if (rootOffset != 0)
          appendAddrAdd(dstReg, dstReg, rootOffset);
        return;
      }
    }
    if (auto* gv = dynamic_cast<GlobalVar*>(v)) {
      appendLoadGlobalAddrToGPR(dstReg, gv->name());
      return;
    }
    int src = materializeValue(v);
    appendRegMove(dstReg, RegClass::GPR, src, RegClass::GPR);
  }

  void materializeValueToFPR(Value* v, int dstReg) {
    if (auto* cf = dynamic_cast<ConstFloat*>(v)) {
      int srcReg = materializeCachedFloatConst(cf->value());
      appendRegMove(dstReg, RegClass::FPR, srcReg, RegClass::FPR);
      return;
    }
    int src = materializeValue(v);
    appendRegMove(dstReg, RegClass::FPR, src, RegClass::FPR);
  }

  void materializeFloatBitsToGPR(Value* v, int dstReg) {
    if (auto* cf = dynamic_cast<ConstFloat*>(v)) {
      appendLoadImmToGPR(dstReg, static_cast<int32_t>(floatBits(cf->value())));
      return;
    }
    int src = materializeValue(v);
    appendRegMove(dstReg, RegClass::GPR, src, RegClass::FPR);
  }

  int materializeFloatOperand(Value* v) {
    if (auto* cf = dynamic_cast<ConstFloat*>(v)) {
      return materializeCachedFloatConst(cf->value());
    }
    return materializeValue(v);
  }

  int materializeBranchGPROperand(Value* v) {
    if (auto* ci = dynamic_cast<ConstInt*>(v)) {
      if (ci->value() == 0)
        return gpr::x0;
    }
    return materializeValue(v);
  }

  // --------------------------------------------------------------
  // 函数级降低
  // --------------------------------------------------------------
  void lowerFunction(Function* func) {
    curFunc_ = func;
    curMF_ = mfMap_[func];
    bbMap_.clear();
    valueVReg_.clear();
    allocaFrame_.clear();
    blockAddrVReg_.clear();
    hoistedAddrVReg_.clear();
    floatConstVReg_.clear();
    invariantIntConstVReg_.clear();

    if (func->blocks().empty()) return; // 仅有声明

    // 为每个 CoreIR 基本块创建 MIR 基本块。
    int idx = 0;
    for (const auto& bb : func->blocks()) {
      std::string label = "." + func->name() + ".bb" + std::to_string(idx++);
      MachineBasicBlock* mbb = curMF_->newBlock(label);
      bbMap_[bb.get()] = mbb;
    }
    // 连接前驱和后继关系。
    for (const auto& bb : func->blocks()) {
      MachineBasicBlock* mbb = bbMap_[bb.get()];
      for (auto* p : bb->preds()) {
        mbb->preds().push_back(bbMap_[p]);
      }
      for (auto* s : bb->succs()) {
        mbb->succs().push_back(bbMap_[s]);
      }
    }

    // 入口块：按照 RV64GC 标量 ABI 接收参数。
    curMBB_ = bbMap_[func->entryBlock()];
    // 按 ABI 实参布局为每个形参在入口块生成搬移：浮点寄存器/整数寄存器
    // 直接移入虚寄存器；栈传递的参数从帧指针加载，
    // 偏移超 12 位时先物化偏移再寻址。
    std::vector<Type> argTypes;
    argTypes.reserve(func->args().size());
    for (const auto& arg : func->args()) argTypes.push_back(arg.type);
    ABIArgLayout argLayout = assignABIArgLayout(argTypes);
    std::vector<std::unique_ptr<MachineInst>> entryProlog;
    for (size_t argIndex = 0; argIndex < func->args().size(); ++argIndex) {
      const auto& arg = func->args()[argIndex];
      const ABIArgLoc& loc = argLayout.locs[argIndex];
      RegClass cls = classFor(arg.type);
      int vreg = curMF_->newVReg(cls);
      valueVReg_[arg.val] = vreg;
      if (loc.kind == ABIArgLocKind::FloatReg) {
        auto inst = std::make_unique<MachineInst>(MIOp::FMV_S);
        inst->addOperand(MOperand::Def(vreg, cls));
        inst->addOperand(MOperand::Use(kFaReg[loc.index], cls));
        entryProlog.push_back(std::move(inst));
      } else if (loc.kind == ABIArgLocKind::IntReg && cls == RegClass::FPR) {
        auto inst = std::make_unique<MachineInst>(MIOp::FMV_W_X);
        inst->addOperand(MOperand::Def(vreg, RegClass::FPR));
        inst->addOperand(MOperand::Use(kAReg[loc.index], RegClass::GPR));
        entryProlog.push_back(std::move(inst));
      } else if (loc.kind == ABIArgLocKind::IntReg) {
        auto inst = std::make_unique<MachineInst>(MIOp::MV);
        inst->addOperand(MOperand::Def(vreg, cls));
        inst->addOperand(MOperand::Use(kAReg[loc.index], cls));
        entryProlog.push_back(std::move(inst));
      } else {
        int off = loc.index * 8;
        MIOp loadOp = loadOpForType(arg.type);
        if (fitsSigned12(off)) {
          auto inst = std::make_unique<MachineInst>(loadOp);
          inst->addOperand(MOperand::Def(vreg, cls));
          inst->addOperand(MOperand::Imm(off));
          inst->addOperand(MOperand::Use(gpr::s0, RegClass::GPR));
          entryProlog.push_back(std::move(inst));
        } else {
          auto li = std::make_unique<MachineInst>(MIOp::LI);
          li->addOperand(MOperand::Def(gpr::t6, RegClass::GPR));
          li->addOperand(MOperand::Imm(off));
          entryProlog.push_back(std::move(li));
          auto add = std::make_unique<MachineInst>(MIOp::ADD);
          add->addOperand(MOperand::Def(gpr::t6, RegClass::GPR));
          add->addOperand(MOperand::Use(gpr::s0, RegClass::GPR));
          add->addOperand(MOperand::Use(gpr::t6, RegClass::GPR));
          entryProlog.push_back(std::move(add));
          auto inst = std::make_unique<MachineInst>(loadOp);
          inst->addOperand(MOperand::Def(vreg, cls));
          inst->addOperand(MOperand::Imm(0));
          inst->addOperand(MOperand::Use(gpr::t6, RegClass::GPR));
          entryProlog.push_back(std::move(inst));
        }
      }
    }
    // 按原始顺序插入：从末尾开始逐个前置，保证第一个参数最终仍在前面。
    for (auto it = entryProlog.rbegin(); it != entryProlog.rend(); ++it) {
      curMBB_->prepend(std::move(*it));
    }
    curMF_->incomingIntArgs = argLayout.intRegsUsed;
    curMF_->incomingFloatArgs = argLayout.floatRegsUsed;
    curMF_->incomingStackArgBytes = argLayout.stackSlotsUsed * 8;

    // 预扫描：在每个基本块开头生成 PHI 伪指令。记录源 Instruction* 到 MIR phi
    // 的映射，后续按对象对应关系填充操作数，而不是按序号匹配。
    std::unordered_map<const Instruction*, MachineInst*> phiMap;
    for (const auto& bb : func->blocks()) {
      MachineBasicBlock* mbb = bbMap_[bb.get()];
      for (const auto& inst : bb->instructions()) {
        if (inst->opcode() == Opcode::Phi) {
          int vreg = ensureVReg(inst.get());
          auto phi = std::make_unique<MachineInst>(MIOp::PHI);
          phi->addOperand(MOperand::Def(vreg, classFor(inst->type())));
          MachineInst* p = phi.get();
          mbb->prepend(std::move(phi));
          phiMap[inst.get()] = p;
        }
      }
    }

    // 主要降低过程。
    for (const auto& bb : func->blocks()) {
      curCoreBB_ = bb.get();
      curMBB_ = bbMap_[bb.get()];
      blockAddrVReg_.clear();
      for (const auto& inst : bb->instructions()) {
        if (inst->opcode() == Opcode::Phi) continue; // 后续统一处理
        lowerInstruction(inst.get());
      }
    }

    // 所有值完成映射后填充 phi 操作数，通过 phiMap 按对象对应关系解析。
    for (const auto& bb : func->blocks()) {
      for (const auto& inst : bb->instructions()) {
        if (inst->opcode() != Opcode::Phi) continue;
        auto it = phiMap.find(inst.get());
        if (it == phiMap.end()) continue;
        MachineInst* mphi = it->second;
        for (int oi = 0; oi + 1 < inst->numOperands(); oi += 2) {
          Value* incVal = inst->operand(oi);
          BasicBlock* incBB = dynamic_cast<BasicBlock*>(inst->operand(oi + 1));
          if (!incBB) continue;
          int srcReg = materializeValue(incVal, bbMap_[incBB]);
          MachineBasicBlock* incMBB = bbMap_[incBB];
          mphi->addOperand(MOperand::Use(srcReg, classFor(incVal->type())));
          mphi->addOperand(MOperand::BB(incMBB));
        }
      }
    }
  }

  // --------------------------------------------------------------
  // 值物化
  // --------------------------------------------------------------
  int ensureVReg(const Value* v) {
    auto it = valueVReg_.find(v);
    if (it != valueVReg_.end()) return it->second;
    int vreg = curMF_->newVReg(classFor(v->type()));
    valueVReg_[v] = vreg;
    return vreg;
  }

  MachineInst* firstControlTransfer(MachineBasicBlock* targetMBB) const {
    for (const auto& inst : targetMBB->insts()) {
      if (mioIsTerminator(inst->opcode()))
        return inst.get();
    }
    return nullptr;
  }

  // 向目标块发射指令。atEdge 为 true 时插入在第一条控制转移之前。条件分支后
  // 可能还有显式跳转终结指令，只使用最后一条终结指令会把物化放到一条出口边上，
  // 而不是放在分支之前。
  void emitInto(MachineBasicBlock* targetMBB, std::unique_ptr<MachineInst> mi,
                bool atEdge) {
    if (atEdge) {
      MachineInst* term = firstControlTransfer(targetMBB);
      if (term) {
        targetMBB->insertBefore(term, std::move(mi));
        return;
      }
    }
    targetMBB->append(std::move(mi));
  }

  int materializeCachedFloatConst(float value) {
    uint32_t bits = floatBits(value);
    auto it = floatConstVReg_.find(bits);
    if (it != floatConstVReg_.end())
      return it->second;

    MachineBasicBlock* entry = curMF_->entryBlock();
    if (!entry)
      throw std::runtime_error("MIR lowering: missing entry block for float const");

    int bitsReg = curMF_->newVReg(RegClass::GPR);
    curMF_->vregInfo(bitsReg).rematerializable = true;
    auto li = std::make_unique<MachineInst>(MIOp::LI);
    li->addOperand(MOperand::Def(bitsReg, RegClass::GPR));
    li->addOperand(MOperand::Imm(static_cast<int32_t>(bits)));
    emitInto(entry, std::move(li), /*atEdge=*/true);

    int valReg = curMF_->newVReg(RegClass::FPR);
    auto mv = std::make_unique<MachineInst>(MIOp::FMV_W_X);
    mv->addOperand(MOperand::Def(valReg, RegClass::FPR));
    mv->addOperand(MOperand::Use(bitsReg, RegClass::GPR));
    emitInto(entry, std::move(mv), /*atEdge=*/true);

    floatConstVReg_[bits] = valReg;
    return valReg;
  }

  int materializeCachedInvariantIntConst(int32_t value) {
    auto it = invariantIntConstVReg_.find(value);
    if (it != invariantIntConstVReg_.end())
      return it->second;

    MachineBasicBlock* entry = curMF_->entryBlock();
    if (!entry)
      throw std::runtime_error("MIR lowering: missing entry block for int const");

    int valueReg = curMF_->newVReg(RegClass::GPR);
    auto li = std::make_unique<MachineInst>(MIOp::LI);
    li->addOperand(MOperand::Def(valueReg, RegClass::GPR));
    li->addOperand(MOperand::Imm(value));
    li->comment = "const-div.invariant";
    emitInto(entry, std::move(li), /*atEdge=*/true);
    invariantIntConstVReg_[value] = valueReg;
    return valueReg;
  }

  bool shouldHoistInvariantAddr(Value* v) const {
    if (auto* inst = dynamic_cast<Instruction*>(v)) {
      if (inst->opcode() != Opcode::Alloca)
        return false;
      // 大私有对象的地址在整个函数中都不变。若继续按“每个块首次
      // 使用时再 materialize”处理，同一个 FrameIdx 会在热路径里反复变成
      // li+add sp。这里把足够大的 alloca 地址统一前提到 entry 并复用
      // （通用 invariant-address hoist）。
      return inst->memorySizeBytes() >= 256;
    }
    if (dynamic_cast<GlobalVar*>(v)) {
      // 全局基址虽然不变，但入口 hoist 会把地址值延长到整个函数。
      // 在缺少局部 live-range splitting / register-pressure 估计前，统一保留
      // 每块缓存和 rematerialize，避免用整函数形态阈值决定某些全局特殊处理。
      return false;
    }
    return false;
  }

  // 将 Value 物化为虚拟寄存器。如果 edgeMBB 非空，常量或全局加载会在该块
  // 的终结指令前插入，用于 PHI 边消解，使值在前驱的出口边上可用。
  int materializeValue(Value* v, MachineBasicBlock* edgeMBB = nullptr) {
    MachineBasicBlock* tgt = edgeMBB ? edgeMBB : curMBB_;
    bool atEdge = edgeMBB != nullptr;

    // 按值类别物化到虚寄存器：整型常量用 LI，浮点常量走常量池缓存；
    // alloca/全局变量的地址按“可提升则提到 entry 块，否则按块缓存”
    // 复用，避免重复生成地址计算。
    if (auto* ci = dynamic_cast<ConstInt*>(v)) {
      int vreg = newTempVReg(RegClass::GPR, /*rematerializable=*/true);
      auto mi = std::make_unique<MachineInst>(MIOp::LI);
      mi->addOperand(MOperand::Def(vreg, RegClass::GPR));
      mi->addOperand(MOperand::Imm(ci->value()));
      emitInto(tgt, std::move(mi), atEdge);
      return vreg;
    }
    if (auto* cf = dynamic_cast<ConstFloat*>(v)) {
      return materializeCachedFloatConst(cf->value());
    }
    if (auto* inst = dynamic_cast<Instruction*>(v)) {
      if (inst->opcode() == Opcode::Alloca) {
        auto fiIt = allocaFrame_.find(inst);
        if (fiIt != allocaFrame_.end()) {
          if (!edgeMBB && shouldHoistInvariantAddr(v)) {
            auto hoistedIt = hoistedAddrVReg_.find(v);
            if (hoistedIt != hoistedAddrVReg_.end())
              return hoistedIt->second;
            MachineBasicBlock* entry = curMF_->entryBlock();
            if (!entry)
              throw std::runtime_error(
                  "MIR lowering: missing entry block for hoisted alloca addr");
            int vreg = newTempVReg(RegClass::GPR, /*rematerializable=*/true);
            auto mi = std::make_unique<MachineInst>(MIOp::ADDI);
            mi->addOperand(MOperand::Def(vreg, RegClass::GPR));
            mi->addOperand(MOperand::Frame(fiIt->second));
            mi->addOperand(MOperand::Imm(0));
            mi->comment = "alloca.addr.hoisted";
            emitInto(entry, std::move(mi), /*atEdge=*/true);
            hoistedAddrVReg_[v] = vreg;
            return vreg;
          }
          if (!edgeMBB) {
            auto cacheIt = blockAddrVReg_.find(v);
            if (cacheIt != blockAddrVReg_.end())
              return cacheIt->second;
          }
          int vreg = newTempVReg(RegClass::GPR, /*rematerializable=*/true);
          auto mi = std::make_unique<MachineInst>(MIOp::ADDI);
          mi->addOperand(MOperand::Def(vreg, RegClass::GPR));
          mi->addOperand(MOperand::Frame(fiIt->second));
          mi->addOperand(MOperand::Imm(0));
          mi->comment = "alloca.addr";
          emitInto(tgt, std::move(mi), atEdge);
          if (!edgeMBB)
            blockAddrVReg_[v] = vreg;
          return vreg;
        }
      }
    }
    if (auto* gv = dynamic_cast<GlobalVar*>(v)) {
      if (!edgeMBB && shouldHoistInvariantAddr(v)) {
        auto hoistedIt = hoistedAddrVReg_.find(v);
        if (hoistedIt != hoistedAddrVReg_.end())
          return hoistedIt->second;
        MachineBasicBlock* entry = curMF_->entryBlock();
        if (!entry)
          throw std::runtime_error(
              "MIR lowering: missing entry block for hoisted global addr");
        int vreg = newTempVReg(RegClass::GPR, /*rematerializable=*/true);
        auto la = std::make_unique<MachineInst>(MIOp::LA);
        la->addOperand(MOperand::Def(vreg, RegClass::GPR));
        la->addOperand(MOperand::Sym(gv->name()));
        la->comment = "global.addr.hoisted";
        emitInto(entry, std::move(la), /*atEdge=*/true);
        hoistedAddrVReg_[v] = vreg;
        return vreg;
      }
      if (!edgeMBB) {
        auto cacheIt = blockAddrVReg_.find(v);
        if (cacheIt != blockAddrVReg_.end())
          return cacheIt->second;
      }
      int vreg = newTempVReg(RegClass::GPR, /*rematerializable=*/true);
      auto la = std::make_unique<MachineInst>(MIOp::LA);
      la->addOperand(MOperand::Def(vreg, RegClass::GPR));
      la->addOperand(MOperand::Sym(gv->name()));
      emitInto(tgt, std::move(la), atEdge);
      if (!edgeMBB)
        blockAddrVReg_[v] = vreg;
      return vreg;
    }
    // 函数参数或指令结果：此时应当已经拥有虚拟寄存器。
    return ensureVReg(v);
  }

  // --------------------------------------------------------------
  // 指令分派
  // --------------------------------------------------------------
  void lowerInstruction(Instruction* inst) {
    switch (inst->opcode()) {
      case Opcode::Alloca:   return lowerAlloca(inst);
      case Opcode::Load:     return lowerLoad(inst);
      case Opcode::Store:    return lowerStore(inst);
      case Opcode::SDivDirectPrepare: return lowerDirectDivisorPrepare(inst);
      case Opcode::SDivDirectMagic: return lowerDirectDivisorMagic(inst);
      case Opcode::SDivDirectShift: return lowerDirectDivisorShift(inst);
      case Opcode::SDivDirectFlag: return lowerDirectDivisorFlag(inst);
      case Opcode::SDivInvariantDirect: return lowerDirectInvariantSignedDiv(inst);
      case Opcode::Add: case Opcode::Sub: case Opcode::Mul: case Opcode::Mulh:
      case Opcode::SDiv: case Opcode::SRem:
      case Opcode::And: case Opcode::Or: case Opcode::Xor:
      case Opcode::Shl: case Opcode::Shr:
      case Opcode::FAdd: case Opcode::FSub: case Opcode::FMul: case Opcode::FDiv:
        return lowerBinary(inst);
      case Opcode::ICmp:
        if (isDirectCondBrCompare(inst))
          return;
        return lowerICmp(inst);
      case Opcode::FCmp:
        if (isDirectCondBrCompare(inst))
          return;
        return lowerFCmp(inst);
      case Opcode::Br:       return lowerBr(inst);
      case Opcode::CondBr:   return lowerCondBr(inst);
      case Opcode::Ret:      return lowerRet(inst);
      case Opcode::Call:     return lowerCall(inst);
      case Opcode::GEP:      return lowerGEP(inst);
      case Opcode::Neg:      return lowerNeg(inst);
      case Opcode::FNeg:     return lowerFNeg(inst);
      case Opcode::BitCast:  return lowerBitCast(inst);
      case Opcode::SIToFP:   return lowerSIToFP(inst);
      case Opcode::FPToSI:   return lowerFPToSI(inst);
      case Opcode::Mov:      return lowerMov(inst);
      case Opcode::Select:   return lowerSelect(inst);
      case Opcode::Phi:      return; // 已在预扫描阶段处理
      default:
        throw std::runtime_error("MIR lowering does not support opcode: " +
                                 std::string(core_ir::opcodeName(inst->opcode())));
    }
  }

  // --------------------------------------------------------------
  // 各操作码的降低
  // --------------------------------------------------------------
  void lowerAlloca(Instruction* inst) {
    int size = inst->memorySizeBytes();
    if (size <= 0) size = 4;
    int align = 4;
    if (size >= 8) align = 8;
    int fi = curMF_->newFrameObject(size, align, /*isSpill=*/false, "alloca");
    allocaFrame_[inst] = fi;
  }

  void lowerLoad(Instruction* inst) {
    Value* base = nullptr;
    int offset = 0;
    if (!splitConstantPtrOffset(inst->operand(0), base, offset)) {
      base = inst->operand(0);
      offset = 0;
    }
    int addrReg = materializeValue(base);
    if (!fitsSigned12(offset)) {
      int foldedBase = newTempVReg(RegClass::GPR, /*rematerializable=*/true);
      appendAddrAdd(foldedBase, addrReg, offset);
      addrReg = foldedBase;
      offset = 0;
    }
    int dst = ensureVReg(inst);
    MIOp op = loadOpForType(inst->type());
    RegClass cls = classFor(inst->type());
    auto mi = std::make_unique<MachineInst>(op);
    mi->addOperand(MOperand::Def(dst, cls));
    mi->addOperand(MOperand::Imm(offset));
    mi->addOperand(MOperand::Use(addrReg, RegClass::GPR));
    curMBB_->append(std::move(mi));
  }

  void lowerStore(Instruction* inst) {
    Value* val = inst->operand(0);
    Value* addr = inst->operand(1);
    int valReg = materializeValue(val);
    Value* base = nullptr;
    int offset = 0;
    if (!splitConstantPtrOffset(addr, base, offset)) {
      base = addr;
      offset = 0;
    }
    int addrReg = materializeValue(base);
    if (!fitsSigned12(offset)) {
      int foldedBase = newTempVReg(RegClass::GPR, /*rematerializable=*/true);
      appendAddrAdd(foldedBase, addrReg, offset);
      addrReg = foldedBase;
      offset = 0;
    }
    MIOp op = storeOpForType(val->type());
    RegClass cls = classFor(val->type());
    auto mi = std::make_unique<MachineInst>(op);
    mi->addOperand(MOperand::Use(valReg, cls));
    mi->addOperand(MOperand::Imm(offset));
    mi->addOperand(MOperand::Use(addrReg, RegClass::GPR));
    curMBB_->append(std::move(mi));
  }

  int zeroExtendWord(int sourceReg, const char* comment) {
    int shifted = appendBinaryRI(MIOp::SLLI, sourceReg, 32, comment);
    return appendBinaryRI(MIOp::SRLI, shifted, 32, comment);
  }

  void lowerDirectDivisorPrepare(Instruction* inst) {
    if (!inst || inst->numOperands() != 1)
      throw std::runtime_error("invalid direct divisor descriptor");
    int divisor = materializeValue(inst->operand(0));

    int sign = appendBinaryRI(MIOp::SRAIW, divisor, 31,
                              "direct-divisor.sign");
    int absoluteBits = appendBinaryRR(MIOp::XOR_, divisor, sign,
                                      "direct-divisor.abs.xor");
    absoluteBits = appendBinaryRR(MIOp::SUBW, absoluteBits, sign,
                                  "direct-divisor.abs");
    int absolute = zeroExtendWord(absoluteBits, "direct-divisor.abs.zext");

    // 用位扩散和分组计数求 floor(log2(abs(divisor)))，只在除数定义后执行
    // 一次，不要求目标实现 Zbb 扩展。
    int spread = absolute;
    for (int bitShift : {1, 2, 4, 8, 16}) {
      int part = appendBinaryRI(MIOp::SRLI, spread, bitShift,
                                "direct-divisor.log2.spread");
      spread = appendBinaryRR(MIOp::OR_, spread, part,
                              "direct-divisor.log2.spread");
    }
    int mask55 = materializeIntConstant(0x55555555);
    int half = appendBinaryRI(MIOp::SRLI, spread, 1,
                              "direct-divisor.popcount.half");
    half = appendBinaryRR(MIOp::AND_, half, mask55,
                          "direct-divisor.popcount.half");
    int count = appendBinaryRR(MIOp::SUBW, spread, half,
                               "direct-divisor.popcount.sub");
    int mask33 = materializeIntConstant(0x33333333);
    int countLow = appendBinaryRR(MIOp::AND_, count, mask33,
                                  "direct-divisor.popcount.low");
    int countHigh = appendBinaryRI(MIOp::SRLI, count, 2,
                                   "direct-divisor.popcount.high");
    countHigh = appendBinaryRR(MIOp::AND_, countHigh, mask33,
                               "direct-divisor.popcount.high");
    count = appendBinaryRR(MIOp::ADDW, countLow, countHigh,
                           "direct-divisor.popcount.pairs");
    int quarter = appendBinaryRI(MIOp::SRLI, count, 4,
                                 "direct-divisor.popcount.quarter");
    count = appendBinaryRR(MIOp::ADDW, count, quarter,
                           "direct-divisor.popcount.quarter");
    int mask0f = materializeIntConstant(0x0f0f0f0f);
    count = appendBinaryRR(MIOp::AND_, count, mask0f,
                           "direct-divisor.popcount.nibbles");
    int countByte = appendBinaryRI(MIOp::SRLI, count, 8,
                                   "direct-divisor.popcount.byte");
    count = appendBinaryRR(MIOp::ADDW, count, countByte,
                           "direct-divisor.popcount.byte");
    int countWord = appendBinaryRI(MIOp::SRLI, count, 16,
                                   "direct-divisor.popcount.word");
    count = appendBinaryRR(MIOp::ADDW, count, countWord,
                           "direct-divisor.popcount.word");
    count = appendBinaryRI(MIOp::ANDI, count, 0x3f,
                           "direct-divisor.popcount.result");
    int floorLog = appendBinaryRI(MIOp::ADDIW, count, -1,
                                  "direct-divisor.log2");

    // 直接乘高位公式采用有符号除法魔数的首个候选幂次。下面按该算法的
    // 终止条件验证候选；条件不满足时继续执行原始 DIVW 路径。
    int one = materializeIntConstant(1);
    int power = appendBinaryRI(MIOp::ADDIW, floorLog, 31,
                               "direct-divisor.power");
    int numerator = appendBinaryRR(MIOp::SLL, one, power,
                                   "direct-divisor.numerator");
    int signedMin = appendBinaryRI(MIOp::SLLI, one, 31,
                                   "direct-divisor.signed-min");
    int minRemainder = appendBinaryRR(MIOp::REMU, signedMin, absolute,
                                      "direct-divisor.anc.remainder");
    int anc = appendBinaryRI(MIOp::ADDI, signedMin, -1,
                             "direct-divisor.anc.base");
    anc = appendBinaryRR(MIOp::SUB, anc, minRemainder,
                         "direct-divisor.anc");

    int q1 = appendBinaryRR(MIOp::DIVU, numerator, anc,
                            "direct-divisor.q1");
    int q1Product = appendBinaryRR(MIOp::MUL, q1, anc,
                                   "direct-divisor.q1.product");
    int r1 = appendBinaryRR(MIOp::SUB, numerator, q1Product,
                            "direct-divisor.r1");
    int q2 = appendBinaryRR(MIOp::DIVU, numerator, absolute,
                            "direct-divisor.q2");
    int q2Product = appendBinaryRR(MIOp::MUL, q2, absolute,
                                   "direct-divisor.q2.product");
    int r2 = appendBinaryRR(MIOp::SUB, numerator, q2Product,
                            "direct-divisor.r2");
    int delta = appendBinaryRR(MIOp::SUB, absolute, r2,
                               "direct-divisor.delta");
    int q1Less = appendBinaryRR(MIOp::SLTU, q1, delta,
                                "direct-divisor.q1.less");
    int q1XorDelta = appendBinaryRR(MIOp::XOR_, q1, delta,
                                    "direct-divisor.q1.delta");
    int q1Equal = appendBinaryRI(MIOp::SLTIU, q1XorDelta, 1,
                                 "direct-divisor.q1.equal");
    int r1Zero = appendBinaryRI(MIOp::SLTIU, r1, 1,
                                "direct-divisor.r1.zero");
    int tiedAtZero = appendBinaryRR(MIOp::AND_, q1Equal, r1Zero,
                                    "direct-divisor.tie");
    int invalid = appendBinaryRR(MIOp::OR_, q1Less, tiedAtZero,
                                 "direct-divisor.invalid");
    int terminates = appendBinaryRI(MIOp::XORI, invalid, 1,
                                    "direct-divisor.terminates");

    int magic = appendBinaryRI(MIOp::ADDI, q2, 1,
                               "direct-divisor.magic");
    int magicFits = appendBinaryRR(MIOp::SLTU, magic, signedMin,
                                   "direct-divisor.magic.fits");
    int greaterThanOne = appendBinaryRR(MIOp::SLT, one, divisor,
                                        "direct-divisor.positive");
    int valid = appendBinaryRR(MIOp::AND_, terminates, magicFits,
                               "direct-divisor.valid.magic");
    valid = appendBinaryRR(MIOp::AND_, valid, greaterThanOne,
                           "direct-divisor.valid");
    int validMask = appendBinaryRR(MIOp::SUB, gpr::x0, valid,
                                   "direct-divisor.valid.mask");
    magic = appendBinaryRR(MIOp::AND_, magic, validMask,
                           "direct-divisor.magic.select");
    int magicLow = zeroExtendWord(magic, "direct-divisor.magic.low");
    int shift = appendBinaryRI(MIOp::ADDIW, floorLog, -1,
                               "direct-divisor.shift");
    shift = appendBinaryRR(MIOp::AND_, shift, validMask,
                           "direct-divisor.shift.select");
    int shiftHigh = appendBinaryRI(MIOp::SLLI, shift, 32,
                                   "direct-divisor.shift.high");
    int validHigh = appendBinaryRI(MIOp::SLLI, valid, 63,
                                   "direct-divisor.valid.high");
    int descriptor = appendBinaryRR(MIOp::OR_, shiftHigh, magicLow,
                                    "direct-divisor.descriptor.base");
    descriptor = appendBinaryRR(MIOp::OR_, descriptor, validHigh,
                                "direct-divisor.descriptor");
    appendRegMove(ensureVReg(inst), RegClass::GPR, descriptor,
                  RegClass::GPR);
  }

  void lowerDirectDivisorMagic(Instruction* inst) {
    if (!inst || inst->numOperands() != 1)
      throw std::runtime_error("invalid direct divisor magic");
    int descriptor = materializeValue(inst->operand(0));
    int magic = appendBinaryRI(MIOp::ADDIW, descriptor, 0,
                               "direct-divisor.magic.extract");
    appendRegMove(ensureVReg(inst), RegClass::GPR, magic, RegClass::GPR);
  }

  void lowerDirectDivisorShift(Instruction* inst) {
    if (!inst || inst->numOperands() != 1)
      throw std::runtime_error("invalid direct divisor shift");
    int descriptor = materializeValue(inst->operand(0));
    int shift = appendBinaryRI(MIOp::SRLI, descriptor, 32,
                               "direct-divisor.shift.extract");
    shift = appendBinaryRI(MIOp::ANDI, shift, 31,
                           "direct-divisor.shift.mask");
    appendRegMove(ensureVReg(inst), RegClass::GPR, shift, RegClass::GPR);
  }

  void lowerDirectDivisorFlag(Instruction* inst) {
    if (!inst || inst->numOperands() != 1)
      throw std::runtime_error("invalid direct divisor flag");
    int descriptor = materializeValue(inst->operand(0));
    int flag = appendBinaryRI(MIOp::SRLI, descriptor, 63,
                              "direct-divisor.flag.extract");
    appendRegMove(ensureVReg(inst), RegClass::GPR, flag, RegClass::GPR);
  }

  void lowerDirectInvariantSignedDiv(Instruction* inst) {
    if (!inst || inst->numOperands() != 3)
      throw std::runtime_error("invalid direct invariant signed division");
    int numerator = materializeValue(inst->operand(0));
    int magic = materializeValue(inst->operand(1));
    int shift = materializeValue(inst->operand(2));
    int product = appendBinaryRR(MIOp::MUL, numerator, magic,
                                 "direct-invariant-div.fullmul");
    int quotient = appendArithmeticShift64By32(
        product, "direct-invariant-div.mulhi");
    quotient = appendBinaryRR(MIOp::SRAW, quotient, shift,
                              "direct-invariant-div.shift");
    int quotientSign = appendBinaryRI(MIOp::SRLIW, quotient, 31,
                                      "direct-invariant-div.sign");
    quotient = appendBinaryRR(MIOp::ADDW, quotient, quotientSign,
                              "direct-invariant-div.round");
    appendRegMove(ensureVReg(inst), RegClass::GPR, quotient, RegClass::GPR);
  }

  void lowerBinary(Instruction* inst) {
    Value* lhs = inst->operand(0);
    Value* rhs = inst->operand(1);
    int dst = ensureVReg(inst);

    Value* rangeValue = nullptr;
    Value* rangeBound = nullptr;
    if (matchNonNegativeUpperBoundAnd(inst, rangeValue, rangeBound)) {
      // 对正上界，0 <= x && x < bound 与无符号 x < bound 完全等价；
      // 直接降低为一条比较，避免生成两个比较、取反和布尔相与。
      auto compare = std::make_unique<MachineInst>(MIOp::SLTU);
      compare->addOperand(MOperand::Def(dst, RegClass::GPR));
      compare->addOperand(
          MOperand::Use(materializeValue(rangeValue), RegClass::GPR));
      compare->addOperand(
          MOperand::Use(materializeValue(rangeBound), RegClass::GPR));
      curMBB_->append(std::move(compare));
      return;
    }

    // 浮点路径。
    if (inst->type().isF32()) {
      int l = materializeFloatOperand(lhs);
      int r = materializeFloatOperand(rhs);
      MIOp op;
      switch (inst->opcode()) {
        case Opcode::FAdd: op = MIOp::FADD_S; break;
        case Opcode::FSub: op = MIOp::FSUB_S; break;
        case Opcode::FMul: op = MIOp::FMUL_S; break;
        case Opcode::FDiv: op = MIOp::FDIV_S; break;
        default: throw std::runtime_error("bad float opcode");
      }
      auto mi = std::make_unique<MachineInst>(op);
      mi->addOperand(MOperand::Def(dst, RegClass::FPR));
      mi->addOperand(MOperand::Use(l, RegClass::FPR));
      mi->addOperand(MOperand::Use(r, RegClass::FPR));
      curMBB_->append(std::move(mi));
      return;
    }

    // 整数路径：先尝试右操作数为常量的形式，必要时交换可交换运算的操作数。
    auto* rhsConst = dynamic_cast<ConstInt*>(rhs);
    auto* lhsConst = dynamic_cast<ConstInt*>(lhs);
    bool commutative = false;
    switch (inst->opcode()) {
      case Opcode::Add: case Opcode::Mul:
      case Opcode::And: case Opcode::Or: case Opcode::Xor:
        commutative = true; break;
      default: break;
    }
    if (commutative && lhsConst && !rhsConst) {
      std::swap(lhs, rhs);
      std::swap(lhsConst, rhsConst);
    }

    // 右操作数为小常量时优先使用立即数形式的指令；
    // 常量除/余走专门的强度削减匹配，失败则回退通用路径。
    if (rhsConst) {
      int32_t imm = rhsConst->value();
      int lReg = materializeValue(lhs);
      auto emitI = [&](MIOp op, int64_t v) {
        auto mi = std::make_unique<MachineInst>(op);
        mi->addOperand(MOperand::Def(dst, RegClass::GPR));
        mi->addOperand(MOperand::Use(lReg, RegClass::GPR));
        mi->addOperand(MOperand::Imm(v));
        curMBB_->append(std::move(mi));
      };
      switch (inst->opcode()) {
        case Opcode::Add:
          if (fitsSigned12(imm)) { emitI(MIOp::ADDIW, imm); return; }
          break;
        case Opcode::Sub:
          if (fitsSigned12(-static_cast<int64_t>(imm))) { emitI(MIOp::ADDIW, -imm); return; }
          break;
        case Opcode::And:
          if (fitsSigned12(imm)) { emitI(MIOp::ANDI, imm); return; }
          break;
        case Opcode::Or:
          if (fitsSigned12(imm)) { emitI(MIOp::ORI, imm); return; }
          break;
        case Opcode::Xor:
          if (fitsSigned12(imm)) { emitI(MIOp::XORI, imm); return; }
          break;
        case Opcode::Shl:
          if (imm >= 0 && imm < 32) { emitI(MIOp::SLLIW, imm); return; }
          break;
        case Opcode::Shr:
          if (imm >= 0 && imm < 32) { emitI(MIOp::SRAIW, imm); return; }
          break;
        case Opcode::SDiv:
        case Opcode::SRem:
          if (tryLowerSignedDivRemConst(inst, lhs, imm, dst))
            return;
          break;
        default: break;
      }
    }

    // 寄存器到寄存器形式。
    int l = materializeValue(lhs);
    int r = materializeValue(rhs);
    MIOp op;
    switch (inst->opcode()) {
      case Opcode::Add: op = MIOp::ADDW; break;
      case Opcode::Sub: op = MIOp::SUBW; break;
      case Opcode::Mul: op = MIOp::MULW; break;
      case Opcode::Mulh: {
        // CoreIR Mulh 表示 i32×i32 乘积的有符号高半部分，
        // 而 RV64 MULH 返回 64×64 乘积的高 64 位。
        int product = appendBinaryRR(MIOp::MUL, l, r, "i32.mulh.full");
        int high = appendArithmeticShift64By32(product, "i32.mulh.high");
        appendRegMove(dst, RegClass::GPR, high, RegClass::GPR);
        return;
      }
      case Opcode::SDiv: op = MIOp::DIVW; break;
      case Opcode::SRem: op = MIOp::REMW; break;
      case Opcode::And: op = MIOp::AND_; break;
      case Opcode::Or:  op = MIOp::OR_; break;
      case Opcode::Xor: op = MIOp::XOR_; break;
      case Opcode::Shl: op = MIOp::SLLW; break;
      case Opcode::Shr: op = MIOp::SRAW; break;
      default: throw std::runtime_error("unexpected int opcode");
    }
    auto mi = std::make_unique<MachineInst>(op);
    mi->addOperand(MOperand::Def(dst, RegClass::GPR));
    mi->addOperand(MOperand::Use(l, RegClass::GPR));
    mi->addOperand(MOperand::Use(r, RegClass::GPR));
    curMBB_->append(std::move(mi));
  }

  void lowerICmp(Instruction* inst) {
    int l = materializeValue(inst->operand(0));
    int r = materializeValue(inst->operand(1));
    int dst = ensureVReg(inst);
    std::string cond = condFromName(inst);
    // 将 RISC-V 整数比较结果生成为取值为 0 或 1 的通用寄存器。
    if (cond == "eq") {
      // dst = (l == r): sub t, l, r; sltiu dst, t, 1
      int t = curMF_->newVReg(RegClass::GPR);
      auto sub = std::make_unique<MachineInst>(MIOp::SUBW);
      sub->addOperand(MOperand::Def(t, RegClass::GPR));
      sub->addOperand(MOperand::Use(l, RegClass::GPR));
      sub->addOperand(MOperand::Use(r, RegClass::GPR));
      curMBB_->append(std::move(sub));
      auto slt = std::make_unique<MachineInst>(MIOp::SLTIU);
      slt->addOperand(MOperand::Def(dst, RegClass::GPR));
      slt->addOperand(MOperand::Use(t, RegClass::GPR));
      slt->addOperand(MOperand::Imm(1));
      curMBB_->append(std::move(slt));
    } else if (cond == "ne") {
      int t = curMF_->newVReg(RegClass::GPR);
      auto sub = std::make_unique<MachineInst>(MIOp::SUBW);
      sub->addOperand(MOperand::Def(t, RegClass::GPR));
      sub->addOperand(MOperand::Use(l, RegClass::GPR));
      sub->addOperand(MOperand::Use(r, RegClass::GPR));
      curMBB_->append(std::move(sub));
      auto slt = std::make_unique<MachineInst>(MIOp::SLTU);
      slt->addOperand(MOperand::Def(dst, RegClass::GPR));
      slt->addOperand(MOperand::Use(gpr::x0, RegClass::GPR));
      slt->addOperand(MOperand::Use(t, RegClass::GPR));
      curMBB_->append(std::move(slt));
    } else if (cond == "lt") {
      auto mi = std::make_unique<MachineInst>(MIOp::SLT);
      mi->addOperand(MOperand::Def(dst, RegClass::GPR));
      mi->addOperand(MOperand::Use(l, RegClass::GPR));
      mi->addOperand(MOperand::Use(r, RegClass::GPR));
      curMBB_->append(std::move(mi));
    } else if (cond == "gt") {
      auto mi = std::make_unique<MachineInst>(MIOp::SLT);
      mi->addOperand(MOperand::Def(dst, RegClass::GPR));
      mi->addOperand(MOperand::Use(r, RegClass::GPR));
      mi->addOperand(MOperand::Use(l, RegClass::GPR));
      curMBB_->append(std::move(mi));
    } else if (cond == "le") {
      // dst = !(l > r) = !slt(r, l)
      int t = curMF_->newVReg(RegClass::GPR);
      auto slt = std::make_unique<MachineInst>(MIOp::SLT);
      slt->addOperand(MOperand::Def(t, RegClass::GPR));
      slt->addOperand(MOperand::Use(r, RegClass::GPR));
      slt->addOperand(MOperand::Use(l, RegClass::GPR));
      curMBB_->append(std::move(slt));
      auto xo = std::make_unique<MachineInst>(MIOp::XORI);
      xo->addOperand(MOperand::Def(dst, RegClass::GPR));
      xo->addOperand(MOperand::Use(t, RegClass::GPR));
      xo->addOperand(MOperand::Imm(1));
      curMBB_->append(std::move(xo));
    } else if (cond == "ge") {
      // dst = !(l < r) = !slt(l, r)
      int t = curMF_->newVReg(RegClass::GPR);
      auto slt = std::make_unique<MachineInst>(MIOp::SLT);
      slt->addOperand(MOperand::Def(t, RegClass::GPR));
      slt->addOperand(MOperand::Use(l, RegClass::GPR));
      slt->addOperand(MOperand::Use(r, RegClass::GPR));
      curMBB_->append(std::move(slt));
      auto xo = std::make_unique<MachineInst>(MIOp::XORI);
      xo->addOperand(MOperand::Def(dst, RegClass::GPR));
      xo->addOperand(MOperand::Use(t, RegClass::GPR));
      xo->addOperand(MOperand::Imm(1));
      curMBB_->append(std::move(xo));
    } else {
      throw std::runtime_error("MIR lowering: unknown icmp cond '" + cond + "'");
    }
  }

  void lowerFCmp(Instruction* inst) {
    int l = materializeFloatOperand(inst->operand(0));
    int r = materializeFloatOperand(inst->operand(1));
    int dst = ensureVReg(inst);
    std::string cond = condFromName(inst);
    auto emit3 = [&](MIOp op, int a, int b) {
      auto mi = std::make_unique<MachineInst>(op);
      mi->addOperand(MOperand::Def(dst, RegClass::GPR));
      mi->addOperand(MOperand::Use(a, RegClass::FPR));
      mi->addOperand(MOperand::Use(b, RegClass::FPR));
      curMBB_->append(std::move(mi));
    };
    if (cond == "eq")      emit3(MIOp::FEQ_S, l, r);
    else if (cond == "lt") emit3(MIOp::FLT_S, l, r);
    else if (cond == "le") emit3(MIOp::FLE_S, l, r);
    else if (cond == "gt") emit3(MIOp::FLT_S, r, l);
    else if (cond == "ge") emit3(MIOp::FLE_S, r, l);
    else if (cond == "ne") {
      int t = curMF_->newVReg(RegClass::GPR);
      auto eq = std::make_unique<MachineInst>(MIOp::FEQ_S);
      eq->addOperand(MOperand::Def(t, RegClass::GPR));
      eq->addOperand(MOperand::Use(l, RegClass::FPR));
      eq->addOperand(MOperand::Use(r, RegClass::FPR));
      curMBB_->append(std::move(eq));
      auto xo = std::make_unique<MachineInst>(MIOp::XORI);
      xo->addOperand(MOperand::Def(dst, RegClass::GPR));
      xo->addOperand(MOperand::Use(t, RegClass::GPR));
      xo->addOperand(MOperand::Imm(1));
      curMBB_->append(std::move(xo));
    } else {
      throw std::runtime_error("MIR lowering: unknown fcmp cond '" + cond + "'");
    }
  }

  void lowerBr(Instruction* inst) {
    auto* succ = dynamic_cast<BasicBlock*>(inst->operand(0));
    if (!succ) throw std::runtime_error("br target is not a basic block");
    auto mi = std::make_unique<MachineInst>(MIOp::J);
    mi->addOperand(MOperand::BB(bbMap_[succ]));
    curMBB_->append(std::move(mi));
  }

  void lowerCondBr(Instruction* inst) {
    auto* tbb = dynamic_cast<BasicBlock*>(inst->operand(1));
    auto* fbb = dynamic_cast<BasicBlock*>(inst->operand(2));
    if (!tbb || !fbb) throw std::runtime_error("condbr targets must be blocks");
    BasicBlock* fallthroughPref = nextBlockInLayout(inst->parent());
    auto emitJump = [&](MachineBasicBlock* target) {
      auto j = std::make_unique<MachineInst>(MIOp::J);
      j->addOperand(MOperand::BB(target));
      curMBB_->append(std::move(j));
    };

    auto emitIntBranch = [&](const std::string& cond,
                             int lhs,
                             int rhs,
                             MachineBasicBlock* target) {
      MIOp op;
      if (cond == "eq") op = MIOp::BEQ;
      else if (cond == "ne") op = MIOp::BNE;
      else if (cond == "lt") op = MIOp::BLT;
      else if (cond == "le") {
        op = MIOp::BGE;
        std::swap(lhs, rhs);
      } else if (cond == "gt") {
        op = MIOp::BLT;
        std::swap(lhs, rhs);
      } else if (cond == "ge") op = MIOp::BGE;
      else throw std::runtime_error("unknown icmp condition '" + cond + "'");
      auto br = std::make_unique<MachineInst>(op);
      br->addOperand(MOperand::Use(lhs, RegClass::GPR));
      br->addOperand(MOperand::Use(rhs, RegClass::GPR));
      br->addOperand(MOperand::BB(target));
      curMBB_->append(std::move(br));
    };

    MachineBasicBlock* trueTarget = bbMap_[tbb];
    MachineBasicBlock* falseTarget = bbMap_[fbb];
    bool preferTrueFallthrough = fallthroughPref == tbb && fallthroughPref != fbb;
    bool preferFalseFallthrough = fallthroughPref == fbb && fallthroughPref != tbb;

    if (auto* condInst = dynamic_cast<Instruction*>(inst->operand(0));
        condInst &&
        isDirectCondBrCompare(condInst) &&
        (condInst->opcode() == Opcode::ICmp || condInst->opcode() == Opcode::FCmp)) {
      if (condInst->opcode() == Opcode::ICmp) {
        int lhs = materializeBranchGPROperand(condInst->operand(0));
        int rhs = materializeBranchGPROperand(condInst->operand(1));
        std::string cond = condFromName(condInst);
        if (preferTrueFallthrough) {
          emitIntBranch(invertICmpCond(cond), lhs, rhs, falseTarget);
          return;
        }
        if (preferFalseFallthrough) {
          emitIntBranch(cond, lhs, rhs, trueTarget);
          return;
        }
        emitIntBranch(cond, lhs, rhs, trueTarget);
      } else {
        int lhs = materializeValue(condInst->operand(0));
        int rhs = materializeValue(condInst->operand(1));
        int cmp = newTempVReg(RegClass::GPR);
        std::string cond = condFromName(condInst);
        auto emitCmp = [&](MIOp op, int a, int b) {
          auto mi = std::make_unique<MachineInst>(op);
          mi->addOperand(MOperand::Def(cmp, RegClass::GPR));
          mi->addOperand(MOperand::Use(a, RegClass::FPR));
          mi->addOperand(MOperand::Use(b, RegClass::FPR));
          curMBB_->append(std::move(mi));
        };
        if (cond == "eq") emitCmp(MIOp::FEQ_S, lhs, rhs);
        else if (cond == "ne") emitCmp(MIOp::FEQ_S, lhs, rhs);
        else if (cond == "lt") emitCmp(MIOp::FLT_S, lhs, rhs);
        else if (cond == "le") emitCmp(MIOp::FLE_S, lhs, rhs);
        else if (cond == "gt") emitCmp(MIOp::FLT_S, rhs, lhs);
        else if (cond == "ge") emitCmp(MIOp::FLE_S, rhs, lhs);
        else throw std::runtime_error("unknown fcmp condition '" + cond + "'");

        auto emitZeroBranch = [&](MIOp op, MachineBasicBlock* target) {
          auto br = std::make_unique<MachineInst>(op);
          br->addOperand(MOperand::Use(cmp, RegClass::GPR));
          br->addOperand(MOperand::BB(target));
          curMBB_->append(std::move(br));
        };
        if (preferTrueFallthrough) {
          if (cond == "ne") emitZeroBranch(MIOp::BNEZ, falseTarget);
          else emitZeroBranch(MIOp::BEQZ, falseTarget);
          return;
        }
        if (cond == "ne") emitZeroBranch(MIOp::BEQZ, trueTarget);
        else emitZeroBranch(MIOp::BNEZ, trueTarget);
      }
    } else {
      int cond = materializeValue(inst->operand(0));
      auto br = std::make_unique<MachineInst>(MIOp::BNEZ);
      br->addOperand(MOperand::Use(cond, RegClass::GPR));
      if (preferTrueFallthrough) {
        br->setOpcode(MIOp::BEQZ);
        br->addOperand(MOperand::BB(falseTarget));
      } else {
        br->addOperand(MOperand::BB(trueTarget));
      }
      curMBB_->append(std::move(br));
      if (!preferTrueFallthrough)
        emitJump(falseTarget);
      return;
    }
    if (!preferTrueFallthrough)
      emitJump(falseTarget);
  }

  void lowerRet(Instruction* inst) {
    RegClass retClass = RegClass::GPR;
    bool hasRetVal = inst->numOperands() > 0;
    if (inst->numOperands() > 0) {
      Value* rv = inst->operand(0);
      if (rv->type().isF32()) {
        retClass = RegClass::FPR;
        materializeValueToFPR(rv, fpr::fa0);
      } else {
        materializeValueToGPR(rv, gpr::a0);
      }
    }
    auto ret = std::make_unique<MachineInst>(MIOp::RET);
    ret->addOperand(MOperand::Use(gpr::ra, RegClass::GPR));
    if (hasRetVal) {
      if (retClass == RegClass::FPR) ret->addOperand(MOperand::Use(fpr::fa0, RegClass::FPR));
      else ret->addOperand(MOperand::Use(gpr::a0, RegClass::GPR));
    }
    curMBB_->append(std::move(ret));
  }

  void lowerCall(Instruction* inst) {
    if (tryLowerInternalModMulCall(inst))
      return;
    if (tryLowerMemsetZeroCall(inst))
      return;
    curMF_->hasCalls = true;
    const int n = inst->numOperands();
    std::vector<Type> argTypes;
    argTypes.reserve(n);
    for (int i = 0; i < n; ++i) {
      argTypes.push_back(inst->operand(i)->type());
    }
    ABIArgLayout argLayout = assignABIArgLayout(argTypes);
    // 普通 ABI 寄存器实参统一交给 RA 后的 parallel-copy 消解。函数是否为入口、
    // 调用所在块是否接近循环以及符号名称都不影响合法性或 lowering 形态。
    // simple-RA 调试路径不支持该伪指令，保留旧的全暂存回退。
    bool canUseParallelCopies =
        sycEnvironmentValue("MIR_SIMPLE_RA") == nullptr;
    for (const ABIArgLoc& loc : argLayout.locs) {
      if (loc.kind == ABIArgLocKind::IntReg && loc.isFloat) {
        canUseParallelCopies = false;
        break;
      }
    }

    if (canUseParallelCopies) {
      const int outgoingBytes = argLayout.stackSlotsUsed * 8;
      if (outgoingBytes > curMF_->outgoingArgBytes)
        curMF_->outgoingArgBytes = outgoingBytes;
      auto copies = std::make_unique<MachineInst>(MIOp::PHI_COPY);
      std::vector<std::pair<Value*, int>> directGPRArgs;
      for (int i = 0; i < n; ++i) {
        Value* arg = inst->operand(i);
        const ABIArgLoc& loc = argLayout.locs[i];
        RegClass cls = arg->type().isF32() ? RegClass::FPR : RegClass::GPR;
        if (loc.kind == ABIArgLocKind::Stack) {
          int src = materializeValue(arg);
          appendStoreToStack(src, cls,
                             cls == RegClass::FPR ? MIOp::FSW : MIOp::SD,
                             loc.index * 8);
          continue;
        }
        int dst = loc.kind == ABIArgLocKind::FloatReg
                      ? kFaReg[loc.index]
                      : kAReg[loc.index];

        bool directGPR = dynamic_cast<ConstInt*>(arg) != nullptr ||
                         dynamic_cast<GlobalVar*>(arg) != nullptr;
        Value* rootBase = nullptr;
        int rootOffset = 0;
        if (!directGPR && cls == RegClass::GPR &&
            splitConstantPtrOffset(arg, rootBase, rootOffset)) {
          directGPR = dynamic_cast<GlobalVar*>(rootBase) != nullptr;
          if (auto* allocaInst = dynamic_cast<Instruction*>(rootBase))
            directGPR |= allocaFrame_.count(allocaInst) != 0;
        }
        if (directGPR && cls == RegClass::GPR) {
          directGPRArgs.push_back({arg, dst});
          continue;
        }

        int src = materializeValue(arg);
        copies->addOperand(MOperand::Def(dst, cls));
        copies->addOperand(MOperand::Use(src, cls));
      }
      if (copies->numOperands() != 0) {
        copies->comment = "call-args";
        curMBB_->append(std::move(copies));
      }
      for (const auto& direct : directGPRArgs)
        materializeValueToGPR(direct.first, direct.second);
    } else {
      const int stageBytes = n * 8;
      if (stageBytes > curMF_->outgoingArgBytes)
        curMF_->outgoingArgBytes = stageBytes;
      for (int i = 0; i < n; ++i) {
        Value* arg = inst->operand(i);
        int off = i * 8;
        if (arg->type().isF32()) {
          materializeValueToFPR(arg, fpr::ft10);
          appendStoreToStack(fpr::ft10, RegClass::FPR, MIOp::FSW, off);
        } else {
          materializeValueToGPR(arg, gpr::t5);
          appendStoreToStack(gpr::t5, RegClass::GPR, MIOp::SD, off);
        }
      }

      for (int i = 0; i < n; ++i) {
        const ABIArgLoc& loc = argLayout.locs[i];
        if (loc.kind == ABIArgLocKind::FloatReg) {
          int off = i * 8;
          appendLoadFromStack(kFaReg[loc.index], RegClass::FPR, MIOp::FLW, off);
        } else if (loc.kind == ABIArgLocKind::IntReg) {
          int off = i * 8;
          appendLoadFromStack(kAReg[loc.index], RegClass::GPR,
                              loc.isFloat ? MIOp::LW : MIOp::LD, off);
        }
      }

      for (int i = 0; i < n; ++i) {
        const ABIArgLoc& loc = argLayout.locs[i];
        if (loc.kind != ABIArgLocKind::Stack)
          continue;
        int srcOff = i * 8;
        int dstOff = loc.index * 8;
        if (srcOff == dstOff)
          continue;
        if (loc.isFloat) {
          appendLoadFromStack(fpr::ft10, RegClass::FPR, MIOp::FLW, srcOff);
          appendStoreToStack(fpr::ft10, RegClass::FPR, MIOp::FSW, dstOff);
        } else {
          appendLoadFromStack(gpr::t5, RegClass::GPR, MIOp::LD, srcOff);
          appendStoreToStack(gpr::t5, RegClass::GPR, MIOp::SD, dstOff);
        }
      }
    }
    auto call = std::make_unique<MachineInst>(MIOp::JAL);
    call->addOperand(MOperand::Sym(inst->name()));
    addCallArgUses(*call, argLayout);
    addCallClobberDefs(*call);
    curMBB_->append(std::move(call));

    if (!inst->type().isVoid()) {
      int dst = ensureVReg(inst);
      if (inst->type().isF32()) {
        auto mv = std::make_unique<MachineInst>(MIOp::FMV_S);
        mv->addOperand(MOperand::Def(dst, RegClass::FPR));
        mv->addOperand(MOperand::Use(fpr::fa0, RegClass::FPR));
        curMBB_->append(std::move(mv));
      } else {
        auto mv = std::make_unique<MachineInst>(MIOp::MV);
        mv->addOperand(MOperand::Def(dst, RegClass::GPR));
        mv->addOperand(MOperand::Use(gpr::a0, RegClass::GPR));
        curMBB_->append(std::move(mv));
      }
    }
  }

  void lowerGEP(Instruction* inst) {
    // AST 降低阶段已经把 CoreIR 中的 GEP 索引换算为字节偏移。
    int64_t constOffset = 0;
    std::vector<Value*> dynamicIndices;
    bool hasDynamicIndex = false;
    for (int i = 1; i < inst->numOperands(); ++i) {
      Value* op = inst->operand(i);
      if (auto* ci = dynamic_cast<ConstInt*>(op)) {
        constOffset += ci->value();
        continue;
      }
      hasDynamicIndex = true;
      dynamicIndices.push_back(op);
    }

    int dst = ensureVReg(inst);
    if (!hasDynamicIndex && tryLowerDirectConstAddr(inst->operand(0), constOffset, dst))
      return;

    int base = materializeValue(inst->operand(0));
    if (!hasDynamicIndex) {
      if (constOffset != 0) {
        appendAddrAdd(dst, base, constOffset);
      } else if (dst != base) {
        appendRegMove(dst, RegClass::GPR, base, RegClass::GPR);
      }
      return;
    }

    int cur = base;
    for (size_t i = 0; i < dynamicIndices.size(); ++i) {
      Value* op = dynamicIndices[i];
      int idx = materializeValue(op);
      bool isLastDynamic = i + 1 == dynamicIndices.size();
      int nxt = (isLastDynamic && constOffset == 0)
                    ? dst
                    : newTempVReg(RegClass::GPR);
      auto add = std::make_unique<MachineInst>(MIOp::ADD);
      add->addOperand(MOperand::Def(nxt, RegClass::GPR));
      add->addOperand(MOperand::Use(cur, RegClass::GPR));
      add->addOperand(MOperand::Use(idx, RegClass::GPR));
      curMBB_->append(std::move(add));
      cur = nxt;
    }
    if (constOffset != 0) {
      appendAddrAdd(dst, cur, constOffset);
      cur = dst;
    }
    // 将 GEP 结果虚拟寄存器绑定到最终地址。
    if (dst != cur) {
      appendRegMove(dst, RegClass::GPR, cur, RegClass::GPR);
    }
  }

  void lowerNeg(Instruction* inst) {
    int s = materializeValue(inst->operand(0));
    int dst = ensureVReg(inst);
    auto mi = std::make_unique<MachineInst>(MIOp::SUBW);
    mi->addOperand(MOperand::Def(dst, RegClass::GPR));
    mi->addOperand(MOperand::Use(gpr::x0, RegClass::GPR));
    mi->addOperand(MOperand::Use(s, RegClass::GPR));
    curMBB_->append(std::move(mi));
  }

  void lowerFNeg(Instruction* inst) {
    int s = materializeFloatOperand(inst->operand(0));
    int dst = ensureVReg(inst);
    auto mi = std::make_unique<MachineInst>(MIOp::FNEG_S);
    mi->addOperand(MOperand::Def(dst, RegClass::FPR));
    mi->addOperand(MOperand::Use(s, RegClass::FPR));
    curMBB_->append(std::move(mi));
  }

  void lowerBitCast(Instruction* inst) {
    int s = materializeValue(inst->operand(0));
    int dst = ensureVReg(inst);
    if (inst->type().isPtr() || inst->operand(0)->type().isPtr()) {
      appendRegMove(dst, RegClass::GPR, s, RegClass::GPR);
    } else if (inst->type().isF32()) {
      auto mi = std::make_unique<MachineInst>(MIOp::FMV_W_X);
      mi->addOperand(MOperand::Def(dst, RegClass::FPR));
      mi->addOperand(MOperand::Use(s, RegClass::GPR));
      curMBB_->append(std::move(mi));
    } else {
      auto mi = std::make_unique<MachineInst>(MIOp::FMV_X_W);
      mi->addOperand(MOperand::Def(dst, RegClass::GPR));
      mi->addOperand(MOperand::Use(s, RegClass::FPR));
      curMBB_->append(std::move(mi));
    }
  }

  void lowerSIToFP(Instruction* inst) {
    int s = materializeValue(inst->operand(0));
    int dst = ensureVReg(inst);
    auto mi = std::make_unique<MachineInst>(MIOp::FCVT_S_W);
    mi->addOperand(MOperand::Def(dst, RegClass::FPR));
    mi->addOperand(MOperand::Use(s, RegClass::GPR));
    curMBB_->append(std::move(mi));
  }

  void lowerFPToSI(Instruction* inst) {
    int s = materializeFloatOperand(inst->operand(0));
    int dst = ensureVReg(inst);
    auto mi = std::make_unique<MachineInst>(MIOp::FCVT_W_S);
    mi->addOperand(MOperand::Def(dst, RegClass::GPR));
    mi->addOperand(MOperand::Use(s, RegClass::FPR));
    curMBB_->append(std::move(mi));
  }

  void lowerMov(Instruction* inst) {
    int dst = ensureVReg(inst);
    if (inst->type().isF32()) {
      materializeValueToFPR(inst->operand(0), dst);
    } else {
      materializeValueToGPR(inst->operand(0), dst);
    }
  }

  // Select(cond, trueVal, falseVal) 的结果为 cond ? trueVal : falseVal。
  // 这里使用按位混合：先把条件转换为 0/1，再构造全零或全一掩码，
  // 分别屏蔽两个候选值，最后合并得到结果。
  void lowerSelect(Instruction* inst) {
    int dst = ensureVReg(inst);
    auto* trueConst = dynamic_cast<ConstInt*>(inst->operand(1));
    auto* falseConst = dynamic_cast<ConstInt*>(inst->operand(2));

    // cond 若是已知 0/1 值与零的 eq/ne，直接从原布尔值构造
    // 掩码：eq(x,0) 的全宽掩码是 x-1，ne(x,0) 则是 -x。
    // 这个重写只使用位语义，不依赖值分布。
    auto* compare = dynamic_cast<Instruction*>(inst->operand(0));
    if (falseConst && falseConst->value() == 0 && compare &&
        compare->opcode() == Opcode::ICmp && compare->numOperands() == 2) {
      Value* booleanValue = nullptr;
      if (auto* rhs = dynamic_cast<ConstInt*>(compare->operand(1));
          rhs && rhs->value() == 0 &&
          isKnownI32Boolean(compare->operand(0))) {
        booleanValue = compare->operand(0);
      } else if (auto* lhs = dynamic_cast<ConstInt*>(compare->operand(0));
                 lhs && lhs->value() == 0 &&
                 isKnownI32Boolean(compare->operand(1))) {
        booleanValue = compare->operand(1);
      }
      const std::string predicate = condFromName(compare);
      if (booleanValue && (predicate == "eq" || predicate == "ne")) {
        const int booleanReg = materializeValue(booleanValue);
        const int maskReg = predicate == "eq"
                                ? appendBinaryRI(MIOp::ADDIW, booleanReg, -1,
                                                 "sel.eq-zero-mask")
                                : appendBinaryRR(MIOp::SUBW, gpr::x0,
                                                 booleanReg,
                                                 "sel.ne-zero-mask");
        const int trueReg = materializeValue(inst->operand(1));
        auto selected = std::make_unique<MachineInst>(MIOp::AND_);
        selected->addOperand(MOperand::Def(dst, RegClass::GPR));
        selected->addOperand(MOperand::Use(trueReg, RegClass::GPR));
        selected->addOperand(MOperand::Use(maskReg, RegClass::GPR));
        selected->comment = "select.boolean-zero";
        curMBB_->append(std::move(selected));
        return;
      }
    }

    int condReg = materializeValue(inst->operand(0));

    // mask = (cond != 0) ? 1 : 0
    int maskReg = appendBinaryRR(MIOp::SLTU, gpr::x0, condReg, "sel.mask");
    // negMask = -mask（mask 为 1 时全一，否则全零）
    int negMaskReg = appendBinaryRR(MIOp::SUBW, gpr::x0, maskReg, "sel.negmask");

    if (falseConst && falseConst->value() == 0) {
      // select(cond, value, 0) == value & -bool(cond)。不物化零值，
      // 也不生成无用的反掩码和 OR。
      int trueReg = materializeValue(inst->operand(1));
      auto mi = std::make_unique<MachineInst>(MIOp::AND_);
      mi->addOperand(MOperand::Def(dst, RegClass::GPR));
      mi->addOperand(MOperand::Use(trueReg, RegClass::GPR));
      mi->addOperand(MOperand::Use(negMaskReg, RegClass::GPR));
      mi->comment = "select.true-or-zero";
      curMBB_->append(std::move(mi));
      return;
    }

    // invMask = ~negMask
    int invMaskReg = appendBinaryRI(MIOp::XORI, negMaskReg, -1, "sel.invmask");
    if (trueConst && trueConst->value() == 0) {
      // select(cond, 0, value) == value & ~-bool(cond)。
      int falseReg = materializeValue(inst->operand(2));
      auto mi = std::make_unique<MachineInst>(MIOp::AND_);
      mi->addOperand(MOperand::Def(dst, RegClass::GPR));
      mi->addOperand(MOperand::Use(falseReg, RegClass::GPR));
      mi->addOperand(MOperand::Use(invMaskReg, RegClass::GPR));
      mi->comment = "select.zero-or-false";
      curMBB_->append(std::move(mi));
      return;
    }

    int trueReg = materializeValue(inst->operand(1));
    int falseReg = materializeValue(inst->operand(2));
    // truePart = trueVal & negMask
    int truePartReg = appendBinaryRR(MIOp::AND_, trueReg, negMaskReg, "sel.true");
    // falsePart = falseVal & invMask
    int falsePartReg = appendBinaryRR(MIOp::AND_, falseReg, invMaskReg, "sel.false");
    // result = truePart | falsePart
    auto mi = std::make_unique<MachineInst>(MIOp::OR_);
    mi->addOperand(MOperand::Def(dst, RegClass::GPR));
    mi->addOperand(MOperand::Use(truePartReg, RegClass::GPR));
    mi->addOperand(MOperand::Use(falsePartReg, RegClass::GPR));
    curMBB_->append(std::move(mi));
  }
};

} // namespace

std::unique_ptr<MachineModule> lowerToMIR(const core_ir::Module& mod) {
  auto mm = std::make_unique<MachineModule>();
  Lowering(mod, *mm).run();
  return mm;
}

} // namespace mir
