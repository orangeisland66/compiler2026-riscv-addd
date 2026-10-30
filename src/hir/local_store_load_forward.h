// 文件职责：声明局部存储到加载的值转发接口。
#pragma once

namespace analysis {
class AnalysisManager;
}

namespace core_ir {
class Module;
}

namespace hir {

bool applyLocalStoreLoadForward(core_ir::Module& mod,
                                analysis::AnalysisManager& analysisManager);

} // namespace hir
