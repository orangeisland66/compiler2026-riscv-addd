// 文件职责：识别 affine copy/view 区域，为视图转发和 producer 删除提供证明。本文件负责声明接口和数据结构。
#pragma once

#include "affine_access.h"
#include "function_analysis.h"
#include "loop_info.h"

#include <vector>

namespace analysis {

struct AffineCopyRegionSummary {
  bool valid = false;
  core_ir::Instruction* store = nullptr;
  core_ir::Instruction* valueInst = nullptr;
  core_ir::Value* dstRoot = nullptr;
  core_ir::Value* srcRoot = nullptr;
  AffineAccessSummary dstAccess;
  AffineAccessSummary srcAccess;
  bool negated = false;
};

std::vector<AffineCopyRegionSummary>
collectAffineCopyRegions(core_ir::Function* func,
                         const NaturalLoop& loop,
                         const std::vector<InductionVar>& ivs);

} // namespace analysis
