# 02_FIRST 与 FOLLOW 计算（LL1）

## 数据结构总览

对象或变量名称 | 数据结构 | 存储结构用途
- | - | -
LL1Info.first | QMap<QString, QSet<QString>> | FIRST 集合映射（符号→集合）
LL1Info.follow | QMap<QString, QSet<QString>> | FOLLOW 集合映射（非终结符→集合）
LL1Info.table | QMap<QString, QMap<QString,int>> | LL(1) 预测表（A×a→产生式索引）
LL1Info.conflicts | QVector<QString> | 表项冲突记录（文本说明）
firstSeq(const QVector<QString>&) | 子过程 | 计算序列的 FIRST，保留 `#` 语义
Grammar.nonterminals/terminals | QSet<QString> | 非终结符/终结符集合（初始化用）
Config::epsilonSymbol()/eofSymbol() | 常量 | `#`（epsilon）与 `$`（EOF）符号
changed | bool | FIRST/FOLLOW 迭代是否发生变化
seq/k | QVector<QString>/int | 右部序列与产生式索引（填表用）

## 算法实现过程
1. FIRST 初始化：为每个终结符 `a` 设置 `FIRST(a)={a}`；为每个非终结符建立空集。
2. FIRST 迭代：对每个产生式 `A→α`，计算 `FIRST(α)` 并并入 `FIRST(A)`；当任何集合大小增加时继续迭代直至不动点。
3. FOLLOW 初始化：开始符的 FOLLOW 包含 `$`；为每个非终结符建立空集。
4. FOLLOW 迭代：对每个产生式与右部位置 `A→α B β`，将 `FIRST(β)` 去除 `#` 并入 `FOLLOW(B)`；若 `β` 为空或 `FIRST(β)` 含 `#`，将 `FOLLOW(A)` 并入 `FOLLOW(B)`；迭代至不动点。
5. 预测表生成：对每个产生式 `A→α`：
   - 对 `FIRST(α)` 中所有非 `#` 的终结符 `a`，置 `TABLE[A][a]=k`
   - 若 `FIRST(α)` 含 `#`，对所有 `b ∈ FOLLOW(A)`，置 `TABLE[A][b]=k`
6. 冲突记录：当同一表项拟写入不同产生式索引时，记录为冲突以便后续展示与调试。

### 扩展要点（细节说明）
- FIRST(序列)处理：逐符扫描 `seq`；遇终结符立即加入并停止；遇非终结符加入其 FIRST 去除 `#`；若其 FIRST 不含 `#` 即停止；所有符号均可空时将 `#` 写入结果。
- FOLLOW 边界：开始符 FOLLOW 必含 `$`；对形如 `A→αB`（β 为空）的情况，将 `FOLLOW(A)` 并入 `FOLLOW(B)`，保持链式传播。
- 去重与排序：集合使用去重结构；在展示表格时按字母序排序，便于人工校验一致性。
- 表填充优先级：`FIRST(α)\{#}` 优先填表；仅当 `FIRST(α)` 含 `#` 时才对 `FOLLOW(A)` 填表，避免表项覆盖。
- 冲突说明：记录冲突位置但不决策；后续由解析器或策略处理；典型冲突来自左递归或公共前缀未拆分的文法。

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
