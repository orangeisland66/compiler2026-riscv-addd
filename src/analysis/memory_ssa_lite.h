// 文件职责：构建按底层对象登记的 MemorySSA-lite，并提供统一的保守版本查询。
#pragma once

#include "function_effects.h"
#include "proof_result.h"

#include <unordered_map>
#include <vector>

namespace analysis {

enum class MemoryAccessKind { LiveOnEntry, Use, Def, Phi };

struct MemoryAccessNode {
  int id = 0;
  MemoryAccessKind kind = MemoryAccessKind::LiveOnEntry;
  core_ir::Instruction* instruction = nullptr;
  core_ir::BasicBlock* block = nullptr;
  int definingAccess = 0;
  int partition = 0;
  std::vector<int> incoming;
};

struct MemoryPartition {
  int id = 0;
  core_ir::Value* object = nullptr;
  bool unknownOrEscaped = true;
  std::vector<int> accesses;
};

struct MemorySSALite {
  ProofResult proof;
  std::vector<MemoryAccessNode> nodes;
  std::unordered_map<const core_ir::Instruction*, int> accessForInstruction;
  std::unordered_map<const core_ir::Instruction*, int> partitionForInstruction;
  std::unordered_map<const core_ir::BasicBlock*, int> liveIn;
  std::unordered_map<const core_ir::BasicBlock*, int> liveOut;
  std::vector<MemoryPartition> partitions;
};

MemorySSALite buildMemorySSALite(
    core_ir::Function* function,
    const FunctionEffectMap* effects = nullptr);

int clobberBefore(const MemorySSALite& memory,
                  const core_ir::Instruction* instruction);
ProofResult sameMemoryVersion(const MemorySSALite& memory,
                              const core_ir::Instruction* lhs,
                              const core_ir::Instruction* rhs,
                              const FunctionEffectMap* effects = nullptr);
ProofResult sameMemoryVersionAcrossCFG(
    const MemorySSALite& memory,
    const core_ir::Instruction* lhs,
    const core_ir::Instruction* rhs,
    const FunctionEffectMap* effects = nullptr);
ProofResult mustOverwrite(const core_ir::Instruction* earlierStore,
                          const core_ir::Instruction* laterStore);
ProofResult storedValueReachesLoad(
    const core_ir::Instruction* store,
    const core_ir::Instruction* load,
    const FunctionEffectMap* effects = nullptr);
ProofResult memoryValueReachesStore(
    const core_ir::Instruction* definingAccess,
    const core_ir::Instruction* store,
    const FunctionEffectMap* effects = nullptr);
ProofResult hasObserverBetween(const core_ir::Instruction* earlierStore,
                               const core_ir::Instruction* laterStore,
                               const FunctionEffectMap* effects = nullptr);

} // namespace analysis
