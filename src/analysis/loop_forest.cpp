// 文件职责：构建循环森林，描述嵌套关系、header/latch/exit 等结构。本文件负责实现具体逻辑。
#ifdef ANALYSIS_LOOP_FOREST_EMBED

#include "loop_forest.h"

#include <algorithm>
#include <functional>
#include <limits>

namespace analysis {

namespace {

bool strictlyContains(const NaturalLoop& outer, const NaturalLoop& inner) {
  if (outer.blocks.size() <= inner.blocks.size())
    return false;
  for (auto* bb : inner.blocks) {
    if (!outer.blocks.count(bb))
      return false;
  }
  return true;
}

} // namespace

const LoopTreeNode* LoopForest::node(int idx) const {
  if (idx < 0 || idx >= static_cast<int>(nodes_.size()))
    return nullptr;
  return &nodes_[idx];
}

int LoopForest::findByHeader(core_ir::BasicBlock* header) const {
  auto it = byHeader_.find(header);
  return it == byHeader_.end() ? -1 : it->second;
}

int LoopForest::loopDepthOf(core_ir::BasicBlock* bb) const {
  auto it = blockDepth_.find(bb);
  return it == blockDepth_.end() ? 0 : it->second;
}

bool LoopForest::contains(int outerIdx, int innerIdx) const {
  const LoopTreeNode* outer = node(outerIdx);
  const LoopTreeNode* inner = node(innerIdx);
  return outer && inner && outer->loop && inner->loop &&
         outer->loop->blocks.count(inner->header) != 0;
}

LoopForest buildLoopForest(const std::vector<NaturalLoop>& loops) {
  LoopForest forest;
  forest.nodes_.reserve(loops.size());
  for (const auto& loop : loops) {
    LoopTreeNode node;
    node.loop = &loop;
    node.header = loop.header;
    node.preheader = dedicatedPreheader(loop);
    for (auto* bb : loop.blocks) {
      for (auto* succ : bb->succs()) {
        if (succ && !loop.blocks.count(succ))
          node.exits.push_back(succ);
      }
    }
    std::sort(node.exits.begin(), node.exits.end(),
              std::less<core_ir::BasicBlock*>());
    node.exits.erase(std::unique(node.exits.begin(), node.exits.end()),
                     node.exits.end());
    forest.byHeader_[node.header] = static_cast<int>(forest.nodes_.size());
    forest.nodes_.push_back(std::move(node));
  }

  for (int i = 0; i < static_cast<int>(loops.size()); ++i) {
    int bestParent = -1;
    size_t bestSize = std::numeric_limits<size_t>::max();
    for (int j = 0; j < static_cast<int>(loops.size()); ++j) {
      if (i == j)
        continue;
      if (!strictlyContains(loops[j], loops[i]))
        continue;
      if (loops[j].blocks.size() < bestSize) {
        bestSize = loops[j].blocks.size();
        bestParent = j;
      }
    }
    forest.nodes_[i].parent = bestParent;
  }

  for (int i = 0; i < static_cast<int>(forest.nodes_.size()); ++i) {
    int parent = forest.nodes_[i].parent;
    if (parent >= 0)
      forest.nodes_[parent].children.push_back(i);
  }
  for (auto& node : forest.nodes_) {
    std::sort(node.children.begin(), node.children.end(),
              [&](int lhs, int rhs) {
                return forest.nodes_[lhs].loop->blocks.size() <
                       forest.nodes_[rhs].loop->blocks.size();
              });
  }

  for (int i = 0; i < static_cast<int>(forest.nodes_.size()); ++i) {
    int depth = 0;
    for (int parent = forest.nodes_[i].parent; parent >= 0;
         parent = forest.nodes_[parent].parent) {
      ++depth;
    }
    forest.nodes_[i].depth = depth;
    for (auto* bb : forest.nodes_[i].loop->blocks) {
      forest.blockDepth_[bb] =
          std::max(forest.blockDepth_[bb], depth + 1);
    }
  }

  return forest;
}

} // namespace analysis

#endif
