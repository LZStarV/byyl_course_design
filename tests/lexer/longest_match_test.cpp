#include <QtTest/QtTest>
#include "../../src/Engine.h"
#include "../../src/syntax/TokenMapBuilder.h"

class LongestMatchTest : public QObject {
    Q_OBJECT
  private:
    QString rules() {
        return QStringLiteral(
            "letter=[A-Za-z_]\n"
            "digit=[0-9]\n"
            "_identifier100=letter(letter|digit)*\n"
            "_keywords200S= if | else | read | write | then | end \n");
    }
  private slots:
    void longest_match_keywords_vs_identifier() {
        Engine eng; auto rf = eng.lexFile(rules()); auto pf = eng.parseFile(rf);
        QVector<int> codes; auto mdfs = eng.buildAllMinDFA(pf, codes);
        auto out = eng.runMultiple(mdfs, codes, QStringLiteral("if abc"), QSet<int>());
        auto toks = out.split(' ', Qt::SkipEmptyParts);
        QVERIFY(!toks.isEmpty());
        // 第一个 token 应为关键字编码200系列之一，而不是标识符100
        QVERIFY(toks[0] != QString::number(100));
    }
};

QTEST_MAIN(LongestMatchTest)
#include "longest_match_test.moc"
