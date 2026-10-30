// 文件职责：归纳循环 guard 和边界信息，为 interior/remainder 拆分提供依据。本文件负责实现具体逻辑。
#include "affine_guard.h"

#include <algorithm>
#include <unordered_set>
#include <utility>

namespace analysis {

namespace {

using core_ir::BasicBlock;
using core_ir::ConstInt;
using core_ir::Instruction;
using core_ir::Opcode;
using core_ir::Value;

struct GuardLeafCompare {
  Instruction* compare = nullptr;
  bool negate = false;
};

std::string cmpCond(Instruction* inst) {
  if (!inst)
    return "";
  auto name = inst->name();
  auto pos = name.find('_');
  return pos == std::string::npos ? name : name.substr(0, pos);
}

std::string invertCmp(const std::string& cond) {
  if (cond == "lt")
    return "gt";
  if (cond == "le")
    return "ge";
  if (cond == "gt")
    return "lt";
  if (cond == "ge")
    return "le";
  return cond;
}

std::string negateCmp(const std::string& cond) {
  if (cond == "lt")
    return "ge";
  if (cond == "le")
    return "gt";
  if (cond == "gt")
    return "le";
  if (cond == "ge")
    return "lt";
  return cond;
}

void mergeTerm(std::vector<AffineTerm>& terms, Value* basis, int64_t coeff) {
  if (!basis || coeff == 0)
    return;
  for (auto& term : terms) {
    if (term.basis == basis) {
      term.coeff += coeff;
      return;
    }
  }
  terms.push_back({basis, coeff});
}

void mergeScaledTerm(std::vector<AffineScaledTerm>& terms, Value* basis,
                     Value* scale, int64_t coeff) {
  if (!basis || !scale || coeff == 0)
    return;
  for (auto& term : terms) {
    if (term.basis == basis && term.invariantScale == scale) {
      term.coeff += coeff;
      return;
    }
  }
  terms.push_back({basis, scale, coeff});
}

AffineExprSummary subtractExpr(const AffineExprSummary& lhs,
                               const AffineExprSummary& rhs) {
  AffineExprSummary out;
  if (!lhs.valid || !rhs.valid)
    return out;
  out.valid = true;
  out.constant = lhs.constant - rhs.constant;
  for (const auto& term : lhs.ivTerms)
    mergeTerm(out.ivTerms, term.basis, term.coeff);
  for (const auto& term : rhs.ivTerms)
    mergeTerm(out.ivTerms, term.basis, -term.coeff);
  for (const auto& term : lhs.scaledIvTerms)
    mergeScaledTerm(out.scaledIvTerms, term.basis, term.invariantScale,
                    term.coeff);
  for (const auto& term : rhs.scaledIvTerms)
    mergeScaledTerm(out.scaledIvTerms, term.basis, term.invariantScale,
                    -term.coeff);
  for (const auto& term : lhs.invariantTerms)
    mergeTerm(out.invariantTerms, term.basis, term.coeff);
  for (const auto& term : rhs.invariantTerms)
    mergeTerm(out.invariantTerms, term.basis, -term.coeff);
  return out;
}

AffineExprSummary negateExpr(const AffineExprSummary& expr) {
  AffineExprSummary out;
  if (!expr.valid)
    return out;
  out.valid = true;
  out.constant = -expr.constant;
  for (const auto& term : expr.ivTerms)
    mergeTerm(out.ivTerms, term.basis, -term.coeff);
  for (const auto& term : expr.scaledIvTerms)
    mergeScaledTerm(out.scaledIvTerms, term.basis, term.invariantScale,
                    -term.coeff);
  for (const auto& term : expr.invariantTerms)
    mergeTerm(out.invariantTerms, term.basis, -term.coeff);
  return out;
}

bool isGuardLocalOp(Instruction* inst) {
  if (!inst)
    return false;
  switch (inst->opcode()) {
  case Opcode::Load:
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Mul:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
  case Opcode::Shl:
  case Opcode::Shr:
  case Opcode::Neg:
  case Opcode::BitCast:
  case Opcode::ICmp:
    return true;
  default:
    return false;
  }
}

bool guardValueMayFeedSuccessor(Instruction* inst) {
  if (!inst)
    return false;
  switch (inst->opcode()) {
  case Opcode::Load:
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Mul:
  case Opcode::Shl:
  case Opcode::Shr:
  case Opcode::Neg:
  case Opcode::BitCast:
    return true;
  default:
    return false;
  }
}

bool blockIsGuardOnly(BasicBlock* bb, Instruction* cmp,
                      BasicBlock* succTrue, BasicBlock* succFalse) {
  if (!bb || !cmp)
    return false;
  std::unordered_set<int> localIds;
  for (auto& instPtr : bb->instructions()) {
    Instruction* inst = instPtr.get();
    if (inst->isTerminator())
      continue;
    if (!isGuardLocalOp(inst))
      return false;
    localIds.insert(inst->id());
  }
  if (!localIds.count(cmp->id()))
    return false;
  for (auto& instPtr : bb->instructions()) {
    Instruction* inst = instPtr.get();
    if (inst->isTerminator() || inst == cmp)
      continue;
    for (auto* user : inst->uses()) {
      if (!user)
        return false;
      BasicBlock* userBB = user->parent();
      if (userBB == bb && (user->isTerminator() || localIds.count(user->id())))
        continue;
      if ((userBB == succTrue || userBB == succFalse) &&
          guardValueMayFeedSuccessor(inst)) {
        // 边界 guard 常会复用同一组 affine 下标值做实际地址。
        // 这些纯 index 计算可以流向直接后继；compare/布尔树仍必须留在
        // guard block 内，保证 tightening 只剥离已证明的条件，不移动副作用。
        continue;
      }
      return false;
    }
  }
  return true;
}

bool collectGuardLeafCompares(Instruction* root,
                              std::vector<GuardLeafCompare>& compares,
                              std::unordered_set<int>& seenIds,
                              bool negate = false) {
  if (!root)
    return false;
  if (root->opcode() == Opcode::ICmp) {
    const std::string cond = cmpCond(root);
    if (cond == "lt" || cond == "le" || cond == "gt" || cond == "ge") {
      if (seenIds.insert(root->id()).second)
        compares.push_back({root, negate});
      return true;
    }

    if ((cond == "eq" || cond == "ne") && root->numOperands() == 2) {
      auto unwrapBoolCompare = [&](Value* maybeConst,
                                   Value* maybeInst) -> bool {
        auto* ci = dynamic_cast<ConstInt*>(maybeConst);
        auto* child = dynamic_cast<Instruction*>(maybeInst);
        if (!ci || !child)
          return false;
        const int32_t v = ci->value();
        if (v != 0 && v != 1)
          return false;
        bool childNegate = false;
        if (cond == "eq")
          childNegate = (v == 0);
        else
          childNegate = (v == 1);
        return collectGuardLeafCompares(child, compares, seenIds,
                                        negate ^ childNegate);
      };
      return unwrapBoolCompare(root->operand(0), root->operand(1)) ||
             unwrapBoolCompare(root->operand(1), root->operand(0));
    }
    return false;
  }
  if (root->opcode() != Opcode::And || root->numOperands() != 2 || negate)
    return false;
  auto* lhs = dynamic_cast<Instruction*>(root->operand(0));
  auto* rhs = dynamic_cast<Instruction*>(root->operand(1));
  if (!lhs || !rhs)
    return false;
  return collectGuardLeafCompares(lhs, compares, seenIds) &&
         collectGuardLeafCompares(rhs, compares, seenIds);
}

bool normalizeCompareForIV(Instruction* cmp, const NaturalLoop& loop,
                           const std::vector<InductionVar>& ivs,
                           Value* induction, AffineGuardSummary& out) {
  if (!cmp || cmp->opcode() != Opcode::ICmp || cmp->numOperands() != 2 ||
      !induction) {
    return false;
  }

  const std::string cond = cmpCond(cmp);
  if (cond != "lt" && cond != "le" && cond != "gt" && cond != "ge")
    return false;

  AffineExprSummary lhs = summarizeAffineExpr(cmp->operand(0), loop, ivs);
  AffineExprSummary rhs = summarizeAffineExpr(cmp->operand(1), loop, ivs);
  AffineExprSummary delta = subtractExpr(lhs, rhs);
  if (!delta.valid || !delta.scaledIvTerms.empty())
    return false;

  int64_t ivCoeff = 0;
  for (const auto& term : delta.ivTerms) {
    if (term.basis != induction)
      return false;
    ivCoeff += term.coeff;
  }
  if (ivCoeff != 1 && ivCoeff != -1)
    return false;

  AffineExprSummary rest;
  rest.valid = true;
  rest.constant = delta.constant;
  rest.invariantTerms = delta.invariantTerms;

  out.valid = true;
  out.compare = cmp;
  out.induction = induction;
  if (ivCoeff == 1) {
    out.relation = cond;
    out.boundExpr = negateExpr(rest);
  } else {
    out.relation = invertCmp(cond);
    out.boundExpr = rest;
  }
  return out.boundExpr.valid;
}

} // namespace

std::vector<AffineGuardSummary>
collectAffineGuards(core_ir::Function* func, const NaturalLoop& loop,
                    const std::vector<InductionVar>& ivs) {
  std::vector<AffineGuardSummary> guards;
  if (!func || !loop.header || ivs.empty())
    return guards;

  for (const auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!bb || bb == loop.header || !loop.blocks.count(bb))
      continue;

    Instruction* term = bb->terminator();
    if (!term || term->opcode() != Opcode::CondBr || term->numOperands() != 3)
      continue;

    auto* condRoot = dynamic_cast<Instruction*>(term->operand(0));
    auto* succTrue = dynamic_cast<BasicBlock*>(term->operand(1));
    auto* succFalse = dynamic_cast<BasicBlock*>(term->operand(2));
    if (!condRoot || !succTrue || !succFalse ||
        !blockIsGuardOnly(bb, condRoot, succTrue, succFalse))
      continue;

    if (!loop.blocks.count(succTrue) || !loop.blocks.count(succFalse) ||
        succTrue == succFalse) {
      continue;
    }

    std::vector<GuardLeafCompare> compares;
    std::unordered_set<int> seenCompareIds;
    if (!collectGuardLeafCompares(condRoot, compares, seenCompareIds) ||
        compares.empty()) {
      continue;
    }

    for (const auto& leaf : compares) {
      auto* cmp = leaf.compare;
      if (!cmp)
        continue;
      for (const auto& iv : ivs) {
        Value* induction = iv.phi ? static_cast<Value*>(iv.phi)
                                  : static_cast<Value*>(iv.storage);
        if (!induction)
          continue;
        AffineGuardSummary summary;
        if (!normalizeCompareForIV(cmp, loop, ivs, induction, summary))
          continue;
        if (leaf.negate)
          summary.relation = negateCmp(summary.relation);

        auto emitSummary = [&](BasicBlock* continueSucc, BasicBlock* workSucc,
                               bool negateRelation,
                               bool compareTrueForWork) {
          AffineGuardSummary oriented = summary;
          if (negateRelation)
            oriented.relation = negateCmp(oriented.relation);
          oriented.loop = &loop;
          oriented.block = bb;
          oriented.header = loop.header;
          oriented.branch = term;
          oriented.continueBlock = continueSucc;
          oriented.workBlock = workSucc;
          oriented.compareTrueForWork = compareTrueForWork;
          guards.push_back(std::move(oriented));
        };

        emitSummary(succTrue, succFalse, false, false);
        emitSummary(succFalse, succTrue, true, true);
      }
    }
  }

  return guards;
}

} // namespace analysis
