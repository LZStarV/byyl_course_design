# byyl 词法/语法分析器（Qt + CMake）

[![Ask Zread](https://img.shields.io/badge/Ask%20Zread-go-00b0aa?style=flat&labelColor=000000)](https://zread.ai/LZStarV/byyl_course_design) ![platform](https://img.shields.io/badge/platform-macOS%20%7C%20Windows-blue) ![Qt](https://img.shields.io/badge/Qt-6.x-41cd52) ![CMake](https://img.shields.io/badge/CMake-%E2%89%A5%203.21-064F8C) ![Ninja](https://img.shields.io/badge/Ninja-build-999999) ![Graphviz](https://img.shields.io/badge/Graphviz-dot-important)

- 环境：Qt 6.x（Widgets、Test），CMake ≥ 3.21，Ninja（macOS / Windows）
- 构建产物目录：`build`（统一使用 CMake / Ninja）

## 快速开始

依赖安装（示例）

> 推荐使用 macOS 使用以下命令行进行构建与运行；Windows 下推荐使用 Qt Creator 打开项目（选中 CMake 项目文件）。

- macOS：`brew install qt graphviz ninja`
- Windows：安装 Qt(含 MSVC)、Graphviz，并确保 `dot` 在 PATH

- 配置（任选其一）：
  - 使用 Qt 的 `qt-cmake`：`qt-cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
  - 使用标准 CMake（需提供 Qt 路径）：`cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=$(brew --prefix qt)`
  - 可选：关闭集成测试以加快构建：在配置命令追加 `-DBYYL_BUILD_INTEGRATION_TESTS=OFF`
- 编译（二选一）：
  - `cmake --build build -j`
  - `ninja -C build`
- 运行：
  - macOS：`open build/byyl.app` 或 `build/byyl.app/Contents/MacOS/byyl`
  - Windows：`build\\byyl.exe`
- 测试：
  - 全量测试：`ctest --test-dir build -V`
  - 仅运行某个测试：`ctest --test-dir build -V -R 'FA核心测试'`
  - 仅运行多个测试：`ctest --test-dir build -V -R 'FA核心测试|LL1算法测试'`
- 全量格式化：`find . -type f \( -name "*.h" -o -name "*.cpp" \) -not -path "./build/*" -not -path "./generated/*" -print0 | xargs -0 -n 50 clang-format -i -style=file`


## 功能
- 实验一（词法）：
  - 正则 → NFA（Thompson） → DFA（子集构造） → MinDFA（Hopcroft）；
  - 表格展示、DOT/PNG 导出与预览；
  - 合并扫描器代码生成与运行。
- 实验二（语法）：
  - BNF 文法解析（ε/EOF/增广后缀可配置）；
  - First / Follow 计算与表格展示；
  - LR(0) 项集 DFA 构造、预览与导出；
  - LR(1) 项集 DFA 构造与预览，Action/GOTO 表生成；
  - LR(1) 解析流程可视化（步骤、描述），冲突策略可配置（prefer_shift / prefer_reduce / error），支持“优先移进终结符列表”（如 `else`、`;`）
  - SLR(1) 检查与冲突明细；
  - 语义动作导入（偶数行文件：产生式行 + 动作行）；
  - 语义语法树 AST 预览/导出（DOT），支持文本树视图（Qt Tree）；
  - 语法分析器代码生成。

## 技术架构
- 核心逻辑（`src/`）：
  - `regex/`：正则词法规则解析（`RegexLexer`、`RegexParser`、`TokenHeaderParser`）。
  - `automata/`：NFA 构造（Thompson）、DFA 子集构造、MinDFA 最小化（Hopcroft）。
  - `syntax/`：文法与解析（`Grammar`、`LL1`、`LR0`、`LR1`、`SLR`、`SyntaxParser`、`LR1Parser`）；语义树与 DOT 导出。
  - `generator/`：词法与语法代码生成（`CodeGenerator`、`SyntaxCodeGenerator`）。
  - `visual/`：图导出（`DotExporter`），依赖 Graphviz `dot`。
  - `config/`：运行时配置加载与环境变量覆盖（`Config`）。
  - `Engine`：串联正则→自动机→词法/语法流程的引擎入口。
- UI（`app/`）：
  - 页面与组件（`pages/`、`components/`），控制器按功能分层（`controllers/`）。
  - 实验页签（`experiments/exp1` 与 `experiments/exp2`）覆盖编辑、表格、预览、代码生成与测试。
- 测试（`tests/`）：
  - 单元与集成测试覆盖自动机、词法、语法、代码生成与 UI；支持 `-DBYYL_BUILD_INTEGRATION_TESTS` 开关。

## 使用
### 词法页签（实验一）
- 步骤：
  - 在“正则编辑”页签，点击“加载”选择示例正则（如 `tests/test_data/regex/tiny.regex` 或 `minic.regex`）。
  - 点击“转换”，在“NFA/DFA/MinDFA”页签查看状态表与预览图；可用导出按钮生成 DOT/PNG。
  - 在“代码查看”页签查看组合词法分析器的生成代码（支持注释/字符串跳过与标识符词素输出）。
  - 在“测试与验证”页签，输入示例源文本（如 `tests/test_data/sample/tiny.txt` 或 `minic.txt`）并运行，得到 Token 序列；若匹配到 `identifier`、`number` 等，其源词素会紧随编码输出。
  - 如需调整权重或跳过策略，可在“设置”中修改或通过环境变量覆盖（见下文“环境变量速查”）。

### 语法页签（实验二）
- 步骤：
  - 在“文法编辑”页签，加载/解析示例文法（如 `tests/test_data/syntax/tiny.txt` 或 `minic.txt`）。
  - 在“First&Follow”页签，查看 First/Follow 计算与表格。
  - 在“LR”页签，查看 LR(0)/LR(1) 项集 DFA 与 Action/GOTO 表，必要时在“设置”中调整 LR(1) 冲突策略（`prefer_shift`/`prefer_reduce`/`error`）与“优先移进终结符列表”。
  - 在“LR(1)分析过程”页签：
    - 提供“正则表达式 + Token 序列 + 当前语义动作”，运行语法分析流程（步骤与描述可视化）。
    - 读取 Token 序列时，若 `identifier`、`number` 等后面紧随词素，将自动消费该词素（用于语义，不进入终结符流）。
  - 在“LR(1)语法树”页签，预览/导出语义树 DOT，或查看文本树视图（Qt Tree）。
  - 语义动作文件导入：每两行一对，第一行是产生式（如 `A -> ...`），第二行是角色位行（根/子/舍弃）；文件行数必须为偶数（参考 `tests/test_data/semantic/语义动作说明.md`）。

### 图导出与预览
- 所有自动机/语法页签均可通过“导出图”按钮生成 DOT 或直接渲染为 PNG 并预览。
- Graphviz：安装 `graphviz` 并确保 `dot` 在 PATH；渲染 DPI 与超时可在“设置”中调整（默认 DPI 150，超时 20000ms）。

### 代码生成与运行
- 词法代码：在实验一“代码查看”页签可导出组合词法分析器代码并运行；支持注释/字符串跳过、标识符词素输出与权重策略。
- 语法代码：在实验二“语法代码查看”页签可导出语法分析器代码；生成输出目录可通过“设置”或 `config/lexer.json` 覆盖。

### 最小示例（Tiny）
- 词法：加载 `tests/test_data/regex/tiny.regex` → 转换并预览 → 测试 `tests/test_data/sample/tiny.txt` → 获得 Token 序列。
- 语法：加载 `tests/test_data/syntax/tiny.txt` → 导入语义动作说明 → 在“LR(1)分析过程”运行 → 在“LR(1)语法树”预览/导出。

## 目录结构
- `app/`：主窗口与入口
  - `components/`：Toast、SettingsDialog、ImagePreviewDialog 等公共组件
  - `controllers/`：按功能分目录（Regex/Automata/Syntax/TestValidation/CodeView/Settings/Generator）
  - `services/`：`DotService`、`FileService`、`NotificationService`
  - `experiments/exp1/tabs/`：正则编辑、NFA/DFA/MinDFA、代码查看、测试与验证
  - `experiments/exp2/tabs/`：文法编辑、First&Follow、语法树、语法代码查看
  - `pages/`：页面容器（主页、实验一/二）
- `src/`：核心逻辑（regex/automata/syntax/generator/model/Engine/config/visual）
- `tests/`：UI、CLI、代码生成、配置、DOT 与语法模块测试
- 生成输出：
  - `generated/lex/`：缺省词法生成根目录（可通过配置覆盖）
  - `generated/lex/syntax/`：缺省语法生成目录（可通过配置覆盖）
  - `generated/lex/graphs/`：图导出根目录（可通过配置覆盖；语法图导出位于其 `syntax/` 子目录）

<details>
<summary>项目文件树（点击展开）</summary>

```text
byyl/
├─ app/
│  ├─ components/           # 通用 UI 组件（Toast、设置、图像预览、导出按钮等）
│  ├─ controllers/          # 控制器：Regex/Automata/Syntax/TestValidation/CodeView/Settings/Generator
│  ├─ experiments/
│  │  ├─ exp1/              # 词法实验页签：正则编辑、NFA/DFA/MinDFA、代码查看、测试与验证
│  │  └─ exp2/              # 语法实验页签：文法编辑、First&Follow、语法树、语法代码查看、LR(1)流程
│  ├─ pages/                # 主页与实验页面容器
│  ├─ main.cpp              # Qt 程序入口
│  ├─ mainwindow.*          # 主窗口逻辑与 UI（.ui）
│  └─ services/             # Dot/File/Notification 服务
├─ src/
│  ├─ regex/                # 正则词法解析：RegexLexer/Parser、TokenHeaderParser
│  ├─ automata/             # 自动机：Thompson、SubsetConstruction、Hopcroft
│  ├─ syntax/               # 文法与解析：Grammar、LL1/LR0/LR1/SLR、SyntaxParser/LR1Parser、AST
│  ├─ generator/            # 代码生成：CodeGenerator、SyntaxCodeGenerator
│  ├─ visual/               # DOT 导出：DotExporter
│  ├─ config/               # 运行时配置：Config
│  ├─ Engine.*              # 引擎入口：串联正则→自动机→词法/语法流程
│  └─ model/                # 基础数据结构（字母表、自动机模型）
├─ tests/
│  ├─ automata/
│  │  └─ fa_core_test.cpp              # 自动机核心：Thompson/DFA/MinDFA 流程与正确性
│  ├─ lexer/
│  │  └─ longest_match_test.cpp       # 词法最长匹配与权重策略验证
│  ├─ syntax/
│  │  ├─ grammar_parser_test.cpp      # 文法解析（BNF、ε/EOF/增广后缀配置）
│  │  ├─ ll1_test.cpp                 # LL(1) 构造与表验证
│  │  ├─ lr0_test.cpp                 # LR(0) 项集与 DFA 构造
│  │  ├─ lr1_test.cpp                 # LR(1) 项集、冲突策略（prefer_shift/reduce/error）
│  │  ├─ slr_test.cpp                 # SLR(1) 检查与冲突明细
│  │  ├─ syntax_parser_test.cpp       # 语法解析主流程
│  │  ├─ lr1_semantic_ast_build_test.cpp   # 语义树 AST 构建
│  │  ├─ lr1_semantic_tree_example_test.cpp# 语义树示例结构
│  │  └─ lr1_semantic_tree_full_test.cpp   # 语义树完整结构
│  ├─ codegen/
│  │  └─ codegen_compile_run_test.cpp # 词法代码生成后编译/运行验证
│  ├─ cli/
│  │  └─ cli_regex_test.cpp           # 命令行正则流程测试
│  └─ ui/
│     └─ auto_test_ui.cpp             # UI 自动化测试（Qt Test）
│  └─ test_data/            # 示例正则/语法与样例源文本/语义动作说明
├─ docs/
│  ├─ Settings.md           # 配置项详解
│  ├─ Tests.md              # 测试说明与常见问题
│  └─ COMMENTING_GUIDE.md   # 注释/提交规范
├─（运行时生成输出目录不在仓库根，默认位于应用目录下，如 macOS `byyl.app/generated/lex`）
├─ CMakeLists.txt           # 构建脚本（Qt6 Widgets/Test、测试目标）
├─ .clang-format / .clangd  # 代码风格与 IDE 配置
├─ README.md                # 项目说明（本文件）
├─ 编译原理课程设计任务书.pdf / 课程设计实验报告书.doc
└─ build/                   # 构建产物（不纳入提交）
```
</details>

### 关键文件与作用说明
- `app/main.cpp`：Qt 应用入口，初始化并启动主窗口。
- `app/mainwindow.ui` / `mainwindow.cpp`：主窗口 UI 与交互逻辑。
- `app/controllers/*`：按功能组织的控制器，驱动 UI 与核心逻辑的桥梁。
- `app/components/ExportGraphButton/*`：图导出按钮组件，调用服务生成 DOT/PNG。
- `app/services/DotService/DotService.cpp`：通过 Graphviz `dot` 将 DOT 渲染为 PNG（需 `dot` 在 PATH）。
- `src/Engine.*`：封装正则→自动机→词法解析与语法流程，提供统一入口。
- `src/regex/*`：解析正则表达式与 Token 头文件，驱动 NFA/DFA/MinDFA 构造的前置步骤。
- `src/automata/*`：自动机构造与最小化（Thompson/SubsetConstruction/Hopcroft）。
- `src/syntax/*`：文法解析、First/Follow、LR(0)/LR(1)/SLR 构造与解析、语义树生成、DOT 导出。
- `src/generator/CodeGenerator.cpp`：组合词法分析器代码生成，支持注释/字符串跳过、标识符词素输出与权重；环境变量：
  - `LEXER_SKIP_*`（`BRACE`/`LINE`/`HASH`/`BLOCK`/`SQ_STRING`/`DQ_STRING`/`TPL_STRING`）控制跳过策略。
  - `LEXER_WEIGHTS` 控制等长匹配的优先级权重（如 `220:3,200:4,100:1,0:0`）。
  - `BYYL_GEN_DIR` 输出根目录覆盖；`emit_identifier_lexeme` 等由 `config/lexer.json` 与环境变量合并。
- `src/visual/DotExporter.*`：根据自动机/文法生成 DOT 文本，支持宏/字符集聚合显示。
- `src/config/Config.*`：配置加载与保存，生成目录与 Graphviz、LR(1) 冲突策略等统一入口。
- `tests/*`：单元/集成测试目标（在 `CMakeLists.txt` 中注册），配合 `tests/test_data` 示例复现各流程。

### 数据与示例
- `tests/test_data/regex/*.regex`：示例正则集合（如 `minic.regex`、`tiny.regex`）。
- `tests/test_data/sample/*.txt`：示例源文本样本（minic/tiny）。
- `tests/test_data/syntax/*.txt`：示例文法（minic/tiny）。
- `tests/test_data/semantic/语义动作说明.md`：语义动作文件使用说明与示例。

## 设置
- 所有配置项可在应用菜单“设置”或 `config/lexer.json` 中管理，支持搜索路径、覆盖默认值与环境变量合并。
- 涵盖目录与输出、权重与跳过、词法与标识符、语法与算法（含 LR(1) 冲突策略与优先移进终结符）、表头与 DOT、Graphviz 与语义策略等。
- 详细配置项与示例说明见 `docs/Settings.md`。

## 测试目标
- 测试覆盖 UI 流程、词法流水线、自动机核心（NFA/DFA/MinDFA）、语法（LL(1)/LR(0)/LR(1)/SLR）与代码生成等主流程；支持按名称筛选与关闭集成测试的快速运行模式。
- 详细说明（目录结构、运行方式、覆盖点与常见问题）见 `docs/Tests.md`。

## 常见问题
- Qt 查找失败：为 `cmake` 添加 Qt 路径（macOS：`-DCMAKE_PREFIX_PATH=$(brew --prefix qt)`；Windows：指向 Qt 安装的 MSVC 目录）。
- Graphviz 渲染失败：安装 `graphviz`，降低 DPI 或导出 DOT 用外部工具查看。
