#include "research_switch.h"
// 文件职责：把 affine 地址和重复标量访问改写成递推、SSA 累加和局部替换。本文件负责实现具体逻辑。
#include "affine_scalar_rewrite.h"

#include "../analysis/analysis_manager.h"
#include "../analysis/affine_access.h"
#include "../analysis/affine_region.h"
#include "../analysis/affine_recurrence.h"
#include "../analysis/cfg_utils.h"
#include "../analysis/dominators.h"
#include "../analysis/induction_var.h"
#include "../analysis/loop_info.h"
#include "../analysis/loop_value_utils.h"

#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace hir {

namespace {

using analysis::AffineAccessSummary;
using analysis::AffineRegionPlan;
using analysis::AffineScaledTerm;
using analysis::AffineTerm;
using analysis::InductionVar;
using analysis::NaturalLoop;
using core_ir::Argument;
using core_ir::BasicBlock;
using core_ir::ConstInt;
using core_ir::Function;
using core_ir::GlobalVar;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Type;
using core_ir::Value;

struct ScalarRewriteRegionApproval {
  bool loopReady = true;
  bool pointerBumpReady = true;
  bool tileLocalScalarReady = true;
  int64_t priorityDelta = 0;
};

bool arrayUpdateTraceEnabled() {
  return sycEnvironmentValue("AFFINE_ARRAY_ACC_TRACE") != nullptr;
}

const char* recurrenceRoleName(analysis::AffineRecurrenceRole role) {
  switch (role) {
  case analysis::AffineRecurrenceRole::Reduction:
    return "Reduction";
  case analysis::AffineRecurrenceRole::Stream:
    return "Stream";
  case analysis::AffineRecurrenceRole::UpdateStream:
    return "UpdateStream";
  case analysis::AffineRecurrenceRole::UpdateSliceAnchor:
    return "UpdateSliceAnchor";
  case analysis::AffineRecurrenceRole::Broadcast:
    return "Broadcast";
  case analysis::AffineRecurrenceRole::UpdateSliceCompanion:
    return "UpdateSliceCompanion";
  case analysis::AffineRecurrenceRole::Generic:
  default:
    return "Generic";
  }
}

std::string traceValueName(Value* value) {
  if (!value)
    return "<null>";
  if (!Value::isLiveValue(value))
    return "<deleted>";
  return value->name();
}

void traceArrayUpdateDecision(const char* verdict, Function* func,
                              const analysis::AffineRecurrencePlan& plan,
                              const char* reason) {
  if (!arrayUpdateTraceEnabled())
    return;
  const std::string functionName = func ? func->name() : "<null>";
  const std::string headerName =
      plan.loop ? traceValueName(plan.loop->header) : "<null>";
  const std::string rootName = traceValueName(plan.access.root);
  std::fprintf(stderr,
               "[affine-array-acc] %s func=%s header=%s role=%s mem=%zu root=%s "
               "same=%d scalar=%d reason=%s family=%s\n",
               verdict, functionName.c_str(), headerName.c_str(),
               recurrenceRoleName(plan.role), plan.memOps.size(),
               rootName.c_str(),
               plan.sameAddressEveryIteration ? 1 : 0,
               plan.tileLocalScalarCandidate ? 1 : 0, reason ? reason : "-",
               plan.familyKey.c_str());
}

struct ArrayUpdateTraceSnapshot {
  std::string function;
  std::string header;
  std::string role;
  std::string root;
  std::string family;
  std::size_t memoryOperations = 0;
  bool sameAddress = false;
  bool scalarCandidate = false;
};

ArrayUpdateTraceSnapshot snapshotArrayUpdateTrace(
    Function* func, const analysis::AffineRecurrencePlan& plan) {
  ArrayUpdateTraceSnapshot snapshot;
  snapshot.function = func ? func->name() : "<null>";
  snapshot.header = plan.loop ? traceValueName(plan.loop->header) : "<null>";
  snapshot.role = recurrenceRoleName(plan.role);
  snapshot.root = traceValueName(plan.access.root);
  snapshot.family = plan.familyKey;
  snapshot.memoryOperations = plan.memOps.size();
  snapshot.sameAddress = plan.sameAddressEveryIteration;
  snapshot.scalarCandidate = plan.tileLocalScalarCandidate;
  return snapshot;
}

void traceArrayUpdateDecision(const char* verdict,
                              const ArrayUpdateTraceSnapshot& snapshot,
                              const char* reason) {
  if (!arrayUpdateTraceEnabled())
    return;
  std::fprintf(stderr,
               "[affine-array-acc] %s func=%s header=%s role=%s mem=%zu root=%s "
               "same=%d scalar=%d reason=%s family=%s\n",
               verdict, snapshot.function.c_str(), snapshot.header.c_str(),
               snapshot.role.c_str(), snapshot.memoryOperations,
               snapshot.root.c_str(), snapshot.sameAddress ? 1 : 0,
               snapshot.scalarCandidate ? 1 : 0, reason ? reason : "-",
               snapshot.family.c_str());
}

bool scalarRewriteTraceEnabled() {
  return sycEnvironmentValue("AFFINE_SCALAR_REWRITE_TRACE") != nullptr;
}

void traceScalarRewriteDecision(const char* verdict, Function* func,
                                const analysis::AffineRecurrencePlan& plan,
                                const char* reason) {
  if (!scalarRewriteTraceEnabled())
    return;
  const std::string functionName = func ? func->name() : "<null>";
  const std::string headerName =
      plan.loop ? traceValueName(plan.loop->header) : "<null>";
  std::fprintf(stderr,
               "[affine-scalar-rewrite] %s func=%s header=%s role=%s mem=%zu "
               "bump=%d scalar=%d anchor=%d companion=%d reason=%s family=%s\n",
               verdict, functionName.c_str(), headerName.c_str(),
               recurrenceRoleName(plan.role), plan.memOps.size(),
               plan.pointerBumpCandidate ? 1 : 0,
               plan.tileLocalScalarCandidate ? 1 : 0,
               plan.updateSliceAnchor ? 1 : 0,
               plan.updateSliceCompanion ? 1 : 0,
               reason ? reason : "-", plan.familyKey.c_str());
}

int64_t recurrenceRolePriority(analysis::AffineRecurrenceRole role) {
  switch (role) {
  case analysis::AffineRecurrenceRole::Reduction:
    return 512;
  case analysis::AffineRecurrenceRole::UpdateStream:
    return 416;
  case analysis::AffineRecurrenceRole::Stream:
    return 320;
  case analysis::AffineRecurrenceRole::UpdateSliceAnchor:
    return 192;
  case analysis::AffineRecurrenceRole::Broadcast:
    return 96;
  case analysis::AffineRecurrenceRole::UpdateSliceCompanion:
    return 32;
  case analysis::AffineRecurrenceRole::Generic:
  default:
    return 0;
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
  if (!inst) {
    memo[val->id()] = false;
    return false;
  }

  memo[val->id()] = false;
  bool depends = false;
  for (auto* op : inst->operands()) {
    if (valueDependsOnCurrentIV(op, currentIV, memo)) {
      depends = true;
      break;
    }
  }
  memo[val->id()] = depends;
  return depends;
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
  if (valueDependsOnCurrentIV(val, currentIV, depMemo) ||
      !analysis::isSimpleInvariantExpr(inst)) {
    invMemo[val->id()] = false;
    return false;
  }

  invMemo[val->id()] = false;
  for (auto* op : inst->operands()) {
    if (!isLoopInvariantValue(op, loop, currentIV, depMemo, invMemo))
      return false;
  }
  invMemo[val->id()] = true;
  return true;
}

bool isI32Value(Value* val) {
  return val && val->type().isI32();
}

bool isSmallLocalScratchRoot(Value* val) {
  auto* inst = dynamic_cast<Instruction*>(val);
  if (!inst || inst->opcode() != Opcode::Alloca)
    return false;
  // 小型局部数组通常是常量表、桶、临时消息块等 scratch。
  // HIR pointer bump 会拉长指针 live range；这类对象更适合由 SROA、
  // mem2reg 和后端寻址清理处理。大型 dense-affine 数组不受此限制。
  return inst->isArrayObject() && inst->memorySizeBytes() > 0 &&
         inst->memorySizeBytes() <= 1024;
}

Value* constI32(Module& mod, int64_t value) {
  return mod.getConstInt(static_cast<int32_t>(value));
}

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

Value* addIfNeeded(Module& mod, BasicBlock* bb, Instruction* pos,
                   Value* acc, Value* term, const std::string& stem) {
  if (!term)
    return acc;
  if (!acc)
    return term;
  return insertBinaryBefore(bb, pos, Opcode::Add, Type::i32(), acc, term, stem);
}

Value* cloneInvariantExpr(Value* val, BasicBlock* preheader,
                          Instruction* preheaderTerm, const NaturalLoop& loop,
                          Value* currentIV,
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
  if (!inst || !inst->parent() || !loop.blocks.count(inst->parent()))
    return val;
  if (inst->opcode() == Opcode::Load && inst->numOperands() == 1 &&
      analysis::isScalarSlot(inst->operand(0)) &&
      !analysis::loopStoresToSlot(loop, inst->operand(0))) {
    auto* clone = makeInstruction(Opcode::Load, inst->type(),
                                  inst->name().empty() ? "affine.inv.load"
                                                       : inst->name() + ".affine.inv");
    clone->addOperand(inst->operand(0));
    preheader->insertBefore(preheaderTerm, clone);
    cache[val->id()] = clone;
    return clone;
  }

  std::unordered_map<int, bool> depMemo;
  std::unordered_map<int, bool> invMemo;
  if (!isLoopInvariantValue(val, loop, currentIV, depMemo, invMemo))
    return nullptr;
  if (!analysis::isSimpleInvariantExpr(inst))
    return nullptr;

  auto* clone = makeInstruction(
      inst->opcode(), inst->type(),
      inst->name().empty() ? "affine.inv" : inst->name() + ".affine.inv");
  for (auto* op : inst->operands()) {
    Value* clonedOp =
        cloneInvariantExpr(op, preheader, preheaderTerm, loop, currentIV, cache);
    if (!clonedOp)
      return nullptr;
    clone->addOperand(clonedOp);
  }
  preheader->insertBefore(preheaderTerm, clone);
  cache[val->id()] = clone;
  return clone;
}

Value* buildLinearExpr(Module& mod, BasicBlock* bb, Instruction* pos,
                       const NaturalLoop& loop, Value* currentIV,
                       const std::vector<std::pair<Value*, int64_t>>& terms,
                       int64_t constant, const std::string& stem) {
  Value* acc = nullptr;
  if (constant != 0)
    acc = constI32(mod, constant);
  std::unordered_map<int, Value*> cloneCache;
  int termIndex = 0;
  for (const auto& [value, coeff] : terms) {
    Value* materialized =
        cloneInvariantExpr(value, bb, pos, loop, currentIV, cloneCache);
    if (!materialized)
      return nullptr;
    Value* scaled = scaleByConst(mod, bb, pos, materialized, coeff,
                                 stem + ".term" + std::to_string(termIndex++));
    acc = addIfNeeded(mod, bb, pos, acc, scaled, stem + ".add");
  }
  if (!acc)
    acc = constI32(mod, 0);
  return acc;
}

Value* multiplyByValue(Module& mod, BasicBlock* bb, Instruction* pos,
                       Value* lhs, Value* rhs, const std::string& stem) {
  if (!lhs || !rhs || !isI32Value(lhs) || !isI32Value(rhs))
    return nullptr;
  if (auto* ci = dynamic_cast<ConstInt*>(lhs)) {
    if (ci->value() == 0)
      return constI32(mod, 0);
    if (ci->value() == 1)
      return rhs;
    if (ci->value() == -1)
      return insertUnaryBefore(bb, pos, Opcode::Neg, Type::i32(), rhs,
                               stem + ".neg");
  }
  if (auto* ci = dynamic_cast<ConstInt*>(rhs)) {
    if (ci->value() == 0)
      return constI32(mod, 0);
    if (ci->value() == 1)
      return lhs;
    if (ci->value() == -1)
      return insertUnaryBefore(bb, pos, Opcode::Neg, Type::i32(), lhs,
                               stem + ".neg");
  }
  return insertBinaryBefore(bb, pos, Opcode::Mul, Type::i32(), lhs, rhs,
                            stem + ".mul");
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
    if (!term.basis || !isI32Value(term.basis) ||
        valueDependsOnCurrentIV(term.basis, currentIV, depMemo)) {
      return false;
    }
  }
  for (const auto& term : access.offset.scaledIvTerms) {
    if (term.basis == currentIV) {
      if (!isI32Value(term.invariantScale) ||
          !isLoopInvariantValue(term.invariantScale, loop, currentIV, depMemo,
                                invMemo)) {
        return false;
      }
      continue;
    }
    if (!term.basis || !isI32Value(term.basis) ||
        !isI32Value(term.invariantScale) ||
        valueDependsOnCurrentIV(term.basis, currentIV, depMemo) ||
        !isLoopInvariantValue(term.invariantScale, loop, currentIV, depMemo,
                              invMemo)) {
      return false;
    }
  }
  for (const auto& term : access.offset.invariantTerms) {
    if (!isI32Value(term.basis) ||
        !isLoopInvariantValue(term.basis, loop, currentIV, depMemo, invMemo))
      return false;
  }
  return true;
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

bool buildCurrentIVCoeffTerms(const AffineAccessSummary& access, Value* currentIV,
                              int64_t step,
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
  if (step != 1) {
    constCoeff *= step;
    for (auto& [scale, coeff] : scaledTerms)
      coeff *= step;
  }
  return constCoeff != 0 || !scaledTerms.empty();
}

Value* buildInvariantOffset(Module& mod, BasicBlock* bb, Instruction* pos,
                            const NaturalLoop& loop, Value* currentIV,
                            const AffineAccessSummary& access,
                            const std::string& stem) {
  std::vector<std::pair<Value*, int64_t>> terms;
  for (const auto& term : access.offset.invariantTerms)
    terms.push_back({term.basis, term.coeff});
  for (const auto& term : access.offset.ivTerms) {
    if (term.basis == currentIV)
      continue;
    terms.push_back({term.basis, term.coeff});
  }

  Value* acc = buildLinearExpr(mod, bb, pos, loop, currentIV, terms,
                               access.offset.constant,
                               stem + ".base");
  int scaledIndex = 0;
  for (const auto& term : access.offset.scaledIvTerms) {
    if (term.basis == currentIV)
      continue;
    Value* scaledBase = multiplyByValue(
        mod, bb, pos, term.basis, term.invariantScale,
        stem + ".scaled" + std::to_string(scaledIndex));
    if (!scaledBase)
      return nullptr;
    Value* scaledTerm = scaleByConst(
        mod, bb, pos, scaledBase, term.coeff,
        stem + ".scaled" + std::to_string(scaledIndex));
    acc = addIfNeeded(mod, bb, pos, acc, scaledTerm,
                      stem + ".scaled.add");
    ++scaledIndex;
  }
  return acc;
}

Value* buildCoeffExpr(Module& mod, BasicBlock* bb, Instruction* pos,
                      const NaturalLoop& loop, Value* currentIV,
                      int64_t constCoeff,
                      const std::vector<std::pair<Value*, int64_t>>& scaledTerms,
                      const std::string& stem) {
  return buildLinearExpr(mod, bb, pos, loop, currentIV, scaledTerms, constCoeff,
                         stem + ".coeff");
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

Value* memoryAddress(Instruction* inst);

int typeSizeBytes(Type ty) {
  if (ty.isI32() || ty.isF32())
    return 4;
  if (ty.isPtr())
    return 8;
  if (ty.isV4I32() || ty.isV4F32())
    return 16;
  return 0;
}

int memoryAccessSizeBytes(Instruction* inst) {
  if (!inst)
    return 0;
  if (isLoadOp(inst))
    return typeSizeBytes(inst->type());
  if (isStoreOp(inst) && inst->numOperands() >= 1)
    return typeSizeBytes(inst->operand(0)->type());
  return 0;
}

Value* memoryRootForNoAlias(Value* value, std::unordered_set<int>& visiting) {
  if (!value || !visiting.insert(value->id()).second)
    return nullptr;
  if (dynamic_cast<GlobalVar*>(value)) {
    visiting.erase(value->id());
    return value;
  }
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst) {
    visiting.erase(value->id());
    return nullptr;
  }

  Value* root = nullptr;
  switch (inst->opcode()) {
  case Opcode::Alloca:
    root = inst;
    break;
  case Opcode::GEP:
  case Opcode::BitCast:
  case Opcode::Mov:
    if (inst->numOperands() >= 1)
      root = memoryRootForNoAlias(inst->operand(0), visiting);
    break;
  default:
    root = nullptr;
    break;
  }
  visiting.erase(value->id());
  return root;
}

bool storeProvenDisjointFromFamilyRoot(Instruction* store,
                                       Value* familyRoot) {
  if (!store || !familyRoot || !isStoreOp(store) || store->numOperands() < 2)
    return false;
  std::unordered_set<int> storeVisiting;
  std::unordered_set<int> familyVisiting;
  Value* storeRoot = memoryRootForNoAlias(store->operand(1), storeVisiting);
  Value* root = memoryRootForNoAlias(familyRoot, familyVisiting);
  if (!storeRoot || !root)
    return false;
  // 这里只采用 BasicAA 风格的“确定不同对象”判断。
  // 不同 global/alloca 根一定不别名；函数参数根不在这里判 disjoint，
  // 因为调用点可能把同一个数组传给多个形参。
  return storeRoot != root;
}

std::vector<std::pair<int, int64_t>>
normalizedAffineTerms(const std::vector<AffineTerm>& terms) {
  std::vector<std::pair<int, int64_t>> normalized;
  normalized.reserve(terms.size());
  for (const auto& term : terms) {
    if (!term.basis || term.coeff == 0)
      continue;
    normalized.push_back({term.basis->id(), term.coeff});
  }
  std::sort(normalized.begin(), normalized.end());
  return normalized;
}

std::vector<std::tuple<int, int, int64_t>>
normalizedAffineScaledTerms(const std::vector<AffineScaledTerm>& terms) {
  std::vector<std::tuple<int, int, int64_t>> normalized;
  normalized.reserve(terms.size());
  for (const auto& term : terms) {
    if (!term.basis || !term.invariantScale || term.coeff == 0)
      continue;
    normalized.push_back(
        {term.basis->id(), term.invariantScale->id(), term.coeff});
  }
  std::sort(normalized.begin(), normalized.end());
  return normalized;
}

bool sameAffineShapeExceptConstant(const AffineAccessSummary& lhs,
                                   const AffineAccessSummary& rhs) {
  if (!lhs.valid || !rhs.valid || !lhs.offset.valid || !rhs.offset.valid ||
      lhs.root != rhs.root) {
    return false;
  }
  return normalizedAffineTerms(lhs.offset.ivTerms) ==
             normalizedAffineTerms(rhs.offset.ivTerms) &&
         normalizedAffineScaledTerms(lhs.offset.scaledIvTerms) ==
             normalizedAffineScaledTerms(rhs.offset.scaledIvTerms) &&
         normalizedAffineTerms(lhs.offset.invariantTerms) ==
             normalizedAffineTerms(rhs.offset.invariantTerms);
}

bool storeProvenDisjointFromFamilyAccess(
    Instruction* store,
    const analysis::AffineRecurrencePlan& plan) {
  if (!store || !plan.loop || !isStoreOp(store))
    return false;
  if (storeProvenDisjointFromFamilyRoot(store, plan.access.root))
    return true;
  Value* storeAddr = memoryAddress(store);
  if (!storeAddr)
    return false;

  std::vector<InductionVar> ivs;
  ivs.push_back(plan.induction);
  AffineAccessSummary storeAccess =
      analysis::summarizeAffineAccess(storeAddr, *plan.loop, ivs);
  if (!sameAffineShapeExceptConstant(storeAccess, plan.access))
    return false;

  int storeBytes = memoryAccessSizeBytes(store);
  int familyBytes = 0;
  for (auto* memOp : plan.memOps) {
    if (!memOp)
      continue;
    familyBytes = std::max(familyBytes, memoryAccessSizeBytes(memOp));
  }
  if (storeBytes <= 0 || familyBytes <= 0)
    return false;
  int64_t delta = storeAccess.offset.constant - plan.access.offset.constant;
  int64_t minSeparation = std::max(storeBytes, familyBytes);
  // 这是 BasicAA + ScalarEvolution 风格的保守 disjoint 证明。
  // 同一个根对象上，只有所有非恒定 affine 项完全相同，且常量字节偏移
  // 相差至少一个访问宽度时，才能证明两个访问不会重叠。
  return delta != 0 && std::llabs(delta) >= minSeparation;
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

Instruction* firstNonPhi(BasicBlock* bb);

Value* transferFamilyAvailability(BasicBlock* bb,
                                  const std::unordered_set<int>& familyMemIds,
                                  const analysis::AffineRecurrencePlan& plan,
                                  Value* available) {
  if (!bb)
    return nullptr;
  for (auto& instPtr : bb->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst)
      continue;

    if (inst->opcode() == Opcode::Call) {
      available = nullptr;
      continue;
    }
    if (isStoreOp(inst) && !familyMemIds.count(inst->id())) {
      if (!storeProvenDisjointFromFamilyAccess(inst, plan))
        available = nullptr;
      continue;
    }
    if (!familyMemIds.count(inst->id()))
      continue;

    if (isLoadOp(inst)) {
      if (!available || available->type() != inst->type())
        available = inst;
      continue;
    }

    if (isStoreOp(inst) && inst->numOperands() >= 1) {
      available = inst->operand(0);
      continue;
    }

    available = nullptr;
  }
  return available;
}

struct ArrayUpdateAccumulatorPlan {
  BasicBlock* preheader = nullptr;
  BasicBlock* header = nullptr;
  BasicBlock* latch = nullptr;
  BasicBlock* exit = nullptr;
  BasicBlock* accessBlock = nullptr;
  std::vector<Instruction*> loads;
  std::vector<Instruction*> stores;
  std::vector<Instruction*> outsideLoads;
};

Value* materializeInvariantPointerForPlan(
    Module& mod, const analysis::AffineRecurrencePlan& plan,
    BasicBlock* preheader, Instruction* preheaderTerm,
    const std::string& stem) {
  if (!plan.loop || !preheader || !preheaderTerm || !plan.access.valid ||
      !plan.access.offset.valid || !plan.access.root || !plan.currentIV) {
    return nullptr;
  }
  std::unordered_map<int, Value*> cloneCache;
  Value* rootBase =
      cloneInvariantExpr(plan.access.root, preheader, preheaderTerm, *plan.loop,
                         plan.currentIV, cloneCache);
  if (!rootBase)
    return nullptr;
  Value* offset = buildInvariantOffset(mod, preheader, preheaderTerm, *plan.loop,
                                       plan.currentIV, plan.access, stem);
  if (!offset)
    offset = constI32(mod, 0);
  bool zeroOffset = false;
  if (auto* ci = dynamic_cast<ConstInt*>(offset))
    zeroOffset = ci->value() == 0;
  if (zeroOffset)
    return rootBase;
  return insertGEPBefore(preheader, preheaderTerm, rootBase, offset,
                         stem + ".ptr");
}

bool collectArrayUpdateAccumulatorPlan(
    Function* func, const NaturalLoop& loop,
    const analysis::AffineRecurrencePlan& plan,
    const analysis::DomSet& doms, ArrayUpdateAccumulatorPlan& out,
    const char** rejectReason = nullptr) {
  auto reject = [&](const char* reason) {
    if (rejectReason)
      *rejectReason = reason;
    return false;
  };
  if (!func || !plan.loop || !plan.sameAddressEveryIteration || !plan.hasLoad ||
      !plan.hasStore || plan.memOps.size() < 2 || !loop.header) {
    return reject("basic-shape");
  }
  if (loopHasCalls(loop))
    return reject("loop-has-call");
  auto latches = collectUniqueLatches(loop);
  if (latches.size() != 1)
    return reject("not-single-latch");
  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  if (!preheader || !preheader->terminator())
    return reject("no-dedicated-preheader");

  Instruction* headerTerm = loop.header->terminator();
  if (!headerTerm || headerTerm->opcode() != Opcode::CondBr ||
      headerTerm->numOperands() != 3) {
    return reject("header-not-condbr");
  }
  auto* trueBB = dynamic_cast<BasicBlock*>(headerTerm->operand(1));
  auto* falseBB = dynamic_cast<BasicBlock*>(headerTerm->operand(2));
  if (!trueBB || !falseBB)
    return reject("header-succ-shape");
  BasicBlock* exit = nullptr;
  if (loop.blocks.count(trueBB) && !loop.blocks.count(falseBB))
    exit = falseBB;
  else if (loop.blocks.count(falseBB) && !loop.blocks.count(trueBB))
    exit = trueBB;
  else
    return reject("no-single-exit-edge");

  std::vector<Instruction*> loads;
  std::vector<Instruction*> stores;
  BasicBlock* accessBlock = nullptr;
  for (auto* memOp : plan.memOps) {
    if (!memOp || !memOp->parent() || !loop.blocks.count(memOp->parent()))
      return reject("memop-outside-loop");
    if (accessBlock && accessBlock != memOp->parent())
      return reject("multi-access-block");
    accessBlock = memOp->parent();
    if (isLoadOp(memOp))
      loads.push_back(memOp);
    else if (isStoreOp(memOp))
      stores.push_back(memOp);
    else
      return reject("non-load-store-memop");
  }
  if (!accessBlock || loads.empty() || stores.empty() ||
      loads.size() != stores.size())
    return reject("load-store-count");
  if (!analysis::dominates(doms, accessBlock, latches.front()))
    return reject("access-not-dominate-latch");

  size_t loadIndex = 0;
  size_t storeIndex = 0;
  bool expectLoad = true;
  for (auto& instPtr : accessBlock->instructions()) {
    Instruction* inst = instPtr.get();
    bool isFamilyLoad = loadIndex < loads.size() && loads[loadIndex] == inst;
    bool isFamilyStore =
        storeIndex < stores.size() && stores[storeIndex] == inst;
    if (!isFamilyLoad && !isFamilyStore) {
      if (inst->opcode() == Opcode::Call)
        return reject("call-in-access-block");
      if (isStoreOp(inst) && !storeProvenDisjointFromFamilyAccess(inst, plan))
        return reject("interfering-store-in-access-block");
      continue;
    }
    if (expectLoad != isFamilyLoad)
      return reject("not-load-store-alternating");
    if (isFamilyLoad) {
      ++loadIndex;
      expectLoad = false;
    } else {
      ++storeIndex;
      expectLoad = true;
    }
  }
  if (loadIndex != loads.size() || storeIndex != stores.size() || !expectLoad)
    return reject("partial-family-order");

  for (size_t i = 0; i < loads.size(); ++i) {
    Instruction* load = loads[i];
    Instruction* store = stores[i];
    if (!comesBeforeInBlock(accessBlock, load, store))
      return reject("load-not-before-store");
    if (store->numOperands() < 1)
      return reject("store-no-value");
    std::unordered_map<int, bool> memo;
    if (!isValueComputedFrom(store->operand(0), load, memo))
      return reject("store-not-derived-from-load");
  }

  std::vector<Instruction*> outsideLoads;
  for (const auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst || std::find(plan.memOps.begin(), plan.memOps.end(), inst) !=
                       plan.memOps.end()) {
        continue;
      }
      if (!isLoadOp(inst) && !isStoreOp(inst))
        continue;
      Value* addr = memoryAddress(inst);
      if (!addr)
        continue;
      std::vector<InductionVar> ivs;
      ivs.push_back(plan.induction);
      AffineAccessSummary access =
          analysis::summarizeAffineAccess(addr, loop, ivs);
      if (!sameAffineShapeExceptConstant(access, plan.access) ||
          access.offset.constant != plan.access.offset.constant) {
        continue;
      }
      if (loop.blocks.count(bb))
        return reject("same-slot-user-still-in-loop");
      if (isStoreOp(inst))
        return reject("outside-store-to-same-slot");
      if (!analysis::dominates(doms, exit, bb))
        return reject("outside-load-before-exit");
      outsideLoads.push_back(inst);
    }
  }

  out.preheader = preheader;
  out.header = loop.header;
  out.latch = latches.front();
  out.exit = exit;
  out.accessBlock = accessBlock;
  out.loads = std::move(loads);
  out.stores = std::move(stores);
  out.outsideLoads = std::move(outsideLoads);
  return true;
}

bool applyArrayUpdateAccumulatorPlan(
    Module& mod, const analysis::AffineRecurrencePlan& plan,
    const ArrayUpdateAccumulatorPlan& accumulatorPlan) {
  if (!accumulatorPlan.preheader || !accumulatorPlan.header ||
      !accumulatorPlan.latch || accumulatorPlan.loads.empty() ||
      accumulatorPlan.stores.empty()) {
    return false;
  }
  Instruction* preheaderTerm = accumulatorPlan.preheader->terminator();
  if (!preheaderTerm)
    return false;

  Value* initPtr = materializeInvariantPointerForPlan(
      mod, plan, accumulatorPlan.preheader, preheaderTerm, "affine.array.acc");
  if (!initPtr)
    return false;

  auto* initLoad = makeInstruction(
      Opcode::Load, accumulatorPlan.loads.front()->type(), "affine.array.acc.init");
  initLoad->addOperand(initPtr);
  accumulatorPlan.preheader->insertBefore(preheaderTerm, initLoad);

  auto* phi = makeInstruction(Opcode::Phi, initLoad->type(), "affine.array.acc");
  Instruction* headerPos = firstNonPhi(accumulatorPlan.header);
  if (headerPos)
    accumulatorPlan.header->insertBefore(headerPos, phi);
  else
    accumulatorPlan.header->pushFront(phi);
  phi->addOperand(initLoad);
  phi->addOperand(accumulatorPlan.preheader);
  phi->addOperand(accumulatorPlan.stores.back()->operand(0));
  phi->addOperand(accumulatorPlan.latch);

  Value* current = phi;
  for (size_t i = 0; i < accumulatorPlan.loads.size(); ++i) {
    accumulatorPlan.loads[i]->replaceAllUsesWith(current);
    current = accumulatorPlan.stores[i]->operand(0);
  }

  for (auto* load : accumulatorPlan.outsideLoads)
    load->replaceAllUsesWith(phi);

  Instruction* exitPos = firstNonPhi(accumulatorPlan.exit);
  if (!exitPos)
    exitPos = accumulatorPlan.exit->terminator();
  if (!exitPos)
    return false;
  auto* exitStore = makeInstruction(Opcode::Store, Type::voidTy(),
                                    "affine.array.acc.flush");
  exitStore->addOperand(phi);
  exitStore->addOperand(initPtr);
  accumulatorPlan.exit->insertBefore(exitPos, exitStore);

  for (auto* load : accumulatorPlan.loads) {
    if (load && load->parent())
      load->parent()->remove(load);
  }
  for (auto* store : accumulatorPlan.stores) {
    if (store && store->parent())
      store->parent()->remove(store);
  }
  for (auto* load : accumulatorPlan.outsideLoads) {
    if (load && load->parent())
      load->parent()->remove(load);
  }
  return true;
}

Value* incomingFamilyAvailability(const NaturalLoop& loop,
                                  BasicBlock* bb,
                                  const std::unordered_map<BasicBlock*, Value*>& outAvail) {
  if (!bb)
    return nullptr;
  bool sawPred = false;
  Value* candidate = nullptr;
  for (auto* pred : bb->preds()) {
    if (!pred || !loop.blocks.count(pred))
      return nullptr;
    auto it = outAvail.find(pred);
    Value* predAvail = it == outAvail.end() ? nullptr : it->second;
    if (!sawPred) {
      candidate = predAvail;
      sawPred = true;
      continue;
    }
    if (candidate != predAvail)
      return nullptr;
  }
  if (!sawPred)
    return nullptr;
  return candidate;
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

void setMemoryAddress(Instruction* inst, Value* addr) {
  if (!inst || !addr)
    return;
  if ((inst->opcode() == Opcode::Load || inst->opcode() == Opcode::VLoad) &&
      inst->numOperands() == 1) {
    inst->setOperand(0, addr);
  } else if ((inst->opcode() == Opcode::Store || inst->opcode() == Opcode::VStore) &&
             inst->numOperands() == 2) {
    inst->setOperand(1, addr);
  }
}

bool forwardScalarFamilyLoads(const analysis::AffineRecurrencePlan& plan) {
  if (!plan.loop || plan.memOps.size() < 2)
    return false;

  std::unordered_set<int> familyMemIds;
  for (auto* memOp : plan.memOps) {
    if (memOp)
      familyMemIds.insert(memOp->id());
  }
  if (familyMemIds.size() < 2)
    return false;

  std::unordered_map<BasicBlock*, Value*> entryAvail;
  std::unordered_map<BasicBlock*, Value*> outAvail;
  for (auto* bb : plan.loop->blocks) {
    if (!bb)
      continue;
    entryAvail[bb] = nullptr;
    outAvail[bb] = nullptr;
  }
  for (int iter = 0; iter < static_cast<int>(plan.loop->blocks.size()) * 2; ++iter) {
    bool iterChanged = false;
    for (auto* bb : plan.loop->blocks) {
      if (!bb)
        continue;
      Value* incoming = incomingFamilyAvailability(*plan.loop, bb, outAvail);
      Value* outgoing =
          transferFamilyAvailability(bb, familyMemIds, plan, incoming);
      if (entryAvail[bb] != incoming) {
        entryAvail[bb] = incoming;
        iterChanged = true;
      }
      if (outAvail[bb] != outgoing) {
        outAvail[bb] = outgoing;
        iterChanged = true;
      }
    }
    if (!iterChanged)
      break;
  }

  bool changed = false;
  for (auto* bb : plan.loop->blocks) {
    if (!bb)
      continue;

    Value* available = entryAvail[bb];
    std::vector<Instruction*> toErase;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;

      if (inst->opcode() == Opcode::Call) {
        available = nullptr;
        continue;
      }
      if (isStoreOp(inst) && !familyMemIds.count(inst->id())) {
        if (!storeProvenDisjointFromFamilyAccess(inst, plan))
          available = nullptr;
        continue;
      }
      if (!familyMemIds.count(inst->id()))
        continue;

      if (isLoadOp(inst)) {
        if (available && available->type() == inst->type()) {
          inst->replaceAllUsesWith(available);
          toErase.push_back(inst);
          changed = true;
          continue;
        }
        available = inst;
        continue;
      }

      if (isStoreOp(inst) && inst->numOperands() >= 1) {
        available = inst->operand(0);
        continue;
      }

      available = nullptr;
    }

    for (auto* inst : toErase) {
      if (inst && inst->parent())
        inst->parent()->remove(inst);
    }
  }
  return changed;
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

bool analyzeMemoryOpForIV(const NaturalLoop& loop,
                          const std::vector<InductionVar>& loopIVs,
                          const InductionVar& iv, Instruction* memOp,
                          AffineAccessSummary& access, int64_t& rawCoeff,
                          std::vector<std::pair<Value*, int64_t>>& scaledTerms) {
  if (!memOp || !iv.isCanonical || !iv.init || iv.step == 0)
    return false;
  Value* currentIV = iv.phi ? static_cast<Value*>(iv.phi)
                            : static_cast<Value*>(iv.storage);
  if (!currentIV)
    return false;
  Value* addr = memoryAddress(memOp);
  if (!addr)
    return false;

  access = analysis::summarizeAffineAccess(addr, loop, loopIVs);
  std::unordered_map<int, bool> depMemo;
  std::unordered_map<int, bool> invMemo;
  if (!access.valid || !access.offset.valid ||
      !isLoopInvariantValue(access.root, loop, currentIV, depMemo, invMemo) ||
      !currentIVTermsOnly(access, currentIV, loop) || !isI32Value(iv.init) ||
      !buildCurrentIVCoeffTerms(access, currentIV, 1, rawCoeff, scaledTerms)) {
    return false;
  }
  return rawCoeff != 0 || !scaledTerms.empty();
}

Value* materializeRecurrenceForPlan(
    Module& mod, const analysis::AffineRecurrencePlan& plan) {
  const NaturalLoop& loop = *plan.loop;
  const InductionVar& iv = plan.induction;
  if (!iv.isCanonical || !iv.init || iv.step == 0)
    return nullptr;
  Value* currentIV = plan.currentIV;
  if (!currentIV)
    return nullptr;
  BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  auto latches = collectUniqueLatches(loop);
  if (!preheader || !preheader->terminator() || latches.empty())
    return nullptr;
  for (auto* latch : latches) {
    if (!latch || !latch->terminator())
      return nullptr;
  }
  if (!plan.access.valid || !plan.access.offset.valid || !plan.access.root)
    return nullptr;
  if (!isI32Value(iv.init))
    return nullptr;

  BasicBlock* header = loop.header;
  Instruction* preheaderTerm = preheader->terminator();
  if (!header)
    return nullptr;

  Value* coeffExpr = buildCoeffExpr(mod, preheader, preheaderTerm, loop,
                                    currentIV, plan.rawCoeff,
                                    plan.scaledTerms, "affine.coeff");
  if (!coeffExpr)
    return nullptr;
  Value* strideExpr = coeffExpr;
  if (iv.step != 1) {
    strideExpr = scaleByConst(mod, preheader, preheaderTerm, coeffExpr, iv.step,
                              "affine.stride");
    if (!strideExpr)
      return nullptr;
  }

  Value* baseOffset = buildInvariantOffset(mod, preheader, preheaderTerm, loop,
                                           currentIV, plan.access,
                                           "affine.offset");
  Value* initContribution =
      multiplyByValue(mod, preheader, preheaderTerm, iv.init, coeffExpr,
                      "affine.init");
  Value* startOffset =
      addIfNeeded(mod, preheader, preheaderTerm, baseOffset, initContribution,
                  "affine.start");
  if (!startOffset)
    startOffset = constI32(mod, 0);

  std::unordered_map<int, Value*> cloneCache;
  Value* rootBase =
      cloneInvariantExpr(plan.access.root, preheader, preheaderTerm, loop,
                         currentIV, cloneCache);
  if (!rootBase)
    return nullptr;
  Value* startPtr = rootBase;
  bool zeroStart = false;
  if (auto* ci = dynamic_cast<ConstInt*>(startOffset))
    zeroStart = ci->value() == 0;
  if (!zeroStart) {
    startPtr = insertGEPBefore(preheader, preheaderTerm, rootBase,
                               startOffset, "affine.ptr.start");
  }

  auto* ptrPhi = makeInstruction(Opcode::Phi, Type::ptr(), "affine.ptr");
  Instruction* headerPos = firstNonPhi(header);
  if (headerPos)
    header->insertBefore(headerPos, ptrPhi);
  else
    header->pushFront(ptrPhi);

  ptrPhi->addOperand(startPtr);
  ptrPhi->addOperand(preheader);
  bool zeroStride = false;
  if (auto* ci = dynamic_cast<ConstInt*>(strideExpr))
    zeroStride = ci->value() == 0;
  for (auto* latch : latches) {
    Instruction* latchTerm = latch->terminator();
    Value* nextPtr = ptrPhi;
    if (!zeroStride) {
      nextPtr = insertGEPBefore(latch, latchTerm, ptrPhi, strideExpr,
                                "affine.ptr.next");
    }
    ptrPhi->addOperand(nextPtr);
    ptrPhi->addOperand(latch);
  }
  return ptrPhi;
}

bool rewritePlan(Module& mod, const analysis::AffineRecurrencePlan& plan) {
  // update-slice companion 与普通 pointer bump 共享同一套
  // recurrence 物化逻辑；区别只在于 companion family 的形成条件更窄，
  // 只服务“load dst + load src + store dst”这类更新切片。
  if (!plan.pointerBumpCandidate && !plan.tileLocalScalarCandidate &&
      !plan.updateSliceCompanion && !plan.updateSliceAnchor)
    return false;
  if (isSmallLocalScratchRoot(plan.access.root))
    return false;
  if (plan.tileLocalScalarCandidate && plan.sameAddressEveryIteration &&
      !plan.pointerBumpCandidate && !plan.updateSliceCompanion &&
      !plan.updateSliceAnchor) {
    // 同地址 reduction/update 的主要收益是把 load-store-load
    // 链条转成 SSA 前递；它的地址本来就不随当前 IV 变化。若先物化
    // 零步长 pointer phi，反而会在热循环里增加 phi/live range 压力，
    // 让后续 blocking/unroll 更难保持紧凑。这里直接消费 family
    // 这里做标量前递，pointer bump 仍留给真正有步长的 stream。
    return forwardScalarFamilyLoads(plan);
  }
  Value* ptrPhi = materializeRecurrenceForPlan(mod, plan);
  if (!ptrPhi)
    return false;
  for (auto* memOp : plan.memOps)
    setMemoryAddress(memOp, ptrPhi);
  bool changed = true;
  if (plan.tileLocalScalarCandidate)
    changed |= forwardScalarFamilyLoads(plan);
  return changed;
}

// 把每迭代地址不变的 load-modify-store 归约为 tile-local 标量累加器：
// 前置检查排除 update-slice 家族与小 scratch 根，随后收集并应用
// accumulator plan，任何一步失败都记录 trace 后拒绝。
bool rewriteArrayUpdateAccumulator(Module& mod, Function* func,
                                   analysis::FunctionAnalysisCache& cache,
                                   const analysis::AffineRecurrencePlan& plan) {
  if (!func || !plan.loop) {
    traceArrayUpdateDecision("reject", func, plan, "no-func-or-loop");
    return false;
  }
  if (!plan.sameAddressEveryIteration) {
    traceArrayUpdateDecision("reject", func, plan, "not-same-address");
    return false;
  }
  if (!plan.tileLocalScalarCandidate) {
    traceArrayUpdateDecision("reject", func, plan, "not-tile-local-scalar");
    return false;
  }
  if (!plan.hasLoad || !plan.hasStore) {
    traceArrayUpdateDecision("reject", func, plan, "missing-load-or-store");
    return false;
  }
  if (plan.updateSliceAnchor || plan.updateSliceCompanion) {
    traceArrayUpdateDecision("reject", func, plan, "update-slice-family");
    return false;
  }
  if (isSmallLocalScratchRoot(plan.access.root)) {
    traceArrayUpdateDecision("reject", func, plan, "small-local-scratch");
    return false;
  }
  ArrayUpdateAccumulatorPlan accumulatorPlan;
  const char* rejectReason = nullptr;
  if (!collectArrayUpdateAccumulatorPlan(func, *plan.loop, plan,
                                         cache.dominators(), accumulatorPlan,
                                         &rejectReason)) {
    traceArrayUpdateDecision("reject", func, plan,
                             rejectReason ? rejectReason : "collect-failed");
    return false;
  }
  const ArrayUpdateTraceSnapshot traceSnapshot =
      snapshotArrayUpdateTrace(func, plan);
  if (!applyArrayUpdateAccumulatorPlan(mod, plan, accumulatorPlan)) {
    traceArrayUpdateDecision("reject", func, plan, "apply-failed");
    return false;
  }
  traceArrayUpdateDecision("accept", traceSnapshot, "applied");
  return true;
}

const AffineRegionPlan* findLoopRegion(
    const NaturalLoop& loop,
    const std::vector<AffineRegionPlan>& regions) {
  if (!loop.header)
    return nullptr;
  const AffineRegionPlan* best = nullptr;
  int64_t bestScore = std::numeric_limits<int64_t>::min();
  for (const auto& region : regions) {
    if (!region.valid || !region.loop || !region.loop->header ||
        region.loop->header != loop.header) {
      continue;
    }
    int64_t score = analysis::affineRegionNetProfitScore(region);
    if (region.profit.tileLocalScalarReady)
      score += 64;
    if (score <= bestScore)
      continue;
    best = &region;
    bestScore = score;
  }
  return best;
}

ScalarRewriteRegionApproval evaluateScalarRewriteRegionApproval(
    const analysis::AffineRecurrencePlan& plan,
    const AffineRegionPlan* region) {
  ScalarRewriteRegionApproval approval;
  if (!region) {
    if (plan.tileLocalScalarCandidate) {
      // tile-local scalar replacement 会改变同地址 load/store 的
      // 可见顺序，必须由 AffineRegionPlan 证明局部工作集、别名和寄存器压力
      // 都可控。没有 region/profit 证据时，只允许低风险 pointer bump 继续。
      approval.tileLocalScalarReady = false;
      approval.priorityDelta -= 256;
    }
    return approval;
  }

  const auto& profit = region->profit;
  const auto& versioning = region->versioning;
  approval.priorityDelta += analysis::affineRegionNetProfitScore(*region) / 4;
  if (versioning.hasMergePhi)
    approval.priorityDelta += static_cast<int64_t>(versioning.phiCount) * 16;
  approval.priorityDelta += profit.postSplitCfgDeltaScore;
  approval.priorityDelta += profit.postSplitMemoryDeltaScore;
  approval.priorityDelta += profit.estimatedWorkingSetDrop / 8;
  approval.priorityDelta += profit.localityProfitScore / 64;
  approval.priorityDelta -= profit.localityRiskScore / 16;
  approval.priorityDelta -=
      static_cast<int64_t>(profit.estimatedScalarRegisterPressure) * 8;

  // 需要 versioning 的 region 必须先形成可消费的 interior
  // fast path。这里把排序和执行共享同一套判断，避免 priority 认为可做、
  // 真正改写时又按另一套规则跳过。
  if (region->needsVersioning &&
      (!profit.interiorFastPathReady || !versioning.hasMergePhi ||
       !profit.postSplitMetricsImproved)) {
    approval.loopReady = false;
    approval.pointerBumpReady = false;
    approval.tileLocalScalarReady = false;
    approval.priorityDelta -= 512;
    return approval;
  }

  const bool versionFreeInteriorReady =
      !region->needsVersioning && profit.interiorFastPathReady &&
      profit.postSplitMetricsImproved;
  const bool versionedInteriorReady = profit.physicalVersioned &&
                                      profit.postSplitMetricsImproved &&
                                      profit.postSplitProfitable &&
                                      versioning.hasMergePhi;
  const bool updateSliceSharedReady =
      (plan.updateSliceAnchor || plan.updateSliceCompanion) &&
      profit.interiorFastPathReady &&
      (profit.postSplitMetricsImproved || profit.postSplitProfitScore >= 0 ||
       profit.localityProfitScore >= profit.localityRiskScore / 2);

  if (!region->needsVersioning && !profit.interiorFastPathReady &&
      !profit.tileLocalScalarReady &&
      profit.localityProfitScore < profit.localityRiskScore) {
    // 非 versioned 弱 region 仍然不适合激进的 tile-local scalar，
    // 但 pointer bump 只是把已证明的 affine 地址链改写成递推指针，风险远小于
    // 标量替换/扩展，因此这里只降级标量化，不再一刀切阻断整条 recurrence 主线。
    approval.tileLocalScalarReady = false;
    approval.priorityDelta -= 256;
  }

  if ((plan.updateSliceAnchor || plan.updateSliceCompanion) &&
      !(versionFreeInteriorReady || versionedInteriorReady ||
        updateSliceSharedReady)) {
    // update-slice 会把同一更新切片里的纯 load stream 绑到共享
    // recurrence 上。它比普通 pointer bump 更依赖稳定的 interior fast path，
    // 但如果 region 已经证明不需要额外 versioning，就可以直接复用这条
    // 通用 fast path，而不是继续被 physical split 的外壳卡住。
    approval.pointerBumpReady = false;
    approval.priorityDelta -= 224;
  }

  if (plan.role == analysis::AffineRecurrenceRole::Broadcast &&
      !profit.tileLocalScalarReady && !profit.multiLevelBlockingReady) {
    // broadcast family 对 dense-affine 热核有帮助，但若 region
    // 既没有 tile-local scalar 准备，也没有稳定 blocking 收益，就先别让
    // 它挤占 reduction/stream 的 rewrite 名额。
    approval.pointerBumpReady = false;
    approval.tileLocalScalarReady = false;
    approval.priorityDelta -= 160;
  }

  if (plan.tileLocalScalarCandidate) {
    if (profit.tileLocalScalarReady) {
      approval.priorityDelta += 320;
      approval.priorityDelta +=
          std::max<int64_t>(0, 16 - profit.estimatedScalarRegisterPressure) * 24;
    } else {
      approval.tileLocalScalarReady = false;
      approval.priorityDelta -= 192;
    }
    if (!profit.postSplitMetricsImproved)
      approval.tileLocalScalarReady = false;
    if (region->needsVersioning && !versioning.hasMergePhi) {
      approval.tileLocalScalarReady = false;
      approval.priorityDelta -= 128;
    }
  }

  if ((plan.pointerBumpCandidate || plan.updateSliceCompanion ||
       plan.updateSliceAnchor) &&
      region->needsVersioning &&
      !profit.postSplitProfitable) {
    approval.pointerBumpReady = false;
    approval.priorityDelta -= 256;
  }
  return approval;
}

int64_t scalarRewritePlanPriority(const analysis::AffineRecurrencePlan& plan,
                                  const AffineRegionPlan* region) {
  int64_t score = plan.estimatedBenefit;
  score += recurrenceRolePriority(plan.role);
  if (plan.pointerBumpCandidate)
    score += 128;
  if (plan.tileLocalScalarCandidate)
    score += 192;
  if (plan.updateSliceAnchor)
    score += 160;
  if (plan.unitStride)
    score += 96;
  if (plan.dynamicStride)
    score -= 96;
  if (plan.sameAddressEveryIteration && !plan.tileLocalScalarCandidate)
    score -= 64;
  score -= static_cast<int64_t>(plan.estimatedRegisterPressure) * 16;
  score += evaluateScalarRewriteRegionApproval(plan, region).priorityDelta;
  return score;
}

bool processLoop(Module& mod, Function* func,
                 analysis::FunctionAnalysisCache& cache,
                 const NaturalLoop& loop,
                 const AffineRegionPlan* region) {
  if (!func || !loop.header)
    return false;

  const auto& plans = cache.affineRecurrences(loop);
  if (plans.empty())
    return false;

  std::vector<analysis::AffineRecurrencePlan> orderedPlans(plans.begin(),
                                                           plans.end());
  std::sort(orderedPlans.begin(), orderedPlans.end(),
            [&](const analysis::AffineRecurrencePlan& lhs,
                const analysis::AffineRecurrencePlan& rhs) {
              int64_t lhsScore = scalarRewritePlanPriority(lhs, region);
              int64_t rhsScore = scalarRewritePlanPriority(rhs, region);
              if (lhsScore != rhsScore)
                return lhsScore > rhsScore;
              if (lhs.memOps.size() != rhs.memOps.size())
                return lhs.memOps.size() > rhs.memOps.size();
              return lhs.familyKey < rhs.familyKey;
            });

  bool changed = false;
  std::unordered_set<int> rewrittenMemOps;
  std::unordered_set<std::string> rewrittenFamilies;
  for (const auto& plan : orderedPlans) {
    if (plan.updateSliceCompanion) {
      traceScalarRewriteDecision("skip", func, plan, "companion-deferred");
      continue;
    }
    if (plan.updateSliceAnchor && !plan.hasLoad &&
        plan.updateSliceCompanionCount < 2) {
      // 单 load -> 单 store 的简单 copy 通常只是把一条线性访存
      // 换成两条 pointer phi，容易拉长 live range；真正值得共享 rewrite
      // 的是多输入切片，或 anchor 本身也含 load/store 的 in-place update。
      traceScalarRewriteDecision("skip", func, plan, "simple-update-slice");
      continue;
    }
    // region 级准入：loop/scalar/pointer 各就绪条件逐一检查，
    // 且 memOp 已被先前 plan 改写过的直接跳过，避免重复重写。
    ScalarRewriteRegionApproval approval =
        evaluateScalarRewriteRegionApproval(plan, region);
    if (!approval.loopReady) {
      if (plan.sameAddressEveryIteration && plan.tileLocalScalarCandidate)
        traceArrayUpdateDecision("skip", func, plan, "region-loop-not-ready");
      if (plan.updateSliceAnchor || plan.role == analysis::AffineRecurrenceRole::UpdateStream)
        traceScalarRewriteDecision("skip", func, plan, "region-loop-not-ready");
      continue;
    }
    if (plan.tileLocalScalarCandidate && !approval.tileLocalScalarReady) {
      if (plan.sameAddressEveryIteration)
        traceArrayUpdateDecision("skip", func, plan, "region-scalar-not-ready");
      if (plan.updateSliceAnchor)
        traceScalarRewriteDecision("skip", func, plan, "region-scalar-not-ready");
      continue;
    }
    if ((plan.pointerBumpCandidate || plan.updateSliceCompanion ||
         plan.updateSliceAnchor) &&
        !approval.pointerBumpReady) {
      if (plan.sameAddressEveryIteration && plan.tileLocalScalarCandidate)
        traceArrayUpdateDecision("skip", func, plan, "region-pointer-not-ready");
      if (plan.updateSliceAnchor || plan.role == analysis::AffineRecurrenceRole::UpdateStream)
        traceScalarRewriteDecision("skip", func, plan, "region-pointer-not-ready");
      continue;
    }
    if (!plan.pointerBumpCandidate && !plan.tileLocalScalarCandidate &&
        !plan.updateSliceAnchor) {
      if (plan.sameAddressEveryIteration)
        traceArrayUpdateDecision("skip", func, plan, "not-rewrite-family");
      if (plan.role == analysis::AffineRecurrenceRole::UpdateStream)
        traceScalarRewriteDecision("skip", func, plan, "not-rewrite-family");
      continue;
    }
    bool blocked = false;
    for (auto* memOp : plan.memOps) {
      if (memOp && rewrittenMemOps.count(memOp->id())) {
        blocked = true;
        break;
      }
    }
    if (blocked) {
      if (plan.sameAddressEveryIteration && plan.tileLocalScalarCandidate)
        traceArrayUpdateDecision("skip", func, plan, "memop-already-rewritten");
      if (plan.updateSliceAnchor || plan.role == analysis::AffineRecurrenceRole::UpdateStream)
        traceScalarRewriteDecision("skip", func, plan, "memop-already-rewritten");
      continue;
    }
    bool rewritten = rewriteArrayUpdateAccumulator(mod, func, cache, plan);
    if (!rewritten)
      rewritten = rewritePlan(mod, plan);
    if (!rewritten) {
      if (plan.updateSliceAnchor || plan.role == analysis::AffineRecurrenceRole::UpdateStream)
        traceScalarRewriteDecision("reject", func, plan, "rewrite-failed");
      continue;
    }
    if (plan.updateSliceAnchor || plan.role == analysis::AffineRecurrenceRole::UpdateStream)
      traceScalarRewriteDecision("accept", func, plan, "rewritten");
    changed = true;
    rewrittenFamilies.insert(plan.familyKey);
    for (auto* memOp : plan.memOps) {
      if (memOp)
        rewrittenMemOps.insert(memOp->id());
    }

    // anchor 改写成功后，同一 update slice 的 companion plan 依次跟进
    // pointer-bump 改写，共享 rewrittenMemOps 冲突检测与记录。
    for (const auto& companion : orderedPlans) {
      if (!companion.updateSliceCompanion ||
          companion.updateSliceAnchorKey != plan.familyKey ||
          rewrittenFamilies.count(companion.familyKey)) {
        continue;
      }
      ScalarRewriteRegionApproval companionApproval =
          evaluateScalarRewriteRegionApproval(companion, region);
      if (!companionApproval.loopReady || !companionApproval.pointerBumpReady) {
        traceScalarRewriteDecision("skip", func, companion,
                                   !companionApproval.loopReady
                                       ? "companion-loop-not-ready"
                                       : "companion-pointer-not-ready");
        continue;
      }
      bool companionBlocked = false;
      for (auto* memOp : companion.memOps) {
        if (memOp && rewrittenMemOps.count(memOp->id())) {
          companionBlocked = true;
          break;
        }
      }
      if (companionBlocked) {
        traceScalarRewriteDecision("skip", func, companion,
                                   "companion-memop-already-rewritten");
        continue;
      }
      if (!rewritePlan(mod, companion)) {
        traceScalarRewriteDecision("reject", func, companion,
                                   "companion-rewrite-failed");
        continue;
      }
      traceScalarRewriteDecision("accept", func, companion,
                                 "companion-rewritten");
      changed = true;
      rewrittenFamilies.insert(companion.familyKey);
      for (auto* memOp : companion.memOps) {
        if (memOp)
          rewrittenMemOps.insert(memOp->id());
      }
    }
  }
  return changed;
}

int64_t scalarRewriteLoopPriority(const NaturalLoop& loop,
                                  const std::vector<AffineRegionPlan>& regions) {
  int64_t score = static_cast<int64_t>(loop.blocks.size()) * 4;
  if (!loop.header)
    return score;
  if (const auto* region = findLoopRegion(loop, regions)) {
    const auto& profit = region->profit;
    // tile-local scalar replacement 优先服务 region plan 已经
    // 证明“局部标量化收益高且寄存器压力可控”的循环；真正改写仍由
    // collectAffineRecurrencePlans/rewritePlan 的别名和 CFG 检查兜底。
    score += analysis::affineRegionNetProfitScore(*region) / 2;
    if (profit.tileLocalScalarReady)
      score += 512;
    if (profit.tileLocalScalarReady)
      score += std::max<int64_t>(0, 16 - profit.estimatedScalarRegisterPressure) * 32;
    if (profit.postSplitProfitable)
      score += 192 + profit.postSplitProfitScore / 4;
    if (profit.multiLevelBlockingReady)
      score += 128;
    if (profit.interiorFastPathReady)
      score += 96;
    score += profit.estimatedWorkingSetDrop / 4;
    score += profit.localityProfitScore / 32;
    score -= profit.localityRiskScore / 8;
  }
  return score;
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

  auto loops = cache.loops();
  std::vector<AffineRegionPlan> regions(cache.affineRegions().begin(),
                                        cache.affineRegions().end());
  std::sort(loops.begin(), loops.end(),
            [&](const NaturalLoop& lhs, const NaturalLoop& rhs) {
              int64_t lhsScore = scalarRewriteLoopPriority(lhs, regions);
              int64_t rhsScore = scalarRewriteLoopPriority(rhs, regions);
              if (lhsScore != rhsScore)
                return lhsScore > rhsScore;
              if (lhs.blocks.size() != rhs.blocks.size())
                return lhs.blocks.size() < rhs.blocks.size();
              return lhs.header->id() < rhs.header->id();
            });

  bool changed = false;
  for (const auto& loop : loops) {
    const AffineRegionPlan* region = findLoopRegion(loop, regions);
    if (!processLoop(mod, func, cache, loop, region))
      continue;
    changed = true;
    // 地址递推会插入 phi/gep 并改写访存操作数，后续 affine
    // region、loop band 和 lowering hint 都必须基于新 IR 重新分析。
    analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
  }
  return changed;
}

} // namespace

bool applyAffineScalarRewrite(Module& mod) {
  analysis::AnalysisManager analysisManager(mod);
  return applyAffineScalarRewrite(mod, analysisManager);
}

bool applyAffineScalarRewrite(Module& mod,
                              analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions())
    changed |= processFunction(mod, funcPtr.get(), analysisManager);
  return changed;
}

} // namespace hir
