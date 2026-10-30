// 文件职责：整理循环的基本结构并执行基础循环变换。
#include "loop_basic.h"
#include "cse.h"
#include "passes_utils.h"
#include "phi_utils.h"
#include "pass_registry.h"
#include "../../analysis/dominators.h"
#include "../../analysis/function_effects.h"
#include "../../analysis/loop_info.h"

#include <algorithm>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace core_ir {

using analysis::buildDomTreeChildren;
using analysis::computeIDom;

struct NaturalLoop {
  BasicBlock* header = nullptr;
  std::unordered_set<BasicBlock*> blocks;
  std::vector<BasicBlock*> latches;
};

std::vector<NaturalLoop> collectNaturalLoops(
    const std::vector<BasicBlock*>& blocks, const DomSet& doms,
    const std::unordered_set<BasicBlock*>& reachable);
BasicBlock* dedicatedPreheader(const NaturalLoop& loop);
bool redirectSuccessor(BasicBlock* pred, BasicBlock* oldSucc, BasicBlock* newSucc) {
  if (!pred || !oldSucc || !newSucc)
    return false;
  Instruction* term = pred->terminator();
  if (!term)
    return false;

  bool changed = false;
  if (term->opcode() == Opcode::Br && term->numOperands() == 1 &&
      term->operand(0) == oldSucc) {
    term->setOperand(0, newSucc);
    changed = true;
  } else if (term->opcode() == Opcode::CondBr && term->numOperands() == 3) {
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

BasicBlock* cachedLoopDedicatedPreheader(const NaturalLoop& loop) {
  return dedicatedPreheader(loop);
}

BasicBlock* cachedLoopDedicatedPreheader(const analysis::NaturalLoop& loop) {
  return analysis::dedicatedPreheader(loop);
}

template <typename LoopT>
bool createDedicatedPreheader(Function* func, const LoopT& loop) {
  if (!func || !loop.header || loop.blocks.empty())
    return false;

  std::vector<BasicBlock*> outsidePreds;
  for (auto* pred : loop.header->preds()) {
    if (pred && !loop.blocks.count(pred))
      outsidePreds.push_back(pred);
  }
  if (outsidePreds.empty())
    return false;
  if (outsidePreds.size() == 1) {
    BasicBlock* existing = cachedLoopDedicatedPreheader(loop);
    if (existing && existing == outsidePreds.front())
      return false;
  }

  auto* preheader = func->newBlock("loop.preheader");
  auto* br = new Instruction(Opcode::Br, Type::voidTy());
  br->addOperand(loop.header);
  preheader->pushBack(br);

  std::unordered_set<BasicBlock*> outsideSet(outsidePreds.begin(), outsidePreds.end());
  auto headerPhis = collectLeadingPhi(loop.header);
  for (auto* phi : headerPhis) {
    std::vector<std::pair<Value*, BasicBlock*>> outsideIncoming;
    std::vector<std::pair<Value*, BasicBlock*>> insideIncoming;
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
      if (!pred)
        continue;
      if (outsideSet.count(pred))
        outsideIncoming.push_back({phi->operand(i), pred});
      else
        insideIncoming.push_back({phi->operand(i), pred});
    }
    if (outsideIncoming.empty())
      continue;

    Value* mergedOutside = outsideIncoming.front().first;
    if (outsideIncoming.size() > 1) {
      auto* prePhi = new Instruction(
          Opcode::Phi, phi->type(),
          phi->name().empty() ? "loop.preheader.phi" : phi->name() + ".pre");
      for (const auto& [incomingVal, incomingPred] : outsideIncoming) {
        prePhi->addOperand(incomingVal);
        prePhi->addOperand(incomingPred);
      }
      preheader->insertBefore(br, prePhi);
      mergedOutside = prePhi;
    }

    phi->dropAllOperands();
    phi->addOperand(mergedOutside);
    phi->addOperand(preheader);
    for (const auto& [incomingVal, incomingPred] : insideIncoming) {
      phi->addOperand(incomingVal);
      phi->addOperand(incomingPred);
    }
  }

  bool redirected = false;
  for (auto* pred : outsidePreds)
    redirected |= redirectSuccessor(pred, loop.header, preheader);
  if (!redirected) {
    func->removeBlock(preheader);
    return false;
  }

  func->recomputeCFG();
  return true;
}

bool loopCanonicalize(Function* func,
                      analysis::FunctionAnalysisCache& cache) {
  if (!func || !func->entryBlock())
    return false;

  const auto& loops = cache.loops();
  if (loops.empty())
    return false;
  // LoopSimplify 风格的 preheader 规范化会被后续 LICM、load hoist
  // 和 affine pass 共同依赖；循环发现复用 FunctionAnalysisCache，避免重复扫描。
  std::vector<analysis::NaturalLoop> sortedLoops(loops.begin(), loops.end());
  std::sort(sortedLoops.begin(), sortedLoops.end(),
            [](const auto& a, const auto& b) {
              if (a.blocks.size() != b.blocks.size())
                return a.blocks.size() < b.blocks.size();
              return a.header->id() < b.header->id();
            });

  bool changed = false;
  for (const auto& loop : sortedLoops)
    changed |= createDedicatedPreheader(func, loop);
  if (changed)
    func->recomputeCFG();
  return changed;
}

bool loopCanonicalize(Module& mod, analysis::AnalysisManager& analysisManager);

bool loopCanonicalize(Module& mod) {
  analysis::AnalysisManager analysisManager(mod);
  return loopCanonicalize(mod, analysisManager);
}

bool loopCanonicalize(Module& mod, analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& func : mod.functions()) {
    analysis::FunctionAnalysisCache& cache =
        analysisManager.function(func.get());
    bool funcChanged = loopCanonicalize(func.get(), cache);
    if (funcChanged) {
      changed = true;
      analysisManager.invalidateFunction(
          func.get(), analysis::AnalysisInvalidation::CFG);
    }
  }
  return changed;
}

bool loopLocalValueNumbering(
    Function* func,
    Module& mod,
    const analysis::FunctionEffectMap& effects,
    analysis::FunctionAnalysisCache& cache) {
  if (!func || !func->entryBlock())
    return false;

  auto blocks = reachableCachedBlocks(cache);
  if (blocks.empty())
    return false;

  const DomSet& doms = cache.dominators();
  auto idom = computeIDom(func, blocks, doms);
  auto children = buildDomTreeChildren(blocks, idom);
  auto loops = cache.loops();
  if (loops.empty())
    return false;

  std::sort(loops.begin(), loops.end(),
            [](const auto& a, const auto& b) {
              if (a.blocks.size() != b.blocks.size())
                return a.blocks.size() < b.blocks.size();
              return a.header->id() < b.header->id();
            });

  bool changed = false;
  for (const auto& loop : loops) {
    if (!loop.header || loop.blocks.size() > 128)
      continue;
    std::function<void(BasicBlock*, std::unordered_map<std::string, Value*>,
                       MemoryVersionState)> walk =
        [&](BasicBlock* bb, std::unordered_map<std::string, Value*> exprs,
            MemoryVersionState memState) {
          if (!bb || !loop.blocks.count(bb))
            return;
          if (bb != loop.header && bb->preds().size() != 1) {
            clearLoadCSEEntries(exprs);
            memState = {};
          }

          // 按序处理本块指令：Load 先尝试存储转发再做 load CSE；Call 在
          // 无副作用时参与 CSE，否则更新内存版本状态；Store 记录新的内存
          // 版本与精确存储值；其余纯运算表达式走通用 CSE。
          for (auto& instPtr : bb->instructions()) {
            Instruction* inst = instPtr.get();
            if (inst->opcode() == Opcode::Load) {
              if (Value* forwarded = forwardedExactStoredValue(inst, memState)) {
                inst->replaceAllUsesWith(forwarded);
                changed = true;
                continue;
              }
              std::string key = loadCSEKey(inst, memState);
              if (!key.empty()) {
                auto it = exprs.find(key);
                if (it != exprs.end()) {
                  inst->replaceAllUsesWith(it->second);
                  changed = true;
                } else {
                  exprs[key] = inst;
                }
              }
              continue;
            }
            if (inst->opcode() == Opcode::Call) {
              std::string key = callCSEKey(inst, mod, effects, memState);
              if (!key.empty()) {
                auto it = exprs.find(key);
                if (it != exprs.end()) {
                  inst->replaceAllUsesWith(it->second);
                  changed = true;
                } else {
                  exprs[key] = inst;
                }
                continue;
              }
              if (callPreservesTrackedMemory(inst, mod, effects))
                continue;
              recordMemoryCallEffects(memState, inst, mod, effects, inst->id());
              continue;
            }
            if (inst->opcode() == Opcode::Store && inst->numOperands() == 2) {
              recordMemoryStore(memState, classifyMemory(inst->operand(1)),
                                inst->id());
              recordExactStoredValue(memState, inst->operand(1), inst->operand(0));
              continue;
            }
            if (!isCSECandidate(inst))
              continue;
            std::string key = cseKey(inst);
            auto it = exprs.find(key);
            if (it != exprs.end()) {
              inst->replaceAllUsesWith(it->second);
              changed = true;
            } else {
              exprs[key] = inst;
            }
          }

          auto childIt = children.find(bb);
          if (childIt == children.end())
            return;
          for (auto* child : childIt->second) {
            if (loop.blocks.count(child))
              walk(child, exprs, memState);
          }
        };

    walk(loop.header, {}, {});
  }

  if (changed)
    func->recomputeCFG();
  return changed;
}

bool loopLocalValueNumbering(Module& mod,
                             analysis::AnalysisManager& analysisManager) {
  const analysis::FunctionEffectMap& effects = analysisManager.functionEffects();
  bool changed = false;
  for (auto& func : mod.functions()) {
    analysis::FunctionAnalysisCache& cache =
        analysisManager.function(func.get());
    changed |= loopLocalValueNumbering(func.get(), mod, effects, cache);
  }
  return changed;
}

bool isLoopInvariantCandidate(Instruction* inst) {
  if (!inst || inst->type().isVoid() || inst->isTerminator()) return false;
  switch (inst->opcode()) {
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Mul:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
  case Opcode::Shl:
  case Opcode::Shr:
  case Opcode::FAdd:
  case Opcode::FSub:
  case Opcode::FMul:
  case Opcode::ICmp:
  case Opcode::FCmp:
  case Opcode::SIToFP:
  case Opcode::FPToSI:
  case Opcode::Neg:
  case Opcode::FNeg:
  case Opcode::BitCast:
  case Opcode::GEP:
  case Opcode::SDiv:
  case Opcode::SRem:
    return true;
  default:
    return false;
  }
}

template <typename LoopT>
bool loopMayWriteMemory(const LoopT& loop, const MemoryAliasInfo& target,
                        const Module& mod,
                        const analysis::FunctionEffectMap& effects) {
  if (!target.valid)
    return true;
  for (auto* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      if (inst->opcode() == Opcode::Store && inst->numOperands() == 2) {
        MemoryAliasInfo written = classifyMemory(inst->operand(1));
        if (!written.valid)
          return true;
        if (memoryMayAlias(written, target))
          return true;
        continue;
      }
      if (inst->opcode() == Opcode::Call &&
          callMayWriteMemory(inst, target, mod, effects)) {
        return true;
      }
    }
  }
  return false;
}

template <typename LoopT>
bool isLoopInvariantHoistableCall(Instruction* inst, const Module& mod,
                                  const analysis::FunctionEffectMap& effects,
                                  const LoopT& loop) {
  if (!inst || inst->opcode() != Opcode::Call || inst->type().isVoid())
    return false;
  const auto* summary = analysis::lookupFunctionEffect(effects, mod, inst->name());
  if (!summary)
    return false;
  if (summary->pure || summary->stableGlobalRead)
    return true;
  if (!summary->readonly || !summary->argMemOnly)
    return false;
  for (auto* op : inst->operands()) {
    if (!op || !op->type().isPtr())
      continue;
    MemoryAliasInfo info = classifyMemory(op);
    if (!info.valid || !info.precise || loopMayWriteMemory(loop, info, mod, effects))
      return false;
  }
  return true;
}

std::vector<NaturalLoop> collectNaturalLoops(
    const std::vector<BasicBlock*>& blocks, const DomSet& doms,
    const std::unordered_set<BasicBlock*>& reachable) {
  std::vector<NaturalLoop> loops;
  std::unordered_map<BasicBlock*, int> loopByHeader;

  for (auto* latch : blocks) {
    for (auto* succ : latch->succs()) {
      if (!succ || !reachable.count(succ) || !dominates(doms, succ, latch))
        continue;

      std::unordered_set<BasicBlock*> loopBlocks;
      loopBlocks.insert(succ);
      loopBlocks.insert(latch);

      std::vector<BasicBlock*> stack;
      if (latch != succ)
        stack.push_back(latch);
      while (!stack.empty()) {
        BasicBlock* bb = stack.back();
        stack.pop_back();
        for (auto* pred : bb->preds()) {
          if (!pred || !reachable.count(pred)) continue;
          if (!loopBlocks.insert(pred).second) continue;
          if (pred != succ)
            stack.push_back(pred);
        }
      }

      auto it = loopByHeader.find(succ);
      if (it == loopByHeader.end()) {
        NaturalLoop loop;
        loop.header = succ;
        loop.blocks = std::move(loopBlocks);
        loop.latches.push_back(latch);
        loopByHeader[succ] = static_cast<int>(loops.size());
        loops.push_back(std::move(loop));
      } else {
        NaturalLoop& loop = loops[it->second];
        loop.blocks.insert(loopBlocks.begin(), loopBlocks.end());
        loop.latches.push_back(latch);
      }
    }
  }

  return loops;
}

BasicBlock* dedicatedPreheader(const NaturalLoop& loop) {
  if (!loop.header) return nullptr;
  std::vector<BasicBlock*> outsidePreds;
  for (auto* pred : loop.header->preds()) {
    if (pred && !loop.blocks.count(pred))
      outsidePreds.push_back(pred);
  }
  if (outsidePreds.size() != 1) return nullptr;

  BasicBlock* preheader = outsidePreds.front();
  if (preheader->succs().size() != 1 || preheader->succs().front() != loop.header)
    return nullptr;

  Instruction* term = preheader->terminator();
  if (!term || term->opcode() != Opcode::Br || term->operand(0) != loop.header)
    return nullptr;
  return preheader;
}

template <typename LoopT>
bool operandAvailableOutsideLoop(Value* val, const LoopT& loop,
                                 const std::unordered_set<Instruction*>& invariant) {
  if (!val) return false;
  if (val->isConstant() || dynamic_cast<Argument*>(val) ||
      dynamic_cast<GlobalVar*>(val) || dynamic_cast<BasicBlock*>(val))
    return true;
  auto* inst = dynamic_cast<Instruction*>(val);
  if (!inst || !inst->parent()) return true;
  if (!loop.blocks.count(inst->parent())) return true;
  return invariant.count(inst) > 0;
}

bool moveInstructionBefore(Instruction* inst, BasicBlock* dest, Instruction* pos) {
  if (!inst || !inst->parent() || !dest || !pos) return false;
  BasicBlock* src = inst->parent();
  if (src == dest) return false;

  auto& srcInsts = src->instructions();
  auto srcIt = std::find_if(srcInsts.begin(), srcInsts.end(),
                            [inst](const auto& p) { return p.get() == inst; });
  if (srcIt == srcInsts.end()) return false;

  std::unique_ptr<Instruction> owned = std::move(*srcIt);
  srcInsts.erase(srcIt);
  owned->setParent(dest);

  auto& dstInsts = dest->instructions();
  auto dstIt = std::find_if(dstInsts.begin(), dstInsts.end(),
                            [pos](const auto& p) { return p.get() == pos; });
  if (dstIt == dstInsts.end()) {
    dstInsts.push_back(std::move(owned));
  } else {
    dstInsts.insert(dstIt, std::move(owned));
  }
  return true;
}

BasicBlock* findDedicatedPreheaderForLoop(const NaturalLoop& loop) {
  return dedicatedPreheader(loop);
}

BasicBlock* findDedicatedPreheaderForLoop(const analysis::NaturalLoop& loop) {
  return analysis::dedicatedPreheader(loop);
}

template <typename LoopT>
bool hoistLoopInvariants(Function* func, Module& mod, const LoopT& loop,
                         const analysis::FunctionEffectMap& effects) {
  BasicBlock* preheader = findDedicatedPreheaderForLoop(loop);
  if (!preheader) return false;

  bool changed = false;
  std::unordered_set<Instruction*> invariant;
  bool localChanged = true;

  while (localChanged) {
    localChanged = false;
    std::vector<Instruction*> toHoist;

    for (auto& bbPtr : func->blocks()) {
      BasicBlock* bb = bbPtr.get();
      if (!loop.blocks.count(bb)) continue;
      for (auto& instPtr : bb->instructions()) {
        Instruction* inst = instPtr.get();
        bool isRegular = isLoopInvariantCandidate(inst);
        bool isCall = isLoopInvariantHoistableCall(inst, mod, effects, loop);
        if ((!isRegular && !isCall) || invariant.count(inst))
          continue;
        if (inst->uses().empty())
          continue;

        bool operandsReady = true;
        for (auto* op : inst->operands()) {
          if (!operandAvailableOutsideLoop(op, loop, invariant)) {
            operandsReady = false;
            break;
          }
        }
        if (!operandsReady)
          continue;

        invariant.insert(inst);
        toHoist.push_back(inst);
      }
    }

    if (toHoist.empty())
      break;

    Instruction* preheaderTerm = preheader->terminator();
    if (!preheaderTerm)
      break;

    for (auto* inst : toHoist) {
      if (!inst->parent() || !loop.blocks.count(inst->parent()))
        continue;
      if (moveInstructionBefore(inst, preheader, preheaderTerm)) {
        changed = true;
        localChanged = true;
      }
    }
  }

  return changed;
}

bool loopInvariantCodeMotion(Function* func, Module& mod,
                             const analysis::FunctionEffectMap& effects,
                             analysis::FunctionAnalysisCache& cache) {
  if (!func || !func->entryBlock()) return false;
  if (cache.instructionCount() > 12000) return false;
  if (reachableCachedBlocks(cache).empty()) return false;

  auto loops = cache.loops();
  if (loops.empty()) return false;

  std::sort(loops.begin(), loops.end(),
            [](const auto& a, const auto& b) {
              return a.blocks.size() < b.blocks.size();
            });

  bool changed = false;
  for (const auto& loop : loops)
    changed |= hoistLoopInvariants(func, mod, loop, effects);
  if (changed)
    func->recomputeCFG();
  return changed;
}

bool loopInvariantCodeMotion(Module& mod,
                             analysis::AnalysisManager& analysisManager) {
  const analysis::FunctionEffectMap& effects = analysisManager.functionEffects();
  bool changed = false;
  for (auto& func : mod.functions()) {
    analysis::FunctionAnalysisCache& cache =
        analysisManager.function(func.get());
    changed |= loopInvariantCodeMotion(func.get(), mod, effects, cache);
  }
  return changed;
}

class LoopCanonicalizePass : public Pass {
public:
    std::string name() const override { return "LoopCanonicalize"; }
    bool run(Module& mod, analysis::AnalysisManager& am) override {
        return loopCanonicalize(mod, am);
    }
};
REGISTER_PASS(LoopCanonicalizePass);

class LoopLVNPass : public Pass {
public:
    std::string name() const override { return "LoopLVN"; }
    bool run(Module& mod, analysis::AnalysisManager& am) override {
        return loopLocalValueNumbering(mod, am);
    }
};
REGISTER_PASS(LoopLVNPass);

class LICMPass : public Pass {
public:
    std::string name() const override { return "LICM"; }
    bool run(Module& mod, analysis::AnalysisManager& am) override {
        return loopInvariantCodeMotion(mod, am);
    }
};
REGISTER_PASS(LICMPass);

} // namespace core_ir
