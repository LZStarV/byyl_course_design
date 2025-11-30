# byyl 词法分析生成器（Qt+CMake）

- 环境：macOS，Qt 6.9.3，CMake 4.2.0，Ninja 1.13.2
- 构建：`qt-cmake -S . -B build-macos -G Ninja`，`cmake --build build-macos -j`
- 运行：`open build-macos/byyl.app` 或 `build-macos/byyl.app/Contents/MacOS/byyl`
- 测试：`ctest --test-dir build-macos -V`

## 功能概览
- 正则解析：支持连接、`|`、`*`、`+`、`?`、`[]`、`()`、`\` 转义、命名宏 `name = expr`
- 自动机转换：RE→NFA（Thompson）、NFA→DFA（子集构造）、DFA→MinDFA（Hopcroft）
- 表格展示：动态列头（字母表与 `#` 表示 ε），首列标记（`-` 初态、`+` 终态）
- 代码生成：从 MinDFA 生成 C/C++ 源码（方法二：Switch-Case）
- 生成代码持久化与复用：完整合并扫描器源码按时间戳+正则哈希保存至 `byyl.app/generated/lex`，并编译到 `byyl.app/generated/lex/bin`；在未更换正则表达式时，GUI 与测试复用当前生成文件
- 测试与验证：并行扫描所有 Token 规则，最长匹配 + 权重策略；跳过空白；按配置跳过注释/字符串（默认跳过 `//`、`/*...*/`、`#` 注释及 `'"`、`""`、`` `...${...}` `` 字符串；TINY 的 `{...}` 注释默认关闭）

## 界面使用
- 页签与控件（objectName）：
  - 正则编辑：`txtInputRegex`，`btnLoadRegex`，`btnStartConvert`
  - NFA/DFA/MinDFA 表：`tblNFA`，`tblDFA`，`tblMinDFA`
  - 代码查看：`txtGeneratedCode`，`btnGenCode`
  - 测试与验证：左侧 `txtSourceTiny`（标签：源程序输入），右侧 `txtLexResult`（标签：Token 编码输出），`btnRunLexer`
- 操作步骤：
  - 在“正则编辑”页点击“从文件加载”，选择正则文件（示例：`tests/regex/javascript.regex`）
  - 点击“转换”查看三阶段状态表
  - 切到“代码查看”，点击“生成代码”：显示完整合并扫描器源码，并保存到 `generated/lex/lex_<yyyyMMdd_HHmmss>_<hash12>.cpp`
  - 切到“测试与验证”，左侧输入源文本（示例片段，支持多语言），点击“运行词法分析”：若当前未有生成文件或正则更换，会先编译当前源码到 `generated/lex/bin/` 并执行；右侧展示 Token 编码输出
  - 若左侧为空，系统会尝试加载 `tests/sample/tiny/tiny1.tny`；仍为空则注入示例文本并提示状态栏
  - 也可点击“选择样例文件”从 `tests/sample/` 目录选择 JS（`javascript/`）、Python（`python/`）或 TINY（`tiny/`）的示例文件
  - 若输出包含 `ERR`，状态栏会提示“存在未识别的词法单元(ERR)，请检查正则与输入”

## 命令行测试
- 构建与运行全部测试：
  - `cmake --build build-macos -j`
  - `ctest --test-dir build-macos -V`
- 测试目标：
  - `GuiTest`：UI 自动化，验证关键控件与基本流程
  - `CliRegexTest`：命令行管线（lex→parse→NFA→DFA→MinDFA→runMultiple），严格断言无 `ERR`
  - `CodegenTest`：生成器输出的 C++ 源码可编译、可运行
- JS 用例示例（已在测试覆盖）：
  - 输入：`abc123 456 def789` → 输出无 `ERR`
  - 输入：`if return == var abc123` → 输出无 `ERR`
  - 合并输入行：`abc123 def456\nif return == var abc123` → 输出无 `ERR`

## 代码生成
- 生成方法：Switch-Case 状态机（方法二）
- 生成内容包含：`Judgechar`、`AcceptState`、`Step` 等函数，完整合并扫描器源码
- 在“代码查看”页点击“生成代码”，会显示完整源码并保存到 `byyl.app/generated/lex/lex_<yyyyMMdd_HHmmss>_<hash12>.cpp`
- GUI 会将该源码编译到 `byyl.app/generated/lex/bin/` 并在“测试与验证”页通过“运行词法分析”执行；未更换正则时复用同一份生成代码与二进制

## 配置
- 工程侧配置文件：`config/lexer.json`
  - `generated_output_dir`：生成代码保存目录，默认 `byyl.app/../../generated/lex`
  - `weight_tiers`：权重阈值数组，按优先级从高到低，例如：
    - `[{"min_code":220,"weight":3},{"min_code":200,"weight":4},{"min_code":100,"weight":1},{"min_code":0,"weight":0}]`
- 环境变量覆盖：
  - `BYYL_GEN_DIR`：覆盖生成代码保存目录
  - `LEXER_WEIGHTS`：生成器可执行程序读取的权重配置，格式 `min:weight` 逗号分隔，例如：`220:3,200:4,100:1,0:0`
  - 注释/字符串跳过：
    - `LEXER_SKIP_LINE_COMMENT`：`1`/`true` 开启 `//` 单行注释跳过（默认开启）
    - `LEXER_SKIP_BLOCK_COMMENT`：开启 `/*...*/` 块注释跳过（默认开启）
    - `LEXER_SKIP_HASH_COMMENT`：开启 `#` 行注释跳过（默认开启）
    - `LEXER_SKIP_SQ_STRING`：开启 `'...'` 字符串跳过（默认开启，含转义）
    - `LEXER_SKIP_DQ_STRING`：开启 `"..."` 字符串跳过（默认开启，含转义）
    - `LEXER_SKIP_TPL_STRING`：开启模板字符串 `` `...${...}` `` 跳过（默认开启，含嵌套插值与转义）
- 默认回退：若未提供配置或环境变量，系统按当前内置阈值与目录工作，确保行为不变。

## 目录结构
 - `app/`：主窗口与 UI（`mainwindow.h/.cpp/.ui`，`main.cpp`）
 - `app/app.pro`：qmake 项目文件（可选）
 - `src/`：核心逻辑（与 GUI 解耦）
  - `regex/`：`RegexLexer.*`，`RegexParser.*`
  - `automata/`：`Thompson.*`，`SubsetConstruction.*`，`Hopcroft.*`
  - `generator/`：`CodeGenerator.*`
  - `model/`：`Alphabet.h`，`Automata.h`
  - `Engine.*`：统一编排与运行器（构建所有规则的 MinDFA 并并行扫描）
- `tests/`：
  - `ui/auto_test_ui.cpp`（目标：`GuiTest`）
  - `cli/cli_regex_test.cpp`（目标：`CliRegexTest`）
  - `codegen/codegen_compile_run_test.cpp`（目标：`CodegenTest`）
  - `config/config_weight_test.cpp`（目标：`ConfigWeightTest`）
  - `config/gen_dir_config_test.cpp`（目标：`GenDirConfigTest`）
- `tests/sample/`：示例源代码（`javascript/`、`python/`、`tiny/`）
- `generated/lex/`：保存生成的合并扫描器源码（时间戳+哈希命名）
- `generated/lex/bin/`：GUI 编译输出的可执行文件
- `CMakeLists.txt`：应用与测试目标定义

## 构建与运行
- 配置：`qt-cmake -S . -B build-macos -G Ninja`
- 编译：`cmake --build build-macos -j`
- 运行 GUI：`open build-macos/byyl.app` 或 `build-macos/byyl.app/Contents/MacOS/byyl`
- 运行测试：`ctest --test-dir build-macos -V`

## 常见问题
- 找不到 Qt：优先使用 `qt-cmake`，或为 `cmake` 添加 `-DCMAKE_PREFIX_PATH=$(brew --prefix qt)`
- 表格无内容：检查是否加载了有效的正则（至少包含一条以 `_` 开头的 Token 规则）
- 输出包含 `ERR`：检查关键字大小写、操作符转义（如 `\+`、`\*`、`\(`、`\)`），以及输入文本是否符合正则
- 关键字大小写：TINY 关键字大小写不敏感（无需写成 `[Ii][Ff]`），JS 关键字大小写敏感。

## 开发规范
- 日志：建议启用 `export QT_LOGGING_RULES="*.debug=true"`，在关键路径输出结构化日志
- 跨平台：路径用 `QDir/QFileInfo`，文件用 `QTextStream`；兼容 `\n/\r`；布局使用 `QLayout`；源码 UTF-8
- UI 自动化：关键控件均设置 `objectName`，`Qt Test` 通过 `findChild` 与 `QTest` 进行交互与断言

## 代码风格与自动格式化
- 风格配置文件：项目根目录 `/.clang-format`
- 基础规范：Google C++ 风格，定制为：
  - 缩进：4 个空格（`IndentWidth: 4`）
  - 大括号：Allman 风格（`BreakBeforeBraces: Allman`）
  - 控制语句与括号之间保留空格（`SpaceBeforeParens: ControlStatements`）
  - 参数/模板不打包，按对齐换行（`BinPackArguments: false`，`BinPackParameters: false`）
  - 列宽限制 100（尽量减少无谓换行，`ColumnLimit: 100`）
  - 保持包含顺序（`SortIncludes: false`，`IncludeBlocks: Preserve`）
  - 注释不重排（`ReflowComments: false`），使用 Doxygen 规范
- 安装工具：
  - `brew install clang-format`
- 检查与格式化（排除构建产物与生成目录）：
  - 生成清单（macOS）：
    - ``find src app tests \( -type d -name "*_autogen" -o -type d -name "generated" -o -type d -name "byyl.app" \) -prune -o \( -type f \( -name "*.cpp" -o -name "*.h" \) -print \) > format_file_list.txt``
  - 干跑检查：
    - ``cat format_file_list.txt | xargs -I{} clang-format -n --Werror --style=file {}``
  - 执行格式化：
    - ``cat format_file_list.txt | xargs -I{} clang-format -i --style=file {}``
  - 完成后可删除 `format_file_list.txt`（仅为一次性清单），`/.clang-format` 请保留。
