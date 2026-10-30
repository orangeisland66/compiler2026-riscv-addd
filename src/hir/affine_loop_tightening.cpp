#include "research_switch.h"
// 文件职责：收紧 affine 循环边界，生成更适合 interior fast path 的形状。本文件负责实现具体逻辑。
#include "affine_loop_tightening.h"

#include "../analysis/analysis_manager.h"
#include "../analysis/affine_region.h"
#include "../analysis/function_analysis.h"
#include "../analysis/affine_guard.h"
#include "../analysis/cfg_utils.h"
#include "../analysis/dominators.h"
#include "../analysis/induction_var.h"
#include "../analysis/loop_info.h"
#include "../analysis/loop_value_utils.h"

#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hir {

namespace {

using analysis::AffineExprSummary;
using analysis::AffineGuardSummary;
using analysis::AffineRegionPlan;
using analysis::InductionVar;
using analysis::NaturalLoop;
using core_ir::BasicBlock;
using core_ir::ConstInt;
using core_ir::Function;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Type;
using core_ir::Value;

struct CanonicalExitLoop {
  BasicBlock* preheader = nullptr;
  BasicBlock* header = nullptr;
  BasicBlock* latch = nullptr;
  BasicBlock* exit = nullptr;
  Instruction* cmp = nullptr;
  Instruction* condBr = nullptr;
  Instruction* inductionValue = nullptr;
  Value* inductionBasis = nullptr;
  Value* initVal = nullptr;
  Value* boundVal = nullptr;
  std::string relation;
};

Instruction* makeInstruction(Opcode op, Type ty, const std::string& name = "") {
  return new Instruction(op, ty, name);
}

bool traceTighteningEnabled() {
  return sycEnvironmentValue("AFFINE_TIGHTEN_TRACE") != nullptr;
}

void traceTightening(const char* verdict, Function* func, const NaturalLoop& loop,
                     const InductionVar& iv, const AffineGuardSummary* guard,
                     const char* reason) {
  if (!traceTighteningEnabled())
    return;
  std::cerr << "[affine-tighten] " << verdict
            << " func=" << (func ? func->name() : "<null>")
            << " header=" << (loop.header ? loop.header->name() : "<null>")
            << " iv=" << (iv.phi ? iv.phi->id() : (iv.storage ? iv.storage->id() : -1));
  if (guard) {
    std::cerr << " guard_bb=" << (guard->block ? guard->block->name() : "<null>")
              << " rel=" << guard->relation
              << " cmp=" << (guard->compare ? guard->compare->id() : -1)
              << " g_iv=" << (guard->induction ? guard->induction->id() : -1)
              << " work=" << (guard->workBlock ? guard->workBlock->name() : "<null>")
              << " cont=" << (guard->continueBlock ? guard->continueBlock->name() : "<null>");
  }
  std::cerr << " reason=" << reason << "\n";
}

Instruction* insertBinaryBefore(BasicBlock* bb, Instruction* pos, Opcode op,
                                Type ty, Value* lhs, Value* rhs,
                                const std::string& name = "") {
  auto* inst = makeInstruction(op, ty, name);
  inst->addOperand(lhs);
  inst->addOperand(rhs);
  bb->insertBefore(pos, inst);
  return inst;
}

Instruction* insertUnaryBefore(BasicBlock* bb, Instruction* pos, Opcode op,
                               Type ty, Value* operand,
                               const std::string& name = "") {
  auto* inst = makeInstruction(op, ty, name);
  inst->addOperand(operand);
  bb->insertBefore(pos, inst);
  return inst;
}

std::string cmpCond(Instruction* inst) {
  if (!inst)
    return "";
  auto name = inst->name();
  auto pos = name.find('_');
  return pos == std::string::npos ? name : name.substr(0, pos);
}

bool isLoopInvariantValue(Value* val, const NaturalLoop& loop,
                          std::unordered_map<int, bool>& memo) {
  if (!val)
    return false;
  if (val->isConstant() || dynamic_cast<core_ir::Argument*>(val) ||
      dynamic_cast<core_ir::GlobalVar*>(val) ||
      dynamic_cast<BasicBlock*>(val)) {
    return true;
  }

  auto it = memo.find(val->id());
  if (it != memo.end())
    return it->second;

  auto* inst = dynamic_cast<Instruction*>(val);
  if (!inst || !inst->parent()) {
    memo[val->id()] = true;
    return true;
  }
  if (!loop.blocks.count(inst->parent())) {
    memo[val->id()] = true;
    return true;
  }
  if (inst->opcode() == Opcode::Load && inst->numOperands() == 1 &&
      analysis::isScalarSlot(inst->operand(0)) &&
      !analysis::loopStoresToSlot(loop, inst->operand(0))) {
    memo[val->id()] = true;
    return true;
  }
  if (!analysis::isSimpleInvariantExpr(inst)) {
    memo[val->id()] = false;
    return false;
  }

  memo[val->id()] = false;
  for (auto* op : inst->operands()) {
    if (!isLoopInvariantValue(op, loop, memo))
      return false;
  }
  memo[val->id()] = true;
  return true;
}

void setCmpCond(Instruction* cmp, const std::string& cond) {
  if (!cmp)
    return;
  auto name = cmp->name();
  auto pos = name.find('_');
  std::string suffix =
      pos == std::string::npos ? "_" + std::to_string(cmp->id())
                               : name.substr(pos);
  cmp->setName(cond + suffix);
}

bool isI32Value(Value* val) {
  return val && val->type().isI32();
}

bool loopContainsCall(const NaturalLoop& loop) {
  for (auto* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      if (instPtr && instPtr->opcode() == Opcode::Call)
        return true;
    }
  }
  return false;
}

Value* constI32(Module& mod, int64_t value) {
  return mod.getConstInt(static_cast<int32_t>(value));
}

Value* scaleByConst(Module& mod, BasicBlock* bb, Instruction* pos, Value* value,
                    int64_t coeff, const std::string& stem) {
  if (!value || !isI32Value(value) || coeff == 0)
    return nullptr;
  if (coeff == 1)
    return value;
  if (coeff == -1)
    return insertUnaryBefore(bb, pos, Opcode::Neg, Type::i32(), value,
                             stem + ".neg");

  int64_t absCoeff = coeff < 0 ? -coeff : coeff;
  Value* factor = constI32(mod, absCoeff);
  Value* mul =
      insertBinaryBefore(bb, pos, Opcode::Mul, Type::i32(), value, factor,
                         stem + ".mul");
  if (coeff < 0)
    return insertUnaryBefore(bb, pos, Opcode::Neg, Type::i32(), mul,
                             stem + ".neg");
  return mul;
}

Value* addIfNeeded(Module& mod, BasicBlock* bb, Instruction* pos, Value* acc,
                   Value* term, const std::string& stem) {
  if (!term)
    return acc;
  if (!acc)
    return term;
  return insertBinaryBefore(bb, pos, Opcode::Add, Type::i32(), acc, term, stem);
}

Value* cloneInvariantExpr(Value* val, BasicBlock* preheader,
                          Instruction* preheaderTerm, const NaturalLoop& loop,
                          std::unordered_map<int, Value*>& cache) {
  if (!val)
    return nullptr;
  if (val->isConstant() || dynamic_cast<core_ir::Argument*>(val) ||
      dynamic_cast<core_ir::GlobalVar*>(val) ||
      dynamic_cast<BasicBlock*>(val)) {
    return val;
  }

  auto it = cache.find(val->id());
  if (it != cache.end())
    return it->second;

  auto* inst = dynamic_cast<Instruction*>(val);
  if (!inst || !inst->parent() || !loop.blocks.count(inst->parent()))
    return val;
  if (inst->opcode() == Opcode::Load && inst->numOperands() == 1 &&
      analysis::isScalarSlot(inst->operand(0)) &&
      !analysis::loopStoresToSlot(loop, inst->operand(0))) {
    auto* clone = makeInstruction(Opcode::Load, inst->type(),
                                  inst->name().empty() ? "affine.tight.inv.load"
                                                       : inst->name() + ".affine.tight");
    clone->addOperand(inst->operand(0));
    preheader->insertBefore(preheaderTerm, clone);
    cache[val->id()] = clone;
    return clone;
  }

  std::unordered_map<int, bool> memo;
  if (!isLoopInvariantValue(val, loop, memo) ||
      !analysis::isSimpleInvariantExpr(inst))
    return nullptr;

  auto* clone = makeInstruction(
      inst->opcode(), inst->type(),
      inst->name().empty() ? "affine.tight.inv" : inst->name() + ".affine.tight");
  for (auto* op : inst->operands()) {
    Value* clonedOp =
        cloneInvariantExpr(op, preheader, preheaderTerm, loop, cache);
    if (!clonedOp)
      return nullptr;
    clone->addOperand(clonedOp);
  }
  preheader->insertBefore(preheaderTerm, clone);
  cache[val->id()] = clone;
  return clone;
}

Value* buildAffineExprValue(Module& mod, BasicBlock* bb, Instruction* pos,
                            const NaturalLoop& loop, const AffineExprSummary& expr,
                            const std::string& stem) {
  if (!expr.valid || !expr.ivTerms.empty() || !expr.scaledIvTerms.empty())
    return nullptr;

  Value* acc = nullptr;
  if (expr.constant != 0)
    acc = constI32(mod, expr.constant);
  std::unordered_map<int, Value*> cloneCache;
  int termIndex = 0;
  for (const auto& term : expr.invariantTerms) {
    if (!isI32Value(term.basis))
      return nullptr;
    Value* materialized =
        cloneInvariantExpr(term.basis, bb, pos, loop, cloneCache);
    if (!materialized)
      return nullptr;
    Value* scaled = scaleByConst(mod, bb, pos, materialized, term.coeff,
                                 stem + ".term" + std::to_string(termIndex++));
    acc = addIfNeeded(mod, bb, pos, acc, scaled, stem + ".add");
  }
  if (!acc)
    acc = constI32(mod, 0);
  return acc;
}

Value* addConst(Module& mod, BasicBlock* bb, Instruction* pos, Value* base,
                int64_t delta, const std::string& stem) {
  if (!base || !isI32Value(base))
    return nullptr;
  if (delta == 0)
    return base;
  if (auto* ci = dynamic_cast<ConstInt*>(base))
    return constI32(mod, static_cast<int64_t>(ci->value()) + delta);
  return insertBinaryBefore(bb, pos, Opcode::Add, Type::i32(), base,
                            constI32(mod, delta), stem);
}

Value* buildLoopExclusiveBoundValue(Module& mod, BasicBlock* bb,
                                    Instruction* pos,
                                    const NaturalLoop& loop,
                                    const CanonicalExitLoop& loopInfo) {
  if (!bb || !pos || !loopInfo.boundVal)
    return nullptr;
  std::unordered_map<int, Value*> cloneCache;
  Value* exclusive =
      cloneInvariantExpr(loopInfo.boundVal, bb, pos, loop, cloneCache);
  if (!exclusive)
    return nullptr;
  if (loopInfo.relation == "le") {
    exclusive = addConst(mod, bb, pos, exclusive, 1,
                         "affine.tight.exclusive_bound");
    if (!exclusive)
      return nullptr;
  }
  return exclusive;
}

Value* buildGuardWorkCondition(Module& mod, BasicBlock* bb, Instruction* pos,
                               const NaturalLoop& loop,
                               const AffineGuardSummary& guard) {
  if (!bb || !pos || !guard.valid || !guard.induction || !isI32Value(guard.induction))
    return nullptr;
  std::unordered_map<int, bool> memo;
  if (!isLoopInvariantValue(guard.induction, loop, memo))
    return nullptr;
  Value* bound = buildAffineExprValue(mod, bb, pos, loop, guard.boundExpr,
                                      "affine.tight.guard.bound");
  if (!bound)
    return nullptr;
  auto* cmp = insertBinaryBefore(bb, pos, Opcode::ICmp, Type::i32(),
                                 guard.induction, bound,
                                 "affine.tight.guard.cmp");
  setCmpCond(cmp, guard.relation);

  // 能做 zero-trip versioning 的 invariant guard 必须已经有
  // canonical continue block；此时 guard.relation 描述的是“本轮没有任何
  // inner-iteration 会执行 work”的失败条件。preheader 分支需要的则是
  // work 条件，所以这里统一取反。原始 guard compare 在 work path 上的
  // 真值仍由 compareTrueForWork 决定，rewriteGuardToWork 继续负责替换。
  auto* workCond = insertBinaryBefore(bb, pos, Opcode::ICmp, Type::i32(), cmp,
                                      constI32(mod, 0),
                                      "affine.tight.guard.work");
  setCmpCond(workCond, "eq");
  return workCond;
}

Instruction* firstNonPhi(BasicBlock* bb) {
  if (!bb)
    return nullptr;
  for (auto& instPtr : bb->instructions()) {
    if (instPtr->opcode() != Opcode::Phi)
      return instPtr.get();
  }
  return nullptr;
}

bool replacePhiIncomingPred(BasicBlock* bb, BasicBlock* oldPred,
                            BasicBlock* newPred) {
  if (!bb || !oldPred || !newPred)
    return false;
  bool changed = false;
  for (auto& instPtr : bb->instructions()) {
    Instruction* phi = instPtr.get();
    if (phi->opcode() != Opcode::Phi)
      break;
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
      if (pred == oldPred) {
        phi->setOperand(i + 1, newPred);
        changed = true;
      }
    }
  }
  return changed;
}

Value* localizePhiIncoming(Value* value, BasicBlock* incomingBlock,
                           Instruction* insertPos,
                           const std::string& name) {
  if (!value || !incomingBlock || !insertPos)
    return nullptr;
  if (value->isConstant() || dynamic_cast<core_ir::Argument*>(value) ||
      dynamic_cast<core_ir::GlobalVar*>(value) ||
      dynamic_cast<BasicBlock*>(value)) {
    return value;
  }
  auto* inst = dynamic_cast<Instruction*>(value);
  if (inst && inst->parent() == incomingBlock)
    return value;

  auto* mov = makeInstruction(Opcode::Mov, value->type(), name);
  mov->addOperand(value);
  incomingBlock->insertBefore(insertPos, mov);
  return mov;
}

bool isContinueTrampoline(const NaturalLoop& loop, BasicBlock* block) {
  if (!block)
    return false;
  if (block == loop.header)
    return true;
  if (!loop.blocks.count(block))
    return false;
  Instruction* term = block->terminator();
  if (!term || term->opcode() != Opcode::Br || term->numOperands() != 1)
    return false;
  if (dynamic_cast<BasicBlock*>(term->operand(0)) != loop.header)
    return false;
  for (auto& instPtr : block->instructions()) {
    if (instPtr.get()->isTerminator())
      continue;
    return false;
  }
  return true;
}

bool isDirectScalarSlotLoad(Value* value, Instruction* slot) {
  auto* load = dynamic_cast<Instruction*>(value);
  return load && load->opcode() == Opcode::Load && load->numOperands() == 1 &&
         load->operand(0) == slot;
}

bool matchAddSubStep(Value* value, Instruction* slot, int64_t& step) {
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->numOperands() != 2)
    return false;
  if (inst->opcode() != Opcode::Add && inst->opcode() != Opcode::Sub)
    return false;

  int sign = 1;
  Value* other = nullptr;
  if (isDirectScalarSlotLoad(inst->operand(0), slot)) {
    other = inst->operand(1);
    if (inst->opcode() == Opcode::Sub)
      sign = -1;
  } else if (inst->opcode() == Opcode::Add &&
             isDirectScalarSlotLoad(inst->operand(1), slot)) {
    other = inst->operand(0);
  } else {
    return false;
  }

  auto* ci = dynamic_cast<ConstInt*>(other);
  if (!ci)
    return false;
  step = static_cast<int64_t>(ci->value()) * sign;
  return true;
}

bool isCanonicalLatchBlock(const NaturalLoop& loop, const CanonicalExitLoop& cl,
                           const InductionVar& iv, BasicBlock* block) {
  if (!block || block != cl.latch || !loop.blocks.count(block))
    return false;
  Instruction* term = block->terminator();
  if (!term || term->opcode() != Opcode::Br || term->numOperands() != 1)
    return false;
  if (dynamic_cast<BasicBlock*>(term->operand(0)) != loop.header)
    return false;
  if (!iv.storage)
    return true;

  bool sawUpdateStore = false;
  for (auto& instPtr : block->instructions()) {
    Instruction* inst = instPtr.get();
    if (inst->isTerminator())
      continue;
    if (inst->opcode() == Opcode::Load && inst->numOperands() == 1 &&
        inst->operand(0) == iv.storage) {
      continue;
    }
    if ((inst->opcode() == Opcode::Add || inst->opcode() == Opcode::Sub) &&
        inst->numOperands() == 2) {
      int64_t step = 0;
      if (matchAddSubStep(inst, iv.storage, step) && step == iv.step)
        continue;
    }
    if (inst->opcode() == Opcode::Store && inst->numOperands() == 2 &&
        inst->operand(1) == iv.storage) {
      int64_t step = 0;
      if (matchAddSubStep(inst->operand(0), iv.storage, step) &&
          step == iv.step) {
        sawUpdateStore = true;
        continue;
      }
    }
    return false;
  }
  return sawUpdateStore;
}

bool isPhiOnlyBranchBlock(BasicBlock* block, BasicBlock*& succOut) {
  succOut = nullptr;
  if (!block)
    return false;
  Instruction* term = block->terminator();
  if (!term || term->opcode() != Opcode::Br || term->numOperands() != 1)
    return false;
  succOut = dynamic_cast<BasicBlock*>(term->operand(0));
  if (!succOut)
    return false;
  for (auto& instPtr : block->instructions()) {
    Instruction* inst = instPtr.get();
    if (inst->isTerminator())
      continue;
    if (inst->opcode() != Opcode::Phi)
      return false;
  }
  return true;
}

bool isCanonicalContinuePath(const NaturalLoop& loop,
                             const CanonicalExitLoop& cl,
                             const InductionVar& iv,
                             BasicBlock* block,
                             std::unordered_set<int>& visited) {
  if (isContinueTrampoline(loop, block))
    return true;
  if (isCanonicalLatchBlock(loop, cl, iv, block))
    return true;
  if (!block || !loop.blocks.count(block) ||
      !visited.insert(block->id()).second) {
    return false;
  }

  if (iv.storage) {
    Instruction* term = block->terminator();
    if (term && term->opcode() == Opcode::Br && term->numOperands() == 1 &&
        dynamic_cast<BasicBlock*>(term->operand(0)) == loop.header) {
      bool sawUpdateStore = false;
      for (auto& instPtr : block->instructions()) {
        Instruction* inst = instPtr.get();
        if (inst->isTerminator())
          continue;
        if (inst->opcode() == Opcode::Load && inst->numOperands() == 1 &&
            inst->operand(0) == iv.storage) {
          continue;
        }
        if ((inst->opcode() == Opcode::Add || inst->opcode() == Opcode::Sub) &&
            inst->numOperands() == 2) {
          int64_t step = 0;
          if (matchAddSubStep(inst, iv.storage, step) && step == iv.step)
            continue;
        }
        if (inst->opcode() == Opcode::Store && inst->numOperands() == 2 &&
            inst->operand(1) == iv.storage) {
          int64_t step = 0;
          if (matchAddSubStep(inst->operand(0), iv.storage, step) &&
              step == iv.step) {
            sawUpdateStore = true;
            continue;
          }
        }
        sawUpdateStore = false;
        break;
      }
      if (sawUpdateStore)
        return true;
    }
  }

  BasicBlock* succ = nullptr;
  if (!isPhiOnlyBranchBlock(block, succ) || !succ || !loop.blocks.count(succ))
    return false;
  return isCanonicalContinuePath(loop, cl, iv, succ, visited);
}

bool isCanonicalContinueBlock(const NaturalLoop& loop,
                              const CanonicalExitLoop& cl,
                              const InductionVar& iv,
                              BasicBlock* block) {
  std::unordered_set<int> visited;
  return isCanonicalContinuePath(loop, cl, iv, block, visited);
}

bool recognizeCanonicalExitLoop(const NaturalLoop& loop, const InductionVar& iv,
                                CanonicalExitLoop& out) {
  if (!loop.header || !iv.isCanonical || iv.step != 1 ||
      loop.latches.size() != 1) {
    return false;
  }
  BasicBlock* latch = loop.latches.front();
  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader)
    return false;
  Instruction* term = loop.header->terminator();
  if (!term || term->opcode() != Opcode::CondBr || term->numOperands() != 3)
    return false;
  auto* cmp = dynamic_cast<Instruction*>(term->operand(0));
  auto* succTrue = dynamic_cast<BasicBlock*>(term->operand(1));
  auto* succFalse = dynamic_cast<BasicBlock*>(term->operand(2));
  if (!cmp || cmp->opcode() != Opcode::ICmp || !succTrue || !succFalse)
    return false;

  Value* lhs = cmp->operand(0);
  Value* rhs = cmp->operand(1);
  Value* inductionBasis = iv.phi ? static_cast<Value*>(iv.phi)
                                 : static_cast<Value*>(iv.storage);
  if (!inductionBasis)
    return false;
  auto headerLoadForSlot = [&](Value* value) -> Instruction* {
    if (!iv.storage)
      return nullptr;
    auto* load = dynamic_cast<Instruction*>(value);
    if (!load || load->opcode() != Opcode::Load || load->numOperands() != 1)
      return nullptr;
    if (load->operand(0) != iv.storage || load->parent() != loop.header)
      return nullptr;
    return load;
  };
  auto matchInductionValue = [&](Value* value) -> Instruction* {
    if (value == iv.phi)
      return iv.phi;
    return headerLoadForSlot(value);
  };

  Value* bound = nullptr;
  Instruction* inductionValue = nullptr;
  std::string relation = cmpCond(cmp);
  if ((inductionValue = matchInductionValue(lhs))) {
    bound = rhs;
  } else if ((inductionValue = matchInductionValue(rhs))) {
    bound = lhs;
    if (relation == "lt")
      relation = "gt";
    else if (relation == "le")
      relation = "ge";
    else if (relation == "gt")
      relation = "lt";
    else if (relation == "ge")
      relation = "le";
  } else {
    return false;
  }
  if (relation != "lt" && relation != "le")
    return false;

  Value* initVal = iv.init;
  if (iv.phi) {
    initVal = nullptr;
    for (int i = 0; i + 1 < iv.phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<BasicBlock*>(iv.phi->operand(i + 1));
      if (pred == preheader) {
        initVal = iv.phi->operand(i);
        break;
      }
    }
  }
  if (!initVal || !isI32Value(bound))
    return false;
  std::unordered_map<int, bool> boundMemo;
  if (!isLoopInvariantValue(bound, loop, boundMemo))
    return false;

  BasicBlock* exit = loop.blocks.count(succTrue) ? succFalse : succTrue;
  if (!exit || loop.blocks.count(exit))
    return false;

  out.preheader = preheader;
  out.header = loop.header;
  out.latch = latch;
  out.exit = exit;
  out.cmp = cmp;
  out.condBr = term;
  out.inductionValue = inductionValue;
  out.inductionBasis = inductionBasis;
  out.initVal = initVal;
  out.boundVal = bound;
  out.relation = relation;
  return true;
}

bool finalizeConstGuardBranch(BasicBlock* block) {
  if (!block)
    return false;
  Instruction* term = block->terminator();
  if (!term || term->opcode() != Opcode::CondBr || term->numOperands() != 3)
    return false;
  auto* cond = dynamic_cast<ConstInt*>(term->operand(0));
  auto* succTrue = dynamic_cast<BasicBlock*>(term->operand(1));
  auto* succFalse = dynamic_cast<BasicBlock*>(term->operand(2));
  if (!cond || !succTrue || !succFalse)
    return false;
  block->remove(term);
  auto* br = makeInstruction(Opcode::Br, Type::voidTy());
  br->addOperand(cond->value() != 0 ? succTrue : succFalse);
  block->pushBack(br);
  return true;
}

void eraseDeadGuardLocalInst(BasicBlock* block, Instruction* inst) {
  if (!block || !inst || inst->parent() != block || inst->isTerminator() ||
      !inst->uses().empty()) {
    return;
  }
  std::vector<Instruction*> operands;
  for (auto* op : inst->operands()) {
    if (auto* opInst = dynamic_cast<Instruction*>(op))
      operands.push_back(opInst);
  }
  inst->dropAllOperands();
  block->remove(inst);
  for (auto* opInst : operands)
    eraseDeadGuardLocalInst(block, opInst);
}

Value* simplifyGuardBoolExpr(Module& mod, BasicBlock* block, Value* value) {
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!block || !inst || inst->parent() != block || inst->isTerminator())
    return value;

  for (int i = 0; i < inst->numOperands(); ++i) {
    Value* simplified = simplifyGuardBoolExpr(mod, block, inst->operand(i));
    if (simplified != inst->operand(i))
      inst->setOperand(i, simplified);
  }

  auto asBoolConst = [](Value* v) -> ConstInt* {
    auto* ci = dynamic_cast<ConstInt*>(v);
    if (!ci)
      return nullptr;
    return ci;
  };

  auto replaceWith = [&](Value* replacement) -> Value* {
    inst->replaceAllUsesWith(replacement);
    eraseDeadGuardLocalInst(block, inst);
    return replacement;
  };

  if (inst->opcode() == Opcode::And && inst->numOperands() == 2) {
    Value* lhs = inst->operand(0);
    Value* rhs = inst->operand(1);
    if (lhs == rhs)
      return replaceWith(lhs);
    if (auto* lhsConst = asBoolConst(lhs)) {
      if (lhsConst->value() == 0)
        return replaceWith(mod.getConstInt(0));
      return replaceWith(rhs);
    }
    if (auto* rhsConst = asBoolConst(rhs)) {
      if (rhsConst->value() == 0)
        return replaceWith(mod.getConstInt(0));
      return replaceWith(lhs);
    }
  }

  if (inst->opcode() == Opcode::Or && inst->numOperands() == 2) {
    Value* lhs = inst->operand(0);
    Value* rhs = inst->operand(1);
    if (lhs == rhs)
      return replaceWith(lhs);
    if (auto* lhsConst = asBoolConst(lhs)) {
      if (lhsConst->value() != 0)
        return replaceWith(mod.getConstInt(1));
      return replaceWith(rhs);
    }
    if (auto* rhsConst = asBoolConst(rhs)) {
      if (rhsConst->value() != 0)
        return replaceWith(mod.getConstInt(1));
      return replaceWith(lhs);
    }
  }

  return inst;
}

bool rewriteGuardToWork(Module& mod, const AffineGuardSummary& guard) {
  BasicBlock* block = guard.block;
  if (!block || !guard.workBlock || !guard.compare)
    return false;
  Instruction* term = block->terminator();
  if (!term || term->opcode() != Opcode::CondBr || term->numOperands() != 3)
    return false;

  if (term->operand(0) == guard.compare) {
    block->remove(term);
    auto* br = makeInstruction(Opcode::Br, Type::voidTy());
    br->addOperand(guard.workBlock);
    block->pushBack(br);
    return true;
  }

  // 当 guard 是多个 affine compare 的 conjunction 时，
  // tightening 只证明当前 compare 在 work 路径上的真值，并把它从 guard
  // 树里剥掉；剩余 compare 交给后续 cleanup + 下一轮 tightening 继续处理。
  Instruction* root = dynamic_cast<Instruction*>(term->operand(0));
  guard.compare->replaceAllUsesWith(
      mod.getConstInt(guard.compareTrueForWork ? 1 : 0));
  guard.compare->dropAllOperands();
  if (guard.compare->parent() == block)
    block->remove(guard.compare);
  if (root)
    simplifyGuardBoolExpr(mod, block, root);
  finalizeConstGuardBranch(block);
  return true;
}

bool createBoundPhiPreheader(Module& mod, Function* func,
                             const NaturalLoop& loop,
                             const CanonicalExitLoop& loopInfo,
                             Value* tightExclusive, Value*& outBound) {
  if (!func || !loopInfo.preheader || !loopInfo.header || !tightExclusive ||
      !loopInfo.boundVal) {
    return false;
  }

  BasicBlock* preheader = loopInfo.preheader;
  Instruction* preTerm = preheader->terminator();
  if (!preTerm || preTerm->opcode() != Opcode::Br || preTerm->numOperands() != 1 ||
      dynamic_cast<BasicBlock*>(preTerm->operand(0)) != loopInfo.header) {
    return false;
  }

  std::unordered_map<int, Value*> cloneCache;
  Value* origExclusive =
      cloneInvariantExpr(loopInfo.boundVal, preheader, preTerm, loop, cloneCache);
  if (!origExclusive)
    return false;
  if (loopInfo.relation == "le") {
    origExclusive = addConst(mod, preheader, preTerm, origExclusive, 1,
                             "affine.tight.orig_exclusive");
    if (!origExclusive)
      return false;
  }

  auto* cmp = insertBinaryBefore(preheader, preTerm, Opcode::ICmp, Type::i32(),
                                 tightExclusive, origExclusive,
                                 "lt_affine.tight.choose");
  setCmpCond(cmp, "lt");

  BasicBlock* tightBB = func->newBlock("affine.interior.bound");
  BasicBlock* keepBB = func->newBlock("affine.remainder.bound");
  BasicBlock* mergeBB = func->newBlock("affine.version.preheader");

  preheader->remove(preTerm);
  auto* condBr = makeInstruction(Opcode::CondBr, Type::voidTy());
  condBr->addOperand(cmp);
  condBr->addOperand(tightBB);
  condBr->addOperand(keepBB);
  preheader->pushBack(condBr);

  auto* tightBr = makeInstruction(Opcode::Br, Type::voidTy());
  tightBr->addOperand(mergeBB);
  tightBB->pushBack(tightBr);

  auto* keepBr = makeInstruction(Opcode::Br, Type::voidTy());
  keepBr->addOperand(mergeBB);
  keepBB->pushBack(keepBr);

  auto* boundPhi =
      makeInstruction(Opcode::Phi, Type::i32(), "affine.interior.bound");
  Value* tightIncoming =
      localizePhiIncoming(tightExclusive, tightBB, tightBr,
                          "affine.bound.incoming");
  Value* keepIncoming =
      localizePhiIncoming(origExclusive, keepBB, keepBr,
                          "affine.bound.incoming");
  if (!tightIncoming || !keepIncoming)
    return false;
  boundPhi->addOperand(tightIncoming);
  boundPhi->addOperand(tightBB);
  boundPhi->addOperand(keepIncoming);
  boundPhi->addOperand(keepBB);
  mergeBB->pushFront(boundPhi);

  auto* mergeBr = makeInstruction(Opcode::Br, Type::voidTy());
  mergeBr->addOperand(loopInfo.header);
  mergeBB->pushBack(mergeBr);

  replacePhiIncomingPred(loopInfo.header, preheader, mergeBB);
  outBound = boundPhi;
  return true;
}

BasicBlock* followContinuePathPredecessor(const NaturalLoop& loop,
                                          BasicBlock* continueBlock,
                                          BasicBlock* targetBlock) {
  if (!continueBlock || !targetBlock)
    return nullptr;
  if (continueBlock == targetBlock)
    return nullptr;

  std::unordered_set<int> visited;
  BasicBlock* current = continueBlock;
  while (current && visited.insert(current->id()).second) {
    Instruction* term = current->terminator();
    if (!term || term->opcode() != Opcode::Br || term->numOperands() != 1)
      return nullptr;
    auto* succ = dynamic_cast<BasicBlock*>(term->operand(0));
    if (!succ || !loop.blocks.count(succ))
      return nullptr;
    if (succ == targetBlock)
      return current;
    BasicBlock* nextSucc = nullptr;
    if (!isPhiOnlyBranchBlock(succ, nextSucc))
      return nullptr;
    current = succ;
  }
  return nullptr;
}

enum class TightInitHeaderPhiMode {
  Stable,
  AddConst,
  GepConst,
  Unsupported,
};

struct TightInitHeaderPhiPlan {
  Instruction* phi = nullptr;
  int preheaderOperandIndex = -1;
  Value* originalInit = nullptr;
  TightInitHeaderPhiMode mode = TightInitHeaderPhiMode::Unsupported;
  int64_t step = 0;
};

bool classifyTightInitHeaderPhi(const NaturalLoop& loop,
                                BasicBlock* continueBlock,
                                Instruction* phi,
                                int preheaderOperandIndex,
                                BasicBlock* latch,
                                TightInitHeaderPhiPlan& out) {
  if (!phi || preheaderOperandIndex < 0 || !latch)
    return false;
  out.phi = phi;
  out.preheaderOperandIndex = preheaderOperandIndex;
  out.originalInit = phi->operand(preheaderOperandIndex);
  out.mode = TightInitHeaderPhiMode::Unsupported;

  Value* nextValue = nullptr;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
    if (pred == latch) {
      nextValue = phi->operand(i);
      break;
    }
  }
  if (!nextValue)
    return false;

  if (nextValue == phi) {
    out.mode = TightInitHeaderPhiMode::Stable;
    return true;
  }

  if (auto* nextInst = dynamic_cast<Instruction*>(nextValue)) {
    if ((nextInst->opcode() == Opcode::Add || nextInst->opcode() == Opcode::Sub) &&
        nextInst->numOperands() == 2) {
      auto* lhsConst = dynamic_cast<ConstInt*>(nextInst->operand(0));
      auto* rhsConst = dynamic_cast<ConstInt*>(nextInst->operand(1));
      if (nextInst->operand(0) == phi && rhsConst) {
        out.mode = TightInitHeaderPhiMode::AddConst;
        out.step = nextInst->opcode() == Opcode::Sub ? -rhsConst->value()
                                                     : rhsConst->value();
        return true;
      }
      if (nextInst->opcode() == Opcode::Add &&
          nextInst->operand(1) == phi && lhsConst) {
        out.mode = TightInitHeaderPhiMode::AddConst;
        out.step = lhsConst->value();
        return true;
      }
    }

    if (nextInst->opcode() == Opcode::GEP && nextInst->numOperands() == 2 &&
        nextInst->operand(0) == phi) {
      if (auto* stepConst = dynamic_cast<ConstInt*>(nextInst->operand(1))) {
        out.mode = TightInitHeaderPhiMode::GepConst;
        out.step = stepConst->value();
        return true;
      }
    }

    if (nextInst->opcode() == Opcode::Phi && nextInst->parent() &&
        loop.blocks.count(nextInst->parent())) {
      BasicBlock* continuePred = nullptr;
      if (continueBlock == nextInst->parent()) {
        for (int i = 0; i + 1 < nextInst->numOperands(); i += 2) {
          auto* pred = dynamic_cast<BasicBlock*>(nextInst->operand(i + 1));
          if (pred && loop.blocks.count(pred) && pred != latch &&
              nextInst->operand(i) == phi) {
            out.mode = TightInitHeaderPhiMode::Stable;
            return true;
          }
        }
      }
      continuePred =
          followContinuePathPredecessor(loop, continueBlock, nextInst->parent());
      if (!continuePred)
        return false;
      for (int i = 0; i + 1 < nextInst->numOperands(); i += 2) {
        auto* pred = dynamic_cast<BasicBlock*>(nextInst->operand(i + 1));
        if (pred == continuePred && nextInst->operand(i) == phi) {
          out.mode = TightInitHeaderPhiMode::Stable;
          return true;
        }
      }
    }
  }

  return false;
}

Value* buildTightenedHeaderPhiInit(Module& mod,
                                   BasicBlock* tightBB,
                                   Instruction* insertPos,
                                   const TightInitHeaderPhiPlan& plan,
                                   Value* deltaI32) {
  if (!tightBB || !insertPos || !deltaI32 || !plan.originalInit)
    return nullptr;
  if (plan.mode == TightInitHeaderPhiMode::Stable)
    return plan.originalInit;
  if (!isI32Value(deltaI32))
    return nullptr;

  if (plan.mode == TightInitHeaderPhiMode::AddConst) {
    if (!isI32Value(plan.originalInit))
      return nullptr;
    Value* deltaStep = scaleByConst(mod, tightBB, insertPos, deltaI32,
                                    plan.step, "affine.tight.init.delta");
    if (!deltaStep)
      return nullptr;
    return addIfNeeded(mod, tightBB, insertPos, plan.originalInit, deltaStep,
                       "affine.tight.init.add");
  }

  if (plan.mode == TightInitHeaderPhiMode::GepConst) {
    Value* deltaStep = deltaI32;
    if (plan.step != 1) {
      deltaStep = scaleByConst(mod, tightBB, insertPos, deltaI32, plan.step,
                               "affine.tight.init.gep.step");
      if (!deltaStep)
        return nullptr;
    }
    auto* gep = makeInstruction(Opcode::GEP, plan.phi->type(),
                                "affine.tight.init.gep");
    gep->addOperand(plan.originalInit);
    gep->addOperand(deltaStep);
    tightBB->insertBefore(insertPos, gep);
    return gep;
  }

  return nullptr;
}

bool createInitPhiPreheader(Module& mod, Function* func,
                            const NaturalLoop& loop,
                            const CanonicalExitLoop& loopInfo,
                            Instruction* inductionPhi,
                            Value* tightInit,
                            BasicBlock* continueBlock) {
  if (!func || !loopInfo.preheader || !loopInfo.header || !inductionPhi ||
      !tightInit || !loopInfo.initVal) {
    return false;
  }

  int phiPreheaderIndex = -1;
  for (int i = 0; i + 1 < inductionPhi->numOperands(); i += 2) {
    auto* pred = dynamic_cast<BasicBlock*>(inductionPhi->operand(i + 1));
    if (pred == loopInfo.preheader) {
      phiPreheaderIndex = i;
      break;
    }
  }
  if (phiPreheaderIndex < 0)
    return false;

  BasicBlock* preheader = loopInfo.preheader;
  Instruction* preTerm = preheader->terminator();
  if (!preTerm || preTerm->opcode() != Opcode::Br || preTerm->numOperands() != 1 ||
      dynamic_cast<BasicBlock*>(preTerm->operand(0)) != loopInfo.header) {
    return false;
  }

  std::unordered_map<int, Value*> cloneCache;
  Value* origInit =
      cloneInvariantExpr(loopInfo.initVal, preheader, preTerm, loop, cloneCache);
  if (!origInit || !isI32Value(origInit))
    return false;

  std::vector<TightInitHeaderPhiPlan> headerPhiPlans;
  for (auto& instPtr : loopInfo.header->instructions()) {
    Instruction* phi = instPtr.get();
    if (phi->opcode() != Opcode::Phi)
      break;
    if (phi == inductionPhi)
      continue;
    int preIdx = -1;
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
      if (pred == loopInfo.preheader) {
        preIdx = i;
        break;
      }
    }
    TightInitHeaderPhiPlan plan;
    if (!classifyTightInitHeaderPhi(loop, continueBlock, phi, preIdx,
                                    loopInfo.latch, plan)) {
      return false;
    }
    headerPhiPlans.push_back(plan);
  }

  auto* cmp = insertBinaryBefore(preheader, preTerm, Opcode::ICmp, Type::i32(),
                                 origInit, tightInit,
                                 "lt_affine.tight.choose_init");
  setCmpCond(cmp, "lt");

  BasicBlock* tightBB = func->newBlock("affine.interior.init");
  BasicBlock* keepBB = func->newBlock("affine.remainder.init");
  BasicBlock* mergeBB = func->newBlock("affine.version.preheader");

  preheader->remove(preTerm);
  auto* condBr = makeInstruction(Opcode::CondBr, Type::voidTy());
  condBr->addOperand(cmp);
  condBr->addOperand(tightBB);
  condBr->addOperand(keepBB);
  preheader->pushBack(condBr);

  auto* tightBr = makeInstruction(Opcode::Br, Type::voidTy());
  tightBr->addOperand(mergeBB);
  tightBB->pushBack(tightBr);

  auto* keepBr = makeInstruction(Opcode::Br, Type::voidTy());
  keepBr->addOperand(mergeBB);
  keepBB->pushBack(keepBr);

  auto* initPhi =
      makeInstruction(Opcode::Phi, Type::i32(), "affine.interior.init");
  Value* tightInitIncoming =
      localizePhiIncoming(tightInit, tightBB, tightBr,
                          "affine.init.incoming");
  Value* keepInitIncoming =
      localizePhiIncoming(origInit, keepBB, keepBr,
                          "affine.init.incoming");
  if (!tightInitIncoming || !keepInitIncoming)
    return false;
  initPhi->addOperand(tightInitIncoming);
  initPhi->addOperand(tightBB);
  initPhi->addOperand(keepInitIncoming);
  initPhi->addOperand(keepBB);
  mergeBB->pushFront(initPhi);

  auto* mergeBr = makeInstruction(Opcode::Br, Type::voidTy());
  mergeBr->addOperand(loopInfo.header);
  mergeBB->pushBack(mergeBr);

  Value* deltaI32 = tightInit;
  if (origInit != mod.getConstInt(0)) {
    deltaI32 = insertBinaryBefore(tightBB, tightBr, Opcode::Sub, Type::i32(),
                                  tightInit, origInit,
                                  "affine.tight.init.delta");
  }

  for (const auto& plan : headerPhiPlans) {
    if (plan.mode == TightInitHeaderPhiMode::Stable) {
      Value* mergeIncoming =
          localizePhiIncoming(plan.originalInit, mergeBB, mergeBr,
                              "affine.carried.merge");
      if (!mergeIncoming)
        return false;
      plan.phi->setOperand(plan.preheaderOperandIndex, mergeIncoming);
      continue;
    }
    Value* tightPhiInit =
        buildTightenedHeaderPhiInit(mod, tightBB, tightBr, plan, deltaI32);
    if (!tightPhiInit)
      return false;
    Value* tightCarriedIncoming =
        localizePhiIncoming(tightPhiInit, tightBB, tightBr,
                            "affine.carried.incoming");
    Value* keepCarriedIncoming =
        localizePhiIncoming(plan.originalInit, keepBB, keepBr,
                            "affine.carried.incoming");
    if (!tightCarriedIncoming || !keepCarriedIncoming)
      return false;
    auto* carriedPhi =
        makeInstruction(Opcode::Phi, plan.phi->type(), "affine.interior.init");
    carriedPhi->addOperand(tightCarriedIncoming);
    carriedPhi->addOperand(tightBB);
    carriedPhi->addOperand(keepCarriedIncoming);
    carriedPhi->addOperand(keepBB);
    mergeBB->pushFront(carriedPhi);
    plan.phi->setOperand(plan.preheaderOperandIndex, carriedPhi);
  }

  replacePhiIncomingPred(loopInfo.header, preheader, mergeBB);
  inductionPhi->setOperand(phiPreheaderIndex, initPhi);
  return true;
}

bool createGuardedInitPhiPreheader(Module& mod, Function* func,
                                   const NaturalLoop& loop,
                                   const CanonicalExitLoop& loopInfo,
                                   Instruction* inductionPhi,
                                   Value* fastForwardInit,
                                   BasicBlock* continueBlock,
                                   Value* workCond) {
  if (!func || !loopInfo.preheader || !loopInfo.header || !inductionPhi ||
      !fastForwardInit || !workCond || !loopInfo.initVal) {
    return false;
  }

  int phiPreheaderIndex = -1;
  for (int i = 0; i + 1 < inductionPhi->numOperands(); i += 2) {
    auto* pred = dynamic_cast<BasicBlock*>(inductionPhi->operand(i + 1));
    if (pred == loopInfo.preheader) {
      phiPreheaderIndex = i;
      break;
    }
  }
  if (phiPreheaderIndex < 0)
    return false;

  BasicBlock* preheader = loopInfo.preheader;
  Instruction* preTerm = preheader->terminator();
  if (!preTerm || preTerm->opcode() != Opcode::Br || preTerm->numOperands() != 1 ||
      dynamic_cast<BasicBlock*>(preTerm->operand(0)) != loopInfo.header) {
    return false;
  }

  std::unordered_map<int, Value*> cloneCache;
  Value* origInit =
      cloneInvariantExpr(loopInfo.initVal, preheader, preTerm, loop, cloneCache);
  if (!origInit || !isI32Value(origInit) || !isI32Value(fastForwardInit))
    return false;

  std::vector<TightInitHeaderPhiPlan> headerPhiPlans;
  for (auto& instPtr : loopInfo.header->instructions()) {
    Instruction* phi = instPtr.get();
    if (phi->opcode() != Opcode::Phi)
      break;
    if (phi == inductionPhi)
      continue;
    int preIdx = -1;
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
      if (pred == loopInfo.preheader) {
        preIdx = i;
        break;
      }
    }
    TightInitHeaderPhiPlan plan;
    if (!classifyTightInitHeaderPhi(loop, continueBlock, phi, preIdx,
                                    loopInfo.latch, plan)) {
      return false;
    }
    headerPhiPlans.push_back(plan);
  }

  BasicBlock* keepBB = func->newBlock("affine.interior.init");
  BasicBlock* skipBB = func->newBlock("affine.zero_trip.init");
  BasicBlock* mergeBB = func->newBlock("affine.version.preheader");

  preheader->remove(preTerm);
  auto* condBr = makeInstruction(Opcode::CondBr, Type::voidTy());
  condBr->addOperand(workCond);
  condBr->addOperand(keepBB);
  condBr->addOperand(skipBB);
  preheader->pushBack(condBr);

  auto* keepBr = makeInstruction(Opcode::Br, Type::voidTy());
  keepBr->addOperand(mergeBB);
  keepBB->pushBack(keepBr);

  auto* skipBr = makeInstruction(Opcode::Br, Type::voidTy());
  skipBr->addOperand(mergeBB);
  skipBB->pushBack(skipBr);

  auto* initPhi =
      makeInstruction(Opcode::Phi, Type::i32(), "affine.versioned.init");
  Value* keepInitIncoming =
      localizePhiIncoming(origInit, keepBB, keepBr,
                          "affine.versioned.incoming");
  Value* skipInitIncoming =
      localizePhiIncoming(fastForwardInit, skipBB, skipBr,
                          "affine.versioned.incoming");
  if (!keepInitIncoming || !skipInitIncoming)
    return false;
  initPhi->addOperand(keepInitIncoming);
  initPhi->addOperand(keepBB);
  initPhi->addOperand(skipInitIncoming);
  initPhi->addOperand(skipBB);
  mergeBB->pushFront(initPhi);

  auto* mergeBr = makeInstruction(Opcode::Br, Type::voidTy());
  mergeBr->addOperand(loopInfo.header);
  mergeBB->pushBack(mergeBr);

  Value* deltaI32 = fastForwardInit;
  if (origInit != mod.getConstInt(0)) {
    deltaI32 = insertBinaryBefore(skipBB, skipBr, Opcode::Sub, Type::i32(),
                                  fastForwardInit, origInit,
                                  "affine.tight.zero_trip.delta");
  }

  for (const auto& plan : headerPhiPlans) {
    if (plan.mode == TightInitHeaderPhiMode::Stable) {
      Value* mergeIncoming =
          localizePhiIncoming(plan.originalInit, mergeBB, mergeBr,
                              "affine.versioned.merge");
      if (!mergeIncoming)
        return false;
      plan.phi->setOperand(plan.preheaderOperandIndex, mergeIncoming);
      continue;
    }
    Value* skipPhiInit =
        buildTightenedHeaderPhiInit(mod, skipBB, skipBr, plan, deltaI32);
    if (!skipPhiInit)
      return false;
    Value* keepCarriedIncoming =
        localizePhiIncoming(plan.originalInit, keepBB, keepBr,
                            "affine.versioned.carried");
    Value* skipCarriedIncoming =
        localizePhiIncoming(skipPhiInit, skipBB, skipBr,
                            "affine.versioned.carried");
    if (!keepCarriedIncoming || !skipCarriedIncoming)
      return false;
    auto* carriedPhi =
        makeInstruction(Opcode::Phi, plan.phi->type(), "affine.versioned.init");
    carriedPhi->addOperand(keepCarriedIncoming);
    carriedPhi->addOperand(keepBB);
    carriedPhi->addOperand(skipCarriedIncoming);
    carriedPhi->addOperand(skipBB);
    mergeBB->pushFront(carriedPhi);
    plan.phi->setOperand(plan.preheaderOperandIndex, carriedPhi);
  }

  replacePhiIncomingPred(loopInfo.header, preheader, mergeBB);
  inductionPhi->setOperand(phiPreheaderIndex, initPhi);
  return true;
}

bool tryTightenGuardWithIV(Module& mod,
                           Function* func,
                           const NaturalLoop& loop,
                           const InductionVar& iv,
                           const AffineGuardSummary& guard) {
  auto reject = [&](const char* reason) {
    traceTightening("skip", func, loop, iv, &guard, reason);
    return false;
  };
  if (!func || !loop.header || !guard.valid || !guard.compare ||
      !guard.workBlock) {
    return reject("invalid-input");
  }
  if (loopContainsCall(loop))
    return reject("loop-contains-call");

  CanonicalExitLoop cl;
  if (!recognizeCanonicalExitLoop(loop, iv, cl))
    return reject("not-canonical-exit-loop");
  Value* currentIV = iv.phi ? static_cast<Value*>(iv.phi)
                            : static_cast<Value*>(iv.storage);
  if (!currentIV || guard.induction != currentIV)
    return reject("guard-not-current-iv");

  if (!isCanonicalContinueBlock(loop, cl, iv, guard.continueBlock))
    return reject("continue-not-canonical");
  if (guard.relation != "gt" && guard.relation != "ge" &&
      guard.relation != "lt" && guard.relation != "le") {
    return reject("unsupported-relation");
  }

  Instruction* preTerm = cl.preheader->terminator();
  if (!preTerm)
    return reject("missing-preheader-term");
  Value* tightExclusive =
      buildAffineExprValue(mod, cl.preheader, preTerm, loop, guard.boundExpr,
                           "affine.tight.bound_expr");
  if (!tightExclusive)
    return reject("bound-materialize-failed");
  if (guard.relation == "gt" || guard.relation == "ge") {
    if (guard.relation == "gt") {
      tightExclusive =
          addConst(mod, cl.preheader, preTerm, tightExclusive, 1,
                   "affine.tight.exclusive");
      if (!tightExclusive)
        return reject("exclusive-adjust-failed");
    }

    Value* finalBound = nullptr;
    if (!createBoundPhiPreheader(mod, func, loop, cl, tightExclusive,
                                 finalBound))
      return reject("bound-version-failed");

    cl.cmp->setOperand(0, cl.inductionValue);
    cl.cmp->setOperand(1, finalBound);
    setCmpCond(cl.cmp, "lt");
    if (!rewriteGuardToWork(mod, guard))
      return reject("rewrite-guard-failed");
    traceTightening("apply", func, loop, iv, &guard, "upper-bound");
    return true;
  }

  if ((guard.relation == "lt" || guard.relation == "le") && iv.phi) {
    Value* tightInit = tightExclusive;
    if (guard.relation == "le") {
      tightInit =
          addConst(mod, cl.preheader, preTerm, tightInit, 1,
                   "affine.tight.init_exclusive");
      if (!tightInit)
        return reject("init-adjust-failed");
    }

  if (!createInitPhiPreheader(mod, func, loop, cl, iv.phi, tightInit,
                              guard.continueBlock))
    return reject("init-version-failed");
  if (!rewriteGuardToWork(mod, guard))
    return reject("rewrite-guard-failed");
  traceTightening("apply", func, loop, iv, &guard, "lower-init");
  return true;
}

  return reject("lower-needs-phi");
}

bool tryVersionInvariantGuardForLoop(Module& mod,
                                     Function* func,
                                     const NaturalLoop& loop,
                                     const InductionVar& iv,
                                     const AffineGuardSummary& guard) {
  auto reject = [&](const char* reason) {
    traceTightening("skip-invariant", func, loop, iv, &guard, reason);
    return false;
  };
  if (!func || !loop.header || !guard.valid || !guard.block || !guard.compare ||
      !guard.workBlock || !guard.continueBlock || !iv.phi) {
    return reject("invalid-input");
  }
  if (loopContainsCall(loop))
    return reject("loop-contains-call");
  if (!loop.blocks.count(guard.block) || !loop.blocks.count(guard.workBlock) ||
      !loop.blocks.count(guard.continueBlock)) {
    return reject("guard-outside-loop");
  }

  Value* currentIV = iv.phi ? static_cast<Value*>(iv.phi)
                            : static_cast<Value*>(iv.storage);
  if (!currentIV || guard.induction == currentIV || !isI32Value(guard.induction))
    return reject("not-invariant-guard");

  CanonicalExitLoop cl;
  if (!recognizeCanonicalExitLoop(loop, iv, cl))
    return reject("not-canonical-exit-loop");
  if (!isCanonicalContinueBlock(loop, cl, iv, guard.continueBlock))
    return reject("continue-not-canonical");

  std::unordered_map<int, bool> memo;
  if (!isLoopInvariantValue(guard.induction, loop, memo))
    return reject("guard-induction-not-invariant");

  Instruction* preTerm = cl.preheader->terminator();
  if (!preTerm)
    return reject("missing-preheader-term");
  Value* workCond =
      buildGuardWorkCondition(mod, cl.preheader, preTerm, loop, guard);
  if (!workCond)
    return reject("work-cond-materialize-failed");
  Value* zeroTripInit =
      buildLoopExclusiveBoundValue(mod, cl.preheader, preTerm, loop, cl);
  if (!zeroTripInit)
    return reject("zero-trip-init-failed");

  // 当 guard 对当前内层 IV 完全不变时，不再让热循环每次迭代都判一次。
  // work path 保持原始起点并把该 compare 视为恒真；non-work path 直接把 IV
  // 快进到退出边界，让整个循环 zero-trip。
  if (!createGuardedInitPhiPreheader(mod, func, loop, cl, iv.phi, zeroTripInit,
                                     guard.continueBlock, workCond))
    return reject("guarded-init-version-failed");
  if (!rewriteGuardToWork(mod, guard))
    return reject("rewrite-guard-failed");
  traceTightening("apply-invariant", func, loop, iv, &guard, "zero-trip");
  return true;
}

bool tryTightenRegionPlan(Module& mod,
                          Function* func,
                          analysis::FunctionAnalysisCache& cache,
                          const AffineRegionPlan& plan) {
  if (!func || !plan.valid || !plan.interiorRemainderReady || !plan.loop ||
      !plan.guard) {
    return false;
  }

  const auto& ivs = cache.inductionVars(*plan.loop);
  for (const auto& iv : ivs) {
    // region plan 只指定“哪个 guard/loop 值得版本化”，
    // 真正 CFG 改写仍要求 IV、continue edge 和 bound 都重新证明。
    if (tryTightenGuardWithIV(mod, func, *plan.loop, iv, *plan.guard))
      return true;
  }
  return false;
}

bool tryTightenLoop(Module& mod, Function* func,
                    analysis::FunctionAnalysisCache& cache,
                    const NaturalLoop& loop) {
  if (!func || !loop.header)
    return false;

  const auto& ivs = cache.inductionVars(loop);
  if (ivs.empty())
    return false;
  auto guards = analysis::collectAffineGuards(func, loop, ivs);
  if (traceTighteningEnabled()) {
    std::cerr << "[affine-tighten] loop func=" << func->name()
              << " header=" << loop.header->name()
              << " ivs=" << ivs.size()
              << " guards=" << guards.size() << "\n";
  }
  if (!guards.empty()) {
    for (const auto& iv : ivs) {
      for (const auto& guard : guards) {
        if (tryTightenGuardWithIV(mod, func, loop, iv, guard))
          return true;
      }
    }
  }

  const auto& allGuards = cache.affineGuards();
  for (const auto& iv : ivs) {
    for (const auto& guard : allGuards) {
      if (tryVersionInvariantGuardForLoop(mod, func, loop, iv, guard))
        return true;
    }
  }

  return false;
}

int64_t regionTighteningPriority(const AffineRegionPlan& plan) {
  // tightening 优先服务已经由 loop-band 证明适合
  // interior/remainder 的区域；这只是排序，不绕过 tryTightenLoop 的 CFG/guard 检查。
  int64_t score = plan.estimatedBenefit;
  const auto& profit = plan.profit;
  const auto& versioning = plan.versioning;
  if (plan.interiorRemainderReady)
    score += 512;
  if (profit.multiLevelBlockingReady)
    score += 128;
  if (profit.postSplitMetricsImproved)
    score += 96;
  if (versioning.hasGuardBranch)
    score += 96;
  score += profit.postSplitCfgDeltaScore / 2;
  if (plan.needsVersioning)
    score -= 96;
  score += profit.estimatedWorkingSetDrop / 4;
  score -= profit.localityRiskScore / 8;
  return score;
}

bool allowPreUnrollRegionTighteningPlan(const AffineRegionPlan& plan) {
  if (!plan.interiorRemainderReady || plan.needsVersioning ||
      plan.dynamicStrideMemoryOps != 0)
    return false;
  const auto& profit = plan.profit;
  if (!profit.interiorFastPathReady || !profit.remainderPathReady)
    return false;

  const int64_t netScore = analysis::affineRegionNetProfitScore(plan);
  if (profit.postSplitMetricsImproved || profit.multiLevelBlockingReady) {
    return netScore >= 128 && profit.estimatedRegisterPressure <= 24;
  }
  return plan.estimatedBenefit >= 192 && netScore >= 512 &&
         profit.estimatedRegisterPressure <= 24;
}

// 按收益优先级逐个尝试收紧仿射区域边界；每轮一旦成功收紧一个区域就
// 重算分析重新排队，直到不再变化或达到安全轮数上限。
bool processFunction(Module& mod,
                     Function* func,
                     analysis::AnalysisManager& analysisManager,
                     AffineLoopTighteningMode mode) {
  if (!func || !func->entryBlock())
    return false;

  bool changed = false;
  bool localChanged = true;
  int safety = 8;
  while (localChanged && safety-- > 0) {
    func->recomputeCFG();
    localChanged = false;

    analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
    std::vector<AffineRegionPlan> regionPlans(cache.affineRegions().begin(),
                                              cache.affineRegions().end());
    std::sort(regionPlans.begin(), regionPlans.end(),
              [](const AffineRegionPlan& lhs, const AffineRegionPlan& rhs) {
                int64_t lhsScore = regionTighteningPriority(lhs);
                int64_t rhsScore = regionTighteningPriority(rhs);
                if (lhsScore != rhsScore)
                  return lhsScore > rhsScore;
                return lhs.loopNode < rhs.loopNode;
              });
    std::vector<const NaturalLoop*> orderedLoops;
    std::unordered_set<int> queuedHeaders;
    orderedLoops.reserve(regionPlans.size() + cache.loops().size());
    for (const auto& plan : regionPlans) {
      if (!plan.valid || !plan.loop || !plan.loop->header)
        continue;
      if (traceTighteningEnabled()) {
        const auto& profit = plan.profit;
        std::cerr << "[affine-tighten] plan"
                  << " func=" << func->name()
                  << " header=" << plan.loop->header->name()
                  << " benefit=" << plan.estimatedBenefit
                  << " net=" << analysis::affineRegionNetProfitScore(plan)
                  << " interior=" << plan.interiorRemainderReady
                  << " hot=" << plan.hotInterior
                  << " version=" << plan.needsVersioning
                  << " post=" << profit.postSplitMetricsImproved
                  << " multi=" << profit.multiLevelBlockingReady
                  << " fast=" << profit.interiorFastPathReady
                  << " rem=" << profit.remainderPathReady
                  << " reg=" << profit.estimatedRegisterPressure
                  << " dyn=" << plan.dynamicStrideMemoryOps
                  << "\n";
      }
      const bool allowedPreUnroll =
          mode != AffineLoopTighteningMode::RegionOnly ||
          allowPreUnrollRegionTighteningPlan(plan);
      // 命中首个可收紧区域后立即终止本轮，让 CFG/分析在下一轮重建。
      if (plan.interiorRemainderReady && allowedPreUnroll &&
          tryTightenRegionPlan(mod, func, cache, plan)) {
        localChanged = true;
        changed = true;
        analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
        break;
      }
      if (!queuedHeaders.insert(plan.loop->header->id()).second)
        continue;
      orderedLoops.push_back(plan.loop);
    }
    if (localChanged)
      continue;

    if (mode == AffineLoopTighteningMode::RegionOnly)
      continue;

    for (const auto& loop : cache.loops()) {
      if (!loop.header)
        continue;
      if (!queuedHeaders.insert(loop.header->id()).second)
        continue;
      orderedLoops.push_back(&loop);
    }

    for (const auto* loop : orderedLoops) {
      if (!loop)
        continue;
      if (tryTightenLoop(mod, func, cache, *loop)) {
        localChanged = true;
        changed = true;
        analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
        break;
      }
    }
  }

  if (changed)
    func->recomputeCFG();
  return changed;
}

} // namespace

bool applyAffineLoopTightening(Module& mod) {
  analysis::AnalysisManager analysisManager(mod);
  return applyAffineLoopTightening(mod, analysisManager);
}

bool applyAffineLoopTightening(Module& mod,
                               analysis::AnalysisManager& analysisManager) {
  return applyAffineLoopTightening(mod, analysisManager,
                                   AffineLoopTighteningMode::Full);
}

bool applyAffineLoopTightening(Module& mod,
                               analysis::AnalysisManager& analysisManager,
                               AffineLoopTighteningMode mode) {
  bool changed = false;
  for (auto& funcPtr : mod.functions())
    changed |= processFunction(mod, funcPtr.get(), analysisManager, mode);
  return changed;
}

} // namespace hir
