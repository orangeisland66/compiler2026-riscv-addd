// 文件职责：提供基础 AST/IR 重写入口和通用重写辅助函数。本文件负责实现具体逻辑。
#include "ir_rewrite.h"

#include <algorithm>

namespace rewrite {

namespace {

bool replaceWithValue(
    core_ir::Instruction* inst,
    core_ir::BasicBlock* bb,
    std::vector<std::unique_ptr<core_ir::Instruction>>::iterator& it,
    core_ir::Value* value,
    bool& changed) {
  inst->replaceAllUsesWith(value);
  inst->dropAllOperands();
  it = bb->instructions().erase(it);
  changed = true;
  return true;
}

bool replaceWithOperation(
    core_ir::Instruction* inst,
    core_ir::BasicBlock* bb,
    std::vector<std::unique_ptr<core_ir::Instruction>>::iterator& it,
    core_ir::Opcode opcode,
    std::initializer_list<core_ir::Value*> operands,
    bool& changed) {
  auto* replacement =
      new core_ir::Instruction(opcode, inst->type(), "ir-rewrite.algebraic");
  for (core_ir::Value* operand : operands)
    replacement->addOperand(operand);
  bb->insertBefore(inst, replacement);
  inst->replaceAllUsesWith(replacement);
  inst->dropAllOperands();
  // insertBefore 可能重新分配指令向量，因此按对象身份重新取得迭代器，
  // 不继续使用已经失效的输入迭代器。
  auto current = std::find_if(
      bb->instructions().begin(), bb->instructions().end(),
      [inst](const auto& candidate) { return candidate.get() == inst; });
  it = current == bb->instructions().end()
           ? bb->instructions().end()
           : bb->instructions().erase(current);
  changed = true;
  return true;
}

bool applyLocalRewrites(core_ir::Function* func) {
  bool changed = false;
  auto* mod = func->parent();

  for (auto& bb : func->blocks()) {
    for (auto it = bb->instructions().begin(); it != bb->instructions().end();) {
      core_ir::Instruction* inst = it->get();
      bool removed = false;

      switch (inst->opcode()) {
      case core_ir::Opcode::Neg: {
        // neg(neg(x)) → x
        auto* inner = dynamic_cast<core_ir::Instruction*>(inst->operand(0));
        if (inner && inner->opcode() == core_ir::Opcode::Neg) {
          inst->replaceAllUsesWith(inner->operand(0));
          inst->dropAllOperands();
          it = bb->instructions().erase(it);
          changed = true;
          removed = true;
        }
        break;
      }
      case core_ir::Opcode::Add: {
        // (x + C1) + C2 → x + (C1+C2)
        auto* innerAdd = dynamic_cast<core_ir::Instruction*>(inst->operand(0));
        auto* outerC = dynamic_cast<core_ir::ConstInt*>(inst->operand(1));
        if (innerAdd && innerAdd->opcode() == core_ir::Opcode::Add && outerC) {
          if (auto* innerC = dynamic_cast<core_ir::ConstInt*>(innerAdd->operand(1))) {
            int32_t sum = innerC->value() + outerC->value();
            inst->setOperand(0, innerAdd->operand(0));
            inst->setOperand(1, mod->getConstInt(sum));
            changed = true;
          }
        }

        auto* lhsSub = dynamic_cast<core_ir::Instruction*>(inst->operand(0));
        auto* rhsSub = dynamic_cast<core_ir::Instruction*>(inst->operand(1));
        if (!removed && lhsSub && lhsSub->opcode() == core_ir::Opcode::Sub) {
          if (lhsSub->operand(1) == inst->operand(1)) {
            changed = true;
            removed = replaceWithValue(
                inst, bb.get(), it, lhsSub->operand(0), changed);
            break;
          }
          if (auto* zero = dynamic_cast<core_ir::ConstInt*>(lhsSub->operand(0))) {
            if (zero->value() == 0 && lhsSub->operand(1) == inst->operand(1)) {
              changed = true;
              removed = replaceWithValue(
                  inst, bb.get(), it, mod->getConstInt(0), changed);
              break;
            }
          }
        }
        if (!removed && rhsSub && rhsSub->opcode() == core_ir::Opcode::Sub) {
          if (rhsSub->operand(1) == inst->operand(0)) {
            changed = true;
            removed = replaceWithValue(
                inst, bb.get(), it, rhsSub->operand(0), changed);
            break;
          }
          if (auto* zero = dynamic_cast<core_ir::ConstInt*>(rhsSub->operand(0))) {
            if (zero->value() == 0 && rhsSub->operand(1) == inst->operand(0)) {
              changed = true;
              removed = replaceWithValue(
                  inst, bb.get(), it, mod->getConstInt(0), changed);
              break;
            }
          }
        }

        // 把 x + (-y) 规范化为 x - y。除了暴露常见的减法恒等式，
        // 还可以避免在 RISC-V 降低边界为取负额外构造 i32 零。
        auto* lhsNeg = dynamic_cast<core_ir::Instruction*>(inst->operand(0));
        auto* rhsNeg = dynamic_cast<core_ir::Instruction*>(inst->operand(1));
        if (!removed && lhsNeg && lhsNeg->opcode() == core_ir::Opcode::Neg &&
            lhsNeg->numOperands() == 1) {
          core_ir::Value* rhs = inst->operand(1);
          removed = replaceWithOperation(
              inst, bb.get(), it, core_ir::Opcode::Sub,
              {rhs, lhsNeg->operand(0)}, changed);
          changed = true;
        } else if (!removed && rhsNeg &&
                   rhsNeg->opcode() == core_ir::Opcode::Neg &&
                   rhsNeg->numOperands() == 1) {
          removed = replaceWithOperation(
              inst, bb.get(), it, core_ir::Opcode::Sub,
              {inst->operand(0), rhsNeg->operand(0)}, changed);
          changed = true;
        }
        break;
      }
      case core_ir::Opcode::Sub: {
        // x - 0 -> x
        if (auto* ci = dynamic_cast<core_ir::ConstInt*>(inst->operand(1))) {
          if (ci->value() == 0) {
            inst->replaceAllUsesWith(inst->operand(0));
            inst->dropAllOperands();
            it = bb->instructions().erase(it);
            changed = true;
            removed = true;
          }
        }
        if (removed)
          break;

        // 项目 IR 中的 i32 算术采用二进制补码模运算，因此 0-x 就是 Neg(x)。
        // 提前规范化后，下面的规则和后端都可以使用同一种直接算术操作。
        if (auto* zero = dynamic_cast<core_ir::ConstInt*>(inst->operand(0));
            zero && zero->value() == 0) {
          core_ir::Value* value = inst->operand(1);
          removed = replaceWithOperation(
              inst, bb.get(), it, core_ir::Opcode::Neg, {value}, changed);
          changed = true;
          break;
        }

        auto* rhsNeg = dynamic_cast<core_ir::Instruction*>(inst->operand(1));
        if (rhsNeg && rhsNeg->opcode() == core_ir::Opcode::Neg &&
            rhsNeg->numOperands() == 1) {
          removed = replaceWithOperation(
              inst, bb.get(), it, core_ir::Opcode::Add,
              {inst->operand(0), rhsNeg->operand(0)}, changed);
          changed = true;
          break;
        }

        if (inst->operand(0) == inst->operand(1)) {
          changed = true;
          removed = replaceWithValue(
              inst, bb.get(), it, mod->getConstInt(0), changed);
          break;
        }

        auto* lhsAdd = dynamic_cast<core_ir::Instruction*>(inst->operand(0));
        if (lhsAdd && lhsAdd->opcode() == core_ir::Opcode::Add) {
          if (lhsAdd->operand(0) == inst->operand(1)) {
            changed = true;
            removed = replaceWithValue(
                inst, bb.get(), it, lhsAdd->operand(1), changed);
            break;
          }
          if (lhsAdd->operand(1) == inst->operand(1)) {
            changed = true;
            removed = replaceWithValue(
                inst, bb.get(), it, lhsAdd->operand(0), changed);
            break;
          }
        }

        auto* rhsAdd = dynamic_cast<core_ir::Instruction*>(inst->operand(1));
        if (rhsAdd && rhsAdd->opcode() == core_ir::Opcode::Add) {
          if (rhsAdd->operand(0) == inst->operand(0)) {
            inst->setOperand(0, mod->getConstInt(0));
            inst->setOperand(1, rhsAdd->operand(1));
            changed = true;
            break;
          }
          if (rhsAdd->operand(1) == inst->operand(0)) {
            inst->setOperand(0, mod->getConstInt(0));
            inst->setOperand(1, rhsAdd->operand(0));
            changed = true;
            break;
          }
        }

        auto* lhsSub = dynamic_cast<core_ir::Instruction*>(inst->operand(0));
        if (lhsSub && lhsSub->opcode() == core_ir::Opcode::Sub &&
            lhsSub->operand(0) == inst->operand(1)) {
          inst->setOperand(0, mod->getConstInt(0));
          inst->setOperand(1, lhsSub->operand(1));
          changed = true;
          break;
        }
        break;
      }
      case core_ir::Opcode::Mul: {
        // x * 1 → x
        if (auto* ci = dynamic_cast<core_ir::ConstInt*>(inst->operand(1))) {
          if (ci->value() == 1) {
            inst->replaceAllUsesWith(inst->operand(0));
            inst->dropAllOperands();
            it = bb->instructions().erase(it);
            changed = true;
            removed = true;
          }
        }
        break;
      }
      case core_ir::Opcode::ICmp: {
        // icmp eq (icmp ne x, 0), 0 → simplify
        auto* innerCmp = dynamic_cast<core_ir::Instruction*>(inst->operand(0));
        if (innerCmp && (innerCmp->opcode() == core_ir::Opcode::ICmp)) {
          if (auto* ci = dynamic_cast<core_ir::ConstInt*>(inst->operand(1))) {
            std::string innerCond = innerCmp->name().substr(0, innerCmp->name().find('_'));
            std::string outerCond = inst->name().substr(0, inst->name().find('_'));

            // icmp eq (icmp ne x, 0), 1 → icmp ne x, 0（与内层比较等价）
            if (outerCond == "eq" && ci->value() == 1) {
              inst->replaceAllUsesWith(innerCmp);
              inst->dropAllOperands();
              it = bb->instructions().erase(it);
              changed = true;
              removed = true;
            }
          }
        }
        break;
      }
      default:
        break;
      }

      if (!removed)
        ++it;
    }
  }
  return changed;
}

} // namespace

bool applyIRAlgebraicRewrite(core_ir::Module& mod) {
  bool changed = false;
  for (auto& func : mod.functions()) {
  // 函数过大时跳过，避免改写范围过度扩大。
    int ic = 0;
    for (auto& bb : func->blocks())
      ic += static_cast<int>(bb->instructions().size());
    if (ic > 3000) continue;
    changed |= applyLocalRewrites(func.get());
  }
  return changed;
}

} // namespace rewrite
