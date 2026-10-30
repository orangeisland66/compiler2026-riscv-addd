// 文件职责：按当前 CoreIR 所有权模型实现两阶段深克隆。
#include "planning_clone.h"

#include <stdexcept>

namespace core_ir::planning {

namespace {

Value* remapValue(Value* source, PlanningClone& clone) {
  if (Value* mapped = clone.mapped(source)) return mapped;
  if (auto* integer = dynamic_cast<ConstInt*>(source))
    return clone.module().getConstInt(integer->value());
  if (auto* floating = dynamic_cast<ConstFloat*>(source))
    return clone.module().getConstFloat(floating->value());
  throw std::runtime_error("planning clone encountered foreign value");
}

Instruction* cloneInstructionShell(const Instruction& source) {
  auto* result = new Instruction(source.opcode(), source.type(), source.name());
  result->setMemoryInfo(source.memoryElemTy(), source.memorySizeBytes(),
                        source.isArrayObject(), source.dims());
  return result;
}

} // namespace

PlanningClone::~PlanningClone() {
  // If release() was not called, the candidate is being rejected. Destroy
  // all candidate Values first, then restore the global diagnostic id stream.
  if (module_) {
    module_.reset();
    Value::restoreIdCheckpoint(valueIdCheckpoint_);
  }
}

Function* PlanningClone::mapped(const Function* source) const {
  auto found = functions_.find(source);
  return found == functions_.end() ? nullptr : found->second;
}

BasicBlock* PlanningClone::mapped(const BasicBlock* source) const {
  auto found = blocks_.find(source);
  return found == blocks_.end() ? nullptr : found->second;
}

Value* PlanningClone::mapped(const Value* source) const {
  auto found = values_.find(source);
  return found == values_.end() ? nullptr : found->second;
}

std::unique_ptr<PlanningClone> cloneModuleForPlanning(const Module& source) {
  auto clone = std::make_unique<PlanningClone>();
  clone->valueIdCheckpoint_ = Value::idCheckpoint();
  clone->module_ = std::make_unique<Module>();

  for (const auto& name : source.builtinNames())
    clone->module_->declareBuiltin(name, source.builtinRetTy(name),
                                   source.builtinParamTys(name));

  for (const auto& global : source.globals()) {
    auto* copy = clone->module_->newGlobal(
        global->name(), global->elemTy(), global->dims(), global->sizeBytes(),
        global->isConst(), global->hasInit());
    copy->initData = global->initData;
    clone->values_[global.get()] = copy;
  }
  for (const auto& function : source.functions()) {
    auto* copy = clone->module_->newFunction(function->name(), function->retTy());
    clone->functions_[function.get()] = copy;
    for (const auto& globalName : function->localizedGlobals())
      copy->markLocalizedGlobal(globalName);
    for (const auto& argument : function->args()) {
      Value* copyArgument = copy->addArg(argument.name, argument.type);
      clone->values_[argument.val] = copyArgument;
    }
    for (const auto& block : function->blocks()) {
      auto* copyBlock = copy->newBlock("planning");
      copyBlock->setName(block->name());
      clone->blocks_[block.get()] = copyBlock;
      clone->values_[block.get()] = copyBlock;
    }
    copy->setEntryBlock(clone->mapped(function->entryBlock()));
  }

  for (const auto& function : source.functions()) {
    auto* copyFunction = clone->mapped(function.get());
    for (const auto& block : function->blocks()) {
      auto* copyBlock = clone->mapped(block.get());
      for (const auto& instruction : block->instructions()) {
        auto* copyInstruction = cloneInstructionShell(*instruction);
        copyBlock->pushBack(copyInstruction);
        clone->values_[instruction.get()] = copyInstruction;
        if (instruction->opcode() == Opcode::Alloca)
          copyFunction->addAlloca(copyInstruction);
      }
    }
  }

  for (const auto& function : source.functions()) {
    for (const auto& block : function->blocks()) {
      auto* copyBlock = clone->mapped(block.get());
      for (std::size_t index = 0; index < block->instructions().size(); ++index) {
        auto* sourceInstruction = block->instructions()[index].get();
        auto* copyInstruction = copyBlock->instructions()[index].get();
        for (Value* operand : sourceInstruction->operands())
          copyInstruction->addOperand(remapValue(operand, *clone));
      }
    }
    clone->mapped(function.get())->recomputeCFG();
  }
  return clone;
}

} // namespace core_ir::planning
