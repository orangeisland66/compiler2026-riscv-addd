# Strict 提交合规整改证据（2026-08-14）

## 依据与身份

- 唯一规范性条件：根目录《关于编译优化合理性及相关违规行为认定的说明.pdf》，
  SHA-256
  `a900ee0c7dcb8605b2bd5b0a56f2018d6f6bd0145493cbd84c7cf093a506ece5`。
- 分支/源码锚点：`master-new` / `0f18dc6e22872e1277dd42780a45de8a4015f123`
  （含未提交整改工作树，提交号不冒充候选内容）。
- strict compiler SHA-256：
  `9045ba9ca832e0671a77ad1597f1bc204bbc2b06a4a52c0ef7f1884100067e7b`。
- research compiler SHA-256：
  `d352c7c110af9799660a1a6c85a3912c463d56e7cf97c6bac9d9b20c252021c2`。
- runtime SHA-256：
  `7f5b21a016eb4525b775bd99bffdf15aaf026dcc6736c076f64beb09a0529cab`。

## 整改内容

1. `radix_sort_lowering.cpp`、`demand_driven_copy_transform.cpp` 和
   `cross_call_array_summary.cpp` 加入 `RESEARCH_SOURCES`，strict 在 source-list
   阶段排除实现。
2. `main.cpp` 的三个入口由编译宏包围。research build 还必须同时设置
   `SYC_RESEARCH=1` 和各自的 `SYC_ENABLE_*`；只有总开关或只有具体开关均不运行。
3. 台账将三项改为 `RESEARCH_ONLY`，并把 CMake source-list 隔离纳入 registry
   门禁。strict `nm -C` 不含三个入口符号；research 二进制包含。
4. `stage_fusion.cpp` 的 map/reduce mutation 继续由编译宏隔离；同 TU 的默认
   row-broadcast 具有独立 observer/effect/live-out/transpose-role 证明、事务
   verifier、RV64 admission 和近似负例。
5. simple RA 的紧邻完整宽度 spill/reload 在同一 frame location 时转发为物理
   register move。home store 保留，窄 load/store 和不同位置拒绝；真实第 9/10 个
   overflow stack argument 仍写入 `0(sp)/8(sp)`。

源码禁止模式扫描未发现 benchmark 标识符、`__FILE__`、源码 hash、审计模式、
输入文件或硬编码结果触发。源码中的 `program.sy` 仅存在于两处“禁止发射固定
`.file`”注释，不参与代码生成。

## 验证结果

- strict/research CMake Release：PASS。
- strict source/switch isolation：PASS；注入三个 enable 后汇编 byte-identical。
- research 无 opt-in、仅 `SYC_RESEARCH=1` 与 strict：byte-identical；完整 opt-in
  正例改变汇编。
- pass semantic registry：`9/9 PASS`。
- strict audit：`12/12 PASS`。
- compliance risks：PASS；另以
  `SYC_TEST_QUARANTINED_AST_PASSES=1` 完整执行隔离实现的正负 corpus，PASS。
- RV64 call argument copies：strict/research 均 PASS，包含真实 overflow stack
  argument 静态检查和 QEMU 输出。
- RV64 planning contracts：PASS。
- remaining pass semantics：PASS；使用正式 RISC-V 性能用例目录执行，覆盖模递推、
  overwrite、除数特化、row-broadcast 与 stencil 的正例/近似负例。
- 正式功能集：O0 `140/140 PASS`（12.5s），O1 `140/140 PASS`（50.9s）。
- `git diff --check`：PASS。

## 汇编影响与结论

相对整改前 strict compiler，正式性能集 `9/60 changed`：

```text
01_mm1 01_mm2 01_mm3
03_sort1 03_sort2 03_sort3
transpose0 transpose1 transpose2
```

变化来自三项算法级 pass 的提交隔离。本轮目标是合规整改，没有对 changed cases
执行动态性能，因此不声明新的内部 timer 收益或回退数值。此前这些 pass 的历史
收益不再属于提交 strict。

按当前可执行证据，PDF 第三条（1）名称/字符串触发、（2）硬编码结果、（3）输入
模式、（4）UB 利用和（5）未证明的特定输入/环境路径均未在 strict 提交配置中发现
未关闭项。技术委员会仍保留 PDF 第四条规定的最终认定权。
