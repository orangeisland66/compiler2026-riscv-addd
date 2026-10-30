// 文件职责：提供 CFG 遍历、可达性和支配相关辅助函数。本文件负责实现具体逻辑。
#include "cfg_utils.h"

namespace analysis {

std::unordered_set<core_ir::BasicBlock*> collectReachableBlocks(core_ir::Function* func) {
  std::unordered_set<core_ir::BasicBlock*> reachable;
  if (!func || !func->entryBlock()) return reachable;
  std::vector<core_ir::BasicBlock*> stack{func->entryBlock()};
  while (!stack.empty()) {
    core_ir::BasicBlock* bb = stack.back();
    stack.pop_back();
    if (!bb || reachable.count(bb)) continue;
    reachable.insert(bb);
    for (auto* succ : bb->succs())
      if (succ && !reachable.count(succ))
        stack.push_back(succ);
  }
  return reachable;
}

int instructionCount(core_ir::Function* func) {
  if (!func) return 0;
  int count = 0;
  for (auto& bb : func->blocks())
    count += static_cast<int>(bb->instructions().size());
  return count;
}

std::vector<core_ir::Instruction*> collectLeadingPhi(core_ir::BasicBlock* bb) {
  std::vector<core_ir::Instruction*> phis;
  if (!bb)
    return phis;
  for (auto& instPtr : bb->instructions()) {
    auto* inst = instPtr.get();
    if (!inst || inst->opcode() != core_ir::Opcode::Phi)
      break;
    phis.push_back(inst);
  }
  return phis;
}

bool replacePhiIncomingPred(core_ir::BasicBlock* bb,
                            core_ir::BasicBlock* oldPred,
                            core_ir::BasicBlock* newPred) {
  if (!bb || !oldPred || !newPred)
    return false;
  bool changed = false;
  for (auto* phi : collectLeadingPhi(bb)) {
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<core_ir::BasicBlock*>(phi->operand(i + 1));
      if (pred != oldPred)
        continue;
      phi->setOperand(i + 1, newPred);
      changed = true;
    }
  }
  return changed;
}

bool removePhiIncomingPred(core_ir::BasicBlock* bb,
                           core_ir::BasicBlock* deadPred) {
  if (!bb || !deadPred)
    return false;
  bool changed = false;
  for (auto* phi : collectLeadingPhi(bb)) {
    std::vector<core_ir::Value*> kept;
    bool phiChanged = false;
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<core_ir::BasicBlock*>(phi->operand(i + 1));
      if (pred == deadPred) {
        phiChanged = true;
        continue;
      }
      kept.push_back(phi->operand(i));
      kept.push_back(phi->operand(i + 1));
    }
    if (!phiChanged)
      continue;
    phi->dropAllOperands();
    for (auto* op : kept)
      phi->addOperand(op);
    changed = true;
  }
  return changed;
}

bool redirectSuccessor(core_ir::BasicBlock* pred,
                       core_ir::BasicBlock* oldSucc,
                       core_ir::BasicBlock* newSucc) {
  if (!pred || !oldSucc || !newSucc)
    return false;
  auto* term = pred->terminator();
  if (!term)
    return false;

  bool changed = false;
  if (term->opcode() == core_ir::Opcode::Br && term->numOperands() == 1 &&
      term->operand(0) == oldSucc) {
    term->setOperand(0, newSucc);
    changed = true;
  } else if (term->opcode() == core_ir::Opcode::CondBr &&
             term->numOperands() == 3) {
    if (term->operand(1) == oldSucc) {
      term->setOperand(1, newSucc);
      changed = true;
    }
    if (term->operand(2) == oldSucc) {
      term->setOperand(2, newSucc);
      changed = true;
    }
  }
  return changed;
}

bool isDirectAllocaUse(core_ir::Instruction* user, core_ir::Instruction* alloca) {
  if (!user || !alloca) return false;
  if (user->opcode() == core_ir::Opcode::Load)
    return user->numOperands() == 1 && user->operand(0) == alloca;
  if (user->opcode() == core_ir::Opcode::Store)
    return user->numOperands() == 2 && user->operand(1) == alloca;
  return false;
}

} // namespace analysis
