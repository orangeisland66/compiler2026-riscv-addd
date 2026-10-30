// 文件职责：实现 loop access inventory，仅产出事实不修改 IR。
#include "loop_access.h"

#include "pointer_range.h"

namespace analysis {

std::vector<LoopAccess> collectLoopAccesses(
    const NaturalLoop& loop, const std::vector<InductionVar>& ivs) {
  std::vector<LoopAccess> result;
  for (auto* block : loop.blocks) {
    if (!block) continue;
    for (const auto& owned : block->instructions()) {
      auto* instruction = owned.get();
      const auto opcode = instruction->opcode();
      if (opcode != core_ir::Opcode::Load &&
          opcode != core_ir::Opcode::Store &&
          opcode != core_ir::Opcode::VLoad &&
          opcode != core_ir::Opcode::VStore)
        continue;
      LoopAccess access;
      access.instruction = instruction;
      access.reads = opcode == core_ir::Opcode::Load ||
                     opcode == core_ir::Opcode::VLoad;
      access.writes = !access.reads;
      const int addressIndex = access.reads ? 0 : 1;
      if (instruction->numOperands() <= addressIndex) {
        access.proof = ProofResult::disproved("malformed-memory-instruction");
        result.push_back(std::move(access));
        continue;
      }
      access.address = instruction->operand(addressIndex);
      access.object = underlyingPointerObject(access.address);
      access.accessBytes = instruction->type().isVector() ? 16 : 4;
      access.affine = summarizeAffineAccess(access.address, loop, ivs);
      if (!access.object)
        access.proof = ProofResult::unknown("access-object-unknown");
      else if (!access.affine.valid)
        access.proof = ProofResult::unknown("access-not-affine");
      else
        access.proof = ProofResult::success("affine-loop-access");
      result.push_back(std::move(access));
    }
  }
  return result;
}

} // namespace analysis
