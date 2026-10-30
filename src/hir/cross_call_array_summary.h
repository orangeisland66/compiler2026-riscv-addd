// 文件职责：声明跨函数数组访问摘要接口。
#ifndef HIR_CROSS_CALL_ARRAY_SUMMARY_H
#define HIR_CROSS_CALL_ARRAY_SUMMARY_H

struct CompUnit;

namespace hir {

bool applyCrossCallArraySummary(CompUnit& compUnit);

} // namespace hir

#endif
