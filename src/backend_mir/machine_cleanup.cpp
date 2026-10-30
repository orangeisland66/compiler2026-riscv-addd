#include "research_switch.h"
// 文件职责：清理机器指令中的死定义、冗余 move 和可安全消除的跳转。本文件负责实现具体逻辑。
#include "machine_cleanup.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>

namespace mir {

namespace {

struct BlockLive {
  std::unordered_set<int> use;
  std::unordered_set<int> def;
  std::unordered_set<int> liveIn;
  std::unordered_set<int> liveOut;
};

using VRegUseCountMap = std::unordered_map<int, int>;

struct KnownRegValue {
  enum class Kind { Imm, Symbol };

  Kind kind = Kind::Imm;
  int64_t imm = 0;
  std::string symbol;
};

struct MachineLoop {
  MachineBasicBlock* header = nullptr;
  MachineBasicBlock* preheader = nullptr;
  std::unordered_set<MachineBasicBlock*> blocks;
};

// MachineLoop uses a set for fast membership queries.  Never let its pointer
// iteration order choose a rewrite representative: ASLR would then make two
// identical compiler invocations produce different address offsets.
std::vector<MachineBasicBlock*> orderedLoopBlocks(const MachineLoop& loop) {
  std::vector<MachineBasicBlock*> blocks(loop.blocks.begin(),
                                         loop.blocks.end());
  std::sort(blocks.begin(), blocks.end(),
            [](const MachineBasicBlock* lhs, const MachineBasicBlock* rhs) {
              if (!lhs || !rhs)
                return lhs != nullptr;
              return lhs->label() < rhs->label();
            });
  return blocks;
}

std::vector<std::unordered_set<int>> computeBlockDominators(
    MachineFunction& mf);
std::vector<MachineLoop> collectNaturalLoops(MachineFunction& mf);

bool traceLoopAddressCleanup() {
  return sycEnvironmentValue("SYC_TRACE_MIR_LOOP_ADDR") != nullptr;
}

bool sameKnownValue(const KnownRegValue& lhs, const KnownRegValue& rhs) {
  if (lhs.kind != rhs.kind)
    return false;
  if (lhs.kind == KnownRegValue::Kind::Imm)
    return lhs.imm == rhs.imm;
  return lhs.symbol == rhs.symbol;
}

bool hasVirtualOperands(MachineFunction& mf) {
  for (auto& bbPtr : mf.blocks()) {
    for (auto& miPtr : bbPtr->insts()) {
      if (!miPtr)
        continue;
      for (const auto& op : miPtr->operands()) {
        if (op.isReg() && isVirtualReg(op.reg))
          return true;
      }
    }
  }
  return false;
}

bool canReachMachineBlock(MachineBasicBlock* from,
                          MachineBasicBlock* target) {
  if (!from || !target)
    return false;
  std::queue<MachineBasicBlock*> worklist;
  std::unordered_set<MachineBasicBlock*> visited;
  worklist.push(from);
  visited.insert(from);
  while (!worklist.empty()) {
    MachineBasicBlock* block = worklist.front();
    worklist.pop();
    if (block == target)
      return true;
    for (MachineBasicBlock* successor : block->succs()) {
      if (successor && visited.insert(successor).second)
        worklist.push(successor);
    }
  }
  return false;
}

bool matchingScalarLoadStore(MIOp load, MIOp store) {
  return (load == MIOp::LW && store == MIOp::SW) ||
         (load == MIOp::LD && store == MIOp::SD) ||
         (load == MIOp::FLW && store == MIOp::FSW);
}

bool definesPhysicalReg(const MachineInst& inst, int reg) {
  for (const auto& operand : inst.operands()) {
    if (operand.isReg() && operand.isDef && operand.reg == reg)
      return true;
  }
  return false;
}

bool hasPostRALoadStoreClobber(
    MachineBasicBlock* loadBlock, size_t loadIndex,
    MachineBasicBlock* storeBlock, size_t storeIndex,
    int valueReg, int addressReg,
    const std::unordered_map<MachineBasicBlock*, size_t>& blockIndex,
    const std::vector<std::unordered_set<int>>& doms) {
  auto loadIt = blockIndex.find(loadBlock);
  auto storeIt = blockIndex.find(storeBlock);
  if (loadIt == blockIndex.end() || storeIt == blockIndex.end() ||
      !doms[storeIt->second].count(static_cast<int>(loadIt->second)))
    return true;

  for (const auto& blockPtr : loadBlock->parent()->blocks()) {
    MachineBasicBlock* block = blockPtr.get();
    auto blockIt = blockIndex.find(block);
    if (blockIt == blockIndex.end() ||
        !doms[blockIt->second].count(static_cast<int>(loadIt->second)) ||
        !canReachMachineBlock(block, storeBlock))
      continue;

    bool afterLoad = block != loadBlock;
    const auto& insts = block->insts();
    for (size_t i = 0; i < insts.size(); ++i) {
      MachineInst* inst = insts[i].get();
      if (!inst)
        continue;
      if (block == loadBlock && !afterLoad) {
        if (i == loadIndex)
          afterLoad = true;
        continue;
      }
      if (block == storeBlock && i == storeIndex)
        break;
      if (mioIsCall(inst->opcode()) || mioMayStore(inst->opcode()) ||
          definesPhysicalReg(*inst, valueReg) ||
          definesPhysicalReg(*inst, addressReg))
        return true;
    }
  }
  return false;
}

struct GlobalMemRef {
  std::string symbol;
  int64_t offset = 0;
};

bool findGlobalMemRef(MachineBasicBlock* block, size_t useIndex, int baseReg,
                      GlobalMemRef& ref) {
  if (!block || !isPhysReg(baseReg))
    return false;

  int currentReg = baseReg;
  int64_t offset = 0;
  const auto& insts = block->insts();
  for (size_t i = useIndex; i > 0; --i) {
    MachineInst* inst = insts[i - 1].get();
    if (!inst)
      continue;

    bool definesCurrent = false;
    for (const auto& operand : inst->operands()) {
      if (operand.isReg() && operand.isDef && operand.reg == currentReg) {
        definesCurrent = true;
        break;
      }
    }
    if (!definesCurrent)
      continue;

    if (inst->opcode() == MIOp::LA && inst->numOperands() == 2 &&
        inst->operand(0).isReg() && inst->operand(0).reg == currentReg &&
        inst->operand(1).isSym()) {
      ref.symbol = inst->operand(1).sym;
      ref.offset = offset;
      return true;
    }

    if (inst->opcode() == MIOp::ADDI && inst->numOperands() == 3 &&
        inst->operand(0).isReg() && inst->operand(0).reg == currentReg &&
        inst->operand(1).isReg() && inst->operand(2).isImm()) {
      offset += inst->operand(2).imm;
      currentReg = inst->operand(1).reg;
      if (!isPhysReg(currentReg))
        return false;
      continue;
    }
    return false;
  }
  return false;
}

bool sameGlobalMemRef(const GlobalMemRef& lhs, const GlobalMemRef& rhs) {
  return lhs.symbol == rhs.symbol && lhs.offset == rhs.offset;
}

bool hasPostRAGlobalLoadClobber(
    MachineBasicBlock* loadBlock, size_t loadIndex,
    MachineBasicBlock* storeBlock, size_t storeIndex, int valueReg,
    const GlobalMemRef& target,
    const std::unordered_map<MachineBasicBlock*, size_t>& blockIndex,
    const std::vector<std::unordered_set<int>>& doms) {
  auto loadIt = blockIndex.find(loadBlock);
  auto storeIt = blockIndex.find(storeBlock);
  if (loadIt == blockIndex.end() || storeIt == blockIndex.end() ||
      !doms[storeIt->second].count(static_cast<int>(loadIt->second)))
    return true;

  for (const auto& blockPtr : loadBlock->parent()->blocks()) {
    MachineBasicBlock* block = blockPtr.get();
    auto blockIt = blockIndex.find(block);
    if (blockIt == blockIndex.end() ||
        !doms[blockIt->second].count(static_cast<int>(loadIt->second)) ||
        !canReachMachineBlock(block, storeBlock))
      continue;

    bool afterLoad = block != loadBlock;
    const auto& insts = block->insts();
    for (size_t i = 0; i < insts.size(); ++i) {
      MachineInst* inst = insts[i].get();
      if (!inst)
        continue;
      if (block == loadBlock && !afterLoad) {
        if (i == loadIndex)
          afterLoad = true;
        continue;
      }
      if (block == storeBlock && i == storeIndex)
        break;
      if (mioIsCall(inst->opcode()) ||
          definesPhysicalReg(*inst, valueReg))
        return true;
      if (!mioMayStore(inst->opcode()))
        continue;

      // SysY 没有指针值：未解析的存储属于数组或栈访问，不是为命名标量全局
      // 对象构造的别名。只有解析到同一命名全局对象的存储才会覆盖该加载。
      if (inst->numOperands() != 3 || !inst->operand(2).isReg())
        continue;
      GlobalMemRef clobber;
      if (findGlobalMemRef(block, i, inst->operand(2).reg, clobber) &&
          sameGlobalMemRef(clobber, target))
        return true;
    }
  }
  return false;
}

bool hoistInvariantPostRALoadsInFunction(MachineFunction& mf) {
  if (hasVirtualOperands(mf))
    return false;

  bool changed = false;
  for (const MachineLoop& loop : collectNaturalLoops(mf)) {
    if (!loop.header || !loop.preheader || loop.header->insts().empty())
      continue;

    MachineBasicBlock* header = loop.header;
    MachineInst* load = header->insts().front().get();
    if (!load || !mioMayLoad(load->opcode()) || load->numOperands() != 3 ||
        !load->operand(0).isReg() || !load->operand(1).isImm() ||
        !load->operand(2).isReg())
      continue;

    const int valueReg = load->operand(0).reg;
    const int addressReg = load->operand(2).reg;
    if (!isPhysReg(valueReg) || !isPhysReg(addressReg))
      continue;
    GlobalMemRef target;
    if (!findGlobalMemRef(header, 0, addressReg, target) &&
        !findGlobalMemRef(loop.preheader, loop.preheader->insts().size(),
                          addressReg, target))
      continue;

    bool safe = true;
    for (MachineBasicBlock* block : loop.blocks) {
      if (!block)
        continue;
      const auto& insts = block->insts();
      for (size_t i = 0; i < insts.size(); ++i) {
        MachineInst* inst = insts[i].get();
        if (!inst || inst == load)
          continue;
        if (mioIsCall(inst->opcode()) ||
            definesPhysicalReg(*inst, valueReg) ||
            definesPhysicalReg(*inst, addressReg)) {
          safe = false;
          break;
        }
        // 只有不执行任何内存写入的循环才提升循环不变量加载。这里的处理范围
        // 小于上面的分配后重复存储清理，未解析的数组存储不能使全局加载失效。
        if (mioMayStore(inst->opcode())) {
          safe = false;
          break;
        }
      }
      if (!safe)
        break;
    }
    if (!safe)
      continue;

    auto hoisted = std::make_unique<MachineInst>(load->opcode());
    for (const auto& operand : load->operands())
      hoisted->addOperand(operand);
    hoisted->comment = load->comment;
    MachineInst* terminator = loop.preheader->terminator();
    if (terminator)
      loop.preheader->insertBefore(terminator, std::move(hoisted));
    else
      loop.preheader->append(std::move(hoisted));
    header->insts().erase(header->insts().begin());
    changed = true;
  }
  return changed;
}

bool removeRedundantPostRAStoresInFunction(MachineFunction& mf) {
  if (hasVirtualOperands(mf))
    return false;
  const auto doms = computeBlockDominators(mf);
  std::unordered_map<MachineBasicBlock*, size_t> blockIndex;
  for (size_t i = 0; i < mf.blocks().size(); ++i)
    blockIndex[mf.blocks()[i].get()] = i;

  std::vector<MachineInst*> redundant;
  for (const auto& storeBlockPtr : mf.blocks()) {
    MachineBasicBlock* storeBlock = storeBlockPtr.get();
    const auto& stores = storeBlock->insts();
    for (size_t storeIndex = 0; storeIndex < stores.size(); ++storeIndex) {
      MachineInst* store = stores[storeIndex].get();
      if (!store || store->numOperands() != 3 ||
          !mioMayStore(store->opcode()) ||
          (store->opcode() != MIOp::SW && store->opcode() != MIOp::SD &&
           store->opcode() != MIOp::FSW) ||
          !store->operand(0).isReg() || !store->operand(1).isImm() ||
          !store->operand(2).isReg())
        continue;
      const int valueReg = store->operand(0).reg;
      const int addressReg = store->operand(2).reg;
      if (!isPhysReg(valueReg) || !isPhysReg(addressReg) ||
          valueReg == addressReg)
        continue;

      bool found = false;
      for (const auto& loadBlockPtr : mf.blocks()) {
        MachineBasicBlock* loadBlock = loadBlockPtr.get();
        const auto loadIt = blockIndex.find(loadBlock);
        const auto storeIt = blockIndex.find(storeBlock);
        if (loadIt == blockIndex.end() || storeIt == blockIndex.end() ||
            !doms[storeIt->second].count(static_cast<int>(loadIt->second)))
          continue;
        const auto& loads = loadBlock->insts();
        for (size_t loadIndex = 0; loadIndex < loads.size(); ++loadIndex) {
          MachineInst* load = loads[loadIndex].get();
          if (!load || !matchingScalarLoadStore(load->opcode(),
                                                store->opcode()) ||
              load->numOperands() != 3 || !load->operand(0).isReg() ||
              !load->operand(1).isImm() || !load->operand(2).isReg() ||
              load->operand(0).reg != valueReg ||
              load->operand(1).imm != store->operand(1).imm ||
              load->operand(2).reg != addressReg)
            continue;
          if (loadBlock == storeBlock && loadIndex >= storeIndex)
            continue;
          if (hasPostRALoadStoreClobber(
                  loadBlock, loadIndex, storeBlock, storeIndex, valueReg,
                  addressReg, blockIndex, doms))
            continue;
          redundant.push_back(store);
          found = true;
          break;
        }
        if (found)
          break;
      }

      if (found)
        continue;

      // 若存在被 store 支配的块里的 load，从同一全局地址按匹配宽度取值，
      // 说明该地址的值已在寄存器中，此 store 可判为冗余。
      GlobalMemRef storeRef;
      if (!findGlobalMemRef(storeBlock, storeIndex, addressReg, storeRef))
        continue;
      for (const auto& loadBlockPtr : mf.blocks()) {
        MachineBasicBlock* loadBlock = loadBlockPtr.get();
        const auto loadIt = blockIndex.find(loadBlock);
        const auto storeIt = blockIndex.find(storeBlock);
        if (loadIt == blockIndex.end() || storeIt == blockIndex.end() ||
            !doms[storeIt->second].count(static_cast<int>(loadIt->second)))
          continue;
        const auto& loads = loadBlock->insts();
        for (size_t loadIndex = 0; loadIndex < loads.size(); ++loadIndex) {
          MachineInst* load = loads[loadIndex].get();
          if (!load || !matchingScalarLoadStore(load->opcode(),
                                                store->opcode()) ||
              load->numOperands() != 3 || !load->operand(0).isReg() ||
              !load->operand(1).isImm() || !load->operand(2).isReg() ||
              load->operand(0).reg != valueReg)
            continue;
          GlobalMemRef loadRef;
          if (!findGlobalMemRef(loadBlock, loadIndex, load->operand(2).reg,
                                loadRef) ||
              loadRef.symbol != storeRef.symbol ||
              loadRef.offset + load->operand(1).imm !=
                  storeRef.offset + store->operand(1).imm)
            continue;
          if (loadBlock == storeBlock && loadIndex >= storeIndex)
            continue;
          if (hasPostRAGlobalLoadClobber(
                  loadBlock, loadIndex, storeBlock, storeIndex, valueReg,
                  storeRef, blockIndex, doms))
            continue;
          redundant.push_back(store);
          found = true;
          break;
        }
        if (found)
          break;
      }
    }
  }

  for (MachineInst* store : redundant) {
    for (const auto& blockPtr : mf.blocks()) {
      auto& insts = blockPtr->insts();
      auto it = std::find_if(
          insts.begin(), insts.end(),
          [store](const std::unique_ptr<MachineInst>& inst) {
            return inst.get() == store;
          });
      if (it != insts.end()) {
        insts.erase(it);
        break;
      }
    }
  }
  return !redundant.empty();
}

std::vector<std::unordered_set<int>> computeBlockDominators(MachineFunction& mf) {
  const int n = static_cast<int>(mf.blocks().size());
  std::unordered_map<MachineBasicBlock*, int> indexOf;
  indexOf.reserve(mf.blocks().size());
  for (int i = 0; i < n; ++i)
    indexOf[mf.blocks()[i].get()] = i;

  std::vector<std::unordered_set<int>> dom(n);
  if (n == 0)
    return dom;

  for (int i = 0; i < n; ++i) {
    if (i == 0) {
      dom[i].insert(i);
    } else {
      for (int j = 0; j < n; ++j)
        dom[i].insert(j);
    }
  }

  bool changed = true;
  while (changed) {
    changed = false;
    for (int i = 1; i < n; ++i) {
      MachineBasicBlock* bb = mf.blocks()[i].get();
      bool sawPred = false;
      std::unordered_set<int> next;
      for (auto* pred : bb->preds()) {
        auto predIt = indexOf.find(pred);
        if (predIt == indexOf.end())
          continue;
        if (!sawPred) {
          next = dom[predIt->second];
          sawPred = true;
          continue;
        }
        for (auto it = next.begin(); it != next.end();) {
          if (!dom[predIt->second].count(*it))
            it = next.erase(it);
          else
            ++it;
        }
      }
      if (!sawPred)
        next.clear();
      next.insert(i);
      if (next != dom[i]) {
        dom[i] = std::move(next);
        changed = true;
      }
    }
  }
  return dom;
}

std::vector<MachineLoop> collectNaturalLoops(MachineFunction& mf) {
  std::vector<MachineLoop> loops;
  const int n = static_cast<int>(mf.blocks().size());
  if (n <= 1)
    return loops;

  std::unordered_map<MachineBasicBlock*, int> indexOf;
  indexOf.reserve(mf.blocks().size());
  for (int i = 0; i < n; ++i)
    indexOf[mf.blocks()[i].get()] = i;

  auto dom = computeBlockDominators(mf);
  for (auto& tailPtr : mf.blocks()) {
    MachineBasicBlock* tail = tailPtr.get();
    auto tailIt = indexOf.find(tail);
    if (tailIt == indexOf.end())
      continue;
    for (auto* header : tail->succs()) {
      auto headerIt = indexOf.find(header);
      if (headerIt == indexOf.end())
        continue;
      if (!dom[tailIt->second].count(headerIt->second))
        continue;

      MachineLoop loop;
      loop.header = header;
      loop.blocks.insert(header);
      loop.blocks.insert(tail);
      std::vector<MachineBasicBlock*> worklist;
      if (tail != header)
        worklist.push_back(tail);
      while (!worklist.empty()) {
        MachineBasicBlock* cur = worklist.back();
        worklist.pop_back();
        for (auto* pred : cur->preds()) {
          if (!pred || loop.blocks.count(pred))
            continue;
          loop.blocks.insert(pred);
          if (pred != header)
            worklist.push_back(pred);
        }
      }

      MachineBasicBlock* preheader = nullptr;
      bool uniquePreheader = true;
      for (auto* pred : header->preds()) {
        if (!loop.blocks.count(pred)) {
          if (preheader && preheader != pred) {
            uniquePreheader = false;
            break;
          }
          preheader = pred;
        }
      }
      if (!uniquePreheader || !preheader)
        continue;
      loop.preheader = preheader;

      bool duplicate = false;
      for (const auto& existing : loops) {
        if (existing.header == loop.header &&
            existing.preheader == loop.preheader &&
            existing.blocks.size() == loop.blocks.size()) {
          bool same = true;
          for (auto* bb : orderedLoopBlocks(loop)) {
            if (!existing.blocks.count(bb)) {
              same = false;
              break;
            }
          }
          if (same) {
            duplicate = true;
            break;
          }
        }
      }
      if (!duplicate)
        loops.push_back(std::move(loop));
    }
  }

  std::sort(loops.begin(), loops.end(),
            [](const MachineLoop& a, const MachineLoop& b) {
              return a.blocks.size() < b.blocks.size();
            });
  return loops;
}

bool loopContainsCall(const MachineLoop& loop) {
  for (auto* bb : orderedLoopBlocks(loop)) {
    for (auto& miPtr : bb->insts()) {
      if (miPtr && mioIsCall(miPtr->opcode()))
        return true;
    }
  }
  return false;
}

bool regHasUseOutsideLoop(MachineFunction& mf,
                          const MachineLoop& loop,
                          int reg) {
  for (auto& bbPtr : mf.blocks()) {
    MachineBasicBlock* bb = bbPtr.get();
    const bool inLoop = loop.blocks.count(bb) != 0;
    for (auto& miPtr : bbPtr->insts()) {
      if (!miPtr)
        continue;
      for (const auto& op : miPtr->operands()) {
        if (op.isReg() && !op.isDef && op.reg == reg && !inLoop)
          return true;
      }
    }
  }
  return false;
}

void replaceRegUsesInLoop(const MachineLoop& loop, int oldReg, int newReg) {
  for (auto* bb : orderedLoopBlocks(loop)) {
    for (auto& miPtr : bb->insts()) {
      if (!miPtr)
        continue;
      for (auto& op : miPtr->operands()) {
        if (op.isReg() && !op.isDef && op.reg == oldReg)
          op.reg = newReg;
      }
    }
  }
}

MachineInst* firstControlTransfer(MachineBasicBlock* bb) {
  if (!bb)
    return nullptr;
  for (auto& miPtr : bb->insts()) {
    if (miPtr && mioIsTerminator(miPtr->opcode()))
      return miPtr.get();
  }
  return nullptr;
}

void insertBeforeTerminator(MachineBasicBlock* bb,
                            std::unique_ptr<MachineInst> inst) {
  if (!bb || !inst)
    return;
  MachineInst* term = firstControlTransfer(bb);
  if (term) {
    bb->insertBefore(term, std::move(inst));
  } else {
    bb->append(std::move(inst));
  }
}

bool loopContainsNestedLoop(const std::vector<MachineLoop>& loops,
                            size_t loopIdx) {
  const auto& loop = loops[loopIdx];
  for (size_t otherIdx = 0; otherIdx < loops.size(); ++otherIdx) {
    if (otherIdx == loopIdx)
      continue;
    const auto& other = loops[otherIdx];
    if (other.header && other.header != loop.header &&
        other.blocks.size() < loop.blocks.size() &&
        loop.blocks.count(other.header)) {
      return true;
    }
  }
  return false;
}

MachineBasicBlock* singleLatchBlock(const MachineLoop& loop) {
  if (!loop.header)
    return nullptr;
  MachineBasicBlock* latch = nullptr;
  for (auto* pred : loop.header->preds()) {
    if (!pred || !loop.blocks.count(pred))
      continue;
    if (std::find(pred->succs().begin(), pred->succs().end(), loop.header) ==
        pred->succs().end()) {
      continue;
    }
    if (latch && latch != pred)
      return nullptr;
    latch = pred;
  }
  return latch;
}

struct MachineDefUseInfo {
  std::unordered_map<int, MachineInst*> defs;
  std::unordered_map<int, std::vector<MachineInst*>> uses;
};

void traceRegDefTree(int reg, const MachineDefUseInfo& info, int depth,
                     std::unordered_set<int>& active) {
  if (!traceLoopAddressCleanup() || depth < 0)
    return;
  for (int i = 0; i < (4 - depth); ++i)
    std::fprintf(stderr, "  ");
  if (!active.insert(reg).second) {
    std::fprintf(stderr, "r%d cycle\n", reg);
    return;
  }
  MachineInst* def = nullptr;
  auto defIt = info.defs.find(reg);
  if (defIt != info.defs.end())
    def = defIt->second;
  std::fprintf(stderr, "r%d def=%s block=%s\n", reg,
               def ? mioName(def->opcode()) : "<none>",
               (def && def->parent()) ? def->parent()->label().c_str()
                                      : "<none>");
  if (def && depth > 0) {
    for (size_t opIdx = 1; opIdx < def->numOperands(); ++opIdx) {
      const auto& op = def->operand(opIdx);
      if (op.isReg())
        traceRegDefTree(op.reg, info, depth - 1, active);
    }
  }
  active.erase(reg);
}

MachineDefUseInfo collectDefUseInfo(MachineFunction& mf) {
  MachineDefUseInfo info;
  for (auto& bbPtr : mf.blocks()) {
    for (auto& miPtr : bbPtr->insts()) {
      MachineInst* mi = miPtr.get();
      if (!mi)
        continue;
      for (const auto& op : mi->operands()) {
        if (!op.isReg())
          continue;
        if (op.isDef) {
          info.defs[op.reg] = mi;
        } else {
          info.uses[op.reg].push_back(mi);
        }
      }
    }
  }
  return info;
}

bool regUsedOutsideLoop(const MachineDefUseInfo& info,
                        const MachineLoop& loop,
                        int reg) {
  auto it = info.uses.find(reg);
  if (it == info.uses.end())
    return false;
  for (MachineInst* use : it->second) {
    if (!use || !use->parent() || !loop.blocks.count(use->parent()))
      return true;
  }
  return false;
}

int64_t wrapI32Value(int64_t value) {
  return static_cast<int64_t>(
      static_cast<int32_t>(static_cast<uint32_t>(value)));
}

bool fitsAddiImm(int64_t value) {
  return value >= -2048 && value <= 2047;
}

struct KnownConstValue {
  bool valid = false;
  int64_t value = 0;
};

KnownConstValue evaluateKnownConstReg(
    int reg, const MachineDefUseInfo& info,
    std::unordered_map<int, KnownConstValue>& memo,
    std::unordered_set<int>& active) {
  auto memoIt = memo.find(reg);
  if (memoIt != memo.end())
    return memoIt->second;

  if (reg == gpr::x0) {
    KnownConstValue zero;
    zero.valid = true;
    zero.value = 0;
    memo[reg] = zero;
    return zero;
  }
  if (!isVirtualReg(reg) && !isPhysReg(reg))
    return memo[reg] = {};
  if (!active.insert(reg).second)
    return memo[reg] = {};

  KnownConstValue result;
  auto defIt = info.defs.find(reg);
  if (defIt == info.defs.end() || !defIt->second) {
    active.erase(reg);
    return memo[reg] = result;
  }

  MachineInst* def = defIt->second;
  // 沿定义链递归求寄存器的已知常量值；仅纯算术指令参与，
  // W 后缀指令的结果需按 32 位回绕。
  switch (def->opcode()) {
  case MIOp::LI:
    if (def->numOperands() == 2 && def->operand(1).isImm()) {
      result.valid = true;
      result.value = def->operand(1).imm;
    }
    break;
  case MIOp::MV:
    if (def->numOperands() == 2 && def->operand(1).isReg())
      result = evaluateKnownConstReg(def->operand(1).reg, info, memo, active);
    break;
  case MIOp::ADDI:
  case MIOp::ADDIW:
    if (def->numOperands() == 3 && def->operand(1).isReg() &&
        def->operand(2).isImm()) {
      result = evaluateKnownConstReg(def->operand(1).reg, info, memo, active);
      if (result.valid) {
        result.value += def->operand(2).imm;
        if (def->opcode() == MIOp::ADDIW)
          result.value = wrapI32Value(result.value);
      }
    }
    break;
  case MIOp::ADD:
  case MIOp::ADDW:
  case MIOp::SUB:
  case MIOp::SUBW:
  case MIOp::MUL:
  case MIOp::MULW:
    if (def->numOperands() == 3 && def->operand(1).isReg() &&
        def->operand(2).isReg()) {
      auto lhs = evaluateKnownConstReg(def->operand(1).reg, info, memo, active);
      auto rhs = evaluateKnownConstReg(def->operand(2).reg, info, memo, active);
      if (lhs.valid && rhs.valid) {
        result.valid = true;
        if (def->opcode() == MIOp::ADD || def->opcode() == MIOp::ADDW)
          result.value = lhs.value + rhs.value;
        else if (def->opcode() == MIOp::SUB || def->opcode() == MIOp::SUBW)
          result.value = lhs.value - rhs.value;
        else
          result.value = lhs.value * rhs.value;
        if (def->opcode() == MIOp::ADDW || def->opcode() == MIOp::SUBW ||
            def->opcode() == MIOp::MULW) {
          result.value = wrapI32Value(result.value);
        }
      }
    }
    break;
  case MIOp::SLLI:
  case MIOp::SLLIW:
    if (def->numOperands() == 3 && def->operand(1).isReg() &&
        def->operand(2).isImm() && def->operand(2).imm >= 0 &&
        def->operand(2).imm < 32) {
      result = evaluateKnownConstReg(def->operand(1).reg, info, memo, active);
      if (result.valid) {
        result.value <<= def->operand(2).imm;
        if (def->opcode() == MIOp::SLLIW)
          result.value = wrapI32Value(result.value);
      }
    }
    break;
  default:
    break;
  }

  active.erase(reg);
  memo[reg] = result;
  return result;
}

struct LinearAddressMatch {
  bool valid = false;
  bool dependsOnIv = false;
  int64_t stepImm = 0;
  std::unordered_set<int> loopRegs;
  std::vector<MachineInst*> loopInsts;
};

void mergeLinearMatch(LinearAddressMatch& dst, const LinearAddressMatch& src) {
  for (int reg : src.loopRegs)
    dst.loopRegs.insert(reg);
  for (MachineInst* inst : src.loopInsts) {
    if (!inst)
      continue;
    if (std::find(dst.loopInsts.begin(), dst.loopInsts.end(), inst) ==
        dst.loopInsts.end()) {
      dst.loopInsts.push_back(inst);
    }
  }
}

LinearAddressMatch matchLoopLinearAddressReg(
    int reg, int ivReg, const MachineLoop& loop, const MachineDefUseInfo& info,
    std::unordered_map<int, LinearAddressMatch>& memo,
    std::unordered_set<int>& active) {
  if (reg == ivReg)
    return {true, true, 1, {}, {}};

  auto memoIt = memo.find(reg);
  if (memoIt != memo.end())
    return memoIt->second;

  MachineInst* def = nullptr;
  auto defIt = info.defs.find(reg);
  if (defIt != info.defs.end())
    def = defIt->second;
  if (!def || !def->parent() || !loop.blocks.count(def->parent()))
    return memo[reg] = {true, false, 0, {}, {}};
  if (!active.insert(reg).second)
    return memo[reg] = {};

  LinearAddressMatch result;
  auto finish = [&](LinearAddressMatch match) {
    active.erase(reg);
    memo[reg] = std::move(match);
    return memo[reg];
  };

  auto finalizeLoopDef = [&](LinearAddressMatch& match) {
    if (!match.valid)
      return;
    match.loopRegs.insert(reg);
    match.loopInsts.push_back(def);
  };

  // 把地址寄存器匹配成“循环不变部分 + 归纳变量 x 步长”的线性形式；
  // 移位/乘法会按比例缩放步长，SUB 仅允许单侧依赖归纳变量。
  switch (def->opcode()) {
  case MIOp::LI:
  case MIOp::LA:
    result.valid = true;
    break;
  case MIOp::MV:
    if (def->numOperands() == 2 && def->operand(1).isReg()) {
      result = matchLoopLinearAddressReg(def->operand(1).reg, ivReg, loop, info,
                                         memo, active);
    }
    break;
  case MIOp::ADDI:
  case MIOp::ADDIW:
    if (def->numOperands() == 3 && def->operand(1).isReg() &&
        def->operand(2).isImm()) {
      result = matchLoopLinearAddressReg(def->operand(1).reg, ivReg, loop, info,
                                         memo, active);
      if (result.valid && result.dependsOnIv &&
          def->opcode() == MIOp::ADDIW) {
        result.stepImm = wrapI32Value(result.stepImm);
      }
    }
    break;
  case MIOp::ADD:
  case MIOp::ADDW:
  case MIOp::SUB:
  case MIOp::SUBW:
    if (def->numOperands() == 3 && def->operand(1).isReg() &&
        def->operand(2).isReg()) {
      auto lhs = matchLoopLinearAddressReg(def->operand(1).reg, ivReg, loop,
                                           info, memo, active);
      auto rhs = matchLoopLinearAddressReg(def->operand(2).reg, ivReg, loop,
                                           info, memo, active);
      if (!lhs.valid || !rhs.valid)
        break;
      mergeLinearMatch(result, lhs);
      mergeLinearMatch(result, rhs);
      if (def->opcode() == MIOp::ADD || def->opcode() == MIOp::ADDW) {
        if (lhs.dependsOnIv == rhs.dependsOnIv) {
          if (!lhs.dependsOnIv)
            result.valid = true;
          break;
        }
        result.valid = true;
        result.dependsOnIv = true;
        result.stepImm = lhs.dependsOnIv ? lhs.stepImm : rhs.stepImm;
      } else {
        if (lhs.dependsOnIv || rhs.dependsOnIv) {
          if (!lhs.dependsOnIv || rhs.dependsOnIv)
            break;
          result.valid = true;
          result.dependsOnIv = true;
          result.stepImm = lhs.stepImm;
        } else {
          result.valid = true;
        }
      }
      if (result.valid && result.dependsOnIv &&
          (def->opcode() == MIOp::ADDW || def->opcode() == MIOp::SUBW)) {
        result.stepImm = wrapI32Value(result.stepImm);
      }
    }
    break;
  case MIOp::SLLI:
  case MIOp::SLLIW:
    if (def->numOperands() == 3 && def->operand(1).isReg() &&
        def->operand(2).isImm() && def->operand(2).imm >= 0 &&
        def->operand(2).imm < 32) {
      result = matchLoopLinearAddressReg(def->operand(1).reg, ivReg, loop, info,
                                         memo, active);
      if (result.valid && result.dependsOnIv) {
        result.stepImm <<= def->operand(2).imm;
        if (def->opcode() == MIOp::SLLIW)
          result.stepImm = wrapI32Value(result.stepImm);
      }
    }
    break;
  case MIOp::MUL:
  case MIOp::MULW:
    if (def->numOperands() == 3 && def->operand(1).isReg() &&
        def->operand(2).isReg()) {
      auto lhs = matchLoopLinearAddressReg(def->operand(1).reg, ivReg, loop,
                                           info, memo, active);
      auto rhs = matchLoopLinearAddressReg(def->operand(2).reg, ivReg, loop,
                                           info, memo, active);
      if (!lhs.valid || !rhs.valid)
        break;
      mergeLinearMatch(result, lhs);
      mergeLinearMatch(result, rhs);
      if (lhs.dependsOnIv == rhs.dependsOnIv) {
        if (!lhs.dependsOnIv)
          result.valid = true;
        break;
      }
      const int constReg =
          lhs.dependsOnIv ? def->operand(2).reg : def->operand(1).reg;
      std::unordered_map<int, KnownConstValue> constMemo;
      std::unordered_set<int> constActive;
      auto knownConst =
          evaluateKnownConstReg(constReg, info, constMemo, constActive);
      if (!knownConst.valid)
        break;
      result.valid = true;
      result.dependsOnIv = true;
      result.stepImm =
          (lhs.dependsOnIv ? lhs.stepImm : rhs.stepImm) * knownConst.value;
      if (def->opcode() == MIOp::MULW)
        result.stepImm = wrapI32Value(result.stepImm);
    }
    break;
  default:
    break;
  }

  finalizeLoopDef(result);
  return finish(result);
}

bool buildLoopAddressSignature(int reg, int ivReg, const MachineLoop& loop,
                               const MachineDefUseInfo& info,
                               std::unordered_map<int, std::string>& memo,
                               std::unordered_set<int>& active,
                               std::string& out) {
  if (reg == ivReg) {
    out = "IV";
    return true;
  }
  auto memoIt = memo.find(reg);
  if (memoIt != memo.end()) {
    out = memoIt->second;
    return !out.empty();
  }

  MachineInst* def = nullptr;
  auto defIt = info.defs.find(reg);
  if (defIt != info.defs.end())
    def = defIt->second;
  if (!def || !def->parent() || !loop.blocks.count(def->parent())) {
    if (reg == gpr::x0)
      out = "#0";
    else
      out = "R" + std::to_string(reg);
    memo[reg] = out;
    return true;
  }
  if (!active.insert(reg).second)
    return false;

  auto finish = [&](std::string value) {
    active.erase(reg);
    memo[reg] = std::move(value);
    out = memo[reg];
    return !out.empty();
  };

  auto buildUnary = [&](const char* name, int useReg, int64_t imm = 0,
                        bool hasImm = false) {
    std::string src;
    if (!buildLoopAddressSignature(useReg, ivReg, loop, info, memo, active,
                                   src)) {
      return false;
    }
    return finish(hasImm ? std::string(name) + "(" + src + "," +
                             std::to_string(imm) + ")"
                         : std::string(name) + "(" + src + ")");
  };

  auto buildBinary = [&](const char* name, int lhsReg, int rhsReg,
                         bool commutative) {
    std::string lhs;
    std::string rhs;
    if (!buildLoopAddressSignature(lhsReg, ivReg, loop, info, memo, active,
                                   lhs) ||
        !buildLoopAddressSignature(rhsReg, ivReg, loop, info, memo, active,
                                   rhs)) {
      return false;
    }
    if (commutative && rhs < lhs)
      std::swap(lhs, rhs);
    return finish(std::string(name) + "(" + lhs + "," + rhs + ")");
  };

  bool ok = false;
  // 为地址表达式构造可读字符串签名：常量以 # 前缀、符号以 @ 前缀，
  // 算术节点按操作码递归展开。
  switch (def->opcode()) {
  case MIOp::LI:
    if (def->numOperands() == 2 && def->operand(1).isImm())
      ok = finish("#" + std::to_string(def->operand(1).imm));
    break;
  case MIOp::LA:
    if (def->numOperands() == 2 && def->operand(1).isSym())
      ok = finish("@" + def->operand(1).sym);
    break;
  case MIOp::MV:
    if (def->numOperands() == 2 && def->operand(1).isReg()) {
      std::string src;
      if (buildLoopAddressSignature(def->operand(1).reg, ivReg, loop, info,
                                    memo, active, src))
        ok = finish(src);
    }
    break;
  case MIOp::ADDI:
  case MIOp::ADDIW:
    if (def->numOperands() == 3 && def->operand(1).isReg() &&
        def->operand(2).isImm()) {
      ok = buildUnary(mioName(def->opcode()), def->operand(1).reg,
                      def->operand(2).imm, true);
    }
    break;
  case MIOp::ADD:
  case MIOp::ADDW:
  case MIOp::MUL:
  case MIOp::MULW:
    if (def->numOperands() == 3 && def->operand(1).isReg() &&
        def->operand(2).isReg()) {
      ok = buildBinary(mioName(def->opcode()), def->operand(1).reg,
                       def->operand(2).reg, true);
    }
    break;
  case MIOp::SUB:
  case MIOp::SUBW:
    if (def->numOperands() == 3 && def->operand(1).isReg() &&
        def->operand(2).isReg()) {
      ok = buildBinary(mioName(def->opcode()), def->operand(1).reg,
                       def->operand(2).reg, false);
    }
    break;
  case MIOp::SLLI:
  case MIOp::SLLIW:
    if (def->numOperands() == 3 && def->operand(1).isReg() &&
        def->operand(2).isImm()) {
      ok = buildUnary(mioName(def->opcode()), def->operand(1).reg,
                      def->operand(2).imm, true);
    }
    break;
  default:
    break;
  }
  if (!ok)
    active.erase(reg);
  return ok;
}

struct NormalizedAddressSignature {
  bool valid = false;
  std::string key;
  int64_t constOffset = 0;
  bool pureConst = false;
  int64_t pureValue = 0;
};

std::string combineAddressKeys(const char* op, const std::string& lhs,
                               const std::string& rhs,
                               bool commutative) {
  if (lhs.empty())
    return rhs;
  if (rhs.empty())
    return lhs;
  std::string a = lhs;
  std::string b = rhs;
  if (commutative && b < a)
    std::swap(a, b);
  return std::string(op) + "(" + a + "," + b + ")";
}

std::string scaledAddressKey(const std::string& key, int64_t scale) {
  if (key.empty() || scale == 1)
    return key;
  return "mulc(" + key + "," + std::to_string(scale) + ")";
}

NormalizedAddressSignature buildNormalizedAddressSignature(
    int reg, int ivReg, const MachineLoop& loop, const MachineDefUseInfo& info,
    std::unordered_map<int, NormalizedAddressSignature>& memo,
    std::unordered_set<int>& active) {
  if (reg == ivReg)
    return {true, "IV", 0, false, 0};

  auto memoIt = memo.find(reg);
  if (memoIt != memo.end())
    return memoIt->second;

  MachineInst* def = nullptr;
  auto defIt = info.defs.find(reg);
  if (defIt != info.defs.end())
    def = defIt->second;
  if (!def || !def->parent() || !loop.blocks.count(def->parent())) {
    NormalizedAddressSignature out;
    out.valid = true;
    if (reg == gpr::x0) {
      out.pureConst = true;
      out.pureValue = 0;
    } else {
      out.key = "R" + std::to_string(reg);
    }
    memo[reg] = out;
    return out;
  }
  if (!active.insert(reg).second)
    return {};

  auto finish = [&](NormalizedAddressSignature out) {
    active.erase(reg);
    memo[reg] = std::move(out);
    return memo[reg];
  };

  auto exactKeyFor = [&](int exactReg) -> std::string {
    std::unordered_map<int, std::string> exactMemo;
    std::unordered_set<int> exactActive;
    std::string exact;
    if (!buildLoopAddressSignature(exactReg, ivReg, loop, info, exactMemo,
                                   exactActive, exact)) {
      return {};
    }
    return exact;
  };

  NormalizedAddressSignature out;
  // 把地址表达式规范化为“符号 key + 常量偏移”形式；
  // 纯常量表达式直接折出数值，乘法按常量侧/零偏移等分支归并 key。
  switch (def->opcode()) {
  case MIOp::LI:
    if (def->numOperands() == 2 && def->operand(1).isImm()) {
      out.valid = true;
      out.pureConst = true;
      out.pureValue = def->operand(1).imm;
      out.constOffset = def->operand(1).imm;
    }
    break;
  case MIOp::LA:
    if (def->numOperands() == 2 && def->operand(1).isSym()) {
      out.valid = true;
      out.key = "@" + def->operand(1).sym;
    }
    break;
  case MIOp::MV:
    if (def->numOperands() == 2 && def->operand(1).isReg()) {
      out = buildNormalizedAddressSignature(def->operand(1).reg, ivReg, loop,
                                            info, memo, active);
    }
    break;
  case MIOp::ADDI:
  case MIOp::ADDIW:
    if (def->numOperands() == 3 && def->operand(1).isReg() &&
        def->operand(2).isImm()) {
      out = buildNormalizedAddressSignature(def->operand(1).reg, ivReg, loop,
                                            info, memo, active);
      if (out.valid) {
        out.constOffset += def->operand(2).imm;
        if (out.pureConst)
          out.pureValue += def->operand(2).imm;
      }
    }
    break;
  case MIOp::ADD:
  case MIOp::ADDW:
  case MIOp::SUB:
  case MIOp::SUBW:
    if (def->numOperands() == 3 && def->operand(1).isReg() &&
        def->operand(2).isReg()) {
      auto lhs = buildNormalizedAddressSignature(def->operand(1).reg, ivReg,
                                                 loop, info, memo, active);
      auto rhs = buildNormalizedAddressSignature(def->operand(2).reg, ivReg,
                                                 loop, info, memo, active);
      if (!lhs.valid || !rhs.valid)
        break;
      out.valid = true;
      bool isSub = def->opcode() == MIOp::SUB || def->opcode() == MIOp::SUBW;
      out.constOffset = isSub ? lhs.constOffset - rhs.constOffset
                              : lhs.constOffset + rhs.constOffset;
      out.pureConst = lhs.pureConst && rhs.pureConst;
      if (out.pureConst)
        out.pureValue = isSub ? lhs.pureValue - rhs.pureValue
                              : lhs.pureValue + rhs.pureValue;
      out.key = combineAddressKeys(isSub ? "sub" : "add", lhs.key, rhs.key,
                                   !isSub);
    }
    break;
  case MIOp::SLLI:
  case MIOp::SLLIW:
    if (def->numOperands() == 3 && def->operand(1).isReg() &&
        def->operand(2).isImm() && def->operand(2).imm >= 0 &&
        def->operand(2).imm < 32) {
      out = buildNormalizedAddressSignature(def->operand(1).reg, ivReg, loop,
                                            info, memo, active);
      if (out.valid) {
        out.constOffset <<= def->operand(2).imm;
        if (out.pureConst)
          out.pureValue <<= def->operand(2).imm;
        if (!out.key.empty()) {
          out.key = std::string(mioName(def->opcode())) + "(" + out.key + "," +
                    std::to_string(def->operand(2).imm) + ")";
        }
      }
    }
    break;
  case MIOp::MUL:
  case MIOp::MULW:
    if (def->numOperands() == 3 && def->operand(1).isReg() &&
        def->operand(2).isReg()) {
      auto lhs = buildNormalizedAddressSignature(def->operand(1).reg, ivReg,
                                                 loop, info, memo, active);
      auto rhs = buildNormalizedAddressSignature(def->operand(2).reg, ivReg,
                                                 loop, info, memo, active);
      if (!lhs.valid || !rhs.valid)
        break;
      out.valid = true;
      if (lhs.pureConst && rhs.pureConst) {
        out.pureConst = true;
        out.pureValue = lhs.pureValue * rhs.pureValue;
        out.constOffset = out.pureValue;
      } else if (lhs.pureConst || rhs.pureConst) {
        const auto& c = lhs.pureConst ? lhs : rhs;
        const auto& expr = lhs.pureConst ? rhs : lhs;
        out.constOffset = expr.constOffset * c.pureValue;
        out.key = scaledAddressKey(expr.key, c.pureValue);
      } else if (lhs.constOffset == 0 && rhs.constOffset == 0) {
        out.key = combineAddressKeys(mioName(def->opcode()), lhs.key, rhs.key,
                                     true);
      } else {
        std::string lhsExact = exactKeyFor(def->operand(1).reg);
        std::string rhsExact = exactKeyFor(def->operand(2).reg);
        if (lhsExact.empty() || rhsExact.empty()) {
          out.valid = false;
          break;
        }
        out.key = combineAddressKeys(mioName(def->opcode()), lhsExact,
                                     rhsExact, true);
        out.constOffset = 0;
      }
    }
    break;
  default:
    break;
  }

  return finish(out);
}

int cloneLoopAddressExprReg(
    int reg, int ivReg, int initReg, MachineBasicBlock* preheader,
    const MachineLoop& loop, const MachineDefUseInfo& info,
    std::unordered_map<int, int>& cloneMemo) {
  if (reg == ivReg)
    return initReg;

  auto memoIt = cloneMemo.find(reg);
  if (memoIt != cloneMemo.end())
    return memoIt->second;

  MachineInst* def = nullptr;
  auto defIt = info.defs.find(reg);
  if (defIt != info.defs.end())
    def = defIt->second;
  if (!def || !def->parent() || !loop.blocks.count(def->parent())) {
    cloneMemo[reg] = reg;
    return reg;
  }

  auto* mf = preheader ? preheader->parent() : nullptr;
  if (!mf || def->numOperands() == 0 || !def->operand(0).isReg() ||
      !def->operand(0).isDef) {
    return -1;
  }

  // 克隆指令本体：寄存器操作数递归克隆到 preheader，
  // 立即数/符号/栈槽/块引用等操作数原样复制。
  auto clone = std::make_unique<MachineInst>(def->opcode());
  const RegClass cls = def->operand(0).regClass;
  const int newReg = mf->newVReg(cls);
  clone->addOperand(MOperand::Def(newReg, cls));
  for (size_t i = 1; i < def->numOperands(); ++i) {
    const MOperand& op = def->operand(i);
    if (op.isReg()) {
      int clonedUse =
          cloneLoopAddressExprReg(op.reg, ivReg, initReg, preheader, loop, info,
                                  cloneMemo);
      if (clonedUse < 0)
        return -1;
      clone->addOperand(MOperand::Use(clonedUse, op.regClass));
      continue;
    }
    if (op.isImm()) {
      clone->addOperand(MOperand::Imm(op.imm));
      continue;
    }
    if (op.isSym()) {
      clone->addOperand(MOperand::Sym(op.sym));
      continue;
    }
    if (op.isFrame()) {
      clone->addOperand(MOperand::Frame(op.frameIdx));
      continue;
    }
    if (op.isMBB()) {
      clone->addOperand(MOperand::BB(op.mbb));
      continue;
    }
    if (op.isFImm()) {
      clone->addOperand(MOperand::FImm(op.fimm));
      continue;
    }
    return -1;
  }
  clone->comment = "loop.addr.init";
  insertBeforeTerminator(preheader, std::move(clone));
  cloneMemo[reg] = newReg;
  return newReg;
}

void insertPhiAtBlockTop(MachineBasicBlock* bb,
                         std::unique_ptr<MachineInst> phi) {
  if (!bb || !phi)
    return;
  for (auto& miPtr : bb->insts()) {
    if (!miPtr)
      continue;
    if (miPtr->opcode() != MIOp::PHI) {
      bb->insertBefore(miPtr.get(), std::move(phi));
      return;
    }
  }
  bb->prepend(std::move(phi));
}

struct LoopIVInfo {
  int phiReg = -1;
  int initReg = -1;
  int64_t iterStepImm = 0;
};

std::vector<LoopIVInfo> collectSingleStepLoopIVs(const MachineLoop& loop,
                                                 MachineBasicBlock* latch,
                                                 const MachineDefUseInfo& info) {
  std::vector<LoopIVInfo> ivs;
  if (!loop.header || !loop.preheader || !latch)
    return ivs;

  // 逐个检查循环头顶部 PHI：仅接受形状为五操作数、且输入对
  // 恰好分别来自 preheader 与 latch 的归纳变量。
  for (auto& miPtr : loop.header->insts()) {
    MachineInst* phi = miPtr.get();
    if (!phi || phi->opcode() != MIOp::PHI)
      break;
    if (traceLoopAddressCleanup()) {
      std::fprintf(stderr,
                   "[mir-loop-addr] header=%s phi r%d operands=%zu\n",
                   loop.header->label().c_str(),
                   (phi->numOperands() > 0 && phi->operand(0).isReg())
                       ? phi->operand(0).reg
                       : -1,
                   phi->numOperands());
    }
    if (phi->numOperands() != 5 || !phi->operand(0).isReg() ||
        !phi->operand(0).isDef) {
      if (traceLoopAddressCleanup()) {
        std::fprintf(stderr,
                     "[mir-loop-addr] header=%s phi-skip=shape\n",
                     loop.header->label().c_str());
      }
      continue;
    }
    int initReg = -1;
    int latchReg = -1;
    bool validPairs = true;
    for (size_t i = 1; i + 1 < phi->numOperands(); i += 2) {
      if (!phi->operand(i).isReg() || !phi->operand(i + 1).isMBB()) {
        validPairs = false;
        break;
      }
      if (phi->operand(i + 1).mbb == loop.preheader) {
        initReg = phi->operand(i).reg;
      } else if (phi->operand(i + 1).mbb == latch) {
        latchReg = phi->operand(i).reg;
      } else {
        validPairs = false;
        break;
      }
    }
    if (!validPairs || initReg < 0 || latchReg < 0)
    {
      if (traceLoopAddressCleanup()) {
        std::fprintf(stderr,
                     "[mir-loop-addr] header=%s phi r%d skip=pairs init=%d "
                     "latch=%d\n",
                     loop.header->label().c_str(), phi->operand(0).reg,
                     initReg, latchReg);
      }
      continue;
    }

    auto stepIt = info.defs.find(latchReg);
    if (stepIt == info.defs.end() || !stepIt->second ||
        !stepIt->second->parent() ||
        !loop.blocks.count(stepIt->second->parent())) {
      if (traceLoopAddressCleanup()) {
        std::fprintf(stderr,
                     "[mir-loop-addr] header=%s phi r%d skip=step-loop "
                     "latch-reg=%d def=%s block=%s latch=%s\n",
                     loop.header->label().c_str(), phi->operand(0).reg,
                     latchReg,
                     (stepIt != info.defs.end() && stepIt->second)
                         ? mioName(stepIt->second->opcode())
                         : "<none>",
                     (stepIt != info.defs.end() && stepIt->second &&
                      stepIt->second->parent())
                         ? stepIt->second->parent()->label().c_str()
                         : "<none>",
                     latch ? latch->label().c_str() : "<none>");
      }
      continue;
    }
    MachineInst* step = stepIt->second;
    if ((step->opcode() != MIOp::ADDI && step->opcode() != MIOp::ADDIW) ||
        step->numOperands() != 3 || !step->operand(1).isReg() ||
        !step->operand(2).isImm() ||
        step->operand(1).reg != phi->operand(0).reg ||
        step->operand(2).imm == 0) {
      if (traceLoopAddressCleanup()) {
        std::fprintf(stderr,
                     "[mir-loop-addr] header=%s phi r%d skip=step-shape "
                     "def=%s\n",
                     loop.header->label().c_str(), phi->operand(0).reg,
                     mioName(step->opcode()));
      }
      continue;
    }

    LoopIVInfo infoEntry;
    infoEntry.phiReg = phi->operand(0).reg;
    infoEntry.initReg = initReg;
    infoEntry.iterStepImm = step->operand(2).imm;
    if (traceLoopAddressCleanup()) {
      std::fprintf(stderr,
                   "[mir-loop-addr] header=%s phi r%d accept step=%lld\n",
                   loop.header->label().c_str(), infoEntry.phiReg,
                   static_cast<long long>(infoEntry.iterStepImm));
    }
    ivs.push_back(infoEntry);
  }
  return ivs;
}

struct LoopAddressGroupPlan {
  LoopIVInfo iv;
  std::string signature;
  int representativeReg = -1;
  int64_t stepImm = 0;
  int64_t baseOffset = 0;
  size_t useCount = 0;
  size_t expressionCost = 0;
  std::unordered_set<int> memberRegs;
  std::unordered_map<int, int64_t> memberOffsets;
};

bool findBestLoopAddressGroup(const MachineLoop& loop,
                              MachineBasicBlock* latch,
                              const MachineDefUseInfo& info,
                              LoopAddressGroupPlan& bestPlan) {
  auto ivs = collectSingleStepLoopIVs(loop, latch, info);
  if (ivs.empty()) {
    if (traceLoopAddressCleanup()) {
      std::fprintf(stderr, "[mir-loop-addr] header=%s no-iv\n",
                   loop.header ? loop.header->label().c_str() : "<null>");
    }
    return false;
  }

  struct GroupState {
    LoopAddressGroupPlan plan;
    std::unordered_set<int> seenRegs;
  };
  std::unordered_map<std::string, GroupState> groups;

  auto scoreBetter = [](const LoopAddressGroupPlan& lhs,
                        const LoopAddressGroupPlan& rhs) {
    if (lhs.useCount != rhs.useCount)
      return lhs.useCount > rhs.useCount;
    if (lhs.memberRegs.size() != rhs.memberRegs.size())
      return lhs.memberRegs.size() > rhs.memberRegs.size();
    if (lhs.expressionCost != rhs.expressionCost)
      return lhs.expressionCost > rhs.expressionCost;
    if (lhs.stepImm != rhs.stepImm)
      return std::llabs(lhs.stepImm) > std::llabs(rhs.stepImm);
    return lhs.signature < rhs.signature;
  };

  bool found = false;
  // 预先统计循环中的内存操作数量。访问较多的大循环会因额外指针 phi
  // 增加寄存器压力，因此单例地址递增只处理紧凑循环。
  size_t loopMemOps = 0;
  for (auto* bb : orderedLoopBlocks(loop)) {
    if (!bb)
      continue;
    for (auto& miPtr : bb->insts()) {
      MachineInst* m = miPtr.get();
      if (m && (mioMayLoad(m->opcode()) || mioMayStore(m->opcode())))
        ++loopMemOps;
    }
  }
  size_t memOpsSeen = 0;
  size_t badMemShape = 0;
  size_t invalidMatch = 0;
  size_t noIvDependence = 0;
  size_t emptyLoopRegs = 0;
  size_t addrIsIv = 0;
  size_t zeroStep = 0;
  size_t badStepImm = 0;
  size_t addrOutside = 0;
  size_t chainEscapes = 0;
  size_t signatureFails = 0;
  bool printedFirstInvalid = false;
  for (const auto& iv : ivs) {
    std::unordered_map<int, LinearAddressMatch> matchMemo;
    std::unordered_set<int> matchActive;
    std::unordered_map<int, NormalizedAddressSignature> sigMemo;
    std::unordered_set<int> sigActive;

    for (auto* bb : orderedLoopBlocks(loop)) {
      if (!bb)
        continue;
      for (auto& miPtr : bb->insts()) {
        MachineInst* mi = miPtr.get();
        if (!mi || !(mioMayLoad(mi->opcode()) || mioMayStore(mi->opcode())))
          continue;
        ++memOpsSeen;
        if (mi->numOperands() < 3 || !mi->operand(2).isReg()) {
          ++badMemShape;
          continue;
        }

        // 对每条访存指令做可提外筛选：匹配须成功、依赖归纳变量、
        // 指针步长非零且落在 ADDI 立即数范围内，且地址链整体不逃出循环。
        const int addrReg = mi->operand(2).reg;
        auto match = matchLoopLinearAddressReg(addrReg, iv.phiReg, loop, info,
                                               matchMemo, matchActive);
        const int64_t pointerStepImm = match.stepImm * iv.iterStepImm;
        if (!match.valid) {
          ++invalidMatch;
          if (traceLoopAddressCleanup() && !printedFirstInvalid) {
            MachineInst* addrDef = nullptr;
            auto addrDefIt = info.defs.find(addrReg);
            if (addrDefIt != info.defs.end())
              addrDef = addrDefIt->second;
            std::fprintf(stderr,
                         "[mir-loop-addr] header=%s first-invalid addr=r%d "
                         "iv=r%d def=%s\n",
                         loop.header ? loop.header->label().c_str()
                                     : "<null>",
                         addrReg, iv.phiReg,
                         addrDef ? mioName(addrDef->opcode()) : "<none>");
            if (addrDef) {
              for (size_t opIdx = 1; opIdx < addrDef->numOperands();
                   ++opIdx) {
                const auto& op = addrDef->operand(opIdx);
                if (!op.isReg())
                  continue;
                MachineInst* opDef = nullptr;
                auto opDefIt = info.defs.find(op.reg);
                if (opDefIt != info.defs.end())
                  opDef = opDefIt->second;
                std::fprintf(stderr,
                             "[mir-loop-addr]   use%zu=r%d def=%s block=%s\n",
                             opIdx, op.reg,
                             opDef ? mioName(opDef->opcode()) : "<none>",
                             (opDef && opDef->parent())
                                 ? opDef->parent()->label().c_str()
                                 : "<none>");
              }
            }
            std::unordered_set<int> treeActive;
            traceRegDefTree(addrReg, info, 4, treeActive);
            printedFirstInvalid = true;
          }
          continue;
        }
        if (!match.dependsOnIv) {
          ++noIvDependence;
          continue;
        }
        if (match.loopRegs.empty()) {
          ++emptyLoopRegs;
          continue;
        }
        if (addrReg == iv.phiReg) {
          ++addrIsIv;
          continue;
        }
        if (pointerStepImm == 0) {
          ++zeroStep;
          continue;
        }
        if (!fitsAddiImm(pointerStepImm)) {
          ++badStepImm;
          continue;
        }
        if (regUsedOutsideLoop(info, loop, addrReg)) {
          ++addrOutside;
          continue;
        }
        bool chainEscapesLoop = false;
        for (int chainReg : match.loopRegs) {
          if (regUsedOutsideLoop(info, loop, chainReg)) {
            chainEscapesLoop = true;
            break;
          }
        }
        if (chainEscapesLoop) {
          ++chainEscapes;
          continue;
        }

        auto signature = buildNormalizedAddressSignature(
            addrReg, iv.phiReg, loop, info, sigMemo, sigActive);
        if (!signature.valid || signature.key.empty()) {
          ++signatureFails;
          continue;
        }

        const std::string groupKey =
            std::to_string(iv.phiReg) + "|" + std::to_string(pointerStepImm) +
            "|" + signature.key;
        auto& group = groups[groupKey];
        if (group.plan.useCount == 0) {
          group.plan.iv = iv;
          group.plan.signature = signature.key;
          group.plan.representativeReg = addrReg;
          group.plan.stepImm = pointerStepImm;
          group.plan.baseOffset = signature.constOffset;
        }
        group.plan.useCount++;
        group.plan.expressionCost =
            std::max(group.plan.expressionCost, match.loopInsts.size());
        if (group.seenRegs.insert(addrReg).second) {
          group.plan.memberRegs.insert(addrReg);
          group.plan.memberOffsets[addrReg] =
              signature.constOffset - group.plan.baseOffset;
        }
        // 单一连续地址本身已经很便宜，继续携带指针可能增加寄存器压力。只有
        // 替换每次迭代至少包含三条指令的非简单表达式时才保留；紧凑循环还会
        // 限制单例地址提升，避免额外指针 PHI 带来的压力超过逐迭代节省。
        const bool profitableSingleton =
            group.plan.useCount == 1 && group.plan.expressionCost >= 3 &&
            std::llabs(group.plan.stepImm) >= 4 && loopMemOps <= 16;
        if ((group.plan.useCount >= 2 || profitableSingleton) &&
            (!found || scoreBetter(group.plan, bestPlan))) {
          bestPlan = group.plan;
          found = true;
        }
      }
    }
  }
  if (traceLoopAddressCleanup()) {
    std::fprintf(stderr,
                 "[mir-loop-addr] header=%s ivs=%zu mem=%zu bad-shape=%zu "
                 "invalid=%zu no-iv=%zu empty-regs=%zu addr-iv=%zu "
                 "zero-step=%zu bad-imm=%zu addr-out=%zu chain-out=%zu "
                 "sig-fail=%zu found=%d\n",
                 loop.header ? loop.header->label().c_str() : "<null>",
                 ivs.size(), memOpsSeen, badMemShape, invalidMatch,
                 noIvDependence, emptyLoopRegs, addrIsIv, zeroStep, badStepImm,
                 addrOutside, chainEscapes, signatureFails, found ? 1 : 0);
  }
  return found;
}

bool addressRegsOnlyUsedAsMemoryBases(const MachineDefUseInfo& info,
                                      const MachineLoop& loop,
                                      const LoopAddressGroupPlan& plan) {
  for (int reg : plan.memberRegs) {
    auto useIt = info.uses.find(reg);
    if (useIt == info.uses.end())
      return false;
    for (MachineInst* use : useIt->second) {
      if (!use || !use->parent() || !loop.blocks.count(use->parent()) ||
          !(mioMayLoad(use->opcode()) || mioMayStore(use->opcode())) ||
          use->numOperands() < 3 || !use->operand(1).isImm() ||
          !use->operand(2).isReg() || use->operand(2).reg != reg) {
        return false;
      }
      auto offIt = plan.memberOffsets.find(reg);
      if (offIt == plan.memberOffsets.end())
        return false;
      if (!fitsAddiImm(use->operand(1).imm + offIt->second))
        return false;
    }
  }
  return true;
}

void rewriteLoopAddressGroupMemoryUses(const MachineLoop& loop,
                                       const LoopAddressGroupPlan& plan,
                                       int ptrPhiReg) {
  for (auto* bb : orderedLoopBlocks(loop)) {
    if (!bb)
      continue;
    for (auto& miPtr : bb->insts()) {
      MachineInst* mi = miPtr.get();
      if (!mi || !(mioMayLoad(mi->opcode()) || mioMayStore(mi->opcode())) ||
          mi->numOperands() < 3 || !mi->operand(1).isImm() ||
          !mi->operand(2).isReg()) {
        continue;
      }
      auto offIt = plan.memberOffsets.find(mi->operand(2).reg);
      if (offIt == plan.memberOffsets.end())
        continue;
      mi->operand(1).imm += offIt->second;
      mi->operand(2).reg = ptrPhiReg;
    }
  }
}

bool applyLoopAddressGroup(MachineFunction& mf, const MachineLoop& loop,
                           MachineBasicBlock* latch,
                           const MachineDefUseInfo& info,
                           const LoopAddressGroupPlan& plan) {
  if (!loop.header || !loop.preheader || !latch || plan.representativeReg < 0 ||
      plan.memberRegs.empty() ||
      (plan.useCount < 2 &&
       (plan.expressionCost < 3 || std::llabs(plan.stepImm) < 4)) ||
      !fitsAddiImm(plan.stepImm) ||
      !addressRegsOnlyUsedAsMemoryBases(info, loop, plan)) {
    return false;
  }

  std::unordered_map<int, int> cloneMemo;
  const int initPtrReg =
      cloneLoopAddressExprReg(plan.representativeReg, plan.iv.phiReg,
                              plan.iv.initReg, loop.preheader, loop, info,
                              cloneMemo);
  if (initPtrReg < 0)
    return false;

  const int ptrPhiReg = mf.newVReg(RegClass::GPR);
  const int ptrNextReg = mf.newVReg(RegClass::GPR);

  auto phi = std::make_unique<MachineInst>(MIOp::PHI);
  phi->addOperand(MOperand::Def(ptrPhiReg, RegClass::GPR));
  phi->addOperand(MOperand::Use(initPtrReg, RegClass::GPR));
  phi->addOperand(MOperand::BB(loop.preheader));
  phi->addOperand(MOperand::Use(ptrNextReg, RegClass::GPR));
  phi->addOperand(MOperand::BB(latch));
  phi->comment = "loop.addr.phi";
  insertPhiAtBlockTop(loop.header, std::move(phi));

  auto bump = std::make_unique<MachineInst>(MIOp::ADDI);
  bump->addOperand(MOperand::Def(ptrNextReg, RegClass::GPR));
  bump->addOperand(MOperand::Use(ptrPhiReg, RegClass::GPR));
  bump->addOperand(MOperand::Imm(plan.stepImm));
  bump->comment = "loop.addr.bump";
  insertBeforeTerminator(latch, std::move(bump));

  rewriteLoopAddressGroupMemoryUses(loop, plan, ptrPhiReg);

  return true;
}

bool strengthReduceLoopAddressRecompute(MachineFunction& mf) {
  if (!hasVirtualOperands(mf))
    return false;

  bool changed = false;
  // 逐循环处理地址外提：含 call、含嵌套循环或没有单一 latch 的循环直接跳过。
  auto loops = collectNaturalLoops(mf);
  if (traceLoopAddressCleanup()) {
    std::fprintf(stderr, "[mir-loop-addr] function=%s loops=%zu\n",
                 mf.name().c_str(), loops.size());
  }
  for (size_t loopIdx = 0; loopIdx < loops.size(); ++loopIdx) {
    const auto& loop = loops[loopIdx];
    if (!loop.header || !loop.preheader) {
      if (traceLoopAddressCleanup()) {
        std::fprintf(stderr,
                     "[mir-loop-addr] function=%s idx=%zu skip=no-header\n",
                     mf.name().c_str(), loopIdx);
      }
      continue;
    }
    if (loopContainsCall(loop)) {
      if (traceLoopAddressCleanup()) {
        std::fprintf(stderr,
                     "[mir-loop-addr] function=%s header=%s skip=call\n",
                     mf.name().c_str(), loop.header->label().c_str());
      }
      continue;
    }
    if (loopContainsNestedLoop(loops, loopIdx)) {
      if (traceLoopAddressCleanup()) {
        std::fprintf(stderr,
                     "[mir-loop-addr] function=%s header=%s skip=nested\n",
                     mf.name().c_str(), loop.header->label().c_str());
      }
      continue;
    }
    MachineBasicBlock* latch = singleLatchBlock(loop);
    if (!latch) {
      if (traceLoopAddressCleanup()) {
        std::fprintf(stderr,
                     "[mir-loop-addr] function=%s header=%s skip=no-latch\n",
                     mf.name().c_str(), loop.header->label().c_str());
      }
      continue;
    }

    while (true) {
      auto info = collectDefUseInfo(mf);
      LoopAddressGroupPlan plan;
      if (!findBestLoopAddressGroup(loop, latch, info, plan)) {
        if (traceLoopAddressCleanup()) {
          std::fprintf(stderr,
                       "[mir-loop-addr] function=%s header=%s no-plan\n",
                       mf.name().c_str(), loop.header->label().c_str());
        }
        break;
      }
      if (traceLoopAddressCleanup()) {
        std::fprintf(stderr,
                     "[mir-loop-addr] function=%s header=%s plan step=%lld "
                     "uses=%zu members=%zu cost=%zu\n",
                     mf.name().c_str(), loop.header->label().c_str(),
                     static_cast<long long>(plan.stepImm), plan.useCount,
                     plan.memberRegs.size(), plan.expressionCost);
      }
      if (!applyLoopAddressGroup(mf, loop, latch, info, plan)) {
        if (traceLoopAddressCleanup()) {
          std::fprintf(stderr,
                       "[mir-loop-addr] function=%s header=%s apply-fail\n",
                       mf.name().c_str(), loop.header->label().c_str());
        }
        break;
      }
      changed = true;
    }
  }
  return changed;
}

bool hoistLoopHeaderBranchConstants(
    MachineFunction& mf, const MachineLoop& loop,
    const MachineDefUseInfo& info) {
  if (!loop.header || !loop.preheader)
    return false;
  std::vector<std::pair<MachineInst*, int>> candidates;
  for (auto& miPtr : loop.header->insts()) {
    MachineInst* mi = miPtr.get();
    if (!mi || mi->opcode() != MIOp::LI || mi->numOperands() != 2 ||
        !mi->operand(0).isReg() || !mi->operand(0).isDef ||
        !isVirtualReg(mi->operand(0).reg) || !mi->operand(1).isImm()) {
      continue;
    }
    const int reg = mi->operand(0).reg;
    auto usesIt = info.uses.find(reg);
    if (usesIt == info.uses.end() || usesIt->second.empty())
      continue;
    bool branchOnly = true;
    for (MachineInst* use : usesIt->second) {
      if (!use || use->parent() != loop.header ||
          !mioIsBranch(use->opcode())) {
        branchOnly = false;
        break;
      }
      bool appearsInBranch = false;
      for (const MOperand& operand : use->operands()) {
        if (operand.isReg() && !operand.isDef && operand.reg == reg) {
          appearsInBranch = true;
          break;
        }
      }
      if (!appearsInBranch) {
        branchOnly = false;
        break;
      }
    }
    if (branchOnly)
      candidates.push_back({mi, reg});
  }

  bool changed = false;
  for (const auto& candidate : candidates) {
    std::unique_ptr<MachineInst> moved;
    auto& insts = loop.header->insts();
    for (auto it = insts.begin(); it != insts.end(); ++it) {
      if (it->get() != candidate.first)
        continue;
      moved = std::move(*it);
      insts.erase(it);
      break;
    }
    if (!moved)
      continue;
    mf.vregInfo(candidate.second).rematerializable = false;
    insertBeforeTerminator(loop.preheader, std::move(moved));
    changed = true;
  }
  return changed;
}

bool hoistLoopLocalSymbolAddresses(MachineFunction& mf) {
  if (!hasVirtualOperands(mf))
    return false;

  bool changed = false;
  auto loops = collectNaturalLoops(mf);
  const bool skipHierarchical =
      sycEnvironmentValue("SYC_SKIP_MIR_HIERARCHICAL_LA_HOIST") != nullptr;
  for (size_t loopIdx = 0; loopIdx < loops.size(); ++loopIdx) {
    const auto& loop = loops[loopIdx];
    if (!loop.header || !loop.preheader || loopContainsCall(loop))
      continue;
    const bool hasNestedLoop = loopContainsNestedLoop(loops, loopIdx);
    const bool extendedHoist =
        !sycEnvironmentValue("SYC_SKIP_MIR_EXTENDED_LA_HOIST");
    if (hasNestedLoop && !extendedHoist)
      continue;

    std::unordered_map<std::string, std::vector<MachineInst*>> bySymbol;
    bool loopMayStore = false;
    for (auto* bb : orderedLoopBlocks(loop)) {
      for (auto& miPtr : bb->insts()) {
        MachineInst* mi = miPtr.get();
        if (mi && mioMayStore(mi->opcode()))
          loopMayStore = true;
        // 内层已提升的符号地址仍位于父循环内部，可以继续逐层提升；
        // 等待删除的旧指令不再参与候选统计。
        if (!mi || mi->opcode() != MIOp::LA || mi->numOperands() != 2 ||
            !mi->operand(0).isReg() || !mi->operand(0).isDef ||
            !isVirtualReg(mi->operand(0).reg) || !mi->operand(1).isSym() ||
            mi->comment == "loop.addr.redundant" ||
            (skipHierarchical && mi->comment == "loop.addr.hoisted")) {
          continue;
        }
        if (!regHasUseOutsideLoop(mf, loop, mi->operand(0).reg))
          bySymbol[mi->operand(1).sym].push_back(mi);
      }
    }

    struct Candidate {
      std::string symbol;
      std::vector<MachineInst*> insts;
    };
    bool hasUnitStepIv = false;
    if (!loopMayStore) {
      if (MachineBasicBlock* latch = singleLatchBlock(loop)) {
        const MachineDefUseInfo defUse = collectDefUseInfo(mf);
        for (const LoopIVInfo& iv : collectSingleStepLoopIVs(loop, latch,
                                                             defUse)) {
          if (iv.iterStepImm == 1 || iv.iterStepImm == -1) {
            hasUnitStepIv = true;
            break;
          }
        }
      }
    }
    size_t loopInstructionCount = 0;
    for (auto* bb : orderedLoopBlocks(loop))
      loopInstructionCount += bb ? bb->insts().size() : 0;
    if (hasNestedLoop &&
        (loop.blocks.size() > 12 || loopInstructionCount > 96)) {
      continue;
    }
    const bool compactLoop =
        loop.blocks.size() <= 4 && loopInstructionCount <= 40;
    // 单例地址只在只读叶循环中提升，避免为写循环或嵌套循环增加长期活跃值；
    // 同一符号出现多次时仍按下方收益排序处理。
    const bool allowSingletons =
        !sycEnvironmentValue("SYC_SKIP_MIR_SINGLETON_LA_HOIST") &&
        !loopMayStore && !hasNestedLoop &&
        (hasUnitStepIv || (extendedHoist && compactLoop));
    std::vector<Candidate> candidates;
    for (auto& kv : bySymbol) {
      if (kv.second.empty() ||
          (kv.second.size() == 1 && !allowSingletons))
        continue;
      candidates.push_back({kv.first, std::move(kv.second)});
    }
    std::sort(candidates.begin(), candidates.end(),
              [](const Candidate& a, const Candidate& b) {
                if (a.insts.size() != b.insts.size())
                  return a.insts.size() > b.insts.size();
                return a.symbol < b.symbol;
              });

    constexpr size_t kMaxHoistedSymbols = 4;
    if (candidates.size() > kMaxHoistedSymbols)
      candidates.resize(kMaxHoistedSymbols);

    for (const auto& cand : candidates) {
      if (traceLoopAddressCleanup()) {
        std::fprintf(stderr,
                     "[mir-loop-symbol] function=%s header=%s symbol=%s "
                     "uses=%zu blocks=%zu insts=%zu nested=%d\n",
                     mf.name().c_str(), loop.header->label().c_str(),
                     cand.symbol.c_str(), cand.insts.size(), loop.blocks.size(),
                     loopInstructionCount, hasNestedLoop ? 1 : 0);
      }
      int hoistedReg = mf.newVReg(RegClass::GPR);
      mf.vregInfo(hoistedReg).rematerializable = true;
      auto la = std::make_unique<MachineInst>(MIOp::LA);
      la->addOperand(MOperand::Def(hoistedReg, RegClass::GPR));
      la->addOperand(MOperand::Sym(cand.symbol));
      la->comment = "loop.addr.hoisted";
      insertBeforeTerminator(loop.preheader, std::move(la));

      for (MachineInst* inst : cand.insts) {
        if (!inst || inst->numOperands() < 1 || !inst->operand(0).isReg())
          continue;
        replaceRegUsesInLoop(loop, inst->operand(0).reg, hoistedReg);
        inst->comment = "loop.addr.redundant";
      }
      changed = true;
    }

    if (!changed)
      continue;

    for (auto* bb : orderedLoopBlocks(loop)) {
      auto& insts = bb->insts();
      std::vector<std::unique_ptr<MachineInst>> kept;
      kept.reserve(insts.size());
      for (auto& miPtr : insts) {
        if (miPtr && miPtr->opcode() == MIOp::LA &&
            miPtr->comment == "loop.addr.redundant") {
          continue;
        }
        kept.push_back(std::move(miPtr));
      }
      insts = std::move(kept);
    }
  }

  return changed;
}

int constantMaterializationCost(int64_t value) {
  if (value >= -32 && value <= 31)
    return 2;
  if (fitsAddiImm(value))
    return 4;
  return 8;
}

bool hoistLoopInvariantConstants(MachineFunction& mf) {
  if (!hasVirtualOperands(mf))
    return false;

  struct Candidate {
    MachineBasicBlock* block = nullptr;
    MachineInst* inst = nullptr;
    int reg = -1;
    int cost = 0;
    size_t useCount = 0;
  };

  bool changed = false;
  auto loops = collectNaturalLoops(mf);
  for (size_t loopIdx = 0; loopIdx < loops.size(); ++loopIdx) {
    const auto& loop = loops[loopIdx];
    if (!loop.header || !loop.preheader)
      continue;
    if (loopContainsNestedLoop(loops, loopIdx))
      continue;

    const MachineDefUseInfo info = collectDefUseInfo(mf);
    changed |= hoistLoopHeaderBranchConstants(mf, loop, info);
    std::vector<Candidate> candidates;
    for (auto* bb : orderedLoopBlocks(loop)) {
      if (!bb || bb == loop.preheader)
        continue;
      for (auto& miPtr : bb->insts()) {
        MachineInst* mi = miPtr.get();
        if (!mi || mi->opcode() != MIOp::LI || mi->numOperands() != 2 ||
            !mi->operand(0).isReg() || !mi->operand(0).isDef ||
            !isVirtualReg(mi->operand(0).reg) || !mi->operand(1).isImm()) {
          continue;
        }

        const int reg = mi->operand(0).reg;
        auto usesIt = info.uses.find(reg);
        if (usesIt == info.uses.end() || usesIt->second.empty())
          continue;
        bool usedOutsideLoop = false;
        bool onlyStoreUses = true;
        for (MachineInst* use : usesIt->second) {
          if (!use || !use->parent() || !loop.blocks.count(use->parent())) {
            usedOutsideLoop = true;
            break;
          }
          if (!mioMayStore(use->opcode()) || use->numOperands() < 1 ||
              !use->operand(0).isReg() || use->operand(0).reg != reg)
            onlyStoreUses = false;
        }
        if (usedOutsideLoop || !onlyStoreUses)
          continue;

        const int cost = constantMaterializationCost(mi->operand(1).imm);
        if (cost < 4)
          continue;
        candidates.push_back({bb, mi, reg, cost, usesIt->second.size()});
      }
    }

    if (candidates.size() < 8)
      continue;
    std::stable_sort(candidates.begin(), candidates.end(),
                     [](const Candidate& lhs, const Candidate& rhs) {
                       if (lhs.cost != rhs.cost)
                         return lhs.cost > rhs.cost;
                       if (lhs.useCount != rhs.useCount)
                         return lhs.useCount > rhs.useCount;
                       return lhs.reg < rhs.reg;
                     });
    constexpr size_t kMaxHoistedConstants = 32;
    if (candidates.size() > kMaxHoistedConstants)
      candidates.resize(kMaxHoistedConstants);

    for (const Candidate& candidate : candidates) {
      if (!candidate.block || !candidate.inst)
        continue;
      std::unique_ptr<MachineInst> moved;
      auto& insts = candidate.block->insts();
      for (auto it = insts.begin(); it != insts.end(); ++it) {
        if (it->get() != candidate.inst)
          continue;
        moved = std::move(*it);
        insts.erase(it);
        break;
      }
      if (!moved)
        continue;
      insertBeforeTerminator(loop.preheader, std::move(moved));
      changed = true;
    }
  }
  return changed;
}

// 无副作用的纯计算类操作码白名单：结果死掉时指令可直接删除。
bool isTriviallyDeadableOp(MIOp op) {
  switch (op) {
    case MIOp::MV:
    case MIOp::FMV_S:
    case MIOp::LI:
    case MIOp::LA:
    case MIOp::ADDI:
    case MIOp::ADDIW:
    case MIOp::ADD:
    case MIOp::ADDW:
    case MIOp::SUB:
    case MIOp::SUBW:
    case MIOp::MUL:
    case MIOp::MULW:
    case MIOp::DIV:
    case MIOp::DIVW:
    case MIOp::DIVU:
    case MIOp::DIVUW:
    case MIOp::REM:
    case MIOp::REMW:
    case MIOp::REMU:
    case MIOp::REMUW:
    case MIOp::AND_:
    case MIOp::OR_:
    case MIOp::XOR_:
    case MIOp::ANDI:
    case MIOp::ORI:
    case MIOp::XORI:
    case MIOp::SLL:
    case MIOp::SLLW:
    case MIOp::SRL:
    case MIOp::SRLW:
    case MIOp::SRA:
    case MIOp::SRAW:
    case MIOp::SLLI:
    case MIOp::SLLIW:
    case MIOp::SRLI:
    case MIOp::SRLIW:
    case MIOp::SRAI:
    case MIOp::SRAIW:
    case MIOp::SLT:
    case MIOp::SLTU:
    case MIOp::SLTI:
    case MIOp::SLTIU:
    case MIOp::FADD_S:
    case MIOp::FSUB_S:
    case MIOp::FMUL_S:
    case MIOp::FDIV_S:
    case MIOp::FSQRT_S:
    case MIOp::FNEG_S:
    case MIOp::FABS_S:
    case MIOp::FMADD_S:
    case MIOp::FMSUB_S:
    case MIOp::FNMADD_S:
    case MIOp::FNMSUB_S:
    case MIOp::FEQ_S:
    case MIOp::FLT_S:
    case MIOp::FLE_S:
    case MIOp::FCVT_S_W:
    case MIOp::FCVT_W_S:
    case MIOp::FMV_W_X:
    case MIOp::FMV_X_W:
      return true;
    default:
      return false;
  }
}

bool isRedundantLocalOp(const MachineInst& mi) {
  if (mi.opcode() == MIOp::MV || mi.opcode() == MIOp::FMV_S) {
    return mi.numOperands() == 2 &&
           mi.operand(0).isReg() && mi.operand(1).isReg() &&
           mi.operand(0).reg == mi.operand(1).reg;
  }
  if (mi.opcode() == MIOp::ADDI || mi.opcode() == MIOp::ADDIW) {
    return mi.numOperands() == 3 &&
           mi.operand(0).isReg() &&
           mi.operand(1).isReg() &&
           mi.operand(2).isImm() &&
           mi.operand(0).reg == mi.operand(1).reg &&
           mi.operand(2).imm == 0;
  }
  if (mi.opcode() == MIOp::ADD || mi.opcode() == MIOp::ADDW) {
    return mi.numOperands() == 3 &&
           mi.operand(0).isReg() &&
           mi.operand(1).isReg() &&
           mi.operand(2).isReg() &&
           ((mi.operand(1).reg == gpr::x0 && mi.operand(0).reg == mi.operand(2).reg) ||
            (mi.operand(2).reg == gpr::x0 && mi.operand(0).reg == mi.operand(1).reg));
  }
  return false;
}

bool isScratchReg(int reg) {
  return reg == gpr::t4 || reg == gpr::t5 || reg == gpr::t6 ||
         reg == fpr::ft9 || reg == fpr::ft10 || reg == fpr::ft11;
}

bool isFrameAddressMaterialization(const MachineInst& mi) {
  return (mi.opcode() == MIOp::ADDI || mi.opcode() == MIOp::ADDIW) &&
         mi.numOperands() == 3 &&
         mi.operand(0).isReg() &&
         mi.operand(0).isDef &&
         mi.operand(1).isFrame() &&
         mi.operand(2).isImm();
}

// 判断指令的结果寄存器能否改指向 move 的目的寄存器：
// 排除终结/调用/存内存/PHI 等特殊形式，只允单定义且无副作用的指令。
bool canRetargetDefToMoveDest(const MachineInst& mi) {
  if (mioIsTerminator(mi.opcode()) || mioIsCall(mi.opcode()) ||
      mioMayStore(mi.opcode()) || mi.opcode() == MIOp::PHI ||
      mi.opcode() == MIOp::PHI_COPY) {
    return false;
  }
  if (mi.numOperands() == 0 || !mi.operand(0).isReg() || !mi.operand(0).isDef)
    return false;
  if (isFrameAddressMaterialization(mi))
    return false;
  switch (mi.opcode()) {
    case MIOp::LI:
    case MIOp::LA:
    case MIOp::ADDI:
    case MIOp::ADDIW:
    case MIOp::ADD:
    case MIOp::ADDW:
    case MIOp::SUB:
    case MIOp::SUBW:
    case MIOp::MUL:
    case MIOp::MULW:
    case MIOp::DIV:
    case MIOp::DIVW:
    case MIOp::DIVU:
    case MIOp::DIVUW:
    case MIOp::REM:
    case MIOp::REMW:
    case MIOp::REMU:
    case MIOp::REMUW:
    case MIOp::AND_:
    case MIOp::OR_:
    case MIOp::XOR_:
    case MIOp::ANDI:
    case MIOp::ORI:
    case MIOp::XORI:
    case MIOp::SLL:
    case MIOp::SLLW:
    case MIOp::SRL:
    case MIOp::SRLW:
    case MIOp::SRA:
    case MIOp::SRAW:
    case MIOp::SLLI:
    case MIOp::SLLIW:
    case MIOp::SRLI:
    case MIOp::SRLIW:
    case MIOp::SRAI:
    case MIOp::SRAIW:
    case MIOp::SLT:
    case MIOp::SLTU:
    case MIOp::SLTI:
    case MIOp::SLTIU:
    case MIOp::LB:
    case MIOp::LH:
    case MIOp::LW:
    case MIOp::LD:
    case MIOp::LBU:
    case MIOp::LHU:
    case MIOp::LWU:
    case MIOp::FLW:
    case MIOp::FADD_S:
    case MIOp::FSUB_S:
    case MIOp::FMUL_S:
    case MIOp::FDIV_S:
    case MIOp::FSQRT_S:
    case MIOp::FNEG_S:
    case MIOp::FABS_S:
    case MIOp::FMADD_S:
    case MIOp::FMSUB_S:
    case MIOp::FNMADD_S:
    case MIOp::FNMSUB_S:
    case MIOp::FEQ_S:
    case MIOp::FLT_S:
    case MIOp::FLE_S:
    case MIOp::FCVT_S_W:
    case MIOp::FCVT_W_S:
    case MIOp::FMV_W_X:
    case MIOp::FMV_X_W:
    case MIOp::FMV_S:
      return true;
    default:
      return false;
  }
}

VRegUseCountMap computeVirtualUseCounts(MachineFunction& mf) {
  VRegUseCountMap counts;
  for (auto& bbPtr : mf.blocks()) {
    for (auto& miPtr : bbPtr->insts()) {
      if (!miPtr)
        continue;
      std::vector<int> uses;
      miPtr->collectUses(uses);
      for (int use : uses) {
        if (use >= 0)
          counts[use]++;
      }
    }
  }
  return counts;
}

bool foldProducerMoves(MachineFunction& mf) {
  bool changed = false;
  VRegUseCountMap useCounts = computeVirtualUseCounts(mf);

  for (auto& bbPtr : mf.blocks()) {
    auto& insts = bbPtr->insts();
    if (insts.size() < 2)
      continue;

    std::vector<std::unique_ptr<MachineInst>> kept;
    kept.reserve(insts.size());
    for (size_t i = 0; i < insts.size(); ++i) {
      if (!insts[i])
        continue;
      MachineInst* prod = insts[i].get();
      if (i + 1 >= insts.size() || !insts[i + 1]) {
        kept.push_back(std::move(insts[i]));
        continue;
      }

      MachineInst* mv = insts[i + 1].get();
      bool isMove = (mv->opcode() == MIOp::MV || mv->opcode() == MIOp::FMV_S) &&
                    mv->numOperands() == 2 &&
                    mv->operand(0).isReg() && mv->operand(0).isDef &&
                    mv->operand(1).isReg();
      if (!isMove || !canRetargetDefToMoveDest(*prod) ||
          prod->numOperands() == 0 || !prod->operand(0).isReg() ||
          !prod->operand(0).isDef) {
        kept.push_back(std::move(insts[i]));
        continue;
      }

      const MOperand& prodDef = prod->operand(0);
      const MOperand& mvDst = mv->operand(0);
      const MOperand& mvSrc = mv->operand(1);
      if (prodDef.reg < 0 || mvSrc.reg != prodDef.reg ||
          prodDef.regClass != mvDst.regClass || prodDef.regClass != mvSrc.regClass ||
          useCounts[prodDef.reg] != 1) {
        kept.push_back(std::move(insts[i]));
        continue;
      }

      prod->operand(0).reg = mvDst.reg;
      prod->operand(0).regClass = mvDst.regClass;
      if (isVirtualReg(mvDst.reg))
        useCounts[mvDst.reg] += useCounts[prodDef.reg];
      useCounts.erase(prodDef.reg);
      kept.push_back(std::move(insts[i]));
      insts[i + 1].reset();
      ++i;
      changed = true;
    }

    for (size_t i = 0; i < insts.size(); ++i) {
      if (insts[i])
        kept.push_back(std::move(insts[i]));
    }
    insts = std::move(kept);
  }
  return changed;
}

bool eliminateRedundantRegisterConstants(MachineFunction& mf) {
  bool changed = false;

  for (auto& bbPtr : mf.blocks()) {
    std::unordered_map<int, KnownRegValue> known;
    std::vector<std::unique_ptr<MachineInst>> kept;
    kept.reserve(bbPtr->insts().size());

    for (auto& miPtr : bbPtr->insts()) {
      MachineInst* mi = miPtr.get();
      if (!mi)
        continue;

      auto constantDefinedBy = [](const MachineInst& inst,
                                  KnownRegValue& out) -> bool {
        if (inst.opcode() == MIOp::LI && inst.numOperands() == 2 &&
            inst.operand(0).isReg() && inst.operand(0).isDef &&
            inst.operand(1).isImm()) {
          out.kind = KnownRegValue::Kind::Imm;
          out.imm = inst.operand(1).imm;
          out.symbol.clear();
          return true;
        }
        if (inst.opcode() == MIOp::LA && inst.numOperands() == 2 &&
            inst.operand(0).isReg() && inst.operand(0).isDef &&
            inst.operand(1).isSym()) {
          out.kind = KnownRegValue::Kind::Symbol;
          out.imm = 0;
          out.symbol = inst.operand(1).sym;
          return true;
        }
        return false;
      };

      bool drop = false;
      KnownRegValue produced;
      const bool producesConstant = constantDefinedBy(*mi, produced);
      if (producesConstant) {
        int dst = mi->operand(0).reg;
        auto it = known.find(dst);
        if (it != known.end() && sameKnownValue(it->second, produced)) {
          // 同一基本块内，寄存器未被重新定义时重复 li/la 没有
          // 语义效果；这是 machine-level local CSE，不跨块也不改变活跃区间。
          drop = true;
        }
      }

      if (drop) {
        changed = true;
        continue;
      }

      std::unordered_map<int, KnownRegValue> copiedConstants;
      if ((mi->opcode() == MIOp::MV || mi->opcode() == MIOp::ADDI) &&
          mi->numOperands() >= 2 && mi->operand(0).isReg() &&
          mi->operand(0).isDef && mi->operand(1).isReg()) {
        bool isPlainCopy = mi->opcode() == MIOp::MV;
        if (mi->opcode() == MIOp::ADDI && mi->numOperands() == 3 &&
            mi->operand(2).isImm() &&
            mi->operand(2).imm == 0) {
          isPlainCopy = true;
        }
        if (isPlainCopy) {
          auto srcIt = known.find(mi->operand(1).reg);
          if (srcIt != known.end())
            copiedConstants[mi->operand(0).reg] = srcIt->second;
        }
      }

      std::vector<int> defs;
      mi->collectDefs(defs);
      for (int def : defs)
        known.erase(def);

      if (producesConstant) {
        known[mi->operand(0).reg] = std::move(produced);
      } else {
        for (auto& kv : copiedConstants)
          known[kv.first] = std::move(kv.second);
      }

      kept.push_back(std::move(miPtr));
    }

    bbPtr->insts() = std::move(kept);
  }

  return changed;
}

bool isSingleSlotSpillLoad(const MachineFunction& mf,
                           const MachineInst& mi,
                           int& frameIdx) {
  if (!((mi.opcode() == MIOp::LD && mi.numOperands() == 3) ||
        (mi.opcode() == MIOp::FLW && mi.numOperands() == 3))) {
    return false;
  }
  if (!mi.operand(0).isReg() || !mi.operand(0).isDef ||
      !mi.operand(1).isFrame()) {
    return false;
  }
  frameIdx = mi.operand(1).frameIdx;
  return frameIdx >= 0 && frameIdx < static_cast<int>(mf.frame().size()) &&
         mf.frame()[frameIdx].isSpill;
}

bool isSingleSlotSpillStore(const MachineFunction& mf,
                            const MachineInst& mi,
                            int& frameIdx) {
  if (!((mi.opcode() == MIOp::SD && mi.numOperands() == 3) ||
        (mi.opcode() == MIOp::FSW && mi.numOperands() == 3))) {
    return false;
  }
  if (!mi.operand(0).isReg() || mi.operand(0).isDef ||
      !mi.operand(1).isFrame()) {
    return false;
  }
  frameIdx = mi.operand(1).frameIdx;
  return frameIdx >= 0 && frameIdx < static_cast<int>(mf.frame().size()) &&
         mf.frame()[frameIdx].isSpill;
}

bool sameSpillClass(const MachineInst& store, const MachineInst& load) {
  return (store.opcode() == MIOp::SD && load.opcode() == MIOp::LD) ||
         (store.opcode() == MIOp::FSW && load.opcode() == MIOp::FLW);
}

std::unique_ptr<MachineInst> makeRegisterCopy(int dst,
                                              int src,
                                              RegClass cls,
                                              MachineBasicBlock* parent) {
  if (dst == src)
    return nullptr;
  auto mv = std::make_unique<MachineInst>(cls == RegClass::FPR ? MIOp::FMV_S
                                                               : MIOp::MV);
  mv->addOperand(MOperand::Def(dst, cls));
  mv->addOperand(MOperand::Use(src, cls));
  mv->comment = "spill-forward";
  mv->setParent(parent);
  return mv;
}

bool forwardSingleUseSpillReloads(MachineFunction& mf) {
  std::unordered_map<int, int> spillLoadCount;
  for (auto& bbPtr : mf.blocks()) {
    for (auto& miPtr : bbPtr->insts()) {
      int frameIdx = -1;
      if (isSingleSlotSpillLoad(mf, *miPtr, frameIdx))
        ++spillLoadCount[frameIdx];
    }
  }

  bool changed = false;
  for (auto& bbPtr : mf.blocks()) {
    auto& insts = bbPtr->insts();
    std::vector<std::unique_ptr<MachineInst>> kept;
    kept.reserve(insts.size());

    for (size_t i = 0; i < insts.size(); ++i) {
      if (!insts[i]) {
        continue;
      }
      if (i + 1 >= insts.size() || !insts[i + 1]) {
        kept.push_back(std::move(insts[i]));
        continue;
      }

      int storeSlot = -1;
      int loadSlot = -1;
      MachineInst* store = insts[i].get();
      MachineInst* load = insts[i + 1].get();
      if (!isSingleSlotSpillStore(mf, *store, storeSlot) ||
          !isSingleSlotSpillLoad(mf, *load, loadSlot) ||
          storeSlot != loadSlot || spillLoadCount[loadSlot] != 1 ||
          !sameSpillClass(*store, *load) ||
          store->operand(0).regClass != load->operand(0).regClass) {
        kept.push_back(std::move(insts[i]));
        continue;
      }

      // 线性扫描解析 phi 时会产生 “store spill-slot; load 同一
      // spill-slot” 的短生命值桥接。如果该 spill slot 在整个函数只有这一
      // 次 reload，就可以等价改成寄存器 copy，并删除这对栈访问。
      if (auto mv = makeRegisterCopy(load->operand(0).reg,
                                     store->operand(0).reg,
                                     load->operand(0).regClass,
                                     bbPtr.get())) {
        kept.push_back(std::move(mv));
      }
      insts[i].reset();
      insts[i + 1].reset();
      ++i;
      changed = true;
    }

    for (auto& mi : insts) {
      if (mi)
        kept.push_back(std::move(mi));
    }
    insts = std::move(kept);
  }

  return changed;
}

bool vectorContainsReg(const std::vector<int>& regs, int reg);
bool regUsedBeforeRedefOrLiveOut(MachineBasicBlock& bb,
                                 size_t startIndex,
                                 int reg,
                                 const std::unordered_set<int>& liveOut);
std::unordered_map<MachineBasicBlock*, BlockLive>
computePhysicalLiveness(MachineFunction& mf);

bool mentionsReg(const MachineInst& inst, int reg, bool asUse, bool asDef) {
  if (reg < 0)
    return false;
  if (asUse) {
    std::vector<int> uses;
    inst.collectUses(uses);
    if (vectorContainsReg(uses, reg))
      return true;
  }
  if (asDef) {
    std::vector<int> defs;
    inst.collectDefs(defs);
    if (vectorContainsReg(defs, reg))
      return true;
  }
  return false;
}

bool touchesSpillSlot(const MachineFunction& mf,
                      const MachineInst& inst,
                      int frameIdx) {
  int slot = -1;
  if (isSingleSlotSpillStore(mf, inst, slot) && slot == frameIdx)
    return true;
  if (isSingleSlotSpillLoad(mf, inst, slot) && slot == frameIdx)
    return true;
  return false;
}

bool canRetargetSpillForwardProducer(MachineFunction& mf,
                                     MachineBasicBlock& bb,
                                     size_t producerIndex,
                                     size_t storeIndex,
                                     size_t loadIndex,
                                     int frameIdx,
                                     int oldDef,
                                     int newDef,
                                     const std::unordered_set<int>& liveOut) {
  auto& insts = bb.insts();
  if (producerIndex >= insts.size() || storeIndex >= insts.size() ||
      loadIndex >= insts.size() || !insts[producerIndex] ||
      !insts[storeIndex] || !insts[loadIndex]) {
    return false;
  }

  MachineInst& producer = *insts[producerIndex];
  if (!canRetargetDefToMoveDest(producer) ||
      producer.numOperands() == 0 || !producer.operand(0).isReg() ||
      !producer.operand(0).isDef || !isPhysReg(oldDef) ||
      !isPhysReg(newDef) || oldDef == newDef) {
    return false;
  }

  if (regUsedBeforeRedefOrLiveOut(bb, storeIndex + 1, oldDef, liveOut))
    return false;

  // 这里会把 reload 目标改写到 producerIndex，并把 loadIndex 后面的
  // uses 指向 newDef。只有 newDef 的旧值在区间内不可观察、未被重定义，且 spill slot
  // 没有中间访问时才安全。
  for (size_t k = producerIndex + 1; k < loadIndex; ++k) {
    MachineInst* inst = insts[k].get();
    if (!inst)
      continue;
    if (k != storeIndex && touchesSpillSlot(mf, *inst, frameIdx))
      return false;
    if (mioIsCall(inst->opcode()) || mioIsTerminator(inst->opcode()))
      return false;
    if (mentionsReg(*inst, newDef, true, true))
      return false;
  }

  return true;
}

bool canDropSameRegSpillReload(MachineFunction& mf,
                               MachineBasicBlock& bb,
                               size_t storeIndex,
                               size_t loadIndex,
                               int frameIdx,
                               int reg) {
  if (!isPhysReg(reg) || storeIndex >= loadIndex)
    return false;

  auto& insts = bb.insts();
  if (storeIndex >= insts.size() || loadIndex >= insts.size())
    return false;

  for (size_t k = storeIndex + 1; k < loadIndex; ++k) {
    MachineInst* inst = insts[k].get();
    if (!inst)
      continue;
    if (touchesSpillSlot(mf, *inst, frameIdx))
      return false;
    if (mioIsCall(inst->opcode()) || mioIsTerminator(inst->opcode()))
      return false;
    if (mentionsReg(*inst, reg, /*asUse=*/false, /*asDef=*/true))
      return false;
  }
  return true;
}

bool forwardSeparatedSpillReloads(MachineFunction& mf) {
  auto live = computePhysicalLiveness(mf);
  std::unordered_map<int, int> spillLoadCount;
  for (auto& bbPtr : mf.blocks()) {
    for (auto& miPtr : bbPtr->insts()) {
      int frameIdx = -1;
      if (isSingleSlotSpillLoad(mf, *miPtr, frameIdx))
        ++spillLoadCount[frameIdx];
    }
  }

  bool changed = false;
  for (auto& bbPtr : mf.blocks()) {
    MachineBasicBlock* bb = bbPtr.get();
    if (!bb)
      continue;
    auto liveIt = live.find(bb);
    const std::unordered_set<int> empty;
    const auto& liveOut = liveIt == live.end() ? empty : liveIt->second.liveOut;
    auto& insts = bb->insts();
    if (insts.size() < 3)
      continue;

    std::vector<bool> drop(insts.size(), false);
    for (size_t i = 0; i + 1 < insts.size(); ++i) {
      if (drop[i] || drop[i + 1] || !insts[i] || !insts[i + 1])
        continue;
      MachineInst* producer = insts[i].get();
      MachineInst* store = insts[i + 1].get();
      if (!producer || !store || producer->numOperands() == 0 ||
          !producer->operand(0).isReg() || !producer->operand(0).isDef)
        continue;

      int storeSlot = -1;
      if (!isSingleSlotSpillStore(mf, *store, storeSlot) ||
          store->operand(0).reg != producer->operand(0).reg ||
          store->operand(0).regClass != producer->operand(0).regClass)
        continue;

      size_t loadIndex = insts.size();
      for (size_t j = i + 2; j < insts.size(); ++j) {
        if (drop[j] || !insts[j])
          continue;
        int loadSlot = -1;
        if (isSingleSlotSpillLoad(mf, *insts[j], loadSlot) &&
            loadSlot == storeSlot) {
          loadIndex = j;
          break;
        }
        if (touchesSpillSlot(mf, *insts[j], storeSlot))
          break;
      }
      if (loadIndex >= insts.size() || !insts[loadIndex])
        continue;

      MachineInst* load = insts[loadIndex].get();
      if (!sameSpillClass(*store, *load) ||
          !load->operand(0).isReg() || !load->operand(0).isDef ||
          load->operand(0).regClass != store->operand(0).regClass)
        continue;

      int oldDef = producer->operand(0).reg;
      int newDef = load->operand(0).reg;
      if (oldDef == newDef) {
        if (!canDropSameRegSpillReload(mf, *bb, i + 1, loadIndex,
                                       storeSlot, oldDef))
          continue;
        if (spillLoadCount[storeSlot] == 1)
          drop[i + 1] = true;
        drop[loadIndex] = true;
        changed = true;
        continue;
      }

      if (spillLoadCount[storeSlot] != 1)
        continue;

      if (!canRetargetSpillForwardProducer(mf, *bb, i, i + 1, loadIndex,
                                           storeSlot, oldDef, newDef,
                                           liveOut))
        continue;

      producer->operand(0).reg = newDef;
      producer->operand(0).regClass = load->operand(0).regClass;
      drop[i + 1] = true;
      drop[loadIndex] = true;
      changed = true;
    }

    if (!changed)
      continue;

    std::vector<std::unique_ptr<MachineInst>> kept;
    kept.reserve(insts.size());
    for (size_t i = 0; i < insts.size(); ++i) {
      if (!drop[i] && insts[i])
        kept.push_back(std::move(insts[i]));
    }
    insts = std::move(kept);
  }
  return changed;
}

bool vectorContainsReg(const std::vector<int>& regs, int reg) {
  for (int value : regs) {
    if (value == reg)
      return true;
  }
  return false;
}

bool regUsedBeforeRedefOrLiveOut(MachineBasicBlock& bb,
                                 size_t startIndex,
                                 int reg,
                                 const std::unordered_set<int>& liveOut) {
  auto& insts = bb.insts();
  for (size_t i = startIndex; i < insts.size(); ++i) {
    MachineInst* inst = insts[i].get();
    if (!inst)
      continue;
    std::vector<int> uses;
    inst->collectUses(uses);
    if (vectorContainsReg(uses, reg))
      return true;
    std::vector<int> defs;
    inst->collectDefs(defs);
    if (vectorContainsReg(defs, reg))
      return false;
  }
  return liveOut.count(reg) != 0;
}

std::unordered_map<MachineBasicBlock*, BlockLive>
computePhysicalLiveness(MachineFunction& mf);

bool foldAdjacentPhysicalProducerMoves(MachineFunction& mf) {
  auto live = computePhysicalLiveness(mf);
  bool changed = false;

  for (auto& bbPtr : mf.blocks()) {
    MachineBasicBlock* bb = bbPtr.get();
    if (!bb)
      continue;
    auto liveIt = live.find(bb);
    const std::unordered_set<int> empty;
    const auto& liveOut = liveIt == live.end() ? empty : liveIt->second.liveOut;
    auto& insts = bb->insts();
    if (insts.size() < 2)
      continue;

    std::vector<std::unique_ptr<MachineInst>> kept;
    kept.reserve(insts.size());
    for (size_t i = 0; i < insts.size(); ++i) {
      if (!insts[i]) {
        continue;
      }
      MachineInst* prod = insts[i].get();
      if (i + 1 >= insts.size() || !insts[i + 1]) {
        kept.push_back(std::move(insts[i]));
        continue;
      }
      MachineInst* mv = insts[i + 1].get();
      const bool isMove =
          (mv->opcode() == MIOp::MV || mv->opcode() == MIOp::FMV_S) &&
          mv->numOperands() == 2 &&
          mv->operand(0).isReg() && mv->operand(0).isDef &&
          mv->operand(1).isReg();
      if (!isMove || !canRetargetDefToMoveDest(*prod) ||
          prod->numOperands() == 0 || !prod->operand(0).isReg() ||
          !prod->operand(0).isDef) {
        kept.push_back(std::move(insts[i]));
        continue;
      }

      const MOperand& prodDef = prod->operand(0);
      const MOperand& mvDst = mv->operand(0);
      const MOperand& mvSrc = mv->operand(1);
      if (!isPhysReg(prodDef.reg) || !isPhysReg(mvDst.reg) ||
          mvSrc.reg != prodDef.reg || mvDst.reg == prodDef.reg ||
          prodDef.regClass != mvDst.regClass ||
          prodDef.regClass != mvSrc.regClass ||
          regUsedBeforeRedefOrLiveOut(*bb, i + 2, prodDef.reg, liveOut)) {
        kept.push_back(std::move(insts[i]));
        continue;
      }

      // 寄存器分配后的复制折叠：生产指令只把结果写入临时物理寄存器，
      // 随后立即复制到真实目的寄存器。相邻复制之后临时寄存器不再使用，
      // 因此可以直接改写生产指令的目的寄存器。
      prod->operand(0).reg = mvDst.reg;
      prod->operand(0).regClass = mvDst.regClass;
      insts[i + 1].reset();
      kept.push_back(std::move(insts[i]));
      ++i;
      changed = true;
    }

    for (auto& mi : insts) {
      if (mi)
        kept.push_back(std::move(mi));
    }
    insts = std::move(kept);
  }

  return changed;
}

bool foldSeparatedPhysicalProducerMoves(MachineFunction& mf) {
  constexpr size_t kMaxProducerMoveDistance = 8;
  auto live = computePhysicalLiveness(mf);
  bool changed = false;

  for (auto& bbPtr : mf.blocks()) {
    MachineBasicBlock* bb = bbPtr.get();
    if (!bb)
      continue;
    auto liveIt = live.find(bb);
    const std::unordered_set<int> empty;
    const auto& liveOut = liveIt == live.end() ? empty : liveIt->second.liveOut;
    auto& insts = bb->insts();
    if (insts.size() < 3)
      continue;

    bool foldedInBlock = false;
    for (size_t producerIndex = 0;
         producerIndex + 2 < insts.size() && !foldedInBlock;
         ++producerIndex) {
      MachineInst* producer = insts[producerIndex].get();
      if (!producer || !canRetargetDefToMoveDest(*producer) ||
          producer->numOperands() == 0 ||
          !producer->operand(0).isReg() ||
          !producer->operand(0).isDef)
        continue;

      const MOperand& producerDef = producer->operand(0);
      if (!isPhysReg(producerDef.reg))
        continue;
      const size_t lastMove = std::min(
          insts.size(), producerIndex + 1 + kMaxProducerMoveDistance);
      for (size_t moveIndex = producerIndex + 2; moveIndex < lastMove;
           ++moveIndex) {
        MachineInst* move = insts[moveIndex].get();
        if (!move ||
            (move->opcode() != MIOp::MV && move->opcode() != MIOp::FMV_S) ||
            move->numOperands() != 2 ||
            !move->operand(0).isReg() || !move->operand(0).isDef ||
            !move->operand(1).isReg())
          continue;

        const MOperand& moveDst = move->operand(0);
        const MOperand& moveSrc = move->operand(1);
        if (!isPhysReg(moveDst.reg) || moveSrc.reg != producerDef.reg ||
            moveDst.reg == producerDef.reg ||
            producerDef.regClass != moveDst.regClass ||
            producerDef.regClass != moveSrc.regClass ||
            regUsedBeforeRedefOrLiveOut(*bb, moveIndex + 1,
                                        producerDef.reg, liveOut))
          continue;

        bool safe = true;
        for (size_t middle = producerIndex + 1; middle < moveIndex;
             ++middle) {
          MachineInst* inst = insts[middle].get();
          if (!inst || mioIsCall(inst->opcode()) ||
              mioIsTerminator(inst->opcode()) ||
              mentionsReg(*inst, moveDst.reg, /*asUse=*/true,
                          /*asDef=*/true) ||
              mentionsReg(*inst, producerDef.reg, /*asUse=*/false,
                          /*asDef=*/true)) {
            safe = false;
            break;
          }
        }
        if (!safe)
          continue;

        // 在重定向产生者之前，先改写间隙中的源使用。这是寄存器合并形式：产生
        // 的值在旧 PHI-copy 前还会流入其他指令，且目标寄存器在间隙中未被改写。
        for (size_t middle = producerIndex + 1; middle < moveIndex;
             ++middle) {
          MachineInst* inst = insts[middle].get();
          for (MOperand& operand : inst->operands()) {
            if (operand.isReg() && !operand.isDef &&
                operand.reg == producerDef.reg)
              operand.reg = moveDst.reg;
          }
        }

        // 移动后源值已无用途；重定向产生者并移除分离的复制即可保留各物理
        // 寄存器活跃区间。
        producer->operand(0).reg = moveDst.reg;
        producer->operand(0).regClass = moveDst.regClass;
        insts[moveIndex].reset();
        foldedInBlock = true;
        changed = true;
      }
    }

    if (foldedInBlock) {
      std::vector<std::unique_ptr<MachineInst>> kept;
      kept.reserve(insts.size());
      for (auto& inst : insts) {
        if (inst)
          kept.push_back(std::move(inst));
      }
      insts = std::move(kept);
    }
  }
  return changed;
}

std::unordered_map<MachineBasicBlock*, BlockLive>
computePhysicalLiveness(MachineFunction& mf) {
  std::unordered_map<MachineBasicBlock*, BlockLive> live;

  for (auto& bbPtr : mf.blocks()) {
    BlockLive info;
    std::unordered_set<int> defined;
    for (auto& miPtr : bbPtr->insts()) {
      if (!miPtr)
        continue;
      std::vector<int> defs;
      std::vector<int> uses;
      miPtr->collectDefs(defs);
      miPtr->collectUses(uses);
      for (int use : uses) {
        if (!defined.count(use))
          info.use.insert(use);
      }
      for (int def : defs) {
        defined.insert(def);
        info.def.insert(def);
      }
    }
    live[bbPtr.get()] = std::move(info);
  }

  bool changed = true;
  while (changed) {
    changed = false;
    for (auto it = mf.blocks().rbegin(); it != mf.blocks().rend(); ++it) {
      MachineBasicBlock* bb = it->get();
      auto& info = live[bb];
      std::unordered_set<int> newOut;
      for (auto* succ : bb->succs()) {
        if (!succ)
          continue;
        auto sit = live.find(succ);
        if (sit == live.end())
          continue;
        newOut.insert(sit->second.liveIn.begin(), sit->second.liveIn.end());
      }
      std::unordered_set<int> newIn = info.use;
      for (int reg : newOut) {
        if (!info.def.count(reg))
          newIn.insert(reg);
      }
      if (newOut != info.liveOut || newIn != info.liveIn) {
        info.liveOut = std::move(newOut);
        info.liveIn = std::move(newIn);
        changed = true;
      }
    }
  }
  return live;
}

bool blockLocalDeadDefElim(MachineBasicBlock& bb,
                           const std::unordered_set<int>& liveOut) {
  bool changed = false;
  std::unordered_set<int> live = liveOut;
  std::vector<std::unique_ptr<MachineInst>> keptReversed;
  keptReversed.reserve(bb.insts().size());

  for (auto it = bb.insts().rbegin(); it != bb.insts().rend(); ++it) {
    auto& miPtr = *it;
    MachineInst* mi = miPtr.get();
    if (!mi)
      continue;

    std::unordered_set<int> liveAfter = live;
    std::vector<int> defs;
    std::vector<int> uses;
    mi->collectDefs(defs);
    mi->collectUses(uses);

    bool drop = false;
    if (isRedundantLocalOp(*mi)) {
      drop = true;
    } else if (isTriviallyDeadableOp(mi->opcode()) && !defs.empty()) {
      drop = true;
      for (int def : defs) {
        if (liveAfter.count(def)) {
          drop = false;
          break;
        }
      }
      if (drop) {
        for (int use : uses) {
          if (isScratchReg(use)) {
            drop = false;
            break;
          }
        }
      }
    }

    if (drop) {
      changed = true;
    } else {
      for (int def : defs)
        live.erase(def);
      for (int use : uses)
        live.insert(use);
      keptReversed.push_back(std::move(miPtr));
    }
  }

  std::vector<std::unique_ptr<MachineInst>> kept;
  kept.reserve(keptReversed.size());
  for (auto rit = keptReversed.rbegin(); rit != keptReversed.rend(); ++rit) {
    if (*rit)
      kept.push_back(std::move(*rit));
  }
  bb.insts() = std::move(kept);
  return changed;
}

int findUnusedCallerSavedGPR(
    const MachineLoop& loop,
    const std::unordered_map<MachineBasicBlock*, BlockLive>& live) {
  bool used[32] = {};
  for (MachineBasicBlock* block : loop.blocks) {
    if (!block)
      continue;
    auto liveIt = live.find(block);
    if (liveIt != live.end()) {
      for (int reg : liveIt->second.liveIn)
        if (isGPR(reg))
          used[reg] = true;
      for (int reg : liveIt->second.liveOut)
        if (isGPR(reg))
          used[reg] = true;
    }
    for (const auto& inst : block->insts()) {
      for (const MOperand& operand : inst->operands()) {
        if (operand.isReg() && isGPR(operand.reg))
          used[operand.reg] = true;
      }
    }
  }
  if (loop.preheader) {
    for (const auto& inst : loop.preheader->insts()) {
      for (const MOperand& operand : inst->operands()) {
        if (operand.isReg() && isGPR(operand.reg))
          used[operand.reg] = true;
      }
    }
  }
  for (int reg : {gpr::t0, gpr::t1, gpr::t2, gpr::t3, gpr::t4, gpr::t5,
                  gpr::t6}) {
    if (!used[reg])
      return reg;
  }
  return -1;
}

bool combineLoopAddressStride(MachineFunction& mf) {
  if (hasVirtualOperands(mf))
    return false;

  auto loops = collectNaturalLoops(mf);
  std::sort(loops.begin(), loops.end(), [](const MachineLoop& lhs,
                                           const MachineLoop& rhs) {
    return lhs.blocks.size() < rhs.blocks.size();
  });
  const auto live = computePhysicalLiveness(mf);
  bool changed = false;
  for (const MachineLoop& loop : loops) {
    if (!loop.header || !loop.preheader || loopContainsCall(loop))
      continue;
    int strideReg = -1;
    int64_t strideValue = 0;
    std::vector<std::pair<MachineBasicBlock*, size_t>> pairs;

    // 合并同块内对同一寄存器连续的两条 ADDI：立即数之和在范围内则直接合并；
    // 否则把步长常量提升到 preheader，用一条 ADD 代替。
    for (MachineBasicBlock* block : loop.blocks) {
      if (!block)
        continue;
      auto& insts = block->insts();
      for (size_t firstIndex = 0; firstIndex < insts.size(); ++firstIndex) {
        MachineInst* first = insts[firstIndex].get();
        if (!first || first->opcode() != MIOp::ADDI ||
            first->numOperands() != 3 || !first->operand(0).isReg() ||
            !first->operand(0).isDef || !first->operand(1).isReg() ||
            !first->operand(2).isImm() ||
            first->operand(0).reg != first->operand(1).reg ||
            first->operand(2).imm == 0)
          continue;
        const int reg = first->operand(0).reg;
        for (size_t secondIndex = firstIndex + 1; secondIndex < insts.size();
             ++secondIndex) {
          MachineInst* second = insts[secondIndex].get();
          if (!second || second->opcode() != MIOp::ADDI ||
              second->numOperands() != 3 || !second->operand(0).isReg() ||
              !second->operand(0).isDef || !second->operand(1).isReg() ||
              !second->operand(2).isImm() ||
              second->operand(0).reg != reg ||
              second->operand(1).reg != reg)
            continue;
          bool regTouched = false;
          for (size_t middle = firstIndex + 1; middle < secondIndex;
               ++middle) {
            std::vector<int> defs;
            std::vector<int> uses;
            insts[middle]->collectDefs(defs);
            insts[middle]->collectUses(uses);
            if (std::find(defs.begin(), defs.end(), reg) != defs.end() ||
                std::find(uses.begin(), uses.end(), reg) != uses.end()) {
              regTouched = true;
              break;
            }
          }
          if (regTouched)
            break;
          const int64_t total = first->operand(2).imm +
                                second->operand(2).imm;
          if (total >= -2048 && total <= 2047) {
            second->operand(2).imm = total;
            insts.erase(insts.begin() + firstIndex);
            changed = true;
          } else {
            if (strideReg < 0) {
              strideReg = findUnusedCallerSavedGPR(loop, live);
              if (strideReg < 0)
                break;
              strideValue = total;
              auto load = std::make_unique<MachineInst>(MIOp::LI);
              load->addOperand(MOperand::Def(strideReg, RegClass::GPR));
              load->addOperand(MOperand::Imm(strideValue));
              insertBeforeTerminator(loop.preheader, std::move(load));
            }
            if (strideValue != total)
              break;
            second->setOpcode(MIOp::ADD);
            second->operand(1) = MOperand::Use(reg, RegClass::GPR);
            second->operand(2) =
                MOperand::Use(strideReg, RegClass::GPR);
            insts.erase(insts.begin() + firstIndex);
            changed = true;
          }
          break;
        }
      }
    }
  }
  return changed;
}

bool findReachingDefBefore(
    const std::vector<std::unique_ptr<MachineInst>>& insts,
    size_t beforeIndex, int reg, size_t& defIndex) {
  if (!isPhysReg(reg) || beforeIndex > insts.size())
    return false;
  for (size_t i = beforeIndex; i > 0; --i) {
    MachineInst* inst = insts[i - 1].get();
    if (!inst)
      continue;
    if (mioIsCall(inst->opcode()))
      return false;
    if (mentionsReg(*inst, reg, false, true)) {
      defIndex = i - 1;
      return true;
    }
  }
  return false;
}

bool rangeTouchesReg(
    const std::vector<std::unique_ptr<MachineInst>>& insts,
    size_t beginIndex, size_t endIndex, int reg,
    bool asUse, bool asDef) {
  if (beginIndex > endIndex || endIndex > insts.size())
    return true;
  for (size_t i = beginIndex; i < endIndex; ++i) {
    MachineInst* inst = insts[i].get();
    if (!inst)
      continue;
    if (mioIsCall(inst->opcode()) ||
        mentionsReg(*inst, reg, asUse, asDef))
      return true;
  }
  return false;
}

// 对 2^k 的有符号除法，截断偏置可以写成 `(x >> 31) & (2^k - 1)`，
// 也可以写成 `unsigned(x) >> (32 - k)`；后者只需一条 RISC-V 指令，并能缩短
// 依赖链。
bool combineSignedPow2Bias(MachineFunction& mf) {
  if (hasVirtualOperands(mf))
    return false;

  const auto live = computePhysicalLiveness(mf);
  bool changed = false;
  for (auto& blockPtr : mf.blocks()) {
    MachineBasicBlock* block = blockPtr.get();
    if (!block)
      continue;
    auto& insts = block->insts();
    std::vector<bool> drop(insts.size(), false);
    auto liveIt = live.find(block);
    const std::unordered_set<int> empty;
    const auto& liveOut =
        liveIt == live.end() ? empty : liveIt->second.liveOut;

    for (size_t i = 0; i < insts.size(); ++i) {
      MachineInst* mask = insts[i].get();
      if (!mask || mask->opcode() != MIOp::ANDI ||
          mask->numOperands() != 3 ||
          !mask->operand(0).isReg() || !mask->operand(0).isDef ||
          !mask->operand(1).isReg() || !mask->operand(2).isImm())
        continue;

      const int64_t rawMask = mask->operand(2).imm;
      if (rawMask <= 0 || rawMask > 0x7fffffffLL)
        continue;
      const uint32_t biasMask = static_cast<uint32_t>(rawMask);
      // 只有 k == 1 可以由一次逻辑移位表示；k > 1 时，unsigned(x) >> (32-k)
      // 取出的是高位数值位，而不是 k 个符号位副本，因此不能覆盖所有负数。
      if (biasMask != 1u)
        continue;
      constexpr int shiftAmount = 31;

      const int biasReg = mask->operand(0).reg;
      const int signReg = mask->operand(1).reg;
      size_t signIndex = 0;
      if (!findReachingDefBefore(insts, i, signReg, signIndex) ||
          drop[signIndex])
        continue;
      MachineInst* sign = insts[signIndex].get();
      if (!sign || sign->opcode() != MIOp::SRAIW ||
          sign->numOperands() != 3 ||
          !sign->operand(0).isReg() || !sign->operand(0).isDef ||
          sign->operand(0).reg != signReg ||
          !sign->operand(1).isReg() ||
          !sign->operand(2).isImm() || sign->operand(2).imm != 31)
        continue;
      const int sourceReg = sign->operand(1).reg;
      if (!isPhysReg(sourceReg) || sourceReg == signReg ||
          rangeTouchesReg(insts, signIndex + 1, i, sourceReg,
                          false, true))
        continue;

      mask->setOpcode(MIOp::SRLIW);
      mask->operand(1) = MOperand::Use(sourceReg, RegClass::GPR);
      mask->operand(2) = MOperand::Imm(shiftAmount);
      mask->comment = "const-div.pow2.bias-fold";

      const bool signUsedBetween =
          rangeTouchesReg(insts, signIndex + 1, i, signReg,
                          true, false);
      const bool signUsedAfter =
          biasReg != signReg &&
          regUsedBeforeRedefOrLiveOut(*block, i + 1, signReg, liveOut);
      if (!signUsedBetween && !signUsedAfter)
        drop[signIndex] = true;
      changed = true;
    }

    std::vector<std::unique_ptr<MachineInst>> kept;
    kept.reserve(insts.size());
    for (size_t i = 0; i < insts.size(); ++i) {
      if (!drop[i] && insts[i])
        kept.push_back(std::move(insts[i]));
    }
    insts = std::move(kept);
  }
  return changed;
}

bool branchTargetProvesZero(const MachineInst& branch,
                            MachineBasicBlock* target,
                            int& testedReg) {
  testedReg = -1;
  if (!target || branch.numOperands() < 2 ||
      !branch.operand(branch.numOperands() - 1).isMBB() ||
      branch.operand(branch.numOperands() - 1).mbb != target)
    return false;

  if (branch.opcode() == MIOp::BEQZ && branch.numOperands() == 2 &&
      branch.operand(0).isReg()) {
    testedReg = branch.operand(0).reg;
    return isPhysReg(testedReg);
  }
  if (branch.opcode() != MIOp::BEQ || branch.numOperands() != 3 ||
      !branch.operand(0).isReg() || !branch.operand(1).isReg())
    return false;
  if (branch.operand(0).reg == gpr::x0)
    testedReg = branch.operand(1).reg;
  else if (branch.operand(1).reg == gpr::x0)
    testedReg = branch.operand(0).reg;
  return isPhysReg(testedReg);
}

// 有符号除以二通常需要在算术右移前做符号修正；在 `(x & 1) == 0` 的边上，
// x 为偶数，正负数右移都已精确。折叠前匹配完整的物理寄存器定义-使用链和
// 对应的前驱条件。
bool simplifyEvenPathSignedHalving(MachineFunction& mf) {
  if (hasVirtualOperands(mf))
    return false;

  const auto live = computePhysicalLiveness(mf);
  bool changed = false;
  for (const auto& blockPtr : mf.blocks()) {
    MachineBasicBlock* block = blockPtr.get();
    if (!block || block->preds().size() != 1)
      continue;
    MachineBasicBlock* pred = block->preds().front();
    MachineInst* branch = pred ? pred->terminator() : nullptr;
    int parityReg = -1;
    if (!branch || !branchTargetProvesZero(*branch, block, parityReg))
      continue;

    const auto& predInsts = pred->insts();
    size_t branchIndex = predInsts.size();
    for (size_t i = 0; i < predInsts.size(); ++i) {
      if (predInsts[i].get() == branch) {
        branchIndex = i;
        break;
      }
    }
    size_t parityIndex = 0;
    if (branchIndex == predInsts.size() ||
        !findReachingDefBefore(predInsts, branchIndex, parityReg,
                              parityIndex))
      continue;
    MachineInst* parity = predInsts[parityIndex].get();
    if (!parity || parity->opcode() != MIOp::ANDI ||
        parity->numOperands() != 3 ||
        !parity->operand(0).isReg() || !parity->operand(0).isDef ||
        parity->operand(0).reg != parityReg ||
        !parity->operand(1).isReg() ||
        !parity->operand(2).isImm() || parity->operand(2).imm != 1)
      continue;
    const int dividendReg = parity->operand(1).reg;
    if (!isPhysReg(dividendReg) || dividendReg == parityReg ||
        rangeTouchesReg(predInsts, parityIndex + 1, branchIndex,
                        dividendReg, false, true))
      continue;

    auto& insts = block->insts();
    for (size_t shiftIndex = 0; shiftIndex < insts.size(); ++shiftIndex) {
      MachineInst* shift = insts[shiftIndex].get();
      if (!shift || shift->opcode() != MIOp::SRAIW ||
          shift->numOperands() != 3 ||
          !shift->operand(0).isReg() || !shift->operand(0).isDef ||
          !shift->operand(1).isReg() ||
          !shift->operand(2).isImm() || shift->operand(2).imm != 1)
        continue;
      const int adjustedReg = shift->operand(1).reg;
      size_t addIndex = 0;
      if (!findReachingDefBefore(insts, shiftIndex, adjustedReg, addIndex))
        continue;
      MachineInst* add = insts[addIndex].get();
      if (!add || add->opcode() != MIOp::ADDW || add->numOperands() != 3 ||
          !add->operand(0).isReg() || !add->operand(0).isDef ||
          add->operand(0).reg != adjustedReg ||
          !add->operand(1).isReg() || !add->operand(2).isReg())
        continue;

      int biasReg = -1;
      if (add->operand(1).reg == dividendReg)
        biasReg = add->operand(2).reg;
      else if (add->operand(2).reg == dividendReg)
        biasReg = add->operand(1).reg;
      else
        continue;
      if (!isPhysReg(biasReg) || biasReg == dividendReg ||
          rangeTouchesReg(insts, addIndex + 1, shiftIndex,
                          adjustedReg, true, true))
        continue;

      size_t biasIndex = 0;
      if (!findReachingDefBefore(insts, addIndex, biasReg, biasIndex))
        continue;
      MachineInst* bias = insts[biasIndex].get();
      if (!bias || bias->opcode() != MIOp::ANDI ||
          bias->numOperands() != 3 ||
          !bias->operand(0).isReg() || !bias->operand(0).isDef ||
          bias->operand(0).reg != biasReg ||
          !bias->operand(1).isReg() ||
          !bias->operand(2).isImm() || bias->operand(2).imm != 1 ||
          rangeTouchesReg(insts, biasIndex + 1, addIndex,
                          biasReg, true, true))
        continue;

      const int signReg = bias->operand(1).reg;
      if (!isPhysReg(signReg) || signReg == dividendReg)
        continue;
      size_t signIndex = 0;
      if (!findReachingDefBefore(insts, biasIndex, signReg, signIndex))
        continue;
      MachineInst* sign = insts[signIndex].get();
      if (!sign || sign->opcode() != MIOp::SRAIW ||
          sign->numOperands() != 3 ||
          !sign->operand(0).isReg() || !sign->operand(0).isDef ||
          sign->operand(0).reg != signReg ||
          !sign->operand(1).isReg() ||
          sign->operand(1).reg != dividendReg ||
          !sign->operand(2).isImm() || sign->operand(2).imm != 31 ||
          rangeTouchesReg(insts, signIndex + 1, biasIndex,
                          signReg, true, true) ||
          rangeTouchesReg(insts, 0, signIndex, dividendReg, false, true))
        continue;

      auto liveIt = live.find(block);
      const std::unordered_set<int> empty;
      const auto& liveOut =
          liveIt == live.end() ? empty : liveIt->second.liveOut;
      if ((signReg != biasReg &&
           regUsedBeforeRedefOrLiveOut(*block, biasIndex + 1,
                                       signReg, liveOut)) ||
          (biasReg != adjustedReg &&
           regUsedBeforeRedefOrLiveOut(*block, addIndex + 1,
                                       biasReg, liveOut)) ||
          (adjustedReg != shift->operand(0).reg &&
           regUsedBeforeRedefOrLiveOut(*block, shiftIndex + 1,
                                       adjustedReg, liveOut)))
        continue;

      shift->operand(1) = MOperand::Use(dividendReg, RegClass::GPR);
      shift->comment = "even-path.exact-halving";
      std::vector<size_t> drop = {signIndex, biasIndex, addIndex};
      std::sort(drop.begin(), drop.end());
      for (auto it = drop.rbegin(); it != drop.rend(); ++it)
        insts.erase(insts.begin() + *it);
      changed = true;
      break;
    }
  }
  return changed;
}

void rewriteAsGPRMove(MachineInst& inst, int dst, int src) {
  inst.setOpcode(MIOp::MV);
  inst.operands().clear();
  inst.addOperand(MOperand::Def(dst, RegClass::GPR));
  inst.addOperand(MOperand::Use(src, RegClass::GPR));
}

void rewriteGPRBinary(MachineInst& inst, MIOp opcode, int dst,
                      int lhs, int rhs) {
  inst.setOpcode(opcode);
  inst.operands().clear();
  inst.addOperand(MOperand::Def(dst, RegClass::GPR));
  inst.addOperand(MOperand::Use(lhs, RegClass::GPR));
  inst.addOperand(MOperand::Use(rhs, RegClass::GPR));
}

void rewriteGPRBinaryImm(MachineInst& inst, MIOp opcode, int dst,
                         int src, int64_t imm) {
  inst.setOpcode(opcode);
  inst.operands().clear();
  inst.addOperand(MOperand::Def(dst, RegClass::GPR));
  inst.addOperand(MOperand::Use(src, RegClass::GPR));
  inst.addOperand(MOperand::Imm(imm));
}

// 条件转换可以把下面的形式
//
//   if (((x & 1) & (y & 1)) == 0)
//     acc += product;
//
// 转换为通用的 xor/and/xor select。直接构造 -1/0 奇偶掩码并屏蔽贡献值，
// 同时缩短依赖链，减少频繁执行的归约步骤中的指令。
bool combineParityMaskedAccumulation(MachineFunction& mf) {
  if (hasVirtualOperands(mf))
    return false;

  const auto live = computePhysicalLiveness(mf);
  bool changed = false;
  for (auto& blockPtr : mf.blocks()) {
    MachineBasicBlock* block = blockPtr.get();
    if (!block)
      continue;
    auto& insts = block->insts();
    auto liveIt = live.find(block);
    const std::unordered_set<int> empty;
    const auto& liveOut =
        liveIt == live.end() ? empty : liveIt->second.liveOut;
    std::vector<bool> drop(insts.size(), false);
    bool blockChanged = false;

    // 奇偶校验模式匹配前半：识别左右操作数各用 ANDI 1 取奇偶位、
    // 相与后用 SLTIU 判偶的指令形状（evenTest 允许在窗口内浮动）。
    for (size_t i = 0; i + 10 < insts.size(); ++i) {
      MachineInst* leftMask = insts[i].get();
      MachineInst* rightMask = insts[i + 1].get();
      MachineInst* parityAnd = insts[i + 2].get();
      const size_t evenSearchEnd = std::min(insts.size(), i + 7);
      size_t evenIndex = i + 3;
      while (evenIndex < evenSearchEnd &&
             (!insts[evenIndex] ||
              insts[evenIndex]->opcode() != MIOp::SLTIU))
        ++evenIndex;
      MachineInst* evenTest =
          evenIndex < evenSearchEnd ? insts[evenIndex].get() : nullptr;
      if (!leftMask || !rightMask || !parityAnd || !evenTest ||
          leftMask->opcode() != MIOp::ANDI ||
          rightMask->opcode() != MIOp::ANDI ||
          parityAnd->opcode() != MIOp::AND_ ||
          evenTest->opcode() != MIOp::SLTIU ||
          leftMask->numOperands() != 3 ||
          rightMask->numOperands() != 3 ||
          parityAnd->numOperands() != 3 ||
          evenTest->numOperands() != 3 ||
          !leftMask->operand(0).isReg() ||
          !leftMask->operand(0).isDef ||
          !leftMask->operand(1).isReg() ||
          !leftMask->operand(2).isImm() ||
          leftMask->operand(2).imm != 1 ||
          !rightMask->operand(0).isReg() ||
          !rightMask->operand(0).isDef ||
          !rightMask->operand(1).isReg() ||
          !rightMask->operand(2).isImm() ||
          rightMask->operand(2).imm != 1 ||
          !parityAnd->operand(0).isReg() ||
          !parityAnd->operand(0).isDef ||
          !parityAnd->operand(1).isReg() ||
          !parityAnd->operand(2).isReg() ||
          !evenTest->operand(0).isReg() ||
          !evenTest->operand(0).isDef ||
          !evenTest->operand(1).isReg() ||
          !evenTest->operand(2).isImm() ||
          evenTest->operand(2).imm != 1)
        continue;

      const int leftParity = leftMask->operand(0).reg;
      const int rightParity = rightMask->operand(0).reg;
      const int leftValue = leftMask->operand(1).reg;
      const int rightValue = rightMask->operand(1).reg;
      const int parity = parityAnd->operand(0).reg;
      const int evenCond = evenTest->operand(0).reg;
      if (!isPhysReg(leftParity) || !isPhysReg(rightParity) ||
          !isPhysReg(leftValue) || !isPhysReg(rightValue) ||
          !isPhysReg(parity) || !isPhysReg(evenCond) ||
          parity != leftParity ||
          (parityAnd->operand(1).reg != leftParity &&
           parityAnd->operand(2).reg != leftParity) ||
          (parityAnd->operand(1).reg != rightParity &&
           parityAnd->operand(2).reg != rightParity) ||
          evenTest->operand(1).reg != parity ||
          rangeTouchesReg(insts, i + 3, evenIndex, parity, true, true))
        continue;

      const size_t scanEnd = std::min(insts.size(), i + 24);
      size_t tail = insts.size();
      for (size_t j = evenIndex + 1; j + 6 < scanEnd; ++j) {
        if (insts[j] && insts[j]->opcode() == MIOp::MULW) {
          tail = j;
          break;
        }
      }
      if (tail == insts.size())
        continue;

      // 模式匹配后半：定位 MULW 之后的无分支符号修正序列
      // （投机加、取零、条件取反、XOR/AND 选择、最终结果校正）。
      MachineInst* multiply = insts[tail].get();
      MachineInst* speculativeAdd = insts[tail + 1].get();
      MachineInst* zero = insts[tail + 2].get();
      MachineInst* negateCond = insts[tail + 3].get();
      MachineInst* selectDelta = insts[tail + 4].get();
      MachineInst* maskedDelta = insts[tail + 5].get();
      MachineInst* selectResult = insts[tail + 6].get();
      if (!multiply || !speculativeAdd || !zero || !negateCond ||
          !selectDelta || !maskedDelta || !selectResult ||
          multiply->numOperands() != 3 ||
          multiply->opcode() != MIOp::MULW ||
          !multiply->operand(0).isReg() ||
          !multiply->operand(0).isDef ||
          speculativeAdd->opcode() != MIOp::ADDW ||
          speculativeAdd->numOperands() != 3 ||
          !speculativeAdd->operand(0).isReg() ||
          !speculativeAdd->operand(0).isDef ||
          !speculativeAdd->operand(1).isReg() ||
          !speculativeAdd->operand(2).isReg() ||
          zero->opcode() != MIOp::LI ||
          zero->numOperands() != 2 ||
          !zero->operand(0).isReg() || !zero->operand(0).isDef ||
          !zero->operand(1).isImm() || zero->operand(1).imm != 0 ||
          negateCond->opcode() != MIOp::SUBW ||
          negateCond->numOperands() != 3 ||
          !negateCond->operand(0).isReg() ||
          !negateCond->operand(0).isDef ||
          !negateCond->operand(1).isReg() ||
          !negateCond->operand(2).isReg() ||
          selectDelta->opcode() != MIOp::XOR_ ||
          selectDelta->numOperands() != 3 ||
          !selectDelta->operand(0).isReg() ||
          !selectDelta->operand(0).isDef ||
          !selectDelta->operand(1).isReg() ||
          !selectDelta->operand(2).isReg() ||
          maskedDelta->opcode() != MIOp::AND_ ||
          maskedDelta->numOperands() != 3 ||
          !maskedDelta->operand(0).isReg() ||
          !maskedDelta->operand(0).isDef ||
          !maskedDelta->operand(1).isReg() ||
          !maskedDelta->operand(2).isReg() ||
          selectResult->opcode() != MIOp::XOR_ ||
          selectResult->numOperands() != 3 ||
          !selectResult->operand(0).isReg() ||
          !selectResult->operand(0).isDef ||
          !selectResult->operand(1).isReg() ||
          !selectResult->operand(2).isReg())
        continue;

      const int product = multiply->operand(0).reg;
      const int sumPlus = speculativeAdd->operand(0).reg;
      const int accumulator =
          speculativeAdd->operand(1).reg == product
              ? speculativeAdd->operand(2).reg
              : speculativeAdd->operand(2).reg == product
                    ? speculativeAdd->operand(1).reg
                    : -1;
      const int zeroReg = zero->operand(0).reg;
      const int mask = negateCond->operand(0).reg;
      const int delta = selectDelta->operand(0).reg;
      const int maskedResult = maskedDelta->operand(0).reg;
      if (!isPhysReg(product) || !isPhysReg(accumulator) ||
          !isPhysReg(zeroReg) || !isPhysReg(mask) ||
          !isPhysReg(delta) || !isPhysReg(maskedResult) ||
          sumPlus != product || delta != product ||
          negateCond->operand(1).reg != zeroReg ||
          negateCond->operand(2).reg != evenCond ||
          !((selectDelta->operand(1).reg == accumulator &&
             selectDelta->operand(2).reg == sumPlus) ||
            (selectDelta->operand(2).reg == accumulator &&
             selectDelta->operand(1).reg == sumPlus)) ||
          !((maskedDelta->operand(1).reg == product &&
             maskedDelta->operand(2).reg == mask) ||
            (maskedDelta->operand(2).reg == product &&
             maskedDelta->operand(1).reg == mask)) ||
          selectResult->operand(0).reg != accumulator ||
          !((selectResult->operand(1).reg == accumulator &&
             selectResult->operand(2).reg == maskedResult) ||
            (selectResult->operand(2).reg == accumulator &&
             selectResult->operand(1).reg == maskedResult)))
        continue;

      const bool leftParityEscapes =
          leftParity == mask
              ? rangeTouchesReg(insts, evenIndex + 1, tail + 1,
                                leftParity, true, true)
              : regUsedBeforeRedefOrLiveOut(
                    *block, evenIndex + 1, leftParity, liveOut);
      if (leftParityEscapes ||
          regUsedBeforeRedefOrLiveOut(*block, i + 3,
                                      rightParity, liveOut) ||
          (parity != leftParity && parity != rightParity &&
           regUsedBeforeRedefOrLiveOut(*block, evenIndex + 1,
                                       parity, liveOut)) ||
          (evenCond != mask &&
           regUsedBeforeRedefOrLiveOut(*block, tail + 4,
                                       evenCond, liveOut)) ||
          regUsedBeforeRedefOrLiveOut(*block, tail + 7, product, liveOut) ||
          (maskedResult != product && maskedResult != accumulator &&
           regUsedBeforeRedefOrLiveOut(*block, tail + 7,
                                       maskedResult, liveOut)))
        continue;

      rewriteGPRBinary(*leftMask, MIOp::AND_, evenCond,
                       leftValue, rightValue);
      rewriteGPRBinaryImm(*rightMask, MIOp::ANDI, evenCond,
                          evenCond, 1);
      rewriteGPRBinaryImm(*speculativeAdd, MIOp::ADDIW, mask,
                          evenCond, -1);
      rewriteGPRBinary(*selectDelta, MIOp::AND_, product,
                       product, mask);
      rewriteGPRBinary(*maskedDelta, MIOp::ADDW, accumulator,
                       accumulator, product);
      leftMask->comment = "parity.masked-accumulate";
      drop[i + 2] = true;
      drop[evenIndex] = true;
      if (zeroReg == mask ||
          !regUsedBeforeRedefOrLiveOut(*block, tail + 4,
                                       zeroReg, liveOut))
        drop[tail + 2] = true;
      drop[tail + 3] = true;
      drop[tail + 6] = true;
      changed = true;
      blockChanged = true;
      i = tail + 6;
    }

    if (!blockChanged)
      continue;
    std::vector<std::unique_ptr<MachineInst>> kept;
    kept.reserve(insts.size());
    for (size_t i = 0; i < insts.size(); ++i)
      if (!drop[i] && insts[i])
        kept.push_back(std::move(insts[i]));
    insts = std::move(kept);
  }
  return changed;
}

// 比较和奇偶掩码已经产生标准的 0/1 值。在物理寄存器块内保留这一信息，
// 使后续降低阶段留下的 `(bool - 0) != 0` 等形式可以恢复为原布尔值。
bool simplifyKnownBooleanNormalizations(MachineFunction& mf) {
  if (hasVirtualOperands(mf))
    return false;

  bool changed = false;
  for (auto& blockPtr : mf.blocks()) {
    MachineBasicBlock* block = blockPtr.get();
    if (!block)
      continue;
    auto& insts = block->insts();
    std::unordered_set<int> knownBoolean;
    std::unordered_set<int> knownZero;
    std::vector<bool> drop(insts.size(), false);

    auto isBoolean = [&](int reg) {
      return reg == gpr::x0 || knownBoolean.count(reg) != 0;
    };
    auto isZero = [&](int reg) {
      return reg == gpr::x0 || knownZero.count(reg) != 0;
    };
    auto clearDefs = [&](MachineInst& inst) {
      std::vector<int> defs;
      inst.collectDefs(defs);
      for (int reg : defs) {
        knownBoolean.erase(reg);
        knownZero.erase(reg);
      }
    };

    for (size_t i = 0; i < insts.size(); ++i) {
      MachineInst* inst = insts[i].get();
      if (!inst)
        continue;
      if (mioIsCall(inst->opcode())) {
        knownBoolean.clear();
        knownZero.clear();
        continue;
      }

      if (inst->opcode() == MIOp::SUBW && inst->numOperands() == 3 &&
          inst->operand(0).isReg() && inst->operand(0).isDef &&
          inst->operand(1).isReg() && inst->operand(2).isReg() &&
          isBoolean(inst->operand(1).reg) &&
          isZero(inst->operand(2).reg)) {
        const int dst = inst->operand(0).reg;
        const int src = inst->operand(1).reg;
        if (dst == src) {
          drop[i] = true;
        } else {
          clearDefs(*inst);
          rewriteAsGPRMove(*inst, dst, src);
          knownBoolean.insert(dst);
          if (isZero(src))
            knownZero.insert(dst);
        }
        changed = true;
        continue;
      }

      if (inst->opcode() == MIOp::SLTU && inst->numOperands() == 3 &&
          inst->operand(0).isReg() && inst->operand(0).isDef &&
          inst->operand(1).isReg() &&
          inst->operand(1).reg == gpr::x0 &&
          inst->operand(2).isReg() &&
          isBoolean(inst->operand(2).reg)) {
        const int dst = inst->operand(0).reg;
        const int src = inst->operand(2).reg;
        if (dst == src) {
          drop[i] = true;
        } else {
          clearDefs(*inst);
          rewriteAsGPRMove(*inst, dst, src);
          knownBoolean.insert(dst);
          if (isZero(src))
            knownZero.insert(dst);
        }
        changed = true;
        continue;
      }

      if (inst->opcode() == MIOp::ANDI && inst->numOperands() == 3 &&
          inst->operand(0).isReg() && inst->operand(0).isDef &&
          inst->operand(1).isReg() && inst->operand(2).isImm() &&
          inst->operand(2).imm == 1 &&
          isBoolean(inst->operand(1).reg)) {
        const int dst = inst->operand(0).reg;
        const int src = inst->operand(1).reg;
        if (dst == src) {
          drop[i] = true;
        } else {
          clearDefs(*inst);
          rewriteAsGPRMove(*inst, dst, src);
          knownBoolean.insert(dst);
          if (isZero(src))
            knownZero.insert(dst);
        }
        changed = true;
        continue;
      }

      const MIOp opcode = inst->opcode();
      const bool hasDef =
          inst->numOperands() > 0 && inst->operand(0).isReg() &&
          inst->operand(0).isDef;
      const int dst = hasDef ? inst->operand(0).reg : -1;
      bool resultBoolean = false;
      bool resultZero = false;

      // 按操作码推导两个事实：结果是否为布尔值、结果是否恒零，
      // 供后续分支简化与冗余消除使用。
      if (opcode == MIOp::LI && inst->numOperands() == 2 &&
          inst->operand(1).isImm()) {
        resultZero = inst->operand(1).imm == 0;
        resultBoolean =
            inst->operand(1).imm == 0 || inst->operand(1).imm == 1;
      } else if ((opcode == MIOp::SLT || opcode == MIOp::SLTU ||
                  opcode == MIOp::SLTI || opcode == MIOp::SLTIU) &&
                 hasDef) {
        resultBoolean = true;
      } else if (opcode == MIOp::ANDI && inst->numOperands() == 3 &&
                 inst->operand(2).isImm() &&
                 inst->operand(2).imm == 1) {
        resultBoolean = true;
      } else if (opcode == MIOp::AND_ && inst->numOperands() == 3 &&
                 inst->operand(1).isReg() && inst->operand(2).isReg() &&
                 (isBoolean(inst->operand(1).reg) ||
                  isBoolean(inst->operand(2).reg))) {
        resultBoolean = true;
      } else if ((opcode == MIOp::OR_ || opcode == MIOp::XOR_) &&
                 inst->numOperands() == 3 &&
                 inst->operand(1).isReg() && inst->operand(2).isReg() &&
                 isBoolean(inst->operand(1).reg) &&
                 isBoolean(inst->operand(2).reg)) {
        resultBoolean = true;
      } else if (opcode == MIOp::XORI && inst->numOperands() == 3 &&
                 inst->operand(1).isReg() && inst->operand(2).isImm() &&
                 inst->operand(2).imm == 1 &&
                 isBoolean(inst->operand(1).reg)) {
        resultBoolean = true;
      } else if (opcode == MIOp::MV && inst->numOperands() == 2 &&
                 inst->operand(1).isReg()) {
        resultBoolean = isBoolean(inst->operand(1).reg);
        resultZero = isZero(inst->operand(1).reg);
      } else if ((opcode == MIOp::ADDI || opcode == MIOp::ADDIW) &&
                 inst->numOperands() == 3 &&
                 inst->operand(1).isReg() &&
                 inst->operand(1).reg == gpr::x0 &&
                 inst->operand(2).isImm()) {
        resultZero = inst->operand(2).imm == 0;
        resultBoolean =
            inst->operand(2).imm == 0 || inst->operand(2).imm == 1;
      }

      clearDefs(*inst);
      if (isPhysReg(dst)) {
        if (resultBoolean)
          knownBoolean.insert(dst);
        if (resultZero)
          knownZero.insert(dst);
      }
    }

    std::vector<std::unique_ptr<MachineInst>> kept;
    kept.reserve(insts.size());
    for (size_t i = 0; i < insts.size(); ++i) {
      if (!drop[i] && insts[i])
        kept.push_back(std::move(insts[i]));
    }
    insts = std::move(kept);
  }
  return changed;
}

bool cleanupFunction(MachineFunction& mf) {
  bool changed = false;
  if (!sycEnvironmentValue("SYC_SKIP_MIR_INVARIANT_CONSTANT_HOIST"))
    changed |= hoistLoopInvariantConstants(mf);
  if (!sycEnvironmentValue("SYC_SKIP_MIR_SYMBOL_HOIST"))
    changed |= hoistLoopLocalSymbolAddresses(mf);
  if (!sycEnvironmentValue("SYC_SKIP_MIR_LOOP_ADDRESS_REDUCTION"))
    changed |= strengthReduceLoopAddressRecompute(mf);
  changed |= combineLoopAddressStride(mf);
  const bool allowVirtualDeadDefElim = !hasVirtualOperands(mf);
  if (allowVirtualDeadDefElim &&
      !sycEnvironmentValue("SYC_SKIP_MIR_EVEN_PATH_HALVING"))
    changed |= simplifyEvenPathSignedHalving(mf);
  // 先执行基于边信息的精确折叠；否则局部偏置折叠可能先改写输入形式，
  // 使后续更大的三合一简化无法识别。
  if (allowVirtualDeadDefElim &&
      !sycEnvironmentValue("SYC_SKIP_MIR_SIGNED_POW2_BIAS_FOLD"))
    changed |= combineSignedPow2Bias(mf);
  if (allowVirtualDeadDefElim &&
      !sycEnvironmentValue("SYC_SKIP_MIR_BOOLEAN_NORMALIZATION"))
    changed |= simplifyKnownBooleanNormalizations(mf);
  for (int iter = 0; iter < 3; ++iter) {
    bool iterChanged = false;
    iterChanged |= foldProducerMoves(mf);
    iterChanged |= eliminateRedundantRegisterConstants(mf);
    iterChanged |= forwardSingleUseSpillReloads(mf);
    iterChanged |= forwardSeparatedSpillReloads(mf);
    if (!sycEnvironmentValue("SYC_SKIP_MIR_ADJACENT_COPY_FOLD"))
      iterChanged |= foldAdjacentPhysicalProducerMoves(mf);
    if (!sycEnvironmentValue("SYC_SKIP_MIR_SEPARATED_COPY_FOLD"))
      iterChanged |= foldSeparatedPhysicalProducerMoves(mf);
    if (allowVirtualDeadDefElim) {
      auto live = computePhysicalLiveness(mf);
      for (auto& bb : mf.blocks()) {
        auto it = live.find(bb.get());
        const std::unordered_set<int> empty;
        iterChanged |= blockLocalDeadDefElim(
            *bb, it == live.end() ? empty : it->second.liveOut);
      }
    }
    changed |= iterChanged;
    if (!iterChanged)
      break;
  }
  if (allowVirtualDeadDefElim &&
      !sycEnvironmentValue("SYC_SKIP_MIR_PARITY_MASKED_ACCUMULATION"))
    changed |= combineParityMaskedAccumulation(mf);
  return changed;
}

} // namespace

void runMachineCleanup(MachineFunction& mf) {
  cleanupFunction(mf);
}

void runMachineCleanup(MachineModule& mm) {
  for (auto& fn : mm.functions())
    runMachineCleanup(*fn);
}

void prepareEarlyReturnShrinkWrapping(MachineModule& mm) {
  for (auto& fnPtr : mm.functions()) {
    MachineFunction& mf = *fnPtr;
    MachineBasicBlock* entry = mf.entryBlock();
    if (!entry || entry->succs().size() != 2)
      continue;

    auto isDirectReturn = [entry](MachineBasicBlock* block) {
      if (!block || block->preds().size() != 1 ||
          block->preds().front() != entry || !block->succs().empty() ||
          block->insts().size() > 4)
        return false;
      MachineInst* term = block->terminator();
      if (!term || term->opcode() != MIOp::RET)
        return false;
      for (const auto& miPtr : block->insts()) {
        if (!miPtr || mioIsCall(miPtr->opcode()) ||
            miPtr->opcode() == MIOp::PHI)
          return false;
        for (const MOperand& operand : miPtr->operands()) {
          if (operand.isFrame())
            return false;
        }
      }
      return true;
    };

    MachineBasicBlock* first = entry->succs()[0];
    MachineBasicBlock* second = entry->succs()[1];
    const bool firstReturns = isDirectReturn(first);
    const bool secondReturns = isDirectReturn(second);
    if (firstReturns == secondReturns)
      continue;
    MachineBasicBlock* fastReturn = firstReturns ? first : second;
    MachineBasicBlock* frameEntry = firstReturns ? second : first;
    if (!frameEntry || frameEntry->preds().size() != 1 ||
        frameEntry->preds().front() != entry)
      continue;

    std::unordered_set<MachineBasicBlock*> framed;
    std::queue<MachineBasicBlock*> worklist;
    framed.insert(frameEntry);
    worklist.push(frameEntry);
    while (!worklist.empty()) {
      MachineBasicBlock* block = worklist.front();
      worklist.pop();
      for (MachineBasicBlock* succ : block->succs()) {
        if (succ != entry && succ != fastReturn && framed.insert(succ).second)
          worklist.push(succ);
      }
    }
    if (framed.count(fastReturn))
      continue;

    std::unordered_set<int> entryDefs;
    for (const auto& miPtr : entry->insts()) {
      if (!miPtr)
        continue;
      std::vector<int> defs;
      miPtr->collectDefs(defs);
      for (int reg : defs) {
        if (isVirtualReg(reg))
          entryDefs.insert(reg);
      }
    }

    struct Split {
      int oldReg = -1;
      int newReg = -1;
      RegClass cls = RegClass::GPR;
    };
    std::vector<Split> splits;
    for (int oldReg : entryDefs) {
      bool usedInFrame = false;
      for (MachineBasicBlock* block : framed) {
        for (const auto& miPtr : block->insts()) {
          if (!miPtr)
            continue;
          for (const MOperand& operand : miPtr->operands()) {
            if (operand.isReg() && !operand.isDef &&
                operand.reg == oldReg) {
              usedInFrame = true;
              break;
            }
          }
          if (usedInFrame)
            break;
        }
        if (usedInFrame)
          break;
      }
      if (!usedInFrame)
        continue;
      const VRegInfo oldInfo = mf.vregInfo(oldReg);
      const int newReg = mf.newVReg(oldInfo.cls);
      VRegInfo& newInfo = mf.vregInfo(newReg);
      newInfo.crossesCall = oldInfo.crossesCall;
      newInfo.loopDepth = oldInfo.loopDepth;
      newInfo.hotLoopWeight = oldInfo.hotLoopWeight;
      newInfo.preferCallerSaved = oldInfo.preferCallerSaved;
      newInfo.rematerializable = false;
      splits.push_back({oldReg, newReg, oldInfo.cls});
    }
    if (splits.empty())
      continue;

    for (MachineBasicBlock* block : framed) {
      for (auto& miPtr : block->insts()) {
        if (!miPtr)
          continue;
        for (MOperand& operand : miPtr->operands()) {
          if (!operand.isReg() || operand.isDef)
            continue;
          for (const Split& split : splits) {
            if (operand.reg == split.oldReg) {
              operand.reg = split.newReg;
              break;
            }
          }
        }
      }
    }

    std::vector<std::unique_ptr<MachineInst>> prefix;
    prefix.reserve(splits.size() + frameEntry->insts().size());
    for (const Split& split : splits) {
      auto move = std::make_unique<MachineInst>(MIOp::MV);
      move->addOperand(MOperand::Def(split.newReg, split.cls));
      move->addOperand(MOperand::Use(split.oldReg, split.cls));
      move->comment = "early-return.slow-copy";
      move->setParent(frameEntry);
      prefix.push_back(std::move(move));
    }
    for (auto& miPtr : frameEntry->insts())
      prefix.push_back(std::move(miPtr));
    frameEntry->insts() = std::move(prefix);
  }
}

bool removeRedundantPostRAStores(MachineModule& mm) {
  bool changed = false;
  for (auto& fn : mm.functions()) {
    if (fn) {
      changed |= removeRedundantPostRAStoresInFunction(*fn);
      changed |= hoistInvariantPostRALoadsInFunction(*fn);
    }
  }
  return changed;
}

} // namespace mir
