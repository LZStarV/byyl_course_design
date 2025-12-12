# 02_FIRST 与 FOLLOW 计算（LL1）

## 数据结构总览

对象或变量名称 | 数据结构 | 存储结构用途
| - | - | -
FIRST(X) | 集合 | 符号 X 的首符集合；终结符 a 的 FIRST(a)={a}
FOLLOW(A) | 集合 | 非终结符 A 的后继终结符集合；开始符含 `$`
TABLE | 映射 | 预测表，TABLE[A][a]=产生式索引 k
`#`/`$` | 特殊符号 | 分别表示空串与输入结束
changed | 布尔 | FIRST/FOLLOW 迭代是否变化的标记
seq | 序列 | 产生式右部符号序列
k | 整数 | 产生式在 A 的候选中的索引

## 算法实现过程
1. FIRST 初始化：为每个终结符 `a` 设置 `FIRST(a)={a}`；为每个非终结符建立空集。
2. FIRST 迭代：对每个产生式 `A→α`，计算 `FIRST(α)` 并并入 `FIRST(A)`；当任何集合大小增加时继续迭代直至不动点。
3. FOLLOW 初始化：开始符的 FOLLOW 包含 `$`；为每个非终结符建立空集。
4. FOLLOW 迭代：对每个产生式与右部位置 `A→α B β`，将 `FIRST(β)` 去除 `#` 并入 `FOLLOW(B)`；若 `β` 为空或 `FIRST(β)` 含 `#`，将 `FOLLOW(A)` 并入 `FOLLOW(B)`；迭代至不动点。
5. 预测表生成：对每个产生式 `A→α`：
   - 对 `FIRST(α)` 中所有非 `#` 的终结符 `a`，置 `TABLE[A][a]=k`
   - 若 `FIRST(α)` 含 `#`，对所有 `b ∈ FOLLOW(A)`，置 `TABLE[A][b]=k`
6. 冲突记录：当同一表项拟写入不同产生式索引时，记录为冲突以便后续展示与调试。

## 算法实现流程图
<!--【流程图】此处需要添加 “FIRST/FOLLOW 计算与 LL(1) 表生成”：初始化→FIRST 迭代→FOLLOW 迭代→生成表→记录冲突。-->

## 关键代码（可选）
```
// FIRST(序列) 与 FIRST/FOLLOW 计算（项目源码节选，已格式化）
static QSet<QString>
firstSeq(const Grammar& g,
         const QVector<QString>& seq,
         const QMap<QString, QSet<QString>>& first)
{
    QSet<QString> r;
    bool allEps = true;

    for (const auto& s : seq)
    {
        if (g.terminals.contains(s))
        {
            r.insert(s);
            allEps = false;
            break;
        }

        auto fs = first.value(s);
        for (const auto& t : fs)
            if (t != Config::epsilonSymbol())
                r.insert(t);

        if (!fs.contains(Config::epsilonSymbol()))
        {
            allEps = false;
            break;
        }
    }

    if (allEps)
        r.insert(Config::epsilonSymbol());

    return r;
}

LL1Info LL1::compute(const Grammar& g)
{
    LL1Info info;

    // 初始化 FIRST
    for (const auto& A : g.nonterminals)
        info.first.insert(A, QSet<QString>());
    for (const auto& a : g.terminals)
    {
        info.first.insert(a, QSet<QString>());
        info.first[a].insert(a);
    }

    // 迭代计算 FIRST
    bool changed = true;
    while (changed)
    {
        changed = false;
        // ......（遍历产生式并并入 FIRST）
        for (auto it = g.productions.begin(); it != g.productions.end(); ++it)
        {
            const QString& A = it.key();
            for (const auto& p : it.value())
            {
                auto set  = firstSeq(g, p.right, info.first);
                int  prev = info.first[A].size();
                for (const auto& x : set)
                    info.first[A].insert(x);
                if (info.first[A].size() > prev)
                    changed = true;
            }
        }
    }

    // 初始化 FOLLOW
    for (const auto& A : g.nonterminals)
        info.follow.insert(A, QSet<QString>());
    if (!g.startSymbol.isEmpty())
        info.follow[g.startSymbol].insert(Config::eofSymbol());

    // 迭代计算 FOLLOW
    changed = true;
    while (changed)
    {
        changed = false;
        // ......（遍历产生式并并入 FOLLOW）
        for (auto it = g.productions.begin(); it != g.productions.end(); ++it)
        {
            const QString& A = it.key();
            for (const auto& p : it.value())
            {
                for (int i = 0; i < p.right.size(); ++i)
                {
                    const auto& B = p.right[i];
                    if (!g.nonterminals.contains(B))
                        continue;

                    QVector<QString> beta;
                    for (int j = i + 1; j < p.right.size(); ++j)
                        beta.push_back(p.right[j]);

                    auto fbeta = firstSeq(g, beta, info.first);
                    int  prev  = info.follow[B].size();

                    for (const auto& x : fbeta)
                        if (x != "#")
                            info.follow[B].insert(x);

                    if (beta.isEmpty() || fbeta.contains("#"))
                    {
                        for (const auto& x : info.follow[A])
                            info.follow[B].insert(x);
                    }

                    if (info.follow[B].size() > prev)
                        changed = true;
                }
            }
        }
    }

    return info;
}
```

## 单元测试
- 输入：
  - 表达式文法（`E/E'/T/T'/F`）
- 预期结果：
  - `FIRST(F)={(,id}`，`FIRST(T')={*,#}`；`FOLLOW(E)={(,),$}`；
  - 预测表关键项符合期望（如 `TABLE[E'][+]` 指向 `E'→+ T E'`）。
- 测试结果：
  - 命令行截图：运行 FIRST/FOLLOW 计算的测试，展示集合与预测表摘要；
  - GUI 截图：在实验二界面解析文法后，FIRST/FOLLOW 两张表中显示集合内容。
  - <!--【GUI 截图】此处需要添加 “FIRST/FOLLOW 表格” 截图：非终结符×终结符集合值。-->
