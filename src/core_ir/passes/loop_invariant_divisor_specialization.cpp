#include "research_switch.h"
// 文件职责：为循环内保持不变的有符号除数生成可复用的除法描述符。
#include "pass_registry.h"
#include "loop_invariant_divisor_specialization.h"

#include "../../analysis/analysis_manager.h"
#include "../../analysis/cfg_utils.h"
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
  return sycEnvironmentValue("SYC_TRACE_COREIR_DIVISOR_SPECIALIZATION") != nullptr;
}

void trace(const char* stage, Function* func, const analysis::NaturalLoop* loop,
           const std::string& reason, Value* divisor = nullptr) {
  if (!traceEnabled())
    return;
  std::cerr << "[divisor-specialize] " << stage
            << " func=" << (func ? func->name() : std::string("<null>"));
  if (loop && loop->header)
    std::cerr << " header=" << loop->header->name();
  if (divisor)
    std::cerr << " divisor=" << divisor->id();
  if (!reason.empty())
    std::cerr << " reason=" << reason;
  std::cerr << "\n";
}

bool valueDefinedInLoop(Value* value, const analysis::NaturalLoop& loop) {
  auto* instruction = dynamic_cast<Instruction*>(value);
  return instruction && instruction->parent() &&
         loop.blocks.count(instruction->parent());
}

bool containsProperSubloop(
    const analysis::NaturalLoop& loop,
    const std::vector<analysis::NaturalLoop>& allLoops) {
  for (const analysis::NaturalLoop& candidate : allLoops) {
    if (!candidate.header || candidate.header == loop.header ||
        candidate.blocks.size() >= loop.blocks.size() ||
        !loop.blocks.count(candidate.header)) {
      continue;
    }
    bool contained = true;
    for (BasicBlock* block : candidate.blocks) {
      if (!loop.blocks.count(block)) {
        contained = false;
        break;
      }
    }
    if (contained)
      return true;
  }
  return false;
}

bool isDescriptorCandidateShape(const analysis::NaturalLoop& loop) {
  if (!loop.header || loop.blocks.empty() || loop.blocks.size() > 18 ||
      loop.latches.size() != 1) {
    return false;
  }
  int instructionCount = 0;
  for (BasicBlock* block : loop.blocks) {
    if (!block)
      return false;
    instructionCount += static_cast<int>(block->instructions().size());
    for (const auto& instructionPtr : block->instructions()) {
      Instruction* instruction = instructionPtr.get();
      if (!instruction || instruction->opcode() == Opcode::Alloca ||
          instruction->opcode() == Opcode::Call ||
          instruction->opcode() == Opcode::Ret) {
        return false;
      }
    }
  }
  // 描述符准备和循环版本化都有固定成本，只在中等规模的单回边内层循环
  // 使用；过小循环难以摊薄准备成本，过大循环则会放大代码体积。
  return instructionCount >= 8 && instructionCount <= 140;
}

struct DivisorInfo {
  Value* divisor = nullptr;
  int uses = 0;
};

struct DescriptorParts {
  Instruction* directDescriptor = nullptr;
  Instruction* directFlag = nullptr;
};

Instruction* instructionAfter(BasicBlock* block, Instruction* position) {
  if (!block || !position)
    return nullptr;
  const auto& instructions = block->instructions();
  for (size_t index = 0; index + 1 < instructions.size(); ++index) {
    if (instructions[index].get() == position)
      return instructions[index + 1].get();
  }
  return nullptr;
}

DescriptorParts createDescriptorParts(Function* func, Value* divisor,
                                      BasicBlock* fallbackBlock) {
  BasicBlock* preparationBlock = fallbackBlock;
  Instruction* insertionPoint =
      fallbackBlock ? fallbackBlock->terminator() : nullptr;
  if (auto* definingInstruction = dynamic_cast<Instruction*>(divisor)) {
    if (definingInstruction->parent()) {
      preparationBlock = definingInstruction->parent();
      insertionPoint = instructionAfter(preparationBlock, definingInstruction);
    }
  } else if (func && func->entryBlock()) {
    preparationBlock = func->entryBlock();
    insertionPoint = preparationBlock->terminator();
  }
  if (!preparationBlock || !insertionPoint)
    return {};

  auto appendPart = [&](Opcode opcode, const char* name, Value* operand) {
    auto* part = new Instruction(opcode, Type::i32(), name);
    part->addOperand(operand);
    preparationBlock->insertBefore(insertionPoint, part);
    return part;
  };

  DescriptorParts parts;
  // 描述符紧跟除数的 SSA 定义生成，因此对同一除数只执行一次，并自然支配
  // 后续所有使用该除数的内层循环。
  auto* directDescriptor = new Instruction(
      Opcode::SDivDirectPrepare, Type::ptr(), "direct.divisor.descriptor");
  directDescriptor->addOperand(divisor);
  preparationBlock->insertBefore(insertionPoint, directDescriptor);
  parts.directDescriptor = directDescriptor;
  parts.directFlag = appendPart(Opcode::SDivDirectFlag,
                                "direct.divisor.flag", directDescriptor);
  return parts;
}

std::vector<DivisorInfo>
findInvariantDivisors(const analysis::NaturalLoop& loop) {
  std::unordered_map<Value*, int> usesByDivisor;
  for (BasicBlock* block : loop.blocks) {
    if (!block)
      continue;
    for (const auto& instructionPtr : block->instructions()) {
      Instruction* instruction = instructionPtr.get();
      if (!instruction || instruction->opcode() != Opcode::SDiv ||
          instruction->numOperands() != 2) {
        continue;
      }
      Value* divisor = instruction->operand(1);
      if (!divisor || !divisor->type().isI32() ||
          dynamic_cast<ConstInt*>(divisor) ||
          valueDefinedInLoop(divisor, loop)) {
        continue;
      }
      ++usesByDivisor[divisor];
    }
  }

  std::vector<DivisorInfo> result;
  result.reserve(usesByDivisor.size());
  for (const auto& entry : usesByDivisor)
    result.push_back({entry.first, entry.second});
  std::sort(result.begin(), result.end(),
            [](const DivisorInfo& lhs, const DivisorInfo& rhs) {
              return lhs.divisor->id() < rhs.divisor->id();
            });
  return result;
}

Value* remapValue(
    Value* value, const std::unordered_map<Value*, Value*>& valueMap,
    const std::unordered_map<BasicBlock*, BasicBlock*>& blockMap) {
  if (auto* block = dynamic_cast<BasicBlock*>(value)) {
    auto mappedBlock = blockMap.find(block);
    if (mappedBlock != blockMap.end())
      return mappedBlock->second;
  }
  auto mappedValue = valueMap.find(value);
  return mappedValue == valueMap.end() ? value : mappedValue->second;
}

Value* incomingFromPredecessor(Instruction* phi, BasicBlock* predecessor) {
  if (!phi || !predecessor)
    return nullptr;
  for (int index = 0; index + 1 < phi->numOperands(); index += 2) {
    if (phi->operand(index + 1) == predecessor)
      return phi->operand(index);
  }
  return nullptr;
}

bool loopCanBeVersioned(const analysis::NaturalLoop& loop, Value* divisor) {
  if (!loop.header || loop.blocks.empty())
    return false;
  for (BasicBlock* block : loop.blocks) {
    if (!block)
      return false;
    for (const auto& instructionPtr : block->instructions()) {
      Instruction* instruction = instructionPtr.get();
      if (!instruction || instruction->opcode() == Opcode::Alloca)
        return false;
      if (instruction->opcode() == Opcode::SDiv &&
          instruction->numOperands() == 2 &&
          instruction->operand(1) != divisor &&
          !dynamic_cast<ConstInt*>(instruction->operand(1))) {
        return false;
      }
      for (Instruction* user : instruction->uses()) {
        BasicBlock* userBlock = user ? user->parent() : nullptr;
        if (!userBlock || loop.blocks.count(userBlock))
          continue;
        if (user->opcode() != Opcode::Phi ||
            !incomingFromPredecessor(user, block)) {
          return false;
        }
      }
    }
    for (BasicBlock* successor : block->succs()) {
      if (!successor || loop.blocks.count(successor))
        continue;
      for (Instruction* phi : analysis::collectLeadingPhi(successor)) {
        if (!incomingFromPredecessor(phi, block))
          return false;
      }
    }
  }
  return true;
}

Instruction* cloneInstructionShell(Instruction* source, Opcode opcode) {
  std::string name = source->name();
  if (source->opcode() != Opcode::Call)
    name = name.empty() ? "loop.divisor.version" : name + ".divisor.version";
  auto* clone = new Instruction(opcode, source->type(), name);
  clone->setMemoryInfo(source->memoryElemTy(), source->memorySizeBytes(),
                       source->isArrayObject(), source->dims());
  return clone;
}

bool cloneDirectDivisionLoop(
    Function* func, const analysis::NaturalLoop& loop,
    Value* divisor, const DescriptorParts& parts,
    std::unordered_map<BasicBlock*, BasicBlock*>& blockMap,
    std::unordered_map<Value*, Value*>& valueMap) {
  std::vector<BasicBlock*> orderedBlocks;
  for (const auto& blockPtr : func->blocks()) {
    if (loop.blocks.count(blockPtr.get()))
      orderedBlocks.push_back(blockPtr.get());
  }
  if (orderedBlocks.size() != loop.blocks.size())
    return false;

  for (BasicBlock* sourceBlock : orderedBlocks)
    blockMap[sourceBlock] = func->newBlock("loop.divisor.direct");

  for (BasicBlock* sourceBlock : orderedBlocks) {
    BasicBlock* targetBlock = blockMap.at(sourceBlock);
    for (const auto& instructionPtr : sourceBlock->instructions()) {
      Instruction* source = instructionPtr.get();
      bool isTargetDivision =
          source->opcode() == Opcode::SDiv && source->numOperands() == 2 &&
          source->operand(1) == divisor;
      Opcode opcode = isTargetDivision ? Opcode::SDivInvariantDirect
                                       : source->opcode();
      Instruction* clone = cloneInstructionShell(source, opcode);
      targetBlock->pushBack(clone);
      valueMap[source] = clone;
    }
  }

  for (BasicBlock* sourceBlock : orderedBlocks) {
    BasicBlock* targetBlock = blockMap.at(sourceBlock);
    size_t targetIndex = 0;
    for (const auto& instructionPtr : sourceBlock->instructions()) {
      Instruction* source = instructionPtr.get();
      Instruction* clone = targetBlock->instructions()[targetIndex++].get();
      bool isTargetDivision =
          source->opcode() == Opcode::SDiv && source->numOperands() == 2 &&
          source->operand(1) == divisor;
      if (isTargetDivision) {
        clone->addOperand(remapValue(source->operand(0), valueMap, blockMap));
        clone->addOperand(parts.directDescriptor);
        clone->addOperand(parts.directDescriptor);
        continue;
      }
      for (Value* operand : source->operands())
        clone->addOperand(remapValue(operand, valueMap, blockMap));
    }
  }
  return true;
}

bool addClonedExitPhiInputs(
    const analysis::NaturalLoop& loop,
    const std::unordered_map<BasicBlock*, BasicBlock*>& blockMap,
    const std::unordered_map<Value*, Value*>& valueMap) {
  for (BasicBlock* sourceBlock : loop.blocks) {
    BasicBlock* clonedSource = blockMap.at(sourceBlock);
    for (BasicBlock* successor : sourceBlock->succs()) {
      if (!successor || loop.blocks.count(successor))
        continue;
      for (Instruction* phi : analysis::collectLeadingPhi(successor)) {
        Value* incoming = incomingFromPredecessor(phi, sourceBlock);
        if (!incoming)
          return false;
        phi->addOperand(remapValue(incoming, valueMap, blockMap));
        phi->addOperand(clonedSource);
      }
    }
  }
  return true;
}

BasicBlock* createForwardingBlock(Function* func, BasicBlock* target,
                                  const char* name) {
  BasicBlock* block = func->newBlock(name);
  auto* branch = new Instruction(Opcode::Br, Type::voidTy());
  branch->addOperand(target);
  block->pushBack(branch);
  return block;
}

bool addDirectDivisionVersion(Function* func,
                              const analysis::NaturalLoop& loop,
                              BasicBlock* preheader, Value* divisor,
                              const DescriptorParts& parts) {
  if (!func || !preheader || !parts.directFlag ||
      !loopCanBeVersioned(loop, divisor)) {
    return false;
  }
  Instruction* terminator = preheader->terminator();
  if (!terminator || terminator->opcode() != Opcode::Br ||
      terminator->numOperands() != 1 || terminator->operand(0) != loop.header) {
    return false;
  }

  std::unordered_map<BasicBlock*, BasicBlock*> blockMap;
  std::unordered_map<Value*, Value*> valueMap;
  if (!cloneDirectDivisionLoop(func, loop, divisor, parts, blockMap,
                               valueMap) ||
      !addClonedExitPhiInputs(loop, blockMap, valueMap)) {
    return false;
  }

  BasicBlock* directHeader = blockMap.at(loop.header);
  BasicBlock* directPreheader =
      createForwardingBlock(func, directHeader, "loop.divisor.direct.entry");
  BasicBlock* generalPreheader =
      createForwardingBlock(func, loop.header, "loop.divisor.general.entry");

  // magic 和 shift 在直接路径前置块中再拆出，使其活跃区间紧贴热点循环，
  // 避免整函数长活跃区间迫使每次迭代都从栈中重新加载。
  Instruction* directTerminator = directPreheader->terminator();
  auto* localMagic = new Instruction(Opcode::SDivDirectMagic, Type::i32(),
                                     "direct.divisor.magic.local");
  localMagic->addOperand(parts.directDescriptor);
  directPreheader->insertBefore(directTerminator, localMagic);
  auto* localShift = new Instruction(Opcode::SDivDirectShift, Type::i32(),
                                     "direct.divisor.shift.local");
  localShift->addOperand(parts.directDescriptor);
  directPreheader->insertBefore(directTerminator, localShift);
  for (const auto& mappedBlock : blockMap) {
    for (const auto& instructionPtr : mappedBlock.second->instructions()) {
      Instruction* instruction = instructionPtr.get();
      if (!instruction ||
          instruction->opcode() != Opcode::SDivInvariantDirect) {
        continue;
      }
      instruction->setOperand(1, localMagic);
      instruction->setOperand(2, localShift);
    }
  }
  analysis::replacePhiIncomingPred(loop.header, preheader, generalPreheader);
  analysis::replacePhiIncomingPred(directHeader, preheader, directPreheader);

  preheader->remove(terminator);
  auto* dispatch = new Instruction(Opcode::CondBr, Type::voidTy());
  dispatch->addOperand(parts.directFlag);
  dispatch->addOperand(directPreheader);
  dispatch->addOperand(generalPreheader);
  preheader->pushBack(dispatch);
  func->recomputeCFG();
  return true;
}

bool applyInvariantDescriptorCandidate(
    Function* func, const analysis::NaturalLoop& loop, BasicBlock* preheader,
    Value* divisor, int uses,
    std::unordered_map<Value*, DescriptorParts>& descriptorCache) {
  if (!func || !loop.header || !preheader || !divisor || uses <= 0)
    return false;
  Instruction* preTerminator = preheader->terminator();
  if (!preTerminator || preTerminator->opcode() != Opcode::Br ||
      preTerminator->numOperands() != 1 ||
      preTerminator->operand(0) != loop.header) {
    return false;
  }

  std::vector<Instruction*> divisions;
  for (BasicBlock* block : loop.blocks) {
    if (!block)
      continue;
    for (const auto& instructionPtr : block->instructions()) {
      Instruction* instruction = instructionPtr.get();
      if (instruction && instruction->opcode() == Opcode::SDiv &&
          instruction->numOperands() == 2 &&
          instruction->operand(1) == divisor) {
        divisions.push_back(instruction);
      }
    }
  }
  if (divisions.empty())
    return false;
  if (!loopCanBeVersioned(loop, divisor)) {
    trace("reject", func, &loop, "versioning-shape", divisor);
    return false;
  }

  DescriptorParts parts;
  auto cached = descriptorCache.find(divisor);
  if (cached == descriptorCache.end()) {
    parts = createDescriptorParts(func, divisor, preheader);
    if (!parts.directDescriptor || !parts.directFlag) {
      return false;
    }
    descriptorCache.emplace(divisor, parts);
  } else {
    parts = cached->second;
  }

  const bool directVersioned =
      addDirectDivisionVersion(func, loop, preheader, divisor, parts);

  trace(directVersioned ? "apply" : "reject", func, &loop,
        "runtime-direct-proof uses=" + std::to_string(divisions.size()) +
            (directVersioned ? " versioned" : " versioning-failed"),
        divisor);
  return directVersioned;
}

bool runOnFunction(Function* func,
                   analysis::AnalysisManager& analysisManager) {
  if (!func || !func->entryBlock())
    return false;

  bool changed = false;
  std::unordered_map<Value*, DescriptorParts> descriptorCache;
  std::unordered_set<BasicBlock*> processedHeaders;

  // 每次 CFG 版本化后重新获取循环分析，避免继续使用旧循环引用。预算只
  // 限制极端多除数函数的编译期代码增长，不参与运行时触发判定。
  int rewriteBudget = 16;
  while (rewriteBudget-- > 0) {
    func->recomputeCFG();
    analysisManager.invalidateFunction(func,
                                       analysis::AnalysisInvalidation::CFG);
    analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
    const auto& loops = cache.loops();

    // 内层循环也属于外层循环的块集合。先处理块数较少的内层循环，避免把
    // 内层除法误认为外层候选，并让描述符尽量靠近真正使用它的循环。
    std::vector<const analysis::NaturalLoop*> orderedLoops;
    orderedLoops.reserve(loops.size());
    for (const auto& loop : loops)
      orderedLoops.push_back(&loop);
    std::sort(orderedLoops.begin(), orderedLoops.end(),
              [](const analysis::NaturalLoop* lhs,
                 const analysis::NaturalLoop* rhs) {
                return lhs->blocks.size() < rhs->blocks.size();
              });

    bool rewroteOne = false;
    for (const analysis::NaturalLoop* loopPtr : orderedLoops) {
      const analysis::NaturalLoop& loop = *loopPtr;
      if (processedHeaders.count(loop.header))
        continue;
      // 版本化会复制整个循环体。只处理不含子循环的最内层循环，避免外层
      // 再次复制已经处理过的除法和整棵循环嵌套。
      if (containsProperSubloop(loop, loops)) {
        trace("reject", func, &loop, "contains-subloop");
        continue;
      }
      if (!isDescriptorCandidateShape(loop)) {
        trace("reject", func, &loop, "descriptor-cost");
        continue;
      }
      BasicBlock* preheader = analysis::dedicatedPreheader(loop);
      if (!preheader) {
        trace("reject", func, &loop, "no-preheader");
        continue;
      }

      std::vector<DivisorInfo> divisors = findInvariantDivisors(loop);
      if (divisors.empty()) {
        trace("reject", func, &loop, "no-invariant-divisor");
        continue;
      }
      for (const DivisorInfo& divisor : divisors) {
        if (!applyInvariantDescriptorCandidate(
                func, loop, preheader, divisor.divisor, divisor.uses,
                descriptorCache)) {
          continue;
        }
        changed = true;
        rewroteOne = true;
        processedHeaders.insert(loop.header);
        break;
      }
      if (rewroteOne)
        break;
    }
    if (!rewroteOne)
      break;
  }
  return changed;
}

} // namespace

bool applyLoopInvariantDivisorSpecialization(
    Module& mod, analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    if (runOnFunction(funcPtr.get(), analysisManager))
      changed = true;
  }
  if (changed)
    analysisManager.invalidateModule(analysis::AnalysisInvalidation::Module);
  return changed;
}

class DivisorSpecializationPass : public Pass {
public:
  std::string name() const override { return "DivisorSpecialization"; }
  bool run(Module& mod, analysis::AnalysisManager& am) override {
    return applyLoopInvariantDivisorSpecialization(mod, am);
  }
};
REGISTER_PASS(DivisorSpecializationPass);

} // namespace core_ir
