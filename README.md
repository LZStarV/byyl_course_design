# byyl 词法/语法分析器（Qt+CMake）

- 环境：Qt 6.9.3，CMake 4.2.0，Ninja 1.13.2（macOS / Windows）
- 构建产物目录：`dist`（统一使用 CMake / Ninja）

## 快速开始
- 配置：`qt-cmake -S . -B dist -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- 编译（二选一）：
  - `cmake --build dist -j`
  - `ninja -C dist`（等效的简洁写法）
- 运行：
  - macOS：`open dist/byyl.app` 或 `dist/byyl.app/Contents/MacOS/byyl`
  - Windows：`dist\\byyl.exe`
- 测试：`ctest --test-dir dist -V`
- 全量格式化命令：`find . -type f \( -name "*.h" -o -name "*.cpp" \) -not -path "./dist/*" -not -path "./build/*" -print0 | xargs -0 -n 50 clang-format -i -style=file`

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
  - LR(1) 解析流程可视化（步骤、描述），冲突策略可配置（prefer_shift / prefer_reduce / error）；
  - SLR(1) 检查与冲突明细；
  - 语义动作导入（偶数行文件：产生式行 + 动作行）；
  - 语义语法树 AST 预览/导出（DOT），支持文本树视图（Qt Tree）；
  - 语法分析器代码生成。

## 使用
- 词法页签：加载正则→转换→查看 NFA/DFA/MinDFA 状态表→生成代码→在“测试与验证”页运行得到 Token 序列。
  - 当匹配到 `identifier` 时，输出将在其编码后紧随源词素（如 `read x;` → `206 100 x 116`）。
- 语法页签：
  - 加载/解析文法；
  - 导入语义动作文件（每两行一对：`A -> ...` 与角色位行，行数必须为偶数）；
  - 在“LR(1)分析过程”页签，基于“正则表达式 + Token 序列 + 当前语义动作”运行语法分析；
    - 读取 token 序列时，若 `identifier` 后紧随词素，将自动消费该词素（用于语义，不进入终结符流）。
  - 在“LR(1)语法树”页签，预览/导出语义树 DOT，或查看文本树视图。
- Graphviz：安装 `graphviz` 并确保 `dot` 在 PATH（用于 PNG 预览与图片导出）。

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

## 设置
- 配置文件：`config/lexer.json`（可通过 `config_search_paths` 指定自定义搜索路径）。所有配置均可在应用菜单“设置”中编辑，左侧目录分区如下：

### 目录与输出
- `generated_output_dir`：词法生成根目录（默认：`generated/lex`）
- `syntax_output_dir`：语法生成目录（默认：`<generated_output_dir>/syntax`）
- `graphs_dir`：图导出根目录（默认：`<generated_output_dir>/graphs`，语法图位于其 `syntax/` 子目录）
- `config_search_paths`：配置文件搜索路径数组（优先于默认相对路径；分号分隔）

### 权重与跳过
- `weight_tiers`：权重阈值数组（例：`[{"min_code":220,"weight":3},...]`）
- `skip`：跳过开关（`brace_comment/line_comment/block_comment/hash_comment/single_quote_string/double_quote_string/template_string`）

### 词法与标识符
- `macro_names.letter/digit`：宏名称（例如 `letter`/`digit`）
- `token_map.use_heuristics`：是否启用名称与组规则的启发式映射
- `whitespaces`：空白字符集合（默认：`[" ", "\t", "\n", "\r"]`）
- `emit_identifier_lexeme`：是否在 `identifier` 后追加词素（默认：`true`）
- `identifier_token_names`：视为标识符的规则名数组（默认：`["identifier"]`，大小写不敏感，按规则名包含判断）
- Token 头部解析约定：`_NAME123`（`_NAME123S` 表示组），用于提取编码与是否为组；无需额外配置。

### 语法与算法
- `epsilon_symbol`（默认：`#`）、`eof_symbol`（默认：`$`）、`aug_suffix`（默认：`'`）
- `lr1_conflict_policy`：`prefer_shift|prefer_reduce|error`（默认：`prefer_shift`）
- `nonterminal_pattern`：非终结符正则（可空）
- `grammar_tokens.multi_ops`：多字符操作符（默认：`["<=", ">=", "<>", ":="]`）
- `grammar_tokens.single_ops`：单字符操作符（默认：`["(", ")", ";", "<", ">", "=", "+", "-", "*", "/", "%", "^"]`）

### 表头与 DOT
- `i18n.table_mark/table_state_id/table_state_set/epsilon_column_label`：表头与 ε 列名
- `dot.rankdir/node_shape/epsilon_label`：DOT 样式（默认：`LR/circle/ε`）

### Graphviz
- `graphviz.executable`：`dot` 可执行名（默认：`dot`）
- `graphviz.default_dpi`：默认 DPI（默认：`150`）
- `graphviz.timeout_ms`：渲染超时毫秒数（默认：`20000`）

### 语义策略
- `semantic_actions.root_selection_policy`：语义树根选择策略（如 `first_1/last_1`）
- `semantic_actions.child_order_policy`：子节点顺序策略（如 `rhs_order`）

### 环境变量（可选，优先级低于显式配置）
- `BYYL_GEN_DIR`：覆盖生成目录根
- `LEXER_WEIGHTS`：`min:weight` 列表，如 `220:3,200:4,...`
- `LEXER_SKIP_*`：跳过注释/字符串（`LINE/BLOCK/HASH/SQ/DQ/TPL`）
- `LEXER_EMIT_IDENTIFIER_LEXEME`：组合扫描器运行时控制在 `identifier` 后追加词素（`1/true/yes` 开启）

## 测试目标
- `GuiTest`：UI 关键控件与流程
- `CliRegexTest`：词法命令行管线
- `CodegenTest`：组合扫描器源码编译运行
- `GrammarParserTest`、`LL1Test`、`LR0Test`、`LR1Test`、`SLRTest`、`DotExportTest`：语法与导出模块

## 常见问题
- Qt 查找失败：为 `cmake` 添加 Qt 路径（macOS：`-DCMAKE_PREFIX_PATH=$(brew --prefix qt)`；Windows：指向 Qt 安装的 MSVC 目录）。
- Graphviz 渲染失败：安装 `graphviz`，降低 DPI 或导出 DOT 用外部工具查看。