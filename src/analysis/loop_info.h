// 文件职责：提取自然循环和循环基本结构，供 HIR/CoreIR pass 复用。本文件负责声明接口和数据结构。
#pragma once
#include "../core_ir/ir/core_ir.h"
#include "dominators.h"
#include <unordered_set>
#include <vector>

namespace analysis {

struct NaturalLoop {
  core_ir::BasicBlock* header = nullptr;
  std::unordered_set<core_ir::BasicBlock*> blocks;
  std::vector<core_ir::BasicBlock*> latches;
};

// 收集函数中的所有自然循环。
std::vector<NaturalLoop> collectNaturalLoops(
    const std::vector<core_ir::BasicBlock*>& blocks, const DomSet& doms,
    const std::unordered_set<core_ir::BasicBlock*>& reachable);

// 返回循环的专用前置块；没有时返回 nullptr。
core_ir::BasicBlock* dedicatedPreheader(const NaturalLoop& loop);

} // namespace analysis
