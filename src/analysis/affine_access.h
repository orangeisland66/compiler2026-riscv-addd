// 文件职责：描述 affine 访存表达式，供循环变换和地址递推使用。本文件负责声明接口和数据结构。
#pragma once

#include "induction_var.h"

#include <vector>

namespace analysis {

struct AffineTerm {
  core_ir::Value* basis = nullptr;
  int64_t coeff = 0;
};

struct AffineScaledTerm {
  core_ir::Value* basis = nullptr;
  core_ir::Value* invariantScale = nullptr;
  int64_t coeff = 0;
};

struct AffineExprSummary {
  bool valid = false;
  int64_t constant = 0;
  std::vector<AffineTerm> ivTerms;
  std::vector<AffineScaledTerm> scaledIvTerms;
  std::vector<AffineTerm> invariantTerms;
};

struct AffineAccessSummary {
  bool valid = false;
  core_ir::Value* root = nullptr;
  AffineExprSummary offset;
};

AffineExprSummary summarizeAffineExpr(core_ir::Value* value,
                                      const NaturalLoop& loop,
                                      const std::vector<InductionVar>& ivs);

AffineAccessSummary summarizeAffineAccess(core_ir::Value* addr,
                                          const NaturalLoop& loop,
                                          const std::vector<InductionVar>& ivs);

int64_t affineConstCoeffFor(const AffineAccessSummary& summary,
                            core_ir::Value* basis);

int64_t affineWeightedCoeffFor(const AffineAccessSummary& summary,
                               core_ir::Value* basis);

// 这里只用于 loop profit 的 stride 成本估计。与 affineWeightedCoeffFor 不同，
// 动态 invariant scale 会被视作非 unit-stride 访问成本，不用于 affine 等价判断。
int64_t affineStrideCostForProfit(const AffineAccessSummary& summary,
                                  core_ir::Value* basis);

bool isLoopInvariantAffine(const AffineExprSummary& expr);

} // namespace analysis
