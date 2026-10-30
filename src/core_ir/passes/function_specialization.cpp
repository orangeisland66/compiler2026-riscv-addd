// 文件职责：实现 LLVM 风格的 FunctionSpecialization，仅负责按常量实参克隆并重写调用点。
#include "pass_registry.h"
#include "function_specialization.h"

#include "../../analysis/cfg_utils.h"
#include "../../analysis/function_effects.h"
#include "../../analysis/function_analysis.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace core_ir {

namespace {

struct ConstantKey {
  Type type = Type::i32();
  int32_t bits = 0;

  bool operator==(const ConstantKey& other) const {
    return type == other.type && bits == other.bits;
  }
};

struct ConstantKeyHash {
  size_t operator()(const ConstantKey& key) const {
    return static_cast<size_t>(key.type.kind) * 1315423911u ^
           static_cast<size_t>(key.type.ptrDepth * 65537u) ^
           static_cast<size_t>(static_cast<uint32_t>(key.bits));
  }
};

struct SpecializationKey {
  Function* callee = nullptr;
  int argIndex = -1;
  ConstantKey constant;

  bool operator==(const SpecializationKey& other) const {
    return callee == other.callee && argIndex == other.argIndex &&
           constant == other.constant;
  }
};

struct SpecializationKeyHash {
  size_t operator()(const SpecializationKey& key) const {
    return reinterpret_cast<size_t>(key.callee) ^
           (static_cast<size_t>(key.argIndex) << 8) ^
           ConstantKeyHash{}(key.constant);
  }
};

struct CallSite {
  Function* caller = nullptr;
  BasicBlock* bb = nullptr;
  Instruction* call = nullptr;
  bool inLoop = false;
};

struct CandidateGroup {
  SpecializationKey key;
  int benefitScore = 0;
  std::vector<CallSite> callSites;
};

struct EqualArgSpecializationKey {
  Function* callee = nullptr;
  std::vector<int> representatives;

  bool operator==(const EqualArgSpecializationKey& other) const {
    return callee == other.callee && representatives == other.representatives;
  }
};

struct EqualArgSpecializationKeyHash {
  size_t operator()(const EqualArgSpecializationKey& key) const {
    size_t hash = reinterpret_cast<size_t>(key.callee);
    for (int rep : key.representatives) {
      hash ^= static_cast<size_t>(rep + 0x9e3779b9) + (hash << 6) + (hash >> 2);
    }
    return hash;
  }
};

struct EqualArgCandidate {
  EqualArgSpecializationKey key;
  std::vector<int> uniqueArgIndices;
  std::vector<CallSite> callSites;
};

int directUserBenefitScore(Value* value, Instruction* user) {
  if (!value || !user)
    return 0;
  switch (user->opcode()) {
  case Opcode::ICmp:
  case Opcode::FCmp:
    return 4;
  case Opcode::Shl:
  case Opcode::Shr:
    return user->numOperands() >= 2 && user->operand(1) == value ? 4 : 1;
  case Opcode::SDiv:
  case Opcode::SRem:
    return user->numOperands() >= 2 && user->operand(1) == value ? 4 : 2;
  case Opcode::Call:
    return 3;
  case Opcode::Phi:
  case Opcode::Mov:
  case Opcode::BitCast:
  case Opcode::SIToFP:
  case Opcode::FPToSI:
    return 0;
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Mul:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
  case Opcode::Neg:
  case Opcode::FNeg:
    return 1;
  default:
    return 0;
  }
}

int specializationUseScore(Value* value,
                           std::unordered_set<Value*>& visiting) {
  if (!value || !visiting.insert(value).second)
    return 0;
  int score = 0;
  for (Instruction* user : value->uses()) {
    if (!user)
      continue;
    score += directUserBenefitScore(value, user);
    switch (user->opcode()) {
    case Opcode::Phi:
    case Opcode::Mov:
    case Opcode::BitCast:
    case Opcode::SIToFP:
    case Opcode::FPToSI:
    case Opcode::Add:
    case Opcode::Sub:
    case Opcode::Mul:
    case Opcode::And:
    case Opcode::Or:
    case Opcode::Xor:
    case Opcode::Neg:
    case Opcode::FNeg:
      score += specializationUseScore(user, visiting);
      break;
    default:
      break;
    }
    if (score >= 64)
      return 64;
  }
  return score;
}

int specializationUseScore(Function* callee, int argIndex) {
  if (!callee || argIndex < 0 ||
      argIndex >= static_cast<int>(callee->args().size()) ||
      !callee->args()[argIndex].val) {
    return 0;
  }
  std::unordered_set<Value*> visiting;
  return specializationUseScore(callee->args()[argIndex].val, visiting);
}

bool specializedArgumentHasInterestingUse(Function* callee, int argIndex) {
  return specializationUseScore(callee, argIndex) > 0;
}

std::optional<ConstantKey> constantKeyForValue(Value* value) {
  if (auto* ci = dynamic_cast<ConstInt*>(value))
    return ConstantKey{Type::i32(), ci->value()};
  if (auto* cf = dynamic_cast<ConstFloat*>(value)) {
    int32_t bits = 0;
    float f = cf->value();
    std::memcpy(&bits, &f, sizeof(bits));
    return ConstantKey{Type::f32(), bits};
  }
  return std::nullopt;
}

Value* materializeConstant(Module& mod, const ConstantKey& key) {
  if (key.type.isI32())
    return mod.getConstInt(key.bits);
  if (key.type.isF32()) {
    float value = 0.0f;
    int32_t bits = key.bits;
    std::memcpy(&value, &bits, sizeof(value));
    return mod.getConstFloat(value);
  }
  return nullptr;
}

std::string constantSuffix(const ConstantKey& key) {
  if (key.type.isI32()) {
    return key.bits >= 0 ? "i32_p" + std::to_string(key.bits)
                         : "i32_n" + std::to_string(-key.bits);
  }
  uint32_t bits = static_cast<uint32_t>(key.bits);
  return "f32_0x" + std::to_string(bits);
}

bool isSelfRecursionPassthrough(Function* callee, int argIndex) {
  if (!callee || argIndex < 0 ||
      argIndex >= static_cast<int>(callee->args().size()) ||
      !callee->args()[argIndex].val) {
    return false;
  }
  Value* formal = callee->args()[argIndex].val;
  bool sawSelfCall = false;
  for (auto& bbPtr : callee->blocks()) {
    for (auto& instPtr : bbPtr->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst || inst->opcode() != Opcode::Call || inst->name() != callee->name())
        continue;
      sawSelfCall = true;
      if (argIndex >= inst->numOperands() || inst->operand(argIndex) != formal)
        return false;
    }
  }
  return sawSelfCall;
}

bool canSpecializeFunction(
    Module& mod,
    Function* callee,
    const analysis::FunctionEffectSummary* summary,
    int argIndex,
    int benefitScore) {
  if (!callee || !summary || callee->name() == "main" ||
      mod.isBuiltin(callee->name()) || argIndex < 0 ||
      argIndex >= static_cast<int>(callee->args().size())) {
    return false;
  }
  Type argTy = callee->args()[argIndex].type;
  if (!argTy.isI32() && !argTy.isF32())
    return false;
  if (summary->instructionCount <= 0 || summary->instructionCount > 96 ||
      summary->loopCount > 2 || callee->blocks().empty() ||
      callee->blocks().size() > 24) {
    return false;
  }
  if (benefitScore <= 0)
    return false;
  if (!summary->noRecurse && !summary->selfRecursive)
    return false;
  if (summary->selfRecursive) {
    if (summary->readsUnknownMemory)
      return false;
    if (!isSelfRecursionPassthrough(callee, argIndex))
      return false;
  }
  return true;
}

bool allowSingleCallsiteSpecialization(
    Function* callee,
    const analysis::FunctionEffectSummary* summary,
    int benefitScore) {
  if (!callee || !summary)
    return false;
  return summary->noRecurse && !summary->selfRecursive &&
         summary->loopCount == 0 &&
         summary->instructionCount > 0 &&
         summary->instructionCount <= 64 &&
         callee->blocks().size() <= 24 &&
         (summary->pure || summary->readonly) &&
         benefitScore >= 24;
}

bool hasLoopCallSite(Function* caller,
                     Instruction* call,
                     analysis::AnalysisManager& analysisManager) {
  if (!caller || !call || !call->parent())
    return false;
  analysis::FunctionAnalysisCache& cache = analysisManager.function(caller);
  for (const auto& loop : cache.loops()) {
    if (loop.blocks.count(call->parent()))
      return true;
  }
  return false;
}

Instruction* cloneInstructionShell(Instruction* src) {
  auto* clone = new Instruction(src->opcode(), src->type(), src->name());
  clone->setMemoryInfo(src->memoryElemTy(), src->memorySizeBytes(),
                       src->isArrayObject(), src->dims());
  return clone;
}

Function* cloneSpecializedFunction(Module& mod,
                                   Function* callee,
                                   int argIndex,
                                   const ConstantKey& constant) {
  if (!callee || argIndex < 0 ||
      argIndex >= static_cast<int>(callee->args().size())) {
    return nullptr;
  }
  Value* constValue = materializeConstant(mod, constant);
  if (!constValue)
    return nullptr;

  std::string name = callee->name() + ".spec.arg" + std::to_string(argIndex) +
                     "." + constantSuffix(constant);
  if (mod.getFunction(name))
    return mod.getFunction(name);

  Function* clone = mod.newFunction(name, callee->retTy());
  std::unordered_map<Value*, Value*> valueMap;
  for (int i = 0; i < static_cast<int>(callee->args().size()); ++i) {
    if (i == argIndex) {
      valueMap[callee->args()[i].val] = constValue;
      continue;
    }
    Value* arg = clone->addArg(callee->args()[i].name, callee->args()[i].type);
    valueMap[callee->args()[i].val] = arg;
  }

  std::unordered_map<BasicBlock*, BasicBlock*> blockMap;
  for (auto& bbPtr : callee->blocks()) {
    BasicBlock* srcBB = bbPtr.get();
    BasicBlock* dstBB = clone->newBlock("spec");
    blockMap[srcBB] = dstBB;
    if (srcBB == callee->entryBlock())
      clone->setEntryBlock(dstBB);
  }

  for (auto& bbPtr : callee->blocks()) {
    BasicBlock* srcBB = bbPtr.get();
    BasicBlock* dstBB = blockMap.at(srcBB);
    for (auto& instPtr : srcBB->instructions()) {
      Instruction* srcInst = instPtr.get();
      Instruction* dstInst = cloneInstructionShell(srcInst);
      dstBB->pushBack(dstInst);
      valueMap[srcInst] = dstInst;
      if (dstInst->opcode() == Opcode::Alloca)
        clone->addAlloca(dstInst);
    }
  }

  for (auto& bbPtr : callee->blocks()) {
    BasicBlock* srcBB = bbPtr.get();
    BasicBlock* dstBB = blockMap.at(srcBB);
    size_t dstIndex = 0;
    for (auto& instPtr : srcBB->instructions()) {
      Instruction* srcInst = instPtr.get();
      Instruction* dstInst = dstBB->instructions()[dstIndex++].get();
      bool selfRecursiveCall =
          dstInst->opcode() == Opcode::Call && srcInst->name() == callee->name();
      for (int operandIndex = 0; operandIndex < srcInst->numOperands();
           ++operandIndex) {
        if (selfRecursiveCall && operandIndex == argIndex)
          continue;
        Value* operand = srcInst->operand(operandIndex);
        auto mappedValue = valueMap.find(operand);
        if (mappedValue != valueMap.end()) {
          dstInst->addOperand(mappedValue->second);
          continue;
        }
        auto mappedBlock = blockMap.find(dynamic_cast<BasicBlock*>(operand));
        if (mappedBlock != blockMap.end()) {
          dstInst->addOperand(mappedBlock->second);
          continue;
        }
        dstInst->addOperand(operand);
      }
      if (selfRecursiveCall)
        dstInst->setName(clone->name());
    }
  }

  clone->recomputeCFG();
  return clone;
}

std::string equalArgSuffix(const std::vector<int>& representatives,
                           int uniqueCount) {
  size_t hash = 1469598103934665603ull;
  for (int rep : representatives) {
    hash ^= static_cast<size_t>(rep + 1);
    hash *= 1099511628211ull;
  }
  return "eqargs." + std::to_string(representatives.size()) + "to" +
         std::to_string(uniqueCount) + ".h" + std::to_string(hash);
}

Function* cloneEqualArgSpecializedFunction(
    Module& mod,
    Function* callee,
    const std::vector<int>& representatives,
    const std::vector<int>& uniqueArgIndices) {
  if (!callee || representatives.size() != callee->args().size() ||
      uniqueArgIndices.empty()) {
    return nullptr;
  }

  std::string name = callee->name() + "." +
                     equalArgSuffix(representatives,
                                    static_cast<int>(uniqueArgIndices.size()));
  if (mod.getFunction(name))
    return mod.getFunction(name);

  Function* clone = mod.newFunction(name, callee->retTy());
  std::unordered_map<int, Value*> argForRepresentative;
  std::unordered_map<Value*, Value*> valueMap;

  for (int argIndex : uniqueArgIndices) {
    if (argIndex < 0 || argIndex >= static_cast<int>(callee->args().size()))
      return nullptr;
    Value* arg = clone->addArg(callee->args()[argIndex].name,
                               callee->args()[argIndex].type);
    argForRepresentative[argIndex] = arg;
  }

  for (int i = 0; i < static_cast<int>(callee->args().size()); ++i) {
    int rep = representatives[static_cast<size_t>(i)];
    auto it = argForRepresentative.find(rep);
    if (it == argForRepresentative.end())
      return nullptr;
    valueMap[callee->args()[i].val] = it->second;
  }

  std::unordered_map<BasicBlock*, BasicBlock*> blockMap;
  for (auto& bbPtr : callee->blocks()) {
    BasicBlock* srcBB = bbPtr.get();
    BasicBlock* dstBB = clone->newBlock("eqspec");
    blockMap[srcBB] = dstBB;
    if (srcBB == callee->entryBlock())
      clone->setEntryBlock(dstBB);
  }

  for (auto& bbPtr : callee->blocks()) {
    BasicBlock* srcBB = bbPtr.get();
    BasicBlock* dstBB = blockMap.at(srcBB);
    for (auto& instPtr : srcBB->instructions()) {
      Instruction* srcInst = instPtr.get();
      Instruction* dstInst = cloneInstructionShell(srcInst);
      dstBB->pushBack(dstInst);
      valueMap[srcInst] = dstInst;
      if (dstInst->opcode() == Opcode::Alloca)
        clone->addAlloca(dstInst);
    }
  }

  for (auto& bbPtr : callee->blocks()) {
    BasicBlock* srcBB = bbPtr.get();
    BasicBlock* dstBB = blockMap.at(srcBB);
    size_t dstIndex = 0;
    for (auto& instPtr : srcBB->instructions()) {
      Instruction* srcInst = instPtr.get();
      Instruction* dstInst = dstBB->instructions()[dstIndex++].get();
      for (int operandIndex = 0; operandIndex < srcInst->numOperands();
           ++operandIndex) {
        Value* operand = srcInst->operand(operandIndex);
        auto mappedValue = valueMap.find(operand);
        if (mappedValue != valueMap.end()) {
          dstInst->addOperand(mappedValue->second);
          continue;
        }
        auto mappedBlock = blockMap.find(dynamic_cast<BasicBlock*>(operand));
        if (mappedBlock != blockMap.end()) {
          dstInst->addOperand(mappedBlock->second);
          continue;
        }
        dstInst->addOperand(operand);
      }
    }
  }

  clone->recomputeCFG();
  return clone;
}

void resetCallOperands(Instruction* call, const std::vector<Value*>& newOperands) {
  if (!call)
    return;
  call->dropAllOperands();
  for (Value* operand : newOperands)
    call->addOperand(operand);
}

std::vector<CandidateGroup> collectCandidates(
    Module& mod,
    analysis::AnalysisManager& analysisManager) {
  const analysis::FunctionEffectMap& effects = analysisManager.functionEffects();
  std::unordered_map<SpecializationKey, CandidateGroup, SpecializationKeyHash>
      groups;

  for (auto& funcPtr : mod.functions()) {
    Function* caller = funcPtr.get();
    if (!caller)
      continue;
    for (auto& bbPtr : caller->blocks()) {
      BasicBlock* bb = bbPtr.get();
      for (auto& instPtr : bb->instructions()) {
        Instruction* call = instPtr.get();
        if (!call || call->opcode() != Opcode::Call)
          continue;
        Function* callee = mod.getFunction(call->name());
        if (!callee)
          continue;
        const auto* summary = analysis::lookupFunctionEffect(effects, callee);
        for (int argIndex = 0; argIndex < call->numOperands(); ++argIndex) {
          auto constant = constantKeyForValue(call->operand(argIndex));
          int benefitScore = specializationUseScore(callee, argIndex);
          if (!constant ||
              !canSpecializeFunction(mod, callee, summary, argIndex,
                                     benefitScore))
            continue;
          SpecializationKey key{callee, argIndex, *constant};
          auto& group = groups[key];
          group.key = key;
          group.benefitScore = benefitScore;
          group.callSites.push_back(
              {caller, bb, call, hasLoopCallSite(caller, call, analysisManager)});
        }
      }
    }
  }

  std::unordered_map<Function*, int> clonesPerFunction;
  int moduleCloneBudget = 12;
  std::vector<CandidateGroup*> orderedGroups;
  orderedGroups.reserve(groups.size());
  for (auto& entry : groups)
    orderedGroups.push_back(&entry.second);
  std::sort(orderedGroups.begin(), orderedGroups.end(),
            [](const CandidateGroup* lhs, const CandidateGroup* rhs) {
              auto score = [](const CandidateGroup* group) {
                bool hasLoopSite = false;
                for (const CallSite& site : group->callSites) {
                  if (site.inLoop) {
                    hasLoopSite = true;
                    break;
                  }
                }
                return std::pair<bool, size_t>(hasLoopSite, group->callSites.size());
              };
              auto lhsScore = score(lhs);
              auto rhsScore = score(rhs);
              if (lhsScore != rhsScore)
                return lhsScore > rhsScore;
              if (lhs->benefitScore != rhs->benefitScore)
                return lhs->benefitScore > rhs->benefitScore;
              if (lhs->key.callee->name() != rhs->key.callee->name())
                return lhs->key.callee->name() < rhs->key.callee->name();
              if (lhs->key.argIndex != rhs->key.argIndex)
                return lhs->key.argIndex < rhs->key.argIndex;
              if (lhs->key.constant.type.kind != rhs->key.constant.type.kind)
                return static_cast<int>(lhs->key.constant.type.kind) <
                       static_cast<int>(rhs->key.constant.type.kind);
              return lhs->key.constant.bits < rhs->key.constant.bits;
            });

  std::vector<CandidateGroup> accepted;
  accepted.reserve(groups.size());
  std::unordered_set<Instruction*> claimedCalls;
  for (CandidateGroup* groupPtr : orderedGroups) {
    CandidateGroup& group = *groupPtr;
    CandidateGroup filtered;
    filtered.key = group.key;
    for (const CallSite& site : group.callSites) {
      if (!site.call || claimedCalls.count(site.call))
        continue;
      filtered.callSites.push_back(site);
    }
    bool hasLoopSite = false;
    for (const CallSite& site : filtered.callSites) {
      if (site.inLoop) {
        hasLoopSite = true;
        break;
      }
    }
    const auto* summary = analysis::lookupFunctionEffect(effects, group.key.callee);
    bool singleCallsiteAllowed =
        filtered.callSites.size() == 1 && !hasLoopSite &&
        allowSingleCallsiteSpecialization(group.key.callee, summary,
                                          group.benefitScore);
    if (filtered.callSites.size() < 2 && !hasLoopSite && !singleCallsiteAllowed)
      continue;
    int& cloneCount = clonesPerFunction[group.key.callee];
    if (cloneCount >= 3 || moduleCloneBudget <= 0)
      continue;
    ++cloneCount;
    --moduleCloneBudget;
    for (const CallSite& site : filtered.callSites)
      claimedCalls.insert(site.call);
    accepted.push_back(std::move(filtered));
  }
  return accepted;
}

bool buildEqualArgPattern(Instruction* call,
                          std::vector<int>& representatives,
                          std::vector<int>& uniqueArgIndices) {
  representatives.clear();
  uniqueArgIndices.clear();
  if (!call || call->numOperands() <= 1)
    return false;

  representatives.reserve(static_cast<size_t>(call->numOperands()));
  for (int i = 0; i < call->numOperands(); ++i) {
    Value* operand = call->operand(i);
    int rep = i;
    for (int uniqueIndex : uniqueArgIndices) {
      Value* uniqueOperand = call->operand(uniqueIndex);
      if (operand == uniqueOperand && operand->type() == uniqueOperand->type()) {
        rep = uniqueIndex;
        break;
      }
    }
    if (rep == i)
      uniqueArgIndices.push_back(i);
    representatives.push_back(rep);
  }
  return uniqueArgIndices.size() < representatives.size();
}

std::vector<EqualArgCandidate> collectEqualArgCandidates(
    Module& mod,
    analysis::AnalysisManager& analysisManager) {
  const analysis::FunctionEffectMap& effects = analysisManager.functionEffects();
  std::unordered_map<EqualArgSpecializationKey, EqualArgCandidate,
                     EqualArgSpecializationKeyHash>
      groups;

  for (auto& funcPtr : mod.functions()) {
    Function* caller = funcPtr.get();
    if (!caller)
      continue;
    for (auto& bbPtr : caller->blocks()) {
      BasicBlock* bb = bbPtr.get();
      for (auto& instPtr : bb->instructions()) {
        Instruction* call = instPtr.get();
        if (!call || call->opcode() != Opcode::Call)
          continue;
        Function* callee = mod.getFunction(call->name());
        if (!callee || callee->name() == "main" || mod.isBuiltin(callee->name()))
          continue;
        if (call->numOperands() != static_cast<int>(callee->args().size()))
          continue;
        const auto* summary = analysis::lookupFunctionEffect(effects, callee);
        if (!summary || !summary->noRecurse || summary->selfRecursive ||
            !(summary->pure || summary->readonly)) {
          continue;
        }

        std::vector<int> representatives;
        std::vector<int> uniqueArgIndices;
        if (!buildEqualArgPattern(call, representatives, uniqueArgIndices))
          continue;

        const int removedArgs =
            static_cast<int>(representatives.size() - uniqueArgIndices.size());
        const bool inLoop = hasLoopCallSite(caller, call, analysisManager);
        if (!inLoop && removedArgs < 4)
          continue;
        if (removedArgs < 2)
          continue;

        EqualArgSpecializationKey key{callee, representatives};
        auto& group = groups[key];
        group.key = key;
        if (group.uniqueArgIndices.empty())
          group.uniqueArgIndices = uniqueArgIndices;
        group.callSites.push_back({caller, bb, call, inLoop});
      }
    }
  }

  std::vector<EqualArgCandidate*> orderedGroups;
  orderedGroups.reserve(groups.size());
  for (auto& entry : groups)
    orderedGroups.push_back(&entry.second);
  std::sort(orderedGroups.begin(), orderedGroups.end(),
            [](const EqualArgCandidate* lhs, const EqualArgCandidate* rhs) {
              auto score = [](const EqualArgCandidate* group) {
                bool hasLoopSite = false;
                for (const CallSite& site : group->callSites)
                  hasLoopSite |= site.inLoop;
                int removed = static_cast<int>(
                    group->key.representatives.size() -
                    group->uniqueArgIndices.size());
                return std::tuple<bool, int, size_t>(
                    hasLoopSite, removed, group->callSites.size());
              };
              auto lhsScore = score(lhs);
              auto rhsScore = score(rhs);
              if (lhsScore != rhsScore)
                return lhsScore > rhsScore;
              return lhs->key.callee->name() < rhs->key.callee->name();
            });

  std::vector<EqualArgCandidate> accepted;
  int moduleCloneBudget = 6;
  std::unordered_map<Function*, int> clonesPerFunction;
  std::unordered_set<Instruction*> claimedCalls;
  for (EqualArgCandidate* groupPtr : orderedGroups) {
    EqualArgCandidate& group = *groupPtr;
    EqualArgCandidate filtered;
    filtered.key = group.key;
    filtered.uniqueArgIndices = group.uniqueArgIndices;
    for (const CallSite& site : group.callSites) {
      if (!site.call || claimedCalls.count(site.call))
        continue;
      filtered.callSites.push_back(site);
    }
    if (filtered.callSites.empty())
      continue;
    int& cloneCount = clonesPerFunction[group.key.callee];
    if (cloneCount >= 2 || moduleCloneBudget <= 0)
      continue;
    ++cloneCount;
    --moduleCloneBudget;
    for (const CallSite& site : filtered.callSites)
      claimedCalls.insert(site.call);
    accepted.push_back(std::move(filtered));
  }
  return accepted;
}

bool applyEqualArgumentSpecialization(
    Module& mod,
    analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (int iter = 0; iter < 2; ++iter) {
    std::vector<EqualArgCandidate> candidates =
        collectEqualArgCandidates(mod, analysisManager);
    if (candidates.empty())
      break;

    bool iterChanged = false;
    std::unordered_map<EqualArgSpecializationKey, Function*,
                       EqualArgSpecializationKeyHash>
        clones;
    for (const EqualArgCandidate& candidate : candidates) {
      Function* specialized = nullptr;
      auto it = clones.find(candidate.key);
      if (it != clones.end()) {
        specialized = it->second;
      } else {
        specialized = cloneEqualArgSpecializedFunction(
            mod, candidate.key.callee, candidate.key.representatives,
            candidate.uniqueArgIndices);
        if (!specialized)
          continue;
        clones.emplace(candidate.key, specialized);
        iterChanged = true;
      }

      for (const CallSite& site : candidate.callSites) {
        if (!site.call || site.call->name() != candidate.key.callee->name())
          continue;
        std::vector<Value*> newOperands;
        newOperands.reserve(candidate.uniqueArgIndices.size());
        for (int argIndex : candidate.uniqueArgIndices)
          newOperands.push_back(site.call->operand(argIndex));
        resetCallOperands(site.call, newOperands);
        site.call->setName(specialized->name());
        iterChanged = true;
      }
    }

    if (!iterChanged)
      break;
    changed = true;
    analysisManager.invalidateModule(analysis::AnalysisInvalidation::Module);
  }
  return changed;
}

} // namespace

// 对常量实参调用点做函数特化：最多迭代 3 轮，每轮收集候选、按
// (callee, argIndex, constant) 复用克隆，并把调用点改指向去掉常量实参的
// 特化版本；最后补充等值实参特化。
bool applyFunctionSpecialization(Module& mod,
                                 analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (int iter = 0; iter < 3; ++iter) {
    std::vector<CandidateGroup> candidates =
        collectCandidates(mod, analysisManager);
    if (candidates.empty())
      break;

    bool iterChanged = false;
    std::unordered_map<SpecializationKey, Function*, SpecializationKeyHash> clones;
    for (const CandidateGroup& group : candidates) {
      Function* specialized = nullptr;
      auto it = clones.find(group.key);
      if (it != clones.end()) {
        specialized = it->second;
      } else {
        specialized = cloneSpecializedFunction(mod, group.key.callee,
                                               group.key.argIndex,
                                               group.key.constant);
        if (!specialized)
          continue;
        clones.emplace(group.key, specialized);
        iterChanged = true;
      }
      for (const CallSite& site : group.callSites) {
        if (!site.call || site.call->name() != group.key.callee->name())
          continue;
        std::vector<Value*> newOperands;
        newOperands.reserve(static_cast<size_t>(site.call->numOperands() - 1));
        // 重建实参列表时跳过被特化掉的常量实参，与特化版形参对齐。
        for (int i = 0; i < site.call->numOperands(); ++i) {
          if (i == group.key.argIndex)
            continue;
          newOperands.push_back(site.call->operand(i));
        }
        resetCallOperands(site.call, newOperands);
        site.call->setName(specialized->name());
        iterChanged = true;
      }
    }
    if (!iterChanged)
      break;
    changed = true;
    analysisManager.invalidateModule(analysis::AnalysisInvalidation::Module);
  }
  if (applyEqualArgumentSpecialization(mod, analysisManager))
    changed = true;
  return changed;
}

class FunctionSpecializationPass : public Pass {
public:
    std::string name() const override { return "FunctionSpecialization"; }
    bool run(Module& mod, analysis::AnalysisManager& am) override {
        return applyFunctionSpecialization(mod, am);
    }
};
REGISTER_PASS(FunctionSpecializationPass);

} // namespace core_ir
