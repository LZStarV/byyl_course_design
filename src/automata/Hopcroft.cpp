#include "Hopcroft.h"
#include <QMap>
#include <QSet>
// Hopcroft 最小化主过程：按输入划分并细分等价类，构造代表状态映射
MinDFA Hopcroft::minimize(const DFA& dfa)
{
    MinDFA m;
    m.alpha = dfa.alpha;
    QSet<int> A, N;
    for (auto it = dfa.states.begin(); it != dfa.states.end(); ++it)
    {
        if (it->accept)
            A.insert(it->id);
        else
            N.insert(it->id);
    }
    QList<QSet<int>> P;
    if (!A.isEmpty())
        P.append(A);
    if (!N.isEmpty())
        P.append(N);
    bool changed = true;
    // 迭代细分分区，直到不再发生变化
    while (changed)
    {
        changed = false;
        for (int i = 0; i < P.size(); ++i)
        {
            auto X = P[i];
            for (auto a : m.alpha.ordered())
            {
                QSet<int> In;
                for (int s : X)
                {
                    int t = dfa.states[s].trans.value(a, -1);
                    if (t != -1)
                        In.insert(t);
                }
                if (In.isEmpty())
                    continue;
                QList<QSet<int>> newP;
                // 对当前分区 P 中的每个块 Y，按 In 划分为 Y1/Y2
                for (auto Y : P)
                {
                    QSet<int> Y1, Y2;
                    for (int s : Y)
                    {
                        if (In.contains(s))
                            Y1.insert(s);
                        else
                            Y2.insert(s);
                    }
                    if (!Y1.isEmpty() && !Y2.isEmpty())
                    {
                        newP.append(Y1);
                        newP.append(Y2);
                        changed = true;
                    }
                    else
                        newP.append(Y);
                }
                if (changed)
                {
                    P = newP;
                    break;
                }
            }
            if (changed)
                break;
        }
    }
    QMap<int, int> repr;
    int            nid = 1;
    for (auto block : P)
    {
        int rid = *block.begin();
        for (int s : block) repr[s] = nid;
        nid++;
    }
    // 构建最小化后的状态与转移：将原状态映射为代表状态
    for (auto it = dfa.states.begin(); it != dfa.states.end(); ++it)
    {
        int ns = repr[it->id];
        if (!m.states.contains(ns))
        {
            DFAState ds;
            ds.id     = ns;
            ds.accept = it->accept;
            m.states.insert(ns, ds);
        }
        for (auto a : m.alpha.ordered())
        {
            int t = it->trans.value(a, -1);
            if (t != -1)
            {
                m.states[ns].trans[a] = repr[t];
            }
        }
    }
    m.start = repr[dfa.start];
    return m;
}
