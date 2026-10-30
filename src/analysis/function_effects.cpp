#include "research_switch.h"
// 文件职责：建立函数副作用摘要，区分只读、递归、稳定状态和 helper 属性。本文件负责实现具体逻辑。
#ifdef ANALYSIS_FUNCTION_EFFECTS_EMBED

#include "function_effects.h"

#include "cfg_utils.h"
#include "dominators.h"
#include "loop_info.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>
#include <unordered_set>

namespace analysis {

namespace {

using core_ir::BasicBlock;
using core_ir::Function;
using core_ir::GlobalVar;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Arg;
using core_ir::Argument;
using core_ir::Value;

bool isMemoGlobal(const GlobalVar* gv) {
  return gv && gv->name().rfind("__memo.", 0) == 0;
}

Value* memoryRoot(Value* addr) {
  Value* cur = addr;
  while (auto* inst = dynamic_cast<Instruction*>(cur)) {
    if ((inst->opcode() == Opcode::GEP || inst->opcode() == Opcode::BitCast) &&
        inst->numOperands() >= 1) {
      cur = inst->operand(0);
      continue;
    }
    break;
  }
  return cur;
}

enum class MemoryKind {
  Local,
  Arg,
  ConstGlobal,
  Global,
  Unknown,
};

MemoryKind classifyMemory(Value* addr) {
  Value* root = memoryRoot(addr);
  if (!root)
    return MemoryKind::Unknown;
  if (dynamic_cast<Argument*>(root))
    return MemoryKind::Arg;
  if (auto* gv = dynamic_cast<GlobalVar*>(root)) {
    if (isMemoGlobal(gv))
      return MemoryKind::Local;
    return gv->isConst() ? MemoryKind::ConstGlobal : MemoryKind::Global;
  }
  auto* inst = dynamic_cast<Instruction*>(root);
  if (inst && inst->opcode() == Opcode::Alloca)
    return MemoryKind::Local;
  return MemoryKind::Unknown;
}

std::optional<std::size_t> argumentIndex(Function* function, Value* value) {
  if (!function || !value)
    return std::nullopt;
  Value* root = memoryRoot(value);
  const auto& arguments = function->args();
  for (std::size_t index = 0; index < arguments.size(); ++index)
    if (arguments[index].val == root)
      return index;
  return std::nullopt;
}

bool isEffectCandidate(Function* func) {
  return func && func->name() != "main";
}

bool functionHasOnlyI32Args(const Function* func) {
  if (!func)
    return false;
  for (const auto& arg : func->args()) {
    if (!arg.type.isI32())
      return false;
  }
  return true;
}

bool instructionDominates(const Instruction* lhs, const Instruction* rhs,
                          const DomSet& doms) {
  if (!lhs || !rhs || !lhs->parent() || !rhs->parent())
    return false;
  if (lhs->parent() != rhs->parent())
    return dominates(doms, lhs->parent(), rhs->parent());
  for (const auto& instPtr : lhs->parent()->instructions()) {
    if (instPtr.get() == lhs)
      return true;
    if (instPtr.get() == rhs)
      return false;
  }
  return false;
}

bool stableGlobalsAreFixedBeforeCalls(
    Module& mod, const Function* recursiveFunc,
    const std::vector<const GlobalVar*>& globals) {
  if (!recursiveFunc || globals.empty())
    return false;

  std::unordered_set<const Function*> reachableFunctions;
  std::vector<const Function*> worklist;
  if (Function* entry = mod.getFunction("main")) {
    reachableFunctions.insert(entry);
    worklist.push_back(entry);
  }
  while (!worklist.empty()) {
    const Function* caller = worklist.back();
    worklist.pop_back();
    for (const auto& bbPtr : caller->blocks()) {
      for (const auto& instPtr : bbPtr->instructions()) {
        Instruction* inst = instPtr.get();
        if (!inst || inst->opcode() != Opcode::Call)
          continue;
        Function* callee = mod.getFunction(inst->name());
        if (callee && reachableFunctions.insert(callee).second)
          worklist.push_back(callee);
      }
    }
  }
  if (!reachableFunctions.count(recursiveFunc))
    return false;

  std::vector<Instruction*> externalCalls;
  for (const auto& funcPtr : mod.functions()) {
    Function* caller = funcPtr.get();
    if (!caller || caller == recursiveFunc ||
        !reachableFunctions.count(caller))
      continue;
    for (const auto& bbPtr : caller->blocks()) {
      for (const auto& instPtr : bbPtr->instructions()) {
        Instruction* inst = instPtr.get();
        if (inst && inst->opcode() == Opcode::Call &&
            inst->name() == recursiveFunc->name())
          externalCalls.push_back(inst);
      }
    }
  }
  if (externalCalls.empty())
    return false;

  for (const GlobalVar* global : globals) {
    std::vector<Instruction*> stores;
    for (const auto& funcPtr : mod.functions()) {
      if (!reachableFunctions.count(funcPtr.get()))
        continue;
      for (const auto& bbPtr : funcPtr->blocks()) {
        for (const auto& instPtr : bbPtr->instructions()) {
          Instruction* inst = instPtr.get();
          if (!inst || inst->opcode() != Opcode::Store ||
              inst->numOperands() < 2 || memoryRoot(inst->operand(1)) != global)
            continue;
          stores.push_back(inst);
        }
      }
    }
    if (stores.empty())
      continue;
    if (stores.size() != 1)
      return false;

    // 唯一的 store 必须位于 main 中，且不落在任何自然循环内。
    Instruction* store = stores.front();
    Function* owner = store->parent() ? store->parent()->parent() : nullptr;
    if (!owner || owner != mod.getFunction("main"))
      return false;
    std::vector<BasicBlock*> blocks;
    for (const auto& bbPtr : owner->blocks())
      blocks.push_back(bbPtr.get());
    auto reachable = collectReachableBlocks(owner);
    DomSet doms = computeDominators(owner, blocks, reachable);
    for (const NaturalLoop& loop : collectNaturalLoops(blocks, doms, reachable)) {
      if (loop.blocks.count(store->parent()))
        return false;
    }
    // 递归证明所有外部调用都发生在 store 之后：main 内调用用支配关系判断，
    // 其他函数中的调用递归证明该函数入口本身在 store 之后；
    // entryProof 做记忆化并防递归环（0未知/1处理中/2已证/3失败）。
    std::unordered_map<const Function*, int> entryProof;
    std::function<bool(const Function*)> enteredAfterStore =
        [&](const Function* target) -> bool {
      if (!target || target == mod.getFunction("main"))
        return false;
      auto proofIt = entryProof.find(target);
      if (proofIt != entryProof.end()) {
        if (proofIt->second == 1)
          return true;
        return proofIt->second == 2;
      }
      entryProof[target] = 1;
      bool sawExternalCall = false;
      for (const auto& callerPtr : mod.functions()) {
        Function* caller = callerPtr.get();
        if (!caller || caller == target || !reachableFunctions.count(caller))
          continue;
        for (const auto& bbPtr : caller->blocks()) {
          for (const auto& instPtr : bbPtr->instructions()) {
            Instruction* call = instPtr.get();
            if (!call || call->opcode() != Opcode::Call ||
                call->name() != target->name())
              continue;
            sawExternalCall = true;
            const bool callAfterStore =
                caller == owner ? instructionDominates(store, call, doms)
                                : enteredAfterStore(caller);
            if (!callAfterStore) {
              entryProof[target] = 3;
              return false;
            }
          }
        }
      }
      entryProof[target] = sawExternalCall ? 2 : 3;
      return sawExternalCall;
    };
    for (Instruction* call : externalCalls) {
      Function* caller =
          call && call->parent() ? call->parent()->parent() : nullptr;
      if (!caller ||
          (caller == owner ? !instructionDominates(store, call, doms)
                           : !enteredAfterStore(caller)))
        return false;
    }
  }
  return true;
}

std::string icmpCond(const Instruction* inst) {
  if (!inst)
    return {};
  auto name = inst->name();
  auto pos = name.find('_');
  return pos == std::string::npos ? name : name.substr(0, pos);
}

struct RecursiveBounds {
  bool hasLower = false;
  bool hasUpper = false;
  int32_t lower = std::numeric_limits<int32_t>::min();
  int32_t upper = std::numeric_limits<int32_t>::max();
};

struct KnownIntRange {
  bool valid = false;
  int64_t minValue = 0;
  int64_t maxValue = 0;
};

void constrainLower(RecursiveBounds& bounds, int32_t lower) {
  if (!bounds.hasLower || lower > bounds.lower) {
    bounds.lower = lower;
    bounds.hasLower = true;
  }
}

void constrainUpper(RecursiveBounds& bounds, int32_t upper) {
  if (!bounds.hasUpper || upper < bounds.upper) {
    bounds.upper = upper;
    bounds.hasUpper = true;
  }
}

RecursiveStateDirection mergeRecursiveDirection(RecursiveStateDirection lhs,
                                                RecursiveStateDirection rhs) {
  if (lhs == RecursiveStateDirection::Unknown)
    return rhs;
  if (rhs == RecursiveStateDirection::Unknown)
    return lhs;
  if (lhs == rhs)
    return lhs;
  if (lhs == RecursiveStateDirection::Stable)
    return rhs;
  if (rhs == RecursiveStateDirection::Stable)
    return lhs;
  return RecursiveStateDirection::Unknown;
}

const Arg* functionArgumentAt(const Function* func, size_t index) {
  if (!func || index >= func->args().size())
    return nullptr;
  return &func->args()[index];
}

bool extractArgumentDelta(const Function* func,
                          Value* expr,
                          Value* argValue,
                          int32_t& deltaOut) {
  if (!func || !expr || !argValue)
    return false;
  if (expr == argValue) {
    deltaOut = 0;
    return true;
  }
  auto* inst = dynamic_cast<Instruction*>(expr);
  if (!inst || inst->numOperands() == 0)
    return false;
  if ((inst->opcode() == Opcode::Mov || inst->opcode() == Opcode::BitCast) &&
      inst->numOperands() == 1) {
    return extractArgumentDelta(func, inst->operand(0), argValue, deltaOut);
  }

  auto foldConst = [](Value* value, int32_t& out) -> bool {
    auto* ci = dynamic_cast<core_ir::ConstInt*>(value);
    if (!ci)
      return false;
    out = ci->value();
    return true;
  };

  if ((inst->opcode() == Opcode::Add || inst->opcode() == Opcode::Sub) &&
      inst->numOperands() == 2) {
    int32_t innerDelta = 0;
    int32_t constValue = 0;
    if (foldConst(inst->operand(1), constValue) &&
        extractArgumentDelta(func, inst->operand(0), argValue, innerDelta)) {
      deltaOut =
          inst->opcode() == Opcode::Add ? innerDelta + constValue
                                        : innerDelta - constValue;
      return true;
    }
    if (inst->opcode() == Opcode::Add &&
        foldConst(inst->operand(0), constValue) &&
        extractArgumentDelta(func, inst->operand(1), argValue, innerDelta)) {
      deltaOut = innerDelta + constValue;
      return true;
    }
  }

  return false;
}

RecursiveBounds collectArgumentCompareBounds(const Function* func,
                                             Value* argValue) {
  RecursiveBounds bounds;
  if (!func || !argValue)
    return bounds;
  for (auto& bb : func->blocks()) {
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst || inst->opcode() != Opcode::ICmp || inst->numOperands() != 2)
        continue;
      auto cond = icmpCond(inst);
      auto* lhsConst = dynamic_cast<core_ir::ConstInt*>(inst->operand(0));
      auto* rhsConst = dynamic_cast<core_ir::ConstInt*>(inst->operand(1));
      if (inst->operand(0) == argValue && rhsConst) {
        int32_t c = rhsConst->value();
        if (cond == "lt")
          constrainUpper(bounds, c - 1);
        else if (cond == "le")
          constrainUpper(bounds, c);
        else if (cond == "gt")
          constrainLower(bounds, c + 1);
        else if (cond == "ge")
          constrainLower(bounds, c);
      } else if (inst->operand(1) == argValue && lhsConst) {
        int32_t c = lhsConst->value();
        if (cond == "lt")
          constrainLower(bounds, c + 1);
        else if (cond == "le")
          constrainLower(bounds, c);
        else if (cond == "gt")
          constrainUpper(bounds, c - 1);
        else if (cond == "ge")
          constrainUpper(bounds, c);
      }
    }
  }
  return bounds;
}

KnownIntRange knownValueRange(Value* value, int depth = 0) {
  if (!value || depth > 6)
    return {};
  if (auto* ci = dynamic_cast<core_ir::ConstInt*>(value))
    return {true, ci->value(), ci->value()};

  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !inst->type().isI32() || inst->numOperands() == 0)
    return {};

  switch (inst->opcode()) {
  case Opcode::Mov:
  case Opcode::BitCast:
    if (inst->numOperands() == 1)
      return knownValueRange(inst->operand(0), depth + 1);
    break;
  case Opcode::Add:
  case Opcode::Sub: {
    if (inst->numOperands() != 2)
      break;
    KnownIntRange lhs = knownValueRange(inst->operand(0), depth + 1);
    KnownIntRange rhs = knownValueRange(inst->operand(1), depth + 1);
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
    if (minValue >= std::numeric_limits<int32_t>::min() &&
        maxValue <= std::numeric_limits<int32_t>::max()) {
      return {true, minValue, maxValue};
    }
    break;
  }
  default:
    break;
  }
  return {};
}

RecursiveArgumentRange analyzeRecursiveArgumentRange(const Module& mod,
                                                    const Function* func,
                                                    size_t argIndex) {
  RecursiveArgumentRange range;
  const Arg* arg = functionArgumentAt(func, argIndex);
  if (!func || !arg || !arg->val)
    return range;

  RecursiveBounds bounds = collectArgumentCompareBounds(func, arg->val);
  int32_t stepGcd = 0;
  RecursiveStateDirection direction = RecursiveStateDirection::Unknown;
  for (auto& bb : func->blocks()) {
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst || inst->opcode() != Opcode::Call || inst->name() != func->name() ||
          argIndex >= static_cast<size_t>(inst->numOperands())) {
        continue;
      }
      int32_t delta = 0;
      if (!extractArgumentDelta(func, inst->operand(static_cast<int>(argIndex)),
                                arg->val, delta)) {
        direction = RecursiveStateDirection::Unknown;
        stepGcd = 0;
        goto finalize_direction;
      }
      if (delta < 0) {
        direction = mergeRecursiveDirection(direction,
                                            RecursiveStateDirection::NonIncreasing);
      } else if (delta > 0) {
        direction = mergeRecursiveDirection(direction,
                                            RecursiveStateDirection::NonDecreasing);
      } else {
        direction = mergeRecursiveDirection(direction,
                                            RecursiveStateDirection::Stable);
      }
      if (delta != 0) {
        stepGcd = stepGcd == 0 ? std::abs(delta) : std::gcd(stepGcd, std::abs(delta));
      }
    }
  }
finalize_direction:
  range.direction = direction;
  range.hasKnownStep = stepGcd > 0;
  range.step = stepGcd;

  bool callsiteValid = false;
  int64_t callsiteMin = 0;
  int64_t callsiteMax = 0;
  for (auto& callerPtr : mod.functions()) {
    Function* caller = callerPtr.get();
    for (auto& bb : caller->blocks()) {
      for (auto& instPtr : bb->instructions()) {
        Instruction* inst = instPtr.get();
        if (!inst || inst->opcode() != Opcode::Call || inst->name() != func->name() ||
            inst->name() == caller->name() ||
            argIndex >= static_cast<size_t>(inst->numOperands())) {
          continue;
        }
        KnownIntRange known =
            knownValueRange(inst->operand(static_cast<int>(argIndex)));
        if (!known.valid)
          continue;
        if (!callsiteValid) {
          callsiteMin = known.minValue;
          callsiteMax = known.maxValue;
          callsiteValid = true;
        } else {
          callsiteMin = std::min(callsiteMin, known.minValue);
          callsiteMax = std::max(callsiteMax, known.maxValue);
        }
      }
    }
  }

  range.nonNegative =
      (bounds.hasLower && bounds.lower >= 0) || (callsiteValid && callsiteMin >= 0);

  if (bounds.hasLower && bounds.hasUpper && bounds.lower <= bounds.upper) {
    range.valid = true;
    range.minValue = bounds.lower;
    range.maxValue = bounds.upper;
  } else if (callsiteValid) {
    if (range.direction == RecursiveStateDirection::NonIncreasing &&
        range.nonNegative) {
      range.valid = true;
      range.minValue = bounds.hasLower ? bounds.lower : 0;
      range.maxValue = static_cast<int32_t>(callsiteMax);
    } else if (range.direction == RecursiveStateDirection::NonDecreasing &&
               bounds.hasUpper) {
      range.valid = true;
      range.minValue = static_cast<int32_t>(callsiteMin);
      range.maxValue = bounds.upper;
    } else if (callsiteMin >= std::numeric_limits<int32_t>::min() &&
               callsiteMax <= std::numeric_limits<int32_t>::max()) {
      range.valid = true;
      range.minValue = static_cast<int32_t>(callsiteMin);
      range.maxValue = static_cast<int32_t>(callsiteMax);
      if (bounds.hasLower)
        range.minValue = std::max(range.minValue, bounds.lower);
      if (bounds.hasUpper)
        range.maxValue = std::min(range.maxValue, bounds.upper);
    }
  }

  if (range.valid && range.minValue > range.maxValue)
    range.valid = false;
  if (range.valid)
    range.nonNegative = range.nonNegative || range.minValue >= 0;
  if (!range.valid) {
    range.minValue = 0;
    range.maxValue = 0;
  }
  return range;
}

void collectReferencedGlobals(Value* value,
                              std::unordered_set<const GlobalVar*>& out,
                              std::unordered_set<Value*>& visiting) {
  if (!value || !visiting.insert(value).second)
    return;
  const bool trace = sycEnvironmentValue("EFFECT_TRACE_RECURSE") != nullptr;
  if (auto* gv = dynamic_cast<GlobalVar*>(value)) {
    out.insert(gv);
    return;
  }
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst)
    return;
  for (int i = 0; i < inst->numOperands(); ++i) {
    if (trace) {
      std::cerr << "[effect-recurse] from=" << inst->id()
                << " op=" << i
                << " child=" << inst->operand(i) << '\n';
    }
    collectReferencedGlobals(inst->operand(i), out, visiting);
  }
}

std::vector<const GlobalVar*> collectStableScalarGlobals(const Function* func) {
  std::unordered_set<const GlobalVar*> globals;
  std::unordered_set<Value*> visiting;
  if (!func)
    return {};
  const bool trace = sycEnvironmentValue("EFFECT_TRACE") != nullptr;
  for (auto& bb : func->blocks()) {
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      for (int i = 0; i < inst->numOperands(); ++i) {
        if (trace) {
          std::cerr << "[effect-trace] func=" << func->name()
                    << " bb=" << bb->name()
                    << " inst=" << inst->id()
                    << " op=" << i
                    << " ptr=" << inst->operand(i) << '\n';
        }
        collectReferencedGlobals(inst->operand(i), globals, visiting);
      }
    }
  }
  std::vector<const GlobalVar*> stable;
  for (const GlobalVar* gv : globals) {
    if (!gv)
      continue;
    if (isMemoGlobal(gv))
      continue;
    if (gv->sizeBytes() == 4 && gv->dims().empty())
      stable.push_back(gv);
  }
  std::sort(stable.begin(), stable.end(),
            [](const GlobalVar* lhs, const GlobalVar* rhs) {
              return lhs->name() < rhs->name();
            });
  return stable;
}

bool referencesOnlyStableScalarGlobals(const Function* func) {
  std::unordered_set<const GlobalVar*> globals;
  std::unordered_set<Value*> visiting;
  if (!func)
    return false;
  const bool trace = sycEnvironmentValue("EFFECT_TRACE") != nullptr;
  for (auto& bb : func->blocks()) {
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      for (int i = 0; i < inst->numOperands(); ++i) {
        if (trace) {
          std::cerr << "[effect-trace] func=" << func->name()
                    << " bb=" << bb->name()
                    << " inst=" << inst->id()
                    << " op=" << i
                    << " ptr=" << inst->operand(i) << '\n';
        }
        collectReferencedGlobals(inst->operand(i), globals, visiting);
      }
    }
  }
  for (const GlobalVar* gv : globals) {
    if (!gv)
      continue;
    if (isMemoGlobal(gv))
      continue;
    if (gv->sizeBytes() != 4 || !gv->dims().empty())
      return false;
  }
  return true;
}

bool isTailRecursiveOnly(const Function* func) {
  if (!func)
    return false;
  for (auto& bb : func->blocks()) {
    Instruction* term = bb->terminator();
    if (!term || term->opcode() != Opcode::Ret || term->numOperands() != 1)
      continue;
    auto* call = dynamic_cast<Instruction*>(term->operand(0));
    if (call && call->opcode() == Opcode::Call && call->name() == func->name())
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (inst->opcode() == Opcode::Call && inst->name() == func->name())
        return false;
    }
  }
  return true;
}

RecursiveFunctionSummary analyzeRecursiveFunction(
    Module& mod,
    const Function* func,
    const FunctionEffectSummary* effects) {
  constexpr size_t kDenseMemoBudgetBytes = 16u * 1024u * 1024u;
  constexpr size_t kDenseMemoCellBytes = 8u;
  constexpr size_t kPersistentDensePrefixCells =
      kDenseMemoBudgetBytes / kDenseMemoCellBytes;
  constexpr size_t kTwoDimDensePrefixCells = 1u << 13;
  constexpr int32_t kSmallRecursivePrefixExtent = 32;

  RecursiveFunctionSummary summary;
  if (!func || !effects)
    return summary;

  summary.selfRecursive = effects->selfRecursive;
  summary.finiteStateI32Tuple = functionHasOnlyI32Args(func);
  summary.readonly = effects->readonly;
  summary.stableInvocationState = effects->stableInvocationState;
  summary.selfRecursiveCallCount = effects->selfRecursiveCallCount;
  summary.tailRecursiveOnly = isTailRecursiveOnly(func);
  summary.returnValuePure = func->retTy().isI32() &&
                            effects->readonly &&
                            effects->stableInvocationState;
  summary.stableScalarGlobals = collectStableScalarGlobals(func);
  summary.needsStateFingerprint = !summary.stableScalarGlobals.empty();
  if (summary.finiteStateI32Tuple) {
    summary.argumentRanges.reserve(func->args().size());
    for (size_t argIndex = 0; argIndex < func->args().size(); ++argIndex) {
      summary.argumentRanges.push_back(
          analyzeRecursiveArgumentRange(mod, func, argIndex));
    }
  }

  if (!summary.selfRecursive) {
    return summary;
  }

  int recursiveBranchCount = 0;
  for (auto& bb : func->blocks()) {
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (inst->opcode() != Opcode::Call || inst->name() != func->name())
        continue;
      ++recursiveBranchCount;
    }
  }

  // 只要有一个状态维度持续推进，就可以表示递归进展；其他维度可以依赖只读
  // 输入状态。
  const bool monotonicStateProgress =
      std::any_of(summary.argumentRanges.begin(), summary.argumentRanges.end(),
                  [](const RecursiveArgumentRange& range) {
                    return range.hasKnownStep &&
                           (range.direction ==
                                RecursiveStateDirection::NonIncreasing ||
                            range.direction ==
                                RecursiveStateDirection::NonDecreasing);
                  });
  summary.recursiveBranchCount = recursiveBranchCount;
  summary.monotonicStateProgress = monotonicStateProgress;
  summary.repeatedSubproblem =
      summary.finiteStateI32Tuple && summary.stableInvocationState &&
      summary.selfRecursiveCallCount >= 2 && recursiveBranchCount >= 2 &&
      monotonicStateProgress;
  const bool scalarOnlyStableState = referencesOnlyStableScalarGlobals(func);
  // 持久化记忆化由只读效果和状态指纹控制。单调性只用于布局提示，不作为额外
  // 的变换条件。
  summary.persistentMemoState =
      summary.finiteStateI32Tuple && effects->stableInvocationState &&
      recursiveBranchCount >= 2 && summary.selfRecursiveCallCount >= 2 &&
      scalarOnlyStableState &&
      summary.stableScalarGlobals.size() <= 4;
  summary.stableStateAcrossCalls =
      summary.persistentMemoState && stableGlobalsAreFixedBeforeCalls(
                                         mod, func,
                                         summary.stableScalarGlobals);

  auto finalizeDenseEstimate = [&](size_t cells) {
    summary.denseCellEstimate = cells;
    summary.denseByteEstimate = cells * kDenseMemoCellBytes;
  };
  auto setDenseLayout = [&](RecursiveMemoLayout layout,
                            std::vector<int32_t> lowerBounds,
                            std::vector<int32_t> extents) {
    summary.memoLayout = layout;
    summary.denseLowerBounds = std::move(lowerBounds);
    summary.denseExtents = std::move(extents);
  };
  auto chooseTwoDimDensePrefix = [&]() -> bool {
    if (summary.argumentRanges.size() != 2 || !summary.repeatedSubproblem ||
        summary.stableScalarGlobals.size() > 1)
      return false;

    std::vector<int32_t> lowerBounds(2, 0);
    std::vector<int32_t> extents(2, 0);
    size_t fixedCells = 1;
    int progressiveArg = -1;

    for (size_t i = 0; i < 2; ++i) {
      const auto& arg = summary.argumentRanges[i];
      if (arg.valid) {
        int64_t extent64 = static_cast<int64_t>(arg.maxValue) -
                           static_cast<int64_t>(arg.minValue) + 1;
        if (extent64 <= 0 ||
            extent64 > static_cast<int64_t>(std::numeric_limits<int32_t>::max()))
          return false;
        size_t extent = static_cast<size_t>(extent64);
        if (fixedCells > kTwoDimDensePrefixCells / extent)
          return false;
        lowerBounds[i] = arg.minValue;
        extents[i] = static_cast<int32_t>(extent);
        fixedCells *= extent;
        continue;
      }

      if (arg.direction == RecursiveStateDirection::NonIncreasing &&
          arg.hasKnownStep && progressiveArg < 0) {
        progressiveArg = static_cast<int>(i);
      } else if (arg.direction == RecursiveStateDirection::NonDecreasing) {
        return false;
      }
    }

    if (progressiveArg < 0)
      return false;
    if (fixedCells == 0 || fixedCells > kTwoDimDensePrefixCells)
      return false;

    size_t remaining = kTwoDimDensePrefixCells / fixedCells;
    if (remaining == 0)
      return false;
    for (size_t i = 0; i < 2; ++i) {
      if (extents[i] != 0)
        continue;
      if (static_cast<int>(i) == progressiveArg) {
        size_t extent =
            std::min<size_t>(kSmallRecursivePrefixExtent, remaining);
        extents[i] = static_cast<int32_t>(std::max<size_t>(1, extent));
        remaining /= static_cast<size_t>(extents[i]);
      }
    }
    for (size_t i = 0; i < 2; ++i) {
      if (extents[i] != 0)
        continue;
      extents[i] =
          static_cast<int32_t>(std::min<size_t>(
              remaining, static_cast<size_t>(std::numeric_limits<int32_t>::max())));
      remaining = 1;
    }

    size_t cells = 1;
    for (int32_t extent : extents) {
      if (extent <= 0 ||
          cells > std::numeric_limits<size_t>::max() /
                      static_cast<size_t>(extent))
        return false;
      cells *= static_cast<size_t>(extent);
    }
    finalizeDenseEstimate(cells);
    if (summary.denseCellEstimate <= 1 ||
        summary.denseByteEstimate > kDenseMemoBudgetBytes)
      return false;
    setDenseLayout(RecursiveMemoLayout::Dense2D, std::move(lowerBounds),
                   std::move(extents));
    return true;
  };

  if (summary.returnValuePure && (summary.repeatedSubproblem ||
      summary.persistentMemoState) &&
      effects->instructionCount <= 256 && effects->loopCount <= 2) {
    summary.mode = RecursiveOptimizationMode::Memoized;
  } else if (summary.returnValuePure) {
    summary.mode = RecursiveOptimizationMode::FastPath;
  }

  // Memoized 模式优先按实参取值范围构造稠密记忆表：单实参取一维、
  // 双实参取二维，格子数与字节预算不超标时才选用对应稠密布局。
  if (summary.mode == RecursiveOptimizationMode::Memoized &&
      summary.finiteStateI32Tuple &&
      summary.stableScalarGlobals.size() <= 1) {
    if (summary.argumentRanges.size() == 1 && summary.argumentRanges[0].valid) {
      const auto& arg0 = summary.argumentRanges[0];
      size_t cells =
          static_cast<size_t>(static_cast<int64_t>(arg0.maxValue) -
                              static_cast<int64_t>(arg0.minValue) + 1);
      finalizeDenseEstimate(cells);
      if (cells > 1 && summary.denseByteEstimate <= kDenseMemoBudgetBytes) {
        setDenseLayout(RecursiveMemoLayout::Dense1D,
                       {arg0.minValue},
                       {static_cast<int32_t>(cells)});
      }
    } else if (summary.argumentRanges.size() == 2 &&
               summary.argumentRanges[0].valid &&
               summary.argumentRanges[1].valid) {
      const auto& arg0 = summary.argumentRanges[0];
      const auto& arg1 = summary.argumentRanges[1];
      size_t extent0 =
          static_cast<size_t>(static_cast<int64_t>(arg0.maxValue) -
                              static_cast<int64_t>(arg0.minValue) + 1);
      size_t extent1 =
          static_cast<size_t>(static_cast<int64_t>(arg1.maxValue) -
                              static_cast<int64_t>(arg1.minValue) + 1);
      if (extent0 > 0 && extent1 > 0 &&
          extent0 <= std::numeric_limits<size_t>::max() / extent1) {
        finalizeDenseEstimate(extent0 * extent1);
        if (summary.denseCellEstimate > 1 &&
            summary.denseByteEstimate <= kDenseMemoBudgetBytes) {
          setDenseLayout(RecursiveMemoLayout::Dense2D,
                         {arg0.minValue, arg1.minValue},
                         {static_cast<int32_t>(extent0),
                          static_cast<int32_t>(extent1)});
        }
      }
    }
    if (summary.memoLayout == RecursiveMemoLayout::None &&
        monotonicStateProgress) {
      chooseTwoDimDensePrefix();
    }
    if (summary.memoLayout == RecursiveMemoLayout::None &&
        summary.persistentMemoState && summary.argumentRanges.size() == 1) {
      // 对一维持久化状态直接索引受保护的前缀，即使上界来自调用状态而不是
      // 常量。超出前缀的值跳过记忆表存储，因此这是有界布局选择，不依赖某个
      // 固定输入规模。
      finalizeDenseEstimate(kPersistentDensePrefixCells);
      setDenseLayout(RecursiveMemoLayout::Dense1D,
                     {0},
                     {static_cast<int32_t>(kPersistentDensePrefixCells)});
    }
    if (summary.memoLayout == RecursiveMemoLayout::None) {
      summary.memoLayout = RecursiveMemoLayout::HashFallback;
    }
  }

  return summary;
}

FunctionEffectSummary computeSeedSummary(Function* func) {
  FunctionEffectSummary summary;
  if (!func)
    return summary;

  summary.instructionCount = instructionCount(func);
  std::vector<BasicBlock*> blocks;
  for (auto& bb : func->blocks())
    blocks.push_back(bb.get());
  auto reachable = collectReachableBlocks(func);
  auto doms = computeDominators(func, blocks, reachable);
  summary.loopCount = static_cast<int>(
      collectNaturalLoops(blocks, doms, reachable).size());
  return summary;
}

FunctionEffectSummary analyzeFunction(Function* func,
                                      Module& mod,
                                      const FunctionEffectMap& known) {
  FunctionEffectSummary summary = computeSeedSummary(func);
  if (!isEffectCandidate(func))
    return summary;

  bool pure = true;
  bool readonly = true;
  bool argMemOnly = true;
  bool noRecurse = true;
  bool selfRecursive = false;
  int selfRecursiveCallCount = 0;
  bool readsArgMemory = false;
  bool readsMutableGlobalMemory = false;
  bool readsUnknownMemory = false;
  bool writesArgMemory = false;
  bool writesMutableGlobalMemory = false;
  bool writesUnknownMemory = false;
  std::unordered_set<std::size_t> readArgumentIndices;
  std::unordered_set<std::size_t> writtenArgumentIndices;
  std::unordered_set<std::size_t> capturedArgumentIndices;
  std::unordered_set<std::size_t> returnedArgumentIndices;
  std::unordered_set<const GlobalVar*> writtenGlobals;

  // 逐指令扫描累计效果事实：纯算术/转换/控制流指令不影响结论；
  // Load/Store 按内存分类（局部/参数/全局/未知）细化 pure/readonly/argMemOnly
  // 等标志；Call 则合并被调函数的已知摘要。
  for (auto& bb : func->blocks()) {
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      switch (inst->opcode()) {
      case Opcode::Alloca:
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
      case Opcode::Phi:
      case Opcode::Neg:
      case Opcode::FNeg:
      case Opcode::BitCast:
      case Opcode::Mov:
      case Opcode::Br:
      case Opcode::CondBr:
        break;
      case Opcode::Ret:
        if (inst->numOperands() == 1 && inst->operand(0) &&
            inst->operand(0)->type().isPtr()) {
          if (auto index = argumentIndex(func, inst->operand(0))) {
            returnedArgumentIndices.insert(*index);
            capturedArgumentIndices.insert(*index);
          } else {
            // 返回某个已知 callee 的指针形参时，将其返回索引
            // 重映射到当前函数的实参。
            auto* call = dynamic_cast<Instruction*>(inst->operand(0));
            if (call && call->opcode() == Opcode::Call) {
              Function* callee = mod.getFunction(call->name());
              const FunctionEffectSummary* calleeSummary =
                  callee ? lookupFunctionEffect(known, callee) : nullptr;
              if (calleeSummary) {
                for (std::size_t returned :
                     calleeSummary->returnedArgumentIndices) {
                  if (returned >=
                      static_cast<std::size_t>(call->numOperands()))
                    continue;
                  if (auto callerIndex = argumentIndex(
                          func, call->operand(static_cast<int>(returned)))) {
                    returnedArgumentIndices.insert(*callerIndex);
                    capturedArgumentIndices.insert(*callerIndex);
                  }
                }
              }
            }
          }
        }
        break;
      case Opcode::Load: {
        if (inst->numOperands() != 1) {
          pure = readonly = argMemOnly = false;
          readsUnknownMemory = true;
          break;
        }
        MemoryKind kind = classifyMemory(inst->operand(0));
        if (kind != MemoryKind::Local && kind != MemoryKind::ConstGlobal)
          pure = false;
        if (kind != MemoryKind::Local && kind != MemoryKind::Arg)
          argMemOnly = false;
        readsArgMemory |= kind == MemoryKind::Arg;
        if (kind == MemoryKind::Arg) {
          if (auto index = argumentIndex(func, inst->operand(0)))
            readArgumentIndices.insert(*index);
          else
            readsUnknownMemory = true;
        }
        readsMutableGlobalMemory |= kind == MemoryKind::Global;
        readsUnknownMemory |= kind == MemoryKind::Unknown;
        break;
      }
      case Opcode::Store: {
        if (inst->numOperands() != 2) {
          pure = readonly = argMemOnly = false;
          writesUnknownMemory = true;
          break;
        }
        MemoryKind kind = classifyMemory(inst->operand(1));
        if (kind != MemoryKind::Local)
          pure = false;
        if (kind != MemoryKind::Local)
          readonly = false;
        if (kind != MemoryKind::Local && kind != MemoryKind::Arg)
          argMemOnly = false;
        writesArgMemory |= kind == MemoryKind::Arg;
        if (kind == MemoryKind::Arg) {
          if (auto index = argumentIndex(func, inst->operand(1)))
            writtenArgumentIndices.insert(*index);
          else
            writesUnknownMemory = true;
        }
        writesMutableGlobalMemory |= kind == MemoryKind::Global;
        writesUnknownMemory |= kind == MemoryKind::Unknown;
        if (kind == MemoryKind::Global) {
          if (auto* global =
                  dynamic_cast<GlobalVar*>(memoryRoot(inst->operand(1)))) {
            writtenGlobals.insert(global);
          } else {
            writesUnknownMemory = true;
          }
        }
        if (inst->operand(0) && inst->operand(0)->type().isPtr()) {
          if (auto index = argumentIndex(func, inst->operand(0)))
            capturedArgumentIndices.insert(*index);
        }
        break;
      }
      case Opcode::Call: {
        if (inst->name() == "__syc_modmul_i32" &&
            !mod.getFunction(inst->name())) {
          // 这是编译器内部生成的纯算术 helper，后端直接降为
          // 少量 RV64 算术。模块内若存在同名用户函数则不会进入此分支。
          break;
        }
        // 常规调用：被调函数未知则保守置为不纯；自递归单独计数；
        // 其余合并被调摘要的读写事实（写参内存时逐指针实参再分类）。
        Function* callee = mod.getFunction(inst->name());
        if (!callee) {
          pure = readonly = argMemOnly = false;
          noRecurse = false;
          readsUnknownMemory = true;
          writesUnknownMemory = true;
          for (Value* operand : inst->operands())
            if (operand && operand->type().isPtr())
              if (auto index = argumentIndex(func, operand))
                capturedArgumentIndices.insert(*index);
          break;
        }
        if (callee == func) {
          selfRecursive = true;
          ++selfRecursiveCallCount;
          noRecurse = false;
          break;
        }
        const FunctionEffectSummary* calleeSummary =
            lookupFunctionEffect(known, callee);
        if (!calleeSummary) {
          pure = readonly = argMemOnly = false;
          noRecurse = false;
          break;
        }
        pure &= calleeSummary->pure;
        readonly &= calleeSummary->readonly;
        argMemOnly &= calleeSummary->argMemOnly;
        noRecurse &= calleeSummary->noRecurse;
        readsMutableGlobalMemory |= calleeSummary->readsMutableGlobalMemory;
        readsUnknownMemory |= calleeSummary->readsUnknownMemory;
        writesUnknownMemory |= calleeSummary->writesUnknownMemory;
        writtenGlobals.insert(calleeSummary->writtenGlobals.begin(),
                              calleeSummary->writtenGlobals.end());
        writesMutableGlobalMemory |=
            calleeSummary->writesMutableGlobalMemory;
        for (std::size_t calleeIndex : calleeSummary->readArgumentIndices) {
          if (calleeIndex >= static_cast<std::size_t>(inst->numOperands())) {
            readsUnknownMemory = true;
            continue;
          }
          Value* operand = inst->operand(static_cast<int>(calleeIndex));
          MemoryKind kind = classifyMemory(operand);
          readsArgMemory |= kind == MemoryKind::Arg;
          readsMutableGlobalMemory |= kind == MemoryKind::Global;
          readsUnknownMemory |= kind == MemoryKind::Unknown;
          if (kind == MemoryKind::Arg) {
            if (auto callerIndex = argumentIndex(func, operand))
              readArgumentIndices.insert(*callerIndex);
            else
              readsUnknownMemory = true;
          }
        }
        for (std::size_t calleeIndex : calleeSummary->writtenArgumentIndices) {
          if (calleeIndex >= static_cast<std::size_t>(inst->numOperands())) {
            writesUnknownMemory = true;
            continue;
          }
          Value* operand = inst->operand(static_cast<int>(calleeIndex));
          MemoryKind kind = classifyMemory(operand);
            writesArgMemory |= kind == MemoryKind::Arg;
            writesMutableGlobalMemory |= kind == MemoryKind::Global;
            writesUnknownMemory |= kind == MemoryKind::Unknown;
            if (kind == MemoryKind::Arg) {
              if (auto callerIndex = argumentIndex(func, operand))
                writtenArgumentIndices.insert(*callerIndex);
              else
                writesUnknownMemory = true;
            }
            if (kind == MemoryKind::Global) {
              if (auto* global =
                      dynamic_cast<GlobalVar*>(memoryRoot(operand))) {
                writtenGlobals.insert(global);
              } else {
                writesUnknownMemory = true;
              }
            }
        }
        for (std::size_t calleeIndex :
             calleeSummary->capturedArgumentIndices) {
          if (calleeIndex >= static_cast<std::size_t>(inst->numOperands()))
            continue;
          if (auto callerIndex = argumentIndex(
                  func, inst->operand(static_cast<int>(calleeIndex))))
            capturedArgumentIndices.insert(*callerIndex);
        }
        break;
      }
      default:
        pure = readonly = argMemOnly = false;
        readsUnknownMemory = true;
        writesUnknownMemory = true;
        break;
      }
    }
  }

  summary.pure = pure;
  summary.readonly = readonly;
  summary.argMemOnly = argMemOnly;
  summary.noRecurse = noRecurse;
  summary.selfRecursive = selfRecursive;
  summary.selfRecursiveCallCount = selfRecursiveCallCount;
  summary.readsArgMemory = readsArgMemory;
  summary.readsMutableGlobalMemory = readsMutableGlobalMemory;
  summary.readsUnknownMemory = readsUnknownMemory;
  summary.writesArgMemory = writesArgMemory;
  summary.writesMutableGlobalMemory = writesMutableGlobalMemory;
  summary.writesUnknownMemory = writesUnknownMemory;
  summary.readArgumentIndices = std::move(readArgumentIndices);
  summary.writtenArgumentIndices = std::move(writtenArgumentIndices);
  summary.capturedArgumentIndices = std::move(capturedArgumentIndices);
  summary.returnedArgumentIndices = std::move(returnedArgumentIndices);
  summary.writtenGlobals = std::move(writtenGlobals);
  summary.stableGlobalRead =
      readonly && !readsArgMemory && !readsMutableGlobalMemory &&
      !readsUnknownMemory;
  summary.stableInvocationState =
      readonly && !readsArgMemory && !readsUnknownMemory;
  int helperInstBudget = summary.pure ? 256 : (summary.readonly ? 256 : 48);
  summary.smallHelper = summary.noRecurse &&
                        summary.instructionCount <= helperInstBudget &&
                        summary.loopCount <= 1;
  if (summary.readonly && summary.instructionCount <= 64 && summary.loopCount == 0)
    summary.smallHelper = true;
  RecursiveFunctionSummary recursiveSummary =
      analyzeRecursiveFunction(mod, func, &summary);
  summary.memoizableRecursion =
      recursiveSummary.mode == RecursiveOptimizationMode::Memoized;
  return summary;
}

} // namespace

FunctionEffectMap summarizeModuleFunctionEffects(Module& mod) {
  FunctionEffectMap effects;
  for (auto& funcPtr : mod.functions())
    effects[funcPtr.get()] = computeSeedSummary(funcPtr.get());

  bool changed = true;
  while (changed) {
    changed = false;
    for (auto& funcPtr : mod.functions()) {
      Function* func = funcPtr.get();
      FunctionEffectSummary summary = analyzeFunction(func, mod, effects);
      auto it = effects.find(func);
      if (it == effects.end() || it->second.pure != summary.pure ||
          it->second.readonly != summary.readonly ||
          it->second.argMemOnly != summary.argMemOnly ||
          it->second.noRecurse != summary.noRecurse ||
          it->second.smallHelper != summary.smallHelper ||
          it->second.selfRecursive != summary.selfRecursive ||
          it->second.readsArgMemory != summary.readsArgMemory ||
          it->second.readsMutableGlobalMemory != summary.readsMutableGlobalMemory ||
          it->second.readsUnknownMemory != summary.readsUnknownMemory ||
          it->second.writesArgMemory != summary.writesArgMemory ||
          it->second.writesMutableGlobalMemory !=
              summary.writesMutableGlobalMemory ||
          it->second.writesUnknownMemory != summary.writesUnknownMemory ||
          it->second.readArgumentIndices != summary.readArgumentIndices ||
          it->second.writtenArgumentIndices !=
              summary.writtenArgumentIndices ||
          it->second.capturedArgumentIndices !=
              summary.capturedArgumentIndices ||
          it->second.returnedArgumentIndices !=
              summary.returnedArgumentIndices ||
          it->second.writtenGlobals != summary.writtenGlobals ||
          it->second.stableGlobalRead != summary.stableGlobalRead ||
          it->second.stableInvocationState != summary.stableInvocationState ||
          it->second.memoizableRecursion != summary.memoizableRecursion ||
          it->second.selfRecursiveCallCount != summary.selfRecursiveCallCount) {
        effects[func] = summary;
        changed = true;
      }
    }
  }

  return effects;
}

RecursiveFunctionMap summarizeModuleRecursiveFunctions(
    Module& mod,
    const FunctionEffectMap& effects) {
  RecursiveFunctionMap summaries;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    summaries.emplace(func, analyzeRecursiveFunction(
                                mod,
                                func, lookupFunctionEffect(effects, func)));
  }
  return summaries;
}

const FunctionEffectSummary* lookupFunctionEffect(
    const FunctionEffectMap& effects,
    const Function* func) {
  auto it = effects.find(func);
  return it == effects.end() ? nullptr : &it->second;
}

const FunctionEffectSummary* lookupFunctionEffect(
    const FunctionEffectMap& effects,
    const Module& mod,
    const std::string& calleeName) {
  return lookupFunctionEffect(effects, mod.getFunction(calleeName));
}

const RecursiveFunctionSummary* lookupRecursiveSummary(
    const RecursiveFunctionMap& summaries,
    const Function* func) {
  auto it = summaries.find(func);
  return it == summaries.end() ? nullptr : &it->second;
}

const char* recursiveOptimizationModeName(RecursiveOptimizationMode mode) {
  switch (mode) {
  case RecursiveOptimizationMode::Memoized:
    return "memoized";
  case RecursiveOptimizationMode::FastPath:
    return "recursive-fastpath";
  case RecursiveOptimizationMode::None:
  default:
    return "none";
  }
}

const char* recursiveMemoLayoutName(RecursiveMemoLayout layout) {
  switch (layout) {
  case RecursiveMemoLayout::Dense1D:
    return "dense1d";
  case RecursiveMemoLayout::Dense2D:
    return "dense2d";
  case RecursiveMemoLayout::HashFallback:
    return "hash-fallback";
  case RecursiveMemoLayout::None:
  default:
    return "none";
  }
}

bool isPureCall(const Instruction* inst,
                const Module& mod,
                const FunctionEffectMap& effects) {
  if (!inst || inst->opcode() != Opcode::Call)
    return false;
  const auto* summary = lookupFunctionEffect(effects, mod, inst->name());
  return summary && summary->pure;
}

bool isReadonlyCall(const Instruction* inst,
                    const Module& mod,
                    const FunctionEffectMap& effects) {
  if (!inst || inst->opcode() != Opcode::Call)
    return false;
  const auto* summary = lookupFunctionEffect(effects, mod, inst->name());
  return summary && summary->readonly;
}

bool isSmallHelperCall(const Instruction* inst,
                       const Module& mod,
                       const FunctionEffectMap& effects) {
  if (!inst || inst->opcode() != Opcode::Call)
    return false;
  const auto* summary = lookupFunctionEffect(effects, mod, inst->name());
  return summary && summary->smallHelper;
}

bool isMemoizableRecursiveFunction(const RecursiveFunctionSummary* summary,
                                   const Function* func) {
  if (!summary || !func)
    return false;
  if (summary->mode != RecursiveOptimizationMode::Memoized)
    return false;
  if (!summary->selfRecursive || !summary->finiteStateI32Tuple)
    return false;
  if (!summary->returnValuePure)
    return false;
  return func->retTy().isI32();
}

bool isMemoizableRecursiveFunction(const FunctionEffectSummary* summary,
                                   const Function* func) {
  if (!summary || !func)
    return false;
  RecursiveFunctionSummary recursiveSummary =
      analyzeRecursiveFunction(*func->parent(), func, summary);
  return isMemoizableRecursiveFunction(&recursiveSummary, func);
}

} // namespace analysis

#endif
