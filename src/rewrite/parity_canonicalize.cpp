// 文件职责：把只观察奇偶性的表达式改写为直接位测试。本文件负责实现具体逻辑。
#include "parity_canonicalize.h"

#include <unordered_map>

namespace rewrite {

namespace {

using core_ir::BasicBlock;
using core_ir::ConstInt;
using core_ir::Function;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Type;
using core_ir::Value;

Instruction* makeInst(Opcode op, Type ty, const std::string& name = "") {
  return new Instruction(op, ty, name);
}

Instruction* insertBinaryBefore(BasicBlock* bb, Instruction* pos, Opcode op,
                                Type ty, Value* lhs, Value* rhs,
                                const std::string& name = "") {
  auto* inst = makeInst(op, ty, name);
  inst->addOperand(lhs);
  inst->addOperand(rhs);
  bb->insertBefore(pos, inst);
  return inst;
}

bool isConstInt(Value* v, int32_t value) {
  auto* ci = dynamic_cast<ConstInt*>(v);
  return ci && ci->value() == value;
}

Instruction* insertICmpBefore(BasicBlock* bb, Instruction* pos,
                              const std::string& cond, Value* lhs,
                              Value* rhs, const std::string& name = "") {
  std::string finalName = name.empty() ? cond + "_parity" : name;
  auto* inst = makeInst(Opcode::ICmp, Type::i32(), finalName);
  inst->addOperand(lhs);
  inst->addOperand(rhs);
  bb->insertBefore(pos, inst);
  return inst;
}

std::string cmpCond(Instruction* inst) {
  if (!inst)
    return "";
  auto name = inst->name();
  auto pos = name.find('_');
  return pos == std::string::npos ? name : name.substr(0, pos);
}

Value* stripBitcasts(Value* value) {
  while (auto* inst = dynamic_cast<Instruction*>(value)) {
    if (inst->opcode() != Opcode::BitCast || inst->numOperands() != 1)
      break;
    value = inst->operand(0);
  }
  return value;
}

bool matchShiftedSignBit(Value* value, Value*& source) {
  auto* inst = dynamic_cast<Instruction*>(stripBitcasts(value));
  if (!inst || inst->opcode() != Opcode::And || inst->numOperands() != 2 ||
      !isConstInt(inst->operand(1), 1)) {
    return false;
  }
  auto* shr = dynamic_cast<Instruction*>(stripBitcasts(inst->operand(0)));
  if (!shr || shr->opcode() != Opcode::Shr || shr->numOperands() != 2 ||
      !isConstInt(shr->operand(1), 31)) {
    return false;
  }
  source = shr->operand(0);
  return true;
}

bool matchSignedMod2Remainder(Value* value, Value*& source) {
  auto* sub = dynamic_cast<Instruction*>(stripBitcasts(value));
  if (!sub || sub->opcode() != Opcode::Sub || sub->numOperands() != 2)
    return false;

  Value* biasSource = nullptr;
  if (!matchShiftedSignBit(sub->operand(1), biasSource))
    return false;

  auto* masked = dynamic_cast<Instruction*>(stripBitcasts(sub->operand(0)));
  if (!masked || masked->opcode() != Opcode::And || masked->numOperands() != 2 ||
      !isConstInt(masked->operand(1), 1)) {
    return false;
  }

  auto* add = dynamic_cast<Instruction*>(stripBitcasts(masked->operand(0)));
  if (!add || add->opcode() != Opcode::Add || add->numOperands() != 2)
    return false;

  Value* lhs = add->operand(0);
  Value* rhs = add->operand(1);
  Value* addBiasSource = nullptr;
  if (matchShiftedSignBit(lhs, addBiasSource)) {
    if (addBiasSource != biasSource)
      return false;
    source = rhs;
    return true;
  }
  if (matchShiftedSignBit(rhs, addBiasSource)) {
    if (addBiasSource != biasSource)
      return false;
    source = lhs;
    return true;
  }
  return false;
}

Value* buildParityBit(Module& mod, BasicBlock* bb, Instruction* pos,
                      Value* value,
                      std::unordered_map<int, Value*>& memo) {
  value = stripBitcasts(value);
  if (!value)
    return nullptr;

  auto memoIt = memo.find(value->id());
  if (memoIt != memo.end())
    return memoIt->second;

  if (auto* ci = dynamic_cast<ConstInt*>(value)) {
    Value* bit = mod.getConstInt(ci->value() & 1);
    memo[value->id()] = bit;
    return bit;
  }

  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !inst->type().isI32()) {
    auto* bit = insertBinaryBefore(bb, pos, Opcode::And, Type::i32(), value,
                                   mod.getConstInt(1), "parity.and");
    memo[value->id()] = bit;
    return bit;
  }

  // 按操作码推导奇偶位：乘法用两侧奇偶位相与，加减异或用异或；取负
  // 奇偶位不变；左移正常数必得偶数（位为 0）；与/或保持原运算；无法
  // 推导时退回统一的 x & 1 兜底形式。
  Value* result = nullptr;
  switch (inst->opcode()) {
  case Opcode::Mul: {
    if (inst->numOperands() == 2) {
      Value* lhsBit = buildParityBit(mod, bb, pos, inst->operand(0), memo);
      Value* rhsBit = buildParityBit(mod, bb, pos, inst->operand(1), memo);
      if (lhsBit && rhsBit) {
        result = insertBinaryBefore(bb, pos, Opcode::And, Type::i32(),
                                    lhsBit, rhsBit, "parity.mul");
      }
    }
    break;
  }
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Xor: {
    if (inst->numOperands() == 2) {
      Value* lhsBit = buildParityBit(mod, bb, pos, inst->operand(0), memo);
      Value* rhsBit = buildParityBit(mod, bb, pos, inst->operand(1), memo);
      if (lhsBit && rhsBit) {
        result = insertBinaryBefore(bb, pos, Opcode::Xor, Type::i32(),
                                    lhsBit, rhsBit, "parity.xor");
      }
    }
    break;
  }
  case Opcode::Neg:
    if (inst->numOperands() == 1)
      result = buildParityBit(mod, bb, pos, inst->operand(0), memo);
    break;
  case Opcode::Shl: {
    if (inst->numOperands() == 2) {
      if (auto* shift = dynamic_cast<ConstInt*>(inst->operand(1))) {
        if (shift->value() > 0) {
          result = mod.getConstInt(0);
          break;
        }
      }
      Value* lhsBit = buildParityBit(mod, bb, pos, inst->operand(0), memo);
      if (lhsBit)
        result = lhsBit;
    }
    break;
  }
  case Opcode::And:
  case Opcode::Or:
    if (inst->numOperands() == 2) {
      Value* lhsBit = buildParityBit(mod, bb, pos, inst->operand(0), memo);
      Value* rhsBit = buildParityBit(mod, bb, pos, inst->operand(1), memo);
      if (lhsBit && rhsBit) {
        result = insertBinaryBefore(bb, pos, inst->opcode(), Type::i32(),
                                    lhsBit, rhsBit,
                                    inst->opcode() == Opcode::And
                                        ? "parity.and"
                                        : "parity.or");
      }
    }
    break;
  default:
    break;
  }

  if (!result) {
    result = insertBinaryBefore(bb, pos, Opcode::And, Type::i32(), value,
                                mod.getConstInt(1), "parity.and");
  }
  memo[value->id()] = result;
  return result;
}

bool rewriteParityCompare(Module& mod, BasicBlock* bb, Instruction* cmp) {
  if (!cmp || cmp->opcode() != Opcode::ICmp || cmp->numOperands() != 2)
    return false;

  std::string cond = cmpCond(cmp);
  if (!(cond == "eq" || cond == "ne"))
    return false;

  Value* expr = nullptr;
  int32_t parityTarget = 0;
  if (auto* rhs = dynamic_cast<ConstInt*>(cmp->operand(1))) {
    expr = cmp->operand(0);
    parityTarget = rhs->value();
  } else if (auto* lhs = dynamic_cast<ConstInt*>(cmp->operand(0))) {
    expr = cmp->operand(1);
    parityTarget = lhs->value();
  } else {
    return false;
  }

  if (parityTarget != 0 && parityTarget != 1)
    return false;

  expr = stripBitcasts(expr);
  auto* exprInst = dynamic_cast<Instruction*>(expr);
  if (!exprInst)
    return false;

  bool parityShape = false;
  bool signedRemainder = false;
  Value* paritySource = nullptr;
  if (exprInst->opcode() == Opcode::SRem && exprInst->numOperands() == 2 &&
      isConstInt(exprInst->operand(1), 2)) {
    parityShape = true;
    signedRemainder = true;
    paritySource = exprInst->operand(0);
  } else if (matchSignedMod2Remainder(expr, paritySource)) {
    parityShape = true;
    signedRemainder = true;
  } else if (exprInst->opcode() == Opcode::Sub && exprInst->numOperands() == 2) {
    auto* lhsInst = dynamic_cast<Instruction*>(exprInst->operand(0));
    auto* rhsInst = dynamic_cast<Instruction*>(exprInst->operand(1));
    if (lhsInst && rhsInst &&
        lhsInst->opcode() == Opcode::And &&
        rhsInst->opcode() == Opcode::And &&
        lhsInst->numOperands() == 2 && rhsInst->numOperands() == 2 &&
        isConstInt(lhsInst->operand(1), 1) &&
        isConstInt(rhsInst->operand(1), 1)) {
      parityShape = true;
      paritySource = expr;
    }
  } else if (exprInst->opcode() == Opcode::And && exprInst->numOperands() == 2 &&
             isConstInt(exprInst->operand(1), 1)) {
    parityShape = true;
    paritySource = exprInst->operand(0);
  }
  if (!parityShape)
    return false;

  std::unordered_map<int, Value*> memo;
  Value* parityBit = buildParityBit(mod, bb, cmp,
                                    paritySource ? paritySource : expr, memo);
  if (!parityBit)
    return false;

  // 有符号余数的取值为 -1、0、1。与 0 比较只取决于奇偶性；与 1
  // 比较还要求原值非负，不能把负奇数的 -1 当作 1。
  if (signedRemainder && parityTarget == 1) {
    Value* odd = insertICmpBefore(bb, cmp, "ne", parityBit,
                                  mod.getConstInt(0), "ne_parity_bit");
    Value* nonNegative = insertICmpBefore(
        bb, cmp, "ge", paritySource, mod.getConstInt(0),
        "ge_parity_source");
    Value* isPositiveRemainder = insertBinaryBefore(
        bb, cmp, Opcode::And, Type::i32(), nonNegative, odd,
        "parity.signed.one");
    Value* replacement = insertICmpBefore(
        bb, cmp, cond == "eq" ? "ne" : "eq", isPositiveRemainder,
        mod.getConstInt(0),
        cond == "eq" ? "ne_signed_parity" : "eq_signed_parity");
    cmp->replaceAllUsesWith(replacement);
    bb->remove(cmp);
    return true;
  }

  int32_t expectOdd = parityTarget;
  if (cond == "ne")
    expectOdd ^= 1;

  Value* replacement = insertICmpBefore(
      bb, cmp, expectOdd ? "ne" : "eq", parityBit, mod.getConstInt(0),
      expectOdd ? "ne_parity" : "eq_parity");
  cmp->replaceAllUsesWith(replacement);
  bb->remove(cmp);
  return true;
}

bool processFunction(Function* func) {
  if (!func)
    return false;
  auto* mod = func->parent();
  if (!mod)
    return false;

  bool changed = false;
  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!bb)
      continue;

    std::vector<Instruction*> cmps;
    for (auto& instPtr : bb->instructions()) {
      auto* inst = instPtr.get();
      if (inst && inst->opcode() == Opcode::ICmp)
        cmps.push_back(inst);
    }
    for (auto* cmp : cmps)
      changed |= rewriteParityCompare(*mod, bb, cmp);
  }
  return changed;
}

} // namespace

bool applyParityCanonicalization(Module& mod) {
  bool changed = false;
  for (auto& funcPtr : mod.functions())
    changed |= processFunction(funcPtr.get());
  return changed;
}

} // namespace rewrite
