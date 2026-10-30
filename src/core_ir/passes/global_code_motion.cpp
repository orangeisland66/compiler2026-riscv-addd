// 文件职责：在满足数据依赖的前提下移动 CoreIR 指令到更合适的位置。
#include "global_code_motion.h"

#include "pass_registry.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../../analysis/analysis_manager.h"
#include "../../analysis/dominators.h"

namespace core_ir {

namespace {

bool isSinkCandidate(const Instruction* inst) {
  if (!inst || !inst->parent() || inst->type().isVoid() ||
      inst->hasSideEffects() || inst->uses().empty()) {
    return false;
  }

  // 除法和取余暂不参与下沉：虽然下沉不会增加执行次数，
  // 但改变它们与可观察副作用的相对顺序，可能改变异常出现的位置。
  switch (inst->opcode()) {
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Mul:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
  case Opcode::Shl:
  case Opcode::Shr:
  case Opcode::FAdd:
  case Opcode::FSub:
  case Opcode::FMul:
  case Opcode::ICmp:
  case Opcode::FCmp:
  case Opcode::SIToFP:
  case Opcode::FPToSI:
  case Opcode::GEP:
  case Opcode::Neg:
  case Opcode::FNeg:
  case Opcode::BitCast:
  case Opcode::Select:
  case Opcode::VAdd:
  case Opcode::VSub:
  case Opcode::VMul:
  case Opcode::VFMA:
  case Opcode::VSplat:
  case Opcode::VExtract:
    return true;
  default:
    return false;
  }
}

bool moveInstructionBefore(Instruction* inst, BasicBlock* dest,
                           Instruction* position) {
  if (!inst || !inst->parent() || !dest || !position)
    return false;
  BasicBlock* source = inst->parent();
  if (source == dest)
    return false;

  auto& sourceInstructions = source->instructions();
  auto sourceIt = std::find_if(
      sourceInstructions.begin(), sourceInstructions.end(),
      [inst](const auto& owned) { return owned.get() == inst; });
  if (sourceIt == sourceInstructions.end())
    return false;

  auto& destinationInstructions = dest->instructions();
  auto destinationIt = std::find_if(
      destinationInstructions.begin(), destinationInstructions.end(),
      [position](const auto& owned) { return owned.get() == position; });
  if (destinationIt == destinationInstructions.end())
    return false;

  std::unique_ptr<Instruction> owned = std::move(*sourceIt);
  sourceInstructions.erase(sourceIt);
  owned->setParent(dest);
  destinationInstructions.insert(destinationIt, std::move(owned));
  return true;
}

class GlobalCodeMotionImpl {
public:
  GlobalCodeMotionImpl(Function* function,
                       analysis::FunctionAnalysisCache& analysis)
      : function_(function), analysis_(analysis), doms_(analysis.dominators()),
        reachable_(analysis.reachable()) {
    size_t index = 0;
    for (const auto& block : function_->blocks())
      blockOrder_[block.get()] = index++;
    for (const auto& loop : analysis_.loops()) {
      for (BasicBlock* block : loop.blocks)
        ++loopDepth_[block];
    }
  }

  bool run() {
    if (!function_ || !function_->entryBlock() ||
        analysis_.instructionCount() > 12000) {
      return false;
    }

    std::vector<Instruction*> candidates;
    for (const auto& block : function_->blocks()) {
      if (!block || !reachable_.count(block.get()))
        continue;
      for (const auto& instruction : block->instructions()) {
        if (isSinkCandidate(instruction.get())) {
          candidates.push_back(instruction.get());
          candidateSet_.insert(instruction.get());
        }
      }
    }

    for (Instruction* candidate : candidates)
      scheduleUsersBeforeDefinition(candidate);
    return changed_;
  }

private:
  enum class VisitState { Visiting, Done };

  Function* function_;
  analysis::FunctionAnalysisCache& analysis_;
  const analysis::DomSet& doms_;
  const std::unordered_set<BasicBlock*>& reachable_;
  std::unordered_map<BasicBlock*, int> loopDepth_;
  std::unordered_map<BasicBlock*, size_t> blockOrder_;
  std::unordered_set<Instruction*> candidateSet_;
  std::unordered_map<Instruction*, VisitState> visitState_;
  bool changed_ = false;

  int loopDepth(BasicBlock* block) const {
    auto it = loopDepth_.find(block);
    return it == loopDepth_.end() ? 0 : it->second;
  }

  size_t dominanceDepth(BasicBlock* block) const {
    auto it = doms_.find(block);
    return it == doms_.end() ? 0 : it->second.size();
  }

  void scheduleUsersBeforeDefinition(Instruction* instruction) {
    auto stateIt = visitState_.find(instruction);
    if (stateIt != visitState_.end())
      return;
    visitState_[instruction] = VisitState::Visiting;

    const std::vector<Instruction*> users = instruction->uses();
    for (Instruction* user : users) {
      if (candidateSet_.count(user))
        scheduleUsersBeforeDefinition(user);
    }

    changed_ |= trySink(instruction);
    visitState_[instruction] = VisitState::Done;
  }

  bool allUsesReachable(Instruction* instruction) const {
    for (Instruction* user : instruction->uses()) {
      if (!user || !user->parent() || !reachable_.count(user->parent()))
        return false;
      if (user->opcode() != Opcode::Phi)
        continue;

      bool foundIncomingValue = false;
      for (int operandIndex = 0; operandIndex + 1 < user->numOperands();
           operandIndex += 2) {
        if (user->operand(operandIndex) != instruction)
          continue;
        foundIncomingValue = true;
        auto* predecessor =
            dynamic_cast<BasicBlock*>(user->operand(operandIndex + 1));
        if (!predecessor || !reachable_.count(predecessor))
          return false;
      }
      if (!foundIncomingValue)
        return false;
    }
    return true;
  }

  std::vector<BasicBlock*> useBlocks(Instruction* instruction) const {
    std::vector<BasicBlock*> result;
    std::unordered_set<BasicBlock*> seen;
    for (Instruction* user : instruction->uses()) {
      if (!user || !user->parent())
        continue;
      if (user->opcode() != Opcode::Phi) {
        if (reachable_.count(user->parent()) && seen.insert(user->parent()).second)
          result.push_back(user->parent());
        continue;
      }

      // PHI 在对应的前驱边上消费输入值，而不是在包含 PHI 的块中消费。
      for (int operandIndex = 0; operandIndex + 1 < user->numOperands();
           operandIndex += 2) {
        if (user->operand(operandIndex) != instruction)
          continue;
        auto* predecessor =
            dynamic_cast<BasicBlock*>(user->operand(operandIndex + 1));
        if (predecessor && reachable_.count(predecessor) &&
            seen.insert(predecessor).second) {
          result.push_back(predecessor);
        }
      }
    }
    return result;
  }

  BasicBlock* chooseTarget(Instruction* instruction,
                           const std::vector<BasicBlock*>& uses) const {
    if (uses.empty())
      return nullptr;
    BasicBlock* source = instruction->parent();
    auto firstIt = doms_.find(uses.front());
    if (!source || firstIt == doms_.end())
      return nullptr;

    BasicBlock* best = nullptr;
    int bestLoopDepth = std::numeric_limits<int>::max();
    size_t bestDominanceDepth = 0;
    size_t bestBlockOrder = std::numeric_limits<size_t>::max();

    for (BasicBlock* candidate : firstIt->second) {
      if (!candidate || !reachable_.count(candidate) ||
          !analysis::dominates(doms_, source, candidate)) {
        continue;
      }
      bool dominatesEveryUse = true;
      for (BasicBlock* useBlock : uses) {
        if (!analysis::dominates(doms_, candidate, useBlock)) {
          dominatesEveryUse = false;
          break;
        }
      }
      if (!dominatesEveryUse)
        continue;

      const int candidateLoopDepth = loopDepth(candidate);
      const size_t candidateDominanceDepth = dominanceDepth(candidate);
      auto orderIt = blockOrder_.find(candidate);
      const size_t candidateBlockOrder =
          orderIt == blockOrder_.end() ? std::numeric_limits<size_t>::max()
                                       : orderIt->second;
      if (!best || candidateLoopDepth < bestLoopDepth ||
          (candidateLoopDepth == bestLoopDepth &&
           candidateDominanceDepth > bestDominanceDepth) ||
          (candidateLoopDepth == bestLoopDepth &&
           candidateDominanceDepth == bestDominanceDepth &&
           candidateBlockOrder < bestBlockOrder)) {
        best = candidate;
        bestLoopDepth = candidateLoopDepth;
        bestDominanceDepth = candidateDominanceDepth;
        bestBlockOrder = candidateBlockOrder;
      }
    }
    return best;
  }

  Instruction* insertionPoint(Instruction* instruction,
                              BasicBlock* target) const {
    std::unordered_set<Instruction*> ordinaryUsers;
    for (Instruction* user : instruction->uses()) {
      if (user && user->parent() == target && user->opcode() != Opcode::Phi)
        ordinaryUsers.insert(user);
    }
    for (const auto& candidate : target->instructions()) {
      if (ordinaryUsers.count(candidate.get()))
        return candidate.get();
    }
    return target->terminator();
  }

  bool trySink(Instruction* instruction) {
    if (!isSinkCandidate(instruction) ||
        !reachable_.count(instruction->parent()) ||
        !allUsesReachable(instruction)) {
      return false;
    }
    const std::vector<BasicBlock*> uses = useBlocks(instruction);
    BasicBlock* target = chooseTarget(instruction, uses);
    if (!target || target == instruction->parent())
      return false;

    // 不增加循环嵌套深度。chooseTarget 通常通过最小化深度实现这一点，
    // 这里的显式判断用于保持变换范围清晰。
    if (loopDepth(target) > loopDepth(instruction->parent()))
      return false;
    Instruction* position = insertionPoint(instruction, target);
    return moveInstructionBefore(instruction, target, position);
  }
};

} // namespace

bool globalCodeMotion(Module& mod, analysis::AnalysisManager& am) {
  bool changed = false;
  for (const auto& function : mod.functions()) {
    if (!function || !function->entryBlock())
      continue;
    changed |=
        GlobalCodeMotionImpl(function.get(), am.function(function.get())).run();
  }
  return changed;
}

class GlobalCodeMotionPass : public Pass {
public:
  std::string name() const override { return "GlobalCodeMotion"; }
  bool run(Module& mod, analysis::AnalysisManager& am) override {
    return globalCodeMotion(mod, am);
  }
};

REGISTER_PASS(GlobalCodeMotionPass);

} // namespace core_ir
