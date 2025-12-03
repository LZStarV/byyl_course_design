# byyl 词法/语法分析器（Qt+CMake）

- 环境：Qt 6.9.3，CMake 4.2.0，Ninja 1.13.2（macOS/Windows）
- 构建产物目录：`dist`（统一使用 CMake/Ninja）

## 快速开始
- 配置：`qt-cmake -S . -B dist -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- 编译：`cmake --build dist -j`
- 运行：
  - macOS：`open dist/byyl.app` 或 `dist/byyl.app/Contents/MacOS/byyl`
  - Windows：`dist\\byyl.exe`
- 测试：`ctest --test-dir dist -V`

## 功能
- 实验一（词法）：正则→NFA（Thompson）→DFA（子集构造）→MinDFA（Hopcroft）；表格展示、DOT/PNG 导出与预览；合并扫描器代码生成与运行。
- 实验二（语法）：BNF 文法解析；First/Follow 计算；LR(0) 项集 DFA 预览与导出；AST 可视化；语法分析器代码生成。

## 使用
- 词法页签：加载正则→转换→查看 NFA/DFA/MinDFA 状态表→生成代码→在“测试与验证”页运行得到 Token 序列。
- 语法页签：加载/解析文法→查看 First/Follow 与 LR(0) 预览→基于 Token 序列运行语法分析→预览/导出语法树 DOT→查看语法代码。
- Graphviz：安装 `graphviz` 并确保 `dot` 在 PATH（用于 PNG 预览与图片导出）。

## 目录结构
- `app/`：主窗口与入口
  - `components/`：Toast、SettingsDialog、ImagePreviewDialog 等公共组件
  - `controllers/`：按功能分目录（Regex/Automata/Syntax/TestValidation/CodeView/Settings/Generator）
  - `services/`：`DotService`、`FileService`、`NotificationService`
  - `experiments/exp1/tabs/`：正则编辑、NFA/DFA/MinDFA、代码查看、测试与验证
  - `experiments/exp2/tabs/`：文法编辑、First&Follow、语法树、语法代码查看
  - `pages/`：页面容器（主页、实验一/二）
- `src/`：核心逻辑（regex/automata/syntax/generator/model/Engine）
- `tests/`：UI、CLI、代码生成、配置、DOT 与语法模块测试
- `generated/lex/` 与 `generated/syntax/`：生成源码与图导出

## 配置
- `config/lexer.json`：
  - `generated_output_dir`：生成目录根（语法代码位于其 `syntax/` 子目录）
  - `weight_tiers`：权重阈值数组
- 环境变量：
  - `BYYL_GEN_DIR`：覆盖生成目录根
  - `LEXER_WEIGHTS`：`min:weight` 列表，如 `220:3,200:4,...`
  - `LEXER_SKIP_*`：跳过注释/字符串（`LINE/BLOCK/HASH/SQ/DQ/TPL`）

## 常见问题
- Qt 查找失败：为 `cmake` 添加 Qt 路径（macOS：`-DCMAKE_PREFIX_PATH=$(brew --prefix qt)`；Windows：指向 Qt 安装的 MSVC 目录）。
- Graphviz 渲染失败：安装 `graphviz`，降低 DPI 或导出 DOT 用外部工具查看。

## 测试目标
- `GuiTest`：UI 关键控件与流程
- `CliRegexTest`：词法命令行管线
- `CodegenTest`：组合扫描器源码编译运行
- `GrammarParserTest`、`LL1Test`、`DotExportTest`：语法与导出模块

## 代码格式化
- 统一使用项目根目录的 `.clang-format`：
  - 全量格式化命令：
    - `find . -type f \( -name "*.h" -o -name "*.cpp" \) -not -path "./dist/*" -not -path "./build/*" -print0 | xargs -0 -n 50 clang-format -i -style=file`
  - 单文件示例：
    - `clang-format -i -style=file app/mainwindow.cpp`
