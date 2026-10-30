#include "research_switch.h"
// 文件职责：基于纯度和调用期稳定状态为可 memoize 自递归生成缓存 wrapper。本文件负责实现具体逻辑。
#include "recursion_memoization.h"

#include "../analysis/analysis_manager.h"
#include "../analysis/function_effects.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>

namespace hir {

namespace {

using core_ir::BasicBlock;
using core_ir::Function;
using core_ir::GlobalVar;
using core_ir::IRBuilder;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Type;
using core_ir::Value;

constexpr int kDefaultMemoTableSize = 1 << 20;
constexpr int kLargeMemoTableSize = 1 << 21;
constexpr int kXLargeMemoTableSize = 1 << 22;
constexpr int kXXLargeMemoTableSize = 1 << 24;

bool isMemoGlobalName(const std::string& name) {
  return name.rfind("__memo.", 0) == 0;
}

bool functionHasMemoGlobals(const Module& mod, const Function* func) {
  if (!func)
    return false;
  const std::string prefix = "__memo." + func->name();
  int matched = 0;
  for (const auto& gvPtr : mod.globals()) {
    const auto& name = gvPtr->name();
    if (name.rfind(prefix + ".", 0) == 0)
      ++matched;
  }
  return matched >= 4;
}

bool functionAlreadyMemoized(const Module& mod, const Function* func) {
  if (!func || !func->entryBlock())
    return false;
  if (functionHasMemoGlobals(mod, func))
    return true;
  for (const auto& bbPtr : func->blocks()) {
    if (bbPtr->name().find("memo.") != std::string::npos)
      return true;
  }
  return false;
}

void collectReferencedGlobals(Value* value,
                              std::unordered_set<const GlobalVar*>& out,
                              std::unordered_set<Value*>& visiting) {
  if (!value || !visiting.insert(value).second)
    return;
  if (auto* gv = dynamic_cast<GlobalVar*>(value)) {
    out.insert(gv);
    return;
  }
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst)
    return;
  for (int i = 0; i < inst->numOperands(); ++i)
    collectReferencedGlobals(inst->operand(i), out, visiting);
}

std::vector<const GlobalVar*> inferStableScalarGlobals(const Function* func) {
  std::unordered_set<const GlobalVar*> referenced;
  std::unordered_set<Value*> visiting;
  if (!func)
    return {};
  for (const auto& bbPtr : func->blocks()) {
    for (const auto& instPtr : bbPtr->instructions()) {
      Instruction* inst = instPtr.get();
      for (int i = 0; i < inst->numOperands(); ++i)
        collectReferencedGlobals(inst->operand(i), referenced, visiting);
    }
  }
  std::vector<const GlobalVar*> stable;
  for (const GlobalVar* gv : referenced) {
    if (!gv || isMemoGlobalName(gv->name()))
      continue;
    if (gv->sizeBytes() == 4 && gv->dims().empty())
      stable.push_back(gv);
  }
  std::sort(stable.begin(), stable.end(),
            [](const GlobalVar* lhs, const GlobalVar* rhs) {
              return lhs->name() < rhs->name();
            });
  return stable;
}

struct MemoGlobals {
  GlobalVar* epoch = nullptr;
  GlobalVar* depth = nullptr;
  GlobalVar* stateHash = nullptr;
  std::vector<GlobalVar*> stateKeys;
  GlobalVar* stamp = nullptr;
  GlobalVar* value = nullptr;
  std::vector<GlobalVar*> keys;
  std::vector<const GlobalVar*> stableScalarGlobals;
  analysis::RecursiveMemoLayout layout = analysis::RecursiveMemoLayout::None;
  bool persistentState = false;
  bool singleInvocation = false;
  int tableEntries = kDefaultMemoTableSize;
  int tableMask = kDefaultMemoTableSize - 1;
  std::vector<int32_t> denseLowerBounds;
  std::vector<int32_t> denseExtents;
};

bool hasSingleExternalCall(const Module& mod, const Function* target,
                           analysis::AnalysisManager& analysisManager) {
  if (!target)
    return false;
  int calls = 0;
  Function* uniqueCaller = nullptr;
  BasicBlock* uniqueCallBlock = nullptr;
  for (const auto& funcPtr : mod.functions()) {
    Function* caller = funcPtr.get();
    if (!caller)
      continue;
    for (const auto& bbPtr : caller->blocks()) {
      for (const auto& instPtr : bbPtr->instructions()) {
        const Instruction* inst = instPtr.get();
        if (!inst || inst->opcode() != Opcode::Call ||
            inst->name() != target->name() || caller == target) {
          continue;
        }
        if (++calls > 1)
          return false;
        uniqueCaller = caller;
        uniqueCallBlock = bbPtr.get();
      }
    }
  }
  if (calls != 1 || !uniqueCaller || uniqueCaller->name() != "main" ||
      !uniqueCallBlock) {
    return false;
  }
  for (const auto& loop : analysisManager.function(uniqueCaller).loops()) {
    if (loop.blocks.count(uniqueCallBlock))
      return false;
  }
  return true;
}

struct ReturnSite {
  BasicBlock* pred = nullptr;
  Value* value = nullptr;
};

Instruction* makeInstruction(Opcode op, Type ty, const std::string& name = "") {
  return new Instruction(op, ty, name);
}

Instruction* insertUnaryBefore(BasicBlock* bb, Instruction* pos,
                               Opcode op, Type ty, Value* operand,
                               const std::string& name = "") {
  auto* inst = makeInstruction(op, ty, name);
  inst->addOperand(operand);
  bb->insertBefore(pos, inst);
  return inst;
}

Instruction* insertBinaryBefore(BasicBlock* bb, Instruction* pos,
                                Opcode op, Type ty, Value* lhs, Value* rhs,
                                const std::string& name = "") {
  auto* inst = makeInstruction(op, ty, name);
  inst->addOperand(lhs);
  inst->addOperand(rhs);
  bb->insertBefore(pos, inst);
  return inst;
}

Instruction* insertGEPBefore(BasicBlock* bb, Instruction* pos,
                             Value* base, Value* offset,
                             const std::string& name = "") {
  auto* inst = makeInstruction(Opcode::GEP, Type::ptr(), name);
  inst->addOperand(base);
  inst->addOperand(offset);
  bb->insertBefore(pos, inst);
  return inst;
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

GlobalVar* findGlobalByName(Module& mod, const std::string& name) {
  for (const auto& gvPtr : mod.globals()) {
    if (gvPtr->name() == name)
      return gvPtr.get();
  }
  return nullptr;
}

int chooseMemoTableEntries(const Function* func,
                           const analysis::RecursiveFunctionSummary* summary) {
  if (!func || !summary)
    return kDefaultMemoTableSize;
  if ((summary->memoLayout == analysis::RecursiveMemoLayout::Dense1D ||
       summary->memoLayout == analysis::RecursiveMemoLayout::Dense2D) &&
      summary->denseCellEstimate > 0 &&
      summary->denseCellEstimate <=
          static_cast<size_t>(std::numeric_limits<int>::max())) {
    return static_cast<int>(summary->denseCellEstimate);
  }
  if (summary->persistentMemoState && func->args().size() == 1 &&
      summary->stableScalarGlobals.size() <= 1 &&
      summary->monotonicStateProgress) {
    return kXXLargeMemoTableSize;
  }
  if ((summary->persistentMemoState || summary->repeatedSubproblem) &&
      func->args().size() <= 2) {
    return kLargeMemoTableSize;
  }
  return kDefaultMemoTableSize;
}

Value* buildStateFingerprint(IRBuilder& builder,
                             const std::vector<Value*>& stateValues) {
  if (stateValues.empty())
    return builder.ci(0);
  if (stateValues.size() == 1)
    return stateValues.front();
  Value* hash = builder.ci(static_cast<int32_t>(0x85ebca6bu));
  for (Value* stateValue : stateValues) {
    if (!stateValue)
      continue;
    Value* shl = builder.createShl(hash, builder.ci(6), "memo.statefp.shl");
    Value* shr = builder.createShr(hash, builder.ci(2), "memo.statefp.shr");
    Value* mix = builder.createAdd(stateValue, shl, "memo.statefp.mix0");
    mix = builder.createAdd(mix, shr, "memo.statefp.mix1");
    hash = builder.createXor(hash, mix, "memo.statefp.hash");
  }
  return hash;
}

std::vector<Value*> loadStableState(IRBuilder& builder,
                                    const MemoGlobals& globals,
                                    const std::string& stem) {
  std::vector<Value*> values;
  values.reserve(globals.stableScalarGlobals.size());
  for (const auto* global : globals.stableScalarGlobals) {
    values.push_back(builder.createLoad(const_cast<GlobalVar*>(global),
                                        Type::i32(), stem + ".cur"));
  }
  return values;
}

Value* compareStoredState(IRBuilder& builder, const MemoGlobals& globals,
                          const std::vector<Value*>& currentValues,
                          Value*& currentHashOut,
                          const std::string& stem) {
  if (!globals.stateHash ||
      globals.stateKeys.size() != currentValues.size()) {
    currentHashOut = builder.ci(0);
    return builder.ci(0);
  }
  currentHashOut = buildStateFingerprint(builder, currentValues);
  Value* storedHash = builder.createLoad(globals.stateHash, Type::i32(),
                                         stem + ".hash.old");
  Value* matches = builder.createICmp("eq", storedHash, currentHashOut,
                                      stem + ".hash.eq");
  // 指纹只承担快速筛选；命中还要逐项比较完整标量状态，两个上下文即使
  // 产生相同的 32 位指纹，也不会复用彼此的递归结果。
  for (size_t index = 0; index < currentValues.size(); ++index) {
    Value* storedValue = builder.createLoad(
        globals.stateKeys[index], Type::i32(),
        stem + ".key" + std::to_string(index) + ".old");
    Value* valueMatches = builder.createICmp(
        "eq", storedValue, currentValues[index],
        stem + ".key" + std::to_string(index) + ".eq");
    matches = builder.createAnd(
        matches, valueMatches,
        stem + ".key" + std::to_string(index) + ".and");
  }
  return matches;
}

void storeStableState(IRBuilder& builder, const MemoGlobals& globals,
                      const std::vector<Value*>& currentValues,
                      Value* currentHash) {
  if (!globals.stateHash ||
      globals.stateKeys.size() != currentValues.size() || !currentHash) {
    return;
  }
  builder.createStore(currentHash, globals.stateHash);
  for (size_t index = 0; index < currentValues.size(); ++index)
    builder.createStore(currentValues[index], globals.stateKeys[index]);
}

Value* buildPersistentHash(IRBuilder& builder,
                           const std::vector<Value*>& args,
                           int tableMask) {
  if (args.empty())
    return builder.ci(0);
  if (args.size() == 1) {
    return builder.createAnd(args.front(), builder.ci(tableMask), "memo.slot");
  }
  Value* hash = builder.ci(static_cast<int32_t>(0x9e3779b9u));
  for (Value* arg : args) {
    Value* mixedArg = builder.createAdd(
        arg, builder.ci(static_cast<int32_t>(0x7f4a7c15u)), "memo.argmix");
    Value* shl = builder.createShl(hash, builder.ci(6), "memo.hash.shl");
    Value* shr = builder.createShr(hash, builder.ci(2), "memo.hash.shr");
    Value* delta = builder.createAdd(mixedArg, shl, "memo.hash.delta0");
    delta = builder.createAdd(delta, shr, "memo.hash.delta1");
    hash = builder.createXor(hash, delta, "memo.hash");
  }
  return builder.createAnd(hash, builder.ci(tableMask), "memo.slot");
}

Value* buildHash(IRBuilder& builder, Function* func, const MemoGlobals& globals);

Value* buildDenseSlotForArgs(IRBuilder& builder,
                             const std::vector<Value*>& args,
                             const MemoGlobals& globals,
                             Value*& inBoundsOut) {
  if (globals.denseExtents.size() != args.size()) {
    inBoundsOut = builder.ci(0);
    return builder.ci(0);
  }

  Value* slot = nullptr;
  Value* inBounds = builder.ci(1);
  for (size_t i = 0; i < args.size(); ++i) {
    Value* shifted = args[i];
    if (globals.denseLowerBounds[i] != 0) {
      shifted = builder.createSub(
          shifted, builder.ci(globals.denseLowerBounds[i]),
          "memo.dense.shift" + std::to_string(i));
    }
    Value* lowerOk = builder.createICmp(
        "ge", shifted, builder.ci(0),
        "memo.dense.lower.ok" + std::to_string(i));
    Value* upperOk = builder.createICmp(
        "lt", shifted, builder.ci(globals.denseExtents[i]),
        "memo.dense.upper.ok" + std::to_string(i));
    Value* dimInBounds = builder.createAnd(
        lowerOk, upperOk, "memo.dense.inbounds" + std::to_string(i));
    inBounds = builder.createAnd(
        inBounds, dimInBounds,
        "memo.dense.inbounds.all" + std::to_string(i));
    if (!slot) {
      slot = shifted;
      continue;
    }
    slot = builder.createMul(slot, builder.ci(globals.denseExtents[i]),
                             "memo.dense.mul" + std::to_string(i));
    slot = builder.createAdd(slot, shifted,
                             "memo.dense.slot" + std::to_string(i));
  }
  if (!slot)
    slot = builder.ci(0);
  inBoundsOut = inBounds;
  return slot;
}

Value* buildDenseSlot(IRBuilder& builder,
                      Function* func,
                      const MemoGlobals& globals,
                      Value*& inBoundsOut) {
  if (!func) {
    inBoundsOut = builder.ci(0);
    return builder.ci(0);
  }
  std::vector<Value*> args;
  args.reserve(func->args().size());
  for (const auto& arg : func->args())
    args.push_back(arg.val);
  return buildDenseSlotForArgs(builder, args, globals, inBoundsOut);
}

Value* buildMemoSlot(IRBuilder& builder,
                     Function* func,
                     const MemoGlobals& globals,
                     Value*& inBoundsOut) {
  switch (globals.layout) {
  case analysis::RecursiveMemoLayout::Dense1D:
  case analysis::RecursiveMemoLayout::Dense2D:
    return buildDenseSlot(builder, func, globals, inBoundsOut);
  case analysis::RecursiveMemoLayout::HashFallback:
  case analysis::RecursiveMemoLayout::None:
  default:
    inBoundsOut = builder.ci(1);
    return buildHash(builder, func, globals);
  }
}

MemoGlobals createMemoGlobals(Module& mod, Function* func,
                              const analysis::RecursiveFunctionSummary* summary,
                              bool singleInvocation) {
  MemoGlobals globals;
  const std::string prefix = "__memo." + func->name();
  globals.persistentState = summary && summary->persistentMemoState;
  globals.singleInvocation = singleInvocation;
  globals.layout =
      summary ? summary->memoLayout : analysis::RecursiveMemoLayout::HashFallback;
  globals.tableEntries = chooseMemoTableEntries(func, summary);
  globals.tableMask = globals.tableEntries - 1;
  if (summary) {
    for (const GlobalVar* global : summary->stableScalarGlobals) {
      if (global && !isMemoGlobalName(global->name()))
        globals.stableScalarGlobals.push_back(global);
    }
  }
  globals.epoch = mod.newGlobal(prefix + ".epoch", Type::i32(), {}, 4,
                                /*isConst=*/false, /*hasInit=*/false);
  globals.depth = mod.newGlobal(prefix + ".depth", Type::i32(), {}, 4,
                                /*isConst=*/false, /*hasInit=*/false);
  if (globals.persistentState) {
    globals.stateHash = mod.newGlobal(prefix + ".state_hash", Type::i32(), {}, 4,
                                      /*isConst=*/false, /*hasInit=*/false);
    if (!(summary && summary->stableStateAcrossCalls)) {
      globals.stateKeys.reserve(globals.stableScalarGlobals.size());
      for (size_t index = 0; index < globals.stableScalarGlobals.size();
           ++index) {
        globals.stateKeys.push_back(mod.newGlobal(
            prefix + ".state_key" + std::to_string(index), Type::i32(), {}, 4,
            /*isConst=*/false, /*hasInit=*/false));
      }
    }
  }
  globals.stamp = mod.newGlobal(prefix + ".stamp", Type::i32(),
                                {globals.tableEntries}, globals.tableEntries * 4,
                                /*isConst=*/false, /*hasInit=*/false);
  globals.value = mod.newGlobal(prefix + ".value", Type::i32(),
                                {globals.tableEntries}, globals.tableEntries * 4,
                                /*isConst=*/false, /*hasInit=*/false);
  // 从 summary 恢复 dense 布局的下界/范围：优先用现成的 dense 参数，
  // 否则按 argumentRanges 推导，无法推导的布局直接清空退回 hash 索引。
  if (summary) {
    if ((globals.layout == analysis::RecursiveMemoLayout::Dense1D ||
         globals.layout == analysis::RecursiveMemoLayout::Dense2D) &&
        func &&
        summary->denseLowerBounds.size() == func->args().size() &&
        summary->denseExtents.size() == func->args().size()) {
      globals.denseLowerBounds = summary->denseLowerBounds;
      globals.denseExtents = summary->denseExtents;
    } else {
      globals.denseLowerBounds.reserve(summary->argumentRanges.size());
      globals.denseExtents.reserve(summary->argumentRanges.size());
      for (const auto& argRange : summary->argumentRanges) {
        if (argRange.valid) {
          globals.denseLowerBounds.push_back(argRange.minValue);
          globals.denseExtents.push_back(
              argRange.maxValue - argRange.minValue + 1);
          continue;
        }
        if (globals.layout == analysis::RecursiveMemoLayout::Dense1D &&
            func && func->args().size() == 1) {
          globals.denseLowerBounds.push_back(0);
          globals.denseExtents.push_back(globals.tableEntries);
          continue;
        }
        globals.denseLowerBounds.clear();
        globals.denseExtents.clear();
        break;
      }
    }
  }
  // HashFallback/None 布局下为每个参数建 key 数组，查表命中时需逐参数比对。
  if (globals.layout == analysis::RecursiveMemoLayout::HashFallback ||
      globals.layout == analysis::RecursiveMemoLayout::None) {
    globals.keys.reserve(func->args().size());
    for (size_t i = 0; i < func->args().size(); ++i) {
      globals.keys.push_back(mod.newGlobal(
          prefix + ".key" + std::to_string(i), Type::i32(),
          {globals.tableEntries}, globals.tableEntries * 4,
          /*isConst=*/false, /*hasInit=*/false));
    }
  }
  return globals;
}

Value* buildHash(IRBuilder& builder, Function* func, const MemoGlobals& globals) {
  std::vector<Value*> args;
  args.reserve(func->args().size());
  for (const auto& arg : func->args())
    args.push_back(arg.val);
  return buildPersistentHash(builder, args, globals.tableMask);
}

Value* buildSlotBytes(IRBuilder& builder, Value* slot) {
  return builder.createShl(slot, builder.ci(2), "memo.slot.bytes");
}

Value* loadTableEntry(IRBuilder& builder, GlobalVar* table, Value* slotBytes,
                      const std::string& stem) {
  auto* addr = builder.createGEP(table, {slotBytes}, stem + ".addr");
  return builder.createLoad(addr, Type::i32(), stem + ".load");
}

void storeTableEntry(IRBuilder& builder, GlobalVar* table, Value* slotBytes,
                     Value* value, const std::string& stem) {
  auto* addr = builder.createGEP(table, {slotBytes}, stem + ".addr");
  builder.createStore(value, addr);
}

bool shouldDeferFastPathToInlining(const Function* caller,
                                   const Function* callee) {
  if (!caller || !callee || caller == callee)
    return false;
  int instCount = 0;
  int callCount = 0;
  for (const auto& bbPtr : caller->blocks()) {
    for (const auto& instPtr : bbPtr->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      ++instCount;
      if (inst->opcode() == Opcode::Call)
        ++callCount;
      if (instCount > 32 || callCount > 2)
        return false;
    }
  }
  return callCount > 0;
}

void moveEntryInstructionsToBody(BasicBlock* entry, BasicBlock* body) {
  if (!entry || !body)
    return;
  std::vector<BasicBlock*> oldSuccs;
  if (auto* term = entry->terminator()) {
    if (term->opcode() == Opcode::Br && term->numOperands() == 1) {
      if (auto* succ = dynamic_cast<BasicBlock*>(term->operand(0)))
        oldSuccs.push_back(succ);
    } else if (term->opcode() == Opcode::CondBr && term->numOperands() == 3) {
      if (auto* succ = dynamic_cast<BasicBlock*>(term->operand(1)))
        oldSuccs.push_back(succ);
      if (auto* succ = dynamic_cast<BasicBlock*>(term->operand(2)))
        oldSuccs.push_back(succ);
    }
  }

  for (auto* succ : oldSuccs)
    replacePhiIncomingPred(succ, entry, body);

  auto& srcInsts = entry->instructions();
  auto& dstInsts = body->instructions();
  while (!srcInsts.empty()) {
    std::unique_ptr<Instruction> inst = std::move(srcInsts.front());
    srcInsts.erase(srcInsts.begin());
    inst->setParent(body);
    dstInsts.push_back(std::move(inst));
  }
}

std::vector<ReturnSite> collectAndRewriteReturns(Function* func,
                                                 BasicBlock* fillBB) {
  std::vector<ReturnSite> sites;
  if (!func || !fillBB)
    return sites;
  std::vector<BasicBlock*> blocks;
  blocks.reserve(func->blocks().size());
  for (auto& bbPtr : func->blocks())
    blocks.push_back(bbPtr.get());

  for (auto* bb : blocks) {
    Instruction* term = bb ? bb->terminator() : nullptr;
    if (!term || term->opcode() != Opcode::Ret || term->numOperands() != 1)
      continue;
    Value* retVal = term->operand(0);
    bb->remove(term);
    auto* br = makeInstruction(Opcode::Br, Type::voidTy());
    br->addOperand(fillBB);
    bb->pushBack(br);
    sites.push_back({bb, retVal});
  }
  return sites;
}

bool memoizeFunction(Module& mod, Function* func,
                     const analysis::RecursiveFunctionSummary* summary,
                     analysis::AnalysisManager& analysisManager) {
  if (!analysis::isMemoizableRecursiveFunction(summary, func))
    return false;
  if (!func || !func->entryBlock() || func->retTy() != Type::i32())
    return false;
  if (functionAlreadyMemoized(mod, func))
    return false;

  BasicBlock* entry = func->entryBlock();
  const bool singleInvocation =
      hasSingleExternalCall(mod, func, analysisManager);
  const bool fixedPersistentState =
      summary && summary->stableStateAcrossCalls;
  const bool constantEpoch = singleInvocation || fixedPersistentState;
  const bool resetEpochOnWrap =
      !singleInvocation && !fixedPersistentState;
  const bool splitPersistentContext = !constantEpoch;
  BasicBlock* innerContextBB = nullptr;
  BasicBlock* outerContextBB = nullptr;
  BasicBlock* outerEpochReadyBB = nullptr;
  BasicBlock* epochClearHeaderBB = nullptr;
  BasicBlock* epochClearBodyBB = nullptr;
  BasicBlock* epochClearDoneBB = nullptr;
  BasicBlock* probeBB = nullptr;
  if (splitPersistentContext) {
    innerContextBB = func->newBlock("memo.ctx.inner");
    outerContextBB = func->newBlock("memo.ctx.outer");
    if (resetEpochOnWrap) {
      outerEpochReadyBB = func->newBlock("memo.epoch.ready");
      epochClearHeaderBB = func->newBlock("memo.epoch.clear.header");
      epochClearBodyBB = func->newBlock("memo.epoch.clear.body");
      epochClearDoneBB = func->newBlock("memo.epoch.clear.done");
    }
    probeBB = func->newBlock("memo.probe");
  }
  BasicBlock* body = func->newBlock("memo.body");
  BasicBlock* hitBB = func->newBlock("memo.hit");
  BasicBlock* fillBB = func->newBlock("memo.fill");

  MemoGlobals globals =
      createMemoGlobals(mod, func, summary, singleInvocation);
  if (resetEpochOnWrap && sycEnvironmentValue("COREIR_RECURSION_TRACE")) {
    std::cerr << "[recursion-memo] function=" << func->name()
              << " epoch-wrap-reset entries=" << globals.tableEntries
              << "\n";
  }
  moveEntryInstructionsToBody(entry, body);
  std::vector<ReturnSite> returns = collectAndRewriteReturns(func, fillBB);
  if (returns.empty())
    return false;

  IRBuilder builder(mod, func);

  builder.setInsertPoint(entry);
  Value* slotInBounds = nullptr;
  Value* slot = buildMemoSlot(builder, func, globals, slotInBounds);
  Value* safeSlot = builder.createMul(slot, slotInBounds, "memo.slot.guarded");
  Value* slotBytes = buildSlotBytes(builder, safeSlot);
  Value* stamp = resetEpochOnWrap
                     ? nullptr
                     : loadTableEntry(builder, globals.stamp, slotBytes,
                                      "memo.stamp");
  Value* oldDepth = constantEpoch
                        ? nullptr
                        : builder.createLoad(globals.depth, Type::i32(),
                                             "memo.depth.old");
  Value* oldEpoch = constantEpoch
                        ? nullptr
                        : builder.createLoad(globals.epoch, Type::i32(),
                                             "memo.epoch.old");
  Value* activeEpoch =
      constantEpoch ? static_cast<Value*>(builder.ci(1)) : nullptr;
  if (constantEpoch) {
    // 单次调用或跨调用状态固定时，所有缓存项共用标记 1。
  } else if (innerContextBB && outerContextBB && probeBB) {
    Value* isInner =
        builder.createICmp("ne", oldDepth, builder.ci(0), "memo.is_inner");
    builder.createCondBr(isInner, innerContextBB, outerContextBB);

    builder.setInsertPoint(innerContextBB);
    builder.createBr(probeBB);

    builder.setInsertPoint(outerContextBB);
    Value* contextChanged = nullptr;
    Value* currentStateHash = nullptr;
    if (!globals.persistentState) {
      contextChanged = builder.ci(1);
    } else {
      Value* epochZero =
        builder.createICmp("eq", oldEpoch, builder.ci(0), "memo.epoch.zero");
      contextChanged = epochZero;
      if (!fixedPersistentState) {
        std::vector<Value*> stableStateValues =
            loadStableState(builder, globals, "memo.state");
        Value* stateMatches = compareStoredState(
            builder, globals, stableStateValues, currentStateHash,
            "memo.state");
        Value* stateChanged = builder.createICmp(
            "eq", stateMatches, builder.ci(0), "memo.state.changed");
        contextChanged = builder.createOr(
            epochZero, stateChanged, "memo.context.changed");
        storeStableState(builder, globals, stableStateValues,
                         currentStateHash);
      }
    }
    Value* outerEpoch =
        builder.createAdd(oldEpoch, contextChanged, "memo.epoch.outer");
    if (resetEpochOnWrap) {
      Value* wrapped = builder.createICmp(
          "eq", outerEpoch, builder.ci(0), "memo.epoch.wrapped");
      builder.createCondBr(wrapped, epochClearHeaderBB, outerEpochReadyBB);

      builder.setInsertPoint(outerEpochReadyBB);
      builder.createStore(outerEpoch, globals.epoch);
      builder.createBr(probeBB);

      // 0 保留为未初始化标记。计数回绕时清空 stamp 表并从 1 重新开始；
      // value/key 表无需清理，因为 stamp 为 0 的槽不会参与命中。
      builder.setInsertPoint(epochClearHeaderBB);
      auto* clearIndex = builder.createPhi(
          Type::i32(), {{builder.ci(0), outerContextBB}},
          "memo.epoch.clear.index");
      Value* clearMore = builder.createICmp(
          "lt", clearIndex, builder.ci(globals.tableEntries),
          "memo.epoch.clear.more");
      builder.createCondBr(clearMore, epochClearBodyBB, epochClearDoneBB);

      builder.setInsertPoint(epochClearBodyBB);
      Value* clearSlotBytes = buildSlotBytes(builder, clearIndex);
      storeTableEntry(builder, globals.stamp, clearSlotBytes, builder.ci(0),
                      "memo.epoch.clear.stamp");
      Value* nextClearIndex = builder.createAdd(
          clearIndex, builder.ci(1), "memo.epoch.clear.next");
      builder.createBr(epochClearHeaderBB);
      clearIndex->addOperand(nextClearIndex);
      clearIndex->addOperand(epochClearBodyBB);

      builder.setInsertPoint(epochClearDoneBB);
      builder.createStore(builder.ci(1), globals.epoch);
      builder.createBr(probeBB);
    } else {
      builder.createStore(outerEpoch, globals.epoch);
      builder.createBr(probeBB);
    }

  // probe 块的 epoch 递推：拆分持久上下文时在 phi 处合并内外路径的 epoch；
  // 否则在入口处直接递增，持久状态场景还要求状态指纹变化才推进 epoch。
    builder.setInsertPoint(probeBB);
    if (resetEpochOnWrap) {
      activeEpoch = builder.createPhi(
          Type::i32(),
          {{oldEpoch, innerContextBB},
           {outerEpoch, outerEpochReadyBB},
           {builder.ci(1), epochClearDoneBB}},
          "memo.epoch.active");
    } else {
      activeEpoch = builder.createPhi(
          Type::i32(),
          {{oldEpoch, innerContextBB}, {outerEpoch, outerContextBB}},
          "memo.epoch.active");
    }
  } else {
    Value* isOuter =
        builder.createICmp("eq", oldDepth, builder.ci(0), "memo.is_outer");
    if (globals.persistentState) {
      std::vector<Value*> stableStateValues =
          loadStableState(builder, globals, "memo.state");
      Value* currentStateHash = nullptr;
      Value* stateMatches = compareStoredState(
          builder, globals, stableStateValues, currentStateHash,
          "memo.state");
      Value* epochZero =
          builder.createICmp("eq", oldEpoch, builder.ci(0), "memo.epoch.zero");
      Value* stateChanged =
          builder.createICmp("eq", stateMatches, builder.ci(0),
                             "memo.state.changed");
      Value* contextChanged =
          builder.createOr(epochZero, stateChanged, "memo.context.changed");
      Value* needBump =
          builder.createAnd(isOuter, contextChanged, "memo.epoch.bump");
      activeEpoch = builder.createAdd(oldEpoch, needBump, "memo.epoch.new");
      builder.createStore(activeEpoch, globals.epoch);
      storeStableState(builder, globals, stableStateValues, currentStateHash);
    } else {
      activeEpoch = builder.createAdd(oldEpoch, isOuter, "memo.epoch.new");
      builder.createStore(activeEpoch, globals.epoch);
    }
  }
  if (!stamp)
    stamp = loadTableEntry(builder, globals.stamp, slotBytes, "memo.stamp");
  if (!constantEpoch) {
    Value* newDepth =
        builder.createAdd(oldDepth, builder.ci(1), "memo.depth.new");
    builder.createStore(newDepth, globals.depth);
  }
  // 命中判定：stamp == activeEpoch 且 slot 未越界；
  // HashFallback/None 布局还要求所有参数 key 逐一相等。
  Value* cacheHit =
      builder.createICmp("eq", stamp, activeEpoch, "memo.hit.stamp");
  cacheHit = builder.createAnd(cacheHit, slotInBounds, "memo.hit.inbounds");
  if (globals.layout == analysis::RecursiveMemoLayout::HashFallback ||
      globals.layout == analysis::RecursiveMemoLayout::None) {
    for (size_t i = 0; i < func->args().size(); ++i) {
      Value* cachedKey = loadTableEntry(
          builder, globals.keys[i], slotBytes,
          "memo.key" + std::to_string(i));
      Value* keyEq = builder.createICmp(
          "eq", cachedKey, func->args()[i].val,
          "memo.key.eq" + std::to_string(i));
      cacheHit = builder.createAnd(cacheHit, keyEq,
                                   "memo.hit.and" + std::to_string(i));
    }
  }
  Value* hitValueAddr =
      builder.createGEP(globals.value, {slotBytes}, "memo.value.addr");
  builder.createCondBr(cacheHit, hitBB, body);

  builder.setInsertPoint(hitBB);
  Value* hitValue =
      builder.createLoad(hitValueAddr, Type::i32(), "memo.value.load");
  if (!constantEpoch) {
    Value* hitDepth = builder.createSub(
        builder.createLoad(globals.depth, Type::i32(), "memo.depth.cur"),
        builder.ci(1), "memo.depth.hit");
    builder.createStore(hitDepth, globals.depth);
  }
  builder.createRet(hitValue);

  auto* resultPhi = makeInstruction(Opcode::Phi, Type::i32(), "memo.result");
  fillBB->pushFront(resultPhi);
  for (const auto& site : returns) {
    resultPhi->addOperand(site.value);
    resultPhi->addOperand(site.pred);
  }

  Instruction* insertPos = fillBB->terminator();
  if (insertPos)
    fillBB->remove(insertPos);

  builder.setInsertPoint(fillBB);
  Value* fillInBounds = nullptr;
  Value* fillSlot = buildMemoSlot(builder, func, globals, fillInBounds);
  Value* safeFillSlot =
      builder.createMul(fillSlot, fillInBounds, "memo.fill.slot.guarded");
  Value* fillSlotBytes = buildSlotBytes(builder, safeFillSlot);
  if (globals.layout == analysis::RecursiveMemoLayout::HashFallback ||
      globals.layout == analysis::RecursiveMemoLayout::None) {
    for (size_t i = 0; i < func->args().size(); ++i) {
      storeTableEntry(builder, globals.keys[i], fillSlotBytes,
                      func->args()[i].val,
                      "memo.fill.key" + std::to_string(i));
    }
  }
  auto* storeBB = func->newBlock("memo.store");
  auto* retBB = func->newBlock("memo.ret");
  builder.createCondBr(fillInBounds, storeBB, retBB);
  builder.setInsertPoint(storeBB);
  storeTableEntry(builder, globals.value, fillSlotBytes, resultPhi,
                  "memo.fill.value");
  Value* fillStamp =
      constantEpoch
          ? static_cast<Value*>(builder.ci(1))
          : builder.createLoad(globals.epoch, Type::i32(), "memo.fill.epoch");
  storeTableEntry(builder, globals.stamp, fillSlotBytes, fillStamp,
                  "memo.fill.stamp");
  builder.createBr(retBB);
  builder.setInsertPoint(retBB);
  if (!constantEpoch) {
    Value* fillDepth =
        builder.createLoad(globals.depth, Type::i32(), "memo.fill.depth");
    Value* nextDepth =
        builder.createSub(fillDepth, builder.ci(1), "memo.fill.depth.next");
    builder.createStore(nextDepth, globals.depth);
  }
  builder.createRet(resultPhi);

  func->recomputeCFG();
  return true;
}

// 按名字回收已物化的 __memo.* 全局变量并从 summary 恢复 dense 布局参数；
// 必需的 epoch/depth/stamp/value（或 key）缺失则返回 false。
bool collectMemoGlobals(
    Module& mod,
    Function* func,
    const analysis::RecursiveFunctionSummary* summary,
    MemoGlobals& globals,
    bool requirePersistentState) {
  if (!func)
    return false;
  const std::string prefix = "__memo." + func->name();
  globals.layout =
      summary && summary->memoLayout != analysis::RecursiveMemoLayout::None
          ? summary->memoLayout
          : analysis::RecursiveMemoLayout::HashFallback;
  globals.persistentState = findGlobalByName(mod, prefix + ".state_hash") != nullptr;
  if (requirePersistentState && !globals.persistentState)
    return false;
  globals.epoch = findGlobalByName(mod, prefix + ".epoch");
  globals.depth = findGlobalByName(mod, prefix + ".depth");
  if (globals.persistentState)
    globals.stateHash = findGlobalByName(mod, prefix + ".state_hash");
  globals.stamp = findGlobalByName(mod, prefix + ".stamp");
  globals.value = findGlobalByName(mod, prefix + ".value");
  if (!globals.epoch || !globals.depth || !globals.stamp || !globals.value)
    return false;
  if (globals.persistentState && !globals.stateHash)
    return false;
  globals.tableEntries = std::max(1, globals.stamp->sizeBytes() / 4);
  globals.tableMask = globals.tableEntries - 1;
  if (globals.layout == analysis::RecursiveMemoLayout::HashFallback) {
    for (size_t i = 0; i < func->args().size(); ++i) {
      auto* key = findGlobalByName(mod, prefix + ".key" + std::to_string(i));
      if (!key)
        return false;
      globals.keys.push_back(key);
    }
  }
  if (summary && (globals.layout == analysis::RecursiveMemoLayout::Dense1D ||
                  globals.layout == analysis::RecursiveMemoLayout::Dense2D)) {
    if (summary->denseLowerBounds.size() == func->args().size() &&
        summary->denseExtents.size() == func->args().size()) {
      globals.denseLowerBounds = summary->denseLowerBounds;
      globals.denseExtents = summary->denseExtents;
    } else {
      globals.denseLowerBounds.reserve(summary->argumentRanges.size());
      globals.denseExtents.reserve(summary->argumentRanges.size());
      for (const auto& argRange : summary->argumentRanges) {
        if (argRange.valid) {
          globals.denseLowerBounds.push_back(argRange.minValue);
          globals.denseExtents.push_back(
              argRange.maxValue - argRange.minValue + 1);
          continue;
        }
        if (globals.layout == analysis::RecursiveMemoLayout::Dense1D &&
            func->args().size() == 1) {
          globals.denseLowerBounds.push_back(0);
          globals.denseExtents.push_back(globals.tableEntries);
          continue;
        }
        return false;
      }
    }
  }
  if (summary && summary->persistentMemoState) {
    for (const GlobalVar* gv : summary->stableScalarGlobals) {
      if (!gv || isMemoGlobalName(gv->name()))
        continue;
      globals.stableScalarGlobals.push_back(gv);
    }
  }
  if (globals.persistentState && globals.stableScalarGlobals.empty())
    globals.stableScalarGlobals = inferStableScalarGlobals(func);
  if (globals.persistentState &&
      !(summary && summary->stableStateAcrossCalls)) {
    globals.stateKeys.reserve(globals.stableScalarGlobals.size());
    for (size_t index = 0; index < globals.stableScalarGlobals.size(); ++index) {
      GlobalVar* stateKey = findGlobalByName(
          mod, prefix + ".state_key" + std::to_string(index));
      if (!stateKey)
        return false;
      globals.stateKeys.push_back(stateKey);
    }
  }
  return true;
}

bool bypassPersistentMemoCall(Module& mod,
                              BasicBlock* callerBB,
                              Instruction* call,
                              Function* callee,
                              const analysis::RecursiveFunctionSummary* summary) {
  if (!callerBB || !call || !callee || call->opcode() != Opcode::Call)
    return false;
  Function* callerFunc = callerBB->parent();
  if (!callerFunc)
    return false;
  MemoGlobals globals;
  if (!collectMemoGlobals(
          mod, callee, summary, globals,
          /*requirePersistentState=*/true))
    return false;
  if (!globals.persistentState)
    return false;
  if (shouldDeferFastPathToInlining(callerFunc, callee))
    return false;

  auto& callerInsts = callerBB->instructions();
  auto callIt = std::find_if(callerInsts.begin(), callerInsts.end(),
                             [call](const auto& instPtr) {
                               return instPtr.get() == call;
                             });
  if (callIt == callerInsts.end())
    return false;
  auto moveBegin = std::next(callIt);
  if (moveBegin == callerInsts.end())
    return false;

  std::vector<Value*> callArgs;
  callArgs.reserve(call->numOperands());
  for (int i = 0; i < call->numOperands(); ++i)
    callArgs.push_back(call->operand(i));

  auto* hitBB = callerFunc->newBlock("memo.fast.hit");
  auto* missBB = callerFunc->newBlock("memo.fast.miss");
  auto* afterBB = callerFunc->newBlock("memo.fast.after");

  std::vector<std::unique_ptr<Instruction>> tailInsts;
  for (auto it = moveBegin; it != callerInsts.end(); ++it) {
    (*it)->setParent(afterBB);
    tailInsts.push_back(std::move(*it));
  }
  callerInsts.erase(moveBegin, callerInsts.end());
  for (auto& instPtr : tailInsts)
    afterBB->instructions().push_back(std::move(instPtr));

  if (Instruction* term = afterBB->terminator()) {
    for (int opIndex = 0; opIndex < term->numOperands(); ++opIndex) {
      auto* succ = dynamic_cast<BasicBlock*>(term->operand(opIndex));
      if (succ)
        replacePhiIncomingPred(succ, callerBB, afterBB);
    }
  }

  IRBuilder builder(mod, callerFunc);
  builder.setInsertPoint(callerBB);
  const bool activeRecursiveContext = callerFunc == callee;
  const bool fixedPersistentState =
      summary && summary->stableStateAcrossCalls;
  Value* oldEpoch = fixedPersistentState
                        ? static_cast<Value*>(builder.ci(1))
                        : builder.createLoad(globals.epoch, Type::i32(),
                                             "memo.fast.epoch.old");
  Value* contextMatches = builder.ci(1);
  if (!activeRecursiveContext && !fixedPersistentState) {
    contextMatches =
        builder.createICmp("ne", oldEpoch, builder.ci(0), "memo.fast.epoch.live");
  }
  if (globals.persistentState && !activeRecursiveContext &&
      !fixedPersistentState) {
    std::vector<Value*> stableStateValues =
        loadStableState(builder, globals, "memo.fast.state");
    Value* stateHash = nullptr;
    Value* stateMatches = compareStoredState(
        builder, globals, stableStateValues, stateHash, "memo.fast.state");
    contextMatches =
        builder.createAnd(contextMatches, stateMatches, "memo.fast.context.eq");
  }

  Value* slotInBounds = builder.ci(1);
  Value* slot =
      globals.layout == analysis::RecursiveMemoLayout::Dense1D ||
              globals.layout == analysis::RecursiveMemoLayout::Dense2D
          ? buildDenseSlotForArgs(builder, callArgs, globals, slotInBounds)
          : buildPersistentHash(builder, callArgs, globals.tableMask);
  Value* safeSlot = builder.createMul(slot, slotInBounds, "memo.fast.slot.guarded");
  Value* slotBytes = buildSlotBytes(builder, safeSlot);
  Value* stamp = loadTableEntry(builder, globals.stamp, slotBytes, "memo.fast.stamp");
  Value* cacheHit =
      builder.createICmp("eq", stamp, oldEpoch, "memo.fast.hit.stamp");
  cacheHit = builder.createAnd(cacheHit, contextMatches, "memo.fast.hit");
  cacheHit =
      builder.createAnd(cacheHit, slotInBounds, "memo.fast.hit.inbounds");
  if (globals.layout == analysis::RecursiveMemoLayout::HashFallback ||
      globals.layout == analysis::RecursiveMemoLayout::None) {
    for (size_t i = 0; i < callArgs.size(); ++i) {
      Value* cachedKey = loadTableEntry(builder, globals.keys[i], slotBytes,
                                        "memo.fast.key" + std::to_string(i));
      Value* keyEq = builder.createICmp("eq", cachedKey, callArgs[i],
                                        "memo.fast.eq" + std::to_string(i));
      cacheHit = builder.createAnd(cacheHit, keyEq,
                                   "memo.fast.and" + std::to_string(i));
    }
  }
  Value* hitValueAddr =
      builder.createGEP(globals.value, {slotBytes}, "memo.fast.value.addr");
  builder.createCondBr(cacheHit, hitBB, missBB);

  builder.setInsertPoint(hitBB);
  Value* hitValue =
      builder.createLoad(hitValueAddr, call->type(), "memo.fast.value.load");
  builder.createBr(afterBB);

  builder.setInsertPoint(missBB);
  Value* missValue =
      builder.createCall(callee->name(), call->type(), callArgs, call->name());
  builder.createBr(afterBB);

  if (!call->type().isVoid()) {
    auto* resultPhi = new Instruction(Opcode::Phi, call->type(), "memo.fast.result");
    resultPhi->addOperand(hitValue);
    resultPhi->addOperand(hitBB);
    resultPhi->addOperand(missValue);
    resultPhi->addOperand(missBB);
    afterBB->pushFront(resultPhi);
    call->replaceAllUsesWith(resultPhi);
  }
  callerBB->remove(call);
  callerFunc->recomputeCFG();
  return true;
}

bool applyPersistentMemoCallBypass(
    Module& mod,
    const analysis::RecursiveFunctionMap& recursiveSummaries,
    analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    std::vector<std::pair<BasicBlock*, Instruction*>> calls;
    for (auto& bbPtr : func->blocks()) {
      BasicBlock* bb = bbPtr.get();
      for (auto& instPtr : bb->instructions()) {
        Instruction* inst = instPtr.get();
        if (inst->opcode() != Opcode::Call)
          continue;
        Function* callee = mod.getFunction(inst->name());
        if (!callee ||
            (callee == func &&
             bb->name().find("memo.fast.") != std::string::npos))
          continue;
        calls.push_back({bb, inst});
      }
    }
    for (const auto& [bb, call] : calls) {
      if (!bb || !call || call->parent() != bb)
        continue;
      Function* callee = mod.getFunction(call->name());
      if (!callee)
        continue;
      // 单次外部调用会在整个调用期间使用固定标记；多调用函数的快路径
      // 根据状态摘要选择固定标记或可变 epoch。
      if (hasSingleExternalCall(mod, callee, analysisManager))
        continue;
      changed |= bypassPersistentMemoCall(
          mod, bb, call, callee,
          analysis::lookupRecursiveSummary(recursiveSummaries, callee));
    }
  }
  return changed;
}

void traceRecursiveSummary(Function* func,
                           const analysis::RecursiveFunctionSummary* summary) {
  if (!func || !summary || !sycEnvironmentValue("COREIR_RECURSION_TRACE"))
    return;
  std::cerr << "[recursion] " << func->name()
            << " mode=" << analysis::recursiveOptimizationModeName(summary->mode)
            << " layout="
            << analysis::recursiveMemoLayoutName(summary->memoLayout)
            << " selfRecursive=" << summary->selfRecursive
            << " finiteStateI32Tuple=" << summary->finiteStateI32Tuple
            << " readonly=" << summary->readonly
            << " stableInvocationState=" << summary->stableInvocationState
            << " repeatedSubproblem=" << summary->repeatedSubproblem
            << " persistentMemoState=" << summary->persistentMemoState
            << " stableStateAcrossCalls=" << summary->stableStateAcrossCalls
            << " tailRecursiveOnly=" << summary->tailRecursiveOnly
            << " recursiveBranchCount=" << summary->recursiveBranchCount
            << " selfRecursiveCallCount=" << summary->selfRecursiveCallCount
            << " monotonicStateProgress=" << summary->monotonicStateProgress
            << " stableScalarGlobals=" << summary->stableScalarGlobals.size()
            << " denseCells=" << summary->denseCellEstimate
            << " denseBytes=" << summary->denseByteEstimate
            << "\n";
}

} // namespace

bool applyRecursiveMemoization(Module& mod) {
  analysis::AnalysisManager analysisManager(mod);
  return applyRecursiveMemoization(mod, analysisManager);
}

bool applyRecursiveMemoization(Module& mod,
                               analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (int iter = 0; iter < 2; ++iter) {
    const auto& recursiveSummaries = analysisManager.recursiveFunctions();
    bool iterChanged = false;
    for (auto& funcPtr : mod.functions()) {
      const auto* summary =
          analysis::lookupRecursiveSummary(recursiveSummaries, funcPtr.get());
      traceRecursiveSummary(funcPtr.get(), summary);
      iterChanged |=
          memoizeFunction(mod, funcPtr.get(), summary, analysisManager);
    }
    changed |= iterChanged;
    if (!iterChanged)
      break;
    // memoization 会插入全局缓存对象并重写 CFG，下一轮固定点必须
    // 基于新的模块形状重新计算 effect/recursive summary。
    analysisManager.invalidateModule(analysis::AnalysisInvalidation::Module);
  }
  const auto& recursiveSummaries = analysisManager.recursiveFunctions();
  changed |= applyPersistentMemoCallBypass(mod, recursiveSummaries,
                                           analysisManager);
  if (changed)
    analysisManager.invalidateModule(analysis::AnalysisInvalidation::Effects);
  return changed;
}

analysis::RecursiveOptimizationMode classifyRecursiveOptimizationMode(
    Module& mod,
    Function* func) {
  analysis::AnalysisManager analysisManager(mod);
  return classifyRecursiveOptimizationMode(mod, func, analysisManager);
}

analysis::RecursiveOptimizationMode classifyRecursiveOptimizationMode(
    Module& mod,
    Function* func,
    analysis::AnalysisManager& analysisManager) {
  if (!func)
    return analysis::RecursiveOptimizationMode::None;
  if (functionAlreadyMemoized(mod, func))
    return analysis::RecursiveOptimizationMode::Memoized;
  const auto& recursiveSummaries = analysisManager.recursiveFunctions();
  const auto* summary = analysis::lookupRecursiveSummary(recursiveSummaries, func);
  traceRecursiveSummary(func, summary);
  return summary ? summary->mode : analysis::RecursiveOptimizationMode::None;
}

} // namespace hir
