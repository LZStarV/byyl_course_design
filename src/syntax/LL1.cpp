#include "LL1.h"

static QSet<QString> firstSeq(const Grammar& g, const QVector<QString>& seq,
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
            if (t != "#") r.insert(t);
        if (!fs.contains("#"))
        {
            allEps = false;
            break;
        }
    }
    if (allEps) r.insert("#");
    return r;
}

LL1Info LL1::compute(const Grammar& g)
{
    LL1Info info;
    for (const auto& A : g.nonterminals)
        info.first.insert(A, QSet<QString>());
    for (const auto& a : g.terminals)
        info.first.insert(a, QSet<QString>());
    for (const auto& a : g.terminals)
        info.first[a].insert(a);
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (auto it = g.productions.begin(); it != g.productions.end(); ++it)
        {
            const QString& A = it.key();
            for (const auto& p : it.value())
            {
                auto set = firstSeq(g, p.right, info.first);
                int  prev = info.first[A].size();
                for (const auto& x : set) info.first[A].insert(x);
                if (info.first[A].size() > prev) changed = true;
            }
        }
    }
    for (const auto& A : g.nonterminals)
        info.follow.insert(A, QSet<QString>());
    if (!g.startSymbol.isEmpty()) info.follow[g.startSymbol].insert("$");
    changed = true;
    while (changed)
    {
        changed = false;
        for (auto it = g.productions.begin(); it != g.productions.end(); ++it)
        {
            const QString& A = it.key();
            for (const auto& p : it.value())
            {
                for (int i = 0; i < p.right.size(); ++i)
                {
                    const auto& B = p.right[i];
                    if (!g.nonterminals.contains(B)) continue;
                    QVector<QString> beta;
                    for (int j = i + 1; j < p.right.size(); ++j) beta.push_back(p.right[j]);
                    auto fbeta = firstSeq(g, beta, info.first);
                    int  prev  = info.follow[B].size();
                    for (const auto& x : fbeta)
                        if (x != "#") info.follow[B].insert(x);
                    if (beta.isEmpty() || fbeta.contains("#"))
                    {
                        for (const auto& x : info.follow[A]) info.follow[B].insert(x);
                    }
                    if (info.follow[B].size() > prev) changed = true;
                }
            }
        }
    }
    for (auto it = g.productions.begin(); it != g.productions.end(); ++it)
    {
        const QString& A = it.key();
        for (int k = 0; k < it.value().size(); ++k)
        {
            const auto& p = it.value()[k];
            auto fs       = firstSeq(g, p.right, info.first);
            for (const auto& a : fs)
            {
                if (a == "#") continue;
                if (info.table[A].contains(a))
                    info.conflicts.push_back(A + "/" + a);
                info.table[A][a] = k;
            }
            if (fs.contains("#"))
            {
                for (const auto& b : info.follow[A])
                {
                    if (info.table[A].contains(b))
                        info.conflicts.push_back(A + "/" + b);
                    info.table[A][b] = k;
                }
            }
        }
    }
    return info;
}
