# 09 组合扫描与编码输出（Engine.runMultiple）

## 数据结构

表 1 数据结构总览
对象或结构名称 mdfas/codes
数据结构类型 向量
存储结构用途 多个最小化 DFA 与对应编码列表

对象或结构名称 identifierCodes/source
数据结构类型 集合/文本
存储结构用途 需要附带词素的编码集合与源文本

对象或结构名称 输出
数据结构类型 文本
存储结构用途 以空格分隔的编码序列；若命中 identifierCodes 且开启词素输出，则追加源词素（如 101 variableName）

## 算法实现过程

- 跳过策略：空白/注释/字符串（单/双/模板）按照配置跳过，并推进位置；
- 最长匹配：对每个 DFA 计算从当前位置的 `matchLen`，取最大长度；
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
