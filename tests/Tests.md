# 测试说明文档

## 测试框架
- 采用 `QtTest` 作为统一测试框架，所有用例以 `QTEST_MAIN` 作为入口。
- 通过 `CMake` 注册测试可执行，并使用 `CTest` 统一运行与汇总结果。
- 集成测试开关：`BYYL_BUILD_INTEGRATION_TESTS`（默认开启）控制是否构建运行较重的集成测试（如代码生成与命令行正则流水线）。

## 目录架构
- `tests/automata/`：自动机核心测试（NFA/DFA/MinDFA）。
- `tests/lexer/`：组合词法与最长匹配测试。
- `tests/syntax/`：文法解析与语法分析（LL(1)、LR(0)、LR(1)、SLR）、语义树构建等。
- `tests/codegen/`：代码生成器编译与运行测试（集成）。
- `tests/cli/`：命令行风格的正则流水线测试（集成）。
- `tests/common/`：共享测试工具（`EnvGuard.h` 环境变量 RAII、`TestIO.h` 稳健读取方法）。
- `tests/test_data/`：测试数据集（正则规则、示例程序、语法文件、语义动作）。
- `tests/ui/`：GUI 自动化测试，验证关键控件加载与基本交互。

### 目录层级示意
```
tests/
  automata/
    fa_core_test.cpp
  lexer/
    longest_match_test.cpp
  syntax/
    grammar_parser_test.cpp
    ll1_test.cpp
    lr0_test.cpp
    slr_test.cpp
    lr1_test.cpp
    lr1_semantic_tree_example_test.cpp
    lr1_semantic_tree_full_test.cpp
  codegen/
    codegen_compile_run_test.cpp
  cli/
    cli_regex_test.cpp
  ui/
    auto_test_ui.cpp
  common/
    EnvGuard.h
    TestIO.h
  test_data/
    regex/ ...
    sample/ ...
    syntax/ ...
    semantic/ ...
```

## 测试流程
- 配置与构建：
  - `cmake -S . -B build -DBYYL_BUILD_INTEGRATION_TESTS=ON`
  - `cmake --build build -j`
- 运行所有测试：
  - `ctest --test-dir build --output-on-failure`
- 运行指定测试：
  - `ctest --test-dir build -R FA核心测试 --output-on-failure`
  - 支持多测试正则筛选：`ctest --test-dir build -V -R 'FA核心测试|LL1算法测试' --output-on-failure`
  - 直接运行测试二进制：`build/FACoreTest`
  - 先构建再运行某测试：`cmake --build build --target FACoreTest -j && build/FACoreTest`
- 关闭集成测试（仅跑快速单元）：
  - `cmake -S . -B build -DBYYL_BUILD_INTEGRATION_TESTS=OFF && cmake --build build && ctest --test-dir build`

## 测试项目罗列
- `FA核心测试` → 可执行 `FACoreTest`
- `最长匹配测试` → 可执行 `LongestMatchTest`
- `界面测试` → 可执行 `GuiTest`
- `文法解析测试` → 可执行 `GrammarParserTest`
- `LL1算法测试` → 可执行 `LL1Test`
- `LR0算法测试` → 可执行 `LR0Test`
- `SLR算法测试` → 可执行 `SLRTest`
- `LR1算法测试` → 可执行 `LR1Test`
- `语义树示例测试` → 可执行 `LR1SemanticTreeExampleTest`
- `语义树完整结构测试` → 可执行 `LR1SemanticTreeFullTest`
- `命令行正则测试` → 可执行 `CliRegexTest`
- `代码生成器测试` → 可执行 `CodegenTest`

## 测试原理（按类别）
- 自动机（FA）核心
  - 原理：从正则/AST 构造 NFA（Thompson），经子集构造得到 DFA，再经 Hopcroft 最小化得到 MinDFA；对结构不变性与接受性进行断言。
  - 关键断言：NFA ε 边存在且连通；DFA 无 ε 边且每状态对字母表至多一条转移；MinDFA 起始状态正确、仅包含可达状态。
- 组合词法与最长匹配
  - 原理：并行运行多个 MinDFA，计算各自 `matchLen` 与权重，选择最长且权重最高的 Token；支持关键字优先策略与跳过字符策略。
  - 关键断言：同起始位置不输出 `ERR`；关键字优先于标识符；映射后的终结符集合与配置一致。
- 文法与语法分析
  - LL(1)：`GrammarParser` 解析 BNF，`LL1::compute` 迭代计算 FIRST/FOLLOW，生成预测表并解析输入，覆盖 ε 产生式与二义性冲突记录。
  - LR(0)/SLR/LR(1)：构造项集 DFA 与 Action/Goto 表，断言闭包/GOTO 结果与表项有效，SLR 分类并报告冲突类型（shift/reduce、reduce/reduce）。
  - 语义树：在 LR(1) 解析成功的前提下，依据角色与策略构建 AST，验证多根聚合与单根选择的正确性。
- 代码生成（集成）
  - 原理：生成 C++ 扫描器源码并调用系统 `clang++` 编译运行；若编译器不可用用例跳过。
  - 关键断言：生成代码可编译、运行结果与 `Engine::runMultiple` 一致。

## 查看与定位测试结果
- 统一运行：`ctest --test-dir build --output-on-failure`（输出失败上下文）
- 精确筛选：`ctest --test-dir build -R LL1算法测试 --output-on-failure`
- 详细日志：`ctest --test-dir build -V -R 'FA核心测试|LL1算法测试'`
- 直接运行某测试：`cmake --build build --target LL1Test -j && build/LL1Test`
- GUI 结果查看：
  - 启动 `GuiTest` 后验证关键控件加载与交互；
  - <!--【GUI 截图】此处插入 “FIRST/FOLLOW 表格” 与 “Action/Goto 表格” 截图占位，用于人工验证。-->
- 生成物定位：代码生成测试的临时文件位于 `QDir::tempPath()` 的子目录，用例结束后清理；如需保留，请在本机复制相关文件。

### 参考输出文件
- 路径：`tests/test_output.txt`
- 内容：一次完整的 `ctest --test-dir build -V` 运行输出（包含各测试的输入、关键步骤与结果摘要）。
- 用途：
  - 作为基线对比，快速确认本地构建与测试是否符合预期；
  - 复核重点断言（如 `ERR数量`、集合与表项摘要、Totals 汇总）。
- 快速定位示例：
  - 查看失败或异常：`grep -n "FAIL\|ERR数量" tests/test_output.txt`
  - 查看某测试段落：`grep -n "LL1算法测试" -n tests/test_output.txt`
  - 总结用例统计：`grep -n "Totals:" tests/test_output.txt`

## 单元测试模板（三段式）
- 输入：明确给出文法/正则/样例串的来源与内容（优先内联或 `tests/test_data/`）。
- 预期结果：列出集合/表项/解析步骤的关键断言与示例条目。
- 测试结果：
  - 命令行：提供运行命令与输出摘要定位方法（`--output-on-failure`）。
  - GUI：提供截图占位与界面定位说明（如 FIRST/FOLLOW 两张表）。

### 示例 1（LL1）
- 输入：表达式文法（非终结符 `E/E'/T/T'/F`），`#` 表示 ε，`$` 表示结束符。
- 预期结果：`FIRST(F)={(,id}`，`FIRST(T')={*,#}`；`FOLLOW(E)={(,),$}`；预测表中 `TABLE[E'][+]` 指向 `E'→+ T E'`。
- 测试结果：运行 `LL1算法测试` 查看集合与预测表摘要；GUI 中检查 FIRST/FOLLOW 两张表。

### 示例 2（最长匹配）
- 输入：`keywords` 与 `identifier` 并行；样例串包含关键字前缀与标识符混合。
- 预期结果：同位置选择关键字优先；不输出 `ERR`。
- 测试结果：运行 `最长匹配测试`，观察控制台输出的 Token 序列与类型；必要时提升日志级别或增加示例串覆盖边界情况。

## 关键逻辑与覆盖点
- 自动机（FA）核心
  - 构造：`Engine::buildNFA`（Thompson），`Engine::buildDFA`（Subset），`Engine::buildMinDFA`（Hopcroft）。
  - 断言：NFA ε边存在、接受状态数量合理；DFA 无 ε 边且确定性；MinDFA 起始状态与可达性。
  - 接受性：使用 `CodeGenerator::generate` 配合 `Engine::run` 验证最小自动机可接受目标串。
- 组合词法与最长匹配
  - 构造多 Token（`identifier` 与 `keywords`）的 DFA 集合并运行 `Engine::runMultiple`。
  - 断言：最长匹配优先关键字，输出不包含 `ERR`。
  - 标识符映射：通过 `TokenMapBuilder::build` 校验映射后的终结符集合。
- 语法分析
  - LL(1)：`GrammarParser::parseString` + `LL1::compute` 生成预测表，`SyntaxParser::parseTokens` 进行解析；覆盖 ε 产生式解析。
  - LR(0)/SLR/LR(1)：构造项集与表（`LR0Builder`、`SLR::check`、`LR1Builder`），断言状态与表项有效。
  - LR(1) 语义树：`LR1Parser::parseWithSemantics` 在简化文法上构建 AST，覆盖正例（如 `assign-stmt`）与负例（缺符号）。
- 代码生成（集成）
  - `tests/codegen/codegen_compile_run_test` 使用临时目录调用系统 `clang++` 编译生成的扫描器，若编译器缺失则 `QSKIP`。

## 数据来源与读取策略
- 数据位置：`tests/test_data/regex`、`tests/test_data/sample`、`tests/test_data/syntax`、`tests/test_data/semantic`。
- 读取方法：
  - 统一使用 `tests/common/TestIO.h` 提供的 `testio_readAllAny` 与 `testio_readTestData`，自动尝试相对路径与可执行所在目录的组合，避免 `..` 脆弱路径。
- 只读约束：`tests/test_data/` 目录数据不参与修改；如需构造输入，优先使用内联样例字符串。

## 环境变量与隔离
- 使用 `tests/common/EnvGuard.h` 对 `LEXER_SKIP_*`、`BYYL_GEN_DIR`、`LEXER_WEIGHTS` 等环境变量进行 RAII 设置与恢复，避免跨用例污染。
- 代码生成器测试将临时文件写入 `QDir::tempPath()` 下的子目录，并在用例结束后清理。

## 测试范围与边界
- 聚焦主流程：正则→AST→NFA→DFA→MinDFA→代码生成与运行；文法→LL(1)/LR(1)→解析与语义树。
- 非主流程测试已移除：自动机 DOT 导出、端到端 TinyLR1 流程、配置持久化与生成目录覆盖、权重环境覆盖等。
- 平衡策略：
  - 单元测试覆盖算法与解析的核心不变性与结构断言。
  - 少量集成测试确保代码生成与运行链路正常，但提供开关与依赖检查以适配 CI 环境。

## 命名与约定
- 测试名称采用中文描述以便 `ctest -R` 精确筛选（例如：`FA核心测试`、`最长匹配测试`）。
- 所有用例采用小而稳的内联样例；涉及外部数据时统一通过 `TestIO` 读取。

## 常见问题
- 编译器缺失：`CodegenTest` 会在 `clang++` 不可用时跳过；如需强制运行，请安装或在本机提供编译器。
- 平台差异：`QtTest` 与 `CTest` 在不同平台输出略有差异，不影响断言逻辑。
- 失败定位：启用 `--output-on-failure` 获取失败上下文；对于 LR(1) 负例，用例断言仅检查 `errorPos >= 0` 而非具体数值。
