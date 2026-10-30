// 文件职责：声明循环终止性分析的数据结构和查询接口。
#pragma once

#include <unordered_map>

#include "../core_ir/ir/core_ir.h"
#include "loop_info.h"

namespace analysis {

class AnalysisManager;

// 函数到"可证明终止"的映射（true 表示该函数可证终止）。
using FunctionTerminationMap = std::unordered_map<const core_ir::Function*, bool>;

// 判断循环是否为可证明有限的单元计数器循环（形如 for(i=0;i<n;++i)）。
bool isProvablyFiniteUnitCounterLoop(const NaturalLoop& loop);

// 判断指令是否为可证明不产生陷阱（除零/越界）的标量指令。
bool isProvablyNonTrappingScalarInstruction(const core_ir::Instruction* inst);

// 对模块内所有函数做终止性证明，返回可证终止的函数集合。
FunctionTerminationMap proveModuleFunctionTermination(core_ir::Module& mod,
                                                      AnalysisManager& analysisManager);

// 判断一条 call 的被调函数是否可证明必然返回（总函数）。
bool isProvablyTotalCall(const core_ir::Instruction* call,
                         const core_ir::Module& mod,
                         const FunctionTerminationMap& termination);

}  // namespace analysis
