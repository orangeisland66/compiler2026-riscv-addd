#include "research_switch.h"
// 文件职责：提供规范化循环识别与 phi 提取的共享实现，供多个 loop transform 复用。
#include "canonical_loop.h"

#include "../analysis/loop_info.h"

#include <cstdio>
#include <cstdlib>
#include <unordered_set>

namespace hir {

using core_ir::BasicBlock;
using core_ir::ConstInt;
using core_ir::Instruction;
using core_ir::Opcode;
using core_ir::Value;

std::string canonicalLoopCmpCond(Instruction* inst) {
  if (!inst)
    return "";
  auto name = inst->name();
  auto pos = name.find('_');
  return pos == std::string::npos ? name : name.substr(0, pos);
}

bool canonicalLoopTraceEnabled() {
  return sycEnvironmentValue("CANONICAL_LOOP_TRACE") != nullptr;
}

bool rejectCanonicalLoop(const analysis::NaturalLoop& loop,
                         const char* reason) {
  if (canonicalLoopTraceEnabled()) {
    std::fprintf(stderr, "[canonical-loop] reject header=%s reason=%s\n",
                 loop.header ? loop.header->name().c_str() : "<null>",
                 reason ? reason : "-");
  }
  return false;
}

bool recognizeCanonicalLoop(const analysis::NaturalLoop& loop,
                            BasicBlock* preheader, CanonicalLoop& out) {
  if (!loop.header || !preheader)
    return rejectCanonicalLoop(loop, "missing-header-or-preheader");

  std::vector<BasicBlock*> latches;
  std::unordered_set<int> seenLatchIds;
  for (auto* candidate : loop.latches) {
    if (!candidate || !seenLatchIds.insert(candidate->id()).second)
      continue;
    latches.push_back(candidate);
  }
  // 循环收集阶段可能因为等价 backedge 重复记录同一个 latch。
  // canonical 识别只关心物理 latch 是否唯一，因此先做去重；若去重后仍有
  // 多个 latch，继续保守拒绝，避免破坏真正多回边循环的语义。
  if (latches.size() != 1)
    return rejectCanonicalLoop(loop, "multi-latch");

  BasicBlock* latch = latches.front();
  BasicBlock* header = loop.header;
  Instruction* term = header->terminator();
  if (!term || term->opcode() != Opcode::CondBr)
    return rejectCanonicalLoop(loop, "header-not-condbr");
  auto* cond = dynamic_cast<Instruction*>(term->operand(0));
  if (!cond || cond->opcode() != Opcode::ICmp)
    return rejectCanonicalLoop(loop, "cond-not-icmp");
  auto* tTrue = dynamic_cast<BasicBlock*>(term->operand(1));
  auto* tFalse = dynamic_cast<BasicBlock*>(term->operand(2));
  if (!tTrue || !tFalse)
    return rejectCanonicalLoop(loop, "bad-cond-target");

  BasicBlock* body = loop.blocks.count(tTrue) ? tTrue : tFalse;
  BasicBlock* exit = loop.blocks.count(tTrue) ? tFalse : tTrue;
  if (!loop.blocks.count(body) || loop.blocks.count(exit))
    return rejectCanonicalLoop(loop, "bad-body-exit");

  Value* lhs = cond->operand(0);
  Value* rhs = cond->operand(1);
  auto* phi = dynamic_cast<Instruction*>(lhs);
  bool ivIsLhs = phi && phi->opcode() == Opcode::Phi && phi->parent() == header;
  if (!ivIsLhs) {
    phi = dynamic_cast<Instruction*>(rhs);
    if (!phi || phi->opcode() != Opcode::Phi || phi->parent() != header)
      return rejectCanonicalLoop(loop, "cmp-has-no-header-phi");
  }
  Value* bound = ivIsLhs ? rhs : lhs;
  if (phi->numOperands() != 4)
    return rejectCanonicalLoop(loop, "iv-phi-arity");

  Value* incA = phi->operand(0);
  auto* incABB = dynamic_cast<BasicBlock*>(phi->operand(1));
  Value* incB = phi->operand(2);
  auto* incBBB = dynamic_cast<BasicBlock*>(phi->operand(3));
  Value* init = nullptr;
  Value* nextV = nullptr;
  if (incABB == preheader && incBBB == latch) {
    init = incA;
    nextV = incB;
  } else if (incABB == latch && incBBB == preheader) {
    init = incB;
    nextV = incA;
  } else {
    return rejectCanonicalLoop(loop, "iv-phi-preds");
  }

  auto* next = dynamic_cast<Instruction*>(nextV);
  if (!next || next->opcode() != Opcode::Add || next->parent() != latch)
    return rejectCanonicalLoop(loop, "next-not-latch-add");
  auto* nextL = dynamic_cast<ConstInt*>(next->operand(0));
  auto* nextR = dynamic_cast<ConstInt*>(next->operand(1));
  bool ok = false;
  if (nextL && nextL->value() == 1 && next->operand(1) == phi)
    ok = true;
  if (nextR && nextR->value() == 1 && next->operand(0) == phi)
    ok = true;
  if (!ok)
    return rejectCanonicalLoop(loop, "step-not-plus-one");

  Instruction* lterm = latch->terminator();
  if (!lterm || lterm->opcode() != Opcode::Br ||
      dynamic_cast<BasicBlock*>(lterm->operand(0)) != header) {
    return rejectCanonicalLoop(loop, "latch-not-direct-br");
  }

  out.preheader = preheader;
  out.header = header;
  out.body = body;
  out.latch = latch;
  out.exit = exit;
  out.ivPhi = phi;
  out.ivNext = next;
  out.cmp = cond;
  out.condBr = term;
  out.initVal = init;
  out.boundVal = bound;
  out.ivIsCmpLhs = ivIsLhs;
  return true;
}

bool collectCanonicalLoopPhis(const CanonicalLoop& loop,
                              std::vector<Instruction*>& phis,
                              std::vector<Value*>& initVals,
                              std::vector<Value*>* nextVals) {
  for (auto& instPtr : loop.header->instructions()) {
    if (instPtr->opcode() != Opcode::Phi)
      break;
    auto* phi = instPtr.get();
    Value* init = nullptr;
    Value* next = nullptr;
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<BasicBlock*>(phi->operand(i + 1));
      if (pred == loop.preheader) {
        init = phi->operand(i);
      } else if (pred == loop.latch) {
        next = phi->operand(i);
      }
    }
    if (!init || (nextVals && !next))
      return false;
    phis.push_back(phi);
    initVals.push_back(init);
    if (nextVals)
      nextVals->push_back(next);
  }
  return !phis.empty();
}

} // namespace hir
