#include "SubsetConstruction.h"
#include <QQueue>
static QString setName(const QSet<int>& s)
{
    QString    r = "{";
    QList<int> v = QList<int>(s.begin(), s.end());
    std::sort(v.begin(), v.end());
    for (int i = 0; i < v.size(); ++i)
    {
        r += QString::number(v[i]);
        if (i + 1 < v.size())
            r += ", ";
    }
    r += "}";
    return r;
}
// 计算集合 S 的 ε-闭包：从 S 出发，经 ε 边可达的全部状态
static QSet<int> epsilonClosure(const NFA& nfa, const QSet<int>& S)
{
    QSet<int>   c = S;
    QQueue<int> q;
    for (int s : S) q.enqueue(s);
    while (!q.isEmpty())
    {
        int  u  = q.dequeue();
        auto it = nfa.states.find(u);
        if (it == nfa.states.end())
            continue;
        for (auto e : it->edges)
        {
            if (e.epsilon)
            {
                if (!c.contains(e.to))
                {
                    c.insert(e.to);
                    q.enqueue(e.to);
                }
            }
        }
    }
    return c;
}
// move：在给定输入符号 a 下，从集合 S 通过显式转移可达的集合
static QSet<int> move(const NFA& nfa, const QSet<int>& S, const QString& a)
{
    QSet<int> r;
    for (int u : S)
    {
        auto it = nfa.states.find(u);
        if (it == nfa.states.end())
            continue;
        for (auto e : it->edges)
        {
            if (!e.epsilon && e.symbol == a)
                r.insert(e.to);
        }
    }
    return r;
}
// 子集构造主过程：按字母表扩展状态集，建立 DFA 转移
DFA SubsetConstruction::build(const NFA& nfa)
{
    DFA d;
    d.alpha                   = nfa.alpha;
    int                nextId = 1;
    QMap<QString, int> idmap;
    QQueue<QSet<int>>  work;
    QSet<int>          startSet;
    startSet.insert(nfa.start);
    auto st          = epsilonClosure(nfa, startSet);
    auto nameStart   = setName(st);
    idmap[nameStart] = nextId;
    DFAState d0;
    d0.id     = nextId;
    d0.nfaSet = st;
    d0.accept = false;
    for (int s : st)
    {
        if (nfa.states[s].accept)
            d0.accept = true;
    }
    d.states.insert(nextId, d0);
    d.start = nextId;
    work.enqueue(st);
    nextId++;
    // 逐步扩展工作队列中的状态集合，生成对应 DFA 状态与转移
    while (!work.isEmpty())
    {
        auto S     = work.dequeue();
        auto Sname = setName(S);
        int  sid   = idmap[Sname];
        for (auto a : d.alpha.ordered())
        {
            auto U = epsilonClosure(nfa, move(nfa, S, a));
            if (U.isEmpty())
                continue;
            auto Uname = setName(U);
            int  uid;
            if (!idmap.contains(Uname))
            {
                uid          = nextId;
                idmap[Uname] = uid;
                DFAState ds;
                ds.id     = uid;
                ds.nfaSet = U;
                ds.accept = false;
                for (int s : U)
                {
                    if (nfa.states[s].accept)
                        ds.accept = true;
                }
                d.states.insert(uid, ds);
                work.enqueue(U);
                nextId++;
            }
            else
                uid = idmap[Uname];
            d.states[sid].trans[a] = uid;
        }
    }
    return d;
}
