// 文件职责：声明循环阶段合并接口。
#pragma once
#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"
#include "../../analysis/loop_info.h"

#include <cstdint>
#include <vector>

namespace core_ir {

// 将矩阵循环边界值规范化（剥掉常量乘法包装，返回维度值）。
Value* normalizedMatrixLoopBound(Value* value);
// 从方阵循环边界中提取维度值（边界为 n*n 形式时返回 n）。
Value* squareMatrixLoopDimension(Value* value);
// 判断 value 是否为全局数组首址（偏移 0）的 GEP。
bool matchRootZeroGEP(Value* value, GlobalVar* root);
// 判断 value 是否为全局数组按索引取行的 GEP，返回数组根与行索引。
bool matchRootVectorIndexGEP(Value* value, GlobalVar*& rootOut, Value* index);
// 判断 phi 是否为逐元素顺序推进的指针递推（步长 stepBytes 字节），返回初值。
bool matchSequentialPointerPhi(Instruction* phi,
                               BasicBlock* preheader,
                               BasicBlock* latch,
                               Value*& initOut,
                               int32_t stepBytes = 4);
// 按 header 块在循环列表中查找对应的自然循环；未找到返回 nullptr。
const analysis::NaturalLoop* loopForHeader(
    const std::vector<analysis::NaturalLoop>& loops,
    BasicBlock* header);
// 阶段合并：将"按行映射 + 行归约求校验和"的相邻循环合并为单循环。
bool applyMapRowReduceChecksumFusion(Module& mod,
                                     analysis::AnalysisManager& analysisManager);
// 阶段合并：将"行广播 + 转置 + 校验和"的相邻循环合并为单循环。
bool applyRowBroadcastTransposeChecksumFusion(
    Module& mod,
    analysis::AnalysisManager& analysisManager);

} // namespace core_ir
