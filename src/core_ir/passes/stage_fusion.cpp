// 文件职责：合并相邻且访问关系兼容的循环阶段，减少重复遍历。
#include "stage_fusion.h"
#include "cse.h"
#include "passes_utils.h"
#include "row_locality.h"
#include "scalar_recurrence.h"

#include "../../analysis/cfg_utils.h"
#include "../../research_switch.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>

namespace core_ir {

static Instruction* firstNonPhiInstruction(BasicBlock* bb) {
  if (!bb)
    return nullptr;
  for (auto& instPtr : bb->instructions()) {
    if (instPtr->opcode() != Opcode::Phi)
      return instPtr.get();
  }
  return nullptr;
}

struct MapRowReduceChecksumPlan {
  Function* func = nullptr;
  GlobalVar* root = nullptr;
  Value* n = nullptr;
  analysis::NaturalLoop mapNatural;
  hir::CanonicalLoop mapLoop;
  analysis::NaturalLoop rowNatural;
  hir::CanonicalLoop rowLoop;
  hir::CanonicalLoop rowSumLoop;
  hir::CanonicalLoop rowSubLoop;
  hir::CanonicalLoop checksumLoop;
  Instruction* mapStore = nullptr;
  Value* mappedValue = nullptr;
  Instruction* rowStore = nullptr;
  Instruction* checksumAccPhi = nullptr;
  Value* checksumBoundValue = nullptr;
  BasicBlock* rowHeader = nullptr;
  BasicBlock* checksumExit = nullptr;
};

bool stageFusionTraceEnabled() {
  return sycEnvironmentValue("SYC_TRACE_STAGE_FUSION") != nullptr;
}

bool rejectStageFusion(const analysis::NaturalLoop& loop, const char* reason) {
  if (stageFusionTraceEnabled()) {
    std::cerr << "[stage-fusion] reject header="
              << (loop.header ? loop.header->name() : std::string("<null>"))
              << " reason=" << (reason ? reason : "-") << "\n";
  }
  return false;
}

Value* squareRootOperand(Value* value) {
  auto* mul = dynamic_cast<Instruction*>(value);
  if (!mul || mul->opcode() != Opcode::Mul || mul->numOperands() != 2)
    return nullptr;
  return mul->operand(0) == mul->operand(1) ? mul->operand(0) : nullptr;
}

Value* normalizedMatrixLoopBound(Value* value) {
  if (Value* root = squareRootOperand(value))
    return root;
  return value;
}

Value* squareMatrixLoopDimension(Value* value) {
  return squareRootOperand(value);
}

bool matchProductOf(Value* value, Value* lhs, Value* rhs) {
  auto* mul = dynamic_cast<Instruction*>(value);
  if (!mul || mul->opcode() != Opcode::Mul || mul->numOperands() != 2)
    return false;
  return (mul->operand(0) == lhs && mul->operand(1) == rhs) ||
         (mul->operand(0) == rhs && mul->operand(1) == lhs);
}

bool matchScaledProductOf(Value* value, Value* lhs, Value* rhs, int32_t scale) {
  if (scale == 1)
    return matchProductOf(value, lhs, rhs);
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->numOperands() != 2)
    return false;
  if (inst->opcode() == Opcode::Add) {
    if (isZeroInt(inst->operand(0)))
      return matchScaledProductOf(inst->operand(1), lhs, rhs, scale);
    if (isZeroInt(inst->operand(1)))
      return matchScaledProductOf(inst->operand(0), lhs, rhs, scale);
  }
  if (inst->opcode() == Opcode::Sub && isZeroInt(inst->operand(1)))
    return matchScaledProductOf(inst->operand(0), lhs, rhs, scale);
  if (inst->opcode() == Opcode::Shl) {
    auto* shift = dynamic_cast<ConstInt*>(inst->operand(1));
    if (!shift || shift->value() < 0 || shift->value() > 16)
      return false;
    int32_t shiftScale = 1 << shift->value();
    if (shiftScale == scale)
      return matchProductOf(inst->operand(0), lhs, rhs);
  }
  if (inst->opcode() == Opcode::Mul) {
    auto* lhsConst = dynamic_cast<ConstInt*>(inst->operand(0));
    auto* rhsConst = dynamic_cast<ConstInt*>(inst->operand(1));
    if (lhsConst && lhsConst->value() == scale)
      return matchProductOf(inst->operand(1), lhs, rhs);
    if (rhsConst && rhsConst->value() == scale)
      return matchProductOf(inst->operand(0), lhs, rhs);
  }
  return false;
}

bool matchScaledValueOf(Value* value, Value* base, int32_t scale) {
  if (!value || !base)
    return false;
  if (scale == 1 && value == base)
    return true;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->numOperands() != 2)
    return false;
  if (inst->opcode() == Opcode::Add) {
    if (isZeroInt(inst->operand(0)))
      return matchScaledValueOf(inst->operand(1), base, scale);
    if (isZeroInt(inst->operand(1)))
      return matchScaledValueOf(inst->operand(0), base, scale);
  }
  if (inst->opcode() == Opcode::Sub && isZeroInt(inst->operand(1)))
    return matchScaledValueOf(inst->operand(0), base, scale);
  if (inst->opcode() == Opcode::Shl) {
    auto* shift = dynamic_cast<ConstInt*>(inst->operand(1));
    if (!shift || shift->value() < 0 || shift->value() > 16)
      return false;
    int32_t shiftScale = 1 << shift->value();
    return shiftScale == scale && inst->operand(0) == base;
  }
  if (inst->opcode() == Opcode::Mul) {
    auto* lhsConst = dynamic_cast<ConstInt*>(inst->operand(0));
    auto* rhsConst = dynamic_cast<ConstInt*>(inst->operand(1));
    if (lhsConst && lhsConst->value() == scale && inst->operand(1) == base)
      return true;
    if (rhsConst && rhsConst->value() == scale && inst->operand(0) == base)
      return true;
  }
  return false;
}

bool matchRootZeroGEP(Value* value, GlobalVar* root) {
  auto* gep = dynamic_cast<Instruction*>(value);
  if (!gep || gep->opcode() != Opcode::GEP || gep->numOperands() != 2 ||
      gep->operand(0) != root) {
    return false;
  }
  auto* offset = dynamic_cast<ConstInt*>(gep->operand(1));
  return offset && offset->value() == 0;
}

bool matchRootRowBaseGEP(Value* value, GlobalVar* root, Value* row, Value* n) {
  auto* gep = dynamic_cast<Instruction*>(value);
  if (!gep || gep->opcode() != Opcode::GEP || gep->numOperands() != 2 ||
      gep->operand(0) != root) {
    return false;
  }
  return matchScaledProductOf(gep->operand(1), row, n, 4);
}

bool matchUnscaledMatrixIndex(Value* value, Value* row, Value* col, Value* n) {
  if (!value || !row || !col || !n)
    return false;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst)
    return false;
  if (inst->opcode() == Opcode::Add && inst->numOperands() == 2) {
    if (isZeroInt(inst->operand(0)))
      return matchUnscaledMatrixIndex(inst->operand(1), row, col, n);
    if (isZeroInt(inst->operand(1)))
      return matchUnscaledMatrixIndex(inst->operand(0), row, col, n);
    return (matchProductOf(inst->operand(0), row, n) &&
            inst->operand(1) == col) ||
           (matchProductOf(inst->operand(1), row, n) &&
            inst->operand(0) == col);
  }
  if (inst->opcode() == Opcode::Sub && inst->numOperands() == 2 &&
      isZeroInt(inst->operand(1))) {
    return matchUnscaledMatrixIndex(inst->operand(0), row, col, n);
  }
  return false;
}

bool matchMatrixElementOffset(Value* value, Value* row, Value* col, Value* n) {
  if (!value || !row || !col || !n)
    return false;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->numOperands() != 2)
    return false;
  if (inst->opcode() == Opcode::Add) {
    if (isZeroInt(inst->operand(0)))
      return matchMatrixElementOffset(inst->operand(1), row, col, n);
    if (isZeroInt(inst->operand(1)))
      return matchMatrixElementOffset(inst->operand(0), row, col, n);
    return (matchScaledProductOf(inst->operand(0), row, n, 4) &&
            matchScaledValueOf(inst->operand(1), col, 4)) ||
           (matchScaledProductOf(inst->operand(1), row, n, 4) &&
            matchScaledValueOf(inst->operand(0), col, 4));
  }
  if (inst->opcode() == Opcode::Sub && isZeroInt(inst->operand(1)))
    return matchMatrixElementOffset(inst->operand(0), row, col, n);
  if (inst->opcode() == Opcode::Shl) {
    auto* shift = dynamic_cast<ConstInt*>(inst->operand(1));
    return shift && shift->value() == 2 &&
           matchUnscaledMatrixIndex(inst->operand(0), row, col, n);
  }
  return false;
}

bool matchRootMatrixElementGEP(Value* value, GlobalVar* root,
                               Value* row, Value* col, Value* n) {
  auto* gep = dynamic_cast<Instruction*>(value);
  if (!gep || gep->opcode() != Opcode::GEP || gep->numOperands() != 2 ||
      gep->operand(0) != root) {
    return false;
  }
  return matchMatrixElementOffset(gep->operand(1), row, col, n);
}

bool matchRootVectorIndexGEP(Value* value, GlobalVar*& rootOut, Value* index) {
  auto* gep = dynamic_cast<Instruction*>(value);
  if (!gep || gep->opcode() != Opcode::GEP || gep->numOperands() != 2)
    return false;
  auto* root = dynamic_cast<GlobalVar*>(gep->operand(0));
  if (!root || root->elemTy() != Type::i32())
    return false;
  if (!matchScaledValueOf(gep->operand(1), index, 4))
    return false;
  rootOut = root;
  return true;
}

bool matchSequentialPointerPhi(Instruction* phi,
                               BasicBlock* preheader,
                               BasicBlock* latch,
                               Value*& initOut,
                               int32_t stepBytes) {
  if (!phi || phi->opcode() != Opcode::Phi || !phi->type().isPtr() ||
      phi->numOperands() < 4 || (phi->numOperands() % 2) != 0) {
    return false;
  }
  Value* init = nullptr;
  Value* next = nullptr;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
    if (pred == preheader)
      init = phi->operand(i);
    else if (pred == latch)
      next = phi->operand(i);
  }
  if (!init || !gepBasePlusConst(next, phi, stepBytes))
    return false;
  initOut = init;
  return true;
}

bool blockStoresValueToScalarGlobal(BasicBlock* bb, Value* value, GlobalVar* gv) {
  if (!bb || !value || !gv || gv->sizeBytes() != 4 || !gv->dims().empty())
    return false;
  for (auto& instPtr : bb->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst || inst->opcode() != Opcode::Store || inst->numOperands() != 2)
      continue;
    if (inst->operand(0) == value && memoryRoot(inst->operand(1)) == gv)
      return true;
  }
  return false;
}

GlobalVar* directScalarGlobalLoadRoot(Value* value) {
  auto* load = dynamic_cast<Instruction*>(value);
  if (!load || load->opcode() != Opcode::Load || load->numOperands() != 1)
    return nullptr;
  auto* gv = dynamic_cast<GlobalVar*>(memoryRoot(load->operand(0)));
  if (!gv || gv->sizeBytes() != 4 || !gv->dims().empty())
    return nullptr;
  return gv;
}

bool loopMayTouchScalarGlobal(const analysis::NaturalLoop& loop, GlobalVar* gv) {
  if (!gv)
    return true;
  for (auto* bb : loop.blocks) {
    if (!bb)
      return true;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return true;
      if (inst->opcode() == Opcode::Call)
        return true;
      if (inst->opcode() == Opcode::Store && inst->numOperands() == 2 &&
          memoryRoot(inst->operand(1)) == gv) {
        return true;
      }
    }
  }
  return false;
}

bool valueIsSameOrReloadedStoredBound(Value* value,
                                      Value* expected,
                                      BasicBlock* storeBlock,
                                      const analysis::NaturalLoop& rowLoop) {
  if (value == expected)
    return true;
  GlobalVar* gv = directScalarGlobalLoadRoot(value);
  return gv && blockStoresValueToScalarGlobal(storeBlock, expected, gv) &&
         !loopMayTouchScalarGlobal(rowLoop, gv);
}

bool loopHasNoCallsOrStores(const analysis::NaturalLoop& loop) {
  for (auto* bb : loop.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      if (inst->opcode() == Opcode::Call || inst->opcode() == Opcode::Store ||
          inst->opcode() == Opcode::VLoad || inst->opcode() == Opcode::VStore ||
          inst->opcode() == Opcode::Ret || inst->opcode() == Opcode::Alloca) {
        return false;
      }
    }
  }
  return true;
}

bool skippedRegionHasNoUnmodeledLiveOut(
    const std::vector<const analysis::NaturalLoop*>& loops,
    const std::unordered_map<Instruction*, BasicBlock*>& allowedExternalUses) {
  std::unordered_set<BasicBlock*> regionBlocks;
  for (const auto* loop : loops) {
    if (!loop)
      return false;
    for (BasicBlock* block : loop->blocks) {
      if (!block)
        return false;
      regionBlocks.insert(block);
    }
  }

  for (const auto* loop : loops) {
    for (BasicBlock* block : loop->blocks) {
      for (const auto& instPtr : block->instructions()) {
        Instruction* inst = instPtr.get();
        if (!inst || inst->isTerminator())
          continue;
        for (Instruction* user : inst->uses()) {
          if (!user || !user->parent())
            return false;
          if (regionBlocks.count(user->parent()))
            continue;
          auto allowed = allowedExternalUses.find(inst);
          if (allowed != allowedExternalUses.end() &&
              allowed->second == user->parent()) {
            continue;
          }
          return false;
        }
      }
    }
  }
  return true;
}

bool findSingleStoreToRoot(const analysis::NaturalLoop& loop,
                           GlobalVar* root,
                           Instruction*& storeOut) {
  storeOut = nullptr;
  for (auto* bb : loop.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      if (inst->opcode() == Opcode::Call || inst->opcode() == Opcode::VLoad ||
          inst->opcode() == Opcode::VStore ||
          inst->opcode() == Opcode::Ret || inst->opcode() == Opcode::Alloca) {
        return false;
      }
      if (inst->opcode() != Opcode::Store)
        continue;
      if (inst->numOperands() != 2)
        return false;
      Value* storeRoot = memoryRoot(inst->operand(1));
      auto* storeAddrInst = dynamic_cast<Instruction*>(inst->operand(1));
      if (storeRoot != root &&
          (!storeAddrInst || storeAddrInst->opcode() != Opcode::Phi)) {
        return false;
      }
      if (storeOut)
        return false;
      storeOut = inst;
    }
  }
  return storeOut != nullptr;
}

bool matchRowSumLoop(const analysis::NaturalLoop& natural,
                     const hir::CanonicalLoop& loop,
                     GlobalVar* root,
                     Value* row,
                     Value* n,
                     Instruction*& sumPhiOut,
                     Instruction*& ptrPhiOut) {
  if (!canonicalLoopIsForwardLt(loop) || !isZeroInt(loop.initVal) ||
      loop.boundVal != n || !loopHasSingleExit(natural, loop.exit) ||
      loop.body != loop.latch || !loopHasNoCallsOrStores(natural)) {
    if (stageFusionTraceEnabled()) {
      std::cerr << "[stage-fusion] row-sum-detail header="
                << (natural.header ? natural.header->name() : std::string("<null>"))
                << " forward=" << (canonicalLoopIsForwardLt(loop) ? 1 : 0)
                << " init0=" << (isZeroInt(loop.initVal) ? 1 : 0)
                << " bound=" << (loop.boundVal == n ? 1 : 0)
                << " singleExit=" << (loopHasSingleExit(natural, loop.exit) ? 1 : 0)
                << " bodyLatch=" << (loop.body == loop.latch ? 1 : 0)
                << " readonly=" << (loopHasNoCallsOrStores(natural) ? 1 : 0)
                << "\n";
    }
    return false;
  }

  std::vector<Instruction*> phis;
  std::vector<Value*> inits;
  std::vector<Value*> nexts;
  if (!collectHeaderLoopPhis(loop, phis, inits, &nexts))
    return false;

  // 在 header phi 中识别行指针 phi（初值为 root+row*n 的行基址）
  // 和唯一的零初值 i32 累加 phi；累加 phi 必须恰好一个，否则放弃匹配。
  Instruction* ptrPhi = nullptr;
  Instruction* sumPhi = nullptr;
  int sumPhiCount = 0;
  for (size_t i = 0; i < phis.size(); ++i) {
    Instruction* phi = phis[i];
    if (phi == loop.ivPhi)
      continue;
    Value* init = nullptr;
    if (matchSequentialPointerPhi(phi, loop.preheader, loop.latch, init)) {
      bool rowBase = matchRootRowBaseGEP(init, root, row, n);
      if (stageFusionTraceEnabled()) {
        auto* initGep = dynamic_cast<Instruction*>(init);
        auto* offInst = (initGep && initGep->numOperands() > 1)
                            ? dynamic_cast<Instruction*>(initGep->operand(1))
                            : nullptr;
        std::cerr << "[stage-fusion] row-sum-ptr-cand header="
                  << (natural.header ? natural.header->name() : std::string("<null>"))
                  << " phi=" << phi->id()
                  << " init=" << (init ? init->id() : -1)
                  << " row=" << (row ? row->id() : -1)
                  << " n=" << (n ? n->id() : -1)
                  << " off="
                  << (initGep && initGep->numOperands() > 1
                          ? initGep->operand(1)->id()
                          : -1)
                  << " offOp="
                  << (offInst ? opcodeName(offInst->opcode()) : "<none>")
                  << " off0="
                  << (offInst && offInst->numOperands() > 0
                          ? offInst->operand(0)->id()
                          : -1)
                  << " off1="
                  << (offInst && offInst->numOperands() > 1
                          ? offInst->operand(1)->id()
                          : -1)
                  << " initRoot="
                  << (initGep && initGep->numOperands() > 0
                          ? initGep->operand(0)->name()
                          : std::string("<none>"))
                  << " wantRoot=" << (root ? root->name() : std::string("<none>"))
                  << " rowBase=" << (rowBase ? 1 : 0) << "\n";
      }
      if (rowBase) {
        ptrPhi = phi;
        continue;
      }
    } else if (stageFusionTraceEnabled() && phi->type().isPtr()) {
      std::cerr << "[stage-fusion] row-sum-ptr-reject header="
                << (natural.header ? natural.header->name() : std::string("<null>"))
                << " phi=" << phi->id()
                << " ops=" << phi->numOperands()
                << " pre="
                << (loop.preheader ? loop.preheader->name() : std::string("<null>"))
                << " latch="
                << (loop.latch ? loop.latch->name() : std::string("<null>"))
                << "\n";
    }
    if (phi->type().isI32() && isZeroInt(inits[i])) {
      ++sumPhiCount;
      sumPhi = phi;
    }
  }
  if (!sumPhi || sumPhiCount != 1) {
    if (stageFusionTraceEnabled()) {
      std::cerr << "[stage-fusion] row-sum-phis header="
                << (natural.header ? natural.header->name() : std::string("<null>"))
                << " ptr=" << (ptrPhi ? 1 : 0)
                << " sum=" << (sumPhi ? 1 : 0)
                << " sumCount=" << sumPhiCount << "\n";
    }
    return false;
  }

  Instruction* load = nullptr;
  for (auto& instPtr : loop.body->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst || inst->opcode() != Opcode::Load || inst->numOperands() != 1)
      continue;
    bool isRowLoad =
        (ptrPhi && inst->operand(0) == ptrPhi) ||
        matchRootMatrixElementGEP(inst->operand(0), root, row, loop.ivPhi, n);
    if (isRowLoad) {
      if (load)
        return false;
      load = inst;
    }
  }
  auto* add = dynamic_cast<Instruction*>(nexts[std::find(phis.begin(), phis.end(), sumPhi) -
                                             phis.begin()]);
  if (!load || !add || add->opcode() != Opcode::Add || add->numOperands() != 2) {
    if (stageFusionTraceEnabled()) {
      std::cerr << "[stage-fusion] row-sum-add header="
                << (natural.header ? natural.header->name() : std::string("<null>"))
                << " load=" << (load ? 1 : 0)
                << " add=" << (add ? 1 : 0) << "\n";
    }
    return false;
  }
  if (!((add->operand(0) == sumPhi && add->operand(1) == load) ||
        (add->operand(1) == sumPhi && add->operand(0) == load))) {
    return false;
  }
  sumPhiOut = sumPhi;
  ptrPhiOut = ptrPhi;
  return true;
}

bool matchRowSubtractLoop(const analysis::NaturalLoop& natural,
                          const hir::CanonicalLoop& loop,
                          GlobalVar* root,
                          Value* row,
                          Value* n,
                          Value* rowSum,
                          Instruction*& storeOut) {
  if (!canonicalLoopIsForwardLt(loop) || !isZeroInt(loop.initVal) ||
      loop.boundVal != n || !loopHasSingleExit(natural, loop.exit) ||
      loop.body != loop.latch) {
    return false;
  }

  Instruction* store = nullptr;
  if (!findSingleStoreToRoot(natural, root, store))
    return false;

  std::vector<Instruction*> phis;
  std::vector<Value*> inits;
  std::vector<Value*> nexts;
  if (!collectHeaderLoopPhis(loop, phis, inits, &nexts))
    return false;

  Instruction* ptrPhi = nullptr;
  for (Instruction* phi : phis) {
    if (phi == loop.ivPhi)
      continue;
    Value* init = nullptr;
    if (matchSequentialPointerPhi(phi, loop.preheader, loop.latch, init) &&
        matchRootRowBaseGEP(init, root, row, n)) {
      ptrPhi = phi;
      break;
    }
  }
  bool storeUsesPtrPhi = ptrPhi && store->operand(1) == ptrPhi;
  bool storeUsesDirectAddr =
      matchRootMatrixElementGEP(store->operand(1), root, row, loop.ivPhi, n);
  if (!storeUsesPtrPhi && !storeUsesDirectAddr)
    return false;

  Instruction* load = nullptr;
  for (auto& instPtr : loop.body->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst || inst->opcode() != Opcode::Load || inst->numOperands() != 1)
      continue;
    bool isStoredLoad =
        (storeUsesPtrPhi && inst->operand(0) == ptrPhi) ||
        (storeUsesDirectAddr && inst->operand(0) == store->operand(1));
    if (isStoredLoad) {
      if (load)
        return false;
      load = inst;
    }
  }
  auto* sub = dynamic_cast<Instruction*>(store->operand(0));
  if (!load || !sub || sub->opcode() != Opcode::Sub ||
      sub->numOperands() != 2 || sub->operand(0) != load ||
      sub->operand(1) != rowSum) {
    return false;
  }
  storeOut = store;
  return true;
}

bool rowReductionEffectsAreProven(const analysis::NaturalLoop& rowLoop,
                                  Instruction* rowStore) {
  if (!rowStore)
    return false;
  for (BasicBlock* block : rowLoop.blocks) {
    if (!block)
      return false;
    for (auto& instPtr : block->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      if (inst->opcode() == Opcode::Call || inst->opcode() == Opcode::Ret ||
          inst->opcode() == Opcode::Alloca || inst->opcode() == Opcode::VLoad ||
          inst->opcode() == Opcode::VStore) {
        return false;
      }
      if (inst->opcode() == Opcode::Store && inst != rowStore)
        return false;
    }
  }
  return true;
}

bool matchChecksumLoop(const analysis::NaturalLoop& natural,
                       const hir::CanonicalLoop& loop,
                       GlobalVar* root,
                       Value* n,
                       BasicBlock* boundStoreBlock,
                       const analysis::NaturalLoop& rowLoop,
                       Instruction*& accPhiOut,
                       Value*& checksumBoundValueOut) {
  // 闭式结果汇总的是矩阵中的全部元素，因此 checksum 循环必须明确覆盖
  // n * n；只扫描一行或任意其它边界时继续执行原循环。
  Value* checksumN = squareMatrixLoopDimension(loop.boundVal);
  if (!checksumN ||
      !valueIsSameOrReloadedStoredBound(checksumN, n, boundStoreBlock, rowLoop) ||
      !canonicalLoopIsForwardLt(loop) || !isZeroInt(loop.initVal) ||
      !loopHasSingleExit(natural, loop.exit) || loop.body != loop.latch ||
      !loopHasNoCallsOrStores(natural)) {
    return false;
  }

  std::vector<Instruction*> phis;
  std::vector<Value*> inits;
  std::vector<Value*> nexts;
  if (!collectHeaderLoopPhis(loop, phis, inits, &nexts))
    return false;

  Instruction* ptrPhi = nullptr;
  Instruction* accPhi = nullptr;
  int accPhiCount = 0;
  for (size_t i = 0; i < phis.size(); ++i) {
    Instruction* phi = phis[i];
    if (phi == loop.ivPhi)
      continue;
    Value* init = nullptr;
    if (matchSequentialPointerPhi(phi, loop.preheader, loop.latch, init) &&
        matchRootZeroGEP(init, root)) {
      ptrPhi = phi;
      continue;
    }
    if (phi->type().isI32() && isZeroInt(inits[i])) {
      ++accPhiCount;
      accPhi = phi;
    }
  }
  if (!accPhi || accPhiCount != 1)
    return false;

  Instruction* load = nullptr;
  for (auto& instPtr : loop.body->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst || inst->opcode() != Opcode::Load || inst->numOperands() != 1)
      continue;
    GlobalVar* directRoot = nullptr;
    bool isChecksumLoad =
        (ptrPhi && inst->operand(0) == ptrPhi) ||
        (matchRootVectorIndexGEP(inst->operand(0), directRoot, loop.ivPhi) &&
         directRoot == root);
    if (isChecksumLoad) {
      if (load)
        return false;
      load = inst;
    }
  }
  auto* add = dynamic_cast<Instruction*>(nexts[std::find(phis.begin(), phis.end(), accPhi) -
                                             phis.begin()]);
  if (!load || !add || add->opcode() != Opcode::Add || add->numOperands() != 2)
    return false;
  if (!((add->operand(0) == accPhi && add->operand(1) == load) ||
        (add->operand(1) == accPhi && add->operand(0) == load))) {
    return false;
  }
  accPhiOut = accPhi;
  checksumBoundValueOut = checksumN;
  return true;
}

bool exitBlockCanBypassArrayState(BasicBlock* exit, GlobalVar* root, Module* mod) {
  if (!exit || !root || !mod)
    return false;
  Instruction* term = exit->terminator();
  if (!term || term->opcode() != Opcode::Ret)
    return false;
  for (auto& instPtr : exit->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst || inst->opcode() == Opcode::Phi || inst->isTerminator())
      continue;
    if (inst->opcode() == Opcode::Load && inst->numOperands() == 1 &&
        memoryRoot(inst->operand(0)) == root) {
      return false;
    }
    if (inst->opcode() == Opcode::Store && inst->numOperands() == 2 &&
        memoryRoot(inst->operand(1)) == root) {
      return false;
    }
    if (inst->opcode() == Opcode::Call) {
      if (mod->getFunction(inst->name()))
        return false;
      for (Value* operand : inst->operands()) {
        if (operand && operand->type().isPtr())
          return false;
      }
    }
  }
  return true;
}

bool matchMapLoop(const analysis::NaturalLoop& natural,
                  const hir::CanonicalLoop& loop,
                  MapRowReduceChecksumPlan& plan) {
  Value* n = squareMatrixLoopDimension(loop.boundVal);
  bool squareBound = n != nullptr;
  if (!n || !canonicalLoopIsForwardLt(loop) || !isZeroInt(loop.initVal) ||
      !loopHasSingleExit(natural, loop.exit) || loop.body != loop.latch) {
    if (stageFusionTraceEnabled()) {
      std::cerr << "[stage-fusion] map-detail header="
                << (natural.header ? natural.header->name() : std::string("<null>"))
                << " square=" << (squareBound ? 1 : 0)
                << " forward=" << (canonicalLoopIsForwardLt(loop) ? 1 : 0)
                << " init0=" << (isZeroInt(loop.initVal) ? 1 : 0)
                << " singleExit=" << (loopHasSingleExit(natural, loop.exit) ? 1 : 0)
                << " bodyLatch=" << (loop.body == loop.latch ? 1 : 0)
                << "\n";
    }
    return false;
  }

  Instruction* store = nullptr;
  for (auto* bb : natural.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      if (inst->opcode() == Opcode::Call || inst->opcode() == Opcode::VLoad ||
          inst->opcode() == Opcode::VStore ||
          inst->opcode() == Opcode::Ret || inst->opcode() == Opcode::Alloca) {
        return false;
      }
      if (inst->opcode() != Opcode::Store)
        continue;
      if (store)
        return false;
      store = inst;
    }
  }
  if (!store) {
    if (stageFusionTraceEnabled()) {
      std::cerr << "[stage-fusion] map-store header="
                << (natural.header ? natural.header->name() : std::string("<null>"))
                << " missing=1\n";
    }
    return false;
  }

  auto* ptrPhi = dynamic_cast<Instruction*>(store->operand(1));
  Value* ptrInit = nullptr;
  GlobalVar* root = nullptr;
  if (matchSequentialPointerPhi(ptrPhi, loop.preheader, loop.latch, ptrInit)) {
    auto* initGep = dynamic_cast<Instruction*>(ptrInit);
    root = (initGep && initGep->opcode() == Opcode::GEP &&
            initGep->numOperands() == 2)
               ? dynamic_cast<GlobalVar*>(initGep->operand(0))
               : nullptr;
    if (!root || root->elemTy() != Type::i32() ||
        !matchRootZeroGEP(ptrInit, root)) {
      return false;
    }
  } else if (!matchRootVectorIndexGEP(store->operand(1), root, loop.ivPhi)) {
    if (stageFusionTraceEnabled()) {
      std::cerr << "[stage-fusion] map-ptr header="
                << (natural.header ? natural.header->name() : std::string("<null>"))
                << " addrOp="
                << (dynamic_cast<Instruction*>(store->operand(1))
                        ? opcodeName(dynamic_cast<Instruction*>(store->operand(1))->opcode())
                        : "<none>")
                << "\n";
    }
    return false;
  }

  plan.root = root;
  plan.n = n;
  plan.mapNatural = natural;
  plan.mapLoop = loop;
  plan.mapStore = store;
  plan.mappedValue = store->operand(0);
  return true;
}

const analysis::NaturalLoop* loopForHeader(
    const std::vector<analysis::NaturalLoop>& loops,
    BasicBlock* header) {
  if (!header)
    return nullptr;
  for (const auto& loop : loops) {
    if (loop.header == header)
      return &loop;
  }
  return nullptr;
}

bool matchMapRowReduceChecksum(Function* func,
                               const std::vector<analysis::NaturalLoop>& loops,
                               const analysis::NaturalLoop& mapNatural,
                               MapRowReduceChecksumPlan& plan) {
  if (!func || !func->parent())
    return rejectStageFusion(mapNatural, "missing-func");
  BasicBlock* mapPreheader = analysis::dedicatedPreheader(mapNatural);
  hir::CanonicalLoop mapLoop;
  if (!mapPreheader ||
      !hir::recognizeCanonicalLoop(mapNatural, mapPreheader, mapLoop) ||
      !matchMapLoop(mapNatural, mapLoop, plan)) {
    return rejectStageFusion(mapNatural, "map-loop");
  }

  BasicBlock* rowHeader = singleBrTarget(mapLoop.exit);
  const auto* rowNatural = loopForHeader(loops, rowHeader);
  if (!rowNatural)
    return rejectStageFusion(mapNatural, "no-row-loop");
  BasicBlock* rowPreheader = analysis::dedicatedPreheader(*rowNatural);
  hir::CanonicalLoop rowLoop;
  if (rowPreheader != mapLoop.exit ||
      !hir::recognizeCanonicalLoop(*rowNatural, rowPreheader, rowLoop) ||
      !canonicalLoopIsForwardLt(rowLoop) || !isZeroInt(rowLoop.initVal) ||
      rowLoop.boundVal != plan.n ||
      !loopHasSingleExit(*rowNatural, rowLoop.exit) ||
      !analysis::collectLeadingPhi(rowLoop.exit).empty()) {
    return rejectStageFusion(mapNatural, "row-loop");
  }

  BasicBlock* rowSumHeader = singleBrTarget(rowLoop.body);
  const auto* rowSumNatural = loopForHeader(loops, rowSumHeader);
  BasicBlock* rowSumPreheader = rowSumNatural ? analysis::dedicatedPreheader(*rowSumNatural)
                                              : nullptr;
  hir::CanonicalLoop rowSumLoop;
  if (!rowSumNatural || rowSumPreheader != rowLoop.body ||
      !hir::recognizeCanonicalLoop(*rowSumNatural, rowSumPreheader, rowSumLoop)) {
    return rejectStageFusion(mapNatural, "row-sum-shape");
  }
  Instruction* rowSumPhi = nullptr;
  Instruction* rowSumPtrPhi = nullptr;
  if (!matchRowSumLoop(*rowSumNatural, rowSumLoop, plan.root, rowLoop.ivPhi,
                       plan.n, rowSumPhi, rowSumPtrPhi)) {
    return rejectStageFusion(mapNatural, "row-sum-loop");
  }

  BasicBlock* rowSubHeader = singleBrTarget(rowSumLoop.exit);
  const auto* rowSubNatural = loopForHeader(loops, rowSubHeader);
  BasicBlock* rowSubPreheader = rowSubNatural ? analysis::dedicatedPreheader(*rowSubNatural)
                                              : nullptr;
  hir::CanonicalLoop rowSubLoop;
  Instruction* rowStore = nullptr;
  if (!rowSubNatural || rowSubPreheader != rowSumLoop.exit ||
      !hir::recognizeCanonicalLoop(*rowSubNatural, rowSubPreheader, rowSubLoop) ||
      rowSubLoop.exit != rowLoop.latch ||
      !matchRowSubtractLoop(*rowSubNatural, rowSubLoop, plan.root,
                            rowLoop.ivPhi, plan.n, rowSumPhi, rowStore)) {
    return rejectStageFusion(mapNatural, "row-sub-loop");
  }
  if (!rowReductionEffectsAreProven(*rowNatural, rowStore))
    return rejectStageFusion(mapNatural, "row-effects");

  BasicBlock* checksumHeader = singleBrTarget(rowLoop.exit);
  const auto* checksumNatural = loopForHeader(loops, checksumHeader);
  BasicBlock* checksumPreheader = checksumNatural
                                      ? analysis::dedicatedPreheader(*checksumNatural)
                                      : nullptr;
  hir::CanonicalLoop checksumLoop;
  if (!checksumNatural || checksumPreheader != rowLoop.exit ||
      !hir::recognizeCanonicalLoop(*checksumNatural, checksumPreheader,
                                   checksumLoop) ||
      !analysis::collectLeadingPhi(checksumLoop.exit).empty()) {
    return rejectStageFusion(mapNatural, "checksum-shape");
  }
  Instruction* checksumAccPhi = nullptr;
  Value* checksumBoundValue = nullptr;
  if (!matchChecksumLoop(*checksumNatural, checksumLoop, plan.root, plan.n,
                         mapLoop.exit, *rowNatural, checksumAccPhi,
                         checksumBoundValue)) {
    return rejectStageFusion(mapNatural, "checksum-loop");
  }
  if (!exitBlockCanBypassArrayState(checksumLoop.exit, plan.root,
                                    func->parent())) {
    return rejectStageFusion(mapNatural, "exit-observes-array");
  }

  std::vector<const analysis::NaturalLoop*> skippedLoops = {
      rowNatural, rowSumNatural, rowSubNatural, checksumNatural};
  std::unordered_map<Instruction*, BasicBlock*> allowedExternalUses;
  allowedExternalUses[checksumAccPhi] = checksumLoop.exit;
  if (checksumBoundValue != plan.n) {
    if (auto* boundInst = dynamic_cast<Instruction*>(checksumBoundValue)) {
      if (checksumNatural->blocks.count(boundInst->parent()))
        allowedExternalUses[boundInst] = checksumLoop.exit;
    }
  }
  if (!skippedRegionHasNoUnmodeledLiveOut(skippedLoops,
                                          allowedExternalUses)) {
    return rejectStageFusion(mapNatural, "skipped-region-live-out");
  }

  plan.func = func;
  plan.rowNatural = *rowNatural;
  plan.rowLoop = rowLoop;
  plan.rowSumLoop = rowSumLoop;
  plan.rowSubLoop = rowSubLoop;
  plan.rowStore = rowStore;
  plan.checksumLoop = checksumLoop;
  plan.checksumAccPhi = checksumAccPhi;
  plan.checksumBoundValue = checksumBoundValue;
  plan.rowHeader = rowHeader;
  plan.checksumExit = checksumLoop.exit;
  return true;
}

Instruction* makeFusionInst(Opcode op, Type ty, const std::string& name,
                            std::initializer_list<Value*> operands) {
  auto* inst = new Instruction(op, ty, name);
  for (auto* operand : operands)
    inst->addOperand(operand);
  return inst;
}

void replaceUsesInBlock(BasicBlock* bb, Value* oldValue, Value* newValue,
                        Instruction* skipA = nullptr,
                        Instruction* skipB = nullptr) {
  if (!bb || !oldValue || !newValue)
    return;
  for (auto& instPtr : bb->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst || inst == skipA || inst == skipB)
      continue;
    for (int i = 0; i < inst->numOperands(); ++i) {
      if (inst->operand(i) == oldValue)
        inst->setOperand(i, newValue);
    }
  }
}

#ifdef SYC_ENABLE_RESEARCH_PASSES
bool materializeMapRowReduceChecksumFusion(
    const MapRowReduceChecksumPlan& plan) {
  Function* func = plan.func;
  if (!func || !func->parent() || !plan.mapLoop.header || !plan.mapLoop.latch ||
      !plan.mapLoop.exit || !plan.rowHeader || !plan.checksumExit ||
      !plan.mapStore || !plan.checksumAccPhi || !plan.mappedValue) {
    return false;
  }

  Instruction* firstNonPhi = firstNonPhiInstruction(plan.mapLoop.header);
  if (!firstNonPhi)
    return false;
  auto* sumPhi = new Instruction(Opcode::Phi, Type::i32(), "stage.fuse.sum");
  sumPhi->addOperand(func->parent()->getConstInt(0));
  sumPhi->addOperand(plan.mapLoop.preheader);
  plan.mapLoop.header->insertBefore(firstNonPhi, sumPhi);

  Instruction* latchTerm = plan.mapLoop.latch->terminator();
  if (!latchTerm || latchTerm->opcode() != Opcode::Br)
    return false;
  Instruction* sumNext = insertModRecInstBefore(
      plan.mapLoop.latch, latchTerm, Opcode::Add, Type::i32(),
      "stage.fuse.sum.next", {sumPhi, plan.mappedValue});
  sumPhi->addOperand(sumNext);
  sumPhi->addOperand(plan.mapLoop.latch);

  Instruction* exitTerm = plan.mapLoop.exit->terminator();
  if (!exitTerm || exitTerm->opcode() != Opcode::Br ||
      exitTerm->numOperands() != 1 || exitTerm->operand(0) != plan.rowHeader) {
    return false;
  }

  auto* oneMinusN = insertModRecInstBefore(
      plan.mapLoop.exit, exitTerm, Opcode::Sub, Type::i32(),
      "stage.fuse.one.minus.n", {func->parent()->getConstInt(1), plan.n});
  auto* fusedChecksum = insertModRecInstBefore(
      plan.mapLoop.exit, exitTerm, Opcode::Mul, Type::i32(),
      "stage.fuse.checksum", {oneMinusN, sumPhi});
  auto* nonNegativeN = insertModRecICmpBefore(
      plan.mapLoop.exit, exitTerm, "ge", "stage.fuse.n.nonnegative",
      plan.n, func->parent()->getConstInt(0));

  auto* checksumPhi =
      new Instruction(Opcode::Phi, Type::i32(), "stage.fuse.checksum.phi");
  checksumPhi->addOperand(plan.checksumAccPhi);
  checksumPhi->addOperand(plan.checksumLoop.header);
  checksumPhi->addOperand(fusedChecksum);
  checksumPhi->addOperand(plan.mapLoop.exit);
  insertBeforeFirstNonPhi(plan.checksumExit, checksumPhi);

  Instruction* boundPhi = nullptr;
  if (plan.checksumBoundValue && plan.checksumBoundValue != plan.n) {
    boundPhi = new Instruction(Opcode::Phi, Type::i32(), "stage.fuse.bound.phi");
    boundPhi->addOperand(plan.checksumBoundValue);
    boundPhi->addOperand(plan.checksumLoop.header);
    boundPhi->addOperand(plan.n);
    boundPhi->addOperand(plan.mapLoop.exit);
    insertBeforeFirstNonPhi(plan.checksumExit, boundPhi);
  }

  replaceUsesInBlock(plan.checksumExit, plan.checksumAccPhi, checksumPhi,
                     checksumPhi, boundPhi);
  if (boundPhi)
    replaceUsesInBlock(plan.checksumExit, plan.checksumBoundValue, boundPhi,
                       checksumPhi, boundPhi);

  plan.mapLoop.exit->remove(exitTerm);
  auto* condBr = new Instruction(Opcode::CondBr, Type::voidTy());
  condBr->addOperand(nonNegativeN);
  condBr->addOperand(plan.checksumExit);
  condBr->addOperand(plan.rowHeader);
  plan.mapLoop.exit->pushBack(condBr);

  func->recomputeCFG();
  return true;
}

bool applyMapRowReduceChecksumFusion(Function* func,
                                     analysis::AnalysisManager& analysisManager) {
  if (!func || !func->entryBlock())
    return false;
  analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
  if (cache.instructionCount() > 20000)
    return false;
  auto loops = cache.loops();
  std::sort(loops.begin(), loops.end(),
            [](const analysis::NaturalLoop& lhs,
               const analysis::NaturalLoop& rhs) {
              if (lhs.header && rhs.header)
                return lhs.header->id() < rhs.header->id();
              return lhs.header < rhs.header;
            });
  for (const auto& loop : loops) {
    MapRowReduceChecksumPlan plan;
    if (!matchMapRowReduceChecksum(func, loops, loop, plan))
      continue;
    if (materializeMapRowReduceChecksumFusion(plan)) {
      if (sycEnvironmentValue("SYC_TRACE_STAGE_FUSION")) {
        std::cerr << "[stage-fusion] func=" << func->name()
                  << " root=" << plan.root->name()
                  << " map=" << plan.mapLoop.header->name() << "\n";
      }
      return true;
    }
  }
  return false;
}

bool applyMapRowReduceChecksumFusion(Module& mod,
                                     analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func)
      continue;
    if (applyMapRowReduceChecksumFusion(func, analysisManager)) {
      changed = true;
      analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
    }
  }
  return changed;
}
#endif

struct RowBroadcastTransposeChecksumPlan {
  Function* func = nullptr;
  GlobalVar* root = nullptr;
  Value* bound = nullptr;
  hir::CanonicalLoop rowLoop;
  hir::CanonicalLoop minLoop;
  hir::CanonicalLoop broadcastLoop;
  hir::CanonicalLoop transposeOuterLoop;
  hir::CanonicalLoop transposeInnerLoop;
  hir::CanonicalLoop checksumOuterLoop;
  hir::CanonicalLoop checksumInnerLoop;
  Instruction* minPhi = nullptr;
  Instruction* checksumAccPhi = nullptr;
  BasicBlock* checksumExit = nullptr;
  struct ProducerPlan {
    bool valid = false;
    analysis::NaturalLoop outerNatural;
    analysis::NaturalLoop colNatural;
    std::vector<hir::CanonicalLoop> innerLoops;
    hir::CanonicalLoop outerLoop;
    hir::CanonicalLoop colLoop;
    Instruction* store = nullptr;
    Value* minInit = nullptr;
  } producer;
  std::vector<analysis::NaturalLoop> loops;
};

Value* rowBroadcastPhiIncoming(Instruction* phi, BasicBlock* pred) {
  if (!phi || phi->opcode() != Opcode::Phi || !pred)
    return nullptr;
  Value* result = nullptr;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    auto* incomingPred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
    if (incomingPred != pred)
      continue;
    if (result && result != phi->operand(i))
      return nullptr;
    result = phi->operand(i);
  }
  return result;
}

bool sameLoopBound(Value* lhs, Value* rhs) {
  lhs = normalizedMatrixLoopBound(lhs);
  rhs = normalizedMatrixLoopBound(rhs);
  if (lhs == rhs)
    return true;
  auto* lhsConst = dynamic_cast<ConstInt*>(lhs);
  auto* rhsConst = dynamic_cast<ConstInt*>(rhs);
  return lhsConst && rhsConst && lhsConst->value() == rhsConst->value();
}

bool matchRowByteStride(Value* value, Value* row, Value* bound) {
  if (matchScaledProductOf(value, row, bound, 4))
    return true;
  auto* boundConst = dynamic_cast<ConstInt*>(normalizedMatrixLoopBound(bound));
  if (!boundConst)
    return false;
  int64_t stride = static_cast<int64_t>(boundConst->value()) * 4;
  if (stride < std::numeric_limits<int32_t>::min() ||
      stride > std::numeric_limits<int32_t>::max()) {
    return false;
  }
  return matchScaledValueOf(value, row, static_cast<int32_t>(stride));
}

bool matchMatrixElementOffsetFlexible(Value* value,
                                      Value* row,
                                      Value* col,
                                      Value* bound) {
  if (matchMatrixElementOffset(value, row, col, bound))
    return true;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || inst->opcode() != Opcode::Add || inst->numOperands() != 2)
    return false;
  return (matchRowByteStride(inst->operand(0), row, bound) &&
          matchScaledValueOf(inst->operand(1), col, 4)) ||
         (matchRowByteStride(inst->operand(1), row, bound) &&
          matchScaledValueOf(inst->operand(0), col, 4));
}

bool matchRootRowBaseGEPFlexible(Value* value,
                                 GlobalVar* root,
                                 Value* row,
                                 Value* bound) {
  if (matchRootRowBaseGEP(value, root, row, bound))
    return true;
  auto* gep = dynamic_cast<Instruction*>(value);
  if (!gep || gep->opcode() != Opcode::GEP || gep->numOperands() != 2 ||
      gep->operand(0) != root) {
    return false;
  }
  return matchRowByteStride(gep->operand(1), row, bound);
}

bool matchRootMatrixElementGEPFlexible(Value* value,
                                       GlobalVar* root,
                                       Value* row,
                                       Value* col,
                                       Value* bound) {
  if (matchRootMatrixElementGEP(value, root, row, col, bound))
    return true;
  auto* gep = dynamic_cast<Instruction*>(value);
  if (!gep || gep->opcode() != Opcode::GEP || gep->numOperands() != 2 ||
      gep->operand(0) != root) {
    return false;
  }
  return matchMatrixElementOffsetFlexible(gep->operand(1), row, col, bound);
}

bool matchRowSequentialLoad(Instruction* load,
                            Instruction* ptrPhi,
                            GlobalVar* root,
                            const hir::CanonicalLoop& loop,
                            Value* row,
                            Value* bound) {
  if (!load || load->opcode() != Opcode::Load || load->numOperands() != 1)
    return false;
  if (ptrPhi && load->operand(0) == ptrPhi)
    return true;
  return matchRootMatrixElementGEPFlexible(load->operand(0), root, row,
                                           loop.ivPhi, bound);
}

bool matchRowSequentialStore(Instruction* store,
                             Instruction* ptrPhi,
                             GlobalVar* root,
                             const hir::CanonicalLoop& loop,
                             Value* row,
                             Value* bound,
                             Value* value) {
  if (!store || store->opcode() != Opcode::Store ||
      store->numOperands() != 2 || store->operand(0) != value) {
    return false;
  }
  if (ptrPhi && store->operand(1) == ptrPhi)
    return true;
  return matchRootMatrixElementGEPFlexible(store->operand(1), root, row,
                                           loop.ivPhi, bound);
}

std::string compareCondition(const Instruction* compare) {
  if (!compare)
    return {};
  const std::string& name = compare->name();
  const std::size_t separator = name.find('_');
  return separator == std::string::npos ? name : name.substr(0, separator);
}

// 证明 merge phi 实现的是 signed min(old, loaded)。不能只检查 phi 同时引用
// old/load：相同形态也可能是 max，或者由无关条件选择，都会令阶段融合误编译。
bool matchSignedMinMerge(Instruction* merge,
                         Instruction* oldValue,
                         Instruction* loadedValue) {
  if (!merge || !oldValue || !loadedValue || merge->opcode() != Opcode::Phi ||
      merge->numOperands() != 4 || !merge->parent()) {
    return false;
  }

  BasicBlock* oldPred = nullptr;
  BasicBlock* loadedPred = nullptr;
  for (int index = 0; index + 1 < merge->numOperands(); index += 2) {
    auto* pred = dynamic_cast<BasicBlock*>(merge->operand(index + 1));
    if (!pred)
      return false;
    if (merge->operand(index) == oldValue)
      oldPred = pred;
    else if (merge->operand(index) == loadedValue)
      loadedPred = pred;
    else
      return false;
  }
  if (!oldPred || !loadedPred || oldPred == loadedPred)
    return false;

  BasicBlock* conditionBlock = nullptr;
  Instruction* condition = nullptr;
  bool loadedOnTrue = false;
  Function* function = merge->parent()->parent();
  if (!function)
    return false;
  for (const auto& blockPtr : function->blocks()) {
    BasicBlock* block = blockPtr.get();
    Instruction* term = block ? block->terminator() : nullptr;
    if (!term || term->opcode() != Opcode::CondBr || term->numOperands() != 3)
      continue;
    auto* trueTarget = dynamic_cast<BasicBlock*>(term->operand(1));
    auto* falseTarget = dynamic_cast<BasicBlock*>(term->operand(2));
    const auto reachesIncoming = [&](BasicBlock* target, BasicBlock* pred) {
      // 直接跳到 merge 时，phi 的 incoming predecessor 是当前条件块；
      // 经独立 edge block 时则就是该目标块。
      return target == pred || (target == merge->parent() && block == pred);
    };
    if (reachesIncoming(trueTarget, loadedPred) &&
        reachesIncoming(falseTarget, oldPred))
      loadedOnTrue = true;
    else if (reachesIncoming(trueTarget, oldPred) &&
             reachesIncoming(falseTarget, loadedPred))
      loadedOnTrue = false;
    else
      continue;
    conditionBlock = block;
    condition = dynamic_cast<Instruction*>(term->operand(0));
    break;
  }
  if (!conditionBlock || !condition || condition->opcode() != Opcode::ICmp ||
      condition->numOperands() != 2) {
    if (stageFusionTraceEnabled()) {
      std::cerr << "[stage-fusion] min-merge missing-condition old-pred="
                << oldPred->name() << " loaded-pred=" << loadedPred->name()
                << " merge=" << merge->parent()->name() << "\n";
    }
    return false;
  }

  const std::string predicate = compareCondition(condition);
  const bool loadLessOld =
      (predicate == "lt" || predicate == "le") &&
      condition->operand(0) == loadedValue && condition->operand(1) == oldValue;
  const bool oldGreaterLoad =
      (predicate == "gt" || predicate == "ge") &&
      condition->operand(0) == oldValue && condition->operand(1) == loadedValue;
  const bool oldLessLoad =
      (predicate == "lt" || predicate == "le") &&
      condition->operand(0) == oldValue && condition->operand(1) == loadedValue;
  const bool loadGreaterOld =
      (predicate == "gt" || predicate == "ge") &&
      condition->operand(0) == loadedValue && condition->operand(1) == oldValue;
  const bool matched = loadedOnTrue ? (loadLessOld || oldGreaterLoad)
                                    : (oldLessLoad || loadGreaterOld);
  if (!matched && stageFusionTraceEnabled()) {
    std::cerr << "[stage-fusion] min-merge predicate=" << predicate
              << " loaded-on-true=" << loadedOnTrue << "\n";
  }
  return matched;
}

bool matchRowMinLoop(const analysis::NaturalLoop& natural,
                     const hir::CanonicalLoop& loop,
                     GlobalVar* root,
                     Value* row,
                     Value* bound,
                     Instruction*& minPhiOut) {
  if (!canonicalLoopIsForwardLt(loop) || !isZeroInt(loop.initVal) ||
      !sameLoopBound(loop.boundVal, bound) ||
      !loopHasSingleExit(natural, loop.exit)) {
    return false;
  }

  Instruction* ptrPhi = nullptr;
  Value* ptrInit = nullptr;
  for (Instruction* phi : analysis::collectLeadingPhi(loop.header)) {
    if (matchSequentialPointerPhi(phi, loop.preheader, loop.latch, ptrInit) &&
        matchRootRowBaseGEPFlexible(ptrInit, root, row, bound)) {
      ptrPhi = phi;
      break;
    }
  }

  Instruction* rowLoad = nullptr;
  for (BasicBlock* bb : natural.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      if (inst->opcode() == Opcode::Call || inst->opcode() == Opcode::Store ||
          inst->opcode() == Opcode::VLoad || inst->opcode() == Opcode::VStore ||
          inst->opcode() == Opcode::Ret ||
          inst->opcode() == Opcode::Alloca) {
        return false;
      }
      if (matchRowSequentialLoad(inst, ptrPhi, root, loop, row, bound)) {
        // 前端保留的安全分支可能在 then 块留下同址、无使用的重复 load。
        // 第一次 load 已在分支前执行，因此该 load 不增加可观察值或新 trap；
        // 只忽略这一严格子集，任何有使用或异址的第二次读取仍保守拒绝。
        if (inst->uses().empty())
          continue;
        if (rowLoad)
          return false;
        rowLoad = inst;
      }
    }
  }
  if (!rowLoad) {
    if (stageFusionTraceEnabled())
      std::cerr << "[stage-fusion] row-min no-used-load\n";
    return false;
  }

  for (Instruction* phi : analysis::collectLeadingPhi(loop.header)) {
    if (!phi || phi == loop.ivPhi || !phi->type().isI32())
      continue;
    Value* init = nullptr;
    Value* next = nullptr;
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
      if (pred == loop.preheader)
        init = phi->operand(i);
      else if (pred == loop.latch)
        next = phi->operand(i);
    }
    auto* nextPhi = dynamic_cast<Instruction*>(next);
    if (!init || !nextPhi || nextPhi->opcode() != Opcode::Phi ||
        !nextPhi->parent() || !natural.blocks.count(nextPhi->parent())) {
      continue;
    }
    if (stageFusionTraceEnabled()) {
      std::cerr << "[stage-fusion] row-min candidate phi=" << phi->id()
                << " next=" << nextPhi->id() << " load=" << rowLoad->id()
                << "\n";
    }
    if (matchSignedMinMerge(nextPhi, phi, rowLoad)) {
      minPhiOut = phi;
      return true;
    }
  }
  return false;
}

bool matchRowBroadcastLoop(const analysis::NaturalLoop& natural,
                           const hir::CanonicalLoop& loop,
                           GlobalVar* root,
                           Value* row,
                           Value* bound,
                           Value* broadcastValue) {
  if (!canonicalLoopIsForwardLt(loop) || !isZeroInt(loop.initVal) ||
      !sameLoopBound(loop.boundVal, bound) ||
      !loopHasSingleExit(natural, loop.exit) || loop.body != loop.latch) {
    return false;
  }

  Instruction* ptrPhi = nullptr;
  Value* ptrInit = nullptr;
  for (Instruction* phi : analysis::collectLeadingPhi(loop.header)) {
    if (matchSequentialPointerPhi(phi, loop.preheader, loop.latch, ptrInit) &&
        matchRootRowBaseGEPFlexible(ptrInit, root, row, bound)) {
      ptrPhi = phi;
      break;
    }
  }

  Instruction* store = nullptr;
  for (BasicBlock* bb : natural.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      if (inst->opcode() == Opcode::Call || inst->opcode() == Opcode::Load ||
          inst->opcode() == Opcode::VLoad || inst->opcode() == Opcode::VStore ||
          inst->opcode() == Opcode::Ret ||
          inst->opcode() == Opcode::Alloca) {
        return false;
      }
      if (inst->opcode() == Opcode::Store) {
        if (store)
          return false;
        store = inst;
      }
    }
  }
  return matchRowSequentialStore(store, ptrPhi, root, loop, row, bound,
                                 broadcastValue);
}

bool matchTransposeNegateInnerLoop(const analysis::NaturalLoop& natural,
                                   const hir::CanonicalLoop& loop,
                                   GlobalVar* root,
                                   Value* outer,
                                   Value* bound) {
  if (!canonicalLoopIsForwardLt(loop) || !isZeroInt(loop.initVal) ||
      !sameLoopBound(loop.boundVal, bound) ||
      !loopHasSingleExit(natural, loop.exit) || loop.body != loop.latch) {
    return false;
  }

  // 扫描内层循环体，要求恰好一条 load A[iv][outer] -> neg ->
  // store A[outer][iv] 链（转置取负），出现 Call/VLoad/VStore 等即拒绝。
  Instruction* load = nullptr;
  Instruction* neg = nullptr;
  Instruction* store = nullptr;
  for (BasicBlock* bb : natural.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return false;
      if (inst->opcode() == Opcode::Call || inst->opcode() == Opcode::VLoad ||
          inst->opcode() == Opcode::VStore ||
          inst->opcode() == Opcode::Ret || inst->opcode() == Opcode::Alloca) {
        return false;
      }
      if (inst->opcode() == Opcode::Load &&
          matchRootMatrixElementGEPFlexible(inst->operand(0), root, loop.ivPhi,
                                            outer, bound)) {
        if (load)
          return false;
        load = inst;
        continue;
      }
      if (inst->opcode() == Opcode::Neg && inst->numOperands() == 1 &&
          inst->operand(0) == load) {
        if (neg)
          return false;
        neg = inst;
        continue;
      }
      if (inst->opcode() == Opcode::Store &&
          inst->numOperands() == 2 &&
          inst->operand(0) == neg &&
          matchRootMatrixElementGEPFlexible(inst->operand(1), root, outer,
                                            loop.ivPhi, bound)) {
        if (store)
          return false;
        store = inst;
      }
    }
  }
  return load && neg && store;
}

bool matchChecksumInnerLoop(const analysis::NaturalLoop& natural,
                            const hir::CanonicalLoop& loop,
                            GlobalVar* root,
                            Value* row,
                            Value* bound,
                            Instruction*& innerAccPhiOut) {
  if (!canonicalLoopIsForwardLt(loop) || !isZeroInt(loop.initVal) ||
      !sameLoopBound(loop.boundVal, bound) ||
      !loopHasSingleExit(natural, loop.exit) || loop.body != loop.latch ||
      !loopHasNoCallsOrStores(natural)) {
    return false;
  }

  Instruction* load = nullptr;
  for (auto& instPtr : loop.body->instructions()) {
    Instruction* inst = instPtr.get();
    if (inst && inst->opcode() == Opcode::Load &&
        matchRootMatrixElementGEPFlexible(inst->operand(0), root, row,
                                          loop.ivPhi, bound)) {
      if (load)
        return false;
      load = inst;
    }
  }
  if (!load)
    return false;

  std::vector<Instruction*> phis;
  std::vector<Value*> inits;
  std::vector<Value*> nexts;
  if (!collectHeaderLoopPhis(loop, phis, inits, &nexts))
    return false;
  for (size_t i = 0; i < phis.size(); ++i) {
    Instruction* phi = phis[i];
    if (!phi || phi == loop.ivPhi || !phi->type().isI32())
      continue;
    auto* add = dynamic_cast<Instruction*>(nexts[i]);
    if (!add || add->opcode() != Opcode::Add || add->numOperands() != 2)
      continue;
    if ((add->operand(0) == phi && add->operand(1) == load) ||
        (add->operand(1) == phi && add->operand(0) == load)) {
      innerAccPhiOut = phi;
      return true;
    }
  }
  return false;
}

bool matchChecksumOuterLoop(const analysis::NaturalLoop& natural,
                            const hir::CanonicalLoop& loop,
                            const std::vector<analysis::NaturalLoop>& loops,
                            GlobalVar* root,
                            Value* bound,
                            Instruction*& outerAccPhiOut,
                            hir::CanonicalLoop& innerLoopOut,
                            BasicBlock*& checksumExitOut) {
  if (!canonicalLoopIsForwardLt(loop) || !isZeroInt(loop.initVal) ||
      !sameLoopBound(loop.boundVal, bound) ||
      !loopHasSingleExit(natural, loop.exit) ||
      !analysis::collectLeadingPhi(loop.exit).empty()) {
    return false;
  }
  BasicBlock* innerHeader = singleBrTarget(loop.body);
  const auto* innerNatural = loopForHeader(loops, innerHeader);
  BasicBlock* innerPreheader = innerNatural ? analysis::dedicatedPreheader(*innerNatural)
                                            : nullptr;
  hir::CanonicalLoop innerLoop;
  if (!innerNatural || innerPreheader != loop.body ||
      !hir::recognizeCanonicalLoop(*innerNatural, innerPreheader, innerLoop) ||
      innerLoop.exit != loop.latch) {
    return false;
  }

  Instruction* innerAccPhi = nullptr;
  if (!matchChecksumInnerLoop(*innerNatural, innerLoop, root, loop.ivPhi,
                              bound, innerAccPhi)) {
    return false;
  }

  std::vector<Instruction*> phis;
  std::vector<Value*> inits;
  std::vector<Value*> nexts;
  if (!collectHeaderLoopPhis(loop, phis, inits, &nexts))
    return false;
  for (size_t i = 0; i < phis.size(); ++i) {
    Instruction* phi = phis[i];
    if (!phi || phi == loop.ivPhi || !phi->type().isI32())
      continue;
    if (isZeroInt(inits[i]) && nexts[i] == innerAccPhi) {
      outerAccPhiOut = phi;
      innerLoopOut = innerLoop;
      checksumExitOut = loop.exit;
      return true;
    }
  }
  return false;
}

bool exitBlockCanBypassArrayStateAfterScalarSummary(BasicBlock* exit,
                                                    Value* scalarLiveOut,
                                                    Module* mod) {
  if (!exit || !scalarLiveOut || !mod)
    return false;
  Instruction* term = exit->terminator();
  if (!term || term->opcode() != Opcode::Br)
    return false;
  for (auto& instPtr : exit->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst || inst->opcode() == Opcode::Phi || inst->isTerminator())
      continue;
    for (Value* operand : inst->operands()) {
      if (operand && operand->type().isPtr())
        return false;
    }
    if (inst->opcode() == Opcode::Call) {
      if (mod->getFunction(inst->name()))
        return false;
      continue;
    }
    if (inst->opcode() == Opcode::Store || inst->opcode() == Opcode::Load ||
        inst->opcode() == Opcode::VLoad || inst->opcode() == Opcode::VStore ||
        inst->opcode() == Opcode::Ret ||
        inst->opcode() == Opcode::Alloca) {
      return false;
    }
  }
  return true;
}

bool matchProducerStoreAddress(Value* addr,
                               Instruction* ptrPhi,
                               GlobalVar* root,
                               const hir::CanonicalLoop& colLoop,
                               Value* row,
                               Value* bound) {
  if (ptrPhi && addr == ptrPhi)
    return true;
  return matchRootMatrixElementGEPFlexible(addr, root, row, colLoop.ivPhi,
                                           bound);
}

bool loopContainedIn(const analysis::NaturalLoop& inner,
                     const analysis::NaturalLoop& outer) {
  for (BasicBlock* bb : inner.blocks) {
    if (!bb || !outer.blocks.count(bb))
      return false;
  }
  return true;
}

bool matrixRowStrideBytes(Value* bound, int32_t& strideOut) {
  auto* boundConst = dynamic_cast<ConstInt*>(normalizedMatrixLoopBound(bound));
  if (!boundConst || boundConst->value() <= 0)
    return false;
  int64_t stride = static_cast<int64_t>(boundConst->value()) * 4;
  if (stride > std::numeric_limits<int32_t>::max())
    return false;
  strideOut = static_cast<int32_t>(stride);
  return true;
}

bool matchMatrixAddressWithLoop(Value* addr,
                                GlobalVar* root,
                                Value* row,
                                Value* col,
                                Value* bound,
                                const hir::CanonicalLoop* loop) {
  if (matchRootMatrixElementGEPFlexible(addr, root, row, col, bound))
    return true;
  auto* ptrPhi = dynamic_cast<Instruction*>(addr);
  if (!ptrPhi || !loop || !loop->ivPhi)
    return false;

  Value* init = nullptr;
  Value* initRow = row;
  Value* initCol = col;
  int32_t stepBytes = 0;
  if (row == loop->ivPhi) {
    if (!matrixRowStrideBytes(bound, stepBytes))
      return false;
    initRow = loop->initVal;
  } else if (col == loop->ivPhi) {
    stepBytes = 4;
    initCol = loop->initVal;
  } else {
    return false;
  }
  return matchSequentialPointerPhi(ptrPhi, loop->preheader, loop->latch,
                                   init, stepBytes) &&
         matchRootMatrixElementGEPFlexible(init, root, initRow, initCol,
                                           bound);
}

bool matchMatrixAddressWithAnyLoop(
    Value* addr,
    GlobalVar* root,
    Value* row,
    Value* col,
    Value* bound,
    const std::vector<hir::CanonicalLoop>& loops) {
  if (matchRootMatrixElementGEPFlexible(addr, root, row, col, bound))
    return true;
  for (const auto& loop : loops) {
    if (matchMatrixAddressWithLoop(addr, root, row, col, bound, &loop))
      return true;
  }
  return false;
}

bool matchLoadMatrixAddressWithAnyLoop(
    Instruction* load,
    GlobalVar* root,
    Value* row,
    Value* col,
    Value* bound,
    const std::vector<hir::CanonicalLoop>& loops) {
  return load && load->opcode() == Opcode::Load && load->numOperands() == 1 &&
         matchMatrixAddressWithAnyLoop(load->operand(0), root, row, col,
                                       bound, loops);
}

std::vector<hir::CanonicalLoop> collectContainedCanonicalLoops(
    const std::vector<analysis::NaturalLoop>& loops,
    const analysis::NaturalLoop& outer,
    BasicBlock* skipA,
    BasicBlock* skipB) {
  std::vector<hir::CanonicalLoop> result;
  for (const auto& natural : loops) {
    if (!natural.header || natural.header == skipA || natural.header == skipB ||
        !loopContainedIn(natural, outer)) {
      continue;
    }
    BasicBlock* preheader = analysis::dedicatedPreheader(natural);
    hir::CanonicalLoop loop;
    if (preheader && hir::recognizeCanonicalLoop(natural, preheader, loop) &&
        loop.ivPhi) {
      result.push_back(loop);
    }
  }
  return result;
}

Instruction* findProducerRowStore(const analysis::NaturalLoop& outerNatural,
                                  const hir::CanonicalLoop& outerLoop,
                                  const hir::CanonicalLoop& colLoop,
                                  GlobalVar* root,
                                  Value* bound) {
  Instruction* ptrPhi = nullptr;
  Value* ptrInit = nullptr;
  for (Instruction* phi : analysis::collectLeadingPhi(colLoop.header)) {
    if (matchSequentialPointerPhi(phi, colLoop.preheader, colLoop.latch,
                                  ptrInit) &&
        matchRootRowBaseGEPFlexible(ptrInit, root, outerLoop.ivPhi, bound)) {
      ptrPhi = phi;
      break;
    }
  }

  Instruction* store = nullptr;
  for (auto& instPtr : colLoop.latch->instructions()) {
    Instruction* inst = instPtr.get();
    if (!inst || inst->opcode() != Opcode::Store || inst->numOperands() != 2)
      continue;
    if (memoryRoot(inst->operand(1)) != root)
      continue;
    if (store)
      return nullptr;
    if (!inst->operand(0) || !inst->operand(0)->type().isI32() ||
        !matchProducerStoreAddress(inst->operand(1), ptrPhi, root, colLoop,
                                   outerLoop.ivPhi, bound)) {
      return nullptr;
    }
    store = inst;
  }
  if (!store)
    return nullptr;

  for (BasicBlock* bb : outerNatural.blocks) {
    if (!bb)
      return nullptr;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        return nullptr;
      if (inst->opcode() == Opcode::Call || inst->opcode() == Opcode::Ret ||
          inst->opcode() == Opcode::Alloca || inst->opcode() == Opcode::VLoad ||
          inst->opcode() == Opcode::VStore) {
        return nullptr;
      }
      if (inst->opcode() == Opcode::Load && inst->numOperands() == 1 &&
          memoryRoot(inst->operand(0)) == root) {
        return nullptr;
      }
      if (inst->opcode() == Opcode::Store && inst->numOperands() == 2 &&
          memoryRoot(inst->operand(1)) == root && inst != store) {
        return nullptr;
      }
    }
  }
  return store;
}

bool matchRowMinProducerBefore(
    const std::vector<analysis::NaturalLoop>& loops,
    const hir::CanonicalLoop& rowLoop,
    GlobalVar* root,
    Value* bound,
    Instruction* minPhi,
    RowBroadcastTransposeChecksumPlan::ProducerPlan& producer) {
  if (!rowLoop.preheader || !root || !bound || !minPhi)
    return false;
  Value* minInit = rowBroadcastPhiIncoming(minPhi, rowLoop.body);
  if (!minInit)
    return false;
  auto* minInitConst = dynamic_cast<ConstInt*>(minInit);
  if (!minInitConst ||
      minInitConst->value() != std::numeric_limits<int32_t>::max()) {
    return false;
  }

  for (const auto& outerNatural : loops) {
    BasicBlock* outerPreheader = analysis::dedicatedPreheader(outerNatural);
    hir::CanonicalLoop outerLoop;
    if (!outerPreheader ||
        !hir::recognizeCanonicalLoop(outerNatural, outerPreheader, outerLoop) ||
        outerLoop.exit != rowLoop.preheader ||
        !canonicalLoopIsForwardLt(outerLoop) ||
        !isZeroInt(outerLoop.initVal) ||
        !sameLoopBound(outerLoop.boundVal, bound) ||
        !loopHasSingleExit(outerNatural, outerLoop.exit)) {
      continue;
    }

    BasicBlock* colHeader = singleBrTarget(outerLoop.body);
    const auto* colNatural = loopForHeader(loops, colHeader);
    BasicBlock* colPreheader =
        colNatural ? analysis::dedicatedPreheader(*colNatural) : nullptr;
    hir::CanonicalLoop colLoop;
    if (!colNatural || colPreheader != outerLoop.body ||
        !hir::recognizeCanonicalLoop(*colNatural, colPreheader, colLoop) ||
        colLoop.exit != outerLoop.latch ||
        !canonicalLoopIsForwardLt(colLoop) || !isZeroInt(colLoop.initVal) ||
        !sameLoopBound(colLoop.boundVal, bound) ||
        !loopHasSingleExit(*colNatural, colLoop.exit)) {
      continue;
    }

    Instruction* store = findProducerRowStore(outerNatural, outerLoop, colLoop,
                                              root, bound);
    if (!store)
      continue;
    producer.valid = true;
    producer.outerNatural = outerNatural;
    producer.colNatural = *colNatural;
    producer.innerLoops =
        collectContainedCanonicalLoops(loops, outerNatural,
                                       outerLoop.header, colLoop.header);
    producer.outerLoop = outerLoop;
    producer.colLoop = colLoop;
    producer.store = store;
    producer.minInit = minInit;
    return true;
  }
  return false;
}

struct SymmetricScoreAccess {
  bool rowK = false;
  bool colK = false;
  bool kRow = false;
  bool kCol = false;
};

bool matchTransposeCopyBeforeProducer(
    const RowBroadcastTransposeChecksumPlan& plan,
    GlobalVar* sourceRoot,
    GlobalVar* transposeRoot) {
  if (!sourceRoot || !transposeRoot || sourceRoot == transposeRoot ||
      !plan.producer.outerLoop.preheader) {
    return false;
  }

  for (const auto& natural : plan.loops) {
    BasicBlock* preheader = analysis::dedicatedPreheader(natural);
    hir::CanonicalLoop outer;
    if (!preheader ||
        !hir::recognizeCanonicalLoop(natural, preheader, outer) ||
        outer.exit != plan.producer.outerLoop.preheader ||
        !canonicalLoopIsForwardLt(outer) || !isZeroInt(outer.initVal) ||
        !sameLoopBound(outer.boundVal, plan.bound) ||
        !loopHasSingleExit(natural, outer.exit)) {
      continue;
    }

    // 在外层循环内扫描转置拷贝：禁止 Call/Ret/Alloca/VLoad/VStore，
    // 写 sourceRoot 的 store 视为非法；写 transposeRoot 的 store 必须
    // 直接来自 load sourceRoot 且行列索引互换，匹配到至少一条才成立。
    std::vector<hir::CanonicalLoop> innerLoops =
        collectContainedCanonicalLoops(plan.loops, natural, outer.header,
                                       nullptr);
    int matchedStores = 0;
    bool bad = false;
    for (BasicBlock* bb : natural.blocks) {
      if (!bb) {
        bad = true;
        break;
      }
      for (auto& instPtr : bb->instructions()) {
        Instruction* inst = instPtr.get();
        if (!inst)
          continue;
        if (inst->opcode() == Opcode::Call || inst->opcode() == Opcode::Ret ||
            inst->opcode() == Opcode::Alloca || inst->opcode() == Opcode::VLoad ||
            inst->opcode() == Opcode::VStore) {
          bad = true;
          break;
        }
        if (inst->opcode() != Opcode::Store)
          continue;
        if (inst->numOperands() != 2 ||
            memoryRoot(inst->operand(1)) == sourceRoot) {
          bad = true;
          break;
        }
        if (memoryRoot(inst->operand(1)) != transposeRoot)
          continue;
        auto* load = dynamic_cast<Instruction*>(inst->operand(0));
        if (!load || load->opcode() != Opcode::Load || load->numOperands() != 1) {
          bad = true;
          break;
        }
        bool matched = false;
        for (const auto& inner : innerLoops) {
          if (!canonicalLoopIsForwardLt(inner) || !isZeroInt(inner.initVal) ||
              !sameLoopBound(inner.boundVal, plan.bound)) {
            continue;
          }
          if (matchMatrixAddressWithLoop(inst->operand(1), transposeRoot,
                                         outer.ivPhi, inner.ivPhi,
                                         plan.bound, &inner) &&
              matchMatrixAddressWithLoop(load->operand(0), sourceRoot,
                                         inner.ivPhi, outer.ivPhi,
                                         plan.bound, &inner)) {
            matched = true;
            break;
          }
        }
        if (!matched) {
          bad = true;
          break;
        }
        ++matchedStores;
      }
      if (bad)
        break;
    }
    if (!bad && matchedStores > 0)
      return true;
  }
  return false;
}

bool producerHasSymmetricMatrixScore(
    const RowBroadcastTransposeChecksumPlan& plan) {
  if (!plan.func || !plan.func->parent() || !plan.producer.valid ||
      !plan.producer.store || !plan.producer.outerLoop.ivPhi ||
      !plan.producer.colLoop.ivPhi || plan.producer.innerLoops.empty()) {
    return false;
  }

  int32_t ignoredStride = 0;
  if (!matrixRowStrideBytes(plan.bound, ignoredStride))
    return false;

  std::unordered_map<GlobalVar*, SymmetricScoreAccess> access;
  for (BasicBlock* bb : plan.producer.outerNatural.blocks) {
    if (!bb)
      return false;
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      if (inst->opcode() == Opcode::Call || inst->opcode() == Opcode::Ret ||
          inst->opcode() == Opcode::Alloca || inst->opcode() == Opcode::VLoad ||
          inst->opcode() == Opcode::VStore) {
        return false;
      }
      if (inst->opcode() != Opcode::Load || inst->numOperands() != 1)
        continue;

      for (auto& globalPtr : plan.func->parent()->globals()) {
        GlobalVar* root = globalPtr.get();
        if (!root || root == plan.root || root->elemTy() != Type::i32())
          continue;
        for (const auto& inner : plan.producer.innerLoops) {
          if (!inner.ivPhi)
            continue;
          if (matchLoadMatrixAddressWithAnyLoop(
                  inst, root, plan.producer.outerLoop.ivPhi, inner.ivPhi,
                  plan.bound, plan.producer.innerLoops)) {
            access[root].rowK = true;
          }
          if (matchLoadMatrixAddressWithAnyLoop(
                  inst, root, plan.producer.colLoop.ivPhi, inner.ivPhi,
                  plan.bound, plan.producer.innerLoops)) {
            access[root].colK = true;
          }
          if (matchLoadMatrixAddressWithAnyLoop(
                  inst, root, inner.ivPhi, plan.producer.outerLoop.ivPhi,
                  plan.bound, plan.producer.innerLoops)) {
            access[root].kRow = true;
          }
          if (matchLoadMatrixAddressWithAnyLoop(
                  inst, root, inner.ivPhi, plan.producer.colLoop.ivPhi,
                  plan.bound, plan.producer.innerLoops)) {
            access[root].kCol = true;
          }
        }
      }
    }
  }

  for (const auto& sourceEntry : access) {
    GlobalVar* sourceRoot = sourceEntry.first;
    const SymmetricScoreAccess& source = sourceEntry.second;
    if (stageFusionTraceEnabled()) {
      std::cerr << "[stage-fusion] symmetric-access root="
                << sourceRoot->name() << " row-k=" << source.rowK
                << " col-k=" << source.colK << " k-row=" << source.kRow
                << " k-col=" << source.kCol << "\n";
    }
    // The direct source must provide the producer row and k-by-column roles.
    // The other two symmetric roles may be direct source loads, or may appear
    // through a separately proved transpose object: transpose[k][col] is
    // source[col][k], while transpose[row][k] is source[k][row]. This keeps
    // the proof independent of whether transpose-view forwarding ran first.
    if (!source.rowK || !source.kCol)
      continue;
    if (source.colK && source.kRow)
      return true;
    for (const auto& transposeEntry : access) {
      GlobalVar* transposeRoot = transposeEntry.first;
      const SymmetricScoreAccess& transpose = transposeEntry.second;
      if (transposeRoot == sourceRoot ||
          !(source.colK || transpose.kCol) ||
          !(source.kRow || transpose.rowK))
        continue;
      const bool transposeMatched =
          matchTransposeCopyBeforeProducer(plan, sourceRoot, transposeRoot);
      if (stageFusionTraceEnabled()) {
        std::cerr << "[stage-fusion] symmetric-transpose source="
                  << sourceRoot->name() << " transpose="
                  << transposeRoot->name() << " matched="
                  << transposeMatched << "\n";
      }
      if (transposeMatched)
        return true;
    }
  }
  return false;
}

void retargetPhiIncomingPred(Instruction* phi,
                             BasicBlock* oldPred,
                             Value* newValue,
                             BasicBlock* newPred) {
  if (!phi || phi->opcode() != Opcode::Phi)
    return;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    if (dynamic_cast<BasicBlock*>(phi->operand(i + 1)) == oldPred) {
      if (newValue)
        phi->setOperand(i, newValue);
      if (newPred)
        phi->setOperand(i + 1, newPred);
    }
  }
}

void retargetHeaderIncomingPred(BasicBlock* header,
                                BasicBlock* oldPred,
                                BasicBlock* newPred) {
  for (Instruction* phi : analysis::collectLeadingPhi(header))
    retargetPhiIncomingPred(phi, oldPred, nullptr, newPred);
}

void clearBlock(BasicBlock* bb) {
  if (!bb)
    return;
  while (!bb->instructions().empty())
    bb->remove(bb->instructions().back().get());
}

bool collectLoopBlocksForRemoval(
    const RowBroadcastTransposeChecksumPlan& plan,
    std::initializer_list<BasicBlock*> headers,
    std::vector<BasicBlock*>& blocksOut) {
  std::unordered_set<BasicBlock*> unique;
  for (BasicBlock* header : headers) {
    const auto* natural = loopForHeader(plan.loops, header);
    if (!natural)
      return false;
    unique.insert(natural->blocks.begin(), natural->blocks.end());
  }
  blocksOut.assign(unique.begin(), unique.end());
  std::sort(blocksOut.begin(), blocksOut.end(),
            [](BasicBlock* lhs, BasicBlock* rhs) {
              return lhs->id() < rhs->id();
            });
  return true;
}

void addBlockForRemoval(std::vector<BasicBlock*>& blocks, BasicBlock* block) {
  if (!block || std::find(blocks.begin(), blocks.end(), block) != blocks.end())
    return;
  blocks.push_back(block);
  std::sort(blocks.begin(), blocks.end(), [](BasicBlock* lhs, BasicBlock* rhs) {
    return lhs->id() < rhs->id();
  });
}

void removeProvedLoopBlocks(Function* function,
                            const std::vector<BasicBlock*>& blocks) {
  if (!function)
    return;
  if (stageFusionTraceEnabled()) {
    std::unordered_set<BasicBlock*> removed(blocks.begin(), blocks.end());
    for (const auto& blockPtr : function->blocks()) {
      BasicBlock* block = blockPtr.get();
      if (!block || removed.count(block))
        continue;
      Instruction* term = block->terminator();
      if (!term)
        continue;
      for (Value* operand : term->operands()) {
        auto* target = dynamic_cast<BasicBlock*>(operand);
        if (target && removed.count(target)) {
          std::cerr << "[stage-fusion] retained-edge source=" << block->name()
                    << " target=" << target->name() << "\n";
        }
      }
    }
  }
  // 先整体断开 use-def，避免删除 Phi/cycle 时依赖遍历顺序。
  for (BasicBlock* block : blocks)
    for (auto& instPtr : block->instructions())
      if (instPtr)
        instPtr->dropAllOperands();
  for (BasicBlock* block : blocks)
    function->removeBlock(block);
}

void appendRowMinUpdate(BasicBlock* bb,
                        Instruction* pos,
                        Module* mod,
                        GlobalVar* root,
                        Value* rowIndex,
                        Value* candidate,
                        const std::string& prefix) {
  Instruction* offset = insertModRecInstBefore(
      bb, pos, Opcode::Shl, Type::i32(), prefix + ".off",
      {rowIndex, mod->getConstInt(2)});
  Instruction* addr = insertModRecInstBefore(
      bb, pos, Opcode::GEP, Type::ptr(), prefix + ".addr",
      {root, offset});
  Instruction* old = insertModRecInstBefore(
      bb, pos, Opcode::Load, Type::i32(), prefix + ".old", {addr});
  Instruction* cmp = insertModRecICmpBefore(
      bb, pos, "lt", prefix + ".cmp", candidate, old);
  Instruction* mask = insertModRecInstBefore(
      bb, pos, Opcode::Sub, Type::i32(), prefix + ".mask",
      {mod->getConstInt(0), cmp});
  Instruction* diff = insertModRecInstBefore(
      bb, pos, Opcode::Xor, Type::i32(), prefix + ".diff", {old, candidate});
  Instruction* masked = insertModRecInstBefore(
      bb, pos, Opcode::And, Type::i32(), prefix + ".masked", {diff, mask});
  Instruction* next = insertModRecInstBefore(
      bb, pos, Opcode::Xor, Type::i32(), prefix + ".next", {old, masked});
  insertModRecInstBefore(bb, pos, Opcode::Store, Type::voidTy(),
                         prefix + ".store", {next, addr});
}

bool materializeSymmetricProducerRowMinChecksumFusion(
    const RowBroadcastTransposeChecksumPlan& plan) {
  if (!producerHasSymmetricMatrixScore(plan) || !plan.func ||
      !plan.func->parent() || !plan.producer.store || !plan.rowLoop.header ||
      !plan.rowLoop.preheader || !plan.rowLoop.ivPhi || !plan.rowLoop.body ||
      !plan.rowLoop.latch || !plan.rowLoop.exit || !plan.checksumExit ||
      !plan.checksumAccPhi || !plan.producer.outerLoop.preheader ||
      !plan.producer.outerLoop.header || !plan.producer.outerLoop.ivPhi ||
      !plan.producer.colLoop.ivPhi || !plan.producer.colLoop.latch ||
      !plan.producer.colLoop.preheader || !plan.checksumOuterLoop.header) {
    return false;
  }

  std::vector<BasicBlock*> skipped;
  if (!collectLoopBlocksForRemoval(
          plan,
          {plan.minLoop.header, plan.broadcastLoop.header,
           plan.transposeOuterLoop.header, plan.checksumOuterLoop.header},
          skipped)) {
    return false;
  }
  addBlockForRemoval(skipped, plan.minLoop.exit);
  addBlockForRemoval(skipped, plan.transposeOuterLoop.exit);
  // The broadcast loop exits through the original outer-row latch. The
  // fused checksum reuses rowLoop.header with a newly materialized sum body
  // as its only latch, so retaining the old latch would add a CFG predecessor
  // with no matching Phi incoming.
  addBlockForRemoval(skipped, plan.rowLoop.latch);

  Module* mod = plan.func->parent();
  Value* produced = plan.producer.store->operand(0);
  BasicBlock* producerPreheader = plan.producer.outerLoop.preheader;

  Instruction* producerPreheaderTerm = producerPreheader->terminator();
  if (!producerPreheaderTerm ||
      producerPreheaderTerm->opcode() != Opcode::Br ||
      producerPreheaderTerm->numOperands() != 1 ||
      producerPreheaderTerm->operand(0) != plan.producer.outerLoop.header) {
    return false;
  }

  BasicBlock* producerLatch = plan.producer.colLoop.latch;
  Instruction* producerLatchTerm =
      producerLatch ? producerLatch->terminator() : nullptr;
  BasicBlock* storeBlock = plan.producer.store->parent();
  BasicBlock* sumHeader = plan.rowLoop.header;
  BasicBlock* sumBody = plan.rowLoop.body;
  BasicBlock* oldRowLatch = plan.rowLoop.latch;
  Instruction* firstNonPhi = firstNonPhiInstruction(sumHeader);
  Instruction* rowExitTerm = plan.rowLoop.exit->terminator();
  if (!producerLatchTerm || producerLatchTerm->opcode() != Opcode::Br ||
      !storeBlock || !firstNonPhi || !rowExitTerm ||
      rowExitTerm->opcode() != Opcode::Br) {
    return false;
  }

  BasicBlock* initHeader = plan.func->newBlock("matrix.rowmin.init.header");
  BasicBlock* initBody = plan.func->newBlock("matrix.rowmin.init.body");
  BasicBlock* initExit = plan.func->newBlock("matrix.rowmin.init.exit");
  producerPreheaderTerm->setOperand(0, initHeader);

  auto* initIdx = new Instruction(Opcode::Phi, Type::i32(),
                                  "matrix.rowmin.init.iv");
  initIdx->addOperand(mod->getConstInt(0));
  initIdx->addOperand(producerPreheader);
  initHeader->pushBack(initIdx);
  auto* initCmp = makeFusionInst(Opcode::ICmp, Type::i32(),
                                 "lt_matrix.rowmin.init.cmp",
                                 {initIdx, plan.bound});
  initHeader->pushBack(initCmp);
  auto* initCond = new Instruction(Opcode::CondBr, Type::voidTy());
  initCond->addOperand(initCmp);
  initCond->addOperand(initBody);
  initCond->addOperand(initExit);
  initHeader->pushBack(initCond);

  auto* initOff = makeFusionInst(Opcode::Shl, Type::i32(),
                                 "matrix.rowmin.init.off",
                                 {initIdx, mod->getConstInt(2)});
  initBody->pushBack(initOff);
  auto* initAddr = makeFusionInst(Opcode::GEP, Type::ptr(),
                                  "matrix.rowmin.init.addr",
                                  {plan.root, initOff});
  initBody->pushBack(initAddr);
  auto* initStore = makeFusionInst(Opcode::Store, Type::voidTy(),
                                   "matrix.rowmin.init.store",
                                   {plan.producer.minInit, initAddr});
  initBody->pushBack(initStore);
  auto* initNext = makeFusionInst(Opcode::Add, Type::i32(),
                                  "matrix.rowmin.init.next",
                                  {initIdx, mod->getConstInt(1)});
  initBody->pushBack(initNext);
  auto* initBack = new Instruction(Opcode::Br, Type::voidTy());
  initBack->addOperand(initHeader);
  initBody->pushBack(initBack);
  initIdx->addOperand(initNext);
  initIdx->addOperand(initBody);

  auto* initExitBr = new Instruction(Opcode::Br, Type::voidTy());
  initExitBr->addOperand(plan.producer.outerLoop.header);
  initExit->pushBack(initExitBr);
  retargetHeaderIncomingPred(plan.producer.outerLoop.header,
                             producerPreheader, initExit);

  retargetPhiIncomingPred(plan.producer.colLoop.ivPhi,
                          plan.producer.colLoop.preheader,
                          plan.producer.outerLoop.ivPhi,
                          nullptr);

  appendRowMinUpdate(producerLatch, producerLatchTerm, mod, plan.root,
                     plan.producer.outerLoop.ivPhi, produced,
                     "matrix.rowmin.row");
  appendRowMinUpdate(producerLatch, producerLatchTerm, mod, plan.root,
                     plan.producer.colLoop.ivPhi, produced,
                     "matrix.rowmin.col");
  storeBlock->remove(plan.producer.store);

  auto* sumPhi = new Instruction(Opcode::Phi, Type::i32(),
                                 "matrix.rowmin.sum");
  sumPhi->addOperand(mod->getConstInt(0));
  sumPhi->addOperand(plan.rowLoop.preheader);
  sumHeader->insertBefore(firstNonPhi, sumPhi);

  clearBlock(sumBody);
  auto* sumOff = makeFusionInst(Opcode::Shl, Type::i32(),
                                "matrix.rowmin.sum.off",
                                {plan.rowLoop.ivPhi, mod->getConstInt(2)});
  sumBody->pushBack(sumOff);
  auto* sumAddr = makeFusionInst(Opcode::GEP, Type::ptr(),
                                 "matrix.rowmin.sum.addr",
                                 {plan.root, sumOff});
  sumBody->pushBack(sumAddr);
  auto* rowMin = makeFusionInst(Opcode::Load, Type::i32(),
                                "matrix.rowmin.sum.load", {sumAddr});
  sumBody->pushBack(rowMin);
  auto* sumNext = makeFusionInst(Opcode::Add, Type::i32(),
                                 "matrix.rowmin.sum.next",
                                 {sumPhi, rowMin});
  sumBody->pushBack(sumNext);
  auto* rowNext = makeFusionInst(Opcode::Add, Type::i32(),
                                 "matrix.rowmin.row.next",
                                 {plan.rowLoop.ivPhi, mod->getConstInt(1)});
  sumBody->pushBack(rowNext);
  auto* sumBack = new Instruction(Opcode::Br, Type::voidTy());
  sumBack->addOperand(sumHeader);
  sumBody->pushBack(sumBack);

  retargetPhiIncomingPred(plan.rowLoop.ivPhi, oldRowLatch, rowNext, sumBody);
  sumPhi->addOperand(sumNext);
  sumPhi->addOperand(sumBody);

  Instruction* fusedChecksum = insertModRecInstBefore(
      plan.rowLoop.exit, rowExitTerm, Opcode::Sub, Type::i32(),
      "matrix.rowmin.fused.checksum", {mod->getConstInt(0), sumPhi});
  replaceUsesInBlock(plan.checksumExit, plan.checksumAccPhi, fusedChecksum,
                     nullptr, nullptr);
  rowExitTerm->setOperand(0, plan.checksumExit);

  removeProvedLoopBlocks(plan.func, skipped);
  plan.func->recomputeCFG();
  return true;
}

bool materializeProducerRowMinChecksumFusion(
    const RowBroadcastTransposeChecksumPlan& plan) {
  if (!plan.producer.valid || !plan.func || !plan.func->parent() ||
      !plan.checksumAccPhi || !plan.checksumExit || !plan.producer.store ||
      !plan.producer.minInit || !plan.producer.outerLoop.header ||
      !plan.producer.outerLoop.latch || !plan.producer.outerLoop.exit ||
      !plan.producer.colLoop.header || !plan.producer.colLoop.latch) {
    return false;
  }
  std::vector<BasicBlock*> skipped;
  if (!collectLoopBlocksForRemoval(
          plan,
          {plan.rowLoop.header, plan.transposeOuterLoop.header,
           plan.checksumOuterLoop.header},
          skipped)) {
    return false;
  }
  addBlockForRemoval(skipped, plan.rowLoop.exit);
  addBlockForRemoval(skipped, plan.transposeOuterLoop.exit);
  Module* mod = plan.func->parent();
  const auto& outer = plan.producer.outerLoop;
  const auto& col = plan.producer.colLoop;

  Instruction* outerFirst = firstNonPhiInstruction(outer.header);
  Instruction* colFirst = firstNonPhiInstruction(col.header);
  if (!outerFirst || !colFirst)
    return false;

  auto* totalPhi =
      new Instruction(Opcode::Phi, Type::i32(), "producer.rowmin.sum");
  totalPhi->addOperand(mod->getConstInt(0));
  totalPhi->addOperand(outer.preheader);
  outer.header->insertBefore(outerFirst, totalPhi);

  auto* rowMinPhi =
      new Instruction(Opcode::Phi, Type::i32(), "producer.rowmin");
  rowMinPhi->addOperand(plan.producer.minInit);
  rowMinPhi->addOperand(col.preheader);
  col.header->insertBefore(colFirst, rowMinPhi);

  Instruction* colLatchTerm = col.latch->terminator();
  if (!colLatchTerm || colLatchTerm->opcode() != Opcode::Br)
    return false;
  Value* produced = plan.producer.store->operand(0);
  Instruction* cmp = insertModRecICmpBefore(
      col.latch, colLatchTerm, "lt", "producer.rowmin.cmp",
      produced, rowMinPhi);
  Instruction* mask = insertModRecInstBefore(
      col.latch, colLatchTerm, Opcode::Sub, Type::i32(),
      "producer.rowmin.mask", {mod->getConstInt(0), cmp});
  Instruction* diff = insertModRecInstBefore(
      col.latch, colLatchTerm, Opcode::Xor, Type::i32(),
      "producer.rowmin.diff", {rowMinPhi, produced});
  Instruction* masked = insertModRecInstBefore(
      col.latch, colLatchTerm, Opcode::And, Type::i32(),
      "producer.rowmin.masked", {diff, mask});
  Instruction* rowMinNext = insertModRecInstBefore(
      col.latch, colLatchTerm, Opcode::Xor, Type::i32(),
      "producer.rowmin.next", {rowMinPhi, masked});
  rowMinPhi->addOperand(rowMinNext);
  rowMinPhi->addOperand(col.latch);

  BasicBlock* storeBlock = plan.producer.store->parent();
  if (!storeBlock)
    return false;
  storeBlock->remove(plan.producer.store);

  Instruction* outerLatchTerm = outer.latch->terminator();
  if (!outerLatchTerm || outerLatchTerm->opcode() != Opcode::Br)
    return false;
  Instruction* totalNext = insertModRecInstBefore(
      outer.latch, outerLatchTerm, Opcode::Add, Type::i32(),
      "producer.rowmin.sum.next", {totalPhi, rowMinPhi});
  totalPhi->addOperand(totalNext);
  totalPhi->addOperand(outer.latch);

  Instruction* exitTerm = outer.exit->terminator();
  if (!exitTerm || exitTerm->opcode() != Opcode::Br ||
      exitTerm->numOperands() != 1 || exitTerm->operand(0) != plan.rowLoop.header) {
    return false;
  }
  Instruction* fusedChecksum = insertModRecInstBefore(
      outer.exit, exitTerm, Opcode::Sub, Type::i32(),
      "producer.rowmin.fused.checksum", {mod->getConstInt(0), totalPhi});

  replaceUsesInBlock(plan.checksumExit, plan.checksumAccPhi, fusedChecksum,
                     nullptr, nullptr);

  exitTerm->setOperand(0, plan.checksumExit);
  removeProvedLoopBlocks(plan.func, skipped);
  plan.func->recomputeCFG();
  return true;
}

bool matchRowBroadcastTransposeChecksum(
    Function* func,
    const std::vector<analysis::NaturalLoop>& loops,
    const analysis::NaturalLoop& rowNatural,
    RowBroadcastTransposeChecksumPlan& plan) {
  if (!func || !func->parent())
    return rejectStageFusion(rowNatural, "row-broadcast-missing-func");
  BasicBlock* rowPreheader = analysis::dedicatedPreheader(rowNatural);
  hir::CanonicalLoop rowLoop;
  if (!rowPreheader ||
      !hir::recognizeCanonicalLoop(rowNatural, rowPreheader, rowLoop) ||
      !canonicalLoopIsForwardLt(rowLoop) || !isZeroInt(rowLoop.initVal) ||
      !loopHasSingleExit(rowNatural, rowLoop.exit)) {
    return rejectStageFusion(rowNatural, "row-broadcast-row-loop");
  }
  Value* bound = normalizedMatrixLoopBound(rowLoop.boundVal);
  if (!bound)
    return rejectStageFusion(rowNatural, "row-broadcast-bound");

  BasicBlock* minHeader = singleBrTarget(rowLoop.body);
  const auto* minNatural = loopForHeader(loops, minHeader);
  BasicBlock* minPreheader = minNatural ? analysis::dedicatedPreheader(*minNatural)
                                        : nullptr;
  hir::CanonicalLoop minLoop;
  if (!minNatural || minPreheader != rowLoop.body ||
      !hir::recognizeCanonicalLoop(*minNatural, minPreheader, minLoop)) {
    return rejectStageFusion(rowNatural, "row-broadcast-min-shape");
  }

  GlobalVar* root = nullptr;
  Instruction* minPhi = nullptr;
  for (auto& globalPtr : func->parent()->globals()) {
    auto* candidateRoot = globalPtr.get();
    if (!candidateRoot || candidateRoot->elemTy() != Type::i32())
      continue;
    Instruction* candidateMinPhi = nullptr;
    if (matchRowMinLoop(*minNatural, minLoop, candidateRoot, rowLoop.ivPhi,
                        bound, candidateMinPhi)) {
      root = candidateRoot;
      minPhi = candidateMinPhi;
      break;
    }
  }
  if (!root || !minPhi)
    return rejectStageFusion(rowNatural, "row-broadcast-min-loop");

  BasicBlock* broadcastHeader = singleBrTarget(minLoop.exit);
  const auto* broadcastNatural = loopForHeader(loops, broadcastHeader);
  BasicBlock* broadcastPreheader =
      broadcastNatural ? analysis::dedicatedPreheader(*broadcastNatural) : nullptr;
  hir::CanonicalLoop broadcastLoop;
  if (!broadcastNatural || broadcastPreheader != minLoop.exit ||
      !hir::recognizeCanonicalLoop(*broadcastNatural, broadcastPreheader,
                                   broadcastLoop) ||
      broadcastLoop.exit != rowLoop.latch ||
      !matchRowBroadcastLoop(*broadcastNatural, broadcastLoop, root,
                             rowLoop.ivPhi, bound, minPhi)) {
    return rejectStageFusion(rowNatural, "row-broadcast-store-loop");
  }

  BasicBlock* transposeOuterHeader = singleBrTarget(rowLoop.exit);
  const auto* transposeOuterNatural = loopForHeader(loops, transposeOuterHeader);
  BasicBlock* transposeOuterPreheader =
      transposeOuterNatural ? analysis::dedicatedPreheader(*transposeOuterNatural)
                            : nullptr;
  hir::CanonicalLoop transposeOuterLoop;
  if (!transposeOuterNatural || transposeOuterPreheader != rowLoop.exit ||
      !hir::recognizeCanonicalLoop(*transposeOuterNatural,
                                   transposeOuterPreheader,
                                   transposeOuterLoop) ||
      !canonicalLoopIsForwardLt(transposeOuterLoop) ||
      !isZeroInt(transposeOuterLoop.initVal) ||
      !sameLoopBound(transposeOuterLoop.boundVal, bound) ||
      !loopHasSingleExit(*transposeOuterNatural, transposeOuterLoop.exit)) {
    return rejectStageFusion(rowNatural, "row-broadcast-transpose-outer");
  }

  BasicBlock* transposeInnerHeader = singleBrTarget(transposeOuterLoop.body);
  const auto* transposeInnerNatural = loopForHeader(loops, transposeInnerHeader);
  BasicBlock* transposeInnerPreheader =
      transposeInnerNatural ? analysis::dedicatedPreheader(*transposeInnerNatural)
                            : nullptr;
  hir::CanonicalLoop transposeInnerLoop;
  if (!transposeInnerNatural || transposeInnerPreheader != transposeOuterLoop.body ||
      !hir::recognizeCanonicalLoop(*transposeInnerNatural,
                                   transposeInnerPreheader,
                                   transposeInnerLoop) ||
      transposeInnerLoop.exit != transposeOuterLoop.latch ||
      !matchTransposeNegateInnerLoop(*transposeInnerNatural,
                                     transposeInnerLoop, root,
                                     transposeOuterLoop.ivPhi, bound)) {
    return rejectStageFusion(rowNatural, "row-broadcast-transpose-inner");
  }

  BasicBlock* checksumOuterHeader = singleBrTarget(transposeOuterLoop.exit);
  const auto* checksumOuterNatural = loopForHeader(loops, checksumOuterHeader);
  BasicBlock* checksumOuterPreheader =
      checksumOuterNatural ? analysis::dedicatedPreheader(*checksumOuterNatural)
                           : nullptr;
  hir::CanonicalLoop checksumOuterLoop;
  if (!checksumOuterNatural ||
      checksumOuterPreheader != transposeOuterLoop.exit ||
      !hir::recognizeCanonicalLoop(*checksumOuterNatural,
                                   checksumOuterPreheader,
                                   checksumOuterLoop)) {
    return rejectStageFusion(rowNatural, "row-broadcast-checksum-outer");
  }

  Instruction* checksumAccPhi = nullptr;
  hir::CanonicalLoop checksumInnerLoop;
  BasicBlock* checksumExit = nullptr;
  if (!matchChecksumOuterLoop(*checksumOuterNatural, checksumOuterLoop, loops,
                              root, bound, checksumAccPhi, checksumInnerLoop,
                              checksumExit) ||
      !exitBlockCanBypassArrayStateAfterScalarSummary(
          checksumExit, checksumAccPhi, func->parent())) {
    return rejectStageFusion(rowNatural, "row-broadcast-checksum");
  }
  if (!skippedRegionHasNoUnmodeledLiveOut(
          {&rowNatural, transposeOuterNatural, checksumOuterNatural},
          {{checksumAccPhi, checksumExit}})) {
    return rejectStageFusion(rowNatural, "row-broadcast-skipped-live-out");
  }

  plan.func = func;
  plan.root = root;
  plan.bound = bound;
  plan.rowLoop = rowLoop;
  plan.minLoop = minLoop;
  plan.broadcastLoop = broadcastLoop;
  plan.transposeOuterLoop = transposeOuterLoop;
  plan.transposeInnerLoop = transposeInnerLoop;
  plan.checksumOuterLoop = checksumOuterLoop;
  plan.checksumInnerLoop = checksumInnerLoop;
  plan.minPhi = minPhi;
  plan.checksumAccPhi = checksumAccPhi;
  plan.checksumExit = checksumExit;
  plan.loops = loops;
  (void)matchRowMinProducerBefore(loops, rowLoop, root, bound, minPhi,
                                  plan.producer);
  return true;
}

bool materializeRowBroadcastTransposeChecksumFusion(
    const RowBroadcastTransposeChecksumPlan& plan) {
  if (materializeSymmetricProducerRowMinChecksumFusion(plan)) {
    if (stageFusionTraceEnabled())
      std::cerr << "[stage-fusion] materialize=symmetric-producer-rowmin\n";
    return true;
  }

  if (sycResearchSwitchEnabled("SYC_ENABLE_COREIR_PRODUCER_ROWMIN_FUSION") &&
      materializeProducerRowMinChecksumFusion(plan)) {
    if (stageFusionTraceEnabled())
      std::cerr << "[stage-fusion] materialize=producer-rowmin\n";
    return true;
  }

  Function* func = plan.func;
  if (!func || !func->parent() || !plan.rowLoop.header ||
      !plan.rowLoop.latch || !plan.rowLoop.exit || !plan.minPhi ||
      !plan.checksumAccPhi || !plan.checksumExit) {
    return false;
  }
  std::vector<BasicBlock*> skipped;
  if (!collectLoopBlocksForRemoval(
          plan,
          {plan.broadcastLoop.header, plan.transposeOuterLoop.header,
           plan.checksumOuterLoop.header},
          skipped)) {
    return false;
  }
  addBlockForRemoval(skipped, plan.transposeOuterLoop.exit);
  Module* mod = func->parent();
  Instruction* firstNonPhi = firstNonPhiInstruction(plan.rowLoop.header);
  if (!firstNonPhi)
    return false;
  auto* rowSumPhi =
      new Instruction(Opcode::Phi, Type::i32(), "row.broadcast.min.sum");
  rowSumPhi->addOperand(mod->getConstInt(0));
  rowSumPhi->addOperand(plan.rowLoop.preheader);
  plan.rowLoop.header->insertBefore(firstNonPhi, rowSumPhi);

  Instruction* rowLatchTerm = plan.rowLoop.latch->terminator();
  if (!rowLatchTerm || rowLatchTerm->opcode() != Opcode::Br)
    return false;
  Instruction* rowSumNext = insertModRecInstBefore(
      plan.rowLoop.latch, rowLatchTerm, Opcode::Add, Type::i32(),
      "row.broadcast.min.sum.next", {rowSumPhi, plan.minPhi});
  rowSumPhi->addOperand(rowSumNext);
  rowSumPhi->addOperand(plan.rowLoop.latch);

  Instruction* minExitTerm = plan.minLoop.exit->terminator();
  if (!minExitTerm || minExitTerm->opcode() != Opcode::Br ||
      minExitTerm->numOperands() != 1 ||
      minExitTerm->operand(0) != plan.broadcastLoop.header) {
    return false;
  }
  minExitTerm->setOperand(0, plan.rowLoop.latch);

  Instruction* exitTerm = plan.rowLoop.exit->terminator();
  if (!exitTerm || exitTerm->opcode() != Opcode::Br || exitTerm->numOperands() != 1)
    return false;
  Instruction* fusedChecksum = insertModRecInstBefore(
      plan.rowLoop.exit, exitTerm, Opcode::Sub, Type::i32(),
      "row.broadcast.fused.checksum", {mod->getConstInt(0), rowSumPhi});

  replaceUsesInBlock(plan.checksumExit, plan.checksumAccPhi, fusedChecksum,
                     nullptr, nullptr);

  plan.rowLoop.exit->remove(exitTerm);
  auto* br = new Instruction(Opcode::Br, Type::voidTy());
  br->addOperand(plan.checksumExit);
  plan.rowLoop.exit->pushBack(br);

  // 被旁路的广播、转置和 checksum 循环必须从函数所有权中一并删除。
  // 仅改入口边会留下不可达但 Phi predecessor 已失配的 CFG；事务 verifier
  // 会正确拒绝那种半提交状态。matcher 已在上面证明这些区域没有未建模
  // live-out，这里先整体断开 use-def，再按稳定顺序释放基本块。
  removeProvedLoopBlocks(func, skipped);
  func->recomputeCFG();
  if (stageFusionTraceEnabled())
    std::cerr << "[stage-fusion] materialize=row-broadcast-summary\n";
  return true;
}

bool applyRowBroadcastTransposeChecksumFusion(
    Function* func, analysis::AnalysisManager& analysisManager) {
  if (!func || !func->entryBlock())
    return false;
  analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
  if (cache.instructionCount() > 20000)
    return false;
  auto loops = cache.loops();
  std::sort(loops.begin(), loops.end(),
            [](const analysis::NaturalLoop& lhs,
               const analysis::NaturalLoop& rhs) {
              if (lhs.header && rhs.header)
                return lhs.header->id() < rhs.header->id();
              return lhs.header < rhs.header;
            });
  for (const auto& loop : loops) {
    RowBroadcastTransposeChecksumPlan plan;
    if (!matchRowBroadcastTransposeChecksum(func, loops, loop, plan))
      continue;
    if (materializeRowBroadcastTransposeChecksumFusion(plan)) {
      if (stageFusionTraceEnabled()) {
        std::cerr << "[stage-fusion] row-broadcast func=" << func->name()
                  << " root=" << plan.root->name()
                  << " row=" << plan.rowLoop.header->name() << "\n";
      }
      return true;
    }
  }
  return false;
}

bool applyRowBroadcastTransposeChecksumFusion(
    Module& mod, analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func)
      continue;
    if (applyRowBroadcastTransposeChecksumFusion(func, analysisManager)) {
      changed = true;
      analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
    }
  }
  return changed;
}


} // namespace core_ir
