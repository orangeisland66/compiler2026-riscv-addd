#include "research_switch.h"
// 文件职责：实现 LLVM InstCombine 风格的局部结构折叠，压缩小常量分派 helper。
#include "pass_registry.h"
#include "inst_combine.h"

#include "../../analysis/cfg_utils.h"

#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace core_ir {

namespace {

std::string cmpCond(Instruction* inst) {
  if (!inst)
    return "";
  auto name = inst->name();
  auto pos = name.find('_');
  return pos == std::string::npos ? name : name.substr(0, pos);
}

std::vector<Instruction*> nonPhiBodyInstructions(BasicBlock* bb) {
  std::vector<Instruction*> result;
  if (!bb)
    return result;
  for (auto& instPtr : bb->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst || inst == bb->terminator() || inst->opcode() == Opcode::Phi)
      continue;
    result.push_back(inst);
  }
  return result;
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

void clearBlock(BasicBlock* bb) {
  if (!bb)
    return;
  std::vector<Instruction*> insts;
  insts.reserve(bb->instructions().size());
  for (auto& instPtr : bb->instructions())
    insts.push_back(instPtr.get());
  for (Instruction* inst : insts)
    bb->remove(inst);
}

bool isSimpleValue(Value* value) {
  return value && (value->isConstant() || dynamic_cast<Argument*>(value));
}

bool matchEqCompareAgainstConstant(Instruction* inst, Value*& valueOut,
                                   int32_t& constantOut) {
  if (!inst || inst->opcode() != Opcode::ICmp || inst->numOperands() != 2)
    return false;
  if (cmpCond(inst) != "eq")
    return false;
  if (auto* rhs = dynamic_cast<ConstInt*>(inst->operand(1))) {
    valueOut = inst->operand(0);
    constantOut = rhs->value();
    return true;
  }
  if (auto* lhs = dynamic_cast<ConstInt*>(inst->operand(0))) {
    valueOut = inst->operand(1);
    constantOut = lhs->value();
    return true;
  }
  return false;
}

struct ShlCaseInfo {
  BasicBlock* pred = nullptr;
  int32_t amount = 0;
};

struct DivPow2CaseInfo {
  BasicBlock* pred = nullptr;
  int32_t amount = 0;
};

struct ShiftHelperMatch {
  Function* func = nullptr;
  BasicBlock* entry = nullptr;
  BasicBlock* merge = nullptr;
  BasicBlock* defaultPred = nullptr;
  Value* base = nullptr;
  Value* shift = nullptr;
  int32_t maxAmount = 0;
  std::vector<BasicBlock*> compareBlocks;
  std::vector<BasicBlock*> blocksToRemove;
  std::vector<ShlCaseInfo> shlCases;
  std::vector<DivPow2CaseInfo> divCases;
  bool isDivPow2 = false;
};

struct LocalShlLadderMatch {
  Function* func = nullptr;
  BasicBlock* head = nullptr;
  BasicBlock* merge = nullptr;
  Instruction* oldPhi = nullptr;
  Instruction* headCmp = nullptr;
  Value* base = nullptr;
  Value* shift = nullptr;
  int32_t maxAmount = 0;
  std::vector<BasicBlock*> compareBlocks;
  std::vector<BasicBlock*> blocksToRemove;
};

struct MaxDiamondMatch {
  Function* func = nullptr;
  BasicBlock* head = nullptr;
  BasicBlock* trueBB = nullptr;
  BasicBlock* falseBB = nullptr;
  BasicBlock* merge = nullptr;
  Instruction* cmp = nullptr;
  Instruction* phi = nullptr;
  Value* lhs = nullptr;
  Value* rhs = nullptr;
};

bool isEmptyBrBlock(BasicBlock* bb, BasicBlock* target) {
  if (!bb || !target || !nonPhiBodyInstructions(bb).empty())
    return false;
  Instruction* term = bb->terminator();
  return term && term->opcode() == Opcode::Br && term->numOperands() == 1 &&
         term->operand(0) == target;
}

bool blockUsesAreLimitedTo(
    BasicBlock* bb, const std::unordered_set<Instruction*>& allowedUsers) {
  if (!bb)
    return false;
  for (Instruction* user : bb->uses()) {
    if (!user || !allowedUsers.count(user))
      return false;
  }
  return true;
}

bool matchMaxDiamondAtHead(BasicBlock* head, MaxDiamondMatch& match) {
  if (!head || !head->parent())
    return false;
  Instruction* term = head->terminator();
  if (!term || term->opcode() != Opcode::CondBr || term->numOperands() != 3)
    return false;
  auto* cmp = dynamic_cast<Instruction*>(term->operand(0));
  auto* trueBB = dynamic_cast<BasicBlock*>(term->operand(1));
  auto* falseBB = dynamic_cast<BasicBlock*>(term->operand(2));
  if (!cmp || cmp->opcode() != Opcode::ICmp || cmp->numOperands() != 2 ||
      !trueBB || !falseBB || trueBB == falseBB)
    return false;
  const std::string cond = cmpCond(cmp);
  if (cond != "lt" && cond != "gt")
    return false;
  if (trueBB->preds().size() != 1 || trueBB->preds().front() != head ||
      falseBB->preds().size() != 1 || falseBB->preds().front() != head)
    return false;

  Instruction* trueTerm = trueBB->terminator();
  Instruction* falseTerm = falseBB->terminator();
  if (!trueTerm || !falseTerm || trueTerm->opcode() != Opcode::Br ||
      falseTerm->opcode() != Opcode::Br || trueTerm->numOperands() != 1 ||
      falseTerm->numOperands() != 1 ||
      trueTerm->operand(0) != falseTerm->operand(0))
    return false;
  auto* merge = dynamic_cast<BasicBlock*>(trueTerm->operand(0));
  if (!merge || !isEmptyBrBlock(trueBB, merge) ||
      !isEmptyBrBlock(falseBB, merge))
    return false;

  auto phis = analysis::collectLeadingPhi(merge);
  if (phis.size() != 1)
    return false;
  Instruction* phi = phis.front();
  Value* trueValue = incomingFromPred(phi, trueBB);
  Value* falseValue = incomingFromPred(phi, falseBB);
  Value* lhs = cmp->operand(0);
  Value* rhs = cmp->operand(1);
  const bool ltMax = cond == "lt" && trueValue == rhs && falseValue == lhs;
  const bool gtMax = cond == "gt" && trueValue == lhs && falseValue == rhs;
  if (!ltMax && !gtMax)
    return false;
  const std::unordered_set<Instruction*> allowedLabelUsers{term, phi};
  if (!blockUsesAreLimitedTo(trueBB, allowedLabelUsers) ||
      !blockUsesAreLimitedTo(falseBB, allowedLabelUsers)) {
    return false;
  }

  match.func = head->parent();
  match.head = head;
  match.trueBB = trueBB;
  match.falseBB = falseBB;
  match.merge = merge;
  match.cmp = cmp;
  match.phi = phi;
  match.lhs = lhs;
  match.rhs = rhs;
  return true;
}

bool matchConstMinusValue(Value* value, Value*& variable, int64_t& constant) {
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->opcode() != Opcode::Sub || inst->numOperands() != 2)
    return false;
  auto* c = dynamic_cast<ConstInt*>(inst->operand(0));
  if (!c || c->value() < 0)
    return false;
  variable = inst->operand(1);
  constant = c->value();
  return variable != nullptr;
}

bool matchReflectiveMax(Value* lhs, Value* rhs, Value*& variable,
                        int64_t& constant, Instruction*& reflected) {
  if (matchConstMinusValue(lhs, variable, constant)) {
    reflected = dynamic_cast<Instruction*>(lhs);
    return rhs == variable;
  }
  if (matchConstMinusValue(rhs, variable, constant)) {
    reflected = dynamic_cast<Instruction*>(rhs);
    return lhs == variable;
  }
  return false;
}

int64_t reflectiveLowerBound(int64_t constant) {
  return (constant + 1) / 2;
}

void eraseIfDead(Instruction* inst) {
  if (inst && inst->parent() && inst->uses().empty())
    inst->parent()->remove(inst);
}

bool rewriteDominatedMaxDiamond(MaxDiamondMatch& match, Value* replacement,
                                Instruction* reflected) {
  if (!match.func || !match.head || !match.merge || !match.phi || !replacement)
    return false;

  Instruction* oldTerm = match.head->terminator();
  if (!oldTerm)
    return false;
  match.head->remove(oldTerm);
  auto* br = new Instruction(Opcode::Br, Type::voidTy());
  br->addOperand(match.merge);
  match.head->pushBack(br);

  match.phi->replaceAllUsesWith(replacement);
  eraseIfDead(match.phi);
  eraseIfDead(match.cmp);
  eraseIfDead(reflected);
  match.func->removeBlock(match.trueBB);
  match.func->removeBlock(match.falseBB);
  match.func->recomputeCFG();
  return true;
}

bool simplifyDominatedReflectiveMax(Function* func) {
  if (sycEnvironmentValue("SYC_SKIP_COREIR_REFLECTIVE_MAX"))
    return false;
  if (!func || !func->entryBlock())
    return false;

  bool changed = false;
  bool iterChanged = true;
  while (iterChanged) {
    iterChanged = false;
    std::unordered_map<Value*, int64_t> lowerBounds;
    std::vector<BasicBlock*> blocks;
    blocks.reserve(func->blocks().size());
    for (auto& bbPtr : func->blocks())
      blocks.push_back(bbPtr.get());

    for (BasicBlock* bb : blocks) {
      MaxDiamondMatch match;
      if (!matchMaxDiamondAtHead(bb, match))
        continue;

      Value* variable = nullptr;
      int64_t constant = 0;
      Instruction* reflected = nullptr;
      if (!matchReflectiveMax(match.lhs, match.rhs, variable, constant,
                              reflected))
        continue;

      const int64_t threshold = reflectiveLowerBound(constant);
      auto lbIt = lowerBounds.find(variable);
      if (lbIt != lowerBounds.end() && lbIt->second >= threshold) {
        // 后续 max(y, C-y) 只有在已知 y >= ceil(C/2) 时才折叠；
        // 下界来自前面的 CoreIR diamond/phi，而不是源码名称或测试名称。
        if (rewriteDominatedMaxDiamond(match, variable, reflected)) {
          iterChanged = true;
          changed = true;
          break;
        }
      }

      const int64_t maxLower =
          lbIt == lowerBounds.end() ? threshold
                                    : std::max(threshold, lbIt->second);
      auto phiIt = lowerBounds.find(match.phi);
      if (phiIt == lowerBounds.end() || maxLower > phiIt->second)
        lowerBounds[match.phi] = maxLower;
    }
  }
  return changed;
}

bool matchShlCaseBlock(BasicBlock* pred, BasicBlock* merge, Value* phiValue,
                       Value*& baseOut, int32_t& amountOut) {
  if (!pred || !merge || !phiValue)
    return false;
  Instruction* term = pred->terminator();
  if (!term || term->opcode() != Opcode::Br || term->numOperands() != 1 ||
      term->operand(0) != merge) {
    return false;
  }
  auto body = nonPhiBodyInstructions(pred);
  if (body.size() != 1 || body.front() != phiValue)
    return false;
  Instruction* shl = body.front();
  if (shl->opcode() != Opcode::Shl || shl->numOperands() != 2)
    return false;
  auto* amount = dynamic_cast<ConstInt*>(shl->operand(1));
  if (!amount || amount->value() < 1 || amount->value() > 30)
    return false;
  baseOut = shl->operand(0);
  amountOut = amount->value();
  return true;
}

bool matchDivPow2CaseBlock(BasicBlock* pred, BasicBlock* merge, Value* phiValue,
                           Value*& baseOut, int32_t& amountOut) {
  if (!pred || !merge || !phiValue)
    return false;
  Instruction* term = pred->terminator();
  if (!term || term->opcode() != Opcode::Br || term->numOperands() != 1 ||
      term->operand(0) != merge) {
    return false;
  }
  auto body = nonPhiBodyInstructions(pred);
  if (body.size() != 4 || body.back() != phiValue)
    return false;

  Instruction* shrFinal = body.back();
  if (shrFinal->opcode() != Opcode::Shr || shrFinal->numOperands() != 2)
    return false;
  auto* amount = dynamic_cast<ConstInt*>(shrFinal->operand(1));
  auto* add = dynamic_cast<Instruction*>(shrFinal->operand(0));
  if (!amount || amount->value() < 1 || amount->value() > 30 || !add ||
      add->opcode() != Opcode::Add || add->numOperands() != 2) {
    return false;
  }

  Instruction* andInst = nullptr;
  Value* baseCandidate = nullptr;
  if (auto* lhs = dynamic_cast<Instruction*>(add->operand(0));
      lhs && lhs->opcode() == Opcode::And) {
    andInst = lhs;
    baseCandidate = add->operand(1);
  } else if (auto* rhs = dynamic_cast<Instruction*>(add->operand(1));
             rhs && rhs->opcode() == Opcode::And) {
    andInst = rhs;
    baseCandidate = add->operand(0);
  } else {
    return false;
  }

  if (!andInst || andInst->numOperands() != 2)
    return false;
  auto* mask = dynamic_cast<ConstInt*>(andInst->operand(1));
  auto* signShr = dynamic_cast<Instruction*>(andInst->operand(0));
  if (!mask || !signShr || signShr->opcode() != Opcode::Shr ||
      signShr->numOperands() != 2 || signShr->operand(0) != baseCandidate) {
    return false;
  }
  if (!dynamic_cast<ConstInt*>(signShr->operand(1)) ||
      dynamic_cast<ConstInt*>(signShr->operand(1))->value() != 31) {
    return false;
  }

  const int32_t expectedMask =
      static_cast<int32_t>((static_cast<int64_t>(1) << amount->value()) - 1);
  if (mask->value() != expectedMask)
    return false;

  baseOut = baseCandidate;
  amountOut = amount->value();
  return true;
}

template <typename CaseInfo>
bool caseAmountsAreContiguous(const std::vector<CaseInfo>& cases,
                              int32_t& maxAmountOut) {
  if (cases.size() < 2)
    return false;
  std::vector<int32_t> amounts;
  amounts.reserve(cases.size());
  for (const auto& info : cases)
    amounts.push_back(info.amount);
  std::sort(amounts.begin(), amounts.end());
  amounts.erase(std::unique(amounts.begin(), amounts.end()), amounts.end());
  if (amounts.size() < 2 || amounts.front() != 1)
    return false;
  for (size_t i = 1; i < amounts.size(); ++i) {
    if (amounts[i] != amounts[i - 1] + 1)
      return false;
  }
  maxAmountOut = amounts.back();
  return true;
}

bool collectCompareChain(Function* func, BasicBlock* defaultPred,
                         const std::unordered_map<int32_t, BasicBlock*>& caseMap,
                         Value*& shiftOut,
                         std::vector<BasicBlock*>& compareBlocksOut) {
  if (!func || !func->entryBlock() || !defaultPred || caseMap.empty())
    return false;

  BasicBlock* current = func->entryBlock();
  std::unordered_set<BasicBlock*> visited;
  std::unordered_set<int32_t> seenCases;
  Value* shift = nullptr;

  while (current && current != defaultPred) {
    if (!visited.insert(current).second)
      return false;
    auto body = nonPhiBodyInstructions(current);
    if (body.size() != 1)
      return false;
    Instruction* cmp = body.front();
    Value* compared = nullptr;
    int32_t amount = 0;
    if (!matchEqCompareAgainstConstant(cmp, compared, amount))
      return false;
    auto caseIt = caseMap.find(amount);
    if (caseIt == caseMap.end())
      return false;

    Instruction* term = current->terminator();
    if (!term || term->opcode() != Opcode::CondBr || term->numOperands() != 3)
      return false;

    auto* trueSucc = dynamic_cast<BasicBlock*>(term->operand(1));
    auto* falseSucc = dynamic_cast<BasicBlock*>(term->operand(2));
    if (!trueSucc || !falseSucc || trueSucc == falseSucc)
      return false;

    BasicBlock* next = nullptr;
    if (trueSucc == caseIt->second) {
      next = falseSucc;
    } else if (falseSucc == caseIt->second) {
      next = trueSucc;
    } else {
      return false;
    }

    if (shift && shift != compared)
      return false;
    shift = compared;
    seenCases.insert(amount);
    compareBlocksOut.push_back(current);
    current = next;
  }

  if (!current || current != defaultPred || !shift || !isSimpleValue(shift))
    return false;
  if (seenCases.size() != caseMap.size())
    return false;
  shiftOut = shift;
  return true;
}

bool matchMergeReturnShape(Function* func, BasicBlock*& mergeOut,
                           Instruction*& phiOut) {
  if (!func)
    return false;
  BasicBlock* merge = nullptr;
  Instruction* phi = nullptr;
  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!bb)
      continue;
    Instruction* term = bb->terminator();
    if (!term || term->opcode() != Opcode::Ret || term->numOperands() != 1)
      continue;
    auto phis = analysis::collectLeadingPhi(bb);
    auto body = nonPhiBodyInstructions(bb);
    if (phis.size() != 1 || !body.empty() ||
        term->operand(0) != phis.front() || phis.front()->uses().size() != 1) {
      return false;
    }
    if (merge)
      return false;
    merge = bb;
    phi = phis.front();
  }
  if (!merge || !phi)
    return false;
  mergeOut = merge;
  phiOut = phi;
  return true;
}

bool collectCoveredBlocks(
    Function* func, BasicBlock* merge, BasicBlock* defaultPred,
    const std::vector<BasicBlock*>& compareBlocks,
    const std::unordered_map<int32_t, BasicBlock*>& caseMap,
    std::vector<BasicBlock*>& coveredOut) {
  if (!func || !merge || !defaultPred)
    return false;
  std::unordered_set<BasicBlock*> covered;
  covered.insert(merge);
  covered.insert(defaultPred);
  covered.insert(func->entryBlock());
  for (BasicBlock* bb : compareBlocks)
    covered.insert(bb);
  for (const auto& [_, bb] : caseMap)
    covered.insert(bb);
  const auto reachable = analysis::collectReachableBlocks(func);
  if (covered.size() != reachable.size())
    return false;
  for (BasicBlock* bb : reachable) {
    if (!covered.count(bb))
      return false;
  }
  coveredOut.assign(covered.begin(), covered.end());
  return true;
}

bool blocksHaveNoExternalLabelUsers(
    const std::vector<BasicBlock*>& covered,
    const std::unordered_set<BasicBlock*>& keepBlocks) {
  std::unordered_set<BasicBlock*> coveredSet(covered.begin(), covered.end());
  for (BasicBlock* bb : covered) {
    if (!bb || keepBlocks.count(bb))
      continue;
    for (Instruction* user : bb->uses()) {
      if (!user || !user->parent())
        return false;
      if (!coveredSet.count(user->parent()))
        return false;
    }
  }
  return true;
}

bool matchShlHelper(Function* func, ShiftHelperMatch& match) {
  BasicBlock* merge = nullptr;
  Instruction* phi = nullptr;
  if (!matchMergeReturnShape(func, merge, phi))
    return false;

  Value* base = nullptr;
  BasicBlock* defaultPred = nullptr;
  std::unordered_map<int32_t, BasicBlock*> caseMap;
  std::vector<ShlCaseInfo> cases;

  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    Value* incoming = phi->operand(i);
    auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
    if (!pred)
      return false;

    Value* incomingBase = nullptr;
    int32_t amount = 0;
    if (matchShlCaseBlock(pred, merge, incoming, incomingBase, amount)) {
      if (!isSimpleValue(incomingBase))
        return false;
      if (!base)
        base = incomingBase;
      if (base != incomingBase || caseMap.count(amount))
        return false;
      caseMap[amount] = pred;
      cases.push_back({pred, amount});
      continue;
    }

    if (!pred->terminator() || pred->terminator()->opcode() != Opcode::Br ||
        pred->terminator()->numOperands() != 1 ||
        pred->terminator()->operand(0) != merge ||
        !nonPhiBodyInstructions(pred).empty()) {
      return false;
    }
    if (defaultPred || !isSimpleValue(incoming))
      return false;
    defaultPred = pred;
    if (!base)
      base = incoming;
    else if (base != incoming)
      return false;
  }

  int32_t maxAmount = 0;
  Value* shift = nullptr;
  std::vector<BasicBlock*> compareBlocks;
  std::vector<BasicBlock*> covered;
  if (!base || !defaultPred || !caseAmountsAreContiguous(cases, maxAmount) ||
      !collectCompareChain(func, defaultPred, caseMap, shift, compareBlocks) ||
      !collectCoveredBlocks(func, merge, defaultPred, compareBlocks, caseMap,
                            covered)) {
    return false;
  }
  if (!blocksHaveNoExternalLabelUsers(
          covered, {func->entryBlock(), merge})) {
    return false;
  }

  match.func = func;
  match.entry = func->entryBlock();
  match.merge = merge;
  match.defaultPred = defaultPred;
  match.base = base;
  match.shift = shift;
  match.maxAmount = maxAmount;
  match.compareBlocks = std::move(compareBlocks);
  match.blocksToRemove = std::move(covered);
  match.shlCases = std::move(cases);
  match.divCases.clear();
  match.isDivPow2 = false;
  return true;
}

bool matchDivPow2Helper(Function* func, ShiftHelperMatch& match) {
  BasicBlock* merge = nullptr;
  Instruction* phi = nullptr;
  if (!matchMergeReturnShape(func, merge, phi))
    return false;

  Value* base = nullptr;
  BasicBlock* defaultPred = nullptr;
  std::unordered_map<int32_t, BasicBlock*> caseMap;
  std::vector<DivPow2CaseInfo> cases;

  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    Value* incoming = phi->operand(i);
    auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
    if (!pred)
      return false;

    Value* incomingBase = nullptr;
    int32_t amount = 0;
    if (matchDivPow2CaseBlock(pred, merge, incoming, incomingBase, amount)) {
      if (!isSimpleValue(incomingBase))
        return false;
      if (!base)
        base = incomingBase;
      if (base != incomingBase || caseMap.count(amount))
        return false;
      caseMap[amount] = pred;
      cases.push_back({pred, amount});
      continue;
    }

    if (!pred->terminator() || pred->terminator()->opcode() != Opcode::Br ||
        pred->terminator()->numOperands() != 1 ||
        pred->terminator()->operand(0) != merge ||
        !nonPhiBodyInstructions(pred).empty()) {
      return false;
    }
    if (defaultPred || !isSimpleValue(incoming))
      return false;
    defaultPred = pred;
    if (!base)
      base = incoming;
    else if (base != incoming)
      return false;
  }

  int32_t maxAmount = 0;
  Value* shift = nullptr;
  std::vector<BasicBlock*> compareBlocks;
  std::vector<BasicBlock*> covered;
  if (!base || !defaultPred || !caseAmountsAreContiguous(cases, maxAmount) ||
      !collectCompareChain(func, defaultPred, caseMap, shift, compareBlocks) ||
      !collectCoveredBlocks(func, merge, defaultPred, compareBlocks, caseMap,
                            covered)) {
    return false;
  }

  match.func = func;
  match.entry = func->entryBlock();
  match.merge = merge;
  match.defaultPred = defaultPred;
  match.base = base;
  match.shift = shift;
  match.maxAmount = maxAmount;
  match.compareBlocks = std::move(compareBlocks);
  match.blocksToRemove = std::move(covered);
  match.shlCases.clear();
  match.divCases = std::move(cases);
  match.isDivPow2 = true;
  return true;
}

Instruction* makeICmp(const std::string& cond, Value* lhs, Value* rhs,
                      const std::string& name) {
  auto* cmp = new Instruction(Opcode::ICmp, Type::i32(), name);
  cmp->addOperand(lhs);
  cmp->addOperand(rhs);
  cmp->setName(cond + "_" + name);
  return cmp;
}

bool rewriteShlHelper(ShiftHelperMatch& match) {
  if (!match.func || !match.entry || !match.merge || !match.base ||
      !match.shift || match.maxAmount < 1) {
    return false;
  }

  Module* mod = match.func->parent();
  if (!mod)
    return false;

  BasicBlock* shiftBB = match.func->newBlock("instcombine.shift");
  clearBlock(match.entry);
  clearBlock(match.merge);

  auto* geLower = makeICmp("ge", match.shift, mod->getConstInt(1),
                           "instcombine.lower");
  match.entry->pushBack(geLower);
  auto* leUpper = makeICmp("le", match.shift, mod->getConstInt(match.maxAmount),
                           "instcombine.upper");
  match.entry->pushBack(leUpper);
  auto* inRange = new Instruction(Opcode::And, Type::i32(), "instcombine.inrange");
  inRange->addOperand(geLower);
  inRange->addOperand(leUpper);
  match.entry->pushBack(inRange);
  auto* entryBr = new Instruction(Opcode::CondBr, Type::voidTy());
  entryBr->addOperand(inRange);
  entryBr->addOperand(shiftBB);
  entryBr->addOperand(match.merge);
  match.entry->pushBack(entryBr);

  auto* shifted = new Instruction(Opcode::Shl, Type::i32(), "instcombine.shl");
  shifted->addOperand(match.base);
  shifted->addOperand(match.shift);
  shiftBB->pushBack(shifted);
  auto* shiftBr = new Instruction(Opcode::Br, Type::voidTy());
  shiftBr->addOperand(match.merge);
  shiftBB->pushBack(shiftBr);

  auto* resultPhi = new Instruction(Opcode::Phi, Type::i32(), "instcombine.result");
  resultPhi->addOperand(match.base);
  resultPhi->addOperand(match.entry);
  resultPhi->addOperand(shifted);
  resultPhi->addOperand(shiftBB);
  match.merge->pushBack(resultPhi);
  auto* ret = new Instruction(Opcode::Ret, Type::voidTy());
  ret->addOperand(resultPhi);
  match.merge->pushBack(ret);

  for (BasicBlock* bb : match.blocksToRemove) {
    if (!bb || bb == match.entry || bb == match.merge)
      continue;
    match.func->removeBlock(bb);
  }
  match.func->recomputeCFG();
  return true;
}

bool rewriteDivPow2Helper(ShiftHelperMatch& match) {
  if (!match.func || !match.entry || !match.merge || !match.base ||
      !match.shift || match.maxAmount < 1) {
    return false;
  }

  Module* mod = match.func->parent();
  if (!mod)
    return false;

  BasicBlock* shiftBB = match.func->newBlock("instcombine.divpow2");
  clearBlock(match.entry);
  clearBlock(match.merge);

  auto* geLower = makeICmp("ge", match.shift, mod->getConstInt(1),
                           "instcombine.lower");
  match.entry->pushBack(geLower);
  auto* leUpper = makeICmp("le", match.shift, mod->getConstInt(match.maxAmount),
                           "instcombine.upper");
  match.entry->pushBack(leUpper);
  auto* inRange = new Instruction(Opcode::And, Type::i32(), "instcombine.inrange");
  inRange->addOperand(geLower);
  inRange->addOperand(leUpper);
  match.entry->pushBack(inRange);
  auto* entryBr = new Instruction(Opcode::CondBr, Type::voidTy());
  entryBr->addOperand(inRange);
  entryBr->addOperand(shiftBB);
  entryBr->addOperand(match.merge);
  match.entry->pushBack(entryBr);

  auto* sign = new Instruction(Opcode::Shr, Type::i32(), "instcombine.sign");
  sign->addOperand(match.base);
  sign->addOperand(mod->getConstInt(31));
  shiftBB->pushBack(sign);
  auto* scale = new Instruction(Opcode::Shl, Type::i32(), "instcombine.scale");
  scale->addOperand(mod->getConstInt(1));
  scale->addOperand(match.shift);
  shiftBB->pushBack(scale);
  auto* mask = new Instruction(Opcode::Sub, Type::i32(), "instcombine.mask");
  mask->addOperand(scale);
  mask->addOperand(mod->getConstInt(1));
  shiftBB->pushBack(mask);
  auto* bias = new Instruction(Opcode::And, Type::i32(), "instcombine.bias");
  bias->addOperand(sign);
  bias->addOperand(mask);
  shiftBB->pushBack(bias);
  auto* adjusted = new Instruction(Opcode::Add, Type::i32(), "instcombine.adjusted");
  adjusted->addOperand(match.base);
  adjusted->addOperand(bias);
  shiftBB->pushBack(adjusted);
  auto* shifted = new Instruction(Opcode::Shr, Type::i32(), "instcombine.shr");
  shifted->addOperand(adjusted);
  shifted->addOperand(match.shift);
  shiftBB->pushBack(shifted);
  auto* shiftBr = new Instruction(Opcode::Br, Type::voidTy());
  shiftBr->addOperand(match.merge);
  shiftBB->pushBack(shiftBr);

  auto* resultPhi = new Instruction(Opcode::Phi, Type::i32(), "instcombine.result");
  resultPhi->addOperand(match.base);
  resultPhi->addOperand(match.entry);
  resultPhi->addOperand(shifted);
  resultPhi->addOperand(shiftBB);
  match.merge->pushBack(resultPhi);
  auto* ret = new Instruction(Opcode::Ret, Type::voidTy());
  ret->addOperand(resultPhi);
  match.merge->pushBack(ret);

  for (BasicBlock* bb : match.blocksToRemove) {
    if (!bb || bb == match.entry || bb == match.merge)
      continue;
    match.func->removeBlock(bb);
  }
  match.func->recomputeCFG();
  return true;
}

bool matchLocalShlLadderAtPhi(Function* func, BasicBlock* merge,
                              Instruction* phi,
                              LocalShlLadderMatch& match) {
  if (!func || !merge || !phi || phi->type() != Type::i32())
    return false;
  if (analysis::collectLeadingPhi(merge).size() != 1)
    return false;

  Value* base = nullptr;
  BasicBlock* defaultPred = nullptr;
  std::unordered_map<int32_t, BasicBlock*> caseMap;
  std::vector<ShlCaseInfo> cases;

  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    Value* incoming = phi->operand(i);
    auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
    if (!pred)
      return false;

    Value* incomingBase = nullptr;
    int32_t amount = 0;
    if (matchShlCaseBlock(pred, merge, incoming, incomingBase, amount)) {
      if (!base)
        base = incomingBase;
      if (base != incomingBase || caseMap.count(amount))
        return false;
      caseMap[amount] = pred;
      cases.push_back({pred, amount});
      continue;
    }

    if (!pred->terminator() || pred->terminator()->opcode() != Opcode::Br ||
        pred->terminator()->numOperands() != 1 ||
        pred->terminator()->operand(0) != merge ||
        !nonPhiBodyInstructions(pred).empty()) {
      return false;
    }
    if (defaultPred)
      return false;
    defaultPred = pred;
    if (!base)
      base = incoming;
    else if (base != incoming)
      return false;
  }

  int32_t maxAmount = 0;
  if (!base || !defaultPred || !caseAmountsAreContiguous(cases, maxAmount))
    return false;

  std::unordered_set<BasicBlock*> compareSet;
  for (const auto& [_, caseBB] : caseMap) {
    if (caseBB->preds().size() != 1)
      return false;
    compareSet.insert(caseBB->preds().front());
  }
  if (defaultPred->preds().size() != 1)
    return false;
  compareSet.insert(defaultPred->preds().front());

  BasicBlock* head = nullptr;
  for (BasicBlock* bb : compareSet) {
    int incomingFromRegion = 0;
    for (BasicBlock* pred : bb->preds()) {
      if (compareSet.count(pred))
        ++incomingFromRegion;
    }
    if (incomingFromRegion == 0) {
      if (head)
        return false;
      head = bb;
    }
  }
  if (!head)
    return false;

  std::unordered_set<BasicBlock*> visited;
  std::unordered_set<int32_t> seenCases;
  std::vector<BasicBlock*> compareBlocks;
  Value* shift = nullptr;
  BasicBlock* current = head;

  while (current && current != defaultPred) {
    if (!compareSet.count(current) || !visited.insert(current).second)
      return false;

    auto body = nonPhiBodyInstructions(current);
    if (body.empty())
      return false;
    Instruction* cmp = body.back();
    Value* compared = nullptr;
    int32_t amount = 0;
    if (!matchEqCompareAgainstConstant(cmp, compared, amount))
      return false;
    if (current != head && body.size() != 1)
      return false;
    if (!shift)
      shift = compared;
    else if (shift != compared)
      return false;
    auto caseIt = caseMap.find(amount);
    if (caseIt == caseMap.end())
      return false;

    Instruction* term = current->terminator();
    if (!term || term->opcode() != Opcode::CondBr || term->numOperands() != 3)
      return false;
    auto* trueSucc = dynamic_cast<BasicBlock*>(term->operand(1));
    auto* falseSucc = dynamic_cast<BasicBlock*>(term->operand(2));
    if (!trueSucc || !falseSucc || trueSucc == falseSucc)
      return false;

    BasicBlock* next = nullptr;
    if (trueSucc == caseIt->second) {
      next = falseSucc;
    } else if (falseSucc == caseIt->second) {
      next = trueSucc;
    } else {
      return false;
    }

    seenCases.insert(amount);
    compareBlocks.push_back(current);
    current = next;
  }

  if (current != defaultPred || !shift || seenCases.size() != caseMap.size())
    return false;
  if (visited.size() != compareSet.size())
    return false;

  std::unordered_set<BasicBlock*> covered;
  covered.insert(merge);
  covered.insert(defaultPred);
  for (BasicBlock* bb : compareBlocks)
    covered.insert(bb);
  for (const auto& [_, caseBB] : caseMap)
    covered.insert(caseBB);

  for (BasicBlock* bb : covered) {
    if (!bb)
      return false;
  }
  std::vector<BasicBlock*> coveredVec(covered.begin(), covered.end());
  if (!blocksHaveNoExternalLabelUsers(coveredVec, {head, merge})) {
    return false;
  }

  match.func = func;
  match.head = head;
  match.merge = merge;
  match.oldPhi = phi;
  match.headCmp = nonPhiBodyInstructions(head).back();
  match.base = base;
  match.shift = shift;
  match.maxAmount = maxAmount;
  match.compareBlocks = std::move(compareBlocks);
  match.blocksToRemove = std::move(coveredVec);
  return true;
}

bool matchAnyLocalShlLadder(Function* func, LocalShlLadderMatch& match) {
  if (!func)
    return false;
  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!bb)
      continue;
    auto phis = analysis::collectLeadingPhi(bb);
    if (phis.size() != 1)
      continue;
    if (matchLocalShlLadderAtPhi(func, bb, phis.front(), match))
      return true;
  }
  return false;
}

bool rewriteLocalShlLadder(LocalShlLadderMatch& match) {
  if (!match.func || !match.head || !match.merge || !match.oldPhi ||
      !match.headCmp || !match.base || !match.shift || match.maxAmount < 1) {
    return false;
  }

  Module* mod = match.func->parent();
  if (!mod)
    return false;

  Instruction* oldTerm = match.head->terminator();
  if (!oldTerm)
    return false;

  match.head->remove(match.headCmp);
  match.head->remove(oldTerm);

  auto* geLower = makeICmp("ge", match.shift, mod->getConstInt(1),
                           "instcombine.lower");
  match.head->pushBack(geLower);
  auto* leUpper = makeICmp("le", match.shift, mod->getConstInt(match.maxAmount),
                           "instcombine.upper");
  match.head->pushBack(leUpper);
  auto* inRange = new Instruction(Opcode::And, Type::i32(), "instcombine.inrange");
  inRange->addOperand(geLower);
  inRange->addOperand(leUpper);
  match.head->pushBack(inRange);

  BasicBlock* computeBB = match.func->newBlock("instcombine.shift");
  auto* headBr = new Instruction(Opcode::CondBr, Type::voidTy());
  headBr->addOperand(inRange);
  headBr->addOperand(computeBB);
  headBr->addOperand(match.merge);
  match.head->pushBack(headBr);

  auto* shifted = new Instruction(Opcode::Shl, Type::i32(), "instcombine.shl");
  shifted->addOperand(match.base);
  shifted->addOperand(match.shift);
  computeBB->pushBack(shifted);
  auto* computeBr = new Instruction(Opcode::Br, Type::voidTy());
  computeBr->addOperand(match.merge);
  computeBB->pushBack(computeBr);

  match.oldPhi->dropAllOperands();
  match.oldPhi->addOperand(match.base);
  match.oldPhi->addOperand(match.head);
  match.oldPhi->addOperand(shifted);
  match.oldPhi->addOperand(computeBB);

  for (BasicBlock* bb : match.blocksToRemove) {
    if (!bb || bb == match.head || bb == match.merge)
      continue;
    match.func->removeBlock(bb);
  }
  match.func->recomputeCFG();
  return true;
}

bool runOnFunction(Function* func) {
  if (!func || !func->entryBlock() || func->retTy() != Type::i32())
    return false;

  if (simplifyDominatedReflectiveMax(func))
    return true;

  ShiftHelperMatch match;
  if (matchShlHelper(func, match))
    return rewriteShlHelper(match);

  if (matchDivPow2Helper(func, match))
    return rewriteDivPow2Helper(match);

  return false;
}

} // namespace

bool applyInstCombine(Module& mod, analysis::AnalysisManager& analysisManager) {
  (void)analysisManager;
  bool changed = false;
  for (auto& funcPtr : mod.functions())
    changed |= runOnFunction(funcPtr.get());
  return changed;
}

bool applyPostInlineInstCombine(Module& mod,
                                analysis::AnalysisManager& analysisManager) {
  (void)analysisManager;
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func || !func->entryBlock() || func->retTy() != Type::i32())
      continue;
    changed |= simplifyDominatedReflectiveMax(func);
  }
  return changed;
}

class InstCombinePass : public Pass {
public:
    std::string name() const override { return "InstCombine"; }
    bool run(Module& mod, analysis::AnalysisManager& am) override {
        return applyInstCombine(mod, am);
    }
};
REGISTER_PASS(InstCombinePass);

} // namespace core_ir
