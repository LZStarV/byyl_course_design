#pragma once
#include <QVector>
#include <QString>
#include <QTableWidget>
#include "../../../src/Engine.h"

class AutomataTableHelper
{
   public:
    static void             fillTable(QTableWidget* tbl, const Tables& t);
    static QVector<QString> unionSyms(const QVector<Tables>& tables, bool includeEps);
};
