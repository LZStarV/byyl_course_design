#pragma once
#include <QString>
#include <QSet>
#include <QVector>
struct Alphabet
{
    QSet<QString> symbols;
    bool          hasLetter = false;
    bool          hasDigit  = false;
    void          add(const QString& s)
    {
        symbols.insert(s);
    }
    QVector<QString> ordered() const
    {
        return QVector<QString>(symbols.begin(), symbols.end());
    }
};
