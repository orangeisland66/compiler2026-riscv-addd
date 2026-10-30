// 文件职责：声明标量递推分析和变换接口。
#pragma once
#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"
#include "../../analysis/loop_info.h"
#include "../../hir/canonical_loop.h"

#include <initializer_list>
#include <string>

namespace core_ir {

// 判断规范循环是否为"i < bound"正向递增形式。
bool canonicalLoopIsForwardLt(const hir::CanonicalLoop& loop);
// 判断自然循环是否只有唯一出口块，且等于预期出口块。
bool loopHasSingleExit(const analysis::NaturalLoop& loop, BasicBlock* expectedExit);
// 在 pos 之前插入一条指定 opcode 的指令（用于递推改写时构造新指令）。
Instruction* insertModRecInstBefore(BasicBlock* bb,
                                    Instruction* pos,
                                    Opcode op,
                                    Type ty,
                                    const std::string& name,
                                    std::initializer_list<Value*> operands);
// 在 pos 之前插入一条指定比较谓词的 icmp 指令。
Instruction* insertModRecICmpBefore(BasicBlock* bb,
                                    Instruction* pos,
                                    const std::string& cond,
                                    const std::string& name,
                                    Value* lhs,
                                    Value* rhs);
// 将指令插入到块的 phi 序列之后、第一条非 phi 指令之前。
void insertBeforeFirstNonPhi(BasicBlock* bb, Instruction* inst);

// 将"已知上界的有符号加法"改写为模加法形式，避免溢出判断。
bool applyBoundedSignedModuloAdd(Module& mod);
// 识别并改写模运算标量递推循环（如计数器按模取值的模式）。
bool applyModularScalarRecurrences(Module& mod,
                                   analysis::AnalysisManager& analysisManager);
// 识别并改写重复归约型循环（多步迭代做同一种归约的模式）。
bool applyRepeatedReductionLoops(Module& mod,
                                 analysis::AnalysisManager& analysisManager);
// 识别并改写小型线性递推循环（将迭代展开为直线表达式）。
bool applySmallLinearRecurrenceLoops(Module& mod,
                                     analysis::AnalysisManager& analysisManager);
// 合并"反复覆写同一位置"的重复循环。
bool applyOverwriteRepeatLoopCollapse(Module& mod,
                                      analysis::AnalysisManager& analysisManager);

} // namespace core_ir
