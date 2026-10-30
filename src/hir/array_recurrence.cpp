#include "research_switch.h"
// 文件职责：识别仿射数组地址并在默认循环流水线中改写为指针递推。
#include "array_recurrence.h"
#include "../analysis/cfg_utils.h"
#include "../analysis/dominators.h"
#include "../analysis/induction_var.h"
#include "../analysis/loop_info.h"
#include "../analysis/loop_value_utils.h"
#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hir {

namespace {

using namespace core_ir;
using analysis::NaturalLoop;

struct OffsetRecurrence {
  int64_t ivConstCoeff = 0;
  std::vector<std::pair<Value*, int64_t>> ivInvariantTerms;
  int64_t constOffset = 0;
  std::vector<std::pair<Value*, int64_t>> invariantTerms;
};

bool isSyntheticRecurrenceGEP(Instruction* inst) {
  if (!inst || inst->opcode() != Opcode::GEP)
    return false;
  const std::string& name = inst->name();
  return name.find(".ar.") != std::string::npos;
}

bool valueDependsOnIV(Value* val, Instruction* iv,
                      std::unordered_map<int, bool>& memo) {
  if (!val)
    return false;
  if (val == iv)
    return true;
  if (val->isConstant() || dynamic_cast<Argument*>(val) ||
      dynamic_cast<GlobalVar*>(val) || dynamic_cast<BasicBlock*>(val)) {
    return false;
  }

  auto it = memo.find(val->id());
  if (it != memo.end())
    return it->second;

  auto* inst = dynamic_cast<Instruction*>(val);
  if (!inst) {
    memo[val->id()] = false;
    return false;
  }

  memo[val->id()] = false;
  bool depends = false;
  for (auto* op : inst->operands()) {
    if (valueDependsOnIV(op, iv, memo)) {
      depends = true;
      break;
    }
  }
  memo[val->id()] = depends;
  return depends;
}

bool isLoopInvariantValue(Value* val, const NaturalLoop& loop, Instruction* iv,
                          std::unordered_map<int, bool>& depMemo,
                          std::unordered_map<int, bool>& invMemo) {
  if (!val)
    return false;
  if (val->isConstant() || dynamic_cast<Argument*>(val) ||
      dynamic_cast<GlobalVar*>(val) || dynamic_cast<BasicBlock*>(val)) {
    return true;
  }

  auto it = invMemo.find(val->id());
  if (it != invMemo.end())
    return it->second;

  auto* inst = dynamic_cast<Instruction*>(val);
  if (!inst || !inst->parent()) {
    invMemo[val->id()] = true;
    return true;
  }
  if (!loop.blocks.count(inst->parent())) {
    invMemo[val->id()] = true;
    return true;
  }
  if (inst->opcode() == Opcode::Load && inst->numOperands() == 1 &&
      analysis::isScalarSlot(inst->operand(0)) &&
      !analysis::loopStoresToSlot(loop, inst->operand(0))) {
    invMemo[val->id()] = true;
    return true;
  }
  if (valueDependsOnIV(val, iv, depMemo) ||
      !analysis::isSimpleInvariantExpr(inst)) {
    invMemo[val->id()] = false;
    return false;
  }

  invMemo[val->id()] = false;
  for (auto* op : inst->operands()) {
    if (!isLoopInvariantValue(op, loop, iv, depMemo, invMemo))
      return false;
  }
  invMemo[val->id()] = true;
  return true;
}

bool matchScaledIV(Value* val, Instruction* iv, int64_t& coeff) {
  if (val == iv) {
    coeff = 1;
    return true;
  }
  auto* inst = dynamic_cast<Instruction*>(val);
  if (!inst || inst->opcode() != Opcode::Mul || inst->numOperands() != 2)
    return false;

  auto* lhsConst = dynamic_cast<ConstInt*>(inst->operand(0));
  auto* rhsConst = dynamic_cast<ConstInt*>(inst->operand(1));
  if (inst->operand(0) == iv && rhsConst) {
    coeff = rhsConst->value();
    return true;
  }
  if (inst->operand(1) == iv && lhsConst) {
    coeff = lhsConst->value();
    return true;
  }
  return false;
}

bool matchScaledLoopCounter(Value* val, const analysis::InductionVar& iv,
                            int64_t& coeff) {
  if (iv.phi && matchScaledIV(val, iv.phi, coeff))
    return true;
  if (!iv.storage)
    return false;

  auto* load = dynamic_cast<Instruction*>(val);
  if (load && load->opcode() == Opcode::Load && load->numOperands() == 1 &&
      load->operand(0) == iv.storage) {
    coeff = 1;
    return true;
  }

  auto* inst = dynamic_cast<Instruction*>(val);
  if (!inst || inst->opcode() != Opcode::Mul || inst->numOperands() != 2)
    return false;
  auto* lhsConst = dynamic_cast<ConstInt*>(inst->operand(0));
  auto* rhsConst = dynamic_cast<ConstInt*>(inst->operand(1));
  if (lhsConst) {
    int64_t innerCoeff = 0;
    if (matchScaledLoopCounter(inst->operand(1), iv, innerCoeff)) {
      coeff = innerCoeff * lhsConst->value();
      return true;
    }
  }
  if (rhsConst) {
    int64_t innerCoeff = 0;
    if (matchScaledLoopCounter(inst->operand(0), iv, innerCoeff)) {
      coeff = innerCoeff * rhsConst->value();
      return true;
    }
  }
  if (inst->opcode() == Opcode::Shl && inst->numOperands() == 2) {
    auto* shift = dynamic_cast<ConstInt*>(inst->operand(1));
    if (shift && shift->value() >= 0 && shift->value() <= 20) {
      int64_t innerCoeff = 0;
      if (matchScaledLoopCounter(inst->operand(0), iv, innerCoeff)) {
        coeff = innerCoeff * (1LL << shift->value());
        return true;
      }
    }
  }
  return false;
}

bool matchInvariantScaledIV(Value* val, const NaturalLoop& loop,
                            const analysis::InductionVar& iv,
                            std::unordered_map<int, bool>& depMemo,
                            std::unordered_map<int, bool>& invMemo,
                            Value*& invariantFactor, int64_t& coeff) {
  auto* inst = dynamic_cast<Instruction*>(val);
  if (!inst || inst->opcode() != Opcode::Mul || inst->numOperands() != 2)
    return false;

  int64_t scaledCoeff = 0;
  Instruction* canonicalIV = iv.phi ? iv.phi : dynamic_cast<Instruction*>(iv.storage);
  if (matchScaledLoopCounter(inst->operand(0), iv, scaledCoeff) &&
      isLoopInvariantValue(inst->operand(1), loop, canonicalIV, depMemo, invMemo)) {
    invariantFactor = inst->operand(1);
    coeff = scaledCoeff;
    return true;
  }
  if (matchScaledLoopCounter(inst->operand(1), iv, scaledCoeff) &&
      isLoopInvariantValue(inst->operand(0), loop, canonicalIV, depMemo, invMemo)) {
    invariantFactor = inst->operand(0);
    coeff = scaledCoeff;
    return true;
  }
  return false;
}

void scaleOffsetRecurrence(OffsetRecurrence& rec, int64_t factor) {
  rec.ivConstCoeff *= factor;
  for (auto& [term, coeff] : rec.ivInvariantTerms)
    coeff *= factor;
  rec.constOffset *= factor;
  for (auto& [term, coeff] : rec.invariantTerms)
    coeff *= factor;
}

bool collectOffsetRecurrence(Value* val, const NaturalLoop& loop,
                             const analysis::InductionVar& iv,
                             std::unordered_map<int, bool>& depMemo,
                             std::unordered_map<int, bool>& invMemo,
                             int sign, OffsetRecurrence& out) {
  if (!val)
    return false;

  if (auto* ci = dynamic_cast<ConstInt*>(val)) {
    out.constOffset += static_cast<int64_t>(sign) * ci->value();
    return true;
  }

  int64_t scaledCoeff = 0;
  if (matchScaledLoopCounter(val, iv, scaledCoeff)) {
    out.ivConstCoeff += static_cast<int64_t>(sign) * scaledCoeff;
    return true;
  }

  Value* invariantFactor = nullptr;
  int64_t invariantCoeff = 0;
  if (matchInvariantScaledIV(val, loop, iv, depMemo, invMemo,
                             invariantFactor, invariantCoeff)) {
    out.ivInvariantTerms.push_back(
        {invariantFactor, static_cast<int64_t>(sign) * invariantCoeff});
    return true;
  }

  // 递归拆解偏移表达式：Add/Sub 保持或翻转符号后递归两操作数；
  // Mul/Shl 要求一侧为常量因子，先递归再整体缩放（移位量限 0..20 防溢出）。
  auto* inst = dynamic_cast<Instruction*>(val);
  if (inst && inst->numOperands() == 2 && inst->opcode() == Opcode::Add) {
    return collectOffsetRecurrence(inst->operand(0), loop, iv, depMemo, invMemo,
                                   sign, out) &&
           collectOffsetRecurrence(inst->operand(1), loop, iv, depMemo, invMemo,
                                   sign, out);
  }
  if (inst && inst->numOperands() == 2 && inst->opcode() == Opcode::Sub) {
    return collectOffsetRecurrence(inst->operand(0), loop, iv, depMemo, invMemo,
                                   sign, out) &&
           collectOffsetRecurrence(inst->operand(1), loop, iv, depMemo, invMemo,
                                   -sign, out);
  }
  if (inst && inst->numOperands() == 2 && inst->opcode() == Opcode::Mul) {
    auto* lhsConst = dynamic_cast<ConstInt*>(inst->operand(0));
    auto* rhsConst = dynamic_cast<ConstInt*>(inst->operand(1));
    if (lhsConst || rhsConst) {
      Value* expr = lhsConst ? inst->operand(1) : inst->operand(0);
      int64_t factor = lhsConst ? lhsConst->value() : rhsConst->value();
      OffsetRecurrence scaled;
      if (!collectOffsetRecurrence(expr, loop, iv, depMemo, invMemo, sign, scaled))
        return false;
      scaleOffsetRecurrence(scaled, factor);
      out.ivConstCoeff += scaled.ivConstCoeff;
      out.ivInvariantTerms.insert(out.ivInvariantTerms.end(),
                                  scaled.ivInvariantTerms.begin(),
                                  scaled.ivInvariantTerms.end());
      out.constOffset += scaled.constOffset;
      out.invariantTerms.insert(out.invariantTerms.end(),
                                scaled.invariantTerms.begin(),
                                scaled.invariantTerms.end());
      return true;
    }
  }
  if (inst && inst->numOperands() == 2 && inst->opcode() == Opcode::Shl) {
    auto* shift = dynamic_cast<ConstInt*>(inst->operand(1));
    if (shift && shift->value() >= 0 && shift->value() <= 20) {
      OffsetRecurrence scaled;
      if (!collectOffsetRecurrence(inst->operand(0), loop, iv, depMemo,
                                   invMemo, sign, scaled)) {
        return false;
      }
      scaleOffsetRecurrence(scaled, 1LL << shift->value());
      out.ivConstCoeff += scaled.ivConstCoeff;
      out.ivInvariantTerms.insert(out.ivInvariantTerms.end(),
                                  scaled.ivInvariantTerms.begin(),
                                  scaled.ivInvariantTerms.end());
      out.constOffset += scaled.constOffset;
      out.invariantTerms.insert(out.invariantTerms.end(),
                                scaled.invariantTerms.begin(),
                                scaled.invariantTerms.end());
      return true;
    }
  }

  // 兜底分支：既不是递推结构也不能再拆解的叶子值，必须为循环不变量，
  // 按当前符号记入 invariantTerms，否则整体匹配失败。
  Instruction* canonicalIV = iv.phi ? iv.phi : dynamic_cast<Instruction*>(iv.storage);
  if (!canonicalIV)
    return false;
  if (!isLoopInvariantValue(val, loop, canonicalIV, depMemo, invMemo))
    return false;
  out.invariantTerms.push_back({val, sign});
  return true;
}

Value* cloneInvariantExpr(Value* val, BasicBlock* preheader,
                          Instruction* preheaderTerm, const NaturalLoop& loop,
                          std::unordered_map<int, Value*>& cache) {
  if (!val)
    return nullptr;
  if (val->isConstant() || dynamic_cast<Argument*>(val) ||
      dynamic_cast<GlobalVar*>(val) || dynamic_cast<BasicBlock*>(val)) {
    return val;
  }

  auto it = cache.find(val->id());
  if (it != cache.end())
    return it->second;

  auto* inst = dynamic_cast<Instruction*>(val);
  if (!inst || !inst->parent())
    return val;
  if (!loop.blocks.count(inst->parent()))
    return val;
  if (inst->opcode() == Opcode::Load && inst->numOperands() == 1 &&
      analysis::isScalarSlot(inst->operand(0)) &&
      !analysis::loopStoresToSlot(loop, inst->operand(0))) {
    auto* clone = new Instruction(Opcode::Load, inst->type(),
                                  inst->name().empty() ? "ar.load"
                                                       : inst->name() + ".ar");
    clone->addOperand(inst->operand(0));
    preheader->insertBefore(preheaderTerm, clone);
    cache[val->id()] = clone;
    return clone;
  }
  if (!analysis::isSimpleInvariantExpr(inst))
    return nullptr;

  auto* clone = new Instruction(inst->opcode(), inst->type(),
                                inst->name().empty() ? "ar.inv"
                                                     : inst->name() + ".ar");
  for (auto* op : inst->operands()) {
    Value* clonedOp = cloneInvariantExpr(op, preheader, preheaderTerm, loop, cache);
    if (!clonedOp)
      return nullptr;
    clone->addOperand(clonedOp);
  }
  preheader->insertBefore(preheaderTerm, clone);
  cache[val->id()] = clone;
  return clone;
}

Value* buildInvariantExpr(int64_t constTerm,
                          const std::vector<std::pair<Value*, int64_t>>& terms,
                          Function* func, BasicBlock* preheader,
                          const NaturalLoop& loop) {
  if (!func || !func->parent() || !preheader || !preheader->terminator())
    return nullptr;

  Module* mod = func->parent();
  Instruction* preheaderTerm = preheader->terminator();
  std::unordered_map<int, Value*> cloneCache;

  Value* acc = mod->getConstInt(static_cast<int32_t>(constTerm));
  for (const auto& [term, coeff] : terms) {
    if (coeff == 0)
      continue;
    Value* cloned = cloneInvariantExpr(term, preheader, preheaderTerm, loop, cloneCache);
    if (!cloned)
      return nullptr;
    Value* scaled = cloned;
    if (coeff != 1 && coeff != -1) {
      int64_t absCoeff = coeff < 0 ? -coeff : coeff;
      auto* mul = new Instruction(Opcode::Mul, Type::i32(), "ar.base.mul");
      mul->addOperand(cloned);
      mul->addOperand(mod->getConstInt(static_cast<int32_t>(absCoeff)));
      preheader->insertBefore(preheaderTerm, mul);
      scaled = mul;
    }
    if (coeff > 0) {
      auto* add = new Instruction(Opcode::Add, Type::i32(), "ar.base.add");
      add->addOperand(acc);
      add->addOperand(scaled);
      preheader->insertBefore(preheaderTerm, add);
      acc = add;
    } else {
      auto* sub = new Instruction(Opcode::Sub, Type::i32(), "ar.base.sub");
      sub->addOperand(acc);
      sub->addOperand(scaled);
      preheader->insertBefore(preheaderTerm, sub);
      acc = sub;
    }
  }
  return acc;
}

Value* buildBaseOffset(const OffsetRecurrence& rec, Value* ivInit, Function* func,
                       BasicBlock* preheader, const NaturalLoop& loop) {
  if (!func || !func->parent() || !preheader || !preheader->terminator())
    return nullptr;

  Module* mod = func->parent();
  Instruction* preheaderTerm = preheader->terminator();
  Value* baseExpr = buildInvariantExpr(rec.constOffset, rec.invariantTerms, func,
                                       preheader, loop);
  Value* coeffExpr = buildInvariantExpr(rec.ivConstCoeff, rec.ivInvariantTerms, func,
                                        preheader, loop);
  if (!baseExpr || !coeffExpr)
    return nullptr;
  if (!ivInit)
    return baseExpr;

  Value* initValue = ivInit;
  if (auto* initInst = dynamic_cast<Instruction*>(ivInit)) {
    if (initInst->parent() && loop.blocks.count(initInst->parent())) {
      std::unordered_map<int, Value*> cloneCache;
      initValue = cloneInvariantExpr(ivInit, preheader, preheaderTerm, loop, cloneCache);
      if (!initValue)
        return nullptr;
    }
  }

  auto* initConst = dynamic_cast<ConstInt*>(initValue);
  auto* coeffConst = dynamic_cast<ConstInt*>(coeffExpr);
  Value* initContribution = nullptr;
  if (initConst && initConst->value() == 0) {
    initContribution = mod->getConstInt(0);
  } else if (initConst && initConst->value() == 1) {
    initContribution = coeffExpr;
  } else if (coeffConst && coeffConst->value() == 0) {
    initContribution = mod->getConstInt(0);
  } else if (coeffConst && coeffConst->value() == 1) {
    initContribution = initValue;
  } else {
    auto* mul = new Instruction(Opcode::Mul, Type::i32(), "ar.init.mul");
    mul->addOperand(initValue);
    mul->addOperand(coeffExpr);
    preheader->insertBefore(preheaderTerm, mul);
    initContribution = mul;
  }

  auto* baseConst = dynamic_cast<ConstInt*>(baseExpr);
  auto* initContributionConst = dynamic_cast<ConstInt*>(initContribution);
  if (baseConst && baseConst->value() == 0)
    return initContribution;
  if (initContributionConst && initContributionConst->value() == 0)
    return baseExpr;

  auto* add = new Instruction(Opcode::Add, Type::i32(), "ar.base.init");
  add->addOperand(baseExpr);
  add->addOperand(initContribution);
  preheader->insertBefore(preheaderTerm, add);
  return add;
}

Value* buildRecurrenceStep(const OffsetRecurrence& rec, int64_t ivStep,
                           Function* func, BasicBlock* preheader,
                           const NaturalLoop& loop) {
  int64_t constStep = rec.ivConstCoeff * ivStep;
  std::vector<std::pair<Value*, int64_t>> stepTerms = rec.ivInvariantTerms;
  for (auto& [term, coeff] : stepTerms)
    coeff *= ivStep;
  if (constStep == 0 && stepTerms.empty())
    return nullptr;
  return buildInvariantExpr(constStep, stepTerms, func, preheader, loop);
}

bool isRecurrenceMemUser(Instruction* user, Instruction* gep,
                         const NaturalLoop& loop) {
  if (!user || !user->parent() || !loop.blocks.count(user->parent()))
    return false;
  if (user->opcode() == Opcode::Load || user->opcode() == Opcode::VLoad)
    return user->numOperands() >= 1 && user->operand(0) == gep;
  if (user->opcode() == Opcode::Store || user->opcode() == Opcode::VStore)
    return user->numOperands() >= 2 && user->operand(1) == gep;
  return false;
}

bool loopHasCalls(const NaturalLoop& loop) {
  for (auto* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      if (instPtr->opcode() == Opcode::Call)
        return true;
    }
  }
  return false;
}

bool profitableSingleUseRecurrence(const NaturalLoop& loop,
                                   const OffsetRecurrence& rec,
                                   int64_t ivStep,
                                   size_t candidateCount,
                                   bool storeOnly) {
  const int64_t step = rec.ivConstCoeff * ivStep;
  const uint64_t absStep = step < 0 ? static_cast<uint64_t>(-step)
                                   : static_cast<uint64_t>(step);
  if (absStep == 0) {
    // 常量分量为零不代表步长为零：`iv * runtime_stride + invariant` 这类仿射
    // 地址会把步长记录在 ivInvariantTerms 中。在循环内重建地址需要一次乘法和
    // 一次加法，而递推形式只在前置块计算一次运行时字节步长，循环内只推进指针。
    //
    // 这里保持收益门槛收紧：紧凑且无调用的循环中只有一个不变缩放项时，才能用
    // 节省的地址计算抵消额外指针 Phi，避免递推值过多增加寄存器压力。
    return rec.ivInvariantTerms.size() == 1 &&
           candidateCount <= 2 && loop.blocks.size() <= 4 &&
           !loopHasCalls(loop);
  }
  // 单位步长的指针推进在紧凑循环中开销较低。较大的拆分循环中，如果单次使用的
  // 地址仍携带不变偏移，换成指针 phi 会延长活跃区间，分块或展开后通常更差。
  if (absStep == 4) {
    const bool invariantFree =
        rec.invariantTerms.empty() && rec.ivInvariantTerms.empty() &&
        rec.constOffset == 0;
    if (!invariantFree && !storeOnly)
      return loop.blocks.size() <= 4 && candidateCount <= 2;
    return storeOnly || candidateCount == 1 || invariantFree ||
           (loop.blocks.size() <= 4 && candidateCount <= 2);
  }
  // 较大的行步长反复重建代价较高，适合只物化一次。这里同时处理二的幂和非二
  // 的幂步长，覆盖常见的矩阵行跨度。
  return absStep >= 1024;
}

bool isLocallyDeadRecurrenceInst(Instruction* inst) {
  if (!inst || inst->hasSideEffects() || !inst->uses().empty())
    return false;
  switch (inst->opcode()) {
  case Opcode::Phi:
  case Opcode::Alloca:
  case Opcode::Br:
  case Opcode::CondBr:
  case Opcode::Ret:
  case Opcode::Call:
    return false;
  default:
    return true;
  }
}

void eraseDeadRecurrenceChain(Instruction* root) {
  std::vector<Instruction*> worklist;
  if (root)
    worklist.push_back(root);
  while (!worklist.empty()) {
    Instruction* inst = worklist.back();
    worklist.pop_back();
    if (!isLocallyDeadRecurrenceInst(inst) || !inst->parent())
      continue;

    std::vector<Instruction*> operandInsts;
    for (auto* op : inst->operands()) {
      auto* operandInst = dynamic_cast<Instruction*>(op);
      if (operandInst)
        operandInsts.push_back(operandInst);
    }
    BasicBlock* parent = inst->parent();
    parent->remove(inst);
    for (auto* operandInst : operandInsts) {
      if (isLocallyDeadRecurrenceInst(operandInst))
        worklist.push_back(operandInst);
    }
  }
}

std::vector<BasicBlock*> collectUniqueLatches(const NaturalLoop& loop) {
  std::vector<BasicBlock*> latches;
  std::unordered_set<int> seenIds;
  for (auto* latch : loop.latches) {
    if (!latch || !seenIds.insert(latch->id()).second)
      continue;
    latches.push_back(latch);
  }
  return latches;
}

bool applyToLoop(Function* func, const NaturalLoop& loop) {
  bool trace = sycEnvironmentValue("ARRAY_RECURRENCE_TRACE") != nullptr;
  if (!func || !loop.header || loop.blocks.empty())
    return false;
  if (trace) {
    std::fprintf(stderr, "[array_recurrence] inspect func=%s header=%s blocks=%zu latches=%zu\n",
                 func->name().c_str(), loop.header->name().c_str(),
                 loop.blocks.size(), loop.latches.size());
  }
  if (loop.blocks.size() > 64 || loop.latches.empty() || loopHasCalls(loop))
    return false;

  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader || !preheader->terminator())
    return false;
  auto latches = collectUniqueLatches(loop);
  if (latches.empty())
    return false;
  for (auto* latch : latches) {
    if (!latch || !latch->terminator())
      return false;
  }
  if (trace && latches.size() != loop.latches.size()) {
    std::fprintf(stderr, "  [array_recurrence] dedup latches %zu -> %zu\n",
                 loop.latches.size(), latches.size());
  }
  if (std::find(latches.begin(), latches.end(), preheader) != latches.end())
    return false;

  auto ivs = analysis::findInductionVars(func, loop);
  if (ivs.empty())
    return false;

  struct Candidate {
    Instruction* gep = nullptr;
    int useCount = 0;
    bool storeOnly = false;
    Value* ivInit = nullptr;
    int64_t ivStep = 0;
    OffsetRecurrence rec;
  };

  std::vector<Candidate> candidates;
  std::unordered_set<int> seenGepIds;

  for (const auto& iv : ivs) {
    if (!iv.isCanonical || iv.step == 0 || !iv.phi)
      continue;

    std::unordered_map<int, bool> depMemo;
    std::unordered_map<int, bool> invMemo;
    Instruction* canonicalIV = iv.phi;
    if (!canonicalIV)
      continue;

    for (auto* bb : loop.blocks) {
      for (auto& instPtr : bb->instructions()) {
        auto* inst = instPtr.get();
        if (!inst || inst->opcode() != Opcode::GEP || inst->numOperands() != 2)
          continue;
        if (isSyntheticRecurrenceGEP(inst))
          continue;
        if (seenGepIds.count(inst->id()))
          continue;
        if (auto* baseInst = dynamic_cast<Instruction*>(inst->operand(0))) {
          if (baseInst->opcode() == Opcode::GEP ||
              baseInst->opcode() == Opcode::Phi) {
            continue;
          }
        }

        // 尝试把 GEP 偏移拆解为归纳变量的线性递推；候选必须真正依赖归纳变量
        // （系数非零或含 iv 不变量项），且基地址为循环不变量。
        OffsetRecurrence rec;
        bool matched = collectOffsetRecurrence(inst->operand(1), loop, iv,
                                               depMemo, invMemo, 1, rec);
        if (trace) {
          auto* offsetInst = dynamic_cast<Instruction*>(inst->operand(1));
          const char* ivName = iv.phi ? iv.phi->name().c_str()
                                      : (iv.storage ? iv.storage->name().c_str() : "slot-iv");
          std::fprintf(stderr,
                       "    [scan] gep=%s id=%d iv=%s off=%s/%s matched=%d coeff=%d const=%lld inv=%zu\n",
                       inst->name().c_str(), inst->id(), ivName,
                       offsetInst ? opcodeName(offsetInst->opcode()) : "leaf",
                       offsetInst ? offsetInst->name().c_str() : inst->operand(1)->name().c_str(),
                       matched ? 1 : 0, static_cast<int>(rec.ivConstCoeff),
                       static_cast<long long>(rec.constOffset),
                       rec.invariantTerms.size());
          if (offsetInst && offsetInst->numOperands() == 2) {
            auto* lhsInst = dynamic_cast<Instruction*>(offsetInst->operand(0));
            auto* rhsInst = dynamic_cast<Instruction*>(offsetInst->operand(1));
            std::fprintf(stderr,
                         "      [off-op] lhs=%s/%s rhs=%s/%s\n",
                         lhsInst ? opcodeName(lhsInst->opcode()) : "leaf",
                         offsetInst->operand(0)->name().c_str(),
                         rhsInst ? opcodeName(rhsInst->opcode()) : "leaf",
                         offsetInst->operand(1)->name().c_str());
            auto dumpMul = [&](Instruction* mul, const char* tag) {
              if (!mul || mul->opcode() != Opcode::Mul || mul->numOperands() != 2)
                return;
              auto* aInst = dynamic_cast<Instruction*>(mul->operand(0));
              auto* bInst = dynamic_cast<Instruction*>(mul->operand(1));
              std::fprintf(stderr,
                           "        [%s-mul] a=%s/%s b=%s/%s\n",
                           tag,
                           aInst ? opcodeName(aInst->opcode()) : "leaf",
                           mul->operand(0)->name().c_str(),
                           bInst ? opcodeName(bInst->opcode()) : "leaf",
                           mul->operand(1)->name().c_str());
            };
            dumpMul(lhsInst, "lhs");
            dumpMul(rhsInst, "rhs");
          }
        }
        // 匹配成功仍需过滤：偏移须真正依赖归纳变量，基地址须为循环不变量。
        if (!matched) {
          continue;
        }
        if (rec.ivConstCoeff == 0 && rec.ivInvariantTerms.empty())
          continue;
        if (!isLoopInvariantValue(inst->operand(0), loop, canonicalIV, depMemo, invMemo))
          continue;

        int useCount = 0;
        bool sawLoad = false;
        bool sawStore = false;
        bool badUsers = false;
        for (auto* user : inst->uses()) {
          if (!user || !user->parent() || !loop.blocks.count(user->parent())) {
            badUsers = true;
            break;
          }
          if (!isRecurrenceMemUser(user, inst, loop)) {
            badUsers = true;
            break;
          }
          if (user->opcode() == Opcode::Load || user->opcode() == Opcode::VLoad)
            sawLoad = true;
          if (user->opcode() == Opcode::Store || user->opcode() == Opcode::VStore)
            sawStore = true;
          ++useCount;
        }
        if (badUsers || useCount == 0)
          continue;

        seenGepIds.insert(inst->id());
        candidates.push_back({inst, useCount, sawStore && !sawLoad,
                              iv.init, iv.step, rec});
      }
    }
  }

  if (candidates.empty())
    return false;

  const size_t candidateCount = candidates.size();
  candidates.erase(
      std::remove_if(candidates.begin(), candidates.end(),
                     [&](const Candidate& cand) {
                       return cand.useCount == 1 &&
                              !profitableSingleUseRecurrence(
                                  loop, cand.rec, cand.ivStep, candidateCount,
                                  cand.storeOnly);
                     }),
      candidates.end());

  if (candidates.empty())
    return false;

  std::sort(candidates.begin(), candidates.end(),
            [](const Candidate& a, const Candidate& b) {
              if (a.useCount != b.useCount)
                return a.useCount > b.useCount;
              bool aHasInvariantStep = !a.rec.ivInvariantTerms.empty();
              bool bHasInvariantStep = !b.rec.ivInvariantTerms.empty();
              if (aHasInvariantStep != bHasInvariantStep)
                return !aHasInvariantStep;
              int64_t aStep = a.rec.ivConstCoeff * a.ivStep;
              int64_t bStep = b.rec.ivConstCoeff * b.ivStep;
              if (std::abs(aStep) != std::abs(bStep))
                return std::abs(aStep) > std::abs(bStep);
              return a.gep->id() < b.gep->id();
            });

  // round7 暂降为 2。6 在 blocked loop 中会产生过多 pointer-phi，
  // 每个 phi 消耗一个寄存器，加上 tile loop 的 IV/bound phi，寄存器压力
  // 迫使热内层 IV 溢出到栈（深度矩阵类内层归纳变量 sd/ld 每迭代一次）。
  constexpr size_t kMaxRecurrencesPerLoop = 2;
  if (candidates.size() > kMaxRecurrencesPerLoop)
    candidates.resize(kMaxRecurrencesPerLoop);

  bool changed = false;
  if (trace) {
    std::fprintf(stderr, "[array_recurrence] func=%s header=%s blocks=%zu cand=%zu\n",
                 func->name().c_str(), loop.header->name().c_str(),
                 loop.blocks.size(), candidates.size());
  }
  for (const auto& cand : candidates) {
    if (trace) {
      std::fprintf(stderr,
                   "  [cand] gep=%s id=%d stepConst=%lld stepInv=%zu uses=%d const=%lld inv=%zu\n",
                   cand.gep->name().c_str(), cand.gep->id(),
                   static_cast<long long>(cand.rec.ivConstCoeff * cand.ivStep),
                   cand.rec.ivInvariantTerms.size(), cand.useCount,
                   static_cast<long long>(cand.rec.constOffset),
                   cand.rec.invariantTerms.size());
    }
    Value* baseOffset = buildBaseOffset(cand.rec, cand.ivInit, func,
                                        preheader, loop);
    if (!baseOffset)
      continue;
    Value* stepValue = buildRecurrenceStep(cand.rec, cand.ivStep, func,
                                          preheader, loop);
    if (!stepValue)
      continue;

    auto* basePtr = new Instruction(Opcode::GEP, Type::ptr(),
                                    cand.gep->name().empty()
                                        ? "ar.base"
                                        : cand.gep->name() + ".ar.base");
    basePtr->addOperand(cand.gep->operand(0));
    basePtr->addOperand(baseOffset);
    preheader->insertBefore(preheader->terminator(), basePtr);

    auto* ptrPhi = new Instruction(Opcode::Phi, Type::ptr(),
                                   cand.gep->name().empty()
                                       ? "ar.ptr"
                                       : cand.gep->name() + ".ar.ptr");
    ptrPhi->addOperand(basePtr);
    ptrPhi->addOperand(preheader);
    loop.header->pushFront(ptrPhi);
    for (auto* latch : latches) {
      ptrPhi->addOperand(basePtr);
      ptrPhi->addOperand(latch);
    }
    for (size_t idx = 0; idx < latches.size(); ++idx) {
      auto* latch = latches[idx];
      auto* nextPtr = new Instruction(Opcode::GEP, Type::ptr(),
                                      cand.gep->name().empty()
                                          ? "ar.next"
                                          : cand.gep->name() + ".ar.next");
      nextPtr->addOperand(ptrPhi);
      nextPtr->addOperand(stepValue);
      latch->insertBefore(latch->terminator(), nextPtr);
      ptrPhi->setOperand(static_cast<int>(2 + idx * 2), nextPtr);
    }

    std::vector<Instruction*> loopUsers = cand.gep->uses();
    for (auto* user : loopUsers) {
      if (!isRecurrenceMemUser(user, cand.gep, loop))
        continue;
      if ((user->opcode() == Opcode::Load || user->opcode() == Opcode::VLoad) &&
          user->operand(0) == cand.gep) {
        user->setOperand(0, ptrPhi);
      } else if ((user->opcode() == Opcode::Store ||
                  user->opcode() == Opcode::VStore) &&
                 user->operand(1) == cand.gep) {
        user->setOperand(1, ptrPhi);
      }
    }
    eraseDeadRecurrenceChain(cand.gep);

    changed = true;
  }

  return changed;
}

} // namespace

bool applyArrayAddressRecurrence(core_ir::Module& mod) {
  bool changed = false;
  for (auto& func : mod.functions()) {
    if (analysis::instructionCount(func.get()) > 50000)
      continue;

    func->recomputeCFG();
    auto reachable = analysis::collectReachableBlocks(func.get());
    std::vector<BasicBlock*> blocks;
    blocks.reserve(func->blocks().size());
    for (auto& bb : func->blocks()) {
      if (reachable.count(bb.get()))
        blocks.push_back(bb.get());
    }
    if (blocks.empty())
      continue;

    auto doms = analysis::computeDominators(func.get(), blocks, reachable);
    auto loops = analysis::collectNaturalLoops(blocks, doms, reachable);
    std::sort(loops.begin(), loops.end(),
              [](const NaturalLoop& a, const NaturalLoop& b) {
                return a.blocks.size() < b.blocks.size();
              });

    bool funcChanged = false;
    for (const auto& loop : loops)
      funcChanged |= applyToLoop(func.get(), loop);

    if (funcChanged) {
      func->recomputeCFG();
      changed = true;
    }
  }
  return changed;
}

} // namespace hir
