// 文件职责：实现按底层对象登记的 MemorySSA-lite，优先保证保守性和确定性。
#include "memory_ssa_lite.h"

#include "alias_analysis.h"
#include "function_modref.h"
#include "pointer_range.h"

#include <algorithm>
#include <unordered_set>

namespace analysis {

namespace {

bool isMemoryInstruction(const core_ir::Instruction* instruction) {
  if (!instruction) return false;
  switch (instruction->opcode()) {
  case core_ir::Opcode::Load:
  case core_ir::Opcode::Store:
  case core_ir::Opcode::VLoad:
  case core_ir::Opcode::VStore:
  case core_ir::Opcode::Call:
    return true;
  default:
    return false;
  }
}

bool isMemoryDef(const core_ir::Instruction* instruction,
                 const FunctionEffectMap* effects) {
  if (instruction->opcode() == core_ir::Opcode::Store ||
      instruction->opcode() == core_ir::Opcode::VStore)
    return true;
  if (instruction->opcode() != core_ir::Opcode::Call) return false;
  if (!effects || !instruction->parent() ||
      !instruction->parent()->parent() ||
      !instruction->parent()->parent()->parent())
    return true;
  auto* module = instruction->parent()->parent()->parent();
  const auto* summary = lookupFunctionEffect(*effects, *module,
                                             instruction->name());
  return !summary || !summary->readonly;
}

core_ir::Value* memoryAddress(const core_ir::Instruction* instruction) {
  if (!instruction) return nullptr;
  switch (instruction->opcode()) {
  case core_ir::Opcode::Load:
  case core_ir::Opcode::VLoad:
    return instruction->numOperands() >= 1 ? instruction->operand(0) : nullptr;
  case core_ir::Opcode::Store:
  case core_ir::Opcode::VStore:
    return instruction->numOperands() >= 2 ? instruction->operand(1) : nullptr;
  default:
    return nullptr;
  }
}

int accessBytes(const core_ir::Instruction* instruction) {
  if (!instruction) return 0;
  if (instruction->memorySizeBytes() > 0)
    return instruction->memorySizeBytes();
  if (instruction->opcode() == core_ir::Opcode::Load)
    return instruction->type().isPtr() ? 8 : 4;
  if (instruction->opcode() == core_ir::Opcode::Store &&
      instruction->numOperands() >= 1)
    return instruction->operand(0)->type().isPtr() ? 8 : 4;
  return 0;
}

bool instructionBeforeInBlock(const core_ir::Instruction* lhs,
                              const core_ir::Instruction* rhs) {
  if (!lhs || !rhs || !lhs->parent() || lhs->parent() != rhs->parent())
    return false;
  for (const auto& owned : lhs->parent()->instructions()) {
    if (owned.get() == lhs) return true;
    if (owned.get() == rhs) return false;
  }
  return false;
}

// 收集两个指令之间必经的唯一 CFG 链。该查询刻意不把 dominance 当作
// post-dominance 使用：只接受每一步唯一 successor 且后继只有当前 predecessor
// 的无合流、无回边路径。任何更一般的控制流都交给后续的支配/后支配版本实现。
bool collectUniquePathInstructions(
    const core_ir::Instruction* first,
    const core_ir::Instruction* second,
    std::vector<const core_ir::Instruction*>& between) {
  if (!first || !second || !first->parent() || !second->parent() ||
      !first->parent()->parent() ||
      first->parent()->parent() != second->parent()->parent())
    return false;
  if (first->parent() == second->parent()) {
    if (!instructionBeforeInBlock(first, second)) return false;
    bool active = false;
    for (const auto& owned : first->parent()->instructions()) {
      const auto* instruction = owned.get();
      if (instruction == first) {
        active = true;
        continue;
      }
      if (instruction == second) return true;
      if (active) between.push_back(instruction);
    }
    return false;
  }

  bool afterFirst = false;
  for (const auto& owned : first->parent()->instructions()) {
    const auto* instruction = owned.get();
    if (instruction == first) {
      afterFirst = true;
      continue;
    }
    if (afterFirst) between.push_back(instruction);
  }

  std::unordered_set<const core_ir::BasicBlock*> visited;
  auto* current = first->parent();
  visited.insert(current);
  while (current != second->parent()) {
    if (current->succs().size() != 1) return false;
    auto* next = current->succs().front();
    if (!next || next->preds().size() != 1 ||
        next->preds().front() != current || !visited.insert(next).second)
      return false;
    current = next;
    for (const auto& owned : current->instructions()) {
      const auto* instruction = owned.get();
      if (current == second->parent() && instruction == second) return true;
      between.push_back(instruction);
    }
  }
  return false;
}

bool collectAllAcyclicPathInstructions(
    const core_ir::Instruction* first,
    const core_ir::Instruction* second,
    std::vector<const core_ir::Instruction*>& between) {
  if (!first || !second || !first->parent() || !second->parent() ||
      !first->parent()->parent() ||
      first->parent()->parent() != second->parent()->parent())
    return false;
  if (first->parent() == second->parent())
    return collectUniquePathInstructions(first, second, between);

  bool afterFirst = false;
  for (const auto& owned : first->parent()->instructions()) {
    const auto* instruction = owned.get();
    if (instruction == first) {
      afterFirst = true;
      continue;
    }
    if (afterFirst) between.push_back(instruction);
  }

  std::unordered_set<const core_ir::BasicBlock*> visiting;
  std::unordered_set<const core_ir::BasicBlock*> completed;
  std::unordered_set<const core_ir::Instruction*> collected;
  auto visit = [&](auto&& self, const core_ir::BasicBlock* block) -> bool {
    if (!block) return false;
    if (block == second->parent()) {
      for (const auto& owned : block->instructions()) {
        const auto* instruction = owned.get();
        if (instruction == second) return true;
        if (collected.insert(instruction).second)
          between.push_back(instruction);
      }
      return false;
    }
    if (completed.count(block)) return true;
    if (!visiting.insert(block).second || block->succs().empty()) return false;
    for (const auto& owned : block->instructions())
      if (collected.insert(owned.get()).second)
        between.push_back(owned.get());
    for (auto* successor : block->succs())
      if (!self(self, successor)) return false;
    visiting.erase(block);
    completed.insert(block);
    return true;
  };

  visiting.insert(first->parent());
  for (auto* successor : first->parent()->succs())
    if (!visit(visit, successor)) return false;
  return true;
}

bool collectProvedPathInstructions(
    const core_ir::Instruction* first,
    const core_ir::Instruction* second,
    std::vector<const core_ir::Instruction*>& between) {
  if (collectUniquePathInstructions(first, second, between)) return true;
  between.clear();
  return collectAllAcyclicPathInstructions(first, second, between);
}

bool callMayWrite(const core_ir::Instruction* call,
                  const FunctionEffectMap* effects,
                  core_ir::Value* location = nullptr,
                  int locationBytes = 0) {
  if (!call || call->opcode() != core_ir::Opcode::Call) return false;
  if (!effects || !call->parent() || !call->parent()->parent() ||
      !call->parent()->parent()->parent())
    return true;
  if (location && locationBytes > 0) {
    const CallModRefResult modRef =
        queryCallModRef(call, location, locationBytes, *effects);
    return !modRef.proof.proved() || modRef.mayWrite;
  }
  const auto* summary = lookupFunctionEffect(
      *effects, *call->parent()->parent()->parent(), call->name());
  return !summary || !summary->readonly;
}

bool callMayRead(const core_ir::Instruction* call,
                 const FunctionEffectMap* effects,
                 core_ir::Value* location,
                 int locationBytes) {
  if (!call || call->opcode() != core_ir::Opcode::Call || !effects ||
      !location || locationBytes <= 0)
    return true;
  const CallModRefResult modRef =
      queryCallModRef(call, location, locationBytes, *effects);
  return !modRef.proof.proved() || modRef.mayRead;
}

std::vector<core_ir::BasicBlock*> reachableOrder(core_ir::Function* function) {
  std::vector<core_ir::BasicBlock*> order;
  if (!function || !function->entryBlock()) return order;
  std::unordered_set<core_ir::BasicBlock*> seen;
  std::vector<core_ir::BasicBlock*> work{function->entryBlock()};
  seen.insert(function->entryBlock());
  for (std::size_t index = 0; index < work.size(); ++index) {
    auto* block = work[index];
    order.push_back(block);
    for (auto* successor : block->succs())
      if (successor && seen.insert(successor).second)
        work.push_back(successor);
  }
  return order;
}

} // namespace

MemorySSALite buildMemorySSALite(core_ir::Function* function,
                                 const FunctionEffectMap* effects) {
  MemorySSALite result;
  result.nodes.push_back({0, MemoryAccessKind::LiveOnEntry, nullptr,
                          function ? function->entryBlock() : nullptr, 0, 0,
                          {}});
  result.partitions.push_back({0, nullptr, true, {0}});
  if (!function || !function->entryBlock()) {
    result.proof = ProofResult::disproved("memoryssa-missing-entry");
    return result;
  }
  auto order = reachableOrder(function);
  std::unordered_map<const core_ir::Instruction*, int> defIds;
  std::unordered_map<const core_ir::BasicBlock*, int> phiIds;
  std::unordered_map<core_ir::Value*, int> objectPartitions;
  int nextId = 1;
  auto partitionFor = [&](const core_ir::Instruction* instruction) {
    core_ir::Value* object = underlyingPointerObject(memoryAddress(instruction));
    if (!object) return 0;
    auto found = objectPartitions.find(object);
    if (found != objectPartitions.end()) return found->second;
    const int id = static_cast<int>(result.partitions.size());
    const bool owned = dynamic_cast<core_ir::GlobalVar*>(object) ||
        (dynamic_cast<core_ir::Instruction*>(object) &&
         static_cast<core_ir::Instruction*>(object)->opcode() ==
             core_ir::Opcode::Alloca);
    result.partitions.push_back({id, object, !owned, {}});
    objectPartitions.emplace(object, id);
    return id;
  };
  for (auto* block : order) {
    if (block != function->entryBlock() && block->preds().size() > 1)
      phiIds[block] = nextId++;
    for (const auto& instruction : block->instructions())
      if (isMemoryInstruction(instruction.get()) &&
          isMemoryDef(instruction.get(), effects))
        defIds[instruction.get()] = nextId++;
  }

  for (auto* block : order) {
    int incoming = 0;
    std::vector<int> predecessorVersions;
    for (auto* predecessor : block->preds()) {
      auto found = result.liveOut.find(predecessor);
      predecessorVersions.push_back(found == result.liveOut.end() ? 0
                                                                  : found->second);
    }
    if (!predecessorVersions.empty()) {
      incoming = predecessorVersions.front();
      if (!std::all_of(predecessorVersions.begin(), predecessorVersions.end(),
                       [&](int version) { return version == incoming; }))
        incoming = phiIds.count(block) ? phiIds[block] : 0;
    }
    result.liveIn[block] = incoming;
    if (phiIds.count(block)) {
      const int id = phiIds[block];
      result.nodes.push_back({id, MemoryAccessKind::Phi, nullptr, block, 0, 0,
                              predecessorVersions});
      incoming = id;
      result.liveIn[block] = id;
    }
    int current = incoming;
    for (const auto& owned : block->instructions()) {
      auto* instruction = owned.get();
      if (!isMemoryInstruction(instruction)) continue;
      const int partition = partitionFor(instruction);
      if (isMemoryDef(instruction, effects)) {
        const int id = defIds.at(instruction);
        result.nodes.push_back({id, MemoryAccessKind::Def, instruction, block,
                                current, partition, {}});
        result.accessForInstruction[instruction] = id;
        result.partitionForInstruction[instruction] = partition;
        result.partitions[partition].accesses.push_back(id);
        current = id;
      } else {
        const int id = nextId++;
        result.nodes.push_back({id, MemoryAccessKind::Use, instruction, block,
                                current, partition, {}});
        result.accessForInstruction[instruction] = id;
        result.partitionForInstruction[instruction] = partition;
        result.partitions[partition].accesses.push_back(id);
      }
    }
    result.liveOut[block] = current;
  }
  // Backedge predecessors may appear after their header in traversal order.
  // Phi ids are stable, so refresh their incoming versions once every block's
  // final outgoing version is known.
  for (auto& node : result.nodes) {
    if (node.kind != MemoryAccessKind::Phi || !node.block) continue;
    node.incoming.clear();
    for (auto* predecessor : node.block->preds()) {
      auto found = result.liveOut.find(predecessor);
      node.incoming.push_back(found == result.liveOut.end() ? 0
                                                            : found->second);
    }
  }
  std::sort(result.nodes.begin() + 1, result.nodes.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.id < rhs.id; });
  result.proof = ProofResult::success("memoryssa-lite-built");
  return result;
}

int clobberBefore(const MemorySSALite& memory,
                  const core_ir::Instruction* instruction) {
  auto access = memory.accessForInstruction.find(instruction);
  if (access == memory.accessForInstruction.end()) return -1;
  const int id = access->second;
  auto node = std::find_if(memory.nodes.begin(), memory.nodes.end(),
                           [&](const MemoryAccessNode& item) {
                             return item.id == id;
                           });
  return node == memory.nodes.end() ? -1 : node->definingAccess;
}

ProofResult sameMemoryVersion(const MemorySSALite& memory,
                              const core_ir::Instruction* lhs,
                              const core_ir::Instruction* rhs,
                              const FunctionEffectMap* effects) {
  if (!lhs || !rhs || lhs->opcode() != core_ir::Opcode::Load ||
      rhs->opcode() != core_ir::Opcode::Load)
    return ProofResult::disproved("memory-version-requires-loads");
  const int bytes = accessBytes(lhs);
  if (bytes <= 0 || bytes != accessBytes(rhs) ||
      queryAlias(memoryAddress(lhs), bytes, memoryAddress(rhs), bytes).kind !=
          AliasKind::MustAlias)
    return ProofResult::disproved("memory-version-location-mismatch");
  std::vector<const core_ir::Instruction*> between;
  const core_ir::Instruction* first = lhs;
  const core_ir::Instruction* second = rhs;
  bool ordered = collectUniquePathInstructions(first, second, between);
  if (!ordered) {
    between.clear();
    std::swap(first, second);
    ordered = collectUniquePathInstructions(first, second, between);
  }
  if (ordered) {
    for (const auto* instruction : between) {
      if (instruction->opcode() == core_ir::Opcode::Call &&
          callMayWrite(instruction, effects, memoryAddress(first), bytes))
        return ProofResult::unknown("memory-version-call-clobber");
      if (instruction->opcode() == core_ir::Opcode::Store ||
          instruction->opcode() == core_ir::Opcode::VStore) {
        const int storeBytes = accessBytes(instruction);
        if (storeBytes <= 0 ||
            queryAlias(memoryAddress(first), bytes,
                       memoryAddress(instruction), storeBytes).kind !=
                AliasKind::NoAlias)
          return ProofResult::unknown("memory-version-may-alias-store");
      }
    }
    return ProofResult::success("same-partition-version");
  }
  const int leftClobber = clobberBefore(memory, lhs);
  const int rightClobber = clobberBefore(memory, rhs);
  if (leftClobber >= 0 && leftClobber == rightClobber)
    return ProofResult::success("same-conservative-memory-version");
  return ProofResult::unknown("memory-version-path-merge-or-clobber");
}

ProofResult sameMemoryVersionAcrossCFG(
    const MemorySSALite& memory,
    const core_ir::Instruction* lhs,
    const core_ir::Instruction* rhs,
    const FunctionEffectMap* effects) {
  if (!lhs || !rhs || lhs->opcode() != core_ir::Opcode::Load ||
      rhs->opcode() != core_ir::Opcode::Load)
    return ProofResult::disproved("memory-version-requires-loads");
  auto leftPartition = memory.partitionForInstruction.find(lhs);
  auto rightPartition = memory.partitionForInstruction.find(rhs);
  if (leftPartition == memory.partitionForInstruction.end() ||
      rightPartition == memory.partitionForInstruction.end() ||
      leftPartition->second != rightPartition->second)
    return ProofResult::unknown("memory-version-partition-unavailable");
  const int bytes = accessBytes(lhs);
  if (bytes <= 0 || bytes != accessBytes(rhs) ||
      queryAlias(memoryAddress(lhs), bytes, memoryAddress(rhs), bytes).kind !=
          AliasKind::MustAlias)
    return ProofResult::disproved("memory-version-location-mismatch");
  std::vector<const core_ir::Instruction*> between;
  const core_ir::Instruction* first = lhs;
  const core_ir::Instruction* second = rhs;
  bool ordered = collectProvedPathInstructions(first, second, between);
  if (!ordered) {
    between.clear();
    std::swap(first, second);
    ordered = collectProvedPathInstructions(first, second, between);
  }
  if (!ordered)
    return ProofResult::unknown("memory-version-path-merge-or-backedge");
  for (const auto* instruction : between) {
    if (instruction->opcode() == core_ir::Opcode::Call &&
        callMayWrite(instruction, effects, memoryAddress(first), bytes))
      return ProofResult::unknown("memory-version-call-clobber");
    if (instruction->opcode() == core_ir::Opcode::Store ||
        instruction->opcode() == core_ir::Opcode::VStore) {
      const int storeBytes = accessBytes(instruction);
      if (storeBytes <= 0 ||
          queryAlias(memoryAddress(first), bytes,
                     memoryAddress(instruction), storeBytes).kind !=
              AliasKind::NoAlias)
        return ProofResult::unknown("memory-version-may-alias-store");
    }
  }
  return ProofResult::success("same-memory-version-all-acyclic-paths");
}

ProofResult mustOverwrite(const core_ir::Instruction* earlierStore,
                          const core_ir::Instruction* laterStore) {
  if (!earlierStore || !laterStore ||
      earlierStore->opcode() != core_ir::Opcode::Store ||
      laterStore->opcode() != core_ir::Opcode::Store)
    return ProofResult::disproved("must-overwrite-requires-stores");
  std::vector<const core_ir::Instruction*> between;
  if (!collectUniquePathInstructions(earlierStore, laterStore, between))
    return ProofResult::unknown("must-overwrite-control-proof-missing");
  const int earlierBytes = accessBytes(earlierStore);
  const int laterBytes = accessBytes(laterStore);
  if (earlierBytes <= 0 || laterBytes < earlierBytes)
    return ProofResult::unknown("must-overwrite-width-insufficient");
  const auto earlier = evaluatePointerRange(memoryAddress(earlierStore),
                                             earlierBytes);
  const auto later = evaluatePointerRange(memoryAddress(laterStore), laterBytes);
  if (!earlier.proof.proved() || !later.proof.proved() ||
      earlier.object != later.object)
    return ProofResult::unknown("must-overwrite-range-unknown");
  if (later.beginByte <= earlier.beginByte &&
      later.endByte >= earlier.endByte)
    return ProofResult::success("must-overwrite-complete-range");
  return ProofResult::disproved("must-overwrite-partial-range");
}

ProofResult storedValueReachesLoad(const core_ir::Instruction* store,
                                   const core_ir::Instruction* load,
                                   const FunctionEffectMap* effects) {
  if (!store || !load || store->opcode() != core_ir::Opcode::Store ||
      load->opcode() != core_ir::Opcode::Load)
    return ProofResult::disproved("store-forward-requires-scalar-accesses");
  std::vector<const core_ir::Instruction*> between;
  if (!collectUniquePathInstructions(store, load, between))
    return ProofResult::unknown("store-forward-control-proof-missing");
  const int storeBytes = accessBytes(store);
  const int loadBytes = accessBytes(load);
  if (storeBytes <= 0 || storeBytes != loadBytes ||
      queryAlias(memoryAddress(store), storeBytes, memoryAddress(load),
                 loadBytes).kind != AliasKind::MustAlias)
    return ProofResult::disproved("store-forward-location-mismatch");

  for (const auto* instruction : between) {
    if (instruction->opcode() == core_ir::Opcode::Call &&
        callMayWrite(instruction, effects, memoryAddress(store), storeBytes))
      return ProofResult::unknown("store-forward-call-clobber");
    if (instruction->opcode() == core_ir::Opcode::Store ||
        instruction->opcode() == core_ir::Opcode::VStore) {
      const int interveningBytes = accessBytes(instruction);
      if (interveningBytes <= 0 ||
          queryAlias(memoryAddress(store), storeBytes,
                     memoryAddress(instruction), interveningBytes).kind !=
              AliasKind::NoAlias)
        return ProofResult::unknown("store-forward-may-alias-write");
    }
  }
  return ProofResult::success("store-value-reaches-load");
}

ProofResult memoryValueReachesStore(
    const core_ir::Instruction* definingAccess,
    const core_ir::Instruction* store,
    const FunctionEffectMap* effects) {
  if (!definingAccess || !store ||
      (definingAccess->opcode() != core_ir::Opcode::Load &&
       definingAccess->opcode() != core_ir::Opcode::Store) ||
      store->opcode() != core_ir::Opcode::Store)
    return ProofResult::disproved("redundant-store-requires-scalar-accesses");
  std::vector<const core_ir::Instruction*> between;
  if (!collectUniquePathInstructions(definingAccess, store, between))
    return ProofResult::unknown("redundant-store-control-proof-missing");
  const int definingBytes = accessBytes(definingAccess);
  const int storeBytes = accessBytes(store);
  if (definingBytes <= 0 || definingBytes != storeBytes ||
      queryAlias(memoryAddress(definingAccess), definingBytes,
                 memoryAddress(store), storeBytes).kind !=
          AliasKind::MustAlias)
    return ProofResult::disproved("redundant-store-location-mismatch");

  for (const auto* instruction : between) {
    if (instruction->opcode() == core_ir::Opcode::Call &&
        callMayWrite(instruction, effects, memoryAddress(definingAccess),
                     definingBytes))
      return ProofResult::unknown("redundant-store-call-clobber");
    if (instruction->opcode() == core_ir::Opcode::Store ||
        instruction->opcode() == core_ir::Opcode::VStore) {
      const int interveningBytes = accessBytes(instruction);
      if (interveningBytes <= 0 ||
          queryAlias(memoryAddress(definingAccess), definingBytes,
                     memoryAddress(instruction), interveningBytes).kind !=
              AliasKind::NoAlias)
        return ProofResult::unknown("redundant-store-may-alias-write");
    }
  }
  return ProofResult::success("memory-value-reaches-store");
}

ProofResult hasObserverBetween(const core_ir::Instruction* earlierStore,
                               const core_ir::Instruction* laterStore,
                               const FunctionEffectMap* effects) {
  if (!earlierStore || !laterStore)
    return ProofResult::unknown("observer-order-unknown");
  std::vector<const core_ir::Instruction*> between;
  if (!collectUniquePathInstructions(earlierStore, laterStore, between))
    return ProofResult::unknown("observer-order-unknown");
  const int bytes = accessBytes(earlierStore);
  for (const auto* instruction : between) {
    if (instruction->opcode() == core_ir::Opcode::Call &&
        callMayRead(instruction, effects, memoryAddress(earlierStore), bytes))
      return ProofResult::success("observer-call");
    if (instruction->opcode() == core_ir::Opcode::Load ||
        instruction->opcode() == core_ir::Opcode::VLoad) {
      const int loadBytes = accessBytes(instruction);
      if (loadBytes <= 0 ||
          queryAlias(memoryAddress(earlierStore), bytes,
                     memoryAddress(instruction), loadBytes).kind !=
              AliasKind::NoAlias)
        return ProofResult::success("observer-aliasing-load");
    }
  }
  return ProofResult::disproved("observer-none-between");
}

} // namespace analysis
