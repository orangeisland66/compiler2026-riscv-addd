// 文件职责：分析循环的边界和步进关系，为循环变换提供终止性信息。
#include "termination.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "analysis_manager.h"
#include "function_analysis.h"

namespace analysis {
namespace {

using namespace core_ir;

bool isInLoop(const NaturalLoop& loop, const Value* value) {
  const auto* inst = dynamic_cast<const Instruction*>(value);
  return inst && inst->parent() && loop.blocks.count(inst->parent());
}

Value* phiIncomingValue(Instruction* phi, BasicBlock* pred) {
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    if (phi->operand(i + 1) == pred)
      return phi->operand(i);
  }
  return nullptr;
}

int matchUnitStep(Value* next, Instruction* ivPhi) {
  auto* inst = dynamic_cast<Instruction*>(next);
  if (!inst || inst->numOperands() != 2)
    return 0;
  Value* lhs = inst->operand(0);
  Value* rhs = inst->operand(1);
  if (inst->opcode() == Opcode::Add) {
    const ConstInt* constant = nullptr;
    if (lhs == ivPhi)
      constant = dynamic_cast<const ConstInt*>(rhs);
    else if (rhs == ivPhi)
      constant = dynamic_cast<const ConstInt*>(lhs);
    if (constant && (constant->value() == 1 || constant->value() == -1))
      return constant->value();
  }
  if (inst->opcode() == Opcode::Sub && lhs == ivPhi) {
    const auto* constant = dynamic_cast<const ConstInt*>(rhs);
    if (constant && (constant->value() == 1 || constant->value() == -1))
      return -constant->value();
  }
  return 0;
}

std::string compareCondition(const Instruction* compare) {
  if (!compare)
    return "";
  const std::string& name = compare->name();
  const auto separator = name.find('_');
  return separator == std::string::npos ? name : name.substr(0, separator);
}

std::string normalizeContinueCondition(std::string condition, bool ivIsLhs, bool continueOnTrue) {
  if (!ivIsLhs) {
    if (condition == "lt")
      condition = "gt";
    else if (condition == "gt")
      condition = "lt";
    else if (condition == "le")
      condition = "ge";
    else if (condition == "ge")
      condition = "le";
  }
  if (!continueOnTrue) {
    if (condition == "ne")
      condition = "eq";
    else if (condition == "eq")
      condition = "ne";
    else if (condition == "lt")
      condition = "ge";
    else if (condition == "ge")
      condition = "lt";
    else if (condition == "le")
      condition = "gt";
    else if (condition == "gt")
      condition = "le";
  }
  return condition;
}

bool hasSafeDivisor(const Instruction* inst) {
  if (!inst || inst->numOperands() != 2)
    return false;
  const auto* divisor = dynamic_cast<const ConstInt*>(inst->operand(1));
  return divisor && divisor->value() != 0 && divisor->value() != -1;
}

bool hasSafeShiftAmount(const Instruction* inst) {
  if (!inst || inst->numOperands() != 2)
    return false;
  const auto* amount = dynamic_cast<const ConstInt*>(inst->operand(1));
  return amount && amount->value() >= 0 && amount->value() < 32;
}

// 判断指令是否必然终止（不陷入、不发散）：除法/取余/移位可能陷入，
// 需可证明操作数安全；Call 需可证明被调函数必然终止；其余列出的
// 操作码视为天然必然终止。
bool instructionIsTotal(const Instruction* inst,
                        const Module& mod,
                        const FunctionTerminationMap& termination) {
  switch (inst->opcode()) {
    case Opcode::SDiv:
    case Opcode::SRem:
    case Opcode::Shl:
    case Opcode::Shr:
      return isProvablyNonTrappingScalarInstruction(inst);
    case Opcode::Call:
      return isProvablyTotalCall(inst, mod, termination);
    case Opcode::Br:
    case Opcode::CondBr:
    case Opcode::Ret:
    case Opcode::Alloca:
    case Opcode::Load:
    case Opcode::Store:
    case Opcode::Add:
    case Opcode::Sub:
    case Opcode::Mul:
    case Opcode::Mulh:
    case Opcode::And:
    case Opcode::Or:
    case Opcode::Xor:
    case Opcode::FAdd:
    case Opcode::FSub:
    case Opcode::FMul:
    case Opcode::FDiv:
    case Opcode::ICmp:
    case Opcode::FCmp:
    case Opcode::SIToFP:
    case Opcode::FPToSI:
    case Opcode::GEP:
    case Opcode::Phi:
    case Opcode::Neg:
    case Opcode::FNeg:
    case Opcode::BitCast:
    case Opcode::Select:
    case Opcode::Mov:
      return true;
    default:
      return false;
  }
}

bool cfgIsAcyclicWithoutBackedges(
    FunctionAnalysisCache& cache,
    const std::unordered_map<BasicBlock*, std::unordered_set<BasicBlock*>>& backedges) {
  enum class VisitState { Unvisited, Visiting, Visited };
  std::unordered_map<BasicBlock*, VisitState> states;
  for (BasicBlock* block : cache.blocks())
    states[block] = VisitState::Unvisited;

  auto visit = [&](auto&& self, BasicBlock* block) -> bool {
    VisitState& state = states[block];
    if (state == VisitState::Visiting)
      return false;
    if (state == VisitState::Visited)
      return true;
    state = VisitState::Visiting;
    auto ignored = backedges.find(block);
    for (BasicBlock* successor : block->succs()) {
      if (!successor || !cache.reachable().count(successor))
        continue;
      if (ignored != backedges.end() && ignored->second.count(successor))
        continue;
      if (!self(self, successor))
        return false;
    }
    state = VisitState::Visited;
    return true;
  };

  Function* function = cache.function();
  return function && function->entryBlock() && visit(visit, function->entryBlock());
}

bool functionIsTotal(Function* function,
                     Module& mod,
                     AnalysisManager& analysisManager,
                     const FunctionTerminationMap& termination) {
  if (!function || !function->entryBlock())
    return false;
  function->recomputeCFG();
  FunctionAnalysisCache& cache = analysisManager.function(function);
  if (cache.blocks().empty())
    return false;

  std::unordered_map<BasicBlock*, std::unordered_set<BasicBlock*>> backedges;
  for (const NaturalLoop& loop : cache.loops()) {
    if (!isProvablyFiniteUnitCounterLoop(loop))
      return false;
    for (BasicBlock* latch : loop.latches)
      backedges[latch].insert(loop.header);
  }
  if (!cfgIsAcyclicWithoutBackedges(cache, backedges))
    return false;

  for (BasicBlock* block : cache.blocks()) {
    if (!cache.reachable().count(block))
      continue;
    if (!block->terminator())
      return false;
    for (const auto& instPtr : block->instructions()) {
      if (!instructionIsTotal(instPtr.get(), mod, termination))
        return false;
    }
  }
  return true;
}

}  // namespace

bool isProvablyNonTrappingScalarInstruction(const Instruction* inst) {
  if (!inst)
    return false;
  if (inst->opcode() == Opcode::SDiv || inst->opcode() == Opcode::SRem)
    return hasSafeDivisor(inst);
  if (inst->opcode() == Opcode::Shl || inst->opcode() == Opcode::Shr)
    return hasSafeShiftAmount(inst);
  return true;
}

bool isProvablyFiniteUnitCounterLoop(const NaturalLoop& loop) {
  if (!loop.header)
    return false;
  std::vector<BasicBlock*> latches;
  for (BasicBlock* latch : loop.latches) {
    if (latch && std::find(latches.begin(), latches.end(), latch) == latches.end())
      latches.push_back(latch);
  }
  if (latches.size() != 1)
    return false;
  BasicBlock* latch = latches.front();

  BasicBlock* outsidePred = nullptr;
  for (BasicBlock* pred : loop.header->preds()) {
    if (loop.blocks.count(pred))
      continue;
    if (outsidePred)
      return false;
    outsidePred = pred;
  }
  if (!outsidePred || loop.header->preds().size() != 2)
    return false;

  Instruction* branch = loop.header->terminator();
  if (!branch || branch->opcode() != Opcode::CondBr || branch->numOperands() != 3)
    return false;
  auto* compare = dynamic_cast<Instruction*>(branch->operand(0));
  if (!compare || compare->opcode() != Opcode::ICmp || compare->numOperands() != 2)
    return false;
  auto* trueBlock = dynamic_cast<BasicBlock*>(branch->operand(1));
  auto* falseBlock = dynamic_cast<BasicBlock*>(branch->operand(2));
  if (!trueBlock || !falseBlock)
    return false;
  bool continueOnTrue = loop.blocks.count(trueBlock);
  if (continueOnTrue == static_cast<bool>(loop.blocks.count(falseBlock)))
    return false;

  Instruction* ivPhi = nullptr;
  Value* bound = nullptr;
  bool ivIsLhs = false;
  Value* lhs = compare->operand(0);
  Value* rhs = compare->operand(1);
  auto* lhsInst = dynamic_cast<Instruction*>(lhs);
  auto* rhsInst = dynamic_cast<Instruction*>(rhs);
  if (lhsInst && lhsInst->opcode() == Opcode::Phi && lhsInst->parent() == loop.header) {
    ivPhi = lhsInst;
    bound = rhs;
    ivIsLhs = true;
  } else if (rhsInst && rhsInst->opcode() == Opcode::Phi && rhsInst->parent() == loop.header) {
    ivPhi = rhsInst;
    bound = lhs;
  } else {
    return false;
  }
  if (!ivPhi->type().isI32() || isInLoop(loop, bound))
    return false;

  Value* initial = phiIncomingValue(ivPhi, outsidePred);
  Value* next = phiIncomingValue(ivPhi, latch);
  if (!initial || !next)
    return false;
  int step = matchUnitStep(next, ivPhi);
  std::string condition =
      normalizeContinueCondition(compareCondition(compare), ivIsLhs, continueOnTrue);
  if ((condition == "lt" && step == 1) ||
      (condition == "gt" && step == -1))
    return true;

  if (condition != "ne")
    return false;
  const auto* initialConstant = dynamic_cast<const ConstInt*>(initial);
  const auto* boundConstant = dynamic_cast<const ConstInt*>(bound);
  if (!initialConstant || !boundConstant)
    return false;
  if (step == 1)
    return initialConstant->value() <= boundConstant->value();
  if (step == -1)
    return initialConstant->value() >= boundConstant->value();
  return false;
}

FunctionTerminationMap proveModuleFunctionTermination(Module& mod,
                                                      AnalysisManager& analysisManager) {
  FunctionTerminationMap termination;
  for (const auto& functionPtr : mod.functions())
    termination[functionPtr.get()] = false;

  bool changed = true;
  while (changed) {
    changed = false;
    for (const auto& functionPtr : mod.functions()) {
      Function* function = functionPtr.get();
      if (termination[function])
        continue;
      if (!functionIsTotal(function, mod, analysisManager, termination))
        continue;
      termination[function] = true;
      changed = true;
    }
  }
  return termination;
}

bool isProvablyTotalCall(const Instruction* call,
                         const Module& mod,
                         const FunctionTerminationMap& termination) {
  if (!call || call->opcode() != Opcode::Call)
    return false;
  Function* callee = mod.getFunction(call->name());
  if (!callee)
    return false;
  auto found = termination.find(callee);
  return found != termination.end() && found->second;
}

}  // namespace analysis
