# RISC-V 优化语义夹具

本目录保存默认 `-O1`、research 隔离和 MIR/backend 变换使用的 SysY
语义夹具。当前共有 111 个由 Git 跟踪的 `.sy` 文件，覆盖可触发正例、
近似但不满足前提的负例，以及边界行为。

## 证据入口

- `default_pass_semantics.tsv`：登记 124 个逻辑入口的实现位置、正确语义、
  正例、负例、状态和证据路径。
- `../tools/test_riscv_pass_semantic_registry.py`：检查台账字段、实现文件、
  证据路径和状态是否完整。
- `../tools/test_riscv_compliance_risks.sh`：验证递归、跨调用数组摘要、
  需求驱动复制、radix、循环展开和其他重点变换。
- `../tools/test_riscv_loop_semantic_optimizations.sh`：验证循环、规约、地址、
  live-out、副作用和终止边界。
- `../tools/test_riscv_remaining_pass_semantics.sh`：为 modular recurrence、
  function specialization、overwrite loop、除数版本化、row locality、
  row broadcast 和 stencil 提供独立正负证据。
- `../tools/test_rv64_abi_pressure.sh`：动态覆盖 12 个整数参数、10 个浮点参数、
  PHI 轮换、深调用、callee/caller-saved 值和超过 12-bit 偏移的大栈帧。
- `evidence/`：保存汇编、QEMU、trace、哈希与全量汇总等可复核产物。

## 命名与编写约定

- `_positive.sy`：满足全部语义前提，必须真实触发对应变换并保持输出。
- `_negative.sy`：形状相近但存在 alias、side effect、live-out、边界、
  终止或数据依赖问题，必须保守拒绝。
- `_research_positive.sy`：只验证 research 构建入口；strict 构建注入同名
  enable 后汇编必须不变。
- 文件顶部使用中文说明夹具意图，CFG、SSA、PHI、alias、live-out 等专有
  名词可以保留英文。

新增默认优化时，应同时更新语义台账、至少一个可触发正例、一个近似负例
和相应门禁脚本。仅有全量回归通过不能替代独立语义证明。
