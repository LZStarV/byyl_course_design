#pragma once
#include <QVector>
#include <QString>
#include <QTableWidget>
#include "../../../../src/Engine.h"

class AutomataTableHelper
{
   public:
    static void                       fillTable(QTableWidget* tbl, const Tables& t);
    static QVector<QString>           unionSyms(const QVector<Tables>& tables, bool includeEps);
    static void                       pruneEmptyColumns(QTableWidget* tbl);
    static QMap<QString, QSet<QChar>> buildMacroSets(const QMap<QString, Rule>& macros);
    static QMap<QString, QString>     buildMacroExprs(const QMap<QString, Rule>& macros);
    static void                       aggregateByMacros(Tables&                           t,
                                                        const QMap<QString, QSet<QChar>>& macroSets,
                                                        const QMap<QString, QString>&     macroExprs);
};
