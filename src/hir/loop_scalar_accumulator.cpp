// 文件职责：识别循环内标量累加器，把 load/store 形态提升为 SSA carried scalar。本文件负责实现具体逻辑。
#include "loop_scalar_accumulator.h"

#include "../analysis/analysis_manager.h"
#include "../analysis/affine_access.h"
#include "../analysis/cfg_utils.h"
#include "../analysis/dominators.h"
#include "../analysis/induction_var.h"
#include "../analysis/loop_info.h"
#include "../analysis/loop_value_utils.h"
#include "../research_switch.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hir {

namespace {

using analysis::NaturalLoop;
using core_ir::Argument;
using core_ir::BasicBlock;
using core_ir::ConstInt;
using core_ir::Function;
using core_ir::GlobalVar;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Type;
using core_ir::Value;

struct AccumulatorPlan {
  Instruction* slot = nullptr;
  BasicBlock* header = nullptr;
  BasicBlock* exit = nullptr;
  BasicBlock* latch = nullptr;
  Instruction* initStore = nullptr;
  BasicBlock* accessBlock = nullptr;
  Value* initValue = nullptr;
  Value* finalUpdateValue = nullptr;
  std::vector<Instruction*> loopLoads;
  std::vector<Instruction*> updateStores;
  std::vector<Instruction*> exitLoads;
};

struct MemoryAccumulatorPlan {
  Value* address = nullptr;
  BasicBlock* header = nullptr;
  BasicBlock* exit = nullptr;
  BasicBlock* latch = nullptr;
  BasicBlock* accessBlock = nullptr;
  Instruction* initStore = nullptr;
  Value* initValue = nullptr;
  Value* finalUpdateValue = nullptr;
  bool fullPromotion = false;
  std::vector<Instruction*> loopLoads;
  std::vector<Instruction*> updateStores;
};

Instruction* firstNonPhi(BasicBlock* bb) {
  if (!bb)
    return nullptr;
  for (auto& instPtr : bb->instructions()) {
    if (instPtr->opcode() != Opcode::Phi)
      return instPtr.get();
  }
  return nullptr;
}

bool isStoreToSlot(Instruction* inst, Instruction* slot) {
  return inst && slot && inst->opcode() == Opcode::Store &&
         inst->numOperands() == 2 && inst->operand(1) == slot;
}

bool isLoadFromSlot(Instruction* inst, Instruction* slot) {
  return inst && slot && inst->opcode() == Opcode::Load &&
         inst->numOperands() == 1 && inst->operand(0) == slot;
}

int instructionIndex(BasicBlock* bb, Instruction* inst) {
  if (!bb || !inst)
    return -1;
  int idx = 0;
  for (auto& instPtr : bb->instructions()) {
    if (instPtr.get() == inst)
      return idx;
    ++idx;
  }
  return -1;
}

bool comesBeforeInBlock(BasicBlock* bb, Instruction* first, Instruction* second) {
  int firstIdx = instructionIndex(bb, first);
  int secondIdx = instructionIndex(bb, second);
  return firstIdx >= 0 && secondIdx >= 0 && firstIdx < secondIdx;
}

bool isLoopMemoryBarrier(Instruction* inst) {
  if (!inst)
    return false;
  switch (inst->opcode()) {
  case Opcode::Call:
  case Opcode::VLoad:
  case Opcode::VStore:
    return true;
  default:
    return false;
  }
}

Value* memoryAddressOperand(Instruction* inst) {
  if (!inst)
    return nullptr;
  if (inst->opcode() == Opcode::Load && inst->numOperands() == 1)
    return inst->operand(0);
  if (inst->opcode() == Opcode::Store && inst->numOperands() == 2)
    return inst->operand(1);
  return nullptr;
}

bool addressAvailableOutsideLoop(Value* addr, const NaturalLoop& loop) {
  if (!addr)
    return false;
  auto* inst = dynamic_cast<Instruction*>(addr);
  return !inst || !inst->parent() || !loop.blocks.count(inst->parent());
}

bool sameTermSet(const std::vector<analysis::AffineTerm>& lhs,
                 const std::vector<analysis::AffineTerm>& rhs) {
  if (lhs.size() != rhs.size())
    return false;
  for (const auto& lt : lhs) {
    bool found = false;
    for (const auto& rt : rhs) {
      if (lt.basis == rt.basis && lt.coeff == rt.coeff) {
        found = true;
        break;
      }
    }
    if (!found)
      return false;
  }
  return true;
}

bool sameScaledTermSet(const std::vector<analysis::AffineScaledTerm>& lhs,
                       const std::vector<analysis::AffineScaledTerm>& rhs) {
  if (lhs.size() != rhs.size())
    return false;
  for (const auto& lt : lhs) {
    bool found = false;
    for (const auto& rt : rhs) {
      if (lt.basis == rt.basis &&
          lt.invariantScale == rt.invariantScale &&
          lt.coeff == rt.coeff) {
        found = true;
        break;
      }
    }
    if (!found)
      return false;
  }
  return true;
}

bool sameAffineExpr(const analysis::AffineExprSummary& lhs,
                    const analysis::AffineExprSummary& rhs) {
  return lhs.valid && rhs.valid && lhs.constant == rhs.constant &&
         sameTermSet(lhs.ivTerms, rhs.ivTerms) &&
         sameScaledTermSet(lhs.scaledIvTerms, rhs.scaledIvTerms) &&
         sameTermSet(lhs.invariantTerms, rhs.invariantTerms);
}

bool sameAffineAccess(const analysis::AffineAccessSummary& lhs,
                      const analysis::AffineAccessSummary& rhs) {
  return lhs.valid && rhs.valid && lhs.root == rhs.root &&
         sameAffineExpr(lhs.offset, rhs.offset);
}

Value* memoryRoot(Value* addr) {
  Value* cur = addr;
  while (auto* inst = dynamic_cast<Instruction*>(cur)) {
    if ((inst->opcode() == Opcode::GEP || inst->opcode() == Opcode::BitCast) &&
        inst->numOperands() >= 1) {
      cur = inst->operand(0);
      continue;
    }
    break;
  }
  return cur;
}

bool rootsProvablyDisjoint(Value* lhs, Value* rhs) {
  lhs = memoryRoot(lhs);
  rhs = memoryRoot(rhs);
  if (!lhs || !rhs || lhs == rhs)
    return false;

  auto* lhsAlloca = dynamic_cast<Instruction*>(lhs);
  auto* rhsAlloca = dynamic_cast<Instruction*>(rhs);
  const bool lhsIsAlloca = lhsAlloca && lhsAlloca->opcode() == Opcode::Alloca;
  const bool rhsIsAlloca = rhsAlloca && rhsAlloca->opcode() == Opcode::Alloca;
  if (lhsIsAlloca || rhsIsAlloca)
    return lhsIsAlloca && rhsIsAlloca ? lhs != rhs : true;

  auto* lhsGlobal = dynamic_cast<GlobalVar*>(lhs);
  auto* rhsGlobal = dynamic_cast<GlobalVar*>(rhs);
  if (lhsGlobal && rhsGlobal)
    return lhsGlobal != rhsGlobal;

  return false;
}

bool memoryAccessProvablyDisjoint(
    Value* addr, const analysis::AffineAccessSummary& target,
    const NaturalLoop& loop, const std::vector<analysis::InductionVar>& ivs) {
  if (!addr || !target.valid || !target.root)
    return false;
  analysis::AffineAccessSummary access =
      analysis::summarizeAffineAccess(addr, loop, ivs);
  if (access.valid && access.root)
    return rootsProvablyDisjoint(access.root, target.root);
  return rootsProvablyDisjoint(addr, target.root);
}

bool sameMemoryLocation(
    Value* addr, const analysis::AffineAccessSummary& target,
    const NaturalLoop& loop, const std::vector<analysis::InductionVar>& ivs) {
  if (!addr || !target.valid)
    return false;
  if (addr == target.root && target.offset.valid && target.offset.constant == 0 &&
      target.offset.ivTerms.empty() && target.offset.scaledIvTerms.empty() &&
      target.offset.invariantTerms.empty()) {
    return true;
  }
  analysis::AffineAccessSummary access =
      analysis::summarizeAffineAccess(addr, loop, ivs);
  return sameAffineAccess(access, target);
}

bool valueUsesTransitively(Value* value, Value* needle,
                           std::unordered_set<int>& seen) {
  if (!value || !needle)
    return false;
  if (value == needle)
    return true;
  if (value->isConstant() || dynamic_cast<Argument*>(value) ||
      dynamic_cast<GlobalVar*>(value) || dynamic_cast<BasicBlock*>(value)) {
    return false;
  }
  if (!seen.insert(value->id()).second)
    return false;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst)
    return false;
  for (auto* op : inst->operands()) {
    if (valueUsesTransitively(op, needle, seen))
      return true;
  }
  return false;
}

bool valueUsesTransitively(Value* value, Value* needle) {
  std::unordered_set<int> seen;
  return valueUsesTransitively(value, needle, seen);
}

bool isAccumulatorUpdate(Value* value, Value* loadedValue,
                         const NaturalLoop& loop) {
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !inst->parent() || !loop.blocks.count(inst->parent()))
    return false;
  if (inst->opcode() != Opcode::Add && inst->opcode() != Opcode::Sub &&
      inst->opcode() != Opcode::Mul) {
    return false;
  }
  return valueUsesTransitively(value, loadedValue);
}

bool memoryAccumulatorTraceEnabled() {
  return sycEnvironmentValue("SYC_TRACE_LOOP_MEMORY_ACCUMULATOR") != nullptr;
}

bool memoryAccumulatorEnabled() {
  return sycResearchSwitchEnabled("SYC_ENABLE_HIR_LOOP_MEMORY_ACCUMULATOR");
}

void traceMemoryAccumulatorDecision(const char* verdict, Function* func,
                                    const NaturalLoop& loop,
                                    const char* reason) {
  if (!memoryAccumulatorTraceEnabled())
    return;
  std::fprintf(stderr, "[loop-memory-acc] %s func=%s header=%s reason=%s\n",
               verdict, func ? func->name().c_str() : "<null>",
               loop.header ? loop.header->name().c_str() : "<null>",
               reason ? reason : "-");
}

std::vector<BasicBlock*> orderedLoopBlocks(Function* func,
                                           const NaturalLoop& loop) {
  std::vector<BasicBlock*> ordered;
  if (!func)
    return ordered;
  ordered.reserve(loop.blocks.size());
  for (const auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (bb && loop.blocks.count(bb))
      ordered.push_back(bb);
  }
  return ordered;
}

bool collectLoopSlotAccesses(Function* func, const NaturalLoop& loop,
                             Instruction* slot,
                             std::vector<Instruction*>& loopLoads,
                             std::vector<Instruction*>& updateStores,
                             BasicBlock*& accessBlock) {
  accessBlock = nullptr;
  for (auto* bb : orderedLoopBlocks(func, loop)) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      auto* inst = instPtr.get();
      if (!isLoadFromSlot(inst, slot) && !isStoreToSlot(inst, slot))
        continue;
      if (accessBlock && accessBlock != bb)
        return false;
      accessBlock = bb;
      if (isLoadFromSlot(inst, slot)) {
        loopLoads.push_back(inst);
        continue;
      }
      updateStores.push_back(inst);
    }
  }
  if (loopLoads.empty() || updateStores.empty() ||
      loopLoads.size() != updateStores.size() || !accessBlock) {
    return false;
  }

  // unroll 之后，同一个标量槽位常会在同一热块里展开成
  // load-store-load-store 的线性链。这里先保守支持“单块、严格交替”的
  // 形状，让 sum/temp/total 这类 reduction 不会因为展开就重新掉回栈槽。
  size_t loadIndex = 0;
  size_t storeIndex = 0;
  bool expectLoad = true;
  for (auto& instPtr : accessBlock->instructions()) {
    auto* inst = instPtr.get();
    bool isLoad = loadIndex < loopLoads.size() && loopLoads[loadIndex] == inst;
    bool isStore =
        storeIndex < updateStores.size() && updateStores[storeIndex] == inst;
    if (!isLoad && !isStore)
      continue;
    if (expectLoad != isLoad)
      return false;
    if (isLoad) {
      ++loadIndex;
      expectLoad = false;
    } else {
      ++storeIndex;
      expectLoad = true;
    }
  }
  return loadIndex == loopLoads.size() && storeIndex == updateStores.size() &&
         expectLoad;
}

bool collectAccumulatorPlan(Function* func, const NaturalLoop& loop,
                            const analysis::DomSet& doms,
                            Instruction* slot, AccumulatorPlan& out) {
  if (!func || !slot || !loop.header || loop.latches.size() != 1)
    return false;
  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader || !preheader->terminator())
    return false;

  Instruction* initStore = nullptr;
  for (auto& instPtr : preheader->instructions()) {
    auto* inst = instPtr.get();
    if (isStoreToSlot(inst, slot)) {
      if (initStore)
        return false;
      initStore = inst;
    }
  }
  if (!initStore)
    return false;

  BasicBlock* latch = loop.latches.front();
  Instruction* headerTerm = loop.header->terminator();
  if (!headerTerm || headerTerm->opcode() != Opcode::CondBr ||
      headerTerm->numOperands() != 3) {
    return false;
  }
  auto* trueBB = dynamic_cast<BasicBlock*>(headerTerm->operand(1));
  auto* falseBB = dynamic_cast<BasicBlock*>(headerTerm->operand(2));
  if (!trueBB || !falseBB)
    return false;
  BasicBlock* body = loop.blocks.count(trueBB) ? trueBB : falseBB;
  BasicBlock* exit = loop.blocks.count(trueBB) ? falseBB : trueBB;
  if (!body || loop.blocks.count(exit))
    return false;

  std::vector<Instruction*> loopLoads;
  std::vector<Instruction*> updateStores;
  BasicBlock* accessBlock = nullptr;
  if (!collectLoopSlotAccesses(func, loop, slot, loopLoads, updateStores,
                               accessBlock)) {
    return false;
  }
  if (!accessBlock)
    return false;
  if (!analysis::dominates(doms, accessBlock, latch))
    return false;

  std::vector<Instruction*> exitLoads;
  std::vector<Instruction*> outsideStores;
  for (auto* user : slot->uses()) {
    if (!user || !user->parent())
      continue;
    if (user == initStore ||
        std::find(loopLoads.begin(), loopLoads.end(), user) != loopLoads.end() ||
        std::find(updateStores.begin(), updateStores.end(), user) !=
            updateStores.end()) {
      continue;
    }
    BasicBlock* userBB = user->parent();
    if (loop.blocks.count(userBB))
      return false;
    if (isLoadFromSlot(user, slot)) {
      if (!analysis::dominates(doms, exit, userBB))
        return false;
      exitLoads.push_back(user);
      continue;
    }
    if (isStoreToSlot(user, slot)) {
      outsideStores.push_back(user);
      continue;
    }
    return false;
  }
  if (exitLoads.empty())
    return false;

  // 嵌套循环里的累加器常常会在当前 inner loop 退出后先读取，
  // 再在外层下一轮重置为 0。旧实现把这些“后续重置 store”也当成非法用户，
  // 导致很多真实的 reduction 累加器吃不到 SSA 提升。这里允许循环外的
  // 直接 store 存在，但要求它不能在任何 exit load 之前拦截该值。
  for (auto* outsideStore : outsideStores) {
    BasicBlock* storeBB = outsideStore->parent();
    if (!storeBB)
      return false;
    for (auto* exitLoad : exitLoads) {
      BasicBlock* loadBB = exitLoad ? exitLoad->parent() : nullptr;
      if (!loadBB)
        return false;
      if (storeBB == loadBB) {
        if (!comesBeforeInBlock(loadBB, exitLoad, outsideStore))
          return false;
        continue;
      }
      if (analysis::dominates(doms, storeBB, loadBB))
        return false;
    }
  }

  out.slot = slot;
  out.header = loop.header;
  out.exit = exit;
  out.latch = latch;
  out.initStore = initStore;
  out.accessBlock = accessBlock;
  out.initValue = initStore->operand(0);
  out.finalUpdateValue = updateStores.back()->operand(0);
  out.loopLoads = std::move(loopLoads);
  out.updateStores = std::move(updateStores);
  out.exitLoads = std::move(exitLoads);
  return true;
}

bool collectLoopMemoryAccesses(
    Function* func, const NaturalLoop& loop,
    const analysis::AffineAccessSummary& target,
    const std::vector<analysis::InductionVar>& ivs,
    std::vector<Instruction*>& loopLoads,
    std::vector<Instruction*>& updateStores,
    BasicBlock*& accessBlock) {
  accessBlock = nullptr;
  for (auto* bb : orderedLoopBlocks(func, loop)) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      auto* inst = instPtr.get();
      if (isLoopMemoryBarrier(inst))
        return false;

      Value* addr = memoryAddressOperand(inst);
      if (!addr)
        continue;
      if (!sameMemoryLocation(addr, target, loop, ivs)) {
        if (memoryAccessProvablyDisjoint(addr, target, loop, ivs))
          continue;
        return false;
      }

      if (accessBlock && accessBlock != bb)
        return false;
      accessBlock = bb;
      if (inst->opcode() == Opcode::Load)
        loopLoads.push_back(inst);
      else
        updateStores.push_back(inst);
    }
  }

  if (loopLoads.size() != 1 || updateStores.size() != 1 || !accessBlock)
    return false;
  return comesBeforeInBlock(accessBlock, loopLoads.front(), updateStores.front());
}

bool findPreheaderInitStore(
    BasicBlock* preheader, const NaturalLoop& loop,
    const analysis::AffineAccessSummary& target,
    const std::vector<analysis::InductionVar>& ivs,
    Instruction*& initStore) {
  initStore = nullptr;
  if (!preheader)
    return false;

  for (auto& instPtr : preheader->instructions()) {
    auto* inst = instPtr.get();
    if (isLoopMemoryBarrier(inst))
      return false;

    Value* addr = memoryAddressOperand(inst);
    if (!addr)
      continue;
    if (!sameMemoryLocation(addr, target, loop, ivs))
      continue;
    if (inst->opcode() == Opcode::Load)
      return false;
    if (initStore)
      return false;
    initStore = inst;
  }
  return true;
}

bool collectMemoryAccumulatorPlan(Function* func, const NaturalLoop& loop,
                                  const analysis::DomSet& doms,
                                  MemoryAccumulatorPlan& out) {
  if (!func || !loop.header || loop.latches.size() != 1)
    return false;
  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader || !preheader->terminator())
    return false;

  BasicBlock* latch = loop.latches.front();
  Instruction* headerTerm = loop.header->terminator();
  if (!headerTerm || headerTerm->opcode() != Opcode::CondBr ||
      headerTerm->numOperands() != 3) {
    return false;
  }
  auto* trueBB = dynamic_cast<BasicBlock*>(headerTerm->operand(1));
  auto* falseBB = dynamic_cast<BasicBlock*>(headerTerm->operand(2));
  if (!trueBB || !falseBB)
    return false;
  BasicBlock* exit = loop.blocks.count(trueBB) ? falseBB : trueBB;
  if (!exit || loop.blocks.count(exit))
    return false;

  std::vector<analysis::InductionVar> ivs =
      analysis::findInductionVars(func, loop);
  for (auto* bb : orderedLoopBlocks(func, loop)) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      auto* store = instPtr.get();
      if (!store || store->opcode() != Opcode::Store ||
          store->numOperands() != 2) {
        continue;
      }

      Value* addr = store->operand(1);
      if (analysis::isScalarSlot(addr)) {
        traceMemoryAccumulatorDecision("skip", func, loop, "scalar-slot");
        continue;
      }
      const bool fullPromotion = addressAvailableOutsideLoop(addr, loop);

      analysis::AffineAccessSummary target =
          analysis::summarizeAffineAccess(addr, loop, ivs);
      if (!target.valid || !target.offset.valid ||
          !analysis::isLoopInvariantAffine(target.offset)) {
        traceMemoryAccumulatorDecision("skip", func, loop, "non-invariant-affine");
        continue;
      }

      std::vector<Instruction*> loopLoads;
      std::vector<Instruction*> updateStores;
      BasicBlock* accessBlock = nullptr;
      if (!collectLoopMemoryAccesses(func, loop, target, ivs, loopLoads,
                                     updateStores, accessBlock)) {
        traceMemoryAccumulatorDecision("skip", func, loop, "loop-access-shape");
        continue;
      }
      if (!analysis::dominates(doms, accessBlock, latch)) {
        traceMemoryAccumulatorDecision("skip", func, loop, "access-not-dominating-latch");
        continue;
      }
      Instruction* load = loopLoads.front();
      Instruction* updateStore = updateStores.front();
      if (!load || !updateStore || load->type() != updateStore->operand(0)->type()) {
        traceMemoryAccumulatorDecision("skip", func, loop, "type-mismatch");
        continue;
      }
      if (!isAccumulatorUpdate(updateStore->operand(0), load, loop)) {
        traceMemoryAccumulatorDecision("skip", func, loop, "not-accumulator-update");
        continue;
      }

      Instruction* initStore = nullptr;
      if (!findPreheaderInitStore(preheader, loop, target, ivs, initStore)) {
        traceMemoryAccumulatorDecision("skip", func, loop, "bad-preheader-init");
        continue;
      }
      if (!fullPromotion && !initStore) {
        traceMemoryAccumulatorDecision("skip", func, loop, "loop-address-without-init");
        continue;
      }

      out.address = addr;
      out.header = loop.header;
      out.exit = exit;
      out.latch = latch;
      out.accessBlock = accessBlock;
      out.initStore = initStore;
      out.initValue = initStore ? initStore->operand(0) : nullptr;
      out.finalUpdateValue = updateStore->operand(0);
      out.fullPromotion = fullPromotion;
      out.loopLoads = std::move(loopLoads);
      out.updateStores = std::move(updateStores);
      traceMemoryAccumulatorDecision("apply", func, loop, "accepted");
      return true;
    }
  }

  return false;
}

bool applyPlan(Function* func, const NaturalLoop& loop, const AccumulatorPlan& plan) {
  if (!func || !plan.slot || !plan.header || !plan.latch ||
      !plan.initValue || !plan.finalUpdateValue || plan.loopLoads.empty() ||
      plan.updateStores.empty())
    return false;

  auto* phi = new Instruction(Opcode::Phi, plan.initValue->type(),
                              plan.slot->name().empty()
                                  ? "loop.acc"
                                  : plan.slot->name() + ".loop.acc");
  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader)
    return false;
  phi->addOperand(plan.initValue);
  phi->addOperand(preheader);
  phi->addOperand(plan.finalUpdateValue);
  phi->addOperand(plan.latch);

  Instruction* insertPos = firstNonPhi(plan.header);
  if (insertPos)
    plan.header->insertBefore(insertPos, phi);
  else
    plan.header->pushFront(phi);

  Value* currentValue = phi;
  for (size_t i = 0; i < plan.loopLoads.size(); ++i) {
    plan.loopLoads[i]->replaceAllUsesWith(currentValue);
    currentValue = plan.updateStores[i]->operand(0);
  }
  for (auto* exitLoad : plan.exitLoads)
    exitLoad->replaceAllUsesWith(currentValue);

  for (auto* loopLoad : plan.loopLoads) {
    if (loopLoad && loopLoad->parent())
      loopLoad->parent()->remove(loopLoad);
  }
  for (auto* updateStore : plan.updateStores) {
    if (updateStore && updateStore->parent())
      updateStore->parent()->remove(updateStore);
  }
  if (plan.initStore->parent())
    plan.initStore->parent()->remove(plan.initStore);
  for (auto* exitLoad : plan.exitLoads) {
    if (exitLoad->parent())
      exitLoad->parent()->remove(exitLoad);
  }
  if (plan.slot->uses().empty() && plan.slot->parent()) {
    plan.slot->parent()->parent()->removeAlloca(plan.slot);
    plan.slot->parent()->remove(plan.slot);
  }
  return true;
}

bool applyMemoryPlan(Function* func, const NaturalLoop& loop,
                     const MemoryAccumulatorPlan& plan) {
  if (!func || !plan.address || !plan.header || !plan.exit || !plan.latch ||
      !plan.finalUpdateValue || plan.loopLoads.empty() ||
      plan.updateStores.empty()) {
    return false;
  }

  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader || !preheader->terminator())
    return false;

  Value* initValue = plan.initValue;
  if (!initValue) {
    if (!plan.fullPromotion)
      return false;
    auto* initLoad = new Instruction(Opcode::Load, plan.loopLoads.front()->type(),
                                     "loop.mem.acc.init");
    initLoad->addOperand(plan.address);
    preheader->insertBefore(preheader->terminator(), initLoad);
    initValue = initLoad;
  }

  auto* phi = new Instruction(Opcode::Phi, initValue->type(), "loop.mem.acc");
  phi->addOperand(initValue);
  phi->addOperand(preheader);
  phi->addOperand(plan.finalUpdateValue);
  phi->addOperand(plan.latch);

  Instruction* headerInsert = firstNonPhi(plan.header);
  if (headerInsert)
    plan.header->insertBefore(headerInsert, phi);
  else
    plan.header->pushFront(phi);

  for (auto* loopLoad : plan.loopLoads)
    loopLoad->replaceAllUsesWith(phi);

  if (plan.fullPromotion) {
    auto* exitStore = new Instruction(Opcode::Store, Type::voidTy());
    exitStore->addOperand(phi);
    exitStore->addOperand(plan.address);
    Instruction* exitInsert = firstNonPhi(plan.exit);
    if (exitInsert)
      plan.exit->insertBefore(exitInsert, exitStore);
    else
      plan.exit->pushBack(exitStore);
  }

  for (auto* loopLoad : plan.loopLoads) {
    if (loopLoad && loopLoad->parent())
      loopLoad->parent()->remove(loopLoad);
  }
  if (plan.fullPromotion) {
    for (auto* updateStore : plan.updateStores) {
      if (updateStore && updateStore->parent())
        updateStore->parent()->remove(updateStore);
    }
    if (plan.initStore && plan.initStore->parent())
      plan.initStore->parent()->remove(plan.initStore);
  }

  return true;
}

bool processFunction(Function* func, analysis::AnalysisManager& analysisManager,
                     bool enableScalarSlots,
                     bool enableMemoryAccumulators) {
  if (!func || !func->entryBlock())
    return false;
  bool changed = false;
  while (true) {
    func->recomputeCFG();

    analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
    const auto& blocks = cache.blocks();
    if (blocks.empty())
      return changed;

    const auto& doms = cache.dominators();
    auto loops = cache.loops();
    std::sort(loops.begin(), loops.end(),
              [](const NaturalLoop& lhs, const NaturalLoop& rhs) {
                if (lhs.blocks.size() != rhs.blocks.size())
                  return lhs.blocks.size() < rhs.blocks.size();
                return lhs.header->id() < rhs.header->id();
              });

    bool localChanged = false;
    for (const auto& loop : loops) {
      if (enableScalarSlots) {
      std::vector<Instruction*> slots;
      for (auto& bbPtr : func->blocks()) {
        for (auto& instPtr : bbPtr->instructions()) {
          auto* inst = instPtr.get();
          if (analysis::isScalarSlot(inst))
            slots.push_back(inst);
        }
      }
      for (auto* slot : slots) {
        if (!slot || !slot->parent())
          continue;
        AccumulatorPlan plan;
        if (!collectAccumulatorPlan(func, loop, doms, slot, plan))
          continue;
        if (!applyPlan(func, loop, plan))
          continue;
        localChanged = true;
        changed = true;
        if (sycEnvironmentValue("SYC_TRACE_LOOP_SCALAR_ACCUMULATOR"))
          std::fprintf(stderr, "[loop-scalar-acc] apply func=%s header=%s\n",
                       func->name().c_str(),
                       loop.header ? loop.header->name().c_str() : "<null>");
        // 单个函数里往往会串联出现多个 sum/temp/total 栈槽。
        // 这里每命中一个就立刻失效并重新分析，继续吃下一批，避免
        // dense-affine 热函数只提升到第一个累加器就提前退出。
        analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
        break;
      }
      }
      if (!localChanged && enableMemoryAccumulators) {
        MemoryAccumulatorPlan memoryPlan;
        if (collectMemoryAccumulatorPlan(func, loop, doms, memoryPlan) &&
            applyMemoryPlan(func, loop, memoryPlan)) {
          localChanged = true;
          changed = true;
          analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
        }
      }
      if (localChanged)
        break;
    }
    if (!localChanged)
      break;
  }
  return changed;
}

} // namespace

bool applyLoopScalarAccumulatorPromotion(Module& mod) {
  analysis::AnalysisManager analysisManager(mod);
  return applyLoopScalarAccumulatorPromotion(mod, analysisManager);
}

bool applyLoopScalarAccumulatorPromotion(Module& mod,
                                         analysis::AnalysisManager& analysisManager) {
  const bool enableMemoryAccumulators = memoryAccumulatorEnabled();
  bool changed = false;
  for (auto& funcPtr : mod.functions())
    changed |= processFunction(funcPtr.get(), analysisManager,
                               /*enableScalarSlots=*/true,
                               enableMemoryAccumulators);
  return changed;
}

bool applyLoopMemoryAccumulatorPromotion(Module& mod,
                                         analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions())
    changed |= processFunction(funcPtr.get(), analysisManager,
                               /*enableScalarSlots=*/false,
                               /*enableMemoryAccumulators=*/true);
  return changed;
}

} // namespace hir
