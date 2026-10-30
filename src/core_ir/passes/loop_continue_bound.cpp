#include "research_switch.h"
// 文件职责：分析循环中的 continue 路径并整理循环边界信息。
#include "loop_continue_bound.h"

#include "../../analysis/analysis_manager.h"
#include "../../analysis/induction_var.h"
#include "../../analysis/loop_info.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace core_ir {
namespace {

using analysis::InductionVar;
using analysis::NaturalLoop;

bool traceEnabled() {
  return sycEnvironmentValue("SYC_TRACE_LOOP_CONTINUE_BOUND") != nullptr;
}

void trace(const char* verdict, Function* func, const NaturalLoop& loop,
           const char* reason) {
  if (!traceEnabled())
    return;
  std::cerr << "[loop-continue-bound] " << verdict
            << " func=" << (func ? func->name() : std::string("<null>"))
            << " header="
            << (loop.header ? loop.header->name() : std::string("<null>"))
            << " reason=" << reason << "\n";
}

bool isConst(Value* value, int64_t expected) {
  auto* c = dynamic_cast<ConstInt*>(value);
  return c && c->value() == expected;
}

bool isIncrement(Value* value, Instruction* phi) {
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->opcode() != Opcode::Add || inst->numOperands() != 2)
    return false;
  return (inst->operand(0) == phi && isConst(inst->operand(1), 1)) ||
         (inst->operand(1) == phi && isConst(inst->operand(0), 1));
}

Value* incoming(Instruction* phi, BasicBlock* pred) {
  if (!phi || !pred)
    return nullptr;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2)
    if (phi->operand(i + 1) == pred)
      return phi->operand(i);
  return nullptr;
}

bool equivalentValue(Value* lhs, Value* rhs,
                     std::unordered_set<uint64_t>& active) {
  if (lhs == rhs)
    return true;
  auto* li = dynamic_cast<Instruction*>(lhs);
  auto* ri = dynamic_cast<Instruction*>(rhs);
  if (!li || !ri || li->opcode() != ri->opcode() ||
      li->numOperands() != ri->numOperands())
    return false;
  auto* lc = dynamic_cast<ConstInt*>(lhs);
  auto* rc = dynamic_cast<ConstInt*>(rhs);
  if (lc || rc)
    return lc && rc && lc->value() == rc->value();
  uint64_t key = (static_cast<uint64_t>(li->id()) << 32) |
                 static_cast<uint32_t>(ri->id());
  if (!active.insert(key).second)
    return true;
  for (int i = 0; i < li->numOperands(); ++i) {
    if (!equivalentValue(li->operand(i), ri->operand(i), active)) {
      active.erase(key);
      return false;
    }
  }
  active.erase(key);
  return true;
}

bool isPureBodyInstruction(Instruction* inst) {
  if (!inst || inst->isTerminator())
    return false;
  switch (inst->opcode()) {
  case Opcode::Phi:
  case Opcode::Alloca:
  case Opcode::Store:
  case Opcode::VStore:
  case Opcode::Call:
  case Opcode::Ret:
    return false;
  default:
    return true;
  }
}

struct Match {
  const NaturalLoop* loop = nullptr;
  BasicBlock* preheader = nullptr;
  BasicBlock* body = nullptr;
  BasicBlock* exit = nullptr;
  BasicBlock* skip = nullptr;
  BasicBlock* work = nullptr;
  Instruction* innerPhi = nullptr;
  Instruction* headerCond = nullptr;
  Instruction* guardCond = nullptr;
  Value* outerIv = nullptr;
};

bool findInnerPhi(const NaturalLoop& loop, BasicBlock* preheader,
                  Instruction*& outPhi) {
  if (!loop.header || !preheader)
    return false;
  for (auto& instPtr : loop.header->instructions()) {
    auto* phi = instPtr.get();
    if (!phi || phi->opcode() != Opcode::Phi)
      break;
    Value* init = incoming(phi, preheader);
    if (!isConst(init, 0))
      continue;
    bool allIncrements = true;
    bool sawLatch = false;
    for (auto* pred : loop.header->preds()) {
      if (!pred || !loop.blocks.count(pred))
        continue;
      sawLatch = true;
      if (!isIncrement(incoming(phi, pred), phi)) {
        allIncrements = false;
        break;
      }
    }
    if (sawLatch && allIncrements) {
      outPhi = phi;
      return true;
    }
  }
  return false;
}

// 识别"外层 IV 守卫内层计数"的循环形态：header 用内层归纳 phi 的 lt 比较退出，
// 体内首块再用外层 IV 对内层 phi 做 lt 守卫，分出 skip/work 两条回边路径。
bool matchLoop(const NaturalLoop& loop,
               const std::unordered_map<BasicBlock*, InductionVar>& outerIvs,
               Match& out) {
  if (!loop.header || loop.latches.size() < 2)
    return false;
  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader)
    return false;
  auto* headerTerm = loop.header->terminator();
  if (!headerTerm || headerTerm->opcode() != Opcode::CondBr ||
      headerTerm->numOperands() != 3)
    return false;
  auto* cmp = dynamic_cast<Instruction*>(headerTerm->operand(0));
  if (!cmp || cmp->opcode() != Opcode::ICmp || cmp->numOperands() != 2 ||
      cmp->name().rfind("lt", 0) != 0)
    return false;
  Instruction* innerPhi = nullptr;
  if (!findInnerPhi(loop, preheader, innerPhi))
    return false;
  if (cmp->operand(0) != innerPhi)
    return false;
  auto* body = dynamic_cast<BasicBlock*>(headerTerm->operand(1));
  auto* exit = dynamic_cast<BasicBlock*>(headerTerm->operand(2));
  if (!body || !exit || !loop.blocks.count(body) || loop.blocks.count(exit)) {
    body = dynamic_cast<BasicBlock*>(headerTerm->operand(2));
    exit = dynamic_cast<BasicBlock*>(headerTerm->operand(1));
  }
  if (!body || !exit || !loop.blocks.count(body) || loop.blocks.count(exit))
    return false;
  if (body->instructions().size() != 2)
    return false;
  auto* guard = body->instructions().front().get();
  auto* guardTerm = body->terminator();
  if (!guard || guard->opcode() != Opcode::ICmp || guard->numOperands() != 2 ||
      !guardTerm || guardTerm->opcode() != Opcode::CondBr ||
      guardTerm->numOperands() != 3 || guard->name().rfind("lt", 0) != 0)
    return false;
  Value* outer = guard->operand(0);
  if (guard->operand(1) != innerPhi)
    return false;
  bool outerFound = false;
  for (const auto& entry : outerIvs) {
    if (entry.second.phi == outer && entry.second.step == 1 &&
        dynamic_cast<ConstInt*>(entry.second.init) &&
        dynamic_cast<ConstInt*>(entry.second.init)->value() >= 0) {
      outerFound = true;
      break;
    }
  }
  if (!outerFound)
    return false;
  // guard 的两个后继必须都在循环内且都以回边结束，形成 skip/work 双路径。
  auto* skip = dynamic_cast<BasicBlock*>(guardTerm->operand(1));
  auto* work = dynamic_cast<BasicBlock*>(guardTerm->operand(2));
  if (!skip || !work || !loop.blocks.count(skip) || !loop.blocks.count(work))
    return false;
  auto* skipTerm = skip->terminator();
  auto* workTerm = work->terminator();
  if (!skipTerm || !workTerm || skipTerm->opcode() != Opcode::Br ||
      workTerm->opcode() != Opcode::Br || skipTerm->operand(0) != loop.header ||
      workTerm->operand(0) != loop.header)
    return false;
  // 被跳过路径只能计算与工作路径相同的循环携带值，从而保持指针和归纳变量
  // 状态不变。
  for (auto& instPtr : skip->instructions()) {
    if (instPtr.get() != skipTerm && !isPureBodyInstruction(instPtr.get()))
      return false;
  }
  for (auto& instPtr : loop.header->instructions()) {
    auto* phi = instPtr.get();
    if (!phi || phi->opcode() != Opcode::Phi)
      break;
    Value* skipVal = incoming(phi, skip);
    Value* workVal = incoming(phi, work);
    if (!skipVal || !workVal)
      return false;
    std::unordered_set<uint64_t> active;
    if (!equivalentValue(skipVal, workVal, active))
      return false;
  }
  out = {&loop, preheader, body, exit, skip, work, innerPhi, cmp, guard,
         outer};
  return true;
}

void removePhiIncoming(Instruction* phi, BasicBlock* pred) {
  if (!phi || !pred)
    return;
  std::vector<Value*> operands;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    if (phi->operand(i + 1) == pred)
      continue;
    operands.push_back(phi->operand(i));
    operands.push_back(phi->operand(i + 1));
  }
  phi->dropAllOperands();
  for (auto* operand : operands)
    phi->addOperand(operand);
}

bool applyMatch(Function* func, Match& match) {
  if (!func || !match.loop || !match.headerCond || !match.body ||
      !match.skip || !match.work)
    return false;
  auto* header = match.loop->header;
  auto* term = header->terminator();
  if (!term || term->opcode() != Opcode::CondBr)
    return false;
  auto* mod = func->parent();
  if (!mod)
    return false;
  // 后端把 <= 表示为交换操作数后的 >= 谓词：outer_iv >= inner_iv 等价于
  // inner_iv <= outer_iv。
  auto* tight = new Instruction(Opcode::ICmp, Type::i32(),
                                "ge_loop_continue_bound");
  tight->addOperand(match.outerIv);
  tight->addOperand(match.innerPhi);
  header->insertBefore(term, tight);
  auto* both = new Instruction(Opcode::And, Type::i32(),
                               "and.loop.continue.bound");
  both->addOperand(match.headerCond);
  both->addOperand(tight);
  header->insertBefore(term, both);
  term->setOperand(0, both);

  for (auto& instPtr : header->instructions()) {
    auto* phi = instPtr.get();
    if (!phi || phi->opcode() != Opcode::Phi)
      break;
    removePhiIncoming(phi, match.skip);
  }
  // 循环体只有 continue 测试，因此把工作边直接指向原工作块并丢弃现在不可达
  // 的两个块。
  BasicBlock* oldBody = match.body;
  term->setOperand(1, match.work);
  func->removeBlock(match.skip);
  func->removeBlock(oldBody);
  func->recomputeCFG();
  return true;
}

} // namespace

bool applyLoopContinueBound(Module& mod, analysis::AnalysisManager& am) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func || !func->entryBlock())
      continue;
    func->recomputeCFG();
    auto& cache = am.function(func);
    auto loops = cache.loops();
    std::unordered_map<BasicBlock*, InductionVar> outerIvs;
    for (const auto& loop : loops) {
      for (const auto& iv : cache.inductionVars(loop)) {
        if (iv.phi)
          outerIvs.emplace(iv.phi->parent(), iv);
      }
    }
    for (const auto& loop : loops) {
      Match match;
      if (!matchLoop(loop, outerIvs, match))
        continue;
      if (applyMatch(func, match)) {
        trace("apply", func, loop, "guard-to-bound");
        am.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
        changed = true;
        break;
      }
    }
  }
  if (changed)
    am.invalidateModule(analysis::AnalysisInvalidation::CFG);
  return changed;
}

} // namespace core_ir
