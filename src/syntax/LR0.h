#pragma once
#include <QString>
#include <QVector>
#include <QMap>
#include <QSet>
#include "Grammar.h"

struct LR0Item
{
    QString          left;
    QVector<QString> right;
    int              dot = 0;
    bool             operator==(const LR0Item& o) const
    {
        return left == o.left && right == o.right && dot == o.dot;
    }
};

struct LR0Graph
{
    QVector<QVector<LR0Item>>     states;
    QMap<int, QMap<QString, int>> edges;
};

class LR0Builder
{
   public:
    static LR0Graph build(const Grammar& g);
    static QString  toDot(const LR0Graph& gr);
};
