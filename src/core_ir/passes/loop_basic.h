// 文件职责：声明 CoreIR 基础循环整理和变换接口。
#pragma once
#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"

namespace core_ir {

// 循环规范化（唯一出口、preheader 规整等），不依赖分析缓存的版本。
bool loopCanonicalize(Module& mod);
// 循环规范化，复用 AnalysisManager 中的分析结果。
bool loopCanonicalize(Module& mod, analysis::AnalysisManager& am);
// 循环内局部值编号（识别循环体内重复计算的表达式）。
bool loopLocalValueNumbering(Module& mod, analysis::AnalysisManager& am);
// 循环不变代码外提（将循环不变量提到 preheader）。
bool loopInvariantCodeMotion(Module& mod, analysis::AnalysisManager& am);

} // namespace core_ir
