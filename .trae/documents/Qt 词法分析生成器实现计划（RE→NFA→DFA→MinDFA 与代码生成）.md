## 环境配置（macOS）
1. 安装基础工具
- 安装 Xcode 命令行工具：`xcode-select --install`
- 安装 Homebrew（若未安装）：`/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"`

2. 安装与配置 Qt/CMake/Ninja
- 安装 Qt 6（推荐 6.8.x）：`brew install qt`（或使用 Qt 官方 Online Installer 安装到 `/Applications/Qt/6.8.0/clang_64`）
- 安装 CMake/Ninja：`brew install cmake ninja`
- 环境变量（二选一）：
  - 使用 Homebrew Qt：`export CMAKE_PREFIX_PATH="$(brew --prefix qt)"`；`export PATH="$(brew --prefix qt)/bin:$PATH"`
  - 使用官方 Qt：`export CMAKE_PREFIX_PATH="/Applications/Qt/6.8.0/macos"`；`export PATH="/Applications/Qt/6.8.0/clang_64/bin:$PATH"`
- 验证：`qt-cmake --version`、`cmake --version`、`ninja --version`、`qmake -v`

3. 构建方式与现有工程兼容
- 现仓库为 qmake（`byyl_course_design_1.pro`）；为满足课程规范（Qt 6.x + CMake + Ninja），计划迁移到 CMake，同时保留 qmake 以便快速验证：
  - 新增根级 `CMakeLists.txt` 与 `src/` 目录组织；
  - 统一使用 `qt-cmake -S . -B build-macos -G Ninja` 构建；
  - 临时可用 `qmake && make` 仅用于现有骨架的连通性验证。

4. 日志与测试运行配置
- 启用日志：`export QT_LOGGING_RULES="*.debug=true"`
- 运行 GUI：`open build-macos/byyl.app` 或 `build-macos/byyl.app/Contents/MacOS/byyl`

---

## 项目总体架构
- `app/`：Qt Widgets GUI（主窗口、Tabs、表格与代码查看器）
- `core/`：与 GUI 解耦的核心库（单元测试直接链接）
  - `regex/`：`RegexLexer`、`RegexParser`（支持宏、字符集、转义、括号与运算符优先级）
  - `automata/`：`NFA`（Thompson）、`DFA`（Subset Construction）、`MinDFA`（Hopcroft）
  - `generator/`：`CodeGenerator`（从 MinDFA 生成 C/C++ 源码，选用方法二：Switch-Case 状态机）
  - `model/`：状态、边、字母表与转移矩阵的数据结构
- `tests/`：Qt Test 单元与 UI 自动化测试
- `resources/`：示例正则、TINY 测试用例、README、CMake 模板

---

## GUI 设计与交互
- 主窗口采用 Tab 结构：
  - Tab1：正则编辑（多行 `QTextEdit`，objectName=`txtInputRegex`；文件打开/保存）
  - Tab2：NFA 状态表（`QTableWidget`，objectName=`tblNFA`；动态列包含 `#` 表示 ε）
  - Tab3：DFA 状态表（`QTableWidget`，objectName=`tblDFA`）
  - Tab4：MinDFA 状态表（`QTableWidget`，objectName=`tblMinDFA`）
  - Tab5：代码查看（`QPlainTextEdit`，objectName=`txtGeneratedCode`；“保存为 .cpp”）
  - Tab6：测试与验证（左：`QPlainTextEdit`，objectName=`txtSourceTiny`；右：`QPlainTextEdit`，objectName=`txtLexResult`；按钮 `btnRunLexer`）
- 菜单栏/工具栏：打开/保存、RE→NFA、NFA→DFA、DFA→Min、生成代码、编译运行、帮助
- 状态栏输出进度：`正在生成NFA...`、`转换成功` 等
- 所有关键控件设置 `objectName`，用于 UI 自动化测试

---

## 正则解析与字母表提取
- 词法：支持 `|`、连接、`*`、`+`、`?`、`[]`、`()`、`\` 及命名宏 `name = expression`
- 解析：生成 AST（Concat/Union/Star/Plus/Question/CharSet/Symbol/Ref）
- 字母表：遍历 AST 提取原子符号（含转义的字面符，如 `\+`、`\*`）与命名集合（如 `letter`、`digit`），用于动态表格列
- Token 规则识别：
  - `_nameNNN = ...` 单 token，编码为 `NNN`
  - `_nameNNNS = x | y | z ...` 组 token，自增编码从 `NNN` 开始

---

## 自动机算法
1. Thompson 构造（RE→NFA）
- 节点结构：`id`、`accept` 标志；边：`symbol`（含 `epsilon`）→目标状态
- 输出：状态表（首列标记、次列状态 ID、后续列为字母表与 `#`）

2. 子集构造（NFA→DFA）
- ε-闭包与 `move` 组合生成状态集合；集合命名规范 `{1,4,7}`
- 输出：状态列为集合，按字母表生成转移

3. Hopcroft 最小化（DFA→MinDFA）
- 初分区：接受/非接受；按输入符号分割
- 输出：重新编号的最小化 DFA，作为代码生成基础

---

## 代码生成（方法二：Switch-Case 状态机）
- 生成 `lex.cpp`（或 `lex.c`），包含：
  - `Judgechar`：基于字母表分类（`isalpha`/`isdigit` 及自定义集合）
  - 输入缓冲与 `GetNext()`
  - `GetToken()`：`while(!done) switch(state)`，转移表来自 MinDFA
  - Token 输出：根据 `_nameNNN`/`_nameNNNS` 规则输出编码；空白与注释处理（`{...}`、空格/换行/Tab）
- 生成关键点：
  - 不区分大小写的关键字匹配（统一将字母归一化或构造大小写等价类）
  - 组 token 的编码自增映射
  - 错误处理与回退（输出 `Error` 或编码 `TOK_ERROR`）

---

## 测试与验证
- 单元测试（Qt Test）：
  - Regex：宏替换、字符集、优先级与括号解析
  - NFA：状态/边计数与 ε-闭包正确性
  - DFA：子集构造闭包与转移表
  - MinDFA：分区与最终状态数
  - 代码生成：生成字符串包含关键片段与可编译性 smoke test（仅字符串层面）
- GUI 自动化：
  - 查找控件、模拟输入 `a|b`、点击转换、断言表格行列
- TINY 集成用例：
  - 输入整套 TINY 正则；生成并编译扫描器；用 `sample.tny` 运行得到 `sample.lex`（GUI 中展示）

---

## 日志与跨平台规范
- 日志：`qInstallMessageHandler` 同步到控制台与 `app.log`；关键埋点：入口、分支、异常、数据摘要
- 跨平台：路径用 `QDir/QFileInfo`；换行兼容 `\r/\n`；避免 `long`，使用 `qint32/qint64`；布局使用 `QLayout`；UTF-8 源文件与 `QStringLiteral`

---

## 迭代步骤与里程碑
1. 完成环境安装与验证（Qt/CMake/Ninja/日志）
2. 初始化 CMake 构建（保留 qmake）并跑通空 GUI
3. 实现 `RegexLexer/Parser` 与字母表提取（含宏）
4. 实现 Thompson（RE→NFA）并输出表格
5. 实现子集构造（NFA→DFA）并输出表格
6. 实现 Hopcroft（DFA→MinDFA）并输出表格
7. 实现 `CodeGenerator`（方法二）并在 GUI 中展示
8. 增加测试工程（Qt Test），补齐单元/UI 测试
9. 集成 TINY 用例，生成并编译扫描器，验证 `sample.tny → sample.lex`
10. 打包与交付 README、测试报告与示例文件

---

## 交付物
- 源码（生成器工具）与独立核心库
- README（环境配置、构建与使用说明）
- 可执行程序（byyl.app / Windows exe）
- 测试相关：TINY 正则、`sample.tny`、生成的扫描器源码、编译出的扫描器、`sample.lex`、测试报告
