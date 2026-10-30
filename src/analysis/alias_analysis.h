// 文件职责：提供基于对象 identity 与已证明范围的保守 alias 查询。
#pragma once

#include "pointer_range.h"

namespace analysis {

enum class AliasKind {
  NoAlias,
  MayAlias,
  MustAlias,
};

struct AliasResult {
  AliasKind kind = AliasKind::MayAlias;
  ProofResult proof = ProofResult::unknown("alias-unknown");
};

AliasResult queryAlias(core_ir::Value* lhs, int lhsBytes,
                       core_ir::Value* rhs, int rhsBytes);

} // namespace analysis
