// 文件职责：提取自然循环和循环基本结构，供 HIR/CoreIR pass 复用。本文件负责实现具体逻辑。
#include "loop_info.h"

namespace analysis {

std::vector<NaturalLoop> collectNaturalLoops(
    const std::vector<core_ir::BasicBlock*>& blocks, const DomSet& doms,
    const std::unordered_set<core_ir::BasicBlock*>& reachable) {
  std::vector<NaturalLoop> loops;
  std::unordered_map<core_ir::BasicBlock*, int> loopByHeader;

  for (auto* latch : blocks) {
    for (auto* succ : latch->succs()) {
      if (!succ || !reachable.count(succ) || !dominates(doms, succ, latch))
        continue;

      std::unordered_set<core_ir::BasicBlock*> loopBlocks;
      loopBlocks.insert(succ);
      loopBlocks.insert(latch);

      std::vector<core_ir::BasicBlock*> stack;
      if (latch != succ)
        stack.push_back(latch);
      while (!stack.empty()) {
        core_ir::BasicBlock* bb = stack.back();
        stack.pop_back();
        for (auto* pred : bb->preds()) {
          if (!pred || !reachable.count(pred)) continue;
          if (!loopBlocks.insert(pred).second) continue;
          if (pred != succ)
            stack.push_back(pred);
        }
      }

      auto it = loopByHeader.find(succ);
      if (it == loopByHeader.end()) {
        NaturalLoop loop;
        loop.header = succ;
        loop.blocks = std::move(loopBlocks);
        loop.latches.push_back(latch);
        loopByHeader[succ] = static_cast<int>(loops.size());
        loops.push_back(std::move(loop));
      } else {
        NaturalLoop& loop = loops[it->second];
        loop.blocks.insert(loopBlocks.begin(), loopBlocks.end());
        loop.latches.push_back(latch);
      }
    }
  }

  return loops;
}

core_ir::BasicBlock* dedicatedPreheader(const NaturalLoop& loop) {
  if (!loop.header) return nullptr;
  std::vector<core_ir::BasicBlock*> outsidePreds;
  for (auto* pred : loop.header->preds()) {
    if (pred && !loop.blocks.count(pred))
      outsidePreds.push_back(pred);
  }
  if (outsidePreds.size() != 1) return nullptr;

  core_ir::BasicBlock* preheader = outsidePreds.front();
  if (preheader->succs().size() != 1 || preheader->succs().front() != loop.header)
    return nullptr;

  core_ir::Instruction* term = preheader->terminator();
  if (!term || term->opcode() != core_ir::Opcode::Br || term->operand(0) != loop.header)
    return nullptr;
  return preheader;
}

} // namespace analysis

#define ANALYSIS_LOOP_FOREST_EMBED
#include "loop_forest.cpp"
