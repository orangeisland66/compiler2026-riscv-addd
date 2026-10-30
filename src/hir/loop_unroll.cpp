#include "research_switch.h"
// 文件职责：按收益模型展开小循环或 tile-local 内层循环，控制代码体积。本文件负责实现具体逻辑。
#include "loop_unroll.h"
#include "canonical_loop.h"

#include "../analysis/analysis_manager.h"
#include "../analysis/cfg_utils.h"
#include "../analysis/dominators.h"
#include "../analysis/loop_info.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hir {

namespace {

using analysis::NaturalLoop;
using core_ir::BasicBlock;
using core_ir::ConstInt;
using core_ir::Function;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Type;
using core_ir::Value;

// 完全展开的最大迭代次数，较大的循环会明显增加代码体积。
constexpr int kMaxUnrollTrips = 32;
constexpr int kMaxCallUnrollTrips = 8;
constexpr int kMaxFullUnrollClones = 192;
constexpr int kMaxPartialUnrollBodyInsts = 128;
constexpr int kMaxCallAwareFullUnrollClones = 256;
constexpr int kMaxGuardedKernelFullUnrollClones = 1024;

void traceUnrollReject(Function* func, const NaturalLoop& loop,
                       const char* phase, const char* reason) {
  if (!sycEnvironmentValue("SYC_TRACE_LOOP_UNROLL"))
    return;
  std::fprintf(stderr, "[loop-unroll] %s header=%s phase=%s reject=%s\n",
               func ? func->name().c_str() : "<null>",
               loop.header ? loop.header->name().c_str() : "<null>",
               phase ? phase : "unknown", reason ? reason : "unknown");
}

void traceUnrollAccept(Function* func, const NaturalLoop& loop,
                       const char* phase, int factor, int cost) {
  if (!sycEnvironmentValue("SYC_TRACE_LOOP_UNROLL"))
    return;
  std::fprintf(stderr,
               "[loop-unroll] %s header=%s phase=%s accept factor=%d cost=%d\n",
               func ? func->name().c_str() : "<null>",
               loop.header ? loop.header->name().c_str() : "<null>",
               phase ? phase : "unknown", factor, cost);
}

void setRejectReason(const char** rejectReason, const char* reason) {
  if (rejectReason)
    *rejectReason = reason;
}

Instruction* cloneInstructionShell(Instruction* inst) {
  auto* clone = new Instruction(inst->opcode(), inst->type(), inst->name());
  clone->setMemoryInfo(inst->memoryElemTy(), inst->memorySizeBytes(),
                       inst->isArrayObject(), inst->dims());
  return clone;
}

std::vector<Instruction*> collectIterationInsts(const CanonicalLoop& cl) {
  std::vector<Instruction*> insts;
  for (auto& instPtr : cl.body->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst->isTerminator())
      insts.push_back(inst);
  }
  if (cl.body != cl.latch) {
    for (auto& instPtr : cl.latch->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst->isTerminator())
        insts.push_back(inst);
    }
  }
  return insts;
}

int countLoopCalls(const NaturalLoop& loop, const CanonicalLoop& cl) {
  int calls = 0;
  for (auto* bb : loop.blocks) {
    if (bb == cl.header)
      continue;
    for (auto& instPtr : bb->instructions()) {
      if (instPtr->opcode() == Opcode::Call)
        ++calls;
    }
  }
  return calls;
}

int countFloatingPointOps(const std::vector<Instruction*>& iterationInsts,
                          bool& hasFDiv) {
  int fpOps = 0;
  hasFDiv = false;
  for (auto* inst : iterationInsts) {
    if (!inst)
      continue;
    switch (inst->opcode()) {
    case Opcode::FAdd:
    case Opcode::FSub:
    case Opcode::FMul:
    case Opcode::FDiv:
    case Opcode::FNeg:
      ++fpOps;
      if (inst->opcode() == Opcode::FDiv)
        hasFDiv = true;
      break;
    default:
      break;
    }
  }
  return fpOps;
}

int countMemoryOps(const std::vector<Instruction*>& iterationInsts) {
  int memOps = 0;
  for (auto* inst : iterationInsts) {
    if (!inst)
      continue;
    if (inst->opcode() == Opcode::Load || inst->opcode() == Opcode::Store)
      ++memOps;
  }
  return memOps;
}

int countIntegerDivRemOps(const std::vector<Instruction*>& iterationInsts) {
  int divRemOps = 0;
  for (auto* inst : iterationInsts) {
    if (!inst)
      continue;
    if (inst->opcode() == Opcode::SDiv || inst->opcode() == Opcode::SRem ||
        inst->opcode() == Opcode::SDivInvariantDirect)
      ++divRemOps;
  }
  return divRemOps;
}

bool integerDivRemDivisorsAreConstant(
    const std::vector<Instruction*>& iterationInsts) {
  for (auto* inst : iterationInsts) {
    if (!inst)
      continue;
    if (inst->opcode() == Opcode::SDivInvariantDirect)
      return false;
    if (inst->opcode() != Opcode::SDiv && inst->opcode() != Opcode::SRem)
      continue;
    if (inst->numOperands() < 2 ||
        !dynamic_cast<ConstInt*>(inst->operand(1))) {
      return false;
    }
  }
  return true;
}

int countAddressOps(const std::vector<Instruction*>& iterationInsts) {
  int addressOps = 0;
  for (auto* inst : iterationInsts) {
    if (!inst)
      continue;
    if (inst->opcode() == Opcode::GEP)
      ++addressOps;
  }
  return addressOps;
}

int countInnerCondBranches(const NaturalLoop& loop, const CanonicalLoop& cl) {
  int branches = 0;
  for (auto* bb : loop.blocks) {
    if (!bb || bb == cl.header || bb == cl.latch)
      continue;
    Instruction* term = bb->terminator();
    if (term && term->opcode() == Opcode::CondBr)
      ++branches;
  }
  return branches;
}

bool isScalarArithmeticOpcode(Opcode op) {
  switch (op) {
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Mul:
  case Opcode::SDiv:
  case Opcode::SRem:
  case Opcode::SDivInvariantDirect:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
  case Opcode::Shl:
  case Opcode::Shr:
  case Opcode::Neg:
  case Opcode::FAdd:
  case Opcode::FSub:
  case Opcode::FMul:
  case Opcode::FDiv:
  case Opcode::FNeg:
    return true;
  default:
    return false;
  }
}

bool containsPreparedInvariantDivision(const NaturalLoop& loop) {
  for (BasicBlock* block : loop.blocks) {
    if (!block)
      continue;
    for (const auto& instruction : block->instructions()) {
      if (instruction &&
          instruction->opcode() == Opcode::SDivInvariantDirect)
        return true;
    }
  }
  return false;
}

bool valueUsesThroughArithmetic(Value* value, Value* needle,
                                bool seenArithmetic,
                                std::unordered_set<int>& visiting) {
  if (!value || !needle)
    return false;
  if (value == needle)
    return seenArithmetic;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !visiting.insert(inst->id()).second)
    return false;
  bool nextSeenArithmetic =
      seenArithmetic || isScalarArithmeticOpcode(inst->opcode());
  for (auto* operand : inst->operands()) {
    if (valueUsesThroughArithmetic(operand, needle, nextSeenArithmetic,
                                   visiting)) {
      visiting.erase(inst->id());
      return true;
    }
  }
  visiting.erase(inst->id());
  return false;
}

bool hasSameAddressArithmeticUpdate(
    const std::vector<Instruction*>& iterationInsts) {
  std::unordered_map<Value*, std::vector<Instruction*>> loadsByAddress;
  for (auto* inst : iterationInsts) {
    if (!inst)
      continue;
    if (inst->opcode() == Opcode::Load && inst->numOperands() == 1) {
      loadsByAddress[inst->operand(0)].push_back(inst);
      continue;
    }
    if (inst->opcode() == Opcode::Store && inst->numOperands() == 2 &&
        loadsByAddress.count(inst->operand(1))) {
      for (auto* load : loadsByAddress[inst->operand(1)]) {
        std::unordered_set<int> visiting;
        if (valueUsesThroughArithmetic(inst->operand(0), load,
                                       /*seenArithmetic=*/false, visiting)) {
          return true;
        }
      }
    }
  }
  return false;
}

bool computeConstForwardTripCount(const CanonicalLoop& cl,
                                  int& trips,
                                  const char** rejectReason = nullptr) {
  auto* initC = dynamic_cast<ConstInt*>(cl.initVal);
  auto* boundC = dynamic_cast<ConstInt*>(cl.boundVal);
  if (!initC || !boundC) {
    setRejectReason(rejectReason, "non-const-trip");
    return false;
  }
  if (!cl.ivIsCmpLhs) {
    setRejectReason(rejectReason, "iv-not-lhs");
    return false;
  }

  const std::string cond = canonicalLoopCmpCond(cl.cmp);
  const int64_t initV = initC->value();
  const int64_t boundV = boundC->value();
  int64_t trip64 = 0;
  if (cond == "lt") {
    if (boundV <= initV) {
      setRejectReason(rejectReason, "empty-trip");
      return false;
    }
    trip64 = boundV - initV;
  } else if (cond == "le") {
    if (boundV < initV) {
      setRejectReason(rejectReason, "empty-trip");
      return false;
    }
    trip64 = boundV - initV + 1;
  } else {
    setRejectReason(rejectReason, "unsupported-cond");
    return false;
  }

  if (trip64 < 1 || trip64 > kMaxUnrollTrips) {
    setRejectReason(rejectReason, "trip-budget");
    return false;
  }
  trips = static_cast<int>(trip64);
  return true;
}

// 检查循环体和 latch 是否适合展开。
bool bodySimpleEnough(const NaturalLoop& loop,
                      const CanonicalLoop& cl,
                      int maxCalls = 0,
                      const char** rejectReason = nullptr) {
  int callsSeen = 0;
  for (auto* bb : loop.blocks) {
    if (bb == cl.header) continue; // 循环头部单独处理
    // 只处理循环体和 latch。
    if (bb != cl.body && bb != cl.latch) {
      setRejectReason(rejectReason, "extra-block");
      return false;
    }
    for (auto& instPtr : bb->instructions()) {
      Opcode op = instPtr->opcode();
      // 循环体中遇到副作用指令或难以复制的操作码时停止展开。
      if (op == Opcode::Alloca) {
        setRejectReason(rejectReason, "alloca");
        return false;
      }
      if (op == Opcode::Call && ++callsSeen > maxCalls) {
        setRejectReason(rejectReason, "call-budget");
        return false;
      }
    }
  }
  // 循环体的终结指令必须指向 latch（body 与 latch 不同时）或循环头部。
  if (cl.body != cl.latch) {
    Instruction* bt = cl.body->terminator();
    if (!bt || bt->opcode() != Opcode::Br) {
      setRejectReason(rejectReason, "body-term");
      return false;
    }
    if (dynamic_cast<BasicBlock*>(bt->operand(0)) != cl.latch) {
      setRejectReason(rejectReason, "body-not-to-latch");
      return false;
    }
  }
  // 出口块不能包含引用循环内定义值的 PHI，否则还需要额外改写这些输入。
  for (auto& instPtr : cl.exit->instructions()) {
    if (instPtr->opcode() != Opcode::Phi) break;
    for (int i = 0; i + 1 < instPtr->numOperands(); i += 2) {
      auto* incBB = dynamic_cast<BasicBlock*>(instPtr->operand(i + 1));
      if (incBB == cl.header) {
        setRejectReason(rejectReason, "exit-phi-header");
        return false; // 循环头即将删除，该入边值也随之失效
      }
    }
  }
  return true;
}

Value* selectLinearizedPhiIncoming(
    Instruction* phi, const CanonicalLoop& cl,
    const std::unordered_map<Value*, Value*>& valueMap) {
  if (!phi || phi->opcode() != Opcode::Phi || !phi->parent())
    return nullptr;

  BasicBlock* expectedPred = nullptr;
  if (phi->parent() == cl.body) {
    expectedPred = cl.header;
  } else if (phi->parent() == cl.latch) {
    expectedPred = (cl.body == cl.latch) ? cl.header : cl.body;
  } else {
    return nullptr;
  }

  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
    if (pred != expectedPred)
      continue;
    Value* incoming = phi->operand(i);
    auto it = valueMap.find(incoming);
    return it != valueMap.end() ? it->second : incoming;
  }
  return nullptr;
}

std::vector<BasicBlock*> collectLoopBlocksInFunctionOrder(Function* func,
                                                          const NaturalLoop& loop,
                                                          const CanonicalLoop& cl) {
  std::vector<BasicBlock*> ordered;
  if (!func)
    return ordered;
  ordered.reserve(loop.blocks.size());
  for (const auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!bb || bb == cl.header || !loop.blocks.count(bb))
      continue;
    ordered.push_back(bb);
  }
  return ordered;
}

bool cfgFullUnrollSimpleEnough(Function* func,
                               const NaturalLoop& loop,
                               const CanonicalLoop& cl,
                               int maxCalls = 0,
                               const char** rejectReason = nullptr) {
  if (!func || loop.blocks.size() < 2) {
    setRejectReason(rejectReason, "bad-loop");
    return false;
  }

  for (auto& instPtr : cl.exit->instructions()) {
    if (instPtr->opcode() != Opcode::Phi)
      break;
    for (int i = 0; i + 1 < instPtr->numOperands(); i += 2) {
      auto* pred = dynamic_cast<BasicBlock*>(instPtr->operand(i + 1));
      if (pred && loop.blocks.count(pred)) {
        setRejectReason(rejectReason, "exit-phi-loop-pred");
        return false;
      }
    }
  }

  int callsSeen = 0;
  for (auto* bb : loop.blocks) {
    if (!bb || !bb->terminator()) {
      setRejectReason(rejectReason, "missing-terminator");
      return false;
    }

    if (bb != cl.header) {
      for (auto& instPtr : bb->instructions()) {
        if (instPtr->opcode() != Opcode::Phi)
          break;
        // CFG full-unroll 会按迭代克隆整个循环体 CFG。
        // 因此内部 diamond/guard merge 上的 PHI 可以被一起克隆；唯一
        // 需要拒绝的是 incoming 来自未克隆的 header 或循环外部。
        for (int i = 0; i + 1 < instPtr->numOperands(); i += 2) {
          auto* pred = dynamic_cast<BasicBlock*>(instPtr->operand(i + 1));
          if (!pred || pred == cl.header || !loop.blocks.count(pred)) {
            setRejectReason(rejectReason, "inner-phi-incoming");
            return false;
          }
        }
      }
    }

    for (auto& instPtr : bb->instructions()) {
      Opcode op = instPtr->opcode();
      if (op == Opcode::Alloca) {
        setRejectReason(rejectReason, "alloca");
        return false;
      }
      if (op == Opcode::Call && ++callsSeen > maxCalls) {
        setRejectReason(rejectReason, "call-budget");
        return false;
      }
    }

    Instruction* term = bb->terminator();
    if (bb == cl.latch) {
      if (term->opcode() != Opcode::Br ||
          dynamic_cast<BasicBlock*>(term->operand(0)) != cl.header) {
        setRejectReason(rejectReason, "latch-not-backedge");
        return false;
      }
      continue;
    }

    if (bb != cl.header &&
        term->opcode() != Opcode::Br &&
        term->opcode() != Opcode::CondBr) {
      setRejectReason(rejectReason, "bad-inner-term");
      return false;
    }

    for (auto* succ : bb->succs()) {
      if (!succ) {
        setRejectReason(rejectReason, "null-succ");
        return false;
      }
      if (succ == cl.header && bb != cl.latch) {
        setRejectReason(rejectReason, "non-latch-backedge");
        return false;
      }
      if (!loop.blocks.count(succ) && succ != cl.exit) {
        setRejectReason(rejectReason, "external-succ");
        return false;
      }
      if (bb != cl.header && !loop.blocks.count(succ)) {
        setRejectReason(rejectReason, "inner-exit");
        return false;
      }
    }
  }

  if (collectLoopBlocksInFunctionOrder(func, loop, cl).empty()) {
    setRejectReason(rejectReason, "empty-order");
    return false;
  }
  return true;
}

bool tryCFGFullUnroll(Function* func, NaturalLoop& loop,
                      int requiredTrips = 0) {
  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader) {
    traceUnrollReject(func, loop, "cfg-full", "no-preheader");
    return false;
  }
  CanonicalLoop cl;
  if (!recognizeCanonicalLoop(loop, preheader, cl)) {
    traceUnrollReject(func, loop, "cfg-full", "not-canonical");
    return false;
  }
  if (bodySimpleEnough(loop, cl))
    return false;

  int trips = 0;
  const char* tripReject = nullptr;
  if (!computeConstForwardTripCount(cl, trips, &tripReject)) {
    traceUnrollReject(func, loop, "cfg-full", tripReject);
    return false;
  }
  if (requiredTrips > 0 && trips != requiredTrips)
    return false;

  int callCount = countLoopCalls(loop, cl);
  if (callCount > 0 && trips > kMaxCallUnrollTrips) {
    traceUnrollReject(func, loop, "cfg-full", "call-trip-budget");
    return false;
  }
  bool callAware = callCount == 1;
  const char* cfgReject = nullptr;
  if (!cfgFullUnrollSimpleEnough(func, loop, cl, callAware ? 1 : 0,
                                 &cfgReject)) {
    traceUnrollReject(func, loop, "cfg-full", cfgReject);
    return false;
  }

  std::vector<Instruction*> phis;
  std::vector<Value*> phiInitVals;
  std::vector<Value*> phiNextVals;
  if (!collectCanonicalLoopPhis(cl, phis, phiInitVals, &phiNextVals)) {
    traceUnrollReject(func, loop, "cfg-full", "phi-collect");
    return false;
  }

  std::vector<BasicBlock*> orderedBlocks = collectLoopBlocksInFunctionOrder(func, loop, cl);
  size_t cloneBudget = orderedBlocks.size();
  for (BasicBlock* bb : orderedBlocks) {
    if (!bb) {
      traceUnrollReject(func, loop, "cfg-full", "bad-block-order");
      return false;
    }
    cloneBudget += bb->instructions().size();
  }
  size_t maxCloneBudget = static_cast<size_t>(
      (callAware ? kMaxCallAwareFullUnrollClones : kMaxFullUnrollClones) * 2);
  int innerCondBranches = countInnerCondBranches(loop, cl);
  if (!callAware && trips <= 8 && orderedBlocks.size() <= 16) {
    // 带边界 guard 的小常量 kernel 循环通常会在内层展开后
    // 暴露较多 diamond/PHI，普通 body-size budget 容易把它拒掉，导致
    // 常量表索引、reduction 和边界条件都留在热循环里。这里只对无调用、
    // 小 trip、CFG 规模受控的通用 guarded kernel 放宽 clone 预算；收益
    // 来自 LLVM FullUnroll/LoopUnswitch 后续 SCCP/CSE 能继续折叠常量路径。
    maxCloneBudget = std::max(
        maxCloneBudget,
        static_cast<size_t>(kMaxGuardedKernelFullUnrollClones));
  } else if (!callAware && innerCondBranches > 0 &&
             cloneBudget * static_cast<size_t>(trips) > 192) {
    // 内部带 guard/diamond 的小常量循环 full-unroll 会复制整片
    // 控制流。若不是上面的受控小 trip kernel，仍保留旧的保守门槛，避免
    // 在 BOOM 上制造过大的热块和寄存器压力。
    traceUnrollReject(func, loop, "cfg-full", "guarded-clone-cost");
    return false;
  }
  if (cloneBudget * static_cast<size_t>(trips) > maxCloneBudget) {
    traceUnrollReject(func, loop, "cfg-full", "clone-budget");
    return false;
  }

  std::vector<std::unordered_map<BasicBlock*, BasicBlock*>> clonedBlocks(trips);
  for (int iter = 0; iter < trips; ++iter) {
    for (BasicBlock* oldBB : orderedBlocks) {
      clonedBlocks[iter][oldBB] = func->newBlock("unroll_cfg");
    }
  }

  std::unordered_map<Instruction*, Value*> curPhiValues;
  for (size_t i = 0; i < phis.size(); ++i)
    curPhiValues[phis[i]] = phiInitVals[i];
  std::vector<Value*> finalPhiValues = phiInitVals;

  for (int iter = 0; iter < trips; ++iter) {
    std::unordered_map<Value*, Value*> valueMap;
    for (size_t i = 0; i < phis.size(); ++i)
      valueMap[phis[i]] = curPhiValues[phis[i]];

    // 内部 if-merge 或嵌套小循环 header 的 PHI 可能引用同一
    // 迭代中稍后才克隆出来的值。先建立所有非 terminator 的 clone shell
    // 和 valueMap，再统一填 operand，才能正确处理这种 SSA 环。
    for (BasicBlock* oldBB : orderedBlocks) {
      BasicBlock* newBB = clonedBlocks[iter][oldBB];
      for (auto& instPtr : oldBB->instructions()) {
        Instruction* inst = instPtr.get();
        if (inst->isTerminator())
          continue;
        auto* clone = cloneInstructionShell(inst);
        newBB->pushBack(clone);
        valueMap[inst] = clone;
      }
    }

    for (BasicBlock* oldBB : orderedBlocks) {
      for (auto& instPtr : oldBB->instructions()) {
        Instruction* inst = instPtr.get();
        if (inst->isTerminator())
          continue;
        auto* clone = dynamic_cast<Instruction*>(valueMap[inst]);
        if (!clone) {
          traceUnrollReject(func, loop, "cfg-full", "clone-missing-shell");
          return false;
        }
        for (int k = 0; k < inst->numOperands(); ++k) {
          Value* op = inst->operand(k);
          if (inst->opcode() == Opcode::Phi && (k % 2) == 1) {
            auto* pred = dynamic_cast<BasicBlock*>(op);
            auto blockIt = pred ? clonedBlocks[iter].find(pred)
                                : clonedBlocks[iter].end();
            if (blockIt == clonedBlocks[iter].end()) {
              traceUnrollReject(func, loop, "cfg-full",
                                "clone-inner-phi-pred");
              return false;
            }
            clone->addOperand(blockIt->second);
            continue;
          }
          auto it = valueMap.find(op);
          clone->addOperand(it != valueMap.end() ? it->second : op);
        }
      }
    }

    std::vector<Value*> nextPhiValues;
    nextPhiValues.reserve(phis.size());
    for (Value* nxt : phiNextVals) {
      auto it = valueMap.find(nxt);
      nextPhiValues.push_back(it != valueMap.end() ? it->second : nxt);
    }

    // 克隆各块终结指令并重接分支目标：latch 回边改指向下一迭代的入口
    // （末次迭代指向 exit），循环内目标映射到本迭代克隆块，循环外仅
    // 允许指向 exit，其余形态一律拒绝。
    for (BasicBlock* oldBB : orderedBlocks) {
      Instruction* term = oldBB->terminator();
      if (!term) {
        traceUnrollReject(func, loop, "cfg-full", "missing-clone-term");
        return false;
      }
      auto* clone = cloneInstructionShell(term);
      for (int k = 0; k < term->numOperands(); ++k) {
        Value* op = term->operand(k);
        if (auto* targetBB = dynamic_cast<BasicBlock*>(op)) {
          BasicBlock* mappedTarget = nullptr;
          if (targetBB == cl.header) {
            if (oldBB != cl.latch) {
              traceUnrollReject(func, loop, "cfg-full",
                                "clone-non-latch-backedge");
              return false;
            }
            mappedTarget = (iter + 1 < trips)
                               ? clonedBlocks[iter + 1][cl.body]
                               : cl.exit;
          } else if (loop.blocks.count(targetBB)) {
            auto it = clonedBlocks[iter].find(targetBB);
            if (it == clonedBlocks[iter].end()) {
              traceUnrollReject(func, loop, "cfg-full", "clone-bad-target");
              return false;
            }
            mappedTarget = it->second;
          } else {
            if (targetBB != cl.exit) {
              traceUnrollReject(func, loop, "cfg-full",
                                "clone-external-target");
              return false;
            }
            mappedTarget = cl.exit;
          }
          clone->addOperand(mappedTarget);
          continue;
        }
        auto it = valueMap.find(op);
        clone->addOperand(it != valueMap.end() ? it->second : op);
      }
      clonedBlocks[iter][oldBB]->pushBack(clone);
    }

    for (size_t i = 0; i < phis.size(); ++i) {
      curPhiValues[phis[i]] = nextPhiValues[i];
      finalPhiValues[i] = nextPhiValues[i];
    }
  }

  for (size_t i = 0; i < phis.size(); ++i) {
    Instruction* phi = phis[i];
    Value* finalVal = finalPhiValues[i];
    std::vector<Instruction*> users = phi->uses();
    for (Instruction* user : users) {
      BasicBlock* userBB = user->parent();
      if (userBB && loop.blocks.count(userBB))
        continue;
      for (int k = 0; k < user->numOperands(); ++k) {
        if (user->operand(k) == phi)
          user->setOperand(k, finalVal);
      }
    }
  }

  Instruction* preTerm = preheader->terminator();
  if (!preTerm) {
    traceUnrollReject(func, loop, "cfg-full", "no-preterm");
    return false;
  }
  BasicBlock* firstBody = clonedBlocks[0][cl.body];
  for (int k = 0; k < preTerm->numOperands(); ++k) {
    if (preTerm->operand(k) == cl.header)
      preTerm->setOperand(k, firstBody);
  }

  traceUnrollAccept(func, loop, "cfg-full", trips,
                    static_cast<int>(cloneBudget * static_cast<size_t>(trips)));

  for (BasicBlock* bb : loop.blocks) {
    for (auto& instPtr : bb->instructions())
      instPtr->dropAllOperands();
  }
  std::vector<BasicBlock*> loopBlocks(loop.blocks.begin(), loop.blocks.end());
  for (BasicBlock* bb : loopBlocks)
    func->removeBlock(bb);

  func->recomputeCFG();
  return true;
}

int choosePartialUnrollFactor(const std::vector<Instruction*>& iterationInsts,
                              size_t phiCount) {
  if (iterationInsts.empty())
    return 1;
  size_t bodyCost = iterationInsts.size();
  bool hasFDiv = false;
  int fpOps = countFloatingPointOps(iterationInsts, hasFDiv);
  int memOps = countMemoryOps(iterationInsts);
  int divRemOps = countIntegerDivRemOps(iterationInsts);
  int addressOps = countAddressOps(iterationInsts);
  if (hasFDiv) {
    // 动态 trip-count 的 FDiv 循环在 BOOM/QEMU 上更容易受
    // live range 和代码体积拖累；保留小幅展开来提供一点调度空间，
    // 但避免 4x/8x 把除法 lowering 序列整片复制。
    if (bodyCost <= 28)
      return 2;
  }
  bool fpStreamingUpdate =
      !hasFDiv && fpOps >= 2 && fpOps <= 3 &&
      memOps >= 3 && memOps <= 4 && addressOps >= 2 &&
      phiCount <= 5 && bodyCost <= 12 &&
      hasSameAddressArithmeticUpdate(iterationInsts);
  if (fpStreamingUpdate &&
      sycEnvironmentValue("SYC_SKIP_HIR_FP_STREAMING_UPDATE_UNROLL8") == nullptr) {
    // 三到四路连续访存配合短浮点更新时，八份副本仍能容纳在浮点
    // 寄存器组中，并能减少地址递增与循环分支的占比。
    return 8;
  }
  if (fpOps >= 2) {
    if (bodyCost <= 20)
      return 4;
    if (bodyCost <= static_cast<size_t>(kMaxPartialUnrollBodyInsts))
      return 2;
  }
  if (divRemOps > 0 && memOps <= 2) {
    // 动态 trip-count 下，少量内存配一个或多个整数除/模通常是
    // latency-bound recurrence；partial unroll 只会复制昂贵 lowering 序列、
    // 拉长 live range，并削弱后续模递推 fast path。矩阵/streaming 循环
    // 仍可通过 memory-heavy 分支单独放行。
    return 1;
  }
  bool highStatePressure = phiCount >= 6;
  bool memoryHeavy = memOps >= 5;
  bool addressHeavy = addressOps >= 3 || (phiCount >= 5 && memOps >= 4);
  bool streamingTileLocal =
      divRemOps == 0 && memOps >= 3 && memOps <= 4 &&
      addressOps >= 2 && phiCount <= 5 &&
      hasSameAddressArithmeticUpdate(iterationInsts);
  if (streamingTileLocal && bodyCost <= 16) {
    // 这是通用 tile-local streaming loop 的 unroll-and-jam
    // 收益规则：同一地址 load 经过算术更新后再 store，说明它是
    // update/reduction recurrence，而不是普通 copy/swap。只有这种形态
    // 才允许提升到更大的展开因子，避免 transpose/stencil 类循环被误放大。
    if (bodyCost <= 12 && phiCount <= 4)
      return 16;
    return 4;
  }
  if (divRemOps > 0 && memoryHeavy && highStatePressure) {
    if (bodyCost <= 16 && memOps <= 8)
      return 2;
    bool denseStencilInner =
        divRemOps == 1 && bodyCost <= 24 && memOps <= 8 &&
        addressOps >= 6 && phiCount <= 10;
    if (denseStencilInner) {
      // 单次动态除法本身具有较长延迟，但每个副本还携带多组地址和加载值。
      // 八倍展开可提供足够的独立除法/访存，同时避免十六倍展开越过整数
      // 寄存器容量并产生大量寄存器溢出访存。
      return 8;
    }
    return 1;
  }
  if (memoryHeavy && addressHeavy) {
    if (bodyCost <= 20)
      return highStatePressure ? 1 : 2;
    return 1;
  }
  if (memOps >= 4 && phiCount >= 7)
    return 1;
  if (bodyCost <= 8) {
    // 无访存的短算术递推使用三倍展开，兼顾短行程覆盖率和尾部开销；
    // 含访存的小循环继续使用四倍，以保持连续访问的批量处理能力。
    return memOps == 0 ? 3 : (streamingTileLocal ? 4 : 1);
  }
  if (bodyCost <= 12)
    return streamingTileLocal ? 4 : 1;
  if (bodyCost <= 24)
    return 4;
  if (bodyCost <= static_cast<size_t>(kMaxPartialUnrollBodyInsts))
    return 2;
  return 1;
}

struct StridedCanonicalLoop {
  CanonicalLoop canonical;
  Value* step = nullptr;
};

bool tryStridedCFGPartialUnroll(Function* func, NaturalLoop& loop,
                                const StridedCanonicalLoop& sl);

// 识别与 CanonicalLoop 相同的简单循环形式，但保留循环不变量步长，不要求
// 增量字面量为 1。通用规范循环识别器继续处理单位步长，其他变换依赖这一约定。
bool recognizeStridedLoop(Function* func, const NaturalLoop& loop,
                          BasicBlock* preheader,
                          StridedCanonicalLoop& out) {
  if (!func || !loop.header || !preheader)
    return false;

  BasicBlock* latch = nullptr;
  std::unordered_set<int> seenLatchIds;
  for (auto* candidate : loop.latches) {
    if (!candidate || !seenLatchIds.insert(candidate->id()).second)
      continue;
    if (latch) return false;
    latch = candidate;
  }
  if (!latch)
    return false;

  Instruction* term = loop.header->terminator();
  if (!term || term->opcode() != Opcode::CondBr)
    return false;
  auto* cond = dynamic_cast<Instruction*>(term->operand(0));
  if (!cond || cond->opcode() != Opcode::ICmp)
    return false;
  auto* trueBB = dynamic_cast<BasicBlock*>(term->operand(1));
  auto* falseBB = dynamic_cast<BasicBlock*>(term->operand(2));
  if (!trueBB || !falseBB)
    return false;

  BasicBlock* body = loop.blocks.count(trueBB) ? trueBB : falseBB;
  BasicBlock* exit = loop.blocks.count(trueBB) ? falseBB : trueBB;
  if (!loop.blocks.count(body) || loop.blocks.count(exit))
    return false;

  Value* lhs = cond->operand(0);
  Value* rhs = cond->operand(1);
  auto* ivPhi = dynamic_cast<Instruction*>(lhs);
  bool ivIsLhs = ivPhi && ivPhi->opcode() == Opcode::Phi &&
                 ivPhi->parent() == loop.header;
  if (!ivIsLhs) {
    ivPhi = dynamic_cast<Instruction*>(rhs);
    if (!ivPhi || ivPhi->opcode() != Opcode::Phi ||
        ivPhi->parent() != loop.header)
      return false;
  }
  if (ivPhi->numOperands() != 4)
    return false;

  Value* first = ivPhi->operand(0);
  auto* firstPred = dynamic_cast<BasicBlock*>(ivPhi->operand(1));
  Value* second = ivPhi->operand(2);
  auto* secondPred = dynamic_cast<BasicBlock*>(ivPhi->operand(3));
  Value* init = nullptr;
  Value* nextValue = nullptr;
  if (firstPred == preheader && secondPred == latch) {
    init = first;
    nextValue = second;
  } else if (firstPred == latch && secondPred == preheader) {
    init = second;
    nextValue = first;
  } else {
    return false;
  }

  auto* next = dynamic_cast<Instruction*>(nextValue);
  if (!next || next->opcode() != Opcode::Add || next->parent() != latch ||
      next->numOperands() != 2)
    return false;

  Value* step = nullptr;
  if (next->operand(0) == ivPhi && next->operand(1) != ivPhi)
    step = next->operand(1);
  else if (next->operand(1) == ivPhi && next->operand(0) != ivPhi)
    step = next->operand(0);
  if (!step || dynamic_cast<BasicBlock*>(step))
    return false;

  // 快路径在两次复制的循环体之间跳过循环头部条件，因此步长和上界都必须
  // 是循环前已定义的 SSA 值。循环携带的上界 PHI 可能在第一次循环体后变化，
  // 不能放入这条路径。
  std::vector<BasicBlock*> functionBlocks;
  functionBlocks.reserve(func->blocks().size());
  for (const auto& bbPtr : func->blocks())
    functionBlocks.push_back(bbPtr.get());
  const auto reachable = analysis::collectReachableBlocks(func);
  const auto doms = analysis::computeDominators(func, functionBlocks,
                                                reachable);
  auto loopInvariantAtPreheader = [&](Value* value) {
    auto* inst = dynamic_cast<Instruction*>(value);
    if (!inst)
      return true; // 参数和常量没有所属的基本块
    if (!inst->parent() || loop.blocks.count(inst->parent()))
      return false;
    return analysis::dominates(doms, inst->parent(), preheader);
  };
  Value* bound = ivIsLhs ? rhs : lhs;
  if (!loopInvariantAtPreheader(step) ||
      !loopInvariantAtPreheader(bound))
    return false;

  Instruction* latchTerm = latch->terminator();
  if (!latchTerm || latchTerm->opcode() != Opcode::Br ||
      dynamic_cast<BasicBlock*>(latchTerm->operand(0)) != loop.header)
    return false;

  out.canonical.preheader = preheader;
  out.canonical.header = loop.header;
  out.canonical.body = body;
  out.canonical.latch = latch;
  out.canonical.exit = exit;
  out.canonical.ivPhi = ivPhi;
  out.canonical.ivNext = next;
  out.canonical.cmp = cond;
  out.canonical.condBr = term;
  out.canonical.initVal = init;
  out.canonical.boundVal = bound;
  out.canonical.ivIsCmpLhs = ivIsLhs;
  out.step = step;
  return true;
}

bool tryStridedPartialUnroll(Function* func, NaturalLoop& loop) {
  if (!func)
    return false;
  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader)
    return false;

  StridedCanonicalLoop sl;
  if (!recognizeStridedLoop(func, loop, preheader, sl))
    return false;
  CanonicalLoop& cl = sl.canonical;
  if (!cl.ivIsCmpLhs || canonicalLoopCmpCond(cl.cmp) != "lt")
    return false;

  // 单位步长循环使用上面的规范路径；这里保留给真正的非单位步长循环，避免
  // 改变已有启发式。
  if (auto* stepConst = dynamic_cast<ConstInt*>(sl.step)) {
    if (stepConst->value() == 1)
      return false;
  }
  if (!bodySimpleEnough(loop, cl))
    return tryStridedCFGPartialUnroll(func, loop, sl);
  if (countLoopCalls(loop, cl) > 0)
    return false;

  std::vector<Instruction*> phis;
  std::vector<Value*> phiInitVals;
  std::vector<Value*> phiNextVals;
  if (!collectCanonicalLoopPhis(cl, phis, phiInitVals, &phiNextVals))
    return false;

  std::vector<Instruction*> iterationInsts = collectIterationInsts(cl);
  if (iterationInsts.empty() ||
      iterationInsts.size() > static_cast<size_t>(kMaxPartialUnrollBodyInsts))
    return false;

  // 动态步长需要在运行时检查每个复制增量。这里将因子限制为 2，使检查开销
  // 小于目标流式或算术循环体。
  constexpr int kDynamicStrideFactor = 2;
  BasicBlock* bodyTarget = nullptr;
  if (dynamic_cast<BasicBlock*>(cl.condBr->operand(1)) == cl.body)
    bodyTarget = cl.body;
  else if (dynamic_cast<BasicBlock*>(cl.condBr->operand(2)) == cl.body)
    bodyTarget = cl.body;
  if (!bodyTarget)
    return false;

  auto* dispatchBB = func->newBlock("strided.unroll.dispatch");
  auto* fastBB = func->newBlock("strided.unroll.fast");

  std::vector<Value*> currentPhiVals;
  currentPhiVals.reserve(phis.size());
  for (auto* phi : phis)
    currentPhiVals.push_back(phi);

  std::vector<Value*> finalPhiVals;
  finalPhiVals.reserve(phis.size());
  for (int iter = 0; iter < kDynamicStrideFactor; ++iter) {
    std::unordered_map<Value*, Value*> valueMap;
    for (size_t i = 0; i < phis.size(); ++i)
      valueMap[phis[i]] = currentPhiVals[i];

    for (Instruction* inst : iterationInsts) {
      if (inst->opcode() == Opcode::Phi) {
        Value* selected = selectLinearizedPhiIncoming(inst, cl, valueMap);
        if (!selected)
          return false;
        valueMap[inst] = selected;
        continue;
      }
      auto* clone = cloneInstructionShell(inst);
      for (int k = 0; k < inst->numOperands(); ++k) {
        Value* op = inst->operand(k);
        auto it = valueMap.find(op);
        clone->addOperand(it != valueMap.end() ? it->second : op);
      }
      fastBB->pushBack(clone);
      valueMap[inst] = clone;
    }

    std::vector<Value*> nextVals;
    nextVals.reserve(phis.size());
    for (Value* nxt : phiNextVals) {
      auto it = valueMap.find(nxt);
      nextVals.push_back(it != valueMap.end() ? it->second : nxt);
    }
    currentPhiVals = std::move(nextVals);
  }
  finalPhiVals = currentPhiVals;

  // 检查两次动态增量都为正且不回绕。对有符号正步长，next > current 同时
  // 表示步长为正且没有 i32 有符号溢出；中间值还要满足原始的 < bound 条件，
  // 这样才能进入两次循环体执行的路径。
  auto* firstUpper = new Instruction(Opcode::Add, Type::i32(),
                                      "strided.unroll.first.upper");
  firstUpper->addOperand(cl.ivPhi);
  firstUpper->addOperand(sl.step);
  dispatchBB->pushBack(firstUpper);

  auto* secondUpper = new Instruction(Opcode::Add, Type::i32(),
                                       "strided.unroll.second.upper");
  secondUpper->addOperand(firstUpper);
  secondUpper->addOperand(sl.step);
  dispatchBB->pushBack(secondUpper);

  auto* firstProgress = new Instruction(Opcode::ICmp, Type::i32(),
                                         "gt_strided.unroll.first.progress");
  firstProgress->addOperand(firstUpper);
  firstProgress->addOperand(cl.ivPhi);
  dispatchBB->pushBack(firstProgress);

  auto* secondProgress = new Instruction(Opcode::ICmp, Type::i32(),
                                          "gt_strided.unroll.second.progress");
  secondProgress->addOperand(secondUpper);
  secondProgress->addOperand(firstUpper);
  dispatchBB->pushBack(secondProgress);

  auto* secondEntry = new Instruction(Opcode::ICmp, Type::i32(),
                                       "lt_strided.unroll.second.entry");
  secondEntry->addOperand(firstUpper);
  secondEntry->addOperand(cl.boundVal);
  dispatchBB->pushBack(secondEntry);

  auto* noWrap = new Instruction(Opcode::And, Type::i32(),
                                 "strided.unroll.no-wrap");
  noWrap->addOperand(firstProgress);
  noWrap->addOperand(secondProgress);
  dispatchBB->pushBack(noWrap);

  auto* safe = new Instruction(Opcode::And, Type::i32(),
                               "strided.unroll.safe");
  safe->addOperand(noWrap);
  safe->addOperand(secondEntry);
  dispatchBB->pushBack(safe);

  auto* fastBr = new Instruction(Opcode::Br, Type::voidTy());
  fastBr->addOperand(cl.header);
  fastBB->pushBack(fastBr);

  auto* dispatchBr = new Instruction(Opcode::CondBr, Type::voidTy());
  dispatchBr->addOperand(safe);
  dispatchBr->addOperand(fastBB);
  dispatchBr->addOperand(cl.body);
  dispatchBB->pushBack(dispatchBr);

  if (dynamic_cast<BasicBlock*>(cl.condBr->operand(1)) == cl.body)
    cl.condBr->setOperand(1, dispatchBB);
  else
    cl.condBr->setOperand(2, dispatchBB);

  for (size_t i = 0; i < phis.size(); ++i) {
    Instruction* phi = phis[i];
    phi->addOperand(finalPhiVals[i]);
    phi->addOperand(fastBB);
  }

  func->recomputeCFG();
  traceUnrollAccept(func, loop, "strided-partial", kDynamicStrideFactor,
                    static_cast<int>(iterationInsts.size()));
  return true;
}

bool tryStridedCFGPartialUnroll(Function* func, NaturalLoop& loop,
                                const StridedCanonicalLoop& sl) {
  if (!func)
    return false;
  const CanonicalLoop& cl = sl.canonical;
  if (!cl.ivIsCmpLhs || canonicalLoopCmpCond(cl.cmp) != "lt")
    return false;
  if (auto* stepConst = dynamic_cast<ConstInt*>(sl.step)) {
    if (stepConst->value() == 1)
      return false;
  }
  if (countLoopCalls(loop, cl) > 0)
    return false;

  const char* rejectReason = nullptr;
  if (!cfgFullUnrollSimpleEnough(func, loop, cl, 0, &rejectReason))
    return false;

  std::vector<Instruction*> phis;
  std::vector<Value*> phiInitVals;
  std::vector<Value*> phiNextVals;
  if (!collectCanonicalLoopPhis(cl, phis, phiInitVals, &phiNextVals))
    return false;

  std::vector<BasicBlock*> orderedBlocks =
      collectLoopBlocksInFunctionOrder(func, loop, cl);
  if (orderedBlocks.empty())
    return false;
  size_t cloneBudget = orderedBlocks.size();
  std::vector<Instruction*> loopInsts;
  for (BasicBlock* bb : orderedBlocks) {
    cloneBudget += bb->instructions().size();
    for (const auto& instPtr : bb->instructions()) {
      if (instPtr && !instPtr->isTerminator())
        loopInsts.push_back(instPtr.get());
    }
  }
  constexpr size_t kMaxDynamicStrideCFGClones =
      static_cast<size_t>(kMaxFullUnrollClones) * 2;
  constexpr int kDynamicStrideFactor = 2;
  if (cloneBudget * static_cast<size_t>(kDynamicStrideFactor) >
      kMaxDynamicStrideCFGClones)
    return false;

  // 运行时除数会保留高延迟除法序列，较大的 CFG 循环继续避免复制；
  // 常量除数会在后续降低为乘法和移位，可沿用现有克隆预算做二倍展开。
  const int divRemOps = countIntegerDivRemOps(loopInsts);
  if (cloneBudget > 16 && divRemOps >= 2 &&
      !integerDivRemDivisorsAreConstant(loopInsts)) {
    traceUnrollReject(func, loop, "strided-cfg-partial",
                      "latency-bound-div-rem");
    return false;
  }

  BasicBlock* bodyTarget = nullptr;
  if (dynamic_cast<BasicBlock*>(cl.condBr->operand(1)) == cl.body)
    bodyTarget = cl.body;
  else if (dynamic_cast<BasicBlock*>(cl.condBr->operand(2)) == cl.body)
    bodyTarget = cl.body;
  if (!bodyTarget)
    return false;

  auto* dispatchBB = func->newBlock("strided.cfg.dispatch");
  auto* fastExitBB = func->newBlock("strided.cfg.exit");

  std::vector<std::unordered_map<BasicBlock*, BasicBlock*>> clonedBlocks(
      kDynamicStrideFactor);
  for (int iter = 0; iter < kDynamicStrideFactor; ++iter) {
    for (BasicBlock* oldBB : orderedBlocks)
      clonedBlocks[iter][oldBB] = func->newBlock("strided.cfg.unroll");
  }

  std::unordered_map<Instruction*, Value*> curPhiValues;
  for (size_t i = 0; i < phis.size(); ++i)
    // 这是部分展开：快路径可能从任意迭代进入，因此第一个副本必须从当前
    // 循环头部 PHI 的活跃值开始，而不是函数原始前置块的值。
    curPhiValues[phis[i]] = phis[i];
  std::vector<Value*> finalPhiValues(phis.begin(), phis.end());

  for (int iter = 0; iter < kDynamicStrideFactor; ++iter) {
    std::unordered_map<Value*, Value*> valueMap;
    for (size_t i = 0; i < phis.size(); ++i)
      valueMap[phis[i]] = curPhiValues[phis[i]];

    // 先建立所有指令外壳，使内部 PHI 可以引用函数顺序中稍后出现的外壳。
    for (BasicBlock* oldBB : orderedBlocks) {
      BasicBlock* newBB = clonedBlocks[iter][oldBB];
      for (auto& instPtr : oldBB->instructions()) {
        Instruction* inst = instPtr.get();
        if (inst->isTerminator())
          continue;
        auto* clone = cloneInstructionShell(inst);
        newBB->pushBack(clone);
        valueMap[inst] = clone;
      }
    }

    for (BasicBlock* oldBB : orderedBlocks) {
      for (auto& instPtr : oldBB->instructions()) {
        Instruction* inst = instPtr.get();
        if (inst->isTerminator())
          continue;
        auto* clone = dynamic_cast<Instruction*>(valueMap[inst]);
        if (!clone)
          return false;
        for (int k = 0; k < inst->numOperands(); ++k) {
          Value* op = inst->operand(k);
          if (inst->opcode() == Opcode::Phi && (k % 2) == 1) {
            auto* pred = dynamic_cast<BasicBlock*>(op);
            auto blockIt = pred ? clonedBlocks[iter].find(pred)
                                : clonedBlocks[iter].end();
            if (blockIt == clonedBlocks[iter].end())
              return false;
            clone->addOperand(blockIt->second);
            continue;
          }
          auto it = valueMap.find(op);
          clone->addOperand(it != valueMap.end() ? it->second : op);
        }
      }
    }

    std::vector<Value*> nextPhiValues;
    nextPhiValues.reserve(phis.size());
    for (Value* nxt : phiNextVals) {
      auto it = valueMap.find(nxt);
      nextPhiValues.push_back(it != valueMap.end() ? it->second : nxt);
    }

    for (BasicBlock* oldBB : orderedBlocks) {
      Instruction* term = oldBB->terminator();
      if (!term)
        return false;
      auto* clone = cloneInstructionShell(term);
      for (int k = 0; k < term->numOperands(); ++k) {
        Value* op = term->operand(k);
        if (auto* targetBB = dynamic_cast<BasicBlock*>(op)) {
          BasicBlock* mappedTarget = nullptr;
          if (targetBB == cl.header) {
            if (oldBB != cl.latch)
              return false;
            mappedTarget = (iter + 1 < kDynamicStrideFactor)
                               ? clonedBlocks[iter + 1][cl.body]
                               : fastExitBB;
          } else if (loop.blocks.count(targetBB)) {
            auto it = clonedBlocks[iter].find(targetBB);
            if (it == clonedBlocks[iter].end())
              return false;
            mappedTarget = it->second;
          } else if (targetBB == cl.exit) {
            mappedTarget = cl.exit;
          } else {
            return false;
          }
          clone->addOperand(mappedTarget);
          continue;
        }
        auto it = valueMap.find(op);
        clone->addOperand(it != valueMap.end() ? it->second : op);
      }
      clonedBlocks[iter][oldBB]->pushBack(clone);
    }

    for (size_t i = 0; i < phis.size(); ++i)
      curPhiValues[phis[i]] = nextPhiValues[i];
    finalPhiValues = std::move(nextPhiValues);
  }

  // 检查两次正向且不回绕的增量，以及第二次循环入口。
  auto* firstUpper = new Instruction(Opcode::Add, Type::i32(),
                                      "strided.cfg.first.upper");
  firstUpper->addOperand(cl.ivPhi);
  firstUpper->addOperand(sl.step);
  dispatchBB->pushBack(firstUpper);
  auto* secondUpper = new Instruction(Opcode::Add, Type::i32(),
                                       "strided.cfg.second.upper");
  secondUpper->addOperand(firstUpper);
  secondUpper->addOperand(sl.step);
  dispatchBB->pushBack(secondUpper);
  auto* firstProgress = new Instruction(Opcode::ICmp, Type::i32(),
                                         "gt_strided.cfg.first.progress");
  firstProgress->addOperand(firstUpper);
  firstProgress->addOperand(cl.ivPhi);
  dispatchBB->pushBack(firstProgress);
  auto* secondProgress = new Instruction(Opcode::ICmp, Type::i32(),
                                          "gt_strided.cfg.second.progress");
  secondProgress->addOperand(secondUpper);
  secondProgress->addOperand(firstUpper);
  dispatchBB->pushBack(secondProgress);
  auto* secondEntry = new Instruction(Opcode::ICmp, Type::i32(),
                                       "lt_strided.cfg.second.entry");
  secondEntry->addOperand(firstUpper);
  secondEntry->addOperand(cl.boundVal);
  dispatchBB->pushBack(secondEntry);
  auto* noWrap = new Instruction(Opcode::And, Type::i32(),
                                 "strided.cfg.no-wrap");
  noWrap->addOperand(firstProgress);
  noWrap->addOperand(secondProgress);
  dispatchBB->pushBack(noWrap);
  auto* safe = new Instruction(Opcode::And, Type::i32(),
                               "strided.cfg.safe");
  safe->addOperand(noWrap);
  safe->addOperand(secondEntry);
  dispatchBB->pushBack(safe);

  auto* fastExitBr = new Instruction(Opcode::Br, Type::voidTy());
  fastExitBr->addOperand(cl.header);
  fastExitBB->pushBack(fastExitBr);

  auto* dispatchBr = new Instruction(Opcode::CondBr, Type::voidTy());
  dispatchBr->addOperand(safe);
  dispatchBr->addOperand(clonedBlocks[0][cl.body]);
  dispatchBr->addOperand(cl.body);
  dispatchBB->pushBack(dispatchBr);

  if (dynamic_cast<BasicBlock*>(cl.condBr->operand(1)) == cl.body)
    cl.condBr->setOperand(1, dispatchBB);
  else
    cl.condBr->setOperand(2, dispatchBB);

  for (size_t i = 0; i < phis.size(); ++i) {
    phis[i]->addOperand(finalPhiValues[i]);
    phis[i]->addOperand(fastExitBB);
  }

  func->recomputeCFG();
  traceUnrollAccept(func, loop, "strided-cfg-partial", kDynamicStrideFactor,
                    static_cast<int>(cloneBudget));
  return true;
}

bool tryPartialUnroll(Function* func, NaturalLoop& loop) {
  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader)
    return false;
  CanonicalLoop cl;
  if (!recognizeCanonicalLoop(loop, preheader, cl))
    return false;
  if (!cl.ivIsCmpLhs || canonicalLoopCmpCond(cl.cmp) != "lt")
    return false;
  if (!bodySimpleEnough(loop, cl))
    return false;
  if (countLoopCalls(loop, cl) > 0)
    return false;

  std::vector<Instruction*> phis;
  std::vector<Value*> phiInitVals;
  std::vector<Value*> phiNextVals;
  if (!collectCanonicalLoopPhis(cl, phis, phiInitVals, &phiNextVals))
    return false;

  std::vector<Instruction*> iterationInsts = collectIterationInsts(cl);
  if (iterationInsts.empty() ||
      iterationInsts.size() > static_cast<size_t>(kMaxPartialUnrollBodyInsts))
    return false;
  int factor = choosePartialUnrollFactor(iterationInsts, phis.size());
  if (factor < 2)
    return false;

  int ivIndex = -1;
  for (size_t i = 0; i < phis.size(); ++i) {
    if (phis[i] == cl.ivPhi) {
      ivIndex = static_cast<int>(i);
      break;
    }
  }
  if (ivIndex < 0)
    return false;

  auto* dispatchBB = func->newBlock("unroll.dispatch");
  auto* fastBB = func->newBlock("unroll.fast");
  std::vector<Value*> finalPhiVals;
  auto appendLinearizedIterations =
      [&](BasicBlock* target, int count,
          std::vector<Value*>& finalValues) -> bool {
    std::vector<Value*> currentValues;
    currentValues.reserve(phis.size());
    for (Instruction* phi : phis)
      currentValues.push_back(phi);
    for (int iter = 0; iter < count; ++iter) {
      std::unordered_map<Value*, Value*> valueMap;
      for (size_t i = 0; i < phis.size(); ++i)
        valueMap[phis[i]] = currentValues[i];

      for (Instruction* inst : iterationInsts) {
        if (inst->opcode() == Opcode::Phi) {
          Value* selected = selectLinearizedPhiIncoming(inst, cl, valueMap);
          if (!selected)
            return false;
          valueMap[inst] = selected;
          continue;
        }
        auto* clone = cloneInstructionShell(inst);
        for (int k = 0; k < inst->numOperands(); ++k) {
          Value* op = inst->operand(k);
          auto it = valueMap.find(op);
          clone->addOperand(it != valueMap.end() ? it->second : op);
        }
        target->pushBack(clone);
        valueMap[inst] = clone;
      }

      std::vector<Value*> nextValues;
      nextValues.reserve(phis.size());
      for (Value* next : phiNextVals) {
        auto it = valueMap.find(next);
        nextValues.push_back(it != valueMap.end() ? it->second : next);
      }
      currentValues = std::move(nextValues);
    }
    finalValues = std::move(currentValues);
    auto* back = new Instruction(Opcode::Br, Type::voidTy());
    back->addOperand(cl.header);
    target->pushBack(back);
    return true;
  };
  if (!appendLinearizedIterations(fastBB, factor, finalPhiVals))
    return false;

  bool usePairTail = factor == 3 && countMemoryOps(iterationInsts) == 0;
  BasicBlock* pairDispatchBB = nullptr;
  BasicBlock* pairBB = nullptr;
  std::vector<Value*> pairFinalPhiVals;
  if (usePairTail) {
    // 三倍展开的纯算术递推常留下两次尾迭代。增加一个两倍尾路径，避免尾部
    // 连续两次返回循环头，同时不扩大其它访存循环的代码体积。
    pairDispatchBB = func->newBlock("unroll.pair.dispatch");
    pairBB = func->newBlock("unroll.pair");
    if (!appendLinearizedIterations(pairBB, 2, pairFinalPhiVals))
      return false;
  }

  auto* fastUpper =
      new Instruction(Opcode::Add, Type::i32(), "partial.unroll.upper");
  fastUpper->addOperand(cl.ivPhi);
  fastUpper->addOperand(func->parent()->getConstInt(factor - 1));
  dispatchBB->pushBack(fastUpper);
  auto* fastCmp = new Instruction(Opcode::ICmp, Type::i32(), cl.cmp->name());
  fastCmp->addOperand(fastUpper);
  fastCmp->addOperand(cl.boundVal);
  dispatchBB->pushBack(fastCmp);
  auto* dispatchBr = new Instruction(Opcode::CondBr, Type::voidTy());
  dispatchBr->addOperand(fastCmp);
  dispatchBr->addOperand(fastBB);
  dispatchBr->addOperand(usePairTail ? pairDispatchBB : cl.body);
  dispatchBB->pushBack(dispatchBr);

  if (usePairTail) {
    auto* pairUpper = new Instruction(Opcode::Add, Type::i32(),
                                      "partial.unroll.pair.upper");
    pairUpper->addOperand(cl.ivPhi);
    pairUpper->addOperand(func->parent()->getConstInt(1));
    pairDispatchBB->pushBack(pairUpper);
    auto* pairCmp =
        new Instruction(Opcode::ICmp, Type::i32(), cl.cmp->name());
    pairCmp->addOperand(pairUpper);
    pairCmp->addOperand(cl.boundVal);
    pairDispatchBB->pushBack(pairCmp);
    auto* pairDispatchBr = new Instruction(Opcode::CondBr, Type::voidTy());
    pairDispatchBr->addOperand(pairCmp);
    pairDispatchBr->addOperand(pairBB);
    pairDispatchBr->addOperand(cl.body);
    pairDispatchBB->pushBack(pairDispatchBr);
  }

  if (dynamic_cast<BasicBlock*>(cl.condBr->operand(1)) == cl.body)
    cl.condBr->setOperand(1, dispatchBB);
  else if (dynamic_cast<BasicBlock*>(cl.condBr->operand(2)) == cl.body)
    cl.condBr->setOperand(2, dispatchBB);
  else
    return false;

  for (size_t i = 0; i < phis.size(); ++i) {
    Instruction* phi = phis[i];
    phi->addOperand(finalPhiVals[i]);
    phi->addOperand(fastBB);
    if (usePairTail) {
      phi->addOperand(pairFinalPhiVals[i]);
      phi->addOperand(pairBB);
    }
  }

  func->recomputeCFG();
  traceUnrollAccept(func, loop,
                    usePairTail ? "partial-pair-tail" : "partial", factor,
                    static_cast<int>(iterationInsts.size()));
  return true;
}

bool tryFullUnroll(Function* func, NaturalLoop& loop,
                   int requiredTrips = 0) {
  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader) {
    traceUnrollReject(func, loop, "full", "no-preheader");
    return false;
  }
  CanonicalLoop cl;
  if (!recognizeCanonicalLoop(loop, preheader, cl)) {
    traceUnrollReject(func, loop, "full", "not-canonical");
    return false;
  }

  int trips = 0;
  const char* tripReject = nullptr;
  if (!computeConstForwardTripCount(cl, trips, &tripReject)) {
    traceUnrollReject(func, loop, "full", tripReject);
    return false;
  }
  if (requiredTrips > 0 && trips != requiredTrips)
    return false;

  int callCount = countLoopCalls(loop, cl);
  if (callCount > 0 && trips > kMaxCallUnrollTrips) {
    traceUnrollReject(func, loop, "full", "call-trip-budget");
    return false;
  }
  bool callAware = callCount == 1;
  const char* simpleReject = nullptr;
  if (!bodySimpleEnough(loop, cl, callAware ? 1 : 0, &simpleReject)) {
    traceUnrollReject(func, loop, "full", simpleReject);
    return false;
  }
  std::vector<Instruction*> bodyInsts = collectIterationInsts(cl);
  size_t cloneBudget = static_cast<size_t>(callAware
                                               ? kMaxCallAwareFullUnrollClones
                                               : kMaxFullUnrollClones);
  if (bodyInsts.empty() ||
      bodyInsts.size() * static_cast<size_t>(trips) > cloneBudget) {
    traceUnrollReject(func, loop, "full", "clone-budget");
    return false;
  }

  // 收集循环头中的所有 phi 节点，即循环携带值。
  std::vector<Instruction*> phis;
  std::vector<Value*> phiInitVals;
  std::vector<Value*> phiNextVals; // 来自锁存块的值
  if (!collectCanonicalLoopPhis(cl, phis, phiInitVals, &phiNextVals)) {
    traceUnrollReject(func, loop, "full", "phi-collect");
    return false;
  }

  Instruction* preTerm = preheader->terminator();
  if (!preTerm) {
    traceUnrollReject(func, loop, "full", "no-preterm");
    return false;
  }

  // 迭代值映射：原 IR 值 -> 当前迭代的替代值。
  // 初始状态为 phi -> 其初始值。
  std::unordered_map<Value*, Value*> curMap;
  for (size_t i = 0; i < phis.size(); ++i) {
    curMap[phis[i]] = phiInitVals[i];
  }

  // 展开：逐次把循环体指令复制到前置块，并替换归纳变量和 phi 值。
  Module* mod = func->parent();
  auto* initC = dynamic_cast<ConstInt*>(cl.initVal);
  int initV = initC ? initC->value() : 0;
  for (int iter = 0; iter < trips; ++iter) {
    int ivConst = initV + iter;
    Value* ivVal = mod->getConstInt(ivConst);
    std::unordered_map<Value*, Value*> iterMap = curMap;
    iterMap[cl.ivPhi] = ivVal;

    for (Instruction* inst : bodyInsts) {
      auto* clone = new Instruction(inst->opcode(), inst->type(), inst->name());
      clone->setMemoryInfo(inst->memoryElemTy(), inst->memorySizeBytes(),
                           inst->isArrayObject(), inst->dims());
      for (int k = 0; k < inst->numOperands(); ++k) {
        Value* opnd = inst->operand(k);
        auto it = iterMap.find(opnd);
        clone->addOperand(it != iterMap.end() ? it->second : opnd);
      }
      preheader->insertBefore(preTerm, clone);
      iterMap[inst] = clone;
    }

    // 为下一次迭代更新 curMap：phi i 使用经本轮映射解析后的 `phiNextVals[i]`。
    std::unordered_map<Value*, Value*> newCur;
    for (size_t i = 0; i < phis.size(); ++i) {
      Value* nxt = phiNextVals[i];
      auto it = iterMap.find(nxt);
      newCur[phis[i]] = (it != iterMap.end()) ? it->second : nxt;
    }
    curMap = std::move(newCur);
  }

  // 展开后，循环头部 PHI 的外部使用改为最后一次迭代的值。遍历所有使用，
  // 将循环块之外的使用重定向到该值。
  for (size_t i = 0; i < phis.size(); ++i) {
    Instruction* phi = phis[i];
    Value* finalVal = curMap[phi];
    // 修改前先保存使用者列表。
    std::vector<Instruction*> users = phi->uses();
    for (Instruction* user : users) {
      BasicBlock* userBB = user->parent();
      if (loop.blocks.count(userBB)) continue; // 仍位于即将删除的循环内部
      for (int k = 0; k < user->numOperands(); ++k) {
        if (user->operand(k) == phi) user->setOperand(k, finalVal);
      }
    }
  }

  // 让前置块的终结指令直接跳转到出口；其原形应为从 dedicatedPreheader
  // 跳转到 header 的分支。
  for (int k = 0; k < preTerm->numOperands(); ++k) {
    if (preTerm->operand(k) == cl.header) preTerm->setOperand(k, cl.exit);
  }

  traceUnrollAccept(func, loop, "full", trips,
                    static_cast<int>(bodyInsts.size() *
                                     static_cast<size_t>(trips)));

  // 从函数中移除循环块。移除顺序不能让块内指令继续引用 header/body/latch；
  // 先丢弃操作数断开使用链，再调用 removeBlock。
  for (BasicBlock* bb : loop.blocks) {
    for (auto& instPtr : bb->instructions()) {
      instPtr->dropAllOperands();
    }
  }
  // 在移除前保存循环块快照（集合可能会
  // 已修改）。
  std::vector<BasicBlock*> loopBlocks(loop.blocks.begin(), loop.blocks.end());
  for (BasicBlock* bb : loopBlocks) {
    func->removeBlock(bb);
  }

  // 重新计算 CFG，使前驱和后继反映新的结构。
  func->recomputeCFG();
  return true;
}

} // namespace

bool applyLoopUnroll(Module& mod, analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (func->blocks().empty()) continue;

    // 检测函数是否有多层 blocking（≥3 层 tile header）。
    // 多层 blocking 的内层循环 partial peel 会产生严重 cascade。
    // 单层 blocking 通常可以安全地进行 partial unroll。
    int tileHeaderCount = 0;
    for (auto& bb : func->blocks()) {
      if (bb->name().find("tile.header") != std::string::npos)
        tileHeaderCount++;
    }
    bool hasDeepBlocking = tileHeaderCount >= 3;

    bool anyInFunc = true;
    int safety = 8;
    while (anyInFunc && safety-- > 0) {
      anyInFunc = false;
      analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
      auto loops = cache.loops();
      std::sort(loops.begin(), loops.end(),
                [](const NaturalLoop& lhs, const NaturalLoop& rhs) {
                  if (lhs.blocks.size() != rhs.blocks.size())
                    return lhs.blocks.size() < rhs.blocks.size();
                  return lhs.header && rhs.header
                             ? lhs.header->id() < rhs.header->id()
                             : lhs.header < rhs.header;
                });
      for (auto& loop : loops) {
        bool didUnroll = false;
        if (tryFullUnroll(func, loop) ||
            tryCFGFullUnroll(func, loop)) {
          didUnroll = true;
        } else if (!hasDeepBlocking || containsPreparedInvariantDivision(loop)) {
          // 描述符除法已经把昂贵准备工作移到循环外；即使函数中还有较深
          // 分块，也允许当前内层循环由既有成本模型决定是否部分展开。
          // 其它循环仍只在没有多层分块时尝试部分展开，避免分块循环体连续
          // 复制并形成级联代码增长。
          didUnroll = tryPartialUnroll(func, loop) ||
                      tryStridedPartialUnroll(func, loop);
        }
        if (didUnroll) {
          changed = true;
          anyInFunc = true;
          analysisManager.invalidateFunction(
              func, analysis::AnalysisInvalidation::CFG);
          break; // CFG 改变后重新发现循环
        }
      }
    }
  }
  return changed;
}

bool applySingleTripLoopUnroll(
    Module& mod, analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func || func->blocks().empty())
      continue;
    bool retry = true;
    int safety = 4;
    while (retry && safety-- > 0) {
      retry = false;
      analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
      auto loops = cache.loops();
      std::sort(loops.begin(), loops.end(),
                [](const NaturalLoop& lhs, const NaturalLoop& rhs) {
                  return lhs.blocks.size() < rhs.blocks.size();
                });
      for (auto& loop : loops) {
        if (!tryFullUnroll(func, loop, /*requiredTrips=*/1) &&
            !tryCFGFullUnroll(func, loop, /*requiredTrips=*/1)) {
          continue;
        }
        changed = true;
        retry = true;
        analysisManager.invalidateFunction(
            func, analysis::AnalysisInvalidation::CFG);
        break;
      }
    }
  }
  return changed;
}

bool applyLoopUnroll(Module& mod) {
  analysis::AnalysisManager analysisManager(mod);
  return applyLoopUnroll(mod, analysisManager);
}

} // namespace hir
