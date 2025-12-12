# 09 组合扫描与编码输出（Engine.runMultiple）

## 数据结构

表 1 数据结构总览

对象或变量名称 | 数据结构 | 存储结构用途
| - | - | -
mdfas | QVector<MinDFA> | 最小化 DFA 集合（组合扫描候选）
codes | QVector<int> | 与 mdfas 一一对应的编码列表
source | QString | 源文本（待扫描）
identifierCodes | QSet<int> | 需要追加词素的编码集合
out | QString | 输出编码序列（空格分隔；必要时追加词素）
pos | int | 当前扫描位置
bestLen/bestIdx/bestW | int | 最长匹配长度 / 选择索引 / 权重
matchLen(const MinDFA&, const QString&, int) | 函数 | 计算指定 DFA 从位置的最长可接受前缀长度
Config::emitIdentifierLexeme() | 布尔 | 是否开启“追加词素”开关
跳过策略开关 | 布尔 | 空白/注释/字符串（单/双/模板）是否跳过

## 算法实现过程

- 跳过策略（逐类说明）
  - 空白：当当前位置字符属于空白集合（空格、制表、换行、回车），直接 `pos++` 并继续扫描。
  - 注释：按开关逐类跳过（位置推进直到闭合标记出现）：
    - 花括号注释：`{ ... }`
    - 行注释：`// ... \n`
    - 井号注释：`# ... \n`
    - 块注释：`/* ... */`
  - 字符串：按开关逐类跳过（处理转义与模板插值）：
    - 单引号：`' ... '`（遇到 `\` 跳过下一字符）
    - 双引号：`" ... "`（遇到 `\` 跳过下一字符）
    - 模板字符串：`` ` ... ${ ... } ... ` ``（支持 `${}` 内部嵌套与转义）
- 最长匹配（核心流程）
  - 对每个 DFA 计算当前位置的最长可接受前缀长度：`len_i = matchLen(mdfa_i, source, pos)`
  - `matchLen` 语义：从 `pos` 开始按字符步进状态，记录最近的接受位置 `lastAcc`；不可移时停止，返回 `lastAcc-pos`（若无接受则返回 0）。
- 选择规则（稳定化）
  - 首先比较长度：取 `len_i` 最大者。
  - 长度相同：比较权重 `w_i`（由编码权重函数给出，值越大越优先）。
  - 长度与权重均相同：比较索引 `i`（较小索引优先），确保选择稳定且可重复。
- 输出与推进（含词素追加）
  - 若 `bestLen > 0`：输出命中编码 `codes[bestIdx]`；若 `emitIdentifierLexeme` 开启且该编码属于 `identifierCodes`，追加词素 `source.mid(pos, bestLen)`；随后 `pos += bestLen`。
  - 若 `bestLen == 0`：输出 `ERR` 并 `pos++`，以避免死循环。
- 复杂场景与边界说明
  - 连续跳过片段：若连续出现多段注释/字符串/空白，逐段推进 `pos`，直到遇到可参与匹配的字符后再计算 `matchLen`。
  - 混合命中场景：当关键字与标识符在同起点同时匹配时，关键字通常具有更高权重或更长长度，最终按“长度>权重>索引”决策选中关键字。
  - 不可识别字符：对任何不在字母表覆盖范围内的单字符，产生 `ERR` 并前进 1，保证扫描持续推进。
- 权重与顺序：长度相同时比较权重（`Config::weightForCode`），再比较索引稳定性；
- 输出编码与词素：输出命中编码，并在需要时追加词素；位置推进 `pos += bestLen`；无匹配则输出 `ERR` 并前进 1；

下图展示测试与验证页签的输出序列示例（包含追加词素的场景）：
<!--【GUI 截图】此处需要添加 '测试与验证页签截图'：输入示例源与输出编码序列。 -->

## 算法实现过程（编号式）

1. 初始化：设置位置 `pos=0` 与输出字符串；准备跳过策略与标识符词素开关。
2. 主循环：当 `pos < source.size()`：
   2.1 读取当前字符并应用跳过策略（空白与注释/字符串类别）；若跳过则更新 `pos` 继续；
   2.2 对每个 DFA 计算 `matchLen(i, source, pos)`；记下最长长度 `bestLen` 与索引 `bestIdx`；
   2.3 若有并列最⻓，比较权重与索引以稳定选择；
   2.4 若 `bestLen > 0`：输出编码；如需追加词素则拼接 `source.mid(pos, bestLen)`；`pos += bestLen`；
   2.5 否则：输出 `ERR` 并 `pos++`。
3. 结束：返回输出字符串。

下图为“组合扫描流程图”占位：
<!--【流程图】此处需要添加 '组合扫描流程图'：跳过策略→遍历 DFA→最长匹配→权重选择→输出与推进。 -->

## 各种情况讲解

- 无匹配：输出 `ERR` 并前进 1。
- 跳过开关调整：根据配置可启用/关闭各类跳过；若启用注释跳过，注释不出现在 Token 序列中。
- 标识符词素输出：可关掉开关以仅输出编码；集合可通过名称映射构造（UI 控制器侧）。

## 关键代码片段

```
// 从 pos 开始，沿指定最小化 DFA 计算最长可接受前缀的长度
static int matchLen(const MinDFA& mdfa, const QString& src, int pos)
{
    int state = mdfa.start; int i = pos; int lastAcc = -1;
    while (i < src.size())
    {
        QChar ch = src[i]; bool moved = false;
        // 遍历字母表符号，查找可达转移
        for (auto a : mdfa.alpha.ordered())
        {
            int t = mdfa.states[state].trans.value(a, -1);
            if (t == -1) continue;
            // 仅当符号与当前字符匹配时推进状态
            if (a.size() == 1 && a[0] == ch) { state = t; moved = true; break; }
        }
        if (!moved) break;
        i++;
        // 记录最近接受位置（用于最长匹配）
        if (mdfa.states[state].accept) lastAcc = i;
    }
    return lastAcc == -1 ? 0 : (lastAcc - pos);
}

// 对源文本进行组合扫描，输出编码序列并可追加词素
QString Engine::runMultiple(const QVector<MinDFA>& mdfas,
                            const QVector<int>&    codes,
                            const QString&         source,
                            const QSet<int>&       identifierCodes)
{
    QString out; int pos = 0;
    while (pos < source.size())
    {
        QChar ch = source[pos];
        // 跳过空白（可配置）
        if (Config::isWhitespace(ch)) { pos++; continue; }
        // 跳过注释/字符串（按配置）
        // 花括号注释、行注释、井号注释、块注释、单双引号与模板字符串（省略具体实现）
        int bestLen = 0, bestIdx = -1, bestW = -1;
        // 遍历所有 DFA，计算最长匹配并记录权重
        for (int i = 0; i < mdfas.size(); ++i)
        {
            int len = matchLen(mdfas[i], source, pos);
            int w   = Config::weightForCode(codes[i]);
            // 决策：长度优先；长度相同时权重优先；再用索引稳定
            if (len > bestLen || (len == bestLen && (w > bestW || (w == bestW && (bestIdx == -1 || i < bestIdx)))))
            { bestLen = len; bestIdx = i; bestW = w; }
        }
        if (bestLen > 0)
        {
            int code = codes[bestIdx]; out += QString::number(code) + " ";
            // 若为标识符等需要附带词素的编码，则追加词素
            if (Config::emitIdentifierLexeme() && identifierCodes.contains(code)) { out += source.mid(pos, bestLen) + " "; }
            pos += bestLen;
        }
        else { out += "ERR "; pos++; }
    }
    return out.trimmed();
}
```

## 输入/输出与示例

输入：
1) 最长匹配对比用例：
```
letter=[A-Za-z_]
digit=[0-9]
_identifier100=letter(letter|digit)*
_keywords200S= if | else | read | write | then | end
源文本："if abc"
```

预测结果：
- 位置 0 处应优先匹配关键字 `if`，而非标识符；输出序列首项不应为 `100`。

测试结果：
- 断言通过：首个编码不等于 100；权重与最长匹配规则生效。

2) 语言管线用例（示例：javascript/tiny/java/cpp/go/rust）：
- 读取相应 `.regex` 与示例源文本；运行组合扫描；统计 `ERR` 数量；

预测结果：
- 在常规输入下 `ERR` 数量为 0 或极少（如 ≤ 2）。

测试结果：
- 多语言用例的 `ERR` 数符合预期，并输出非空编码序列；
- GUI 截图：词法管线与 CLI 测试（LongestMatchTest / CliRegexTest）的运行截图（显示最长匹配断言与多语言输出统计）。
<!--【GUI 截图】此处需要添加 'LongestMatchTest / CliRegexTest 运行截图'：显示断言与输出统计。 -->
下图展示“测试与验证页签”的输出示例（包含追加词素的场景）：
<!--【GUI 截图】此处需要添加 '测试与验证页签截图'：输入示例源与输出编码序列。 -->
