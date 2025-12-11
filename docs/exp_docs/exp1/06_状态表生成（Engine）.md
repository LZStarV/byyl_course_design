# 06 状态表生成（Engine）

## 数据结构

表 1 数据结构总览
对象或结构名称 Tables
数据结构类型 结构体
存储结构用途 用于展示自动机的列（符号与标记）、状态名称与行数据

对象或结构名称 输入/输出定义
数据结构类型 文本说明
存储结构用途 自动机（输入：NFA/DFA/MinDFA 任一）；状态表（输出：列/标记/状态/行的表格数据）

## 算法实现过程

- 表类型与目标：
  - NFA 表：标记列、状态 id、符号列与 ε 列；行按状态填充每个符号的目标集合与 ε 目标集合；
  - DFA 表：标记列、状态集合名（如 `{0, 1}`）与符号列；行填充目标集合的名称；
  - MinDFA 表：标记列、状态 id 与符号列；行填充目标状态 id；
- 列构造与行填充：读取字母表的有序符号依次加入符号列（NFA 追加 ε 列）；逐状态收集各符号目标，NFA 的目标集合以逗号拼接，DFA/MinDFA 直接展示目标；
- 标记设置：起始态标记 `-`，接受态标记 `+`，其余为空；
- 宏聚合：计算每个宏的字符集合，收集列索引并合并显示，同时保留未被聚合的原始列；
- 输出：列与行数据用于 GUI 展示或导出。

下图展示三类状态表的表头与示例行（含标记列与符号列）：
<!--【GUI 截图】此处需要添加 'NFA/DFA/MinDFA 状态表截图'：表头与若干行示例。 -->

## 算法实现过程（编号式）

1. 输入自动机与宏映射（可选）；初始化表：列、标记、状态、行集合。
2. 列构造：
   2.1 将标记列与状态列加入表头；
   2.2 读取字母表的有序符号依次加入符号列；
   2.3 若为 NFA 表，追加 ε 列；
3. 行填充：逐状态遍历并对每个符号求目标（NFA 为集合；DFA 为单状态；MinDFA 为单状态 id）；NFA 的目标集合以逗号拼接。
4. 标记设置：起始态标记 `-`，接受态标记 `+`，其余空字符串。
5. 宏聚合（可选）：按宏字符集将相邻单字符列聚合成宏列并合并目标；保留未被聚合的原始列。
6. 输出状态表：列与行数据用于 GUI 展示或导出。

下图为“状态表生成流程图”占位：
<!--【流程图】此处需要添加 '状态表生成流程图'：列构造→行填充→标记设置→宏聚合。 -->

## 各种情况讲解

- 空列：某符号无转移时显示为空；在 NFA 表中可能出现多目标转移，需稳定排序与去重。
- 标记列：起始态以 `-` 标记，接受态以 `+` 标记；其余为空字符串。

## 关键代码片段

```
// NFA 状态表：包含 ε 列，目标集合以逗号拼接
static Tables tableFromNFA(const NFA& nfa);

// DFA 状态表：集合名显示（例如 "{1,2}"），无 ε 列
static Tables tableFromDFA(const DFA& dfa);

// MinDFA 状态表：目标以状态 id 显示
static Tables tableFromMin(const MinDFA& dfa);

// 将单字符列按宏集合进行聚合，提升表格可读性
static void aggregateTableByMacros(Tables& t, const QMap<QString, QSet<QChar>>& msets, const QMap<QString, QString>& mexpr);
```

```
static Tables tableFromNFA(const NFA& nfa)
{
    Tables t; // 列构造：标记、状态 id、符号列与 ε 列
    QVector<QString> cols; cols.push_back(Config::tableMarkLabel()); cols.push_back(Config::tableStateIdLabel());
    auto syms = nfa.alpha.ordered(); for (auto s : syms) cols.push_back(s); cols.push_back(Config::epsilonColumnLabel());
    t.columns = cols;
    // 行填充：逐状态收集各符号目标集合与 ε 目标集合
    for (auto it = nfa.states.begin(); it != nfa.states.end(); ++it)
    {
        QString mark = it->id == nfa.start ? "-" : (it->accept ? "+" : "");
        QString sid  = QString::number(it->id);
        QVector<QString> row; row.push_back(mark); row.push_back(sid);
        for (auto s : syms)
        {
            QString dst; for (auto e : it->edges) { if (!e.epsilon && e.symbol == s) dst += QString::number(e.to) + ","; }
            if (!dst.isEmpty()) dst.chop(1); row.push_back(dst);
        }
        QString edst; for (auto e : it->edges) { if (e.epsilon) edst += QString::number(e.to) + ","; }
        if (!edst.isEmpty()) edst.chop(1); row.push_back(edst);
        t.rows.push_back(row); t.marks.push_back(mark); t.states.push_back(sid);
    }
    return t;
}

static Tables tableFromDFA(const DFA& dfa)
{
    Tables t; // 列构造：标记、状态集合名与符号列
    QVector<QString> cols; cols.push_back(Config::tableMarkLabel()); cols.push_back(Config::tableStateSetLabel());
    auto syms = dfa.alpha.ordered(); for (auto s : syms) cols.push_back(s);
    t.columns = cols;
    // 行填充：目标以集合名称展示
    for (auto it = dfa.states.begin(); it != dfa.states.end(); ++it)
    {
        QString mark = it->id == dfa.start ? "-" : (it->accept ? "+" : "");
        QString sid  = setName(it->nfaSet);
        QVector<QString> row; row.push_back(mark); row.push_back(sid);
        for (auto s : syms) { int to = it->trans.value(s, -1); row.push_back(to == -1 ? QString() : setName(dfa.states[to].nfaSet)); }
        t.rows.push_back(row); t.marks.push_back(mark); t.states.push_back(sid);
    }
    return t;
}

static Tables tableFromMin(const MinDFA& dfa)
{
    Tables t; // 列构造：标记、状态 id 与符号列
    QVector<QString> cols; cols.push_back(Config::tableMarkLabel()); cols.push_back(Config::tableStateIdLabel());
    auto syms = dfa.alpha.ordered(); for (auto s : syms) cols.push_back(s);
    t.columns = cols;
    // 行填充：目标以状态 id 展示
    for (auto it = dfa.states.begin(); it != dfa.states.end(); ++it)
    {
        QString mark = it->id == dfa.start ? "-" : (it->accept ? "+" : "");
        QString sid  = QString::number(it->id);
        QVector<QString> row; row.push_back(mark); row.push_back(sid);
        for (auto s : syms) { int to = it->trans.value(s, -1); row.push_back(to == -1 ? QString() : QString::number(to)); }
        t.rows.push_back(row); t.marks.push_back(mark); t.states.push_back(sid);
    }
    return t;
}

static void aggregateTableByMacros(Tables& t,
                                   const QMap<QString, QSet<QChar>>& msets,
                                   const QMap<QString, QString>&     mexpr)
{
    // 无宏或列不足时直接返回
    if (msets.isEmpty()) return; int colCount = t.columns.size(); if (colCount < 3) return;
    bool hasEps = (colCount > 0 && t.columns.last() == QStringLiteral("#"));
    int symStart = 2, symEnd = hasEps ? (colCount - 1) : colCount;
    // 建立单字符列索引映射
    QMap<QChar, int> charIdx; for (int ci = symStart; ci < symEnd; ++ci) { const QString& c = t.columns[ci]; if (c.size() == 1) charIdx.insert(c[0], ci); }
    // 记录将被移除的单字符列索引与保留列索引
    QSet<int> removeIdx; QVector<QString> newCols; newCols << t.columns[0] << t.columns[1]; QVector<int> keepIdx;
    auto mkeys = msets.keys(); std::sort(mkeys.begin(), mkeys.end()); QMap<QString, QVector<int>> macroHit;
    // 对每个宏，收集命中的单字符列，并新建宏列标签
    for (const auto& name : mkeys)
    {
        const auto& set = msets.value(name); QVector<int> idxs;
        for (auto ch : set) if (charIdx.contains(ch)) idxs.push_back(charIdx.value(ch));
        if (!idxs.isEmpty())
        {
            std::sort(idxs.begin(), idxs.end()); macroHit.insert(name, idxs);
            for (int id : idxs) removeIdx.insert(id);
            QString label = name; if (mexpr.contains(name)) label += QStringLiteral(" (") + mexpr.value(name) + QStringLiteral(")");
            newCols.push_back(label);
        }
    }
    // 保留未聚合的原始列，并追加 ε 列（若存在）
    for (int ci = symStart; ci < symEnd; ++ci) { if (!removeIdx.contains(ci)) { newCols.push_back(t.columns[ci]); keepIdx.push_back(ci); } }
    if (hasEps) newCols.push_back(QStringLiteral("#"));
    // 合并多个列目标为宏列目标（去重并排序）
    auto mergeTargets = [&](const QStringList& parts)
    {
        QSet<QString> uniq; for (const auto& p : parts) { for (const auto& seg : p.split(',', Qt::SkipEmptyParts)) uniq.insert(seg.trimmed()); }
        QList<QString> v = QList<QString>(uniq.begin(), uniq.end()); std::sort(v.begin(), v.end()); return v.isEmpty() ? QString() : v.join(',');
    };
    // 构造新行：宏列为合并后的目标，保留列照旧，ε 列照旧
    QVector<QVector<QString>> newRows;
    for (const auto& row : t.rows)
    {
        QVector<QString> nr; nr << row[0] << row[1];
        for (const auto& name : mkeys)
        {
            if (!macroHit.contains(name)) continue; QStringList parts; for (int id : macroHit.value(name)) parts << row[id]; nr.push_back(mergeTargets(parts));
        }
        for (int id : keepIdx) nr.push_back(row[id]); if (hasEps) nr.push_back(row[symEnd]); newRows.push_back(nr);
    }
    t.columns = newCols; t.rows = newRows;
}
```

## 单元测试

输入：
- 选取小型规则集（如 `_tok100 = a | b` 或 TINY 规则集合），构造自动机并生成状态表。

预测结果：
- NFA 表存在 ε 列，DFA/MinDFA 表无 ε 列；
- 起始态标记为 `-`，至少一个接受态标记为 `+`；

测试结果：
- 文本表格生成成功；
- 下图展示三类状态表的表头与示例行：
<!--【GUI 截图】此处需要添加 'NFA/DFA/MinDFA 状态表截图'：表头与若干行示例。 -->
