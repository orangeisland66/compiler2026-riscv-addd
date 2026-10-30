// 文件职责：识别归纳变量和循环步进形式，服务循环规范化与强度削减。本文件负责声明接口和数据结构。
#pragma once
#include "../core_ir/ir/core_ir.h"
#include "integer_facts.h"
#include "loop_info.h"
#include "proof_result.h"
#include <optional>
#include <vector>

namespace analysis {

struct InductionVar {
  core_ir::Instruction* phi;      // 定义归纳变量的 PHI
  core_ir::Instruction* storage;  // 可选的归纳变量标量 alloca
  core_ir::Value* init;           // 循环前的初值
  int64_t step;                   // 常量步长（+c 或 -c）
  core_ir::Value* stepVal;        // 步长值，非固定步长时为空
  bool isCanonical;               // i = i ± c 的简单形式
};

enum class TripCountKind {
  Unknown,
  RuntimeBounded,
  ConstantMachineExact,
};

struct LoopTripRecipe {
  core_ir::Instruction* induction = nullptr;
  core_ir::Value* initial = nullptr;
  core_ir::Value* bound = nullptr;
  core_ir::BasicBlock* exitingBlock = nullptr;
  core_ir::BasicBlock* exitBlock = nullptr;
  std::string predicate;
  int64_t step = 0;
  int64_t constantTripCount = -1;
  TripCountKind kind = TripCountKind::Unknown;
  ProofResult proof = ProofResult::unknown("trip-count-not-analyzed");
};

// 识别 i = phi(init, i + c) 形式的简单归纳变量。
std::vector<InductionVar> findInductionVars(
    core_ir::Function* func, const analysis::NaturalLoop& loop);

LoopTripRecipe analyzeLoopTrip(core_ir::Function* func,
                               const NaturalLoop& loop);

std::optional<int64_t> maximumTripCount(const LoopTripRecipe& recipe);

// 对循环内使用归纳变量的 GEP，计算 base_address + induction_var * stride *
// elem_size。可以表示为指针递推时返回按字节计的步长，否则返回 0。
int computeGEPStride(core_ir::Instruction* gep, core_ir::Instruction* indVar,
                     const NaturalLoop& loop);

} // namespace analysis
