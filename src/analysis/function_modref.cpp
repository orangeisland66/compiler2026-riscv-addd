// 文件职责：实现不依赖名称的 call-site mod/ref 查询，未知事实保守失败。
#include "function_modref.h"

#include "pointer_range.h"

namespace analysis {

namespace {

bool operandMayReachLocation(core_ir::Value* operand,
                             core_ir::Value* location) {
  if (!operand || !location || !operand->type().isPtr())
    return false;
  core_ir::Value* operandObject = underlyingPointerObject(operand);
  core_ir::Value* locationObject = underlyingPointerObject(location);
  if (!operandObject || !locationObject)
    return true;
  if (operandObject == locationObject)
    return true;
  auto isAlloca = [](core_ir::Value* value) {
    auto* instruction = dynamic_cast<core_ir::Instruction*>(value);
    return instruction && instruction->opcode() == core_ir::Opcode::Alloca;
  };
  // 当前激活记录中的 alloca 不可能与函数入口指针或全局对象
  // 是同一对象；不同 direct global 也不相交。两个普通指针参数
  // 仍可以别名，不因 Value* 不同而判 NoAlias。
  if (isAlloca(operandObject) || isAlloca(locationObject))
    return false;
  if (dynamic_cast<core_ir::GlobalVar*>(operandObject) &&
      dynamic_cast<core_ir::GlobalVar*>(locationObject))
    return false;
  return true;
}

} // namespace

CallModRefResult queryCallModRef(const core_ir::Instruction* call,
                                 core_ir::Value* location,
                                 int locationBytes,
                                 const FunctionEffectMap& effects) {
  CallModRefResult result;
  if (!call || call->opcode() != core_ir::Opcode::Call || !location ||
      locationBytes <= 0 || !call->parent() || !call->parent()->parent() ||
      !call->parent()->parent()->parent()) {
    result.proof = ProofResult::unknown("call-modref-invalid-query");
    return result;
  }
  const auto& module = *call->parent()->parent()->parent();
  const FunctionEffectSummary* summary =
      lookupFunctionEffect(effects, module, call->name());
  if (!summary) {
    result.proof = ProofResult::unknown("call-modref-unknown-callee");
    return result;
  }

  bool mayRead = summary->readsUnknownMemory;
  bool mayWrite = summary->writesUnknownMemory;
  auto* locationGlobal = dynamic_cast<core_ir::GlobalVar*>(
      underlyingPointerObject(location));
  if (locationGlobal) {
    mayRead |= summary->readsMutableGlobalMemory;
    mayWrite |= summary->writtenGlobals.count(locationGlobal) != 0;
    if (summary->writesMutableGlobalMemory && summary->writtenGlobals.empty())
      mayWrite = true;
  }

  for (std::size_t index : summary->readArgumentIndices) {
    if (index >= static_cast<std::size_t>(call->numOperands())) {
      mayRead = true;
      continue;
    }
    mayRead |= operandMayReachLocation(
        call->operand(static_cast<int>(index)), location);
  }
  for (std::size_t index : summary->writtenArgumentIndices) {
    if (index >= static_cast<std::size_t>(call->numOperands())) {
      mayWrite = true;
      continue;
    }
    mayWrite |= operandMayReachLocation(
        call->operand(static_cast<int>(index)), location);
  }

  result.mayRead = mayRead;
  result.mayWrite = mayWrite;
  result.proof = ProofResult::success(
      mayRead || mayWrite ? "call-modref-classified" : "call-no-modref");
  return result;
}

} // namespace analysis
