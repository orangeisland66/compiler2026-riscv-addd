// 文件职责：实现不依赖符号名的 pointer object/range 解析。
#include "pointer_range.h"

#include <limits>
#include <unordered_set>

namespace analysis {

namespace {

core_ir::Value* underlying(core_ir::Value* value,
                           std::unordered_set<core_ir::Value*>& active) {
  if (!value || !active.insert(value).second) return nullptr;
  auto* instruction = dynamic_cast<core_ir::Instruction*>(value);
  if (!instruction) return value;
  if ((instruction->opcode() == core_ir::Opcode::GEP ||
       instruction->opcode() == core_ir::Opcode::BitCast) &&
      instruction->numOperands() >= 1)
    return underlying(instruction->operand(0), active);
  if (instruction->opcode() == core_ir::Opcode::Phi &&
      instruction->numOperands() >= 2 && instruction->numOperands() % 2 == 0) {
    core_ir::Value* common = nullptr;
    for (int index = 0; index < instruction->numOperands(); index += 2) {
      std::unordered_set<core_ir::Value*> branchActive = active;
      core_ir::Value* candidate =
          underlying(instruction->operand(index), branchActive);
      if (!candidate || (common && common != candidate)) return nullptr;
      common = candidate;
    }
    return common;
  }
  return value->type().isPtr() ? value : nullptr;
}

bool addChecked(int64_t lhs, int64_t rhs, int64_t& result) {
  if ((rhs > 0 && lhs > std::numeric_limits<int64_t>::max() - rhs) ||
      (rhs < 0 && lhs < std::numeric_limits<int64_t>::min() - rhs))
    return false;
  result = lhs + rhs;
  return true;
}

} // namespace

core_ir::Value* underlyingPointerObject(core_ir::Value* pointer) {
  std::unordered_set<core_ir::Value*> active;
  return underlying(pointer, active);
}

PointerRange evaluatePointerRange(core_ir::Value* pointer, int accessBytes) {
  PointerRange result;
  result.object = underlyingPointerObject(pointer);
  if (!result.object || accessBytes <= 0) {
    result.proof = ProofResult::unknown("pointer-object-unknown");
    return result;
  }
  int64_t offset = 0;
  core_ir::Value* current = pointer;
  std::unordered_set<core_ir::Value*> visited;
  while (auto* instruction = dynamic_cast<core_ir::Instruction*>(current)) {
    if (!visited.insert(current).second) {
      result.proof = ProofResult::unknown("cyclic-pointer-expression");
      return result;
    }
    if (instruction->opcode() == core_ir::Opcode::BitCast &&
        instruction->numOperands() == 1) {
      current = instruction->operand(0);
      continue;
    }
    if (instruction->opcode() != core_ir::Opcode::GEP ||
        instruction->numOperands() < 1)
      break;
    for (int index = 1; index < instruction->numOperands(); ++index) {
      auto* constant =
          dynamic_cast<core_ir::ConstInt*>(instruction->operand(index));
      if (!constant) {
        result.proof = ProofResult::unknown("dynamic-pointer-offset");
        return result;
      }
      int64_t next = 0;
      if (!addChecked(offset, static_cast<int64_t>(constant->value()) * 4,
                      next)) {
        result.proof = ProofResult::disproved("pointer-offset-overflow");
        return result;
      }
      offset = next;
    }
    current = instruction->operand(0);
  }
  int64_t end = 0;
  if (!addChecked(offset, accessBytes, end)) {
    result.proof = ProofResult::disproved("pointer-range-overflow");
    return result;
  }
  result.beginByte = offset;
  result.endByte = end;
  result.proof = ProofResult::success("constant-pointer-range");
  return result;
}

} // namespace analysis
