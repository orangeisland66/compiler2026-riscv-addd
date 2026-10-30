// 文件职责：识别位运算等价惯用法，把逐位模拟替换为原生位运算。本文件负责声明接口和数据结构。
#pragma once
#include "../core_ir/ir/core_ir.h"

struct CompUnit;
namespace analysis { class AnalysisManager; }

namespace rewrite {

// CoreIR 子集：把无副作用的双参数位累加辅助函数替换为等价的原生 i32 运算。
// 数字辅助函数、整循环模式和递归模乘模式仍由专门阶段处理。
bool applyBitAccumulatorRecognition(
    core_ir::Module& mod, analysis::AnalysisManager& analysisManager);

// 识别无副作用的二次幂进制数字提取辅助函数。该子集有独立的范围条件，
// 更宽的位模式集合仍由研究配置使用。
bool applyPow2DigitHelperRecognition(
    core_ir::Module& mod, analysis::AnalysisManager& analysisManager);

// 处理匹配完整的递归模乘递推。输入超出 [0, modulus) 时保留原递归 CFG。
bool applyRecursiveModMulRecognition(
    core_ir::Module& mod, analysis::AnalysisManager& analysisManager);

// 在过程间优化完成后，为有符号位模拟闭式表达式补充非负输入快路径；
// 慢路径保留完整 i32 语义，避免快路径条件干扰前序内联和循环优化。
bool lowerSignedBitClosureFastPaths(core_ir::Module& mod);

// 位运算惯用法规范化，对应 LLVM InstCombine / DAGCombine。
//
// 当循环或纯 helper 只是在逐位模拟 and/or/xor/not/shl/shr 等语义时，
// 可以替换成原生位运算，减少除余、分支和逐位累加。替换依据来自表达式
// 关系、无副作用属性和 IR/AST 结构。
//
// AST 层 helper 识别以 BitwiseEmulationSummary 风格的表达式摘要为入口：
// 调用点根据 callee 的纯表达式闭包和实参闭包决定替换，不按 helper 名称触发。
// 后续摘要可继续复用 FunctionEffectSummary。
bool applyBitPatternRecognition(core_ir::Module& mod);
bool applyBitPatternRecognition(core_ir::Module& mod,
                                analysis::AnalysisManager& analysisManager);
bool applyBitPatternRecognition(CompUnit& cu);

} // namespace rewrite
