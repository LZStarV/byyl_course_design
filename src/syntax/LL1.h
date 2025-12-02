#pragma once
#include <QString>
#include <QSet>
#include <QMap>
#include <QVector>
#include "Grammar.h"

struct LL1Info
{
    QMap<QString, QSet<QString>> first;
    QMap<QString, QSet<QString>> follow;
    QMap<QString, QMap<QString, int>> table;
    QVector<QString> conflicts;
};

class LL1
{
   public:
    static LL1Info compute(const Grammar& g);
};
