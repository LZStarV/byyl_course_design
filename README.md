[![zread](https://img.shields.io/badge/Ask_Zread-_.svg?style=flat&color=00b0aa&labelColor=000000&logo=data%3Aimage%2Fsvg%2Bxml%3Bbase64%2CPHN2ZyB3aWR0aD0iMTYiIGhlaWdodD0iMTYiIHZpZXdCb3g9IjAgMCAxNiAxNiIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTQuOTYxNTYgMS42MDAxSDIuMjQxNTZDMS44ODgxIDEuNjAwMSAxLjYwMTU2IDEuODg2NjQgMS42MDE1NiAyLjI0MDFWNC45NjAxQzEuNjAxNTYgNS4zMTM1NiAxLjg4ODEgNS42MDAxIDIuMjQxNTYgNS42MDAxSDQuOTYxNTZDNS4zMTUwMiA1LjYwMDEgNS42MDE1NiA1LjMxMzU2IDUuNjAxNTYgNC45NjAxVjIuMjQwMUM1LjYwMTU2IDEuODg2NjQgNS4zMTUwMiAxLjYwMDEgNC45NjE1NiAxLjYwMDFaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik00Ljk2MTU2IDEwLjM5OTlIMi4yNDE1NkMxLjg4ODEgMTAuMzk5OSAxLjYwMTU2IDEwLjY4NjQgMS42MDE1NiAxMS4wMzk5VjEzLjc1OTlDMS42MDE1NiAxNC4xMTM0IDEuODg4MSAxNC4zOTk5IDIuMjQxNTYgMTQuMzk5OUg0Ljk2MTU2QzUuMzE1MDIgMTQuMzk5OSA1LjYwMTU2IDE0LjExMzQgNS42MDE1NiAxMy43NTk5VjExLjAzOTlDNS42MDE1NiAxMC42ODY0IDUuMzE1MDIgMTAuMzk5OSA0Ljk2MTU2IDEwLjM5OTlaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik0xMy43NTg0IDEuNjAwMUgxMS4wMzg0QzEwLjY4NSAxLjYwMDEgMTAuMzk4NCAxLjg4NjY0IDEwLjM5ODQgMi4yNDAxVjQuOTYwMUMxMC4zOTg0IDUuMzEzNTYgMTAuNjg1IDUuNjAwMSAxMS4wMzg0IDUuNjAwMUgxMy43NTg0QzE0LjExMTkgNS42MDAxIDE0LjM5ODQgNS4zMTM1NiAxNC4zOTg0IDQuOTYwMVYyLjI0MDFDMTQuMzk4NCAxLjg4NjY0IDE0LjExMTkgMS42MDAxIDEzLjc1ODQgMS42MDAxWiIgZmlsbD0iI2ZmZiIvPgo8cGF0aCBkPSJNNCAxMkwxMiA0TDQgMTJaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik00IDEyTDEyIDQiIHN0cm9rZT0iI2ZmZiIgc3Ryb2tlLXdpZHRoPSIxLjUiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIvPgo8L3N2Zz4K&logoColor=ffffff)](https://zread.ai/LZStarV/byyl_course_design)

# byyl 词法/语法分析器（Qt+CMake）

- 环境：macOS，Qt 6.9.3，CMake 4.2.0，Ninja 1.13.2
- 特性：在实验一（正则→自动机→词法代码生成）基础上，增量集成实验二：LL(1) 语法分析器（BNF 解析、First/Follow、预测分析表、表驱动分析、AST 可视化、语法代码生成）。

## 快速开始（CMake 统一构建）

### macOS
- 配置：`qt-cmake -S . -B dist -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- 编译：`cmake --build dist -j`
- 运行：`open dist/byyl.app` 或 `dist/byyl.app/Contents/MacOS/byyl`
- 测试：`ctest --test-dir dist -V`

### Windows
- 配置：`qt-cmake -S . -B dist -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- 编译：`cmake --build dist -j`
- 运行：`dist\\byyl.exe`
- 测试：`ctest --test-dir dist -V`

- 说明：完整细节见下文“构建指南（CMake Only）”。

## 构建指南（CMake Only）

### 依赖准备

#### macOS
- 安装 Qt/CMake/Ninja：`brew install qt cmake ninja`
- Graphviz 用于 DOT 预览：`brew install graphviz`

#### Windows
- **Qt**：从 https://www.qt.io/download 下载并安装，选择最新的 LTS 版本，安装时确保勾选 CMake 和 Ninja
- **CMake**：从 https://cmake.org/download 下载 Windows 安装程序，安装时选择 "Add CMake to the system PATH for all users"
- **Ninja**：从 https://github.com/ninja-build/ninja/releases 下载 `ninja.exe`，将其复制到系统 PATH 中的目录（如 `C:\Windows\System32`）
- **Graphviz（可选）**：从 https://graphviz.org/download/ 下载 Windows 安装程序，安装时选择 "Add Graphviz to the system PATH for all users"

### 构建步骤

#### 配置（初次或变更后）
- `qt-cmake -S . -B dist -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- 可选（macOS）：`ln -sf dist/compile_commands.json compile_commands.json`
- 可选（Windows）：将 `dist\compile_commands.json` 复制到项目根目录，用于支持 clangd 等语言服务器

#### 编译
- `cmake --build dist -j`

#### 运行 GUI
- **macOS**：`open dist/byyl.app` 或 `dist/byyl.app/Contents/MacOS/byyl`
- **Windows**：`dist\\byyl.exe` 或双击 `dist\byyl.exe`

#### 运行测试
- `ctest --test-dir dist -V`

#### 清理/重建
- **macOS**：`rm -rf dist && qt-cmake -S . -B dist -G Ninja && cmake --build dist -j`
- **Windows**：删除 `dist` 目录，然后重新执行配置和编译命令

### 重要说明
- 本项目统一使用 CMake 构建（不使用 qmake）

### 修改代码后的重新构建
- 常规代码改动（源文件/头文件）：`cmake --build dist -j`
- 构建配置改动（`CMakeLists.txt` 等）：先 `qt-cmake` 重新配置，再编译。

### 编辑器诊断（推荐）
- 生成编译数据库：`qt-cmake -S . -B dist -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- 建立链接：
  - **macOS/Linux**：`ln -sf dist/compile_commands.json compile_commands.json`
  - **Windows**：`copy dist\compile_commands.json compile_commands.json /y`

## 功能概览
- 正则解析与自动机：RE→NFA（Thompson）、NFA→DFA（子集构造）、DFA→MinDFA（Hopcroft）；表格展示与 DOT 导出、PNG 预览；合并扫描器源码生成与复用。
- LL(1) 语法分析（实验二）：
  - 文法解析（BNF）：`A -> α | β`，`#` 表示 ε；支持行首 `//` 与 `#` 注释；非终结符为 `snake_case`（含下划线），终结符为不含下划线的词或符号。
  - First/Follow：迭代计算与 ε 传播；起始符 Follow 含 `$`。
  - 预测分析表：基于 First/Follow 构建；自动检测 `(非终结符, 终结符)` 冲突并提示非 LL(1)。
  - 表驱动分析与 AST：以测试页的 Token 序列为输入（来自词法分析输出 `txtLexResult`），构建分析栈并同步生成语法树。
  - AST 可视化：生成 Graphviz DOT 字符串并调用 `dot` 预览 PNG；支持导出 DOT 至 `generated/syntax/graphs/`。
  - 语法代码生成：生成包含预测表与表驱动循环的 C++ 源码至 `generated/syntax/syntax_parser.cpp`，在“代码查看”页签展示。

## 文法与数据格式
- 文法文件：纯文本，UTF-8；每行一条规则，如：`exp -> term exp_prime`；`|` 分隔产生式；`#` 代表 ε；行首 `//` 或 `#` 为注释。
- 非终结符：小写+下划线（如 `exp_prime`）；终结符：不含下划线的词或符号（如 `id`、`number`、`IF`、`+`）。
- DOT 导出：遵循 Graphviz DOT 规范，推荐 `rankdir=TB`（自顶向下布局）。
- 生成源码：C++17，遵循项目 `.clang-format`。

## 界面使用
- 页签与控件（objectName）：
  - 正则编辑：`txtInputRegex`，`btnLoadRegex`，`btnStartConvert`
  - NFA/DFA/MinDFA 表：`tblNFA`，`tblDFA`，`tblMinDFA`
  - DOT 导出与预览：`btnExportNFA`、`btnPreviewNFA`、`btnExportDFA`、`btnPreviewDFA`、`btnExportMin`、`btnPreviewMin`，分辨率输入 `edtGraphDpi*`
  - 代码查看（词法）：`txtGeneratedCode`，`btnGenCode`
  - 文法分析（语法）：`txtInputGrammar`，`btnLoadGrammar`，`btnParseGrammar`，`tblFirstSet`，`tblFollowSet`，`tblParsingTable`
  - 语法树：`viewSyntaxTree`，`btnExportSyntaxDot`，`btnPreviewSyntaxTree`
  - 测试与验证：左 `txtSourceTiny`（源程序），右 `txtLexResult`（Token 编码），`btnRunLexer`，`btnSaveLexResult`（保存结果），`btnSaveLexResultAs`（另存为）
- 操作步骤：
  - 词法：加载正则→转换查看状态表→生成合并扫描器→运行词法分析→在 `txtLexResult` 获取 Token 序列。
  - 语法：在“文法分析”页加载或输入 `.grammar/.txt` 文法→点击“解析文法并计算”生成 First/Follow 与预测表→在“语法树”页点击“运行语法分析”基于“测试与验证”页保存的 `txtLexResult` Token 输出构建语法树→预览或导出 DOT。
  - 代码查看：语法分析完成后自动生成 `generated/syntax/syntax_parser.cpp`，在“代码查看”页签的“语法分析器”子页查看。

## 命令行测试
- 构建与运行全部测试：`cmake --build dist -j`；`ctest --test-dir dist -V`
- 测试目标：
  - `GuiTest`：UI 自动化（关键控件与流程）
  - `CliRegexTest`：命令行管线（lex→parse→NFA→DFA→MinDFA→runMultiple），断言无 `ERR`
  - `CodegenTest`：生成器输出的 C++ 源码可编译可运行
  - `GrammarParserTest`：BNF 文法解析与注释处理
  - `LL1Test`：First/Follow 与 LL(1) 预测分析表构建

## 代码生成
- 词法：Switch-Case 状态机；点击“生成代码”后保存至 `generated/lex/lex_<yyyyMMdd_HHmmss>_<hash12>.cpp` 并编译到 `generated/lex/bin/`；在“测试与验证”页运行。
- 语法：生成基础表驱动源码至 `generated/syntax/syntax_parser.cpp`，在“代码查看”页展示。

## 配置
- 工程配置：`config/lexer.json`
  - `generated_output_dir`：生成目录根，默认 `byyl.app/../../generated/lex`（语法代码生成在该目录下的 `syntax/` 子目录）
  - `weight_tiers`：权重阈值数组，如 `[{"min_code":220,"weight":3}, ...]`
- 环境变量：
  - `BYYL_GEN_DIR`：覆盖生成目录根
  - `LEXER_WEIGHTS`：`min:weight` 逗号分隔，例如 `220:3,200:4,100:1,0:0`
  - 注释/字符串跳过：`LEXER_SKIP_*` 系列（`LINE/BLOCK/HASH/SQ/DQ/TPL`）

## 目录结构
- `app/`：主窗口与 UI（`mainwindow.*`，`main.cpp`）
- `app/ui/`：ToastManager/ToastWidget
- `app/pages/exp2/`：实验二页面（文法分析/语法树/代码查看）
- `src/`：核心逻辑（与 GUI 解耦）
  - `regex/`：`RegexLexer.*`，`RegexParser.*`
  - `automata/`：`Thompson.*`，`SubsetConstruction.*`，`Hopcroft.*`
  - `syntax/`：`Grammar.*`，`GrammarParser.*`，`LL1.*`，`SyntaxParser.*`，`AST.h`，`DotGenerator.*`
  - `generator/`：`CodeGenerator.*`，`SyntaxCodeGenerator.*`
  - `model/`：`Alphabet.h`，`Automata.h`
  - `Engine.*`：统一编排与运行器
- `tests/`：UI、CLI、代码生成、配置、DOT 与语法模块测试
- `tests/test_data/sample/`：示例源（`javascript/`、`python/`、`tiny/` 等）
- `tests/test_data/regex/`：正则定义
- `tests/test_data/syntax/`：TINY 文法样例
- `generated/lex/`：合并扫描器源码与 `bin/`
- `generated/syntax/`：语法分析器源码与 `graphs/`

## 常见问题

### 通用问题
- 找不到 Qt：优先使用 `qt-cmake`，或为 `cmake` 添加 Qt 安装路径：
  - **macOS**：`-DCMAKE_PREFIX_PATH=$(brew --prefix qt)`
  - **Windows**：`-DCMAKE_PREFIX_PATH="C:\Qt\<版本号>\msvc2019_64"`（根据实际安装路径调整）
- 文法解析失败或提示左递归：检查产生式是否存在直接左递归（`A -> A α`）；将文法改写为适用 LL(1) 的形式。
- 非 LL(1) 冲突：状态栏将提示冲突的非终结符与终结符，参考 First/Follow 修正文法。
- 语法错误：在“测试与验证”页确保先运行词法分析，`txtLexResult` 中的 Token 序列用于语法分析；若失败，请检查起始符与预测表项是否覆盖该输入。
- Graphviz 渲染失败：安装 `graphviz` 并确保 `dot` 在 PATH 中；降低 DPI 或导出 DOT 用外部工具查看。

### Windows 特有问题
- **找不到 qt-cmake 或 cmake**：确保 Qt 和 CMake 已正确安装，并已添加到系统 PATH 中。验证方法：在命令行中运行 `qt-cmake --version` 和 `cmake --version`。
- **构建失败，提示缺少头文件或库**：确保 Qt 安装时选择了正确的编译器组件（如 MSVC 2022 64-bit）。建议重新安装 Qt，确保勾选了与您系统编译器匹配的组件。
- **运行时缺少 DLL 文件**：
  1. 使用 Qt 提供的 `windeployqt` 工具：`windeployqt --release dist/byyl.exe`
  2. 或者将 Qt 安装目录下的 `bin` 文件夹添加到系统 PATH 中

## 开发规范
- 日志：
  - **macOS/Linux**：`export QT_LOGGING_RULES="*.debug=true"`
  - **Windows（CMD）**：`set QT_LOGGING_RULES=*.debug=true`
  - **Windows（PowerShell）**：`$env:QT_LOGGING_RULES="*.debug=true"`
- 跨平台：路径用 `QDir/QFileInfo`，文件用 `QTextStream`；布局使用 `QLayout`；源码 UTF-8
- UI 自动化：关键控件均设置 `objectName`，`Qt Test` 通过 `findChild` 与 `QTest` 交互断言
- 代码格式：使用 `.clang-format` 保持代码风格一致，建议安装 clang-format 插件到您的 IDE

### 开发建议
- **IDE 选择**：
  - 推荐使用 Qt Creator，提供了良好的跨平台开发体验
  - 也可使用 Visual Studio Code 配合 CMake 和 C++ 扩展
- **调试**：在 Qt Creator 中打开项目，可直接进行跨平台调试
- **测试**：定期运行测试套件，确保代码变更不会破坏现有功能
- **路径处理**：始终使用 Qt 提供的跨平台路径 API（如 `QDir`、`QFileInfo`），避免直接使用平台特定的路径分隔符

## DOT 格式规范（开发者）
- NFA：节点编号为状态 ID，接受态 `shape=doublecircle`；起始态通过隐含点 `__start -> <start>` 指示；ε 标签为 `ε`
- DFA：节点编号为状态 ID，`label` 显示对应 NFA 集合；转移标签按字母表符号输出（含 `letter`/`digit`）
- MinDFA：节点编号为代表 ID；接受态 `shape=doublecircle`；其余与 DFA 一致
- 所有标签进行基本转义（`"` 与 `\`），图布局 `rankdir=LR`
