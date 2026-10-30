// 文件职责：解析保守的底层对象与常量 GEP 范围。
#pragma once

#include "../core_ir/ir/core_ir.h"
#include "proof_result.h"

#include <cstdint>

namespace analysis {

struct PointerRange {
  ProofResult proof;
  core_ir::Value* object = nullptr;
  int64_t beginByte = 0;
  int64_t endByte = 0; // exclusive
};

core_ir::Value* underlyingPointerObject(core_ir::Value* pointer);
PointerRange evaluatePointerRange(core_ir::Value* pointer,
                                  int accessBytes = 1);

} // namespace analysis
