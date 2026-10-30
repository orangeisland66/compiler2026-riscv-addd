// 文件职责：清理 CoreIR 中的冗余指令、基本块和临时内存状态。
#include "cleanup.h"

#include "../../analysis/termination.h"
#include "../../analysis/pointer_range.h"
#include "cse.h"
#include "mem2reg.h"
#include "phi_utils.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace core_ir {

bool lowerPhiToEdgeStores(Module& mod) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    func->recomputeCFG();

    std::vector<BasicBlock*> blocks;
    for (auto& bbPtr : func->blocks())
      blocks.push_back(bbPtr.get());

    for (auto* bb : blocks) {
      auto phis = collectLeadingPhi(bb);
      if (phis.empty())
        continue;

      std::vector<BasicBlock*> oldPreds = bb->preds();
      for (auto* pred : oldPreds)
        splitCriticalEdgeForPhi(func, pred, bb);
    }

    func->recomputeCFG();
    changed |= repairPhiIncoming(func);

    blocks.clear();
    for (auto& bbPtr : func->blocks())
      blocks.push_back(bbPtr.get());

    for (auto* bb : blocks) {
      auto phis = collectLeadingPhi(bb);
      if (phis.empty())
        continue;

      Instruction* firstNonPhi = nullptr;
      for (auto& instPtr : bb->instructions()) {
        if (instPtr->opcode() != Opcode::Phi) {
          firstNonPhi = instPtr.get();
          break;
        }
      }

      std::vector<Instruction*> toRemove;
      for (auto* phi : phis) {
        Type ty = phi->type();
        auto* slot = createScalarSlotAllocaInEntry(
            func, ty,
            phi->name().empty() ? "phi.slot" : (phi->name() + ".slot"));

        for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
          Value* inVal = phi->operand(i);
          auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
          if (!pred)
            continue;
          Instruction* term = pred->terminator();
          if (!term)
            continue;

          auto* st = new Instruction(Opcode::Store, Type::voidTy());
          st->addOperand(inVal);
          st->addOperand(slot);
          pred->insertBefore(term, st);
        }

        auto* ld = new Instruction(
            Opcode::Load, ty,
            phi->name().empty() ? "phi.load" : (phi->name() + ".load"));
        ld->addOperand(slot);
        if (firstNonPhi)
          bb->insertBefore(firstNonPhi, ld);
        else
          bb->pushBack(ld);

        phi->replaceAllUsesWith(ld);
        toRemove.push_back(phi);
        changed = true;
      }

      for (auto* phi : toRemove) {
        if (phi->parent() == bb)
          bb->remove(phi);
      }
    }

    if (changed)
      func->recomputeCFG();
  }
  return changed;
}

namespace {

bool isProvablyDereferenceableLoad(const Instruction* inst) {
  if (!inst || (inst->opcode() != Opcode::Load &&
                inst->opcode() != Opcode::VLoad) ||
      inst->numOperands() < 1)
    return false;
  const int bytes = inst->opcode() == Opcode::VLoad
                        ? 16
                        : (inst->type().isPtr() ? 8 : 4);
  const auto range = analysis::evaluatePointerRange(inst->operand(0), bytes);
  if (!range.proof.proved() || range.beginByte < 0)
    return false;
  if (auto* global = dynamic_cast<GlobalVar*>(range.object))
    return range.endByte <= global->sizeBytes();
  auto* object = dynamic_cast<Instruction*>(range.object);
  return object && object->opcode() == Opcode::Alloca &&
         object->memorySizeBytes() > 0 &&
         range.endByte <= object->memorySizeBytes();
}

bool isRemovableDeadInstruction(const Instruction* inst) {
  if (!inst || inst->hasSideEffects() || !inst->uses().empty())
    return false;
  // 普通 load 在合法程序中通常可执行，但若地址只在某条未证明路径上有效，
  // 删除仍可能改变 trap 行为；没有 dereferenceability 合同时保守保留。
  if (inst->opcode() == Opcode::Load || inst->opcode() == Opcode::VLoad)
    return isProvablyDereferenceableLoad(inst);
  return analysis::isProvablyNonTrappingScalarInstruction(inst);
}

bool runWorklistDCE(Module& mod) {
  bool changed = false;
  std::deque<Instruction*> worklist;
  std::unordered_set<Instruction*> queued;

  auto enqueueIfDead = [&](Instruction* inst) {
    if (!inst || !Value::isLiveValue(inst) || !inst->parent())
      return;
    if (!isRemovableDeadInstruction(inst))
      return;
    if (queued.insert(inst).second)
      worklist.push_back(inst);
  };

  for (auto& func : mod.functions()) {
    for (auto& bb : func->blocks()) {
      for (auto& instPtr : bb->instructions())
        enqueueIfDead(instPtr.get());
    }
  }

  while (!worklist.empty()) {
    Instruction* inst = worklist.front();
    worklist.pop_front();
    queued.erase(inst);

    if (!inst || !Value::isLiveValue(inst) || !inst->parent())
      continue;
    if (!isRemovableDeadInstruction(inst))
      continue;

    std::vector<Instruction*> producers;
    producers.reserve(static_cast<size_t>(inst->numOperands()));
    for (Value* operand : inst->operands()) {
      if (auto* producer = dynamic_cast<Instruction*>(operand))
        producers.push_back(producer);
    }

    BasicBlock* parent = inst->parent();
    parent->remove(inst);
    changed = true;

    for (Instruction* producer : producers)
      enqueueIfDead(producer);
  }

  return changed;
}

} // namespace

bool dce(Module& mod, analysis::AnalysisManager& analysisManager) {
  bool changed = runWorklistDCE(mod);
  if (deadPureCallElimination(mod, analysisManager)) {
    changed = true;
    changed |= runWorklistDCE(mod);
  }
  return changed;
}

bool eliminateUnreachableFunctions(Module& mod) {
  Function* entry = mod.getFunction("main");
  if (!entry || !entry->entryBlock())
    return false;

  std::unordered_map<std::string, Function*> definitions;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (func)
      definitions[func->name()] = func;
  }

  std::unordered_set<Function*> reachable;
  std::vector<Function*> worklist = {entry};
  while (!worklist.empty()) {
    Function* func = worklist.back();
    worklist.pop_back();
    if (!func || !reachable.insert(func).second)
      continue;
    for (auto& bbPtr : func->blocks()) {
      for (auto& instPtr : bbPtr->instructions()) {
        Instruction* inst = instPtr.get();
          if (!inst || inst->opcode() != Opcode::Call)
            continue;
          auto calleeIt = definitions.find(inst->name());
        if (calleeIt != definitions.end() && !reachable.count(calleeIt->second))
          worklist.push_back(calleeIt->second);
      }
    }
  }

  std::vector<Function*> dead;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (func && !reachable.count(func) && !mod.isBuiltin(func->name()))
      dead.push_back(func);
  }
  for (Function* func : dead)
    mod.removeFunction(func);
  return !dead.empty();
}

} // namespace core_ir
