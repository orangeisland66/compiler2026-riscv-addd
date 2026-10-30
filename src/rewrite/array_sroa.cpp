// 文件职责：实现小型聚合对象标量替换，配合 Mem2Reg 消除局部数组访存。本文件负责实现具体逻辑。
#include "array_sroa.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace rewrite {

namespace {

using core_ir::BasicBlock;
using core_ir::ConstInt;
using core_ir::Function;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Type;
using core_ir::Value;

constexpr int kMaxBytes = 64;
constexpr int kMaxElems = 16;

// 可提升数组 alloca 的信息。
struct Candidate {
  Instruction* alloca;
  BasicBlock* parent;
  int elemSize;        // 每个元素的字节数，例如 i32 为 4
  int numElems;
  // 为每个产生地址的使用者记录字节偏移和对应的地址指令，后续使用者可
  // 直接改写为标量 alloca。
  std::vector<std::pair<int /*byteOffset*/, Instruction*>> addrInsts;
  // 按后序处理只产生地址的节点，使叶节点先于基地址移除。
  std::vector<Instruction*> removeOrder;
  struct ZeroInit {
    int byteOffset;
    int byteSize;
    Instruction* call;
  };
  std::vector<ZeroInit> zeroInits;
};

bool collectConstantAddressUses(Value* expectedBase,
                                Value* addr,
                                int totalBytes,
                                int elemSize,
                                int baseOffset,
                                Candidate& out,
                                std::unordered_map<int, int>& seen,
                                std::unordered_set<int>& removeSeen) {
  auto* inst = dynamic_cast<Instruction*>(addr);
  if (!inst)
    return false;

  auto seenIt = seen.find(inst->id());
  if (seenIt != seen.end())
    return seenIt->second == baseOffset;
  seen[inst->id()] = baseOffset;

  int offset = baseOffset;
  if (inst->opcode() == Opcode::Call) {
    if (inst->name() != "memset" || inst->numOperands() != 3 ||
        inst->operand(0) != expectedBase || !inst->uses().empty()) {
      return false;
    }
    auto* fill = dynamic_cast<ConstInt*>(inst->operand(1));
    auto* size = dynamic_cast<ConstInt*>(inst->operand(2));
    if (!fill || !size || fill->value() != 0)
      return false;
    int byteSize = size->value();
    if (byteSize <= 0 || offset < 0 || offset + byteSize > totalBytes ||
        offset % elemSize != 0 || byteSize % elemSize != 0) {
      return false;
    }
    out.zeroInits.push_back({offset, byteSize, inst});
    return true;
  } else if (inst->opcode() == Opcode::GEP) {
    if (inst->numOperands() < 2 || inst->operand(0) != expectedBase)
      return false;
    for (int i = 1; i < inst->numOperands(); ++i) {
      auto* offC = dynamic_cast<ConstInt*>(inst->operand(i));
      if (!offC)
        return false;
      offset += offC->value();
    }
    if (offset < 0 || offset >= totalBytes || offset % elemSize != 0)
      return false;
  } else if (inst->opcode() == Opcode::BitCast) {
    if (inst->numOperands() != 1 || inst->operand(0) != expectedBase)
      return false;
  } else {
    return false;
  }

  bool sawRealUse = false;
  for (Instruction* user : inst->uses()) {
    if (user->opcode() == Opcode::Load) {
      if (user->numOperands() != 1 || user->operand(0) != inst)
        return false;
      sawRealUse = true;
      continue;
    }
    if (user->opcode() == Opcode::Store) {
      if (user->numOperands() != 2 || user->operand(1) != inst || user->operand(0) == inst)
        return false;
      sawRealUse = true;
      continue;
    }
    if (!collectConstantAddressUses(inst, user, totalBytes, elemSize, offset,
                                    out, seen, removeSeen)) {
      return false;
    }
  }

  if (sawRealUse)
    out.addrInsts.push_back({offset, inst});
  if (removeSeen.insert(inst->id()).second)
    out.removeOrder.push_back(inst);
  return true;
}

bool isAllocaCandidate(Instruction* alloca, Candidate& out) {
  if (alloca->opcode() != Opcode::Alloca) return false;
  if (!alloca->isArrayObject()) return false;
  int total = alloca->memorySizeBytes();
  if (total <= 0 || total > kMaxBytes) return false;
  Type elemTy = alloca->memoryElemTy();
  // 当前处理 i32/f32 元素类型，即 4 字节标量。
  if (!(elemTy.isI32() || elemTy.isF32())) return false;
  int elemSize = 4;
  if (total % elemSize != 0) return false;
  int numElems = total / elemSize;
  if (numElems <= 0 || numElems > kMaxElems) return false;
  out.alloca = alloca;
  out.parent = alloca->parent();
  out.elemSize = elemSize;
  out.numElems = numElems;

  std::unordered_map<int, int> seenOffsets;
  std::unordered_set<int> removeSeen;
  for (Instruction* u : alloca->uses()) {
    if (!u || u->numOperands() < 1 || u->operand(0) != alloca)
      return false;
    if (!collectConstantAddressUses(alloca, u, total, elemSize, 0, out,
                                    seenOffsets, removeSeen)) {
      return false;
    }
  }
  return !out.addrInsts.empty();
}

void promote(Module& mod, Function* func, const Candidate& c) {
  // 1. 在同一父块中为 N 个元素创建标量 alloca，紧挨原 alloca 插入，
  //    使它们支配所有 GEP 使用。
  Type elemTy = c.alloca->memoryElemTy();
  std::vector<Instruction*> scalars(c.numElems);
  for (int i = 0; i < c.numElems; ++i) {
    auto* a = new Instruction(Opcode::Alloca, Type::ptr(),
                              "sroa." + c.alloca->name() + "." + std::to_string(i));
    a->setMemoryInfo(elemTy, c.elemSize, /*arrayObject=*/false, /*dims=*/{});
    c.parent->insertBefore(c.alloca, a);
    // 向函数登记，使后续 Mem2Reg 阶段能够处理这些 alloca。
    func->addAlloca(a);
    scalars[i] = a;
  }

  // 2. 把每条生成地址的指令改为对应标量 alloca 的地址。
  for (auto& [off, gep] : c.addrInsts) {
    int idx = off / c.elemSize;
    gep->replaceAllUsesWith(scalars[idx]);
  }

  Value* zeroValue = elemTy.isF32()
      ? static_cast<Value*>(mod.getConstFloat(0.0f))
      : static_cast<Value*>(mod.getConstInt(0));
  for (const auto& init : c.zeroInits) {
    BasicBlock* bb = init.call ? init.call->parent() : nullptr;
    if (!bb || !init.call)
      continue;
    for (int off = 0; off < init.byteSize; off += c.elemSize) {
      int idx = (init.byteOffset + off) / c.elemSize;
      auto* st = new Instruction(Opcode::Store, Type::voidTy());
      st->addOperand(zeroValue);
      st->addOperand(scalars[idx]);
      bb->insertBefore(init.call, st);
    }
    bb->remove(init.call);
  }

  // 3. 先删除只用于生成地址的整条指令闭包，再删除原 alloca。
  for (Instruction* addrInst : c.removeOrder) {
    if (!addrInst)
      continue;
    BasicBlock* bb = addrInst->parent();
    if (bb)
      bb->remove(addrInst);
  }
  func->removeAlloca(c.alloca);
  c.parent->remove(c.alloca);
}

} // namespace

bool applyArraySROA(Module& mod) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();

    // 从函数的 alloca 列表收集候选。先复制列表，
    // 因为 promote() 会修改原列表。
    std::vector<Instruction*> allocas;
    for (Instruction* a : func->allocas()) allocas.push_back(a);

    std::vector<Candidate> candidates;
    for (Instruction* a : allocas) {
      Candidate c;
      if (isAllocaCandidate(a, c)) candidates.push_back(std::move(c));
    }
    if (candidates.empty()) continue;
    for (auto& c : candidates) {
      promote(mod, func, c);
      changed = true;
    }
  }
  return changed;
}

} // namespace rewrite
