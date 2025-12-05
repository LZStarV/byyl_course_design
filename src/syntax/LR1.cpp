#include "LR1.h"
#include "LL1.h"
#include "../config/Config.h"

static bool isTerminal(const QSet<QString>& terms, const QString& s)
{
    return terms.contains(s) || s == Config::eofSymbol();
}

static QSet<QString> firstSeqLL1(const Grammar&          g,
                                 const LL1Info&          info,
                                 const QVector<QString>& seq,
                                 const QString&          la)
{
    QSet<QString> res;
    if (seq.isEmpty())
    {
        res.insert(la);
        return res;
    }
    bool allEps = true;
    for (int i = 0; i < seq.size(); ++i)
    {
        const QString& X = seq[i];
        if (isTerminal(g.terminals, X) && X != Config::epsilonSymbol())
        {
            res.insert(X);
            allEps = false;
            break;
        }
        // 非终结符：FIRST(X) 去除 #
        QSet<QString> f = info.first.value(X);
        for (const auto& a : f)
            if (a != Config::epsilonSymbol())
                res.insert(a);
        if (!f.contains(Config::epsilonSymbol()))
        {
            allEps = false;
            break;
        }
    }
    if (allEps)
        res.insert(la);
    return res;
}

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
                if (!isTerminal(g.terminals, B) && B != "#")
                {
                    // β = right[dot+1..]
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

static QVector<LR1Item> goToLL1(const Grammar&          g,
                                const LL1Info&          info,
                                const QVector<LR1Item>& I,
                                const QString&          X)
{
    if (X.isEmpty() || X == Config::epsilonSymbol())
        return {};
    QVector<LR1Item> moved;
    for (const auto& it : I)
    {
        if (it.dot < it.right.size() && it.right[it.dot] == X)
        {
            LR1Item ni = it;
            ni.dot++;
            moved.push_back(ni);
        }
    }
    return closureLL1(g, info, moved);
}

static QString serializeSet(const QVector<LR1Item>& I)
{
    QString          s;
    QVector<QString> lines;
    for (const auto& it : I)
    {
        lines.push_back(it.left + "->" + it.right.join(" ") + "." + QString::number(it.dot) + "/" +
                        it.lookahead);
    }
    std::sort(lines.begin(), lines.end());
    return lines.join("\n");
}

LR1Graph LR1Builder::build(const Grammar& g)
{
    Grammar aug = g;
    if (!aug.startSymbol.isEmpty())
    {
        // 增广文法 S' -> S
        QString Sprime = g.startSymbol + "'";
        if (!aug.productions.contains(Sprime))
        {
            aug.productions[Sprime].push_back({Sprime, QVector<QString>{g.startSymbol}, -1});
            aug.startSymbol = Sprime;
            aug.nonterminals.insert(Sprime);
        }
    }
    if (aug.startSymbol.isEmpty())
    {
        return LR1Graph{};
    }
    LL1Info          info = LL1::compute(aug);
    QVector<LR1Item> I0 =
        closureLL1(aug,
                   info,
                   QVector<LR1Item>{LR1Item{
                       aug.startSymbol, aug.productions[aug.startSymbol][0].right, 0, "$"}});
    LR1Graph gr;
    gr.states.push_back(I0);
    QMap<QString, int> stateIndex;
    stateIndex[serializeSet(I0)] = 0;
    bool changed                 = true;
    while (changed)
    {
        changed = false;
        int S   = gr.states.size();
        for (int i = 0; i < S; ++i)
        {
            auto          I = gr.states[i];
            QSet<QString> nextSymbols;
            for (const auto& it : I)
            {
                if (it.dot < it.right.size())
                {
                    QString X = it.right[it.dot];
                    if (!X.isEmpty() && X != Config::epsilonSymbol())
                        nextSymbols.insert(X);
                }
            }
            for (auto ns = nextSymbols.begin(); ns != nextSymbols.end(); ++ns)
            {
                QString X = *ns;
                auto    J = goToLL1(aug, info, I, X);
                if (J.isEmpty())
                    continue;
                QString keyJ = serializeSet(J);
                int     idx  = stateIndex.value(keyJ, -1);
                if (idx < 0)
                {
                    idx = gr.states.size();
                    gr.states.push_back(J);
                    stateIndex[keyJ] = idx;
                    changed          = true;
                }
                gr.edges[i][X] = idx;
            }
        }
    }
    return gr;
}

QString LR1Builder::toDot(const LR1Graph& gr)
{
    QString dot = "digraph LR1 {\nrankdir=LR; node [shape=box,fontname=Helvetica];\n";
    for (int i = 0; i < gr.states.size(); ++i)
    {
        QMap<QString, int> core;
        for (const auto& it : gr.states[i])
        {
            QString rhs;
            for (int k = 0; k < it.right.size(); ++k)
            {
                if (k == it.dot)
                    rhs += " •";
                rhs += " " + it.right[k];
            }
            if (it.dot == it.right.size())
                rhs += " •";
            QString coreKey = it.left + " →" + rhs;
            core[coreKey]   = core.value(coreKey, 0) + 1;
        }
        QString label = QString("I%1\\n").arg(i);
        for (auto it = core.begin(); it != core.end(); ++it)
        {
            label += it.key() + QString(" /%1\\n").arg(it.value());
        }
        dot += QString("s%1 [label=\"%2\"];\n").arg(i).arg(label.replace("\"", "\\\""));
    }
    for (auto eit = gr.edges.begin(); eit != gr.edges.end(); ++eit)
    {
        int from = eit.key();
        for (auto sit = eit.value().begin(); sit != eit.value().end(); ++sit)
        {
            dot +=
                QString("s%1 -> s%2 [label=\"%3\"];\n").arg(from).arg(sit.value()).arg(sit.key());
        }
    }
    dot += "}\n";
    return dot;
}

static void putAction(QMap<int, QMap<QString, QString>>& action,
                      int                                st,
                      const QString&                     a,
                      const QString&                     val)
{
    QString prev = action[st].value(a);
    if (!prev.isEmpty() && prev != val)
        action[st][a] = prev + "|" + val;
    else
        action[st][a] = val;
}

LR1ActionTable LR1Builder::computeActionTable(const Grammar& g, const LR1Graph& gr)
{
    LR1ActionTable t;
    for (int i = 0; i < gr.states.size(); ++i)
    {
        const auto& I = gr.states[i];
        for (const auto& it : I)
        {
            if (it.dot < it.right.size())
            {
                QString X = it.right[it.dot];
                if (isTerminal(g.terminals, X))
                {
                    auto edIt = gr.edges.constFind(i);
                    int  to   = -1;
                    if (edIt != gr.edges.constEnd())
                        to = edIt.value().value(X, -1);
                    if (to >= 0)
                        putAction(t.action, i, X, QString("s%1").arg(to));
                }
            }
            else
            {
                if (it.left.endsWith(Config::augSuffix()) && it.lookahead == Config::eofSymbol())
                {
                    putAction(t.action, i, Config::eofSymbol(), "acc");
                }
                QString a = it.lookahead;
                if (!a.isEmpty() && a != "#")
                {
                    QString r = QString("r %1 -> %2").arg(it.left).arg(it.right.join(" "));
                    putAction(t.action, i, a, r);
                }
            }
        }
        auto edIt = gr.edges.constFind(i);
        if (edIt != gr.edges.constEnd())
        {
            const auto& edgesMap = edIt.value();
            for (auto eit = edgesMap.begin(); eit != edgesMap.end(); ++eit)
            {
                QString X  = eit.key();
                int     to = eit.value();
                if (!isTerminal(g.terminals, X) && X != "#")
                    t.gotoTable[i][X] = to;
            }
        }
    }
    return t;
}
