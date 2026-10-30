// 文件职责：实现 LLVM 风格的 CorrelatedValuePropagation，只传播支配路径上的条件事实。
#include "pass_registry.h"
#include "correlated_value_propagation.h"

#include "../../analysis/dominators.h"
#include "../../analysis/function_analysis.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <string>
#include <unordered_map>
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

struct RelativeConstraint {
  Value* lower = nullptr;
  Value* upper = nullptr;
  bool strict = false;
};

struct FactState {
  bool feasible = true;
  std::unordered_map<Value*, Value*> exactValues;
  std::unordered_map<Value*, IntConstraint> intConstraints;
  std::vector<RelativeConstraint> relativeConstraints;
  const std::unordered_map<Value*, Value*>* monotonicLowerInits = nullptr;
  const std::unordered_map<Value*, Value*>* monotonicUpperInits = nullptr;
};

IntConstraint inferredIntConstraint(const FactState& state, Value* value,
                                    int depth = 0) {
  IntConstraint result;
  if (!value || depth > 12)
    return result;

  if (auto* constant = dynamic_cast<ConstInt*>(value)) {
    result.hasExact = true;
    result.exact = constant->value();
    result.hasLower = true;
    result.lower = constant->value();
    result.hasUpper = true;
    result.upper = constant->value();
    return result;
  }

  auto directIt = state.intConstraints.find(value);
  if (directIt != state.intConstraints.end())
    result = directIt->second;

  auto mergeLower = [&](int64_t lower) {
    if (lower < std::numeric_limits<int32_t>::min() ||
        lower > std::numeric_limits<int32_t>::max())
      return;
    const int32_t bound = static_cast<int32_t>(lower);
    if (!result.hasLower || bound > result.lower) {
      result.hasLower = true;
      result.lower = bound;
    }
  };
  auto mergeUpper = [&](int64_t upper) {
    if (upper < std::numeric_limits<int32_t>::min() ||
        upper > std::numeric_limits<int32_t>::max())
      return;
    const int32_t bound = static_cast<int32_t>(upper);
    if (!result.hasUpper || bound < result.upper) {
      result.hasUpper = true;
      result.upper = bound;
    }
  };

  auto inferShifted = [&](Value* base, int64_t offset) {
    IntConstraint baseConstraint =
        inferredIntConstraint(state, base, depth + 1);
    if (baseConstraint.hasLower)
      mergeLower(static_cast<int64_t>(baseConstraint.lower) + offset);
    if (baseConstraint.hasUpper)
      mergeUpper(static_cast<int64_t>(baseConstraint.upper) + offset);
    if (baseConstraint.hasExact) {
      const int64_t exact =
          static_cast<int64_t>(baseConstraint.exact) + offset;
      if (exact >= std::numeric_limits<int32_t>::min() &&
          exact <= std::numeric_limits<int32_t>::max()) {
        result.hasExact = true;
        result.exact = static_cast<int32_t>(exact);
      }
    }
  };

  if (auto* inst = dynamic_cast<Instruction*>(value)) {
    if ((inst->opcode() == Opcode::Mov ||
         inst->opcode() == Opcode::BitCast) &&
        inst->numOperands() == 1) {
      inferShifted(inst->operand(0), 0);
    } else if (inst->opcode() == Opcode::Add && inst->numOperands() == 2) {
      if (auto* rhs = dynamic_cast<ConstInt*>(inst->operand(1)))
        inferShifted(inst->operand(0), rhs->value());
      else if (auto* lhs = dynamic_cast<ConstInt*>(inst->operand(0)))
        inferShifted(inst->operand(1), lhs->value());
    } else if (inst->opcode() == Opcode::Sub && inst->numOperands() == 2) {
      if (auto* rhs = dynamic_cast<ConstInt*>(inst->operand(1)))
        inferShifted(inst->operand(0),
                     -static_cast<int64_t>(rhs->value()));
    }
  }

  if (state.monotonicLowerInits) {
    auto it = state.monotonicLowerInits->find(value);
    if (it != state.monotonicLowerInits->end()) {
      IntConstraint init =
          inferredIntConstraint(state, it->second, depth + 1);
      if (init.hasLower)
        mergeLower(init.lower);
    }
    for (const auto& [phi, initValue] : *state.monotonicLowerInits) {
      if (initValue != value)
        continue;
      auto phiConstraint = state.intConstraints.find(phi);
      if (phiConstraint != state.intConstraints.end() &&
          phiConstraint->second.hasUpper) {
        mergeUpper(phiConstraint->second.upper);
      }
    }
  }
  if (state.monotonicUpperInits) {
    auto it = state.monotonicUpperInits->find(value);
    if (it != state.monotonicUpperInits->end()) {
      IntConstraint init =
          inferredIntConstraint(state, it->second, depth + 1);
      if (init.hasUpper)
        mergeUpper(init.upper);
    }
    for (const auto& [phi, initValue] : *state.monotonicUpperInits) {
      if (initValue != value)
        continue;
      auto phiConstraint = state.intConstraints.find(phi);
      if (phiConstraint != state.intConstraints.end() &&
          phiConstraint->second.hasLower) {
        mergeLower(phiConstraint->second.lower);
      }
    }
  }

  if (result.hasLower && result.hasUpper &&
      result.lower == result.upper) {
    result.hasExact = true;
    result.exact = result.lower;
  }
  return result;
}

bool provesOrdered(const FactState& state, Value* lower, Value* upper,
                   bool requireStrict) {
  if (!lower || !upper)
    return false;
  struct WorkItem {
    Value* value;
    bool strict;
  };
  std::vector<WorkItem> work{{lower, false}};
  std::unordered_map<Value*, unsigned> visited;

  for (size_t index = 0; index < work.size() && index < 64; ++index) {
    WorkItem item = work[index];
    const unsigned bit = item.strict ? 2u : 1u;
    if (visited[item.value] & bit)
      continue;
    visited[item.value] |= bit;
    if (item.value == upper && (!requireStrict || item.strict))
      return true;

    for (const RelativeConstraint& relation : state.relativeConstraints) {
      if (relation.lower == item.value && relation.upper)
        work.push_back(
            {relation.upper, item.strict || relation.strict});
    }
    if (state.monotonicLowerInits) {
      for (const auto& [phi, init] : *state.monotonicLowerInits) {
        if (init == item.value)
          work.push_back({phi, item.strict});
      }
    }
    if (state.monotonicUpperInits) {
      for (const auto& [phi, init] : *state.monotonicUpperInits) {
        if (phi == item.value)
          work.push_back({init, item.strict});
      }
    }
  }
  return false;
}

void recordRelativeConstraint(FactState& state, Value* lower, Value* upper,
                              bool strict) {
  if (!state.feasible || !lower || !upper)
    return;
  if (lower == upper) {
    if (strict)
      state.feasible = false;
    return;
  }
  for (RelativeConstraint& relation : state.relativeConstraints) {
    if (relation.lower == lower && relation.upper == upper) {
      relation.strict |= strict;
      return;
    }
  }
  if (state.relativeConstraints.size() < 32)
    state.relativeConstraints.push_back({lower, upper, strict});
}

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

void constrainLower(Module& mod, FactState& state, Value* value, int64_t lower) {
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

void constrainUpper(Module& mod, FactState& state, Value* value, int64_t upper) {
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

  auto* cmp = dynamic_cast<Instruction*>(value);
  if (!cmp)
    return;

  Value* intValue = nullptr;
  int32_t intConstant = 0;
  std::string cond = "";
  if (matchIntCompareAgainstConstant(cmp, intValue, intConstant, cond)) {
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

  if (cmp->opcode() == Opcode::ICmp && cmp->numOperands() == 2) {
    const std::string effectiveCond =
        isTrue ? cmpCond(cmp) : negatedCond(cmpCond(cmp));
    if (effectiveCond == "lt")
      recordRelativeConstraint(state, cmp->operand(0), cmp->operand(1), true);
    else if (effectiveCond == "le")
      recordRelativeConstraint(state, cmp->operand(0), cmp->operand(1), false);
    else if (effectiveCond == "gt")
      recordRelativeConstraint(state, cmp->operand(1), cmp->operand(0), true);
    else if (effectiveCond == "ge")
      recordRelativeConstraint(state, cmp->operand(1), cmp->operand(0), false);
    return;
  }

  Value* floatValue = nullptr;
  float floatConstant = 0.0f;
  if (matchFloatCompareAgainstConstant(cmp, floatValue, floatConstant, cond)) {
    applyFloatComparisonFact(mod, state, floatValue, cond, floatConstant,
                             isTrue);
    return;
  }

  if (cmp->opcode() == Opcode::And && cmp->numOperands() == 2 &&
      isBooleanLikeValue(cmp->operand(0)) &&
      isBooleanLikeValue(cmp->operand(1))) {
    if (isTrue) {
      applyBooleanFact(mod, state, cmp->operand(0), true, depth + 1);
      applyBooleanFact(mod, state, cmp->operand(1), true, depth + 1);
    }
    return;
  }

  if (cmp->opcode() == Opcode::Or && cmp->numOperands() == 2 &&
      isBooleanLikeValue(cmp->operand(0)) &&
      isBooleanLikeValue(cmp->operand(1))) {
    if (!isTrue) {
      applyBooleanFact(mod, state, cmp->operand(0), false, depth + 1);
      applyBooleanFact(mod, state, cmp->operand(1), false, depth + 1);
    }
    return;
  }

  if (cmp->opcode() == Opcode::Xor && cmp->numOperands() == 2 &&
      isBooleanLikeValue(cmp->operand(0)) &&
      isBooleanLikeValue(cmp->operand(1))) {
    auto lhs = knownIntConstant(state, cmp->operand(0));
    auto rhs = knownIntConstant(state, cmp->operand(1));
    if (lhs && (*lhs == 0 || *lhs == 1)) {
      applyBooleanFact(mod, state, cmp->operand(1),
                       isTrue ^ (*lhs != 0), depth + 1);
      return;
    }
    if (rhs && (*rhs == 0 || *rhs == 1)) {
      applyBooleanFact(mod, state, cmp->operand(0),
                       isTrue ^ (*rhs != 0), depth + 1);
      return;
    }
    return;
  }

  if ((cmp->opcode() == Opcode::Mov || cmp->opcode() == Opcode::BitCast) &&
      cmp->numOperands() == 1 && isBooleanLikeValue(cmp->operand(0))) {
    applyBooleanFact(mod, state, cmp->operand(0), isTrue, depth + 1);
  }
}

void applyBranchFacts(Module& mod, FactState& state, Value* condValue,
                      bool takeTrue) {
  if (!state.feasible || !condValue)
    return;
  if (isBooleanLikeValue(condValue))
    applyBooleanFact(mod, state, condValue, takeTrue);
  else if (auto* cmp = dynamic_cast<Instruction*>(condValue);
           cmp && (cmp->opcode() == Opcode::ICmp || cmp->opcode() == Opcode::FCmp)) {
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

  const std::string directCond = cmpCond(inst);
  Value* lhsValue = inst->operand(0);
  Value* rhsValue = inst->operand(1);
  if (directCond == "lt") {
    if (provesOrdered(state, lhsValue, rhsValue, true))
      return true;
    if (provesOrdered(state, rhsValue, lhsValue, false))
      return false;
  } else if (directCond == "le") {
    if (provesOrdered(state, lhsValue, rhsValue, false))
      return true;
    if (provesOrdered(state, rhsValue, lhsValue, true))
      return false;
  } else if (directCond == "gt") {
    if (provesOrdered(state, rhsValue, lhsValue, true))
      return true;
    if (provesOrdered(state, lhsValue, rhsValue, false))
      return false;
  } else if (directCond == "ge") {
    if (provesOrdered(state, rhsValue, lhsValue, false))
      return true;
    if (provesOrdered(state, lhsValue, rhsValue, true))
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

  // 无精确常量时，改用沿分支路径收集的整数约束（上下界、notEquals）
  // 判断比较是否可静态定值：常数能被约束证实/证伪时直接返回结论。
  IntConstraint constraint = inferredIntConstraint(state, value);
  if (!constraint.hasExact && !constraint.hasLower && !constraint.hasUpper &&
      constraint.notEquals.empty())
    return std::nullopt;
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

bool rewriteInstructionWithFacts(Module& mod, Instruction* inst,
                                 FactState& state) {
  if (!inst || !state.feasible)
    return false;
  bool changed = false;

  if (inst->opcode() != Opcode::Phi) {
    for (int i = 0; i < inst->numOperands(); ++i) {
      Value* operand = inst->operand(i);
      if (!operand || operand->type().isLabel())
        continue;
      Value* replacement = exactReplacement(mod, state, operand);
      if ((!replacement || replacement == operand)) {
        if (auto* operandInst = dynamic_cast<Instruction*>(operand)) {
          bool hasNonConstantOperand = false;
          for (Value* nestedOperand : operandInst->operands()) {
            if (nestedOperand && !nestedOperand->type().isLabel() &&
                !nestedOperand->isConstant()) {
              hasNonConstantOperand = true;
              break;
            }
          }
          if (hasNonConstantOperand)
            replacement = foldInstructionWithFacts(mod, operandInst, state);
        }
      }
      if (replacement && replacement != operand) {
        inst->setOperand(i, replacement);
        changed = true;
      }
    }
  }

  if (!inst->type().isVoid()) {
    bool hasNonConstantOperand = false;
    for (Value* operand : inst->operands()) {
      if (operand && !operand->type().isLabel() &&
          !operand->isConstant()) {
        hasNonConstantOperand = true;
        break;
      }
    }
    if (Value* folded = hasNonConstantOperand
                            ? foldInstructionWithFacts(mod, inst, state)
                            : nullptr) {
      if (folded != inst) {
        state.exactValues[inst] = folded;
        if (folded->isConstant())
          recordExact(mod, state, inst, folded);
      }
    } else if (Value* replacement = exactReplacement(mod, state, inst)) {
      if (replacement != inst) {
        state.exactValues[inst] = replacement;
      }
    }
  }

  return changed;
}

bool runOnFunction(Module& mod, Function* func,
                   analysis::AnalysisManager& analysisManager) {
  if (!func || !func->entryBlock())
    return false;

  analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
  const auto& blocks = cache.blocks();
  const auto& reachable = cache.reachable();
  if (reachable.empty())
    return false;

  const analysis::DomSet& doms = cache.dominators();
  auto idom = analysis::computeIDom(func, blocks, doms);
  auto domChildren = analysis::buildDomTreeChildren(blocks, idom);
  std::unordered_map<Value*, Value*> monotonicLowerInits;
  std::unordered_map<Value*, Value*> monotonicUpperInits;
  for (const auto& loop : cache.loops()) {
    Instruction* term = loop.header ? loop.header->terminator() : nullptr;
    if (!term || term->opcode() != Opcode::CondBr ||
        term->numOperands() != 3)
      continue;
    auto* trueBB = dynamic_cast<BasicBlock*>(term->operand(1));
    auto* falseBB = dynamic_cast<BasicBlock*>(term->operand(2));
    if (!trueBB || !falseBB || !loop.blocks.count(trueBB) ||
        loop.blocks.count(falseBB))
      continue;
    auto* cmp = dynamic_cast<Instruction*>(term->operand(0));
    if (!cmp || cmp->opcode() != Opcode::ICmp || cmp->numOperands() != 2)
      continue;

    for (const auto& iv : cache.inductionVars(loop)) {
      if (!iv.isCanonical || !iv.phi || !iv.init)
        continue;
      std::string cond = cmpCond(cmp);
      if (cmp->operand(1) == iv.phi)
        cond = swappedCond(cond);
      else if (cmp->operand(0) != iv.phi)
        continue;

      // 循环条件表明执行迭代时单位步长不会发生回绕，
      // 因此每个可达 phi 都从初值开始保持单调变化。
      if (iv.step == 1 && cond == "lt")
        monotonicLowerInits[iv.phi] = iv.init;
      else if (iv.step == -1 && cond == "gt")
        monotonicUpperInits[iv.phi] = iv.init;
    }
  }
  bool changed = false;

  std::function<void(BasicBlock*, FactState)> visit =
      [&](BasicBlock* bb, FactState state) {
        if (!bb || !reachable.count(bb) || !state.feasible)
          return;
        for (auto& instPtr : bb->instructions())
          changed |= rewriteInstructionWithFacts(mod, instPtr.get(), state);

        Instruction* term = bb->terminator();
        for (BasicBlock* child : domChildren[bb]) {
          if (!child || !reachable.count(child))
            continue;
          FactState childState = state;
          if (term && term->opcode() == Opcode::CondBr &&
              term->numOperands() == 3) {
            auto* trueBB = dynamic_cast<BasicBlock*>(term->operand(1));
            auto* falseBB = dynamic_cast<BasicBlock*>(term->operand(2));
            const bool hasExclusiveIncomingEdge =
                child->preds().size() == 1 && child->preds().front() == bb;
            if (child == trueBB && hasExclusiveIncomingEdge)
              applyBranchFacts(mod, childState, term->operand(0), true);
            else if (child == falseBB && hasExclusiveIncomingEdge)
              applyBranchFacts(mod, childState, term->operand(0), false);
          }
          visit(child, childState);
        }
      };

  FactState initialState;
  initialState.monotonicLowerInits = &monotonicLowerInits;
  initialState.monotonicUpperInits = &monotonicUpperInits;
  visit(func->entryBlock(), std::move(initialState));
  return changed;
}

} // namespace

bool applyCorrelatedValuePropagation(
    Module& mod, analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions())
    changed |= runOnFunction(mod, funcPtr.get(), analysisManager);
  return changed;
}

class CVP_Pass : public Pass {
public:
    std::string name() const override { return "CVP"; }
    bool run(Module& mod, analysis::AnalysisManager& am) override {
        return applyCorrelatedValuePropagation(mod, am);
    }
};
REGISTER_PASS(CVP_Pass);

} // namespace core_ir
