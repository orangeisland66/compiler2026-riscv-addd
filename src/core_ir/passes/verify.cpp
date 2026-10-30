// 文件职责：检查 CoreIR CFG/SSA/PHI/类型/use-def/支配不变量。
#include "verify.h"

#include "pass_registry.h"
#include "analysis/cfg_utils.h"
#include "analysis/dominators.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace core_ir {

namespace {

std::string typeName(Type ty) {
  if (ty.isI32()) return "i32";
  if (ty.isF32()) return "f32";
  if (ty.isVoid()) return "void";
  if (ty.isPtr()) return "ptr";
  if (ty.isLabel()) return "label";
  if (ty.isV4I32()) return "v4i32";
  if (ty.isV4F32()) return "v4f32";
  return "unknown";
}

[[noreturn]] void fail(const std::string& code, const std::string& detail) {
  throw std::runtime_error(code + ": " + detail);
}

bool hasUser(const Value* value, const Instruction* user) {
  return std::find(value->uses().begin(), value->uses().end(), user) !=
         value->uses().end();
}

void verifyInstructionShape(const Instruction* inst, const Function* function,
                            const BasicBlock* block) {
  auto require = [&](bool condition, const char* code) {
    if (!condition) fail(code, block->name());
  };
  switch (inst->opcode()) {
  case Opcode::Br:
    require(inst->numOperands() == 1 &&
                dynamic_cast<BasicBlock*>(inst->operand(0)),
            "bad-branch");
    break;
  case Opcode::CondBr:
    require(inst->numOperands() == 3 && inst->operand(0)->type().isI32() &&
                dynamic_cast<BasicBlock*>(inst->operand(1)) &&
                dynamic_cast<BasicBlock*>(inst->operand(2)),
            "bad-conditional-branch");
    break;
  case Opcode::Ret:
    if (function->retTy().isVoid())
      require(inst->numOperands() == 0, "bad-void-return");
    else
      require(inst->numOperands() == 1 &&
                  inst->operand(0)->type() == function->retTy(),
              "return-type-mismatch");
    break;
  case Opcode::Phi:
    require(inst->numOperands() >= 2 && inst->numOperands() % 2 == 0,
            "bad-phi-arity");
    for (int index = 0; index < inst->numOperands(); index += 2) {
      require(inst->operand(index)->type() == inst->type(),
              "phi-type-mismatch");
      require(dynamic_cast<BasicBlock*>(inst->operand(index + 1)),
              "bad-phi-predecessor");
    }
    break;
  case Opcode::Load:
  case Opcode::VLoad:
    require(inst->numOperands() == 1 && inst->operand(0)->type().isPtr(),
            "bad-load");
    break;
  case Opcode::Store:
  case Opcode::VStore:
    if (inst->numOperands() != 2 || !inst->operand(1)->type().isPtr()) {
      std::string detail = block->name();
      if (inst->numOperands() > 1) {
        detail += " address=";
        detail += typeName(inst->operand(1)->type());
        if (auto* address = dynamic_cast<Instruction*>(inst->operand(1))) {
          detail += "/";
          detail += opcodeName(address->opcode());
          detail += "/" + address->name();
        }
      }
      fail("bad-store", detail);
    }
    break;
  case Opcode::GEP:
    require(inst->numOperands() >= 1 && inst->operand(0)->type().isPtr() &&
                inst->type().isPtr(),
            "bad-gep");
    break;
  case Opcode::Select:
    require(inst->numOperands() == 3 && inst->operand(0)->type().isI32() &&
                inst->operand(1)->type() == inst->operand(2)->type() &&
                inst->type() == inst->operand(1)->type(),
            "bad-select");
    break;
  default:
    break;
  }
}

void verifyFunction(const Function* function) {
  if (!function->entryBlock())
    fail("missing-entry", function->name());
  std::vector<BasicBlock*> blocks;
  std::unordered_set<BasicBlock*> blockSet;
  std::unordered_set<const Value*> localValues;
  std::unordered_map<const Instruction*, std::size_t> instructionIndex;
  for (const auto& argument : function->args()) localValues.insert(argument.val);
  for (const auto& owned : function->blocks()) {
    blocks.push_back(owned.get());
    blockSet.insert(owned.get());
    localValues.insert(owned.get());
    for (std::size_t index = 0; index < owned->instructions().size(); ++index) {
      localValues.insert(owned->instructions()[index].get());
      instructionIndex[owned->instructions()[index].get()] = index;
    }
  }
  auto reachable = analysis::collectReachableBlocks(
      const_cast<Function*>(function));
  auto dominators = analysis::computeDominators(
      const_cast<Function*>(function), blocks, reachable);

  for (const auto& ownedBlock : function->blocks()) {
    auto* block = ownedBlock.get();
    bool sawNonPhi = false;
    bool sawTerminator = false;
    std::unordered_set<BasicBlock*> phiPredecessors;
    for (std::size_t index = 0; index < block->instructions().size(); ++index) {
      auto* inst = block->instructions()[index].get();
      if (sawTerminator)
        fail("instruction-after-terminator", block->name());
      if (inst->parent() != block)
        fail("instruction-parent-mismatch", block->name());
      if (inst->opcode() == Opcode::Phi && sawNonPhi)
        fail("phi-not-leading", block->name());
      sawNonPhi = sawNonPhi || inst->opcode() != Opcode::Phi;
      for (int operandIndex = 0; operandIndex < inst->numOperands();
           ++operandIndex) {
        Value* operand = inst->operand(operandIndex);
        if (!operand || !Value::isLiveValue(operand))
          fail("null-or-dead-operand", std::string(opcodeName(inst->opcode())));
        if (!hasUser(operand, inst))
          fail("missing-use-list-edge", std::string(opcodeName(inst->opcode())));
        if (auto* operandBlock = dynamic_cast<BasicBlock*>(operand)) {
          if (!blockSet.count(operandBlock))
            fail("foreign-basic-block", block->name());
          continue;
        }
        auto* definition = dynamic_cast<Instruction*>(operand);
        if (!definition) continue;
        if (!localValues.count(definition))
          fail("foreign-instruction", block->name());
        if (inst->opcode() == Opcode::Phi && operandIndex % 2 == 0) {
          auto* predecessor = dynamic_cast<BasicBlock*>(
              inst->operand(operandIndex + 1));
          if (!predecessor ||
              !analysis::dominates(dominators, definition->parent(),
                                   predecessor))
            fail("phi-value-does-not-dominate-edge", block->name());
        } else if (definition->parent() == block) {
          if (instructionIndex[definition] >= index)
            fail("use-before-definition", block->name());
        } else if (!analysis::dominates(dominators, definition->parent(), block)) {
          fail("definition-does-not-dominate-use", block->name());
        }
      }
      verifyInstructionShape(inst, function, block);
      if (inst->opcode() == Opcode::Phi) {
        phiPredecessors.clear();
        for (int operandIndex = 1; operandIndex < inst->numOperands();
             operandIndex += 2) {
          auto* predecessor =
              dynamic_cast<BasicBlock*>(inst->operand(operandIndex));
          if (!phiPredecessors.insert(predecessor).second)
            fail("duplicate-phi-predecessor", block->name());
        }
        std::unordered_set<BasicBlock*> cfgPredecessors(block->preds().begin(),
                                                        block->preds().end());
        if (phiPredecessors != cfgPredecessors)
          fail("phi-predecessor-mismatch", block->name());
      }
      sawTerminator = inst->isTerminator();
    }
    if (!block->terminator())
      fail("missing-terminator", block->name());
    for (auto* successor : block->succs()) {
      if (!successor || !blockSet.count(successor) ||
          std::find(successor->preds().begin(), successor->preds().end(),
                    block) == successor->preds().end())
        fail("cfg-successor-mismatch", block->name());
    }
    for (auto* predecessor : block->preds()) {
      if (!predecessor || !blockSet.count(predecessor) ||
          std::find(predecessor->succs().begin(), predecessor->succs().end(),
                    block) == predecessor->succs().end())
        fail("cfg-predecessor-mismatch", block->name());
    }
  }
}

} // namespace

IRVerificationResult verifyModuleChecked(const Module& mod) noexcept {
  try {
    for (const auto& function : mod.functions()) verifyFunction(function.get());
    return {};
  } catch (const std::exception& error) {
    std::string message = error.what();
    auto separator = message.find(": ");
    return {false, message.substr(0, separator),
            separator == std::string::npos ? std::string{}
                                           : message.substr(separator + 2)};
  }
}

void verifyModule(const Module& mod) {
  auto result = verifyModuleChecked(mod);
  if (!result.ok)
    throw std::runtime_error("CoreIR verify failed [" + result.reasonCode +
                             "]: " + result.detail);
}

class VerifyPass : public Pass {
public:
  std::string name() const override { return "FinalVerify"; }
  bool run(Module& mod, analysis::AnalysisManager&) override {
    verifyModule(mod);
    return false;
  }
};
REGISTER_PASS(VerifyPass);

} // namespace core_ir
