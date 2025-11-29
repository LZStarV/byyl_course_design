## 问题复盘
- 当前解析器将未定义宏的词面（如 `if`、`==`、`===`、`=>`）当作“单一符号”处理，导致 NFA 边上出现“多字符标签”，而代码生成与运行只按单字符或类（letter/digit）匹配，造成几乎空输出。
- `CodeGenerator` 与 `Engine::run` 对多字符标签没有逐字符匹配逻辑，无法识别关键词与多字符运算符。

## 修复目标
- 解析器：将所有非元字符的词面按“逐字符串联”的 AST（Concat(Symbol('i'), Symbol('f'))）处理，宏引用仍保留为类（letter/digit）。
- 自动机：保持按“单字符或类”驱动的边；多字符运算符通过连续单字符边实现（如 `==` 变为 `'='`→`'='`）。
- 生成器与运行器：仅生成“单字符/类”的转移；去除对多字符标签的特殊分支。
- 测试：新增 Qt Test 用例，命令行跑同一 Engine 管线，输出与 UI 一致的表与结果。

## 实施方案
1. 解析器修正（RegexParser）
- 新增“词面分割”逻辑：
  - 在 `parseAtom/parseConcat` 中，连续的非元字符（且非宏名）按逐字符构造串联 AST；例如 `if`→`Concat(Symbol('i'), Symbol('f'))`，`==`→`Concat(Symbol('='), Symbol('='))`。
  - 保留 `letter`、`digit`、`[]` 字符集、转义 `\+`/`\*` 等现有能力。
- 遍历 AST 提取字母表时：仅加入单字符字母表与 `letter/digit` 类。

2. 自动机与运行器调整
- Thompson：保持对 `Symbol(ch)` 与 `CharSet`、`Ref(letter/digit)` 的现有构造；多字符词面已经在解析阶段展开，无需额外改动。
- CodeGenerator：
  - 删除对 `a.size()>1` 的分支；仅生成单字符匹配与 `isalpha/isdigit` 类匹配；
  - Switch-Case 的转移列表均为单字符或类。
- Engine::run：
  - 与生成器逻辑一致：按单字符与类驱动转移；
  - 继续跳过空白与 `{...}` 注释；当到达不可继续的字符时判断是否在 `accept` 状态，输出编码或错误码。

3. 命令行一致性测试（Qt Test）
- 新增 `tests/cli_regex_test.cpp`：
  - 读取 `tests/regex/javascript.regex`/`python.regex` 文件；
  - 调用 Engine 管线（lex→parse→NFA→DFA→MinDFA→generateCode），将三阶段表打印到 stdout（列头与内容）；
  - 从 `resources/sample.tny` 或自带 JS 片段中执行 `run`，打印编码序列；
  - 断言表格列数/行数大于 0、编码序列非空；
- CMake：增加测试目标与 `add_test`，可通过 `ctest -V` 查看输出。

4. 验证与回归
- 使用 `tests/regex/javascript.regex` 验证关键词与多字符运算符均能正确生成非空表与编码输出；
- 使用 `tests/regex/python.regex` 验证标识符、数字与关键字识别；
- 保持 UI 行为不变，命令行输出与 UI 状态表一致（同样的列头与状态内容）。

## 交付物
- 修复后的解析与生成逻辑代码；
- 新增 Qt Test 命令行测试文件与 CMake 测试目标；
- 在 README 中补充“命令行测试”章节与示例执行命令：`ctest --test-dir build-macos -V`、查看表/编码输出的指引。

## 风险与兼容
- 解析器改为按字符展开后，若输入包含未转义的运算符（如 `*`、`+`）仍按元字符处理（与要求一致）；
- 代码生成与运行器统一按单字符驱动，不会再出现多字符标签不匹配的问题；
- 该调整不会破坏 `letter/digit` 类与字符集。