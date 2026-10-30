# RV64 proof/planning 实施证据（2026-08-13）

## 基线与构建

- 分支：`master-new`
- `HEAD` / `master-new`：`0f18dc6e22872e1277dd42780a45de8a4015f123`
- `master`：`c7eba4837484dab6b80e19dabf610ebfa78a02fc`
- strict：`SYC_ENABLE_RESEARCH_PASSES=OFF`，Release 构建成功
- research：`SYC_ENABLE_RESEARCH_PASSES=ON`，Release 构建成功
- 基线工具：`tools/riscv_migration_baseline.py`

基线工具原先会递归进入 `tests/riscv/evidence/`，把三份历史 `.sy` 误计为当前
夹具；现已排除 evidence/build/hidden 目录并增加单元测试。当前 108 个顶层
`tests/riscv/*.sy` 的重跑汇总：

| 模式 | 编译通过 | 汇编字节 | 指令数 | 总编译时间 |
|---|---:|---:|---:|---:|
| O0 | 108/108 | 675679 | 36691 | 198.984 ms |
| O1 | 108/108 | 427624 | 18849 | 1856.301 ms |

原始逐用例 JSON 在构建目录生成，不提交编译器二进制、ELF、外部 runtime 或
其他 build artifact。

## 正确性与隔离

| 检查 | 结果 |
|---|---:|
| Python 工具测试 | 28/28 |
| pass semantic registry | 9/9 |
| strict environment/source audit | 12/12 |
| strict / research Release 构建 | PASS / PASS |
| 仓库语义夹具 O0 compile-only | 108/108 |
| 仓库语义夹具 O1 compile-only | 108/108 |
| research AST matmul 正例与 strict 隔离 | PASS |
| research if-convert 宽模式隔离与 strict-safe 子集 | PASS |
| RV64 planning/analysis/verifier/admission 可执行合同 | PASS |
| loop nest transactional semantics | PASS |
| default recognizer 动态语义 | PASS |
| compliance risk gate | PASS |
| analysis dump | 0/108 changed |
| strict 注入 semantic env | 0/108 changed |
| strict / research（未 opt-in） | 0/108 changed |
| 外部功能 O0/O1 140 点 | 140/140 / 140/140 |
| 外部性能 60 点 | 60/60；39 identical / 21 changed；7 个 changed family + 1 control 对比正确 |
| remaining / loop semantic 全量门禁 | 本轮未运行：相关脚本尚未按正式性能目录重新执行 |

strict 隔离检查注入 `SYC_RESEARCH=1`、实验 pass、CoreIR skip 和 simple RA；108 个
夹具逐文件比较均不变。research 构建未设置总开关/具体开关时也与 strict 逐文件
相同；`ast_matmul_interchange_research_positive.sy` 在完整 opt-in 后触发并保持运行
结果一致。

本轮补齐 A2/A3：函数 effect/termination/trap/escape 摘要、ordered region
effects、write coverage、statement dependence、region state equivalence 和 observer
closure。分析 dump 与正常编译的汇编逐字节相同。planning clone 同时保存 builtin
声明，并在拒绝候选后恢复 Value ID 检查点。

本轮继续补齐保守 integer facts 与 trip recipe：exact range、may-wrap Unknown、
常量四次循环、runtime masked bound 最大 15 次和 `INT_MAX` 回绕拒绝均由临时合同
覆盖。trip recipe 只进入缓存/dump/query，不参与现有终止性或变换授权，因此保持
analysis-only 边界。

事务式强度削减 pilot 在 `cross_call_extra_scalar_positive.sy` 上先产生
`accepted`，后续无候选/无机器收益尝试分别稳定返回
`candidate-transform-rejected` 与 `no-machine-benefit`。planned 与默认汇编
hash 不同，但两者链接运行均输出 `882490586`。同一开关注入 strict 后，strict
汇编仍逐字节不变。

## 验证器发现的历史问题

增强 IR verifier 在 O1 枚举编译中拒绝了两个原先会流入后端的非法 IR。根因
是 reduction split 用 `i32 Add(pointer, 4)` 构造相邻地址。实现已统一改为
`ptr GEP(base, 1)`，修复后迁移夹具和三组全量运行回归全部通过。

事务接线还捕获了 row locality 与 stage fusion 生成的
`phi-predecessor-mismatch`，以及 stencil 的 `code-growth` 和 recursive
memoization 的 `frame-growth`；这些候选均未提交。另一次确定性审计发现 MIR
地址归纳按 `unordered_set<MachineBasicBlock*>` 指针顺序选择代表。现已统一按块标签
稳定遍历，`conv2d-1.sy` 六次独立编译得到同一 SHA-256。

本轮专项动态回归又捕获到宽 if-conversion 的真实错误：提前物化 then 分支 load
后，`if_convert_accumulation_positive.sy` 从正确输出 `4400` 变为 `0`。宽模式已
fail-closed 到 strict 的无 load speculation gate，research enable 不能绕过；专项
脚本验证误编夹具不触发、strict hostile rollback 环境不改变汇编、research rollback
仍可关闭 strict-safe 子集。

`pure_call_last_value` 与 `loop_state_fixpoint` 在基线提交 `0f18dc6e` 已从 pipeline
移除，但旧台账仍标作已接线。本轮将二者改为 `DORMANT`，旧触发段仅在
`SYC_TEST_DORMANT_STATE_TRANSFORMS=1` 时执行；恢复前必须重新完成证明和动态验收。

## 正式套件与历史证据边界

- 功能：`/home/orangeisland66/下载/设计文档/2026初赛RISCV赛道功能用例`，枚举
  `140` 个 `.sy`；
- 性能：`/home/orangeisland66/下载/设计文档/2026初赛RISCV赛道性能用例`，枚举
  `60` 个 `.sy`；
- runtime：`build-plan/libsysy_runtime.a`；工具链为系统
  `riscv64-linux-gnu-gcc` 与 `qemu-riscv64`。

2026-08-14 使用当前 strict compiler 现场串行复核：功能 O0 `140/140`（`12.0s`）、
功能 O1 `140/140`（`46.8s`）；性能命令严格使用 `tests/run_performance.py --opt
--jobs 1 --runs 1`，结果 `60/60`，`total_run_ms=6895.786`、
`total_compile_ms=2218.264`、`total_link_ms=4133.034`、`wall_s=13.351`、
`total_internal_us=1133000.000`。原始报告保存在
`/tmp/syc-riscv-plan-performance-current/report.{csv,json,md}`，临时 ELF 与报告不提交。

同目录的 compliance remediation 证据记录过外部正式功能 O0/O1 140/140 和性能
正确性 60/60。2026-08-14 已重新定位上述正式 RISC-V 套件并完成下文动态复核；
附近 ARM 用例目录未用于替代 RISC-V 性能证据。

## 复现命令

```bash
python3 tools/riscv_migration_baseline.py --build-dir build-plan-strict \
  --output /tmp/rv64-plan-baseline.json --cases tests/riscv
python3 -m unittest discover -s tests/tools -p 'test_*.py'
BUILD_DIR=build-plan-strict bash tests/tools/test_rv64_planning_contracts.sh
COMPILER="$PWD/build-plan-strict/compiler" \
RESEARCH_COMPILER="$PWD/build-plan-research/compiler" \
RUNTIME="$PWD/build-plan/libsysy_runtime.a" \
  bash tests/tools/test_riscv_research_if_convert.sh
COMPILER=build-plan-research/compiler \
  CASE=<performance>/conv2d-1.sy \
  bash tests/tools/test_rv64_codegen_determinism.sh
```

全量动态回归需要显式传入外部 SysY runtime 与外部用例目录；这些路径不写入
编译器源码，也不作为隐式依赖。

2026-08-14 收口复核再次通过 strict/research Release 增量构建、`9/9` pass
semantic registry、`12/12` strict audit、`28/28` Python 工具测试和 RV64
planning contracts。planning contract 额外覆盖 RV64GC I/M/A/F/D/C 能力、V=false、
16 字节栈对齐和 DIVW/ADDW 保守成本关系。

## 2026-08-14 动态复核与新修复

本机后续找到了官方 RISC-V 功能/性能目录，重新执行后不再沿用“外部套件未
提供”的旧边界：

- official functional O0：`140/140`；
- official functional O1：`140/140`；
- `reduction_split_address_positive` 及 odd-bound/j-side-effect near-miss：
  default/rollback 输出一致；
- strict 跨进程确定性：`6/6` 汇编 SHA-256 相同；
- Python 工具：`28/28`，semantic registry：`9/9`，strict audit：`12/12`。

动态门禁发现并修复了一处 P1：reduction split 原先用 `GEP(cStoreAddr, 1)`
生成 lane1 结果地址。由于 `cStoreAddr` 已是线性化裸指针且不再携带 i32 元素
宽度，RV64 lowering 实际生成 `sw ..., 1(addr)`，正例从 `9280` 误编为 `0`。
修复后从原对象根复制完整地址表达式，并以 `j -> j+1` 重映射归纳变量；strict
输出恢复为 `9280`，汇编中不再出现该未对齐字节偏移。上文“统一改为
`GEP(base, 1)`”只修复了 IR 类型，未闭合裸指针步长语义，现以本段结论为准。

CandidateEvaluation 同时补齐独立的
`discovered/proved/transformed/verified/admitted/committed` 阶段位，并按 pass
统计 proof/transform/verify/machine rejection、提交数、评估耗时和 IR 增长；
planning contract 已覆盖 proof reject 与 verifier reject 的阶段/统计不变量。

M7 回退定位修复：过程内联的单调用者准入收紧为恰有一个直接 call site；短访存
循环的 partial-unroll fallback 收紧为仅在 streaming recurrence 事实成立时展开。
changed-only 串行复核保持输出正确；本轮仍观察到相对 master 的运行回退，故 M7
未完成。

G1 后续批次把 MemorySSA-lite 扩展为按底层 object identity 的分区登记，并提供
`clobberBefore`、`sameMemoryVersion`、`mustOverwrite`、`hasObserverBetween`
公共查询。同块查询使用精确 byte range 证明跨越不相交 store；may-alias store、
unknown call 和跨路径不一致仍返回 Unknown。AnalysisManager 同时增加
Values/Memory/CFG/Calls/ModuleGlobals 失效类别，合同测试覆盖 epoch 和失效后重建。

G2 首批把 CoreIR 标量 load CSE/store-to-load forwarding 和同块 DSE 改为
消费上述公共查询。新增 `storedValueReachesLoad` 与
`memoryValueReachesStore`；只有等宽 must-alias 且中间无 may-alias write 时才转发
或删除同值写回。DSE 删除早期 store 还需要 `mustOverwrite` 成功且
`hasObserverBetween` 明确返回 Disproved。未知 call、部分覆盖、可能别名和范围
不明均 fail-closed。普通 RV64 指针 load/store 的默认宽度同时由错误的
4 byte 修正为 8 byte。

这些规则不检查运行时输入值：同值写回的候选只比较 CoreIR SSA
identity，再由 alias/range/effect 证明授权。planning contract 专门以未知函数参数
作为该 SSA 值的正例，并以中间同址 clobber 作为 Unknown 负例。
最终 strict O1 正式功能集为 `140/140`，planning contract、确定性、语义
台账和 strict audit 均通过。

为隔离本批影响，在 `/tmp/syc-g2-pre.1dLrAe` 从当前源树仅反向 G2 diff
构建 pre-G2 Release compiler。官方 60 点静态 A/B 为 `57 identical / 3 changed`，
只有 `transpose0..2` 改变；另加 byte-identical `matmul1` control。两侧均使用
`tests/run_performance.py --opt --jobs 1 --runs 1` 串行运行且输出正确。单轮
transpose `total_run_ms` 为 `218.079 -> 185.160`（`-15.1%`），control 为
`60.048 -> 60.071 ms`。逐点为 `14.513 -> 18.174`、`23.680 -> 22.152`、
`179.886 -> 144.833 ms`；`transpose0` 的单轮回退作为异常值保留，不将该样本
表述为 median 或稳定结论。原始报告位于 `/tmp/syc-g2-perf-run/`。

G4/G5 基础批次将函数内存摘要从粗粒度 `reads/writesArgMemory`
布尔值扩展为 `readArgumentIndices` / `writtenArgumentIndices`，并新增
`queryCallModRef(call, location, bytes, effects)`。调用点现在能区分只读形参、
只写形参和未传入的 nonescaping alloca；普通指针形参之间仍保守
may-alias，unknown external/unsupported opcode 显式记为 unknown read/write。MemorySSA-lite
的 load version、store forwarding 和 DSE observer 查询均消费该 call-site mod/ref。
摘要还记录 `capturedArgumentIndices` 与 `returnedArgumentIndices`：直接返回、
将指针形参写入内存、或传给 unknown/capturing callee 都会保守标记 capture，
已知 callee 的返回形参索引会重映射到 caller。这两类事实当前仅供分析
与后续 planning 消费，不单独启用删除。

observer 基础同时增加 `buildRegionDemandSlice`，从已证明的外部 observers
逆向收集 region 内值依赖和 live-ins。它只是值 demand 合同；未进入 slice
不会自动授权删除，effect/trap/control/termination 仍需后续证明。合同测试
覆盖了“第 0 个指针只读、第 1 个只写、未传入 alloca NoModRef”与逆向
demand slice。最终 strict/research Release、O1 功能 `140/140`、planning contract、
determinism、semantic registry 和 strict audit 通过。

MemorySSA-lite 的公共版本查询进一步支持跨基本块的唯一 CFG 链：从定义到使用
的每一步必须只有一个 successor，后继也必须只有当前 predecessor，并显式拒绝
回边。合同正例使用未知运行时形参作为 store 值，证明决策不依赖输入数值；只把
控制流改成 diamond 合流的 near-miss 即返回 Unknown。相对本节上一版 60 份汇编，
本次 analysis-only 扩展为逐字节 `60/60 identical`。更一般的多前驱合流需要独立
dominance/post-dominance 与逐分区 Phi 证明，当前没有据此授权删除。

在 demand slice 之上新增 `proveRegionValueDeletion` 与事务式
`makeRegionValueDeletionCandidate`。第一版仅接受 use-closed、无副作用且已证明
不陷阱的纯标量子图；load/store/call/control、向量操作和除数/移位范围未知的
指令均不进入删除集。合同中的常量非零除数死链完成
`prove -> clone -> transform -> verify -> machine admission`，但正式 MIR cleanup
已消除同一死链，故候选以稳定原因 `no-machine-benefit` 拒绝且不 commit；相近的
运行时未知除数指令在 proof 阶段就被保留。该结果证明事务接线可执行，同时避免
把“IR 变小”误报为 RV64 收益。内存 materialization/region store 删除仍需完整
state equivalence、observer 与覆盖证明，尚未进入 strict。

事务检查还发现 `PlanningClone` 当前 commit 实际替换整个 Module，而候选原先可按
`Values` 等细粒度类别调用失效。这会让 AnalysisManager 保留以已销毁 Function/
Instruction 指针为 key 的缓存。现阶段 commit 强制执行 Module 级失效；等实现真正
的 function/region 局部所有权替换后，才能安全恢复候选声明的细粒度类别。该修复
与 region proof 接线后再次扫描正式 60 点，相对上一批仍为逐字节
`60/60 identical`。

G5 strict 首个可提交 region 候选实现为 dead local materialization。分析从本函数
sized alloca 出发，只沿 GEP/BitCast 枚举完整 use closure；每个 store 都必须有
对象内常量 byte range，且整个对象不存在 load、call、return、pointer capture、
Phi/Select 混合或其他未知使用。store 的标量值可以是未知运行时参数，因为删除
授权来自对象不可观察性而不是该值的数值特征。planning contract 中该候选实际
完成 admission 和 commit；带 reader 的近似反例明确拒绝。

删除 store 会让其值 producer 变成 dead，因此同步修复了通用 DCE 的 trap 合同：
运行时未知除数/移位范围保留；load 只有在地址被证明落在 global/alloca 的完整
对象范围内时才可作为 dead value 删除；dead pure call 还必须证明 callee
`total && nonTrapping`。合同覆盖“常量非零除数删除、运行时未知除数保留”和
“已知对象 load/未知 pointer load”的正负边界。

默认接入后正式 O1 功能回归为 `140/140`。正式性能 60 点静态影响为
`33 identical / 27 changed`，涉及 `03_sort`、`conv2d`、`fft`、`h-8`、
`knapsack_naive`、`many_mat_cal`、`matmul`、`shuffle`、`transpose` 九个 family；
changed cases 与 `crc1` byte-identical control 两侧输出全部正确。单轮 B→C 的
changed-case `total_run_ms` 为 `3857.574 -> 3769.548`（`-2.3%`），编译时间
`970.194 -> 1103.649 ms`；control 为 `12.431 -> 8.889 ms`，显示本轮环境噪声较大，
故不把运行差异表述为稳定收益。逐 family 原始报告位于
`/tmp/syc-g5-dead-local-perf/`。

G5 partial-observer 扩展复用同一 alloca use inventory：当对象仍有 reader 时，
逐 store 比较精确 byte range，只把与所有 load 均不重叠的写入加入候选。两字段
合同中字段 0 的 store/load 保留、字段 1 的 store 实际通过 admission 并 commit；
同字段 reader、动态 GEP 和 escape near-miss 返回稳定拒绝原因。整对象与部分对象
候选合并为一次 planning transaction，避免按字段重复 shadow lowering。

本批最终 strict O1 正式功能为 `140/140`。相对上一批正式性能 60 份汇编的
normalized sequence 为 `60/60 identical`，所以按影响面规则没有运行动态性能；
上一批 `33 identical / 27 changed` 及其单轮计时结论保持不变。

相对 pre-G4 已验证二进制，正式 60 点汇编为 `57 identical / 3 changed`，
仅 `huffman-01..03` 改变，汇编均从 `7894` 降为 `7804` bytes。changed-only
与 `matmul1` byte-identical control 两侧均输出正确。单轮 huffman
`total_run_ms=44.323 -> 39.863`（`-10.1%`），control
`58.316 -> 57.872 ms`；不将单轮结果表述为稳定收益。原始报告位于
`/tmp/syc-g4-perf-run/`。

相对计划指定的 `master@c7eba4837484dab6b80e19dabf610ebfa78a02fc`，官方
60 点 `-O1` 汇编扫描为 `39 identical / 21 changed`。按 changed-only 规则只
串行运行 7 个 changed family，并加入 byte-identical `sl1` control；全部输出
正确，但单轮 `total_run_ms` 从 `436.979` 增至 `812.310`（`+85.9%`），control
从 `63.807` 到 `60.087 ms`（`-5.8%`）。主要回退为 `01_mm +149.0%`、
`matmul +68.9%`、`transpose +490.0%`；因此 M7 的“无新增稳定回退”退出条件
明确未满足，不能把当前工作区表述为完整发布候选。原始临时报告位于
`/tmp/syc-generic-plan-perf/`，未提交 ELF、外部套件或临时构建产物。

## 2026-08-14 C1 分析闭环首批

按后续计划暂缓 C0/M7，先实施 C1；本批只扩展分析合同，不启用新的默认优化：

- MemorySSA-lite 新增 `sameMemoryVersionAcrossCFG`。它枚举定义与使用之间的全部
  无环 CFG 路径，只有每条路径均到达使用且不存在相关 store/call clobber 时才
  证明同版本；diamond 无写正例 proved，单分支写入 near-miss 为 Unknown，回边
  也拒绝。现有 CSE/DSE 仍消费原唯一链查询，未扩大默认变换范围。
- 模块级 `FunctionSemanticSummary` 迭代到不动点，显式携带参数级 read/write、
  capture、returned pointer、written globals、termination 和跨调用 non-trapping
  事实；callee 可能除零时 caller 同样标为 may-trap。
- `RegionDemandSlice` 在值 slice 外登记 ordered effect 指令与底层 memory object；
  ordered effect 含未知内存行为时返回 `region-memory-demand-unknown`，不能据此删除。

验证：strict Release 增量构建成功，RV64 planning contracts PASS，pass semantic
registry `9/9`，strict audit `12/12`，正式 O1 功能 `140/140`。使用实施前保留的
`build-plan/compiler` 与当前 `build-plan-strict/compiler` 对正式 RISC-V 性能 60 点
逐份生成 `-S -O1` 汇编，结果 `0/60 changed`；当前 compiler 正常环境与
`SYC_DUMP_ANALYSIS=1` 对比同样 `0/60 changed`。本批未运行性能计时，也不改变
M7 尚未完成的结论。
