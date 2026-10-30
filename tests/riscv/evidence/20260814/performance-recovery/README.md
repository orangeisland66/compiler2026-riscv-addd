# 2026-08-14 通用性能恢复证据

> 提交合规后续（2026-08-14）：本报告记录的是性能恢复阶段的历史候选。
> 由于 radix、demand-copy 和 cross-call summary 的完整证明档案尚未与实现闭环，
> 三项随后重新进入编译期 research 隔离，不再属于当前 strict 提交配置。最终提交
> 边界见 `../submission-compliance/README.md`；本报告中的三项收益不得继续作为
> 当前 strict 性能结论。

## 身份与口径

- 当前分支：`master-new`
- 当前 HEAD：`0f18dc6e22872e1277dd42780a45de8a4015f123`（含未提交工作区）
- 对照 ref：`origin/master`
- 对照提交：`c7eba4837484dab6b80e19dabf610ebfa78a02fc`
- strict compiler SHA-256：`58a112e1255d84841c484a5e7468affe83abfb61fa32e5d65136f454715b70a0`
  （本报告不把 dirty HEAD 冒充二进制身份）。
- runtime SHA-256：`7f5b21a016eb4525b775bd99bffdf15aaf026dcc6736c076f64beb09a0529cab`
- 用例：`/home/orangeisland66/下载/设计文档/2026初赛RISCV赛道性能用例/performance`
- 命令：公开 `-S -O1 -o`，`tests/run_performance.py --runs 1 --jobs 1`
- 工具链：`riscv64-linux-gnu-gcc 13.3.0`、`qemu-riscv64 8.2.2`

PDF 合规检查以仓库根目录《关于编译优化合理性及相关违规行为认定的说明.pdf》
为规范条件。实现不读取用例名、路径、普通函数名、源码 hash 或运行时输入；触发
事实仅包含 AST/CoreIR 类型、调用和递归结构、use-def、数组读写/索引映射、
规范循环范围、对象容量、effect、live-out 与 RV64 指令语义。

## 实现

1. strict 默认恢复递归 radix 迭代降低、最终需求驱动复制切片和跨调用数组摘要。
   三者的普通 `SYC_SKIP_*` 在 strict 构建中无效；research build 只有同时设置
   `SYC_RESEARCH=1` 才能做同二进制 rollback。
2. 条件累积 if-conversion 允许提前 load 的严格子集：每个地址必须由所有包围
   规范循环的常量范围证明落在 direct global object 内，且中间 i32 地址算术
   全部无回绕。参数指针、动态范围和未知对象拒绝。
3. 清理 if-conversion 后无使用的合流 Phi，避免后续事务 verifier 遇到
   `phi-not-leading`。
4. reduction split 只复制第二车道累加结果的反向值切片，不按块内指令数量或
   样例形态复制无关计算。
5. MIR 将 `select(cond, value, 0)` 降低为布尔全宽掩码与 `AND`；当条件是已证明
   0/1 值与零的 `eq/ne` 时直接由原布尔值生成掩码。
6. research row-broadcast fusion matcher 补上 branch-form signed-min 的谓词证明，
   并原子删除已证明无 live-out 的旁路循环区域；该变换仍未进入 strict。

## 汇编影响面

相对本轮修改前保存的 60 份 current 汇编，最终有 15/60 改变：

```text
01_mm1 01_mm2 01_mm3
03_sort1 03_sort2 03_sort3
many_mat_cal-1 many_mat_cal-2 many_mat_cal-3
matmul1 matmul2 matmul3
transpose0 transpose1 transpose2
```

`fft` 与 `h-5` 在本轮修复前后 byte-identical，因此本轮不以其计时噪声声明代码
收益；`fft` 作为 byte-identical control 复测。

## 动态结果

前三类使用同一个 research compiler 做 `rollback baseline -> candidate` 和反序
`candidate -> rollback baseline`。表中均为三用例 `total_run_ms`，每格是一轮原始
总和，不称为 median。

| family | B→C baseline | B→C candidate | 改善 | C→B candidate | C→B baseline | 改善 |
|---|---:|---:|---:|---:|---:|---:|
| transpose | 243.699 | 35.089 | -85.60% | 35.687 | 213.796 | -83.31% |
| 01_mm | 188.840 | 73.925 | -60.85% | 65.577 | 199.867 | -67.19% |
| 03_sort | 131.349 | 89.469 | -31.88% | 97.393 | 118.605 | -17.88% |

与历史 master 单轮锚点相比，strict 候选的代表值为：transpose `32.057 ms`
（master `40.123 ms`）、01_mm `74.033 ms`（master `67.035 ms`）、03_sort
`88.277 ms`（master `83.099 ms`）。

`matmul` 在本报告阶段仍未关闭；后续已由具备完整对称访问证明、事务 verifier
和 RV64 admission 的默认融合关闭。最终结果与本表的历史中间状态分开记录于
[`../matmul-recovery/README.md`](../matmul-recovery/README.md)。

本报告阶段的 master/current 反序样本：

| 顺序 | master run_ms / internal_us | current run_ms / internal_us | run_ms 差异 |
|---|---:|---:|---:|
| master → current | 193.936 / 70000 | 235.031 / 131000 | +21.19% |
| current → master | 183.391 / 76000 | 239.819 / 138000 | +30.77% |

原始单轮为 `+53.45%`；bounded-load if-conversion 和零臂 select 降低已缩小缺口，
但仍未追平 master。research row-broadcast fusion 在 verifier 修复后可接受并正确
运行，单轮 `227.036 ms / 131000 us`，没有形成额外收益，所以未进入 strict。

`fft` byte-identical control 在两种顺序分别得到 master/current
`46.301/39.473 ms` 与 current/master `42.151/36.682 ms`，胜负随顺序反转且内部
计时仅 `0--2 ms`，不能支持原 `+38.70%` 为代码回退。`h-5` 本轮汇编不变；独立
复测为 current `1852.026 ms`、master 历史锚点 `1867.841 ms`，原 `+8.17%`
同样未复现。

`many_mat_cal` 是新增影响 family。同二进制 if-convert rollback/candidate 为
`160.842/164.269 ms`，内部 `108000/113000 us`；单轮差异较小，保留为残余噪声/
成本风险，不宣称收益。

## 正确性与门禁

- strict `-O1` 正式功能集：`100/100 PASS`
- `tests/tools/test_riscv_research_if_convert.sh`：PASS（bounded-load 正例、
  side-effect 负例、strict 环境确定性、research rollback）
- `tests/tools/test_riscv_pass_semantic_registry.py`：9/9 PASS
- `tests/tools/test_riscv_strict_audit.py`：12/12 PASS
- `git diff --check`：PASS
- 9 个 AST 恢复用例和 6 个 matmul/fft 对照用例均由性能 runner 校验输出。

更大的 `test_riscv_compliance_risks.sh` 在进入本轮 AST corpus 前，被既有
`single-register void call still uses stack staging` 检查阻断；该项与本轮 AST/
if-convert 修改无关，因此不能把整套脚本报告为 PASS。

## 残余风险

- `matmul` 的该项差距已在后续通用修复中关闭；当前不再列为残余性能回退，详见
  [`../matmul-recovery/README.md`](../matmul-recovery/README.md)。
- `many_mat_cal` 的单轮小幅变化需要在发布里程碑做预先约定的交错多轮统计。
- 本轮按 changed-only 规则验证，没有重复运行完整 60/60 动态计时。
