#include "research_switch.h"
// 文件职责：在 MIR 中调整条件分支布局，减少不必要的跳转。
#include "loop_branch_rotate.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mir {
namespace {

MIOp invertBranch(MIOp op) {
  switch (op) {
    case MIOp::BEQ: return MIOp::BNE;
    case MIOp::BNE: return MIOp::BEQ;
    case MIOp::BLT: return MIOp::BGE;
    case MIOp::BGE: return MIOp::BLT;
    case MIOp::BLTU: return MIOp::BGEU;
    case MIOp::BGEU: return MIOp::BLTU;
    case MIOp::BEQZ: return MIOp::BNEZ;
    case MIOp::BNEZ: return MIOp::BEQZ;
    case MIOp::BLTZ: return MIOp::BGEZ;
    case MIOp::BGEZ: return MIOp::BLTZ;
    case MIOp::BLEZ: return MIOp::BGTZ;
    case MIOp::BGTZ: return MIOp::BLEZ;
    default: return MIOp::COUNT;
  }
}

MachineBasicBlock* targetOf(const MachineInst* inst) {
  if (!inst || inst->numOperands() == 0) return nullptr;
  const MOperand& target = inst->operand(inst->numOperands() - 1);
  return target.isMBB() ? target.mbb : nullptr;
}

bool contains(const std::vector<MachineBasicBlock*>& blocks,
              MachineBasicBlock* block) {
  return std::find(blocks.begin(), blocks.end(), block) != blocks.end();
}

void eraseBlock(std::vector<MachineBasicBlock*>& blocks,
                MachineBasicBlock* block) {
  blocks.erase(std::remove(blocks.begin(), blocks.end(), block), blocks.end());
}

void addUnique(std::vector<MachineBasicBlock*>& blocks,
               MachineBasicBlock* block) {
  if (block && !contains(blocks, block)) blocks.push_back(block);
}

bool canReachWithout(MachineBasicBlock* start, MachineBasicBlock* goal,
                     MachineBasicBlock* forbidden) {
  if (!start || !goal || start == forbidden) return false;
  std::vector<MachineBasicBlock*> worklist{start};
  std::unordered_set<MachineBasicBlock*> visited;
  while (!worklist.empty()) {
    MachineBasicBlock* block = worklist.back();
    worklist.pop_back();
    if (!block || block == forbidden || !visited.insert(block).second) continue;
    if (block == goal) return true;
    for (MachineBasicBlock* succ : block->succs()) {
      if (succ != forbidden) worklist.push_back(succ);
    }
  }
  return false;
}

bool isJumpTo(const MachineInst* inst, MachineBasicBlock* target) {
  return inst && inst->opcode() == MIOp::J && inst->numOperands() == 1 &&
         inst->operand(0).isMBB() && inst->operand(0).mbb == target;
}

struct Candidate {
  MachineBasicBlock* header = nullptr;
  MachineBasicBlock* body = nullptr;
  MachineBasicBlock* exit = nullptr;
  MachineBasicBlock* latch = nullptr;
  MachineInst* branch = nullptr;
  MIOp latchBranchOp = MIOp::COUNT;
};

bool matchCandidate(MachineBasicBlock* header, Candidate& candidate) {
  if (!header || header == header->parent()->entryBlock()) return false;
  if (header->succs().size() != 2 || header->preds().size() != 2) return false;

  // 循环头部只能包含条件控制转移以及可选的显式跳转。头部不能定义需要在
  // 旋转回边上重新计算的分支操作数。
  MachineInst* branch = nullptr;
  MachineInst* jump = nullptr;
  for (auto& instPtr : header->insts()) {
    MachineInst* inst = instPtr.get();
    if (mioIsBranch(inst->opcode())) {
      if (branch) return false;
      branch = inst;
    } else if (inst->opcode() == MIOp::J) {
      if (jump) return false;
      jump = inst;
    } else if (inst->opcode() != MIOp::PHI && inst->opcode() != MIOp::PHI_COPY) {
      return false;
    }
  }
  if (!branch || !targetOf(branch)) return false;
  if (jump && header->insts().back().get() != jump) return false;

  MachineBasicBlock* first = header->succs()[0];
  MachineBasicBlock* second = header->succs()[1];
  MachineBasicBlock* latch = nullptr;
  MachineBasicBlock* body = nullptr;
  MachineBasicBlock* exit = nullptr;
  for (MachineBasicBlock* pred : header->preds()) {
    if (!pred || pred->insts().empty() ||
        !isJumpTo(pred->insts().back().get(), header))
      continue;
    if (pred != first && pred != second)
      continue;
    latch = pred;
    body = pred;
    exit = pred == first ? second : first;
    break;
  }
  if (!latch) {
    for (MachineBasicBlock* pred : header->preds()) {
      if (!pred || pred->insts().empty() ||
          !isJumpTo(pred->insts().back().get(), header))
        continue;
      bool firstLoops = canReachWithout(first, pred, header);
      bool secondLoops = canReachWithout(second, pred, header);
      if (firstLoops == secondLoops) continue;
      if (latch) return false;
      latch = pred;
      body = firstLoops ? first : second;
      exit = firstLoops ? second : first;
    }
  }
  if (!latch || latch->succs().size() != 1 || latch->succs()[0] != header)
    return false;

  // 不能有更早的控制转移绕过循环锁存块末尾的跳转。
  for (size_t i = 0; i + 1 < latch->insts().size(); ++i) {
    if (mioIsTerminator(latch->insts()[i]->opcode())) return false;
  }

  MachineBasicBlock* branchTarget = targetOf(branch);
  MIOp latchOp = branch->opcode();
  if (branchTarget == exit) latchOp = invertBranch(latchOp);
  else if (branchTarget != body) return false;
  if (latchOp == MIOp::COUNT) return false;

  candidate = {header, body, exit, latch, branch, latchOp};
  return true;
}

bool isMovablePreheaderOp(const MachineInst* inst) {
  if (!inst || mioIsTerminator(inst->opcode()) || mioIsBranch(inst->opcode()) ||
      mioMayLoad(inst->opcode()) || mioMayStore(inst->opcode()) ||
      mioIsCall(inst->opcode()) || inst->opcode() == MIOp::PHI ||
      inst->opcode() == MIOp::PHI_COPY)
    return false;
  std::vector<int> defs;
  inst->collectDefs(defs);
  return !defs.empty();
}

struct RecomputePlan {
  MachineInst* inst = nullptr;
  MachineInst* producer = nullptr;
  int clobberedReg = -1;
  int scratchReg = -1;
  size_t sourceIndex = 0;
  size_t producerIndex = 0;
};

bool recomputeClobberedPreheaderProducts(
    MachineBasicBlock* source, MachineBasicBlock* body,
    MachineBasicBlock* sink, size_t terminatorIndex) {
  if (!source || !body || !sink || source->insts().empty())
    return false;
  terminatorIndex = source->insts().size() - 1;

  std::unordered_map<const MachineInst*, size_t> positions;
  for (size_t i = 0; i < source->insts().size(); ++i)
    positions[source->insts()[i].get()] = i;

  std::vector<RecomputePlan> plans;
  for (size_t i = 0; i < terminatorIndex; ++i) {
    MachineInst* inst = source->insts()[i].get();
    if (!inst || inst->opcode() != MIOp::FMUL_S || inst->numOperands() != 3)
      continue;
    std::vector<int> defs;
    inst->collectDefs(defs);
    if (defs.size() != 1 || !isFPR(defs.front()))
      continue;

    bool usesOnlyInSink = true;
    for (const auto& block : source->parent()->blocks()) {
      for (const auto& userPtr : block->insts()) {
        MachineInst* user = userPtr.get();
        std::vector<int> uses;
        user->collectUses(uses);
        if (std::find(uses.begin(), uses.end(), defs.front()) == uses.end())
          continue;
        if (user->parent() == source) {
          auto it = positions.find(user);
          if (it != positions.end() && it->second <= i)
            continue;
        }
        if (user->parent() == body) {
          bool overwrittenInSink = false;
          for (const auto& sinkInst : sink->insts()) {
            std::vector<int> sinkDefs;
            sinkInst->collectDefs(sinkDefs);
            if (std::find(sinkDefs.begin(), sinkDefs.end(), defs.front()) !=
                sinkDefs.end()) {
              overwrittenInSink = true;
              break;
            }
          }
          if (overwrittenInSink)
            continue;
        }
        if (user->parent() != sink) {
          usesOnlyInSink = false;
          break;
        }
      }
      if (!usesOnlyInSink)
        break;
    }
    if (!usesOnlyInSink) {
      continue;
    }

    std::vector<int> operandRegs;
    for (size_t operandIndex = 1; operandIndex < inst->numOperands();
         ++operandIndex) {
      const MOperand& operand = inst->operand(operandIndex);
      if (operand.isReg() && isFPR(operand.reg))
        operandRegs.push_back(operand.reg);
    }
    if (operandRegs.size() != 2)
      continue;

    int clobberedReg = -1;
    for (int reg : operandRegs) {
      bool redefined = false;
      for (size_t later = i + 1; later < terminatorIndex; ++later) {
        std::vector<int> laterDefs;
        source->insts()[later]->collectDefs(laterDefs);
        if (std::find(laterDefs.begin(), laterDefs.end(), reg) !=
            laterDefs.end()) {
          redefined = true;
          break;
        }
      }
      if (!redefined)
        continue;
      if (clobberedReg != -1) {
        clobberedReg = -1;
        break;
      }
      clobberedReg = reg;
    }
    if (clobberedReg == -1) {
      plans.push_back({inst, nullptr, -1, -1, i, 0});
      continue;
    }

    MachineInst* producer = nullptr;
    size_t producerIndex = 0;
    for (size_t earlier = 0; earlier < i; ++earlier) {
      std::vector<int> earlierDefs;
      source->insts()[earlier]->collectDefs(earlierDefs);
      if (std::find(earlierDefs.begin(), earlierDefs.end(), clobberedReg) ==
          earlierDefs.end())
        continue;
      producer = source->insts()[earlier].get();
      producerIndex = earlier;
    }
    if (!producer || (producer->opcode() != MIOp::FSUB_S &&
                      producer->opcode() != MIOp::FADD_S) ||
        producer->numOperands() != 3) {
      continue;
    }

    bool producerInputsStable = true;
    for (size_t operandIndex = 1; operandIndex < producer->numOperands();
         ++operandIndex) {
      const MOperand& operand = producer->operand(operandIndex);
      if (!operand.isReg())
        continue;
      for (size_t later = producerIndex + 1; later < terminatorIndex; ++later) {
        std::vector<int> laterDefs;
        source->insts()[later]->collectDefs(laterDefs);
        if (std::find(laterDefs.begin(), laterDefs.end(), operand.reg) !=
            laterDefs.end()) {
          producerInputsStable = false;
          break;
        }
      }
      if (!producerInputsStable)
        break;
    }
    if (!producerInputsStable) {
      continue;
    }

    plans.push_back({inst, producer, clobberedReg, -1, i, producerIndex});
  }

  if (plans.size() < 2)
    return false;

  bool usedPhysReg[64] = {};
  for (const auto& block : source->parent()->blocks()) {
    for (const auto& instPtr : block->insts()) {
      for (const MOperand& operand : instPtr->operands()) {
        if (operand.isReg() && isPhysReg(operand.reg))
          usedPhysReg[operand.reg] = true;
      }
    }
  }
  std::vector<int> freeScratch;
  for (int reg = 32; reg < 64; ++reg) {
    if (!usedPhysReg[reg])
      freeScratch.push_back(reg);
  }
  size_t scratchCount = 0;
  for (const RecomputePlan& plan : plans)
    scratchCount += plan.producer ? 1 : 0;
  if (freeScratch.size() < scratchCount)
    return false;
  size_t nextScratch = 0;
  for (RecomputePlan& plan : plans) {
    if (plan.producer)
      plan.scratchReg = freeScratch[nextScratch++];
  }

  MachineInst* sinkInsertPoint =
      sink->insts().empty() ? nullptr : sink->insts().front().get();
  if (!sinkInsertPoint)
    return false;
  for (const RecomputePlan& plan : plans) {
    if (plan.producer) {
      auto producerClone =
          std::make_unique<MachineInst>(plan.producer->opcode());
      MOperand producerDef = plan.producer->operand(0);
      producerDef.reg = plan.scratchReg;
      producerDef.isDef = true;
      producerClone->addOperand(producerDef);
      producerClone->addOperand(plan.producer->operand(1));
      producerClone->addOperand(plan.producer->operand(2));
      sink->insertBefore(sinkInsertPoint, std::move(producerClone));
    }

    auto productClone = std::make_unique<MachineInst>(MIOp::FMUL_S);
    for (size_t operandIndex = 0; operandIndex < plan.inst->numOperands();
         ++operandIndex) {
      MOperand operand = plan.inst->operand(operandIndex);
      if (plan.producer && operandIndex > 0 && operand.isReg() &&
          operand.reg == plan.clobberedReg)
        operand.reg = plan.scratchReg;
      productClone->addOperand(std::move(operand));
    }
    sink->insertBefore(sinkInsertPoint, std::move(productClone));
  }

  for (const RecomputePlan& plan : plans) {
    auto it = std::find_if(source->insts().begin(), source->insts().end(),
                           [&plan](const auto& inst) {
                             return inst.get() == plan.inst;
                           });
    if (it != source->insts().end())
      source->insts().erase(it);
  }
  return true;
}

bool sinkRotatedPreheaderOps(const Candidate& candidate) {
  if (!candidate.header || candidate.latch != candidate.body)
    return false;

  MachineBasicBlock* header = candidate.header;
  MachineBasicBlock* body = candidate.body;
  if (header->preds().size() != 1 || body->preds().size() < 2)
    return false;
  MachineBasicBlock* source = header->preds().front();
  if (!source || source->succs().size() != 1 || source->succs().front() != header ||
      source->insts().empty() ||
      !isJumpTo(source->insts().back().get(), header))
    return false;

  std::unordered_map<const MachineInst*, size_t> sourcePositions;
  for (size_t i = 0; i < source->insts().size(); ++i)
    sourcePositions[source->insts()[i].get()] = i;
  const size_t terminatorIndex = source->insts().size() - 1;
  std::unordered_set<const MachineInst*> selected;
  size_t firstSelected = terminatorIndex;

  for (size_t i = terminatorIndex; i-- > 0;) {
    MachineInst* inst = source->insts()[i].get();
    if (!isMovablePreheaderOp(inst))
      break;

    std::vector<int> defs;
    inst->collectDefs(defs);
    bool valid = true;
    bool dependsOnSelected = false;
    for (int reg : defs) {
      for (const auto& block : header->parent()->blocks()) {
        for (const auto& userPtr : block->insts()) {
          MachineInst* user = userPtr.get();
          std::vector<int> uses;
          user->collectUses(uses);
          if (std::find(uses.begin(), uses.end(), reg) == uses.end())
            continue;
          if (user->parent() == source) {
            const size_t userIndex = sourcePositions[user];
            if (userIndex <= i)
              continue;
            if (selected.count(user)) {
              dependsOnSelected = true;
              continue;
            }
            valid = false;
            break;
          }
          if (user->parent() != body) {
            valid = false;
            break;
          }
        }
        if (!valid)
          break;
      }
      if (!valid || dependsOnSelected)
        break;
    }
    if (!valid || dependsOnSelected)
      break;
    selected.insert(inst);
    firstSelected = i;
  }

  if (selected.size() < 2)
    return false;

  auto sink = header->parent()->newBlock(".loop.conditional.preheader");
  auto jump = std::make_unique<MachineInst>(MIOp::J);
  jump->addOperand(MOperand::BB(body));

  auto& sourceInsts = source->insts();
  for (size_t i = firstSelected; i < terminatorIndex; ++i) {
    auto owned = std::move(sourceInsts[firstSelected]);
    sourceInsts.erase(sourceInsts.begin() + firstSelected);
    sink->append(std::move(owned));
  }
  sink->append(std::move(jump));

  for (auto& succ : header->succs()) {
    if (succ == body) {
      succ = sink;
      break;
    }
  }
  sink->preds().push_back(header);
  sink->succs().push_back(body);
  for (auto& pred : body->preds()) {
    if (pred == header)
      pred = sink;
  }

  recomputeClobberedPreheaderProducts(source, body, sink, terminatorIndex);

  if (targetOf(candidate.branch) == body) {
    candidate.branch->operand(candidate.branch->numOperands() - 1).mbb = sink;
  }
  return true;
}

std::unique_ptr<MachineInst> cloneBranchToBody(const Candidate& candidate) {
  auto clone = std::make_unique<MachineInst>(candidate.latchBranchOp);
  for (size_t i = 0; i < candidate.branch->numOperands(); ++i) {
    MOperand operand = candidate.branch->operand(i);
    if (i + 1 == candidate.branch->numOperands())
      operand = MOperand::BB(candidate.body);
    clone->addOperand(std::move(operand));
  }
  clone->comment = "rotated-loop-branch";
  return clone;
}

bool rotate(const Candidate& candidate) {
  auto& latchInsts = candidate.latch->insts();
  if (latchInsts.empty() ||
      !isJumpTo(latchInsts.back().get(), candidate.header)) return false;

  latchInsts.pop_back();
  candidate.latch->append(cloneBranchToBody(candidate));
  auto exitJump = std::make_unique<MachineInst>(MIOp::J);
  exitJump->addOperand(MOperand::BB(candidate.exit));
  exitJump->comment = "rotated-loop-exit";
  candidate.latch->append(std::move(exitJump));

  eraseBlock(candidate.header->preds(), candidate.latch);
  candidate.latch->succs().clear();
  candidate.latch->succs().push_back(candidate.body);
  candidate.latch->succs().push_back(candidate.exit);
  addUnique(candidate.body->preds(), candidate.latch);
  addUnique(candidate.exit->preds(), candidate.latch);
  sinkRotatedPreheaderOps(candidate);
  return true;
}

} // namespace

void runLoopBranchRotate(MachineFunction& mf) {
  int rotated = 0;
  // 先匹配、后修改：一次旋转造成的 CFG 变化不能让后续候选依赖过期的遍历假设。
  std::vector<Candidate> candidates;
  for (auto& block : mf.blocks()) {
    Candidate candidate;
    if (matchCandidate(block.get(), candidate)) candidates.push_back(candidate);
  }
  std::unordered_set<MachineBasicBlock*> changed;
  for (const Candidate& candidate : candidates) {
    if (changed.count(candidate.header) || changed.count(candidate.latch)) continue;
    if (rotate(candidate)) {
      changed.insert(candidate.header);
      changed.insert(candidate.latch);
      ++rotated;
    }
  }
  if (sycEnvironmentValue("MIR_TRACE_LOOP_BRANCH_ROTATE") && rotated != 0)
    std::cerr << "[mir-loop-branch-rotate] " << mf.name() << ": "
              << rotated << " loop(s)\n";
}

void runLoopBranchRotate(MachineModule& mm) {
  for (auto& function : mm.functions()) runLoopBranchRotate(*function);
}

} // namespace mir
