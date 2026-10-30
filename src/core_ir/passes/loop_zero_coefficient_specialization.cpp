#include "research_switch.h"
// 文件职责：识别循环中的零系数并整理可以跳过的计算。
#include "pass_registry.h"
#include "loop_zero_coefficient_specialization.h"

#include "../../analysis/analysis_manager.h"
#include "../../analysis/cfg_utils.h"
#include "../../analysis/dominators.h"
#include "../../analysis/function_analysis.h"
#include "../../analysis/loop_info.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace core_ir {
namespace {

bool traceEnabled() {
  return sycEnvironmentValue("SYC_TRACE_COREIR_ZERO_COEFF_SPECIALIZATION") != nullptr;
}

void trace(const char* stage, Function* func, const analysis::NaturalLoop* loop,
           const std::string& reason, Value* coeff = nullptr) {
  if (!traceEnabled())
    return;
  std::cerr << "[zero-coeff-specialize] " << stage
            << " func=" << (func ? func->name() : std::string("<null>"));
  if (loop && loop->header)
    std::cerr << " header=" << loop->header->name();
  if (coeff)
    std::cerr << " coeff=" << coeff->id();
  if (!reason.empty())
    std::cerr << " reason=" << reason;
  std::cerr << "\n";
}

Instruction* cloneInstructionShell(Instruction* src,
                                   const std::string& suffix = ".zerocoeff") {
  std::string name = src->name();
  if (src->opcode() != Opcode::Call)
    name = src->name().empty() ? "loop.zerocoeff" : src->name() + suffix;
  auto* clone = new Instruction(src->opcode(), src->type(), name);
  clone->setMemoryInfo(src->memoryElemTy(), src->memorySizeBytes(),
                       src->isArrayObject(), src->dims());
  return clone;
}

Value* remapValue(Value* value,
                  const std::unordered_map<Value*, Value*>& valueMap,
                  const std::unordered_map<BasicBlock*, BasicBlock*>& blockMap) {
  if (auto* bb = dynamic_cast<BasicBlock*>(value)) {
    auto it = blockMap.find(bb);
    if (it != blockMap.end())
      return it->second;
  }
  auto it = valueMap.find(value);
  return it == valueMap.end() ? value : it->second;
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

int loopInstructionCount(const analysis::NaturalLoop& loop) {
  int count = 0;
  for (BasicBlock* bb : loop.blocks) {
    if (bb)
      count += static_cast<int>(bb->instructions().size());
  }
  return count;
}

bool valueDefinedInLoop(Value* value, const analysis::NaturalLoop& loop) {
  auto* inst = dynamic_cast<Instruction*>(value);
  return inst && inst->parent() && loop.blocks.count(inst->parent());
}

bool valueAvailableAtPreheader(Value* value, BasicBlock* preheader,
                               const analysis::DomSet& doms) {
  if (!value || !preheader)
    return false;
  if (value->isConstant() || dynamic_cast<Argument*>(value) ||
      dynamic_cast<GlobalVar*>(value))
    return true;
  auto* inst = dynamic_cast<Instruction*>(value);
  return inst && inst->parent() &&
         analysis::dominates(doms, inst->parent(), preheader);
}

bool loopHasUnsupportedEscapes(const analysis::NaturalLoop& loop) {
  for (BasicBlock* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      for (Instruction* user : inst->uses()) {
        BasicBlock* userParent = user ? user->parent() : nullptr;
        if (!userParent || loop.blocks.count(userParent))
          continue;
        if (user->opcode() != Opcode::Phi || !incomingFromPred(user, bb))
          return true;
      }
    }
  }
  return false;
}

bool loopHasUnsupportedInstruction(const analysis::NaturalLoop& loop) {
  for (BasicBlock* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      switch (inst->opcode()) {
      case Opcode::Alloca:
      case Opcode::Call:
      case Opcode::Ret:
        return true;
      default:
        break;
      }
    }
  }
  return false;
}

bool loopExitPhisAreCompatible(const analysis::NaturalLoop& loop) {
  for (BasicBlock* srcBB : loop.blocks) {
    if (!srcBB)
      continue;
    for (BasicBlock* succ : srcBB->succs()) {
      if (!succ || loop.blocks.count(succ))
        continue;
      for (Instruction* phi : analysis::collectLeadingPhi(succ)) {
        if (!incomingFromPred(phi, srcBB))
          return false;
      }
    }
  }
  return true;
}

bool isProfitableLoopCandidate(const analysis::NaturalLoop& loop) {
  if (!loop.header || loop.blocks.empty() || loop.blocks.size() > 24)
    return false;
  if (loop.latches.empty() || loop.latches.size() > 4)
    return false;
  const int instCount = loopInstructionCount(loop);
  if (instCount < 8 || instCount > 220)
    return false;
  if (loopHasUnsupportedInstruction(loop) || loopHasUnsupportedEscapes(loop))
    return false;
  return loopExitPhisAreCompatible(loop);
}

bool isLoadFromAddress(Value* value, Value* addr) {
  auto* load = dynamic_cast<Instruction*>(value);
  return load && load->opcode() == Opcode::Load && load->numOperands() == 1 &&
         load->operand(0) == addr;
}

bool matchRMWMul(Value* value, Value* storeAddr, Value*& coeffOut) {
  auto* mul = dynamic_cast<Instruction*>(value);
  if (!mul || mul->opcode() != Opcode::Mul || mul->numOperands() != 2)
    return false;
  if (isLoadFromAddress(mul->operand(0), storeAddr)) {
    coeffOut = mul->operand(1);
    return true;
  }
  if (isLoadFromAddress(mul->operand(1), storeAddr)) {
    coeffOut = mul->operand(0);
    return true;
  }
  return false;
}

bool matchZeroCoeffRMWStore(Instruction* store, Value*& coeffOut) {
  if (!store || store->opcode() != Opcode::Store || store->numOperands() != 2)
    return false;

  auto* add = dynamic_cast<Instruction*>(store->operand(0));
  if (!add || add->opcode() != Opcode::Add || add->numOperands() != 2)
    return false;

  Value* coeff = nullptr;
  if (matchRMWMul(add->operand(0), store->operand(1), coeff) ||
      matchRMWMul(add->operand(1), store->operand(1), coeff)) {
    if (!coeff || !coeff->type().isI32() || dynamic_cast<ConstInt*>(coeff))
      return false;
    coeffOut = coeff;
    return true;
  }
  return false;
}

struct CoeffInfo {
  Value* coeff = nullptr;
  int stores = 0;
};

CoeffInfo findInvariantZeroCoeff(const analysis::NaturalLoop& loop,
                                 BasicBlock* preheader,
                                 const analysis::DomSet& doms) {
  std::unordered_map<Value*, int> storesByCoeff;
  for (BasicBlock* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      Value* coeff = nullptr;
      if (!matchZeroCoeffRMWStore(instPtr.get(), coeff))
        continue;
      if (valueDefinedInLoop(coeff, loop))
        continue;
      if (!valueAvailableAtPreheader(coeff, preheader, doms))
        continue;
      ++storesByCoeff[coeff];
    }
  }

  CoeffInfo best;
  for (auto& entry : storesByCoeff) {
    if (entry.second > best.stores) {
      best.coeff = entry.first;
      best.stores = entry.second;
    }
  }
  return best;
}

bool cloneLoopBlocks(Function* func, Module& mod,
                     const analysis::NaturalLoop& loop, Value* coeff,
                     std::unordered_map<BasicBlock*, BasicBlock*>& blockMap,
                     std::unordered_map<Value*, Value*>& valueMap) {
  if (!func || !loop.header || !coeff)
    return false;

  std::vector<BasicBlock*> orderedBlocks;
  orderedBlocks.reserve(loop.blocks.size());
  for (auto& bbPtr : func->blocks()) {
    if (loop.blocks.count(bbPtr.get()))
      orderedBlocks.push_back(bbPtr.get());
  }
  if (orderedBlocks.size() != loop.blocks.size())
    return false;

  for (BasicBlock* srcBB : orderedBlocks)
    blockMap[srcBB] = func->newBlock("loop.zerocoeff");

  for (BasicBlock* srcBB : orderedBlocks) {
    BasicBlock* dstBB = blockMap.at(srcBB);
    for (auto& instPtr : srcBB->instructions()) {
      Instruction* srcInst = instPtr.get();
      Instruction* dstInst = cloneInstructionShell(srcInst);
      dstBB->pushBack(dstInst);
      valueMap[srcInst] = dstInst;
    }
  }

  ConstInt* zero = mod.getConstInt(0);
  for (BasicBlock* srcBB : orderedBlocks) {
    BasicBlock* dstBB = blockMap.at(srcBB);
    size_t dstIndex = 0;
    for (auto& instPtr : srcBB->instructions()) {
      Instruction* srcInst = instPtr.get();
      Instruction* dstInst = dstBB->instructions()[dstIndex++].get();
      for (int i = 0; i < srcInst->numOperands(); ++i) {
        Value* operand = srcInst->operand(i);
        dstInst->addOperand(
            operand == coeff ? zero : remapValue(operand, valueMap, blockMap));
      }
    }
  }

  return true;
}

bool addClonedExitPhiIncoming(
    const analysis::NaturalLoop& loop,
    const std::unordered_map<BasicBlock*, BasicBlock*>& blockMap,
    const std::unordered_map<Value*, Value*>& valueMap) {
  for (BasicBlock* srcBB : loop.blocks) {
    if (!srcBB)
      continue;
    auto clonedSrcIt = blockMap.find(srcBB);
    if (clonedSrcIt == blockMap.end())
      return false;
    BasicBlock* clonedSrc = clonedSrcIt->second;
    for (BasicBlock* succ : srcBB->succs()) {
      if (!succ || loop.blocks.count(succ))
        continue;
      for (Instruction* phi : analysis::collectLeadingPhi(succ)) {
        Value* incoming = incomingFromPred(phi, srcBB);
        if (!incoming)
          return false;
        phi->addOperand(remapValue(incoming, valueMap, blockMap));
        phi->addOperand(clonedSrc);
      }
    }
  }
  return true;
}

BasicBlock* createForwardingPreheader(Function* func, BasicBlock* target,
                                      const std::string& suffix) {
  if (!func || !target)
    return nullptr;
  BasicBlock* bb = func->newBlock(suffix);
  auto* br = new Instruction(Opcode::Br, Type::voidTy());
  br->addOperand(target);
  bb->pushBack(br);
  return bb;
}

bool rewritePreheaderToVersionDispatch(BasicBlock* preheader, Value* coeff,
                                       BasicBlock* fastPreheader,
                                       BasicBlock* slowPreheader,
                                       Module& mod) {
  if (!preheader || !coeff || !fastPreheader || !slowPreheader)
    return false;
  Instruction* term = preheader->terminator();
  if (!term || term->opcode() != Opcode::Br || term->numOperands() != 1)
    return false;

  auto* cmp = new Instruction(Opcode::ICmp, Type::i32(), "eq_loop_zerocoeff");
  cmp->addOperand(coeff);
  cmp->addOperand(mod.getConstInt(0));
  preheader->insertBefore(term, cmp);

  preheader->remove(term);
  auto* br = new Instruction(Opcode::CondBr, Type::voidTy());
  br->addOperand(cmp);
  br->addOperand(fastPreheader);
  br->addOperand(slowPreheader);
  preheader->pushBack(br);
  return true;
}

bool applyCandidate(Module& mod, Function* func,
                    const analysis::NaturalLoop& loop, BasicBlock* preheader,
                    Value* coeff) {
  if (!func || !loop.header || !preheader || !coeff)
    return false;
  Instruction* preTerm = preheader->terminator();
  if (!preTerm || preTerm->opcode() != Opcode::Br ||
      preTerm->numOperands() != 1 || preTerm->operand(0) != loop.header)
    return false;

  std::unordered_map<BasicBlock*, BasicBlock*> blockMap;
  std::unordered_map<Value*, Value*> valueMap;
  if (!cloneLoopBlocks(func, mod, loop, coeff, blockMap, valueMap))
    return false;
  if (!addClonedExitPhiIncoming(loop, blockMap, valueMap))
    return false;

  BasicBlock* clonedHeader = blockMap.at(loop.header);
  BasicBlock* fastPreheader =
      createForwardingPreheader(func, clonedHeader, "loop.zerocoeff.fast");
  BasicBlock* slowPreheader =
      createForwardingPreheader(func, loop.header, "loop.zerocoeff.slow");
  if (!fastPreheader || !slowPreheader)
    return false;

  analysis::replacePhiIncomingPred(loop.header, preheader, slowPreheader);
  analysis::replacePhiIncomingPred(clonedHeader, preheader, fastPreheader);

  if (!rewritePreheaderToVersionDispatch(preheader, coeff, fastPreheader,
                                         slowPreheader, mod))
    return false;

  func->recomputeCFG();
  trace("apply", func, &loop, "", coeff);
  return true;
}

bool runOnFunction(Module& mod, Function* func,
                   analysis::AnalysisManager& analysisManager) {
  if (!func || !func->entryBlock())
    return false;

  bool changed = false;
  std::unordered_set<BasicBlock*> versionedHeaders;
  int budget = 6;
  while (budget-- > 0) {
    func->recomputeCFG();
    analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
    analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
    const auto& loops = cache.loops();
    const analysis::DomSet& doms = cache.dominators();

    const analysis::NaturalLoop* selectedLoop = nullptr;
    BasicBlock* selectedPreheader = nullptr;
    Value* selectedCoeff = nullptr;
    int selectedStores = 0;

    for (const auto& loop : loops) {
      if (versionedHeaders.count(loop.header)) {
        trace("reject", func, &loop, "already-versioned");
        continue;
      }
      if (!isProfitableLoopCandidate(loop)) {
        trace("reject", func, &loop, "shape");
        continue;
      }
      BasicBlock* preheader = analysis::dedicatedPreheader(loop);
      if (!preheader) {
        trace("reject", func, &loop, "no-preheader");
        continue;
      }
      CoeffInfo coeff = findInvariantZeroCoeff(loop, preheader, doms);
      if (!coeff.coeff || coeff.stores <= 0) {
        trace("reject", func, &loop, "no-rmw-coeff");
        continue;
      }
      if (coeff.stores > selectedStores) {
        selectedLoop = &loop;
        selectedPreheader = preheader;
        selectedCoeff = coeff.coeff;
        selectedStores = coeff.stores;
      }
    }

    if (!selectedLoop || !selectedPreheader || !selectedCoeff)
      break;
    BasicBlock* versionedHeader = selectedLoop->header;
    if (!applyCandidate(mod, func, *selectedLoop, selectedPreheader,
                        selectedCoeff))
      break;
    versionedHeaders.insert(versionedHeader);
    changed = true;
  }
  return changed;
}

} // namespace

bool applyLoopZeroCoefficientSpecialization(
    Module& mod, analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    if (runOnFunction(mod, funcPtr.get(), analysisManager))
      changed = true;
  }
  if (changed)
    analysisManager.invalidateModule(analysis::AnalysisInvalidation::Module);
  return changed;
}

class ZeroCoefficientSpecializationPass : public Pass {
public:
    std::string name() const override { return "ZeroCoefficientSpecialization"; }
    bool run(Module& mod, analysis::AnalysisManager& am) override {
        return applyLoopZeroCoefficientSpecialization(mod, am);
    }
};
REGISTER_PASS(ZeroCoefficientSpecializationPass);

} // namespace core_ir
