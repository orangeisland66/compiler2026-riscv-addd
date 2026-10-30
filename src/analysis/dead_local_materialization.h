// 文件职责：证明本函数 nonescaping alloca 的全部 materialization 写不可观察。
#pragma once

#include "../core_ir/ir/core_ir.h"
#include "proof_result.h"

#include <cstddef>
#include <vector>

namespace analysis {

struct DeadLocalMaterializationProof {
  ProofResult proof;
  core_ir::Instruction* object = nullptr;
  std::vector<core_ir::Instruction*> instructions;
  std::size_t stores = 0;
  std::size_t bytes = 0;
};

struct PartialLocalMaterializationProof {
  ProofResult proof;
  core_ir::Instruction* object = nullptr;
  std::vector<core_ir::Instruction*> storesToDelete;
  std::size_t retainedLoads = 0;
  std::size_t retainedStores = 0;
};

DeadLocalMaterializationProof proveDeadLocalMaterialization(
    core_ir::Instruction* allocaInstruction);

std::vector<DeadLocalMaterializationProof> findDeadLocalMaterializations(
    core_ir::Module& module);

PartialLocalMaterializationProof provePartialLocalMaterialization(
    core_ir::Instruction* allocaInstruction);

std::vector<PartialLocalMaterializationProof>
findPartialLocalMaterializations(core_ir::Module& module);

} // namespace analysis
