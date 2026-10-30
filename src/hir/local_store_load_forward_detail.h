// 文件职责：提供局部存储转发使用的字节区间判定。
#pragma once

#include <cstdint>

namespace hir::detail {

struct MemoryByteRange {
  int64_t offset = 0;
  int64_t width = 0;
};

inline bool memoryByteRangesOverlap(const MemoryByteRange& lhs,
                                    const MemoryByteRange& rhs) {
  if (lhs.width <= 0 || rhs.width <= 0)
    return false;
  if (lhs.offset <= rhs.offset) {
    const uint64_t distance = static_cast<uint64_t>(rhs.offset) -
                              static_cast<uint64_t>(lhs.offset);
    return distance < static_cast<uint64_t>(lhs.width);
  }
  const uint64_t distance = static_cast<uint64_t>(lhs.offset) -
                            static_cast<uint64_t>(rhs.offset);
  return distance < static_cast<uint64_t>(rhs.width);
}

} // namespace hir::detail
