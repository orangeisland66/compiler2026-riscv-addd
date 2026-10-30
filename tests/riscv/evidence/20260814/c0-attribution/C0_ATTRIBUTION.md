# C0 性能账本与收益归因（2026-08-14）

## 可复现输入

- 候选分支/HEAD：`master-new` / `0f18dc6e22872e1277dd42780a45de8a4015f123`
  加当前工作树；候选编译器 SHA-256 记录在 `attribution.json`。
- 独立基线：从 `master@c7eba4837484dab6b80e19dabf610ebfa78a02fc`
  通过 `git archive` 导出后单独执行 CMake Release 构建。
- 正式 RISC-V 性能目录：60 个 `.sy`；`.sy/.in/.out` 清单聚合 SHA-256 为
  `0aeba89f07488860c51ed8f6dfc069952ba6452e18a364e4b41db7a11c9d2a04`。
- runtime SHA-256：
  `7f5b21a016eb4525b775bd99bffdf15aaf026dcc6736c076f64beb09a0529cab`。
- 工具链：GCC `13.3.0`，QEMU `8.2.2`，`rv64gc/medany/static`。
- 动态顺序固定为一轮 `B -> C`，`--opt --jobs 1 --runs 1`；内部 timer 是唯一
  性能指标，`run_ms` 只作辅助噪声诊断。下列数字不是多轮 median，也不作为稳定
  收益声明。

## 静态影响面

60 个性能用例全部正确编译，规范化汇编为 `0 identical / 60 changed`。其中 48 个
首差位于优化后 CoreIR，12 个 CoreIR 相同、首差位于 MIR/frame 后端。逐 case 的
CoreIR/汇编首差行、hash 和指标位于 `attribution.json`。

| 指标 | baseline | candidate | 变化 | 解释 |
|---|---:|---:|---:|---|
| 汇编字节 | 418511 | 393632 | -24879 | 正向静态收益 |
| 指令 | 19010 | 16990 | -2020 | 正向静态收益 |
| load/store | 2784/1983 | 2388/1590 | -396/-393 | 正向静态收益 |
| 栈访问 | 2358 | 1830 | -528 | frame/prologue 收缩有效 |
| 地址物化 | 2461 | 2303 | -158 | 地址递推/物化改善 |
| mul/div-rem | 558/75 | 539/63 | -19/-12 | 局部强度削减有效 |
| 条件分支 | 1149 | 1203 | +54 | 热循环回退信号 |
| call | 486 | 513 | +27 | 内联/特化组合仍有额外调用 |

## 动态总账

性能集 60 点全部 changed，因此另从正式功能集选择两侧规范化汇编完全相同的
`00_main.sy` 作为 control。两侧 `61/61` 输出正确。

| 口径 | baseline | candidate | 变化 |
|---|---:|---:|---:|
| **changed internal timer（唯一性能指标）** | **701000 us** | **1102000 us** | **+57.20%** |
| changed `run_ms`（辅助） | 6455.356 | 6798.928 | +5.32% |
| changed `compile_ms`（独立编译成本） | 2029.422 | 2177.113 | +7.28% |
| changed `link_ms`（辅助） | 3925.108 | 3711.016 | -5.45% |
| byte-identical control `run_ms`（辅助） | 9.657 | 9.778 | +1.2% |

候选整体代码更小，但内部 timer 明确回退；因此不能用静态尺寸或辅助 `run_ms`
替代 M7 性能结论。byte-identical control 没有有效内部 timer，只能用于辅助确认
QEMU 墙钟环境未出现同量级漂移，不能参与性能胜负。

## 各 family 收益

`internal_us` 为唯一性能指标；`0 -> 0` 或基线为 0 表示计时区间缺失/低于当前
粒度，该 family 不作性能胜负结论。`run_ms` 仅列作辅助。静态列为 family 合计。

| family | 主触发能力 | 指令/分支/栈 | internal_us B -> C | 主指标变化 | run_ms 辅助 | 判定 |
|---|---|---:|---:|---:|---:|---|
| 01_mm | blocking + partial unroll | +153/+3/+60 | 0 -> 104000 | 不可量化 | +136.3% | 候选跨越计时粒度，强回退信号 |
| 03_sort | bit helper + partial unroll | -528/+9/+12 | 3000 -> 22000 | +633.3% | +15.4% | 明确负收益 |
| conv2d | global scalar + inline + blocking | -531/+9/+75 | 9000 -> 23000 | +155.6% | +31.5% | 明确负收益 |
| crc | bit helper + inline + loop address | +135/+21/+18 | 0 -> 0 | 无有效计时 | +15.9% | 不作性能判定 |
| crypto | frame/prologue only | -36/0/-36 | 112000 -> 97000 | -13.4% | -9.5% | 正收益 |
| fft | modmul + affine rewrite + frame | -114/0/-105 | 1000 -> 0 | 低于粒度 | -9.5% | 正向信号，不量化稳定收益 |
| h-1 | frame/prologue only | -18/0/-18 | 14000 -> 16000 | +14.3% | -1.5% | 内部 timer 负收益 |
| h-10 | inline + overwrite + blocking/unroll | -108/-15/-18 | 25000 -> 27000 | +8.0% | +1.2% | 负收益 |
| h-4 | frame/prologue only | -36/0/-36 | 66000 -> 65000 | -1.5% | -6.1% | 小幅正收益 |
| h-5 | affine update rewrite | -420/-30/-72 | 0 -> 0 | 无有效计时 | -0.9% | 不作性能判定 |
| h-8 | inline + blocking + affine rewrite | -15/0/-30 | 0 -> 0 | 无有效计时 | -2.8% | 不作性能判定 |
| h-9 | global scalar + loop address | -54/-24/-24 | 9000 -> 9000 | 0.0% | +5.9% | 性能中性 |
| huffman | global scalar + multi-inline | -243/-21/-102 | 0 -> 6000 | 不可量化 | +3.9% | 回退信号 |
| knapsack_naive | dead local + frame | -33/0/-36 | 0 -> 0 | 无有效计时 | +0.2% | 不作性能判定 |
| many_mat_cal | blocking + affine rewrite | 0/+15/-30 | 99000 -> 107000 | +8.1% | +6.9% | 负收益 |
| matmul | multi-level blocking | +11/+48/-24 | 66000 -> 164000 | +148.5% | +68.7% | 明确负收益 |
| optimization_scheduling | frame/prologue only | -30/0/-30 | 0 -> 0 | 无有效计时 | +14.4% | 不作性能判定 |
| shuffle | global scalar + inline | -33/0/-36 | 153000 -> 134000 | -12.4% | -0.1% | 正收益 |
| sl | divisor versioning + blocking/unroll | -84/-6/-42 | 144000 -> 164000 | +13.9% | +10.2% | 负收益 |
| transpose | inline + blocking | -36/+12/-54 | 0 -> 164000 | 不可量化 | +506.4% | 最大回退信号 |

## 按优化项明确收益

| 优化/基础设施 | 独立或当前证据 | 收益结论 |
|---|---|---|
| 分区 MemorySSA、mod/ref、observer、proof/planning | analysis-only 对既有批次 `0/60 changed` | 性能收益 0；价值是统一 legality 与 fail-closed |
| MemorySSA 驱动 CSE/DSE | 隔离批次只有辅助 `run_ms`，未保存有效内部 timer 对照 | 性能收益证据不足；旧 `-15.1% run_ms` 仅保留为辅助信号 |
| 参数级 mod/ref/global promotion 基础 | 隔离批次只有辅助 `run_ms`，当前 huffman 为 `0 -> 6000 us` | 不能再用旧 `-10.1% run_ms` 宣称收益；当前组合有回退信号 |
| dead local materialization | 隔离批次没有可比较的有效内部 timer 总账 | 性能收益证据不足；旧 `-2.3% run_ms` 仅作辅助 |
| partial-observer field slicing | 相对上一批正式 60 点 `60/60 identical` | 当前性能收益 0 |
| 通用 frame/prologue 收缩 | crypto `112000 -> 97000 us`，h-4 `66000 -> 65000 us` | 分别 +13.4%/+1.5% 单轮正收益 |
| affine rewrite/地址递推 | FFT 降到计时粒度以下；h-5/h-8 无有效内部 timer | FFT 有正向信号，其余性能证据不足 |
| blocking/unroll | 03_sort +633.3%、matmul +148.5%；01_mm/transpose 从 0 增至 104000/164000 us | 当前最大负收益，应先修成本模型/准入 |
| aggressive procedure inline | conv2d +155.6%；huffman 从 0 增至 6000 us；shuffle -12.4% | 跨 family 收益不稳定，需按 pressure/call-site 收紧 |
| divisor versioning | sl `144000 -> 164000 us`，+13.9% | 负收益，版本化分支成本未被当前模型覆盖 |
| BOOM profile/机器指标 | profile 未标定，不改变代码生成 | 性能收益 0；只提供后续 admission 数据 |

## C0 结论

C0 的“重建性能账本与定位首差”已完成：60 个 changed case 均有 CoreIR 或
MIR/backend 首差、机器指标、family 触发能力和单轮动态结果。当前首要根因不是
RA/frame；frame 总体改善。阻断项集中在 CoreIR 的 blocking/unroll、过度 inline 和
divisor versioning 成本判断。唯一主指标 `total_internal_us` 回退 57.20%。C0 不等于
M7：上述回退尚未修复，因此当前工作树仍不是性能发布候选。

归因 trace 还发现并修复一处可观测路径 UAF：affine accumulator 改写删除 loop
header 后，后续 trace 曾继续解引用旧指针。现在 mutation 前快照成功诊断字段，
其他路径在打印前验证 CoreIR `Value` 仍存活；`03_sort1` 的失效 header 稳定输出
`<deleted>`。该门禁已接入 loop semantic 脚本，正常代码生成不读取 trace 快照。
