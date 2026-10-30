#include "research_switch.h"
// 文件职责：合并迭代空间相同且访问关系兼容的相邻循环。
#include "loop_fusion.h"
#include "canonical_loop.h"

#include "../analysis/affine_access.h"
#include "../analysis/analysis_manager.h"
#include "../analysis/cfg_utils.h"
#include "../analysis/dominators.h"
#include "../analysis/function_analysis.h"
#include "../analysis/loop_forest.h"
#include "../analysis/induction_var.h"
#include "../analysis/loop_info.h"
#include "../analysis/loop_value_utils.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdio>
#include <cstdlib>

namespace hir {

namespace {

using analysis::NaturalLoop;
using core_ir::BasicBlock;
using core_ir::Function;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Value;

bool shareIterationSpace(const CanonicalLoop& a, const CanonicalLoop& b) {
  if (!a.ivIsCmpLhs || !b.ivIsCmpLhs) return false;
  auto sameValue = [](Value* lhs, Value* rhs) {
    if (lhs == rhs) return true;
    auto* lhsConst = dynamic_cast<core_ir::ConstInt*>(lhs);
    auto* rhsConst = dynamic_cast<core_ir::ConstInt*>(rhs);
    return lhsConst && rhsConst && lhsConst->value() == rhsConst->value();
  };
  if (!sameValue(a.initVal, b.initVal)) return false;
  if (!sameValue(a.boundVal, b.boundVal)) return false;
  return true;
}

bool hasFusionPreventingDependence(const NaturalLoop& loopA,
                                   const NaturalLoop& loopB) {
  auto hasGlobalStore = [](const NaturalLoop& l) {
    for (auto* bb : l.blocks)
      for (auto& p : bb->instructions())
        if (p->opcode() == Opcode::Store && p->numOperands() >= 2 &&
            !analysis::isScalarSlot(p->operand(1)))
          return true;
    return false;
  };
  auto hasGlobalLoad = [](const NaturalLoop& l) {
    for (auto* bb : l.blocks)
      for (auto& p : bb->instructions())
        if (p->opcode() == Opcode::Load && p->numOperands() >= 1 &&
            !analysis::isScalarSlot(p->operand(0)))
          return true;
    return false;
  };
  // 当一个循环写数组、另一个循环读数组时，按存在依赖处理。
  if (hasGlobalStore(loopA) && hasGlobalLoad(loopB)) return true;
  if (hasGlobalStore(loopB) && hasGlobalLoad(loopA)) return true;
  return false;
}

bool bodyIsSimple(const NaturalLoop& loop) {
  for (auto* bb : loop.blocks)
    for (auto& p : bb->instructions())
      if (p->opcode() == Opcode::Call || p->opcode() == Opcode::Alloca)
        return false;
  return true;
}

bool bodyCanBeCloned(const CanonicalLoop& loop,
                     const NaturalLoop& naturalLoop) {
  for (auto* bb : naturalLoop.blocks) {
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst || inst->isTerminator() || inst->opcode() == Opcode::Phi)
        continue;
      for (int i = 0; i < inst->numOperands(); ++i) {
        auto* operand = dynamic_cast<Instruction*>(inst->operand(i));
        if (!operand)
          continue;
        // 当前合并过程会复制循环体，只共享归纳变量。如果第二个循环还携带
        // 归约值或指针递推，删除原循环头后可能留下悬空使用，因此保留原形状。
        if (operand->parent() == loop.header && operand != loop.ivPhi)
          return false;
        if (operand->parent() == loop.latch && loop.latch != loop.body)
          return false;
      }
    }
  }
  return true;
}

bool areAdjacentLoops(const CanonicalLoop& first,
                      const CanonicalLoop& second,
                      const NaturalLoop& firstLoop,
                      const NaturalLoop& secondLoop) {
  auto* firstExitTerm = first.exit->terminator();
  if (!firstExitTerm || firstExitTerm->opcode() != Opcode::Br)
    return false;
  BasicBlock* firstExitTarget =
      dynamic_cast<BasicBlock*>(firstExitTerm->operand(0));
  if (!firstExitTarget || firstExitTarget != secondLoop.header)
    return false;
  auto* secondPH = analysis::dedicatedPreheader(secondLoop);
  return first.exit == secondPH;
}

bool fuseLoops(Function* func, CanonicalLoop& first, CanonicalLoop& second,
               NaturalLoop& firstLoop, NaturalLoop& secondLoop) {
  // 将第二个循环体复制到第一个循环体中。
  if (second.body && first.body) {
    Instruction* firstBodyTerm = first.body->terminator();
    std::unordered_map<Value*, Value*> valueMap;
    valueMap[second.ivPhi] = first.ivPhi;
    for (auto& instPtr : second.body->instructions()) {
      Instruction* inst = instPtr.get();
      if (inst->opcode() == Opcode::Br ||
          inst->opcode() == Opcode::CondBr ||
          inst->opcode() == Opcode::Phi)
        continue;
      auto* clone = new Instruction(inst->opcode(), inst->type(),
                                     inst->name());
      for (int k = 0; k < inst->numOperands(); ++k) {
        Value* op = inst->operand(k);
        auto it = valueMap.find(op);
        clone->addOperand(it != valueMap.end() ? it->second : op);
      }
      first.body->insertBefore(firstBodyTerm, clone);
      valueMap[inst] = clone;
    }
  }

  // 将第二个循环的 latch 重定向到第一个循环头。
  if (second.latch) {
    auto* lt = second.latch->terminator();
    if (lt && lt->opcode() == Opcode::Br) {
      for (int i = 0; i < lt->numOperands(); ++i)
        if (dynamic_cast<BasicBlock*>(lt->operand(i)) == second.header)
          lt->setOperand(i, first.header);
    }
  }

  // 在顺序结构中，第一个循环的出口就是第二个循环的前置块。循环体已经
  // 复制到第一个循环，因此 false 边需要跳过第二个循环头并继续到第二个出口。
  // 删除第二个循环头之前先完成这条边的重定向。
  if (first.exit) {
    auto* firstExitTerm = first.exit->terminator();
    if (firstExitTerm && firstExitTerm->opcode() == Opcode::Br) {
      for (int i = 0; i < firstExitTerm->numOperands(); ++i)
        if (dynamic_cast<BasicBlock*>(firstExitTerm->operand(i)) ==
            second.header)
          firstExitTerm->setOperand(i, second.exit);
    }
  }

  // 替换第二个循环归纳变量的使用。
  second.ivPhi->replaceAllUsesWith(first.ivPhi);

  // 删除第二个循环的基本块。
  func->recomputeCFG();
  std::vector<BasicBlock*> toRemove;
  for (auto* bb : secondLoop.blocks) {
    if (bb != first.header && bb != first.body && bb != first.latch &&
        bb != first.exit && bb != first.preheader)
      toRemove.push_back(bb);
  }
  for (auto* bb : toRemove) {
    for (auto& p : bb->instructions()) p->dropAllOperands();
    func->removeBlock(bb);
  }
  func->recomputeCFG();
  return true;
}

bool processFunction(Function* func,
                     analysis::AnalysisManager& analysisManager) {
  if (!func || func->blocks().empty()) return false;

  bool changed = false;
  for (int iter = 0; iter < 8; ++iter) {
    analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
    const auto& forest = cache.loopForest();

    int bestFirst = -1, bestSecond = -1;
    for (int i = 0; i < static_cast<int>(forest.nodes().size()); ++i) {
      const analysis::LoopTreeNode* nodeI = forest.node(i);
      if (!nodeI || !nodeI->loop) continue;
      CanonicalLoop clI;
      if (!recognizeCanonicalLoop(*nodeI->loop, nodeI->preheader, clI))
        continue;
      if (!bodyIsSimple(*nodeI->loop)) continue;

      for (int j : nodeI->children) {
        const analysis::LoopTreeNode* nodeJ = forest.node(j);
        if (!nodeJ || !nodeJ->loop) continue;
        CanonicalLoop clJ;
        if (!recognizeCanonicalLoop(*nodeJ->loop, nodeJ->preheader, clJ))
          continue;
        if (!bodyIsSimple(*nodeJ->loop)) continue;
        if (!bodyCanBeCloned(clJ, *nodeJ->loop)) continue;

        if (!areAdjacentLoops(clI, clJ, *nodeI->loop, *nodeJ->loop))
          continue;
        if (!shareIterationSpace(clI, clJ))
          continue;
        if (hasFusionPreventingDependence(*nodeI->loop, *nodeJ->loop))
          continue;

        bestFirst = i;
        bestSecond = j;
        break;
      }
      if (bestFirst >= 0) break;
    }
    if (bestFirst < 0) break;

    const auto* n1 = forest.node(bestFirst);
    const auto* n2 = forest.node(bestSecond);
    if (!n1 || !n2) break;
    CanonicalLoop cl1, cl2;
    if (!recognizeCanonicalLoop(*n1->loop, n1->preheader, cl1)) break;
    if (!recognizeCanonicalLoop(*n2->loop, n2->preheader, cl2)) break;
    const std::string firstHeaderName = cl1.header->name();
    const std::string secondHeaderName = cl2.header->name();
    NaturalLoop l1 = *n1->loop, l2 = *n2->loop;
    if (fuseLoops(func, cl1, cl2, l1, l2)) {
      if (sycEnvironmentValue("SYC_TRACE_HIR_LOOP_FUSION")) {
        std::fprintf(stderr, "[loop-fusion] function=%s first=%s second=%s\n",
                     func->name().c_str(), firstHeaderName.c_str(),
                     secondHeaderName.c_str());
      }
      changed = true;
      analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
    }
    if (!changed) break;
  }
  return changed;
}

} // namespace

bool applyLoopFusion(Module& mod) {
  analysis::AnalysisManager analysisManager(mod);
  return applyLoopFusion(mod, analysisManager);
}

bool applyLoopFusion(Module& mod, analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions())
    changed |= processFunction(funcPtr.get(), analysisManager);
  return changed;
}

} // namespace hir
