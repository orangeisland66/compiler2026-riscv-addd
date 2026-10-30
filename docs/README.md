# 项目文档索引

本目录存放项目设计说明和默认优化审计标准。编译器源码位于 `src/`，回归
测试与工具位于 `tests/` 和 `tools/`；SysY 运行时由评测平台或本地外部路径
提供，不属于仓库提交内容。

## 设计文档

- [编译器设计说明（Markdown）](compiler-design.md)：介绍两条编译通道、
  前端、AST/HIR、CoreIR、分析层、优化流水线、MIR/RISC-V 后端、ABI、
  外部运行时边界和测试体系。
- [编译器设计说明（PDF）](compiler-design.pdf)：与 Markdown 正文一致的
  排版版本，便于离线阅读和提交材料归档。
- [RISC-V 默认优化泛化性审计标准](compliance/generality-audit-standard.md)：
  定义默认 `-O1` 的通用触发、禁止触发、语义证明和证据要求。

## 可复核材料

- pass 语义、正例和负例索引：`tests/riscv/default_pass_semantics.tsv`
- RISC-V 汇编与 QEMU 回归证据：`tests/riscv/evidence/20260804/`、
  `tests/riscv/evidence/20260805/`
- RISC-V 回归测试脚本：`tests/tools/`
- RISC-V 测试与基准工具：`tools/`

临时构建目录和本地实验记录不属于正式提交材料。
