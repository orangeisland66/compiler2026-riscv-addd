// 文件职责：识别归纳变量和循环步进形式，服务循环规范化与强度削减。本文件负责实现具体逻辑。
#include "induction_var.h"
#include "../hir/canonical_loop.h"

#include <algorithm>
#include <limits>
#include <unordered_set>

namespace analysis {

namespace {

using namespace core_ir;

bool isDirectScalarSlotLoad(Value* value, Instruction* slot) {
  auto* load = dynamic_cast<Instruction*>(value);
  return load && load->opcode() == Opcode::Load && load->numOperands() == 1 &&
         load->operand(0) == slot;
}

bool matchAddSubStep(Value* value, Instruction* slot, Value*& stepVal,
                     int64_t& step) {
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->numOperands() != 2)
    return false;
  if (inst->opcode() != Opcode::Add && inst->opcode() != Opcode::Sub)
    return false;

  int sign = 1;
  Value* other = nullptr;
  if (isDirectScalarSlotLoad(inst->operand(0), slot)) {
    other = inst->operand(1);
    if (inst->opcode() == Opcode::Sub)
      sign = -1;
  } else if (inst->opcode() == Opcode::Add &&
             isDirectScalarSlotLoad(inst->operand(1), slot)) {
    other = inst->operand(0);
  } else {
    return false;
  }

  auto* ci = dynamic_cast<ConstInt*>(other);
  if (!ci)
    return false;
  stepVal = other;
  step = static_cast<int64_t>(ci->value()) * sign;
  return true;
}

bool storeDefinesSlot(Instruction* inst, Instruction* slot) {
  return inst && inst->opcode() == Opcode::Store && inst->numOperands() == 2 &&
         inst->operand(1) == slot;
}

bool tryFindScalarSlotInduction(Function* func, const NaturalLoop& loop,
                                Instruction* slot, InductionVar& iv) {
  if (!func || !slot || !loop.header || !loop.blocks.count(loop.header))
    return false;

  Value* init = nullptr;
  std::unordered_set<BasicBlock*> bodyStores;
  bool sawHeaderLoad = false;

  for (auto* user : slot->uses()) {
    if (!user || !user->parent())
      continue;
    BasicBlock* bb = user->parent();
    if (user->opcode() == Opcode::Load && user->numOperands() == 1 &&
        user->operand(0) == slot && bb == loop.header) {
      sawHeaderLoad = true;
      continue;
    }
    if (!loop.blocks.count(bb)) {
      if (storeDefinesSlot(user, slot) && bb->succs().size() == 1 &&
          bb->succs().front() == loop.header) {
        if (!init) {
          init = user->operand(0);
        } else if (init != user->operand(0)) {
          return false;
        }
      }
      continue;
    }

    if (!storeDefinesSlot(user, slot))
      continue;
    bodyStores.insert(bb);
  }

  if (!init || !sawHeaderLoad || bodyStores.empty())
    return false;

  Value* commonStepVal = nullptr;
  bool sawBackEdge = false;
  bool sawConstStep = false;
  int64_t canonicalStep = 0;

  for (auto* latch : loop.latches) {
    if (!latch || !loop.blocks.count(latch))
      continue;
    sawBackEdge = true;

    Instruction* storeInst = nullptr;
    for (auto& instPtr : latch->instructions()) {
      auto* inst = instPtr.get();
      if (storeDefinesSlot(inst, slot))
        storeInst = inst;
    }
    if (!storeInst)
      return false;

    Value* stepVal = nullptr;
    int64_t step = 0;
    if (!matchAddSubStep(storeInst->operand(0), slot, stepVal, step))
      return false;

    if (!commonStepVal) {
      commonStepVal = stepVal;
    } else if (commonStepVal != stepVal) {
      commonStepVal = nullptr;
    }

    if (!sawConstStep) {
      canonicalStep = step;
      sawConstStep = true;
    } else if (canonicalStep != step) {
      return false;
    }
  }

  if (!sawBackEdge || !sawConstStep)
    return false;

  iv.phi = nullptr;
  iv.storage = slot;
  iv.init = init;
  iv.step = canonicalStep;
  iv.stepVal = commonStepVal;
  iv.isCanonical = true;
  return true;
}

std::string invertPredicate(std::string predicate) {
  if (predicate == "eq") return "ne";
  if (predicate == "ne") return "eq";
  if (predicate == "lt") return "ge";
  if (predicate == "le") return "gt";
  if (predicate == "gt") return "le";
  if (predicate == "ge") return "lt";
  return {};
}

std::string swapPredicate(std::string predicate) {
  if (predicate == "lt") return "gt";
  if (predicate == "le") return "ge";
  if (predicate == "gt") return "lt";
  if (predicate == "ge") return "le";
  return predicate;
}

bool checkedTripCount(int32_t initial, int32_t bound, int64_t step,
                      const std::string& predicate, int64_t& tripCount) {
  if (step == 0) return false;
  const __int128 distance = static_cast<__int128>(bound) - initial;
  __int128 trip = -1;
  if (predicate == "lt" && step > 0)
    trip = distance <= 0 ? 0 : (distance + step - 1) / step;
  else if (predicate == "le" && step > 0)
    trip = distance < 0 ? 0 : distance / step + 1;
  else if (predicate == "gt" && step < 0)
    trip = distance >= 0 ? 0 : (-distance + (-step) - 1) / (-step);
  else if (predicate == "ge" && step < 0)
    trip = distance > 0 ? 0 : (-distance) / (-step) + 1;
  else if (predicate == "ne" && distance == 0)
    trip = 0;
  else if (predicate == "ne" && distance % step == 0 &&
           ((distance > 0) == (step > 0)))
    trip = distance / step;
  if (trip < 0 || trip > std::numeric_limits<int64_t>::max()) return false;
  const __int128 finalValue = static_cast<__int128>(initial) + trip * step;
  if (finalValue < std::numeric_limits<int32_t>::min() ||
      finalValue > std::numeric_limits<int32_t>::max())
    return false;
  tripCount = static_cast<int64_t>(trip);
  return true;
}

} // namespace

std::vector<InductionVar> findInductionVars(
    core_ir::Function* func, const analysis::NaturalLoop& loop) {
  std::vector<InductionVar> result;

  // 遍历循环头部 PHI。
  for (auto& instPtr : loop.header->instructions()) {
    core_ir::Instruction* inst = instPtr.get();
    if (inst->opcode() != core_ir::Opcode::Phi) continue;
    if (inst->type().isF32()) continue; // 当前只处理整数归纳变量
    if (inst->numOperands() < 2) continue;

    // 查找 phi(init, prev + step) 或 phi(init, prev - step) 形式，并检查每条
    // 循环内输入边。多 latch 循环要求每条回边使用相同的常量步长。
    core_ir::Value* init = nullptr;
    core_ir::Value* stepVal = nullptr;
    bool sawBackEdge = false;
    bool canonical = true;
    bool sawConstStep = false;
    int64_t canonicalStep = 0;

    for (int i = 0; i + 1 < inst->numOperands(); i += 2) {
      core_ir::BasicBlock* pred = dynamic_cast<core_ir::BasicBlock*>(inst->operand(i + 1));
      if (!pred) continue;
      if (!loop.blocks.count(pred)) {
        if (!init) {
          init = inst->operand(i);
        } else if (init != inst->operand(i)) {
          canonical = false;
          break;
        }
        continue;
      }

      sawBackEdge = true;
      core_ir::Value* backEdgeVal = inst->operand(i);

      // 检查回边值是否为包含该 PHI 的加法或减法。
      auto* backInst = dynamic_cast<core_ir::Instruction*>(backEdgeVal);
      if (!backInst) {
        canonical = false;
        break;
      }

      core_ir::Opcode op = backInst->opcode();
      if (op != core_ir::Opcode::Add && op != core_ir::Opcode::Sub) {
        canonical = false;
        break;
      }

      core_ir::Value* other = nullptr;
      int sign = 1;
      if (backInst->operand(0) == inst) {
        other = backInst->operand(1);
        if (op == core_ir::Opcode::Sub) sign = -1;
      } else if (backInst->operand(1) == inst && op == core_ir::Opcode::Add) {
        other = backInst->operand(0);
      } else {
        canonical = false;
        break;
      }

      if (!stepVal) {
        stepVal = other;
      } else if (stepVal != other) {
        stepVal = nullptr;
      }

      auto* ci = dynamic_cast<core_ir::ConstInt*>(other);
      if (!ci) {
        canonical = false;
        break;
      }

      int64_t step = static_cast<int64_t>(ci->value()) * sign;
      if (!sawConstStep) {
        canonicalStep = step;
        sawConstStep = true;
      } else if (canonicalStep != step) {
        canonical = false;
        break;
      }
    }

    InductionVar iv;
    iv.phi = inst;
    iv.storage = nullptr;
    iv.init = init;
    iv.stepVal = stepVal;
    iv.step = sawConstStep ? canonicalStep : 0;
    iv.isCanonical = init && sawBackEdge && canonical && sawConstStep;

    result.push_back(iv);
  }

  if (!func)
    return result;

  std::unordered_set<int> seenSlots;
  for (auto& bbPtr : func->blocks()) {
    for (auto& instPtr : bbPtr->instructions()) {
      auto* inst = instPtr.get();
      if (!inst || inst->opcode() != Opcode::Alloca || inst->isArrayObject() ||
          inst->memorySizeBytes() <= 0 || inst->memorySizeBytes() > 8) {
        continue;
      }
      if (!seenSlots.insert(inst->id()).second)
        continue;

      InductionVar iv;
      if (tryFindScalarSlotInduction(func, loop, inst, iv))
        result.push_back(iv);
    }
  }

  return result;
}

LoopTripRecipe analyzeLoopTrip(core_ir::Function* func,
                               const NaturalLoop& loop) {
  LoopTripRecipe recipe;
  if (!func || !loop.header) {
    recipe.proof = ProofResult::unknown("trip-count-missing-loop");
    return recipe;
  }
  std::vector<InductionVar> canonical;
  for (const auto& induction : findInductionVars(func, loop))
    if (induction.isCanonical && induction.phi)
      canonical.push_back(induction);
  if (canonical.size() != 1) {
    recipe.proof = ProofResult::unknown("trip-count-non-unique-ssa-iv");
    return recipe;
  }
  recipe.induction = canonical.front().phi;
  recipe.initial = canonical.front().init;
  recipe.step = canonical.front().step;

  std::vector<std::pair<BasicBlock*, BasicBlock*>> exits;
  for (auto* block : loop.blocks)
    for (auto* successor : block->succs())
      if (successor && !loop.blocks.count(successor))
        exits.push_back({block, successor});
  std::sort(exits.begin(), exits.end(), [](const auto& lhs, const auto& rhs) {
    if (lhs.first->name() != rhs.first->name())
      return lhs.first->name() < rhs.first->name();
    return lhs.second->name() < rhs.second->name();
  });
  exits.erase(std::unique(exits.begin(), exits.end()), exits.end());
  if (exits.size() != 1) {
    recipe.proof = ProofResult::unknown("trip-count-non-unique-exit");
    return recipe;
  }
  recipe.exitingBlock = exits.front().first;
  recipe.exitBlock = exits.front().second;
  auto* terminator = recipe.exitingBlock->terminator();
  auto* compare = terminator && terminator->opcode() == Opcode::CondBr &&
                          terminator->numOperands() == 3
                      ? dynamic_cast<Instruction*>(terminator->operand(0))
                      : nullptr;
  if (!compare || compare->opcode() != Opcode::ICmp ||
      compare->numOperands() != 2) {
    recipe.proof = ProofResult::unknown("trip-count-unsupported-condition");
    return recipe;
  }
  auto* trueBlock = dynamic_cast<BasicBlock*>(terminator->operand(1));
  auto* falseBlock = dynamic_cast<BasicBlock*>(terminator->operand(2));
  const bool trueContinues = trueBlock && loop.blocks.count(trueBlock);
  const bool falseContinues = falseBlock && loop.blocks.count(falseBlock);
  if (trueContinues == falseContinues) {
    recipe.proof = ProofResult::unknown("trip-count-ambiguous-condition");
    return recipe;
  }
  recipe.predicate = hir::canonicalLoopCmpCond(compare);
  if (!trueContinues) recipe.predicate = invertPredicate(recipe.predicate);
  if (compare->operand(0) == recipe.induction) {
    recipe.bound = compare->operand(1);
  } else if (compare->operand(1) == recipe.induction) {
    recipe.bound = compare->operand(0);
    recipe.predicate = swapPredicate(recipe.predicate);
  } else {
    recipe.proof = ProofResult::unknown("trip-count-condition-not-on-iv");
    return recipe;
  }
  auto* boundInstruction = dynamic_cast<Instruction*>(recipe.bound);
  if (boundInstruction && boundInstruction->parent() &&
      loop.blocks.count(boundInstruction->parent())) {
    recipe.proof = ProofResult::unknown("trip-count-loop-variant-bound");
    return recipe;
  }

  auto* initial = dynamic_cast<ConstInt*>(recipe.initial);
  auto* bound = dynamic_cast<ConstInt*>(recipe.bound);
  if (initial && bound) {
    if (checkedTripCount(initial->value(), bound->value(), recipe.step,
                         recipe.predicate, recipe.constantTripCount)) {
      recipe.kind = TripCountKind::ConstantMachineExact;
      recipe.proof = ProofResult::success("trip-count-machine-exact");
    } else {
      recipe.proof = ProofResult::disproved(
          "trip-count-wrap-or-nontermination");
    }
    return recipe;
  }

  const bool supportedDirection =
      (recipe.step > 0 &&
       (recipe.predicate == "lt" || recipe.predicate == "le")) ||
      (recipe.step < 0 &&
       (recipe.predicate == "gt" || recipe.predicate == "ge"));
  if (!supportedDirection) {
    recipe.proof = ProofResult::unknown("trip-count-direction-unknown");
    return recipe;
  }
  recipe.kind = TripCountKind::RuntimeBounded;
  recipe.proof = ProofResult::unknown("trip-count-runtime-guard-required");
  return recipe;
}

std::optional<int64_t> maximumTripCount(const LoopTripRecipe& recipe) {
  if (recipe.kind == TripCountKind::ConstantMachineExact &&
      recipe.constantTripCount >= 0)
    return recipe.constantTripCount;
  if (recipe.kind != TripCountKind::RuntimeBounded || !recipe.initial ||
      !recipe.bound || recipe.step == 0)
    return std::nullopt;
  const IntegerFacts initial = evaluateIntegerFacts(recipe.initial);
  const IntegerFacts bound = evaluateIntegerFacts(recipe.bound);
  if (!initial.proof.proved() || !bound.proof.proved() ||
      !initial.rangeKnown || initial.minimum != initial.maximum ||
      !bound.rangeKnown)
    return std::nullopt;
  const __int128 start = initial.minimum;
  const __int128 step = recipe.step;
  __int128 count = 0;
  if (step > 0 && (recipe.predicate == "lt" || recipe.predicate == "le")) {
    const __int128 limit = bound.maximum;
    if (recipe.predicate == "lt") {
      if (limit > start) count = (limit - start + step - 1) / step;
    } else if (limit >= start) {
      count = (limit - start) / step + 1;
    }
  } else if (step < 0 &&
             (recipe.predicate == "gt" || recipe.predicate == "ge")) {
    const __int128 limit = bound.minimum;
    const __int128 magnitude = -step;
    if (recipe.predicate == "gt") {
      if (limit < start) count = (start - limit + magnitude - 1) / magnitude;
    } else if (limit <= start) {
      count = (start - limit) / magnitude + 1;
    }
  } else {
    return std::nullopt;
  }
  if (count < 0 || count > std::numeric_limits<int64_t>::max())
    return std::nullopt;
  const __int128 finalValue = start + count * step;
  if (finalValue < std::numeric_limits<int32_t>::min() ||
      finalValue > std::numeric_limits<int32_t>::max())
    return std::nullopt;
  return static_cast<int64_t>(count);
}

int computeGEPStride(core_ir::Instruction* gep, core_ir::Instruction* indVar,
                     const NaturalLoop& loop) {
  if (!gep || gep->opcode() != core_ir::Opcode::GEP) return 0;
  if (gep->numOperands() < 2) return 0;

  // 查找 base、indVar * stride 形式的 GEP。第一个操作数之后的下标表示按
  // 元素大小缩放的偏移；二维数组 gep(base, i, j) 中，i 贡献
  // i * (dim1 * elemSize) 字节。

  int accumulated = 0;
  for (int i = 1; i < gep->numOperands(); ++i) {
    core_ir::Value* idx = gep->operand(i);
    // 检查当前下标是否为归纳变量。
    if (idx == indVar) {
      // 找到归纳变量所在维度。
      // 步长是剩余维度乘以元素大小；这里返回 i32 元素大小 4，调用者再结合
      // 内存访问大小计算步长。
      return 4;
    }
    // 检查下标是否为常量加归纳变量。
    if (auto* addInst = dynamic_cast<core_ir::Instruction*>(idx)) {
      if (addInst->opcode() == core_ir::Opcode::Add ||
          addInst->opcode() == core_ir::Opcode::Sub) {
        if (addInst->operand(0) == indVar || addInst->operand(1) == indVar)
          return 4;
      }
    }
    // 检查下标是否为归纳变量乘常量。
    if (auto* mulInst = dynamic_cast<core_ir::Instruction*>(idx)) {
      if (mulInst->opcode() == core_ir::Opcode::Mul) {
        if (mulInst->operand(0) == indVar || mulInst->operand(1) == indVar)
          return 4;
      }
    }
  }
  return accumulated;
}

} // namespace analysis

#define ANALYSIS_AFFINE_ACCESS_EMBED
#include "affine_access.cpp"
