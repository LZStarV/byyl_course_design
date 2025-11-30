#include "Engine.h"
#include "config/Config.h"
RegexFile Engine::lexFile(const QString& text)
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
    cols.push_back("标记");
    cols.push_back("状态 ID");
    auto syms = nfa.alpha.ordered();
    for (auto s : syms) cols.push_back(s);
    cols.push_back("#");
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
    cols.push_back("标记");
    cols.push_back("状态集合");
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
    cols.push_back("标记");
    cols.push_back("状态 ID");
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
        QChar ch = source[pos++];
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
            continue;
        if (ch == '{')
        {
            while (pos < source.size() && source[pos++] != '}')
            {
            }
            continue;
        }
        int  state = mdfa.start;
        bool acc   = false;
        while (true)
        {
            bool moved = false;
            for (auto a : mdfa.alpha.ordered())
            {
                int t = mdfa.states[state].trans.value(a, -1);
                if (t == -1)
                    continue;
                if (a.compare("letter", Qt::CaseInsensitive) == 0)
                {
                    if (ch.isLetter())
                    {
                        state = t;
                        moved = true;
                        if (pos < source.size())
                            ch = source[pos++];
                        else
                        {
                            ch = QChar();
                        }
                        break;
                    }
                }
                else if (a.compare("digit", Qt::CaseInsensitive) == 0)
                {
                    if (ch.isDigit())
                    {
                        state = t;
                        moved = true;
                        if (pos < source.size())
                            ch = source[pos++];
                        else
                        {
                            ch = QChar();
                        }
                        break;
                    }
                }
                else
                {
                    if (a.size() == 1 && ch == a[0])
                    {
                        state = t;
                        moved = true;
                        if (pos < source.size())
                            ch = source[pos++];
                        else
                        {
                            ch = QChar();
                        }
                        break;
                    }
                }
            }
            if (!moved || ch.isNull())
                break;
            acc = mdfa.states[state].accept;
        }
        if (acc)
        {
            out += QString::number(tokenCode) + " ";
        }
        else
        {
            out += "ERR ";
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
            int  base   = pt.rule.code;
            int  idx    = 0;
            bool tinyKw = pt.rule.name.startsWith("_keywords", Qt::CaseInsensitive) &&
                          (pt.rule.expr.contains("repeat", Qt::CaseInsensitive) ||
                           pt.rule.expr.contains("until", Qt::CaseInsensitive));
            if (tinyKw)
            {
                QStringList kws({"if", "then", "else", "end", "repeat", "until", "read", "write"});
                for (auto kw : kws)
                {
                    auto ast  = makeKeywordCI(kw);
                    auto nfa  = buildNFA(ast, pf.alpha);
                    auto dfa  = buildDFA(nfa);
                    auto mdfa = buildMinDFA(dfa);
                    result.push_back(mdfa);
                    codes.push_back(base + (idx++));
                }
            }
            else
            {
                for (auto alt : alts)
                {
                    auto nfa  = buildNFA(alt, pf.alpha);
                    auto dfa  = buildDFA(nfa);
                    auto mdfa = buildMinDFA(dfa);
                    result.push_back(mdfa);
                    codes.push_back(base + (idx++));
                }
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
            if (a.compare("letter", Qt::CaseInsensitive) == 0)
            {
                if (ch.isLetter() || ch == '_' || ch == '$')
                {
                    state = t;
                    moved = true;
                    break;
                }
            }
            else if (a.compare("digit", Qt::CaseInsensitive) == 0)
            {
                if (ch.isDigit())
                {
                    state = t;
                    moved = true;
                    break;
                }
            }
            else if (a.size() == 1)
            {
                if (ch == a[0])
                {
                    state = t;
                    moved = true;
                    break;
                }
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
                            const QString&         source)
{
    QString out;
    int     pos = 0;
    while (pos < source.size())
    {
        QChar ch = source[pos];
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
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
            if (len > bestLen || (len == bestLen && w > bestW))
            {
                bestLen = len;
                bestIdx = i;
                bestW   = w;
            }
        }
        if (bestLen > 0)
        {
            out += QString::number(codes[bestIdx]) + " ";
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
