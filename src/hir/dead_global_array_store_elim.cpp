#include "research_switch.h"
// 文件职责：实现未读全局数组死存储消除——全模块无读、指针不逃逸的全局
// 数组的所有存储不可观察，整体删除（sl 族 y 数组 N^3 死写即此模式）。
#include "dead_global_array_store_elim.h"

#include "../analysis/analysis_manager.h"

#include <cstdlib>
#include <iostream>
#include <unordered_set>
#include <vector>

using namespace core_ir;

namespace hir {
namespace {

bool traceOn() {
  return sycEnvironmentValue("SYC_TRACE_HIR_DEAD_GLOBAL_ARRAY_STORE_ELIM") != nullptr;
}

// 从全局数组出发做前向派生闭包。仅允许两类用途：
//   1) GEP/BitCast 且本值位于基址位（operand 0）——产生派生指针，入闭包；
//   2) Store/VStore 且本值位于地址位（operand 1）且不出现在值位——收集为死写。
// 其余任何用途（Load/VLoad 读、Call 实参逃逸、Phi/Select 汇合、GEP 索引位、
// Store 值位等）一律判定不可删。返回 true 时 deadStores 为 G 的全部存储。
bool collectDeadStores(GlobalVar* gv, std::vector<Instruction*>& deadStores) {
  std::vector<Value*> worklist{gv};
  std::unordered_set<Value*> visited{gv};
  while (!worklist.empty()) {
    Value* cur = worklist.back();
    worklist.pop_back();
    for (Instruction* user : cur->uses()) {
      switch (user->opcode()) {
      case Opcode::GEP:
      case Opcode::BitCast:
        if (user->numOperands() < 1 || user->operand(0) != cur)
          return false; // 派生指针只认基址位；索引位出现指针直接放弃
        if (visited.insert(user).second)
          worklist.push_back(user);
        break;
      case Opcode::Store:
      case Opcode::VStore:
        if (user->numOperands() < 2 || user->operand(1) != cur ||
            user->operand(0) == cur)
          return false; // 指针出现在值位=写入内存逃逸
        deadStores.push_back(user);
        break;
      default:
        return false; // Load/VLoad/Call/Phi/Select/Mov 等：读或逃逸
      }
    }
  }
  return true;
}

} // namespace

bool applyDeadGlobalArrayStoreElim(core_ir::Module& mod,
                                   analysis::AnalysisManager&) {
  bool changed = false;
  for (const auto& gvPtr : mod.globals()) {
    GlobalVar* gv = gvPtr.get();
    // 只处理非 const 数组；标量全局交由既有 pass，const 数组本就无写。
    if (gv->isConst() || gv->dims().empty())
      continue;
    std::vector<Instruction*> deadStores;
    if (!collectDeadStores(gv, deadStores))
      continue;
    if (deadStores.empty())
      continue;
    if (traceOn()) {
      std::cerr << "[dead-global-array-store-elim] phase=late gv="
                << gv->name() << " removed_stores=" << deadStores.size()
                << "\n";
    }
    for (Instruction* store : deadStores) {
      if (store->parent())
        store->parent()->remove(store);
    }
    changed = true;
  }
  return changed;
}

} // namespace hir
