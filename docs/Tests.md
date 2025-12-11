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
