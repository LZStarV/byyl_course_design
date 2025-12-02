#include "Grammar.h"
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QIODevice>

static bool isNonTerminal(const QString& s)
{
    if (s.isEmpty()) return false;
    if (!s.contains('_')) return false;
    for (auto ch : s)
        if (!(ch.isLower() || ch == '_')) return false;
    return true;
}

static bool isTerminal(const QString& s)
{
    if (s.isEmpty()) return false;
    if (s == "#") return true;
    if (s.contains('_')) return false;
    return true;
}

static QString trim(const QString& s)
{
    return s.trimmed();
}

static QVector<QString> splitRhs(const QString& rhs)
{
    QVector<QString> v;
    auto parts = rhs.split(' ', Qt::SkipEmptyParts);
    for (auto p : parts) v.push_back(p.trimmed());
    return v;
}

static bool detectDirectLeftRecursion(const Grammar& g, QString& who)
{
    for (auto it = g.productions.begin(); it != g.productions.end(); ++it)
    {
        const QString& A = it.key();
        for (const auto& p : it.value())
        {
            if (!p.right.isEmpty() && p.right[0] == A)
            {
                who = A;
                return true;
            }
        }
    }
    return false;
}

static void addSymbols(Grammar& g)
{
    for (auto it = g.productions.begin(); it != g.productions.end(); ++it)
    {
        g.nonterminals.insert(it.key());
        for (const auto& p : it.value())
        {
            for (const auto& s : p.right)
            {
                if (s == "#") continue;
                if (isNonTerminal(s)) g.nonterminals.insert(s);
                else g.terminals.insert(s);
            }
        }
    }
}

static bool parseLine(const QString& line, int lineNo, Grammar& g, QString& err)
{
    QString t = line;
    if (t.trimmed().isEmpty()) return true;
    if (t.trimmed().startsWith("//")) return true;
    if (t.indexOf("->") < 0)
    {
        err = QString::number(lineNo);
        return false;
    }
    auto parts = t.split("->");
    if (parts.size() != 2) { err = QString::number(lineNo); return false; }
    QString left = trim(parts[0]);
    QString rhs  = trim(parts[1]);
    auto alts    = rhs.split('|');
    if (g.startSymbol.isEmpty() && !left.isEmpty()) g.startSymbol = left;
    for (auto a : alts)
    {
        Production p;
        p.left = left;
        p.right = splitRhs(trim(a));
        p.line = lineNo;
        g.productions[left].push_back(p);
    }
    return true;
}

static Grammar parseText(const QString& text, QString& error)
{
    Grammar g;
    auto lines = text.split('\n');
    for (int i = 0; i < lines.size(); ++i)
    {
        QString l = lines[i];
        QString s = l.trimmed();
        if (s.startsWith('#'))
        {
            bool allHash = true;
            for (int k = 0; k < s.size(); ++k) if (s[k] != '#') { allHash = false; break; }
            if (allHash) continue;
        }
        QString err;
        if (!parseLine(l, i + 1, g, err)) { error = err; return Grammar(); }
    }
    addSymbols(g);
    QString who;
    if (detectDirectLeftRecursion(g, who)) { error = who; return Grammar(); }
    return g;
}

namespace GrammarParser
{
    Grammar parseFile(const QString& path, QString& error);
    Grammar parseString(const QString& text, QString& error);
}

Grammar GrammarParser::parseString(const QString& text, QString& error)
{
    return parseText(text, error);
}

Grammar GrammarParser::parseFile(const QString& path, QString& error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) { error = "open"; return Grammar(); }
    QTextStream in(&f);
    auto content = in.readAll();
    f.close();
    return parseText(content, error);
}
