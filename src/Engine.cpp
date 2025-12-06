#include "Engine.h"
#include "config/Config.h"
#include "syntax/GrammarParser.h"
static int matchLen(const MinDFA& mdfa, const QString& src, int pos);
RegexFile  Engine::lexFile(const QString& text)
{
    return RegexLexer::lex(text);
}
ParsedFile Engine::parseFile(const RegexFile& f)
{
    return RegexParser::parse(f);
}
NFA Engine::buildNFA(ASTNode* ast, const Alphabet& alpha)
{
    return Thompson::build(ast, alpha);
}
DFA Engine::buildDFA(const NFA& nfa)
{
    return SubsetConstruction::build(nfa);
}
MinDFA Engine::buildMinDFA(const DFA& dfa)
{
    return Hopcroft::minimize(dfa);
}
static Tables tableFromNFA(const NFA& nfa)
{
    Tables           t;
    QVector<QString> cols;
    cols.push_back(Config::tableMarkLabel());
    cols.push_back(Config::tableStateIdLabel());
    auto syms = nfa.alpha.ordered();
    for (auto s : syms) cols.push_back(s);
    cols.push_back(Config::epsilonColumnLabel());
    t.columns = cols;
    for (auto it = nfa.states.begin(); it != nfa.states.end(); ++it)
    {
        QString          mark = it->id == nfa.start ? "-" : (it->accept ? "+" : "");
        QString          sid  = QString::number(it->id);
        QVector<QString> row;
        row.push_back(mark);
        row.push_back(sid);
        for (auto s : syms)
        {
            QString dst;
            for (auto e : it->edges)
            {
                if (!e.epsilon && e.symbol == s)
                {
                    dst += QString::number(e.to) + ",";
                }
            }
            if (!dst.isEmpty())
                dst.chop(1);
            row.push_back(dst);
        }
        // 跳过双引号字符串（支持转义）
        QString edst;
        for (auto e : it->edges)
        {
            if (e.epsilon)
            {
                edst += QString::number(e.to) + ",";
            }
        }
        if (!edst.isEmpty())
            edst.chop(1);
        row.push_back(edst);
        t.rows.push_back(row);
        t.marks.push_back(mark);
        t.states.push_back(sid);
    }
    return t;
}
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
static Tables tableFromDFA(const DFA& dfa)
{
    Tables           t;
    QVector<QString> cols;
    cols.push_back(Config::tableMarkLabel());
    cols.push_back(Config::tableStateSetLabel());
    auto syms = dfa.alpha.ordered();
    for (auto s : syms) cols.push_back(s);
    t.columns = cols;
    for (auto it = dfa.states.begin(); it != dfa.states.end(); ++it)
    {
        QString          mark = it->id == dfa.start ? "-" : (it->accept ? "+" : "");
        QString          sid  = setName(it->nfaSet);
        QVector<QString> row;
        row.push_back(mark);
        row.push_back(sid);
        for (auto s : syms)
        {
            int to = it->trans.value(s, -1);
            row.push_back(to == -1 ? QString() : setName(dfa.states[to].nfaSet));
        }
        t.rows.push_back(row);
        t.marks.push_back(mark);
        t.states.push_back(sid);
    }
    return t;
}
static Tables tableFromMin(const MinDFA& dfa)
{
    Tables           t;
    QVector<QString> cols;
    cols.push_back(Config::tableMarkLabel());
    cols.push_back(Config::tableStateIdLabel());
    auto syms = dfa.alpha.ordered();
    for (auto s : syms) cols.push_back(s);
    t.columns = cols;
    for (auto it = dfa.states.begin(); it != dfa.states.end(); ++it)
    {
        QString          mark = it->id == dfa.start ? "-" : (it->accept ? "+" : "");
        QString          sid  = QString::number(it->id);
        QVector<QString> row;
        row.push_back(mark);
        row.push_back(sid);
        for (auto s : syms)
        {
            int to = it->trans.value(s, -1);
            row.push_back(to == -1 ? QString() : QString::number(to));
        }
        t.rows.push_back(row);
        t.marks.push_back(mark);
        t.states.push_back(sid);
    }
    return t;
}
Tables Engine::nfaTable(const NFA& nfa)
{
    return tableFromNFA(nfa);
}
Tables Engine::dfaTable(const DFA& dfa)
{
    return tableFromDFA(dfa);
}
Tables Engine::minTable(const MinDFA& dfa)
{
    return tableFromMin(dfa);
}
QString Engine::generateCode(const MinDFA& mdfa, const QMap<QString, int>& tokenCodes)
{
    return CodeGenerator::generate(mdfa, tokenCodes);
}
static int classify(const MinDFA& mdfa, QChar ch)
{
    if (mdfa.alpha.hasLetter && (ch.isLetter() || ch == '_' || ch == '$'))
        return 1;
    if (mdfa.alpha.hasDigit && ch.isDigit())
        return 0;
    return -2;
}
QString Engine::run(const MinDFA& mdfa, const QString& source, int tokenCode)
{
    QString out;
    int     pos = 0;
    while (pos < source.size())
    {
        QChar ch = source[pos];
        if (Config::isWhitespace(ch))
        {
            pos++;
            continue;
        }
        if (Config::skipBraceComment() && ch == '{')
        {
            pos++;
            while (pos < source.size() && source[pos++] != '}')
            {
            }
            continue;
        }
        if (Config::skipLineComment() && ch == '/' && pos + 1 < source.size() &&
            source[pos + 1] == '/')
        {
            pos += 2;
            while (pos < source.size() && source[pos++] != '\n')
            {
            }
            continue;
        }
        if (Config::skipHashComment() && ch == '#')
        {
            pos++;
            while (pos < source.size() && source[pos++] != '\n')
            {
            }
            continue;
        }
        if (Config::skipBlockComment() && ch == '/' && pos + 1 < source.size() &&
            source[pos + 1] == '*')
        {
            pos += 2;
            while (pos + 1 < source.size())
            {
                if (source[pos] == '*' && source[pos + 1] == '/')
                {
                    pos += 2;
                    break;
                }
                pos++;
            }
            continue;
        }
        if (Config::skipSingleQuoteString() && ch == '\'')
        {
            pos++;
            while (pos < source.size())
            {
                QChar c = source[pos++];
                if (c == '\\')
                {
                    if (pos < source.size())
                        pos++;
                    continue;
                }
                if (c == '\'')
                    break;
            }
            continue;
        }
        if (Config::skipDoubleQuoteString() && ch == '"')
        {
            pos++;
            while (pos < source.size())
            {
                QChar c = source[pos++];
                if (c == '\\')
                {
                    if (pos < source.size())
                        pos++;
                    continue;
                }
                if (c == '"')
                    break;
            }
            continue;
        }
        if (Config::skipTemplateString() && ch == '`')
        {
            pos++;
            while (pos < source.size())
            {
                QChar c = source[pos++];
                if (c == '\\')
                {
                    if (pos < source.size())
                        pos++;
                    continue;
                }
                if (c == '`')
                    break;
                if (c == '$' && pos < source.size() && source[pos] == '{')
                {
                    pos++;
                    int depth = 1;
                    while (pos < source.size() && depth > 0)
                    {
                        QChar c2 = source[pos++];
                        if (c2 == '\\')
                        {
                            if (pos < source.size())
                                pos++;
                            continue;
                        }
                        if (c2 == '{')
                            depth++;
                        else if (c2 == '}')
                            depth--;
                    }
                }
            }
            continue;
        }
        int len = matchLen(mdfa, source, pos);
        if (len > 0)
        {
            out += QString::number(tokenCode) + " ";
            pos += len;
        }
        else
        {
            out += "ERR ";
            pos++;
        }
    }
    return out.trimmed();
}

static void collectAlternatives(ASTNode* n, QVector<ASTNode*>& out)
{
    if (!n)
        return;
    if (n->type == ASTNode::Union)
    {
        collectAlternatives(n->children[0], out);
        collectAlternatives(n->children[1], out);
    }
    else
    {
        out.push_back(n);
    }
}

static ASTNode* makeSymbol(QChar ch)
{
    auto n   = new ASTNode();
    n->type  = ASTNode::Symbol;
    n->value = QString(ch);
    return n;
}
static ASTNode* makeConcat(ASTNode* a, ASTNode* b)
{
    auto n      = new ASTNode();
    n->type     = ASTNode::Concat;
    n->children = {a, b};
    return n;
}
static ASTNode* makeCharSet(const QString& chars)
{
    auto n   = new ASTNode();
    n->type  = ASTNode::CharSet;
    n->value = chars;
    return n;
}
static ASTNode* makeKeywordCI(const QString& word)
{
    if (word.isEmpty())
        return nullptr;
    auto     first = makeCharSet(QString(word[0]).toLower() + QString(word[0]).toUpper());
    ASTNode* seq   = first;
    for (int i = 1; i < word.size(); ++i)
    {
        QString ch = QString(word[i]);
        auto    cs = makeCharSet(ch.toLower() + ch.toUpper());
        seq        = makeConcat(seq, cs);
    }
    return seq;
}

QVector<MinDFA> Engine::buildAllMinDFA(const ParsedFile& pf, QVector<int>& codes)
{
    QVector<MinDFA> result;
    codes.clear();
    for (const auto& pt : pf.tokens)
    {
        if (pt.rule.isGroup)
        {
            QVector<ASTNode*> alts;
            collectAlternatives(pt.ast, alts);
            int base = pt.rule.code;
            int idx  = 0;
            for (auto alt : alts)
            {
                auto nfa  = buildNFA(alt, pf.alpha);
                auto dfa  = buildDFA(nfa);
                auto mdfa = buildMinDFA(dfa);
                result.push_back(mdfa);
                codes.push_back(base + (idx++));
            }
        }
        else
        {
            auto nfa  = buildNFA(pt.ast, pf.alpha);
            auto dfa  = buildDFA(nfa);
            auto mdfa = buildMinDFA(dfa);
            result.push_back(mdfa);
            codes.push_back(pt.rule.code);
        }
    }
    return result;
}

static int matchLen(const MinDFA& mdfa, const QString& src, int pos)
{
    int state   = mdfa.start;
    int i       = pos;
    int lastAcc = -1;
    while (i < src.size())
    {
        QChar ch    = src[i];
        bool  moved = false;
        for (auto a : mdfa.alpha.ordered())
        {
            int t = mdfa.states[state].trans.value(a, -1);
            if (t == -1)
                continue;
            if (a.size() == 1 && a[0] == ch)
            {
                state = t;
                moved = true;
                break;
            }
        }
        if (!moved)
            break;
        i++;
        if (mdfa.states[state].accept)
            lastAcc = i;
    }
    return lastAcc == -1 ? 0 : (lastAcc - pos);
}

QString Engine::runMultiple(const QVector<MinDFA>& mdfas,
                            const QVector<int>&    codes,
                            const QString&         source,
                            const QSet<int>&       identifierCodes)
{
    QString out;
    int     pos = 0;
    while (pos < source.size())
    {
        QChar ch = source[pos];
        if (Config::isWhitespace(ch))
        {
            pos++;
            continue;
        }
        if (Config::skipBraceComment() && ch == '{')
        {
            pos++;
            while (pos < source.size() && source[pos++] != '}')
            {
            }
            continue;
        }
        if (Config::skipLineComment() && ch == '/' && pos + 1 < source.size() &&
            source[pos + 1] == '/')
        {
            pos += 2;
            while (pos < source.size() && source[pos++] != '\n')
            {
            }
            continue;
        }
        if (Config::skipHashComment() && ch == '#')
        {
            pos++;
            while (pos < source.size() && source[pos++] != '\n')
            {
            }
            continue;
        }
        if (Config::skipBlockComment() && ch == '/' && pos + 1 < source.size() &&
            source[pos + 1] == '*')
        {
            pos += 2;
            while (pos + 1 < source.size())
            {
                if (source[pos] == '*' && source[pos + 1] == '/')
                {
                    pos += 2;
                    break;
                }
                pos++;
            }
            continue;
        }
        if (Config::skipSingleQuoteString() && ch == '\'')
        {
            pos++;
            while (pos < source.size())
            {
                QChar c = source[pos++];
                if (c == '\\')
                {
                    if (pos < source.size())
                        pos++;
                    continue;
                }
                if (c == '\'')
                    break;
            }
            continue;
        }
        if (Config::skipDoubleQuoteString() && ch == '\"')
        {
            pos++;
            while (pos < source.size())
            {
                QChar c = source[pos++];
                if (c == '\\')
                {
                    if (pos < source.size())
                        pos++;
                    continue;
                }
                if (c == '\"')
                    break;
            }
            continue;
        }
        if (Config::skipTemplateString() && ch == '`')
        {
            pos++;
            while (pos < source.size())
            {
                QChar c = source[pos++];
                if (c == '\\')
                {
                    if (pos < source.size())
                        pos++;
                    continue;
                }
                if (c == '`')
                    break;
                if (c == '$' && pos < source.size() && source[pos] == '{')
                {
                    pos++;
                    int depth = 1;
                    while (pos < source.size() && depth > 0)
                    {
                        QChar c2 = source[pos++];
                        if (c2 == '\\')
                        {
                            if (pos < source.size())
                                pos++;
                            continue;
                        }
                        if (c2 == '{')
                            depth++;
                        else if (c2 == '}')
                            depth--;
                    }
                }
            }
            continue;
        }
        int bestLen = 0;
        int bestIdx = -1;
        int bestW   = -1;
        for (int i = 0; i < mdfas.size(); ++i)
        {
            int len = matchLen(mdfas[i], source, pos);
            int w   = Config::weightForCode(codes[i]);
            // 最长匹配优先，仅在长度相同时考虑权重，权重相同则按顺序优先
            if (len > bestLen ||
                (len == bestLen && (w > bestW || (w == bestW && (bestIdx == -1 || i < bestIdx)))))
            {
                bestLen = len;
                bestIdx = i;
                bestW   = w;
            }
        }
        if (bestLen > 0)
        {
            int code = codes[bestIdx];
            out += QString::number(code) + " ";
            if (Config::emitIdentifierLexeme() && identifierCodes.contains(code))
            {
                out += source.mid(pos, bestLen) + " ";
            }
            pos += bestLen;
        }
        else
        {
            out += "ERR ";
            pos++;
        }
    }
    return out.trimmed();
}

Grammar Engine::parseGrammarText(const QString& text, QString& error)
{
    return GrammarParser::parseString(text, error);
}

LL1Info Engine::computeLL1(const Grammar& g)
{
    return LL1::compute(g);
}

QMap<QString, QVector<QString>> Engine::firstFollowAsRows(const LL1Info& info)
{
    QMap<QString, QVector<QString>> r;
    for (auto it = info.first.begin(); it != info.first.end(); ++it)
    {
        QVector<QString> v;
        QList<QString>   s = QList<QString>(it.value().begin(), it.value().end());
        std::sort(s.begin(), s.end());
        for (auto x : s) v.push_back(x);
        r[it.key()] = v;
    }
    return r;
}

QMap<QString, QMap<QString, QString>> Engine::parsingTableAsRows(const Grammar& g,
                                                                 const LL1Info& info)
{
    QMap<QString, QMap<QString, QString>> r;
    for (auto A : g.nonterminals)
    {
        QMap<QString, QString> row;
        for (auto a : g.terminals)
        {
            int idx = info.table.value(A).value(a, -1);
            if (idx >= 0)
            {
                const auto& p = g.productions[A][idx];
                QString     rhs;
                for (int i = 0; i < p.right.size(); ++i)
                {
                    rhs += p.right[i];
                    if (i + 1 < p.right.size())
                        rhs += " ";
                }
                row[a] = rhs;
            }
        }
        row["$"] = info.table.value(A).contains("$") ? row.value("$") : QString();
        r[A]     = row;
    }
    return r;
}
static QSet<QChar> expandRangesLocal(const QString& ranges)
{
    QSet<QChar> set;
    int         i    = 0;
    int         n    = ranges.size();
    auto        skip = [&]()
    {
        while (i < n && (ranges[i] == ' ' || ranges[i] == '\t')) i++;
    };
    skip();
    while (i < n)
    {
        skip();
        if (i >= n)
            break;
        QChar a = ranges[i++];
        if (i < n && ranges[i] == '-')
        {
            i++;
            if (i < n)
            {
                QChar  b  = ranges[i++];
                ushort ua = a.unicode(), ub = b.unicode();
                if (ua <= ub)
                {
                    for (ushort u = ua; u <= ub; ++u) set.insert(QChar(u));
                }
                else
                {
                    for (ushort u = ub; u <= ua; ++u) set.insert(QChar(u));
                }
            }
        }
        else
        {
            set.insert(a);
        }
        skip();
        if (i < n && (ranges[i] == ',' || ranges[i] == ';'))
            i++;
        skip();
    }
    return set;
}
static QMap<QString, QSet<QChar>> macroSetsFromRules(const QMap<QString, Rule>& macros)
{
    QMap<QString, QSet<QChar>> m;
    auto                       keys = macros.keys();
    for (const auto& k : keys)
    {
        const auto&    r = macros.value(k);
        QSet<QChar>    set;
        const QString& expr = r.expr;
        int            i = 0, n = expr.size();
        while (i < n)
        {
            while (i < n &&
                   (expr[i] == ' ' || expr[i] == '\t' || expr[i] == '\n' || expr[i] == '\r'))
                i++;
            if (i >= n)
                break;
            if (expr[i] == '[')
            {
                int     j = i + 1;
                QString ranges;
                while (j < n && expr[j] != ']')
                {
                    ranges.append(expr[j]);
                    j++;
                }
                if (j < n && expr[j] == ']')
                {
                    set.unite(expandRangesLocal(ranges));
                    i = j + 1;
                }
                else
                    break;
            }
            else if (expr[i] == '(')
            {
                int     j = i + 1, depth = 1;
                QString inner;
                while (j < n && depth > 0)
                {
                    if (expr[j] == '(')
                        depth++;
                    else if (expr[j] == ')')
                        depth--;
                    if (depth > 0)
                        inner.append(expr[j]);
                    j++;
                }
                set.unite(expandRangesLocal(inner));
                i = j;
            }
            else if (expr[i] == '\\')
            {
                if (i + 1 < n)
                {
                    set.insert(expr[i + 1]);
                    i += 2;
                }
                else
                    break;
            }
            else
            {
                set.insert(expr[i]);
                i++;
            }
        }
        if (!set.isEmpty())
            m.insert(r.name, set);
    }
    return m;
}
static QMap<QString, QString> macroExprsFromRules(const QMap<QString, Rule>& macros)
{
    QMap<QString, QString> m;
    for (auto it = macros.begin(); it != macros.end(); ++it)
    {
        m.insert(it->name, it->expr);
    }
    return m;
}
static void aggregateTableByMacros(Tables&                           t,
                                   const QMap<QString, QSet<QChar>>& msets,
                                   const QMap<QString, QString>&     mexpr)
{
    if (msets.isEmpty())
        return;
    int colCount = t.columns.size();
    if (colCount < 3)
        return;
    bool             hasEps   = (colCount > 0 && t.columns.last() == QStringLiteral("#"));
    int              symStart = 2;
    int              symEnd   = hasEps ? (colCount - 1) : colCount;
    QMap<QChar, int> charIdx;
    for (int ci = symStart; ci < symEnd; ++ci)
    {
        const QString& c = t.columns[ci];
        if (c.size() == 1)
            charIdx.insert(c[0], ci);
    }
    QSet<int>        removeIdx;
    QVector<QString> newCols;
    newCols << t.columns[0] << t.columns[1];
    QVector<int> keepIdx;
    auto         mkeys = msets.keys();
    std::sort(mkeys.begin(), mkeys.end());
    QMap<QString, QVector<int>> macroHit;
    for (const auto& name : mkeys)
    {
        const auto&  set = msets.value(name);
        QVector<int> idxs;
        for (auto ch : set)
        {
            if (charIdx.contains(ch))
                idxs.push_back(charIdx.value(ch));
        }
        if (!idxs.isEmpty())
        {
            std::sort(idxs.begin(), idxs.end());
            macroHit.insert(name, idxs);
            for (int id : idxs) removeIdx.insert(id);
            QString label = name;
            if (mexpr.contains(name))
                label += QStringLiteral(" (") + mexpr.value(name) + QStringLiteral(")");
            newCols.push_back(label);
        }
    }
    for (int ci = symStart; ci < symEnd; ++ci)
    {
        if (!removeIdx.contains(ci))
        {
            newCols.push_back(t.columns[ci]);
            keepIdx.push_back(ci);
        }
    }
    if (hasEps)
        newCols.push_back(QStringLiteral("#"));
    auto mergeTargets = [&](const QStringList& parts)
    {
        QSet<QString> uniq;
        for (const auto& p : parts)
        {
            for (const auto& seg : p.split(',', Qt::SkipEmptyParts)) uniq.insert(seg.trimmed());
        }
        QList<QString> v = QList<QString>(uniq.begin(), uniq.end());
        std::sort(v.begin(), v.end());
        return v.isEmpty() ? QString() : v.join(',');
    };
    QVector<QVector<QString>> newRows;
    for (const auto& row : t.rows)
    {
        QVector<QString> nr;
        nr << row[0] << row[1];
        for (const auto& name : mkeys)
        {
            if (!macroHit.contains(name))
                continue;
            QStringList parts;
            for (int id : macroHit.value(name)) parts << row[id];
            nr.push_back(mergeTargets(parts));
        }
        for (int id : keepIdx) nr.push_back(row[id]);
        if (hasEps)
            nr.push_back(row[symEnd]);
        newRows.push_back(nr);
    }
    t.columns = newCols;
    t.rows    = newRows;
}
Tables Engine::nfaTableWithMacros(const NFA& nfa, const QMap<QString, Rule>& macros)
{
    Tables t     = tableFromNFA(nfa);
    auto   msets = macroSetsFromRules(macros);
    auto   mexpr = macroExprsFromRules(macros);
    aggregateTableByMacros(t, msets, mexpr);
    return t;
}
Tables Engine::dfaTableWithMacros(const DFA& dfa, const QMap<QString, Rule>& macros)
{
    Tables t     = tableFromDFA(dfa);
    auto   msets = macroSetsFromRules(macros);
    auto   mexpr = macroExprsFromRules(macros);
    aggregateTableByMacros(t, msets, mexpr);
    return t;
}
Tables Engine::minTableWithMacros(const MinDFA& dfa, const QMap<QString, Rule>& macros)
{
    Tables t     = tableFromMin(dfa);
    auto   msets = macroSetsFromRules(macros);
    auto   mexpr = macroExprsFromRules(macros);
    aggregateTableByMacros(t, msets, mexpr);
    return t;
}
