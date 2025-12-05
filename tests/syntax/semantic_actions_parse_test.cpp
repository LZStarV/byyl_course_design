#include <gtest/gtest.h>
#include "../../src/config/Config.h"
#include "../../src/syntax/LR1Parser.h"
#include "../../src/syntax/GrammarParser.h"

static QMap<QString, QVector<QVector<int>>> parseActions(const QVector<QString>& lines)
{
    QMap<QString, QVector<QVector<int>>> m;
    for (int i = 0; i + 1 < lines.size(); i += 2)
    {
        QString               prod  = lines[i].trimmed();
        QString               acts  = lines[i + 1].trimmed();
        int                   arrow = prod.indexOf("->");
        QString               L     = prod.left(arrow).trimmed();
        QString               Rall  = prod.mid(arrow + 2).trimmed();
        auto                  rhss  = Rall.split('|');
        auto                  actc  = acts.split('|');
        QVector<QVector<int>> seqs;
        for (int k = 0; k < rhss.size() && k < actc.size(); ++k)
        {
            auto rhs    = rhss[k].trimmed();
            auto actstr = actc[k].trimmed();
            auto syms   = rhs == "#" ? QVector<QString>() : rhs.split(' ', Qt::SkipEmptyParts);
            auto bits   = actstr.split(' ', Qt::SkipEmptyParts);
            QVector<int> vs;
            for (auto b : bits) vs.push_back(b.toInt());
            seqs.push_back(vs);
        }
        m[L] = seqs;
    }
    return m;
}

TEST(SemanticActionsParse, Basic)
{
    QVector<QString> lines = {
        "read-stmt -> read identifier",
        "1 2",
        "assign-stmt -> identifier := exp",
        "2 1 2",
    };
    auto m = parseActions(lines);
    ASSERT_TRUE(m.contains("read-stmt"));
    ASSERT_TRUE(m.contains("assign-stmt"));
    ASSERT_EQ(m.value("read-stmt").size(), 1);
    ASSERT_EQ(m.value("read-stmt")[0].size(), 2);
    ASSERT_EQ(m.value("read-stmt")[0][0], 1);
    ASSERT_EQ(m.value("read-stmt")[0][1], 2);
}
