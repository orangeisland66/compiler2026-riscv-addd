// 文件职责：以完整 use closure 和常量对象内范围证明死局部对象写不可观察。
#include "dead_local_materialization.h"

#include "pointer_range.h"

#include <algorithm>
#include <unordered_set>

namespace analysis {

namespace {

int accessBytes(const core_ir::Instruction* instruction) {
  if (!instruction)
    return 0;
  if (instruction->opcode() == core_ir::Opcode::VStore ||
      instruction->opcode() == core_ir::Opcode::VLoad)
    return 16;
  if (instruction->opcode() == core_ir::Opcode::Store &&
      instruction->numOperands() >= 2)
    return instruction->operand(0)->type().isPtr() ? 8 : 4;
  if (instruction->opcode() == core_ir::Opcode::Load &&
      instruction->numOperands() >= 1)
    return instruction->type().isPtr() ? 8 : 4;
  return 0;
}

bool isDerivedAddressUse(const core_ir::Instruction* user,
                         const core_ir::Value* address) {
  if (!user || !address || user->numOperands() < 1 ||
      user->operand(0) != address)
    return false;
  if (user->opcode() == core_ir::Opcode::GEP)
    return true;
  return user->opcode() == core_ir::Opcode::BitCast &&
         user->numOperands() == 1;
}

struct LocalMemoryAccess {
  core_ir::Instruction* instruction = nullptr;
  int64_t beginByte = 0;
  int64_t endByte = 0;
};

struct LocalObjectInventory {
  ProofResult proof;
  core_ir::Instruction* object = nullptr;
  std::unordered_set<core_ir::Instruction*> addressInstructions;
  std::vector<LocalMemoryAccess> loads;
  std::vector<LocalMemoryAccess> stores;
};

LocalObjectInventory inventoryLocalObject(
    core_ir::Instruction* allocaInstruction) {
  LocalObjectInventory result;
  result.object = allocaInstruction;
  if (!allocaInstruction ||
      allocaInstruction->opcode() != core_ir::Opcode::Alloca ||
      !allocaInstruction->parent() || allocaInstruction->memorySizeBytes() <= 0) {
    result.proof = ProofResult::disproved("dead-local-requires-sized-alloca");
    return result;
  }
  result.addressInstructions.insert(allocaInstruction);
  std::vector<core_ir::Value*> work{allocaInstruction};
  std::unordered_set<core_ir::Value*> seenAddresses;
  for (std::size_t index = 0; index < work.size(); ++index) {
    auto* address = work[index];
    if (!address || !seenAddresses.insert(address).second)
      continue;
    const auto users = address->uses();
    for (auto* user : users) {
      if (!user || !user->parent()) {
        result.proof = ProofResult::unknown("dead-local-user-not-live");
        return result;
      }
      if (isDerivedAddressUse(user, address)) {
        if (!user->type().isPtr()) {
          result.proof = ProofResult::unknown("dead-local-derived-type-unknown");
          return result;
        }
        result.addressInstructions.insert(user);
        work.push_back(user);
        continue;
      }
      const bool store = user->opcode() == core_ir::Opcode::Store ||
                         user->opcode() == core_ir::Opcode::VStore;
      const bool load = user->opcode() == core_ir::Opcode::Load ||
                        user->opcode() == core_ir::Opcode::VLoad;
      const bool validAddressUse =
          user->numOperands() >= (store ? 2 : 1) &&
          user->operand(store ? 1 : 0) == address;
      if ((store || load) && validAddressUse &&
          (!store || user->operand(0) != address)) {
        const int bytes = accessBytes(user);
        const auto range = evaluatePointerRange(address, bytes);
        if (bytes <= 0 || !range.proof.proved() ||
            range.object != allocaInstruction || range.beginByte < 0 ||
            range.endByte > allocaInstruction->memorySizeBytes()) {
          result.proof = ProofResult::unknown(
              store ? "dead-local-store-range-unknown"
                    : "dead-local-load-range-unknown");
          return result;
        }
        LocalMemoryAccess access{user, range.beginByte, range.endByte};
        (store ? result.stores : result.loads).push_back(access);
        continue;
      }
      result.proof = ProofResult::unknown("dead-local-address-escapes");
      return result;
    }
  }
  result.proof = ProofResult::success("local-materialization-inventory-complete");
  return result;
}

bool overlaps(const LocalMemoryAccess& lhs, const LocalMemoryAccess& rhs) {
  return lhs.beginByte < rhs.endByte && rhs.beginByte < lhs.endByte;
}

} // namespace

DeadLocalMaterializationProof proveDeadLocalMaterialization(
    core_ir::Instruction* allocaInstruction) {
  DeadLocalMaterializationProof result;
  result.object = allocaInstruction;
  const auto inventory = inventoryLocalObject(allocaInstruction);
  if (!inventory.proof.proved()) {
    result.proof = inventory.proof;
    return result;
  }
  result.bytes = static_cast<std::size_t>(allocaInstruction->memorySizeBytes());
  if (!inventory.loads.empty()) {
    result.proof = ProofResult::disproved("dead-local-has-reader");
    return result;
  }
  if (inventory.stores.empty()) {
    result.proof = ProofResult::disproved("dead-local-has-no-materialization");
    return result;
  }
  std::unordered_set<core_ir::Instruction*> instructions =
      inventory.addressInstructions;
  for (const auto& store : inventory.stores)
    instructions.insert(store.instruction);
  result.stores = inventory.stores.size();
  result.instructions.assign(instructions.begin(), instructions.end());
  std::sort(result.instructions.begin(), result.instructions.end(),
            [](const auto* lhs, const auto* rhs) {
              return lhs->id() < rhs->id();
            });
  result.proof = ProofResult::success("dead-local-materialization-proved");
  return result;
}

PartialLocalMaterializationProof provePartialLocalMaterialization(
    core_ir::Instruction* allocaInstruction) {
  PartialLocalMaterializationProof result;
  result.object = allocaInstruction;
  const auto inventory = inventoryLocalObject(allocaInstruction);
  if (!inventory.proof.proved()) {
    result.proof = inventory.proof;
    return result;
  }
  result.retainedLoads = inventory.loads.size();
  if (inventory.loads.empty()) {
    result.proof = ProofResult::disproved("partial-local-has-no-reader");
    return result;
  }
  for (const auto& store : inventory.stores) {
    const bool observed = std::any_of(
        inventory.loads.begin(), inventory.loads.end(),
        [&](const auto& load) { return overlaps(store, load); });
    if (observed)
      ++result.retainedStores;
    else
      result.storesToDelete.push_back(store.instruction);
  }
  std::sort(result.storesToDelete.begin(), result.storesToDelete.end(),
            [](const auto* lhs, const auto* rhs) {
              return lhs->id() < rhs->id();
            });
  result.proof = result.storesToDelete.empty()
                     ? ProofResult::disproved("partial-local-all-stores-observed")
                     : ProofResult::success("partial-local-store-slice-proved");
  return result;
}

std::vector<DeadLocalMaterializationProof> findDeadLocalMaterializations(
    core_ir::Module& module) {
  std::vector<DeadLocalMaterializationProof> result;
  for (const auto& ownedFunction : module.functions()) {
    for (const auto& ownedBlock : ownedFunction->blocks()) {
      for (const auto& ownedInstruction : ownedBlock->instructions()) {
        auto* instruction = ownedInstruction.get();
        if (instruction->opcode() != core_ir::Opcode::Alloca)
          continue;
        auto proof = proveDeadLocalMaterialization(instruction);
        if (proof.proof.proved())
          result.push_back(std::move(proof));
      }
    }
  }
  std::sort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) {
    return lhs.object->id() < rhs.object->id();
  });
  return result;
}

std::vector<PartialLocalMaterializationProof>
findPartialLocalMaterializations(core_ir::Module& module) {
  std::vector<PartialLocalMaterializationProof> result;
  for (const auto& ownedFunction : module.functions()) {
    for (const auto& ownedBlock : ownedFunction->blocks()) {
      for (const auto& ownedInstruction : ownedBlock->instructions()) {
        auto* instruction = ownedInstruction.get();
        if (instruction->opcode() != core_ir::Opcode::Alloca)
          continue;
        auto proof = provePartialLocalMaterialization(instruction);
        if (proof.proof.proved())
          result.push_back(std::move(proof));
      }
    }
  }
  std::sort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) {
    return lhs.object->id() < rhs.object->id();
  });
  return result;
}

} // namespace analysis
