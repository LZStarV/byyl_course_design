#pragma once
#include <QString>
#include <QVector>
#include <QMap>
#include <QSet>

struct Production
{
    QString left;
    QVector<QString> right;
    int line = -1;
};

class Grammar
{
   public:
    QSet<QString> terminals;
    QSet<QString> nonterminals;
    QString startSymbol;
    QMap<QString, QVector<Production>> productions;
    bool hasEpsilon(const QVector<QString>& rhs) const;
};

