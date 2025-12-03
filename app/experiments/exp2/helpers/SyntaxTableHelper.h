#pragma once
#include <QMap>
#include <QVector>
#include <QSet>
#include <QString>
#include <QTableWidget>
#include <QPlainTextEdit>

class SyntaxTableHelper
{
   public:
    static void fillFirstTable(QTableWidget* tbl, const QMap<QString, QVector<QString>>& firstRows);
    static void fillFollowTable(QTableWidget* tbl, const QMap<QString, QSet<QString>>& followRows);
    static void syncTokensView(QPlainTextEdit* dest, QPlainTextEdit* source);
};
