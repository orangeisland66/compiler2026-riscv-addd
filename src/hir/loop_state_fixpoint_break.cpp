#include "research_switch.h"
// 文件职责：实现循环状态运行期不动点提前退出——确定性循环体的跨轮状态收敛
// 后，剩余迭代与上一轮完全重复（观察等价），插入快照比较提前 break。
#include "loop_state_fixpoint_break.h"

#include "../analysis/analysis_manager.h"
#include "../analysis/function_analysis.h"
#include "../analysis/function_effects.h"
#include "../analysis/loop_info.h"
#include "canonical_loop.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace core_ir;

namespace hir {
namespace {

// 模块内每个定义函数的全局读/写集（含被调函数的传递闭包）。
// 遇到 builtin（IO/getarray 等）或经指针参数的访存即标 unknown。
struct GlobalRW {
  std::unordered_set<const GlobalVar*> reads;
  std::unordered_set<const GlobalVar*> writes;
  bool unknown = false;
  bool hasLoopCallee = false; // 闭包内存在含循环的函数（收益门槛用）
};

struct FixpointPlan {
  BasicBlock* preheader = nullptr;
  BasicBlock* header = nullptr;
  BasicBlock* latch = nullptr;
  BasicBlock* exit = nullptr;
  Instruction* ivPhi = nullptr;
  std::vector<Value*> snapshotAddrs; // GlobalVar* 或循环外 Alloca
};

bool traceOn() {
  return sycEnvironmentValue("SYC_TRACE_LOOP_STATE_FIXPOINT_BREAK") != nullptr;
}

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

Value* phiIncomingValue(Instruction* phi, BasicBlock* pred) {
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    if (phi->operand(i + 1) == pred)
      return phi->operand(i);
  }
  return nullptr;
}

std::string normalizeContinueCond(const std::string& cond, bool ivIsLhs,
                                  bool continueOnTrue) {
  std::string c = cond;
  if (!ivIsLhs) {
    if (c == "lt") c = "gt";
    else if (c == "gt") c = "lt";
    else if (c == "le") c = "ge";
    else if (c == "ge") c = "le";
  }
  if (!continueOnTrue) {
    if (c == "ne") c = "eq";
    else if (c == "eq") c = "ne";
    else if (c == "lt") c = "ge";
    else if (c == "ge") c = "lt";
    else if (c == "le") c = "gt";
    else if (c == "gt") c = "le";
  }
  return c;
}

int matchUnitStep(Value* next, Instruction* ivPhi) {
  auto* inst = dynamic_cast<Instruction*>(next);
  if (!inst || inst->numOperands() != 2)
    return 0;
  Value* lhs = inst->operand(0);
  Value* rhs = inst->operand(1);
  if (inst->opcode() == Opcode::Add) {
    const ConstInt* c = nullptr;
    if (lhs == ivPhi)
      c = dynamic_cast<const ConstInt*>(rhs);
    else if (rhs == ivPhi)
      c = dynamic_cast<const ConstInt*>(lhs);
    if (c && (c->value() == 1 || c->value() == -1))
      return c->value();
    return 0;
  }
  if (inst->opcode() == Opcode::Sub && lhs == ivPhi) {
    const auto* c = dynamic_cast<const ConstInt*>(rhs);
    if (c && (c->value() == 1 || c->value() == -1))
      return -c->value();
  }
  return 0;
}

bool bodyOpAllowed(Instruction* inst) {
  switch (inst->opcode()) {
  case Opcode::Load: case Opcode::GEP: case Opcode::Phi:
  case Opcode::Add: case Opcode::Sub: case Opcode::Mul: case Opcode::Mulh:
  case Opcode::SDiv: case Opcode::SRem:
  case Opcode::And: case Opcode::Or: case Opcode::Xor:
  case Opcode::Shl: case Opcode::Shr:
  case Opcode::FAdd: case Opcode::FSub: case Opcode::FMul: case Opcode::FDiv:
  case Opcode::ICmp: case Opcode::FCmp:
  case Opcode::SIToFP: case Opcode::FPToSI:
  case Opcode::Neg: case Opcode::FNeg: case Opcode::BitCast:
  case Opcode::Select: case Opcode::Mov:
    return true;
  default:
    return false;
  }
}

// 迭代不动点求模块级全局读写集。builtin 调用（模块内无定义体，
// 含全部 IO）直接令调用方 unknown。经指针实参的调用同样 unknown（保守）。
std::unordered_map<const Function*, GlobalRW>
computeModuleGlobalRW(Module& mod, const analysis::FunctionEffectMap& effects) {
  std::unordered_map<const Function*, GlobalRW> rw;
  for (auto& funcPtr : mod.functions())
    rw[funcPtr.get()];
  bool changed = true;
  while (changed) {
    changed = false;
    for (auto& funcPtr : mod.functions()) {
      Function* func = funcPtr.get();
      GlobalRW& info = rw[func];
      if (info.unknown)
        continue;
      GlobalRW next = info;
      for (auto& bbPtr : func->blocks()) {
        for (auto& instPtr : bbPtr->instructions()) {
          Instruction* inst = instPtr.get();
          if (inst->opcode() == Opcode::Load) {
            Value* base = memoryRoot(inst->operand(0));
            if (auto* gv = dynamic_cast<GlobalVar*>(base)) {
              next.reads.insert(gv);
            } else if (auto* bi = dynamic_cast<Instruction*>(base)) {
              if (bi->opcode() != Opcode::Alloca)
                next.unknown = true; // 指针参数等未知来源
            } else {
              next.unknown = true;
            }
          } else if (inst->opcode() == Opcode::Store ||
                     inst->opcode() == Opcode::VStore) {
            Value* base = memoryRoot(inst->operand(1));
            if (auto* gv = dynamic_cast<GlobalVar*>(base)) {
              next.writes.insert(gv);
            } else if (auto* bi = dynamic_cast<Instruction*>(base)) {
              if (bi->opcode() != Opcode::Alloca)
                next.unknown = true;
            } else {
              next.unknown = true;
            }
          } else if (inst->opcode() == Opcode::VLoad) {
            // 向量 load 只允许目标是全局变量，来源不明即保守置 unknown。
            Value* base = memoryRoot(inst->operand(0));
            if (auto* gv = dynamic_cast<GlobalVar*>(base))
              next.reads.insert(gv);
            else
              next.unknown = true;
          } else if (inst->opcode() == Opcode::Call) {
            Function* callee = mod.getFunction(inst->name());
            if (!callee) {
              next.unknown = true; // builtin：IO/getarray/内建 memset 等
            } else {
              const GlobalRW& c = rw[callee];
              if (c.unknown)
                next.unknown = true;
              next.reads.insert(c.reads.begin(), c.reads.end());
              next.writes.insert(c.writes.begin(), c.writes.end());
              if (c.hasLoopCallee)
                next.hasLoopCallee = true;
              const auto* summary =
                  analysis::lookupFunctionEffect(effects, callee);
              if (summary && summary->loopCount > 0)
                next.hasLoopCallee = true;
              for (Value* arg : inst->operands()) {
                if (arg->type().isPtr())
                  next.unknown = true; // 指针实参：数组传参不做
              }
            }
          }
        }
      }
      if (next.unknown != info.unknown || next.reads != info.reads ||
          next.writes != info.writes ||
          next.hasLoopCallee != info.hasLoopCallee) {
        info = std::move(next);
        changed = true;
      }
    }
  }
  return rw;
}

bool isScalarI32Global(const GlobalVar* gv) {
  return gv->dims().empty() && gv->elemTy().isI32() && gv->sizeBytes() == 4 &&
         !gv->isConst();
}

// alloca 全函数无逃逸——所有使用只能是 load 或作为 store 的地址。
bool allocaNoEscape(Instruction* alloca) {
  for (Instruction* user : alloca->uses()) {
    if (user->opcode() == Opcode::Load)
      continue;
    if (user->opcode() == Opcode::Store && user->operand(1) == alloca &&
        user->operand(0) != alloca)
      continue;
    return false;
  }
  return true;
}

bool collectPlan(const analysis::NaturalLoop& loop, Module& mod,
                 const std::unordered_map<const Function*, GlobalRW>& rwMap,
                 FixpointPlan& plan) {
  auto reject = [&](const char* reason) {
    if (traceOn()) {
      std::cerr << "[loop-state-fixpoint-break] reject header="
                << (loop.header ? loop.header->name() : std::string("<null>"))
                << " reason=" << reason << "\n";
    }
    return false;
  };
  if (loop.latches.size() != 1)
    return reject("latches");
  BasicBlock* header = loop.header;
  BasicBlock* latch = loop.latches.front();
  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader || loop.blocks.count(preheader))
    return reject("preheader");
  if (header->preds().size() != 2)
    return reject("header-preds");
  Instruction* latchTerm = latch->terminator();
  if (!latchTerm || latchTerm->opcode() != Opcode::Br ||
      latchTerm->operand(0) != header)
    return reject("latch-term");

  // header 结构：phi* + icmp + condbr。
  std::vector<Instruction*> phis;
  Instruction* cmp = nullptr;
  Instruction* condBr = nullptr;
  for (auto& instPtr : header->instructions()) {
    Instruction* inst = instPtr.get();
    if (inst->opcode() == Opcode::Phi) {
      if (cmp || condBr)
        return reject("header-order");
      phis.push_back(inst);
      continue;
    }
    if (inst->opcode() == Opcode::ICmp) {
      if (cmp || condBr)
        return reject("header-order");
      cmp = inst;
      continue;
    }
    if (inst->opcode() == Opcode::CondBr) {
      if (!cmp || condBr)
        return reject("header-order");
      condBr = inst;
      continue;
    }
    return reject("header-extra-inst");
  }
  if (!cmp || !condBr || condBr != header->terminator())
    return reject("header-structure");
  if (condBr->operand(0) != cmp || cmp->uses().size() != 1)
    return reject("cmp-uses");

  auto* trueBB = dynamic_cast<BasicBlock*>(condBr->operand(1));
  auto* falseBB = dynamic_cast<BasicBlock*>(condBr->operand(2));
  if (!trueBB || !falseBB)
    return reject("condbr-targets");
  bool continueOnTrue;
  BasicBlock* exit = nullptr;
  if (loop.blocks.count(trueBB) && !loop.blocks.count(falseBB)) {
    continueOnTrue = true;
    exit = falseBB;
  } else if (loop.blocks.count(falseBB) && !loop.blocks.count(trueBB)) {
    continueOnTrue = false;
    exit = trueBB;
  } else {
    return reject("exit-edge");
  }
  if (exit->preds().size() != 1 || exit->preds().front() != header)
    return reject("exit-preds");
  if (!exit->instructions().empty() &&
      exit->instructions().front()->opcode() == Opcode::Phi)
    return reject("exit-phi");

  // 唯一 header phi 即 IV：跨轮标量状态必须全部经内存传递，快照才完整。
  if (phis.size() != 1)
    return reject("multi-phi");
  Instruction* ivPhi = phis.front();
  Value* lhs = cmp->operand(0);
  Value* rhs = cmp->operand(1);
  Value* bound = nullptr;
  bool ivIsLhs = false;
  if (lhs == ivPhi) { bound = rhs; ivIsLhs = true; }
  else if (rhs == ivPhi) { bound = lhs; ivIsLhs = false; }
  else return reject("iv-not-in-cmp");
  auto* boundInst = dynamic_cast<Instruction*>(bound);
  if (boundInst && boundInst->parent() && loop.blocks.count(boundInst->parent()))
    return reject("bound-in-loop");
  if (!ivPhi->type().isI32())
    return reject("iv-type");

  Value* ivNextV = phiIncomingValue(ivPhi, latch);
  if (!ivNextV)
    return reject("iv-incoming");
  int step = matchUnitStep(ivNextV, ivPhi);
  if (step == 0)
    return reject("iv-step");
  auto* ivNext = dynamic_cast<Instruction*>(ivNextV);
  if (!ivNext || ivNext->uses().size() != 1)
    return reject("iv-next");
  // IV 只能被守卫比较与自增使用：提前 break 时 IV 值与正常退出不同，
  // 绝不能存在体内或循环外的其他读取。
  for (Instruction* user : ivPhi->uses()) {
    if (user != cmp && user != ivNext)
      return reject("iv-used");
  }
  // 只接受严格 lt/gt 单调计数形态：原循环在 IV 越界前必然退出。
  std::string cond = normalizeContinueCond(canonicalLoopCmpCond(cmp), ivIsLhs,
                                           continueOnTrue);
  if (!((cond == "lt" && step == 1) ||
        (cond == "gt" && step == -1)))
    return reject("cond-step");

  // 体内扫描：汇总循环级全局/alloca 读写集；调用只允许模块内定义函数
  //（builtin 即 IO，直接拒绝）；终结指令目标必须留在循环内。
  std::unordered_set<const GlobalVar*> reads, writes;
  std::vector<Instruction*> allocaWrites;
  bool hasLoopCall = false;
  for (BasicBlock* bb : loop.blocks) {
    if (bb == header)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (inst->isTerminator()) {
        if (inst->opcode() == Opcode::Br) {
          if (!loop.blocks.count(dynamic_cast<BasicBlock*>(inst->operand(0))))
            return reject("body-exit-br");
        } else if (inst->opcode() == Opcode::CondBr) {
          auto* t = dynamic_cast<BasicBlock*>(inst->operand(1));
          auto* f = dynamic_cast<BasicBlock*>(inst->operand(2));
          if (!loop.blocks.count(t) || !loop.blocks.count(f))
            return reject("body-exit-condbr");
        } else {
          return reject("body-term");
        }
        continue;
      }
      if (inst->opcode() == Opcode::Load) {
        Value* base = memoryRoot(inst->operand(0));
        if (auto* gv = dynamic_cast<GlobalVar*>(base)) {
          reads.insert(gv);
        } else {
          auto* bi = dynamic_cast<Instruction*>(base);
          if (!bi || bi->opcode() != Opcode::Alloca ||
              (bi->parent() && loop.blocks.count(bi->parent())))
            return reject("load-base");
          // 循环外 alloca 的读：体内不写则循环不变；体内也写则由
          // allocaWrites 分支统一纳入快照集，此处无需记录。
        }
        continue;
      }
      if (inst->opcode() == Opcode::Store) {
        Value* base = memoryRoot(inst->operand(1));
        if (auto* gv = dynamic_cast<GlobalVar*>(base)) {
          writes.insert(gv);
        } else {
          auto* bi = dynamic_cast<Instruction*>(base);
          if (!bi || bi->opcode() != Opcode::Alloca ||
              (bi->parent() && loop.blocks.count(bi->parent())))
            return reject("store-base");
          if (std::find(allocaWrites.begin(), allocaWrites.end(), bi) ==
              allocaWrites.end())
            allocaWrites.push_back(bi);
        }
        continue;
      }
      if (inst->opcode() == Opcode::Call) {
        Function* callee = mod.getFunction(inst->name());
        if (!callee)
          return reject("builtin-call");
        auto it = rwMap.find(callee);
        if (it == rwMap.end() || it->second.unknown)
          return reject("callee-unknown");
        for (Value* arg : inst->operands()) {
          if (arg->type().isPtr())
            return reject("ptr-arg");
        }
        reads.insert(it->second.reads.begin(), it->second.reads.end());
        writes.insert(it->second.writes.begin(), it->second.writes.end());
        if (it->second.hasLoopCallee)
          hasLoopCall = true;
        continue;
      }
      if (!bodyOpAllowed(inst))
        return reject("body-inst");
    }
  }
  // 收益门槛：体内必须调用含循环的函数（每轮代价高），否则不值得插比较。
  if (!hasLoopCall)
    return reject("no-loop-call");

  // 状态集分类：写集标量全局 → 快照；写集数组不得被体内读（写不读残留
  // 数组不影响体行为）；读集数组不得被体内写（只读输入）。
  std::vector<Value*> snapshotAddrs;
  for (const GlobalVar* gv : writes) {
    if (isScalarI32Global(gv)) {
      snapshotAddrs.push_back(const_cast<GlobalVar*>(gv));
      continue;
    }
    if (reads.count(gv))
      return reject("rw-array");
  }
  // 写集 alloca：无逃逸 i32 标量 → 快照（无逃逸保证调用无法触及）。
  for (Instruction* a : allocaWrites) {
    if (!allocaNoEscape(a))
      return reject("alloca-escape");
    if (a->isArrayObject() || !a->dims().empty() ||
        !a->memoryElemTy().isI32() || a->memorySizeBytes() != 4)
      return reject("alloca-not-scalar-i32");
    snapshotAddrs.push_back(a);
  }
  if (snapshotAddrs.empty())
    return reject("no-state");
  if (snapshotAddrs.size() > 8)
    return reject("snapshot-size");

  plan.preheader = preheader;
  plan.header = header;
  plan.latch = latch;
  plan.exit = exit;
  plan.ivPhi = ivPhi;
  plan.snapshotAddrs = std::move(snapshotAddrs);
  return true;
}

void applyPlan(Function* func, Module& mod, FixpointPlan& plan) {
  // 1) entry 建 prev 快照槽与首轮标志槽（alloca 集中在 entry 头部）。
  BasicBlock* entry = func->entryBlock();
  Instruction* firstNonAlloca = nullptr;
  for (auto& instPtr : entry->instructions()) {
    if (instPtr->opcode() != Opcode::Alloca) {
      firstNonAlloca = instPtr.get();
      break;
    }
  }
  auto makeSlot = [&](const std::string& name) {
    auto* slot = new Instruction(Opcode::Alloca, Type::ptr(), name);
    slot->setMemoryInfo(Type::i32(), 4, false, {});
    if (firstNonAlloca)
      entry->insertBefore(firstNonAlloca, slot);
    else
      entry->pushBack(slot);
    func->addAlloca(slot);
    return slot;
  };
  std::vector<Instruction*> prevSlots;
  for (size_t i = 0; i < plan.snapshotAddrs.size(); ++i)
    prevSlots.push_back(makeSlot("fixpoint.prev"));
  Instruction* flagSlot = makeSlot("fixpoint.flag");

  // 2) preheader 初始化：flag=0，prev 槽清零（值被 flag 遮蔽，仅求确定性）。
  Instruction* phTerm = plan.preheader->terminator();
  auto storeTo = [&](Value* val, Value* addr, Instruction* before,
                     BasicBlock* bb) {
    auto* st = new Instruction(Opcode::Store, Type::voidTy());
    st->addOperand(val);
    st->addOperand(addr);
    if (before)
      bb->insertBefore(before, st);
    else
      bb->pushBack(st);
    return st;
  };
  storeTo(mod.getConstInt(0), flagSlot, phTerm, plan.preheader);
  for (Instruction* slot : prevSlots)
    storeTo(mod.getConstInt(0), slot, phTerm, plan.preheader);

  // 3) 新建 FIXPOINT 块：curr 全等于 prev 且非首轮 → break 到 exit。
  BasicBlock* check = func->newBlock("fixpoint");
  auto* flagLoad = new Instruction(Opcode::Load, Type::i32(), "fixpoint.flag");
  flagLoad->addOperand(flagSlot);
  check->pushBack(flagLoad);
  Value* acc = flagLoad;
  for (size_t i = 0; i < plan.snapshotAddrs.size(); ++i) {
    auto* cur = new Instruction(Opcode::Load, Type::i32(), "fixpoint.cur");
    cur->addOperand(plan.snapshotAddrs[i]);
    check->pushBack(cur);
    auto* prev = new Instruction(Opcode::Load, Type::i32(), "fixpoint.old");
    prev->addOperand(prevSlots[i]);
    check->pushBack(prev);
    auto* eq = new Instruction(Opcode::ICmp, Type::i32(), "eq_fixpoint");
    eq->addOperand(cur);
    eq->addOperand(prev);
    check->pushBack(eq);
    auto* conj = new Instruction(Opcode::And, Type::i32(), "fixpoint.and");
    conj->addOperand(acc);
    conj->addOperand(eq);
    check->pushBack(conj);
    acc = conj;
    storeTo(cur, prevSlots[i], nullptr, check);
  }
  storeTo(mod.getConstInt(1), flagSlot, nullptr, check);
  auto* br = new Instruction(Opcode::CondBr, Type::voidTy());
  br->addOperand(acc);
  br->addOperand(plan.exit);
  br->addOperand(plan.header);
  check->pushBack(br);

  // 4) 回边改经 FIXPOINT；IV phi 的 latch 入边随之换块。
  plan.latch->terminator()->setOperand(0, check);
  for (int i = 0; i + 1 < plan.ivPhi->numOperands(); i += 2) {
    if (plan.ivPhi->operand(i + 1) == plan.latch)
      plan.ivPhi->setOperand(i + 1, check);
  }
  func->recomputeCFG();
}

bool processFunction(Function* func, Module& mod,
                     analysis::AnalysisManager& analysisManager,
                     const std::unordered_map<const Function*, GlobalRW>& rwMap) {
  if (!func || !func->entryBlock())
    return false;
  bool changed = false;
  while (true) {
    func->recomputeCFG();
    analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
    if (cache.blocks().empty())
      return changed;
    bool localChanged = false;
    for (const auto& loop : cache.loops()) {
      FixpointPlan plan;
      if (!collectPlan(loop, mod, rwMap, plan))
        continue;
      applyPlan(func, mod, plan);
      localChanged = true;
      changed = true;
      analysisManager.invalidateFunction(func,
                                         analysis::AnalysisInvalidation::CFG);
      break;
    }
    if (!localChanged)
      break;
  }
  return changed;
}

} // namespace

bool applyLoopStateFixpointBreak(Module& mod,
                                 analysis::AnalysisManager& analysisManager) {
  if (sycEnvironmentValue("SYC_SKIP_HIR_LOOP_STATE_FIXPOINT_BREAK"))
    return false;
  const analysis::FunctionEffectMap& effects =
      analysisManager.functionEffects();
  auto rwMap = computeModuleGlobalRW(mod, effects);
  bool changed = false;
  for (auto& funcPtr : mod.functions())
    changed |= processFunction(funcPtr.get(), mod, analysisManager, rwMap);
  return changed;
}

} // namespace hir
