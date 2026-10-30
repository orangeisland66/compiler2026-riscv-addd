// 文件职责：归纳循环 guard 和边界信息，为 interior/remainder 拆分提供依据。本文件负责声明接口和数据结构。
#pragma once

#include "affine_access.h"

#include <string>
#include <vector>

namespace analysis {

struct AffineGuardSummary {
  bool valid = false;
  const NaturalLoop* loop = nullptr;
  core_ir::BasicBlock* block = nullptr;
  core_ir::BasicBlock* header = nullptr;
  core_ir::BasicBlock* continueBlock = nullptr;
  core_ir::BasicBlock* workBlock = nullptr;
  core_ir::Instruction* branch = nullptr;
  core_ir::Instruction* compare = nullptr;
  core_ir::Value* induction = nullptr;
  bool compareTrueForWork = true;
  AffineExprSummary boundExpr;
  std::string relation;
};

std::vector<AffineGuardSummary>
collectAffineGuards(core_ir::Function* func, const NaturalLoop& loop,
                    const std::vector<InductionVar>& ivs);

} // namespace analysis
