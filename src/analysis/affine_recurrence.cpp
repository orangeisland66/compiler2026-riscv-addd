#include "research_switch.h"
// 文件职责：分析 affine 地址递推族，帮助后续生成 pointer bump 和标量替换计划。本文件负责实现具体逻辑。
#include "affine_recurrence.h"
#include "loop_value_utils.h"

#include "../core_ir/ir/core_ir.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace analysis {

namespace {

using core_ir::Argument;
using core_ir::BasicBlock;
using core_ir::Function;
using core_ir::GlobalVar;
using core_ir::Instruction;
using core_ir::Opcode;
using core_ir::Value;

bool affineRecurrenceTraceEnabled() {
  return sycEnvironmentValue("AFFINE_RECURRENCE_TRACE") != nullptr;
}

const char* recurrenceRoleName(AffineRecurrenceRole role) {
  switch (role) {
  case AffineRecurrenceRole::Reduction:
    return "Reduction";
  case AffineRecurrenceRole::Stream:
    return "Stream";
  case AffineRecurrenceRole::UpdateStream:
    return "UpdateStream";
  case AffineRecurrenceRole::Broadcast:
    return "Broadcast";
  case AffineRecurrenceRole::UpdateSliceAnchor:
    return "UpdateSliceAnchor";
  case AffineRecurrenceRole::UpdateSliceCompanion:
    return "UpdateSliceCompanion";
  case AffineRecurrenceRole::Generic:
  default:
    return "Generic";
  }
}

void traceFamilyDecision(const char* stage, const AffineRecurrencePlan& family) {
  if (!affineRecurrenceTraceEnabled())
    return;
  std::fprintf(stderr,
               "[affine-recurrence] %s header=%s role=%s mem=%zu load=%d store=%d "
               "same=%d bump=%d scalar=%d companion=%d anchor=%d root=%s family=%s\n",
               stage,
               (family.loop && family.loop->header) ? family.loop->header->name().c_str()
                                                    : "<null>",
               recurrenceRoleName(family.role), family.memOps.size(),
               family.hasLoad ? 1 : 0, family.hasStore ? 1 : 0,
               family.sameAddressEveryIteration ? 1 : 0,
               family.pointerBumpCandidate ? 1 : 0,
               family.tileLocalScalarCandidate ? 1 : 0,
               family.updateSliceCompanion ? 1 : 0,
               family.updateSliceAnchor ? 1 : 0,
               family.access.root ? family.access.root->name().c_str() : "<null>",
               family.familyKey.c_str());
}

std::string invariantValueKey(Value* value,
                              std::unordered_map<int, std::string>& memo,
                              std::unordered_set<int>& active) {
  if (!value)
    return "null";
  auto memoIt = memo.find(value->id());
  if (memoIt != memo.end())
    return memoIt->second;
  if (!active.insert(value->id()).second)
    return "cycle:" + std::to_string(value->id());

  std::string key;
  if (auto* ci = dynamic_cast<core_ir::ConstInt*>(value)) {
    key = "ci:" + std::to_string(ci->value());
  } else if (auto* arg = dynamic_cast<Argument*>(value)) {
    key = "arg:" + arg->name();
  } else if (auto* gv = dynamic_cast<GlobalVar*>(value)) {
    key = "gv:" + gv->name();
  } else if (auto* bb = dynamic_cast<BasicBlock*>(value)) {
    key = "bb:" + bb->name();
  } else if (auto* inst = dynamic_cast<Instruction*>(value)) {
    if (inst->opcode() == Opcode::Alloca) {
      key = "alloca:" + std::to_string(inst->id());
      active.erase(value->id());
      memo[value->id()] = key;
      return key;
    }
    if (inst->opcode() == Opcode::Phi) {
      key = "phi:" + std::to_string(inst->id());
      active.erase(value->id());
      memo[value->id()] = key;
      return key;
    }
    // 递推族分组不应依赖临时 SSA id。这里对 loop-invariant
    // 简单表达式做结构归一化，让“语义相同但值编号不同”的地址族能并到一起。
    key = std::string("inst:") + core_ir::opcodeName(inst->opcode());
    key += ":" + std::to_string(static_cast<int>(inst->type().kind));
    key += ":" + std::to_string(inst->type().ptrDepth);
    if (inst->opcode() == Opcode::Load && inst->numOperands() == 1 &&
        isScalarSlot(inst->operand(0))) {
      key += ":slot(" + invariantValueKey(inst->operand(0), memo, active) + ")";
    } else if (!isSimpleInvariantExpr(inst, /*allowGEP=*/true)) {
      key += ":id(" + std::to_string(inst->id()) + ")";
    } else {
      for (auto* op : inst->operands())
        key += "(" + invariantValueKey(op, memo, active) + ")";
    }
  } else {
    key = "val:" + std::to_string(value->id());
  }

  active.erase(value->id());
  memo[value->id()] = key;
  return key;
}

bool isMemoryOp(Instruction* inst) {
  return inst && (inst->opcode() == Opcode::Load || inst->opcode() == Opcode::Store ||
                  inst->opcode() == Opcode::VLoad || inst->opcode() == Opcode::VStore);
}

bool isLoadOp(Instruction* inst) {
  return inst && (inst->opcode() == Opcode::Load || inst->opcode() == Opcode::VLoad);
}

bool isStoreOp(Instruction* inst) {
  return inst && (inst->opcode() == Opcode::Store || inst->opcode() == Opcode::VStore);
}

Value* memoryAddress(Instruction* inst) {
  if (!inst)
    return nullptr;
  if ((inst->opcode() == Opcode::Load || inst->opcode() == Opcode::VLoad) &&
      inst->numOperands() == 1)
    return inst->operand(0);
  if ((inst->opcode() == Opcode::Store || inst->opcode() == Opcode::VStore) &&
      inst->numOperands() == 2)
    return inst->operand(1);
  return nullptr;
}

bool valueMatchesCurrentIV(Value* val, Value* currentIV) {
  if (!val || !currentIV)
    return false;
  if (val == currentIV)
    return true;
  auto* load = dynamic_cast<Instruction*>(val);
  return load && load->opcode() == Opcode::Load && load->numOperands() == 1 &&
         load->operand(0) == currentIV;
}

bool valueDependsOnCurrentIV(Value* val, Value* currentIV,
                             std::unordered_map<int, bool>& memo) {
  if (!val || !currentIV)
    return false;
  if (valueMatchesCurrentIV(val, currentIV))
    return true;
  if (val->isConstant() || dynamic_cast<Argument*>(val) ||
      dynamic_cast<GlobalVar*>(val) || dynamic_cast<BasicBlock*>(val)) {
    return false;
  }

  auto it = memo.find(val->id());
  if (it != memo.end())
    return it->second;

  auto* inst = dynamic_cast<Instruction*>(val);
  if (!inst)
    return memo[val->id()] = false;

  memo[val->id()] = false;
  for (auto* op : inst->operands()) {
    if (valueDependsOnCurrentIV(op, currentIV, memo))
      return memo[val->id()] = true;
  }
  return false;
}

bool isLoopInvariantValue(Value* val, const NaturalLoop& loop, Value* currentIV,
                          std::unordered_map<int, bool>& depMemo,
                          std::unordered_map<int, bool>& invMemo) {
  if (!val)
    return false;
  if (val->isConstant() || dynamic_cast<Argument*>(val) ||
      dynamic_cast<GlobalVar*>(val) || dynamic_cast<BasicBlock*>(val)) {
    return true;
  }

  auto memoIt = invMemo.find(val->id());
  if (memoIt != invMemo.end())
    return memoIt->second;

  auto* inst = dynamic_cast<Instruction*>(val);
  if (!inst || !inst->parent())
    return invMemo[val->id()] = true;
  if (!loop.blocks.count(inst->parent()))
    return invMemo[val->id()] = true;
  if (inst->opcode() == Opcode::Load && inst->numOperands() == 1 &&
      isScalarSlot(inst->operand(0)) && !loopStoresToSlot(loop, inst->operand(0))) {
    return invMemo[val->id()] = true;
  }
  if (valueDependsOnCurrentIV(val, currentIV, depMemo) ||
      !isSimpleInvariantExpr(inst)) {
    return invMemo[val->id()] = false;
  }

  invMemo[val->id()] = false;
  for (auto* op : inst->operands()) {
    if (!isLoopInvariantValue(op, loop, currentIV, depMemo, invMemo))
      return false;
  }
  return invMemo[val->id()] = true;
}

bool currentIVTermsOnly(const AffineAccessSummary& access, Value* currentIV,
                        const NaturalLoop& loop) {
  if (!access.valid || !access.offset.valid || !currentIV)
    return false;
  std::unordered_map<int, bool> depMemo;
  std::unordered_map<int, bool> invMemo;
  for (const auto& term : access.offset.ivTerms) {
    if (term.basis == currentIV)
      continue;
    if (!term.basis || !term.basis->type().isI32() ||
        valueDependsOnCurrentIV(term.basis, currentIV, depMemo)) {
      return false;
    }
  }
  for (const auto& term : access.offset.scaledIvTerms) {
    if (term.basis == currentIV) {
      if (!isLoopInvariantValue(term.invariantScale, loop, currentIV,
                                depMemo, invMemo)) {
        return false;
      }
      continue;
    }
    if (!term.basis || !term.basis->type().isI32() ||
        valueDependsOnCurrentIV(term.basis, currentIV, depMemo) ||
        !isLoopInvariantValue(term.invariantScale, loop, currentIV,
                              depMemo, invMemo)) {
      return false;
    }
  }
  for (const auto& term : access.offset.invariantTerms) {
    if (!isLoopInvariantValue(term.basis, loop, currentIV, depMemo, invMemo))
      return false;
  }
  return true;
}

bool buildCurrentIVCoeffTerms(const AffineAccessSummary& access, Value* currentIV,
                              int64_t& constCoeff,
                              std::vector<std::pair<Value*, int64_t>>& scaledTerms) {
  constCoeff = 0;
  scaledTerms.clear();
  for (const auto& term : access.offset.ivTerms) {
    if (term.basis == currentIV)
      constCoeff += term.coeff;
  }
  for (const auto& term : access.offset.scaledIvTerms) {
    if (term.basis == currentIV)
      scaledTerms.push_back({term.invariantScale, term.coeff});
  }
  return constCoeff != 0 || !scaledTerms.empty();
}

int instructionIndex(BasicBlock* bb, Instruction* inst) {
  if (!bb || !inst)
    return -1;
  int idx = 0;
  for (auto& instPtr : bb->instructions()) {
    if (instPtr.get() == inst)
      return idx;
    ++idx;
  }
  return -1;
}

bool comesBeforeInBlock(BasicBlock* bb, Instruction* first, Instruction* second) {
  int firstIdx = instructionIndex(bb, first);
  int secondIdx = instructionIndex(bb, second);
  return firstIdx >= 0 && secondIdx >= 0 && firstIdx < secondIdx;
}

bool isValueComputedFrom(Value* root, Value* target,
                         std::unordered_map<int, bool>& memo) {
  if (!root || !target)
    return false;
  if (root == target)
    return true;
  auto it = memo.find(root->id());
  if (it != memo.end())
    return it->second;
  auto* inst = dynamic_cast<Instruction*>(root);
  if (!inst)
    return memo[root->id()] = false;
  memo[root->id()] = false;
  for (auto* op : inst->operands()) {
    if (isValueComputedFrom(op, target, memo))
      return memo[root->id()] = true;
  }
  return false;
}

bool sameCurrentIVProgression(const AffineRecurrencePlan& lhs,
                              const AffineRecurrencePlan& rhs) {
  // update-slice 只需要证明两个 family 共享同一条 current-IV
  // 进展线，并且数据流确实从纯 load 传到更新 store。这里不再把 rawCoeff
  // 或 scaled-term 系数当成硬性相等条件，否则大常量步长、行流/列流这类
  // 通用 affine 进展会被错误拆成不同族，无法进入共享 rewrite 主线。
  return lhs.currentIV == rhs.currentIV;
}

void updateFamilyProfit(AffineRecurrencePlan& family) {
  family.hasLoad = false;
  family.hasStore = false;
  for (auto* memOp : family.memOps) {
    if (!memOp)
      continue;
    family.hasLoad = family.hasLoad || isLoadOp(memOp);
    family.hasStore = family.hasStore || isStoreOp(memOp);
  }
  family.sameAddressEveryIteration =
      family.rawCoeff == 0 && family.scaledTerms.empty();
  family.unitStride = !family.sameAddressEveryIteration &&
                      family.scaledTerms.empty() &&
                      (family.rawCoeff == 1 || family.rawCoeff == -1);
  family.dynamicStride = !family.scaledTerms.empty();
  // 默认仍不改写孤立 load/store stream，因为收益容易被 live
  // range 和寄存器压力抵消。孤立单 stream 仍会参与 update-slice 配对，
  // 但不直接生成 pointer phi；真正直接改写的 pointer bump 先保留给
  // 同一 family 内有复用的访存族。
  family.pointerBumpCandidate =
      family.valid && !family.sameAddressEveryIteration &&
      !family.dynamicStride && family.rawCoeff != 0 &&
      family.memOps.size() >= 2;
  family.tileLocalScalarCandidate =
      family.memOps.size() >= 2 && family.hasLoad && family.hasStore &&
      family.sameAddressEveryIteration;
  family.updateSliceAnchor = false;
  // update-slice 不是“单个 load”的特例，而是纯 load stream
  // 与写回端形成的通用 view/copy 关系。后续会再用真实的 load->store
  // 数据流和 CFG 顺序收紧，因此这里先按纯 load family 计入候选。
  family.updateSliceCompanion = family.hasLoad && !family.hasStore;
  family.role = AffineRecurrenceRole::Generic;
  if (family.tileLocalScalarCandidate)
    family.role = AffineRecurrenceRole::Reduction;
  else if (family.sameAddressEveryIteration && family.hasLoad && !family.hasStore)
    family.role = AffineRecurrenceRole::Broadcast;
  else if (family.pointerBumpCandidate && family.hasLoad && family.hasStore)
    family.role = AffineRecurrenceRole::UpdateStream;
  else if (family.pointerBumpCandidate)
    family.role = AffineRecurrenceRole::Stream;
  family.estimatedRegisterPressure =
      1 + static_cast<int>(family.scaledTerms.size()) +
      (family.tileLocalScalarCandidate ? 1 : 0) +
      (family.dynamicStride ? 1 : 0);
  family.estimatedBenefit =
      static_cast<int64_t>(family.memOps.size()) * 64 +
      (family.unitStride ? 64 : 0) +
      (family.pointerBumpCandidate ? 48 : 0) +
      (family.updateSliceCompanion ? 24 : 0) +
      (family.tileLocalScalarCandidate ? 192 : 0) -
      family.strideCost * 4 -
      static_cast<int64_t>(family.estimatedRegisterPressure) * 12;
}

void markUpdateSliceCompanions(std::vector<AffineRecurrencePlan>& plans) {
  for (auto& family : plans) {
    family.updateSliceAnchor = false;
    family.updateSliceCompanion = false;
    family.updateSliceCompanionCount = 0;
    family.updateSliceAnchorKey.clear();
    if (family.tileLocalScalarCandidate) {
      family.role = AffineRecurrenceRole::Reduction;
    } else if (family.sameAddressEveryIteration && family.hasLoad &&
               !family.hasStore) {
      family.role = AffineRecurrenceRole::Broadcast;
    } else if (family.pointerBumpCandidate && family.hasLoad &&
               family.hasStore) {
      family.role = AffineRecurrenceRole::UpdateStream;
    } else if (family.pointerBumpCandidate) {
      family.role = AffineRecurrenceRole::Stream;
    } else {
      family.role = AffineRecurrenceRole::Generic;
    }
    traceFamilyDecision("base-role", family);
  }

  for (auto& dstFamily : plans) {
    const bool dstHasAffineProgress =
        !dstFamily.sameAddressEveryIteration && !dstFamily.dynamicStride &&
        dstFamily.rawCoeff != 0;
    if (!dstFamily.valid || !dstFamily.hasStore || !dstHasAffineProgress) {
      continue;
    }

    Instruction* dstStore = nullptr;
    for (auto* memOp : dstFamily.memOps) {
      if (!memOp)
        continue;
      if (!dstStore &&
          (memOp->opcode() == Opcode::Store || memOp->opcode() == Opcode::VStore)) {
        dstStore = memOp;
      }
    }
    if (!dstStore)
      continue;
    BasicBlock* bb = dstStore->parent();
    if (!bb)
      continue;
    if (dstStore->numOperands() < 1)
      continue;
    int companionCount = 0;
    for (auto& srcFamily : plans) {
      if (&srcFamily == &dstFamily || !srcFamily.valid || !srcFamily.hasLoad ||
          srcFamily.hasStore || srcFamily.sameAddressEveryIteration ||
          !sameCurrentIVProgression(srcFamily, dstFamily)) {
        continue;
      }
      for (auto* srcLoad : srcFamily.memOps) {
        if (!srcLoad || srcLoad->parent() != bb)
          continue;
        if (!comesBeforeInBlock(bb, srcLoad, dstStore))
          continue;
        std::unordered_map<int, bool> srcMemo;
        if (!isValueComputedFrom(dstStore->operand(0), srcLoad, srcMemo)) {
          continue;
        }
        // update-slice companion 的本质是“与更新流共享同一
        // current-IV progression，并且真实参与 store RHS 数据流”的纯
        // load stream。它不要求自己先拥有两条以上 memOp；即使当前 loop
        // 里每轮只出现一条 companion load，后续 recurrence / locality
        // 主线仍然可以基于这组共享 progression 把它和更新流绑定起来。
        ++companionCount;
        srcFamily.updateSliceCompanion = true;
        srcFamily.updateSliceAnchorKey = dstFamily.familyKey;
        srcFamily.role = AffineRecurrenceRole::UpdateSliceCompanion;
        traceFamilyDecision("mark-companion", srcFamily);
        break;
      }
    }
    if (companionCount > 0) {
      dstFamily.updateSliceAnchor = true;
      dstFamily.updateSliceCompanionCount = companionCount;
      dstFamily.role = AffineRecurrenceRole::UpdateSliceAnchor;
      traceFamilyDecision("mark-anchor", dstFamily);
    }
  }
}

std::string buildFamilyKey(const AffineAccessSummary& access, Value* currentIV,
                           int64_t rawCoeff,
                           const std::vector<std::pair<Value*, int64_t>>& scaledTerms) {
  std::unordered_map<int, std::string> structuralMemo;
  std::unordered_set<int> structuralActive;
  std::string key = invariantValueKey(access.root, structuralMemo, structuralActive);
  key += "|c=" + std::to_string(access.offset.constant);
  std::vector<std::pair<std::string, int64_t>> invariantTerms;
  for (const auto& term : access.offset.invariantTerms)
    invariantTerms.push_back(
        {invariantValueKey(term.basis, structuralMemo, structuralActive), term.coeff});
  for (const auto& term : access.offset.ivTerms) {
    if (term.basis == currentIV)
      continue;
    invariantTerms.push_back(
        {invariantValueKey(term.basis, structuralMemo, structuralActive), term.coeff});
  }
  std::sort(invariantTerms.begin(), invariantTerms.end());
  for (const auto& [exprKey, coeff] : invariantTerms)
    key += "|i" + exprKey + ":" + std::to_string(coeff);

  std::vector<std::tuple<std::string, std::string, int64_t>> invariantScaledTerms;
  for (const auto& term : access.offset.scaledIvTerms) {
    if (term.basis == currentIV)
      continue;
    invariantScaledTerms.push_back(
        {invariantValueKey(term.basis, structuralMemo, structuralActive),
         invariantValueKey(term.invariantScale, structuralMemo, structuralActive),
         term.coeff});
  }
  std::sort(invariantScaledTerms.begin(), invariantScaledTerms.end());
  for (const auto& [basisKey, scaleKey, coeff] : invariantScaledTerms) {
    key += "|is" + basisKey + ":" + scaleKey +
           ":" + std::to_string(coeff);
  }

  key += "|iv=" + std::to_string(currentIV ? currentIV->id() : -1);
  key += "|raw=" + std::to_string(rawCoeff);
  std::vector<std::pair<std::string, int64_t>> scaled;
  for (const auto& [scale, coeff] : scaledTerms)
    scaled.push_back({invariantValueKey(scale, structuralMemo, structuralActive), coeff});
  std::sort(scaled.begin(), scaled.end());
  for (const auto& [scaleKey, coeff] : scaled)
    key += "|s" + scaleKey + ":" + std::to_string(coeff);
  return key;
}

} // namespace

std::vector<AffineRecurrencePlan>
collectAffineRecurrencePlans(Function* func,
                             const NaturalLoop& loop,
                             const std::vector<InductionVar>& ivs) {
  std::vector<AffineRecurrencePlan> plans;
  if (!func || !loop.header || ivs.empty())
    return plans;

  std::vector<Instruction*> memOps;
  for (auto* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      if (isMemoryOp(instPtr.get()))
        memOps.push_back(instPtr.get());
    }
  }
  if (memOps.empty())
    return plans;

  for (const auto& iv : ivs) {
    if (!iv.isCanonical || !iv.init || iv.step == 0)
      continue;
    Value* currentIV = iv.phi ? static_cast<Value*>(iv.phi)
                              : static_cast<Value*>(iv.storage);
    if (!currentIV)
      continue;

    std::unordered_map<std::string, AffineRecurrencePlan> families;
    for (auto* memOp : memOps) {
      Value* addr = memoryAddress(memOp);
      if (!addr)
        continue;
      auto access = summarizeAffineAccess(addr, loop, ivs);
      std::unordered_map<int, bool> depMemo;
      std::unordered_map<int, bool> invMemo;
      if (!access.valid || !access.offset.valid ||
          !isLoopInvariantValue(access.root, loop, currentIV, depMemo, invMemo) ||
          !currentIVTermsOnly(access, currentIV, loop)) {
        continue;
      }
      int64_t rawCoeff = 0;
      std::vector<std::pair<Value*, int64_t>> scaledTerms;
      bool hasCurrentIVContribution =
          buildCurrentIVCoeffTerms(access, currentIV, rawCoeff, scaledTerms);
      bool sameAddressEveryIteration =
          !hasCurrentIVContribution && access.valid && access.offset.valid;
      if (!hasCurrentIVContribution && !sameAddressEveryIteration)
        continue;

      std::string key = buildFamilyKey(access, currentIV, rawCoeff, scaledTerms);
      auto& family = families[key];
      if (family.memOps.empty()) {
        family.valid = true;
        family.loop = &loop;
        family.induction = iv;
        family.currentIV = currentIV;
        family.access = access;
        family.rawCoeff = rawCoeff;
        family.scaledTerms = scaledTerms;
        family.strideCost = affineStrideCostForProfit(access, currentIV);
        family.familyKey = key;
      }
      family.memOps.push_back(memOp);
    }

    for (auto& [key, family] : families)
      updateFamilyProfit(family);
    for (auto& [key, family] : families)
      plans.push_back(std::move(family));
  }

  markUpdateSliceCompanions(plans);

  std::sort(plans.begin(), plans.end(),
            [](const AffineRecurrencePlan& lhs, const AffineRecurrencePlan& rhs) {
              if (lhs.role != rhs.role)
                return static_cast<int>(lhs.role) > static_cast<int>(rhs.role);
              if (lhs.tileLocalScalarCandidate != rhs.tileLocalScalarCandidate)
                return lhs.tileLocalScalarCandidate > rhs.tileLocalScalarCandidate;
              if (lhs.updateSliceAnchor != rhs.updateSliceAnchor)
                return lhs.updateSliceAnchor > rhs.updateSliceAnchor;
              if (lhs.updateSliceCompanion != rhs.updateSliceCompanion)
                return lhs.updateSliceCompanion > rhs.updateSliceCompanion;
              if (lhs.estimatedBenefit != rhs.estimatedBenefit)
                return lhs.estimatedBenefit > rhs.estimatedBenefit;
              if (lhs.memOps.size() != rhs.memOps.size())
                return lhs.memOps.size() > rhs.memOps.size();
              return lhs.familyKey < rhs.familyKey;
            });
  return plans;
}

} // namespace analysis
