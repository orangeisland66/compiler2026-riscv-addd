// 文件职责：消除跨控制流路径重复出现的计算。
#include "partial_redundancy_elimination.h"

#include "pass_registry.h"

#include <string>
#include <vector>

#include "../../analysis/analysis_manager.h"
#include "../../analysis/dominators.h"

namespace core_ir {

namespace {

bool isPRECandidate(const Instruction* inst) {
  if (!inst || !inst->parent() || inst->type().isVoid() ||
      inst->hasSideEffects() || inst->uses().empty()) {
    return false;
  }
  // PRE 会在每条入边上引入一次 phi 复制，因此只处理明显比复制更昂贵的操作。
  // 对加法、移位或 GEP 使用该变换可能增加代码体积，却不能降低延迟。
  switch (inst->opcode()) {
  case Opcode::Mul:
  case Opcode::FMul:
  case Opcode::SIToFP:
  case Opcode::FPToSI:
  case Opcode::VMul:
  case Opcode::VFMA:
    return true;
  default:
    return false;
  }
}

bool sameExpression(const Instruction* lhs, const Instruction* rhs) {
  if (!isPRECandidate(lhs) || !isPRECandidate(rhs) ||
      lhs->opcode() != rhs->opcode() || !(lhs->type() == rhs->type()) ||
      lhs->numOperands() != rhs->numOperands()) {
    return false;
  }
  for (int index = 0; index < lhs->numOperands(); ++index) {
    if (lhs->operand(index) != rhs->operand(index))
      return false;
  }
  return true;
}

bool appearsBeforeTerminator(const Instruction* inst, BasicBlock* block) {
  if (!inst || inst->parent() != block)
    return false;
  for (const auto& current : block->instructions()) {
    if (current.get() == inst)
      return true;
    if (current->isTerminator())
      return false;
  }
  return false;
}

bool valueAvailableOnEdge(Value* value, BasicBlock* predecessor,
                          const analysis::DomSet& doms) {
  if (!value || !predecessor)
    return false;
  if (value->isConstant() || dynamic_cast<Argument*>(value) ||
      dynamic_cast<GlobalVar*>(value)) {
    return true;
  }
  auto* instruction = dynamic_cast<Instruction*>(value);
  if (!instruction || !instruction->parent())
    return false;
  if (instruction->parent() == predecessor)
    return appearsBeforeTerminator(instruction, predecessor);
  return analysis::dominates(doms, instruction->parent(), predecessor);
}

Instruction* cloneForEdge(const Instruction* source) {
  auto* clone = new Instruction(source->opcode(), source->type(), source->name());
  for (Value* operand : source->operands())
    clone->addOperand(operand);
  return clone;
}

void insertPhiAtBlockStart(BasicBlock* block, Instruction* phi) {
  for (const auto& instruction : block->instructions()) {
    if (instruction->opcode() != Opcode::Phi) {
      block->insertBefore(instruction.get(), phi);
      return;
    }
  }
  block->pushFront(phi);
}

class PREImpl {
public:
  PREImpl(Function* function, analysis::FunctionAnalysisCache& analysis)
      : function_(function), analysis_(analysis) {}

  bool run() {
    if (!function_ || !function_->entryBlock() ||
        analysis_.instructionCount() > 12000) {
      return false;
    }

    bool changed = false;
    // 每次改写后重新建立候选列表。新插入的 PHI 可能让同一链上的下一个
    // 表达式也变成部分冗余。
    for (int iteration = 0; iteration < 64; ++iteration) {
      if (!rewriteOne())
        break;
      changed = true;
    }
    return changed;
  }

private:
  Function* function_;
  analysis::FunctionAnalysisCache& analysis_;

  Instruction* findAvailableExpression(
      Instruction* expression, BasicBlock* predecessor,
      const std::vector<Instruction*>& candidates,
      const analysis::DomSet& doms) const {
    Instruction* best = nullptr;
    size_t bestDepth = 0;
    for (Instruction* candidate : candidates) {
      if (candidate == expression || !sameExpression(candidate, expression) ||
          !valueAvailableOnEdge(candidate, predecessor, doms)) {
        continue;
      }
      auto domIt = doms.find(candidate->parent());
      const size_t depth = domIt == doms.end() ? 0 : domIt->second.size();
      if (!best || candidate->parent() == predecessor || depth > bestDepth) {
        best = candidate;
        bestDepth = depth;
        if (candidate->parent() == predecessor)
          break;
      }
    }
    return best;
  }

  bool rewriteOne() {
    const analysis::DomSet& doms = analysis_.dominators();
    const auto& reachable = analysis_.reachable();

    std::vector<Instruction*> candidates;
    for (const auto& block : function_->blocks()) {
      if (!block || !reachable.count(block.get()))
        continue;
      for (const auto& instruction : block->instructions()) {
        if (isPRECandidate(instruction.get()))
          candidates.push_back(instruction.get());
      }
    }

    for (const auto& joinOwned : function_->blocks()) {
      BasicBlock* join = joinOwned.get();
      if (!join || !reachable.count(join) || join->preds().size() != 2)
        continue;

      const std::vector<BasicBlock*> predecessors = join->preds();
      if (!reachable.count(predecessors[0]) ||
          !reachable.count(predecessors[1])) {
        continue;
      }
      const std::vector<Instruction*> joinInstructions = [&]() {
        std::vector<Instruction*> result;
        for (const auto& instruction : join->instructions())
          result.push_back(instruction.get());
        return result;
      }();

      for (Instruction* expression : joinInstructions) {
        if (!isPRECandidate(expression))
          continue;

        std::vector<Instruction*> incoming(2, nullptr);
        int availableCount = 0;
        int missingIndex = -1;
        for (int index = 0; index < 2; ++index) {
          incoming[index] = findAvailableExpression(
              expression, predecessors[index], candidates, doms);
          if (incoming[index])
            ++availableCount;
          else
            missingIndex = index;
        }
        if (availableCount == 0)
          continue;
        if (availableCount == 2 && incoming[0] == incoming[1]) {
          expression->replaceAllUsesWith(incoming[0]);
          join->remove(expression);
          return true;
        }
        if (availableCount == 1) {
          BasicBlock* missingPredecessor = predecessors[missingIndex];
          if (missingPredecessor->succs().size() != 1 ||
              missingPredecessor->succs().front() != join ||
              !missingPredecessor->terminator()) {
            continue;
          }
          bool operandsAvailable = true;
          for (Value* operand : expression->operands()) {
            if (!valueAvailableOnEdge(operand, missingPredecessor, doms)) {
              operandsAvailable = false;
              break;
            }
          }
          if (!operandsAvailable)
            continue;

          incoming[missingIndex] = cloneForEdge(expression);
          missingPredecessor->insertBefore(missingPredecessor->terminator(),
                                           incoming[missingIndex]);
        }

        auto* phi = new Instruction(Opcode::Phi, expression->type(),
                                    expression->name() + ".pre");
        for (int index = 0; index < 2; ++index) {
          phi->addOperand(incoming[index]);
          phi->addOperand(predecessors[index]);
        }
        insertPhiAtBlockStart(join, phi);
        expression->replaceAllUsesWith(phi);
        join->remove(expression);
        return true;
      }
    }
    return false;
  }
};

} // namespace

bool partialRedundancyElimination(Module& mod,
                                  analysis::AnalysisManager& am) {
  bool changed = false;
  for (const auto& function : mod.functions()) {
    if (!function || !function->entryBlock())
      continue;
    changed |= PREImpl(function.get(), am.function(function.get())).run();
  }
  return changed;
}

class PartialRedundancyEliminationPass : public Pass {
public:
  std::string name() const override { return "PartialRedundancyElimination"; }
  bool run(Module& mod, analysis::AnalysisManager& am) override {
    return partialRedundancyElimination(mod, am);
  }
};

REGISTER_PASS(PartialRedundancyEliminationPass);

} // namespace core_ir
