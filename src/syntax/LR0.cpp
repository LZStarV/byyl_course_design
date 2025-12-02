#include "LR0.h"

static bool containsItem(const QVector<LR0Item>& set, const LR0Item& it)
{
    for (const auto& x : set) if (x == it) return true;
    return false;
}

static QVector<LR0Item> closure(const QVector<LR0Item>& I, const Grammar& g)
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
                        if (!containsItem(res, ni)) { res.push_back(ni); changed = true; }
                    }
                }
            }
        }
    }
    return res;
}

static QVector<LR0Item> gotoSet(const QVector<LR0Item>& I, const QString& X, const Grammar& g)
{
    QVector<LR0Item> moved;
    for (const auto& it : I)
    {
        if (it.dot < it.right.size() && it.right[it.dot] == X)
        {
            LR0Item adv = it; adv.dot++;
            moved.push_back(adv);
        }
    }
    return closure(moved, g);
}

static bool equalSet(const QVector<LR0Item>& a, const QVector<LR0Item>& b)
{
    if (a.size() != b.size()) return false;
    for (const auto& x : a)
    {
        bool found = false;
        for (const auto& y : b) if (x == y) { found = true; break; }
        if (!found) return false;
    }
    return true;
}

LR0Graph LR0Builder::build(const Grammar& g)
{
    Grammar aug = g;
    QString Sdash = g.startSymbol + "'";
    if (!aug.productions.contains(Sdash))
    {
        Production p; p.left = Sdash; p.right = QVector<QString>{g.startSymbol}; p.line = -1;
        aug.productions[Sdash].push_back(p);
        aug.nonterminals.insert(Sdash);
    }
    LR0Graph gr;
    QVector<LR0Item> I0; I0.push_back(LR0Item{Sdash, QVector<QString>{g.startSymbol}, 0});
    I0 = closure(I0, aug);
    gr.states.push_back(I0);
    QMap<int, QMap<QString,int>> edges;
    bool added = true;
    while (added)
    {
        added = false;
        int n = gr.states.size();
        for (int i = 0; i < n; ++i)
        {
            QSet<QString> symbols;
            for (const auto& it : gr.states[i])
            {
                if (it.dot < it.right.size()) symbols.insert(it.right[it.dot]);
            }
            for (const auto& X : symbols)
            {
                auto J = gotoSet(gr.states[i], X, aug);
                if (J.isEmpty()) continue;
                int existing = -1;
                for (int k = 0; k < gr.states.size(); ++k)
                    if (equalSet(gr.states[k], J)) { existing = k; break; }
                if (existing < 0)
                {
                    gr.states.push_back(J);
                    existing = gr.states.size() - 1;
                    added = true;
                }
                gr.edges[i][X] = existing;
            }
        }
    }
    gr.edges = edges;
    return gr;
}

static QString escape(const QString& s)
{
    QString o;
    for (QChar c : s)
    {
        if (c == '"' || c == '\\') o += '\\' + QString(c);
        else o += c;
    }
    return o;
}

QString LR0Builder::toDot(const LR0Graph& gr)
{
    QString s;
    s += "digraph G {\nrankdir=LR\n";
    for (int i = 0; i < gr.states.size(); ++i)
    {
        QString label;
        for (const auto& it : gr.states[i])
        {
            label += escape(it.left) + " -> ";
            for (int j = 0; j < it.right.size(); ++j)
            {
                if (j == it.dot) label += " .";
                label += " " + escape(it.right[j]);
            }
            if (it.dot >= it.right.size()) label += " .";
            label += "\\n";
        }
        s += QString("  n%1 [shape=box,label=\"%2\"]\n").arg(i).arg(label);
    }
    for (int i = 0; i < gr.states.size(); ++i)
    {
        auto row = gr.edges.value(i);
        for (auto it = row.begin(); it != row.end(); ++it)
        {
            s += QString("  n%1 -> n%2 [label=\"%3\"]\n").arg(i).arg(it.value()).arg(escape(it.key()));
        }
    }
    s += "}\n";
    return s;
}

