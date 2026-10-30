// 文件职责：组织 CoreIR 优化流水线，包括清理、内联、CSE、循环优化等。本文件负责声明接口和数据结构。
#pragma once

#include "../ir/core_ir.h"

#include <string>
#include <unordered_set>

namespace analysis { class AnalysisManager; }

namespace core_ir {

struct PipelineOptions {
  // 在中端执行常量强度削减，把常量乘/除/模改成移位、加减或
  // 等价递推链。性能入口默认开启，接口默认关闭，避免调用方被动承受激进
  // 变换。
  bool enableConstStrengthReduce = false;
  // 把小数组 alloca 拆成逐元素标量槽，便于后续 Mem2Reg 提升。
  bool enableArraySROA = false;
  // 当循环内不存在别名写时，把循环不变 load 提升到 preheader。
  bool enableLoadHoist = false;
  // 把 affine 地址链改写成 pointer-bump recurrence。该能力主要
  // 服务 MIR lowering，因此保持显式开关，避免非性能入口误吃。
  bool enableArrayRecurrence = false;
  // 融合共享相同迭代空间的相邻循环，消除冗余循环控制开销
  // 并减少内存流量。处理前检查迭代空间等价和依赖关系。
  bool enableLoopFusion = false;
  // 在 loop forest 上尝试相邻循环交换，目标是把更好的 unit-stride
  // 维推到内层；处理前检查内层体和 affine 访问关系。
  bool enableLoopInterchange = false;
  // 对小 trip-count 循环做全展开，对简单热内层做保守部分展开，
  // 以减少控制开销并暴露更多 ILP。
  bool enableLoopUnroll = false;
  // 对 affine loop band 做保守 strip-mining / blocking，让后续
  // 地址递推和局部标量替换面向更小的 tile-local 工作集。
  bool enableLoopBlocking = false;
  // 若某函数的全部调用点都忽略返回值，则把该函数的返回值替换为零
  // 常量，使原返回值计算链可被后续 DCE 清除。实验性开关，默认关闭。
  bool enableDeadReturnElim = false;
  // 循环标量累加器提升会把 load/store 累加槽改成 SSA phi。
  // 该变换需要寄存器压力收益模型保护，默认先关闭，避免扩大 live range。
  bool enableLoopScalarAccumulator = false;
  // 是否启用完整的 affine/loop-memory HIR 主线，包括 band
  // canonicalization、tightening、scalar recurrence rewrite 以及其上的
  // loop 变换。若高层重写破坏了这类结构，可只保留标量 SSA 主线。
  bool enableHIRLoopMemory = true;
  // 即使完整 loop-memory 主线关闭，一些高层改写后仍可安全补跑
  // 低风险循环清理。这里表达的是能力，而不是具体 pass 名单。
  bool enableStandaloneLoopUnroll = false;
  bool enableStandaloneAffineScalarRewrite = false;
  // 若 HIR 已经改写过递归结构，则后续 CoreIR 递归 pass 需要
  // 保守对待这批函数，避免重复重写。
  bool rewroteRecursiveStructure = false;
  std::unordered_set<std::string> blockedPostHIRRecursiveFunctions;
  // O3 级流水线的统一分析入口。当前先作为生命周期锚点，后续 pass 逐步
  // 改为从这里取 dominator、loop、effect、affine 等缓存结果。
  analysis::AnalysisManager* analysisManager = nullptr;
};

void verifyModule(const Module& mod);
bool runCoreIROptimizations(
    Module& mod,
    const PipelineOptions& options = {});

} // namespace core_ir
