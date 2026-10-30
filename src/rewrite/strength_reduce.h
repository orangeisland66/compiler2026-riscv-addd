// 文件职责：执行常量强度削减，把昂贵算术替换为等价低成本形式。本文件负责声明接口和数据结构。
#pragma once
#include "../core_ir/ir/core_ir.h"

namespace rewrite {

// 对右操作数为常量的 Mul、SDiv、SRem 执行常量强度削弱。
//
// 在 CoreIR 层改写，使两个后端都能受益，并让后续 CSE/DCE 继续简化：
//
//   乘法：
//     x * 0           → 0
//     x * 1           → x
//     x * -1          → -x
//     x * 2^k         -> x << k                     （单次移位）
//     x * (2^a - 2^b) -> (x << a) - (x << b)        （Booth，例如 *7 = (<<3)-x）
//     x * (2^a + 2^b) -> (x << a) + (x << b)        （例如 *9 = (<<3)+x）
//     x * 负数常量 C -> -expand(x * |C|)
//
//   SDiv（使用有符号正确的移位序列，通常比 DIVW 成本低）：
//     x / 1           → x
//     x / -1          → -x
//     x / 2^k         -> 带符号处理的移位序列：
//                       sign = x >> 31              （算术移位，0 或 -1）
//                       bias = sign & (2^k - 1)
//                       result = (x + bias) >> k
//
//   SRem（保持有符号结果，避免 REMW）：
//     x % 1, x % -1   → 0
//     x % 2^k         -> ((x + bias) & mask) - bias  （与除法使用相同 bias）
//
// 所有移位都使用 CoreIR 的 Shr，后续会降低为算术右移。通过
// `t & (2^k - 1)` 将 (0/-1) 符号掩码转换为 (0/2^k-1) 偏置，无需逻辑右移。
//
// popcount 大于 2 的常量会产生较长的依赖链，因此不再展开；MULW 通常更合适。
// Booth 风格的移位减法仍覆盖 *7、*15、*31 等常见的“2 的幂减一”常量。
bool applyConstantStrengthReduction(core_ir::Module& mod);

} // namespace rewrite
