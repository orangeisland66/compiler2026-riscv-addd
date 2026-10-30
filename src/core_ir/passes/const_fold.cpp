// 文件职责：计算编译期可确定的常量表达式并替换原始指令。
#include "const_fold.h"
#include "passes_utils.h"
#include "pass_registry.h"

namespace core_ir {

namespace {


// 内部辅助函数 — 仅在本文件中使用

// 用一侧操作数的保守整数范围静态判定比较结果：rangeOnLhs 指示范围
// 位于比较左侧还是右侧；可判定时返回 0/1 常量，否则返回 nullptr。
Value* foldCmpWithKnownRange(Module& mod, const std::string& cond,
                             const KnownIntRange& range, int32_t rhs,
                             bool rangeOnLhs) {
  if (!range.valid)
    return nullptr;
  const int64_t c = rhs;
  bool known = false;
  bool result = false;
  if (range.minValue == range.maxValue) {
    known = true;
    result = rangeOnLhs ? evalIntCmp(cond, static_cast<int32_t>(range.minValue), rhs)
                        : evalIntCmp(cond, rhs, static_cast<int32_t>(range.minValue));
  } else if (rangeOnLhs) {
    if (cond == "lt") {
      if (range.maxValue < c) known = result = true;
      else if (range.minValue >= c) known = true;
    } else if (cond == "le") {
      if (range.maxValue <= c) known = result = true;
      else if (range.minValue > c) known = true;
    } else if (cond == "gt") {
      if (range.minValue > c) known = result = true;
      else if (range.maxValue <= c) known = true;
    } else if (cond == "ge") {
      if (range.minValue >= c) known = result = true;
      else if (range.maxValue < c) known = true;
    } else if (cond == "eq") {
      if (c < range.minValue || c > range.maxValue) known = true;
    } else if (cond == "ne") {
      if (c < range.minValue || c > range.maxValue)
        known = result = true;
    }
  } else {
    if (cond == "lt") {
      if (c < range.minValue) known = result = true;
      else if (c >= range.maxValue) known = true;
    } else if (cond == "le") {
      if (c <= range.minValue) known = result = true;
      else if (c > range.maxValue) known = true;
    } else if (cond == "gt") {
      if (c > range.maxValue) known = result = true;
      else if (c <= range.minValue) known = true;
    } else if (cond == "ge") {
      if (c >= range.maxValue) known = result = true;
      else if (c < range.minValue) known = true;
    } else if (cond == "eq") {
      if (c < range.minValue || c > range.maxValue) known = true;
    } else if (cond == "ne") {
      if (c < range.minValue || c > range.maxValue)
        known = result = true;
    }
  }
  if (!known)
    return nullptr;
  // 这是 InstCombine/ValueTracking 风格的值域折叠，只依赖
  // SSA 表达式的保守整数范围。
  return mod.getConstInt(result ? 1 : 0);
}

// 对单条指令做局部常量折叠：双操作数均为常量时直接计算（除法查零、
// 移位限合法范围），否则套代数恒等式（加减 0、乘 0/1、`(x % C) % C` 等）。
Value* foldInstruction(Module& mod, Instruction* inst) {
  auto intConst = [](Value* v) -> ConstInt* { return dynamic_cast<ConstInt*>(v); };
  auto floatConst = [](Value* v) -> ConstFloat* { return dynamic_cast<ConstFloat*>(v); };

  switch (inst->opcode()) {
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Mul:
  case Opcode::SDiv:
  case Opcode::SRem:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
  case Opcode::Shl:
  case Opcode::Shr: {
    Value* lhs = inst->operand(0);
    Value* rhs = inst->operand(1);
    auto* l = intConst(lhs);
    auto* r = intConst(rhs);
    if (l && r) {
      int32_t lv = l->value();
      int32_t rv = r->value();
      switch (inst->opcode()) {
      case Opcode::Add: return mod.getConstInt(lv + rv);
      case Opcode::Sub: return mod.getConstInt(lv - rv);
      case Opcode::Mul: return mod.getConstInt(lv * rv);
      case Opcode::SDiv: if (rv != 0) return mod.getConstInt(lv / rv); break;
      case Opcode::SRem: if (rv != 0) return mod.getConstInt(lv % rv); break;
      case Opcode::And: return mod.getConstInt(lv & rv);
      case Opcode::Or: return mod.getConstInt(lv | rv);
      case Opcode::Xor: return mod.getConstInt(lv ^ rv);
      case Opcode::Shl:
        if (rv >= 0 && rv < 32)
          return mod.getConstInt(static_cast<int32_t>(
              static_cast<uint32_t>(lv) << rv));
        break;
      case Opcode::Shr:
        if (rv >= 0 && rv < 32)
          return mod.getConstInt(lv >> rv);
        break;
      default: break;
      }
    }
    // 两操作数不全为常量时，退化为按 opcode 分类的代数恒等式化简。
    if (inst->opcode() == Opcode::Add) {
      if (isConstZero(rhs)) return lhs;
      if (isConstZero(lhs)) return rhs;
    } else if (inst->opcode() == Opcode::Sub) {
      if (isConstZero(rhs)) return lhs;
    } else if (inst->opcode() == Opcode::Mul) {
      if (isConstZero(lhs) || isConstZero(rhs)) return mod.getConstInt(0);
      if (isConstOne(rhs)) return lhs;
      if (isConstOne(lhs)) return rhs;
    } else if (inst->opcode() == Opcode::SDiv) {
      if (isConstOne(rhs)) return lhs;
    } else if (inst->opcode() == Opcode::SRem) {
      if (isConstOne(rhs)) return mod.getConstInt(0);
      int32_t innerMod = 0;
      if (auto* inner = sremWithConstMod(lhs, innerMod)) {
        auto* outerMod = constIntValue(rhs);
        if (outerMod && outerMod->value() == innerMod) {
          // InstCombine 通用恒等式 `(x % C) % C == x % C`。
          // 对正负被除数都成立，因为第一次 srem 的结果绝对值已经小于 C。
          return inner;
        }
      }
    } else if (inst->opcode() == Opcode::And) {
      if (isConstZero(lhs) || isConstZero(rhs)) return mod.getConstInt(0);
    } else if (inst->opcode() == Opcode::Or) {
      if (isConstZero(lhs)) return rhs;
      if (isConstZero(rhs)) return lhs;
    } else if (inst->opcode() == Opcode::Xor) {
      if (isConstZero(lhs)) return rhs;
      if (isConstZero(rhs)) return lhs;
      if (lhs == rhs) return mod.getConstInt(0);
    } else if (inst->opcode() == Opcode::Shl ||
               inst->opcode() == Opcode::Shr) {
      if (isConstZero(rhs)) return lhs;
    }
    break;
  }

  case Opcode::FAdd:
  case Opcode::FSub:
  case Opcode::FMul:
  case Opcode::FDiv: {
    auto* l = floatConst(inst->operand(0));
    auto* r = floatConst(inst->operand(1));
    if (!l || !r) break;
    float lv = l->value();
    float rv = r->value();
    switch (inst->opcode()) {
    case Opcode::FAdd: return mod.getConstFloat(lv + rv);
    case Opcode::FSub: return mod.getConstFloat(lv - rv);
    case Opcode::FMul: return mod.getConstFloat(lv * rv);
    case Opcode::FDiv: if (rv != 0.0f) return mod.getConstFloat(lv / rv); break;
    default: break;
    }
    break;
  }

  case Opcode::ICmp: {
    auto* l = intConst(inst->operand(0));
    auto* r = intConst(inst->operand(1));
    auto cond = cmpCond(inst);
    if (auto* lhsInst = dynamic_cast<Instruction*>(inst->operand(0))) {
      bool lhsIsBool = lhsInst->opcode() == Opcode::ICmp ||
                       lhsInst->opcode() == Opcode::FCmp;
      if (lhsIsBool && r) {
        if (cond == "ne" && r->value() == 0)
          return lhsInst;
        if (cond == "eq" && r->value() == 1)
          return lhsInst;
      }
    }
    if (l && r)
      return mod.getConstInt(evalIntCmp(cond, l->value(), r->value()));
    if (r) {
      if (auto* folded = foldCmpWithKnownRange(
              mod, cond, knownIntRange(inst->operand(0)), r->value(),
              /*rangeOnLhs=*/true)) {
        return folded;
      }
    }
    if (l) {
      if (auto* folded = foldCmpWithKnownRange(
              mod, cond, knownIntRange(inst->operand(1)), l->value(),
              /*rangeOnLhs=*/false)) {
        return folded;
      }
    }
    if (r) {
      int32_t remMod = 0;
      if (sremWithConstMod(inst->operand(0), remMod) && remMod > 0) {
        // `x % C` 的有符号结果范围是 `(-C, C)`。
        // 这里仅折叠和同一正模数边界相关的恒真/恒假比较，保持完全通用。
        if (cond == "lt" && r->value() >= remMod)
          return mod.getConstInt(1);
        if (cond == "ge" && r->value() >= remMod)
          return mod.getConstInt(0);
        if (cond == "gt" && r->value() <= -remMod)
          return mod.getConstInt(1);
        if (cond == "le" && r->value() <= -remMod)
          return mod.getConstInt(0);
      }
    }
    if (l) {
      int32_t remMod = 0;
      if (sremWithConstMod(inst->operand(1), remMod) && remMod > 0) {
        // 对称处理常量在左侧的比较，仍只使用 srem 的通用值域。
        if (cond == "gt" && l->value() >= remMod)
          return mod.getConstInt(1);
        if (cond == "le" && l->value() >= remMod)
          return mod.getConstInt(0);
        if (cond == "lt" && l->value() <= -remMod)
          return mod.getConstInt(1);
        if (cond == "ge" && l->value() <= -remMod)
          return mod.getConstInt(0);
      }
    }
    if (inst->operand(0) == inst->operand(1)) {
      if (cond == "eq" || cond == "le" || cond == "ge") return mod.getConstInt(1);
      if (cond == "ne" || cond == "lt" || cond == "gt") return mod.getConstInt(0);
    }
    break;
  }

  case Opcode::FCmp: {
    auto* l = floatConst(inst->operand(0));
    auto* r = floatConst(inst->operand(1));
    if (l && r)
      return mod.getConstInt(evalFloatCmp(cmpCond(inst), l->value(), r->value()));
    break;
  }

  case Opcode::Neg:
    if (auto* v = intConst(inst->operand(0))) return mod.getConstInt(-v->value());
    break;

  case Opcode::FNeg:
    if (auto* v = floatConst(inst->operand(0))) return mod.getConstFloat(-v->value());
    break;

  case Opcode::SIToFP:
    if (auto* v = intConst(inst->operand(0)))
      return mod.getConstFloat(static_cast<float>(v->value()));
    break;

  case Opcode::FPToSI:
    if (auto* v = floatConst(inst->operand(0)))
      return mod.getConstInt(static_cast<int32_t>(v->value()));
    break;

  case Opcode::Select:
    if (auto* c = intConst(inst->operand(0))) {
      // Select(constCond, trueVal, falseVal) -> constCond ? trueVal : falseVal
      Value* result = (c->value() != 0) ? inst->operand(1) : inst->operand(2);
      if (auto* ri = intConst(result)) return ri;
      if (auto* rf = floatConst(result)) return rf;
      return result;  // 不是常量，但可以直接使用折叠后的结果
    }
    break;

  default:
    break;
  }
  return nullptr;
}


} // namespace

bool constFold(Module& mod) {
  bool changed = false;
  for (auto& func : mod.functions()) {
    for (auto& bb : func->blocks()) {
      for (auto& instPtr : bb->instructions()) {
        auto* inst = instPtr.get();
        if (inst->type().isVoid()) continue;
        if (auto* repl = foldInstruction(mod, inst)) {
          inst->replaceAllUsesWith(repl);
          changed = true;
        }
      }
    }
  }
  return changed;
}



class ConstFoldPass : public Pass {
public:
    std::string name() const override { return "ConstFold"; }
    bool run(Module& mod, analysis::AnalysisManager&) override {
        return constFold(mod);
    }
};
REGISTER_PASS(ConstFoldPass);

} // namespace core_ir
