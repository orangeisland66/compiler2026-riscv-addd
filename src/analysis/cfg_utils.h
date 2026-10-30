// 文件职责：提供 CFG 遍历、可达性和支配相关辅助函数。本文件负责声明接口和数据结构。
#pragma once
#include "../core_ir/ir/core_ir.h"
#include <unordered_set>
#include <vector>

namespace analysis {

// 收集从入口块可达的所有基本块。
std::unordered_set<core_ir::BasicBlock*> collectReachableBlocks(core_ir::Function* func);

// 统计函数中的指令总数。
int instructionCount(core_ir::Function* func);

// 收集基本块开头连续的 PHI 指令。
std::vector<core_ir::Instruction*> collectLeadingPhi(core_ir::BasicBlock* bb);

// CFG 修改后重写 PHI 的输入前驱标签。
bool replacePhiIncomingPred(core_ir::BasicBlock* bb,
                            core_ir::BasicBlock* oldPred,
                            core_ir::BasicBlock* newPred);

// 移除不再对应 CFG 前驱的 PHI 输入边。
bool removePhiIncomingPred(core_ir::BasicBlock* bb,
                           core_ir::BasicBlock* deadPred);

// 修改终结指令的一个后继，同时保留其他边。
bool redirectSuccessor(core_ir::BasicBlock* pred,
                       core_ir::BasicBlock* oldSucc,
                       core_ir::BasicBlock* newSucc);

// 检查使用指令是否直接加载或存储给定的 alloca。
bool isDirectAllocaUse(core_ir::Instruction* user, core_ir::Instruction* alloca);

} // namespace analysis
