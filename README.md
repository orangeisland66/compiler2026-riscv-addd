# Compiler2026-RISCV

2026 年全国大学生计算机系统能力大赛编译系统实现赛 —— RISC-V 赛道

- 队伍名称：啊对对对队
- 学校：武汉大学

## 项目简介

Compiler2026-RISCV 是一个使用 C++17 编写的 SysY2022 编译器，负责将
SysY2022 源程序编译为可链接、可运行的 RISC-V 64 位汇编代码。项目包含
从前端分析到后端代码生成的完整编译流程，并针对 RISC-V 平台进行了多级
中间表示优化、指令选择和寄存器分配。

编译器生成 RV64GC 汇编，支持较大地址空间下的 `medany` 代码模型。运行时库
由评测平台按 SysY 运行时规范提供，本仓库不将自定义运行时作为编译器提交内容。

## 编译流程

```text
SysY2022 源程序
        │
        ▼
词法分析、语法分析、语义分析与 AST
        │
        ▼
Analysis：控制流、数据流和类型信息
        │
        ▼
HIR → CoreIR → Rewrite：中间表示优化与变换
        │
        ▼
MIR：指令选择、地址模式和调用约定处理
        │
        ▼
寄存器分配与 RISC-V 汇编输出
```

## 主要功能

- SysY2022 词法、语法和语义分析；
- 常量、数组、函数、循环、递归和控制流处理；
- HIR、CoreIR 和 MIR 多级中间表示；
- 循环优化、递归优化、数组访问优化和数据流分析；
- RISC-V RV64GC 指令选择、调用约定处理和寄存器分配；
- `-O1` 性能编译模式。

## 优化设计

项目的默认优化围绕中间表示和目标代码特征展开，主要包括：

- 纯函数调用结果复用与局部公共表达式消除；
- 循环状态分析、循环展开和循环不变量处理；
- 归约拆分、数组递推和数组访问模式优化；
- 转置视图转发、全局数组死写删除和地址模式优化；
- 跨调用数组摘要、需求驱动复制、递归基数排序和递归累加器优化；
- MIR 阶段的指令清理、地址计算优化和寄存器相关优化。

部分实验性变换保留在独立的研究配置中，便于进行边界测试和性能对照；
默认构建使用稳定的主流水线。

## 评测接口

评测平台将编译器命名为 `compiler`，并使用以下命令：

```text
compiler -S -o testcase.s testcase.sy
compiler -S -o testcase.s testcase.sy -O1
```

第一条命令用于功能测试，第二条命令用于性能测试。输入和输出路径可以是
绝对路径。

生成的汇编由评测平台使用其 SysY 运行时环境链接。线上评测不依赖本仓库中的
运行时源码或静态库。

本地若要链接运行，可显式指定本地准备好的运行时归档：

```bash
riscv64-linux-gnu-gcc -march=rv64gc -mcmodel=medany -static \
  testcase.s <path-to-sysy-runtime.a> -lm -o testcase.elf
```

## 本地构建

推荐环境：

- CMake 3.20 或更高版本；
- 支持 C++17 的 C++ 编译器；
- `riscv64-linux-gnu-gcc` 和 `riscv64-linux-gnu-ar`；
- `qemu-riscv64` 或 `qemu-riscv64-static`。

仅构建编译器：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

如需本地 QEMU 回归，可额外提供仅保存在本地的运行时源码：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DSYC_LOCAL_RUNTIME_SOURCE=<path-to-local-sysy_runtime.c>
cmake --build build --parallel
```

构建结果：

- `build/compiler`：编译器可执行文件；
- 若配置了 `SYC_LOCAL_RUNTIME_SOURCE`，才会额外生成本地测试用的
  `build/libsysy_runtime.a`；该文件和对应源码不属于线上提交内容。

编译并运行单个程序：

```bash
./build/compiler -S -o testcase.s testcase.sy
riscv64-linux-gnu-gcc -march=rv64gc -mcmodel=medany -static \
  testcase.s <path-to-sysy-runtime.a> -lm -o testcase.elf
qemu-riscv64-static ./testcase.elf
```

## 测试与验证

仓库中的 `tests/riscv/` 保存语义边界、正例和负例夹具，`tests/tools/` 保存
相应的回归脚本。公开功能和性能用例可从比赛公开用例仓库获取，并通过
`--cases` 参数传给测试工具：

```bash
python3 tests/run_functional.py \
  --compiler ./build/compiler \
  --cases <公开功能用例目录> \
  --runtime <path-to-sysy-runtime.a>
```

## 项目结构

```text
.
├── CMakeLists.txt          # 本地构建入口
├── README.md               # 项目说明和使用方法
├── src/                    # 编译器实现
│   ├── frontend/           # 前端与 AST
│   ├── analysis/           # 分析模块
│   ├── hir/                # HIR 与高层优化
│   ├── core_ir/            # CoreIR 与中层优化
│   ├── rewrite/            # 重写和等价变换
│   ├── backend_mir/        # MIR 与指令选择
│   └── backend/            # RISC-V 后端
├── tests/                  # 语义夹具和测试脚本
├── tools/                  # 汇编、性能和回归分析工具
└── docs/                   # 技术文档和验证记录
```

技术文档索引见 [docs/README.md](docs/README.md)，其中包含 Markdown/PDF
双版本编译器设计说明和默认优化审计标准。
