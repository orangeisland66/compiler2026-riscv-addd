// 文件职责：复用规范化循环识别与 phi 收集逻辑，避免多个循环变换各自维护同一套公共逻辑。
#pragma once

#include "../analysis/loop_info.h"
#include "../core_ir/ir/core_ir.h"

#include <string>
#include <vector>

namespace hir {

// 这里统一描述 HIR/CoreIR 中最常见的 for-loop 形状：
// preheader -> header(cond+phi) -> body/latch -> header，步长固定为 +1。
// 多个 loop transform 共享这层识别，能减少重复代码和细节漂移。
struct CanonicalLoop {
  core_ir::BasicBlock* preheader = nullptr;
  core_ir::BasicBlock* header = nullptr;
  core_ir::BasicBlock* body = nullptr;
  core_ir::BasicBlock* latch = nullptr;
  core_ir::BasicBlock* exit = nullptr;
  core_ir::Instruction* ivPhi = nullptr;
  core_ir::Instruction* ivNext = nullptr;
  core_ir::Instruction* cmp = nullptr;
  core_ir::Instruction* condBr = nullptr;
  core_ir::Value* initVal = nullptr;
  core_ir::Value* boundVal = nullptr;
  bool ivIsCmpLhs = false;
};

std::string canonicalLoopCmpCond(core_ir::Instruction* inst);

bool recognizeCanonicalLoop(const analysis::NaturalLoop& loop,
                            core_ir::BasicBlock* preheader,
                            CanonicalLoop& out);

// `nextVals` 为空时只提取 preheader incoming；需要完整 loop-carried
// 信息的 pass 再额外请求 latch incoming，避免各个 pass 自己重复扫描 phi。
bool collectCanonicalLoopPhis(
    const CanonicalLoop& loop, std::vector<core_ir::Instruction*>& phis,
    std::vector<core_ir::Value*>& initVals,
    std::vector<core_ir::Value*>* nextVals = nullptr);

} // namespace hir
