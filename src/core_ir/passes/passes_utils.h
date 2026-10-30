// 文件职责：声明 CoreIR pass 共用的辅助函数。
#pragma once
#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"
#include <string>
#include <cstdint>
#include <stdexcept>
#include <limits>

namespace core_ir {

// ============================================================
// 共享工具函数 — 所有 pass 文件都可以使用
// ============================================================

// 类型名称
std::string typeName(Type ty);

// 错误终止
[[noreturn]] void fail(const std::string& msg);

// 调试用实时操作数检查
void verifyLiveOperandsForDebug(const Module& mod, const char* stage);

// 常量检查
bool isConstZero(Value* v);
bool isConstOne(Value* v);
ConstInt* constIntValue(Value* v);
Instruction* sremWithConstMod(Value* v, int32_t& modOut);

// 比较条件
std::string cmpCond(Instruction* inst);
int evalIntCmp(const std::string& cond, int32_t lhs, int32_t rhs);
int evalFloatCmp(const std::string& cond, float lhs, float rhs);

// 循环内存候选检查
bool hasHIRLoopMemoryCandidates(Module& mod, analysis::AnalysisManager& am);

// 整数范围分析
struct KnownIntRange {
    bool valid = false;
    int64_t minValue = 0;
    int64_t maxValue = 0;
};

bool rangeWithinI32(int64_t minValue, int64_t maxValue);
KnownIntRange knownIntRange(Value* value, int depth = 0);

// 常量表达式求值
bool evaluateConstIntExpression(Value* value, int64_t& out, int depth = 0);
bool isScalarAlloca(Value* v);

} // namespace core_ir
