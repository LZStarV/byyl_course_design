#pragma once
#include <QString>
#include <QMap>
#include "../regex/RegexParser.h"

class TokenMapBuilder
{
   public:
    static QMap<QString, QString> build(const QString& regexText, const ParsedFile& pf);
    static bool                   saveJson(const QMap<QString, QString>& m, const QString& path);
};
