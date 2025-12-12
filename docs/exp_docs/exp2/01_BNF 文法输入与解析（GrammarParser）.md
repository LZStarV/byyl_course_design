# 01_BNF 文法输入与解析（GrammarParser）

## 数据结构总览

对象或变量名称 | 数据结构 | 存储结构用途
| - | - | -
Production | 结构体 | 存储产生式的左部、右部符号序列与来源行号
Grammar | 类/映射 | 存储终结符、非终结符集合、开始符、产生式映射
`#`/`$` | 特殊符号 | 分别表示空串与输入结束（用于集合/前瞻）
lineNo | 整数 | 当前解析行号，记录错误定位
startSymbol | 字符串 | 首个出现的左部，作为文法开始符
tokens | 序列 | 右部切分后的符号序列

## 算法实现过程
1. 文本读取与行过滤：按行处理文法文本；空行与仅由 `#` 构成的行忽略；`//` 开头的行为注释行忽略。
2. 产生式解析：每行按 `A -> α | β | …` 的格式解析；首个出现的左部设为开始符；右部按照 `|` 分割为候选。
3. 右部切分规则：候选右部进一步按“多字符运算符优先、再单字符分隔符、再词法单词”切分为符号序列；支持 `<=, >=, ==, !=, :=, ++, --` 等；常见分隔符包括 `(){}[];,<>=+-*/%^`；单词由字母/数字/下划线/短横线组成。
4. 符号归类：所有左部均记为非终结符；右部符号若不在左部集合且不等于 `#`，归为终结符；构建完成后得到终结符/非终结符集合与产生式表。
5. 左递归检测（留空扩展）：当前实现未启用直接左递归检测；如需支持可在归类后对每个非终结符的候选首符进行扫描并快捷判定。
6. 结果输出：返回包含开始符、产生式集、终结符/非终结符集合的文法对象，供 FIRST/FOLLOW 与 LR 系列算法使用。

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
