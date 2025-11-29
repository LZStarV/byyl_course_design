## 样例目录结构
- 新增 `tests/sample/` 作为各语言代码片段根目录
- 新增 `tests/sample/tiny/`，放入示例：
  - `tiny1.tny`：包含你给出的完整 TINY 片段（read/if/repeat/until/write/end）
- 未来可扩展：`tests/sample/javascript/`、`tests/sample/python/` 等

## UI 改造（测试与验证 + 代码查看）
- 测试与验证页：
  - 增加“选择样例文件”按钮（仅浏览 `tests/sample/` 目录），选择后将路径与内容填入左侧输入区
  - 保留现有“运行词法分析”以便使用引擎并行扫描（即时验证）
- 代码查看页：
  - 增强“编译并运行生成器”逻辑：基于“代码查看”生成的组合扫描器，针对“选择样例文件”的路径进行编译运行并生成 Token 输出与 `.lex` 文件

## 组合扫描器代码生成（多规则）
- 扩展 `CodeGenerator`：新增 `generateCombined(const QVector<MinDFA>& mdfas, const QVector<int>& codes, const Alphabet& alpha)`
  - 生成内容：
    - `Judgechar`（letter/digit 分类）、空白与 `{}` 注释跳过
    - 每个 DFA 的 `AcceptState_i` 与 `Step_i`（Switch-Case 版，沿用现有风格）
    - `matchLen(i, src, pos)`：计算第 i 个 DFA 在 `pos` 的最长匹配长度
    - `codeWeight(code)`：编码优先级（沿用 `Engine.cpp` 中权重策略）
    - `runMultiple(src)`：循环选择最佳匹配（最长优先，其次权重），拼接编码字符串
    - `main`：读取文件内容或 `stdin`，调用 `runMultiple` 并输出结果
  - 规则来源：使用 `Engine::buildAllMinDFA(parsed, codes)` 的返回内容构造组合扫描器

## 编译运行流程（UI 按钮）
- 代码查看页“编译并运行生成器”：
  - 若未转换→自动解析并构建所有规则的 MinDFA
  - 调用 `generateCombined(...)` 返回 C++ 源码，落地到应用目录（如 `gen_combined.cpp`）
  - 使用 `clang++ -std=c++17` 编译为 `gen_combined_bin`
  - 读取“选择样例文件”路径的文本，作为标准输入运行 `gen_combined_bin`
  - 将标准输出显示到右侧结果区，并同时写出同名 `.lex`（与样例文件同目录或应用目录）
  - 若运行包含 `ERR` → 弹窗或状态栏中文提示并视为失败

## 测试与验证（CI/ctest）
- 在 `CliRegexTest` 中新增用例：
  - 从 `tests/sample/tiny/tiny1.tny` 读取文本，使用引擎并行扫描验证 `ERR == 0`
  - 可新增一个“代码生成器组合版”测试，编译 `generateCombined` 产物并运行样例，断言输出非空且无 `ERR`

## 文档更新
- 更新 `README.md`：
  - 增加“样例目录”说明与使用步骤
  - 说明“代码查看”页可一键编译并运行生成器，生成 `.lex`
  - 标注命令行测试覆盖 TINY 与 JS，并严格中文化输出

## 迁移与兼容
- 保留现有“引擎并行扫描”逻辑，作为 UI 即时验证路径
- 新增组合扫描器生成不影响单规则生成器（原方法二），两者并行存在

## 交付与安全
- 不引入硬编码绝对路径；统一使用相对路径与 `QCoreApplication::applicationDirPath()`
- 不写入除应用目录与样例目录以外的路径；不引入敏感配置或外部依赖

请确认以上方案，确认后我将开始实现目录、UI 交互、代码生成器扩展与测试用例更新。