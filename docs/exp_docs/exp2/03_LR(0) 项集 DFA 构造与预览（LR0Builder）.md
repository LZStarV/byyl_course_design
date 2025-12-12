# 03_LR(0) 项集 DFA 构造与预览（LR0Builder）

## 数据结构总览

对象或变量名称 | 数据结构 | 存储结构用途
- | - | -
LR0Item.left/right/dot | 结构体字段 | 项的左部/右部序列/点位
LR0Graph.states | QVector<QVector<LR0Item>> | 项集族（每个状态的项集合）
LR0Graph.edges | QMap<int, QMap<QString,int>> | 状态迁移边（i×符号→目标 j）
symbols | QSet<QString> | 状态中点后可移进的符号集合
equalSet(a,b) | 函数 | 判断两项集合是否相等（去重用）
closure(const QVector<LR0Item>&) | 子过程 | LR(0) 闭包（对点后非终结符展开）
gotoSet(const QVector<LR0Item>&, const QString&) | 子过程 | 对符号 X 右移点得到集合
Config::augSuffix() | 常量 | 增广后缀（形成 S'）
added | bool | 构造过程中是否新增状态的标记

## 算法实现过程
1. 增广文法：在原开始符外包一条 `S'→S` 的产生式，便于构造初始项集与接受判定。
2. 初始项集：以 `S'→•S` 作为起点做闭包，得到 `I0` 并作为 DFA 的第一个状态。
3. 闭包（LR(0)）：对于每个项 `A→α•Bβ`，若 `B` 为非终结符，则将所有 `B→•γ` 的项加入集合；重复直到集合不再增大。
4. 迁移（GOTO）：枚举当前状态中点后符号集合 `symbols`，对每个符号 `X` 将满足条件的项点右移形成新集合，并对该集合做闭包。
5. 去重与入图：若新集合与已有某状态完全相等则复用其索引，否则加入为新状态，并记录一条以 `X` 为标签的迁移边。
6. 终止与输出：当一轮状态扩展没有新增时停止；输出所有状态与边以供预览或导出。

### 扩展要点（细节说明）
- 集合相等判定：按项的 `left/right/dot` 全量相等判断；顺序不敏感（集合语义），避免重复状态。
- 符号枚举：仅枚举行内项的点后符号，减少不必要 GOTO 计算；保证每状态的出边覆盖真实可能迁移。
- DOT 导出：将项集合序列化为标签，使用 `•` 标示点位；对引号与反斜杠做转义，确保 Graphviz 兼容。
- 性能提示：闭包与去重为主要开销；在大文法上建议缓存闭包结果与高效集合比较。

## 算法实现流程图
<!--【流程图】此处需要添加 “LR(0) 项集 DFA 构造流程图”：增广→I0 闭包→遍历符号→GOTO→去重入图→迭代至稳定。-->

## 关键代码（可选）
```
// 闭包与构造（项目源码节选，已格式化）
static QVector<LR0Item>
closure(const QVector<LR0Item>& I, const Grammar& g)
{
    QVector<LR0Item> res = I;
    bool changed = true;

    while (changed)
    {
        changed = false;
        for (const auto& it : res)
        {
            if (it.dot < it.right.size())
            {
                const QString& B = it.right[it.dot];
                if (g.nonterminals.contains(B))
                {
                    const auto prods = g.productions.value(B);
                    for (const auto& p : prods)
                    {
                        LR0Item ni{B, p.right, 0};
                        bool exists = false;
                        for (const auto& x : res)
                        {
                            if (x == ni) { exists = true; break; }
                        }
                        if (!exists)
                        {
                            res.push_back(ni);
                            changed = true;
                        }
                    }
                }
            }
        }
    }
    return res;
}

LR0Graph LR0Builder::build(const Grammar& g)
{
    Grammar aug = g;
    QString Sdash = g.startSymbol + Config::augSuffix();

    if (!aug.productions.contains(Sdash))
    {
        Production p;
        p.left  = Sdash;
        p.right = QVector<QString>{g.startSymbol};
        p.line  = -1;
        aug.productions[Sdash].push_back(p);
        aug.nonterminals.insert(Sdash);
    }

    LR0Graph gr;
    QVector<LR0Item> I0;
    I0.push_back(LR0Item{Sdash, QVector<QString>{g.startSymbol}, 0});
    I0 = closure(I0, aug);
    gr.states.push_back(I0);

    bool added = true;
    while (added)
    {
        added = false;
        int n = gr.states.size();
        for (int i = 0; i < n; ++i)
        {
            QSet<QString> symbols;
            for (const auto& it : gr.states[i])
                if (it.dot < it.right.size())
                    symbols.insert(it.right[it.dot]);

            for (const auto& X : symbols)
            {
                auto J = gotoSet(gr.states[i], X, aug);
                if (J.isEmpty())
                    continue;

                int existing = -1;
                for (int k = 0; k < gr.states.size(); ++k)
                {
                    if (equalSet(gr.states[k], J))
                    {
                        existing = k;
                        break;
                    }
                }

                if (existing < 0)
                {
                    gr.states.push_back(J);
                    existing = gr.states.size() - 1;
                    added    = true;
                }

                gr.edges[i][X] = existing;
            }
        }
    }

    return gr;
}
```

## 单元测试
- 输入：
  - 表达式文法（含 `+/*/()/id`）
- 预期结果：
  - 初始项集 `I0` 含增广项与其闭包项；状态数与边数均大于 0；
  - 迁移边存在针对 `+`、`*`、括号与 `id` 的前进。
- 测试结果：
  - 命令行截图：运行 LR(0) 构造并导出 DOT 的测试，显示状态与迁移；
  - GUI 截图：在实验二界面点击“LR(0) 预览”，弹出图像预览窗口。
  - <!--【GUI 截图】此处需要添加 “LR(0) DFA 预览” 截图：节点为项集合并标签，边为符号迁移。-->
