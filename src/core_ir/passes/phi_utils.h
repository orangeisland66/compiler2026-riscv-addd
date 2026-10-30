// 文件职责：声明 CoreIR phi 节点处理辅助接口。
#pragma once
#include "../ir/core_ir.h"
#include <vector>

namespace core_ir {

/// Phi 辅助函数
std::vector<Instruction*> collectLeadingPhi(BasicBlock* bb);
bool replacePhiIncomingPred(BasicBlock* bb, BasicBlock* oldPred, BasicBlock* newPred);
bool replaceBlockUses(Function* func, BasicBlock* oldBB, BasicBlock* newBB);
bool removePhiIncomingPred(BasicBlock* bb, BasicBlock* deadPred);
bool repairPhiIncoming(Function* func);
BasicBlock* splitCriticalEdgeForPhi(Function* func, BasicBlock* pred, BasicBlock* succ);
bool normalizePhiEdges(Module& mod);

} // namespace core_ir
