# 当前编译器与 GCC `-O3` 的 RV64 汇编能力差距报告

日期：2026-08-14  
当前分支：`master-new`  
当前 HEAD：`0f18dc6e22872e1277dd42780a45de8a4015f123`  
当前 strict 编译器 SHA-256：`b2c47e227b1beb1ad1f639bc7c6f7dd9406f81ce493524726cdf888a4c23c4da`

## 1. 结论

当前编译器和 GCC 的差距不是简单的“生成指令更多”。当前编译器在 60 个正式
RISC-V 性能用例上的总 `.text` 只有 GCC `-O3` 的 61.9%，静态指令只有 60.6%；
36 个用例指令更少，21 个更多，3 个相同。主要原因是 GCC 对 `fft` 和
`03_sort` 两族进行了激进的递归拆分、函数复制和展开，单是这 6 个用例就在
GCC 侧产生 16,500 条指令。

去掉 `fft` 和 `03_sort` 两个 GCC 代码增长最强的族后，当前编译器为 16,177 条
指令，GCC 为 15,603 条，当前反而多 3.68%。因此不能用全套总指令数宣称当前
中后端已经优于 GCC；也不能用 GCC 更大的代码体积直接推导 GCC 运行更慢。

对比时最明确且覆盖全部样本的通用缺口是 MIR 调用实参约束与寄存器分配衔接：
当时汇编存在 237 组相邻的“将值 `sd` 到栈槽，再立即 `ld` 到 `a0..a7`”序列，
分布在 60/60 个用例中；GCC 为 0。该缺口已在本报告同日的后续实现中修复，
修复后为 0/60；原始表格仍作为发现缺口时的可复核基线保留。

第二层差距集中在指针归纳变量、运行时内存 idiom、跨过程部分内联、相邻访存
合并以及复杂变换后的寄存器压力控制。相反，当前编译器在常量除模降低、部分
模乘专化和限制递归代码增长方面已有明确优势，后续不应通过无条件模仿 GCC
`-O3` 的展开预算破坏这些能力。

> 本报告只比较汇编形态和静态机器码，不给出动态性能结论。按仓库性能规范，
> 单次静态差异不能替代串行 changed-only 动态测量。

## 2. 对照口径

样本是正式 2026 初赛 RISC-V 性能目录中的 60 个 `.sy` 文件：

```text
/home/orangeisland66/下载/设计文档/2026初赛RISCV赛道性能用例/performance
```

当前编译器使用唯一公开优化通道：

```bash
compiler -S -O1 -o output.s input.sy
```

GCC 13.3.0 使用最高标准优化等级 `-O3`：

```bash
riscv64-linux-gnu-gcc \
  -march=rv64gc -mabi=lp64d -mcmodel=medany \
  -O3 -fwrapv -fno-strict-aliasing \
  -fno-pie -fno-stack-protector \
  -std=gnu11 -include sysy_gcc_compat.h \
  -S -o output.s source.c
```

- 没有使用 `-Ofast`，因为它会放宽浮点语义，不能作为 SysY 严格语义的主对照。
- `-fwrapv` 防止 GCC 利用 C 有符号溢出 UB，贴近本项目要求的 i32 模 `2^32`
  行为；`-fno-strict-aliasing` 避免把 C alias 规则优势混入比较。
- `-fno-pie` 与当前非 PIC 输出一致；`-fno-stack-protector` 排除 GCC 安全插桩造成
  的额外指令。二者不降低 `-O3` 优化级别。
- 兼容层只声明 SysY runtime、把 `starttime/stoptime` 展开为带行号 runtime 调用，
  并把文件作用域 `const int` 常量改写为等值 C `enum`，没有根据样例或输入改变
  计算。
- 两侧汇编均再次用交叉 GCC 汇编为 RV64GC 目标文件；60/60 成功。统计来自
  `objdump -d -M no-aliases` 和目标文件 `.text*` section，而不是把伪指令或汇编
  directive 当成机器指令。

复现工具为 `tools/riscv_gcc_asm_compare.py`，摘要证据位于
`tests/riscv/evidence/20260814/gcc-o3-assembly-comparison/`。完整临时汇编和逐例
CSV 位于 `/tmp/syc-rv64-gcc-o3-asm-final-20260814`，不纳入仓库。

## 3. 总体静态数据

| 指标 | 当前编译器 | GCC `-O3` | 当前 / GCC | 解释限制 |
|---|---:|---:|---:|---|
| 机器指令 | 19,441 | 32,103 | 0.606 | 被 GCC 在两个递归族的大幅展开主导 |
| `.text*` 字节 | 61,518 | 99,330 | 0.619 | 目标文件、链接松弛前 |
| load | 1,944 | 1,599 | 1.216 | 当前即使总代码更小，load 仍更多 |
| store | 831 | 1,812 | 0.459 | GCC 展开和大栈对象显著抬高总数 |
| 条件分支 | 1,203 | 3,066 | 0.392 | 同样受 GCC 展开影响 |
| 跳转/返回 | 1,515 | 2,079 | 0.729 | 不等价于动态执行次数 |
| 栈访问 | 2,658 | 5,010 | 0.531 | GCC 的 sort/fft 大代码体积主导 |
| 汇编函数体 | 102 | 237 | 0.430 | GCC 含 partial clone/拆分函数 |

逐例指令比的中位数是 0.946，几何平均是 0.914。总量、逐例中位数和运行时间
是三个不同概念，本报告不互相替代。

## 4. 分族结果

比值大于 1 表示当前编译器生成的静态机器指令更多：

| family | 用例数 | 当前 | GCC | 当前 / GCC | 当前 load / GCC load |
|---|---:|---:|---:|---:|---:|
| sl | 3 | 2,325 | 675 | 3.444 | 363 / 21 |
| h-1 | 3 | 669 | 300 | 2.230 | 69 / 6 |
| many_mat_cal | 3 | 1,452 | 693 | 2.095 | 153 / 18 |
| matmul | 3 | 775 | 456 | 1.700 | 57 / 24 |
| h-10 | 3 | 795 | 483 | 1.646 | 114 / 21 |
| 01_mm | 3 | 933 | 573 | 1.628 | 186 / 21 |
| h-4 | 3 | 651 | 522 | 1.247 | 0 / 0 |
| h-8 | 3 | 594 | 594 | 1.000 | 84 / 39 |
| transpose | 3 | 375 | 387 | 0.969 | 27 / 12 |
| h-5 | 3 | 678 | 708 | 0.958 | 75 / 57 |
| h-9 | 3 | 519 | 555 | 0.935 | 21 / 6 |
| shuffle | 3 | 603 | 717 | 0.841 | 108 / 66 |
| crypto | 3 | 1,821 | 2,436 | 0.748 | 201 / 144 |
| conv2d | 3 | 1,773 | 2,412 | 0.735 | 138 / 333 |
| optimization_scheduling | 3 | 216 | 318 | 0.679 | 0 / 0 |
| knapsack_naive | 3 | 471 | 705 | 0.668 | 33 / 48 |
| crc | 3 | 378 | 648 | 0.583 | 21 / 6 |
| huffman | 3 | 1,149 | 2,421 | 0.475 | 105 / 96 |
| fft | 3 | 2,178 | 9,846 | 0.221 | 69 / 51 |
| 03_sort | 3 | 1,086 | 6,654 | 0.163 | 120 / 630 |

family 名只用于报告分组，不参与任何编译或优化决策。

## 5. 已确认的通用能力差距

### G1：调用实参 coalescing 与 RA/ABI 衔接，已完成

**后续状态：已于 2026-08-14 修复。** 默认 lowering 现已删除入口函数和 CFG
位置特例，寄存器实参统一通过 RA 后 parallel copy 进入 ABI 寄存器；十整数参数
定向负例只把第 9、10 个参数写入 `0(sp)`、`8(sp)`。正式 60 例中的相邻
`sd → ld a0..a7` 从 237 组降为 0，目标文件指令减少 834 条，`.text` 减少
1,758 bytes；140/140 功能和 60/60 性能输出正确性均通过。证据见
`tests/riscv/evidence/20260814/call-argument-parallel-copy/summary.json`。

动态计时又按相同 runtime、GCC、QEMU、正式 60 例和 `--jobs 1` 各执行一轮
`baseline → candidate` 与反向 `candidate → baseline`。以候选相对基线计算，
外部 `total_run_ms` 分别为 `+4.3608%` 和 `+0.4409%`，两种顺序都没有显示总运行
时间改善；但内部计时分别为 `+1.4273%`、`-2.6110%`，方向反转。byte-identical
control 的波动也从 `+4.5783%` 反转为 `-19.5590%`，20 个 family 中只有 8 个
方向一致。因此这两次独立单轮样本既未证明稳定动态收益，也不足以判定稳定回退；
静态指令、栈访存和代码尺寸下降的结论不受影响。原始 CSV、family 汇总和完整计时
口径见同一证据目录下的 `performance-summary.json`。

修复前的输出在每一个样本中都出现如下形态：

```asm
sd   t5, 0(sp)
ld   a0, 0(sp)
call callee
```

两条访存可以在寄存器约束允许时变为 `mv a0, t5`，或由预着色/parallel copy
直接把 producer 放入 `a0`。修复前共 237 组，GCC 为 0。它同时解释了为什么原始
总代码只有 GCC 的 60.6%，load 总数却是 GCC 的 121.6%。

通用修复方向：让 call operand constraint、parallel copy、linear scan 和
post-RA cleanup 共享同一 ABI copy 模型；区分真正的 overflow stack argument、
跨 call spill home 和只为搬入 `a0..a7/fa0..fa7` 建立的临时栈槽。证明只依赖
live interval、寄存器类、call clobber 和 psABI，不能依赖 callee 名称。

### G2：指针归纳变量和地址 recurrence 不完整

在 `sl`、`matmul`、`01_mm` 等多个无关循环中，当前输出保留较多 `mulw/slliw`、
重复 `la` 和下标地址重算；GCC 更常把 `(base, index, stride)` 归一为 64 位指针
PHI，以 `addi` 更新并用 end pointer 比较。例如 `matmul` 的输入循环中，GCC
保留一次数组基址后每轮把行指针加 800，当前仍按行号乘 800 再加基址。

通用修复方向：统一 Loop canonical IV、affine access 和 MIR address recurrence；
证明 object、元素宽度、步长、回绕边界和 zero-trip 后建立 pointer IV。SysY int
归纳仍保持 i32，只有已证明的地址计算使用 64 位语义。

### G3：运行时长度 memory idiom 覆盖不足

GCC 在 12/60 个样本中生成 15 个 `memset`/`memcpy` 调用，覆盖动态长度清零、
填充和连续复制；当前能把部分固定小范围清零展开成 `sd zero`，但没有为这些
运行时范围形成统一 idiom。例如 `many_mat_cal` 的两段二维填充被 GCC 降为
`memset`，`h-10` 的连续复制被降为 `memcpy`。

通用修复方向：在 MemorySSA/object-range 之上识别单写 loop region，证明 byte
pattern、连续范围、方向、overlap、对齐、中间 observer 和退出覆盖；零/重复字节
用 `memset`，不重叠复制用 `memcpy`，可能重叠时只能用 `memmove` 或保留循环。
候选需经真实 RV64 call/RA/frame 成本 admission。

### G4：跨过程部分内联和递归 SCC 简化仍窄

`h-1` 中 GCC 保留公开的 `fun` 函数以满足外部可见性，同时把其控制逻辑内联到
`main` 的调用点；当前生成额外递归 helper 和 memo 状态，`main` 仍有内部调用，
该族指令比为 2.23。GCC 在全套中生成 12 个 `.part/.constprop/.isra` 类拆分函数，
当前生成 3 个 `.spec` 函数，说明当前已有专化基础，但对递归 SCC、部分内联和
调用点范围事实的消费不完整。

通用修复方向：基于 call graph SCC、参数常量/范围、effect、termination、
递归边和机器 pressure 做 partial inline/clone；保留 external ABI body，并限制
clone 总数和代码增长。不得识别 Collatz、FFT 等算法名称或固定常量。

### G5：复杂 region 变换后的机器压力约束不足

`many_mat_cal` 当前单函数静态指令约为 GCC 的 2.10 倍，且出现 4,208-byte frame；
GCC 对应 `main` frame 为 96 bytes。该源程序没有需要 4 KiB 的局部数组，frame
主要反映复杂区域降低后临时值和 spill 压力。当前已有 machine admission，但
仍有默认 HIR/CoreIR 变换没有统一经过相同的 clone/RA/frame 比较。

通用修复方向：所有会内联、融合、复制 CFG 或扩大 live range 的默认变换都纳入
`prove → clone → transform → verify → RV64 admission → commit`；成本必须包含
peak GPR/FPR pressure、spill slot、frame、循环深度加权访存和 code size。

### G6：相邻访存合并与标量 SLP 不完整

GCC 能在证明全局对象对齐和相邻两个 i32 写入值相同时，合并为一次 64 位 store；
也能把连续 copy 交给 `memcpy`。当前主要停留在单条 store forwarding/DSE 和
固定清零展开，缺少一般的相邻 lane 打包。

通用修复方向：从同一 object 的精确 byte range、little-endian 布局、至少 8-byte
对齐、两条 store 的值构造和 observer-free 顺序证明出发，形成 RV64 store-pair
candidate；若需要额外 pack 指令或提高 pressure，由机器成本拒绝。

### G7：尾调用和分支布局仍有小而稳定的后端缺口

当前 60 例汇编中没有 `tail`，GCC 有 6 处；当前也更常保留显式 `j` 和基于 i32
下标的上下界分支。尾调用不是主差距，但它反映 call lowering、frame teardown
和 block layout 尚未形成统一合同。

通用修复方向：仅在返回值/参数 ABI 一致、无未恢复 frame/callee-save、无剩余
live value 且目标可直接跳转时做 sibling-call lowering；分支简化继续消费 CFG
频率、fallthrough 和 compare-use，而不是按函数或用例启用。

## 6. 当前编译器已有的相对优势

1. **控制代码增长更稳。** GCC 在 `fft` 和 `03_sort` 上分别生成约 4.5 倍和
   6.1 倍于当前的静态指令。当前没有盲目复制整个递归体，整体 `.text` 更小。
2. **常量除模降低较积极。** 发射汇编中当前只有 12 条 `remw`，GCC 有 1,017
   条；当前大量使用 `mulh`/magic-number 序列和通用模乘 helper。静态条数不能
   证明动态收益，但说明强度降低能力真实存在。
3. **已具备 region 和函数专化基础。** 在 `conv2d` 等样本中当前能消除多个中间
   helper call，并且已有 `.spec` 函数、MemorySSA-lite、effect/termination summary
   与 machine admission。后续应补公共分析精度和接入覆盖，不应恢复按算法骨架
   的旧 matcher。
4. **压缩指令利用率相近。** 当前目标文件 19,441 条指令中 8,123 条来自 C 扩展
   编码，GCC 为 32,103 中的 14,541 条；没有证据表明 C 扩展选择是首要瓶颈。

## 7. 建议实施顺序

1. 修复 call argument 的寄存器 coalescing，并增加“无临时栈往返”的 MIR 正负
   夹具；这是最确定、覆盖面最大的收益点。
2. 把 pointer IV/address recurrence 接到现有 induction、affine access 和 RV64
   cost model，先覆盖单一 object、常量 stride、nonescaping/直接 global。
3. 增加运行时长度 memset/memcpy/memmove idiom proof，并使用 MemorySSA observer
   查询；保留 overlap/unknown call/dynamic out-of-range 负例。
4. 将仍绕过 transaction 的大变换接入真实 RA/frame admission，优先消除
   `many_mat_cal` 类 4 KiB 非语义 frame。
5. 在统一 SCC summary 上实现受预算的 partial inline 和 tail-call lowering。
6. 最后扩展相邻 store packing/标量 SLP；它需要更完整的 alignment、端序和
   pressure proof，风险高于前三项。

每一项默认优化都应继续遵守重命名/扰动不变性、近似负例、140 功能回归和
60 份汇编影响面流程。报告中的 family 仅用于衡量覆盖面，不得成为 pass 触发或
成本参数。

## 8. 残余风险与未覆盖项

- 没有执行 GCC 与当前编译器的动态性能 A/B，因此不能判断 GCC 的大规模展开在
  QEMU 或真实 RV64 上是否盈利。
- `-fwrapv` 约束了 C 有符号加减乘溢出优化，但 C 对 `INT_MIN / -1` 等边界与
  SysY 的完整机器语义仍不完全相同；涉及这些边界的具体指令差异不能只凭 GCC
  输出作为正确性依据。
- 目标文件尚未经过最终静态链接 relaxation；绝对 `.text` 字节可能在链接后改变，
  但本报告的 load/store/branch 和相邻栈往返结论不依赖该变化。
- 正式性能样本存在每族三个输入副本，形态覆盖仍有限。上述优化建议必须用新建
  的无关正例、重命名变体和 alias/effect/zero-trip/overflow near-miss 验证，不能
  用 60 例通过替代通用 legality proof。
