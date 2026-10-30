// 文件职责：声明行局部性分析和变换接口。
#pragma once
#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"
#include "../../analysis/loop_info.h"
#include "../../hir/canonical_loop.h"

#include <cstdint>
#include <vector>

namespace core_ir {

// 判断 value 是否为整数常量 0。
bool isZeroInt(Value* value);
// 若 bb 只有一条无条件跳转后继，返回该目标块；否则返回 nullptr。
BasicBlock* singleBrTarget(BasicBlock* bb);
// 判断 value 是否为 base 加常量偏移 delta 形式的 GEP 地址。
bool gepBasePlusConst(Value* value, Value* base, int64_t delta);
// 收集循环 header 的 phi（含初值与回边值）；不匹配循环形态时返回 false。
bool collectHeaderLoopPhis(const hir::CanonicalLoop& loop,
                           std::vector<Instruction*>& phis,
                           std::vector<Value*>& initVals,
                           std::vector<Value*>* nextVals);
// 行局部性变换：将按列遍历改写为按行遍历以提升缓存命中率；返回是否有改动。
bool applyInPlaceRowLocality(Module& mod,
                             analysis::AnalysisManager& analysisManager);

} // namespace core_ir
