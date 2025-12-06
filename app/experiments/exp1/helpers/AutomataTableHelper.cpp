#include "AutomataTableHelper.h"
#include <QSet>
#include <QMap>
#include <QStringList>
#include <QTableWidget>
#include "../../../src/regex/RegexLexer.h"

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
    pruneEmptyColumns(tbl);
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

void AutomataTableHelper::pruneEmptyColumns(QTableWidget* tbl)
{
    if (!tbl)
        return;
    QSet<QString> keep;
    keep.insert(QStringLiteral("标记"));
    keep.insert(QStringLiteral("状态 ID"));
    keep.insert(QStringLiteral("状态集合"));
    for (int c = tbl->columnCount() - 1; c >= 0; --c)
    {
        QString header =
            tbl->horizontalHeaderItem(c) ? tbl->horizontalHeaderItem(c)->text() : QString();
        if (keep.contains(header))
            continue;
        bool any = false;
        for (int r = 0; r < tbl->rowCount(); ++r)
        {
            auto it = tbl->item(r, c);
            if (it && !it->text().trimmed().isEmpty())
            {
                any = true;
                break;
            }
        }
        if (!any)
            tbl->removeColumn(c);
    }
}

static QSet<QChar> expandRanges(const QString& ranges)
{
    QSet<QChar> set;
    int         i      = 0;
    auto        n      = ranges.size();
    auto        skipWs = [&]()
    {
        while (i < n && (ranges[i] == ' ' || ranges[i] == '\t')) i++;
    };
    skipWs();
    while (i < n)
    {
        skipWs();
        if (i >= n)
            break;
        QChar a = ranges[i++];
        if (i < n && ranges[i] == '-')
        {
            i++;
            if (i < n)
            {
                QChar  b  = ranges[i++];
                ushort ua = a.unicode(), ub = b.unicode();
                if (ua <= ub)
                {
                    for (ushort u = ua; u <= ub; ++u) set.insert(QChar(u));
                }
                else
                {
                    for (ushort u = ub; u <= ua; ++u) set.insert(QChar(u));
                }
            }
        }
        else
        {
            set.insert(a);
        }
        skipWs();
        if (i < n && (ranges[i] == ',' || ranges[i] == ';'))
            i++;
        skipWs();
    }
    return set;
}

QMap<QString, QSet<QChar>> AutomataTableHelper::buildMacroSets(const QMap<QString, Rule>& macros)
{
    QMap<QString, QSet<QChar>> msets;
    auto                       keys = macros.keys();
    for (const auto& k : keys)
    {
        const auto&    r = macros.value(k);
        QSet<QChar>    set;
        const QString& expr = r.expr;
        int            i = 0, n = expr.size();
        while (i < n)
        {
            while (i < n &&
                   (expr[i] == ' ' || expr[i] == '\t' || expr[i] == '\n' || expr[i] == '\r'))
                i++;
            if (i >= n)
                break;
            if (expr[i] == '[')
            {
                int     j = i + 1;
                QString ranges;
                while (j < n && expr[j] != ']')
                {
                    ranges.append(expr[j]);
                    j++;
                }
                if (j < n && expr[j] == ']')
                {
                    set.unite(expandRanges(ranges));
                    i = j + 1;
                }
                else
                {
                    break;
                }
            }
            else if (expr[i] == '(')
            {
                int     j = i + 1, depth = 1;
                QString inner;
                while (j < n && depth > 0)
                {
                    if (expr[j] == '(')
                        depth++;
                    else if (expr[j] == ')')
                        depth--;
                    if (depth > 0)
                        inner.append(expr[j]);
                    j++;
                }
                set.unite(expandRanges(inner));
                i = j;
            }
            else if (expr[i] == '\\')
            {
                if (i + 1 < n)
                {
                    set.insert(expr[i + 1]);
                    i += 2;
                }
                else
                    break;
            }
            else
            {
                set.insert(expr[i]);
                i++;
            }
        }
        if (!set.isEmpty())
            msets.insert(r.name, set);
    }
    return msets;
}

QMap<QString, QString> AutomataTableHelper::buildMacroExprs(const QMap<QString, Rule>& macros)
{
    QMap<QString, QString> m;
    for (auto it = macros.begin(); it != macros.end(); ++it)
    {
        m.insert(it->name, it->expr);
    }
    return m;
}

static QString mergeTargets(const QStringList& parts)
{
    QSet<QString> uniq;
    for (const auto& p : parts)
    {
        for (const auto& seg : p.split(',', Qt::SkipEmptyParts)) uniq.insert(seg.trimmed());
    }
    QList<QString> v = QList<QString>(uniq.begin(), uniq.end());
    std::sort(v.begin(), v.end());
    return v.isEmpty() ? QString() : v.join(',');
}

void AutomataTableHelper::aggregateByMacros(Tables&                           t,
                                            const QMap<QString, QSet<QChar>>& macroSets,
                                            const QMap<QString, QString>&     macroExprs)
{
    if (macroSets.isEmpty())
        return;
    int colCount = t.columns.size();
    if (colCount < 3)
        return;
    bool             hasEps   = t.columns.last() == QStringLiteral("#");
    int              symStart = 2;
    int              symEnd   = hasEps ? (colCount - 1) : colCount;
    QMap<QChar, int> charColIdx;
    for (int ci = symStart; ci < symEnd; ++ci)
    {
        const QString& c = t.columns[ci];
        if (c.size() == 1)
            charColIdx.insert(c[0], ci);
    }
    QSet<int>        removeIdx;
    QVector<QString> newCols;
    newCols.push_back(t.columns[0]);
    newCols.push_back(t.columns[1]);
    QVector<int> keepIdx;
    auto         mkeys = macroSets.keys();
    std::sort(mkeys.begin(), mkeys.end());
    QMap<QString, QVector<int>> macroHitCols;
    for (const auto& name : mkeys)
    {
        const auto&  set = macroSets.value(name);
        QVector<int> idxs;
        for (auto ch : set)
        {
            if (charColIdx.contains(ch))
                idxs.push_back(charColIdx.value(ch));
        }
        if (!idxs.isEmpty())
        {
            std::sort(idxs.begin(), idxs.end());
            macroHitCols.insert(name, idxs);
            for (int id : idxs) removeIdx.insert(id);
            QString label = name;
            if (macroExprs.contains(name))
                label += QStringLiteral(" (") + macroExprs.value(name) + QStringLiteral(")");
            newCols.push_back(label);
        }
    }
    for (int ci = symStart; ci < symEnd; ++ci)
        if (!removeIdx.contains(ci))
        {
            newCols.push_back(t.columns[ci]);
            keepIdx.push_back(ci);
        }
    if (hasEps)
        newCols.push_back(QStringLiteral("#"));
    QVector<QVector<QString>> newRows;
    for (const auto& row : t.rows)
    {
        QVector<QString> nr;
        nr.push_back(row[0]);
        nr.push_back(row[1]);
        for (const auto& name : mkeys)
        {
            if (!macroHitCols.contains(name))
                continue;
            QStringList parts;
            for (int id : macroHitCols.value(name)) parts << row[id];
            nr.push_back(mergeTargets(parts));
        }
        for (int id : keepIdx) nr.push_back(row[id]);
        if (hasEps)
            nr.push_back(row[symEnd]);
        newRows.push_back(nr);
    }
    t.columns = newCols;
    t.rows    = newRows;
}
