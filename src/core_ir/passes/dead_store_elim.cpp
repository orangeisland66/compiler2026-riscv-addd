// DSE：消除无效存储。
// 消除后续又被覆盖且中间未被读取的存储指令。
//
// 借鉴自2025大赛优秀作品：
// - 电子科技大学 0x676e616c63: DSE(4处) 消除冗余存储
// - 南开大学 LinguaBreak: DSE(4处) 存储转发

#include "dead_store_elim.h"
#include "pass_registry.h"
#include "../../analysis/memory_ssa_lite.h"

#include <unordered_set>
#include <vector>

namespace core_ir {

static bool eliminateNeverLoadedAllocas(Module& mod);

bool eliminateDeadFunctions(Module& mod);

namespace {

bool isAddressProjection(Instruction* inst) {
  return inst &&
         (inst->opcode() == Opcode::GEP || inst->opcode() == Opcode::BitCast);
}

bool isMemoryLoad(Instruction* inst) {
  return inst &&
         (inst->opcode() == Opcode::Load || inst->opcode() == Opcode::VLoad);
}

bool isMemoryStore(Instruction* inst) {
  return inst &&
         (inst->opcode() == Opcode::Store || inst->opcode() == Opcode::VStore);
}

Value* addressRoot(Value* addr) {
  Value* root = addr;
  while (root && Value::isLiveValue(root)) {
    auto* inst = dynamic_cast<Instruction*>(root);
    if (!isAddressProjection(inst) || inst->numOperands() < 1)
      break;
    root = inst->operand(0);
  }
  return root && Value::isLiveValue(root) ? root : nullptr;
}

struct DeadAllocaPlan {
  std::vector<Instruction*> stores;
  std::vector<Instruction*> projections;
};

bool collectDeadAllocaPlan(Instruction* alloca, DeadAllocaPlan& plan) {
  if (!alloca || !Value::isLiveValue(alloca) ||
      alloca->opcode() != Opcode::Alloca)
    return false;

  std::vector<Instruction*> worklist = alloca->uses();
  std::unordered_set<Instruction*> visited;

  while (!worklist.empty()) {
    Instruction* user = worklist.back();
    worklist.pop_back();
    if (!user || !Value::isLiveValue(user) || !user->parent())
      return false;
    if (!visited.insert(user).second)
      continue;

    if (isAddressProjection(user)) {
      plan.projections.push_back(user);
      for (Instruction* next : user->uses())
        worklist.push_back(next);
      continue;
    }

    if (isMemoryLoad(user))
      return false;

    if (isMemoryStore(user)) {
      if (user->numOperands() < 2 || addressRoot(user->operand(1)) != alloca)
        return false;
      for (int i = 0; i < user->numOperands(); ++i) {
        if (i == 1)
          continue;
        if (user->operand(i) == alloca ||
            addressRoot(user->operand(i)) == alloca)
          return false;
      }
      plan.stores.push_back(user);
      continue;
    }

    return false;
  }

  return true;
}

bool removeIfPresent(Instruction* inst) {
  if (!inst || !Value::isLiveValue(inst))
    return false;
  BasicBlock* bb = inst->parent();
  if (!bb)
    return false;
  for (auto& ptr : bb->instructions()) {
    if (ptr.get() == inst) {
      bb->remove(inst);
      return true;
    }
  }
  return false;
}

} // namespace

bool deadStoreElimination(Module& mod,
                          analysis::AnalysisManager& analysisManager) {
  bool changed = false;

  changed |= eliminateNeverLoadedAllocas(mod);
  if (changed)
    analysisManager.invalidateModule(analysis::AnalysisInvalidation::Module);

  const analysis::FunctionEffectMap& effects =
      analysisManager.functionEffects();

  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();

    for (auto& bbPtr : func->blocks()) {
      BasicBlock* bb = bbPtr.get();
      auto& insts = bb->instructions();

      // 候选 store 只是搜索空间；真正的删除必须通过统一的
      // MemorySSA-lite 区间覆盖与 observer 查询，不依赖地址 SSA 值
      // 恰好相同。
      std::vector<Instruction*> candidateStores;
      std::vector<Instruction*> deadStores;
      std::unordered_set<Instruction*> deadSet;

      for (auto& instPtr : insts) {
        Instruction* inst = instPtr.get();
        if (!inst) continue;

        if (inst->opcode() == Opcode::Store && inst->numOperands() >= 2) {
          for (Instruction* earlier : candidateStores) {
            if (!earlier || deadSet.count(earlier))
              continue;
            if (!analysis::mustOverwrite(earlier, inst).proved())
              continue;
            const analysis::ProofResult observer =
                analysis::hasObserverBetween(earlier, inst, &effects);
            if (observer.state != analysis::ProofState::Disproved)
              continue;
            deadStores.push_back(earlier);
            deadSet.insert(earlier);
          }
          candidateStores.push_back(inst);
        }
      }

      // 逆序删除无用存储，保持其余指令索引稳定。
      for (auto it = deadStores.rbegin(); it != deadStores.rend(); ++it) {
        Instruction* dead = *it;
        // 确认指令仍在当前基本块中。
        bool found = false;
        for (auto& instPtr : bb->instructions()) {
          if (instPtr.get() == dead) {
            found = true;
            break;
          }
        }
        if (found) {
          dead->dropAllOperands();
          bb->remove(dead);
          changed = true;
        }
      }
    }
  }

  return changed;
}

class DeadStoreEliminationPass : public Pass {
public:
  std::string name() const override { return "DeadStoreElimination"; }
  bool run(Module& mod, analysis::AnalysisManager& am) override {
    return deadStoreElimination(mod, am);
  }
};
class DeadAllocaEliminationPass : public Pass {
public:
  std::string name() const override { return "DeadAllocaElimination"; }
  bool run(Module& mod, analysis::AnalysisManager&) override {
    return eliminateNeverLoadedAllocas(mod);
  }
};
REGISTER_PASS(DeadAllocaEliminationPass);

REGISTER_PASS(DeadStoreEliminationPass);

static bool eliminateNeverLoadedAllocas(Module& mod) {
  bool changed = false;
  // 保留嵌套 DFE 的结果，使模块级变化能够正确刷新缓存分析。
  changed |= eliminateDeadFunctions(mod);
  for (auto& fp : mod.functions()) {
    Function* func = fp.get();
    std::vector<Instruction*> allocaInsts;

    for (auto& bp : func->blocks()) {
      for (auto& ip : bp->instructions()) {
        Instruction* inst = ip.get();
        if (inst->opcode() == Opcode::Alloca) {
          allocaInsts.push_back(inst);
        }
      }
    }

    for (Instruction* alloca : allocaInsts) {
      DeadAllocaPlan plan;
      if (!collectDeadAllocaPlan(alloca, plan))
        continue;

      for (Instruction* store : plan.stores)
        changed |= removeIfPresent(store);

      bool removedProjection = true;
      while (removedProjection) {
        removedProjection = false;
        for (Instruction* projection : plan.projections) {
          if (projection && Value::isLiveValue(projection) &&
              projection->parent() && projection->uses().empty()) {
            removedProjection |= removeIfPresent(projection);
          }
        }
      }

      if (alloca->uses().empty())
        changed |= removeIfPresent(alloca);
    }
  }
  return changed;
}



// 消除死函数：移除从 main 不可到达的函数。
// 处理包含未使用函数的普通程序。
bool eliminateDeadFunctions(Module& mod) {
  // fprintf(stderr, "[DFE] CALLED! funcs=%zu\n", mod.functions().size());
  Function* mainFunc = nullptr;
  for (auto& fp : mod.functions()) {
    if (fp->name() == "main") { mainFunc = fp.get(); break; }
  }
  if (!mainFunc) return false;

  std::unordered_map<std::string, std::unordered_set<std::string>> callGraph;
  for (auto& fp : mod.functions()) {
    Function* func = fp.get();
    auto& callees = callGraph[func->name()];
    for (auto& bp : func->blocks()) {
      for (auto& ip : bp->instructions()) {
        Instruction* inst = ip.get();
        if (!inst) continue;
        if (inst->opcode() == Opcode::Call && !inst->name().empty()) {
          callees.insert(inst->name());
        }
      }
    }
  }

  std::unordered_set<std::string> reachable;
  std::vector<std::string> queue;
  queue.push_back("main");
  reachable.insert("main");
  while (!queue.empty()) {
    std::string curr = queue.back();
    queue.pop_back();
    auto it = callGraph.find(curr);
    if (it == callGraph.end()) continue;
    for (const auto& callee : it->second) {
      if (reachable.insert(callee).second) queue.push_back(callee);
    }
  }

  bool changed = false;
  std::vector<Function*> unreachable;
  for (auto& fp : mod.functions()) {
    if (reachable.count(fp->name()) == 0 && !mod.isBuiltin(fp->name()))
      unreachable.push_back(fp.get());
  }
  for (Function* func : unreachable) {
    mod.removeFunction(func);
    changed = true;
  }
  return changed;
}

class DeadFunctionEliminationPass : public Pass {
public:
  std::string name() const override { return "DeadFunctionElimination"; }
  bool run(Module& mod, analysis::AnalysisManager&) override {
    return eliminateDeadFunctions(mod);
  }
};
REGISTER_PASS(DeadFunctionEliminationPass);
} // namespace core_ir
