#include "research_switch.h"
// 文件职责：提供 CoreIR pass 共享的指令、控制流和验证辅助函数。
#include "passes_utils.h"

namespace core_ir {

std::string typeName(Type ty) {
  if (ty.isI32()) return "i32";
  if (ty.isF32()) return "f32";
  if (ty.isVoid()) return "void";
  if (ty.isPtr()) return "ptr";
  if (ty.isLabel()) return "label";
  if (ty.isV4I32()) return "v4i32";
  if (ty.isV4F32()) return "v4f32";
  return "unknown";
}

void fail(const std::string& msg) {
  throw std::runtime_error("CoreIR verify failed: " + msg);
}

void verifyLiveOperandsForDebug(const Module& mod, const char* stage) {
  if (!sycEnvironmentValue("COREIR_VERIFY_LIVE"))
    return;
  for (auto& func : mod.functions()) {
    for (auto& bb : func->blocks()) {
      for (auto& instPtr : bb->instructions()) {
        Instruction* inst = instPtr.get();
        for (int i = 0; i < inst->numOperands(); ++i) {
          Value* op = inst->operand(i);
          if (!Value::isLiveValue(op)) {
            fail(std::string("dangling operand after ") + stage +
                 " func=" + func->name() +
                 " bb=" + bb->name() +
                 " inst=" + std::to_string(inst->id()) +
                 " op=" + std::to_string(i));
          }
        }
      }
    }
  }
}

bool isConstZero(Value* v) {
  if (auto* ci = dynamic_cast<ConstInt*>(v)) return ci->value() == 0;
  if (auto* cf = dynamic_cast<ConstFloat*>(v)) return cf->value() == 0.0f;
  return false;
}

bool isConstOne(Value* v) {
  if (auto* ci = dynamic_cast<ConstInt*>(v)) return ci->value() == 1;
  if (auto* cf = dynamic_cast<ConstFloat*>(v)) return cf->value() == 1.0f;
  return false;
}

ConstInt* constIntValue(Value* v) {
  return dynamic_cast<ConstInt*>(v);
}

Instruction* sremWithConstMod(Value* v, int32_t& modOut) {
  auto* inst = dynamic_cast<Instruction*>(v);
  if (!inst || inst->opcode() != Opcode::SRem || inst->numOperands() != 2)
    return nullptr;
  auto* mod = constIntValue(inst->operand(1));
  if (!mod || mod->value() == 0)
    return nullptr;
  modOut = mod->value();
  return inst;
}

std::string cmpCond(Instruction* inst) {
  auto name = inst->name();
  auto pos = name.find('_');
  return pos == std::string::npos ? name : name.substr(0, pos);
}

int evalIntCmp(const std::string& cond, int32_t lhs, int32_t rhs) {
  if (cond == "eq") return lhs == rhs;
  if (cond == "ne") return lhs != rhs;
  if (cond == "lt") return lhs < rhs;
  if (cond == "le") return lhs <= rhs;
  if (cond == "gt") return lhs > rhs;
  if (cond == "ge") return lhs >= rhs;
  return 0;
}

int evalFloatCmp(const std::string& cond, float lhs, float rhs) {
  if (cond == "eq") return lhs == rhs;
  if (cond == "ne") return lhs != rhs;
  if (cond == "lt") return lhs < rhs;
  if (cond == "le") return lhs <= rhs;
  if (cond == "gt") return lhs > rhs;
  if (cond == "ge") return lhs >= rhs;
  return 0;
}

bool hasHIRLoopMemoryCandidates(Module& mod, analysis::AnalysisManager& am) {
  for (auto& funcPtr : mod.functions()) {
    auto* func = funcPtr.get();
    if (!func || !func->entryBlock())
      continue;
    analysis::FunctionAnalysisCache& cache = am.function(func);
    if (!cache.affineRegions().empty())
      return true;
  }
  return false;
}

bool rangeWithinI32(int64_t minValue, int64_t maxValue) {
  return minValue >= std::numeric_limits<int32_t>::min() &&
         maxValue <= std::numeric_limits<int32_t>::max() &&
         minValue <= maxValue;
}

// 对 i32 表达式做保守值域推断：仅处理 And/SRem/Add/Sub，递归有深度上限，
// 结果越出 i32 范围时返回空区间，供比较折叠等下游判断使用。
KnownIntRange knownIntRange(Value* value, int depth) {
  if (!value || depth > 6)
    return {};
  if (auto* ci = dynamic_cast<ConstInt*>(value))
    return {true, ci->value(), ci->value()};

  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !inst->type().isI32())
    return {};

  switch (inst->opcode()) {
  case Opcode::Phi: {
    KnownIntRange merged;
    for (int index = 0; index + 1 < inst->numOperands(); index += 2) {
      KnownIntRange incoming =
          knownIntRange(inst->operand(index), depth + 1);
      if (!incoming.valid)
        return {};
      if (!merged.valid) {
        merged = incoming;
      } else {
        merged.minValue = std::min(merged.minValue, incoming.minValue);
        merged.maxValue = std::max(merged.maxValue, incoming.maxValue);
      }
    }
    return merged;
  }
  case Opcode::And: {
    if (auto* rhs = dynamic_cast<ConstInt*>(inst->operand(1))) {
      if (rhs->value() >= 0)
        return {true, 0, rhs->value()};
    }
    if (auto* lhs = dynamic_cast<ConstInt*>(inst->operand(0))) {
      if (lhs->value() >= 0)
        return {true, 0, lhs->value()};
    }
    break;
  }
  case Opcode::SRem: {
    auto* rhs = dynamic_cast<ConstInt*>(inst->operand(1));
    if (!rhs || rhs->value() <= 1)
      break;
    KnownIntRange lhsRange = knownIntRange(inst->operand(0), depth + 1);
    if (lhsRange.valid && lhsRange.minValue >= 0)
      return {true, 0, rhs->value() - 1};
    return {true, -(static_cast<int64_t>(rhs->value()) - 1),
            static_cast<int64_t>(rhs->value()) - 1};
  }
  case Opcode::Add:
  case Opcode::Sub: {
    KnownIntRange lhs = knownIntRange(inst->operand(0), depth + 1);
    KnownIntRange rhs = knownIntRange(inst->operand(1), depth + 1);
    if (!lhs.valid || !rhs.valid)
      break;
    int64_t minValue = 0;
    int64_t maxValue = 0;
    if (inst->opcode() == Opcode::Add) {
      minValue = lhs.minValue + rhs.minValue;
      maxValue = lhs.maxValue + rhs.maxValue;
    } else {
      minValue = lhs.minValue - rhs.maxValue;
      maxValue = lhs.maxValue - rhs.minValue;
    }
    if (rangeWithinI32(minValue, maxValue))
      return {true, minValue, maxValue};
    break;
  }
  default:
    break;
  }
  return {};
}

// 递归求值纯常量 SSA 表达式（Add/Sub/Mul/Shl/Shr/Neg）：任一操作数
// 无法归约为常量即失败；移位量限 0..30 以避免未定义行为。
bool evaluateConstIntExpression(Value* value, int64_t& out, int depth) {
  if (!value || depth > 8)
    return false;
  if (auto* ci = dynamic_cast<ConstInt*>(value)) {
    out = ci->value();
    return true;
  }
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !inst->type().isI32())
    return false;

  int64_t lhs = 0;
  int64_t rhs = 0;
  switch (inst->opcode()) {
  case Opcode::Add:
    if (evaluateConstIntExpression(inst->operand(0), lhs, depth + 1) &&
        evaluateConstIntExpression(inst->operand(1), rhs, depth + 1)) {
      out = lhs + rhs;
      return true;
    }
    break;
  case Opcode::Sub:
    if (evaluateConstIntExpression(inst->operand(0), lhs, depth + 1) &&
        evaluateConstIntExpression(inst->operand(1), rhs, depth + 1)) {
      out = lhs - rhs;
      return true;
    }
    break;
  case Opcode::Mul:
    if (evaluateConstIntExpression(inst->operand(0), lhs, depth + 1) &&
        evaluateConstIntExpression(inst->operand(1), rhs, depth + 1)) {
      out = lhs * rhs;
      return true;
    }
    break;
  case Opcode::Shl:
    if (evaluateConstIntExpression(inst->operand(0), lhs, depth + 1) &&
        evaluateConstIntExpression(inst->operand(1), rhs, depth + 1) &&
        rhs >= 0 && rhs <= 30) {
      out = lhs << rhs;
      return true;
    }
    break;
  case Opcode::Shr:
    if (evaluateConstIntExpression(inst->operand(0), lhs, depth + 1) &&
        evaluateConstIntExpression(inst->operand(1), rhs, depth + 1) &&
        rhs >= 0 && rhs <= 30) {
      out = lhs >> rhs;
      return true;
    }
    break;
  case Opcode::Neg:
    if (evaluateConstIntExpression(inst->operand(0), lhs, depth + 1)) {
      out = -lhs;
      return true;
    }
    break;
  default:
    break;
  }
  return false;
}

bool isScalarAlloca(Value* v) {
  auto* inst = dynamic_cast<Instruction*>(v);
  return inst && inst->opcode() == Opcode::Alloca && !inst->isArrayObject() &&
         inst->memorySizeBytes() <= 8;
}

} // namespace core_ir
