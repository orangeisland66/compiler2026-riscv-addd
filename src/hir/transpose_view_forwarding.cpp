#include "research_switch.h"
// 文件职责：转发 affine copy/view 的只读 consumer，避免不必要的物化数组。本文件负责实现具体逻辑。
#include "transpose_view_forwarding.h"

#include "../analysis/analysis_manager.h"
#include "../analysis/affine_access.h"
#include "../analysis/affine_copy_region.h"
#include "../analysis/cfg_utils.h"
#include "../analysis/dominators.h"
#include "../analysis/induction_var.h"
#include "../analysis/loop_info.h"

#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace hir {

namespace {

using analysis::AffineCopyRegionSummary;
using analysis::AffineExprSummary;
using analysis::AffineTerm;
using analysis::InductionVar;
using analysis::NaturalLoop;
using core_ir::BasicBlock;
using core_ir::Function;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Type;
using core_ir::Value;

struct CopyForwardPlan {
  bool valid = false;
  bool allowBypass = false;
  const NaturalLoop* producerLoop = nullptr;
  BasicBlock* producerExit = nullptr;
  Value* dstRoot = nullptr;
  Value* srcRoot = nullptr;
  bool negated = false;
  int64_t dstConstant = 0;
  int64_t srcConstant = 0;
  std::unordered_map<int64_t, int64_t> coeffMap;
  std::vector<AffineTerm> deltaInvariantTerms;
  int producerBlockCount = 0;
  int producerStoreCount = 0;
  int64_t producerBenefit = 0;
};

struct CollectiveBypassSummary {
  bool enabled = false;
  int matchedLoads = 0;
  int64_t currentStrideSum = 0;
  int64_t forwardedStrideSum = 0;
  int64_t producerBenefit = 0;
};

bool isLocallyDeadInst(Instruction* inst) {
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

void eraseDeadChain(Instruction* root);

bool collectDeadPrivateAllocaClosure(
    Value* value,
    Instruction* rootAlloca,
    std::unordered_set<int>& visitedValues,
    std::unordered_set<int>& recordedStores,
    std::unordered_set<int>& recordedAddrNodes,
    std::vector<Instruction*>& stores,
    std::vector<Instruction*>& addrRemoveOrder) {
  auto* inst = dynamic_cast<Instruction*>(value);
  if (inst && !visitedValues.insert(inst->id()).second)
    return true;

  for (auto* user : value->uses()) {
    if (!user || !user->parent())
      return false;
    switch (user->opcode()) {
    case Opcode::Load:
    case Opcode::VLoad:
      // 只要还有真实 load，这个私有 alloca 就不是死槽位。
      if (user->numOperands() != 1 || user->operand(0) != value)
        return false;
      return false;
    case Opcode::Store:
    case Opcode::VStore:
      // 地址只能作为“被写入的目标”，不能作为被存储的值逃逸。
      if (user->numOperands() != 2 || user->operand(1) != value)
        return false;
      if (recordedStores.insert(user->id()).second)
        stores.push_back(user);
      continue;
    case Opcode::GEP:
    case Opcode::BitCast:
    case Opcode::Mov:
      if (user->numOperands() < 1 || user->operand(0) != value)
        return false;
      if (!collectDeadPrivateAllocaClosure(user, rootAlloca, visitedValues,
                                           recordedStores, recordedAddrNodes,
                                           stores, addrRemoveOrder)) {
        return false;
      }
      if (user != rootAlloca && recordedAddrNodes.insert(user->id()).second)
        addrRemoveOrder.push_back(user);
      continue;
    default:
      return false;
    }
  }
  return true;
}

bool pruneDeadPrivateAlloca(Function* func, Instruction* alloca) {
  if (!func || !alloca || alloca->opcode() != Opcode::Alloca)
    return false;

  std::unordered_set<int> visitedValues;
  std::unordered_set<int> recordedStores;
  std::unordered_set<int> recordedAddrNodes;
  std::vector<Instruction*> stores;
  std::vector<Instruction*> addrRemoveOrder;
  if (!collectDeadPrivateAllocaClosure(alloca, alloca, visitedValues,
                                       recordedStores, recordedAddrNodes,
                                       stores, addrRemoveOrder)) {
    return false;
  }
  if (stores.empty())
    return false;

  for (auto* store : stores) {
    if (!store || !store->parent())
      continue;
    std::vector<Instruction*> operands;
    for (auto* op : store->operands()) {
      auto* operandInst = dynamic_cast<Instruction*>(op);
      if (operandInst)
        operands.push_back(operandInst);
    }
    BasicBlock* parent = store->parent();
    parent->remove(store);
    for (auto* operandInst : operands)
      eraseDeadChain(operandInst);
  }
  for (auto it = addrRemoveOrder.rbegin(); it != addrRemoveOrder.rend(); ++it)
    eraseDeadChain(*it);

  if (!alloca->uses().empty() || !alloca->parent())
    return false;
  alloca->parent()->parent()->removeAlloca(alloca);
  alloca->parent()->remove(alloca);
  return true;
}

bool pruneDeadPrivateAllocas(Function* func) {
  if (!func)
    return false;
  bool changed = false;
  std::vector<Instruction*> allocas(func->allocas().begin(), func->allocas().end());
  for (auto* alloca : allocas)
    changed |= pruneDeadPrivateAlloca(func, alloca);
  return changed;
}

void eraseDeadChain(Instruction* root) {
  std::vector<Instruction*> worklist;
  if (root)
    worklist.push_back(root);
  while (!worklist.empty()) {
    Instruction* inst = worklist.back();
    worklist.pop_back();
    if (!isLocallyDeadInst(inst) || !inst->parent())
      continue;

    std::vector<Instruction*> operands;
    for (auto* op : inst->operands()) {
      auto* operandInst = dynamic_cast<Instruction*>(op);
      if (operandInst)
        operands.push_back(operandInst);
    }
    BasicBlock* parent = inst->parent();
    parent->remove(inst);
    for (auto* operandInst : operands) {
      if (isLocallyDeadInst(operandInst))
        worklist.push_back(operandInst);
    }
  }
}

Instruction* makeInstruction(Opcode op, Type ty, const std::string& name = "") {
  return new Instruction(op, ty, name);
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

Instruction* insertGEPBefore(BasicBlock* bb, Instruction* pos, Value* base,
                             Value* offset, const std::string& name = "") {
  auto* inst = makeInstruction(Opcode::GEP, Type::ptr(), name);
  inst->addOperand(base);
  inst->addOperand(offset);
  bb->insertBefore(pos, inst);
  return inst;
}

bool isI32Value(Value* value) {
  return value && value->type().isI32();
}

Value* constI32(Module& mod, int64_t value) {
  return mod.getConstInt(static_cast<int32_t>(value));
}

bool loopContainsLoop(const NaturalLoop& outer, const NaturalLoop& inner);

Value* scaleByConst(Module& mod, BasicBlock* bb, Instruction* pos,
                    Value* value, int64_t coeff,
                    const std::string& stem) {
  if (!value || !isI32Value(value) || coeff == 0)
    return nullptr;
  if (coeff == 1)
    return value;
  if (coeff == -1)
    return insertUnaryBefore(bb, pos, Opcode::Neg, Type::i32(), value,
                             stem + ".neg");

  int64_t absCoeff = coeff < 0 ? -coeff : coeff;
  Value* factor = constI32(mod, absCoeff);
  Value* mul = insertBinaryBefore(bb, pos, Opcode::Mul, Type::i32(), value,
                                  factor, stem + ".mul");
  if (coeff < 0)
    return insertUnaryBefore(bb, pos, Opcode::Neg, Type::i32(), mul,
                             stem + ".neg");
  return mul;
}

Value* addIfNeeded(BasicBlock* bb, Instruction* pos,
                   Value* acc, Value* term,
                   const std::string& stem) {
  if (!term)
    return acc;
  if (!acc)
    return term;
  return insertBinaryBefore(bb, pos, Opcode::Add, Type::i32(), acc, term, stem);
}

bool affineExprSimpleForView(const AffineExprSummary& expr) {
  return expr.valid && expr.scaledIvTerms.empty();
}

bool isPrivateAllocaViewRoot(Value* value) {
  auto* inst = dynamic_cast<Instruction*>(value);
  return inst && inst->opcode() == Opcode::Alloca;
}

bool isLargeMaterializedViewRoot(Value* value) {
  if (!value)
    return false;
  if (dynamic_cast<core_ir::GlobalVar*>(value) ||
      dynamic_cast<core_ir::Argument*>(value)) {
    return true;
  }
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->opcode() != Opcode::Alloca)
    return false;
  // transpose/view forwarding 面向“先物化整块 view，再只读消费”的
  // 大型数组。小型私有 scratch（如桶、摘要轮常量、小临时 buffer）更适合
  // SROA/标量提升；在这些对象上删除 producer loop 很容易误删仍有语义的
  // 初始化或重排步骤。因此本 pass 只处理足够大的物化数组。
  return inst->isArrayObject() && inst->memorySizeBytes() >= 4096;
}

Value* pointerCarrierRoot(Instruction* slot, std::unordered_set<int>& visiting);

// 沿操作数链回溯指针的"根基址"：GlobalVar/Argument 自身即根；
// GEP/BitCast/Mov 透传基址操作数，Load 转查槽位的指针载体根；
// phi 要求所有入边收敛到同一根，否则返回 nullptr 保守放弃。
Value* pointerBaseRoot(Value* value, std::unordered_set<int>& visiting) {
  if (!value || !value->type().isPtr())
    return nullptr;
  if (!visiting.insert(value->id()).second)
    return nullptr;
  if (dynamic_cast<core_ir::GlobalVar*>(value) ||
      dynamic_cast<core_ir::Argument*>(value)) {
    visiting.erase(value->id());
    return value;
  }
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst) {
    visiting.erase(value->id());
    return value;
  }

  Value* root = nullptr;
  switch (inst->opcode()) {
  case Opcode::Alloca:
    root = inst;
    break;
  case Opcode::GEP:
  case Opcode::BitCast:
  case Opcode::Mov:
    root = pointerBaseRoot(inst->operand(0), visiting);
    break;
  case Opcode::Load: {
    auto* slot = dynamic_cast<Instruction*>(inst->operand(0));
    if (slot)
      root = pointerCarrierRoot(slot, visiting);
    break;
  }
  case Opcode::Phi: {
    bool sawConcreteRoot = false;
    for (int i = 0; i + 1 < inst->numOperands(); i += 2) {
      Value* incomingRoot = pointerBaseRoot(inst->operand(i), visiting);
      if (!incomingRoot)
        continue;
      if (!root)
        root = incomingRoot;
      else if (root != incomingRoot) {
        root = nullptr;
        break;
      }
      sawConcreteRoot = true;
    }
    if (!sawConcreteRoot)
      root = nullptr;
    break;
  }
  default:
    root = nullptr;
    break;
  }
  visiting.erase(value->id());
  return root;
}

Value* pointerCarrierRoot(Instruction* slot, std::unordered_set<int>& visiting) {
  if (!slot || slot->opcode() != Opcode::Alloca || slot->isArrayObject())
    return nullptr;
  if (!slot->memoryElemTy().isPtr() && slot->memorySizeBytes() != 8)
    return nullptr;
  if (!visiting.insert(slot->id()).second)
    return nullptr;

  Value* root = nullptr;
  bool sawStore = false;
  bool sawConcreteRoot = false;
  for (auto* user : slot->uses()) {
    if (!user)
      continue;
    if (user->opcode() == Opcode::Load && user->numOperands() == 1 &&
        user->operand(0) == slot) {
      continue;
    }
    if (user->opcode() != Opcode::Store || user->numOperands() != 2 ||
        user->operand(1) != slot) {
      return nullptr;
    }
    Value* storedRoot = pointerBaseRoot(user->operand(0), visiting);
    if (!storedRoot)
      continue;
    if (!root)
      root = storedRoot;
    else if (root != storedRoot)
      return visiting.erase(slot->id()), nullptr;
    sawStore = true;
    sawConcreteRoot = true;
  }
  visiting.erase(slot->id());
  return (sawStore && sawConcreteRoot) ? root : nullptr;
}

Value* ivBasisFor(const InductionVar& iv) {
  if (iv.phi)
    return iv.phi;
  return iv.storage;
}

bool traceEnabled() {
  return sycEnvironmentValue("TRANSPOSE_VIEW_TRACE") != nullptr;
}

BasicBlock* producerExitBlock(const NaturalLoop& loop) {
  if (!loop.header)
    return nullptr;
  auto* term = loop.header->terminator();
  if (!term || term->opcode() != Opcode::CondBr || term->numOperands() != 3)
    return nullptr;
  auto* succA = dynamic_cast<BasicBlock*>(term->operand(1));
  auto* succB = dynamic_cast<BasicBlock*>(term->operand(2));
  if (!succA || !succB)
    return nullptr;
  if (loop.blocks.count(succA) && !loop.blocks.count(succB))
    return succB;
  if (loop.blocks.count(succB) && !loop.blocks.count(succA))
    return succA;
  return nullptr;
}

bool buildProducerPlan(const NaturalLoop& loop,
                       const std::vector<InductionVar>& ivs,
                       const AffineCopyRegionSummary& copy,
                       CopyForwardPlan& out) {
  if (!copy.valid || !copy.dstRoot || !copy.srcRoot)
    return false;
  if (copy.dstRoot == copy.srcRoot)
    return false;
  if (!isLargeMaterializedViewRoot(copy.dstRoot) ||
      !isLargeMaterializedViewRoot(copy.srcRoot)) {
    return false;
  }
  if (!affineExprSimpleForView(copy.dstAccess.offset) ||
      !affineExprSimpleForView(copy.srcAccess.offset)) {
    return false;
  }

  std::unordered_map<int64_t, int64_t> coeffMap;
  std::vector<AffineTerm> deltaInvariantTerms;
  int mapped = 0;
  auto addMapping = [&](int64_t dstCoeff, int64_t srcCoeff) -> bool {
    if (dstCoeff == 0 || srcCoeff == 0)
      return true;
    auto it = coeffMap.find(dstCoeff);
    if (it != coeffMap.end())
      return it->second == srcCoeff;
    coeffMap[dstCoeff] = srcCoeff;
    ++mapped;
    return true;
  };
  auto mergeDeltaInvariant = [&](Value* basis, int64_t coeff) {
    if (!basis || coeff == 0)
      return;
    for (auto& term : deltaInvariantTerms) {
      if (term.basis == basis) {
        term.coeff += coeff;
        return;
      }
    }
    deltaInvariantTerms.push_back({basis, coeff});
  };

  for (const auto& iv : ivs) {
    Value* basis = ivBasisFor(iv);
    if (!basis)
      continue;
    if (!addMapping(analysis::affineWeightedCoeffFor(copy.dstAccess, basis),
                    analysis::affineWeightedCoeffFor(copy.srcAccess, basis))) {
      return false;
    }
  }
  for (const auto& term : copy.srcAccess.offset.invariantTerms)
    mergeDeltaInvariant(term.basis, term.coeff);
  for (const auto& term : copy.dstAccess.offset.invariantTerms)
    mergeDeltaInvariant(term.basis, -term.coeff);
  deltaInvariantTerms.erase(
      std::remove_if(deltaInvariantTerms.begin(), deltaInvariantTerms.end(),
                     [](const AffineTerm& term) { return term.coeff == 0; }),
      deltaInvariantTerms.end());
  if (mapped < 1)
    return false;

  BasicBlock* exit = producerExitBlock(loop);
  if (!exit)
    return false;

  if (traceEnabled()) {
    std::cerr << "[transpose-view] plan dstRoot="
              << (copy.dstRoot ? copy.dstRoot->id() : -1)
              << " srcRoot=" << (copy.srcRoot ? copy.srcRoot->id() : -1)
              << " mapped=" << mapped << "\n";
  }
  out.valid = true;
  // 删除 producer loop 只有在目标 view 是函数内私有 alloca 时才
  // 安全；若 dstRoot 是参数或全局，写入语义会逃逸到当前函数外部，不能因为
  // 本函数里暂时看不到后续 load 就把整段物化 copy 删掉。
  out.allowBypass = isPrivateAllocaViewRoot(copy.dstRoot);
  out.producerLoop = &loop;
  out.producerExit = exit;
  out.dstRoot = copy.dstRoot;
  out.srcRoot = copy.srcRoot;
  out.negated = copy.negated;
  out.dstConstant = copy.dstAccess.offset.constant;
  out.srcConstant = copy.srcAccess.offset.constant;
  out.coeffMap = std::move(coeffMap);
  out.deltaInvariantTerms = std::move(deltaInvariantTerms);
  return true;
}

bool copyMatchesPlanRoots(const AffineCopyRegionSummary& copy,
                          const CopyForwardPlan& plan) {
  return copy.valid && copy.dstRoot == plan.dstRoot &&
         copy.srcRoot == plan.srcRoot && copy.negated == plan.negated;
}

std::vector<InductionVar> collectNestedInductionVars(
    analysis::FunctionAnalysisCache& cache,
    const std::vector<NaturalLoop>& loops,
    const NaturalLoop& region) {
  std::vector<InductionVar> result;
  std::unordered_set<int> seenBases;
  auto appendIvs = [&](const NaturalLoop& loop) {
    const auto& ivs = cache.inductionVars(loop);
    for (const auto& iv : ivs) {
      Value* basis = ivBasisFor(iv);
      if (!basis || !seenBases.insert(basis->id()).second)
        continue;
      result.push_back(iv);
    }
  };

  // copy/view 的 producer 一旦扩展到外层循环，就必须把整条
  // loop nest 的 IV 都纳入 affine 映射。否则只能看到内层下标，无法证明
  // `dst[i][j] = src[j][i]` 这种二维 view 的双向系数关系。
  appendIvs(region);
  for (const auto& loop : loops) {
    if (&loop == &region)
      continue;
    if (loopContainsLoop(region, loop))
      appendIvs(loop);
  }
  return result;
}

bool rebuildProducerPlanForLoop(Function* func,
                                analysis::FunctionAnalysisCache& cache,
                                const std::vector<NaturalLoop>& loops,
                                const NaturalLoop& producer,
                                const CopyForwardPlan& oldPlan,
                                CopyForwardPlan& rebuilt) {
  auto ivs = collectNestedInductionVars(cache, loops, producer);
  if (ivs.empty())
    return false;
  auto copies = analysis::collectAffineCopyRegions(func, producer, ivs);
  for (const auto& copy : copies) {
    if (!copyMatchesPlanRoots(copy, oldPlan))
      continue;
    CopyForwardPlan candidate;
    if (!buildProducerPlan(producer, ivs, copy, candidate))
      continue;
    rebuilt = std::move(candidate);
    return true;
  }
  return false;
}

bool buildForwardedOffset(Module& mod, BasicBlock* bb, Instruction* pos,
                          const analysis::AffineAccessSummary& access,
                          const CopyForwardPlan& plan,
                          Value*& outOffset) {
  if (!access.valid || !affineExprSimpleForView(access.offset))
    return false;

  std::unordered_map<int, int64_t> merged;
  int64_t constant = access.offset.constant - plan.dstConstant + plan.srcConstant;
  for (const auto& term : access.offset.ivTerms) {
    auto it = plan.coeffMap.find(term.coeff);
    if (it == plan.coeffMap.end() || !term.basis || !isI32Value(term.basis))
      return false;
    merged[term.basis->id()] += it->second;
  }
  for (const auto& term : access.offset.invariantTerms) {
    auto it = plan.coeffMap.find(term.coeff);
    if (it == plan.coeffMap.end() || !term.basis || !isI32Value(term.basis))
      return false;
    merged[term.basis->id()] += it->second;
  }

  Value* acc = constant != 0 ? constI32(mod, constant) : nullptr;
  int termIndex = 0;
  auto emitTerms = [&](const std::vector<analysis::AffineTerm>& terms) {
    for (const auto& term : terms) {
      auto mit = merged.find(term.basis->id());
      if (mit == merged.end())
        continue;
      int64_t coeff = mit->second;
      merged.erase(mit);
      if (coeff == 0)
        continue;
      Value* scaled = scaleByConst(mod, bb, pos, term.basis, coeff,
                                   "transpose.view.term" +
                                       std::to_string(termIndex++));
      acc = addIfNeeded(bb, pos, acc, scaled, "transpose.view.add");
    }
  };
  emitTerms(access.offset.ivTerms);
  emitTerms(access.offset.invariantTerms);
  if (!merged.empty())
    return false;
  int deltaIndex = 0;
  for (const auto& term : plan.deltaInvariantTerms) {
    if (!term.basis || !isI32Value(term.basis))
      return false;
    Value* scaled = scaleByConst(mod, bb, pos, term.basis, term.coeff,
                                 "transpose.view.delta" +
                                     std::to_string(deltaIndex++));
    acc = addIfNeeded(bb, pos, acc, scaled, "transpose.view.add");
  }
  if (!acc)
    acc = constI32(mod, 0);
  outOffset = acc;
  return true;
}

int64_t loopStrideMagnitude(const analysis::AffineAccessSummary& access,
                            const std::vector<InductionVar>& ivs) {
  if (!access.valid)
    return 0;
  int64_t best = 0;
  for (const auto& iv : ivs) {
    Value* basis = ivBasisFor(iv);
    if (!basis)
      continue;
    int64_t coeff = analysis::affineWeightedCoeffFor(access, basis);
    if (coeff == 0)
      continue;
    int64_t absCoeff = coeff < 0 ? -coeff : coeff;
    if (best == 0 || absCoeff < best)
      best = absCoeff;
  }
  return best;
}

bool forwardedStrideMagnitude(const analysis::AffineAccessSummary& access,
                              const std::vector<InductionVar>& ivs,
                              const CopyForwardPlan& plan,
                              int64_t& outStride);

bool analyzeLoadStrideProfile(const NaturalLoop& loop,
                              const std::vector<InductionVar>& ivs,
                              const CopyForwardPlan& plan,
                              Instruction* load,
                              int64_t& currentStride,
                              int64_t& forwardedStride) {
  if (!load || load->opcode() != Opcode::Load || load->numOperands() != 1)
    return false;
  auto access = analysis::summarizeAffineAccess(load->operand(0), loop, ivs);
  if (!access.valid || access.root != plan.dstRoot)
    return false;
  currentStride = loopStrideMagnitude(access, ivs);
  forwardedStride = 0;
  if (currentStride == 0)
    return false;
  if (!forwardedStrideMagnitude(access, ivs, plan, forwardedStride))
    return false;
  return forwardedStride != 0;
}

// 用 CopyForwardPlan 的 coeffMap 映射 load 仿射偏移的系数，
// 得到转发后地址随 IV 变化的步长幅度；取各 IV basis 上的最小绝对值，
// 任一项无映射即判定无法转发。
bool forwardedStrideMagnitude(const analysis::AffineAccessSummary& access,
                              const std::vector<InductionVar>& ivs,
                              const CopyForwardPlan& plan,
                              int64_t& outStride) {
  if (!access.valid)
    return false;
  int64_t best = 0;
  for (const auto& iv : ivs) {
    Value* basis = ivBasisFor(iv);
    if (!basis)
      continue;
    int64_t mappedCoeff = 0;
    bool sawBasis = false;
    for (const auto& term : access.offset.ivTerms) {
      if (term.basis != basis)
        continue;
      auto it = plan.coeffMap.find(term.coeff);
      if (it == plan.coeffMap.end())
        return false;
      mappedCoeff += it->second;
      sawBasis = true;
    }
    for (const auto& term : access.offset.invariantTerms) {
      if (term.basis != basis)
        continue;
      auto it = plan.coeffMap.find(term.coeff);
      if (it == plan.coeffMap.end())
        return false;
      mappedCoeff += it->second;
      sawBasis = true;
    }
    if (!sawBasis || mappedCoeff == 0)
      continue;
    int64_t absCoeff = mappedCoeff < 0 ? -mappedCoeff : mappedCoeff;
    if (best == 0 || absCoeff < best)
      best = absCoeff;
  }
  outStride = best;
  return true;
}

bool instructionClobbersPlan(const CopyForwardPlan& plan, Instruction* inst) {
  if (!inst || inst->isTerminator())
    return false;
  if (inst->opcode() == Opcode::Call)
    return true;
  if (!inst->hasSideEffects())
    return false;
  if (inst->opcode() != Opcode::Store)
    return true;
  std::unordered_set<int> visiting;
  Value* root = pointerBaseRoot(inst->operand(1), visiting);
  if (!root)
    return true;
  return root == plan.dstRoot || root == plan.srcRoot;
}

bool hasInterveningClobber(const CopyForwardPlan& plan, Instruction* load,
                           const analysis::DomSet& doms) {
  if (!plan.producerLoop || !plan.producerExit || !load || !load->parent())
    return true;
  BasicBlock* loadBlock = load->parent();
  if (!analysis::dominates(doms, plan.producerExit, loadBlock)) {
    if (traceEnabled()) {
      std::cerr << "[transpose-view] reject-load dominance load=" << load->id()
                << " exit=" << plan.producerExit->name() << " block="
                << loadBlock->name() << "\n";
    }
    return true;
  }

  std::unordered_set<BasicBlock*> canReachLoad;
  std::vector<BasicBlock*> reverseWorklist{loadBlock};
  while (!reverseWorklist.empty()) {
    BasicBlock* bb = reverseWorklist.back();
    reverseWorklist.pop_back();
    if (!bb || !canReachLoad.insert(bb).second)
      continue;
    for (auto* pred : bb->preds()) {
      if (!pred)
        continue;
      if (plan.producerLoop->blocks.count(pred))
        continue;
      reverseWorklist.push_back(pred);
    }
  }
  if (!canReachLoad.count(plan.producerExit)) {
    if (traceEnabled()) {
      std::cerr << "[transpose-view] reject-load no-path load=" << load->id()
                << " exit=" << plan.producerExit->name() << " block="
                << loadBlock->name() << "\n";
    }
    return true;
  }

  std::vector<BasicBlock*> forwardWorklist{plan.producerExit};
  std::unordered_set<BasicBlock*> visited;
  while (!forwardWorklist.empty()) {
    BasicBlock* bb = forwardWorklist.back();
    forwardWorklist.pop_back();
    if (!bb || !visited.insert(bb).second)
      continue;

    for (auto& instPtr : bb->instructions()) {
      auto* inst = instPtr.get();
      if (!inst || inst == load)
        break;
      if (instructionClobbersPlan(plan, inst)) {
        if (traceEnabled()) {
          std::cerr << "[transpose-view] clobber blocks load=" << load->id()
                    << " producerExit=" << plan.producerExit->name()
                    << " clobberInst=" << inst->id() << " block="
                    << bb->name() << "\n";
        }
        return true;
      }
    }

    if (bb == loadBlock)
      continue;
    for (auto* succ : bb->succs()) {
      if (!succ || !canReachLoad.count(succ))
        continue;
      forwardWorklist.push_back(succ);
    }
  }
  return false;
}

CollectiveBypassSummary summarizeCollectiveBypass(
    analysis::FunctionAnalysisCache& cache,
    const analysis::DomSet& doms,
    const std::vector<NaturalLoop>& loops,
    const CopyForwardPlan& plan) {
  CollectiveBypassSummary summary;
  if (!plan.valid || !plan.allowBypass || plan.producerBenefit < 64) {
    return summary;
  }

  for (const auto& loop : loops) {
    if (plan.producerLoop == &loop)
      continue;
    const auto& ivs = cache.inductionVars(loop);
    if (ivs.empty())
      continue;
    for (auto* bb : loop.blocks) {
      if (!bb)
        continue;
      for (auto& instPtr : bb->instructions()) {
        auto* load = instPtr.get();
        if (!load || load->opcode() != Opcode::Load)
          continue;
        int64_t currentStride = 0;
        int64_t forwardedStride = 0;
        if (!analyzeLoadStrideProfile(loop, ivs, plan, load,
                                      currentStride, forwardedStride)) {
          continue;
        }
        if (hasInterveningClobber(plan, load, doms))
          continue;
        ++summary.matchedLoads;
        summary.currentStrideSum += currentStride;
        summary.forwardedStrideSum += forwardedStride;
      }
    }
  }

  // 整张 affine view 的删除收益需要按整体判断，而不是让每条
  // consumer load 各自做短视决策。不过“总 stride 持平”在热内层里经常
  // 只是把一个 unit-stride load 换成另一个大步长 load；producer 删除收益
  // 通常不足以抵消。因此 collective bypass 仍要看到一段明确的净改善，
  // 只是门槛不再死卡在 2x，避免把中等收益的通用 view forwarding 错杀掉。
  summary.producerBenefit = plan.producerBenefit;
  summary.enabled =
      summary.matchedLoads >= 2 &&
      summary.forwardedStrideSum +
              static_cast<int64_t>(summary.matchedLoads) * 64 <=
          summary.currentStrideSum &&
      summary.producerBenefit >= 64;
  if (traceEnabled()) {
    std::cerr << "[transpose-view] collective matched="
              << summary.matchedLoads << " currentSum="
              << summary.currentStrideSum << " forwardedSum="
              << summary.forwardedStrideSum << " producerBenefit="
              << summary.producerBenefit << " enabled="
              << (summary.enabled ? 1 : 0) << "\n";
  }
  return summary;
}

// 将命中转发计划的 load 改写为从 srcRoot 读取的转发 load。
// forwardedStride 大于/等于 currentStride 时默认无收益而放弃，
// 仅在私有 alloca bypass 或 collective 汇总收益达标时例外放行。
bool rewriteLoad(Module& mod, const NaturalLoop& loop,
                 const analysis::DomSet& doms,
                 const std::vector<InductionVar>& ivs,
                 const CopyForwardPlan& plan, Instruction* load,
                 const CollectiveBypassSummary* collectiveBypass) {
  if (!load || load->opcode() != Opcode::Load || load->numOperands() != 1)
    return false;
  auto access = analysis::summarizeAffineAccess(load->operand(0), loop, ivs);
  if (!access.valid || access.root != plan.dstRoot)
    return false;
  int64_t currentStride = loopStrideMagnitude(access, ivs);
  int64_t forwardedStride = 0;
  const bool privateAllocaRoot = isPrivateAllocaViewRoot(plan.dstRoot);
  if (currentStride == 0 ||
      !forwardedStrideMagnitude(access, ivs, plan, forwardedStride)) {
    if (traceEnabled()) {
      std::cerr << "[transpose-view] skip-unprofitable load=" << load->id()
                << " currentStride=" << currentStride
                << " forwardedStride=" << forwardedStride << "\n";
    }
    return false;
  }
  const bool equalStridePrivateBypass =
      privateAllocaRoot && plan.allowBypass &&
      forwardedStride != 0 && forwardedStride == currentStride &&
      plan.producerBenefit >= 64;
  const bool collectiveBypassAllowed =
      collectiveBypass && collectiveBypass->enabled &&
      plan.allowBypass;
  if (forwardedStride != 0 && forwardedStride > currentStride) {
    if (collectiveBypassAllowed) {
      if (traceEnabled()) {
        std::cerr << "[transpose-view] allow-collective-bypass load="
                  << load->id() << " currentStride=" << currentStride
                  << " forwardedStride=" << forwardedStride
                  << " totalCurrent=" << collectiveBypass->currentStrideSum
                  << " totalForwarded=" << collectiveBypass->forwardedStrideSum
                  << "\n";
      }
    } else {
      if (traceEnabled()) {
        std::cerr << "[transpose-view] skip-unprofitable load=" << load->id()
                  << " currentStride=" << currentStride
                  << " forwardedStride=" << forwardedStride << "\n";
      }
      return false;
    }
  }
  if (forwardedStride != 0 && forwardedStride == currentStride &&
      !equalStridePrivateBypass && !collectiveBypassAllowed) {
    if (traceEnabled()) {
      std::cerr << "[transpose-view] skip-unprofitable load=" << load->id()
                << " currentStride=" << currentStride
                << " forwardedStride=" << forwardedStride << "\n";
    }
    return false;
  }
  if (hasInterveningClobber(plan, load, doms))
    return false;

  Value* offset = nullptr;
  if (!buildForwardedOffset(mod, load->parent(), load, access, plan, offset)) {
    if (traceEnabled()) {
      std::cerr << "[transpose-view] root-match but offset-build failed load="
                << load->id() << " root="
                << (access.root ? access.root->id() : -1) << "\n";
    }
    return false;
  }

  auto* gep = insertGEPBefore(load->parent(), load, plan.srcRoot, offset,
                              "transpose.view.gep");
  auto* forwardedLoad = makeInstruction(Opcode::Load, load->type(),
                                        "transpose.view.load");
  forwardedLoad->addOperand(gep);
  load->parent()->insertBefore(load, forwardedLoad);
  Value* result = forwardedLoad;
  if (plan.negated) {
    result = insertUnaryBefore(load->parent(), load, Opcode::Neg, Type::i32(),
                               forwardedLoad, "transpose.view.neg");
  }
  if (traceEnabled()) {
    std::cerr << "[transpose-view] rewrote load=" << load->id()
              << " dstRoot=" << (plan.dstRoot ? plan.dstRoot->id() : -1)
              << " srcRoot=" << (plan.srcRoot ? plan.srcRoot->id() : -1) << "\n";
  }
  std::vector<Instruction*> oldOperands;
  for (auto* op : load->operands()) {
    auto* inst = dynamic_cast<Instruction*>(op);
    if (inst)
      oldOperands.push_back(inst);
  }
  load->replaceAllUsesWith(result);
  load->parent()->remove(load);
  for (auto* operandInst : oldOperands)
    eraseDeadChain(operandInst);
  return true;
}

Value* zeroIterValue(Value* value, BasicBlock* preheader,
                     const NaturalLoop& loop) {
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !inst->parent() || !loop.blocks.count(inst->parent()))
    return value;
  if (inst->opcode() != Opcode::Phi || inst->parent() != loop.header)
    return nullptr;
  for (int i = 0; i + 1 < inst->numOperands(); i += 2) {
    auto* pred = dynamic_cast<BasicBlock*>(inst->operand(i + 1));
    if (pred == preheader)
      return inst->operand(i);
  }
  return nullptr;
}

// 校验循环体的副作用只有写向 plan.dstRoot 的 Store（且至少一条），
// 含 Call 或指向其它根的写操作时拒绝，保证转发改写不破坏其它内存语义。
bool loopSideEffectsAreCopyOnly(Function* func,
                                analysis::FunctionAnalysisCache& cache,
                                const NaturalLoop& loop,
                                const CopyForwardPlan& plan) {
  const auto& ivs = cache.inductionVars(loop);
  if (ivs.empty()) {
    if (traceEnabled())
      std::cerr << "[transpose-view] reject-copy-only no-ivs header="
                << loop.header->name() << "\n";
    return false;
  }
  bool sawMatchingStore = false;
  for (auto* bb : loop.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      auto* inst = instPtr.get();
      if (!inst || inst->isTerminator())
        continue;
      if (!inst->hasSideEffects())
        continue;
      if (inst->opcode() != Opcode::Store) {
        if (traceEnabled()) {
          std::cerr << "[transpose-view] reject-copy-only side-effect inst="
                    << inst->id() << " op=" << core_ir::opcodeName(inst->opcode())
                    << " header=" << loop.header->name() << "\n";
        }
        return false;
      }
      std::unordered_set<int> visiting;
      Value* root = pointerBaseRoot(inst->operand(1), visiting);
      if (root != plan.dstRoot) {
        if (traceEnabled()) {
          std::cerr << "[transpose-view] reject-copy-only store-root inst="
                    << inst->id() << " addr=" << inst->operand(1)->id()
                    << " root=" << (root ? root->id() : -1)
                    << " expect=" << (plan.dstRoot ? plan.dstRoot->id() : -1)
                    << " header=" << loop.header->name() << "\n";
        }
        return false;
      }
      sawMatchingStore = true;
    }
  }
  if (!sawMatchingStore && traceEnabled()) {
    std::cerr << "[transpose-view] reject-copy-only no-store header="
              << loop.header->name() << "\n";
  }
  return sawMatchingStore;
}

int countMatchingProducerStores(const NaturalLoop& loop,
                                const CopyForwardPlan& plan) {
  int count = 0;
  for (auto* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      auto* inst = instPtr.get();
      if (!inst || inst->opcode() != Opcode::Store || inst->numOperands() != 2)
        continue;
      std::unordered_set<int> visiting;
      Value* root = pointerBaseRoot(inst->operand(1), visiting);
      if (root == plan.dstRoot)
        ++count;
    }
  }
  return count;
}

void refreshProducerBenefit(CopyForwardPlan& plan) {
  if (!plan.producerLoop) {
    plan.producerBlockCount = 0;
    plan.producerStoreCount = 0;
    plan.producerBenefit = 0;
    return;
  }
  plan.producerBlockCount = static_cast<int>(plan.producerLoop->blocks.size());
  plan.producerStoreCount = countMatchingProducerStores(*plan.producerLoop, plan);
  // producer 删除收益只来自通用结构：可删 loop 的块数和
  // copy store 数量。
  plan.producerBenefit =
      static_cast<int64_t>(plan.producerBlockCount) * 16 +
      static_cast<int64_t>(plan.producerStoreCount) * 32;
}

bool loopContainsLoop(const NaturalLoop& outer, const NaturalLoop& inner) {
  if (!outer.header || !inner.header)
    return false;
  if (outer.blocks.size() <= inner.blocks.size())
    return false;
  for (auto* bb : inner.blocks) {
    if (!outer.blocks.count(bb))
      return false;
  }
  return true;
}

void widenProducerRegion(Function* func,
                         analysis::FunctionAnalysisCache& cache,
                         const std::vector<NaturalLoop>& loops,
                         CopyForwardPlan& plan) {
  if (!func || !plan.valid || !plan.producerLoop)
    return;

  const NaturalLoop* chosen = plan.producerLoop;
  CopyForwardPlan chosenPlan = plan;
  bool expanded = true;
  while (expanded) {
    expanded = false;
    for (const auto& candidate : loops) {
      if (&candidate == chosen)
        continue;
      if (!loopContainsLoop(candidate, *chosen))
        continue;
      BasicBlock* exit = producerExitBlock(candidate);
      if (!exit)
        continue;
      if (!loopSideEffectsAreCopyOnly(func, cache, candidate, plan))
        continue;
      CopyForwardPlan rebuilt;
      if (!rebuildProducerPlanForLoop(func, cache, loops, candidate, chosenPlan,
                                      rebuilt)) {
        continue;
      }
      chosen = &candidate;
      chosenPlan = std::move(rebuilt);
      expanded = true;
      break;
    }
  }

  plan = std::move(chosenPlan);
  refreshProducerBenefit(plan);
}

bool useEscapesRegion(Instruction* inst, std::unordered_set<int>& visiting) {
  if (!inst)
    return false;
  if (!visiting.insert(inst->id()).second)
    return false;

  // producer 删除只需要拦住“仍有真实语义”的残余 use。
  // 纯地址传递链（GEP/phi/bitcast/mov）以及只流向纯计算、最终无副作用落点的
  // 指令链，都可以视为后续 DCE 会收掉的死链，不能把它们误判成 escape。
  if (!inst->hasSideEffects() && inst->uses().empty()) {
    if (isLocallyDeadInst(inst))
      eraseDeadChain(inst);
    visiting.erase(inst->id());
    return false;
  }
  if (inst->hasSideEffects()) {
    visiting.erase(inst->id());
    return true;
  }
  for (auto* user : inst->uses()) {
    if (!user)
      continue;
    if (isLocallyDeadInst(user)) {
      eraseDeadChain(user);
      continue;
    }
    if (useEscapesRegion(user, visiting)) {
      visiting.erase(inst->id());
      return true;
    }
  }
  visiting.erase(inst->id());
  return false;
}

bool hasLiveExternalDstUse(const CopyForwardPlan& plan, BasicBlock* preheader) {
  bool pruned = true;
  while (pruned) {
    pruned = false;
    std::vector<Instruction*> users = plan.dstRoot->uses();
    for (auto* user : users) {
      if (!user)
        continue;
      if (isLocallyDeadInst(user)) {
        eraseDeadChain(user);
        pruned = true;
      }
    }
  }

  for (auto* user : plan.dstRoot->uses()) {
    if (!user || !user->parent())
      continue;
    if (plan.producerLoop && plan.producerLoop->blocks.count(user->parent()))
      continue;
    if (preheader && user->parent() == preheader)
      continue;
    if (isLocallyDeadInst(user)) {
      eraseDeadChain(user);
      continue;
    }
    std::unordered_set<int> visiting;
    if (useEscapesRegion(user, visiting)) {
      if (traceEnabled()) {
        std::cerr << "[transpose-view] live-external-use inst=" << user->id()
                  << " block="
                  << (user->parent() ? user->parent()->name() : "<none>")
                  << "\n";
      }
      return true;
    }
  }
  return false;
}

bool addressChainHasExternalLoad(Value* value,
                                 const CopyForwardPlan& plan,
                                 BasicBlock* preheader,
                                 std::unordered_set<int>& visiting) {
  if (!value || !visiting.insert(value->id()).second)
    return false;
  for (auto* user : value->uses()) {
    if (!user || !user->parent())
      continue;
    if (plan.producerLoop && plan.producerLoop->blocks.count(user->parent()))
      continue;
    if (preheader && user->parent() == preheader)
      continue;
    switch (user->opcode()) {
    case Opcode::Load:
    case Opcode::VLoad:
      visiting.erase(value->id());
      return true;
    case Opcode::GEP:
    case Opcode::BitCast:
    case Opcode::Mov:
    case Opcode::Phi:
      if (addressChainHasExternalLoad(user, plan, preheader, visiting)) {
        visiting.erase(value->id());
        return true;
      }
      break;
    default:
      break;
    }
  }
  visiting.erase(value->id());
  return false;
}

bool hasUnforwardedExternalDstLoad(const CopyForwardPlan& plan,
                                   BasicBlock* preheader) {
  // 删除 producer loop 之前，必须确认 dst view 的真实读者都已经
  // 被转发到 src。普通 escape 检查会沿纯计算链追到副作用，但 producer
  // 删除更严格：只要还有一条外部 load 从 dst 地址链读取，就不能删写入者。
  std::unordered_set<int> visiting;
  return addressChainHasExternalLoad(plan.dstRoot, plan, preheader, visiting);
}

bool bypassProducerLoop(Function* func,
                        analysis::FunctionAnalysisCache& cache,
                        const CopyForwardPlan& plan) {
  if (!func || !plan.valid || !plan.producerLoop || !plan.producerLoop->header)
    return false;
  const NaturalLoop& loop = *plan.producerLoop;
  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader) {
    if (traceEnabled())
      std::cerr << "[transpose-view] reject-bypass no-preheader header="
                << loop.header->name() << "\n";
    return false;
  }
  auto* headerTerm = loop.header->terminator();
  if (!headerTerm || headerTerm->opcode() != Opcode::CondBr ||
      headerTerm->numOperands() != 3) {
    if (traceEnabled())
      std::cerr << "[transpose-view] reject-bypass non-canonical-header="
                << loop.header->name() << "\n";
    return false;
  }
  BasicBlock* exit = plan.producerExit ? plan.producerExit : producerExitBlock(loop);
  if (!exit) {
    if (traceEnabled())
      std::cerr << "[transpose-view] reject-bypass no-single-exit header="
                << loop.header->name() << "\n";
    return false;
  }

  if (!loopSideEffectsAreCopyOnly(func, cache, loop, plan))
    return false;
  if (hasUnforwardedExternalDstLoad(plan, preheader))
    return false;
  if (hasLiveExternalDstUse(plan, preheader))
    return false;

  for (auto* phi : analysis::collectLeadingPhi(exit)) {
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
      if (pred != loop.header)
        continue;
      Value* mapped = zeroIterValue(phi->operand(i), preheader, loop);
      if (!mapped) {
        if (traceEnabled()) {
          std::cerr << "[transpose-view] reject-bypass exit-phi value="
                    << phi->operand(i)->id() << " exit=" << exit->name()
                    << " header=" << loop.header->name() << "\n";
        }
        return false;
      }
      phi->setOperand(i, mapped);
      phi->setOperand(i + 1, preheader);
    }
  }

  if (!analysis::redirectSuccessor(preheader, loop.header, exit)) {
    if (traceEnabled())
      std::cerr << "[transpose-view] reject-bypass redirect-failed header="
                << loop.header->name() << "\n";
    return false;
  }
  if (traceEnabled()) {
    std::cerr << "[transpose-view] bypassed producer header="
              << loop.header->name() << " dstRoot="
              << (plan.dstRoot ? plan.dstRoot->id() : -1) << "\n";
  }
  func->recomputeCFG();
  return true;
}

bool processFunction(Module& mod,
                     Function* func,
                     analysis::AnalysisManager& analysisManager) {
  if (!func || !func->entryBlock())
    return false;

  func->recomputeCFG();
  analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
  if (cache.blocks().empty())
    return false;

  const auto& doms = cache.dominators();
  auto loops = cache.loops();
  std::sort(loops.begin(), loops.end(),
            [](const NaturalLoop& lhs, const NaturalLoop& rhs) {
              if (lhs.blocks.size() != rhs.blocks.size())
                return lhs.blocks.size() < rhs.blocks.size();
              return lhs.header->id() < rhs.header->id();
            });

  std::vector<CopyForwardPlan> plans;
  for (const auto& loop : loops) {
    const auto& ivs = cache.inductionVars(loop);
    if (ivs.empty())
      continue;
    auto copies = analysis::collectAffineCopyRegions(func, loop, ivs);
    for (const auto& copy : copies) {
      CopyForwardPlan plan;
      if (buildProducerPlan(loop, ivs, copy, plan))
        plans.push_back(std::move(plan));
    }
  }
  if (plans.empty())
    return false;
  for (auto& plan : plans)
    widenProducerRegion(func, cache, loops, plan);
  std::sort(plans.begin(), plans.end(),
            [](const CopyForwardPlan& lhs, const CopyForwardPlan& rhs) {
              if (lhs.producerBenefit != rhs.producerBenefit)
                return lhs.producerBenefit > rhs.producerBenefit;
              if (lhs.producerBlockCount != rhs.producerBlockCount)
                return lhs.producerBlockCount > rhs.producerBlockCount;
              return lhs.producerStoreCount > rhs.producerStoreCount;
            });
  std::vector<CollectiveBypassSummary> collectiveBypasses;
  collectiveBypasses.reserve(plans.size());
  for (const auto& plan : plans) {
    // collective bypass summary 与具体 plan 一一对应。
    // 这里必须在排序后的 plans 顺序上重新建立 summary，避免多 plan
    // 场景下“收益画像”和“实际转发计划”错位，误伤通用矩阵核的
    // producer-bypass 决策。
    collectiveBypasses.push_back(
        summarizeCollectiveBypass(cache, doms, loops, plan));
  }
  if (traceEnabled()) {
    std::cerr << "[transpose-view] function=" << func->name()
              << " plans=" << plans.size() << "\n";
  }

  bool changed = false;
  for (const auto& loop : loops) {
    const auto& ivs = cache.inductionVars(loop);
    if (ivs.empty())
      continue;
    for (auto* bb : loop.blocks) {
      if (!bb)
        continue;
      std::vector<Instruction*> loads;
      for (auto& instPtr : bb->instructions()) {
        auto* inst = instPtr.get();
        if (inst && inst->opcode() == Opcode::Load)
          loads.push_back(inst);
      }
      for (auto* load : loads) {
        for (size_t planIdx = 0; planIdx < plans.size(); ++planIdx) {
          const auto& plan = plans[planIdx];
          if (plan.producerLoop == &loop)
            continue;
          if (rewriteLoad(mod, loop, doms, ivs, plan, load,
                          &collectiveBypasses[planIdx])) {
            changed = true;
            break;
          }
        }
      }
    }
  }

  for (const auto& plan : plans) {
    if (!plan.allowBypass)
      continue;
    changed |= bypassProducerLoop(func, cache, plan);
  }

  if (changed)
    changed |= pruneDeadPrivateAllocas(func);

  if (changed)
    func->recomputeCFG();
  if (changed) {
    // view forwarding 会删除 load/GEP，producer bypass 还会改 CFG；
    // 后续 loop band 和 affine recurrence 不能继续复用旧分析。
    analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
  }
  return changed;
}

} // namespace

bool applyTransposeViewForwarding(Module& mod) {
  analysis::AnalysisManager analysisManager(mod);
  return applyTransposeViewForwarding(mod, analysisManager);
}

bool applyTransposeViewForwarding(Module& mod,
                                  analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions())
    changed |= processFunction(mod, funcPtr.get(), analysisManager);
  return changed;
}

} // namespace hir
