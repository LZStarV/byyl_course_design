# 04_SLR(1) 判定与原因呈现（SLR）

## 数据结构总览

对象或变量名称 | 数据结构 | 存储结构用途
- | - | -
SLRCheckResult.isSLR1 | bool | 是否为 SLR(1) 的结论标记
SLRCheckResult.conflicts | QVector<SLRConflict> | 冲突条目集合
SLRConflict.state/terminal/type/detail | 结构体字段 | 状态编号/终结符/类型/明细字符串
actionsSet | QMap<int, QMap<QString, QSet<QString>>> | `(state, terminal) → 动作集合` 聚合结构
act 字符串 | QString | `sN`（移进）/`r A -> α`（规约）动作文本
LR0Graph.states/edges | 结构 | LR(0) 项集与迁移边（移进依据）
LL1Info.follow | 映射 | FOLLOW 集（规约分发依据）

## 算法实现过程
1. LR(0) 支持集：先构造 LR(0) 项集 DFA，用于确定移进的目标状态。
2. 移进动作聚合：若项的点后是终结符 `a`，且存在以 `a` 为标签的迁移边，则将 `sN` 加入对应 `(state, a)` 的动作集合。
3. 规约动作聚合：若项点位在末尾（产生式完成），则对 `FOLLOW(left)` 中的每个终结符 `b` 加入规约动作 `r left -> right` 到 `(state, b)` 的集合。
4. 冲突检测：当某个 `(state, terminal)` 的动作集合大小 ≥ 2：
   - 同时出现移进与规约 → 类型为 `shift/reduce`
   - 出现两个及以上规约 → 类型为 `reduce/reduce`
   - 将动作按字典序拼接为 `detail`，便于展示与定位
5. 结论输出：若冲突集合为空，则判定为 SLR(1)；否则输出冲突列表与类型说明。

### 扩展要点（细节说明）
- 动作字符串：移进以 `sN` 表示目标状态；规约以 `r A -> α` 表示具体产生式；右部为空以 `#` 表示。
- 归约分发依据：严格使用 FOLLOW 集而非项内前瞻；这是 SLR(1) 判定的根本依据，可能在某些文法上比 LR(1) 更宽松或更严格。
- 冲突分类：`shift/reduce` 多见于公共前缀未消除的文法；`reduce/reduce` 多见于多个候选右部均可空且 FOLLOW 集重叠的情况。
- 输出稳定性：冲突明细排序后拼接，便于重复运行时保持一致的呈现顺序。

## 算法实现流程图
<!--【流程图】此处需要添加 “SLR(1) 判定流程图”：构造 LR(0)→聚合移进/规约→检测并分类冲突→输出结论与明细。-->

## 关键代码（可选）
```
// SLR(1) 检查（项目源码节选，已格式化）
SLRCheckResult SLR::check(const Grammar& g, const LL1Info& ll1)
{
    auto gr = LR0Builder::build(g);
    QMap<int, QMap<QString, QSet<QString>>> actionsSet;

    for (int st = 0; st < gr.states.size(); ++st)
    {
        const auto& items = gr.states[st];
        for (const auto& it : items)
        {
            if (it.dot < it.right.size())
            {
                QString a = it.right[it.dot];
                if (g.terminals.contains(a))
                {
                    int to = gr.edges.value(st).value(a, -1);
                    if (to >= 0)
                    {
                        actionsSet[st][a].insert(QString("s%1").arg(to));
                    }
                }
            }
            else
            {
                auto    followA = ll1.follow.value(it.left);
                QString rhsText = it.right.isEmpty() ? QString("#") : it.right.join(" ");
                QString red     = QString("r %1 -> %2").arg(it.left).arg(rhsText);
                for (const auto& a : followA)
                {
                    actionsSet[st][a].insert(red);
                }
            }
        }
    }

    SLRCheckResult res;
    for (auto sit = actionsSet.begin(); sit != actionsSet.end(); ++sit)
    {
        int st = sit.key();
        for (auto ait = sit.value().begin(); ait != sit.value().end(); ++ait)
        {
            const QString& a   = ait.key();
            const auto&    set = ait.value();
            if (set.size() >= 2)
            {
                bool hasShift  = false;
                bool hasReduce = false;
                for (const auto& act : set)
                {
                    if (act.startsWith("s")) hasShift  = true;
                    if (act.startsWith("r")) hasReduce = true;
                }

                QStringList list = QStringList(set.begin(), set.end());
                std::sort(list.begin(), list.end());

                SLRConflict c;
                c.state    = st;
                c.terminal = a;
                c.type     = (hasShift && hasReduce) ? "shift/reduce" : "reduce/reduce";
                c.detail   = list.join("|");
                res.conflicts.push_back(c);
            }
        }
    }

    res.isSLR1 = res.conflicts.isEmpty();
    return res;
}
```

## 单元测试
- 输入：
  - 经典表达式文法（可能产生移进/规约冲突）
- 预期结果：
  - 判定“不是 SLR(1) 文法”；至少出现一条 `shift/reduce` 或 `reduce/reduce` 冲突明细。
- 测试结果：
  - 命令行截图：运行 SLR(1) 检查的测试，输出结论与冲突列表；
  - GUI 截图：在实验二界面点击“SLR(1) 检查”，弹窗展示结论与冲突明细。
  - <!--【GUI 截图】此处需要添加 “SLR(1) 检查结果” 截图：结论行与若干冲突明细示例。-->
