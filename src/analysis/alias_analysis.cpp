// 文件职责：实现 fail-closed alias 分类。
#include "alias_analysis.h"

namespace analysis {

AliasResult queryAlias(core_ir::Value* lhs, int lhsBytes,
                       core_ir::Value* rhs, int rhsBytes) {
  if (!lhs || !rhs)
    return {AliasKind::MayAlias, ProofResult::unknown("null-pointer-value")};
  if (lhs == rhs)
    return {AliasKind::MustAlias, ProofResult::success("same-ssa-pointer")};
  auto left = evaluatePointerRange(lhs, lhsBytes);
  auto right = evaluatePointerRange(rhs, rhsBytes);
  if (!left.object || !right.object)
    return {AliasKind::MayAlias,
            ProofResult::unknown("underlying-object-unknown")};
  if (left.object != right.object) {
    const bool leftOwned = dynamic_cast<core_ir::GlobalVar*>(left.object) ||
        (dynamic_cast<core_ir::Instruction*>(left.object) &&
         static_cast<core_ir::Instruction*>(left.object)->opcode() ==
             core_ir::Opcode::Alloca);
    const bool rightOwned = dynamic_cast<core_ir::GlobalVar*>(right.object) ||
        (dynamic_cast<core_ir::Instruction*>(right.object) &&
         static_cast<core_ir::Instruction*>(right.object)->opcode() ==
             core_ir::Opcode::Alloca);
    if (leftOwned && rightOwned)
      return {AliasKind::NoAlias,
              ProofResult::success("distinct-owned-objects")};
    return {AliasKind::MayAlias,
            ProofResult::unknown("distinct-unowned-objects")};
  }
  if (!left.proof.proved() || !right.proof.proved())
    return {AliasKind::MayAlias,
            ProofResult::unknown("object-range-unknown")};
  if (left.endByte <= right.beginByte || right.endByte <= left.beginByte)
    return {AliasKind::NoAlias,
            ProofResult::success("disjoint-constant-ranges")};
  if (left.beginByte == right.beginByte && left.endByte == right.endByte)
    return {AliasKind::MustAlias,
            ProofResult::success("equal-constant-ranges")};
  return {AliasKind::MayAlias,
          ProofResult::unknown("overlapping-ranges")};
}

} // namespace analysis
