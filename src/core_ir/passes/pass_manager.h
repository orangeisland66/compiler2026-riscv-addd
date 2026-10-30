// 文件职责：描述并执行 CoreIR 模块级 pass，为 AnalysisManager 提供统一失效入口。
#pragma once

#include "../ir/core_ir.h"
#include "../../analysis/analysis_manager.h"
#include "../../analysis/proof_result.h"

#include <functional>
#include <string>
#include <vector>

namespace core_ir {

struct ModulePassSpec {
  std::string stage;
  std::string name;
  bool enabled = true;
  analysis::AnalysisInvalidation invalidation =
      analysis::AnalysisInvalidation::CFG;
  std::function<bool(Module&, analysis::AnalysisManager&)> run;
  bool transactional = false;
  analysis::ProofResult legality =
      analysis::ProofResult::unknown("pass-legality-not-specified");
};

class ModulePassManager {
public:
  void add(ModulePassSpec spec);
  bool run(Module& mod, analysis::AnalysisManager& analysisManager) const;

private:
  std::vector<ModulePassSpec> passes_;
};

// 小型流水线片段可以用这个 helper 表达 pass 改写 IR 后如何刷新分析缓存。
// 它不改变 pass 本身的收益判断，只负责集中管理 Phase 1 的缓存协议。
bool runModulePass(const ModulePassSpec& spec,
                   Module& mod,
                   analysis::AnalysisManager& analysisManager);

} // namespace core_ir
