// 文件职责：提升可证明稳定的 load，减少循环或热路径中的重复访存。本文件负责实现具体逻辑。
#include "load_hoist.h"

#include "../analysis/analysis_manager.h"
#include "../analysis/cfg_utils.h"
#include "../analysis/dominators.h"
#include "../analysis/function_effects.h"
#include "../analysis/loop_info.h"
#include "../analysis/loop_value_utils.h"
#include "../core_ir/passes/cse.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace rewrite {

namespace {

using analysis::NaturalLoop;
using core_ir::BasicBlock;
using core_ir::Function;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Value;

bool inLoop(BasicBlock* bb, const NaturalLoop& loop) {
  return loop.blocks.count(bb) > 0;
}

bool canMaterializeInvariant(Value* v, const NaturalLoop& loop,
                             std::unordered_map<int, bool>& memo) {
  if (!v)
    return false;
  if (v->isConstant() || dynamic_cast<core_ir::Argument*>(v) ||
      dynamic_cast<core_ir::GlobalVar*>(v) ||
      dynamic_cast<BasicBlock*>(v)) {
    return true;
  }

  auto it = memo.find(v->id());
  if (it != memo.end())
    return it->second;

  auto* inst = dynamic_cast<Instruction*>(v);
  if (!inst || !inst->parent() || !inLoop(inst->parent(), loop))
    return memo[v->id()] = true;
  if (inst->opcode() == Opcode::Load && inst->numOperands() == 1 &&
      analysis::isScalarSlot(inst->operand(0)) &&
      !analysis::loopStoresToSlot(loop, inst->operand(0))) {
    return memo[v->id()] = true;
  }
  if (!analysis::isSimpleInvariantExpr(inst, true))
    return memo[v->id()] = false;

  memo[v->id()] = false;
  for (auto* op : inst->operands()) {
    if (!canMaterializeInvariant(op, loop, memo))
      return false;
  }
  return memo[v->id()] = true;
}

Value* cloneInvariant(Value* v, BasicBlock* preheader, Instruction* pos,
                      const NaturalLoop& loop,
                      std::unordered_map<int, Value*>& cache,
                      std::unordered_map<int, bool>& memo) {
  if (!v)
    return nullptr;
  if (v->isConstant() || dynamic_cast<core_ir::Argument*>(v) ||
      dynamic_cast<core_ir::GlobalVar*>(v) ||
      dynamic_cast<BasicBlock*>(v)) {
    return v;
  }

  auto cacheIt = cache.find(v->id());
  if (cacheIt != cache.end())
    return cacheIt->second;

  auto* inst = dynamic_cast<Instruction*>(v);
  if (!inst || !inst->parent() || !inLoop(inst->parent(), loop))
    return v;
  if (!canMaterializeInvariant(v, loop, memo))
    return nullptr;

  auto* clone = new Instruction(inst->opcode(), inst->type(), inst->name());
  clone->setMemoryInfo(inst->memoryElemTy(), inst->memorySizeBytes(),
                       inst->isArrayObject(), inst->dims());
  for (auto* op : inst->operands()) {
    Value* clonedOp = cloneInvariant(op, preheader, pos, loop, cache, memo);
    if (!clonedOp) {
      delete clone;
      return nullptr;
    }
    clone->addOperand(clonedOp);
  }
  if (pos)
    preheader->insertBefore(pos, clone);
  else
    preheader->pushBack(clone);
  cache[v->id()] = clone;
  return clone;
}

bool hoistInLoop(const NaturalLoop& loop,
                 core_ir::Module& mod,
                 const analysis::FunctionEffectMap& effects) {
  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader) return false;

  // 先收集写入位置，再使用共享的 root/ModRef 分析查询每个循环不变加载。
  // 指针参数之间按 MayAlias 处理，当前栈帧中的 alloca 与参数和全局对象
  // 按不同来源区分。
  std::vector<core_ir::MemoryAliasInfo> storedLocations;
  std::vector<Instruction*> calls;
  for (auto* bb : loop.blocks) {
    for (auto& instPtr : bb->instructions()) {
      if (instPtr->opcode() == Opcode::Store) {
        if (instPtr->numOperands() < 2) continue;
        storedLocations.push_back(core_ir::classifyMemory(instPtr->operand(1)));
      } else if (instPtr->opcode() == Opcode::Call) {
        calls.push_back(instPtr.get());
      }
    }
  }

  bool changed = false;
  // 去重：每个循环中同一地址的加载只提升一次。
  std::unordered_map<Value*, Value*> hoistedByAddr;
  std::unordered_map<int, Value*> clonedInvariantValues;
  std::unordered_map<int, bool> invariantMemo;

  for (auto* bb : loop.blocks) {
    // 先收集候选，避免修改指令列表时使迭代器失效。
    std::vector<Instruction*> loads;
    for (auto& instPtr : bb->instructions()) {
      if (instPtr->opcode() == Opcode::Load) loads.push_back(instPtr.get());
    }
    for (Instruction* load : loads) {
      if (load->numOperands() == 0) continue;
      Value* addr = load->operand(0);
      if (!canMaterializeInvariant(addr, loop, invariantMemo)) continue;
      core_ir::MemoryAliasInfo location = core_ir::classifyMemory(addr);
      if (!location.valid) continue;
      bool clobbered = false;
      for (const auto& stored : storedLocations) {
        if (core_ir::memoryMayAlias(location, stored)) {
          clobbered = true;
          break;
        }
      }
      if (clobbered) continue;
      for (Instruction* call : calls) {
        if (core_ir::callMayWriteMemory(call, location, mod, effects)) {
          clobbered = true;
          break;
        }
      }
      if (clobbered) continue;

      // 该地址是否已经提升过？
      auto it = hoistedByAddr.find(addr);
      if (it != hoistedByAddr.end()) {
        load->replaceAllUsesWith(it->second);
        bb->remove(load);
        changed = true;
        continue;
      }

      // 将加载复制到循环前置块的终结指令之前。
      Instruction* term = preheader->terminator();
      Value* hoistedAddr = cloneInvariant(addr, preheader, term, loop,
                                          clonedInvariantValues, invariantMemo);
      if (!hoistedAddr)
        continue;
      auto* newLoad = new Instruction(Opcode::Load, load->type(), load->name());
      newLoad->setMemoryInfo(load->memoryElemTy(), load->memorySizeBytes(),
                             load->isArrayObject(), load->dims());
      newLoad->addOperand(hoistedAddr);
      if (term) preheader->insertBefore(term, newLoad);
      else preheader->pushBack(newLoad);

      load->replaceAllUsesWith(newLoad);
      bb->remove(load);
      hoistedByAddr[addr] = newLoad;
      changed = true;
    }
  }
  return changed;
}

struct InvariantStoreGroup {
  Value* root = nullptr;
  BasicBlock* block = nullptr;
  std::vector<Instruction*> stores;
  bool rejected = false;
};

bool availableBeforeLoop(Value* value, const NaturalLoop& loop,
                         BasicBlock* preheader,
                         const analysis::DomSet& doms,
                         std::unordered_map<int, bool>& memo,
                         std::unordered_set<int>& active) {
  if (!value || value->isConstant() ||
      dynamic_cast<core_ir::Argument*>(value) ||
      dynamic_cast<core_ir::GlobalVar*>(value) ||
      dynamic_cast<BasicBlock*>(value)) {
    return value != nullptr;
  }

  auto memoIt = memo.find(value->id());
  if (memoIt != memo.end())
    return memoIt->second;
  if (!active.insert(value->id()).second)
    return memo[value->id()] = false;

  auto* inst = dynamic_cast<Instruction*>(value);
  bool available = inst && inst->parent() &&
                   !inLoop(inst->parent(), loop) &&
                   analysis::dominates(doms, inst->parent(), preheader);
  if (available) {
    for (Value* operand : inst->operands()) {
      if (!availableBeforeLoop(operand, loop, preheader, doms, memo,
                               active)) {
        available = false;
        break;
      }
    }
  }
  active.erase(value->id());
  memo[value->id()] = available;
  return available;
}

InvariantStoreGroup* findStoreGroup(
    std::vector<InvariantStoreGroup>& groups, Value* root) {
  for (auto& group : groups) {
    if (group.root == root)
      return &group;
  }
  groups.push_back({});
  groups.back().root = root;
  return &groups.back();
}

bool instructionUsesRoot(Instruction* inst, Value* root) {
  if (!inst || !root)
    return false;
  for (Value* operand : inst->operands()) {
    if (operand && operand->type().isPtr() &&
        core_ir::memoryRoot(operand) == root)
      return true;
  }
  return false;
}

bool hoistInvariantStoreGroup(
    Function* func, const NaturalLoop& loop, BasicBlock* preheader,
    const analysis::DomSet& doms, InvariantStoreGroup& group) {
  if (!func || !preheader || group.rejected || group.stores.size() < 8 ||
      !group.block || !preheader->terminator()) {
    return false;
  }
  for (BasicBlock* latch : loop.latches) {
    if (!latch || !analysis::dominates(doms, group.block, latch))
      return false;
  }

  std::unordered_set<Instruction*> groupStores(group.stores.begin(),
                                                group.stores.end());
  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;

      if (inst->opcode() == Opcode::Load && inst->numOperands() == 1 &&
          core_ir::memoryRoot(inst->operand(0)) == group.root) {
        if (!loop.blocks.count(bb) ||
            !analysis::dominates(doms, group.block, bb))
          return false;
        if (bb == group.block) {
          for (Instruction* store : group.stores) {
            bool seenLoad = false;
            for (auto& orderPtr : bb->instructions()) {
              if (orderPtr.get() == inst)
                seenLoad = true;
              if (orderPtr.get() == store && seenLoad)
                return false;
            }
          }
        }
      } else if (inst->opcode() == Opcode::Store &&
                 inst->numOperands() == 2 &&
                 core_ir::memoryRoot(inst->operand(1)) == group.root) {
        if (loop.blocks.count(bb) && !groupStores.count(inst))
          return false;
      }

      if ((inst->opcode() == Opcode::Call ||
           inst->opcode() == Opcode::Ret) &&
          instructionUsesRoot(inst, group.root)) {
        return false;
      }
      if (inst->opcode() == Opcode::Store && inst->numOperands() >= 1 &&
          inst->operand(0)->type().isPtr() &&
          core_ir::memoryRoot(inst->operand(0)) == group.root) {
        return false;
      }
    }
  }

  Instruction* term = preheader->terminator();
  for (Instruction* store : group.stores) {
    if (!store || !store->parent())
      return false;
    auto* clone = new Instruction(Opcode::Store, core_ir::Type::voidTy(),
                                  store->name());
    clone->setMemoryInfo(store->memoryElemTy(), store->memorySizeBytes(),
                          store->isArrayObject(), store->dims());
    clone->addOperand(store->operand(0));
    clone->addOperand(store->operand(1));
    preheader->insertBefore(term, clone);
  }
  for (Instruction* store : group.stores) {
    if (store && store->parent())
      store->parent()->remove(store);
  }
  return true;
}

bool hoistStoresInLoop(Function* func, const NaturalLoop& loop,
                       analysis::FunctionAnalysisCache& cache) {
  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader)
    return false;
  const auto& doms = cache.dominators();
  std::vector<InvariantStoreGroup> groups;
  std::unordered_map<int, bool> invariantMemo;
  std::unordered_set<int> active;

  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!loop.blocks.count(bb))
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst || inst->opcode() != Opcode::Store ||
          inst->numOperands() != 2)
        continue;
      core_ir::MemoryAliasInfo info =
          core_ir::classifyMemory(inst->operand(1));
      auto* rootInst = dynamic_cast<Instruction*>(info.root);
      if (!info.valid || info.rootKind != core_ir::MemoryRootKind::Alloca ||
          !rootInst || !rootInst->isArrayObject())
        continue;

      InvariantStoreGroup* group = findStoreGroup(groups, info.root);
      if (group->block && group->block != bb)
        group->rejected = true;
      group->block = bb;
      if (!availableBeforeLoop(inst->operand(0), loop, preheader, doms,
                               invariantMemo, active) ||
          !availableBeforeLoop(inst->operand(1), loop, preheader, doms,
                               invariantMemo, active)) {
        group->rejected = true;
      }
      group->stores.push_back(inst);
    }
  }

  bool changed = false;
  for (auto& group : groups)
    changed |= hoistInvariantStoreGroup(func, loop, preheader, doms, group);
  return changed;
}

} // namespace

bool applyLoopInvariantLoadHoist(Module& mod) {
  analysis::AnalysisManager analysisManager(mod);
  return applyLoopInvariantLoadHoist(mod, analysisManager);
}

bool applyLoopInvariantLoadHoist(Module& mod,
                                 analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  const analysis::FunctionEffectMap& effects = analysisManager.functionEffects();
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (func->blocks().empty()) continue;
    analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
    auto loops = cache.loops();
    bool funcChanged = false;
    // 优先处理较小循环，等价于先处理内层循环，方便后续外层再看到
    // 已经减少的重复 load；该排序只基于 loop 形状，不基于测试集。
    std::sort(loops.begin(), loops.end(),
              [](const NaturalLoop& lhs, const NaturalLoop& rhs) {
                if (lhs.blocks.size() != rhs.blocks.size())
                  return lhs.blocks.size() < rhs.blocks.size();
                return lhs.header->id() < rhs.header->id();
              });
    for (auto& loop : loops) {
      if (hoistInLoop(loop, mod, effects)) {
        changed = true;
        funcChanged = true;
      }
    }
    if (funcChanged) {
      // 虽然 CFG 不变，但指令和 uses 已变化，丢弃派生缓存更安全。
      analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
    }
  }
  return changed;
}

bool applyLoopInvariantStoreHoist(Module& mod,
                                  analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func || func->blocks().empty())
      continue;
    auto& cache = analysisManager.function(func);
    auto loops = cache.loops();
    std::sort(loops.begin(), loops.end(),
              [](const NaturalLoop& lhs, const NaturalLoop& rhs) {
                if (lhs.blocks.size() != rhs.blocks.size())
                  return lhs.blocks.size() < rhs.blocks.size();
                return lhs.header->id() < rhs.header->id();
              });
    bool functionChanged = false;
    for (const auto& loop : loops)
      functionChanged |= hoistStoresInLoop(func, loop, cache);
    if (functionChanged) {
      changed = true;
      analysisManager.invalidateFunction(func,
                                         analysis::AnalysisInvalidation::CFG);
    }
  }
  return changed;
}

} // namespace rewrite
