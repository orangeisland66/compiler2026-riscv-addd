// 文件职责：删除不可达控制流并合并可以简化的 CoreIR 基本块。
#include "simplify_cfg.h"
#include "phi_utils.h"
#include "passes_utils.h"
#include <unordered_set>
#include <vector>

namespace core_ir {

void markReachable(BasicBlock* bb, std::unordered_set<BasicBlock*>& seen) {
  if (!bb || seen.count(bb)) return;
  seen.insert(bb);
  auto* term = bb->terminator();
  if (!term) return;
  if (term->opcode() == Opcode::Br) {
    markReachable(dynamic_cast<BasicBlock*>(term->operand(0)), seen);
  } else if (term->opcode() == Opcode::CondBr) {
    markReachable(dynamic_cast<BasicBlock*>(term->operand(1)), seen);
    markReachable(dynamic_cast<BasicBlock*>(term->operand(2)), seen);
  }
}

bool simplifyCFG(Module& mod) {
  bool changed = false;
  for (auto& func : mod.functions()) {
    for (auto& bb : func->blocks()) {
      Instruction* term = bb->terminator();
      if (!term || term->opcode() != Opcode::CondBr) continue;
      auto* cond = dynamic_cast<ConstInt*>(term->operand(0));
      if (!cond) continue;
      auto* trueBB = dynamic_cast<BasicBlock*>(term->operand(1));
      auto* falseBB = dynamic_cast<BasicBlock*>(term->operand(2));
      auto* target = cond->value() ? trueBB : falseBB;
      auto* deadEdge = cond->value() ? falseBB : trueBB;
      if (!target) continue;
      bb->remove(term);
      auto* br = new Instruction(Opcode::Br, Type::voidTy());
      br->addOperand(target);
      bb->pushBack(br);
      if (deadEdge && deadEdge != target)
        removePhiIncomingPred(deadEdge, bb.get());
      changed = true;
    }

    func->recomputeCFG();

    std::unordered_set<BasicBlock*> reachable;
    markReachable(func->entryBlock(), reachable);
    std::vector<BasicBlock*> deadBlocks;
    for (auto& bb : func->blocks())
      if (!reachable.count(bb.get()))
        deadBlocks.push_back(bb.get());

    if (!deadBlocks.empty()) {
      std::unordered_set<BasicBlock*> deadSet(deadBlocks.begin(), deadBlocks.end());
      // 先把所有 reachable phi 上来自 dead block 的 incoming
      // 删掉，再判断 dead island 是否还能被安全物理删除。否则 block-value
      // 和 paired value 仍短暂挂在 live phi 上，会被误判成“外部逃逸”，
      // 导致 specialization 暴露出的常量分支永远留下一串不可达 case 块。
      for (auto& bb : func->blocks()) {
        if (!reachable.count(bb.get()) || deadSet.count(bb.get()))
          continue;
        for (auto* dead : deadBlocks)
          changed |= removePhiIncomingPred(bb.get(), dead);
      }

      std::unordered_set<BasicBlock*> removableSet = deadSet;
      bool closureChanged = true;
      while (closureChanged) {
        closureChanged = false;
        for (auto* dead : deadBlocks) {
          if (!removableSet.count(dead))
            continue;
          bool escapes = false;
          for (Instruction* user : dead->uses()) {
            BasicBlock* userParent = user ? user->parent() : nullptr;
            if (!userParent || !removableSet.count(userParent)) {
              escapes = true;
              break;
            }
          }
          for (auto& instPtr : dead->instructions()) {
            if (escapes)
              break;
            Instruction* inst = instPtr.get();
            for (Instruction* user : inst->uses()) {
              BasicBlock* userParent = user ? user->parent() : nullptr;
              if (!userParent || !removableSet.count(userParent)) {
                escapes = true;
                break;
              }
            }
          }
          if (escapes) {
            // 不可达块删除必须保持 SSA use-def 完整。如果某个
            // dead island 的定义仍被保留块使用，就不能物理删掉这部分块；
            // 否则后续 effect/CFG 分析会遇到悬空 Value*。
            removableSet.erase(dead);
            closureChanged = true;
          }
        }
      }
      for (auto* bb : deadBlocks) {
        if (!removableSet.count(bb))
          continue;
        for (auto& inst : bb->instructions())
          inst->dropAllOperands();
      }
      for (auto* bb : deadBlocks) {
        if (!removableSet.count(bb))
          continue;
        func->removeBlock(bb);
      }
      changed = changed || !removableSet.empty();
    }

    if (changed) func->recomputeCFG();
  }
  return changed;
}

bool hasLeadingPhi(BasicBlock* bb) {
  if (!bb || bb->instructions().empty()) return false;
  return bb->instructions().front()->opcode() == Opcode::Phi;
}

bool mergeWouldCreateExcessiveBlock(Function* func, BasicBlock* bb,
                                    BasicBlock* succ) {
  if (!func || !bb || !succ)
    return true;

  // 对很大的直线区域保留较小的 CFG 清理范围。把空的函数入口块并入巨大后继
  // 会让参数准备和整段计算共用一个后端块，扩大 RISC-V 大参数函数的活跃区间
  // 和回存数量；小型入口合并仍然保留。
  constexpr size_t kMaxMergedEntryBlockInstructions = 1024;
  constexpr size_t kMaxMergedBlockInstructions = 8192;
  const size_t bbInsts = bb->instructions().size();
  const size_t succInsts = succ->instructions().size();
  const size_t mergedInsts = bbInsts + succInsts - (bbInsts > 0 ? 1 : 0);

  if (bb == func->entryBlock() &&
      mergedInsts > kMaxMergedEntryBlockInstructions)
    return true;
  return mergedInsts > kMaxMergedBlockInstructions;
}

bool mergeLinearBlocks(Module& mod) {
  bool changed = false;
  bool localChanged = true;
  while (localChanged) {
    localChanged = false;

    for (auto& funcPtr : mod.functions()) {
      Function* func = funcPtr.get();
      func->recomputeCFG();

      std::vector<BasicBlock*> blocks;
      for (auto& bbPtr : func->blocks())
        blocks.push_back(bbPtr.get());

      for (auto* bb : blocks) {
        if (!bb || !bb->parent()) continue;
        Instruction* term = bb->terminator();
        if (!term || term->opcode() != Opcode::Br) continue;
        auto* op0 = term->operand(0);
        if (!op0) continue;
        auto* succ = dynamic_cast<BasicBlock*>(op0);
        if (!succ || succ == bb || succ == func->entryBlock()) continue;
        if (succ->preds().size() != 1 || succ->preds().front() != bb) continue;
        if (hasLeadingPhi(succ)) continue;
        if (mergeWouldCreateExcessiveBlock(func, bb, succ)) continue;
        // 如果 bb 中的 phi 通过入边引用 succ，则不合并。
        // 合并后 phi 会引用自身，形成自环并干扰后续处理。
        {
          bool bbPhiUsesSucc = false;
          for (auto& inst : bb->instructions()) {
            if (inst->opcode() != Opcode::Phi) break;
            for (int i = 0; i < inst->numOperands(); i += 2) {
              auto* incomingBB = dynamic_cast<BasicBlock*>(inst->operand(i + 1));
              if (incomingBB && incomingBB == succ) {
                bbPhiUsesSucc = true;
                break;
              }
            }
            if (bbPhiUsesSucc) break;
          }
          if (bbPhiUsesSucc) continue;
        }

        bb->remove(term);

        auto& srcInsts = succ->instructions();
        auto& dstInsts = bb->instructions();
        while (!srcInsts.empty()) {
          std::unique_ptr<Instruction> inst = std::move(srcInsts.front());
          srcInsts.erase(srcInsts.begin());
          inst->setParent(bb);
          dstInsts.push_back(std::move(inst));
        }

        for (auto* succSucc : succ->succs())
          replacePhiIncomingPred(succSucc, succ, bb);

        // 块合并后，旧 successor 不能再作为任何 operand 留在
        // 模块里。除了正常的 successor phi，历史变换也可能留下非标准
        // block operand；统一替换可避免后续删块后出现悬空指针。
        replaceBlockUses(func, succ, bb);
        func->removeBlock(succ);
        func->recomputeCFG();
        changed = true;
        localChanged = true;
        break;
      }

      if (localChanged)
        break;
    }
  }
  return changed;
}
} // namespace core_ir
