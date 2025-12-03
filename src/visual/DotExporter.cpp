#include "DotExporter.h"
#include <QFile>
#include <QTextStream>

static QString esc(const QString& s)
{
    QString r;
    for (auto ch : s)
    {
        if (ch == '"' || ch == '\\')
            r += '\\';
        r += ch;
    }
    return r;
}

static QString header(const QString& name)
{
    return QStringLiteral("digraph ") + name +
           QStringLiteral(" {\nrankdir=LR;\nnode [shape=circle];\n");
}
static QString trailer()
{
    return QStringLiteral("}\n");
}

static void appendStartArrow(QString& out, const QString& startNode)
{
    out += QStringLiteral("__start [shape=point,label=\"\",width=0.1];\n");
    out += QStringLiteral("__start -> ") + startNode + QStringLiteral(";\n");
}

QString DotExporter::toDot(const NFA& nfa)
{
    QString out = header("NFA");
    for (auto it = nfa.states.begin(); it != nfa.states.end(); ++it)
    {
        int  id  = it->id;
        bool acc = it->accept;
        out += QString::number(id) + QStringLiteral(" [shape=") +
               (acc ? "doublecircle" : "circle") + QStringLiteral("];\n");
    }
    appendStartArrow(out, QString::number(nfa.start));
    for (auto it = nfa.states.begin(); it != nfa.states.end(); ++it)
    {
        int from = it->id;
        for (const auto& e : it->edges)
        {
            QString lbl = e.epsilon ? QStringLiteral("ε") : esc(e.symbol);
            out += QString::number(from) + QStringLiteral(" -> ") + QString::number(e.to) +
                   QStringLiteral(" [label=\"") + lbl + QStringLiteral("\"];\n");
        }
    }
    out += trailer();
    return out;
}

QString DotExporter::toDot(const DFA& dfa)
{
    QString out = header("DFA");
    for (auto it = dfa.states.begin(); it != dfa.states.end(); ++it)
    {
        int     id    = it->id;
        bool    acc   = it->accept;
        QString label = "";
        // label show set of NFA states
        QList<int> v = QList<int>(it->nfaSet.begin(), it->nfaSet.end());
        std::sort(v.begin(), v.end());
        label += "{";
        for (int i = 0; i < v.size(); ++i)
        {
            label += QString::number(v[i]);
            if (i + 1 < v.size())
                label += ", ";
        }
        label += "}";
        out += QString::number(id) + QStringLiteral(" [shape=") +
               (acc ? "doublecircle" : "circle") + QStringLiteral(",label=\"") + esc(label) +
               QStringLiteral("\"];\n");
    }
    appendStartArrow(out, QString::number(dfa.start));
    for (auto it = dfa.states.begin(); it != dfa.states.end(); ++it)
    {
        int from = it->id;
        for (auto a : dfa.alpha.ordered())
        {
            int to = it->trans.value(a, -1);
            if (to == -1)
                continue;
            out += QString::number(from) + QStringLiteral(" -> ") + QString::number(to) +
                   QStringLiteral(" [label=\"") + esc(a) + QStringLiteral("\"];\n");
        }
    }
    out += trailer();
    return out;
}

QString DotExporter::toDot(const MinDFA& mdfa)
{
    QString out = header("MinDFA");
    for (auto it = mdfa.states.begin(); it != mdfa.states.end(); ++it)
    {
        int  id  = it->id;
        bool acc = it->accept;
        out += QString::number(id) + QStringLiteral(" [shape=") +
               (acc ? "doublecircle" : "circle") + QStringLiteral("];\n");
    }
    appendStartArrow(out, QString::number(mdfa.start));
    for (auto it = mdfa.states.begin(); it != mdfa.states.end(); ++it)
    {
        int from = it->id;
        for (auto a : mdfa.alpha.ordered())
        {
            int to = it->trans.value(a, -1);
            if (to == -1)
                continue;
            out += QString::number(from) + QStringLiteral(" -> ") + QString::number(to) +
                   QStringLiteral(" [label=\"") + esc(a) + QStringLiteral("\"];\n");
        }
    }
    out += trailer();
    return out;
}

static bool writeFile(const QString& path, const QString& content)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream o(&f);
    o << content;
    f.close();
    return true;
}

bool DotExporter::exportToDot(const NFA& nfa, const QString& filename)
{
    return writeFile(filename, toDot(nfa));
}
bool DotExporter::exportToDot(const DFA& dfa, const QString& filename)
{
    return writeFile(filename, toDot(dfa));
}
bool DotExporter::exportToDot(const MinDFA& mdfa, const QString& filename)
{
    return writeFile(filename, toDot(mdfa));
}
