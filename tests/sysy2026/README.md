# SysY2026 语义夹具

本目录覆盖当前已接入的 SysY2026 语义：char 与隐式提升/显式转换、do-while、
switch fallthrough、static 局部对象、字符串初始化、字符串常量下标、相邻字符串
拼接、printf，以及类型、const、break、case 和格式参数错误；表达式覆盖逻辑非
`!`、按位与 `&`、按位或 `|`、按位异或 `^` 和按位非 `~`，并按 C 的运算符
优先级解析。运行：

```bash
python3 tests/tools/test_sysy2026_semantics.py --compiler ./build-local/compiler
```

正例必须在 O0 和 O1 两条路径均生成汇编；错误程序必须返回非零、输出约定错误
码且不创建汇编文件。
