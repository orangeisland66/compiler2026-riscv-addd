// 文件职责：描述 affine 访存表达式，供循环变换和地址递推使用。本文件负责实现具体逻辑。
#ifdef ANALYSIS_AFFINE_ACCESS_EMBED

#include "affine_access.h"
#include "loop_value_utils.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace analysis {

namespace {

using core_ir::Argument;
using core_ir::BasicBlock;
using core_ir::ConstInt;
using core_ir::GlobalVar;
using core_ir::Instruction;
using core_ir::Opcode;
using core_ir::Value;

bool isInvariantValue(Value* val, const NaturalLoop& loop) {
  if (!val)
    return false;
  if (val->isConstant() || dynamic_cast<Argument*>(val) ||
      dynamic_cast<GlobalVar*>(val) || dynamic_cast<BasicBlock*>(val)) {
    return true;
  }
  auto* inst = dynamic_cast<Instruction*>(val);
  if (!inst || !inst->parent())
    return true;
  if (!loop.blocks.count(inst->parent()))
    return true;
  if (inst->opcode() == Opcode::Load && inst->numOperands() == 1) {
    Value* slot = inst->operand(0);
    if (isScalarSlot(slot) && !loopStoresToSlot(loop, slot)) {
      return true;
    }
  }
  switch (inst->opcode()) {
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Mul:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
  case Opcode::Shl:
  case Opcode::Shr:
  case Opcode::BitCast:
  case Opcode::Neg:
    break;
  default:
    return false;
  }
  for (auto* op : inst->operands()) {
    if (!isInvariantValue(op, loop))
      return false;
  }
  return true;
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

void mergeScaledTerm(std::vector<AffineScaledTerm>& terms,
                     Value* basis,
                     Value* invariantScale,
                     int64_t coeff) {
  if (!basis || coeff == 0)
    return;
  for (auto& term : terms) {
    if (term.basis == basis && term.invariantScale == invariantScale) {
      term.coeff += coeff;
      return;
    }
  }
  terms.push_back({basis, invariantScale, coeff});
}

void addExpr(AffineExprSummary& dst, const AffineExprSummary& src, int sign) {
  dst.constant += sign * src.constant;
  for (const auto& term : src.ivTerms)
    mergeTerm(dst.ivTerms, term.basis, sign * term.coeff);
  for (const auto& term : src.scaledIvTerms)
    mergeScaledTerm(dst.scaledIvTerms, term.basis, term.invariantScale,
                    sign * term.coeff);
  for (const auto& term : src.invariantTerms)
    mergeTerm(dst.invariantTerms, term.basis, sign * term.coeff);
}

struct AffineContext {
  const NaturalLoop& loop;
  std::unordered_map<int, Value*> ivBases;
  std::unordered_map<int, AffineExprSummary> memo;
  std::unordered_set<int> active;
};

Value* canonicalIVBasis(Value* val, const AffineContext& ctx) {
  if (!val)
    return nullptr;

  auto directIt = ctx.ivBases.find(val->id());
  if (directIt != ctx.ivBases.end())
    return directIt->second;

  auto* inst = dynamic_cast<Instruction*>(val);
  if (!inst || inst->opcode() != Opcode::Load || inst->numOperands() != 1)
    return nullptr;

  Value* slot = inst->operand(0);
  if (!slot)
    return nullptr;
  auto slotIt = ctx.ivBases.find(slot->id());
  if (slotIt == ctx.ivBases.end())
    return nullptr;
  return slotIt->second;
}

bool isDirectIV(Value* val, const AffineContext& ctx) {
  return canonicalIVBasis(val, ctx) != nullptr;
}

AffineExprSummary summarizeValue(Value* val, AffineContext& ctx);

AffineExprSummary summarizeMul(Instruction* inst, AffineContext& ctx) {
  AffineExprSummary out;
  if (!inst || inst->numOperands() != 2)
    return out;

  auto* lc = dynamic_cast<ConstInt*>(inst->operand(0));
  auto* rc = dynamic_cast<ConstInt*>(inst->operand(1));
  if (lc || rc) {
    int64_t factor = lc ? lc->value() : rc->value();
    Value* other = lc ? inst->operand(1) : inst->operand(0);
    AffineExprSummary inner = summarizeValue(other, ctx);
    if (!inner.valid)
      return out;
    out.valid = true;
    out.constant = inner.constant * factor;
    for (const auto& term : inner.ivTerms)
      mergeTerm(out.ivTerms, term.basis, term.coeff * factor);
    for (const auto& term : inner.scaledIvTerms) {
      mergeScaledTerm(out.scaledIvTerms, term.basis, term.invariantScale,
                      term.coeff * factor);
    }
    for (const auto& term : inner.invariantTerms)
      mergeTerm(out.invariantTerms, term.basis, term.coeff * factor);
    return out;
  }

  Value* invariant = nullptr;
  Value* maybeIV = nullptr;
  if (isInvariantValue(inst->operand(0), ctx.loop) &&
      isDirectIV(inst->operand(1), ctx)) {
    invariant = inst->operand(0);
    maybeIV = inst->operand(1);
  } else if (isInvariantValue(inst->operand(1), ctx.loop) &&
             isDirectIV(inst->operand(0), ctx)) {
    invariant = inst->operand(1);
    maybeIV = inst->operand(0);
  }
  if (!invariant || !maybeIV)
    return out;

  out.valid = true;
  mergeScaledTerm(out.scaledIvTerms, canonicalIVBasis(maybeIV, ctx), invariant,
                  1);
  return out;
}

AffineExprSummary summarizeValue(Value* val, AffineContext& ctx) {
  AffineExprSummary out;
  if (!val)
    return out;
  if (auto* ci = dynamic_cast<ConstInt*>(val)) {
    out.valid = true;
    out.constant = ci->value();
    return out;
  }
  if (isDirectIV(val, ctx)) {
    out.valid = true;
    mergeTerm(out.ivTerms, canonicalIVBasis(val, ctx), 1);
    return out;
  }

  auto memoIt = ctx.memo.find(val->id());
  if (memoIt != ctx.memo.end())
    return memoIt->second;
  if (!ctx.active.insert(val->id()).second)
    return out;

  auto* inst = dynamic_cast<Instruction*>(val);
  if (!inst) {
    ctx.active.erase(val->id());
    return out;
  }

  // 按操作码把定义递归归并为仿射表达式：加减线性合并，取负变号，
  // 常量左移等价于系数乘以 2 的幂；乘法委托 summarizeMul，其余不可
  // 归并时留待循环不变量兜底。
  switch (inst->opcode()) {
  case Opcode::Add:
  case Opcode::Sub: {
    AffineExprSummary lhs = summarizeValue(inst->operand(0), ctx);
    AffineExprSummary rhs = summarizeValue(inst->operand(1), ctx);
    if (!lhs.valid || !rhs.valid)
      break;
    out.valid = true;
    addExpr(out, lhs, 1);
    addExpr(out, rhs, inst->opcode() == Opcode::Add ? 1 : -1);
    break;
  }
  case Opcode::Neg: {
    AffineExprSummary inner = summarizeValue(inst->operand(0), ctx);
    if (!inner.valid)
      break;
    out.valid = true;
    addExpr(out, inner, -1);
    break;
  }
  case Opcode::Shl: {
    auto* shift = dynamic_cast<ConstInt*>(inst->operand(1));
    if (!shift || shift->value() < 0 || shift->value() > 30)
      break;
    AffineExprSummary inner = summarizeValue(inst->operand(0), ctx);
    if (!inner.valid)
      break;
    int64_t factor = int64_t{1} << shift->value();
    out.valid = true;
    out.constant = inner.constant * factor;
    for (const auto& term : inner.ivTerms)
      mergeTerm(out.ivTerms, term.basis, term.coeff * factor);
    for (const auto& term : inner.scaledIvTerms) {
      mergeScaledTerm(out.scaledIvTerms, term.basis, term.invariantScale,
                      term.coeff * factor);
    }
    for (const auto& term : inner.invariantTerms)
      mergeTerm(out.invariantTerms, term.basis, term.coeff * factor);
    break;
  }
  case Opcode::Mul:
    out = summarizeMul(inst, ctx);
    break;
  case Opcode::BitCast:
    if (inst->numOperands() == 1)
      out = summarizeValue(inst->operand(0), ctx);
    break;
  default:
    break;
  }

  if (!out.valid && isInvariantValue(val, ctx.loop)) {
    out.valid = true;
    mergeTerm(out.invariantTerms, val, 1);
  }

  ctx.active.erase(val->id());
  ctx.memo[val->id()] = out;
  return out;
}

} // namespace

AffineAccessSummary summarizeAffineAccess(Value* addr,
                                          const NaturalLoop& loop,
                                          const std::vector<InductionVar>& ivs) {
  AffineAccessSummary summary;
  if (!addr)
    return summary;

  AffineContext ctx{loop};
  for (const auto& iv : ivs) {
    if (iv.phi)
      ctx.ivBases[iv.phi->id()] = iv.phi;
    if (iv.storage)
      ctx.ivBases[iv.storage->id()] = iv.storage;
  }

  summary.valid = true;
  summary.root = addr;

  std::vector<Value*> parts;
  Value* cur = addr;
  while (auto* inst = dynamic_cast<Instruction*>(cur)) {
    if (inst->opcode() == Opcode::BitCast && inst->numOperands() == 1) {
      cur = inst->operand(0);
      continue;
    }
    if (inst->opcode() == Opcode::GEP && inst->numOperands() >= 1) {
      for (int i = 1; i < inst->numOperands(); ++i)
        parts.push_back(inst->operand(i));
      cur = inst->operand(0);
      continue;
    }
    break;
  }
  summary.root = cur;

  summary.offset.valid = true;
  for (auto it = parts.rbegin(); it != parts.rend(); ++it) {
    AffineExprSummary piece = summarizeValue(*it, ctx);
    if (!piece.valid) {
      summary.valid = false;
      summary.offset.valid = false;
      return summary;
    }
    addExpr(summary.offset, piece, 1);
  }

  return summary;
}

AffineExprSummary summarizeAffineExpr(Value* value,
                                      const NaturalLoop& loop,
                                      const std::vector<InductionVar>& ivs) {
  AffineContext ctx{loop};
  for (const auto& iv : ivs) {
    if (iv.phi)
      ctx.ivBases[iv.phi->id()] = iv.phi;
    if (iv.storage)
      ctx.ivBases[iv.storage->id()] = iv.storage;
  }
  return summarizeValue(value, ctx);
}

int64_t affineConstCoeffFor(const AffineAccessSummary& summary,
                            Value* basis) {
  if (!summary.valid || !summary.offset.valid || !basis)
    return 0;
  for (const auto& term : summary.offset.ivTerms) {
    if (term.basis == basis)
      return term.coeff;
  }
  return 0;
}

int64_t affineWeightedCoeffFor(const AffineAccessSummary& summary,
                               Value* basis) {
  if (!summary.valid || !summary.offset.valid || !basis)
    return 0;
  int64_t coeff = 0;
  for (const auto& term : summary.offset.ivTerms) {
    if (term.basis == basis)
      coeff += term.coeff;
  }
  for (const auto& term : summary.offset.scaledIvTerms) {
    if (term.basis == basis)
      coeff += term.coeff;
  }
  return coeff;
}

int64_t affineStrideCostForProfit(const AffineAccessSummary& summary,
                                  Value* basis) {
  if (!summary.valid || !summary.offset.valid || !basis)
    return 0;
  // 运行期缩放项通常代表一整行、一整面或用户输入规模带来的
  // 大步长。这里给收益模型一个保守权重，只影响“是否值得交换/分块”，
  // 不参与 affine 等价判断。
  constexpr int64_t kDynamicScaleStrideWeight = 64;
  int64_t cost = 0;
  for (const auto& term : summary.offset.ivTerms) {
    if (term.basis == basis)
      cost += term.coeff < 0 ? -term.coeff : term.coeff;
  }
  for (const auto& term : summary.offset.scaledIvTerms) {
    if (term.basis != basis)
      continue;
    int64_t coeff = term.coeff < 0 ? -term.coeff : term.coeff;
    cost += coeff * kDynamicScaleStrideWeight;
  }
  return cost;
}

bool isLoopInvariantAffine(const AffineExprSummary& expr) {
  return expr.valid && expr.ivTerms.empty() && expr.scaledIvTerms.empty();
}

} // namespace analysis

#endif
