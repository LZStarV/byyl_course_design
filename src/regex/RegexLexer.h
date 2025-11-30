#pragma once
#include <QString>
#include <QVector>
#include <QMap>
struct RegexToken { QString kind; QString text; };
struct Rule { QString name; QString expr; bool isToken=false; int code=0; bool isGroup=false; };
struct RegexFile { QMap<QString,Rule> rules; QVector<Rule> tokens; };
class RegexLexer {
public:
    static RegexFile lex(const QString& input);
};
