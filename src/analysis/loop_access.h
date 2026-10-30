// 文件职责：统一枚举循环 load/store 的对象与 affine 地址事实。
#pragma once

#include "affine_access.h"
#include "proof_result.h"

namespace analysis {

struct LoopAccess {
  core_ir::Instruction* instruction = nullptr;
  core_ir::Value* address = nullptr;
  core_ir::Value* object = nullptr;
  bool reads = false;
  bool writes = false;
  int accessBytes = 4;
  AffineAccessSummary affine;
  ProofResult proof;
};

std::vector<LoopAccess> collectLoopAccesses(
    const NaturalLoop& loop, const std::vector<InductionVar>& ivs);

} // namespace analysis
