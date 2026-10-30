// 文件职责：创建、移动和整理 CoreIR 中的 phi 节点。
#include "phi_utils.h"
#include "passes_utils.h"
#include <unordered_set>
#include <vector>
#include <unordered_map>

namespace core_ir {

std::vector<Instruction*> collectLeadingPhi(BasicBlock* bb) {
  std::vector<Instruction*> phis;
  for (auto& instPtr : bb->instructions()) {
    Instruction* inst = instPtr.get();
    if (inst->opcode() != Opcode::Phi)
      break;
    phis.push_back(inst);
  }
  return phis;
}

bool replacePhiIncomingPred(BasicBlock* bb, BasicBlock* oldPred, BasicBlock* newPred) {
  if (!bb || !oldPred || !newPred) return false;
  bool changed = false;
  for (auto* phi : collectLeadingPhi(bb)) {
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
      if (pred == oldPred) {
        phi->setOperand(i + 1, newPred);
        changed = true;
      }
    }
  }
  return changed;
}

bool replaceBlockUses(Function* func, BasicBlock* oldBB, BasicBlock* newBB) {
  if (!func || !oldBB || !newBB || oldBB == newBB)
    return false;
  bool changed = false;
  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      for (int i = 0; i < inst->numOperands(); ++i) {
        if (inst->operand(i) == oldBB) {
          inst->setOperand(i, newBB);
          changed = true;
        }
      }
    }
  }
  return changed;
}

bool removePhiIncomingPred(BasicBlock* bb, BasicBlock* deadPred) {
  if (!bb || !deadPred) return false;
  bool changed = false;
  for (auto* phi : collectLeadingPhi(bb)) {
    std::vector<Value*> newOps;
    bool phiChanged = false;
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
      if (pred == deadPred) {
        phiChanged = true;
        continue;
      }
      newOps.push_back(phi->operand(i));
      newOps.push_back(phi->operand(i + 1));
    }
    if (!phiChanged) continue;
    phi->dropAllOperands();
    for (auto* op : newOps)
      phi->addOperand(op);
    changed = true;
  }
  return changed;
}

bool repairPhiIncoming(Function* func) {
  if (!func) return false;
  func->recomputeCFG();

  bool changed = false;
  std::vector<std::pair<BasicBlock*, Instruction*>> toRemove;

  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    auto phis = collectLeadingPhi(bb);
    if (phis.empty()) continue;

    std::unordered_map<BasicBlock*, int> predCounts;
    for (auto* pred : bb->preds()) {
      if (pred) ++predCounts[pred];
    }

    for (auto* phi : phis) {
      auto predBudget = predCounts;
      std::vector<std::pair<Value*, BasicBlock*>> incoming;
      incoming.reserve(phi->numOperands() / 2);

      for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
        Value* inVal = phi->operand(i);
        auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
        auto it = pred ? predBudget.find(pred) : predBudget.end();
        if (!pred || it == predBudget.end() || it->second == 0) {
          changed = true;
          continue;
        }
        incoming.push_back({inVal, pred});
        --it->second;
      }

      if (static_cast<int>(incoming.size() * 2) != phi->numOperands()) {
        phi->dropAllOperands();
        for (auto& [inVal, pred] : incoming) {
          phi->addOperand(inVal);
          phi->addOperand(pred);
        }
      }

      if (incoming.empty()) continue;

      bool allSame = true;
      for (size_t i = 1; i < incoming.size(); ++i) {
        if (incoming[i].first != incoming[0].first) {
          allSame = false;
          break;
        }
      }

      if ((incoming.size() == 1 || allSame) && incoming[0].first != phi) {
        // 自引用 phi 不能按“唯一 incoming”折叠。否则会删除
        // 仍被用户引用的 phi，留下悬空 Value*。
        phi->replaceAllUsesWith(incoming[0].first);
        toRemove.push_back({bb, phi});
        changed = true;
      }
    }
  }

  for (auto& [bb, phi] : toRemove) {
    if (bb && phi && phi->parent() == bb)
      bb->remove(phi);
  }

  if (changed)
    func->recomputeCFG();
  return changed;
}

BasicBlock* splitCriticalEdgeForPhi(Function* func, BasicBlock* pred, BasicBlock* succ) {
  if (!func || !pred || !succ) return nullptr;
  Instruction* term = pred->terminator();
  if (!term || term->numOperands() == 0) return nullptr;
  if (pred->succs().size() <= 1) return pred;

  auto* edge = func->newBlock("phi_edge");
  auto* br = new Instruction(Opcode::Br, Type::voidTy());
  br->addOperand(succ);
  edge->pushBack(br);

  bool replaced = false;
  if (term->opcode() == Opcode::Br) {
    if (term->operand(0) == succ) {
      term->setOperand(0, edge);
      replaced = true;
    }
  } else if (term->opcode() == Opcode::CondBr) {
    if (term->operand(1) == succ) {
      term->setOperand(1, edge);
      replaced = true;
    }
    if (term->operand(2) == succ) {
      term->setOperand(2, edge);
      replaced = true;
    }
  }
  if (!replaced)
    return pred;

  replacePhiIncomingPred(succ, pred, edge);
  return edge;
}

bool normalizePhiEdges(Module& mod) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    func->recomputeCFG();

    std::vector<BasicBlock*> blocks;
    for (auto& bbPtr : func->blocks())
      blocks.push_back(bbPtr.get());

    for (auto* bb : blocks) {
      auto phis = collectLeadingPhi(bb);
      if (phis.empty()) continue;

      std::vector<BasicBlock*> oldPreds = bb->preds();
      for (auto* pred : oldPreds) {
        auto* edge = splitCriticalEdgeForPhi(func, pred, bb);
        if (edge && edge != pred)
          changed = true;
      }
    }

    func->recomputeCFG();
    changed |= repairPhiIncoming(func);
  }
  return changed;
}

} // namespace core_ir
