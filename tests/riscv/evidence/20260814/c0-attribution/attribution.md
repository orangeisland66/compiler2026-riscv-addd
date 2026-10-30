# C0 RV64 attribution

- baseline: `master@c7eba4837484dab6b80e19dabf610ebfa78a02fc`
- candidate: `master-new@0f18dc6e22872e1277dd42780a45de8a4015f123+worktree`
- cases: `60/60`
- assembly: `0 identical / 60 changed`
- CoreIR-first: `48`
- MIR/backend-first: `12`

## Aggregate machine delta

| metric | baseline | candidate | delta |
|---|---:|---:|---:|
| bytes | 418511 | 393632 | -24879 |
| instructions | 19010 | 16990 | -2020 |
| loads | 2784 | 2388 | -396 |
| stores | 1983 | 1590 | -393 |
| calls | 486 | 513 | +27 |
| branches | 2184 | 2205 | +21 |
| conditional_branches | 1149 | 1203 | +54 |
| stack_accesses | 2358 | 1830 | -528 |
| address_materializations | 2461 | 2303 | -158 |
| mul | 558 | 539 | -19 |
| div_rem | 75 | 63 | -12 |

## Changed cases

| case | family | first layer | inst | load | store | branch | call | stack | bytes |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|
| `01_mm1.sy` | 01_mm | CoreIR | +51 | +41 | +17 | +1 | +2 | +20 | +901 |
| `01_mm2.sy` | 01_mm | CoreIR | +51 | +41 | +17 | +1 | +2 | +20 | +901 |
| `01_mm3.sy` | 01_mm | CoreIR | +51 | +41 | +17 | +1 | +2 | +20 | +901 |
| `03_sort1.sy` | 03_sort | CoreIR | -176 | -35 | -10 | +3 | +1 | +4 | -2569 |
| `03_sort2.sy` | 03_sort | CoreIR | -176 | -35 | -10 | +3 | +1 | +4 | -2569 |
| `03_sort3.sy` | 03_sort | CoreIR | -176 | -35 | -10 | +3 | +1 | +4 | -2569 |
| `conv2d-1.sy` | conv2d | CoreIR | -177 | +7 | -5 | +3 | +0 | +25 | -2439 |
| `conv2d-2.sy` | conv2d | CoreIR | -177 | +7 | -5 | +3 | +0 | +25 | -2439 |
| `conv2d-3.sy` | conv2d | CoreIR | -177 | +7 | -5 | +3 | +0 | +25 | -2439 |
| `crc1.sy` | crc | CoreIR | +45 | +3 | +3 | +7 | +0 | +6 | +904 |
| `crc2.sy` | crc | CoreIR | +45 | +3 | +3 | +7 | +0 | +6 | +904 |
| `crc3.sy` | crc | CoreIR | +45 | +3 | +3 | +7 | +0 | +6 | +904 |
| `crypto-1.sy` | crypto | MIR/backend | -12 | -6 | -6 | +0 | +0 | -12 | -181 |
| `crypto-2.sy` | crypto | MIR/backend | -12 | -6 | -6 | +0 | +0 | -12 | -181 |
| `crypto-3.sy` | crypto | MIR/backend | -12 | -6 | -6 | +0 | +0 | -12 | -181 |
| `fft0.sy` | fft | CoreIR | -38 | -16 | -27 | +0 | +3 | -35 | -146 |
| `fft1.sy` | fft | CoreIR | -38 | -16 | -27 | +0 | +3 | -35 | -146 |
| `fft2.sy` | fft | CoreIR | -38 | -16 | -27 | +0 | +3 | -35 | -146 |
| `h-1-01.sy` | h-1 | MIR/backend | -6 | -3 | -3 | +0 | +0 | -6 | -56 |
| `h-1-02.sy` | h-1 | MIR/backend | -6 | -3 | -3 | +0 | +0 | -6 | -56 |
| `h-1-03.sy` | h-1 | MIR/backend | -6 | -3 | -3 | +0 | +0 | -6 | -56 |
| `h-10-01.sy` | h-10 | CoreIR | -36 | -11 | -7 | -5 | +0 | -6 | -617 |
| `h-10-02.sy` | h-10 | CoreIR | -36 | -11 | -7 | -5 | +0 | -6 | -617 |
| `h-10-03.sy` | h-10 | CoreIR | -36 | -11 | -7 | -5 | +0 | -6 | -617 |
| `h-4-01.sy` | h-4 | MIR/backend | -12 | -6 | -6 | +0 | +0 | -12 | -110 |
| `h-4-02.sy` | h-4 | MIR/backend | -12 | -6 | -6 | +0 | +0 | -12 | -110 |
| `h-4-03.sy` | h-4 | MIR/backend | -12 | -6 | -6 | +0 | +0 | -12 | -110 |
| `h-5-01.sy` | h-5 | CoreIR | -140 | -44 | -12 | -10 | +0 | -24 | -2271 |
| `h-5-02.sy` | h-5 | CoreIR | -140 | -44 | -12 | -10 | +0 | -24 | -2271 |
| `h-5-03.sy` | h-5 | CoreIR | -140 | -44 | -12 | -10 | +0 | -24 | -2271 |
| `h-8-01.sy` | h-8 | CoreIR | -5 | +1 | -5 | +0 | +0 | -10 | -72 |
| `h-8-02.sy` | h-8 | CoreIR | -5 | +1 | -5 | +0 | +0 | -10 | -72 |
| `h-8-03.sy` | h-8 | CoreIR | -5 | +1 | -5 | +0 | +0 | -10 | -72 |
| `h-9-01.sy` | h-9 | CoreIR | -18 | -2 | -2 | -8 | +2 | -8 | -200 |
| `h-9-02.sy` | h-9 | CoreIR | -18 | -2 | -2 | -8 | +2 | -8 | -200 |
| `h-9-03.sy` | h-9 | CoreIR | -18 | -2 | -2 | -8 | +2 | -8 | -200 |
| `huffman-01.sy` | huffman | CoreIR | -81 | -17 | -18 | -7 | +1 | -34 | -1249 |
| `huffman-02.sy` | huffman | CoreIR | -81 | -17 | -18 | -7 | +1 | -34 | -1249 |
| `huffman-03.sy` | huffman | CoreIR | -81 | -17 | -18 | -7 | +1 | -34 | -1249 |
| `knapsack_naive-1.sy` | knapsack_naive | CoreIR | -11 | -5 | -6 | +0 | +0 | -12 | -112 |
| `knapsack_naive-2.sy` | knapsack_naive | CoreIR | -11 | -5 | -6 | +0 | +0 | -12 | -112 |
| `knapsack_naive-3.sy` | knapsack_naive | CoreIR | -11 | -5 | -6 | +0 | +0 | -12 | -112 |
| `many_mat_cal-1.sy` | many_mat_cal | CoreIR | +0 | -4 | -5 | +5 | +0 | -10 | +152 |
| `many_mat_cal-2.sy` | many_mat_cal | CoreIR | +0 | -4 | -5 | +5 | +0 | -10 | +152 |
| `many_mat_cal-3.sy` | many_mat_cal | CoreIR | +0 | -4 | -5 | +5 | +0 | -10 | +152 |
| `matmul1.sy` | matmul | CoreIR | +7 | -11 | -12 | +16 | +0 | -8 | +723 |
| `matmul2.sy` | matmul | CoreIR | -3 | -11 | -12 | +16 | +0 | -8 | +273 |
| `matmul3.sy` | matmul | CoreIR | +7 | -11 | -12 | +16 | +0 | -8 | +729 |
| `optimization_scheduling1.sy` | optimization_scheduling | MIR/backend | -10 | -5 | -5 | +0 | +0 | -10 | -110 |
| `optimization_scheduling2.sy` | optimization_scheduling | MIR/backend | -10 | -5 | -5 | +0 | +0 | -10 | -110 |
| `optimization_scheduling3.sy` | optimization_scheduling | MIR/backend | -10 | -5 | -5 | +0 | +0 | -10 | -110 |
| `shuffle0.sy` | shuffle | CoreIR | -11 | -5 | -6 | +0 | +0 | -12 | -146 |
| `shuffle1.sy` | shuffle | CoreIR | -11 | -5 | -6 | +0 | +0 | -12 | -146 |
| `shuffle2.sy` | shuffle | CoreIR | -11 | -5 | -6 | +0 | +0 | -12 | -146 |
| `sl1.sy` | sl | CoreIR | -28 | -7 | -11 | -2 | +0 | -14 | -398 |
| `sl2.sy` | sl | CoreIR | -28 | -7 | -11 | -2 | +0 | -14 | -398 |
| `sl3.sy` | sl | CoreIR | -28 | -7 | -11 | -2 | +0 | -14 | -398 |
| `transpose0.sy` | transpose | CoreIR | -12 | -7 | -5 | +4 | +0 | -18 | -149 |
| `transpose1.sy` | transpose | CoreIR | -12 | -7 | -5 | +4 | +0 | -18 | -149 |
| `transpose2.sy` | transpose | CoreIR | -12 | -7 | -5 | +4 | +0 | -18 | -149 |
