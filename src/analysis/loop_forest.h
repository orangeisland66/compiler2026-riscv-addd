// 文件职责：构建循环森林，描述嵌套关系、header/latch/exit 等结构。本文件负责声明接口和数据结构。
#pragma once

#include "loop_info.h"

#include <unordered_map>
#include <vector>

namespace analysis {

// 循环森林中的一个节点：对应一个自然循环及其父子/深度信息。
struct LoopTreeNode {
  const NaturalLoop* loop = nullptr;
  core_ir::BasicBlock* header = nullptr;
  core_ir::BasicBlock* preheader = nullptr;
  std::vector<core_ir::BasicBlock*> exits;
  int parent = -1;
  int depth = 0;
  std::vector<int> children;
};

class LoopForest {
public:
  // 返回全部循环节点。
  const std::vector<LoopTreeNode>& nodes() const { return nodes_; }
  // 按索引返回节点；越界返回 nullptr。
  const LoopTreeNode* node(int idx) const;
  // 按 header 块查找循环节点索引；未找到返回 -1。
  int findByHeader(core_ir::BasicBlock* header) const;
  // 返回基本块所在的最内层循环深度（不在循环内为 0）。
  int loopDepthOf(core_ir::BasicBlock* bb) const;
  // 判断 outerIdx 对应的循环是否包含 innerIdx 对应的循环。
  bool contains(int outerIdx, int innerIdx) const;

private:
  friend LoopForest buildLoopForest(const std::vector<NaturalLoop>& loops);

  std::vector<LoopTreeNode> nodes_;
  std::unordered_map<core_ir::BasicBlock*, int> byHeader_;
  std::unordered_map<core_ir::BasicBlock*, int> blockDepth_;
};

// 由自然循环列表构建循环森林（推断嵌套关系、header/preheader/出口块）。
LoopForest buildLoopForest(const std::vector<NaturalLoop>& loops);

} // namespace analysis
