// 文件职责：识别循环中的标量递推并整理其循环携带值。
#include "scalar_recurrence.h"
#include "cse.h"
#include "passes_utils.h"
#include "phi_utils.h"

#include "../../analysis/cfg_utils.h"
#include "../../analysis/loop_info.h"
#include "../../hir/canonical_loop.h"
#include "../../research_switch.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace core_ir {

struct ModularScalarRecurrencePlan {
  hir::CanonicalLoop loop;
  Instruction* accPhi = nullptr;
  Value* accInit = nullptr;
  Instruction* rem = nullptr;
  Instruction* add = nullptr;
  int32_t step = 0;
  int32_t mod = 0;
  bool ivHasOutsideUse = false;
};

Instruction* firstNonPhiInstruction(BasicBlock* bb) {
  if (!bb)
    return nullptr;
  for (auto& instPtr : bb->instructions()) {
    if (instPtr->opcode() != Opcode::Phi)
      return instPtr.get();
  }
  return nullptr;
}

bool valueAvailableAtBlock(Value* value,
                           BasicBlock* block,
                           const analysis::NaturalLoop& loop,
                           const DomSet& doms) {
  if (!value || !block)
    return false;
  if (value->isConstant() || dynamic_cast<Argument*>(value) ||
      dynamic_cast<GlobalVar*>(value) || dynamic_cast<BasicBlock*>(value)) {
    return true;
  }
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !inst->parent())
    return false;
  if (loop.blocks.count(inst->parent()))
    return false;
  return dominates(doms, inst->parent(), block);
}

bool collectLoopPhiIncoming(Instruction* phi,
                            const hir::CanonicalLoop& loop,
                            Value*& init,
                            Value*& next) {
  if (!phi || phi->opcode() != Opcode::Phi)
    return false;
  init = nullptr;
  next = nullptr;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
    if (pred == loop.preheader)
      init = phi->operand(i);
    else if (pred == loop.latch)
      next = phi->operand(i);
  }
  return init && next;
}

bool canonicalLoopIsForwardLt(const hir::CanonicalLoop& loop) {
  if (!loop.condBr || !loop.cmp)
    return false;
  auto* trueBB = dynamic_cast<BasicBlock*>(loop.condBr->operand(1));
  if (trueBB != loop.body)
    return false;
  const std::string cond = hir::canonicalLoopCmpCond(loop.cmp);
  if (loop.ivIsCmpLhs)
    return cond == "lt";
  return cond == "gt";
}

bool loopHasSingleExit(const analysis::NaturalLoop& loop, BasicBlock* expectedExit) {
  if (!expectedExit)
    return false;
  std::unordered_set<BasicBlock*> exits;
  for (auto* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto* succ : bb->succs()) {
      if (succ && !loop.blocks.count(succ))
        exits.insert(succ);
    }
  }
  return exits.size() == 1 && exits.count(expectedExit);
}

bool valueHasOutsideUse(Value* value, const analysis::NaturalLoop& loop) {
  if (!value)
    return false;
  for (auto* user : value->uses()) {
    if (user && user->parent() && !loop.blocks.count(user->parent()))
      return true;
  }
  return false;
}

bool matchAddConstStep(Instruction* add, Value* carried, int32_t& step) {
  if (!add || add->opcode() != Opcode::Add || add->numOperands() != 2)
    return false;
  Value* other = nullptr;
  if (add->operand(0) == carried) {
    other = add->operand(1);
  } else if (add->operand(1) == carried) {
    other = add->operand(0);
  } else {
    return false;
  }
  auto* ci = dynamic_cast<ConstInt*>(other);
  if (!ci)
    return false;
  step = ci->value();
  return true;
}

bool loopDefinitionsAreBypassable(const analysis::NaturalLoop& loop,
                                  Instruction* accPhi,
                                  Instruction* ivPhi) {
  for (auto* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      switch (inst->opcode()) {
      case Opcode::Store:
      case Opcode::VStore:
      case Opcode::Call:
      case Opcode::Ret:
      case Opcode::Alloca:
        return false;
      default:
        break;
      }

      for (auto* user : inst->uses()) {
        if (!user || !user->parent() || loop.blocks.count(user->parent()))
          continue;
        if (user->opcode() == Opcode::Phi)
          return false;
        if (inst != accPhi && inst != ivPhi)
          return false;
      }
    }
  }
  return true;
}

bool matchModularScalarRecurrence(const analysis::NaturalLoop& naturalLoop,
                                  analysis::FunctionAnalysisCache& cache,
                                  ModularScalarRecurrencePlan& plan) {
  BasicBlock* preheader = analysis::dedicatedPreheader(naturalLoop);
  hir::CanonicalLoop loop;
  if (!hir::recognizeCanonicalLoop(naturalLoop, preheader, loop))
    return false;
  if (!canonicalLoopIsForwardLt(loop))
    return false;
  if (!loopHasSingleExit(naturalLoop, loop.exit))
    return false;
  if (!analysis::collectLeadingPhi(loop.exit).empty())
    return false;

  const DomSet& doms = cache.dominators();
  if (!valueAvailableAtBlock(loop.initVal, loop.preheader, naturalLoop, doms) ||
      !valueAvailableAtBlock(loop.boundVal, loop.preheader, naturalLoop, doms)) {
    return false;
  }

  std::vector<Instruction*> phis = analysis::collectLeadingPhi(loop.header);
  Instruction* accPhi = nullptr;
  Value* accInit = nullptr;
  Instruction* rem = nullptr;
  Instruction* add = nullptr;
  int32_t step = 0;
  int32_t mod = 0;

  for (auto* phi : phis) {
    if (!phi || phi == loop.ivPhi)
      continue;
    if (accPhi)
      return false;
    Value* init = nullptr;
    Value* next = nullptr;
    if (!collectLoopPhiIncoming(phi, loop, init, next))
      return false;
    auto* remInst = dynamic_cast<Instruction*>(next);
    if (!remInst || remInst->opcode() != Opcode::SRem ||
        remInst->numOperands() != 2) {
      return false;
    }
    auto* modConst = dynamic_cast<ConstInt*>(remInst->operand(1));
    if (!modConst || modConst->value() <= 1)
      return false;
    auto* addInst = dynamic_cast<Instruction*>(remInst->operand(0));
    int32_t stepConst = 0;
    if (!matchAddConstStep(addInst, phi, stepConst))
      return false;
    if (stepConst <= 0 || stepConst >= modConst->value())
      return false;
    if (!valueAvailableAtBlock(init, loop.preheader, naturalLoop, doms))
      return false;

    accPhi = phi;
    accInit = init;
    rem = remInst;
    add = addInst;
    step = stepConst;
    mod = modConst->value();
  }

  if (!accPhi)
    return false;
  if (!loopDefinitionsAreBypassable(naturalLoop, accPhi, loop.ivPhi))
    return false;

  plan.loop = loop;
  plan.accPhi = accPhi;
  plan.accInit = accInit;
  plan.rem = rem;
  plan.add = add;
  plan.step = step;
  plan.mod = mod;
  plan.ivHasOutsideUse = valueHasOutsideUse(loop.ivPhi, naturalLoop);
  return true;
}

Instruction* insertModRecInstBefore(BasicBlock* bb,
                                    Instruction* pos,
                                    Opcode op,
                                    Type ty,
                                    const std::string& name,
                                    std::initializer_list<Value*> operands) {
  auto* inst = new Instruction(op, ty, name);
  for (auto* operand : operands)
    inst->addOperand(operand);
  bb->insertBefore(pos, inst);
  return inst;
}

Instruction* appendModRecInst(BasicBlock* bb,
                              Opcode op,
                              Type ty,
                              const std::string& name,
                              std::initializer_list<Value*> operands) {
  auto* inst = new Instruction(op, ty, name);
  for (auto* operand : operands)
    inst->addOperand(operand);
  bb->pushBack(inst);
  return inst;
}

Instruction* insertModRecICmpBefore(BasicBlock* bb,
                                    Instruction* pos,
                                    const std::string& cond,
                                    const std::string& name,
                                    Value* lhs,
                                    Value* rhs) {
  return insertModRecInstBefore(bb, pos, Opcode::ICmp, Type::i32(),
                                cond + "_" + name, {lhs, rhs});
}

Instruction* insertModRecAndBefore(BasicBlock* bb,
                                   Instruction* pos,
                                   Value* lhs,
                                   Value* rhs,
                                   const std::string& name) {
  return insertModRecInstBefore(bb, pos, Opcode::And, Type::i32(), name,
                                {lhs, rhs});
}

void insertExitPhi(BasicBlock* exit, Instruction* phi) {
  Instruction* pos = firstNonPhiInstruction(exit);
  if (pos)
    exit->insertBefore(pos, phi);
  else
    exit->pushBack(phi);
}

void replaceOutsideUses(Value* oldValue,
                        Value* newValue,
                        const analysis::NaturalLoop& loop) {
  std::vector<Instruction*> users = oldValue->uses();
  for (auto* user : users) {
    if (!user || user == newValue || !user->parent() ||
        loop.blocks.count(user->parent())) {
      continue;
    }
    for (int i = 0; i < user->numOperands(); ++i) {
      if (user->operand(i) == oldValue)
        user->setOperand(i, newValue);
    }
  }
}

bool materializeModularScalarRecurrence(Function* func,
                                        Module& mod,
                                        const analysis::NaturalLoop& naturalLoop,
                                        const ModularScalarRecurrencePlan& plan) {
  if (!func || !plan.loop.preheader || !plan.loop.header || !plan.loop.exit ||
      !plan.accPhi || !plan.accInit) {
    return false;
  }
  Instruction* preTerm = plan.loop.preheader->terminator();
  if (!preTerm || preTerm->opcode() != Opcode::Br ||
      preTerm->operand(0) != plan.loop.header) {
    return false;
  }

  BasicBlock* fastBB = func->newBlock("modrec.fast");
  ConstInt* zero = mod.getConstInt(0);
  ConstInt* modValue = mod.getConstInt(plan.mod);
  ConstInt* intMax = mod.getConstInt(std::numeric_limits<int32_t>::max());
  ConstInt* stepValue = mod.getConstInt(plan.step);

  Instruction* trip = insertModRecInstBefore(
      plan.loop.preheader, preTerm, Opcode::Sub, Type::i32(), "modrec.trip",
      {plan.loop.boundVal, plan.loop.initVal});
  Instruction* tripPositive = insertModRecICmpBefore(
      plan.loop.preheader, preTerm, "gt", "modrec.trip.positive", trip, zero);
  Instruction* accNonNegative = insertModRecICmpBefore(
      plan.loop.preheader, preTerm, "ge", "modrec.acc.nonneg", plan.accInit,
      zero);
  Instruction* accBelowMod = insertModRecICmpBefore(
      plan.loop.preheader, preTerm, "lt", "modrec.acc.inrange", plan.accInit,
      modValue);
  Instruction* guard = insertModRecAndBefore(
      plan.loop.preheader, preTerm, tripPositive, accNonNegative,
      "modrec.guard.0");
  guard = insertModRecAndBefore(plan.loop.preheader, preTerm, guard, accBelowMod,
                                "modrec.guard.1");

  const int32_t maxTrip =
      std::numeric_limits<int32_t>::max() / plan.step;
  Instruction* tripInRange = insertModRecICmpBefore(
      plan.loop.preheader, preTerm, "le", "modrec.trip.inrange", trip,
      mod.getConstInt(maxTrip));
  Instruction* delta = insertModRecInstBefore(
      plan.loop.preheader, preTerm, Opcode::Mul, Type::i32(), "modrec.delta",
      {trip, stepValue});
  Instruction* maxMinusDelta = insertModRecInstBefore(
      plan.loop.preheader, preTerm, Opcode::Sub, Type::i32(),
      "modrec.sum.limit", {intMax, delta});
  Instruction* sumSafe = insertModRecICmpBefore(
      plan.loop.preheader, preTerm, "le", "modrec.sum.safe", plan.accInit,
      maxMinusDelta);
  guard = insertModRecAndBefore(plan.loop.preheader, preTerm, guard, tripInRange,
                                "modrec.guard.2");
  guard = insertModRecAndBefore(plan.loop.preheader, preTerm, guard, sumSafe,
                                "modrec.guard.3");

  Instruction* fastSum = appendModRecInst(
      fastBB, Opcode::Add, Type::i32(), "modrec.fast.sum", {plan.accInit, delta});
  Instruction* fastAcc = appendModRecInst(
      fastBB, Opcode::SRem, Type::i32(), "modrec.fast.rem",
      {fastSum, modValue});
  auto* fastBr = new Instruction(Opcode::Br, Type::voidTy());
  fastBr->addOperand(plan.loop.exit);
  fastBB->pushBack(fastBr);

  auto* accExit = new Instruction(Opcode::Phi, Type::i32(), "modrec.acc.exit");
  accExit->addOperand(plan.accPhi);
  accExit->addOperand(plan.loop.header);
  accExit->addOperand(fastAcc);
  accExit->addOperand(fastBB);
  insertExitPhi(plan.loop.exit, accExit);

  Instruction* ivExit = nullptr;
  if (plan.ivHasOutsideUse) {
    ivExit = new Instruction(Opcode::Phi, Type::i32(), "modrec.iv.exit");
    ivExit->addOperand(plan.loop.ivPhi);
    ivExit->addOperand(plan.loop.header);
    ivExit->addOperand(plan.loop.boundVal);
    ivExit->addOperand(fastBB);
    insertExitPhi(plan.loop.exit, ivExit);
  }

  replaceOutsideUses(plan.accPhi, accExit, naturalLoop);
  if (ivExit)
    replaceOutsideUses(plan.loop.ivPhi, ivExit, naturalLoop);

  plan.loop.preheader->remove(preTerm);
  auto* condBr = new Instruction(Opcode::CondBr, Type::voidTy());
  condBr->addOperand(guard);
  condBr->addOperand(fastBB);
  condBr->addOperand(plan.loop.header);
  plan.loop.preheader->pushBack(condBr);
  func->recomputeCFG();
  return true;
}

bool valueDependsOnValueForMod(Value* value,
                               Value* needle,
                               std::unordered_set<int>& visiting,
                               int depth = 0) {
  if (!value || !needle || depth > 32)
    return false;
  if (value == needle)
    return true;
  if (value->isConstant() || dynamic_cast<Argument*>(value) ||
      dynamic_cast<GlobalVar*>(value) || dynamic_cast<BasicBlock*>(value)) {
    return false;
  }
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !visiting.insert(inst->id()).second)
    return false;
  for (Value* operand : inst->operands()) {
    if (valueDependsOnValueForMod(operand, needle, visiting, depth + 1)) {
      visiting.erase(inst->id());
      return true;
    }
  }
  visiting.erase(inst->id());
  return false;
}

bool addRange(int64_t lhsMin, int64_t lhsMax,
              int64_t rhsMin, int64_t rhsMax,
              int64_t& outMin, int64_t& outMax) {
  if (lhsMin < std::numeric_limits<int32_t>::min() - rhsMin ||
      lhsMax > std::numeric_limits<int32_t>::max() - rhsMax) {
    return false;
  }
  outMin = lhsMin + rhsMin;
  outMax = lhsMax + rhsMax;
  return true;
}

bool subRange(int64_t lhsMin, int64_t lhsMax,
              int64_t rhsMin, int64_t rhsMax,
              int64_t& outMin, int64_t& outMax) {
  if (lhsMin < std::numeric_limits<int32_t>::min() + rhsMax ||
      lhsMax > std::numeric_limits<int32_t>::max() + rhsMin) {
    return false;
  }
  outMin = lhsMin - rhsMax;
  outMax = lhsMax - rhsMin;
  return true;
}

bool collectSignedModuloAddRange(Value* value,
                                 Instruction* targetRem,
                                 int32_t mod,
                                 Instruction*& accPhi,
                                 int64_t& minValue,
                                 int64_t& maxValue,
                                 int depth = 0) {
  if (!value || !targetRem || mod <= 1 || depth > 16)
    return false;
  if (auto* ci = dynamic_cast<ConstInt*>(value)) {
    minValue = ci->value();
    maxValue = ci->value();
    return true;
  }

  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !inst->type().isI32())
    return false;

  if (inst->opcode() == Opcode::Phi) {
    Instruction* phi = inst;
    if (accPhi && accPhi != phi)
      return false;
    accPhi = phi;
    minValue = -(static_cast<int64_t>(mod) - 1);
    maxValue = static_cast<int64_t>(mod) - 1;
    return true;
  }

  if (inst->opcode() == Opcode::Add || inst->opcode() == Opcode::Sub) {
    int64_t lhsMin = 0;
    int64_t lhsMax = 0;
    int64_t rhsMin = 0;
    int64_t rhsMax = 0;
    if (!collectSignedModuloAddRange(inst->operand(0), targetRem, mod, accPhi,
                                     lhsMin, lhsMax, depth + 1) ||
        !collectSignedModuloAddRange(inst->operand(1), targetRem, mod, accPhi,
                                     rhsMin, rhsMax, depth + 1)) {
      return false;
    }
    return inst->opcode() == Opcode::Add
               ? addRange(lhsMin, lhsMax, rhsMin, rhsMax, minValue, maxValue)
               : subRange(lhsMin, lhsMax, rhsMin, rhsMax, minValue, maxValue);
  }

  if (inst->opcode() == Opcode::SRem && inst->numOperands() == 2) {
    auto* rhs = dynamic_cast<ConstInt*>(inst->operand(1));
    if (!rhs || rhs->value() <= 1 || rhs->value() >= mod)
      return false;
    std::unordered_set<int> visiting;
    if (accPhi && valueDependsOnValueForMod(inst->operand(0), accPhi, visiting))
      return false;
    minValue = -(static_cast<int64_t>(rhs->value()) - 1);
    maxValue = static_cast<int64_t>(rhs->value()) - 1;
    return true;
  }

  KnownIntRange range = knownIntRange(value);
  if (!range.valid)
    return false;
  if (accPhi) {
    std::unordered_set<int> visiting;
    if (valueDependsOnValueForMod(value, accPhi, visiting))
      return false;
  }
  minValue = range.minValue;
  maxValue = range.maxValue;
  return true;
}

bool phiIsSignedModuloAccumulator(Instruction* phi,
                                  Instruction* rem,
                                  int32_t mod) {
  if (!phi || !rem || phi->opcode() != Opcode::Phi ||
      (phi->numOperands() % 2) != 0) {
    return false;
  }
  bool hasBackedge = false;
  bool hasInitialValue = false;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    Value* incoming = phi->operand(i);
    if (incoming == rem) {
      hasBackedge = true;
      continue;
    }
    auto* ci = dynamic_cast<ConstInt*>(incoming);
    if (!ci || ci->value() <= -mod || ci->value() >= mod)
      return false;
    hasInitialValue = true;
  }
  return hasBackedge && hasInitialValue;
}

bool materializeBoundedSignedModuloAdd(Module& mod, Instruction* rem) {
  auto traceReject = [&](const char* reason) {
    if (!sycEnvironmentValue("MOD_BOUNDED_REM_TRACE"))
      return;
    std::cerr << "[bounded-rem] reject reason=" << reason;
    if (rem) {
      std::cerr << " rem=" << rem->id();
      if (rem->parent())
        std::cerr << " bb=" << rem->parent()->name();
    }
    std::cerr << "\n";
  };
  if (!rem || rem->opcode() != Opcode::SRem || rem->numOperands() != 2 ||
      !rem->parent()) {
    traceReject("shape");
    return false;
  }
  auto* modConst = dynamic_cast<ConstInt*>(rem->operand(1));
  if (!modConst || modConst->value() <= 1 ||
      modConst->value() > std::numeric_limits<int32_t>::max() / 2) {
    traceReject("mod");
    return false;
  }

  const int32_t modValue = modConst->value();
  Instruction* accPhi = nullptr;
  int64_t totalMin = 0;
  int64_t totalMax = 0;
  if (!collectSignedModuloAddRange(rem->operand(0), rem, modValue, accPhi,
                                   totalMin, totalMax) ||
      !phiIsSignedModuloAccumulator(accPhi, rem, modValue)) {
    traceReject("range-or-phi");
    return false;
  }
  const int64_t mod64 = modValue;
  if (totalMin <= -2 * mod64 || totalMax >= 2 * mod64) {
    traceReject("range-width");
    return false;
  }

  BasicBlock* bb = rem->parent();
  Value* raw = rem->operand(0);
  auto* geHigh = insertModRecICmpBefore(bb, rem, "ge", "bounded.rem.high",
                                        raw, mod.getConstInt(modValue));
  auto* highMask = insertModRecInstBefore(
      bb, rem, Opcode::Sub, Type::i32(), "bounded.rem.high.mask",
      {mod.getConstInt(0), geHigh});
  auto* highDelta = insertModRecInstBefore(
      bb, rem, Opcode::And, Type::i32(), "bounded.rem.high.delta",
      {mod.getConstInt(modValue), highMask});
  auto* highAdjusted = insertModRecInstBefore(
      bb, rem, Opcode::Sub, Type::i32(), "bounded.rem.high.adjusted",
      {raw, highDelta});

  auto* leLow = insertModRecICmpBefore(bb, rem, "le", "bounded.rem.low",
                                       highAdjusted, mod.getConstInt(-modValue));
  auto* lowMask = insertModRecInstBefore(
      bb, rem, Opcode::Sub, Type::i32(), "bounded.rem.low.mask",
      {mod.getConstInt(0), leLow});
  auto* lowDelta = insertModRecInstBefore(
      bb, rem, Opcode::And, Type::i32(), "bounded.rem.low.delta",
      {mod.getConstInt(modValue), lowMask});
  auto* normalized = insertModRecInstBefore(
      bb, rem, Opcode::Add, Type::i32(), "bounded.rem.normalized",
      {highAdjusted, lowDelta});

  rem->replaceAllUsesWith(normalized);
  if (rem->uses().empty())
    bb->remove(rem);
  return true;
}

bool applyBoundedSignedModuloAdd(Module& mod) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func)
      continue;
    std::vector<Instruction*> remainders;
    for (auto& bbPtr : func->blocks()) {
      for (auto& instPtr : bbPtr->instructions()) {
        Instruction* inst = instPtr.get();
        if (inst && inst->opcode() == Opcode::SRem)
          remainders.push_back(inst);
      }
    }
    for (Instruction* rem : remainders) {
      if (!Value::isLiveValue(rem))
        continue;
      int remId = rem->id();
      if (materializeBoundedSignedModuloAdd(mod, rem)) {
        changed = true;
        if (sycEnvironmentValue("MOD_BOUNDED_REM_TRACE")) {
          std::cerr << "[bounded-rem] func=" << func->name()
                    << " rem=" << remId << "\n";
        }
      }
    }
  }
  return changed;
}

bool applyModularScalarRecurrence(Function* func,
                                  Module& mod,
                                  analysis::AnalysisManager& analysisManager) {
  if (!func || !func->entryBlock())
    return false;
  analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
  if (cache.instructionCount() > 16000)
    return false;
  auto loops = cache.loops();
  if (loops.empty())
    return false;
  std::sort(loops.begin(), loops.end(),
            [](const analysis::NaturalLoop& lhs,
               const analysis::NaturalLoop& rhs) {
              if (lhs.blocks.size() != rhs.blocks.size())
                return lhs.blocks.size() < rhs.blocks.size();
              return lhs.header->id() < rhs.header->id();
            });

  const bool trace = sycEnvironmentValue("MOD_SCALAR_RECURRENCE_TRACE") != nullptr;
  for (const auto& loop : loops) {
    ModularScalarRecurrencePlan plan;
    if (!matchModularScalarRecurrence(loop, cache, plan))
      continue;
    if (materializeModularScalarRecurrence(func, mod, loop, plan)) {
      if (trace) {
        std::cerr << "[mod-scalar-rec] fastpath func=" << func->name()
                  << " header=" << loop.header->name()
                  << " step=" << plan.step << " mod=" << plan.mod << "\n";
      }
      return true;
    }
  }
  return false;
}

bool applyModularScalarRecurrences(Module& mod,
                                   analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  bool localChanged = true;
  int budget = 64;
  while (localChanged && budget-- > 0) {
    localChanged = false;
    for (auto& funcPtr : mod.functions()) {
      Function* func = funcPtr.get();
      if (!func)
        continue;
      if (applyModularScalarRecurrence(func, mod, analysisManager)) {
        changed = true;
        localChanged = true;
        analysisManager.invalidateFunction(
            func, analysis::AnalysisInvalidation::CFG);
      }
    }
  }
  // 研究开关只允许在递推阶段提前物化；默认流水线仍会在最终整理窗口
  // 对同一条通用值域规则运行一次。
  if (sycResearchSwitchEnabled("SYC_ENABLE_COREIR_BOUNDED_SIGNED_REM")) {
    bool boundedRemChanged = applyBoundedSignedModuloAdd(mod);
    changed |= boundedRemChanged;
    if (boundedRemChanged)
      analysisManager.invalidateModule(analysis::AnalysisInvalidation::Effects);
  }
  return changed;
}

bool valueDependsOnNeedle(Value* value, Value* needle,
                          std::unordered_set<int>& visiting,
                          int depth = 0) {
  if (!value || !needle || depth > 32)
    return false;
  if (value == needle)
    return true;
  if (value->isConstant() || dynamic_cast<Argument*>(value) ||
      dynamic_cast<GlobalVar*>(value) || dynamic_cast<BasicBlock*>(value)) {
    return false;
  }
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !visiting.insert(inst->id()).second)
    return false;
  for (auto* op : inst->operands()) {
    if (valueDependsOnNeedle(op, needle, visiting, depth + 1)) {
      visiting.erase(inst->id());
      return true;
    }
  }
  visiting.erase(inst->id());
  return false;
}

// 证明外层重复规约对累加器是平移变换：F(acc) = acc + delta。
// 从累加 PHI 沿 use 图传播污点，只允许“恰好一个污点操作数”的加法、
// “左侧污点”的减法以及保持位模式的 Mov/BitCast。PHI 除外层累加 PHI
// 之外，所有数值输入都必须带污点；这样既覆盖内层规约 PHI，也拒绝按
// 累加器条件选择常量、乘法放大旧值或在地址/分支中使用累加器的情况。
bool repeatedReductionHasTranslationTransition(
    const analysis::NaturalLoop& loop,
    Instruction* accPhi,
    Value* accNext) {
  if (!accPhi || !accNext)
    return false;

  std::unordered_set<Value*> tainted;
  std::vector<Value*> worklist;
  tainted.insert(accPhi);
  worklist.push_back(accPhi);

  for (size_t index = 0; index < worklist.size(); ++index) {
    Value* value = worklist[index];
    for (Instruction* user : value->uses()) {
      if (!user || !user->parent())
        return false;
      if (!loop.blocks.count(user->parent())) {
        // 最终累加值允许在循环外读取；中间污点值不得逃逸。
        if (value == accPhi && user->opcode() != Opcode::Phi)
          continue;
        return false;
      }
      // accNext 作为回边输入重新连到外层累加 PHI。
      if (user == accPhi)
        continue;

      switch (user->opcode()) {
      case Opcode::Phi:
      case Opcode::Add:
      case Opcode::Sub:
      case Opcode::Mov:
      case Opcode::BitCast:
        if (tainted.insert(user).second)
          worklist.push_back(user);
        break;
      default:
        return false;
      }
    }
  }

  if (!tainted.count(accNext))
    return false;
  for (Value* value : tainted) {
    auto* inst = dynamic_cast<Instruction*>(value);
    if (!inst || inst == accPhi)
      continue;
    switch (inst->opcode()) {
    case Opcode::Phi:
      for (int i = 0; i + 1 < inst->numOperands(); i += 2) {
        if (!tainted.count(inst->operand(i)))
          return false;
      }
      break;
    case Opcode::Add: {
      if (inst->numOperands() != 2)
        return false;
      const int taintedOperands =
          static_cast<int>(tainted.count(inst->operand(0))) +
          static_cast<int>(tainted.count(inst->operand(1)));
      if (taintedOperands != 1)
        return false;
      break;
    }
    case Opcode::Sub:
      if (inst->numOperands() != 2 ||
          !tainted.count(inst->operand(0)) ||
          tainted.count(inst->operand(1))) {
        return false;
      }
      break;
    case Opcode::Mov:
    case Opcode::BitCast:
      if (inst->numOperands() != 1 ||
          !tainted.count(inst->operand(0))) {
        return false;
      }
      break;
    default:
      return false;
    }
  }
  return true;
}

bool loopUsesIVOnlyForControl(const analysis::NaturalLoop& loop,
                              const hir::CanonicalLoop& cl) {
  for (auto* bb : loop.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst || inst == cl.cmp || inst == cl.ivNext || inst == cl.ivPhi)
        continue;
      for (auto* op : inst->operands()) {
        if (op == cl.cmp || dynamic_cast<BasicBlock*>(op))
          continue;
        std::unordered_set<int> visiting;
        if (valueDependsOnNeedle(op, cl.ivPhi, visiting))
          return false;
      }
    }
  }
  return true;
}

bool loopHasOnlyReadEffects(const analysis::NaturalLoop& loop) {
  for (auto* bb : loop.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      switch (inst->opcode()) {
      case Opcode::Store:
      case Opcode::VStore:
      case Opcode::Call:
      case Opcode::Ret:
      case Opcode::Alloca:
        return false;
      default:
        break;
      }
    }
  }
  return true;
}

// 正迭代次数的归约在物化累加器后可以只保留一次循环体执行；循环内不读取
// 根对象时，不变量标量存储也保留在这一次迭代中。
bool collectInvariantLoopStores(const analysis::NaturalLoop& loop,
                                std::vector<Instruction*>& stores) {
  stores.clear();
  std::unordered_set<Value*> reads;
  std::unordered_set<Value*> writes;
  for (auto* bb : loop.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      switch (inst->opcode()) {
      case Opcode::Call:
      case Opcode::Ret:
      case Opcode::Alloca:
      case Opcode::VStore:
      case Opcode::VLoad:
        return false;
      case Opcode::Load: {
        if (inst->numOperands() != 1)
          return false;
        Value* root = memoryRoot(inst->operand(0));
        if (!root)
          return false;
        reads.insert(root);
        break;
      }
      case Opcode::Store: {
        if (inst->numOperands() != 2)
          return false;
        auto isDefinedInLoop = [&](Value* value) {
          auto* def = dynamic_cast<Instruction*>(value);
          return def && def->parent() && loop.blocks.count(def->parent());
        };
        if (isDefinedInLoop(inst->operand(0)) ||
            isDefinedInLoop(inst->operand(1)))
          return false;
        Value* root = memoryRoot(inst->operand(1));
        if (!root || reads.count(root) || !writes.insert(root).second)
          return false;
        stores.push_back(inst);
        break;
      }
      default:
        break;
      }
    }
  }
  return !stores.empty();
}

bool hasGuaranteedPositiveConstantTrip(const hir::CanonicalLoop& loop) {
  auto* init = dynamic_cast<ConstInt*>(loop.initVal);
  auto* bound = dynamic_cast<ConstInt*>(loop.boundVal);
  return init && bound &&
         static_cast<int64_t>(bound->value()) > init->value();
}

bool hasRepeatedReductionSyntheticBound(Value* value) {
  auto* inst = dynamic_cast<Instruction*>(value);
  return inst && inst->name().find("repeat.once.bound") != std::string::npos;
}

void insertBeforeFirstNonPhi(BasicBlock* bb, Instruction* inst) {
  if (!bb || !inst)
    return;
  for (auto& instPtr : bb->instructions()) {
    if (instPtr->opcode() == Opcode::Phi)
      continue;
    bb->insertBefore(instPtr.get(), inst);
    return;
  }
  bb->pushBack(inst);
}

Instruction* makeRepeatedReductionInst(Opcode op, Type ty,
                                       const std::string& name,
                                       std::initializer_list<Value*> ops) {
  auto* inst = new Instruction(op, ty, name);
  for (auto* opnd : ops)
    inst->addOperand(opnd);
  return inst;
}

bool materializeRepeatedReduction(Function* func,
                                  const analysis::NaturalLoop& naturalLoop,
                                  const hir::CanonicalLoop& cl,
                                  Instruction* accPhi,
                                  Value* accInit,
                                  Value* accNext,
                                  const std::vector<Instruction*>& stores) {
  if (!func || !func->parent() || !accPhi || !accInit || !accNext)
    return false;
  if (hasRepeatedReductionSyntheticBound(cl.boundVal))
    return false;

  std::vector<Instruction*> outsideUsers;
  for (auto* user : accPhi->uses()) {
    if (!user || !user->parent())
      return false;
    if (naturalLoop.blocks.count(user->parent()))
      continue;
    if (user->opcode() == Opcode::Phi)
      return false;
    outsideUsers.push_back(user);
  }
  if (outsideUsers.empty())
    return false;

  auto* preTerm = cl.preheader->terminator();
  if (!preTerm)
    return false;
  // 保留一次循环迭代中的不变量存储。改写后的单次上界循环在正迭代路径上
  // 执行一次循环体，也让后续阶段继续利用存储到加载的前递。
  (void)stores;

  // onceBound = init + (init < bound ? 1 : 0)，保留零迭代循环，并让正迭代
  // 的重复归约只执行一次。
  auto* hasTrip = makeRepeatedReductionInst(Opcode::ICmp, Type::i32(),
                                            "lt_repeat.has_trip",
                                            {cl.initVal, cl.boundVal});
  cl.preheader->insertBefore(preTerm, hasTrip);
  auto* onceBound = makeRepeatedReductionInst(Opcode::Add, Type::i32(),
                                              "repeat.once.bound",
                                              {cl.initVal, hasTrip});
  cl.preheader->insertBefore(preTerm, onceBound);

  if (cl.ivIsCmpLhs)
    cl.cmp->setOperand(1, onceBound);
  else
    cl.cmp->setOperand(0, onceBound);

  auto* tripCount = makeRepeatedReductionInst(Opcode::Sub, Type::i32(),
                                              "repeat.trip",
                                              {cl.boundVal, cl.initVal});
  insertBeforeFirstNonPhi(cl.exit, tripCount);
  auto* delta = makeRepeatedReductionInst(Opcode::Sub, Type::i32(),
                                          "repeat.delta",
                                          {accPhi, accInit});
  insertBeforeFirstNonPhi(cl.exit, delta);
  auto* scaled = makeRepeatedReductionInst(Opcode::Mul, Type::i32(),
                                           "repeat.scaled",
                                           {delta, tripCount});
  insertBeforeFirstNonPhi(cl.exit, scaled);
  auto* finalValue = makeRepeatedReductionInst(Opcode::Add, Type::i32(),
                                               "repeat.final",
                                               {accInit, scaled});
  insertBeforeFirstNonPhi(cl.exit, finalValue);

  for (auto* user : outsideUsers) {
    for (int i = 0; i < user->numOperands(); ++i) {
      if (user->operand(i) == accPhi)
        user->setOperand(i, finalValue);
    }
  }

  func->recomputeCFG();
  return true;
}

bool matchRepeatedReductionLoop(const analysis::NaturalLoop& loop,
                                hir::CanonicalLoop& cl,
                                Instruction*& accPhi,
                                Value*& accInit,
                                Value*& accNext,
                                std::vector<Instruction*>& invariantStores) {
  invariantStores.clear();
  auto reject = [&](const char* reason) {
    if (sycEnvironmentValue("REPEATED_REDUCTION_TRACE")) {
      std::cerr << "[repeated-reduction] reject header="
                << (loop.header ? loop.header->name() : std::string("<null>"))
                << " reason=" << (reason ? reason : "-") << "\n";
    }
    return false;
  };
  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader || !hir::recognizeCanonicalLoop(loop, preheader, cl))
    return reject("not-canonical");
  if (!cl.ivIsCmpLhs || hir::canonicalLoopCmpCond(cl.cmp) != "lt")
    return reject("not-forward-lt");
  if (!cl.ivPhi->type().isI32() || !cl.boundVal || !cl.boundVal->type().isI32())
    return reject("non-i32");
  if (!loopHasSingleExit(loop, cl.exit))
    return reject("multi-exit");
  if (loop.blocks.size() < 4)
    return reject("too-small");
  if (!loopHasOnlyReadEffects(loop)) {
    if (!collectInvariantLoopStores(loop, invariantStores) ||
        !hasGuaranteedPositiveConstantTrip(cl))
      return reject("effects");
  }
  if (!loopUsesIVOnlyForControl(loop, cl))
    return reject("iv-use");

  std::vector<Instruction*> phis;
  std::vector<Value*> initVals;
  std::vector<Value*> nextVals;
  if (!hir::collectCanonicalLoopPhis(cl, phis, initVals, &nextVals))
    return reject("phis");

  accPhi = nullptr;
  accInit = nullptr;
  accNext = nullptr;
  for (size_t i = 0; i < phis.size(); ++i) {
    Instruction* phi = phis[i];
    if (phi == cl.ivPhi)
      continue;
    if (!phi->type().isI32())
      continue;
    Value* candidateNext = nextVals[i];
    if (!candidateNext || candidateNext == phi)
      continue;
    std::unordered_set<int> visiting;
    if (valueDependsOnNeedle(candidateNext, cl.ivPhi, visiting))
      continue;
    if (accPhi)
      return reject("multi-acc");
    accPhi = phi;
    accInit = initVals[i];
    accNext = candidateNext;
  }
  if (!accPhi)
    return reject("no-acc");
  if (!repeatedReductionHasTranslationTransition(loop, accPhi, accNext))
    return reject("non-translation-accumulator");

  bool hasLoad = false;
  bool hasArithmetic = false;
  for (auto* bb : loop.blocks) {
    for (auto& instPtr : bb->instructions()) {
      Opcode op = instPtr->opcode();
      hasLoad |= op == Opcode::Load || op == Opcode::VLoad;
      hasArithmetic |= op == Opcode::Add || op == Opcode::Sub ||
                       op == Opcode::Mul || op == Opcode::SDiv ||
                       op == Opcode::SRem;
    }
  }
  if (!hasLoad || !hasArithmetic)
    return reject("no-work");
  return true;
}

bool applyRepeatedReductionLoop(Function* func,
                                analysis::AnalysisManager& analysisManager) {
  if (!func || !func->entryBlock())
    return false;
  analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
  auto loops = cache.loops();
  if (loops.empty())
    return false;
  std::sort(loops.begin(), loops.end(),
            [](const analysis::NaturalLoop& lhs,
               const analysis::NaturalLoop& rhs) {
              if (lhs.blocks.size() != rhs.blocks.size())
                return lhs.blocks.size() > rhs.blocks.size();
              return lhs.header && rhs.header
                         ? lhs.header->id() < rhs.header->id()
                         : lhs.header < rhs.header;
            });
  const bool trace = sycEnvironmentValue("REPEATED_REDUCTION_TRACE") != nullptr;
  for (const auto& loop : loops) {
    hir::CanonicalLoop cl;
    Instruction* accPhi = nullptr;
    Value* accInit = nullptr;
    Value* accNext = nullptr;
    std::vector<Instruction*> invariantStores;
    if (!matchRepeatedReductionLoop(loop, cl, accPhi, accInit, accNext,
                                    invariantStores))
      continue;
    if (materializeRepeatedReduction(func, loop, cl, accPhi, accInit, accNext,
                                     invariantStores)) {
      if (trace) {
        std::cerr << "[repeated-reduction] func=" << func->name()
                  << " header=" << loop.header->name()
                  << " acc=" << accPhi->name() << "\n";
      }
      return true;
    }
  }
  return false;
}

bool applyRepeatedReductionLoops(Module& mod,
                                 analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func)
      continue;
    if (applyRepeatedReductionLoop(func, analysisManager)) {
      changed = true;
      analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
    }
  }
  return changed;
}

struct LinearRecurrenceExpr {
  bool valid = false;
  std::vector<int32_t> coeffs;
};

struct LinearRecurrencePlan {
  hir::CanonicalLoop loop;
  std::vector<Instruction*> states;
  std::vector<Value*> initValues;
  std::vector<std::vector<int32_t>> matrix;
  bool ivHasOutsideUse = false;
};

constexpr int32_t kLinearRecurrenceFastThreshold = 1 << 17;

bool linearRecurrenceTraceEnabled() {
  return sycEnvironmentValue("SYC_TRACE_COREIR_LINEAR_RECURRENCE") != nullptr;
}

bool rejectLinearRecurrence(const analysis::NaturalLoop& loop,
                            const char* reason) {
  if (linearRecurrenceTraceEnabled()) {
    std::cerr << "[linear-recurrence] reject header="
              << (loop.header ? loop.header->name() : std::string("<null>"))
              << " reason=" << (reason ? reason : "-") << "\n";
  }
  return false;
}

bool combineLinearExpr(const LinearRecurrenceExpr& lhs,
                       const LinearRecurrenceExpr& rhs,
                       int rhsSign,
                       LinearRecurrenceExpr& out) {
  if (!lhs.valid || !rhs.valid || lhs.coeffs.size() != rhs.coeffs.size())
    return false;
  out.valid = true;
  out.coeffs.assign(lhs.coeffs.size(), 0);
  for (size_t i = 0; i < lhs.coeffs.size(); ++i) {
    int64_t value = static_cast<int64_t>(lhs.coeffs[i]) +
                    static_cast<int64_t>(rhsSign) * rhs.coeffs[i];
    if (value < -16 || value > 16)
      return false;
    out.coeffs[i] = static_cast<int32_t>(value);
  }
  return true;
}

bool valueToLinearExpr(
    Value* value,
    const std::vector<Instruction*>& states,
    const std::unordered_set<BasicBlock*>& loopBlocks,
    std::unordered_map<Value*, LinearRecurrenceExpr>& memo,
    LinearRecurrenceExpr& out,
    int depth = 0) {
  if (!value || depth > 16)
    return false;
  auto memoIt = memo.find(value);
  if (memoIt != memo.end()) {
    out = memoIt->second;
    return out.valid;
  }

  for (size_t i = 0; i < states.size(); ++i) {
    if (value == states[i]) {
      out.valid = true;
      out.coeffs.assign(states.size(), 0);
      out.coeffs[i] = 1;
      memo[value] = out;
      return true;
    }
  }

  if (auto* ci = dynamic_cast<ConstInt*>(value)) {
    if (ci->value() != 0)
      return false;
    out.valid = true;
    out.coeffs.assign(states.size(), 0);
    memo[value] = out;
    return true;
  }

  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !inst->parent() || !loopBlocks.count(inst->parent()))
    return false;

  if (inst->opcode() == Opcode::Add || inst->opcode() == Opcode::Sub) {
    LinearRecurrenceExpr lhs;
    LinearRecurrenceExpr rhs;
    if (!valueToLinearExpr(inst->operand(0), states, loopBlocks, memo, lhs,
                           depth + 1) ||
        !valueToLinearExpr(inst->operand(1), states, loopBlocks, memo, rhs,
                           depth + 1)) {
      return false;
    }
    if (!combineLinearExpr(lhs, rhs, inst->opcode() == Opcode::Sub ? -1 : 1,
                           out)) {
      return false;
    }
    memo[value] = out;
    return true;
  }

  if (inst->opcode() == Opcode::Neg && inst->numOperands() == 1) {
    LinearRecurrenceExpr inner;
    if (!valueToLinearExpr(inst->operand(0), states, loopBlocks, memo, inner,
                           depth + 1)) {
      return false;
    }
    out = inner;
    for (int32_t& coeff : out.coeffs)
      coeff = -coeff;
    memo[value] = out;
    return true;
  }

  return false;
}

bool linearRecurrenceDefinitionsAreBypassable(
    const analysis::NaturalLoop& loop,
    const std::unordered_set<Instruction*>& allowedOutsideDefs) {
  for (auto* bb : loop.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      switch (inst->opcode()) {
      case Opcode::Store:
      case Opcode::VStore:
      case Opcode::Call:
      case Opcode::Ret:
      case Opcode::Alloca:
      case Opcode::Load:
      case Opcode::VLoad:
        return false;
      default:
        break;
      }

      for (Instruction* user : inst->uses()) {
        if (!user || !user->parent() || loop.blocks.count(user->parent()))
          continue;
        if (!allowedOutsideDefs.count(inst))
          return false;
      }
    }
  }
  return true;
}

bool matchSmallLinearRecurrenceLoop(const analysis::NaturalLoop& naturalLoop,
                                    analysis::FunctionAnalysisCache& cache,
                                    LinearRecurrencePlan& plan) {
  BasicBlock* preheader = analysis::dedicatedPreheader(naturalLoop);
  hir::CanonicalLoop loop;
  if (!preheader || !hir::recognizeCanonicalLoop(naturalLoop, preheader, loop))
    return rejectLinearRecurrence(naturalLoop, "not-canonical");
  if (!loop.ivIsCmpLhs || hir::canonicalLoopCmpCond(loop.cmp) != "lt")
    return rejectLinearRecurrence(naturalLoop, "not-forward-lt");
  if (!isConstZero(loop.initVal) || !loop.boundVal || !loop.boundVal->type().isI32())
    return rejectLinearRecurrence(naturalLoop, "bad-bound");
  if (!loopHasSingleExit(naturalLoop, loop.exit))
    return rejectLinearRecurrence(naturalLoop, "multi-exit");
  if (!analysis::collectLeadingPhi(loop.exit).empty())
    return rejectLinearRecurrence(naturalLoop, "exit-phi");
  if (!loopHasOnlyReadEffects(naturalLoop))
    return rejectLinearRecurrence(naturalLoop, "effects");
  if (!loopUsesIVOnlyForControl(naturalLoop, loop))
    return rejectLinearRecurrence(naturalLoop, "iv-use");

  const DomSet& doms = cache.dominators();
  if (!valueAvailableAtBlock(loop.boundVal, loop.preheader, naturalLoop, doms))
    return rejectLinearRecurrence(naturalLoop, "bound-not-available");

  std::vector<Instruction*> phis;
  std::vector<Value*> initVals;
  std::vector<Value*> nextVals;
  if (!hir::collectCanonicalLoopPhis(loop, phis, initVals, &nextVals))
    return rejectLinearRecurrence(naturalLoop, "phis");

  std::vector<Instruction*> states;
  std::vector<Value*> stateInits;
  std::vector<Value*> stateNexts;
  for (size_t i = 0; i < phis.size(); ++i) {
    Instruction* phi = phis[i];
    if (!phi || phi == loop.ivPhi)
      continue;
    if (!phi->type().isI32())
      return rejectLinearRecurrence(naturalLoop, "non-i32-state");
    if (!valueAvailableAtBlock(initVals[i], loop.preheader, naturalLoop, doms))
      return rejectLinearRecurrence(naturalLoop, "init-not-available");
    states.push_back(phi);
    stateInits.push_back(initVals[i]);
    stateNexts.push_back(nextVals[i]);
  }
  if (states.size() < 2 || states.size() > 4)
    return rejectLinearRecurrence(naturalLoop, "state-count");

  std::unordered_map<Value*, LinearRecurrenceExpr> memo;
  std::vector<std::vector<int32_t>> matrix;
  matrix.reserve(states.size());
  for (Value* next : stateNexts) {
    LinearRecurrenceExpr expr;
    if (!valueToLinearExpr(next, states, naturalLoop.blocks, memo, expr))
      return rejectLinearRecurrence(naturalLoop, "non-linear-next");
    matrix.push_back(expr.coeffs);
  }

  bool nonIdentity = false;
  for (size_t row = 0; row < matrix.size(); ++row) {
    for (size_t col = 0; col < matrix[row].size(); ++col) {
      int32_t expected = row == col ? 1 : 0;
      if (matrix[row][col] != expected)
        nonIdentity = true;
    }
  }
  if (!nonIdentity)
    return rejectLinearRecurrence(naturalLoop, "identity");

  std::unordered_set<Instruction*> allowedOutsideDefs(states.begin(), states.end());
  allowedOutsideDefs.insert(loop.ivPhi);
  if (!linearRecurrenceDefinitionsAreBypassable(naturalLoop, allowedOutsideDefs))
    return rejectLinearRecurrence(naturalLoop, "outside-use");

  plan.loop = loop;
  plan.states = std::move(states);
  plan.initValues = std::move(stateInits);
  plan.matrix = std::move(matrix);
  plan.ivHasOutsideUse = valueHasOutsideUse(loop.ivPhi, naturalLoop);
  return true;
}

Value* appendDynamicDot(Module& mod, BasicBlock* bb,
                        const std::vector<Value*>& lhs,
                        const std::vector<Value*>& rhs,
                        const std::string& name) {
  Value* acc = nullptr;
  for (size_t i = 0; i < lhs.size(); ++i) {
    Value* product = appendModRecInst(bb, Opcode::Mul, Type::i32(),
                                      name + ".mul", {lhs[i], rhs[i]});
    if (!acc) {
      acc = product;
    } else {
      acc = appendModRecInst(bb, Opcode::Add, Type::i32(), name + ".add",
                             {acc, product});
    }
  }
  return acc ? acc : mod.getConstInt(0);
}

int32_t wrapLinRecI32(int64_t value) {
  return static_cast<int32_t>(static_cast<uint32_t>(value));
}

int32_t addLinRecI32(int32_t lhs, int32_t rhs) {
  return wrapLinRecI32(static_cast<uint32_t>(lhs) + static_cast<uint32_t>(rhs));
}

int32_t subLinRecI32(int32_t lhs, int32_t rhs) {
  return wrapLinRecI32(static_cast<uint32_t>(lhs) - static_cast<uint32_t>(rhs));
}

int32_t mulLinRecI32(int32_t lhs, int32_t rhs) {
  return wrapLinRecI32(static_cast<int64_t>(lhs) * static_cast<int64_t>(rhs));
}

Value* appendLinRecAdd(Module& mod, BasicBlock* bb, Value* lhs, Value* rhs,
                       const std::string& name) {
  if (isConstZero(lhs))
    return rhs;
  if (isConstZero(rhs))
    return lhs;
  auto* lc = constIntValue(lhs);
  auto* rc = constIntValue(rhs);
  if (lc && rc)
    return mod.getConstInt(addLinRecI32(lc->value(), rc->value()));
  return appendModRecInst(bb, Opcode::Add, Type::i32(), name, {lhs, rhs});
}

Value* appendLinRecSub(Module& mod, BasicBlock* bb, Value* lhs, Value* rhs,
                       const std::string& name) {
  if (isConstZero(rhs))
    return lhs;
  auto* lc = constIntValue(lhs);
  auto* rc = constIntValue(rhs);
  if (lc && rc)
    return mod.getConstInt(subLinRecI32(lc->value(), rc->value()));
  return appendModRecInst(bb, Opcode::Sub, Type::i32(), name, {lhs, rhs});
}

Value* appendLinRecMul(Module& mod, BasicBlock* bb, Value* lhs, Value* rhs,
                       const std::string& name) {
  if (isConstZero(lhs) || isConstZero(rhs))
    return mod.getConstInt(0);
  if (isConstOne(lhs))
    return rhs;
  if (isConstOne(rhs))
    return lhs;
  auto* lc = constIntValue(lhs);
  auto* rc = constIntValue(rhs);
  if (lc && rc)
    return mod.getConstInt(mulLinRecI32(lc->value(), rc->value()));
  return appendModRecInst(bb, Opcode::Mul, Type::i32(), name, {lhs, rhs});
}

Value* appendLinRecMulConst(Module& mod, BasicBlock* bb, Value* value,
                            int32_t coeff, const std::string& name) {
  if (coeff == 0 || isConstZero(value))
    return mod.getConstInt(0);
  if (coeff == 1)
    return value;
  if (auto* ci = constIntValue(value))
    return mod.getConstInt(mulLinRecI32(ci->value(), coeff));
  if (coeff == -1)
    return appendModRecInst(bb, Opcode::Sub, Type::i32(), name + ".neg",
                            {mod.getConstInt(0), value});
  return appendModRecInst(bb, Opcode::Mul, Type::i32(), name,
                          {value, mod.getConstInt(coeff)});
}

using LinRecIntPoly = std::vector<int64_t>;

LinRecIntPoly multiplyIntPoly(const LinRecIntPoly& lhs,
                              const LinRecIntPoly& rhs) {
  LinRecIntPoly out(lhs.size() + rhs.size() - 1, 0);
  for (size_t i = 0; i < lhs.size(); ++i) {
    for (size_t j = 0; j < rhs.size(); ++j)
      out[i + j] += lhs[i] * rhs[j];
  }
  return out;
}

int permutationSign(const std::vector<int>& perm) {
  int inversions = 0;
  for (size_t i = 0; i < perm.size(); ++i) {
    for (size_t j = i + 1; j < perm.size(); ++j) {
      if (perm[i] > perm[j])
        ++inversions;
    }
  }
  return (inversions % 2) == 0 ? 1 : -1;
}

std::vector<int32_t> characteristicLowCoeffs(
    const std::vector<std::vector<int32_t>>& matrix) {
  const size_t n = matrix.size();
  LinRecIntPoly det(n + 1, 0);
  std::vector<int> perm(n);
  for (size_t i = 0; i < n; ++i)
    perm[i] = static_cast<int>(i);

  do {
    LinRecIntPoly term(1, permutationSign(perm));
    for (size_t row = 0; row < n; ++row) {
      const size_t col = static_cast<size_t>(perm[row]);
      LinRecIntPoly entry;
      if (row == col) {
        entry = {-matrix[row][col], 1};
      } else {
        entry = {-matrix[row][col]};
      }
      term = multiplyIntPoly(term, entry);
    }
    for (size_t i = 0; i < term.size(); ++i)
      det[i] += term[i];
  } while (std::next_permutation(perm.begin(), perm.end()));

  if (det.size() != n + 1 || det[n] != 1)
    return {};
  std::vector<int32_t> coeffs(n, 0);
  for (size_t i = 0; i < n; ++i)
    coeffs[i] = wrapLinRecI32(det[i]);
  return coeffs;
}

std::vector<std::vector<int32_t>> multiplyConstMatrix(
    const std::vector<std::vector<int32_t>>& lhs,
    const std::vector<std::vector<int32_t>>& rhs) {
  const size_t n = lhs.size();
  std::vector<std::vector<int32_t>> out(n, std::vector<int32_t>(n, 0));
  for (size_t row = 0; row < n; ++row) {
    for (size_t col = 0; col < n; ++col) {
      int32_t acc = 0;
      for (size_t k = 0; k < n; ++k)
        acc = addLinRecI32(acc, mulLinRecI32(lhs[row][k], rhs[k][col]));
      out[row][col] = acc;
    }
  }
  return out;
}

std::vector<std::vector<std::vector<int32_t>>> matrixPowerBasis(
    const std::vector<std::vector<int32_t>>& matrix) {
  const size_t n = matrix.size();
  std::vector<std::vector<std::vector<int32_t>>> powers;
  powers.reserve(n);
  std::vector<std::vector<int32_t>> cur(n, std::vector<int32_t>(n, 0));
  for (size_t i = 0; i < n; ++i)
    cur[i][i] = 1;
  for (size_t i = 0; i < n; ++i) {
    powers.push_back(cur);
    cur = multiplyConstMatrix(matrix, cur);
  }
  return powers;
}

std::vector<Value*> appendPolynomialMulMod(
    Module& mod,
    BasicBlock* bb,
    const std::vector<Value*>& lhs,
    const std::vector<Value*>& rhs,
    const std::vector<int32_t>& charCoeffs,
    const std::string& name) {
  const size_t n = charCoeffs.size();
  std::vector<Value*> tmp(2 * n - 1, mod.getConstInt(0));
  for (size_t i = 0; i < n; ++i) {
    for (size_t j = 0; j < n; ++j) {
      Value* product = appendLinRecMul(
          mod, bb, lhs[i], rhs[j],
          name + ".mul." + std::to_string(i) + "." + std::to_string(j));
      tmp[i + j] = appendLinRecAdd(
          mod, bb, tmp[i + j], product,
          name + ".acc." + std::to_string(i) + "." + std::to_string(j));
    }
  }

  for (int deg = static_cast<int>(2 * n - 2); deg >= static_cast<int>(n);
       --deg) {
    Value* coeff = tmp[deg];
    if (isConstZero(coeff))
      continue;
    for (size_t i = 0; i < n; ++i) {
      if (charCoeffs[i] == 0)
        continue;
      Value* term = appendLinRecMulConst(
          mod, bb, coeff, charCoeffs[i],
          name + ".red.mul." + std::to_string(deg) + "." + std::to_string(i));
      tmp[deg - static_cast<int>(n) + static_cast<int>(i)] =
          appendLinRecSub(
              mod, bb,
              tmp[deg - static_cast<int>(n) + static_cast<int>(i)], term,
              name + ".red.sub." + std::to_string(deg) + "." +
                  std::to_string(i));
    }
  }

  std::vector<Value*> out;
  out.reserve(n);
  for (size_t i = 0; i < n; ++i)
    out.push_back(tmp[i]);
  return out;
}

Instruction* appendPhi(BasicBlock* bb, const std::string& name,
                       Value* value, BasicBlock* pred) {
  auto* phi = new Instruction(Opcode::Phi, Type::i32(), name);
  phi->addOperand(value);
  phi->addOperand(pred);
  bb->pushBack(phi);
  return phi;
}

bool materializeSmallLinearRecurrenceFastPath(
    Function* func,
    Module& mod,
    const analysis::NaturalLoop& naturalLoop,
    const LinearRecurrencePlan& plan) {
  if (!func || !plan.loop.preheader || !plan.loop.header || !plan.loop.exit ||
      plan.states.empty() || plan.states.size() != plan.matrix.size()) {
    return false;
  }

  Instruction* preTerm = plan.loop.preheader->terminator();
  if (!preTerm || preTerm->opcode() != Opcode::Br ||
      preTerm->numOperands() != 1 || preTerm->operand(0) != plan.loop.header) {
    return false;
  }

  auto* constBound = constIntValue(plan.loop.boundVal);
  // 带保护的 O(log n) 路径有固定的代码体积和寄存器代价。迭代次数只能在运行
  // 时确定时，不生成可能永远不执行的冷矩阵幂 CFG；只对编译期已知较大的
  // 递推使用该路径，使固定代价能够摊平。
  if (!constBound || constBound->value() < kLinearRecurrenceFastThreshold)
    return false;

  // 二进制幂降低后仍有明显的固定代价。小型和中型迭代次数继续使用紧凑的
  // 标量循环，只有 O(log n) 更有利的范围才使用多项式路径。
  auto* useScalarLoop = insertModRecICmpBefore(
      plan.loop.preheader, preTerm, "lt", "linrec.trip.scalar",
      plan.loop.boundVal, mod.getConstInt(kLinearRecurrenceFastThreshold));

  const size_t n = plan.states.size();
  std::vector<int32_t> charCoeffs = characteristicLowCoeffs(plan.matrix);
  if (charCoeffs.size() != n)
    return false;
  std::vector<std::vector<std::vector<int32_t>>> powers =
      matrixPowerBasis(plan.matrix);
  if (powers.size() != n)
    return false;

  BasicBlock* fastEntry = func->newBlock("linrec.fast.entry");
  BasicBlock* fastHead = func->newBlock("linrec.fast.head");
  BasicBlock* fastBody = func->newBlock("linrec.fast.body");
  BasicBlock* fastApply = func->newBlock("linrec.fast.apply");
  BasicBlock* fastSquare = func->newBlock("linrec.fast.square");
  BasicBlock* fastExit = func->newBlock("linrec.fast.exit");

  plan.loop.preheader->remove(preTerm);
  auto* dispatch = new Instruction(Opcode::CondBr, Type::voidTy());
  dispatch->addOperand(useScalarLoop);
  dispatch->addOperand(plan.loop.header);
  dispatch->addOperand(fastEntry);
  plan.loop.preheader->pushBack(dispatch);

  auto* entryBr = new Instruction(Opcode::Br, Type::voidTy());
  entryBr->addOperand(fastHead);
  fastEntry->pushBack(entryBr);

  Instruction* nPhi =
      appendPhi(fastHead, "linrec.n", plan.loop.boundVal, fastEntry);
  std::vector<Instruction*> coeffPhi;
  coeffPhi.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    coeffPhi.push_back(appendPhi(fastHead, "linrec.coeff" + std::to_string(i),
                                 mod.getConstInt(i == 0 ? 1 : 0), fastEntry));
  }
  std::vector<Instruction*> basePhi;
  basePhi.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    basePhi.push_back(appendPhi(fastHead, "linrec.base" + std::to_string(i),
                                mod.getConstInt(i == 1 ? 1 : 0), fastEntry));
  }
  auto* hasWork = appendModRecInst(fastHead, Opcode::ICmp, Type::i32(),
                                   "gt_linrec.has.work",
                                   {nPhi, mod.getConstInt(0)});
  auto* headBr = new Instruction(Opcode::CondBr, Type::voidTy());
  headBr->addOperand(hasWork);
  headBr->addOperand(fastBody);
  headBr->addOperand(fastExit);
  fastHead->pushBack(headBr);

  auto* oddBit = appendModRecInst(fastBody, Opcode::And, Type::i32(),
                                  "linrec.odd.bit",
                                  {nPhi, mod.getConstInt(1)});
  auto* isOdd = appendModRecInst(fastBody, Opcode::ICmp, Type::i32(),
                                 "ne_linrec.odd",
                                 {oddBit, mod.getConstInt(0)});
  auto* bodyBr = new Instruction(Opcode::CondBr, Type::voidTy());
  bodyBr->addOperand(isOdd);
  bodyBr->addOperand(fastApply);
  bodyBr->addOperand(fastSquare);
  fastBody->pushBack(bodyBr);

  std::vector<Value*> curCoeff(coeffPhi.begin(), coeffPhi.end());
  std::vector<Value*> curBase(basePhi.begin(), basePhi.end());
  std::vector<Value*> appliedCoeff = appendPolynomialMulMod(
      mod, fastApply, curCoeff, curBase, charCoeffs, "linrec.poly.apply");
  auto* applyBr = new Instruction(Opcode::Br, Type::voidTy());
  applyBr->addOperand(fastSquare);
  fastApply->pushBack(applyBr);

  std::vector<Instruction*> squareCoeffPhi;
  squareCoeffPhi.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    auto* phi = new Instruction(Opcode::Phi, Type::i32(),
                                "linrec.square.coeff" + std::to_string(i));
    phi->addOperand(coeffPhi[i]);
    phi->addOperand(fastBody);
    phi->addOperand(appliedCoeff[i]);
    phi->addOperand(fastApply);
    fastSquare->pushBack(phi);
    squareCoeffPhi.push_back(phi);
  }
  std::vector<Value*> squareCoeff(squareCoeffPhi.begin(), squareCoeffPhi.end());
  std::vector<Value*> squaredBase = appendPolynomialMulMod(
      mod, fastSquare, curBase, curBase, charCoeffs, "linrec.poly.square");
  auto* nHalf = appendModRecInst(fastSquare, Opcode::Shr, Type::i32(),
                                 "linrec.n.half",
                                 {nPhi, mod.getConstInt(1)});
  auto* squareBr = new Instruction(Opcode::Br, Type::voidTy());
  squareBr->addOperand(fastHead);
  fastSquare->pushBack(squareBr);

  nPhi->addOperand(nHalf);
  nPhi->addOperand(fastSquare);
  for (size_t i = 0; i < n; ++i) {
    coeffPhi[i]->addOperand(squareCoeff[i]);
    coeffPhi[i]->addOperand(fastSquare);
    basePhi[i]->addOperand(squaredBase[i]);
    basePhi[i]->addOperand(fastSquare);
  }

  std::vector<Value*> finalStates;
  finalStates.reserve(n);
  for (size_t row = 0; row < n; ++row) {
    Value* stateAcc = mod.getConstInt(0);
    for (size_t power = 0; power < n; ++power) {
      Value* basisAcc = mod.getConstInt(0);
      for (size_t col = 0; col < n; ++col) {
        int32_t coeff = powers[power][row][col];
        if (coeff == 0)
          continue;
        Value* term = appendLinRecMulConst(
            mod, fastExit, plan.initValues[col], coeff,
            "linrec.basis.mul." + std::to_string(row) + "." +
                std::to_string(power) + "." + std::to_string(col));
        basisAcc = appendLinRecAdd(
            mod, fastExit, basisAcc, term,
            "linrec.basis.add." + std::to_string(row) + "." +
                std::to_string(power) + "." + std::to_string(col));
      }
      Value* term = appendLinRecMul(
          mod, fastExit, coeffPhi[power], basisAcc,
          "linrec.final.mul." + std::to_string(row) + "." +
              std::to_string(power));
      stateAcc = appendLinRecAdd(
          mod, fastExit, stateAcc, term,
          "linrec.final.add." + std::to_string(row) + "." +
              std::to_string(power));
    }
    finalStates.push_back(stateAcc);
  }

  auto* exitBr = new Instruction(Opcode::Br, Type::voidTy());
  exitBr->addOperand(plan.loop.exit);
  fastExit->pushBack(exitBr);

  for (size_t i = 0; i < n; ++i) {
    auto* exitPhi = new Instruction(Opcode::Phi, Type::i32(),
                                    "linrec.state.exit");
    exitPhi->addOperand(plan.states[i]);
    exitPhi->addOperand(plan.loop.header);
    exitPhi->addOperand(finalStates[i]);
    exitPhi->addOperand(fastExit);
    insertBeforeFirstNonPhi(plan.loop.exit, exitPhi);
    replaceOutsideUses(plan.states[i], exitPhi, naturalLoop);
  }

  if (plan.ivHasOutsideUse) {
    auto* ivExit = new Instruction(Opcode::Phi, Type::i32(), "linrec.iv.exit");
    ivExit->addOperand(plan.loop.ivPhi);
    ivExit->addOperand(plan.loop.header);
    ivExit->addOperand(plan.loop.boundVal);
    ivExit->addOperand(fastExit);
    insertBeforeFirstNonPhi(plan.loop.exit, ivExit);
    replaceOutsideUses(plan.loop.ivPhi, ivExit, naturalLoop);
  }

  func->recomputeCFG();
  if (linearRecurrenceTraceEnabled()) {
    std::cerr << "[linear-recurrence] func=" << func->name()
              << " header=" << plan.loop.header->name()
              << " states=" << n << "\n";
  }
  return true;
}

bool applySmallLinearRecurrenceLoop(Function* func,
                                    Module& mod,
                                    analysis::AnalysisManager& analysisManager) {
  if (!func || !func->entryBlock())
    return false;
  analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
  auto loops = cache.loops();
  if (loops.empty())
    return false;
  std::sort(loops.begin(), loops.end(),
            [](const analysis::NaturalLoop& lhs,
               const analysis::NaturalLoop& rhs) {
              if (lhs.blocks.size() != rhs.blocks.size())
                return lhs.blocks.size() < rhs.blocks.size();
              return lhs.header && rhs.header
                         ? lhs.header->id() < rhs.header->id()
                         : lhs.header < rhs.header;
            });
  for (const auto& loop : loops) {
    LinearRecurrencePlan plan;
    if (!matchSmallLinearRecurrenceLoop(loop, cache, plan))
      continue;
    if (materializeSmallLinearRecurrenceFastPath(func, mod, loop, plan))
      return true;
  }
  return false;
}

bool applySmallLinearRecurrenceLoops(Module& mod,
                                     analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  bool localChanged = true;
  int budget = 8;
  while (localChanged && budget-- > 0) {
    localChanged = false;
    for (auto& funcPtr : mod.functions()) {
      Function* func = funcPtr.get();
      if (!func)
        continue;
      if (applySmallLinearRecurrenceLoop(func, mod, analysisManager)) {
        changed = true;
        localChanged = true;
        analysisManager.invalidateFunction(
            func, analysis::AnalysisInvalidation::CFG);
        break;
      }
    }
  }
  return changed;
}

struct OverwriteLoopEffects {
  std::unordered_set<Value*> reads;
  std::unordered_set<Value*> writes;
  bool impure = false;
};

bool isPureOverwriteLoopInst(Instruction* inst) {
  if (!inst)
    return false;
  switch (inst->opcode()) {
  case Opcode::Br:
  case Opcode::CondBr:
  case Opcode::Phi:
  case Opcode::Load:
  case Opcode::Store:
  case Opcode::GEP:
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
  case Opcode::Neg:
  case Opcode::FNeg:
  case Opcode::BitCast:
  case Opcode::Mov:
    return true;
  default:
    return false;
  }
}

bool collectOverwriteEffectsForBlock(BasicBlock* bb,
                                     OverwriteLoopEffects& effects) {
  if (!bb)
    return false;
  for (auto& instPtr : bb->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst)
      return false;
    if (!isPureOverwriteLoopInst(inst)) {
      effects.impure = true;
      return false;
    }
    if (inst->opcode() == Opcode::Load && inst->numOperands() == 1) {
      Value* root = memoryRoot(inst->operand(0));
      if (root)
        effects.reads.insert(root);
      continue;
    }
    if (inst->opcode() == Opcode::Store && inst->numOperands() == 2) {
      Value* root = memoryRoot(inst->operand(1));
      if (root)
        effects.writes.insert(root);
    }
  }
  return true;
}

bool collectOverwriteEffectsForLoop(const analysis::NaturalLoop& loop,
                                    OverwriteLoopEffects& effects) {
  for (auto* bb : loop.blocks) {
    if (!collectOverwriteEffectsForBlock(bb, effects))
      return false;
  }
  return !effects.impure;
}

bool overwriteEffectIsReset(const OverwriteLoopEffects& effects,
                            Value*& rootOut) {
  if (effects.impure || effects.writes.size() != 1)
    return false;
  Value* root = *effects.writes.begin();
  if (!root || effects.reads.count(root))
    return false;
  rootOut = root;
  return true;
}

bool loopContainsLoop(const analysis::NaturalLoop& outer,
                      const analysis::NaturalLoop& inner) {
  if (!inner.header || inner.header == outer.header)
    return false;
  for (auto* bb : inner.blocks) {
    if (!outer.blocks.count(bb))
      return false;
  }
  return true;
}

BasicBlock* singleLoopExitInsideOuter(const analysis::NaturalLoop& inner,
                                      const analysis::NaturalLoop& outer) {
  std::unordered_set<BasicBlock*> exits;
  for (auto* bb : inner.blocks) {
    if (!bb)
      return nullptr;
    for (auto* succ : bb->succs()) {
      if (!succ || inner.blocks.count(succ))
        continue;
      if (!outer.blocks.count(succ))
        return nullptr;
      exits.insert(succ);
    }
  }
  if (exits.size() != 1)
    return nullptr;
  return *exits.begin();
}

bool onlyLoopControlUsesIV(const analysis::NaturalLoop& loop,
                           const hir::CanonicalLoop& cl) {
  if (!cl.ivPhi || !cl.ivNext || !cl.cmp)
    return false;
  for (auto* user : cl.ivPhi->uses()) {
    if (!user || !user->parent())
      return false;
    if (!loop.blocks.count(user->parent()))
      continue;
    if (user != cl.ivNext && user != cl.cmp)
      return false;
  }
  for (auto* user : cl.ivNext->uses()) {
    if (!user || !user->parent())
      return false;
    if (user != cl.ivPhi)
      return false;
  }
  return true;
}

bool hasOnlyCanonicalIVPhi(const hir::CanonicalLoop& cl) {
  auto phis = analysis::collectLeadingPhi(cl.header);
  return phis.size() == 1 && phis.front() == cl.ivPhi;
}

bool constantPositiveRepeatedTrip(const hir::CanonicalLoop& cl) {
  if (!cl.ivIsCmpLhs || hir::canonicalLoopCmpCond(cl.cmp) != "lt")
    return false;
  auto* init = dynamic_cast<ConstInt*>(cl.initVal);
  auto* bound = dynamic_cast<ConstInt*>(cl.boundVal);
  if (!init || !bound)
    return false;
  return bound->value() - init->value() > 1;
}

bool matchOverwriteRepeatLoop(
    const analysis::NaturalLoop& loop,
    const std::vector<analysis::NaturalLoop>& loops,
    hir::CanonicalLoop& cl) {
  auto reject = [&](const char* reason) {
    if (sycEnvironmentValue("OVERWRITE_LOOP_TRACE")) {
      std::cerr << "[overwrite-loop] reject header="
                << (loop.header ? loop.header->name() : std::string("<null>"))
                << " reason=" << (reason ? reason : "-") << "\n";
    }
    return false;
  };

  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader || !hir::recognizeCanonicalLoop(loop, preheader, cl))
    return reject("not-canonical");
  if (!constantPositiveRepeatedTrip(cl))
    return reject("trip");
  if (!loopHasSingleExit(loop, cl.exit) || !analysis::collectLeadingPhi(cl.exit).empty())
    return reject("exit");
  if (!hasOnlyCanonicalIVPhi(cl) || !onlyLoopControlUsesIV(loop, cl))
    return reject("iv-use");

  std::unordered_map<BasicBlock*, const analysis::NaturalLoop*> childByHeader;
  for (const auto& candidate : loops) {
    if (loopContainsLoop(loop, candidate))
      childByHeader[candidate.header] = &candidate;
  }

  std::unordered_set<Value*> resetRoots;
  std::unordered_set<Value*> readsBeforeReset;
  bool sawReset = false;
  std::unordered_set<BasicBlock*> visited;
  BasicBlock* cur = cl.body;
  int steps = 0;

  while (cur && cur != cl.latch) {
    if (++steps > 128)
      return reject("walk-limit");
    if (!loop.blocks.count(cur))
      return reject("walk-left-loop");
    if (!visited.insert(cur).second)
      return reject("top-cycle");

    OverwriteLoopEffects effects;
    auto childIt = childByHeader.find(cur);
    if (childIt != childByHeader.end()) {
      const analysis::NaturalLoop* child = childIt->second;
      if (!collectOverwriteEffectsForLoop(*child, effects))
        return reject("child-effect");
      cur = singleLoopExitInsideOuter(*child, loop);
      if (!cur)
        return reject("child-exit");
    } else {
      if (!collectOverwriteEffectsForBlock(cur, effects))
        return reject("block-effect");
      Instruction* term = cur->terminator();
      if (!term)
        return reject("no-term");
      if (term->opcode() == Opcode::Br && term->numOperands() == 1) {
        cur = dynamic_cast<BasicBlock*>(term->operand(0));
      } else {
        return reject("top-branch");
      }
    }

    if (effects.impure)
      return reject("impure");

    Value* resetRoot = nullptr;
    if (overwriteEffectIsReset(effects, resetRoot)) {
      if (readsBeforeReset.count(resetRoot))
        return reject("late-reset");
      resetRoots.insert(resetRoot);
      sawReset = true;
      continue;
    }

    if (!sawReset) {
      if (!effects.reads.empty() || !effects.writes.empty())
        return reject("memory-before-reset");
      continue;
    }

    for (Value* root : effects.reads) {
      if (!resetRoots.count(root))
        readsBeforeReset.insert(root);
    }
    for (Value* root : effects.writes) {
      if (!resetRoots.count(root))
        return reject("write-unreset-root");
    }
  }

  if (cur != cl.latch || !sawReset)
    return reject("no-reset");
  return true;
}

bool collapseOverwriteRepeatLoops(Function* func,
                                  analysis::AnalysisManager& analysisManager) {
  if (!func)
    return false;
  func->recomputeCFG();
  auto& cache = analysisManager.function(func);
  auto loops = cache.loops();
  std::sort(loops.begin(), loops.end(),
            [](const analysis::NaturalLoop& a, const analysis::NaturalLoop& b) {
              return a.blocks.size() > b.blocks.size();
            });

  for (const auto& loop : loops) {
    hir::CanonicalLoop cl;
    if (!matchOverwriteRepeatLoop(loop, loops, cl))
      continue;
    if (sycEnvironmentValue("OVERWRITE_LOOP_TRACE")) {
      std::cerr << "[overwrite-loop] apply func=" << func->name()
                << " header=" << loop.header->name() << "\n";
    }
    Instruction* latchTerm = cl.latch ? cl.latch->terminator() : nullptr;
    if (!latchTerm || latchTerm->opcode() != Opcode::Br ||
        latchTerm->numOperands() != 1 || latchTerm->operand(0) != cl.header) {
      continue;
    }
    latchTerm->setOperand(0, cl.exit);
    func->recomputeCFG();
    repairPhiIncoming(func);
    analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
    return true;
  }
  return false;
}

bool applyOverwriteRepeatLoopCollapse(Module& mod,
                                      analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  bool localChanged = true;
  for (int iter = 0; iter < 4 && localChanged; ++iter) {
    localChanged = false;
    for (auto& funcPtr : mod.functions()) {
      if (collapseOverwriteRepeatLoops(funcPtr.get(), analysisManager)) {
        changed = true;
        localChanged = true;
      }
    }
  }
  return changed;
}


} // namespace core_ir
