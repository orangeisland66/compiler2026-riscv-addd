// 文件职责：执行常量强度削减，把昂贵算术替换为等价低成本形式。本文件负责实现具体逻辑。
#include "strength_reduce.h"

#include <cstdint>
#include <utility>
#include <vector>

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

uint32_t magnitude(int32_t v) {
  return v < 0 ? static_cast<uint32_t>(-static_cast<int64_t>(v))
               : static_cast<uint32_t>(v);
}

bool isPow2(uint32_t v) { return v != 0 && (v & (v - 1)) == 0; }
int log2Pow2(uint32_t v) {
  int s = 0;
  while ((uint32_t{1} << s) != v) ++s;
  return s;
}
int popcount32(uint32_t v) {
  int c = 0;
  while (v) { v &= v - 1; ++c; }
  return c;
}

Value* makeSignMask(Module& mod, BasicBlock* bb, Instruction* before, Value* x) {
  auto* shr = new Instruction(Opcode::Shr, Type::i32(), "sr.sign");
  shr->addOperand(x);
  shr->addOperand(mod.getConstInt(31));
  bb->insertBefore(before, shr);
  return shr;
}

Value* makeShl(Module& mod, BasicBlock* bb, Instruction* before, Value* x, int k) {
  if (k == 0) return x;
  auto* shl = new Instruction(Opcode::Shl, Type::i32(), "sr.shl");
  shl->addOperand(x);
  shl->addOperand(mod.getConstInt(k));
  bb->insertBefore(before, shl);
  return shl;
}

Value* makeShr(Module& mod, BasicBlock* bb, Instruction* before, Value* x, int k) {
  if (k == 0) return x;
  auto* shr = new Instruction(Opcode::Shr, Type::i32(), "sr.shr");
  shr->addOperand(x);
  shr->addOperand(mod.getConstInt(k));
  bb->insertBefore(before, shr);
  return shr;
}

Value* makeAdd(BasicBlock* bb, Instruction* before, Value* a, Value* b) {
  auto* add = new Instruction(Opcode::Add, Type::i32(), "sr.add");
  add->addOperand(a);
  add->addOperand(b);
  bb->insertBefore(before, add);
  return add;
}

Value* makeSub(BasicBlock* bb, Instruction* before, Value* a, Value* b) {
  auto* sub = new Instruction(Opcode::Sub, Type::i32(), "sr.sub");
  sub->addOperand(a);
  sub->addOperand(b);
  bb->insertBefore(before, sub);
  return sub;
}

Value* makeAnd(Module& mod, BasicBlock* bb, Instruction* before, Value* a, int32_t imm) {
  auto* andI = new Instruction(Opcode::And, Type::i32(), "sr.and");
  andI->addOperand(a);
  andI->addOperand(mod.getConstInt(imm));
  bb->insertBefore(before, andI);
  return andI;
}

Value* makeNeg(BasicBlock* bb, Instruction* before, Value* x) {
  auto* neg = new Instruction(Opcode::Neg, Type::i32(), "sr.neg");
  neg->addOperand(x);
  bb->insertBefore(before, neg);
  return neg;
}

// ---- 乘法 ----

Value* expandMulConst(Module& mod, BasicBlock* bb, Instruction* before,
                     Value* x, int32_t c) {
  if (c == 0) return mod.getConstInt(0);
  if (c == 1) return x;
  if (c == -1) return makeNeg(bb, before, x);

  uint32_t absC = magnitude(c);

  if (isPow2(absC)) {
    int k = log2Pow2(absC);
    Value* res = makeShl(mod, bb, before, x, k);
    return c < 0 ? makeNeg(bb, before, res) : res;
  }

  if (popcount32(static_cast<uint32_t>(absC)) == 2) {
    int hi = -1, lo = -1;
    for (int b = 0; b < 32; ++b) {
      if (absC & (uint32_t{1} << b)) {
        if (lo < 0) lo = b;
        else hi = b;
      }
    }
    Value* hiVal = makeShl(mod, bb, before, x, hi);
    Value* loVal = makeShl(mod, bb, before, x, lo);
    Value* sum = makeAdd(bb, before, hiVal, loVal);
    return c < 0 ? makeNeg(bb, before, sum) : sum;
  }

  {
    int firstSet = -1, lastSet = -1;
    bool contiguous = true;
    bool seenZero = false;
    for (int b = 0; b < 32; ++b) {
      bool bit = (absC & (uint32_t{1} << b)) != 0;
      if (bit) {
        if (firstSet < 0) firstSet = b;
        if (seenZero) { contiguous = false; break; }
        lastSet = b;
      } else if (firstSet >= 0) {
        seenZero = true;
      }
    }
    if (contiguous && firstSet >= 0 && lastSet > firstSet) {
      Value* hi = makeShl(mod, bb, before, x, lastSet + 1);
      Value* lo = makeShl(mod, bb, before, x, firstSet);
      Value* diff = makeSub(bb, before, hi, lo);
      return c < 0 ? makeNeg(bb, before, diff) : diff;
    }
  }

  return nullptr;
}

// ---- 按 2 的幂做有符号除法 ----

Value* expandSDivConst(Module& mod, BasicBlock* bb, Instruction* before,
                      Value* x, int32_t c) {
  if (c == 1) return x;
  if (c == -1) return makeNeg(bb, before, x);
  uint32_t absC = magnitude(c);
  if (!isPow2(absC)) return nullptr;
  int k = log2Pow2(absC);
  Value* sign = makeSignMask(mod, bb, before, x);
  Value* bias = makeAnd(
      mod, bb, before, sign,
      static_cast<int32_t>((uint32_t{1} << k) - 1));
  Value* adj = makeAdd(bb, before, x, bias);
  Value* res = makeShr(mod, bb, before, adj, k);
  return c < 0 ? makeNeg(bb, before, res) : res;
}

// ---- 按 2 的幂做有符号取余 ----

Value* expandSRemConst(Module& mod, BasicBlock* bb, Instruction* before,
                      Value* x, int32_t c) {
  uint32_t absC = magnitude(c);
  if (absC <= 1) return mod.getConstInt(0);
  if (!isPow2(absC)) return nullptr;
  int k = log2Pow2(absC);
  Value* sign = makeSignMask(mod, bb, before, x);
  const int32_t mask =
      static_cast<int32_t>((uint32_t{1} << k) - 1);
  Value* bias = makeAnd(mod, bb, before, sign, mask);
  Value* adj = makeAdd(bb, before, x, bias);
  Value* masked = makeAnd(mod, bb, before, adj, mask);
  return makeSub(bb, before, masked, bias);
}
} // namespace

bool applyConstantStrengthReduction(Module& mod) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();

    std::vector<std::pair<Instruction*, BasicBlock*>> candidates;
    for (auto& bbPtr : func->blocks()) {
      for (auto& instPtr : bbPtr->instructions()) {
        Instruction* inst = instPtr.get();
        Opcode op = inst->opcode();
        if (op != Opcode::Mul && op != Opcode::SDiv && op != Opcode::SRem) continue;
        if (inst->numOperands() != 2) continue;
        if (!inst->type().isI32()) continue;
        Value* lhs = inst->operand(0);
        Value* rhs = inst->operand(1);
        bool lhsConst = dynamic_cast<ConstInt*>(lhs) != nullptr;
        bool rhsConst = dynamic_cast<ConstInt*>(rhs) != nullptr;
        if (op == Opcode::Mul) {
          if (!lhsConst && !rhsConst) continue;
        } else {
          if (!rhsConst) continue;
        }
        candidates.push_back({inst, bbPtr.get()});
      }
    }

    for (auto& [inst, bb] : candidates) {
      Value* lhs = inst->operand(0);
      Value* rhs = inst->operand(1);
      if (inst->opcode() == Opcode::Mul) {
        auto* lc = dynamic_cast<ConstInt*>(lhs);
        auto* rc = dynamic_cast<ConstInt*>(rhs);
        if (lc && !rc) { std::swap(lhs, rhs); }
      }
      auto* rhsC = dynamic_cast<ConstInt*>(rhs);
      if (!rhsC) continue;
      int32_t c = rhsC->value();
      Value* newVal = nullptr;
      switch (inst->opcode()) {
        case Opcode::Mul:
          newVal = expandMulConst(mod, bb, inst, lhs, c);
          break;
        case Opcode::SDiv: {
          newVal = expandSDivConst(mod, bb, inst, lhs, c);
          break;
        }
        case Opcode::SRem: {
          newVal = expandSRemConst(mod, bb, inst, lhs, c);
          break;
        }
        default: break;
      }
      if (newVal && newVal != inst) {
        inst->replaceAllUsesWith(newVal);
        bb->remove(inst);
        changed = true;
      }
    }
  }
  return changed;
}

} // namespace rewrite
