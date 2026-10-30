// 文件职责：声明按需复制变换接口。
#ifndef HIR_DEMAND_DRIVEN_COPY_TRANSFORM_H
#define HIR_DEMAND_DRIVEN_COPY_TRANSFORM_H

struct CompUnit;

namespace hir {

bool applyDemandDrivenCopyTransform(CompUnit& compUnit);

} // namespace hir

#endif
