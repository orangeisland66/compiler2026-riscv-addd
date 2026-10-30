// 文件职责：实现不依赖符号拼写的函数语义摘要；未知调用和 pointer escape 保守拒绝。
#include "function_semantic_summary.h"

#include <unordered_set>

namespace analysis {

namespace {

bool isPointerArgument(const core_ir::Function* function,
                       const core_ir::Value* value) {
  if (!function || !value || !value->type().isPtr()) return false;
  for (const auto& argument : function->args())
    if (argument.val == value) return true;
  return false;
}

bool derivedFromPointerArgument(const core_ir::Function* function,
                                const core_ir::Value* value,
                                std::unordered_set<const core_ir::Value*>& seen) {
  if (!value || !seen.insert(value).second) return false;
  if (isPointerArgument(function, value)) return true;
  const auto* instruction = dynamic_cast<const core_ir::Instruction*>(value);
  if (!instruction) return false;
  if (instruction->opcode() != core_ir::Opcode::GEP &&
      instruction->opcode() != core_ir::Opcode::BitCast &&
      instruction->opcode() != core_ir::Opcode::Phi &&
      instruction->opcode() != core_ir::Opcode::Select)
    return false;
  for (auto* operand : instruction->operands())
    if (operand && operand->type().isPtr() &&
        derivedFromPointerArgument(function, operand, seen))
      return true;
  return false;
}

bool derivedFromPointerArgument(const core_ir::Function* function,
                                const core_ir::Value* value) {
  std::unordered_set<const core_ir::Value*> seen;
  return derivedFromPointerArgument(function, value, seen);
}

} // namespace

FunctionSemanticSummary summarizeFunctionSemantics(
    core_ir::Function* function, const FunctionEffectMap& effects,
    const FunctionTerminationMap& termination,
    const FunctionSemanticSummaryMap* known) {
  FunctionSemanticSummary result;
  if (!function || !function->entryBlock()) {
    result.proof = ProofResult::disproved("semantic-summary-missing-entry");
    return result;
  }
  const auto* effect = lookupFunctionEffect(effects, function);
  if (!effect) {
    result.proof = ProofResult::unknown("function-effect-missing");
    return result;
  }
  result.pure = effect->pure;
  result.readonly = effect->readonly;
  result.readArgumentIndices = effect->readArgumentIndices;
  result.writtenArgumentIndices = effect->writtenArgumentIndices;
  result.capturedArgumentIndices = effect->capturedArgumentIndices;
  result.returnedArgumentIndices = effect->returnedArgumentIndices;
  result.writtenGlobals = effect->writtenGlobals;
  result.pointerArgumentEscapes = !effect->capturedArgumentIndices.empty();
  auto total = termination.find(function);
  result.total = total != termination.end() && total->second;
  result.returnsPointer = function->retTy().isPtr();
  result.nonTrapping = true;
  for (const auto& block : function->blocks()) {
    for (const auto& owned : block->instructions()) {
      const auto* instruction = owned.get();
      switch (instruction->opcode()) {
      case core_ir::Opcode::Load:
      case core_ir::Opcode::VLoad:
        ++result.memoryReads;
        break;
      case core_ir::Opcode::Store:
      case core_ir::Opcode::VStore:
        ++result.memoryWrites;
        if (instruction->numOperands() >= 1 &&
            derivedFromPointerArgument(function, instruction->operand(0)))
          result.pointerArgumentEscapes = true;
        break;
      case core_ir::Opcode::Call: {
        ++result.calls;
        auto* calleeFunction = function->parent()
                                   ? function->parent()->getFunction(
                                         instruction->name())
                                   : nullptr;
        const auto* callee = calleeFunction
                                 ? lookupFunctionEffect(effects, calleeFunction)
                                 : nullptr;
        if (!callee || !callee->pure) result.hasObservableCall = true;
        const FunctionSemanticSummary* calleeSemantic = nullptr;
        if (known && calleeFunction) {
          auto found = known->find(calleeFunction);
          if (found != known->end()) calleeSemantic = &found->second;
        }
        if (!calleeSemantic || !calleeSemantic->nonTrapping)
          result.nonTrapping = false;
        for (auto* argument : instruction->operands())
          if (derivedFromPointerArgument(function, argument) &&
              (!callee || !callee->readonly))
            result.pointerArgumentEscapes = true;
        break;
      }
      case core_ir::Opcode::Ret:
        if (instruction->numOperands() == 1 &&
            derivedFromPointerArgument(function, instruction->operand(0)))
          result.pointerArgumentEscapes = true;
        break;
      default:
        break;
      }
      if (!isProvablyNonTrappingScalarInstruction(instruction) &&
          (instruction->opcode() == core_ir::Opcode::SDiv ||
           instruction->opcode() == core_ir::Opcode::SRem ||
           instruction->opcode() == core_ir::Opcode::Shl ||
           instruction->opcode() == core_ir::Opcode::Shr))
        result.nonTrapping = false;
    }
  }
  result.deterministic = !effect->readsUnknownMemory &&
                         !effect->writesUnknownMemory &&
                         !result.hasObservableCall;
  if (!result.total)
    result.proof = ProofResult::unknown("function-termination-unproved");
  else if (result.pointerArgumentEscapes)
    result.proof = ProofResult::unknown("pointer-argument-escapes");
  else if (!result.nonTrapping)
    result.proof = ProofResult::unknown("function-may-trap");
  else
    result.proof = ProofResult::success("function-semantics-summarized");
  return result;
}

FunctionSemanticSummaryMap summarizeModuleFunctionSemantics(
    core_ir::Module& module, const FunctionEffectMap& effects,
    const FunctionTerminationMap& termination) {
  FunctionSemanticSummaryMap summaries;
  for (const auto& function : module.functions()) {
    FunctionSemanticSummary seed;
    seed.nonTrapping = true;
    summaries.emplace(function.get(), std::move(seed));
  }
  bool changed = true;
  while (changed) {
    changed = false;
    for (const auto& function : module.functions()) {
      auto summary = summarizeFunctionSemantics(
          function.get(), effects, termination, &summaries);
      auto& previous = summaries[function.get()];
      if (previous.proof.state != summary.proof.state ||
          previous.proof.reasonCode != summary.proof.reasonCode ||
          previous.pure != summary.pure ||
          previous.readonly != summary.readonly ||
          previous.total != summary.total ||
          previous.deterministic != summary.deterministic ||
          previous.nonTrapping != summary.nonTrapping ||
          previous.hasObservableCall != summary.hasObservableCall ||
          previous.pointerArgumentEscapes != summary.pointerArgumentEscapes ||
          previous.returnsPointer != summary.returnsPointer ||
          previous.readArgumentIndices != summary.readArgumentIndices ||
          previous.writtenArgumentIndices != summary.writtenArgumentIndices ||
          previous.capturedArgumentIndices != summary.capturedArgumentIndices ||
          previous.returnedArgumentIndices != summary.returnedArgumentIndices ||
          previous.writtenGlobals != summary.writtenGlobals ||
          previous.memoryReads != summary.memoryReads ||
          previous.memoryWrites != summary.memoryWrites ||
          previous.calls != summary.calls) {
        previous = std::move(summary);
        changed = true;
      }
    }
  }
  return summaries;
}

} // namespace analysis
