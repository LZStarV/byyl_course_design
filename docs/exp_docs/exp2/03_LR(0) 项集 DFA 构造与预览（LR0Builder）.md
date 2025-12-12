# 03_LR(0) 项集 DFA 构造与预览（LR0Builder）

## 数据结构总览

对象或变量名称 | 数据结构 | 存储结构用途
| - | - | -
LR0Item | 结构体 | 存储左部、右部与点位的项
状态集合 I | 集合 | 若干 LR0Item 构成的 DFA 状态
edges | 映射 | 符号到目标状态的迁移记录
S' | 非终结符 | 增广开始符使初始项集可接受判定
symbols | 集合 | 状态中点后可移进的符号集合
added | 布尔 | 构造过程中是否新增状态的标记

## 算法实现过程
1. 增广文法：在原开始符外包一条 `S'→S` 的产生式，便于构造初始项集与接受判定。
2. 初始项集：以 `S'→•S` 作为起点做闭包，得到 `I0` 并作为 DFA 的第一个状态。
3. 闭包（LR(0)）：对于每个项 `A→α•Bβ`，若 `B` 为非终结符，则将所有 `B→•γ` 的项加入集合；重复直到集合不再增大。
4. 迁移（GOTO）：枚举当前状态中点后符号集合 `symbols`，对每个符号 `X` 将满足条件的项点右移形成新集合，并对该集合做闭包。
5. 去重与入图：若新集合与已有某状态完全相等则复用其索引，否则加入为新状态，并记录一条以 `X` 为标签的迁移边。
6. 终止与输出：当一轮状态扩展没有新增时停止；输出所有状态与边以供预览或导出。

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
