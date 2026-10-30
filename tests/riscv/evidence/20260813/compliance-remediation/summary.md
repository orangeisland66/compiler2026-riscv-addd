# 2026-08-13 strict 合规整改验证

- 起点：`master-new@0f18dc6e22872e1277dd42780a45de8a4015f123`。
- strict 编译器 SHA-256：
  `daed38b67318c5125332042518401c60b861c376d80cde822326fde79a198a47`。
- 从 HEAD 导出后重建的基线编译器 SHA-256：
  `962e7f17c2e0104a6937544ea0a10d794e04337ea8e6acbd6d76566a11f32f29`。
- strict/research Release 构建：PASS。
- `test_riscv_pass_semantic_registry.py`：8/8 PASS。
- `test_riscv_strict_audit.py`：12/12 PASS（以 `python3 -m unittest` 运行）。
- strict hostile-environment 汇编逐字节不变：PASS。
- RV64GC/medany 静态链接 + QEMU：功能集 140/140，性能集 60/60。
- 60 点 HEAD/current 汇编：60/60 可编译，48 点不变，12 点变化；
  静态指令总数 `19053 -> 18054`（-5.24%）。
- 变化点只在 `conv2d-{1,2,3}`、`h-1-{01,02,03}`、
  `knapsack_naive-{1,2,3}`、`many_mat_cal-{1,2,3}`，指令数分别为
  `690 -> 607`、`198 -> 65`、`133 -> 101`、`423 -> 338`。
- 每家族一点的两对 QEMU smoke：HEAD 基线 4/4 更快。该样本数不满足
  13 对 strict 性能协议，只证明隔离代价需要后续通过通用 proof/planning
  能力恢复，不作稳定幅度结论。

临时构建、ELF、runtime 和单次运行产物留在 `/tmp`，未收入仓库。
