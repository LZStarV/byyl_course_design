# 05 Hopcroft 最小化算法（DFA→MinDFA）

## 数据结构

表 1 最小化中的关键结构与中间变量
对象或变量名称 功能 存储结构
MinDFA 最小化后的自动机（alpha/start/states） 结构体
QList<QSet<int>> P 分区集合（初始为接受/非接受两类） 容器
QSet<int> Pre 某分区在符号上的前驱集合 集合
QMap<int,int> repr 原状态到代表状态（新编号）的映射 映射
int nid 新状态编号计数器 int
Alphabet alpha 字母表（按 ordered() 遍历符号） 自定义类型

## 算法实现过程

1. 初始划分：将 DFA 状态分为“接受态集合 A”与“非接受态集合 N”，为空的集合不入列；以此初始化分区集合 `P`。
2. 驱动细分：外层循环直至稳定，内层对每个分区 `X` 和每个符号 `a`，计算前驱集合 `Pre = { s | trans(s,a) ∈ X }`。
3. 分区切分：对 `P` 中每个分区 `Y`，用 `Pre` 将其二分为 `Y1/Y2`；若均非空则用 `Y1/Y2` 替换 `Y` 并标记发生变化。
4. 达到稳定：当一轮遍历中不再发生任何切分，算法收敛，`P` 即为最终等价类分区集合。
5. 重建最小化：为每个分区选择一个代表 `rid`，建立映射 `repr[s]=nid` 并递增 `nid`；按原转移将目标重映射到代表编号，得到 `MinDFA.states` 与 `MinDFA.start`。
下图展示 Hopcroft 划分的过程示意（初始划分与若干轮细分后的分区对比）：
<!--【示意图】此处需要添加 'Hopcroft 划分示意图'：分区迭代与重建过程。 -->

## 详细过程讲解

- 分割条件：
  - 若分区内的状态对在某符号上的转移落到不同的分区，则该分区按该符号被细分。
  - 使用待处理队列维护“需要检查的分区与符号组合”，直至队列为空。

- 重编号与表结构：
  - 根据最终分区顺序为新状态分配 `id`，并为每个符号建立 `trans` 到目标分区 id。

## 各种情况讲解

- 不可达状态：可在最小化前剔除或在分区过程中保持为独立分区，最终是否保留由实现决定。
- 等价类边界：字符集富集时，某些状态在多个符号上行为一致但在某类上不同，从而触发细分。

## 关键代码片段

```
// Hopcroft 算法入口：对 DFA 进行最小化（按等价关系划分）
class Hopcroft { public: static MinDFA minimize(const DFA& dfa); };

// 用于展示最小化后的 DFA 状态表
static Tables tableFromMin(const MinDFA& dfa);
```

```
// Hopcroft 主过程：按输入符号驱动分区细分，最终重建最小化 DFA
MinDFA Hopcroft::minimize(const DFA& dfa)
{
    MinDFA m; m.alpha = dfa.alpha;
    // 初始划分：接受态与非接受态两类
    QSet<int> A, N;
    for (auto it = dfa.states.begin(); it != dfa.states.end(); ++it)
        if (it->accept) A.insert(it->id); else N.insert(it->id);
    QList<QSet<int>> P; if (!A.isEmpty()) P.append(A); if (!N.isEmpty()) P.append(N);
    // 迭代细分：按符号的前驱集合划分分区
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (int i = 0; i < P.size(); ++i)
        {
            auto X = P[i];
            for (auto a : m.alpha.ordered())
            {
                QSet<int> Pre;
                for (auto it2 = dfa.states.begin(); it2 != dfa.states.end(); ++it2)
                {
                    int t = it2->trans.value(a, -1);
                    if (t != -1 && X.contains(t)) Pre.insert(it2->id);
                }
                if (Pre.isEmpty()) continue;
                QList<QSet<int>> newP;
                for (auto Y : P)
                {
                    QSet<int> Y1, Y2;
                    for (int s : Y) { if (Pre.contains(s)) Y1.insert(s); else Y2.insert(s); }
                    // 如果分区被切分，则标记变更并替换分区集合
                    if (!Y1.isEmpty() && !Y2.isEmpty()) { newP.append(Y1); newP.append(Y2); changed = true; }
                    else { newP.append(Y); }
                }
                if (changed) { P = newP; break; }
            }
            if (changed) break;
        }
    }
    // 重建最小化 DFA：为每个最终分区分配新 id，并重映射转移
    QMap<int, int> repr; int nid = 1;
    for (auto block : P) { int rid = *block.begin(); for (int s : block) repr[s] = nid; nid++; }
    for (auto it = dfa.states.begin(); it != dfa.states.end(); ++it)
    {
        int ns = repr[it->id];
        if (!m.states.contains(ns)) { DFAState ds; ds.id = ns; ds.accept = it->accept; m.states.insert(ns, ds); }
        for (auto a : m.alpha.ordered()) { int t = it->trans.value(a, -1); if (t != -1) m.states[ns].trans[a] = repr[t]; }
    }
    m.start = repr[dfa.start];
    return m;
}
```

## 单元测试

输入：
```
_tok101 = (a|b)*
```

预测结果：
- 最小化后的 MinDFA 具有有效的起始态，并能在状态集合中找到；
- 最小化过程对等价状态进行合并，保持可达结构。

测试结果：
- 起始态存在且可达的断言通过；
- GUI 截图：自动机核心单元测试（FACoreTest）MinDFA 部分的运行截图（含起始态断言结果）。
<!--【GUI 截图】此处需要添加 'FACoreTest MinDFA 运行截图'：显示起始态存在且可达的断言结果。 -->
下图展示“闭包（Star）”表达式最小化前后结构示意：
<!--【示意图】此处需要添加 'Hopcroft 划分示意图'：初始划分与若干轮细分后的分区对比。 -->
