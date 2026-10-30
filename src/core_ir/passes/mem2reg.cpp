#include "research_switch.h"
// 文件职责：把可提升的内存读写转换为 CoreIR SSA 值和 phi 节点。
#include "mem2reg.h"
#include "passes_utils.h"
#include "phi_utils.h"
#include "../../analysis/dominators.h"

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace core_ir {

using analysis::DomSet;
using analysis::buildDomTreeChildren;
using analysis::computeDomFrontier;
using analysis::computeIDom;

bool usesOnlyDirectScalarLoadsStores(Instruction* alloca, BasicBlock*& useBlock) {
  useBlock = nullptr;
  auto users = alloca->uses();
  if (users.empty()) return false;

  for (auto* user : users) {
    if (!user || !user->parent()) return false;

    if (!useBlock) useBlock = user->parent();
    if (useBlock != user->parent()) return false;

    if (user->opcode() == Opcode::Load) {
      if (user->numOperands() != 1 || user->operand(0) != alloca)
        return false;
    } else if (user->opcode() == Opcode::Store) {
      if (user->numOperands() != 2 || user->operand(1) != alloca)
        return false;
    } else {
      return false;
    }
  }

  return useBlock != nullptr;
}

bool tryPromoteSingleBlockScalarAlloca(Instruction* alloca) {
  BasicBlock* block = nullptr;
  if (!usesOnlyDirectScalarLoadsStores(alloca, block)) return false;
  if (alloca->parent() != block) return false;

  Value* current = nullptr;
  std::vector<Instruction*> toRemove;
  bool sawUse = false;

  for (auto& instPtr : block->instructions()) {
    auto* inst = instPtr.get();
    if (inst == alloca) continue;

    if (inst->opcode() == Opcode::Store && inst->operand(1) == alloca) {
      current = inst->operand(0);
      toRemove.push_back(inst);
      sawUse = true;
    } else if (inst->opcode() == Opcode::Load && inst->operand(0) == alloca) {
      if (!current) return false;
      inst->replaceAllUsesWith(current);
      toRemove.push_back(inst);
      sawUse = true;
    }
  }

  if (!sawUse) return false;

  for (auto* inst : toRemove)
    if (inst->parent() == block)
      block->remove(inst);

  if (alloca->uses().empty() && alloca->parent() == block) {
    block->parent()->removeAlloca(alloca);
    block->remove(alloca);
  }

  return true;
}

bool tryPromoteEntrySingleStoreScalarAlloca(Instruction* alloca) {
  if (!alloca->parent() || !alloca->parent()->parent()) return false;
  auto* entry = alloca->parent()->parent()->entryBlock();
  if (!entry || alloca->parent() != entry) return false;

  auto users = alloca->uses();
  if (users.empty()) return false;

  Instruction* store = nullptr;
  std::vector<Instruction*> loads;
  for (auto* user : users) {
    if (!user || !user->parent()) return false;
    if (user->opcode() == Opcode::Load) {
      if (user->numOperands() != 1 || user->operand(0) != alloca)
        return false;
      loads.push_back(user);
    } else if (user->opcode() == Opcode::Store) {
      if (user->numOperands() != 2 || user->operand(1) != alloca)
        return false;
      if (store) return false;
      store = user;
    } else {
      return false;
    }
  }

  if (!store || store->parent() != entry || loads.empty()) return false;

  bool seenStore = false;
  for (auto& instPtr : entry->instructions()) {
    auto* inst = instPtr.get();
    if (inst == store) {
      seenStore = true;
    } else if (!seenStore && inst->opcode() == Opcode::Load &&
               inst->operand(0) == alloca) {
      return false;
    }
  }
  if (!seenStore) return false;

  Value* storedValue = store->operand(0);
  for (auto* load : loads)
    load->replaceAllUsesWith(storedValue);

  for (auto* load : loads)
    if (load->parent())
      load->parent()->remove(load);

  if (store->parent())
    store->parent()->remove(store);

  if (alloca->uses().empty() && alloca->parent()) {
    alloca->parent()->parent()->removeAlloca(alloca);
    alloca->parent()->remove(alloca);
  }

  return true;
}

bool singleBlockScalarMem2Reg(Module& mod) {
  bool changed = false;
  for (auto& func : mod.functions()) {
    std::vector<Instruction*> candidates;
    for (auto& bb : func->blocks()) {
      for (auto& instPtr : bb->instructions()) {
        auto* inst = instPtr.get();
        if (inst->opcode() == Opcode::Alloca && isScalarAlloca(inst))
          candidates.push_back(inst);
      }
    }

    for (auto* alloca : candidates) {
      if (!alloca->parent()) continue;
      if (tryPromoteEntrySingleStoreScalarAlloca(alloca)) {
        changed = true;
      } else if (alloca->parent() && tryPromoteSingleBlockScalarAlloca(alloca)) {
        changed = true;
      }
    }
  }
  return changed;
}

std::unordered_set<BasicBlock*> collectReachableBlocks(Function* func) {
  std::unordered_set<BasicBlock*> reachable;
  if (!func || !func->entryBlock()) return reachable;
  std::vector<BasicBlock*> stack{func->entryBlock()};
  while (!stack.empty()) {
    BasicBlock* bb = stack.back();
    stack.pop_back();
    if (!bb || reachable.count(bb)) continue;
    reachable.insert(bb);
    for (auto* succ : bb->succs())
      if (succ && !reachable.count(succ))
        stack.push_back(succ);
  }
  return reachable;
}

bool isDirectAllocaUse(Instruction* user, Instruction* alloca) {
  if (!user || !alloca) return false;
  if (user->opcode() == Opcode::Load)
    return user->numOperands() == 1 && user->operand(0) == alloca;
  if (user->opcode() == Opcode::Store)
    return user->numOperands() == 2 && user->operand(1) == alloca;
  return false;
}

bool isPromotableScalarAlloca(Instruction* alloca) {
  if (!isScalarAlloca(alloca)) return false;
  auto users = alloca->uses();
  if (users.empty()) return false;
  bool sawStore = false;
  for (auto* user : users) {
    if (!isDirectAllocaUse(user, alloca))
      return false;
    if (user->opcode() == Opcode::Store)
      sawStore = true;
  }
  return sawStore;
}

bool hasEntryStoreBeforeAnyLoad(Function* func, Instruction* alloca) {
  if (!func || !alloca || !func->entryBlock())
    return false;
  bool sawStore = false;
  for (auto& instPtr : func->entryBlock()->instructions()) {
    auto* inst = instPtr.get();
    if (inst->opcode() == Opcode::Store && inst->numOperands() == 2 &&
        inst->operand(1) == alloca) {
      sawStore = true;
    } else if (inst->opcode() == Opcode::Load && inst->numOperands() == 1 &&
               inst->operand(0) == alloca && !sawStore) {
      return false;
    }
  }
  return sawStore;
}

// 批量计算可能包含未初始化加载的 alloca。所有 alloca 通过位掩码在一次数据流
// 遍历中同时处理，返回没有未定义加载的 alloca 集合。
std::unordered_set<Instruction*> collectSafeAllocas(
    Function* func,
    const std::vector<Instruction*>& allocas,
    const std::unordered_set<BasicBlock*>& reachable) {

  if (allocas.empty()) return {};

  constexpr int kMaskBits = 64;
  int n = static_cast<int>(std::min<size_t>(allocas.size(), kMaskBits));
  // 批量数据流使用 64 位掩码，必须与调用方的提升数量上限保持一致；移位达到
  // 64 位会失去定义，并可能错误标记无关 alloca。
  using Mask = uint64_t;
  auto fullMask = [n]() -> Mask {
    return n == kMaskBits ? ~Mask{0} : ((Mask{1} << n) - 1);
  };
  std::unordered_map<Instruction*, int> idxOf;
  for (int i = 0; i < n; ++i) idxOf[allocas[i]] = i;
  Mask entryDefinedMask = 0;
  for (int i = 0; i < n; ++i) {
    if (hasEntryStoreBeforeAnyLoad(func, allocas[i]))
      entryDefinedMask |= (Mask{1} << i);
  }

  // inMask[bb] 表示进入 bb 时已经定义的 alloca 集合。
  std::unordered_map<BasicBlock*, Mask> inMask, outMask;
  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!reachable.count(bb)) continue;
    inMask[bb] = (bb != func->entryBlock()) ? fullMask() : entryDefinedMask;
    outMask[bb] = (bb != func->entryBlock()) ? 0 : entryDefinedMask;
  }
  inMask[func->entryBlock()] = entryDefinedMask;

  bool changed = true;
  while (changed) {
    changed = false;

    // 正向传播：in[b] 等于所有前驱 out 集合的交集。
    for (auto& bbPtr : func->blocks()) {
      BasicBlock* bb = bbPtr.get();
      if (!reachable.count(bb) || bb == func->entryBlock()) continue;
      Mask newIn = fullMask();
      bool hasReachablePred = false;
      for (auto* pred : bb->preds()) {
        if (!pred || !reachable.count(pred)) continue;
        hasReachablePred = true;
        newIn &= outMask[pred];
      }
      if (!hasReachablePred) newIn = 0;
      if (newIn != inMask[bb]) {
        inMask[bb] = newIn;
        changed = true;
      }
    }

    // 局部传递：扫描指令并更新已定义集合。
    for (auto& bbPtr : func->blocks()) {
      BasicBlock* bb = bbPtr.get();
      if (!reachable.count(bb)) continue;
      Mask cur = inMask[bb];
      for (auto& instPtr : bb->instructions()) {
        Instruction* inst = instPtr.get();
        if (inst->opcode() == Opcode::Store) {
          auto it = idxOf.find(dynamic_cast<Instruction*>(inst->operand(1)));
          if (it != idxOf.end())
            cur |= (Mask{1} << it->second);
        }
      }
      if (cur != outMask[bb]) {
        outMask[bb] = cur;
        changed = true;
      }
    }
  }

  // 数据流结果稳定后，检查读取前尚未定义的加载。
  Mask undefMask = 0;
  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!reachable.count(bb)) continue;
    Mask cur = inMask[bb];
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (inst->opcode() == Opcode::Load) {
        auto it = idxOf.find(dynamic_cast<Instruction*>(inst->operand(0)));
        if (it != idxOf.end()) {
          if (!(cur & (Mask{1} << it->second)))
            undefMask |= (Mask{1} << it->second);
        }
      } else if (inst->opcode() == Opcode::Store) {
        auto it = idxOf.find(dynamic_cast<Instruction*>(inst->operand(1)));
        if (it != idxOf.end())
          cur |= (Mask{1} << it->second);
      }
    }
  }

  std::unordered_set<Instruction*> safe;
  for (int i = 0; i < n; ++i)
    if (!(undefMask & (Mask{1} << i)))
      safe.insert(allocas[i]);
  return safe;
}

bool hasStoreBeforeLoadInBlock(Instruction* alloca, Instruction* load) {
  if (!alloca || !load || !load->parent())
    return false;
  for (auto& instPtr : load->parent()->instructions()) {
    Instruction* inst = instPtr.get();
    if (inst == load)
      break;
    if (inst->opcode() == Opcode::Store && inst->numOperands() == 2 &&
        inst->operand(1) == alloca) {
      return true;
    }
  }
  return false;
}

bool allLoadsDominatedByStore(
    Instruction* alloca,
    const std::unordered_map<BasicBlock*, std::unordered_set<BasicBlock*>>& doms,
    const std::unordered_set<BasicBlock*>& reachable) {
  if (!alloca)
    return false;

  std::vector<Instruction*> stores;
  bool sawLoad = false;
  for (auto* user : alloca->uses()) {
    if (!user || !user->parent() || !reachable.count(user->parent()))
      continue;
    if (user->opcode() == Opcode::Store && user->numOperands() == 2 &&
        user->operand(1) == alloca) {
      stores.push_back(user);
    }
  }

  if (stores.empty())
    return false;

  for (auto* user : alloca->uses()) {
    if (!user || user->opcode() != Opcode::Load || user->numOperands() != 1 ||
        user->operand(0) != alloca || !user->parent() ||
        !reachable.count(user->parent())) {
      continue;
    }
    sawLoad = true;
    if (hasStoreBeforeLoadInBlock(alloca, user))
      continue;

    BasicBlock* loadBB = user->parent();
    bool covered = false;
    auto domIt = doms.find(loadBB);
    if (domIt != doms.end()) {
      for (auto* store : stores) {
        BasicBlock* storeBB = store->parent();
        if (!storeBB || storeBB == loadBB)
          continue;
        if (domIt->second.count(storeBB)) {
          covered = true;
          break;
        }
      }
    }
    if (!covered)
      return false;
  }

  return sawLoad;
}


struct AllocaBlockSummary {
  bool hasDef = false;
  bool useBeforeDef = false;
};

AllocaBlockSummary summarizeAllocaBlock(BasicBlock* bb, Instruction* alloca) {
  AllocaBlockSummary summary;
  if (!bb || !alloca)
    return summary;

  for (auto& instPtr : bb->instructions()) {
    Instruction* inst = instPtr.get();
    if (inst->opcode() == Opcode::Load && inst->numOperands() == 1 &&
        inst->operand(0) == alloca) {
      if (!summary.hasDef)
        summary.useBeforeDef = true;
      continue;
    }
    if (inst->opcode() == Opcode::Store && inst->numOperands() == 2 &&
        inst->operand(1) == alloca) {
      summary.hasDef = true;
      continue;
    }
  }

  return summary;
}

std::unordered_set<BasicBlock*> computeAllocaLiveInBlocks(
    Function* func, Instruction* alloca,
    const std::unordered_set<BasicBlock*>& reachable) {
  std::unordered_set<BasicBlock*> liveInBlocks;
  if (!func || !alloca)
    return liveInBlocks;

  std::unordered_map<BasicBlock*, AllocaBlockSummary> summaries;
  std::unordered_map<BasicBlock*, bool> liveIn;
  std::unordered_map<BasicBlock*, bool> liveOut;
  std::vector<BasicBlock*> blocks;
  blocks.reserve(func->blocks().size());
  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!reachable.count(bb))
      continue;
    blocks.push_back(bb);
    summaries[bb] = summarizeAllocaBlock(bb, alloca);
    liveIn[bb] = summaries[bb].useBeforeDef;
    liveOut[bb] = false;
  }

  bool changed = true;
  while (changed) {
    changed = false;
    for (auto it = blocks.rbegin(); it != blocks.rend(); ++it) {
      BasicBlock* bb = *it;
      bool newLiveOut = false;
      for (auto* succ : bb->succs()) {
        if (!succ || !reachable.count(succ))
          continue;
        if (liveIn[succ]) {
          newLiveOut = true;
          break;
        }
      }
      bool newLiveIn = summaries[bb].useBeforeDef ||
                       (newLiveOut && !summaries[bb].hasDef);
      if (newLiveOut != liveOut[bb]) {
        liveOut[bb] = newLiveOut;
        changed = true;
      }
      if (newLiveIn != liveIn[bb]) {
        liveIn[bb] = newLiveIn;
        changed = true;
      }
    }
  }

  for (auto* bb : blocks) {
    if (liveIn[bb])
      liveInBlocks.insert(bb);
  }
  return liveInBlocks;
}

Instruction* createScalarSlotAllocaInEntry(Function* func, Type elemTy,
                                           const std::string& nameHint) {
  auto* entry = func->entryBlock();
  auto* slot = new Instruction(Opcode::Alloca, Type::ptr(), nameHint);
  slot->setMemoryInfo(elemTy, elemTy.isPtr() ? 8 : 4, false, {});

  Instruction* firstNonAlloca = nullptr;
  for (auto& instPtr : entry->instructions()) {
    if (instPtr->opcode() != Opcode::Alloca) {
      firstNonAlloca = instPtr.get();
      break;
    }
  }
  if (firstNonAlloca)
    entry->insertBefore(firstNonAlloca, slot);
  else
    entry->pushBack(slot);
  return slot;
}

bool promoteSingleAllocaWithPhi(
    Function* func, Instruction* alloca,
    const std::unordered_set<BasicBlock*>& reachable,
    const std::unordered_map<BasicBlock*, std::vector<BasicBlock*>>& domChildren,
    const std::unordered_map<BasicBlock*, std::unordered_set<BasicBlock*>>& df) {
  bool trace = sycEnvironmentValue("COREIR_MEM2REG_TRACE") != nullptr;
  std::unordered_set<BasicBlock*> defBlocks;
  for (auto* user : alloca->uses()) {
    if (!user || user->opcode() != Opcode::Store || user->numOperands() != 2 ||
        user->operand(1) != alloca || !user->parent())
      continue;
    if (reachable.count(user->parent()))
      defBlocks.insert(user->parent());
  }
  if (defBlocks.empty()) return false;
  auto liveInBlocks = computeAllocaLiveInBlocks(func, alloca, reachable);

  std::unordered_map<BasicBlock*, Instruction*> phiAt;
  std::unordered_set<BasicBlock*> hasPhi;
  std::queue<BasicBlock*> workQ;
  for (auto* bb : defBlocks) workQ.push(bb);

  Type promotedTy = alloca->memoryElemTy();
  while (!workQ.empty()) {
    BasicBlock* bb = workQ.front();
    workQ.pop();
    auto dfIt = df.find(bb);
    if (dfIt == df.end()) continue;
    for (auto* y : dfIt->second) {
      if (!liveInBlocks.count(y))
        continue;
      if (hasPhi.count(y)) continue;
      auto* phi = new Instruction(Opcode::Phi, promotedTy, alloca->name() + ".phi");
      y->pushFront(phi);
      phiAt[y] = phi;
      hasPhi.insert(y);
      if (!defBlocks.count(y))
        workQ.push(y);
    }
  }

  std::vector<Value*> stack;
  std::unordered_map<BasicBlock*, std::vector<Instruction*>> removeInBlock;
  std::vector<std::pair<Instruction*, Value*>> loadReplacements;
  std::unordered_map<Instruction*, std::vector<std::pair<Value*, BasicBlock*>>> phiIncoming;
  bool changed = false;
  bool failed = false;

  std::function<void(BasicBlock*)> rename = [&](BasicBlock* bb) {
    if (!bb || failed) return;
    size_t mark = stack.size();

    auto phiIt = phiAt.find(bb);
    if (phiIt != phiAt.end())
      stack.push_back(phiIt->second);

    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (inst->opcode() == Opcode::Store && inst->numOperands() == 2 &&
          inst->operand(1) == alloca) {
        Value* pushed = inst->operand(0);
        stack.push_back(pushed);
        removeInBlock[bb].push_back(inst);
        changed = true;
      } else if (inst->opcode() == Opcode::Load && inst->numOperands() == 1 &&
                 inst->operand(0) == alloca) {
        if (stack.empty()) {
          if (trace) {
            std::cerr << "[mem2reg] failed: empty stack for load in function "
                      << func->name() << ", alloca %" << alloca->id()
                      << ", block " << bb->name() << "\n";
          }
          failed = true;
          break;
        }
        Value* top = stack.back();
        loadReplacements.push_back({inst, top});
        removeInBlock[bb].push_back(inst);
        changed = true;
      }
    }
    if (failed) return;

    Value* outValue = stack.empty() ? nullptr : stack.back();
    for (auto* succ : bb->succs()) {
      if (!succ || !reachable.count(succ)) continue;
      auto succPhiIt = phiAt.find(succ);
      if (succPhiIt == phiAt.end()) continue;
      if (!outValue) {
        if (trace) {
          std::cerr << "[mem2reg] failed: null outgoing value in function "
                    << func->name() << ", alloca %" << alloca->id()
                    << ", block " << bb->name() << " -> " << succ->name()
                    << "\n";
        }
        failed = true;
        break;
      }
      phiIncoming[succPhiIt->second].push_back({outValue, bb});
    }
    if (failed) return;

    auto childIt = domChildren.find(bb);
    if (childIt != domChildren.end()) {
      for (auto* child : childIt->second)
        rename(child);
    }

    while (stack.size() > mark)
      stack.pop_back();
  };

  rename(func->entryBlock());
  if (failed) {
    for (auto& [bb, phi] : phiAt) {
      if (phi && phi->parent() == bb)
        bb->remove(phi);
    }
    return false;
  }

  for (auto& [phi, incoming] : phiIncoming) {
    if (!phi)
      continue;
    for (const auto& [value, pred] : incoming) {
      phi->addOperand(value);
      phi->addOperand(pred);
    }
  }

  for (const auto& [load, replacement] : loadReplacements) {
    if (load)
      load->replaceAllUsesWith(replacement);
  }

  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    auto it = removeInBlock.find(bb);
    if (it == removeInBlock.end()) continue;
    for (auto* inst : it->second) {
      if (inst->parent() == bb)
        bb->remove(inst);
    }
  }

  if (alloca->uses().empty() && alloca->parent()) {
    alloca->parent()->parent()->removeAlloca(alloca);
    alloca->parent()->remove(alloca);
  }

  return changed;
}

bool globalScalarMem2Reg(Function* func,
                         analysis::FunctionAnalysisCache& cache) {
  if (!func || !func->entryBlock()) return false;
  bool trace = sycEnvironmentValue("COREIR_MEM2REG_TRACE") != nullptr;

  const auto& reachable = cache.reachable();
  std::vector<BasicBlock*> blocks;
  blocks.reserve(cache.blocks().size());
  for (auto* bb : cache.blocks()) {
    if (reachable.count(bb))
      blocks.push_back(bb);
  }
  if (blocks.empty()) return false;

  struct Candidate {
    Instruction* alloca;
    int uses;
  };

  std::vector<Instruction*> promotableAllocas;
  int totalAllocas = 0;
  int scalarAllocas = 0;
  for (auto& bbPtr : func->blocks()) {
    for (auto& instPtr : bbPtr->instructions()) {
      auto* inst = instPtr.get();
      if (inst->opcode() == Opcode::Alloca) {
        totalAllocas++;
        if (isScalarAlloca(inst)) scalarAllocas++;
      }
      if (isPromotableScalarAlloca(inst))
        promotableAllocas.push_back(inst);
    }
  }
  if (promotableAllocas.empty()) {
    return false;
  }


  // 按使用次数排序（热点优先），并限制处理数量。
  std::sort(promotableAllocas.begin(), promotableAllocas.end(),
            [](Instruction* a, Instruction* b) {
              int ua = static_cast<int>(a->uses().size());
              int ub = static_cast<int>(b->uses().size());
              if (ua != ub) return ua > ub;
              return a->id() < b->id();
            });

  // collectSafeAllocas() 使用一个 uint64_t 位集合批量分析加载前的初始化状态，
  // 因此一次提升批次最多处理 64 个 alloca。
  constexpr int kMaxPromotedAllocasPerFunc = 64;
  if (static_cast<int>(promotableAllocas.size()) > kMaxPromotedAllocasPerFunc)
    promotableAllocas.resize(kMaxPromotedAllocasPerFunc);

  // 批量检查：前面的 alloca 中哪些没有未定义加载。
  auto safeAllocas = collectSafeAllocas(func, promotableAllocas, reachable);
  const DomSet& doms = cache.dominators();

  std::vector<Candidate> candidates;
  for (auto* a : promotableAllocas) {
    bool safe = safeAllocas.count(a) ||
                allLoadsDominatedByStore(a, doms, reachable);
    if (trace) {
      std::cerr << "[mem2reg] candidate function " << func->name()
                << ", alloca %" << a->id()
                << ", uses=" << a->uses().size()
                << ", safe=" << (safe ? 1 : 0) << "\n";
    }
    if (safe)
      candidates.push_back({a, static_cast<int>(a->uses().size())});
  }
  if (candidates.empty()) return false;

  auto idom = computeIDom(func, blocks, doms);
  auto domChildren = buildDomTreeChildren(blocks, idom);
  auto df = computeDomFrontier(blocks, idom, reachable);

  bool changed = false;
  for (const auto& cand : candidates) {
    Instruction* alloca = cand.alloca;
    if (!alloca->parent()) continue;
    bool promoted = promoteSingleAllocaWithPhi(
        func, alloca, reachable, domChildren, df);
    if (trace) {
      std::cerr << "[mem2reg] result function " << func->name()
                << ", alloca %" << alloca->id()
                << ", promoted=" << (promoted ? 1 : 0) << "\n";
    }
    changed |= promoted;
  }

  return changed;
}

bool globalScalarMem2Reg(Module& mod) {
  analysis::AnalysisManager analysisManager(mod);
  return globalScalarMem2Reg(mod, analysisManager);
}

bool globalScalarMem2Reg(Module& mod,
                         analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& func : mod.functions()) {
    analysis::FunctionAnalysisCache& cache =
        analysisManager.function(func.get());
    bool funcChanged = globalScalarMem2Reg(func.get(), cache);
    if (funcChanged) {
      changed = true;
      // mem2reg 会插入 phi、删除 alloca/load/store，后续分析必须重建。
      analysisManager.invalidateFunction(
          func.get(), analysis::AnalysisInvalidation::CFG);
    }
  }
  return changed;
}

} // namespace core_ir
