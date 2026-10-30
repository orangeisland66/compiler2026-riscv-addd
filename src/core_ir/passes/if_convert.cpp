#include "research_switch.h"
// 文件职责：把适合的条件分支计算改写为数据选择形式。
#include "if_convert.h"
#include "phi_utils.h"
#include "passes_utils.h"
#include "pass_registry.h"
#include <cstdlib>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace core_ir {

bool traceIfConvertEnabled() {
  return sycEnvironmentValue("SYC_TRACE_COREIR_IFCONV") != nullptr;
}

bool isConstIntValue(Value* value, int32_t expected) {
  auto* ci = dynamic_cast<ConstInt*>(value);
  return ci && ci->value() == expected;
}

bool containsLowBitMask(Value* value, int depth = 0) {
  if (!value || depth > 8)
    return false;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst)
    return false;
  if (inst->opcode() == Opcode::And && inst->numOperands() == 2 &&
      (isConstIntValue(inst->operand(0), 1) ||
       isConstIntValue(inst->operand(1), 1)))
    return true;
  switch (inst->opcode()) {
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
  case Opcode::ICmp:
    for (int i = 0; i < inst->numOperands(); ++i) {
      if (containsLowBitMask(inst->operand(i), depth + 1))
        return true;
    }
    break;
  default:
    break;
  }
  return false;
}

bool isParityLikeBranchCondition(Value* cond) {
  auto* inst = dynamic_cast<Instruction*>(cond);
  if (!inst || inst->opcode() != Opcode::ICmp || inst->numOperands() != 2)
    return false;
  const std::string condName = cmpCond(inst);
  if (condName != "eq" && condName != "ne")
    return false;
  return containsLowBitMask(inst->operand(0)) ||
         containsLowBitMask(inst->operand(1));
}

bool isLoopCarriedAddRecPhi(Value* value) {
  auto* phi = dynamic_cast<Instruction*>(value);
  if (!phi || phi->opcode() != Opcode::Phi || phi->numOperands() < 4)
    return false;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    auto* update = dynamic_cast<Instruction*>(phi->operand(i));
    if (!update || (update->opcode() != Opcode::Add &&
                    update->opcode() != Opcode::Sub) ||
        update->numOperands() != 2) {
      continue;
    }
    if (update->operand(0) == phi || update->operand(1) == phi)
      return true;
  }
  return false;
}

bool conditionUsesLoopCarriedPhi(Value* cond) {
  auto* cmp = dynamic_cast<Instruction*>(cond);
  if (!cmp || cmp->opcode() != Opcode::ICmp || cmp->numOperands() != 2)
    return false;
  return isLoopCarriedAddRecPhi(cmp->operand(0)) ||
         isLoopCarriedAddRecPhi(cmp->operand(1));
}

bool isSpeculatableIfConvertedInst(Instruction* inst) {
  if (!inst || inst->isTerminator() || inst->opcode() == Opcode::Phi)
    return false;
  switch (inst->opcode()) {
  case Opcode::Load:
  case Opcode::GEP:
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Mul:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
  case Opcode::Shl:
  case Opcode::Shr:
  case Opcode::ICmp:
  case Opcode::Neg:
    return inst->type().isI32() || inst->type().isPtr();
  default:
    return false;
  }
}

bool isTrivialBranchTo(BasicBlock* bb, BasicBlock* target) {
  if (!bb || !target)
    return false;
  Instruction* term = bb->terminator();
  if (!term || term->opcode() != Opcode::Br || term->operand(0) != target)
    return false;
  for (auto& instPtr : bb->instructions()) {
    Instruction* inst = instPtr.get();
    if (inst != term && inst->opcode() != Opcode::Phi)
      return false;
  }
  return true;
}

BasicBlock* singleBranchTarget(BasicBlock* bb) {
  if (!bb)
    return nullptr;
  Instruction* term = bb->terminator();
  if (!term || term->opcode() != Opcode::Br || term->numOperands() != 1)
    return nullptr;
  return dynamic_cast<BasicBlock*>(term->operand(0));
}

bool externalUsesStayInsideDiamond(
    const std::vector<Instruction*>& insts, BasicBlock* computeBB,
    BasicBlock* mergeBB) {
  std::unordered_set<Instruction*> instSet(insts.begin(), insts.end());
  for (Instruction* inst : insts) {
    for (Instruction* user : inst->uses()) {
      if (!user || !user->parent())
        return false;
      if (user->parent() == computeBB)
        continue;
      if (user->parent() == mergeBB && user->opcode() == Opcode::Phi)
        continue;
      if (instSet.count(user))
        continue;
      return false;
    }
  }
  return true;
}

Value* incomingFromPred(Instruction* phi, BasicBlock* pred) {
  if (!phi || !pred)
    return nullptr;
  Value* result = nullptr;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    auto* inPred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
    if (inPred != pred)
      continue;
    if (result && result != phi->operand(i))
      return nullptr;
    result = phi->operand(i);
  }
  return result;
}

Value* bitwiseComplementSource(Value* value) {
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->numOperands() != 2)
    return nullptr;
  if (inst->opcode() == Opcode::Xor) {
    if (isConstIntValue(inst->operand(0), -1))
      return inst->operand(1);
    if (isConstIntValue(inst->operand(1), -1))
      return inst->operand(0);
    return nullptr;
  }
  if (inst->opcode() != Opcode::Sub ||
      !isConstIntValue(inst->operand(1), 1)) {
    return nullptr;
  }
  auto* neg = dynamic_cast<Instruction*>(inst->operand(0));
  if (!neg || neg->opcode() != Opcode::Neg || neg->numOperands() != 1)
    return nullptr;
  return neg->operand(0);
}

bool areBitwiseComplements(Value* lhs, Value* rhs) {
  return bitwiseComplementSource(lhs) == rhs ||
         bitwiseComplementSource(rhs) == lhs;
}

Value* materializeComplementSelectMask(Module& mod, BasicBlock* pred,
                                       Value* cond, bool computeOnTrue,
                                       Instruction* term) {
  if (!pred || !cond || !term || !cond->type().isI32())
    return nullptr;
  Value* predicate = cond;
  auto* condInst = dynamic_cast<Instruction*>(cond);
  if (!condInst || (condInst->opcode() != Opcode::ICmp &&
                    condInst->opcode() != Opcode::FCmp)) {
    auto* zeroCmp = new Instruction(Opcode::ICmp, Type::i32(),
                                    "ne_ifc.complement");
    zeroCmp->addOperand(cond);
    zeroCmp->addOperand(mod.getConstInt(0));
    pred->insertBefore(term, zeroCmp);
    predicate = zeroCmp;
  }

  // 条件成立的一侧取反时使用 -predicate；条件不成立的一侧取反时
  // 使用 predicate-1。两者都会直接生成 0 或全一位掩码。
  auto* fullMask = new Instruction(Opcode::Sub, Type::i32(),
                                   "ifc.complement.mask");
  if (computeOnTrue) {
    fullMask->addOperand(mod.getConstInt(0));
    fullMask->addOperand(predicate);
  } else {
    fullMask->addOperand(predicate);
    fullMask->addOperand(mod.getConstInt(1));
  }
  pred->insertBefore(term, fullMask);
  return fullMask;
}

Instruction* cloneIntoBefore(BasicBlock* dst, Instruction* pos,
                             Instruction* src,
                             std::unordered_map<Value*, Value*>& valueMap) {
  auto* clone = new Instruction(
      src->opcode(), src->type(),
      src->name().empty() ? "ifc" : (src->name() + ".ifc"));
  clone->setMemoryInfo(src->memoryElemTy(), src->memorySizeBytes(),
                       src->isArrayObject(), src->dims());
  for (Value* operand : src->operands()) {
    auto it = valueMap.find(operand);
    clone->addOperand(it == valueMap.end() ? operand : it->second);
  }
  dst->insertBefore(pos, clone);
  valueMap[src] = clone;
  return clone;
}

bool materializeBranchMask(Module& mod, BasicBlock* pred, Value* cond,
                           bool computeOnTrue, Instruction* term,
                           Value*& mask) {
  if (!pred || !cond || !term || !cond->type().isI32())
    return false;

  auto* condInst = dynamic_cast<Instruction*>(cond);
  if (condInst && (condInst->opcode() == Opcode::ICmp ||
                   condInst->opcode() == Opcode::FCmp)) {
    mask = cond;
  } else {
    auto* zeroCmp = new Instruction(Opcode::ICmp, Type::i32(), "ne_ifc.mask");
    zeroCmp->addOperand(cond);
    zeroCmp->addOperand(mod.getConstInt(0));
    pred->insertBefore(term, zeroCmp);
    mask = zeroCmp;
  }
  if (!computeOnTrue) {
    auto* inverted = new Instruction(Opcode::Sub, Type::i32(), "ifc.mask.not");
    inverted->addOperand(mod.getConstInt(1));
    inverted->addOperand(mask);
    pred->insertBefore(term, inverted);
    mask = inverted;
  }
  return true;
}

bool tryIfConvertSmallDiamondSelect(Module& mod, Function* func,
                                    BasicBlock* pred,
                                    bool allowNonEmptyParity) {
  if (!func || !pred)
    return false;
  Instruction* term = pred->terminator();
  if (!term || term->opcode() != Opcode::CondBr || term->numOperands() != 3)
    return false;
  Value* cond = term->operand(0);
  if (!cond || !cond->type().isI32())
    return false;

  auto* trueBB = dynamic_cast<BasicBlock*>(term->operand(1));
  auto* falseBB = dynamic_cast<BasicBlock*>(term->operand(2));
  if (!trueBB || !falseBB || trueBB == falseBB)
    return false;

  BasicBlock* trueTarget = singleBranchTarget(trueBB);
  BasicBlock* falseTarget = singleBranchTarget(falseBB);
  BasicBlock* mergeBB = nullptr;
  BasicBlock* computeBB = nullptr;
  BasicBlock* skipPred = nullptr;
  bool computeOnTrue = false;

  if (trueTarget && falseTarget && trueTarget == falseTarget) {
    mergeBB = trueTarget;
    const bool trueTrivial = isTrivialBranchTo(trueBB, mergeBB);
    const bool falseTrivial = isTrivialBranchTo(falseBB, mergeBB);
    if (trueTrivial && falseTrivial) {
      // 空菱形是 `if (cond) value = a; else value = b` 的常见降低形态。
      // 两侧都不需要复制，合流处的 PHI 可以直接改成选择结果。
      computeBB = trueBB;
      skipPred = falseBB;
      computeOnTrue = true;
    } else if (trueTrivial == falseTrivial) {
      return false;
    } else {
      computeBB = trueTrivial ? falseBB : trueBB;
      skipPred = trueTrivial ? trueBB : falseBB;
      computeOnTrue = computeBB == trueBB;
    }
  } else if (trueTarget == falseBB) {
    mergeBB = falseBB;
    computeBB = trueBB;
    skipPred = pred;
    computeOnTrue = true;
  } else if (falseTarget == trueBB) {
    mergeBB = trueBB;
    computeBB = falseBB;
    skipPred = pred;
    computeOnTrue = false;
  } else {
    return false;
  }

  if (!mergeBB || !computeBB || !skipPred || computeBB == mergeBB)
    return false;
  if (computeBB->preds().size() != 1 || computeBB->preds().front() != pred)
    return false;
  if (skipPred != pred &&
      (skipPred->preds().size() != 1 || skipPred->preds().front() != pred))
    return false;
  if (mergeBB->preds().size() != 2)
    return false;

  std::vector<Instruction*> computeInsts;
  for (auto& instPtr : computeBB->instructions()) {
    Instruction* inst = instPtr.get();
    if (inst == computeBB->terminator())
      continue;
    if (!isSpeculatableIfConvertedInst(inst))
      return false;
    computeInsts.push_back(inst);
  }
  if (computeInsts.size() > 8)
    return false;
  // 直接由循环携带递推值控制的分支通常是单调阈值判断。
  // 保留该分支有利于后端维持稳定的控制流路径，
  // 下面由数据决定的菱形结构仍可改为无分支选择。
  if (!isParityLikeBranchCondition(cond) && conditionUsesLoopCarriedPhi(cond))
    return false;
  bool hasSpeculativeLoad = false;
  for (auto* inst : computeInsts) {
    if (inst && inst->opcode() == Opcode::Load) {
      hasSpeculativeLoad = true;
      break;
    }
  }
  // 普通条件分支可能是边界检查，不能把 load 推测到分支前。
  // 只有已归一化的 parity 数据谓词允许推测加载，覆盖“只用条件决定
  // 是否累加”的热路径；其它 diamond 只做纯算术 if-convert。
  if (hasSpeculativeLoad && !isParityLikeBranchCondition(cond))
    return false;
  const bool parityLike = isParityLikeBranchCondition(cond);
  const bool loopCarriedCond = conditionUsesLoopCarriedPhi(cond);
  if (!externalUsesStayInsideDiamond(computeInsts, computeBB, mergeBB))
    return false;

  auto phis = collectLeadingPhi(mergeBB);
  if (phis.empty())
    return false;

  struct PhiRewrite {
    Instruction* phi = nullptr;
    Value* oldValue = nullptr;
    Value* updatedValue = nullptr;
    bool sameValue = false;
  };
  std::vector<PhiRewrite> rewrites;
  rewrites.reserve(phis.size());

  for (Instruction* phi : phis) {
    Value* updated = incomingFromPred(phi, computeBB);
    Value* oldValue = incomingFromPred(phi, skipPred);
    if (!updated || !oldValue)
      return false;
    if (!updated->type().isI32() || !oldValue->type().isI32())
      return false;
    rewrites.push_back({phi, oldValue, updated, updated == oldValue});
  }
  if (rewrites.empty())
    return false;
  if (!computeInsts.empty()) {
    const bool profitableParity = parityLike && allowNonEmptyParity;
    const bool profitableSingleResult =
        rewrites.size() == 1 &&
        (computeInsts.size() >= 2 ||
         (computeInsts.size() == 1 && func->blocks().size() > 256)) &&
        !hasSpeculativeLoad;
    if (!profitableParity && !profitableSingleResult)
      return false;
  }
  if (computeInsts.empty()) {
    for (const PhiRewrite& rewrite : rewrites) {
      if (!rewrite.sameValue && !isConstIntValue(rewrite.oldValue, 0))
        return false;
    }
  }

  if (traceIfConvertEnabled()) {
    std::cerr << "[if-convert] apply"
              << " func=" << func->name()
              << " pred=" << pred->name()
              << " compute=" << computeBB->name()
              << " merge=" << mergeBB->name()
              << " insts=" << computeInsts.size()
              << " phis=" << rewrites.size()
              << " load=" << hasSpeculativeLoad
              << " parity=" << parityLike
              << " loop_phi_cond=" << loopCarriedCond
              << " compute_on_true=" << computeOnTrue
              << "\n";
  }

  bool complementSelect =
      sycEnvironmentValue("SYC_SKIP_COREIR_COMPLEMENT_SELECT_IFCONV") == nullptr;
  bool sawComplement = false;
  for (const PhiRewrite& rewrite : rewrites) {
    if (rewrite.sameValue)
      continue;
    if (!areBitwiseComplements(rewrite.oldValue, rewrite.updatedValue)) {
      complementSelect = false;
      break;
    }
    sawComplement = true;
  }
  complementSelect &= sawComplement;
  if (complementSelect && traceIfConvertEnabled()) {
    std::cerr << "[if-convert] complement-select"
              << " func=" << func->name()
              << " pred=" << pred->name()
              << " compute_on_true=" << computeOnTrue
              << " phis=" << rewrites.size()
              << "\n";
  }

  std::unordered_map<Value*, Value*> clonedValues;
  if (!complementSelect) {
    for (Instruction* inst : computeInsts)
      cloneIntoBefore(pred, term, inst, clonedValues);
  }

  Value* mask = nullptr;
  Value* complementMask = nullptr;
  if (complementSelect) {
    complementMask = materializeComplementSelectMask(
        mod, pred, cond, computeOnTrue, term);
    if (!complementMask)
      return false;
  } else if (!materializeBranchMask(mod, pred, cond, computeOnTrue, term,
                                    mask)) {
    return false;
  }

  // 这是通用 if-conversion / predication 规则。
  // 只要两个分支在 merge 点汇成 i32 选择，而且计算侧是可推测的小
  // diamond，就把它压成 branchless 的 SSA 选择表达式。
  for (const PhiRewrite& rewrite : rewrites) {
    if (rewrite.sameValue) {
      rewrite.phi->replaceAllUsesWith(rewrite.oldValue);
      continue;
    }
    if (complementSelect) {
      auto* selected = new Instruction(Opcode::Xor, Type::i32(),
                                       "ifc.complement.select");
      selected->addOperand(rewrite.oldValue);
      selected->addOperand(complementMask);
      pred->insertBefore(term, selected);
      rewrite.phi->replaceAllUsesWith(selected);
      continue;
    }

    Value* updatedValue = rewrite.updatedValue;
    auto clonedIt = clonedValues.find(rewrite.updatedValue);
    if (clonedIt != clonedValues.end())
      updatedValue = clonedIt->second;
    auto* updatedInst = dynamic_cast<Instruction*>(rewrite.updatedValue);
    if (updatedInst && updatedInst->parent() == computeBB &&
        clonedIt == clonedValues.end()) {
      return false;
    }

    Value* selected = nullptr;
    if (isConstIntValue(rewrite.oldValue, 0)) {
      auto* scaled = new Instruction(Opcode::Mul, Type::i32(), "ifc.scaled");
      scaled->addOperand(mask);
      scaled->addOperand(updatedValue);
      pred->insertBefore(term, scaled);
      selected = scaled;
    } else {
      auto* fullMask =
          new Instruction(Opcode::Sub, Type::i32(), "ifc.fullmask");
      fullMask->addOperand(mod.getConstInt(0));
      fullMask->addOperand(mask);
      pred->insertBefore(term, fullMask);
      auto* diffBits =
          new Instruction(Opcode::Xor, Type::i32(), "ifc.diffbits");
      diffBits->addOperand(rewrite.oldValue);
      diffBits->addOperand(updatedValue);
      pred->insertBefore(term, diffBits);
      auto* maskedBits =
          new Instruction(Opcode::And, Type::i32(), "ifc.masked");
      maskedBits->addOperand(diffBits);
      maskedBits->addOperand(fullMask);
      pred->insertBefore(term, maskedBits);
      auto* bitSelect =
          new Instruction(Opcode::Xor, Type::i32(), "ifc.select");
      bitSelect->addOperand(rewrite.oldValue);
      bitSelect->addOperand(maskedBits);
      pred->insertBefore(term, bitSelect);
      selected = bitSelect;
    }
    rewrite.phi->replaceAllUsesWith(selected);
  }

  pred->remove(term);
  auto* br = new Instruction(Opcode::Br, Type::voidTy());
  br->addOperand(mergeBB);
  pred->pushBack(br);
  func->recomputeCFG();
  return true;
}

bool ifConvertSmallDiamonds(Module& mod) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    func->recomputeCFG();
    std::vector<BasicBlock*> blocks;
    for (auto& bbPtr : func->blocks())
      blocks.push_back(bbPtr.get());
    unsigned parityBranchCount = 0;
    for (BasicBlock* bb : blocks) {
      Instruction* term = bb ? bb->terminator() : nullptr;
      if (term && term->opcode() == Opcode::CondBr &&
          term->numOperands() == 3 &&
          isParityLikeBranchCondition(term->operand(0))) {
        ++parityBranchCount;
      }
    }
    const bool allowNonEmptyParity = parityBranchCount <= 1;
    for (BasicBlock* bb : blocks) {
      if (bb && tryIfConvertSmallDiamondSelect(
                    mod, func, bb, allowNonEmptyParity))
        changed = true;
    }
    if (changed)
      func->recomputeCFG();
  }
  return changed;
}


} // namespace core_ir
