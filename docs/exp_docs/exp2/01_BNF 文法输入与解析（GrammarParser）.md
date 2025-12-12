# 01_BNF 文法输入与解析（GrammarParser）

## 数据结构总览

对象或变量名称 | 数据结构 | 存储结构用途
- | - | -
Production.left/right/line | 结构体字段 | 左部、右部符号序列、来源行号
Grammar.terminals | QSet<QString> | 终结符集合
Grammar.nonterminals | QSet<QString> | 非终结符集合
Grammar.startSymbol | QString | 文法开始符
Grammar.productions | QMap<QString, QVector<Production>> | 产生式映射（左部→候选列表）
lineNo | int | 当前解析行号（错误定位）
left | QString | 当前行的产生式左部
tokens | QVector<QString> | 右部切分后的符号序列
epsilon（`#`） | 特殊符号 | 空串标记（不计入非终结符）
mops | QVector<QString> | 多字符运算符集合（如 `<=, >=, ==, !=, :=, ++, --`）
ops | QSet<QChar> | 单字符分隔符集合（如 `(){}[];,<>=+-*/%^`）
isWordChar(c) | 函数 | 判断是否为词法单词字符（字母/数字/`_`/`-`）
splitRhs(const QString&) | 子过程 | 右部切分：多字符优先→单字符分隔→词法单词
parseLine(const QString&, int, Grammar&, QString&) | 子过程 | 行解析：`A -> α | β ...`，首个左部设为开始符
parseText/parseString/parseFile | 子过程 | 文本/字符串/文件入口（聚合行解析与归类）

## 算法实现过程
1. 文本读取与行过滤：按行处理文法文本；空行与仅由 `#` 构成的行忽略；`//` 开头的行为注释行忽略。
2. 产生式解析：每行按 `A -> α | β | …` 的格式解析；首个出现的左部设为开始符；右部按照 `|` 分割为候选。
3. 右部切分规则：候选右部进一步按“多字符运算符优先、再单字符分隔符、再词法单词”切分为符号序列；支持 `<=, >=, ==, !=, :=, ++, --` 等；常见分隔符包括 `(){}[];,<>=+-*/%^`；单词由字母/数字/下划线/短横线组成。
4. 符号归类：所有左部均记为非终结符；右部符号若不在左部集合且不等于 `#`，归为终结符；构建完成后得到终结符/非终结符集合与产生式表。
5. 左递归检测（留空扩展）：当前实现未启用直接左递归检测；如需支持可在归类后对每个非终结符的候选首符进行扫描并快捷判定。
6. 结果输出：返回包含开始符、产生式集、终结符/非终结符集合的文法对象，供 FIRST/FOLLOW 与 LR 系列算法使用。

### 扩展要点（细节说明）
- 错误恢复：当某行不含 `->` 或分割为两段失败时，记录错误行号并忽略该行，整体解析不中断；保证后续行仍可参与构建。
- 开始符稳定性：首次出现的左部即为开始符，后续不再覆盖；若文法来自多段文本，应确保文件内一致性（建议同文件定义）。
- `#` 的语义：作为空串（epsilon）仅用于右部；不参与左部归类；右部切分时若单独成词，按 `#` 处理，不被分隔符再拆分。
- 运算符优先：多字符运算符的优先匹配避免被单字符拆分；例如 `<=` 在扫描时整体匹配后再推进下一个位置。
- 分隔符集合：固定集合用于快速切分常见符号；如需支持额外符号（引号/反引号等），可在配置或源码中扩展集合。
- 文本规范化：切分后的词法单词保留原大小写与下划线/短横线；不进行大小写转换与正则化，便于与生成流程对齐。

## 算法实现流程图
<!--【流程图】此处需要添加 “BNF 文法解析流程图”：文本读取→行过滤→产生式解析→右部切分→符号归类→结果输出。-->

## 关键代码（可选）
```
// 右部切分（项目源码节选，已格式化）
static QVector<QString> splitRhs(const QString& rhs)
{
    QVector<QString> v;
    QString s = rhs;
    int i = 0;

    auto isWordChar = [](QChar c)
    {
        return c.isLetterOrNumber() || c == '_' || c == '-';
    };

    auto isSingleOp = [](QChar c)
    {
        static QSet<QChar> ops;
        if (ops.isEmpty())
        {
            const QChar arr[] = {
                '(', ')', '{', '}', '[', ']', ';', ',',
                '<', '>', '=', '+', '-', '*', '/', '%', '^'
            };
            for (QChar ch : arr) ops.insert(ch);
        }
        return ops.contains(c);
    };

    auto matchMultiOp = [&](const QString& s, int i) -> QString
    {
        static const QVector<QString> mops = {"<=", ">=", "==", "!=", ":=", "++", "--"};
        for (const auto& op : mops)
        {
            int L = op.size();
            if (L > 0 && i + L <= s.size() && s.mid(i, L) == op)
                return op;
        }
        return QString();
    };

    while (i < s.size())
    {
        QChar c = s[i];
        if (c.isSpace()) { i++; continue; }

        QString mop = matchMultiOp(s, i);
        if (!mop.isEmpty())
        {
            v.push_back(mop);
            i += mop.size();
            continue;
        }

        if (isSingleOp(c))
        {
            v.push_back(QString(c));
            i++;
            continue;
        }

        QString w;
        while (i < s.size() && isWordChar(s[i]))
        {
            w += s[i];
            i++;
        }
        if (!w.isEmpty()) v.push_back(w);
        else i++;
    }
    return v;
}

// 行解析（项目源码节选，已格式化）
static bool parseLine(const QString& line, int lineNo, Grammar& g, QString& err)
{
    QString t = line;
    if (t.trimmed().isEmpty()) return true;
    if (t.trimmed().startsWith("//")) return true;

    if (t.indexOf("->") < 0)
    {
        err = QString::number(lineNo);
        return false;
    }

    auto parts = t.split("->");
    if (parts.size() != 2)
    {
        err = QString::number(lineNo);
        return false;
    }

    QString left = parts[0].trimmed();
    QString rhs  = parts[1].trimmed();
    auto    alts = rhs.split('|');

    if (g.startSymbol.isEmpty() && !left.isEmpty())
        g.startSymbol = left;

    for (auto a : alts)
    {
        Production p;
        p.left  = left;
        p.right = splitRhs(a.trimmed());
        p.line  = lineNo;
        g.productions[left].push_back(p);
    }
    return true;
}
```

## 单元测试
- 输入：
  - `E -> E + T | T`
  - `T -> T * F | F`
  - `F -> ( E ) | id`
- 预期结果：
  - 开始符为 `E`；`E/T/F` 候选被正确切分；非终结符含 `E,T,F`；终结符含 `+,*,(,),id`；注释与纯 `#` 行被忽略。
- 测试结果：
  - 命令行截图：运行 BNF 解析测试，打印开始符/集合/产生式数量统计；
  - GUI 截图：在实验二界面加载文法文本并点击“解析”，成功提示与集合预览。
  - <!--【GUI 截图】此处需要添加 “BNF 文本输入与解析” 截图：文本编辑区、加载与解析按钮、提示输出。-->
