// 文件职责：声明 CoreIR 内存到寄存器转换接口。
#pragma once
#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"

namespace core_ir {

// 全局标量 Mem2Reg
bool globalScalarMem2Reg(Module& mod);
bool globalScalarMem2Reg(Module& mod, analysis::AnalysisManager& am);

// 单块标量 Mem2Reg
bool singleBlockScalarMem2Reg(Module& mod);

Instruction* createScalarSlotAllocaInEntry(Function* func, Type elemTy,
                                           const std::string& nameHint);

} // namespace core_ir
