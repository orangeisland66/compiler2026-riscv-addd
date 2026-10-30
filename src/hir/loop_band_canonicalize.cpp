// 文件职责：规范化循环 band，使后续 interchange/blocking/recurrence 能稳定消费。本文件负责实现具体逻辑。
#include "loop_band_canonicalize.h"

#include "../analysis/analysis_manager.h"
#include "../analysis/cfg_utils.h"
#include "../analysis/dominators.h"
#include "../analysis/induction_var.h"
#include "../analysis/loop_info.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hir {

namespace {

using analysis::NaturalLoop;
using core_ir::BasicBlock;
using core_ir::Function;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Type;
using core_ir::Value;

bool createDedicatedPreheader(Function* func, const NaturalLoop& loop) {
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
    BasicBlock* existing = analysis::dedicatedPreheader(loop);
    if (existing && existing == outsidePreds.front())
      return false;
  }

  auto* preheader = func->newBlock("loop.band.preheader");
  auto* br = new Instruction(core_ir::Opcode::Br, Type::voidTy());
  br->addOperand(loop.header);
  preheader->pushBack(br);

  std::unordered_set<BasicBlock*> outsideSet(outsidePreds.begin(), outsidePreds.end());
  auto headerPhis = analysis::collectLeadingPhi(loop.header);
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
      auto* prePhi = new Instruction(core_ir::Opcode::Phi, phi->type(),
                                     phi->name().empty() ? "loop.band.pre.phi"
                                                         : phi->name() + ".band.pre");
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
    redirected |= analysis::redirectSuccessor(pred, loop.header, preheader);
  if (!redirected) {
    func->removeBlock(preheader);
    return false;
  }

  func->recomputeCFG();
  return true;
}

Instruction* firstNonPhi(BasicBlock* bb) {
  if (!bb)
    return nullptr;
  for (auto& instPtr : bb->instructions()) {
    if (instPtr->opcode() != Opcode::Phi)
      return instPtr.get();
  }
  return nullptr;
}

bool isLoadFromSlot(Instruction* inst, Instruction* slot) {
  return inst && slot && inst->opcode() == Opcode::Load &&
         inst->numOperands() == 1 && inst->operand(0) == slot;
}

bool isStoreToSlot(Instruction* inst, Instruction* slot) {
  return inst && slot && inst->opcode() == Opcode::Store &&
         inst->numOperands() == 2 && inst->operand(1) == slot;
}

bool promoteScalarSlotIV(Function* func, const NaturalLoop& loop,
                         const analysis::InductionVar& iv) {
  if (!func || !loop.header || !iv.isCanonical || !iv.storage || !iv.init)
    return false;
  auto* slot = iv.storage;
  auto* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader)
    return false;

  auto* headerTerm = loop.header->terminator();
  if (!headerTerm || headerTerm->opcode() != Opcode::CondBr ||
      headerTerm->numOperands() != 3) {
    return false;
  }
  auto* cmp = dynamic_cast<Instruction*>(headerTerm->operand(0));
  if (!cmp || cmp->opcode() != Opcode::ICmp || cmp->numOperands() != 2)
    return false;

  Instruction* cmpLoad = nullptr;
  for (int i = 0; i < 2; ++i) {
    auto* load = dynamic_cast<Instruction*>(cmp->operand(i));
    if (isLoadFromSlot(load, slot) && load->parent() == loop.header) {
      cmpLoad = load;
      break;
    }
  }
  if (!cmpLoad)
    return false;

  std::unordered_map<BasicBlock*, Instruction*> latchStores;
  for (auto* latch : loop.latches) {
    if (!latch || !loop.blocks.count(latch))
      return false;
    Instruction* foundStore = nullptr;
    bool seenStore = false;
    for (auto& instPtr : latch->instructions()) {
      auto* inst = instPtr.get();
      if (!isStoreToSlot(inst, slot))
        continue;
      if (seenStore)
        return false;
      seenStore = true;
      foundStore = inst;
    }
    if (!foundStore)
      return false;
    latchStores[latch] = foundStore;
  }

  for (auto* bb : loop.blocks) {
    if (!bb)
      continue;
    if (bb != loop.header && std::find(loop.latches.begin(), loop.latches.end(), bb) ==
                                  loop.latches.end()) {
      for (auto& instPtr : bb->instructions()) {
        if (isStoreToSlot(instPtr.get(), slot))
          return false;
      }
    }
  }

  std::vector<Instruction*> loadsToReplace;
  for (auto* bb : loop.blocks) {
    if (!bb)
      continue;
    Instruction* storeInBlock = nullptr;
    auto it = latchStores.find(bb);
    if (it != latchStores.end())
      storeInBlock = it->second;

    bool pastStore = false;
    for (auto& instPtr : bb->instructions()) {
      auto* inst = instPtr.get();
      if (inst == storeInBlock)
        pastStore = true;
      if (!isLoadFromSlot(inst, slot))
        continue;
      if (pastStore)
        return false;
      loadsToReplace.push_back(inst);
    }
  }

  if (loadsToReplace.empty())
    return false;

  auto* phi = new Instruction(Opcode::Phi, Type::i32(),
                              slot->name().empty() ? "loop.slot.iv"
                                                   : slot->name() + ".loop.iv");
  phi->addOperand(iv.init);
  phi->addOperand(preheader);
  for (auto* latch : loop.latches) {
    auto* store = latchStores[latch];
    phi->addOperand(store->operand(0));
    phi->addOperand(latch);
  }

  Instruction* insertPos = firstNonPhi(loop.header);
  if (insertPos)
    loop.header->insertBefore(insertPos, phi);
  else
    loop.header->pushFront(phi);

  for (auto* load : loadsToReplace)
    load->replaceAllUsesWith(phi);
  return true;
}

bool promoteScalarSlotIVs(Function* func, const NaturalLoop& loop) {
  auto ivs = analysis::findInductionVars(func, loop);
  bool changed = false;
  for (const auto& iv : ivs) {
    if (!iv.storage || !iv.isCanonical)
      continue;
    changed |= promoteScalarSlotIV(func, loop, iv);
  }
  return changed;
}

bool processFunction(Function* func, analysis::AnalysisManager& analysisManager) {
  if (!func || !func->entryBlock())
    return false;

  bool changed = false;
  for (int iter = 0; iter < 4; ++iter) {
    func->recomputeCFG();
    analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
    const auto& loopsRef = cache.loops();
    if (loopsRef.empty())
      break;

    std::vector<NaturalLoop> loops = loopsRef;
    std::sort(loops.begin(), loops.end(),
              [](const NaturalLoop& lhs, const NaturalLoop& rhs) {
                if (lhs.blocks.size() != rhs.blocks.size())
                  return lhs.blocks.size() > rhs.blocks.size();
                return lhs.header->id() < rhs.header->id();
              });

    bool iterChanged = false;
    for (const auto& loop : loops) {
      iterChanged |= createDedicatedPreheader(func, loop);
      iterChanged |= promoteScalarSlotIVs(func, loop);
    }
    changed |= iterChanged;
    if (iterChanged)
      analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
    if (!iterChanged)
      break;
  }
  return changed;
}

} // namespace

bool applyLoopBandCanonicalization(Module& mod) {
  analysis::AnalysisManager analysisManager(mod);
  return applyLoopBandCanonicalization(mod, analysisManager);
}

bool applyLoopBandCanonicalization(Module& mod,
                                   analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions())
    changed |= processFunction(funcPtr.get(), analysisManager);
  return changed;
}

} // namespace hir
