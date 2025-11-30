#include "RegexLexer.h"
#include <QStringList>
#include <QRegularExpression>
static bool parseTokenHeader(const QString& name, int& code, bool& isGroup)
{
    if (!name.startsWith('_'))
        return false;
    QRegularExpression re("^_([A-Za-z][A-Za-z0-9_]*?)(\\d+)(S)?$");
    auto               m = re.match(name);
    if (!m.hasMatch())
        return false;
    code    = m.captured(2).toInt();
    isGroup = !m.captured(3).isEmpty();
    return true;
}
RegexFile RegexLexer::lex(const QString& input)
{
    RegexFile f;
    auto      lines = input.split('\n');
    for (auto line : lines)
    {
        auto trimmed = line.trimmed();
        if (trimmed.isEmpty())
            continue;
        int eq = trimmed.indexOf('=');
        if (eq < 0)
            continue;
        QString left  = trimmed.left(eq).trimmed();
        QString right = trimmed.mid(eq + 1).trimmed();
        Rule    r;
        r.name       = left;
        r.expr       = right;
        int  code    = 0;
        bool isGroup = false;
        if (parseTokenHeader(left, code, isGroup))
        {
            r.isToken = true;
            r.code    = code;
            r.isGroup = isGroup;
            f.tokens.push_back(r);
        }
        else
        {
            r.isToken = false;
            f.rules.insert(left, r);
        }
    }
    return f;
}
