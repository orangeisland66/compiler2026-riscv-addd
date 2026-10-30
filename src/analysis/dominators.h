// 文件职责：计算支配关系，是 LICM、循环分析和安全重写的基础。本文件负责声明接口和数据结构。
#pragma once
#include "../core_ir/ir/core_ir.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace analysis {

using DomSet = std::unordered_map<core_ir::BasicBlock*, std::unordered_set<core_ir::BasicBlock*>>;

// 计算所有可达基本块的支配集合。
DomSet computeDominators(core_ir::Function* func,
                         const std::vector<core_ir::BasicBlock*>& blocks,
                         const std::unordered_set<core_ir::BasicBlock*>& reachable);

// 根据支配集合计算直接支配者。
std::unordered_map<core_ir::BasicBlock*, core_ir::BasicBlock*> computeIDom(
    core_ir::Function* func, const std::vector<core_ir::BasicBlock*>& blocks, const DomSet& doms);

// 根据直接支配者映射构造支配树的子节点列表。
std::unordered_map<core_ir::BasicBlock*, std::vector<core_ir::BasicBlock*>> buildDomTreeChildren(
    const std::vector<core_ir::BasicBlock*>& blocks,
    const std::unordered_map<core_ir::BasicBlock*, core_ir::BasicBlock*>& idom);

// 计算支配边界。
std::unordered_map<core_ir::BasicBlock*, std::unordered_set<core_ir::BasicBlock*>> computeDomFrontier(
    const std::vector<core_ir::BasicBlock*>& blocks,
    const std::unordered_map<core_ir::BasicBlock*, core_ir::BasicBlock*>& idom,
    const std::unordered_set<core_ir::BasicBlock*>& reachable);

// 判断基本块 a 是否支配基本块 b。
bool dominates(const DomSet& doms, core_ir::BasicBlock* a, core_ir::BasicBlock* b);

} // namespace analysis
