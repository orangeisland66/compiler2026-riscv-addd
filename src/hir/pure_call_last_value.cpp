#include "research_switch.h"
// 文件职责：实现"仅末值存活"计数循环的塌缩——体内只读、携带值整体覆盖时，
// 保留 header 守卫并用末轮 IV 单次执行体替换循环。
#include "pure_call_last_value.h"

#include "../analysis/analysis_manager.h"
#include "../analysis/function_analysis.h"
#include "../analysis/function_effects.h"
#include "../analysis/loop_info.h"
#include "../analysis/termination.h"
#include "canonical_loop.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>

using namespace core_ir;

namespace hir {
namespace {

struct CollapsePlan {
  BasicBlock* preheader = nullptr;
  BasicBlock* header = nullptr;
  BasicBlock* body = nullptr;
  BasicBlock* exit = nullptr;
  Instruction* cmp = nullptr;
  Instruction* condBr = nullptr;
  Instruction* ivPhi = nullptr;
  Value* ivInit = nullptr;
  Value* bound = nullptr;
  int step = 0;          // 迭代步长
  int lastOffset = 0;    // lastIV = bound + lastOffset
  int exitOffset = 0;    // exitIV = bound + exitOffset（末轮之后的 IV）
  std::vector<Instruction*> phis; // header 全部 phi（含 ivPhi）
};

bool isInLoop(const analysis::NaturalLoop& loop, const Value* v) {
  const auto* inst = dynamic_cast<const Instruction*>(v);
  if (!inst || !inst->parent())
    return false;
  return loop.blocks.count(inst->parent()) != 0;
}

// 把"iv 在左、真边继续"的比较条件归一化。iv 在右侧时交换方向，
// 假边继续时取反。返回空串表示不支持。
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

// 由归一化后的 continue 条件与步长解出末轮 IV / 退出时 IV 相对
// bound 的偏移。i32 单位步长 ne 在位向量语义下必达 bound；严格 lt/gt 在
// 守卫为真时先到达 bound 再可能越界。包含边界的 le/ge 不在此证明范围内。
bool solveLastIV(const std::string& cond, int step, int& lastOffset,
                 int& exitOffset) {
  if (cond == "ne" && step == -1) { lastOffset = 1; exitOffset = 0; return true; }
  if (cond == "ne" && step == 1) { lastOffset = -1; exitOffset = 0; return true; }
  if (cond == "gt" && step == -1) { lastOffset = 1; exitOffset = 0; return true; }
  if (cond == "lt" && step == 1) { lastOffset = -1; exitOffset = 0; return true; }
  return false;
}

Value* phiIncomingValue(Instruction* phi, BasicBlock* pred) {
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    if (phi->operand(i + 1) == pred)
      return phi->operand(i);
  }
  return nullptr;
}

// 识别 iv 的 latch 更新是否为 iv±1，返回步长（0 表示不匹配）。
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

bool bodyInstAllowed(Instruction* inst, Module& mod,
                     const analysis::FunctionEffectMap& effects,
                     const analysis::FunctionTerminationMap& termination) {
  switch (inst->opcode()) {
  case Opcode::Call:
    return analysis::isReadonlyCall(inst, mod, effects) &&
           analysis::isProvablyTotalCall(inst, mod, termination);
  case Opcode::Load:
  case Opcode::GEP:
  case Opcode::Add: case Opcode::Sub: case Opcode::Mul: case Opcode::Mulh:
  case Opcode::And: case Opcode::Or: case Opcode::Xor:
  case Opcode::FAdd: case Opcode::FSub: case Opcode::FMul: case Opcode::FDiv:
  case Opcode::ICmp: case Opcode::FCmp:
  case Opcode::SIToFP: case Opcode::FPToSI:
  case Opcode::Neg: case Opcode::FNeg: case Opcode::BitCast:
  case Opcode::Select: case Opcode::Mov:
    return true;
  case Opcode::SDiv: case Opcode::SRem:
  case Opcode::Shl: case Opcode::Shr:
    return analysis::isProvablyNonTrappingScalarInstruction(inst);
  default:
    return false;
  }
}

bool collectPlan(const analysis::NaturalLoop& loop,
                 Module& mod, const analysis::FunctionEffectMap& effects,
                 const analysis::FunctionTerminationMap& termination,
                 CollapsePlan& plan) {
  // 诊断追踪——设置 SYC_TRACE_PURE_CALL_LAST_VALUE 时打印拒绝原因。
  auto reject = [&](const char* reason) {
    if (sycEnvironmentValue("SYC_TRACE_PURE_CALL_LAST_VALUE")) {
      std::cerr << "[pure-call-last-value] reject header="
                << (loop.header ? loop.header->name() : std::string("<null>"))
                << " reason=" << reason << "\n";
    }
    return false;
  };
  if (loop.blocks.size() != 2 || loop.latches.size() != 1)
    return reject("shape");
  BasicBlock* header = loop.header;
  BasicBlock* body = loop.latches.front();
  if (body == header || !loop.blocks.count(body))
    return reject("latch");
  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader || loop.blocks.count(preheader))
    return reject("preheader");
  if (header->preds().size() != 2)
    return reject("header-preds");

  // header 结构：phi* + icmp + condbr，不允许其他指令。
  std::vector<Instruction*> phis;
  Instruction* cmp = nullptr;
  Instruction* condBr = nullptr;
  for (auto& instPtr : header->instructions()) {
    Instruction* inst = instPtr.get();
    if (inst->opcode() == Opcode::Phi) {
      if (cmp || condBr)
        return false;
      phis.push_back(inst);
      continue;
    }
    if (inst->opcode() == Opcode::ICmp) {
      if (cmp || condBr)
        return false;
      cmp = inst;
      continue;
    }
    if (inst->opcode() == Opcode::CondBr) {
      if (!cmp || condBr)
        return false;
      condBr = inst;
      continue;
    }
    return false;
  }
  if (!cmp || !condBr || condBr != header->terminator())
    return reject("header-structure");
  if (condBr->operand(0) != cmp)
    return reject("condbr-cond");
  // cmp 只允许被本循环的 condbr 使用，避免比较值泄漏到别处。
  if (cmp->uses().size() != 1)
    return reject("cmp-uses");

  auto* trueBB = dynamic_cast<BasicBlock*>(condBr->operand(1));
  auto* falseBB = dynamic_cast<BasicBlock*>(condBr->operand(2));
  if (!trueBB || !falseBB)
    return reject("condbr-targets");
  bool continueOnTrue;
  BasicBlock* exit = nullptr;
  if (trueBB == body && !loop.blocks.count(falseBB)) {
    continueOnTrue = true;
    exit = falseBB;
  } else if (falseBB == body && !loop.blocks.count(trueBB)) {
    continueOnTrue = false;
    exit = trueBB;
  } else {
    return reject("exit-edge");
  }
  // 出口块仅由 header 进入，phi 手术才可控。
  if (exit->preds().size() != 1 || exit->preds().front() != header)
    return reject("exit-preds");

  // 识别 iv：cmp 一侧为 header phi，另一侧为循环外不变值。
  Value* lhs = cmp->operand(0);
  Value* rhs = cmp->operand(1);
  Instruction* ivPhi = nullptr;
  Value* bound = nullptr;
  bool ivIsLhs = false;
  for (auto* phi : phis) {
    if (lhs == phi) { ivPhi = phi; bound = rhs; ivIsLhs = true; break; }
    if (rhs == phi) { ivPhi = phi; bound = lhs; ivIsLhs = false; break; }
  }
  if (!ivPhi || !bound || !ivPhi->type().isI32())
    return reject("iv-phi");
  if (isInLoop(loop, bound))
    return reject("bound-in-loop");

  std::string cond = normalizeContinueCond(canonicalLoopCmpCond(cmp), ivIsLhs,
                                           continueOnTrue);
  Value* ivInit = phiIncomingValue(ivPhi, preheader);
  Value* ivNext = phiIncomingValue(ivPhi, body);
  if (!ivInit || !ivNext)
    return reject("iv-incoming");
  int step = matchUnitStep(ivNext, ivPhi);
  if (step == 0)
    return reject("iv-step");
  int lastOffset = 0, exitOffset = 0;
  if (!solveLastIV(cond, step, lastOffset, exitOffset))
    return reject("cond-step");

  // 循环体的终结指令必须回到 header；其余指令只读，且不能读取非 iv PHI。
  // 允许写入循环外标量 alloca 且循环体不回读的覆盖式 store，因为只保留末轮
  // 写入时，结果仍是相同的最终值。
  bool hasCall = false;
  std::vector<Value*> storeTargets;
  for (auto& instPtr : body->instructions()) {
    Instruction* inst = instPtr.get();
    if (inst == body->terminator()) {
      if (inst->opcode() != Opcode::Br || inst->operand(0) != header)
        return reject("latch-term");
      continue;
    }
    if (inst->opcode() == Opcode::Store) {
      auto* addr = dynamic_cast<Instruction*>(inst->operand(1));
      if (!addr || addr->opcode() != Opcode::Alloca ||
          (addr->parent() && loop.blocks.count(addr->parent())))
        return reject("store-target");
      storeTargets.push_back(addr);
    } else if (!bodyInstAllowed(inst, mod, effects, termination)) {
      if (sycEnvironmentValue("SYC_TRACE_PURE_CALL_LAST_VALUE")) {
        std::cerr << "[pure-call-last-value]   body-inst opcode="
                  << static_cast<int>(inst->opcode()) << " name="
                  << inst->name();
        for (Value* op : inst->operands())
          std::cerr << " op=" << op->name();
        std::cerr << "\n";
      }
      return reject("body-inst");
    }
    if (inst->opcode() == Opcode::Call)
      hasCall = true;
    for (Value* op : inst->operands()) {
      if (op == ivPhi)
        continue;
      if (std::find(phis.begin(), phis.end(), op) != phis.end())
        return reject("body-reads-phi");
    }
  }
  // 收益门槛：体内至少有一个（只读）调用，避免无谓改写引发大面积汇编抖动。
  if (!hasCall)
    return reject("no-call");

  // store 目标 alloca 的全部使用必须是 load/作为 store 地址（无逃逸，
  // 只读调用无法观察它），且循环内不得有 load 回读（否则构成真递推）。
  for (Value* target : storeTargets) {
    for (Instruction* user : target->uses()) {
      const bool isLoad = user->opcode() == Opcode::Load;
      const bool isStoreAddr =
          user->opcode() == Opcode::Store && user->operand(1) == target &&
          user->operand(0) != target;
      if (!isLoad && !isStoreAddr)
        return reject("store-escape");
      if (isLoad && user->parent() && loop.blocks.count(user->parent()))
        return reject("store-read-back");
    }
  }

  // 非 iv phi 的 latch 值只能是体内指令 / 循环外不变值 / ivPhi 本身。
  for (auto* phi : phis) {
    if (phi == ivPhi)
      continue;
    if (phi->numOperands() != 4)
      return reject("phi-arity");
    Value* latchVal = phiIncomingValue(phi, body);
    if (!latchVal)
      return reject("phi-latch");
    if (latchVal != ivPhi &&
        std::find(phis.begin(), phis.end(), latchVal) != phis.end())
      return reject("phi-recurrence");
  }
  if (ivPhi->numOperands() != 4)
    return reject("iv-arity");

  plan.preheader = preheader;
  plan.header = header;
  plan.body = body;
  plan.exit = exit;
  plan.cmp = cmp;
  plan.condBr = condBr;
  plan.ivPhi = ivPhi;
  plan.ivInit = ivInit;
  plan.bound = bound;
  plan.step = step;
  plan.lastOffset = lastOffset;
  plan.exitOffset = exitOffset;
  plan.phis = phis;
  return true;
}

// 物化 bound+offset。bound 为常量时直接折叠；否则在 body 头部
// 插入加法指令。offset 为 0 时原样返回 bound。
Value* materializeOffset(Module& mod, BasicBlock* body, Value* bound,
                         int offset) {
  if (offset == 0)
    return bound;
  if (const auto* c = dynamic_cast<const ConstInt*>(bound)) {
    const int64_t adjusted = static_cast<int64_t>(c->value()) + offset;
    if (adjusted >= std::numeric_limits<int32_t>::min() &&
        adjusted <= std::numeric_limits<int32_t>::max())
      return mod.getConstInt(static_cast<int32_t>(adjusted));
  }
  auto* add = new Instruction(Opcode::Add, Type::i32(), "lastiv");
  add->addOperand(bound);
  add->addOperand(mod.getConstInt(offset));
  Instruction* first = body->instructions().empty()
                           ? nullptr
                           : body->instructions().front().get();
  if (first)
    body->insertBefore(first, add);
  else
    body->pushBack(add);
  return add;
}

bool applyPlan(Function* func, Module& mod, CollapsePlan& plan) {
  Value* lastIV = materializeOffset(mod, plan.body, plan.bound,
                                    plan.lastOffset);
  // 体内 iv 使用改为末轮值。
  for (auto& instPtr : plan.body->instructions()) {
    Instruction* inst = instPtr.get();
    if (inst == lastIV)
      continue;
    for (int i = 0; i < inst->numOperands(); ++i) {
      if (inst->operand(i) == plan.ivPhi)
        inst->setOperand(i, lastIV);
    }
  }

  // cmp 改用初值做守卫。
  for (int i = 0; i < plan.cmp->numOperands(); ++i) {
    if (plan.cmp->operand(i) == plan.ivPhi)
      plan.cmp->setOperand(i, plan.ivInit);
  }

  // 逐个 phi：外部使用改接 exit 上新建的 phi（未进入走初值，进入走末轮值）。
  Value* exitIV = nullptr;
  for (auto* phi : plan.phis) {
    Value* initVal = phiIncomingValue(phi, plan.preheader);
    Value* latchVal = phiIncomingValue(phi, plan.body);
    Value* bodyVal = nullptr;
    if (phi == plan.ivPhi) {
      if (!exitIV)
        exitIV = materializeOffset(mod, plan.body, plan.bound,
                                   plan.exitOffset);
      bodyVal = exitIV;
    } else {
      bodyVal = (latchVal == plan.ivPhi) ? lastIV : latchVal;
    }
    if (!phi->uses().empty()) {
      auto* newPhi = new Instruction(Opcode::Phi, phi->type(), "collapse");
      newPhi->addOperand(initVal);
      newPhi->addOperand(plan.header);
      newPhi->addOperand(bodyVal);
      newPhi->addOperand(plan.body);
      plan.exit->pushFront(newPhi);
      phi->replaceAllUsesWith(newPhi);
    }
    plan.header->remove(phi);
  }

  // 回边改为直达出口。
  Instruction* bodyBr = plan.body->terminator();
  bodyBr->setOperand(0, plan.exit);
  func->recomputeCFG();

  // 清理体内死代码（原 iv 递增链等）。
  bool removedAny = true;
  while (removedAny) {
    removedAny = false;
    for (auto& instPtr : plan.body->instructions()) {
      Instruction* inst = instPtr.get();
      if (inst->isTerminator() || !inst->uses().empty())
        continue;
      if (inst->opcode() == Opcode::Call || inst->hasSideEffects())
        continue;
      plan.body->remove(inst);
      removedAny = true;
      break;
    }
  }
  return true;
}

bool processFunction(Function* func, Module& mod,
                     analysis::AnalysisManager& analysisManager,
                     const analysis::FunctionTerminationMap& termination) {
  if (!func || !func->entryBlock())
    return false;
  bool changed = false;
  while (true) {
    func->recomputeCFG();
    analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
    if (cache.blocks().empty())
      return changed;
    const analysis::FunctionEffectMap& effects =
        analysisManager.functionEffects();
    bool localChanged = false;
    for (const auto& loop : cache.loops()) {
      CollapsePlan plan;
      if (!collectPlan(loop, mod, effects, termination, plan))
        continue;
      if (!applyPlan(func, mod, plan))
        continue;
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

bool applyPureCallLastValueCollapse(Module& mod,
                                    analysis::AnalysisManager& analysisManager) {
  if (sycEnvironmentValue("SYC_SKIP_HIR_PURE_CALL_LAST_VALUE"))
    return false;
  const analysis::FunctionTerminationMap termination =
      analysis::proveModuleFunctionTermination(mod, analysisManager);
  bool changed = false;
  for (auto& funcPtr : mod.functions())
    changed |= processFunction(funcPtr.get(), mod, analysisManager,
                               termination);
  return changed;
}

} // namespace hir
