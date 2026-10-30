#include "research_switch.h"
// 文件职责：实现 LLVM 风格的 LoopUnswitch，只提升 dedicated-preheader 小循环里的不变量条件。
#include "pass_registry.h"
#include "loop_unswitch.h"

#include "../../analysis/cfg_utils.h"
#include "../../analysis/dominators.h"
#include "../../analysis/function_analysis.h"
#include "../../analysis/loop_info.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace core_ir {

namespace {

bool loopUnswitchTraceEnabled() {
  return sycEnvironmentValue("SYC_TRACE_COREIR_LOOP_UNSWITCH") != nullptr;
}

void traceLoopUnswitch(const char* stage, Function* func,
                       const analysis::NaturalLoop* loop, const char* reason,
                       BasicBlock* bb = nullptr) {
  if (!loopUnswitchTraceEnabled())
    return;
  std::cerr << "[loop-unswitch] " << stage
            << " func=" << (func ? func->name() : std::string("<null>"));
  if (loop && loop->header)
    std::cerr << " header=" << loop->header->name();
  if (bb)
    std::cerr << " branch=" << bb->name();
  if (reason)
    std::cerr << " reason=" << reason;
  std::cerr << "\n";
}

struct UnswitchCandidate {
  Function* func = nullptr;
  analysis::NaturalLoop loop;
  BasicBlock* preheader = nullptr;
  BasicBlock* branchBB = nullptr;
  Instruction* branchTerm = nullptr;
  BasicBlock* trueSucc = nullptr;
  BasicBlock* falseSucc = nullptr;
  Value* cond = nullptr;
  bool valid = false;
};

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

Instruction* cloneInstructionShell(Instruction* src,
                                   const std::string& suffix = ".unsw") {
  std::string name = src->name();
  if (src->opcode() != Opcode::Call) {
    name = src->name().empty() ? "loop.unswitch" : src->name() + suffix;
  }
  auto* clone =
      new Instruction(src->opcode(), src->type(), name);
  clone->setMemoryInfo(src->memoryElemTy(), src->memorySizeBytes(),
                       src->isArrayObject(), src->dims());
  return clone;
}

Value* remapValue(
    Value* value, const std::unordered_map<Value*, Value*>& valueMap,
    const std::unordered_map<BasicBlock*, BasicBlock*>& blockMap) {
  if (auto* bb = dynamic_cast<BasicBlock*>(value)) {
    auto it = blockMap.find(bb);
    if (it != blockMap.end())
      return it->second;
  }
  auto it = valueMap.find(value);
  return it == valueMap.end() ? value : it->second;
}

Value* incomingFromPred(Instruction* phi, BasicBlock* pred) {
  if (!phi || !pred)
    return nullptr;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    if (phi->operand(i + 1) == pred)
      return phi->operand(i);
  }
  return nullptr;
}

bool loopCallsAreBuiltinOnly(const analysis::NaturalLoop& loop,
                             Module& mod) {
  for (BasicBlock* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (inst && inst->opcode() == Opcode::Call && !mod.isBuiltin(inst->name()))
        return false;
    }
  }
  return true;
}

bool loopHasPointerCallArg(const analysis::NaturalLoop& loop) {
  for (BasicBlock* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst || inst->opcode() != Opcode::Call)
        continue;
      for (Value* operand : inst->operands()) {
        if (operand && operand->type().isPtr())
          return true;
      }
    }
  }
  return false;
}

bool loopStoresToRoot(const analysis::NaturalLoop& loop, Value* root) {
  if (!root)
    return true;
  for (BasicBlock* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      if (inst->opcode() != Opcode::Store && inst->opcode() != Opcode::VStore)
        continue;
      if (inst->numOperands() < 2)
        return true;
      if (memoryRoot(inst->operand(1)) == root)
        return true;
    }
  }
  return false;
}

bool isHoistableExprOpcode(Opcode op) {
  switch (op) {
  case Opcode::Load:
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Mul:
  case Opcode::SDiv:
  case Opcode::SRem:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
  case Opcode::Shl:
  case Opcode::Shr:
  case Opcode::FAdd:
  case Opcode::FSub:
  case Opcode::FMul:
  case Opcode::FDiv:
  case Opcode::ICmp:
  case Opcode::FCmp:
  case Opcode::SIToFP:
  case Opcode::FPToSI:
  case Opcode::GEP:
  case Opcode::Neg:
  case Opcode::FNeg:
  case Opcode::BitCast:
  case Opcode::Mov:
    return true;
  default:
    return false;
  }
}

bool canHoistInvariantLoad(Instruction* load,
                           const analysis::NaturalLoop& loop,
                           Module& mod) {
  if (!load || load->opcode() != Opcode::Load || load->numOperands() != 1)
    return false;
  Value* root = memoryRoot(load->operand(0));
  auto* rootInst = dynamic_cast<Instruction*>(root);
  const bool directScalarGlobal =
      dynamic_cast<GlobalVar*>(root) && root->type().isPtr();
  const bool outerAlloca =
      rootInst && rootInst->opcode() == Opcode::Alloca && rootInst->parent() &&
      !loop.blocks.count(rootInst->parent());
  if (!directScalarGlobal && !outerAlloca)
    return false;
  if (loopStoresToRoot(loop, root))
    return false;
  if (!loopCallsAreBuiltinOnly(loop, mod))
    return false;
  if (loopHasPointerCallArg(loop))
    return false;
  return true;
}

bool canHoistInvariantExpr(Value* value, const analysis::NaturalLoop& loop,
                           BasicBlock* preheader, const analysis::DomSet& doms,
                           Module& mod,
                           std::unordered_map<int, bool>& memo) {
  if (!value)
    return false;
  if (value->isConstant() || dynamic_cast<Argument*>(value) ||
      dynamic_cast<GlobalVar*>(value) || dynamic_cast<BasicBlock*>(value)) {
    return true;
  }
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst)
    return false;
  if (!inst->parent() || !loop.blocks.count(inst->parent()))
    return inst->parent() && analysis::dominates(doms, inst->parent(), preheader);

  auto memoIt = memo.find(inst->id());
  if (memoIt != memo.end())
    return memoIt->second;
  memo[inst->id()] = false;

  if (inst->opcode() == Opcode::Phi)
    return false;
  if (inst->opcode() == Opcode::Load) {
    if (!canHoistInvariantLoad(inst, loop, mod))
      return false;
  } else if (inst->hasSideEffects() || !isHoistableExprOpcode(inst->opcode())) {
    return false;
  }

  for (Value* operand : inst->operands()) {
    if (!canHoistInvariantExpr(operand, loop, preheader, doms, mod, memo))
      return false;
  }
  memo[inst->id()] = true;
  return true;
}

Value* cloneInvariantExprToPreheader(
    Value* value, BasicBlock* preheader, Instruction* preheaderTerm,
    const analysis::NaturalLoop& loop, Module& mod,
    std::unordered_map<Value*, Value*>& cache) {
  if (!value)
    return nullptr;
  if (value->isConstant() || dynamic_cast<Argument*>(value) ||
      dynamic_cast<GlobalVar*>(value) || dynamic_cast<BasicBlock*>(value)) {
    return value;
  }

  auto cacheIt = cache.find(value);
  if (cacheIt != cache.end())
    return cacheIt->second;

  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst)
    return nullptr;
  if (!inst->parent() || !loop.blocks.count(inst->parent())) {
    cache[value] = value;
    return value;
  }
  if (inst->opcode() == Opcode::Load && !canHoistInvariantLoad(inst, loop, mod))
    return nullptr;
  if (inst->opcode() != Opcode::Load && !isHoistableExprOpcode(inst->opcode()))
    return nullptr;

  auto* clone = cloneInstructionShell(inst, ".unsw.pre");
  for (Value* operand : inst->operands()) {
    Value* mapped =
        cloneInvariantExprToPreheader(operand, preheader, preheaderTerm, loop,
                                      mod, cache);
    if (!mapped) {
      clone->dropAllOperands();
      delete clone;
      return nullptr;
    }
    clone->addOperand(mapped);
  }
  preheader->insertBefore(preheaderTerm, clone);
  cache[value] = clone;
  return clone;
}

int loopInstructionCount(const analysis::NaturalLoop& loop) {
  int count = 0;
  for (BasicBlock* bb : loop.blocks) {
    if (!bb)
      continue;
    count += static_cast<int>(bb->instructions().size());
  }
  return count;
}

bool loopHasAlloca(const analysis::NaturalLoop& loop) {
  for (BasicBlock* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (inst && inst->opcode() == Opcode::Alloca)
        return true;
    }
  }
  return false;
}

bool loopHasUnsupportedEscapes(const analysis::NaturalLoop& loop) {
  for (BasicBlock* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      for (Instruction* user : inst->uses()) {
        BasicBlock* userParent = user ? user->parent() : nullptr;
        if (!userParent || loop.blocks.count(userParent))
          continue;
        if (user->opcode() != Opcode::Phi || !incomingFromPred(user, bb))
          return true;
      }
    }
  }
  return false;
}

bool blockDominatesAllLatches(const analysis::DomSet& doms, BasicBlock* bb,
                              const analysis::NaturalLoop& loop) {
  if (!bb)
    return false;
  for (BasicBlock* latch : loop.latches) {
    if (!latch || !analysis::dominates(doms, bb, latch))
      return false;
  }
  return true;
}

bool isProfitableLoopCandidate(const analysis::NaturalLoop& loop, Module& mod) {
  if (!loop.header || loop.blocks.size() < 2 || loop.blocks.size() > 20)
    return false;
  if (loop.latches.size() != 1)
    return false;
  if (loopInstructionCount(loop) > 96)
    return false;
  if (loopHasAlloca(loop) || loopHasUnsupportedEscapes(loop))
    return false;
  return loopCallsAreBuiltinOnly(loop, mod);
}

UnswitchCandidate findCandidate(Function* func, Module& mod,
                                analysis::AnalysisManager& analysisManager) {
  UnswitchCandidate candidate;
  if (!func || !func->entryBlock())
    return candidate;

  analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
  const auto& loops = cache.loops();
  const auto& blocks = cache.blocks();
  const auto& doms = cache.dominators();

  // 逐个检查循环：要求形状有利且有专用 preheader，再在循环体内寻找支配所有
  // latch 的循环不变条件分支，其条件可整体提升到 preheader，命中即作为候选返回。
  for (const auto& loop : loops) {
    if (!isProfitableLoopCandidate(loop, mod)) {
      if (loopUnswitchTraceEnabled()) {
        std::cerr << "[loop-unswitch] reject func=" << func->name()
                  << " header="
                  << (loop.header ? loop.header->name() : std::string("<null>"))
                  << " reason=loop-shape"
                  << " blocks=" << loop.blocks.size()
                  << " latches=" << loop.latches.size()
                  << " inst=" << loopInstructionCount(loop)
                  << " allocas=" << (loopHasAlloca(loop) ? 1 : 0)
                  << " escapes=" << (loopHasUnsupportedEscapes(loop) ? 1 : 0)
                  << " builtinOnlyCalls="
                  << (loopCallsAreBuiltinOnly(loop, mod) ? 1 : 0) << "\n";
      }
      continue;
    }
    BasicBlock* preheader = analysis::dedicatedPreheader(loop);
    if (!preheader) {
      traceLoopUnswitch("reject", func, &loop, "preheader");
      continue;
    }
    bool sawCondBr = false;
    for (BasicBlock* bb : blocks) {
      if (!bb || !loop.blocks.count(bb))
        continue;
      Instruction* term = bb->terminator();
      if (!term || term->opcode() != Opcode::CondBr || term->numOperands() != 3)
        continue;
      sawCondBr = true;
      auto* trueSucc = dynamic_cast<BasicBlock*>(term->operand(1));
      auto* falseSucc = dynamic_cast<BasicBlock*>(term->operand(2));
      if (!trueSucc || !falseSucc || trueSucc == falseSucc)
        continue;
      if (!loop.blocks.count(trueSucc) || !loop.blocks.count(falseSucc))
        continue;
      // 分支块必须支配所有 latch，保证每次迭代都会经过该判断，才能安全外提。
      if (!blockDominatesAllLatches(doms, bb, loop)) {
        traceLoopUnswitch("reject", func, &loop, "not-every-iter", bb);
        continue;
      }
      std::unordered_map<int, bool> memo;
      if (!canHoistInvariantExpr(term->operand(0), loop, preheader, doms, mod,
                                 memo)) {
        traceLoopUnswitch("reject", func, &loop, "cond-not-hoistable", bb);
        continue;
      }
      candidate.func = func;
      candidate.loop = loop;
      candidate.preheader = preheader;
      candidate.branchBB = bb;
      candidate.branchTerm = term;
      candidate.trueSucc = trueSucc;
      candidate.falseSucc = falseSucc;
      candidate.cond = term->operand(0);
      candidate.valid = true;
      traceLoopUnswitch("accept", func, &loop, "candidate", bb);
      return candidate;
    }
    if (!sawCondBr)
      traceLoopUnswitch("reject", func, &loop, "no-condbr");
  }
  return candidate;
}

bool cloneLoopBlocks(
    Function* func, const analysis::NaturalLoop& loop,
    std::unordered_map<BasicBlock*, BasicBlock*>& blockMap,
    std::unordered_map<Value*, Value*>& valueMap) {
  if (!func || !loop.header)
    return false;

  std::vector<BasicBlock*> orderedBlocks;
  orderedBlocks.reserve(loop.blocks.size());
  for (auto& bbPtr : func->blocks()) {
    if (loop.blocks.count(bbPtr.get()))
      orderedBlocks.push_back(bbPtr.get());
  }
  if (orderedBlocks.size() != loop.blocks.size())
    return false;

  for (BasicBlock* srcBB : orderedBlocks) {
    BasicBlock* dstBB = func->newBlock("loop_unswitch");
    blockMap[srcBB] = dstBB;
  }

  for (BasicBlock* srcBB : orderedBlocks) {
    BasicBlock* dstBB = blockMap.at(srcBB);
    for (auto& instPtr : srcBB->instructions()) {
      Instruction* srcInst = instPtr.get();
      Instruction* dstInst = cloneInstructionShell(srcInst);
      dstBB->pushBack(dstInst);
      valueMap[srcInst] = dstInst;
    }
  }

  for (BasicBlock* srcBB : orderedBlocks) {
    BasicBlock* dstBB = blockMap.at(srcBB);
    size_t dstIndex = 0;
    for (auto& instPtr : srcBB->instructions()) {
      Instruction* srcInst = instPtr.get();
      Instruction* dstInst = dstBB->instructions()[dstIndex++].get();
      for (Value* operand : srcInst->operands())
        dstInst->addOperand(remapValue(operand, valueMap, blockMap));
    }
  }

  return true;
}

bool rewriteCondBranchToBr(BasicBlock* bb, BasicBlock* keptSucc,
                           BasicBlock* droppedSucc) {
  if (!bb || !keptSucc || !droppedSucc)
    return false;
  Instruction* term = bb->terminator();
  if (!term || term->opcode() != Opcode::CondBr)
    return false;
  analysis::removePhiIncomingPred(droppedSucc, bb);
  bb->remove(term);
  auto* br = new Instruction(Opcode::Br, Type::voidTy());
  br->addOperand(keptSucc);
  bb->pushBack(br);
  return true;
}

bool rewritePreheaderToCondBr(BasicBlock* preheader, Value* cond,
                              BasicBlock* trueHeader,
                              BasicBlock* falseHeader) {
  if (!preheader || !cond || !trueHeader || !falseHeader)
    return false;
  Instruction* term = preheader->terminator();
  if (!term || term->opcode() != Opcode::Br || term->numOperands() != 1)
    return false;
  preheader->remove(term);
  auto* br = new Instruction(Opcode::CondBr, Type::voidTy());
  br->addOperand(cond);
  br->addOperand(trueHeader);
  br->addOperand(falseHeader);
  preheader->pushBack(br);
  return true;
}

BasicBlock* createDedicatedUnswitchPreheader(Function* func,
                                             BasicBlock* target,
                                             const std::string& suffix) {
  if (!func || !target)
    return nullptr;
  BasicBlock* preheader = func->newBlock(suffix);
  auto* br = new Instruction(Opcode::Br, Type::voidTy());
  br->addOperand(target);
  preheader->pushBack(br);
  return preheader;
}

bool addClonedExitPhiIncoming(
    const analysis::NaturalLoop& loop,
    const std::unordered_map<BasicBlock*, BasicBlock*>& blockMap,
    const std::unordered_map<Value*, Value*>& valueMap) {
  for (BasicBlock* srcBB : loop.blocks) {
    if (!srcBB)
      continue;
    auto clonedSrcIt = blockMap.find(srcBB);
    if (clonedSrcIt == blockMap.end())
      return false;
    BasicBlock* clonedSrc = clonedSrcIt->second;
    for (BasicBlock* succ : srcBB->succs()) {
      if (!succ || loop.blocks.count(succ))
        continue;
      for (Instruction* phi : analysis::collectLeadingPhi(succ)) {
        Value* incoming = incomingFromPred(phi, srcBB);
        if (!incoming)
          return false;
        phi->addOperand(remapValue(incoming, valueMap, blockMap));
        phi->addOperand(clonedSrc);
      }
    }
  }
  return true;
}

bool removeUnreachableBlocks(Function* func) {
  if (!func || !func->entryBlock())
    return false;
  std::unordered_set<BasicBlock*> reachable =
      analysis::collectReachableBlocks(func);
  std::vector<BasicBlock*> deadBlocks;
  for (auto& bbPtr : func->blocks()) {
    if (!reachable.count(bbPtr.get()))
      deadBlocks.push_back(bbPtr.get());
  }
  if (deadBlocks.empty())
    return false;

  std::unordered_set<BasicBlock*> deadSet(deadBlocks.begin(), deadBlocks.end());
  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!bb || !reachable.count(bb) || deadSet.count(bb))
      continue;
    for (BasicBlock* dead : deadBlocks)
      analysis::removePhiIncomingPred(bb, dead);
  }

  std::unordered_set<BasicBlock*> removable(deadSet.begin(), deadSet.end());
  bool changed = true;
  while (changed) {
    changed = false;
    for (BasicBlock* dead : deadBlocks) {
      if (!removable.count(dead))
        continue;
      bool escapes = false;
      for (Instruction* user : dead->uses()) {
        BasicBlock* userParent = user ? user->parent() : nullptr;
        if (!userParent || !removable.count(userParent)) {
          escapes = true;
          break;
        }
      }
      for (auto& instPtr : dead->instructions()) {
        if (escapes)
          break;
        Instruction* inst = instPtr.get();
        if (!inst)
          continue;
        for (Instruction* user : inst->uses()) {
          BasicBlock* userParent = user ? user->parent() : nullptr;
          if (!userParent || !removable.count(userParent)) {
            escapes = true;
            break;
          }
        }
      }
      if (escapes) {
        removable.erase(dead);
        changed = true;
      }
    }
  }

  bool removedAny = false;
  for (BasicBlock* dead : deadBlocks) {
    if (!removable.count(dead))
      continue;
    for (auto& instPtr : dead->instructions()) {
      if (instPtr)
        instPtr->dropAllOperands();
    }
  }
  for (BasicBlock* dead : deadBlocks) {
    if (!removable.count(dead))
      continue;
    func->removeBlock(dead);
    removedAny = true;
  }
  if (removedAny)
    func->recomputeCFG();
  return removedAny;
}

bool applyCandidate(Module& mod, const UnswitchCandidate& candidate) {
  if (!candidate.valid || !candidate.func || !candidate.preheader ||
      !candidate.branchBB || !candidate.branchTerm || !candidate.trueSucc ||
      !candidate.falseSucc) {
    return false;
  }

  Instruction* preheaderTerm = candidate.preheader->terminator();
  if (!preheaderTerm)
    return false;

  std::unordered_map<Value*, Value*> preheaderExprCache;
  Value* hoistedCond = cloneInvariantExprToPreheader(
      candidate.cond, candidate.preheader, preheaderTerm, candidate.loop, mod,
      preheaderExprCache);
  if (!hoistedCond)
    return false;

  std::unordered_map<BasicBlock*, BasicBlock*> blockMap;
  std::unordered_map<Value*, Value*> valueMap;
  if (!cloneLoopBlocks(candidate.func, candidate.loop, blockMap, valueMap))
    return false;
  if (!addClonedExitPhiIncoming(candidate.loop, blockMap, valueMap))
    return false;

  BasicBlock* clonedHeader = blockMap.at(candidate.loop.header);
  BasicBlock* clonedBranch = blockMap.at(candidate.branchBB);
  BasicBlock* clonedTrueSucc = blockMap.at(candidate.trueSucc);
  BasicBlock* clonedFalseSucc = blockMap.at(candidate.falseSucc);
  BasicBlock* truePreheader = createDedicatedUnswitchPreheader(
      candidate.func, candidate.loop.header, "loop_unswitch.pretrue");
  BasicBlock* falsePreheader = createDedicatedUnswitchPreheader(
      candidate.func, clonedHeader, "loop_unswitch.prefalse");
  if (!truePreheader || !falsePreheader)
    return false;

  if (!rewriteCondBranchToBr(candidate.branchBB, candidate.trueSucc,
                             candidate.falseSucc))
    return false;
  if (!rewriteCondBranchToBr(clonedBranch, clonedFalseSucc, clonedTrueSucc))
    return false;
  analysis::replacePhiIncomingPred(candidate.loop.header, candidate.preheader,
                                   truePreheader);
  analysis::replacePhiIncomingPred(clonedHeader, candidate.preheader,
                                   falsePreheader);
  if (!rewritePreheaderToCondBr(candidate.preheader, hoistedCond, truePreheader,
                                falsePreheader))
    return false;

  candidate.func->recomputeCFG();
  removeUnreachableBlocks(candidate.func);
  candidate.func->recomputeCFG();
  traceLoopUnswitch("apply", candidate.func, &candidate.loop, "done",
                    candidate.branchBB);
  return true;
}

bool runOnFunction(Module& mod, Function* func,
                   analysis::AnalysisManager& analysisManager) {
  if (!func || !func->entryBlock())
    return false;

  bool changed = false;
  int budget = 6;
  while (budget-- > 0) {
    func->recomputeCFG();
    analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
    UnswitchCandidate candidate = findCandidate(func, mod, analysisManager);
    if (!candidate.valid)
      break;
    if (!applyCandidate(mod, candidate))
      break;
    changed = true;
    analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
  }
  return changed;
}

} // namespace

bool applyLoopUnswitch(Module& mod,
                       analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    if (runOnFunction(mod, funcPtr.get(), analysisManager))
      changed = true;
  }
  if (changed)
    analysisManager.invalidateModule(analysis::AnalysisInvalidation::Module);
  return changed;
}

class LoopUnswitchPass : public Pass {
public:
    std::string name() const override { return "LoopUnswitch"; }
    bool run(Module& mod, analysis::AnalysisManager& am) override {
        return applyLoopUnswitch(mod, am);
    }
};
REGISTER_PASS(LoopUnswitchPass);

} // namespace core_ir
