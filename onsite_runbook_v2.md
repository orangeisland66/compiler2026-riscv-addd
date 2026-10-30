# RISC-V 决赛现场操作手册 v1

适用仓库：决赛机上的实际仓库检出目录（下文用 `REPO` 表示）  
预期保底：`master-sysY2026` @ `462a17835dfddb9afd0d452c0480ef38a96f4ea9`

标记：**CONFIRMED** = 本轮从源码/命令确认；**INFERENCE** = 按真实调用链给出的排查顺序；**UNKNOWN/UNVERIFIED** = 当前机器未能确认。

## 0. 先记住

1. **CONFIRMED**：公开 CLI 只有 `compiler -S -o output.s input.sy`，以及附加 `-O1`；`-O2`、`-O3`、`--legacy-backend`、`--mir-backend` 在 `src/main.cpp:parseArgs` 被拒绝。
2. **CONFIRMED**：Lexer/Parser 后先经过 `Sema::analyze`；无 `-O1` 走 `src/backend/codegen.cpp`，`-O1` 走 AST/HIR -> CoreIR -> passes -> MIR -> RA/frame/assembly。
3. **CONFIRMED**：默认 strict CMake 构建排除 research sources；不要用 research 构建作为决赛保底。
4. **INFERENCE**：先比较 O0/O1 的 `.s`，找到最后一个仍正确的层，再定位源码。
5. **UNKNOWN**：现场 runtime、suite 绝对路径可能不同；命令中的 `<...>` 必须替换。

## 1. Git 生存操作

```bash
REPO=/path/to/compiler2026-riscv
BASELINE_REF=462a17835dfddb9afd0d452c0480ef38a96f4ea9
cd "$REPO"
git branch --show-current
git rev-parse HEAD
git status --short --branch
git log --oneline --decorate -12
```

分支名以现场实际提交分支为准；保底身份以完整 commit SHA 为准。`status --short` 没有后续文件行才是 clean，并且必须同时检查未跟踪文件。

确认保底锚点未被污染：

```bash
git show --no-patch --format='%H%n%D%n%s' "$BASELINE_REF"
git diff --exit-code "$BASELINE_REF" --
git status --porcelain=v1 --untracked-files=all
```

查看自己的改动：

```bash
git status --short
git diff --stat
git diff -- path/to/file.cpp
git diff --cached -- path/to/file.cpp
git diff HEAD -- path/to/file.cpp
```

新建/切换分支（只在工作区 clean 或已安全保存后执行）：

```bash
git status --short
git switch master-sysY2026
git switch -c exp/<short-name>
git switch <existing-branch>
git branch --list
```

安全保留当前工作：优先新建实验分支；临时保存可用：

```bash
git stash push -u -m 'before <experiment>'
git stash list
git stash show --stat stash@{0}
git stash apply stash@{0}   # 应用但保留 stash
git stash pop               # 应用并删除，可能冲突
```

**DANGER：现场主流程不使用 `git restore --worktree`、`git reset --hard` 或其他整树回滚命令。** 未提交内容可能无法恢复；如果确定要放弃改动，必须先完成 commit、补丁备份或 `git stash push -u`，再对精确文件操作。

标准 commit 流程：

```bash
git status --short
git diff --check
git diff -- path/to/file.cpp
git add path/to/file.cpp
git diff --cached --check
git diff --cached --stat
git commit -m '简短说明一个完整改动'
git status --short --branch
git log -1 --oneline --decorate
```

将当前提交强制上传为远端 `origin/master`：

**DANGER：下面的 `git push --force` 会直接改写远端 `master` 历史。必须确认目标就是 `origin/master`，且不需要保留该远端分支的现有提交。未提交的工作区内容不会被上传。**

```bash
git remote get-url origin
git branch --show-current
git rev-parse HEAD
git status --short --branch
git log -1 --oneline --decorate
git ls-remote --heads origin refs/heads/master

git push --force origin HEAD:refs/heads/master

git ls-remote --heads origin refs/heads/master
```

推送后最后一条输出的 SHA 应与推送前 `git rev-parse HEAD` 完全一致；若不一致，立即停止后续交付操作并核对远端。

比较两个 commit/branch、追文件历史、找行作者：

```bash
git diff <commit-A>..<commit-B> -- src/backend_mir
git diff master-sysY2026..exp/<short-name> --stat
git log --follow --oneline -- src/backend_mir/linear_scan.cpp
git log -p -- src/backend_mir/linear_scan.cpp
git blame -L 1087,1148 src/backend_mir/linear_scan.cpp
```

交付前再次检查当前分支、完整 SHA、clean 状态和与锚点的 diff。实验只放 `exp/...`；不要在 dirty tree 上宣称 baseline。

## 2. 准确 Build / Test 流程

### Build

**CONFIRMED**：`CMakeLists.txt` 使用 C++17、CMake >= 3.20，默认编译选项含 `-O2`；默认不配置 runtime 时只构建 compiler。

```bash
cmake -S . -B build-local -DCMAKE_BUILD_TYPE=Release
cmake --build build-local --parallel
test -x build-local/compiler && realpath build-local/compiler
```

如要本地 QEMU 回归，`CMakeLists.txt` 的真实选项是：

```bash
cmake -S . -B build-local -DCMAKE_BUILD_TYPE=Release \
  -DSYC_LOCAL_RUNTIME_SOURCE=/path/to/sysy_runtime.c
cmake --build build-local --parallel
```

这会产生 `build-local/libsysy_runtime.a`（仅本地测试）。为避免旧 CMake 缓存和权限污染，现场优先使用全新的 `build-local/`；若源码树下不可写，可改用明确的 `/tmp` 构建目录：

```bash
cmake -S . -B /tmp/compiler2026-riscv-clean-build \
  -DCMAKE_BUILD_TYPE=Release \
  -DSYC_LOCAL_RUNTIME_SOURCE=/home/mowang/riscv-experiment-assets/runtime/sysy_runtime.c
cmake --build /tmp/compiler2026-riscv-clean-build --parallel 2
```

### 编译、链接、执行

```bash
COMP=./build-local/compiler
SRC=/path/to/testcase.sy
RUNTIME=/path/to/libsysy_runtime.a

$COMP -S -o /tmp/testcase-O0.s "$SRC" \
  > /tmp/testcase-O0.compile.stdout 2> /tmp/testcase-O0.compile.stderr
$COMP -S -O1 -o /tmp/testcase-O1.s "$SRC" \
  > /tmp/testcase-O1.compile.stdout 2> /tmp/testcase-O1.compile.stderr

riscv64-linux-gnu-gcc -march=rv64gc -mcmodel=medany -static \
  -o /tmp/testcase-O1.elf /tmp/testcase-O1.s "$RUNTIME" -lm \
  > /tmp/testcase-O1.link.stdout 2> /tmp/testcase-O1.link.stderr
qemu-riscv64 /tmp/testcase-O1.elf \
  > /tmp/testcase-O1.run.stdout 2> /tmp/testcase-O1.run.stderr
RUN_RC=$?
{ cat /tmp/testcase-O1.run.stdout; printf '%s\n' "$RUN_RC"; } \
  > /tmp/testcase-O1.actual
```

有输入时：`qemu-riscv64 /tmp/testcase-O1.elf < testcase.in`，并同样保存返回码。性能 runner 使用 `-march=rv64gc -mcmodel=medany -static ... runtime -lm`；功能 runner 当前不显式添加 `-lm`，手工链接时可保留 `-lm`。

reference `.out` 的最小比较（stdout 后必须追加返回码，与 runner 一致）：

```bash
diff -u <(sed 's/[[:space:]]*$//' testcase.out) \
        <(sed 's/[[:space:]]*$//' /tmp/testcase-O1.actual)
```

### Runner

已实际执行过帮助：

```bash
python3 tests/run_functional.py --help
python3 tests/run_performance.py --help
python3 tools/riscv_suite_runner.py --help
```

`<suite-root>` 必须包含：

```text
<suite-root>/2026初赛RISCV赛道功能用例/functional
<suite-root>/2026初赛RISCV赛道功能用例/h_functional
<suite-root>/2026初赛RISCV赛道性能用例/performance
```

功能 O0（省略 `--opt`）：

```bash
python3 tests/run_functional.py --compiler ./build-local/compiler \
  --cases <suite-root>/2026初赛RISCV赛道功能用例 \
  --runtime /path/to/libsysy_runtime.a --work-dir /tmp/func-O0
```

功能 O1：同命令加 `--opt`。单 testcase：加 `--filter 'case-fragment'`，也可加 `--verbose`。

性能脚本要求显式 `--opt`，并在内部固定传 `compiler -S -O1`：

```bash
python3 tests/run_performance.py --compiler ./build-local/compiler \
  --cases <suite-root>/2026初赛RISCV赛道性能用例/performance \
  --runtime /path/to/libsysy_runtime.a --opt \
  --work-dir /tmp/perf-work --report /tmp/perf-report \
  --runs 1 --jobs 1
```

单 testcase 再加 `--filter 'case-fragment'`。

固定全量 runner 的 functional/performance/full 命令：

```bash
python3 tools/riscv_suite_runner.py --suite-root <suite-root> \
  --build ./build-local --runtime /path/to/libsysy_runtime.a \
  --out-dir /tmp/suite-functional --category functional
python3 tools/riscv_suite_runner.py --suite-root <suite-root> \
  --build ./build-local --runtime /path/to/libsysy_runtime.a \
  --out-dir /tmp/suite-performance --category performance
python3 tools/riscv_suite_runner.py --suite-root <suite-root> \
  --build ./build-local --runtime /path/to/libsysy_runtime.a \
  --out-dir /tmp/suite-all --category all
```

runner 的 `--filter` 是逗号分隔的 testcase stem 子串；它会写 `summary.json`、`cases.csv`、`asm/`，不要把这些测试产物放进源码提交。

## 3. 源码地图：问题 -> 真实入口

统一搜索：`rg -n '关键词' src tests tools`。

| 类别 | 文件 | 重要 class/function | 推荐搜索 | 在 pipeline 中的职责 |
|---|---|---|---|---|
| A CLI | `src/main.cpp` | `parseArgs`, `main` | `rg -n 'parseArgs|unsupported CLI|runMIRBackend|CodeGen' src/main.cpp` | 读源、创建 Lexer/Parser、O0/O1 分叉 |
| B lexer | `src/frontend/lexer.h/.cpp` | `Lexer::next`, `peek`, `readToken`, `readNumber`, `readIdOrKw`, `skipWS` | `rg -n 'readToken|readIdOrKw|enum class TK' src/frontend` | 文本 -> token |
| C parser | `src/frontend/parser.h/.cpp` | `parseCompUnit`, `parseTopLevel`, `parseFuncDef`, `parseStmt`, `parseExpr`, `expect` | `rg -n 'parseCompUnit|parseStmt|parse[A-Z]|expect\(' src/frontend/parser.*` | token -> AST；语法错误看 `Parser::error` |
| D AST | `src/frontend/ast.h` | `CompUnit`, `FuncDef`, `BlockStmt`, `BinaryExpr`, `LValExpr`, `CallExpr` | `rg -n 'struct (CompUnit|FuncDef|BinaryExpr|LValExpr)' src/frontend/ast.h` | 前端树节点 |
| E 类型/语义 | `src/frontend/sema.h/.cpp`, `src/frontend/ast.h`, `src/core_ir/lowering/ast_to_ir.cpp`, `src/backend/codegen.cpp` | `Sema::analyze`, `BaseType`, `SysType`, `ASTToIR::lower`, `CodeGen::lookupSym`, `genLValAddr` | `rg -n 'Sema::analyze|BaseType|ASTToIR::lower|lookupSym|genLValAddr' src` | Parser 后先做统一语义检查，再进入 O0/O1 分叉 |
| F AST/HIR 优化 | `src/hir/ast_simplify.cpp`, `src/hir/*.h` | `hir::simplifyAST`, `eliminateTailRecursion`, `normalizeRecursiveAccumulators` | `rg -n 'simplifyAST|eliminateTailRecursion|normalizeRecursiveAccumulators' src` | O1、ASTToIR 前 |
| G Value | `src/core_ir/ir/core_ir.h/.cpp` | `Value::addUse`, `replaceAllUsesWith`, `ConstInt`, `ConstFloat` | `rg -n 'class Value|replaceAllUsesWith|addUse' src/core_ir/ir` | 所有 CoreIR 值/use-def |
| G Instruction | 同上 | `Instruction::addOperand`, `setOperand`, `hasSideEffects`, `isTerminator` | `rg -n 'class Instruction|hasSideEffects|isTerminator' src/core_ir/ir` | 指令 |
| G BasicBlock | 同上 | `BasicBlock::pushBack`, `terminator`, `replaceSuccessor`, `replacePredecessor` | `rg -n 'class BasicBlock|replaceSuccessor|terminator' src/core_ir/ir` | CFG 基本块 |
| G Function | 同上 | `Function::addArg`, `newBlock`, `recomputeCFG`, `addAlloca` | `rg -n 'class Function|recomputeCFG|newBlock' src/core_ir/ir` | 函数级 IR |
| G Module | 同上 | `Module::newFunction`, `getFunction`, `newGlobal`, `declareBuiltin` | `rg -n 'class Module|newFunction|declareBuiltin' src/core_ir/ir` | 模块/内建声明 |
| H pipeline | `src/core_ir/passes/pipeline_builder.cpp`, `passes.cpp`, `pass_manager.cpp` | `buildPipelinePlan`, `runCoreIROptimizations`, `runModulePass`, `ModulePassManager::run` | `rg -n 'runCoreIROptimizations|ModulePassSpec|runModulePass|enable[A-Z]' src/core_ir/passes` | O1 pass 编排/分析失效 |
| I const fold | `src/core_ir/passes/const_fold.cpp/.h` | `constFold`, `ConstFoldPass` | `rg -n 'constFold|ConstFoldPass' src/core_ir/passes` | 常量折叠 |
| J DCE | `src/core_ir/passes/cleanup.cpp/.h` | `dce`, `eliminateUnreachableFunctions` | `rg -n '\bdce\b|eliminateUnreachable' src/core_ir/passes` | cleanup 中删除死代码 |
| K mem2reg | `src/core_ir/passes/mem2reg.cpp/.h` | `globalScalarMem2Reg`, `singleBlockScalarMem2Reg` | `rg -n 'globalScalarMem2Reg|singleBlockScalarMem2Reg|runMem2RegFixpoint' src/core_ir/passes` | alloca/load/store -> SSA |
| L SSA/PHI | `src/core_ir/passes/phi_utils.cpp/.h`, `mem2reg.cpp`, `core_ir.h` | `collectLeadingPhi`, `repairPhiIncoming`, `splitCriticalEdgeForPhi` | `rg -n 'Phi|PHI|repairPhiIncoming|splitCriticalEdge' src/core_ir` | 合流值；后端 RA 时边复制 |
| M CFG cleanup | `src/core_ir/passes/cleanup.cpp`, `simplify_cfg.cpp` | `simplifyCFG`, `mergeLinearBlocks`, `normalizePhiEdges` | `rg -n 'simplifyCFG|mergeLinearBlocks|normalizePhiEdges' src/core_ir/passes` | 分支折叠、线性块合并、修 PHI |
| N CSE/GVN | `src/core_ir/passes/cse.cpp/.h`, `gvn.cpp/.h` | `localLoadForwarding`, `dominatorTreeCSE`, `globalValueNumbering`, `basicBlockCSE` | `rg -n 'dominatorTreeCSE|globalValueNumbering|basicBlockCSE|loadCSEKey' src/core_ir/passes` | 表达式/内存版本复用 |
| O loop | `src/core_ir/passes/loop_basic.cpp`, `src/analysis/loop_info.cpp`, `src/hir` | `loopCanonicalize`, `loopLocalValueNumbering`, `loopInvariantCodeMotion`, `collectNaturalLoops`, `buildLoopForest` | `rg -n 'loopCanonicalize|collectNaturalLoops|buildLoopForest|loopInvariantCodeMotion' src` | 循环事实、规范化、LICM |
| P alias | `src/analysis/alias_analysis.cpp/.h`, `pointer_range.cpp` | `queryAlias`, `evaluatePointerRange`, `underlyingPointerObject` | `rg -n 'queryAlias|AliasKind|evaluatePointerRange' src/analysis` | 内存变换 legality |
| Q effect/mod-ref | `src/analysis/function_effects.cpp/.h`, `function_modref.cpp/.h`, `memory_ssa_lite.cpp` | `summarizeModuleFunctionEffects`, `analyzeFunction`, `queryCallModRef`, `buildMemorySSALite` | `rg -n 'summarizeModuleFunctionEffects|queryCallModRef|buildMemorySSALite|isPureCall' src/analysis` | call 读写/纯度/内存版本 |
| R range | `src/analysis/integer_facts.cpp/.h`, `pointer_range.cpp` | `evaluateIntegerFacts`, `rangeFacts`, `evaluatePointerRange` | `rg -n 'evaluateIntegerFacts|IntegerFacts|rangeFacts|PointerRange' src/analysis` | i32 值域/地址范围 |
| S CoreIR->MIR | `src/backend_mir/lower.cpp/.h`, `mir_backend.cpp` | `lowerToMIR`, `lowerForPlanning`, `runMIRBackend` | `rg -n 'lowerToMIR|lowerForPlanning|runMIRBackend' src/backend_mir` | IR 指令/CFG -> MachineModule |
| T MIR | `src/backend_mir/machine_ir.h/.cpp` | `MIOp`, `MOperand`, `MachineInst`, `MachineBasicBlock`, `MachineFunction`, `printAsm` | `rg -n 'enum class MIOp|struct MOperand|class MachineInst|printAsm' src/backend_mir` | 机器指令、operand、发射 |
| U liveness | `src/backend_mir/mir_liveness.cpp/.h` | `numberInstructions`, `computeLiveness`, `buildLiveIntervals`, `analyzeLiveness` | `rg -n 'analyzeLiveness|computeLiveness|LiveInterval|numberInstructions' src/backend_mir` | RA 活跃区间 |
| V linear scan RA | `src/backend_mir/linear_scan.cpp/.h` | `LinearScan::allocate`, `runLinearScanRA` | `rg -n 'class LinearScan|runLinearScanRA|resolveParallelCopies' src/backend_mir/linear_scan.cpp` | vreg -> physreg；call/PHI copy |
| W spill/reload | `src/backend_mir/linear_scan.cpp` | `makeSpill`, `makeReload`, `rewriteBlock`, `spillSlot` | `rg -n 'makeSpill|makeReload|rewriteBlock|SPILL|RELOAD' src/backend_mir` | 溢出到栈槽 |
| X frame/ABI | `src/backend_mir/prologue.cpp/.h`, `lower.cpp` | `lowerFrame`, `assignABIArgLayout`, `addCallArgUses`, `addCallClobberDefs` | `rg -n 'lowerFrame|assignABIArgLayout|addCallArgUses|calleeSaved|frameSize' src/backend_mir` | RV64GC psABI、sp 对齐、序言尾声 |
| Y scheduling | `src/backend_mir/schedule.cpp/.h` | `runListSchedule`, `runBlockLayout` | `rg -n 'runListSchedule|runBlockLayout' src/backend_mir/schedule.cpp` | RA 后调度/布局 |
| Z peephole | `src/backend_mir/peephole.cpp/.h` | `runPeephole` | `rg -n 'runPeephole|fallthrough|redundant' src/backend_mir/peephole.cpp` | 局部冗余清理 |
| AA branch/layout | `src/backend_mir/schedule.cpp`, `loop_branch_rotate.cpp`, `machine_cleanup.cpp` | `runBlockLayout`, `runLoopBranchRotate`, `runMachineCleanup` | `rg -n 'runBlockLayout|runLoopBranchRotate|cleanupFunction' src/backend_mir` | CFG 形状、分支、落空路径 |
| AB asm | `src/backend_mir/machine_ir.cpp`, `mir_backend.cpp`, `backend/codegen.cpp` | `printAsm`, `emitChecked`, `CodeGen::emit` | `rg -n 'printAsm|emitChecked|\.text|\.data' src/backend_mir src/backend` | `.s` 输出 |
| AC verifier | `src/core_ir/passes/verify.cpp`, `src/backend_mir/mir_backend.cpp` | `verifyModuleChecked`, `verifyModule`, `verifyMachineModule`, `emitChecked` | `rg -n 'verifyModule|verifyMachineModule|emitChecked|reasonCode' src` | IR/MIR CFG、use-def、reg/frame 合法性 |
| AD runner | `tests/run_functional.py`, `tests/run_performance.py`, `tools/riscv_suite_runner.py` | `find_cases`, `run_one`, `run_case`, `main` | `rg -n 'find_cases|run_one|run_case|--category|--filter' tests tools` | compile/link/QEMU/.out |

**RVV/vector：** `src/backend_mir/rv64_cost_model.h` 的 `RV64TargetInfo::hasV=false`，`MIOp` 是标量 RV64GC/单精度浮点；搜索 `rg -n 'vector|RVV|hasV|v4i32|v4f32' src`，不要猜测 V 扩展。

## 4. 现场 decision tree

### 新增关键字/语法

新语法最小 `.sy` -> `compiler -S -o /tmp/x.s x.sy 2>/tmp/x.err` -> `lexer.cpp:readIdOrKw/readToken` -> `parser.cpp:parse*` -> `ast.h` 节点 -> `Sema::analyze` -> `ast_to_ir.cpp` 或 `CodeGen`。诊断错误先看 Lexer/Parser/Sema；合法程序的生成错误再区分 O0 直接后端和 O1 CoreIR/MIR。

### 能编译但答案错

同一输入生成 O0/O1 两份 `.s`，分别静态链接、QEMU，并保存 stdout/stderr。O0 错先查 `CodeGen::genExpr/genStmt/genCall/genLValAddr`；O0 对而 O1 错，先查 `verifyModule`、Phi、alias/effect、MIR verifier。只在 call/循环/大数组错，优先查 `assignABIArgLayout`、`resolveParallelCopies`、`lowerFrame`、spill/reload。

### O0 对、O1 错

先确认命令真的含 `-O1`；用 `SYC_DUMP_ANALYSIS=1`、`COREIR_DUMP=1` 做诊断（不要把环境开关当修复），按 HIR -> CoreIR -> MIR 顺序二分；优先 `runCoreIROptimizations` 内最近变化的 pass 和 `verifyModule`。

### `.s` 汇编不过

```bash
riscv64-linux-gnu-gcc -march=rv64gc -mcmodel=medany -c /tmp/x.s -o /tmp/x.o 2>/tmp/x.as.err
```

看报错行和前后指令；查 `machine_ir.cpp:printAsm`、`emitChecked`、`lowerFrame`、`MIOp` operand class、`lower.cpp:fitsSigned12`。大于 signed 12-bit 的偏移查 `emitAddrAdd`/frame materialization。

### 能汇编但 QEMU crash

看非零返回和 `run.stderr`；先查 `prologue.cpp:lowerFrame`（ra/s*、sp 16 对齐）、`lower.cpp:assignABIArgLayout`，再查 `linear_scan.cpp:resolveCallArgCopies/resolveBranchPhis`。

### 功能全对但性能慢

```bash
python3 tests/run_performance.py --compiler ./build-local/compiler \
  --cases <performance-dir> --runtime <runtime.a> --opt \
  --filter '<case-fragment>' --runs 1 --jobs 1
```

每次对比先记录 baseline/candidate 的分支和完整 SHA，先生成并比较汇编，只动态运行 changed cases，另加至少一个 byte-identical control。性能胜负只以 `internal_us` 和 `total_internal_us` 为主口径；`run_ms` 仅用于诊断 QEMU/宿主噪声，`compile_ms`、`link_ms` 和 `wall_s` 分开报告。日常默认只跑一轮，只有需要稳定性结论时才预先约定多轮。然后检查 `.s` 的 spill/reload、frame、循环 load/store、div/rem，依次查 CoreIR `loopInvariantCodeMotion/globalValueNumbering`、analysis `queryAlias/analyzeLoopDependences/function_effects`、MIR `runLinearScanRA/lowerFrame/runListSchedule/runPeephole/rv64_cost_model`。禁止 testcase 名称特判。

### 语言/目标/调用/数组

- 新语言功能：lexer -> parser -> AST -> `Sema` -> `ASTToIR` + `CodeGen` -> 两种后端 smoke。
- RISC-V 特性：先看 `rv64_cost_model.h`（RV64GC、`hasV=false`），再看 `MIOp`、`lower.cpp`、ABI/frame、`printAsm`。
- 调用参数错误：O0 `CodeGen::genCall`；O1 `assignABIArgLayout` + `addCallArgUses` + `resolveCallArgCopies`。
- 数组/指针错误：O0 `genLValAddr/genLValLoad`；O1 GEP/load/store lowering、`queryAlias/evaluatePointerRange`、MIR 地址 lowering；同时做 alias/边界负例。

## 5. rg 模板

```bash
rg -n '^[[:space:]]*(class|struct|enum)[[:space:]]' src
rg -n 'runCoreIROptimizations|buildPipelinePlan|runMIRBackend|runMachinePipeline' src
rg -n 'runModulePass\(|ModulePassSpec\{' src/core_ir/passes/passes.cpp
rg -n '\bMIOp::(PHI|SPILL|RELOAD|CALL|LW|SW|BEQ|J)\b' src
rg -n 'class (Value|Instruction|BasicBlock|Function|Module)|Value::|Instruction::|BasicBlock::|Function::|Module::' src/core_ir/ir
rg -n 'phi|Phi|PHI|repairPhiIncoming|resolveBranchPhis' src/core_ir src/backend_mir
rg -n 'spill|reload|SPILL|RELOAD|spillSlot' src/backend_mir
rg -n 'frame|FrameIdx|lowerFrame|prologue|epilogue|calleeSaved' src/backend_mir
rg -n 'alias|queryAlias|mayAlias|MustAlias|NoAlias' src/analysis src/core_ir/passes
rg -n 'effect|modref|ModRef|isPureCall|mayRead|mayWrite' src/analysis
rg -n 'vector|RVV|hasV|v4i32|v4f32' src
rg -n 'SYC_ENABLE_|SYC_SKIP_|SYC_RESEARCH|research' src CMakeLists.txt
rg -n 'Error:|expected |cannot open|unsupported CLI' src tests
rg -n 'Sema::analyze|Diagnostic|diagnostics\(' src/frontend src/main.cpp
rg -n 'run_functional|run_performance|riscv_suite_runner|--category|--filter' tests tools
```

## 6. 实际 pipeline 速查

无 `-O1`：

```text
.sy -> main::parseArgs -> Lexer::readToken/next -> Parser::parseCompUnit
 -> CompUnit (ast.h) -> Sema::analyze
 -> CodeGen::generate/genGlobal/genFunc/genStmt/genExpr
 -> RISC-V .text/.data/.rodata
```

`-O1`：

```text
.sy -> Lexer/Parser/AST -> Sema::analyze
 -> hir::simplifyAST + tail/recursive normalization
 -> ASTToIR::lower
 -> AnalysisManager
 -> buildPipelinePlan -> runCoreIROptimizations
 -> lowerToMIR
 -> runMachinePipeline:
    machine cleanup -> FP-hot schedule -> linear scan RA
    -> call/PHI parallel copies -> schedule -> block layout -> peephole
    -> lowerFrame -> verifyMachineModule -> printAsm
```

两条路径共享 Lexer/Parser/AST/Sema；只有语义检查通过后才在 `src/main.cpp` 的 `if (args.optimize)` 分叉。新增语言语义通常两条路径都要照顾；优化/RA/frame 修改只影响 O1，但必须保留 O0 作为对照。

## 7. 到机器前 15 分钟（最多 20 条）

1. 设置 `REPO`、`BASELINE_REF`、`RUNTIME`、`SUITE_ROOT`，然后 `cd "$REPO"`。
2. `git branch --show-current`，确认分支。
3. `git rev-parse HEAD`，记录 SHA。
4. `git status --short --branch`，必须 clean。
5. `git show --no-patch "$BASELINE_REF"`，并检查与锚点的 diff。
6. `command -v cmake riscv64-linux-gnu-gcc qemu-riscv64`。
7. 确认 runtime `.a` 存在。
8. 确认 suite 三层目录名正确。
9. 配置 Release build。
10. 构建并确认 `build-local/compiler`。
11. 一个 `.sy` 做 O0 `-S`。
12. 同一个 `.sy` 做 O1 `-S -O1`。
13. 用 `-march=rv64gc -mcmodel=medany -static` 链接。
14. QEMU smoke，保存 stdout/stderr/返回码。
15. 与对应 `.out` 比较。
16. `python3 tools/riscv_suite_runner.py --help`。
17. 先跑 functional 单 testcase。
18. 再跑完整 functional，查看 `summary.json`。
19. 用 `tests/run_performance.py --opt --filter ...` 跑单性能例。
20. 最后再次 `git status --short --branch`，不要把产物带进源码树。

## 8. 本轮验证与未确认项

已执行命令：

```text
git branch --show-current
git rev-parse HEAD
git status --short --branch
git log --oneline --decorate -12
python3 tools/riscv_suite_runner.py --help
python3 tests/run_functional.py --help
python3 tests/run_performance.py --help
cmake -S . -B /tmp/compiler2026-riscv-clean-build -DCMAKE_BUILD_TYPE=Release -DSYC_LOCAL_RUNTIME_SOURCE=/home/mowang/riscv-experiment-assets/runtime/sysy_runtime.c
cmake --build /tmp/compiler2026-riscv-clean-build --parallel 2
compiler -S -o ... tests/riscv/dead_global_array_positive.sy
compiler -S -O1 -o ... tests/riscv/dead_global_array_positive.sy
riscv64-linux-gnu-gcc -march=rv64gc -mcmodel=medany -static ...
qemu-riscv64 ...
```

O0/O1 smoke 的 QEMU stdout 都是 `7`，stderr 为空。当时原有 `build/` 直接增量构建因 glob 校验文件无法 touch 失败；这是当时机器的历史记录，不是决赛环境的通用结论。

无法确认：决赛机器是否有本机的 `/home/mowang/riscv-experiment-assets`；现场 runtime 归档的确切名字；完整 suite 本轮未执行；`qemu-riscv64-static` 本机未找到。当前工作树已有 `src/frontend/sema.h/.cpp`，交付前应再以最终 clean commit 核对本手册的 Sema 描述和验证记录。
