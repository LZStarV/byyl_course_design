#include "AutomataTableHelper.h"

void AutomataTableHelper::fillTable(QTableWidget* tbl, const Tables& t)
{
    if (!tbl)
        return;
    tbl->clear();
    tbl->setColumnCount(t.columns.size());
    tbl->setRowCount(t.rows.size());
    QStringList headers;
    for (auto c : t.columns) headers << c;
    tbl->setHorizontalHeaderLabels(headers);
    for (int r = 0; r < t.rows.size(); ++r)
    {
        auto row = t.rows[r];
        for (int c = 0; c < row.size(); ++c)
        {
            tbl->setItem(r, c, new QTableWidgetItem(row[c]));
        }
    }
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

QVector<QString> AutomataTableHelper::unionSyms(const QVector<Tables>& tables, bool includeEps)
{
    QSet<QString> s;
    for (const auto& t : tables)
    {
        for (int i = 2; i < t.columns.size(); ++i)
        {
            QString c = t.columns[i];
            if (includeEps || c != "#")
                s.insert(c);
        }
    }
    QVector<QString> v = QVector<QString>(s.begin(), s.end());
    std::sort(v.begin(), v.end());
    return v;
}
