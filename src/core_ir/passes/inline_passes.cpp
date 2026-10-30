#include "research_switch.h"
// 文件职责：执行 CoreIR 函数内联及相关调用图整理。
#include "inline_passes.h"
#include "cse.h"
#include "phi_utils.h"
#include "../../analysis/function_effects.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace core_ir {

int argIndexOf(const std::vector<Arg>& args, Value* val) {
  for (int i = 0; i < static_cast<int>(args.size()); ++i) {
    if (args[i].val == val)
      return i;
  }
  return -1;
}

bool isTrivialSelectHelper(Function* func) {
  // main 是程序入口，不按普通纯 helper 的结构做 helper 分类。
  if (!func || func->name() == "main" || func->blocks().size() != 4)
    return false;
  if (func->retTy().isF32() && func->blocks().size() > 1)
    return false;
  for (const auto& arg : func->args()) {
    if (arg.type.isF32() && func->blocks().size() > 1)
      return false;
  }
  if (!func->retTy().isI32() && !func->retTy().isPtr())
    return false;
  const auto& args = func->args();
  if (args.size() != 2 || !args[0].val || !args[1].val)
    return false;
  if (args[0].type != func->retTy() || args[1].type != func->retTy())
    return false;

  BasicBlock* entry = func->entryBlock();
  if (!entry)
    return false;
  Instruction* entryTerm = entry->terminator();
  if (!entryTerm || entryTerm->opcode() != Opcode::CondBr ||
      entryTerm->numOperands() != 3 || entry->instructions().size() != 2)
    return false;

  auto* cmp = dynamic_cast<Instruction*>(entryTerm->operand(0));
  auto* trueBB = dynamic_cast<BasicBlock*>(entryTerm->operand(1));
  auto* falseBB = dynamic_cast<BasicBlock*>(entryTerm->operand(2));
  if (!cmp || cmp->opcode() != Opcode::ICmp || !trueBB || !falseBB ||
      cmp->numOperands() != 2)
    return false;
  if (argIndexOf(args, cmp->operand(0)) < 0 ||
      argIndexOf(args, cmp->operand(1)) < 0)
    return false;

  auto branchesTo = [](BasicBlock* bb, BasicBlock*& mergeBB) {
    if (!bb || !bb->terminator() || bb->terminator()->opcode() != Opcode::Br ||
        bb->terminator()->numOperands() != 1 || bb->instructions().size() != 1)
      return false;
    auto* succ = dynamic_cast<BasicBlock*>(bb->terminator()->operand(0));
    if (!succ)
      return false;
    if (!mergeBB)
      mergeBB = succ;
    return mergeBB == succ;
  };

  BasicBlock* retBB = nullptr;
  if (!branchesTo(trueBB, retBB) || !branchesTo(falseBB, retBB) || !retBB)
    return false;
  if (!retBB->terminator() || retBB->terminator()->opcode() != Opcode::Ret ||
      retBB->terminator()->numOperands() != 1 || retBB->instructions().size() != 2)
    return false;

  auto* phi = retBB->instructions().front().get();
  if (!phi || phi->opcode() != Opcode::Phi || phi->type() != func->retTy() ||
      phi->numOperands() != 4 || retBB->terminator()->operand(0) != phi)
    return false;

  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
    if ((pred != trueBB && pred != falseBB) ||
        argIndexOf(args, phi->operand(i)) < 0) {
      return false;
    }
  }
  return true;
}

bool hasInlineableLoopShape(Function* func,
                            analysis::FunctionAnalysisCache& cache,
                            int maxLoopCount,
                            int maxLoopInstsPerLoop,
                            int maxLatchCount) {
  if (!func || func->blocks().empty())
    return true;
  // 内联成本模型会重复查询循环形状，因此复用函数级缓存，
  // 不再为每个候选 helper 单独重复计算 reachable/dominator/loop。
  const auto& loops = cache.loops();
  if (static_cast<int>(loops.size()) > maxLoopCount)
    return false;
  if (loops.empty())
    return true;
  for (const auto& loop : loops) {
    // 普通小 helper 仍要求单 latch；单调用者过程允许少量由 continue/break
    // 普通小 helper 仍要求单 latch；单调用者过程允许少量由 continue/break
    // 形成的多 latch 循环，因为内联不会跨调用者复制代码，后续循环规范化和循环内存
    // 阶段会继续检查具体形状。

    if (loop.latches.empty() ||
        static_cast<int>(loop.latches.size()) > maxLatchCount) {
      return false;
    }
    int instCount = 0;
    for (auto* bb : loop.blocks) {
      for (auto& instPtr : bb->instructions()) {
        Opcode op = instPtr->opcode();
        if (op == Opcode::Call || op == Opcode::Alloca)
          return false;
        if (!instPtr->isTerminator() && op != Opcode::Phi &&
            ++instCount > maxLoopInstsPerLoop) {
          return false;
        }
      }
    }
  }
  return true;
}

// 延迟敏感的算术循环不进入通用 late inline。包含多个整数除法或取模操作的
// helper 在标量清理后可能看起来很小，但内联会复制昂贵的降低过程并丢失有用的
// 过程边界。循环展开由独立的收益判断处理。
bool hasLatencyBoundArithmeticLoop(analysis::FunctionAnalysisCache& cache) {
  for (const auto& loop : cache.loops()) {
    bool hasDivRem = false;
    int arithmeticOps = 0;
    int memoryOps = 0;
    int bodyInsts = 0;
    for (BasicBlock* block : loop.blocks) {
      if (!block)
        return false;
      for (const auto& instPtr : block->instructions()) {
        if (!instPtr || instPtr->isTerminator())
          continue;
        ++bodyInsts;
        if (instPtr->opcode() == Opcode::Load ||
            instPtr->opcode() == Opcode::Store) {
          ++memoryOps;
        }
        if (instPtr->opcode() == Opcode::Mul ||
            instPtr->opcode() == Opcode::SDiv ||
            instPtr->opcode() == Opcode::SRem) {
          ++arithmeticOps;
        }
        if (instPtr->opcode() == Opcode::SDiv ||
            instPtr->opcode() == Opcode::SRem) {
          hasDivRem = true;
        }
      }
      if (bodyInsts >= 8 && memoryOps == 0 &&
          (hasDivRem || arithmeticOps >= 2))
        return true;
    }
  }
  return false;
}

bool hasAtMostOneSmallLoop(Function* func,
                           analysis::FunctionAnalysisCache& cache) {
  return hasInlineableLoopShape(func, cache, 1, 64, 1);
}

bool hasFiniteProcedureLoopNest(Function* func,
                                analysis::FunctionAnalysisCache& cache) {
  // 单调用点过程内联不会复制代码，因此这里允许有限层数的
  // 小循环族和少量 continue/break 形成的多 latch，只要没有调用和栈对象，
  // 继续使用 block/cost 总预算约束。
  return hasInlineableLoopShape(func, cache, 6, 192, 4);
}

bool hasStatefulHelperLoopShape(Function* func,
                                analysis::FunctionAnalysisCache& cache) {
  // 只读写标量全局状态的小 helper 常见于伪随机数、计数器、
  // rolling hash 等状态推进。它们留在热循环中会阻断后续 LICM、
  // 常量取模强度化和 loop-memory 分析；这里按通用 effect/loop 形状放宽。
  return hasInlineableLoopShape(func, cache, 1, 192, 4);
}

bool isSmallScalarGlobal(Value* value) {
  auto* gv = dynamic_cast<GlobalVar*>(memoryRoot(value));
  return gv && gv->sizeBytes() == 4 && gv->dims().empty();
}

bool usesOnlyScalarGlobalState(Function* func) {
  if (!func)
    return false;
  bool touchedGlobal = false;
  for (auto& bbPtr : func->blocks()) {
    for (auto& instPtr : bbPtr->instructions()) {
      Instruction* inst = instPtr.get();
      if (inst->opcode() != Opcode::Load && inst->opcode() != Opcode::Store)
        continue;
      Value* addr =
          inst->opcode() == Opcode::Load ? inst->operand(0) : inst->operand(1);
      Value* root = memoryRoot(addr);
      if (dynamic_cast<Instruction*>(root) &&
          dynamic_cast<Instruction*>(root)->opcode() == Opcode::Alloca) {
        continue;
      }
      if (!isSmallScalarGlobal(addr))
        return false;
      touchedGlobal = true;
    }
  }
  return touchedGlobal;
}

struct DirectCallUseSummary {
  int callSiteCount = 0;
  int callerCount = 0;
};

struct ReturnChainCallSite {
  Function* caller = nullptr;
  bool resultIgnored = false;
  bool resultReturned = false;
};

  // 如果纯 callee 的结果只会通过同样丢弃结果的调用者返回，它就不会产生可观察
  // 工作。先保留调用，等外层 helper 内联后再由 deadPureCallElimination 删除完整
  // 计算，避免把这段计算复制到热点调用者中。
std::unordered_set<Function*> collectDeadReturnChainFunctions(
    Module& mod, const analysis::FunctionEffectMap& effects) {
  std::unordered_map<std::string, Function*> definitions;
  std::unordered_map<Function*, std::vector<ReturnChainCallSite>> callSites;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func)
      continue;
    definitions[func->name()] = func;
    callSites[func] = {};
  }

  for (auto& funcPtr : mod.functions()) {
    Function* caller = funcPtr.get();
    if (!caller)
      continue;
    for (auto& bbPtr : caller->blocks()) {
      for (auto& instPtr : bbPtr->instructions()) {
        Instruction* call = instPtr.get();
        if (!call || call->opcode() != Opcode::Call || call->type().isVoid())
          continue;
        auto calleeIt = definitions.find(call->name());
        if (calleeIt == definitions.end())
          continue;

        bool resultReturned = !call->uses().empty();
        for (Instruction* user : call->uses()) {
          if (!user || user->opcode() != Opcode::Ret ||
              user->numOperands() != 1 || user->operand(0) != call) {
            resultReturned = false;
            break;
          }
        }
        callSites[calleeIt->second].push_back(
            {caller, call->uses().empty(), resultReturned});
      }
    }
  }

  std::unordered_set<Function*> deadReturns;
  bool changed = true;
  while (changed) {
    changed = false;
    for (auto& entry : callSites) {
      Function* callee = entry.first;
      if (!callee || deadReturns.count(callee) || entry.second.empty())
        continue;
      bool allDead = true;
      for (const ReturnChainCallSite& site : entry.second) {
        if (site.resultIgnored)
          continue;
        if (!site.resultReturned || !deadReturns.count(site.caller)) {
          allDead = false;
          break;
        }
      }
      if (allDead) {
        deadReturns.insert(callee);
        changed = true;
      }
    }
  }

  std::unordered_set<Function*> pureDeadReturns;
  for (Function* func : deadReturns) {
    const auto* summary = analysis::lookupFunctionEffect(effects, func);
    if (summary && summary->pure && !summary->selfRecursive)
      pureDeadReturns.insert(func);
  }
  return pureDeadReturns;
}

std::unordered_map<Function*, DirectCallUseSummary>
collectDirectCallUseSummary(Module& mod) {
  std::unordered_map<std::string, Function*> nameToFunc;
  std::unordered_map<Function*, DirectCallUseSummary> useSummary;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func)
      continue;
    nameToFunc[func->name()] = func;
    useSummary[func] = {};
  }
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func)
      continue;
    std::unordered_set<Function*> seenCalleesFromCaller;
    for (auto& bbPtr : func->blocks()) {
      for (auto& instPtr : bbPtr->instructions()) {
        Instruction* inst = instPtr.get();
        if (!inst || inst->opcode() != Opcode::Call)
          continue;
        auto it = nameToFunc.find(inst->name());
        if (it == nameToFunc.end())
          continue;
        auto& summary = useSummary[it->second];
        ++summary.callSiteCount;
        if (seenCalleesFromCaller.insert(it->second).second)
          ++summary.callerCount;
      }
    }
  }
  return useSummary;
}

bool tryComputeInlineCost(Function* func,
                          const std::unordered_map<std::string, int>& knownCosts,
                          const std::unordered_map<Function*, DirectCallUseSummary>&
                              callUseSummary,
                          const analysis::FunctionEffectMap& effects,
  analysis::FunctionAnalysisCache& cache,
  int& cost) {
  (void)knownCosts;
  // main 是入口语义边界，inline 成本模型只处理普通可内联函数。
  if (!func || func->name() == "main")
    return false;
  const auto* summary = analysis::lookupFunctionEffect(effects, func);
  bool smallHelper = summary && summary->smallHelper;
  int callSiteCount = 0;
  int callerCount = 0;
  auto callUseIt = callUseSummary.find(func);
  if (callUseIt != callUseSummary.end()) {
    callSiteCount = callUseIt->second.callSiteCount;
    callerCount = callUseIt->second.callerCount;
  }
  bool smallStatefulHelper =
      summary && summary->noRecurse &&
      summary->instructionCount <= 320 &&
      summary->loopCount <= 1 &&
      !summary->readsArgMemory &&
      !summary->readsUnknownMemory &&
      usesOnlyScalarGlobalState(func);
  bool singleCallerProcedure =
      summary && summary->noRecurse &&
      callerCount == 1 &&
      callSiteCount == 1 &&
      summary->instructionCount <= 128 &&
      summary->loopCount == 0 &&
      func->blocks().size() <= 4 &&
      summary->readonly &&
      !summary->readsUnknownMemory;
  // single-caller procedure 只覆盖单调用点、无循环、只读的小
  // wrapper/helper；较大的含循环 arg-memory 过程交给专门 loop-memory 和
  // 后端 RA 处理，避免 late inline 在矩阵类 kernel 上制造 CFG 膨胀。
  bool allowArgStores = summary && summary->argMemOnly && summary->noRecurse;
  // arg-memory 路径处理从参数得到的地址表达式。
  bool smallArgReader = summary && summary->readonly && summary->noRecurse
      && summary->loopCount == 0 && summary->instructionCount <= 40
      && func->blocks().size() <= 6;
  // 额外参数和 getvalue 形成的地址链按通用摘要处理；smallArgReader 只表示
  // 当前分类，不改变地址表达式的处理方式。
  bool smallArgAccessor = summary && summary->noRecurse
      && !summary->readsUnknownMemory
      && summary->loopCount == 0 && summary->instructionCount <= 48
      && func->blocks().size() <= 6
      && summary->loopCount == 0 && summary->instructionCount <= 40
      && func->blocks().size() <= 6;
  bool smallArgReaderOrAccessor = smallArgReader || smallArgAccessor;
  size_t kMaxInlineBlocks = singleCallerProcedure ? 4
                          : smallStatefulHelper ? 48
                          : smallHelper ? 24
                          : smallArgReaderOrAccessor ? 6
                          : 8;
  bool loopShapeAllowed = singleCallerProcedure
                              ? hasFiniteProcedureLoopNest(func, cache)
                              : smallStatefulHelper
                                    ? hasStatefulHelperLoopShape(func, cache)
                                    : smallArgReader
                                    ? true  // smallArgReader/smallArgAccessor：忽略循环形状
                                    : hasAtMostOneSmallLoop(func, cache);
  if (func->blocks().empty() || func->blocks().size() > kMaxInlineBlocks)
    return false;
  if (!loopShapeAllowed)
    return false;
  if (!smallArgReaderOrAccessor &&
      hasLatencyBoundArithmeticLoop(cache))
    return false;

  int retCount = 0;
  Instruction* soleRet = nullptr;
  int kMaxInlineCost = smallHelper ? 256
                       : (smallStatefulHelper ? 512
                       : (singleCallerProcedure ? 192
                       : (smallArgReaderOrAccessor ? (smallArgReader ? 56 : 64) : 24)));
  cost = 0;
  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!bb->terminator())
      return false;
    if (bb->terminator()->opcode() == Opcode::Ret) {
      ++retCount;
      soleRet = bb->terminator();
    }

    // 逐指令计成本并检查内联资格：嵌套调用（modmul 例外）、alloca、
    // 副作用指令与不允许的 store 直接拒绝；其余指令累计成本，
    // 超过该类别的成本上限即拒绝。
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (inst->opcode() == Opcode::Ret)
        continue;
      if (inst->isTerminator()) {
        ++cost;
        if (cost > kMaxInlineCost)
          return false;
        continue;
      }
      if (inst->opcode() == Opcode::Call &&
          inst->name() != "__syc_modmul_i32")
        return false;
      if (inst->opcode() == Opcode::Alloca)
        return false;
      if (inst->opcode() == Opcode::Store) {
        if (!allowArgStores && !singleCallerProcedure && !smallArgReader && !smallArgAccessor) {
          if (!smallStatefulHelper || !isSmallScalarGlobal(inst->operand(1)))
            return false;
        }
        ++cost;
        if (smallStatefulHelper && isSmallScalarGlobal(inst->operand(1)))
          cost += 2;
        if (cost > kMaxInlineCost)
          return false;
        continue;
      }
      if (inst->opcode() == Opcode::Load && smallStatefulHelper &&
          inst->numOperands() == 1 && isSmallScalarGlobal(inst->operand(0))) {
        cost += 2;
        if (cost > kMaxInlineCost)
          return false;
        continue;
      }
      if (inst->hasSideEffects())
        return false;
      ++cost;
      if (cost > kMaxInlineCost)
        return false;
    }
  }
  if (retCount != 1)
    return false;
  if (cost > 0)
    return true;

  // 零成本函数体只包含返回语句。只有在效果分析表明它是非递归纯叶子函数，且
  // 返回值由调用接口中的常量或形参决定时，才将其视为可直接展开的函数。这样
  // 可以避免内联器转发无关的模块值，同时保留常量和恒等 helper 的简化机会。
  if (!summary || !summary->pure || !summary->noRecurse || !soleRet)
    return false;
  if (func->retTy().isVoid())
    return soleRet->numOperands() == 0;
  if (soleRet->numOperands() != 1)
    return false;
  Value* retValue = soleRet->operand(0);
  return retValue &&
         (retValue->isConstant() || argIndexOf(func->args(), retValue) >= 0);
}

bool isTinyWrapper(Function* func,
                   const analysis::FunctionEffectMap& effects) {
  // main 是入口语义边界，不能作为普通 wrapper 被折叠。
  if (!func || func->name() == "main")
    return false;
  const auto* summary = analysis::lookupFunctionEffect(effects, func);
  if (!summary || summary->selfRecursive)
    return false;
  if (func->retTy().isF32() && func->blocks().size() > 1)
    return false;
  for (const auto& arg : func->args()) {
    if (arg.type.isF32() && func->blocks().size() > 1)
      return false;
  }
  if (!func->retTy().isI32() && !func->retTy().isPtr())
    return false;
  if (func->blocks().empty() || func->blocks().size() > 6)
    return false;

  int nonTermInsts = 0;
  int callCount = 0;
  int retCount = 0;
  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!bb || !bb->terminator())
      return false;
    if (bb->terminator()->opcode() == Opcode::Ret)
      ++retCount;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (inst->opcode() == Opcode::Ret)
        continue;
      if (inst->isTerminator())
        continue;
      ++nonTermInsts;
      if (inst->opcode() == Opcode::Alloca || inst->opcode() == Opcode::Store)
        return false;
      if (inst->opcode() == Opcode::Call) {
        if (inst->name() != "__syc_modmul_i32") {
          ++callCount;
          if (callCount > 1)
            return false;
        }
        continue;
      }
      if (inst->hasSideEffects())
        return false;
    }
  }
  if (retCount != 1 || callCount != 1)
    return false;
  return nonTermInsts <= 12;
}

bool isSmallArgMemoryHelper(
    Function* func, Module& mod,
    const analysis::FunctionEffectMap& effects) {
  if (!func || func->name() == "main" || func->blocks().empty() ||
      func->blocks().size() > 6)
    return false;
  if (!func->retTy().isVoid() && !func->retTy().isI32() &&
      !func->retTy().isPtr())
    return false;

  int nonTermInsts = 0;
  bool sawArgMemory = false;
  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!bb || !bb->terminator())
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst || inst->opcode() == Opcode::Ret || inst->isTerminator())
        continue;
      if (++nonTermInsts > 64)
        return false;
      if (inst->opcode() == Opcode::Alloca || inst->opcode() == Opcode::VLoad ||
          inst->opcode() == Opcode::VStore)
        return false;
      if (inst->opcode() == Opcode::Call) {
        const auto* summary =
            analysis::lookupFunctionEffect(effects, mod, inst->name());
        if (!summary || !summary->pure || summary->selfRecursive)
          return false;
        continue;
      }
      if (inst->opcode() == Opcode::Load || inst->opcode() == Opcode::Store) {
        const int addressIndex = inst->opcode() == Opcode::Load ? 0 : 1;
        if (inst->numOperands() <= addressIndex)
          return false;
        Value* root = memoryRoot(inst->operand(addressIndex));
        if (argIndexOf(func->args(), root) < 0)
          return false;
        sawArgMemory = true;
        continue;
      }
      if (inst->hasSideEffects())
        return false;
    }
  }
  return nonTermInsts > 0 && sawArgMemory;
}

Value* mappedInlineValue(Value* val, const std::unordered_map<Value*, Value*>& valueMap) {
  auto it = valueMap.find(val);
  return it == valueMap.end() ? val : it->second;
}

Instruction* cloneInlineInstructionShell(Instruction* src) {
  auto* clone = new Instruction(src->opcode(), src->type(), src->name());
  clone->setMemoryInfo(src->memoryElemTy(), src->memorySizeBytes(),
                       src->isArrayObject(), src->dims());
  return clone;
}

Value* mappedInlineOperand(
    Value* val,
    const std::unordered_map<Value*, Value*>& valueMap,
    const std::unordered_map<BasicBlock*, BasicBlock*>& blockMap) {
  if (auto* bb = dynamic_cast<BasicBlock*>(val)) {
    auto it = blockMap.find(bb);
    if (it != blockMap.end())
      return it->second;
  }
  return mappedInlineValue(val, valueMap);
}

Instruction* findInlineReturn(Function* callee, BasicBlock*& retBB) {
  retBB = nullptr;
  Instruction* ret = nullptr;
  if (!callee)
    return nullptr;
  for (auto& bbPtr : callee->blocks()) {
    Instruction* term = bbPtr->terminator();
    if (!term || term->opcode() != Opcode::Ret)
      continue;
    if (ret)
      return nullptr;
    ret = term;
    retBB = bbPtr.get();
  }
  return ret;
}

bool canonicalizeReturnsForInlining(Function* func, int maxReturns) {
  if (!func || maxReturns < 2)
    return false;
  std::vector<std::pair<BasicBlock*, Instruction*>> returns;
  for (auto& bbPtr : func->blocks()) {
    Instruction* term = bbPtr->terminator();
    if (term && term->opcode() == Opcode::Ret)
      returns.push_back({bbPtr.get(), term});
  }
  if (returns.size() <= 1)
    return false;
  if (returns.size() > static_cast<size_t>(maxReturns))
    return false;

  const bool hasValue = !func->retTy().isVoid();
  if (hasValue) {
    for (const auto& entry : returns) {
      if (!entry.second || entry.second->numOperands() != 1)
        return false;
    }
  } else {
    for (const auto& entry : returns) {
      if (!entry.second || entry.second->numOperands() != 0)
        return false;
    }
  }

  BasicBlock* exitBB = func->newBlock("inline.ret.exit");
  Instruction* phi = nullptr;
  if (hasValue) {
    phi = new Instruction(Opcode::Phi, func->retTy(), "inline.ret.value");
    for (const auto& entry : returns) {
      phi->addOperand(entry.second->operand(0));
      phi->addOperand(entry.first);
    }
    exitBB->pushBack(phi);
  }
  auto* ret = new Instruction(Opcode::Ret, Type::voidTy());
  if (hasValue)
    ret->addOperand(phi);
  exitBB->pushBack(ret);

  for (const auto& entry : returns) {
    auto* br = new Instruction(Opcode::Br, Type::voidTy());
    br->addOperand(exitBB);
    entry.first->remove(entry.second);
    entry.first->pushBack(br);
  }
  func->recomputeCFG();
  return true;
}

bool inlineCallIntoBlock(
    BasicBlock* callerBB, Instruction* call, Function* callee,
    const std::vector<BasicBlock*>* templateBlocks = nullptr,
    BasicBlock* templateEntry = nullptr, BasicBlock* templateRetBB = nullptr,
    Instruction* templateRet = nullptr) {
  if (!callerBB || !call || !callee)
    return false;
  if (!templateBlocks) {
    for (const auto& _b : callee->blocks()) {
      const std::string& name = _b->name();
      if (name.rfind("inline_", 0) == 0)
        return false;
    }
  }
  BasicBlock* retBB = nullptr;
  Instruction* ret = templateRet;
  if (templateBlocks) {
    retBB = templateRetBB;
    if (!ret || !retBB)
      return false;
  } else {
    ret = findInlineReturn(callee, retBB);
  }
  if (!ret || !retBB)
    return false;
  if (!call->type().isVoid() && ret->numOperands() != 1)
    return false;
  if (call->type().isVoid() && ret->numOperands() != 0)
    return false;
  if (call->numOperands() != static_cast<int>(callee->args().size()))
    return false;
  Function* callerFunc = callerBB->parent();
  if (!callerFunc)
    return false;

  auto& callerInsts = callerBB->instructions();
  auto callIt = std::find_if(callerInsts.begin(), callerInsts.end(),
                             [call](const auto& instPtr) { return instPtr.get() == call; });
  if (callIt == callerInsts.end())
    return false;
  auto moveBegin = std::next(callIt);
  if (moveBegin == callerInsts.end())
    return false;

  // 递归内联会交替使用 caller 和 callee。拆分 callerBB 之前先保存源指令列表，
  // 否则包含调用的源基本块可能丢失末尾终结指令。
  std::unordered_map<BasicBlock*, std::vector<Instruction*>> templateInsts;
  if (templateBlocks) {
    for (BasicBlock* sourceBB : *templateBlocks) {
      auto& insts = templateInsts[sourceBB];
      for (auto& instPtr : sourceBB->instructions())
        insts.push_back(instPtr.get());
    }
  }

  auto* afterBB = callerFunc->newBlock("inline_after");
  std::vector<std::unique_ptr<Instruction>> tailInsts;
  for (auto it = moveBegin; it != callerInsts.end(); ++it) {
    (*it)->setParent(afterBB);
    tailInsts.push_back(std::move(*it));
  }
  callerInsts.erase(moveBegin, callerInsts.end());
  for (auto& instPtr : tailInsts)
    afterBB->instructions().push_back(std::move(instPtr));

  if (Instruction* term = afterBB->terminator()) {
    for (int opIndex = 0; opIndex < term->numOperands(); ++opIndex) {
      auto* succ = dynamic_cast<BasicBlock*>(term->operand(opIndex));
      if (succ)
        replacePhiIncomingPred(succ, callerBB, afterBB);
    }
  }

  std::unordered_map<Value*, Value*> valueMap;
  const auto& args = callee->args();
  for (int i = 0; i < call->numOperands(); ++i) {
    if (!args[i].val)
      return false;
    valueMap[args[i].val] = call->operand(i);
  }

  const std::vector<BasicBlock*> defaultBlocks = [&]() {
    std::vector<BasicBlock*> blocks;
    blocks.reserve(callee->blocks().size());
    for (auto& bbPtr : callee->blocks())
      blocks.push_back(bbPtr.get());
    return blocks;
  }();
  const std::vector<BasicBlock*>& sourceBlocks =
      templateBlocks ? *templateBlocks : defaultBlocks;
  std::unordered_map<BasicBlock*, BasicBlock*> blockMap;
  for (BasicBlock* srcBB : sourceBlocks)
    blockMap[srcBB] = callerFunc->newBlock("inline_" + callee->name());

  for (BasicBlock* srcBB : sourceBlocks) {
    BasicBlock* dstBB = blockMap[srcBB];
    const auto sourceInstructions = templateBlocks
                                        ? templateInsts.at(srcBB)
                                        : std::vector<Instruction*>{};
    if (templateBlocks) {
      for (Instruction* src : sourceInstructions) {
        if (src == ret)
          continue;
        auto* clone = cloneInlineInstructionShell(src);
        dstBB->pushBack(clone);
        valueMap[src] = clone;
      }
      continue;
    }
    for (auto& instPtr : srcBB->instructions()) {
      Instruction* src = instPtr.get();
      if (src == ret)
        continue;
      auto* clone = cloneInlineInstructionShell(src);
      dstBB->pushBack(clone);
      valueMap[src] = clone;
    }
  }

  for (BasicBlock* srcBB : sourceBlocks) {
    BasicBlock* dstBB = blockMap[srcBB];
    size_t dstIndex = 0;
    const auto sourceInstructions = templateBlocks
                                        ? templateInsts.at(srcBB)
                                        : std::vector<Instruction*>{};
    auto addOperands = [&](Instruction* src) {
      if (src == ret)
        return;
      Instruction* clone = dstBB->instructions()[dstIndex++].get();
      for (auto* op : src->operands())
        clone->addOperand(mappedInlineOperand(op, valueMap, blockMap));
    };
    if (templateBlocks) {
      for (Instruction* src : sourceInstructions)
        addOperands(src);
      continue;
    }
    for (auto& instPtr : srcBB->instructions())
      addOperands(instPtr.get());
  }

  auto* entryBr = new Instruction(Opcode::Br, Type::voidTy());
  BasicBlock* sourceEntry = templateBlocks ? templateEntry : callee->entryBlock();
  if (!sourceEntry || !blockMap.count(sourceEntry))
    return false;
  entryBr->addOperand(blockMap.at(sourceEntry));
  callerBB->pushBack(entryBr);

  auto* exitBr = new Instruction(Opcode::Br, Type::voidTy());
  exitBr->addOperand(afterBB);
  blockMap.at(retBB)->pushBack(exitBr);

  if (!call->type().isVoid()) {
    Value* retVal = mappedInlineValue(ret->operand(0), valueMap);
    call->replaceAllUsesWith(retVal);
  }
  callerBB->remove(call);
  callerFunc->recomputeCFG();
  return true;
}

bool blockLeadsOnlyToVoidReturn(BasicBlock* bb,
                                std::unordered_set<BasicBlock*>& visited) {
  if (!bb || !visited.insert(bb).second)
    return false;
  for (auto& instPtr : bb->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst)
      return false;
    if (inst->opcode() == Opcode::Ret)
      return inst->numOperands() == 0;
    if (inst->opcode() == Opcode::Br)
      continue;
    if (inst->isTerminator() || inst->hasSideEffects())
      return false;
  }
  Instruction* term = bb->terminator();
  if (!term || term->opcode() != Opcode::Br || term->numOperands() != 1)
    return false;
  auto* next = dynamic_cast<BasicBlock*>(term->operand(0));
  return blockLeadsOnlyToVoidReturn(next, visited);
}

bool matchVoidEarlyReturnGuard(Function* callee,
                               Value*& guard,
                               bool& trueReturns) {
  guard = nullptr;
  trueReturns = false;
  if (!callee || !callee->retTy().isVoid() || !callee->entryBlock())
    return false;
  Instruction* term = callee->entryBlock()->terminator();
  if (!term || term->opcode() != Opcode::CondBr || term->numOperands() != 3)
    return false;
  auto* trueBB = dynamic_cast<BasicBlock*>(term->operand(1));
  auto* falseBB = dynamic_cast<BasicBlock*>(term->operand(2));
  std::unordered_set<BasicBlock*> trueVisited;
  std::unordered_set<BasicBlock*> falseVisited;
  bool trueIsReturn = blockLeadsOnlyToVoidReturn(trueBB, trueVisited);
  bool falseIsReturn = blockLeadsOnlyToVoidReturn(falseBB, falseVisited);
  if (trueIsReturn == falseIsReturn)
    return false;
  guard = term->operand(0);
  trueReturns = trueIsReturn;
  return guard != nullptr;
}

// 在调用点前递归克隆守卫值的定义链：实参/常量/全局直接映射；
// 仅克隆纯算术、比较与移位指令，其余指令类型立即失败。
bool cloneGuardValueBeforeCall(Value* value,
                               BasicBlock* bb,
                               Instruction* call,
                               const std::unordered_map<Value*, Value*>& argMap,
                               std::unordered_map<Value*, Value*>& cloned,
                               Value*& out) {
  if (!value || !bb || !call)
    return false;
  auto argIt = argMap.find(value);
  if (argIt != argMap.end()) {
    out = argIt->second;
    return true;
  }
  if (value->isConstant() || dynamic_cast<GlobalVar*>(value)) {
    out = value;
    return true;
  }
  auto clonedIt = cloned.find(value);
  if (clonedIt != cloned.end()) {
    out = clonedIt->second;
    return true;
  }
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->type().isVoid())
    return false;
  switch (inst->opcode()) {
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Mul:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
  case Opcode::Shl:
  case Opcode::Shr:
  case Opcode::ICmp:
    break;
  default:
    return false;
  }
  auto* clone = new Instruction(inst->opcode(), inst->type(), inst->name());
  for (auto* operand : inst->operands()) {
    Value* mapped = nullptr;
    if (!cloneGuardValueBeforeCall(operand, bb, call, argMap, cloned, mapped))
      return false;
    clone->addOperand(mapped);
  }
  bb->insertBefore(call, clone);
  cloned[value] = clone;
  out = clone;
  return true;
}

bool guardVoidCallWithCalleeEarlyReturn(BasicBlock* callerBB,
                                        Instruction* call,
                                        Function* callee) {
  if (!callerBB || !call || !callee || !call->type().isVoid() ||
      call->numOperands() != static_cast<int>(callee->args().size())) {
    return false;
  }
  Value* calleeGuard = nullptr;
  bool trueReturns = false;
  if (!matchVoidEarlyReturnGuard(callee, calleeGuard, trueReturns))
    return false;

  std::unordered_map<Value*, Value*> argMap;
  for (int i = 0; i < call->numOperands(); ++i) {
    if (!callee->args()[i].val)
      return false;
    argMap[callee->args()[i].val] = call->operand(i);
  }
  std::unordered_map<Value*, Value*> cloned;
  Value* guard = nullptr;
  if (!cloneGuardValueBeforeCall(calleeGuard, callerBB, call, argMap, cloned,
                                 guard)) {
    return false;
  }

  Function* callerFunc = callerBB->parent();
  if (!callerFunc)
    return false;
  auto& insts = callerBB->instructions();
  auto callIt = std::find_if(insts.begin(), insts.end(),
                             [call](const auto& ptr) { return ptr.get() == call; });
  if (callIt == insts.end())
    return false;
  auto moveBegin = std::next(callIt);
  if (moveBegin == insts.end())
    return false;

  BasicBlock* callBB = callerFunc->newBlock("call_guard.call");
  BasicBlock* afterBB = callerFunc->newBlock("call_guard.after");
  (*callIt)->setParent(callBB);
  callBB->instructions().push_back(std::move(*callIt));
  std::vector<std::unique_ptr<Instruction>> tailInsts;
  for (auto it = moveBegin; it != insts.end(); ++it) {
    (*it)->setParent(afterBB);
    tailInsts.push_back(std::move(*it));
  }
  insts.erase(callIt, insts.end());
  for (auto& instPtr : tailInsts)
    afterBB->instructions().push_back(std::move(instPtr));

  if (Instruction* term = afterBB->terminator()) {
    for (int opIndex = 0; opIndex < term->numOperands(); ++opIndex) {
      auto* succ = dynamic_cast<BasicBlock*>(term->operand(opIndex));
      if (succ)
        replacePhiIncomingPred(succ, callerBB, afterBB);
    }
  }

  auto* branch = new Instruction(Opcode::CondBr, Type::voidTy());
  branch->addOperand(guard);
  branch->addOperand(trueReturns ? afterBB : callBB);
  branch->addOperand(trueReturns ? callBB : afterBB);
  callerBB->pushBack(branch);
  auto* callExit = new Instruction(Opcode::Br, Type::voidTy());
  callExit->addOperand(afterBB);
  callBB->pushBack(callExit);
  callerFunc->recomputeCFG();
  return true;
}

bool applyVoidEarlyReturnCallGuards(Module& mod,
                                    analysis::AnalysisManager& analysisManager) {
  std::unordered_map<std::string, Function*> nameToFunc;
  for (auto& funcPtr : mod.functions())
    if (funcPtr)
      nameToFunc[funcPtr->name()] = funcPtr.get();

  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func)
      continue;
    std::vector<Instruction*> calls;
    for (auto& bbPtr : func->blocks()) {
      for (auto& instPtr : bbPtr->instructions()) {
        Instruction* inst = instPtr.get();
        if (inst && inst->opcode() == Opcode::Call && inst->type().isVoid())
          calls.push_back(inst);
      }
    }
    for (auto* call : calls) {
      if (!call || !call->parent())
        continue;
      auto it = nameToFunc.find(call->name());
      if (it == nameToFunc.end())
        continue;
      if (guardVoidCallWithCalleeEarlyReturn(call->parent(), call, it->second)) {
        changed = true;
        analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
        break;
      }
    }
  }
  return changed;
}

bool isScalarRecursiveInlineType(const Type& type) {
  return type.isI32() || type.isF32();
}

std::unique_ptr<Function> makeSelfRecursiveInlineTemplate(Function* source,
                                                           Module& mod) {
  if (!source)
    return nullptr;
  auto templateFunc =
      std::make_unique<Function>(source->name(), source->retTy(), &mod);
  std::unordered_map<Value*, Value*> valueMap;
  for (const auto& arg : source->args()) {
    Value* clonedArg = templateFunc->addArg(arg.name, arg.type);
    valueMap[arg.val] = clonedArg;
  }

  std::unordered_map<BasicBlock*, BasicBlock*> blockMap;
  for (const auto& bbPtr : source->blocks()) {
    BasicBlock* clonedBB = templateFunc->newBlock("template");
    blockMap[bbPtr.get()] = clonedBB;
    if (bbPtr.get() == source->entryBlock())
      templateFunc->setEntryBlock(clonedBB);
  }

  for (const auto& bbPtr : source->blocks()) {
    BasicBlock* sourceBB = bbPtr.get();
    BasicBlock* clonedBB = blockMap.at(sourceBB);
    for (const auto& instPtr : sourceBB->instructions()) {
      Instruction* clone = cloneInlineInstructionShell(instPtr.get());
      clonedBB->pushBack(clone);
      valueMap[instPtr.get()] = clone;
    }
  }

  for (const auto& bbPtr : source->blocks()) {
    BasicBlock* sourceBB = bbPtr.get();
    BasicBlock* clonedBB = blockMap.at(sourceBB);
    size_t index = 0;
    for (const auto& instPtr : sourceBB->instructions()) {
      Instruction* clone = clonedBB->instructions()[index++].get();
      for (Value* operand : instPtr->operands())
        clone->addOperand(mappedInlineOperand(operand, valueMap, blockMap));
    }
  }
  templateFunc->recomputeCFG();
  return templateFunc;
}

bool isBoundedSelfRecursiveInlineCandidate(
    Function* func, const analysis::FunctionEffectSummary* summary) {
  if (!func || !summary || !summary->selfRecursive || !summary->pure ||
      summary->readsUnknownMemory || summary->writesUnknownMemory ||
      summary->writesArgMemory || summary->writesMutableGlobalMemory ||
      summary->instructionCount <= 0 || summary->instructionCount > 96 ||
      summary->selfRecursiveCallCount <= 0 || func->blocks().empty() ||
      func->blocks().size() > 12 || !isScalarRecursiveInlineType(func->retTy()))
    return false;

  for (const auto& arg : func->args()) {
    if (!isScalarRecursiveInlineType(arg.type))
      return false;
  }

  // 指令白名单检查：只允许纯标量算术/转换/控制流指令；
  // SDiv/SRem 拒绝；Call 仅允许自递归调用。
  for (const auto& bbPtr : func->blocks()) {
    for (const auto& instPtr : bbPtr->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      switch (inst->opcode()) {
      case Opcode::Add:
      case Opcode::Sub:
      case Opcode::Mul:
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
      case Opcode::Neg:
      case Opcode::FNeg:
      case Opcode::Phi:
      case Opcode::Select:
      case Opcode::Br:
      case Opcode::CondBr:
      case Opcode::Ret:
        break;
      case Opcode::SDiv:
      case Opcode::SRem:
        return false;
      case Opcode::Call:
        if (inst->name() != func->name())
          return false;
        break;
      default:
        return false;
      }
    }
  }
  return true;
}

// GCC -O2 会对紧凑的标量递推执行有限层数的递归内联。保留边界调用，同时
// 暴露少量递推层次给常量折叠和寄存器分配。模板基本块列表保持固定，因为
// caller 和 callee 是同一个函数；复制不断增长的函数会让每次展开重复复制
// 之前的所有展开结果。
bool inlineBoundedSelfRecursiveFunctions(
    Module& mod, analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  const analysis::FunctionEffectMap& effects =
      analysisManager.functionEffects();
  constexpr int kMaxExpansionDepth = 4;

  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    const auto* summary =
        analysis::lookupFunctionEffect(effects, func);
    if (!isBoundedSelfRecursiveInlineCandidate(func, summary))
      continue;

    // inlineCallIntoBlock 要求只有一个返回块。这里先做局部 CFG 整理，只用于
    // 小型标量候选函数。
    canonicalizeReturnsForInlining(func, /*maxReturns=*/4);
    auto templateFunc = makeSelfRecursiveInlineTemplate(func, mod);
    BasicBlock* templateRetBB = nullptr;
    Instruction* templateRet = findInlineReturn(templateFunc.get(), templateRetBB);
    if (!templateFunc || !templateRet || !templateRetBB)
      continue;
    std::unordered_map<BasicBlock*, int> blockDepth;
    std::unordered_set<BasicBlock*> knownBlocks;
    for (auto& bbPtr : func->blocks()) {
      BasicBlock* bb = bbPtr.get();
      blockDepth[bb] = 0;
      knownBlocks.insert(bb);
    }

    while (true) {
      bool expanded = false;
      std::vector<BasicBlock*> blocks;
      blocks.reserve(func->blocks().size());
      for (auto& bbPtr : func->blocks())
        blocks.push_back(bbPtr.get());

      for (BasicBlock* callerBB : blocks) {
        auto depthIt = blockDepth.find(callerBB);
        const int depth = depthIt == blockDepth.end() ? 0 : depthIt->second;
        if (depth >= kMaxExpansionDepth)
          continue;

        std::vector<Instruction*> calls;
        for (const auto& instPtr : callerBB->instructions()) {
          Instruction* inst = instPtr.get();
          if (inst && inst->opcode() == Opcode::Call &&
              inst->name() == func->name())
            calls.push_back(inst);
        }
        for (Instruction* call : calls) {
          if (!call || call->parent() != callerBB)
            continue;
          if (!inlineCallIntoBlock(callerBB, call, templateFunc.get()))
            continue;

          // 新建的 after 基本块保持在 caller 深度；复制的模板基本块表示更深一层
          // 的递归层次。
          for (auto& bbPtr : func->blocks()) {
            BasicBlock* bb = bbPtr.get();
            if (!knownBlocks.insert(bb).second)
              continue;
            const bool isAfter =
                bb->name().find(".inline_after") != std::string::npos;
            blockDepth[bb] = isAfter ? depth : depth + 1;
          }
          changed = true;
          expanded = true;
          break;
        }
        if (expanded)
          break;
      }
      if (!expanded)
        break;
    }
  }

  if (changed)
    analysisManager.invalidateModule(analysis::AnalysisInvalidation::Effects);
  return changed;
}

bool inlineSmallFunctions(Module& mod,
                          analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  if (inlineBoundedSelfRecursiveFunctions(mod, analysisManager))
    changed = true;
  int budget = 4096;
  while (budget > 0) {
    if (deadPureCallElimination(mod, analysisManager)) {
      changed = true;
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::Effects);
    }
    const analysis::FunctionEffectMap& effects =
        analysisManager.functionEffects();
    const std::unordered_set<Function*> deadReturnPureFunctions =
        collectDeadReturnChainFunctions(mod, effects);
    std::unordered_map<Function*, DirectCallUseSummary> callUseSummary =
        collectDirectCallUseSummary(mod);
    std::unordered_map<std::string, int> inlineCosts;
    bool costChanged = true;
    while (costChanged) {
      costChanged = false;
      for (auto& funcPtr : mod.functions()) {
        Function* func = funcPtr.get();
        int cost = 0;
        analysis::FunctionAnalysisCache& cache =
            analysisManager.function(func);
        if (!tryComputeInlineCost(func, inlineCosts, callUseSummary, effects,
                                  cache, cost))
          continue;
        auto it = inlineCosts.find(func->name());
        if (it == inlineCosts.end() || it->second != cost) {
          inlineCosts[func->name()] = cost;
          costChanged = true;
        }
      }
    }

    std::unordered_map<std::string, Function*> inlineable;
    for (auto& funcPtr : mod.functions()) {
      Function* func = funcPtr.get();
      if (inlineCosts.count(func->name()) &&
          !hasLatencyBoundArithmeticLoop(analysisManager.function(func)))
        inlineable[func->name()] = func;
    }
    for (auto& funcPtr : mod.functions()) {
      Function* func = funcPtr.get();
      if (isTrivialSelectHelper(func))
        inlineable[func->name()] = func;
      else if (isTinyWrapper(func, effects))
        inlineable[func->name()] = func;
      else if (isSmallArgMemoryHelper(func, mod, effects))
        inlineable[func->name()] = func;
    }
    for (Function* func : deadReturnPureFunctions)
      inlineable.erase(func->name());
    if (inlineable.empty())
      break;

    bool roundChanged = false;
    bool needsEffectInvalidation = false;
    for (auto& funcPtr : mod.functions()) {
      Function* func = funcPtr.get();
      std::vector<BasicBlock*> blocks;
      for (auto& bbPtr : func->blocks())
        blocks.push_back(bbPtr.get());
      for (auto* bb : blocks) {
        std::vector<Instruction*> calls;
        for (auto& instPtr : bb->instructions()) {
          Instruction* inst = instPtr.get();
          if (inst->opcode() == Opcode::Call)
            calls.push_back(inst);
        }
        for (auto* call : calls) {
          if (budget <= 0 || call->parent() != bb)
            continue;
          auto it = inlineable.find(call->name());
          if (it == inlineable.end() || it->second == func)
            continue;
          // 避免重复内联：跳过已经包含 inline_ 前缀基本块的 callee，这些基本块
          // 来自之前的内联轮次，可以避免浮点常量参数被重复处理。
          {
            bool calleeHasInlineBlocks = false;
            for (const auto& cb : it->second->blocks()) {
              if ((int)cb->name().find("inline_") == 0) {
                calleeHasInlineBlocks = true;
                break;
              }
            }
            if (calleeHasInlineBlocks)
              continue;
          }

          // 跳过带浮点参数且已经由递归变换得到的函数，因为这类函数包含复杂控制
          // 流，继续内联会改变浮点计算过程。
          {
            bool calleeHasFloatAndComplex = false;
            if (it->second->blocks().size() > 2) {
              if (it->second->retTy().isF32())
                calleeHasFloatAndComplex = true;
              else {
                for (const auto& arg : it->second->args()) {
                  if (arg.type.isF32()) { calleeHasFloatAndComplex = true; break; }
                }
              }
            }
            if (calleeHasFloatAndComplex)
              continue;
          }

            // 本轮前一次内联可能在 callee 中暴露延迟敏感的算术循环。使用已有的
            // 内联成本记录前，先刷新它的 CFG 视图。
          analysisManager.invalidateFunction(
              it->second, analysis::AnalysisInvalidation::CFG);
          if (hasLatencyBoundArithmeticLoop(
                  analysisManager.function(it->second)))
            continue;

          if (inlineCallIntoBlock(bb, call, it->second)) {
            --budget;
            roundChanged = true;
            changed = true;
            // 本轮 inline 决策已经基于轮首 effect 摘要固定，
            // 成功内联后不需要每个 call 立刻清空全模块缓存；统一延迟到
            // 本轮结束再失效，下一轮再重算即可，减少 AnalysisManager 抖动。
            needsEffectInvalidation = true;
            for (const auto& _cb : func->blocks()) {
              if ((int)_cb->name().find("inline_") == 0) {
                inlineable.erase(func->name());
                break;
              }
            }
          }
        }
      }
    }
    if (needsEffectInvalidation)
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::Effects);
    if (!roundChanged)
      break;
  }
  return changed;
}

bool hasSingleInlineReturn(Function* func) {
  BasicBlock* retBB = nullptr;
  return findInlineReturn(func, retBB) != nullptr && retBB != nullptr;
}

bool hasOnlyProcedureInlineSafeOps(Function* func) {
  if (!func || !hasSingleInlineReturn(func))
    return false;
  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!bb || !bb->terminator())
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      if (inst->opcode() == Opcode::Call || inst->opcode() == Opcode::Alloca)
        return false;
    }
  }
  return true;
}

bool callsOnlyBuiltins(Function* func, Module& mod) {
  if (!func)
    return false;
  bool sawCall = false;
  for (auto& bbPtr : func->blocks()) {
    for (auto& instPtr : bbPtr->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst || inst->opcode() != Opcode::Call)
        continue;
      sawCall = true;
      if (!mod.isBuiltin(inst->name()))
        return false;
    }
  }
  return sawCall;
}

bool hasOnlyBuiltinInlineSafeOps(Function* func, Module& mod) {
  if (!func || !hasSingleInlineReturn(func))
    return false;
  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!bb || !bb->terminator())
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      if (inst->opcode() == Opcode::Alloca)
        return false;
      if (inst->opcode() == Opcode::Call && !mod.isBuiltin(inst->name()))
        return false;
    }
  }
  return true;
}

bool isLocalMemoryRoot(Value* root) {
  auto* inst = dynamic_cast<Instruction*>(root);
  return inst && inst->opcode() == Opcode::Alloca;
}

bool procedureMemoryIsInlineLocal(Value* addr, bool isStore) {
  Value* root = memoryRoot(addr);
  if (!root)
    return false;
  if (dynamic_cast<Argument*>(root) || isLocalMemoryRoot(root))
    return true;
  auto* gv = dynamic_cast<GlobalVar*>(root);
  if (!gv)
    return false;
  // 在这里的过程间决策中，标量全局读可以视作额外的只读参数。全局写入仍然
  // 保持拒绝，以免把有状态 helper 折叠进大型循环过程。
  return !isStore && gv->sizeBytes() == 4 && gv->dims().empty();
}

bool isDirectLoadFromGlobal(Value* value, GlobalVar* gv) {
  auto* inst = dynamic_cast<Instruction*>(value);
  return inst && inst->opcode() == Opcode::Load && inst->numOperands() == 1 &&
         dynamic_cast<GlobalVar*>(memoryRoot(inst->operand(0))) == gv;
}

bool isIdempotentScalarGlobalStore(Instruction* store) {
  if (!store || (store->opcode() != Opcode::Store &&
                 store->opcode() != Opcode::VStore) ||
      store->numOperands() != 2) {
    return false;
  }
  auto* gv = dynamic_cast<GlobalVar*>(memoryRoot(store->operand(1)));
  if (!gv || gv->sizeBytes() != 4 || !gv->dims().empty())
    return false;
  return isDirectLoadFromGlobal(store->operand(0), gv);
}

bool hasProcedureInlineMemoryShape(Function* func) {
  if (!func)
    return false;
  for (auto& bbPtr : func->blocks()) {
    for (auto& instPtr : bbPtr->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      if (inst->opcode() == Opcode::Load || inst->opcode() == Opcode::VLoad) {
        if (inst->numOperands() != 1 ||
            !procedureMemoryIsInlineLocal(inst->operand(0), false))
          return false;
      } else if (inst->opcode() == Opcode::Store ||
                 inst->opcode() == Opcode::VStore) {
        if (isIdempotentScalarGlobalStore(inst))
          continue;
        if (inst->numOperands() != 2 ||
            !procedureMemoryIsInlineLocal(inst->operand(1), true))
          return false;
      }
    }
  }
  return true;
}

bool hasAggregateGlobalMemoryAccess(Function* func) {
  if (!func)
    return false;
  for (auto& bbPtr : func->blocks()) {
    for (auto& instPtr : bbPtr->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      Value* addr = nullptr;
      if ((inst->opcode() == Opcode::Load || inst->opcode() == Opcode::VLoad) &&
          inst->numOperands() == 1) {
        addr = inst->operand(0);
      } else if ((inst->opcode() == Opcode::Store ||
                  inst->opcode() == Opcode::VStore) &&
                 inst->numOperands() == 2) {
        addr = inst->operand(1);
      }
      auto* gv = dynamic_cast<GlobalVar*>(memoryRoot(addr));
      if (gv && (gv->sizeBytes() > 4 || !gv->dims().empty()))
        return true;
    }
  }
  return false;
}

bool hasOnlyTinyPureInlineOps(Function* func) {
  if (!func || !hasSingleInlineReturn(func))
    return false;
  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!bb || !bb->terminator())
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      switch (inst->opcode()) {
      case Opcode::Call:
      case Opcode::Alloca:
      case Opcode::Store:
      case Opcode::VStore:
        return false;
      default:
        break;
      }
    }
  }
  return true;
}

bool inlineTinyPureHelpers(Module& mod,
                           analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  int budget = 512;
  while (budget-- > 0) {
    const analysis::FunctionEffectMap& effects =
        analysisManager.functionEffects();
    const std::unordered_set<Function*> deadReturnPureFunctions =
        collectDeadReturnChainFunctions(mod, effects);
    std::unordered_map<Function*, DirectCallUseSummary> callUseSummary =
        collectDirectCallUseSummary(mod);
    std::unordered_map<std::string, Function*> inlineable;
    for (auto& funcPtr : mod.functions()) {
      Function* callee = funcPtr.get();
      if (!callee || callee->name() == "main")
        continue;
      auto useIt = callUseSummary.find(callee);
      if (useIt == callUseSummary.end() || useIt->second.callSiteCount <= 0 ||
          useIt->second.callSiteCount > 64)
        continue;
      const auto* summary = analysis::lookupFunctionEffect(effects, callee);
      bool tinyReadonlyScalarHelper =
          summary && summary->readonly && usesOnlyScalarGlobalState(callee);
      if (!summary || (!summary->pure && !tinyReadonlyScalarHelper) ||
          !summary->noRecurse ||
          summary->loopCount != 0 || summary->instructionCount <= 0 ||
          summary->instructionCount > 32 || callee->blocks().size() > 4) {
        continue;
      }
      if (!hasOnlyTinyPureInlineOps(callee))
        continue;
      if (deadReturnPureFunctions.count(callee))
        continue;
      inlineable[callee->name()] = callee;
    }
    if (inlineable.empty())
      break;

    // 每轮只内联一个调用点就退出重算摘要（内联可能改变函数形状），
    // 直到没有可内联目标为止。
    bool roundChanged = false;
    for (auto& funcPtr : mod.functions()) {
      Function* caller = funcPtr.get();
      if (!caller)
        continue;
      std::vector<BasicBlock*> blocks;
      for (auto& bbPtr : caller->blocks())
        blocks.push_back(bbPtr.get());
      for (auto* bb : blocks) {
        if (!bb)
          continue;
        std::vector<Instruction*> calls;
        for (auto& instPtr : bb->instructions()) {
          Instruction* inst = instPtr.get();
          if (inst && inst->opcode() == Opcode::Call)
            calls.push_back(inst);
        }
        for (auto* call : calls) {
          if (!call || call->parent() != bb)
            continue;
          auto it = inlineable.find(call->name());
          if (it == inlineable.end() || it->second == caller)
            continue;
          if (inlineCallIntoBlock(bb, call, it->second)) {
            changed = true;
            roundChanged = true;
            analysisManager.invalidateModule(
                analysis::AnalysisInvalidation::CFG);
            analysisManager.invalidateModule(
                analysis::AnalysisInvalidation::Effects);
            break;
          }
        }
        if (roundChanged)
          break;
      }
      if (roundChanged)
        break;
    }
    if (!roundChanged)
      break;
  }
  return changed;
}

int loopDepthForInstruction(Function* func,
                            Instruction* inst,
                            analysis::AnalysisManager& analysisManager) {
  if (!func || !inst || !inst->parent())
    return 0;
  analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
  int depth = 0;
  for (const auto& loop : cache.loops()) {
    if (loop.blocks.count(inst->parent()))
      ++depth;
  }
  return depth;
}

bool hasLoopCallSite(Function* caller,
                     const std::string& calleeName,
                     analysis::AnalysisManager& analysisManager) {
  if (!caller)
    return false;
  for (auto& bbPtr : caller->blocks()) {
    for (auto& instPtr : bbPtr->instructions()) {
      Instruction* inst = instPtr.get();
      if (inst && inst->opcode() == Opcode::Call &&
          inst->name() == calleeName &&
          loopDepthForInstruction(caller, inst, analysisManager) > 0) {
        return true;
      }
    }
  }
  return false;
}

bool allDirectCallSitesAreLoopHot(Module& mod,
                                  const std::string& calleeName,
                                  analysis::AnalysisManager& analysisManager) {
  bool sawCallSite = false;
  for (auto& funcPtr : mod.functions()) {
    Function* caller = funcPtr.get();
    if (!caller)
      continue;
    for (auto& bbPtr : caller->blocks()) {
      for (auto& instPtr : bbPtr->instructions()) {
        Instruction* inst = instPtr.get();
        if (!inst || inst->opcode() != Opcode::Call || inst->name() != calleeName)
          continue;
        sawCallSite = true;
        if (loopDepthForInstruction(caller, inst, analysisManager) <= 0)
          return false;
      }
    }
  }
  return sawCallSite;
}

Function* findSingleDirectCaller(Module& mod, const std::string& calleeName) {
  Function* caller = nullptr;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func)
      continue;
    bool sawInFunc = false;
    for (auto& bbPtr : func->blocks()) {
      for (auto& instPtr : bbPtr->instructions()) {
        Instruction* inst = instPtr.get();
        if (inst && inst->opcode() == Opcode::Call &&
            inst->name() == calleeName) {
          sawInFunc = true;
        }
      }
    }
    if (!sawInFunc)
      continue;
    if (caller && caller != func)
      return nullptr;
    caller = func;
  }
  return caller;
}

bool procedureInlineTraceEnabled() {
  return sycEnvironmentValue("SYC_TRACE_COREIR_PROCEDURE_INLINE") != nullptr;
}

void traceProcedureInlineDecision(const char* stage,
                                  Function* callee,
                                  const char* reason,
                                  const analysis::FunctionEffectSummary* summary,
                                  const DirectCallUseSummary* useSummary) {
  if (!procedureInlineTraceEnabled())
    return;
  std::cerr << "[procedure-inline] " << stage << " callee="
            << (callee ? callee->name() : std::string("<null>"))
            << " reason=" << (reason ? reason : "-");
  if (summary) {
    std::cerr << " inst=" << summary->instructionCount
              << " loops=" << summary->loopCount
              << " argMemOnly=" << (summary->argMemOnly ? 1 : 0)
              << " noRecurse=" << (summary->noRecurse ? 1 : 0)
              << " readsUnknown=" << (summary->readsUnknownMemory ? 1 : 0)
              << " readsGlobal=" << (summary->readsMutableGlobalMemory ? 1 : 0);
  }
  if (useSummary) {
    std::cerr << " callers=" << useSummary->callerCount
              << " calls=" << useSummary->callSiteCount;
  }
  std::cerr << "\n";
}

bool isLoopProcedureInlineCandidate(
    Module& mod,
    Function* callee,
    const analysis::FunctionEffectSummary* summary,
    const DirectCallUseSummary& useSummary,
    analysis::AnalysisManager& analysisManager) {
  auto reject = [&](const char* reason) {
    traceProcedureInlineDecision("reject", callee, reason, summary,
                                 &useSummary);
    return false;
  };
  if (!callee || !summary || callee->name() == "main")
    return false;
  const bool standardMemoryShape =
      summary->argMemOnly || hasProcedureInlineMemoryShape(callee);
  const bool compactAggregateGlobalProcedure =
      !standardMemoryShape && summary->loopCount <= 2 &&
      summary->instructionCount <= 96 && hasAggregateGlobalMemoryAccess(callee);
  // 一个热点调用可能通过有限的叶子过程暴露更多标量全局转发机会，收益不只
  // 来自调用开销本身。只有所有直接调用点都位于循环中时，才允许稍大的聚合
  // 状态 helper 进入这里；四层以内的过程保留少量清理余量，避免叶子内联后
  // 刚好越过原预算，同时继续排除更深的循环结构和冷代码。
  const bool hotAggregateGlobalProcedure =
      !standardMemoryShape && summary->loopCount <= 4 &&
      summary->instructionCount <= 144 &&
      hasAggregateGlobalMemoryAccess(callee) &&
      allDirectCallSitesAreLoopHot(mod, callee->name(), analysisManager);
  const bool aggregateGlobalProcedure =
      compactAggregateGlobalProcedure || hotAggregateGlobalProcedure;
  const bool builtinOnlyHelper =
      callsOnlyBuiltins(callee, mod) &&
      useSummary.callerCount == 1 &&
      useSummary.callSiteCount == 1 &&
      summary->instructionCount > 0 &&
      summary->instructionCount <= 64 &&
      summary->loopCount == 0 &&
      callee->blocks().size() <= 24;
  const bool splitHotStatefulUse =
      aggregateGlobalProcedure &&
      useSummary.callerCount == useSummary.callSiteCount &&
      useSummary.callerCount > 1 &&
      useSummary.callerCount <= 2 &&
      useSummary.callSiteCount <= 2 &&
      allDirectCallSitesAreLoopHot(mod, callee->name(), analysisManager);
  // 小型单循环状态过程即使来自两个调用者，完整展开后的总代码量仍在预算
  // 内；允许这类有限多用例继续暴露循环状态，避免一个冷调用点阻断热点调用
  // 的过程间标量化。
  const bool compactStatefulMultiUse =
      hasAggregateGlobalMemoryAccess(callee) &&
      !summary->readsUnknownMemory && summary->loopCount == 1 &&
      summary->instructionCount <= 64 && useSummary.callerCount > 1 &&
      useSummary.callerCount <= 2 && useSummary.callSiteCount > 1 &&
      useSummary.callSiteCount <= 2;
  // 下面是一组拒绝条件：副作用/递归、调用点数量、体积与循环数阈值；
  // 全部通过后再检查指令集安全性与调用点所在循环的热度。
  if ((!summary->noRecurse && !builtinOnlyHelper) || summary->readsUnknownMemory ||
      (!standardMemoryShape && !aggregateGlobalProcedure && !builtinOnlyHelper)) {
    return reject("effects");
  }
  const bool singleCallerUse =
      useSummary.callerCount == 1 &&
      useSummary.callSiteCount == 1;
  if ((!singleCallerUse && !splitHotStatefulUse &&
       !compactStatefulMultiUse) ||
      useSummary.callSiteCount <= 0 || useSummary.callSiteCount > 2) {
    return reject("use-count");
  }
  if (summary->instructionCount <= 0 || summary->instructionCount > 384 ||
      summary->instructionCount * useSummary.callSiteCount > 640 ||
      (!builtinOnlyHelper && summary->loopCount <= 0) ||
      summary->loopCount > 5 ||
      callee->blocks().size() > 48) {
    return reject("size");
  }
  if (aggregateGlobalProcedure &&
      canonicalizeReturnsForInlining(callee, /*maxReturns=*/4)) {
    analysisManager.invalidateFunction(callee,
                                       analysis::AnalysisInvalidation::CFG);
  }
  if (!(hasOnlyProcedureInlineSafeOps(callee) ||
        (builtinOnlyHelper && hasOnlyBuiltinInlineSafeOps(callee, mod))))
    return reject("unsafe-ops");
  if (builtinOnlyHelper) {
    Function* caller = findSingleDirectCaller(mod, callee->name());
    if (!caller || caller == callee)
      return reject("caller");
    if (!hasLoopCallSite(caller, callee->name(), analysisManager))
      return reject("cold-callsite");
    traceProcedureInlineDecision("accept", callee, "loop-builtin-helper",
                                 summary, &useSummary);
    return true;
  }
  analysis::FunctionAnalysisCache& calleeCache = analysisManager.function(callee);
  if (!hasInlineableLoopShape(callee, calleeCache, 5, 128, 3))
    return reject("loop-shape");
  if (splitHotStatefulUse || compactStatefulMultiUse) {
    traceProcedureInlineDecision("accept", callee, "loop-procedure-multi",
                                 summary, &useSummary);
    return true;
  }
  Function* caller = findSingleDirectCaller(mod, callee->name());
  if (!caller || caller == callee)
    return reject("caller");
  if (useSummary.callSiteCount != 1 &&
      !hasLoopCallSite(caller, callee->name(), analysisManager))
    return reject("cold-callsite");
  traceProcedureInlineDecision("accept", callee, "loop-procedure", summary,
                               &useSummary);
  return true;
}

bool inlineSingleCallerLoopProcedures(
    Module& mod,
    analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  int budget = 32;
  while (budget-- > 0) {
    const analysis::FunctionEffectMap& effects =
        analysisManager.functionEffects();
    const std::unordered_set<Function*> deadReturnPureFunctions =
        collectDeadReturnChainFunctions(mod, effects);
    std::unordered_map<Function*, DirectCallUseSummary> callUseSummary =
        collectDirectCallUseSummary(mod);
    std::unordered_map<std::string, Function*> inlineable;
    for (auto& funcPtr : mod.functions()) {
      Function* callee = funcPtr.get();
      auto useIt = callUseSummary.find(callee);
      if (useIt == callUseSummary.end())
        continue;
      if (deadReturnPureFunctions.count(callee))
        continue;
      const auto* summary = analysis::lookupFunctionEffect(effects, callee);
      if (isLoopProcedureInlineCandidate(mod, callee, summary, useIt->second,
                                         analysisManager)) {
        inlineable[callee->name()] = callee;
      }
    }
    if (inlineable.empty())
      break;

    // 与小型 helper 内联相同的逐轮策略：每轮只展开一个调用点，
    // 内联成功后失效 CFG 与副作用摘要再重新收集。
    bool roundChanged = false;
    for (auto& funcPtr : mod.functions()) {
      Function* caller = funcPtr.get();
      if (!caller)
        continue;
      std::vector<BasicBlock*> blocks;
      for (auto& bbPtr : caller->blocks())
        blocks.push_back(bbPtr.get());
      for (auto* bb : blocks) {
        if (!bb)
          continue;
        std::vector<Instruction*> calls;
        for (auto& instPtr : bb->instructions()) {
          Instruction* inst = instPtr.get();
          if (inst && inst->opcode() == Opcode::Call)
            calls.push_back(inst);
        }
        for (auto* call : calls) {
          if (!call || call->parent() != bb) {
            continue;
          }
          auto it = inlineable.find(call->name());
          if (it == inlineable.end() || it->second == caller)
            continue;
          if (inlineCallIntoBlock(bb, call, it->second)) {
            changed = true;
            roundChanged = true;
            analysisManager.invalidateModule(
                analysis::AnalysisInvalidation::CFG);
            analysisManager.invalidateModule(
                analysis::AnalysisInvalidation::Effects);
            break;
          }
        }
        if (roundChanged)
          break;
      }
      if (roundChanged)
        break;
    }
    if (!roundChanged)
      break;
  }
  return changed;
}


} // namespace core_ir
