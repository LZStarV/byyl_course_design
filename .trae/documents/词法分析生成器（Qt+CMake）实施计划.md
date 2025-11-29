## 环境配置（macOS + Qt 6.x + CMake + Ninja）
1. 安装基础工具
- 安装 Xcode 命令行工具：`xcode-select --install`
- 安装 Homebrew：`/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"`

2. 安装与配置 Qt/CMake/Ninja
- 安装 Qt 6：`brew install qt`
- 安装 CMake/Ninja：`brew install cmake ninja`
- 设置环境变量（Homebrew 版本）：`export PATH="$(brew --prefix qt)/bin:$PATH"`，`export CMAKE_PREFIX_PATH="$(brew --prefix qt)"`
- 验证：`qt-cmake --version`、`cmake --version`、`ninja --version`、`qmake -v`

3. 构建与运行
- 生成构建文件：`qt-cmake -S . -B build-macos -G Ninja`
- 编译：`cmake --build build-macos -j`
- 启动 GUI：`open build-macos/byyl.app` 或 `build-macos/byyl.app/Contents/MacOS/byyl`
- 打包：`macdeployqt build-macos/byyl.app -verbose=2 -dmg`

---

## 项目目录与架构
- `app/`：Qt Widgets GUI（主窗口、菜单、Tabs、表格与代码查看器）
- `core/`：业务核心库（与 GUI 解耦，可独立测试）
  - `regex/`：`RegexLexer`、`RegexParser`（宏、字符集、转义、括号与运算符）
  - `automata/`：`NFA`（Thompson）、`DFA`（子集构造）、`MinDFA`（Hopcroft）
  - `generator/`：`CodeGenerator`（从 MinDFA 生成 C/C++ 源码，采用 Switch-Case 状态机）
  - `model/`：`State`、`Edge`、`Alphabet`、`TransitionTable` 等数据结构
- `tests/`：Qt Test 单元与 UI 自动化测试
- `resources/`：示例正则、TINY 测试用例、`sample.tny`、README
- 构建文件：根级 `CMakeLists.txt`（主应用与测试目标），保留原有 qmake 文件以兼容

---

## GUI 设计与交互
- 主窗口采用 Tab 结构（均设置 `objectName`）：
  - Tab1 正则编辑：`QTextEdit`（`txtInputRegex`），文件打开/保存按钮
  - Tab2 NFA 状态表：`QTableWidget`（`tblNFA`），动态列含 `#` 表示 ε
  - Tab3 DFA 状态表：`QTableWidget`（`tblDFA`）
  - Tab4 MinDFA 状态表：`QTableWidget`（`tblMinDFA`）
  - Tab5 代码查看：`QPlainTextEdit`（`txtGeneratedCode`），“保存为 .cpp”按钮
  - Tab6 测试与验证：左 `QPlainTextEdit`（`txtSourceTiny`），右 `QPlainTextEdit`（`txtLexResult`），按钮 `btnRunLexer`
- 菜单栏/工具栏：新建/打开/保存、RE→NFA、NFA→DFA、DFA→Min、生成代码、编译运行、帮助
- 状态栏：显示进度与结果（如“正在生成NFA...”“转换成功”）

---

## 输入文件与正则解析
- 词法支持：连接、`|`、`*`、`+`、`?`、`[]`、`()`、`\` 转义、命名宏 `name = expression`
- Token 定义规范：
  - `_nameNNN = ...` 单 token，编码为 `NNN`
  - `_nameNNNS = x | y | z ...` 组 token，编码从 `NNN` 递增
- AST 设计：`Concat`、`Union`、`Star`、`Plus`、`Question`、`CharSet`、`Symbol`、`Ref`
- 字母表提取：遍历 AST 收集原子符号与命名集合（如 `letter`、`digit`、`\+`、`\*`），用于动态表格列头

---

## 自动机转换与可视化
1. RE→NFA（Thompson）
- 节点 `State{id, accept}` 与边 `Edge{from, symbol/epsilon, to}`
- 输出状态表结构：首列标记（`-` 初态，`+` 终态）、次列状态 ID、后续列为字母表与 `#`

2. NFA→DFA（子集构造）
- 实现 `epsilon-closure` 与 `move`；状态集合以 `{1,4,7}` 命名
- 输出：状态列为集合；列为字母表；显示目标集合

3. DFA→MinDFA（Hopcroft）
- 初分区：接受/非接受；按输入分割迭代
- 输出：重新编号的最小化 DFA，作为代码生成基础

---

## 代码生成（方法二：Switch-Case 状态机）
- 选择方法二（Switch-Case）：根据 MinDFA 生成 `lex.cpp`
- 结构：
  - `Judgechar`：结合 `isalpha/isdigit` 与自定义集合，实现字符分类
  - 输入缓冲与 `GetNext()`
  - `GetToken()`：`while(!done){switch(state){...}}`；转移表源自 MinDFA
  - Token 输出：遵循 `_nameNNN` / `_nameNNNS` 编码与大小写不敏感关键字（字母归一化或等价类）
  - 空白与注释处理：空白（空格/换行/Tab）与 `{...}` 注释（跨行、不嵌套）
- 错误处理：无法识别时输出 `Error` 或 `TOK_ERROR`

---

## 测试与验收（Qt Test）
- 单元测试（`Qt6::Test`）：
  - Regex：宏替换、字符集与优先级、括号与转义
  - NFA：节点/边计数与 ε-闭包正确性
  - DFA：子集构造的闭包与转移表
  - MinDFA：分区与最终状态数
  - 代码生成：生成字符串包含关键片段与基本可编译性校验（字符串级）
- GUI 自动化：
  - 通过 `objectName` 查找控件，模拟输入与点击，断言表格行列与非空输出
- 集成测试（TINY）：
  - 输入 TINY 正则；生成并编译扫描器；对 `sample.tny` 运行，产出 `sample.lex` 并在 GUI 查看
- CMake 配置：`find_package(Qt6 REQUIRED COMPONENTS Widgets Test)`、`enable_testing()`、`add_test(...)`

---

## 日志与调试规范
- 启用：`export QT_LOGGING_RULES="*.debug=true"`
- 重定向：使用 `qInstallMessageHandler` 输出到控制台与 `app.log`
- 埋点：入口、分支决策、异常、数据快照（如“DFA states=N, edges=M”）

---

## 跨平台兼容性
- 路径：`QDir/QFileInfo`，避免硬编码分隔符
- 行尾：文件读取用 `QTextStream`；空白处理兼容 `\n/\r`
- 类型：使用 `qint32/qint64`，避免 `long`
- 布局：使用 `QLayout` 适配高 DPI
- 编码：源文件 UTF-8；中文字符串用 `QStringLiteral`

---

## 构建与运行命令
- 配置：`qt-cmake -S . -B build-macos -G Ninja`
- 编译：`cmake --build build-macos -j`
- 运行：`open build-macos/byyl.app` 或 `build-macos/byyl.app/Contents/MacOS/byyl`
- 测试：`ctest --test-dir build-macos -V`
- 打包：`macdeployqt build-macos/byyl.app -verbose=2 -dmg`

---

## 里程碑
1. 环境安装与验证（Qt/CMake/Ninja）
2. 初始化 CMake 构建并跑通空 GUI（保留 qmake）
3. 完成 RegexLexer/Parser 与字母表提取（含宏）
4. 实现 Thompson（RE→NFA）并在 GUI 展示
5. 实现子集构造（NFA→DFA）并展示
6. 实现 Hopcroft（DFA→MinDFA）并展示
7. 完成 CodeGenerator（方法二）并在 GUI 展示与保存
8. 建立 Qt Test（单元 + UI 自动化）
9. 集成 TINY 用例，生成扫描器并验证 `sample.tny → sample.lex`
10. 打包交付 README、测试报告与示例文件

---

## 交付物
- 源码（生成器工具）与核心库
- README（环境、构建、使用说明）
- 可执行程序（byyl.app / Windows exe）
- 测试相关：TINY 正则、`sample.tny`、生成扫描器源码、编译出的扫描器、`sample.lex`、测试报告