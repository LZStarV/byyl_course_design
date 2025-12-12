# 05_LR(1) 项集 DFA 构造与预览（LR1Builder）

## 数据结构总览

对象或变量名称 | 数据结构 | 存储结构用途
| - | - | -
LR1Item.left/right/dot/lookahead | 结构体字段 | 项的左部/右部序列/点位/前瞻终结符
LR1Graph.states | QVector<QVector<LR1Item>> | 含向前看的项集族
LR1Graph.edges | QMap<int, QMap<QString,int>> | 状态迁移边（i×符号→目标 j）
firstSeqLL1(seq, la) | 子过程 | 计算 FIRST(β+a) 的集合
closureLL1(items) | 子过程 | LR(1) 闭包（基于 FIRST(β+a) 展开）
goToLL1(items, X) | 子过程 | 对符号 X 右移点并再闭包
serializeSet(items) | 函数 | 项集序列化为稳定字符串键（去重用）
Config::eofSymbol()/augSuffix() | 常量 | `$`（EOF）与增广后缀
nextSymbols/key/changed | 变量 | 点后符号集/序列化键/是否新增状态标记

## 算法实现过程
1. 增广文法：在开始符外包一条 `S'→S` 的产生式，并以 `$` 作为初始前瞻终结符。
2. 初始项集：以 `S'→•S / $` 作为起点做 LR(1) 闭包，得到 `I0` 并作为首个状态。
3. FIRST(β+a)：求序列 `β` 的 `FIRST`，若全可空则将外部前瞻 `a` 加入；否则返回 `FIRST(β)` 去除 `#` 的集合。
4. 闭包（LR(1)）：对每个项 `A→α•Bβ / a`，使用 `FIRST(β+a)` 产生前瞻集合；为其中每个 `a'` 加入派生项 `B→•γ / a'`；重复直到集合稳定。
5. 迁移（GOTO）：枚举状态中点后符号集合，对每个符号 `X` 将满足条件的项点右移形成新集合，并对其做 LR(1) 闭包。
6. 去重与入图：将项集序列化为稳定键；若已存在则复用索引，否则加入新状态并记录迁移边。
7. 终止与输出：迭代至不再新增状态；输出所有状态与边用于预览或导出。

### 扩展要点（细节说明）
- 序列化键：将 `left/right/dot/lookahead` 序列化为字符串键用于去重；确保同集合具有同键，避免重复状态。
- `$` 的处理：作为终结符参与 FIRST/FOLLOW；在初始闭包中作为唯一前瞻，推动接受项生成。
- 非终结符识别：仅当点后为非终结符时展开闭包；点后为终结符或 `#` 时不展开，保证集合规模可控。
- GOTO 后闭包：先右移点构造基本项集，再做 LR(1) 闭包以补全派生项，确保项集语义完整。

## 算法实现流程图
<!--【流程图】此处需要添加 “LR(1) 项集 DFA 构造流程图”：增广→I0 闭包（含前瞻）→遍历符号→GOTO（闭包）→去重入图→迭代至稳定。-->

## 关键代码（可选）
```
// LR(1) 闭包（项目源码节选，不超过100行）
static QVector<LR1Item> closureLL1(const Grammar& g, const LL1Info& info, const QVector<LR1Item>& I)
{
    QVector<LR1Item> items = I;
    QSet<QString>    seen;  // serialize item as string key
    auto             key = [](const LR1Item& it)
    {
        return it.left + "->" + it.right.join(" ") + "." + QString::number(it.dot) + "/" +
               it.lookahead;
    };
    for (const auto& it : items) seen.insert(key(it));
    bool changed = true;
    while (changed)
    {
        changed = false;
        int n   = items.size();
        for (int i = 0; i < n; ++i)
        {
            const auto& it = items[i];
            if (it.dot < it.right.size())
            {
                QString B = it.right[it.dot];
                if (!isTerminal(g.terminals, B) && B != Config::epsilonSymbol())
                {
                    QVector<QString> beta;
                    for (int k = it.dot + 1; k < it.right.size(); ++k) beta.push_back(it.right[k]);
                    auto lookaheads = firstSeqLL1(g, info, beta, it.lookahead);
                    for (const auto& prod : g.productions.value(B))
                    {
                        for (const auto& a : lookaheads)
                        {
                            LR1Item ni{prod.left, prod.right, 0, a};
                            QString sk = key(ni);
                            if (!seen.contains(sk))
                            {
                                items.push_back(ni);
                                seen.insert(sk);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }
    return items;
}
```

## 各种情况讲解（示例）
- 空后缀前瞻传播：当 `β` 全可空时，`FIRST(β+a)` 返回外部前瞻 `a`，确保向后派生项带入正确的 lookahead。
- 非终结符展开限制：闭包仅在点后为非终结符时展开，点后若为终结符或空串符号则不展开，避免无效项膨胀。

## 单元测试
- 输入：
  - 表达式文法（含 `E/E'/T/T'/F`，`#/$`）
- 预期结果：
  - 初始项集 `S'→•S / $` 的闭包包含以 `$` 为前瞻的派生项；多个状态项的 `lookahead` 差异可见；迁移覆盖终结符与非终结符。
- 测试结果：
  - 命令行截图：运行 LR(1) 构造与导出 DOT 的测试，可得项集与迁移总览；
  - GUI 截图：在实验二界面点击“LR(1) 预览”按钮，弹出图像预览窗口。
  - <!--【GUI 截图】此处需要添加 “LR(1) DFA 预览” 截图：节点标签含项与前瞻，边为符号迁移。-->
