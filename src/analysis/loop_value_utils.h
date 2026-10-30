// 文件职责：沉淀循环分析与变换共享的小型 IR 判定，避免 analysis/hir/rewrite 重复维护。
#pragma once

#include "loop_info.h"

namespace analysis {

// 这里统一“标量槽位”的最小定义，只描述小尺寸 alloca，不掺入具体 pass 的
// 收益条件。
inline bool isScalarSlot(core_ir::Value* val) {
  auto* inst = dynamic_cast<core_ir::Instruction*>(val);
  return inst && inst->opcode() == core_ir::Opcode::Alloca &&
         !inst->isArrayObject() && inst->memorySizeBytes() > 0 &&
         inst->memorySizeBytes() <= 8;
}

inline bool loopStoresToSlot(const NaturalLoop& loop, core_ir::Value* slot) {
  for (auto* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      auto* inst = instPtr.get();
      if (inst->opcode() == core_ir::Opcode::Store &&
          inst->numOperands() == 2 && inst->operand(1) == slot) {
        return true;
      }
    }
  }
  return false;
}

// 这里仅编码“容易在 loop-memory 路径里当作简单不变量表达式”
// 的通用指令集合。是否允许 GEP 由调用方显式决定，避免把地址物化特例
// 偷偷扩散到所有 pass。
inline bool isSimpleInvariantExpr(core_ir::Instruction* inst,
                                  bool allowGEP = false) {
  if (!inst || inst->type().isVoid())
    return false;
  switch (inst->opcode()) {
  case core_ir::Opcode::Add:
  case core_ir::Opcode::Sub:
  case core_ir::Opcode::Mul:
  case core_ir::Opcode::And:
  case core_ir::Opcode::Or:
  case core_ir::Opcode::Xor:
  case core_ir::Opcode::Shl:
  case core_ir::Opcode::Shr:
  case core_ir::Opcode::Neg:
  case core_ir::Opcode::BitCast:
    return true;
  case core_ir::Opcode::GEP:
    return allowGEP;
  default:
    return false;
  }
}

} // namespace analysis
