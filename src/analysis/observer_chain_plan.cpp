// 文件职责：实现确定性的 observer closure；store/call/ret 是观察边界，未知调用拒绝。
#include "observer_chain_plan.h"

#include "termination.h"

#include <algorithm>

namespace analysis {

ObserverChainPlan buildObserverChainPlan(
    const std::vector<core_ir::Value*>& roots,
    const std::unordered_set<core_ir::BasicBlock*>& region,
    const FunctionEffectMap* effects) {
  ObserverChainPlan result;
  if (roots.empty()) {
    result.proof = ProofResult::disproved("observer-roots-empty");
    return result;
  }
  std::vector<core_ir::Value*> work = roots;
  std::unordered_set<core_ir::Value*> seen;
  for (std::size_t index = 0; index < work.size(); ++index) {
    auto* value = work[index];
    if (!value || !seen.insert(value).second) continue;
    for (auto* user : value->uses()) {
      if (!user || !user->parent()) {
        result.proof = ProofResult::unknown("observer-user-not-live");
        return result;
      }
      if (region.count(user->parent())) continue;
      result.valueSlice.insert(user);
      switch (user->opcode()) {
      case core_ir::Opcode::Ret:
        result.escapesFunction = true;
        result.observers.push_back(user);
        break;
      case core_ir::Opcode::Store:
      case core_ir::Opcode::VStore:
        result.reachesMemoryWrite = true;
        result.observers.push_back(user);
        break;
      case core_ir::Opcode::Call: {
        result.observers.push_back(user);
        const auto* function = user->parent()->parent();
        const FunctionEffectSummary* callee = nullptr;
        if (effects && function && function->parent())
          callee = lookupFunctionEffect(*effects, *function->parent(),
                                        user->name());
        if (!callee || !callee->readonly)
          result.reachesUnknownCall = true;
        break;
      }
      case core_ir::Opcode::Br:
      case core_ir::Opcode::CondBr:
        result.observers.push_back(user);
        break;
      default:
        work.push_back(user);
        break;
      }
    }
  }
  std::sort(result.observers.begin(), result.observers.end(),
            [](const auto* lhs, const auto* rhs) {
              return lhs->id() < rhs->id();
            });
  result.observers.erase(
      std::unique(result.observers.begin(), result.observers.end()),
      result.observers.end());
  if (result.reachesUnknownCall)
    result.proof = ProofResult::unknown("observer-reaches-unknown-call");
  else
    result.proof = ProofResult::success("observer-closure-complete");
  return result;
}

RegionDemandSlice buildRegionDemandSlice(
    const std::vector<core_ir::Instruction*>& observers,
    const std::unordered_set<core_ir::BasicBlock*>& region,
    const OrderedRegionEffectSummary* effects) {
  RegionDemandSlice result;
  if (observers.empty() || region.empty()) {
    result.proof = ProofResult::disproved("demand-slice-empty-boundary");
    return result;
  }
  std::vector<core_ir::Value*> work;
  std::unordered_set<core_ir::Value*> seen;
  for (auto* observer : observers) {
    if (!observer || !observer->parent()) {
      result.proof = ProofResult::unknown("demand-observer-not-live");
      return result;
    }
    for (auto* operand : observer->operands())
      if (operand)
        work.push_back(operand);
  }
  for (std::size_t index = 0; index < work.size(); ++index) {
    core_ir::Value* value = work[index];
    if (!value || !seen.insert(value).second)
      continue;
    auto* instruction = dynamic_cast<core_ir::Instruction*>(value);
    if (!instruction || !instruction->parent() ||
        !region.count(instruction->parent())) {
      result.liveIns.insert(value);
      continue;
    }
    result.instructions.insert(instruction);
    for (auto* operand : instruction->operands())
      if (operand)
        work.push_back(operand);
  }
  if (effects) {
    result.hasUnknownMemoryDemand = effects->hasUnknownEffect;
    for (const auto& event : effects->events) {
      if (event.instruction)
        result.effectInstructions.insert(event.instruction);
      if (event.object)
        result.memoryObjects.insert(event.object);
    }
    if (result.hasUnknownMemoryDemand) {
      result.proof = ProofResult::unknown("region-memory-demand-unknown");
      return result;
    }
  }
  result.proof = ProofResult::success("region-value-demand-complete");
  return result;
}

RegionValueDeletionProof proveRegionValueDeletion(
    const RegionDemandSlice& demand,
    const std::unordered_set<core_ir::BasicBlock*>& region) {
  RegionValueDeletionProof result;
  if (!demand.proof.proved() || region.empty()) {
    result.proof = ProofResult::unknown("region-demand-proof-missing");
    return result;
  }

  std::unordered_set<core_ir::Instruction*> candidates;
  for (auto* block : region) {
    if (!block) {
      result.proof = ProofResult::unknown("region-block-not-live");
      return result;
    }
    for (const auto& owned : block->instructions()) {
      auto* instruction = owned.get();
      if (demand.instructions.count(instruction) ||
          instruction->hasSideEffects() || instruction->isTerminator())
        continue;
      switch (instruction->opcode()) {
      case core_ir::Opcode::Alloca:
      case core_ir::Opcode::Load:
      case core_ir::Opcode::VLoad:
      case core_ir::Opcode::VAdd:
      case core_ir::Opcode::VSub:
      case core_ir::Opcode::VMul:
      case core_ir::Opcode::VDiv:
      case core_ir::Opcode::VFMA:
      case core_ir::Opcode::VSplat:
      case core_ir::Opcode::VExtract:
        continue;
      default:
        break;
      }
      if (isProvablyNonTrappingScalarInstruction(instruction))
        candidates.insert(instruction);
    }
  }

  // 求最大的 use-closed 子集。若一个候选仍被 demanded/out-of-region 指令使用，
  // 它及其生产者都不能作为本次删除的一部分。
  bool changed = true;
  while (changed) {
    changed = false;
    std::vector<core_ir::Instruction*> rejected;
    for (auto* instruction : candidates) {
      for (auto* user : instruction->uses()) {
        if (!user || !candidates.count(user)) {
          rejected.push_back(instruction);
          break;
        }
      }
    }
    for (auto* instruction : rejected)
      changed |= candidates.erase(instruction) != 0;
  }

  result.instructions.assign(candidates.begin(), candidates.end());
  std::sort(result.instructions.begin(), result.instructions.end(),
            [](const auto* lhs, const auto* rhs) {
              return lhs->id() < rhs->id();
            });
  result.proof = result.instructions.empty()
                     ? ProofResult::disproved("region-no-dead-value-subgraph")
                     : ProofResult::success("region-dead-value-subgraph-proved");
  return result;
}

} // namespace analysis
