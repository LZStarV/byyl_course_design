#include "Grammar.h"
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QIODevice>

static bool isNonTerminal(const QString& s)
{
    if (s.isEmpty())
        return false;
    if (!s.contains('_'))
        return false;
    for (auto ch : s)
        if (!(ch.isLower() || ch == '_'))
            return false;
    return true;
}

static bool isTerminal(const QString& s)
{
    if (s.isEmpty())
        return false;
    if (s == "#")
        return true;
    if (s.contains('_'))
        return false;
    return true;
}

static QString trim(const QString& s)
{
    return s.trimmed();
}

static QVector<QString> splitRhs(const QString& rhs)
{
    QVector<QString> v;
    QString          s = rhs;
    int              i = 0;
    auto isWordChar    = [](QChar c) { return c.isLetterOrNumber() || c == '_' || c == '-'; };
    auto matchMulti    = [&s, &i](const QString& pat)
    {
        if (i + pat.size() <= s.size() && s.mid(i, pat.size()) == pat)
        {
            i += pat.size();
            return true;
        }
        return false;
    };
    QSet<QString> singles{"(", ")", ";", "<", ">", "=", "+", "-", "*", "/", "%", "^"};
    while (i < s.size())
    {
        QChar c = s[i];
        if (c.isSpace())
        {
            i++;
            continue;
        }
        if (matchMulti(":=") || matchMulti("<=") || matchMulti(">=") || matchMulti("<>"))
        {
            v.push_back(s.mid(i - 2, 2));  // last matched 2-ch op
            continue;
        }
        if (singles.contains(QString(c)))
        {
            v.push_back(QString(c));
            i++;
            continue;
        }
        QString w;
        while (i < s.size() && isWordChar(s[i]))
        {
            w += s[i];
            i++;
        }
        if (!w.isEmpty())
            v.push_back(w);
        else
            i++;
    }
    return v;
}

static bool detectDirectLeftRecursion(const Grammar& g, QString& who)
{
    for (auto it = g.productions.begin(); it != g.productions.end(); ++it)
    {
        const QString& A = it.key();
        for (const auto& p : it.value())
        {
            if (!p.right.isEmpty())
            {
                const QString& X = p.right[0];
                if (X == A && isNonTerminal(X))
                {
                    who = A;
                    return true;
                }
            }
        }
    }
    return false;
}

static void addSymbols(Grammar& g)
{
    QSet<QString> lhs;
    for (auto it = g.productions.begin(); it != g.productions.end(); ++it) lhs.insert(it.key());
    for (auto it = g.productions.begin(); it != g.productions.end(); ++it)
    {
        g.nonterminals.insert(it.key());
        for (const auto& p : it.value())
        {
            for (const auto& s : p.right)
            {
                if (s == "#")
                    continue;
                if (lhs.contains(s))
                    g.nonterminals.insert(s);
                else
                    g.terminals.insert(s);
            }
        }
    }
}

static bool parseLine(const QString& line, int lineNo, Grammar& g, QString& err)
{
    QString t = line;
    if (t.trimmed().isEmpty())
        return true;
    if (t.trimmed().startsWith("//"))
        return true;
    if (t.indexOf("->") < 0)
    {
        err = QString::number(lineNo);
        return false;
    }
    auto parts = t.split("->");
    if (parts.size() != 2)
    {
        err = QString::number(lineNo);
        return false;
    }
    QString left = trim(parts[0]);
    QString rhs  = trim(parts[1]);
    auto    alts = rhs.split('|');
    if (g.startSymbol.isEmpty() && !left.isEmpty())
        g.startSymbol = left;
    for (auto a : alts)
    {
        Production p;
        p.left  = left;
        p.right = splitRhs(trim(a));
        p.line  = lineNo;
        g.productions[left].push_back(p);
    }
    return true;
}

static Grammar parseText(const QString& text, QString& error)
{
    Grammar g;
    auto    lines = text.split('\n');
    for (int i = 0; i < lines.size(); ++i)
    {
        QString l = lines[i];
        QString s = l.trimmed();
        if (s.startsWith('#'))
        {
            bool allHash = true;
            for (int k = 0; k < s.size(); ++k)
                if (s[k] != '#')
                {
                    allHash = false;
                    break;
                }
            if (allHash)
                continue;
        }
        QString err;
        if (!parseLine(l, i + 1, g, err))
        {
            error = err;
            return Grammar();
        }
    }
    addSymbols(g);
    QString who;
    if (detectDirectLeftRecursion(g, who))
    {
        error = who;
        return Grammar();
    }
    return g;
}

namespace GrammarParser
{
    Grammar parseFile(const QString& path, QString& error);
    Grammar parseString(const QString& text, QString& error);
}  // namespace GrammarParser

Grammar GrammarParser::parseString(const QString& text, QString& error)
{
    return parseText(text, error);
}

Grammar GrammarParser::parseFile(const QString& path, QString& error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        error = "open";
        return Grammar();
    }
    QTextStream in(&f);
    auto        content = in.readAll();
    f.close();
    return parseText(content, error);
}
