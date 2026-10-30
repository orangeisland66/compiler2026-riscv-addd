// 文件职责：实现 LLVM 风格的 JumpThreading，只在线程化边上复制小型可证明路径。
#include "pass_registry.h"
#include "jump_threading.h"

#include "../../analysis/cfg_utils.h"
#include "../../analysis/function_analysis.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace core_ir {

namespace {

std::string cmpCond(Instruction* inst) {
  if (!inst)
    return "";
  auto name = inst->name();
  auto pos = name.find('_');
  return pos == std::string::npos ? name : name.substr(0, pos);
}

std::string swappedCond(const std::string& cond) {
  if (cond == "lt") return "gt";
  if (cond == "le") return "ge";
  if (cond == "gt") return "lt";
  if (cond == "ge") return "le";
  return cond;
}

std::string negatedCond(const std::string& cond) {
  if (cond == "eq") return "ne";
  if (cond == "ne") return "eq";
  if (cond == "lt") return "ge";
  if (cond == "le") return "gt";
  if (cond == "gt") return "le";
  if (cond == "ge") return "lt";
  return cond;
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

bool isConstZero(Value* value) {
  if (auto* ci = dynamic_cast<ConstInt*>(value))
    return ci->value() == 0;
  if (auto* cf = dynamic_cast<ConstFloat*>(value))
    return cf->value() == 0.0f;
  return false;
}

bool isConstOne(Value* value) {
  if (auto* ci = dynamic_cast<ConstInt*>(value))
    return ci->value() == 1;
  if (auto* cf = dynamic_cast<ConstFloat*>(value))
    return cf->value() == 1.0f;
  return false;
}

struct IntConstraint {
  bool hasExact = false;
  int32_t exact = 0;
  bool hasLower = false;
  int32_t lower = std::numeric_limits<int32_t>::min();
  bool hasUpper = false;
  int32_t upper = std::numeric_limits<int32_t>::max();
  std::vector<int32_t> notEquals;
};

struct FactState {
  bool feasible = true;
  std::unordered_map<Value*, Value*> exactValues;
  std::unordered_map<Value*, IntConstraint> intConstraints;
};

std::optional<int32_t> knownIntConstant(const FactState& state, Value* value) {
  if (auto* ci = dynamic_cast<ConstInt*>(value))
    return ci->value();
  auto exactIt = state.exactValues.find(value);
  if (exactIt != state.exactValues.end()) {
    if (auto* ci = dynamic_cast<ConstInt*>(exactIt->second))
      return ci->value();
  }
  auto factIt = state.intConstraints.find(value);
  if (factIt != state.intConstraints.end() && factIt->second.hasExact)
    return factIt->second.exact;
  return std::nullopt;
}

std::optional<float> knownFloatConstant(const FactState& state, Value* value) {
  if (auto* cf = dynamic_cast<ConstFloat*>(value))
    return cf->value();
  auto exactIt = state.exactValues.find(value);
  if (exactIt != state.exactValues.end()) {
    if (auto* cf = dynamic_cast<ConstFloat*>(exactIt->second))
      return cf->value();
  }
  return std::nullopt;
}

Value* exactReplacement(Module& mod, const FactState& state, Value* value) {
  if (!value || value->isConstant())
    return value;
  auto exactIt = state.exactValues.find(value);
  if (exactIt != state.exactValues.end())
    return exactIt->second;
  if (auto known = knownIntConstant(state, value))
    return mod.getConstInt(*known);
  if (auto known = knownFloatConstant(state, value))
    return mod.getConstFloat(*known);
  return nullptr;
}

void recordExact(Module& mod, FactState& state, Value* value, Value* constant) {
  if (!state.feasible || !value || !constant || !constant->isConstant())
    return;
  state.exactValues[value] = constant;
  if (!value->type().isI32())
    return;
  auto* ci = dynamic_cast<ConstInt*>(constant);
  if (!ci)
    return;
  IntConstraint& constraint = state.intConstraints[value];
  if (constraint.hasExact && constraint.exact != ci->value()) {
    state.feasible = false;
    return;
  }
  if (constraint.hasLower && ci->value() < constraint.lower) {
    state.feasible = false;
    return;
  }
  if (constraint.hasUpper && ci->value() > constraint.upper) {
    state.feasible = false;
    return;
  }
  if (std::find(constraint.notEquals.begin(), constraint.notEquals.end(),
                ci->value()) != constraint.notEquals.end()) {
    state.feasible = false;
    return;
  }
  constraint.hasExact = true;
  constraint.exact = ci->value();
  constraint.hasLower = true;
  constraint.lower = ci->value();
  constraint.hasUpper = true;
  constraint.upper = ci->value();
  constraint.notEquals.clear();
  state.exactValues[value] = mod.getConstInt(ci->value());
}

void updateExactFromBounds(Module& mod, FactState& state, Value* value) {
  auto it = state.intConstraints.find(value);
  if (it == state.intConstraints.end())
    return;
  IntConstraint& constraint = it->second;
  if (constraint.hasLower && constraint.hasUpper &&
      constraint.lower == constraint.upper) {
    recordExact(mod, state, value, mod.getConstInt(constraint.lower));
  }
}

void constrainLower(Module& mod, FactState& state, Value* value,
                    int64_t lower) {
  if (!state.feasible || !value || !value->type().isI32())
    return;
  if (lower > std::numeric_limits<int32_t>::max()) {
    state.feasible = false;
    return;
  }
  IntConstraint& constraint = state.intConstraints[value];
  const int32_t bound = static_cast<int32_t>(lower);
  if (constraint.hasExact) {
    if (constraint.exact < bound)
      state.feasible = false;
    return;
  }
  if (!constraint.hasLower || bound > constraint.lower) {
    constraint.hasLower = true;
    constraint.lower = bound;
  }
  if (constraint.hasUpper && constraint.lower > constraint.upper) {
    state.feasible = false;
    return;
  }
  updateExactFromBounds(mod, state, value);
}

void constrainUpper(Module& mod, FactState& state, Value* value,
                    int64_t upper) {
  if (!state.feasible || !value || !value->type().isI32())
    return;
  if (upper < std::numeric_limits<int32_t>::min()) {
    state.feasible = false;
    return;
  }
  IntConstraint& constraint = state.intConstraints[value];
  const int32_t bound = static_cast<int32_t>(upper);
  if (constraint.hasExact) {
    if (constraint.exact > bound)
      state.feasible = false;
    return;
  }
  if (!constraint.hasUpper || bound < constraint.upper) {
    constraint.hasUpper = true;
    constraint.upper = bound;
  }
  if (constraint.hasLower && constraint.lower > constraint.upper) {
    state.feasible = false;
    return;
  }
  updateExactFromBounds(mod, state, value);
}

void constrainNotEqual(FactState& state, Value* value, int32_t constant) {
  if (!state.feasible || !value || !value->type().isI32())
    return;
  IntConstraint& constraint = state.intConstraints[value];
  if (constraint.hasExact) {
    if (constraint.exact == constant)
      state.feasible = false;
    return;
  }
  if (constraint.hasLower && constraint.hasUpper &&
      (constant < constraint.lower || constant > constraint.upper)) {
    return;
  }
  if (std::find(constraint.notEquals.begin(), constraint.notEquals.end(),
                constant) == constraint.notEquals.end()) {
    if (constraint.notEquals.size() < 16)
      constraint.notEquals.push_back(constant);
  }
}

bool matchIntCompareAgainstConstant(Instruction* inst, Value*& value,
                                    int32_t& constant,
                                    std::string& condOut) {
  if (!inst || inst->opcode() != Opcode::ICmp || inst->numOperands() != 2)
    return false;
  if (auto* rhs = dynamic_cast<ConstInt*>(inst->operand(1))) {
    value = inst->operand(0);
    constant = rhs->value();
    condOut = cmpCond(inst);
    return true;
  }
  if (auto* lhs = dynamic_cast<ConstInt*>(inst->operand(0))) {
    value = inst->operand(1);
    constant = lhs->value();
    condOut = swappedCond(cmpCond(inst));
    return true;
  }
  return false;
}

bool matchFloatCompareAgainstConstant(Instruction* inst, Value*& value,
                                      float& constant,
                                      std::string& condOut) {
  if (!inst || inst->opcode() != Opcode::FCmp || inst->numOperands() != 2)
    return false;
  if (auto* rhs = dynamic_cast<ConstFloat*>(inst->operand(1))) {
    value = inst->operand(0);
    constant = rhs->value();
    condOut = cmpCond(inst);
    return true;
  }
  if (auto* lhs = dynamic_cast<ConstFloat*>(inst->operand(0))) {
    value = inst->operand(1);
    constant = lhs->value();
    condOut = swappedCond(cmpCond(inst));
    return true;
  }
  return false;
}

bool isBooleanLikeValue(Value* value, int depth = 0) {
  if (!value || depth > 6)
    return false;
  if (auto* ci = dynamic_cast<ConstInt*>(value))
    return ci->value() == 0 || ci->value() == 1;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst)
    return false;
  switch (inst->opcode()) {
  case Opcode::ICmp:
  case Opcode::FCmp:
    return true;
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
    return inst->numOperands() == 2 &&
           isBooleanLikeValue(inst->operand(0), depth + 1) &&
           isBooleanLikeValue(inst->operand(1), depth + 1);
  case Opcode::Mov:
  case Opcode::BitCast:
    return inst->numOperands() == 1 &&
           isBooleanLikeValue(inst->operand(0), depth + 1);
  default:
    return false;
  }
}

void applyIntComparisonFact(Module& mod, FactState& state, Value* value,
                            const std::string& cond, int32_t constant) {
  if (!state.feasible || !value || !value->type().isI32())
    return;
  if (cond == "eq") {
    recordExact(mod, state, value, mod.getConstInt(constant));
  } else if (cond == "ne") {
    constrainNotEqual(state, value, constant);
  } else if (cond == "lt") {
    constrainUpper(mod, state, value,
                   static_cast<int64_t>(constant) - 1);
  } else if (cond == "le") {
    constrainUpper(mod, state, value, constant);
  } else if (cond == "gt") {
    constrainLower(mod, state, value,
                   static_cast<int64_t>(constant) + 1);
  } else if (cond == "ge") {
    constrainLower(mod, state, value, constant);
  }
}

void applyFloatComparisonFact(Module& mod, FactState& state, Value* value,
                              const std::string& cond, float constant,
                              bool takeTrue) {
  if (!state.feasible || !value || !value->type().isF32())
    return;
  if ((takeTrue && cond == "eq") || (!takeTrue && cond == "ne"))
    recordExact(mod, state, value, mod.getConstFloat(constant));
}

std::optional<bool> inferBooleanTruthFromCompare(const std::string& cond,
                                                 int32_t constant,
                                                 bool takeTrue) {
  std::optional<bool> inferred;
  for (int candidate = 0; candidate <= 1; ++candidate) {
    const bool result = evalIntCmp(cond, candidate, constant) != 0;
    if (result != takeTrue)
      continue;
    if (inferred.has_value())
      return std::nullopt;
    inferred = candidate != 0;
  }
  return inferred;
}

void applyBooleanFact(Module& mod, FactState& state, Value* value, bool isTrue,
                      int depth = 0) {
  if (!state.feasible || !value || depth > 8)
    return;
  if (isBooleanLikeValue(value))
    recordExact(mod, state, value, mod.getConstInt(isTrue ? 1 : 0));

  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst)
    return;

  Value* intValue = nullptr;
  int32_t intConstant = 0;
  std::string cond = "";
  if (matchIntCompareAgainstConstant(inst, intValue, intConstant, cond)) {
    if (isBooleanLikeValue(intValue)) {
      if (auto boolTruth =
              inferBooleanTruthFromCompare(cond, intConstant, isTrue)) {
        applyBooleanFact(mod, state, intValue, *boolTruth, depth + 1);
        return;
      }
    }
    applyIntComparisonFact(mod, state, intValue,
                           isTrue ? cond : negatedCond(cond), intConstant);
    return;
  }

  Value* floatValue = nullptr;
  float floatConstant = 0.0f;
  if (matchFloatCompareAgainstConstant(inst, floatValue, floatConstant, cond)) {
    applyFloatComparisonFact(mod, state, floatValue, cond, floatConstant,
                             isTrue);
    return;
  }

  if (inst->opcode() == Opcode::And && inst->numOperands() == 2 &&
      isBooleanLikeValue(inst->operand(0)) &&
      isBooleanLikeValue(inst->operand(1))) {
    if (isTrue) {
      applyBooleanFact(mod, state, inst->operand(0), true, depth + 1);
      applyBooleanFact(mod, state, inst->operand(1), true, depth + 1);
    }
    return;
  }

  if (inst->opcode() == Opcode::Or && inst->numOperands() == 2 &&
      isBooleanLikeValue(inst->operand(0)) &&
      isBooleanLikeValue(inst->operand(1))) {
    if (!isTrue) {
      applyBooleanFact(mod, state, inst->operand(0), false, depth + 1);
      applyBooleanFact(mod, state, inst->operand(1), false, depth + 1);
    }
    return;
  }

  if (inst->opcode() == Opcode::Xor && inst->numOperands() == 2 &&
      isBooleanLikeValue(inst->operand(0)) &&
      isBooleanLikeValue(inst->operand(1))) {
    auto lhs = knownIntConstant(state, inst->operand(0));
    auto rhs = knownIntConstant(state, inst->operand(1));
    if (lhs && (*lhs == 0 || *lhs == 1)) {
      applyBooleanFact(mod, state, inst->operand(1),
                       isTrue ^ (*lhs != 0), depth + 1);
      return;
    }
    if (rhs && (*rhs == 0 || *rhs == 1)) {
      applyBooleanFact(mod, state, inst->operand(0),
                       isTrue ^ (*rhs != 0), depth + 1);
      return;
    }
    return;
  }

  if ((inst->opcode() == Opcode::Mov || inst->opcode() == Opcode::BitCast) &&
      inst->numOperands() == 1 && isBooleanLikeValue(inst->operand(0))) {
    applyBooleanFact(mod, state, inst->operand(0), isTrue, depth + 1);
  }
}

void applyBranchFacts(Module& mod, FactState& state, Value* condValue,
                      bool takeTrue) {
  if (!state.feasible || !condValue)
    return;
  if (isBooleanLikeValue(condValue))
    applyBooleanFact(mod, state, condValue, takeTrue);
  else if (auto* inst = dynamic_cast<Instruction*>(condValue);
           inst && (inst->opcode() == Opcode::ICmp ||
                    inst->opcode() == Opcode::FCmp)) {
    applyBooleanFact(mod, state, condValue, takeTrue);
  }
}

std::optional<bool> evaluateIntCompareWithFacts(const FactState& state,
                                                Instruction* inst) {
  if (!inst || inst->opcode() != Opcode::ICmp || inst->numOperands() != 2)
    return std::nullopt;
  if (auto lhs = knownIntConstant(state, inst->operand(0))) {
    if (auto rhs = knownIntConstant(state, inst->operand(1)))
      return evalIntCmp(cmpCond(inst), *lhs, *rhs) != 0;
  }
  if (inst->operand(0) == inst->operand(1)) {
    const std::string cond = cmpCond(inst);
    if (cond == "eq" || cond == "le" || cond == "ge")
      return true;
    if (cond == "ne" || cond == "lt" || cond == "gt")
      return false;
  }

  Value* value = nullptr;
  int32_t constant = 0;
  std::string cond = "";
  if (!matchIntCompareAgainstConstant(inst, value, constant, cond))
    return std::nullopt;
  auto exact = knownIntConstant(state, value);
  if (exact)
    return evalIntCmp(cond, *exact, constant) != 0;

  // 无精确常量时，查路径上积累的整数约束（上下界、notEquals）：
  // 若约束足以证实或证伪该比较，则直接给出静态结果。
  auto factIt = state.intConstraints.find(value);
  if (factIt == state.intConstraints.end())
    return std::nullopt;
  const IntConstraint& constraint = factIt->second;
  if (cond == "eq") {
    if (constraint.hasLower && constant < constraint.lower)
      return false;
    if (constraint.hasUpper && constant > constraint.upper)
      return false;
    if (std::find(constraint.notEquals.begin(), constraint.notEquals.end(),
                  constant) != constraint.notEquals.end()) {
      return false;
    }
  } else if (cond == "ne") {
    if (constraint.hasLower && constant < constraint.lower)
      return true;
    if (constraint.hasUpper && constant > constraint.upper)
      return true;
    if (std::find(constraint.notEquals.begin(), constraint.notEquals.end(),
                  constant) != constraint.notEquals.end()) {
      return true;
    }
  // 有序比较（lt/le/gt/ge）仅凭上下界作单向证实或证伪。
  } else if (cond == "lt") {
    if (constraint.hasUpper && constraint.upper < constant)
      return true;
    if (constraint.hasLower && constraint.lower >= constant)
      return false;
  } else if (cond == "le") {
    if (constraint.hasUpper && constraint.upper <= constant)
      return true;
    if (constraint.hasLower && constraint.lower > constant)
      return false;
  } else if (cond == "gt") {
    if (constraint.hasLower && constraint.lower > constant)
      return true;
    if (constraint.hasUpper && constraint.upper <= constant)
      return false;
  } else if (cond == "ge") {
    if (constraint.hasLower && constraint.lower >= constant)
      return true;
    if (constraint.hasUpper && constraint.upper < constant)
      return false;
  }
  return std::nullopt;
}

std::optional<bool> evaluateFloatCompareWithFacts(const FactState& state,
                                                  Instruction* inst) {
  if (!inst || inst->opcode() != Opcode::FCmp || inst->numOperands() != 2)
    return std::nullopt;
  auto lhs = knownFloatConstant(state, inst->operand(0));
  auto rhs = knownFloatConstant(state, inst->operand(1));
  if (!lhs || !rhs)
    return std::nullopt;
  return evalFloatCmp(cmpCond(inst), *lhs, *rhs) != 0;
}

// 依据路径事实（FactState）对指令做常量折叠与代数恒等式化简；
// 可化简时返回用于替换 inst 的 Value（常量或已有操作数），否则返回 nullptr。
Value* foldInstructionWithFacts(Module& mod, Instruction* inst,
                                const FactState& state) {
  if (!inst)
    return nullptr;

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
    auto lhs = knownIntConstant(state, inst->operand(0));
    auto rhs = knownIntConstant(state, inst->operand(1));
    if (lhs && rhs) {
      switch (inst->opcode()) {
      case Opcode::Add: return mod.getConstInt(*lhs + *rhs);
      case Opcode::Sub: return mod.getConstInt(*lhs - *rhs);
      case Opcode::Mul: return mod.getConstInt(*lhs * *rhs);
      case Opcode::SDiv:
        if (*rhs != 0)
          return mod.getConstInt(*lhs / *rhs);
        break;
      case Opcode::SRem:
        if (*rhs != 0)
          return mod.getConstInt(*lhs % *rhs);
        break;
      case Opcode::And: return mod.getConstInt(*lhs & *rhs);
      case Opcode::Or: return mod.getConstInt(*lhs | *rhs);
      case Opcode::Xor: return mod.getConstInt(*lhs ^ *rhs);
      case Opcode::Shl:
        if (*rhs >= 0 && *rhs <= 30)
          return mod.getConstInt(*lhs << *rhs);
        break;
      case Opcode::Shr:
        if (*rhs >= 0 && *rhs <= 30)
          return mod.getConstInt(*lhs >> *rhs);
        break;
      default:
        break;
      }
    }
    // 两侧不全为常量时退化为代数恒等式化简（加/减 0、乘 0/1、除 1、
    // 取模 1、位运算单位元），将指令替换为其操作数或常量。
    if (inst->opcode() == Opcode::Add && isConstZero(inst->operand(1)))
      return inst->operand(0);
    if (inst->opcode() == Opcode::Add && isConstZero(inst->operand(0)))
      return inst->operand(1);
    if (inst->opcode() == Opcode::Sub && isConstZero(inst->operand(1)))
      return inst->operand(0);
    if (inst->opcode() == Opcode::Mul) {
      if (isConstZero(inst->operand(0)) || isConstZero(inst->operand(1)))
        return mod.getConstInt(0);
      if (isConstOne(inst->operand(0)))
        return inst->operand(1);
      if (isConstOne(inst->operand(1)))
        return inst->operand(0);
    }
    if (inst->opcode() == Opcode::SDiv && isConstOne(inst->operand(1)))
      return inst->operand(0);
    if (inst->opcode() == Opcode::SRem && isConstOne(inst->operand(1)))
      return mod.getConstInt(0);
    if (inst->opcode() == Opcode::And) {
      if (isConstZero(inst->operand(0)) || isConstZero(inst->operand(1)))
        return mod.getConstInt(0);
    }
    if (inst->opcode() == Opcode::Or || inst->opcode() == Opcode::Xor) {
      if (isConstZero(inst->operand(0)))
        return inst->operand(1);
      if (isConstZero(inst->operand(1)))
        return inst->operand(0);
    }
    break;
  }

  case Opcode::FAdd:
  case Opcode::FSub:
  case Opcode::FMul:
  case Opcode::FDiv: {
    auto lhs = knownFloatConstant(state, inst->operand(0));
    auto rhs = knownFloatConstant(state, inst->operand(1));
    if (lhs && rhs) {
      switch (inst->opcode()) {
      case Opcode::FAdd: return mod.getConstFloat(*lhs + *rhs);
      case Opcode::FSub: return mod.getConstFloat(*lhs - *rhs);
      case Opcode::FMul: return mod.getConstFloat(*lhs * *rhs);
      case Opcode::FDiv:
        if (*rhs != 0.0f)
          return mod.getConstFloat(*lhs / *rhs);
        break;
      default:
        break;
      }
    }
    break;
  }

  case Opcode::ICmp: {
    if (auto known = evaluateIntCompareWithFacts(state, inst))
      return mod.getConstInt(*known ? 1 : 0);
    break;
  }

  case Opcode::FCmp: {
    if (auto known = evaluateFloatCompareWithFacts(state, inst))
      return mod.getConstInt(*known ? 1 : 0);
    break;
  }

  case Opcode::Neg: {
    auto value = knownIntConstant(state, inst->operand(0));
    if (value)
      return mod.getConstInt(-*value);
    break;
  }

  case Opcode::FNeg: {
    auto value = knownFloatConstant(state, inst->operand(0));
    if (value)
      return mod.getConstFloat(-*value);
    break;
  }

  case Opcode::SIToFP: {
    auto value = knownIntConstant(state, inst->operand(0));
    if (value)
      return mod.getConstFloat(static_cast<float>(*value));
    break;
  }

  case Opcode::FPToSI: {
    auto value = knownFloatConstant(state, inst->operand(0));
    if (value)
      return mod.getConstInt(static_cast<int32_t>(*value));
    break;
  }

  case Opcode::Mov:
  case Opcode::BitCast:
    if (inst->numOperands() == 1 && inst->operand(0)->type() == inst->type())
      return inst->operand(0);
    break;

  default:
    break;
  }
  return nullptr;
}

void copyFactsFromValue(Module& mod, FactState& state, Value* dst, Value* src) {
  if (!dst || !src)
    return;
  if (src->isConstant()) {
    recordExact(mod, state, dst, src);
    return;
  }
  auto exactIt = state.exactValues.find(src);
  if (exactIt != state.exactValues.end() && exactIt->second->isConstant())
    recordExact(mod, state, dst, exactIt->second);
  auto factsIt = state.intConstraints.find(src);
  if (factsIt != state.intConstraints.end() && dst->type().isI32()) {
    state.intConstraints[dst] = factsIt->second;
    updateExactFromBounds(mod, state, dst);
  }
}

Value* incomingFromPred(Instruction* phi, BasicBlock* pred) {
  if (!phi || !pred)
    return nullptr;
  Value* result = nullptr;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    auto* inPred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
    if (inPred != pred)
      continue;
    if (result && result != phi->operand(i))
      return nullptr;
    result = phi->operand(i);
  }
  return result;
}

bool isThreadableInstruction(Instruction* inst) {
  if (!inst || inst->isTerminator() || inst->opcode() == Opcode::Phi)
    return false;
  switch (inst->opcode()) {
  case Opcode::Load:
  case Opcode::VLoad:
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Mul:
  case Opcode::SDiv:
  case Opcode::SRem:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
  case Opcode::Shl:
  case Opcode::Shr:
  case Opcode::FAdd:
  case Opcode::FSub:
  case Opcode::FMul:
  case Opcode::FDiv:
  case Opcode::ICmp:
  case Opcode::FCmp:
  case Opcode::SIToFP:
  case Opcode::FPToSI:
  case Opcode::GEP:
  case Opcode::Neg:
  case Opcode::FNeg:
  case Opcode::BitCast:
  case Opcode::Mov:
    return true;
  default:
    return false;
  }
}

struct ThreadPathEntry {
  BasicBlock* block = nullptr;
  BasicBlock* incomingPred = nullptr;
  BasicBlock* chosenSucc = nullptr;
  std::vector<Instruction*> instructions;
};

struct ThreadCandidate {
  BasicBlock* pred = nullptr;
  BasicBlock* start = nullptr;
  BasicBlock* finalTarget = nullptr;
  BasicBlock* finalIncomingPred = nullptr;
  std::vector<ThreadPathEntry> path;
};

bool simulateThreadedPath(Module& mod, BasicBlock* pred, BasicBlock* start,
                          ThreadCandidate& out) {
  if (!pred || !start)
    return false;

  FactState state;
  applyBranchFacts(mod, state,
                   pred->terminator() && pred->terminator()->opcode() == Opcode::CondBr &&
                           pred->terminator()->numOperands() == 3
                       ? pred->terminator()->operand(0)
                       : nullptr,
                   pred->terminator() && pred->terminator()->opcode() == Opcode::CondBr &&
                           pred->terminator()->numOperands() == 3 &&
                           dynamic_cast<BasicBlock*>(pred->terminator()->operand(1)) == start);

  BasicBlock* current = start;
  BasicBlock* incomingPred = pred;
  std::unordered_set<BasicBlock*> visited;
  std::vector<ThreadPathEntry> path;
  int totalClonedInstructions = 0;
  constexpr size_t kMaxThreadBlocks = 12;
  constexpr int kMaxClonedInstructions = 32;

  while (current && visited.insert(current).second &&
         path.size() < kMaxThreadBlocks &&
         totalClonedInstructions <= kMaxClonedInstructions) {
    for (Instruction* phi : analysis::collectLeadingPhi(current)) {
      Value* incoming = incomingFromPred(phi, incomingPred);
      if (!incoming)
        return false;
      copyFactsFromValue(mod, state, phi, incoming);
    }
    if (!state.feasible)
      return false;

    std::vector<Instruction*> instructions;
    for (auto& instPtr : current->instructions()) {
      Instruction* inst = instPtr.get();
      if (inst == current->terminator() || inst->opcode() == Opcode::Phi)
        continue;
      if (!isThreadableInstruction(inst)) {
        if (path.empty())
          return false;
        out = {pred, start, current, incomingPred, path};
        return true;
      }
      instructions.push_back(inst);
      ++totalClonedInstructions;
      if (totalClonedInstructions > kMaxClonedInstructions)
        return false;
      if (!inst->type().isVoid()) {
        if (Value* folded = foldInstructionWithFacts(mod, inst, state)) {
          if (folded->isConstant())
            recordExact(mod, state, inst, folded);
          else if (Value* replacement = exactReplacement(mod, state, folded);
                   replacement && replacement->isConstant()) {
            recordExact(mod, state, inst, replacement);
          }
        }
      }
      if (!state.feasible)
        return false;
    }

    Instruction* term = current->terminator();
    if (!term)
      return false;

    if (term->opcode() == Opcode::Br && term->numOperands() == 1) {
      auto* succ = dynamic_cast<BasicBlock*>(term->operand(0));
      if (!succ)
        return false;
      path.push_back({current, incomingPred, succ, instructions});
      incomingPred = current;
      current = succ;
      continue;
    }

    if (term->opcode() != Opcode::CondBr || term->numOperands() != 3) {
      if (path.empty())
        return false;
      out = {pred, start, current, incomingPred, path};
      return true;
    }

    Value* cond = term->operand(0);
    std::optional<int32_t> exactCond = knownIntConstant(state, cond);
    if (!exactCond) {
      if (auto* condInst = dynamic_cast<Instruction*>(cond)) {
        if (Value* folded = foldInstructionWithFacts(mod, condInst, state)) {
          if (auto* ci = dynamic_cast<ConstInt*>(folded))
            exactCond = ci->value();
        }
      }
    }
    if (!exactCond) {
      if (path.empty())
        return false;
      out = {pred, start, current, incomingPred, path};
      return true;
    }

    const bool takeTrue = *exactCond != 0;
    auto* chosenSucc = dynamic_cast<BasicBlock*>(
        takeTrue ? term->operand(1) : term->operand(2));
    if (!chosenSucc)
      return false;

    path.push_back({current, incomingPred, chosenSucc, instructions});
    applyBranchFacts(mod, state, cond, takeTrue);
    if (!state.feasible)
      return false;
    incomingPred = current;
    current = chosenSucc;
  }

  if (path.empty() || !current)
    return false;
  out = {pred, start, current, incomingPred, path};
  return true;
}

Value* remapValue(Value* value,
                  const std::unordered_map<Value*, Value*>& cloneMap) {
  auto it = cloneMap.find(value);
  return it == cloneMap.end() ? value : it->second;
}

Instruction* firstNonPhiInstruction(BasicBlock* bb) {
  if (!bb)
    return nullptr;
  for (auto& instPtr : bb->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst || inst->opcode() != Opcode::Phi)
      return inst;
  }
  return nullptr;
}

bool hasPhiHeader(BasicBlock* start) {
  if (!start)
    return false;
  return !analysis::collectLeadingPhi(start).empty();
}

Instruction* cloneThreadedInstruction(BasicBlock* dst, Instruction* src,
                                      std::unordered_map<Value*, Value*>& cloneMap) {
  auto* clone = new Instruction(
      src->opcode(), src->type(),
      src->name().empty() ? "jt" : (src->name() + ".jt"));
  clone->setMemoryInfo(src->memoryElemTy(), src->memorySizeBytes(),
                       src->isArrayObject(), src->dims());
  for (Value* operand : src->operands())
    clone->addOperand(remapValue(operand, cloneMap));
  dst->pushBack(clone);
  cloneMap[src] = clone;
  return clone;
}

bool commitThreadingCandidate(Function* func, const ThreadCandidate& candidate) {
  if (!func || !candidate.pred || !candidate.start || !candidate.finalTarget ||
      candidate.path.empty() || candidate.finalTarget == candidate.start)
    return false;

  auto targetPhis = analysis::collectLeadingPhi(candidate.finalTarget);
  for (Instruction* phi : targetPhis) {
    if (!incomingFromPred(phi, candidate.finalIncomingPred))
      return false;
  }

  BasicBlock* edge = func->newBlock("jt");
  std::unordered_map<Value*, Value*> cloneMap;

  for (const ThreadPathEntry& entry : candidate.path) {
    for (Instruction* phi : analysis::collectLeadingPhi(entry.block)) {
      Value* incoming = incomingFromPred(phi, entry.incomingPred);
      if (!incoming)
        return false;
      cloneMap[phi] = remapValue(incoming, cloneMap);
    }
    for (Instruction* inst : entry.instructions)
      cloneThreadedInstruction(edge, inst, cloneMap);
  }

  auto* branch = new Instruction(Opcode::Br, Type::voidTy());
  branch->addOperand(candidate.finalTarget);
  edge->pushBack(branch);

  std::unordered_map<Value*, Instruction*> mergePhis;
  Instruction* phiInsertPos = firstNonPhiInstruction(candidate.finalTarget);
  std::vector<Instruction*> targetInsts;
  for (auto& instPtr : candidate.finalTarget->instructions()) {
    Instruction* inst = instPtr.get();
    if (inst && inst->opcode() != Opcode::Phi)
      targetInsts.push_back(inst);
  }
  for (Instruction* inst : targetInsts) {
    if (!inst || inst->opcode() == Opcode::Phi)
      continue;
    for (int i = 0; i < inst->numOperands(); ++i) {
      Value* operand = inst->operand(i);
      if (!operand || operand->type().isLabel())
        continue;
      auto it = cloneMap.find(operand);
      if (it == cloneMap.end() || it->second == operand)
        continue;
      Instruction*& phi = mergePhis[operand];
      if (!phi) {
        phi = new Instruction(Opcode::Phi, operand->type(), "jt.merge");
        phi->addOperand(operand);
        phi->addOperand(candidate.finalIncomingPred);
        phi->addOperand(it->second);
        phi->addOperand(edge);
        if (phiInsertPos)
          candidate.finalTarget->insertBefore(phiInsertPos, phi);
        else
          candidate.finalTarget->pushBack(phi);
      }
      inst->setOperand(i, phi);
    }
  }

  if (!analysis::redirectSuccessor(candidate.pred, candidate.start, edge))
    return false;
  analysis::removePhiIncomingPred(candidate.start, candidate.pred);

  for (Instruction* phi : targetPhis) {
    Value* incoming = incomingFromPred(phi, candidate.finalIncomingPred);
    if (!incoming)
      return false;
    phi->addOperand(remapValue(incoming, cloneMap));
    phi->addOperand(edge);
  }

  func->recomputeCFG();
  return true;
}

bool runOnFunction(Module& mod, Function* func,
                   analysis::AnalysisManager& analysisManager) {
  if (!func || !func->entryBlock())
    return false;

  bool changed = false;
  int threadBudget = 12;
  bool localChanged = true;
  while (localChanged && threadBudget > 0) {
    localChanged = false;
    func->recomputeCFG();
    std::unordered_set<BasicBlock*> reachable =
        analysis::collectReachableBlocks(func);
    if (reachable.empty())
      break;
    std::vector<BasicBlock*> blocks;
    for (auto& bbPtr : func->blocks())
      blocks.push_back(bbPtr.get());

    for (BasicBlock* start : blocks) {
      if (!start || !reachable.count(start))
        continue;
      if (start->preds().empty())
        continue;
      if (hasPhiHeader(start))
        continue;
      for (BasicBlock* pred : start->preds()) {
        if (!pred || pred == start)
          continue;
        ThreadCandidate candidate;
        if (!simulateThreadedPath(mod, pred, start, candidate))
          continue;
        if (candidate.path.empty() || candidate.finalTarget == start)
          continue;
        if (!commitThreadingCandidate(func, candidate))
          continue;
        changed = true;
        localChanged = true;
        --threadBudget;
        analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
        break;
      }
      if (localChanged || threadBudget <= 0)
        break;
    }
  }

  return changed;
}

} // namespace

bool applyJumpThreading(Module& mod,
                        analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions())
    changed |= runOnFunction(mod, funcPtr.get(), analysisManager);
  return changed;
}

class JumpThreadingPass : public Pass {
public:
    std::string name() const override { return "JumpThreading"; }
    bool run(Module& mod, analysis::AnalysisManager& am) override {
        return applyJumpThreading(mod, am);
    }
};
REGISTER_PASS(JumpThreadingPass);

} // namespace core_ir
