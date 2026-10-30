// 文件职责：计算支配关系，是 LICM、循环分析和安全重写的基础。本文件负责实现具体逻辑。
#include "dominators.h"

namespace analysis {

DomSet computeDominators(core_ir::Function* func,
                         const std::vector<core_ir::BasicBlock*>& blocks,
                         const std::unordered_set<core_ir::BasicBlock*>& reachable) {
  DomSet doms;
  if (!func || !func->entryBlock()) return doms;
  core_ir::BasicBlock* entry = func->entryBlock();

  // 使用 Cooper-Harvey-Kennedy 算法计算直接支配者（参考《A Simple, Fast
  // Dominance Algorithm》）。
  // 经典迭代式支配集数据流每轮都要为每个块复制并求交 unordered_set，
  // 在包含大量块的 CFG 上会产生较多内存流量和计算量。CHK 算法先计算直接支配者，
  // 再按原有接口构造完整支配集；可达块的结果与原迭代结果保持一致，
  // 下游分析仍按原方式工作。

  // 1）沿后继边遍历入口可达块，并生成逆后序序列。
  std::vector<core_ir::BasicBlock*> postorder;
  postorder.reserve(blocks.size());
  {
    std::unordered_set<core_ir::BasicBlock*> visited;
    // 使用显式工作表保存（基本块、下一个后继索引），执行迭代式 DFS。
    std::vector<std::pair<core_ir::BasicBlock*, size_t>> stack;
    visited.insert(entry);
    stack.push_back({entry, 0});
    while (!stack.empty()) {
      auto& top = stack.back();
      core_ir::BasicBlock* bb = top.first;
      const auto& succs = bb->succs();
      if (top.second < succs.size()) {
        core_ir::BasicBlock* s = succs[top.second++];
        if (s && visited.insert(s).second)
          stack.push_back({s, 0});
      } else {
        postorder.push_back(bb);
        stack.pop_back();
      }
    }
  }

  // postNum：编号越大表示后序位置越靠后，也就是越接近入口。
  std::unordered_map<core_ir::BasicBlock*, int> postNum;
  postNum.reserve(postorder.size() * 2);
  for (size_t i = 0; i < postorder.size(); ++i)
    postNum[postorder[i]] = static_cast<int>(i);

  // rpo 是后序序列的逆序，入口块位于最前面。
  std::vector<core_ir::BasicBlock*> rpo(postorder.rbegin(), postorder.rend());

  // 2）迭代 CHK 算法直到结果稳定；idom 用入口块作为自身的哨兵。
  std::unordered_map<core_ir::BasicBlock*, core_ir::BasicBlock*> idom;
  idom.reserve(rpo.size() * 2);
  idom[entry] = entry;

  auto intersect = [&](core_ir::BasicBlock* b1,
                       core_ir::BasicBlock* b2) -> core_ir::BasicBlock* {
    while (b1 != b2) {
      while (postNum[b1] < postNum[b2]) b1 = idom[b1];
      while (postNum[b2] < postNum[b1]) b2 = idom[b2];
    }
    return b1;
  };

  bool changed = true;
  while (changed) {
    changed = false;
    for (core_ir::BasicBlock* bb : rpo) {
      if (bb == entry) continue;
      core_ir::BasicBlock* newIdom = nullptr;
      for (core_ir::BasicBlock* pred : bb->preds()) {
        if (!pred || !reachable.count(pred)) continue;
        if (idom.find(pred) == idom.end()) continue; // 尚未处理
        newIdom = newIdom ? intersect(pred, newIdom) : pred;
      }
      if (newIdom && idom[bb] != newIdom) {
        idom[bb] = newIdom;
        changed = true;
      }
    }
  }

  // 3）构造完整支配集，保持原接口约定：入口块的集合为自身；
  // 可达块包含沿 idom 链回溯到入口的所有块；不可达块只包含自身。
  for (core_ir::BasicBlock* bb : blocks) {
    if (bb == entry) {
      doms[bb] = {entry};
      continue;
    }
    auto it = idom.find(bb);
    if (it == idom.end()) {
      // 不可从入口到达，只把自身视为支配集合成员。
      doms[bb] = {bb};
      continue;
    }
    std::unordered_set<core_ir::BasicBlock*>& set = doms[bb];
    for (core_ir::BasicBlock* cur = bb; cur; ) {
      set.insert(cur);
      if (cur == entry) break;
      auto cit = idom.find(cur);
      cur = (cit == idom.end()) ? nullptr : cit->second;
    }
  }
  return doms;
}

std::unordered_map<core_ir::BasicBlock*, core_ir::BasicBlock*> computeIDom(
    core_ir::Function* func, const std::vector<core_ir::BasicBlock*>& blocks, const DomSet& doms) {
  std::unordered_map<core_ir::BasicBlock*, core_ir::BasicBlock*> idom;
  if (!func || !func->entryBlock()) return idom;
  core_ir::BasicBlock* entry = func->entryBlock();
  idom[entry] = nullptr;

  // 一个块的严格支配者构成 entry -> ... -> idom(bb) 的链。直接支配者是
  // 距离当前块最近的严格支配者；沿支配链向入口方向，其支配集合大小逐步
  // 变化，因此可以通过集合大小选出该节点。
  for (auto* bb : blocks) {
    if (bb == entry) continue;
    core_ir::BasicBlock* chosen = nullptr;
    size_t bestSize = 0;
    for (auto* d : doms.at(bb)) {
      if (d == bb) continue;
      auto it = doms.find(d);
      size_t sz = (it == doms.end()) ? 0 : it->second.size();
      if (!chosen || sz > bestSize) {
        chosen = d;
        bestSize = sz;
      }
    }
    idom[bb] = chosen;
  }

  return idom;
}

std::unordered_map<core_ir::BasicBlock*, std::vector<core_ir::BasicBlock*>> buildDomTreeChildren(
    const std::vector<core_ir::BasicBlock*>& blocks,
    const std::unordered_map<core_ir::BasicBlock*, core_ir::BasicBlock*>& idom) {
  std::unordered_map<core_ir::BasicBlock*, std::vector<core_ir::BasicBlock*>> children;
  for (auto* bb : blocks) children[bb] = {};
  for (auto* bb : blocks) {
    auto it = idom.find(bb);
    if (it == idom.end()) continue;
    core_ir::BasicBlock* parent = it->second;
    if (parent) children[parent].push_back(bb);
  }
  return children;
}

std::unordered_map<core_ir::BasicBlock*, std::unordered_set<core_ir::BasicBlock*>> computeDomFrontier(
    const std::vector<core_ir::BasicBlock*>& blocks,
    const std::unordered_map<core_ir::BasicBlock*, core_ir::BasicBlock*>& idom,
    const std::unordered_set<core_ir::BasicBlock*>& reachable) {
  std::unordered_map<core_ir::BasicBlock*, std::unordered_set<core_ir::BasicBlock*>> df;
  for (auto* bb : blocks) df[bb] = {};

  for (auto* bb : blocks) {
    std::vector<core_ir::BasicBlock*> preds;
    for (auto* pred : bb->preds()) {
      if (pred && reachable.count(pred))
        preds.push_back(pred);
    }
    if (preds.size() < 2) continue;

    for (auto* pred : preds) {
      core_ir::BasicBlock* runner = pred;
      while (runner) {
        auto idIt = idom.find(bb);
        core_ir::BasicBlock* stop = (idIt == idom.end() ? nullptr : idIt->second);
        if (runner == stop) break;
        df[runner].insert(bb);
        auto runIdIt = idom.find(runner);
        if (runIdIt == idom.end())
          break;
        runner = runIdIt->second;
      }
    }
  }
  return df;
}

bool dominates(const DomSet& doms, core_ir::BasicBlock* a, core_ir::BasicBlock* b) {
  auto it = doms.find(b);
  return it != doms.end() && it->second.count(a);
}

} // namespace analysis
