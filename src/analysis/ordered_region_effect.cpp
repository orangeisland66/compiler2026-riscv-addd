// 文件职责：实现 conservative ordered-effect inventory，未知地址/调用/出口均显式标记。
#include "ordered_region_effect.h"

namespace analysis {

OrderedRegionEffectSummary analyzeOrderedRegionEffects(
    core_ir::Function* function,
    const std::unordered_set<core_ir::BasicBlock*>& region,
    const FunctionEffectMap* effects) {
  OrderedRegionEffectSummary result;
  if (!function || region.empty()) {
    result.proof = ProofResult::disproved("empty-ordered-region");
    return result;
  }
  for (const auto& ownedBlock : function->blocks()) {
    auto* block = ownedBlock.get();
    if (!region.count(block)) continue;
    for (const auto& ownedInstruction : block->instructions()) {
      auto* instruction = ownedInstruction.get();
      if (instruction->opcode() == core_ir::Opcode::Load ||
          instruction->opcode() == core_ir::Opcode::VLoad ||
          instruction->opcode() == core_ir::Opcode::Store ||
          instruction->opcode() == core_ir::Opcode::VStore) {
        const bool write = instruction->opcode() == core_ir::Opcode::Store ||
                           instruction->opcode() == core_ir::Opcode::VStore;
        const int addressIndex = write ? 1 : 0;
        if (instruction->numOperands() <= addressIndex) {
          result.hasUnknownEffect = true;
          continue;
        }
        auto range = evaluatePointerRange(instruction->operand(addressIndex),
                                          instruction->opcode() ==
                                                  core_ir::Opcode::VLoad ||
                                              instruction->opcode() ==
                                                  core_ir::Opcode::VStore
                                              ? 16
                                              : 4);
        auto* object = range.object;
        result.events.push_back({write ? OrderedEffectKind::Write
                                       : OrderedEffectKind::Read,
                                 instruction, object, range});
        if (write)
          result.writtenObjects.insert(object);
        else
          result.readObjects.insert(object);
        if (!range.proof.proved()) result.hasUnknownEffect = true;
      } else if (instruction->opcode() == core_ir::Opcode::Call) {
        const FunctionEffectSummary* callee = nullptr;
        if (effects && function->parent())
          callee = lookupFunctionEffect(*effects, *function->parent(),
                                        instruction->name());
        if (!callee || !callee->pure) {
          result.events.push_back({OrderedEffectKind::ReadWriteCall,
                                   instruction, nullptr, {}});
          result.hasObservableCall = true;
          if (!callee || !callee->readonly) result.hasUnknownEffect = true;
        }
      }
    }
    for (auto* successor : block->succs()) {
      if (!region.count(successor)) {
        result.hasSideExit = true;
        result.events.push_back({OrderedEffectKind::ControlExit,
                                 block->terminator(), nullptr, {}});
      }
    }
  }
  if (result.hasUnknownEffect)
    result.proof = ProofResult::unknown("ordered-region-unknown-effect");
  else
    result.proof = ProofResult::success("ordered-region-effects-complete");
  return result;
}

} // namespace analysis
