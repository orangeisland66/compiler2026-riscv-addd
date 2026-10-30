// 文件职责：识别 affine copy/view 区域，为视图转发和 producer 删除提供证明。本文件负责实现具体逻辑。
#include "affine_copy_region.h"

#include "../core_ir/ir/core_ir.h"

namespace analysis {

namespace {

using core_ir::Function;
using core_ir::Instruction;
using core_ir::Opcode;
using core_ir::Value;

bool isSimpleAffineValue(Instruction* inst, Value*& srcAddr, bool& negated) {
  srcAddr = nullptr;
  negated = false;
  if (!inst)
    return false;
  if (inst->opcode() == Opcode::Load && inst->numOperands() == 1) {
    srcAddr = inst->operand(0);
    return true;
  }
  if (inst->opcode() == Opcode::Neg && inst->numOperands() == 1) {
    auto* inner = dynamic_cast<Instruction*>(inst->operand(0));
    if (!inner || inner->opcode() != Opcode::Load || inner->numOperands() != 1)
      return false;
    srcAddr = inner->operand(0);
    negated = true;
    return true;
  }
  return false;
}

bool rootsDiffer(Value* dstRoot, Value* srcRoot) {
  return dstRoot && srcRoot && dstRoot != srcRoot;
}

} // namespace

std::vector<AffineCopyRegionSummary>
collectAffineCopyRegions(Function* func,
                         const NaturalLoop& loop,
                         const std::vector<InductionVar>& ivs) {
  std::vector<AffineCopyRegionSummary> out;
  if (!func)
    return out;

  for (auto* bb : loop.blocks) {
    if (!bb)
      continue;
    for (auto& instPtr : bb->instructions()) {
      auto* store = instPtr.get();
      if (!store || store->opcode() != Opcode::Store || store->numOperands() != 2)
        continue;

      auto* valueInst = dynamic_cast<Instruction*>(store->operand(0));
      Value* srcAddr = nullptr;
      bool negated = false;
      if (!isSimpleAffineValue(valueInst, srcAddr, negated))
        continue;

      auto dstAccess = summarizeAffineAccess(store->operand(1), loop, ivs);
      auto srcAccess = summarizeAffineAccess(srcAddr, loop, ivs);
      if (!dstAccess.valid || !dstAccess.offset.valid ||
          !srcAccess.valid || !srcAccess.offset.valid)
        continue;
      if (!rootsDiffer(dstAccess.root, srcAccess.root))
        continue;

      AffineCopyRegionSummary summary;
      summary.valid = true;
      summary.store = store;
      summary.valueInst = valueInst;
      summary.dstRoot = dstAccess.root;
      summary.srcRoot = srcAccess.root;
      summary.dstAccess = dstAccess;
      summary.srcAccess = srcAccess;
      summary.negated = negated;
      out.push_back(std::move(summary));
    }
  }
  return out;
}

} // namespace analysis
