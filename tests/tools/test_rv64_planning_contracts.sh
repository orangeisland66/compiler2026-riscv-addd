#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build-plan}"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/rv64-planning-contracts.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

mapfile -t OBJECTS < <(
  find "$BUILD_DIR/CMakeFiles/compiler.dir/src" -type f -name '*.o' \
    ! -path '*/main.cpp.o' | sort
)
if [[ ${#OBJECTS[@]} -eq 0 ]]; then
  echo "build compiler first: $BUILD_DIR" >&2
  exit 1
fi

# 仓库仍只有 src/main.cpp 一个正式入口；此 helper 只在临时目录生成和链接。
cat >"$TMP/contracts.cpp" <<'CPP'
#include "analysis/alias_analysis.h"
#include "analysis/analysis_manager.h"
#include "analysis/integer_facts.h"
#include "analysis/function_modref.h"
#include "analysis/observer_chain_plan.h"
#include "analysis/ordered_region_effect.h"
#include "analysis/region_write_coverage.h"
#include "analysis/region_state_equivalence.h"
#include "backend_mir/rv64_admission.h"
#include "backend_mir/rv64_cost_model.h"
#include "core_ir/planning/candidate_plan.h"
#include "core_ir/planning/region_deletion.h"
#include "core_ir/passes/cleanup.h"
#include "core_ir/passes/verify.h"

#include <algorithm>
#include <cassert>
#include <sstream>

using namespace core_ir;

static std::string snapshot(const Module& module) {
  std::ostringstream output;
  printModule(output, module);
  return output.str();
}

int main() {
  const auto& target = mir::defaultRV64GCTarget();
  assert(target.hasI && target.hasM && target.hasA && target.hasF &&
         target.hasD && target.hasC && !target.hasV);
  assert(target.stackAlignment == 16);
  assert(target.issueWidth == 1 && target.loadUseLatency == 0);
  const auto& boom =
      mir::rv64TargetProfile(mir::RV64TargetProfileKind::BoomRV64GC);
  assert(boom.hasI && boom.hasM && boom.hasA && boom.hasF && boom.hasD &&
         boom.hasC && !boom.hasV);
  assert(boom.issueWidth == 2 && boom.loadUseLatency == 0);
  assert(!boom.costsCalibratedForSpecificCPU);
  assert(mir::rv64InstructionCost(mir::MIOp::DIVW).latency >=
         mir::rv64InstructionCost(mir::MIOp::ADDW).latency);

  Module module;
  module.declareBuiltin("putint", Type::voidTy(), {Type::i32()});
  auto* function = module.newFunction("contract", Type::i32());
  auto* entry = function->newBlock("entry");
  auto* exit = function->newBlock("exit");
  function->setEntryBlock(entry);
  IRBuilder builder(module, function);
  builder.setInsertPoint(entry);
  auto* object = builder.createAlloca(Type::i32(), 8, true, {2}, "object");
  builder.createStore(builder.ci(11), object);
  auto* second = builder.createGEP(object, {builder.ci(1)}, "second");
  builder.createStore(builder.ci(13), second);
  auto* sum = builder.createAdd(builder.ci(11), builder.ci(13), "sum");
  builder.createBr(exit);
  builder.setInsertPoint(exit);
  builder.createRet(sum);
  function->recomputeCFG();
  assert(verifyModuleChecked(module).ok);

  auto clone = planning::cloneModuleForPlanning(module);
  assert(clone->module().isBuiltin("putint"));
  assert(clone->module().builtinParamTys("putint").size() == 1);
  assert(verifyModuleChecked(clone->module()).ok);

  analysis::AnalysisManager manager(module);
  assert(manager.memorySSA(function).proof.proved());
  assert(manager.functionSemantics(function).proof.proved());
  std::ostringstream dump;
  manager.dump(dump);
  assert(dump.str().find("memory-proof=memoryssa-lite-built") !=
         std::string::npos);

  assert(analysis::queryAlias(object, 4, second, 4).kind ==
         analysis::AliasKind::NoAlias);
  const auto exactInteger = analysis::evaluateIntegerFacts(sum);
  assert(exactInteger.proof.proved() && exactInteger.rangeKnown);
  assert(exactInteger.minimum == 24 && exactInteger.maximum == 24);
  auto* mayWrap = new Instruction(Opcode::Add, Type::i32(), "may-wrap");
  mayWrap->addOperand(module.getConstInt(2147483647));
  mayWrap->addOperand(module.getConstInt(1));
  const auto unknownInteger = analysis::evaluateIntegerFacts(mayWrap);
  assert(unknownInteger.proof.state == analysis::ProofState::Unknown);
  assert(unknownInteger.proof.reasonCode == "integer-arithmetic-may-wrap");
  mayWrap->dropAllOperands();
  delete mayWrap;
  auto* masked = new Instruction(Opcode::And, Type::i32(), "masked");
  masked->addOperand(module.getConstInt(-1));
  masked->addOperand(module.getConstInt(15));
  const auto maskedInteger = analysis::evaluateIntegerFacts(masked);
  assert(maskedInteger.proof.proved() && maskedInteger.nonNegative);
  assert(maskedInteger.minimum == 0 && maskedInteger.maximum == 15);
  masked->dropAllOperands();
  delete masked;
  std::unordered_set<BasicBlock*> region{entry};
  const auto ordered = analysis::analyzeOrderedRegionEffects(
      function, region, &manager.functionEffects());
  assert(ordered.proof.proved());
  const auto coverage = analysis::analyzeRegionWriteCoverage(ordered);
  assert(coverage.proof.proved());
  assert(coverage.objects.size() == 1);
  assert(coverage.objects.front().kind ==
         analysis::RegionCoverageKind::Complete);
  assert(analysis::compareRegionStateEffects(ordered, ordered).proof.proved());
  const auto observers = analysis::buildObserverChainPlan(
      {sum}, region, &manager.functionEffects());
  assert(observers.proof.proved());
  assert(observers.escapesFunction && observers.observers.size() == 1);
  const auto demandSlice =
      analysis::buildRegionDemandSlice(observers.observers, region, &ordered);
  assert(demandSlice.proof.proved());
  assert(demandSlice.instructions.count(sum));
  assert(demandSlice.effectInstructions.size() == ordered.events.size());
  assert(demandSlice.memoryObjects.count(object));
  const auto valueEpoch = manager.functionEpoch(function);
  const auto valueModuleEpoch = manager.moduleEpoch();
  manager.invalidateFunction(function, analysis::AnalysisInvalidation::Values);
  assert(manager.functionEpoch(function) == valueEpoch + 1);
  assert(manager.moduleEpoch() == valueModuleEpoch);

  // 在 region coverage/observer 合同完成后再插入版本查询夹具，避免这些
  // 验证用 load 本身改变上面的观察者闭包。
  auto* terminator = entry->terminator();
  auto* firstLoad = new Instruction(Opcode::Load, Type::i32(), "first-load");
  firstLoad->addOperand(object);
  entry->insertBefore(terminator, firstLoad);
  auto* disjointStore = new Instruction(Opcode::Store, Type::voidTy());
  disjointStore->addOperand(module.getConstInt(17));
  disjointStore->addOperand(second);
  entry->insertBefore(terminator, disjointStore);
  auto* secondLoad = new Instruction(Opcode::Load, Type::i32(), "second-load");
  secondLoad->addOperand(object);
  entry->insertBefore(terminator, secondLoad);
  manager.invalidateFunction(function, analysis::AnalysisInvalidation::CFG);
  const auto& partitionedMemory = manager.memorySSA(function);
  assert(partitionedMemory.partitions.size() == 2);
  assert(partitionedMemory.partitionForInstruction.at(firstLoad) ==
         partitionedMemory.partitionForInstruction.at(secondLoad));
  assert(analysis::sameMemoryVersion(partitionedMemory, firstLoad, secondLoad,
                                     &manager.functionEffects()).proved());
  assert(analysis::clobberBefore(partitionedMemory, firstLoad) >= 0);
  const auto memoryModuleEpoch = manager.moduleEpoch();
  manager.invalidateFunction(function, analysis::AnalysisInvalidation::Memory);
  assert(manager.moduleEpoch() == memoryModuleEpoch + 1);
  assert(manager.memorySSA(function).proof.proved());

  Module memoryQueryModule;
  auto* memoryQueryFunction =
      memoryQueryModule.newFunction("memory_query_contract", Type::i32());
  auto* memoryQueryRuntimeValue =
      memoryQueryFunction->addArg("runtime_value", Type::i32());
  auto* memoryQueryEntry = memoryQueryFunction->newBlock("entry");
  memoryQueryFunction->setEntryBlock(memoryQueryEntry);
  IRBuilder memoryQueryBuilder(memoryQueryModule, memoryQueryFunction);
  memoryQueryBuilder.setInsertPoint(memoryQueryEntry);
  auto* memoryQueryObject =
      memoryQueryBuilder.createAlloca(Type::i32(), 8, true, {2}, "object");
  auto* memoryQueryFirstAddress = memoryQueryBuilder.createGEP(
      memoryQueryObject, {memoryQueryBuilder.ci(0)}, "first-address");
  auto* memoryQueryFirstStore = memoryQueryBuilder.createStore(
      memoryQueryRuntimeValue, memoryQueryFirstAddress);
  auto* memoryQueryDisjointAddress = memoryQueryBuilder.createGEP(
      memoryQueryObject, {memoryQueryBuilder.ci(1)}, "disjoint-address");
  memoryQueryBuilder.createStore(memoryQueryBuilder.ci(37),
                                 memoryQueryDisjointAddress);
  auto* memoryQueryEquivalentAddress = memoryQueryBuilder.createGEP(
      memoryQueryObject, {memoryQueryBuilder.ci(0)}, "equivalent-address");
  auto* memoryQueryLoad =
      memoryQueryBuilder.createLoad(memoryQueryEquivalentAddress, "load");
  auto* memoryQueryObservedStore = memoryQueryBuilder.createStore(
      memoryQueryBuilder.ci(41), memoryQueryEquivalentAddress);
  auto* memoryQueryFinalStore = memoryQueryBuilder.createStore(
      memoryQueryBuilder.ci(43), memoryQueryFirstAddress);
  auto* memoryQueryLoadBeforeClobber =
      memoryQueryBuilder.createLoad(memoryQueryFirstAddress, "before-clobber");
  memoryQueryBuilder.createStore(memoryQueryBuilder.ci(47),
                                 memoryQueryEquivalentAddress);
  auto* memoryQueryStoreAfterClobber = memoryQueryBuilder.createStore(
      memoryQueryLoadBeforeClobber, memoryQueryFirstAddress);
  memoryQueryBuilder.createRet(memoryQueryLoad);
  memoryQueryFunction->recomputeCFG();
  analysis::AnalysisManager memoryQueryManager(memoryQueryModule);
  assert(analysis::storedValueReachesLoad(
             memoryQueryFirstStore, memoryQueryLoad,
             &memoryQueryManager.functionEffects()).proved());
  assert(analysis::memoryValueReachesStore(
             memoryQueryLoad, memoryQueryObservedStore,
             &memoryQueryManager.functionEffects()).proved());
  assert(analysis::mustOverwrite(memoryQueryFirstStore,
                                 memoryQueryObservedStore).proved());
  assert(analysis::hasObserverBetween(
             memoryQueryFirstStore, memoryQueryObservedStore,
             &memoryQueryManager.functionEffects()).proved());
  assert(analysis::mustOverwrite(memoryQueryObservedStore,
                                 memoryQueryFinalStore).proved());
  assert(analysis::hasObserverBetween(
             memoryQueryObservedStore, memoryQueryFinalStore,
             &memoryQueryManager.functionEffects()).state ==
         analysis::ProofState::Disproved);
  assert(analysis::memoryValueReachesStore(
             memoryQueryLoadBeforeClobber, memoryQueryStoreAfterClobber,
             &memoryQueryManager.functionEffects()).state ==
         analysis::ProofState::Unknown);

  // 跨块版本查询接受无回边的完整 CFG 路径证明；运行时值不参与判定。
  // diamond 的每条路径都无相关写入时可证明，任一分支写入则必须 Unknown。
  Module crossBlockMemoryModule;
  auto* crossBlockFunction = crossBlockMemoryModule.newFunction(
      "cross_block_memory_contract", Type::i32());
  auto* crossBlockRuntimeValue =
      crossBlockFunction->addArg("runtime_value", Type::i32());
  auto* crossBlockEntry = crossBlockFunction->newBlock("entry");
  auto* crossBlockMiddle = crossBlockFunction->newBlock("middle");
  auto* crossBlockExit = crossBlockFunction->newBlock("exit");
  crossBlockFunction->setEntryBlock(crossBlockEntry);
  IRBuilder crossBlockBuilder(crossBlockMemoryModule, crossBlockFunction);
  crossBlockBuilder.setInsertPoint(crossBlockEntry);
  auto* crossBlockObject =
      crossBlockBuilder.createAlloca(Type::i32(), 4, false, {}, "object");
  auto* crossBlockFirstStore =
      crossBlockBuilder.createStore(crossBlockRuntimeValue, crossBlockObject);
  crossBlockBuilder.createBr(crossBlockMiddle);
  crossBlockBuilder.setInsertPoint(crossBlockMiddle);
  auto* crossBlockFirstLoad =
      crossBlockBuilder.createLoad(crossBlockObject, "first-load");
  crossBlockBuilder.createBr(crossBlockExit);
  crossBlockBuilder.setInsertPoint(crossBlockExit);
  auto* crossBlockSecondLoad =
      crossBlockBuilder.createLoad(crossBlockObject, "second-load");
  auto* crossBlockFinalStore =
      crossBlockBuilder.createStore(crossBlockRuntimeValue, crossBlockObject);
  crossBlockBuilder.createRet(crossBlockSecondLoad);
  crossBlockFunction->recomputeCFG();
  analysis::AnalysisManager crossBlockManager(crossBlockMemoryModule);
  const auto& crossBlockEffects = crossBlockManager.functionEffects();
  const auto& crossBlockMemory = crossBlockManager.memorySSA(crossBlockFunction);
  assert(analysis::storedValueReachesLoad(
             crossBlockFirstStore, crossBlockFirstLoad,
             &crossBlockEffects).proved());
  assert(analysis::sameMemoryVersion(
             crossBlockMemory, crossBlockFirstLoad, crossBlockSecondLoad,
             &crossBlockEffects).proved());
  assert(analysis::mustOverwrite(crossBlockFirstStore,
                                 crossBlockFinalStore).proved());
  assert(analysis::hasObserverBetween(
             crossBlockFirstStore, crossBlockFinalStore,
             &crossBlockEffects).proved());

  Module diamondMemoryModule;
  auto* diamondFunction =
      diamondMemoryModule.newFunction("diamond_memory_contract", Type::i32());
  auto* diamondCondition = diamondFunction->addArg("condition", Type::i32());
  auto* diamondEntry = diamondFunction->newBlock("entry");
  auto* diamondLeft = diamondFunction->newBlock("left");
  auto* diamondRight = diamondFunction->newBlock("right");
  auto* diamondMerge = diamondFunction->newBlock("merge");
  diamondFunction->setEntryBlock(diamondEntry);
  IRBuilder diamondBuilder(diamondMemoryModule, diamondFunction);
  diamondBuilder.setInsertPoint(diamondEntry);
  auto* diamondObject =
      diamondBuilder.createAlloca(Type::i32(), 4, false, {}, "object");
  auto* diamondFirstStore =
      diamondBuilder.createStore(diamondBuilder.ci(1), diamondObject);
  auto* diamondBranchCondition = diamondBuilder.createICmp(
      "ne", diamondCondition, diamondBuilder.ci(0), "condition");
  diamondBuilder.createCondBr(diamondBranchCondition, diamondLeft,
                              diamondRight);
  diamondBuilder.setInsertPoint(diamondLeft);
  diamondBuilder.createBr(diamondMerge);
  diamondBuilder.setInsertPoint(diamondRight);
  diamondBuilder.createBr(diamondMerge);
  diamondBuilder.setInsertPoint(diamondMerge);
  auto* diamondFinalStore =
      diamondBuilder.createStore(diamondBuilder.ci(2), diamondObject);
  diamondBuilder.createRet(diamondBuilder.ci(0));
  auto* diamondFirstLoad = new Instruction(
      Opcode::Load, Type::i32(), "diamond-first-load");
  diamondFirstLoad->addOperand(diamondObject);
  diamondEntry->insertBefore(diamondEntry->terminator(), diamondFirstLoad);
  auto* diamondSecondLoad = new Instruction(
      Opcode::Load, Type::i32(), "diamond-second-load");
  diamondSecondLoad->addOperand(diamondObject);
  diamondMerge->insertBefore(diamondFinalStore, diamondSecondLoad);
  diamondFunction->recomputeCFG();
  analysis::AnalysisManager diamondManager(diamondMemoryModule);
  assert(analysis::sameMemoryVersionAcrossCFG(
             diamondManager.memorySSA(diamondFunction), diamondFirstLoad,
             diamondSecondLoad, &diamondManager.functionEffects()).proved());
  assert(analysis::mustOverwrite(diamondFirstStore, diamondFinalStore).state ==
         analysis::ProofState::Unknown);
  assert(analysis::hasObserverBetween(diamondFirstStore, diamondFinalStore)
             .state == analysis::ProofState::Unknown);

  Module diamondClobberModule;
  auto* diamondClobberFunction = diamondClobberModule.newFunction(
      "diamond_clobber_contract", Type::i32());
  auto* diamondClobberCondition =
      diamondClobberFunction->addArg("condition", Type::i32());
  auto* diamondClobberEntry = diamondClobberFunction->newBlock("entry");
  auto* diamondClobberLeft = diamondClobberFunction->newBlock("left");
  auto* diamondClobberRight = diamondClobberFunction->newBlock("right");
  auto* diamondClobberMerge = diamondClobberFunction->newBlock("merge");
  diamondClobberFunction->setEntryBlock(diamondClobberEntry);
  IRBuilder diamondClobberBuilder(diamondClobberModule,
                                  diamondClobberFunction);
  diamondClobberBuilder.setInsertPoint(diamondClobberEntry);
  auto* diamondClobberObject = diamondClobberBuilder.createAlloca(
      Type::i32(), 4, false, {}, "object");
  auto* diamondClobberFirstLoad =
      diamondClobberBuilder.createLoad(diamondClobberObject, "first-load");
  auto* diamondClobberBranchCondition = diamondClobberBuilder.createICmp(
      "ne", diamondClobberCondition, diamondClobberBuilder.ci(0),
      "condition");
  diamondClobberBuilder.createCondBr(diamondClobberBranchCondition,
                                     diamondClobberLeft,
                                     diamondClobberRight);
  diamondClobberBuilder.setInsertPoint(diamondClobberLeft);
  diamondClobberBuilder.createStore(diamondClobberBuilder.ci(7),
                                    diamondClobberObject);
  diamondClobberBuilder.createBr(diamondClobberMerge);
  diamondClobberBuilder.setInsertPoint(diamondClobberRight);
  diamondClobberBuilder.createBr(diamondClobberMerge);
  diamondClobberBuilder.setInsertPoint(diamondClobberMerge);
  auto* diamondClobberSecondLoad =
      diamondClobberBuilder.createLoad(diamondClobberObject, "second-load");
  diamondClobberBuilder.createRet(diamondClobberSecondLoad);
  diamondClobberFunction->recomputeCFG();
  analysis::AnalysisManager diamondClobberManager(diamondClobberModule);
  assert(analysis::sameMemoryVersionAcrossCFG(
             diamondClobberManager.memorySSA(diamondClobberFunction),
             diamondClobberFirstLoad, diamondClobberSecondLoad,
             &diamondClobberManager.functionEffects()).state ==
         analysis::ProofState::Unknown);

  Module modRefModule;
  auto* modRefCallee =
      modRefModule.newFunction("modref_contract_callee", Type::voidTy());
  auto* modRefReadArgument =
      modRefCallee->addArg("read_pointer", Type::ptr());
  auto* modRefWriteArgument =
      modRefCallee->addArg("write_pointer", Type::ptr());
  auto* modRefCalleeEntry = modRefCallee->newBlock("entry");
  modRefCallee->setEntryBlock(modRefCalleeEntry);
  IRBuilder modRefCalleeBuilder(modRefModule, modRefCallee);
  modRefCalleeBuilder.setInsertPoint(modRefCalleeEntry);
  auto* modRefRead =
      modRefCalleeBuilder.createLoad(modRefReadArgument, "read");
  modRefCalleeBuilder.createStore(modRefRead, modRefWriteArgument);
  modRefCalleeBuilder.createRet();
  modRefCallee->recomputeCFG();

  auto* returnedPointerFunction =
      modRefModule.newFunction("returned_pointer_contract", Type::ptr());
  auto* returnedPointerArgument =
      returnedPointerFunction->addArg("pointer", Type::ptr());
  auto* returnedPointerEntry = returnedPointerFunction->newBlock("entry");
  returnedPointerFunction->setEntryBlock(returnedPointerEntry);
  IRBuilder returnedPointerBuilder(modRefModule, returnedPointerFunction);
  returnedPointerBuilder.setInsertPoint(returnedPointerEntry);
  returnedPointerBuilder.createRet(returnedPointerArgument);
  returnedPointerFunction->recomputeCFG();

  auto* modRefCaller =
      modRefModule.newFunction("modref_contract_caller", Type::i32());
  auto* modRefCallerEntry = modRefCaller->newBlock("entry");
  modRefCaller->setEntryBlock(modRefCallerEntry);
  IRBuilder modRefCallerBuilder(modRefModule, modRefCaller);
  modRefCallerBuilder.setInsertPoint(modRefCallerEntry);
  auto* modRefReadObject =
      modRefCallerBuilder.createAlloca(Type::i32(), 4, false, {}, "read");
  auto* modRefWriteObject =
      modRefCallerBuilder.createAlloca(Type::i32(), 4, false, {}, "write");
  auto* modRefUnrelatedObject =
      modRefCallerBuilder.createAlloca(Type::i32(), 4, false, {}, "unrelated");
  auto* modRefCall = modRefCallerBuilder.createCall(
      "modref_contract_callee", Type::voidTy(),
      {modRefReadObject, modRefWriteObject});
  modRefCallerBuilder.createRet(modRefCallerBuilder.ci(0));
  modRefCaller->recomputeCFG();
  analysis::AnalysisManager modRefManager(modRefModule);
  const auto& modRefEffects = modRefManager.functionEffects();
  const auto* modRefSummary =
      analysis::lookupFunctionEffect(modRefEffects, modRefCallee);
  assert(modRefSummary && modRefSummary->readArgumentIndices.count(0));
  assert(modRefSummary->writtenArgumentIndices.count(1));
  const auto* returnedPointerSummary =
      analysis::lookupFunctionEffect(modRefEffects, returnedPointerFunction);
  assert(returnedPointerSummary &&
         returnedPointerSummary->returnedArgumentIndices.count(0));
  assert(returnedPointerSummary->capturedArgumentIndices.count(0));
  const auto readModRef = analysis::queryCallModRef(
      modRefCall, modRefReadObject, 4, modRefEffects);
  const auto writeModRef = analysis::queryCallModRef(
      modRefCall, modRefWriteObject, 4, modRefEffects);
  const auto unrelatedModRef = analysis::queryCallModRef(
      modRefCall, modRefUnrelatedObject, 4, modRefEffects);
  assert(readModRef.proof.proved() && readModRef.mayRead &&
         !readModRef.mayWrite);
  assert(writeModRef.proof.proved() && !writeModRef.mayRead &&
         writeModRef.mayWrite);
  assert(unrelatedModRef.proof.proved() && !unrelatedModRef.mayRead &&
         !unrelatedModRef.mayWrite);

  Module semanticCallModule;
  auto* trappingCallee = semanticCallModule.newFunction(
      "trapping_callee", Type::i32());
  auto* trappingDivisor = trappingCallee->addArg("divisor", Type::i32());
  auto* trappingCalleeEntry = trappingCallee->newBlock("entry");
  trappingCallee->setEntryBlock(trappingCalleeEntry);
  IRBuilder trappingCalleeBuilder(semanticCallModule, trappingCallee);
  trappingCalleeBuilder.setInsertPoint(trappingCalleeEntry);
  auto* trappingValue = trappingCalleeBuilder.createSDiv(
      trappingCalleeBuilder.ci(1), trappingDivisor, "may-trap");
  trappingCalleeBuilder.createRet(trappingValue);
  trappingCallee->recomputeCFG();
  auto* trappingCaller = semanticCallModule.newFunction(
      "trapping_caller", Type::i32());
  auto* trappingCallerDivisor =
      trappingCaller->addArg("divisor", Type::i32());
  auto* trappingCallerEntry = trappingCaller->newBlock("entry");
  trappingCaller->setEntryBlock(trappingCallerEntry);
  IRBuilder trappingCallerBuilder(semanticCallModule, trappingCaller);
  trappingCallerBuilder.setInsertPoint(trappingCallerEntry);
  auto* trappingCall = trappingCallerBuilder.createCall(
      "trapping_callee", Type::i32(), {trappingCallerDivisor}, "call");
  trappingCallerBuilder.createRet(trappingCall);
  trappingCaller->recomputeCFG();
  analysis::AnalysisManager semanticCallManager(semanticCallModule);
  assert(!semanticCallManager.functionSemantics(trappingCallee).nonTrapping);
  assert(!semanticCallManager.functionSemantics(trappingCaller).nonTrapping);

  Module regionDeletionModule;
  auto* regionDeletionFunction = regionDeletionModule.newFunction(
      "region_deletion_contract", Type::i32());
  auto* regionDeletionRuntime =
      regionDeletionFunction->addArg("runtime", Type::i32());
  auto* regionDeletionEntry = regionDeletionFunction->newBlock("entry");
  auto* regionDeletionExit = regionDeletionFunction->newBlock("exit");
  regionDeletionFunction->setEntryBlock(regionDeletionEntry);
  IRBuilder regionDeletionBuilder(regionDeletionModule,
                                  regionDeletionFunction);
  regionDeletionBuilder.setInsertPoint(regionDeletionEntry);
  auto* regionDeletionLive = regionDeletionBuilder.createAdd(
      regionDeletionRuntime, regionDeletionBuilder.ci(1), "live");
  auto* regionDeletionDead = regionDeletionBuilder.createSDiv(
      regionDeletionRuntime, regionDeletionBuilder.ci(3), "dead-safe-div");
  auto* regionDeletionMayTrap = regionDeletionBuilder.createSDiv(
      regionDeletionBuilder.ci(9), regionDeletionRuntime,
      "dead-may-trap-div");
  regionDeletionBuilder.createBr(regionDeletionExit);
  regionDeletionBuilder.setInsertPoint(regionDeletionExit);
  regionDeletionBuilder.createRet(regionDeletionLive);
  regionDeletionFunction->recomputeCFG();
  std::unordered_set<BasicBlock*> regionDeletionBlocks{regionDeletionEntry};
  const auto regionDeletionObservers = analysis::buildObserverChainPlan(
      {regionDeletionLive}, regionDeletionBlocks);
  const auto regionDeletionDemand = analysis::buildRegionDemandSlice(
      regionDeletionObservers.observers, regionDeletionBlocks);
  const auto regionDeletionProof = analysis::proveRegionValueDeletion(
      regionDeletionDemand, regionDeletionBlocks);
  assert(regionDeletionProof.proof.proved());
  assert(std::find(regionDeletionProof.instructions.begin(),
                   regionDeletionProof.instructions.end(),
                   regionDeletionDead) != regionDeletionProof.instructions.end());
  assert(std::find(regionDeletionProof.instructions.begin(),
                   regionDeletionProof.instructions.end(),
                   regionDeletionMayTrap) == regionDeletionProof.instructions.end());
  analysis::AnalysisManager regionDeletionManager(regionDeletionModule);
  auto regionDeletionCandidate = planning::makeRegionValueDeletionCandidate(
      regionDeletionFunction, regionDeletionProof, &regionDeletionManager);
  planning::CandidateTransaction regionDeletionTransaction(
      regionDeletionModule, std::move(regionDeletionCandidate));
  const auto regionDeletionEvaluation = regionDeletionTransaction.evaluate();
  assert(regionDeletionEvaluation.proved && regionDeletionEvaluation.transformed &&
         regionDeletionEvaluation.verified);
  assert(!regionDeletionEvaluation.admitted &&
         regionDeletionEvaluation.reasonCode == "no-machine-benefit");

  Module deadLocalModule;
  auto* deadLocalFunction =
      deadLocalModule.newFunction("dead_local_contract", Type::i32());
  auto* deadLocalRuntime = deadLocalFunction->addArg("runtime", Type::i32());
  auto* deadLocalEntry = deadLocalFunction->newBlock("entry");
  deadLocalFunction->setEntryBlock(deadLocalEntry);
  IRBuilder deadLocalBuilder(deadLocalModule, deadLocalFunction);
  deadLocalBuilder.setInsertPoint(deadLocalEntry);
  auto* deadLocalObject =
      deadLocalBuilder.createAlloca(Type::i32(), 4, false, {}, "object");
  deadLocalBuilder.createStore(deadLocalRuntime, deadLocalObject);
  deadLocalBuilder.createRet(deadLocalRuntime);
  deadLocalFunction->recomputeCFG();
  const auto deadLocalProof =
      analysis::proveDeadLocalMaterialization(deadLocalObject);
  assert(deadLocalProof.proof.proved() && deadLocalProof.stores == 1);

  auto* deadLocalArrayFunction = deadLocalModule.newFunction(
      "dead_local_array_contract", Type::i32());
  auto* deadLocalArrayRuntime =
      deadLocalArrayFunction->addArg("runtime", Type::i32());
  auto* deadLocalArrayEntry = deadLocalArrayFunction->newBlock("entry");
  deadLocalArrayFunction->setEntryBlock(deadLocalArrayEntry);
  IRBuilder deadLocalArrayBuilder(deadLocalModule, deadLocalArrayFunction);
  deadLocalArrayBuilder.setInsertPoint(deadLocalArrayEntry);
  auto* deadLocalArrayObject = deadLocalArrayBuilder.createAlloca(
      Type::i32(), 8, true, {2}, "array-object");
  auto* deadLocalArrayElement = deadLocalArrayBuilder.createGEP(
      deadLocalArrayObject, {deadLocalArrayBuilder.ci(1)}, "element");
  deadLocalArrayBuilder.createStore(deadLocalArrayRuntime,
                                    deadLocalArrayElement);
  deadLocalArrayBuilder.createRet(deadLocalArrayRuntime);
  deadLocalArrayFunction->recomputeCFG();
  assert(analysis::proveDeadLocalMaterialization(deadLocalArrayObject)
             .proof.proved());

  auto* observedLocalFunction =
      deadLocalModule.newFunction("observed_local_contract", Type::i32());
  auto* observedLocalEntry = observedLocalFunction->newBlock("entry");
  observedLocalFunction->setEntryBlock(observedLocalEntry);
  IRBuilder observedLocalBuilder(deadLocalModule, observedLocalFunction);
  observedLocalBuilder.setInsertPoint(observedLocalEntry);
  auto* observedLocalObject = observedLocalBuilder.createAlloca(
      Type::i32(), 4, false, {}, "observed-object");
  observedLocalBuilder.createStore(observedLocalBuilder.ci(7),
                                   observedLocalObject);
  auto* observedLocalLoad =
      observedLocalBuilder.createLoad(observedLocalObject, "observed-load");
  observedLocalBuilder.createRet(observedLocalLoad);
  observedLocalFunction->recomputeCFG();
  assert(analysis::proveDeadLocalMaterialization(observedLocalObject)
             .proof.reasonCode == "dead-local-has-reader");
  assert(analysis::provePartialLocalMaterialization(observedLocalObject)
             .proof.reasonCode == "partial-local-all-stores-observed");

  auto* dynamicLocalFunction =
      deadLocalModule.newFunction("dynamic_local_contract", Type::i32());
  auto* dynamicLocalIndex =
      dynamicLocalFunction->addArg("index", Type::i32());
  auto* dynamicLocalEntry = dynamicLocalFunction->newBlock("entry");
  dynamicLocalFunction->setEntryBlock(dynamicLocalEntry);
  IRBuilder dynamicLocalBuilder(deadLocalModule, dynamicLocalFunction);
  dynamicLocalBuilder.setInsertPoint(dynamicLocalEntry);
  auto* dynamicLocalObject = dynamicLocalBuilder.createAlloca(
      Type::i32(), 8, true, {2}, "dynamic-object");
  auto* dynamicLocalAddress = dynamicLocalBuilder.createGEP(
      dynamicLocalObject, {dynamicLocalIndex}, "dynamic-address");
  dynamicLocalBuilder.createStore(dynamicLocalBuilder.ci(1),
                                  dynamicLocalAddress);
  dynamicLocalBuilder.createRet(dynamicLocalBuilder.ci(0));
  dynamicLocalFunction->recomputeCFG();
  assert(analysis::proveDeadLocalMaterialization(dynamicLocalObject)
             .proof.reasonCode == "dead-local-store-range-unknown");

  analysis::AnalysisManager deadLocalManager(deadLocalModule);
  assert(deadLocalManager.memorySSA(deadLocalFunction).proof.proved());
  const auto deadLocalModuleEpoch = deadLocalManager.moduleEpoch();
  planning::CandidateTransaction deadLocalTransaction(
      deadLocalModule, planning::makeDeadLocalMaterializationCandidate(
                           {deadLocalProof}, &deadLocalManager));
  const auto deadLocalEvaluation = deadLocalTransaction.evaluate();
  assert(deadLocalEvaluation.admitted);
  assert(deadLocalTransaction.commit());
  assert(deadLocalManager.moduleEpoch() == deadLocalModuleEpoch + 1);
  auto* replacedDeadLocalFunction =
      deadLocalModule.getFunction("dead_local_contract");
  assert(replacedDeadLocalFunction &&
         deadLocalManager.memorySSA(replacedDeadLocalFunction).proof.proved());

  Module partialLocalModule;
  auto* partialLocalFunction =
      partialLocalModule.newFunction("partial_local_contract", Type::i32());
  auto* partialLocalRuntime =
      partialLocalFunction->addArg("runtime", Type::i32());
  auto* partialLocalEntry = partialLocalFunction->newBlock("entry");
  partialLocalFunction->setEntryBlock(partialLocalEntry);
  IRBuilder partialLocalBuilder(partialLocalModule, partialLocalFunction);
  partialLocalBuilder.setInsertPoint(partialLocalEntry);
  auto* partialLocalObject = partialLocalBuilder.createAlloca(
      Type::i32(), 8, true, {2}, "object");
  auto* partialLocalObservedAddress = partialLocalBuilder.createGEP(
      partialLocalObject, {partialLocalBuilder.ci(0)}, "observed-address");
  auto* partialLocalDeadAddress = partialLocalBuilder.createGEP(
      partialLocalObject, {partialLocalBuilder.ci(1)}, "dead-address");
  partialLocalBuilder.createStore(partialLocalRuntime,
                                  partialLocalObservedAddress);
  auto* partialLocalDeadStore = partialLocalBuilder.createStore(
      partialLocalRuntime, partialLocalDeadAddress);
  auto* partialLocalLoad = partialLocalBuilder.createLoad(
      partialLocalObservedAddress, "observed-load");
  partialLocalBuilder.createRet(partialLocalLoad);
  partialLocalFunction->recomputeCFG();
  const auto partialLocalProof =
      analysis::provePartialLocalMaterialization(partialLocalObject);
  assert(partialLocalProof.proof.proved());
  assert(partialLocalProof.storesToDelete.size() == 1 &&
         partialLocalProof.storesToDelete.front() == partialLocalDeadStore);
  assert(partialLocalProof.retainedLoads == 1 &&
         partialLocalProof.retainedStores == 1);
  analysis::AnalysisManager partialLocalManager(partialLocalModule);
  planning::CandidateTransaction partialLocalTransaction(
      partialLocalModule,
      planning::makeLocalMaterializationSlicingCandidate(
          {}, {partialLocalProof}, &partialLocalManager));
  const auto partialLocalEvaluation = partialLocalTransaction.evaluate();
  assert(partialLocalEvaluation.admitted);
  assert(partialLocalTransaction.commit());

  Module dceTrapModule;
  auto* dceKnownGlobal = dceTrapModule.newGlobal(
      "dce_known_global", Type::i32(), {}, 4, false, true);
  auto* dceTrapFunction =
      dceTrapModule.newFunction("dce_trap_contract", Type::i32());
  auto* dceTrapRuntime = dceTrapFunction->addArg("runtime", Type::i32());
  auto* dceUnknownPointer =
      dceTrapFunction->addArg("pointer", Type::ptr());
  auto* dceTrapEntry = dceTrapFunction->newBlock("entry");
  dceTrapFunction->setEntryBlock(dceTrapEntry);
  IRBuilder dceTrapBuilder(dceTrapModule, dceTrapFunction);
  dceTrapBuilder.setInsertPoint(dceTrapEntry);
  auto* dceSafeDivision = dceTrapBuilder.createSDiv(
      dceTrapRuntime, dceTrapBuilder.ci(3), "safe-dead-division");
  auto* dceMayTrapDivision = dceTrapBuilder.createSDiv(
      dceTrapBuilder.ci(9), dceTrapRuntime, "may-trap-dead-division");
  auto* dceKnownLoad =
      dceTrapBuilder.createLoad(dceKnownGlobal, "known-dead-load");
  auto* dceUnknownLoad =
      dceTrapBuilder.createLoad(dceUnknownPointer, "unknown-dead-load");
  dceTrapBuilder.createRet(dceTrapBuilder.ci(0));
  dceTrapFunction->recomputeCFG();
  analysis::AnalysisManager dceTrapManager(dceTrapModule);
  assert(core_ir::dce(dceTrapModule, dceTrapManager));
  assert(!Value::isLiveValue(dceSafeDivision));
  assert(Value::isLiveValue(dceMayTrapDivision));
  assert(!Value::isLiveValue(dceKnownLoad));
  assert(Value::isLiveValue(dceUnknownLoad));

  Module loopModule;
  auto* loopFunction = loopModule.newFunction("loop_contract", Type::i32());
  auto* preheader = loopFunction->newBlock("preheader");
  auto* header = loopFunction->newBlock("header");
  auto* body = loopFunction->newBlock("body");
  auto* loopExit = loopFunction->newBlock("exit");
  loopFunction->setEntryBlock(preheader);
  IRBuilder loopBuilder(loopModule, loopFunction);
  loopBuilder.setInsertPoint(preheader);
  loopBuilder.createBr(header);
  loopBuilder.setInsertPoint(header);
  auto* induction = loopBuilder.createPhi(
      Type::i32(), {{loopBuilder.ci(0), preheader}}, "iv");
  auto* condition = loopBuilder.createICmp(
      "lt", induction, loopBuilder.ci(4), "bound");
  loopBuilder.createCondBr(condition, body, loopExit);
  loopBuilder.setInsertPoint(body);
  auto* next = loopBuilder.createAdd(induction, loopBuilder.ci(1), "next");
  loopBuilder.createBr(header);
  induction->addOperand(next);
  induction->addOperand(body);
  loopBuilder.setInsertPoint(loopExit);
  loopBuilder.createRet(induction);
  loopFunction->recomputeCFG();
  assert(verifyModuleChecked(loopModule).ok);
  analysis::NaturalLoop exactLoop;
  exactLoop.header = header;
  exactLoop.blocks = {header, body};
  exactLoop.latches = {body};
  const auto exactTrip = analysis::analyzeLoopTrip(loopFunction, exactLoop);
  assert(exactTrip.proof.proved());
  assert(exactTrip.kind == analysis::TripCountKind::ConstantMachineExact);
  assert(exactTrip.constantTripCount == 4);

  Module boundedModule;
  auto* boundedFunction = boundedModule.newFunction(
      "bounded_loop_contract", Type::i32());
  auto* runtimeBound = boundedFunction->addArg("bound", Type::i32());
  auto* boundedPreheader = boundedFunction->newBlock("preheader");
  auto* boundedHeader = boundedFunction->newBlock("header");
  auto* boundedBody = boundedFunction->newBlock("body");
  auto* boundedExit = boundedFunction->newBlock("exit");
  boundedFunction->setEntryBlock(boundedPreheader);
  IRBuilder boundedBuilder(boundedModule, boundedFunction);
  boundedBuilder.setInsertPoint(boundedPreheader);
  auto* maskedBound = boundedBuilder.createAnd(
      runtimeBound, boundedBuilder.ci(15), "masked-bound");
  boundedBuilder.createBr(boundedHeader);
  boundedBuilder.setInsertPoint(boundedHeader);
  auto* boundedInduction = boundedBuilder.createPhi(
      Type::i32(), {{boundedBuilder.ci(0), boundedPreheader}}, "iv");
  auto* boundedCondition = boundedBuilder.createICmp(
      "lt", boundedInduction, maskedBound, "bound");
  boundedBuilder.createCondBr(boundedCondition, boundedBody, boundedExit);
  boundedBuilder.setInsertPoint(boundedBody);
  auto* boundedNext = boundedBuilder.createAdd(
      boundedInduction, boundedBuilder.ci(1), "next");
  boundedBuilder.createBr(boundedHeader);
  boundedInduction->addOperand(boundedNext);
  boundedInduction->addOperand(boundedBody);
  boundedBuilder.setInsertPoint(boundedExit);
  boundedBuilder.createRet(boundedInduction);
  boundedFunction->recomputeCFG();
  assert(verifyModuleChecked(boundedModule).ok);
  analysis::NaturalLoop boundedLoop;
  boundedLoop.header = boundedHeader;
  boundedLoop.blocks = {boundedHeader, boundedBody};
  boundedLoop.latches = {boundedBody};
  const auto boundedTrip =
      analysis::analyzeLoopTrip(boundedFunction, boundedLoop);
  assert(boundedTrip.kind == analysis::TripCountKind::RuntimeBounded);
  assert(boundedTrip.proof.state == analysis::ProofState::Unknown);
  assert(analysis::maximumTripCount(boundedTrip).value() == 15);

  Module wrapModule;
  auto* wrapFunction = wrapModule.newFunction("wrap_contract", Type::i32());
  auto* wrapPreheader = wrapFunction->newBlock("preheader");
  auto* wrapHeader = wrapFunction->newBlock("header");
  auto* wrapBody = wrapFunction->newBlock("body");
  auto* wrapExit = wrapFunction->newBlock("exit");
  wrapFunction->setEntryBlock(wrapPreheader);
  IRBuilder wrapBuilder(wrapModule, wrapFunction);
  wrapBuilder.setInsertPoint(wrapPreheader);
  wrapBuilder.createBr(wrapHeader);
  wrapBuilder.setInsertPoint(wrapHeader);
  auto* wrapInduction = wrapBuilder.createPhi(
      Type::i32(), {{wrapBuilder.ci(2147483646), wrapPreheader}}, "iv");
  auto* wrapCondition = wrapBuilder.createICmp(
      "le", wrapInduction, wrapBuilder.ci(2147483647), "bound");
  wrapBuilder.createCondBr(wrapCondition, wrapBody, wrapExit);
  wrapBuilder.setInsertPoint(wrapBody);
  auto* wrapNext = wrapBuilder.createAdd(
      wrapInduction, wrapBuilder.ci(1), "next");
  wrapBuilder.createBr(wrapHeader);
  wrapInduction->addOperand(wrapNext);
  wrapInduction->addOperand(wrapBody);
  wrapBuilder.setInsertPoint(wrapExit);
  wrapBuilder.createRet(wrapInduction);
  wrapFunction->recomputeCFG();
  assert(verifyModuleChecked(wrapModule).ok);
  analysis::NaturalLoop wrapLoop;
  wrapLoop.header = wrapHeader;
  wrapLoop.blocks = {wrapHeader, wrapBody};
  wrapLoop.latches = {wrapBody};
  const auto wrapTrip = analysis::analyzeLoopTrip(wrapFunction, wrapLoop);
  assert(wrapTrip.proof.state == analysis::ProofState::Disproved);
  assert(wrapTrip.proof.reasonCode == "trip-count-wrap-or-nontermination");

  const std::string before = snapshot(module);
  const int baselineNextValueId = Value::idCheckpoint();
  bool transformCalled = false;
  planning::resetCandidateStatistics();
  planning::CandidatePlan unknown;
  unknown.passName = "unknown-proof-contract";
  unknown.legality = analysis::ProofResult::unknown("test-unknown");
  unknown.transform = [&](planning::PlanningClone&, std::string&) {
    transformCalled = true;
    return true;
  };
  planning::CandidateTransaction unknownTransaction(module,
                                                     std::move(unknown));
  const auto unknownEvaluation = unknownTransaction.evaluate();
  assert(unknownEvaluation.reasonCode == "test-unknown");
  assert(unknownEvaluation.discovered && !unknownEvaluation.proved);
  assert(!unknownEvaluation.transformed && !unknownEvaluation.verified);
  assert(!unknownEvaluation.admitted && !unknownEvaluation.committed);
  assert(!transformCalled && snapshot(module) == before);

  planning::CandidatePlan unknownFact;
  unknownFact.passName = "unknown-fact-contract";
  unknownFact.legality = analysis::ProofResult::success("test-proved");
  unknownFact.proofFacts.push_back(
      analysis::ProofResult::unknown("test-proof-fact-unknown"));
  unknownFact.transform = [&](planning::PlanningClone&, std::string&) {
    transformCalled = true;
    return true;
  };
  planning::CandidateTransaction unknownFactTransaction(
      module, std::move(unknownFact));
  assert(unknownFactTransaction.evaluate().reasonCode ==
         "test-proof-fact-unknown");
  assert(!transformCalled && snapshot(module) == before);

  planning::CandidatePlan invalid;
  invalid.passName = "invalid-ir-contract";
  invalid.legality = analysis::ProofResult::success("test-proved");
  invalid.transform = [](planning::PlanningClone& candidate,
                         std::string&) {
    auto* block = candidate.module().functions().front()->entryBlock();
    auto* bad = new Instruction(Opcode::Store, Type::voidTy(), "bad-store");
    bad->addOperand(candidate.module().getConstInt(1));
    block->insertBefore(block->terminator(), bad);
    return true;
  };
  planning::CandidateTransaction invalidTransaction(module,
                                                     std::move(invalid));
  const auto invalidEvaluation = invalidTransaction.evaluate();
  assert(invalidEvaluation.reasonCode == "candidate-ir-verifier-failed");
  assert(invalidEvaluation.discovered && invalidEvaluation.proved);
  assert(invalidEvaluation.transformed && !invalidEvaluation.verified);
  assert(invalidEvaluation.baselineIRBytes == before.size());
  assert(invalidEvaluation.candidateIRBytes != 0);
  assert(snapshot(module) == before);
  assert(Value::idCheckpoint() == baselineNextValueId);
  const auto& planningStats = planning::candidateStatistics();
  assert(planningStats.at("unknown-proof-contract").proofRejected == 1);
  assert(planningStats.at("unknown-fact-contract").proofRejected == 1);
  assert(planningStats.at("invalid-ir-contract").verifyRejected == 1);
  std::ostringstream statisticsDump;
  planning::dumpCandidateStatistics(statisticsDump);
  assert(statisticsDump.str().find("pass=invalid-ir-contract") !=
         std::string::npos);

  mir::MachinePipelineResult baseline;
  mir::MachinePipelineResult candidate;
  baseline.module = std::make_unique<mir::MachineModule>();
  candidate.module = std::make_unique<mir::MachineModule>();
  baseline.metrics.instructions = candidate.metrics.instructions = 100;
  candidate.metrics.spillSlots = 1;
  assert(mir::admitRV64Candidate(baseline, candidate).reasonCode ==
         "spill-growth");
  candidate.metrics.spillSlots = 0;
  candidate.metrics.frameBytes = 16;
  assert(mir::admitRV64Candidate(baseline, candidate).reasonCode ==
         "frame-growth");
  candidate.metrics.frameBytes = 0;
  candidate.metrics.frameBytes = (1U << 20) + 1;
  assert(mir::admitRV64Candidate(baseline, candidate).reasonCode ==
         "frame-limit-exceeded");
  candidate.metrics.frameBytes = 0;
  baseline.metrics.estimatedCriticalPath = 100;
  candidate.metrics.estimatedCriticalPath = 111;
  assert(mir::admitRV64Candidate(baseline, candidate).reasonCode ==
         "critical-path-growth");
  candidate.metrics.estimatedCriticalPath = 100;
  candidate.metrics.cyclicCFGEdges = 1;
  assert(mir::admitRV64Candidate(baseline, candidate).reasonCode ==
         "cyclic-cfg-growth");
  candidate.metrics.cyclicCFGEdges = 0;
  baseline.metrics.emittedAssemblyBytes = 100;
  candidate.metrics.emittedAssemblyBytes = 111;
  assert(mir::admitRV64Candidate(baseline, candidate).reasonCode ==
         "assembly-size-growth");

  mir::MachineModule invalidMachine;
  invalidMachine.newFunction("missing-entry");
  assert(mir::verifyMachineModule(invalidMachine, true).reasonCode ==
         "missing-machine-entry");
}
CPP

"${CXX:-c++}" -std=c++17 -I"$ROOT/src" "$TMP/contracts.cpp" \
  "${OBJECTS[@]}" -o "$TMP/contracts"
"$TMP/contracts"
echo "rv64 planning contracts: PASS"
