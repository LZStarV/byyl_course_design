#include <QtTest/QtTest>
#include "../../src/syntax/GrammarParser.h"
#include "../../src/syntax/LL1.h"

class LL1Test : public QObject
{
    Q_OBJECT
private slots:
    void compute_first_follow()
    {
        QString err;
        auto g = GrammarParser::parseString("s -> a s | #\n", err);
        auto info = LL1::compute(g);
        QVERIFY(info.first.contains("s"));
        QVERIFY(info.follow.contains("s"));
        QVERIFY(info.table.contains("s"));
    }
};

QTEST_MAIN(LL1Test)
#include "ll1_test.moc"
