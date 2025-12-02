# byyl 词法分析生成器（Qt+CMake）

- 环境：macOS，Qt 6.9.3，CMake 4.2.0，Ninja 1.13.2

## 快速开始（CMake 统一构建）
- 配置：`qt-cmake -S . -B dist -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- 编译：`cmake --build dist -j`
- 运行：`open dist/byyl.app` 或 `dist/byyl.app/Contents/MacOS/byyl`
- 测试：`ctest --test-dir dist -V`
- 说明：完整细节见下文“构建指南（CMake Only）”。

## 构建指南（CMake Only）
- 依赖准备：
  - 安装 Qt/CMake/Ninja：`brew install qt cmake ninja`
  - 如不使用 `qt-cmake`，为 `cmake` 指定 Qt 路径：`-DCMAKE_PREFIX_PATH=$(brew --prefix qt)`
- 配置（初次或变更后）：
  - `qt-cmake -S . -B dist -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
  - 可选：`ln -sf dist/compile_commands.json compile_commands.json`
- 编译：
  - `cmake --build dist -j`
- 运行 GUI：
  - `open dist/byyl.app`
  - 或 `dist/byyl.app/Contents/MacOS/byyl`
- 运行测试：
  - `ctest --test-dir dist -V`
- 清理/重建：
  - `rm -rf dist && qt-cmake -S . -B dist -G Ninja && cmake --build dist -j`
- 重要说明：
  - 本项目统一使用 CMake 构建（不使用 qmake）。

### 修改代码后的重新构建
- 常规代码改动（源文件/头文件）：
  - 运行 `cmake --build dist -j` 即可增量编译，Ninja 只会重新编译受影响的目标。
- 构建配置改动（`CMakeLists.txt`、新增/删除源文件、切换 Qt/CMake 版本）：
  - 先重新配置：`qt-cmake -S . -B dist -G Ninja`
  - 然后编译：`cmake --build dist -j`
- 运行与测试：
  - 运行 GUI：`open dist/byyl.app`
  - 运行测试：`ctest --test-dir dist -V`

### 编辑器诊断（可选但推荐）
- 为 clangd 生成并启用编译数据库（使 IDE 正确解析 Qt 头与 AUTOUIC 产物）：
  - `qt-cmake -S . -B dist -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
  - `ln -sf dist/compile_commands.json compile_commands.json`
- 当切换构建目录时（如 `build`），请同步更新上述链接。

## 功能概览
- 正则解析：支持连接、`|`、`*`、`+`、`?`、`[]`、`()`、`\` 转义、命名宏 `name = expr`
- 自动机转换：RE→NFA（Thompson）、NFA→DFA（子集构造）、DFA→MinDFA（Hopcroft）
- 表格展示：动态列头（字母表与 `#` 表示 ε），首列标记（`-` 初态、`+` 终态）
- 代码生成：从 MinDFA 生成 C/C++ 源码（方法二：Switch-Case）
- 生成代码持久化与复用：完整合并扫描器源码按时间戳+正则哈希保存至 `byyl.app/generated/lex`，并编译到 `byyl.app/generated/lex/bin`；在未更换正则表达式时，GUI 与测试复用当前生成文件
- 测试与验证：并行扫描所有 Token 规则，最长匹配 + 权重策略；跳过空白；按配置跳过注释/字符串（默认跳过 `//`、`/*...*/`、`#` 注释及 `'"`、`""`、`` `...${...}` `` 字符串；TINY 的 `{...}` 注释默认关闭）

## 正则规则约定与示例
- Token 命名约定：`_name<code>[S] = expr`
  - 以 `_` 开头表示 Token；`<code>` 为整数编码；可选 `S` 表示该 Token 为分组选项（按 `|` 展开）
  - 非 `_` 开头的规则为宏（可在表达式中引用），不会参与扫描输出
- 元字符与转义：解析器将 `| * + ? ( ) [ ]` 视为元字符；如需匹配其字面量，请使用反斜杠转义：`\|`、`\+`、`\*`、`\(`、`\)`、`\[`、`\]`
- 操作符 Token 示例：
  - 单行写法：
    - `_symbol220S = \+ | - | \* | / | % | & | \| | < | > | <= | >= | == | != | = | \+= | -= | \*= | /= | %= | &= | \^= | <<= | >>= | << | >> | \^`
  - 拆分宏写法（更易维护）：
    - `kw_ops1 = \+ | - | \* | / | % | & | \|`
    - `kw_ops2 = < | > | <= | >= | == | != | =`
    - `kw_ops3 = \+= | -= | \*= | /= | %= | &= | \^= | <<= | >>= | << | >> | \^`
    - `_symbol220S = kw_ops1 | kw_ops2 | kw_ops3`

## 错误分析与排查指引
- 未找到 Token 定义：
  - 原因：规则未按 `_name<code>[S] = expr` 命名，或全部是宏（不参与扫描）
  - 处理：至少添加一条以 `_` 开头且含编码的 Token 规则；例如：`_identifier100 = letter(letter|digit)*`
- 元字符未转义：
  - 现象：大量 `ERR` 或分组被错误拆分
  - 处理：将 `| * + ? ( ) [ ]` 作为字面量使用时加 `\` 转义；例如 `\|`、`\+`、`\*`、`\(`、`\)`、`\[`、`\]`
- 预处理行被跳过：
  - 现象：C/C++ 的 `#include` 行无输出
  - 处理：在 C/C++ 样例环境下关闭 `LEXER_SKIP_HASH_COMMENT`，或在操作符集中加入 `#`

## 界面使用
- 页签与控件（objectName）：
  - 正则编辑：`txtInputRegex`，`btnLoadRegex`，`btnStartConvert`
  - NFA/DFA/MinDFA 表：`tblNFA`，`tblDFA`，`tblMinDFA`
  - 代码查看：`txtGeneratedCode`，`btnGenCode`
  - 测试与验证：左侧 `txtSourceTiny`（标签：源程序输入），右侧 `txtLexResult`（标签：Token 编码输出），`btnRunLexer`
- 操作步骤：
    - 在“正则编辑”页点击“从文件加载”，选择正则文件（示例：`tests/test_data/regex/javascript.regex`）
  - 点击“转换”查看三阶段状态表
  - 切到“代码查看”，点击“生成代码”：显示完整合并扫描器源码，并保存到 `generated/lex/lex_<yyyyMMdd_HHmmss>_<hash12>.cpp`
  - 切到“测试与验证”，左侧输入源文本（示例片段，支持多语言），点击“运行词法分析”：若当前未有生成文件或正则更换，会先编译当前源码到 `generated/lex/bin/` 并执行；右侧展示 Token 编码输出
    - 若左侧为空，系统会尝试加载 `tests/test_data/sample/tiny/tiny1.tny`；仍为空则注入示例文本并提示状态栏
    - 也可点击“选择样例文件”从 `tests/test_data/sample/` 目录选择 JS（`javascript/`）、Python（`python/`）或 TINY（`tiny/`）的示例文件
  - 若输出包含 `ERR`，状态栏会提示“存在未识别的词法单元(ERR)，请检查正则与输入”

## 命令行测试
- 构建与运行全部测试：
  - `cmake --build dist -j`
  - `ctest --test-dir dist -V`
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
 - `app/ui/`：可复用 UI 组件（ToastManager/ToastWidget）
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
- `tests/test_data/sample/`：示例源代码（`javascript/`、`python/`、`tiny/`、`cpp/`、`java/`、`go/`、`rs/`）
- `tests/test_data/regex/`：供 GUI 选择加载的正则定义文件
- `generated/lex/`：保存生成的合并扫描器源码（时间戳+哈希命名）
- `generated/lex/bin/`：GUI 编译输出的可执行文件
- `CMakeLists.txt`：应用与测试目标定义

## 构建与运行
- 参考“构建指南（CMake Only）”。

## 常见问题
- 找不到 Qt：优先使用 `qt-cmake`，或为 `cmake` 添加 `-DCMAKE_PREFIX_PATH=$(brew --prefix qt)`
- 表格无内容：检查是否加载了有效的正则（至少包含一条以 `_` 开头的 Token 规则）
- 输出包含 `ERR`：检查关键字大小写、操作符转义（如 `\+`、`\*`、`\(`、`\)`），以及输入文本是否符合正则
- 关键字大小写：TINY 关键字大小写不敏感（无需写成 `[Ii][Ff]`），JS 关键字大小写敏感。

## 开发规范
- 日志：建议启用 `export QT_LOGGING_RULES="*.debug=true"`，在关键路径输出结构化日志
- 跨平台：路径用 `QDir/QFileInfo`，文件用 `QTextStream`；兼容 `\n/\r`；布局使用 `QLayout`；源码 UTF-8
- UI 自动化：关键控件均设置 `objectName`，`Qt Test` 通过 `findChild` 与 `QTest` 进行交互与断言
