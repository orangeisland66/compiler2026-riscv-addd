# matmul 通用恢复证据（2026-08-14）

## 身份与口径

- 当前分支/源码锚点：`master-new` / `0f18dc6e22872e1277dd42780a45de8a4015f123`
  （工作树有本轮修改，不能把该提交号冒充候选二进制内容）。
- 基线：`master@c7eba4837484dab6b80e19dabf610ebfa78a02fc`。
- 基线编译器 SHA-256：
  `0646d2e53cabcc1700e213ba1d34809dd3783ad15b6058ec483140edb8519237`。
- 候选 strict 编译器 SHA-256：
  `3463dfc6618b919523c7bef8272d93e6850a2c6c564c0dd35ed14ce48dd9d916`。
- runtime SHA-256：
  `7f5b21a016eb4525b775bd99bffdf15aaf026dcc6736c076f64beb09a0529cab`。
- 正式用例：
  `/home/orangeisland66/下载/设计文档/2026初赛RISCV赛道性能用例/performance`。
- 两侧均使用公开 `-S -O1 -o`、同一 runtime、GCC/QEMU 和
  `tests/run_performance.py --runs 1 --jobs 1`。
- `internal_us` 是唯一性能判定指标；`run_ms` 只记录为 QEMU/宿主环境辅助数据。
  以下为单轮正序和反序原始样本，不表述为 median。

## 通用修复

恢复的不是 matmul 名称或固定形状特判，而是带完整证明和机器准入的
row-broadcast producer-rowmin 融合：

1. 在 reduction split 之前事务式运行候选，避免后者复制 producer store 后破坏
   唯一 producer/observer 关系。
2. 从 CoreIR 的循环、use-def、对象、仿射下标和 transpose-copy 事实证明四个对称
   访问角色。transpose 关系可以组合得到缺失角色，不依赖 transpose forwarding
   是否先运行。
3. 只在 producer、row minimum、broadcast、transpose 和 checksum 的 effect、
   live-out 与 CFG 闭包均成立时物化；旧 outer latch 与被旁路循环统一纳入原子
   删除集合，随后重建 CFG 并验证 Phi/predecessor。
4. 强对称证明成功时进入 strict；未证明的非对称 producer 路径继续保持
   research-only。
5. RV64 admission 使用实际 linear-scan 池容量（24 GPR、29 FPR）判断压力越界，
   并继续以 post-RA spill、frame 和真实 MIR 指标作最终门禁。

候选的事务式机器指标为：

```text
gpr 17 -> 17, fpr 0 -> 0, spill+reload 0 -> 0, frame 64 -> 64
instructions 205 -> 165
hot-loop instructions 164 -> 117
hot-loop loads 9 -> 8
hot-loop stores 4 -> 4
hot-loop branches 20 -> 11
```

相对同一 research 二进制的统一 rollback，正式性能集仅
`matmul1/2/3` 改变，即 `3/60 changed`。strict 注入普通 skip/enable 环境变量、
research 未完整 opt-in 时，三例汇编均与 strict 候选 byte-identical。

## 动态结果

| 顺序/实现 | matmul1 internal_us / run_ms | matmul2 | matmul3 | total internal_us | total run_ms（辅助） |
|---|---:|---:|---:|---:|---:|
| 正序 master | 11000 / 33.751 | 40000 / 81.733 | 25000 / 62.269 | 76000 | 177.752 |
| 正序 current | 12000 / 38.583 | 38000 / 75.371 | 20000 / 48.904 | 70000 | 162.857 |
| 反序 master | 13000 / 42.154 | 42000 / 85.908 | 23000 / 57.818 | 78000 | 185.880 |

- 正序按唯一指标：`76000 -> 70000 us`，current 改善 `7.89%`。
- 反序按唯一指标：current `70000 us`，随后 master `78000 us`，current 改善
  `10.26%`。
- 相对修复前 current 的 `131000--138000 us`，当前 `70000 us` 降低
  `46.56%--49.28%`，并在两种顺序样本中追平且超过 master。

## 正确性与门禁

- 正式 strict `-O1` 功能集：`140/140 PASS`。
- row-broadcast 通用正例、正式 symmetric producer 正例和 weighted 近似负例：
  PASS。
- loop semantic optimizations：PASS。
- RV64 planning contracts：PASS。
- pass 语义台账：`9/9 PASS`。
- strict audit：`12/12 PASS`。
- strict/research 环境隔离：PASS。
- `git diff --check`：PASS。

完整 `test_riscv_compliance_risks.sh` 本轮未通过，其首个阻断为
`signed bit accumulator positive did not trigger`。该检查不属于本次 matmul 路径，
因此这里不把整套 compliance risk 脚本报告为 PASS，也不以 matmul 的定向门禁掩盖
该残余问题。

## 结论与残余风险

matmul 的既知稳定回退已关闭。结论限于上述单轮正/反序内部计时与 3/60 静态
影响面；它不是完整 60 点发布性能结论。`many_mat_cal` 的内部计时小幅变化仍需
按预先约定样本复核，完整 compliance risk 阻断也需要独立关闭。
