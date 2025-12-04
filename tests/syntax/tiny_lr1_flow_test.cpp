#include <QtTest/QtTest>
#include "../../src/Engine.h"
#include "../../src/syntax/GrammarParser.h"
#include "../../src/syntax/LR1.h"
#include "../../src/syntax/LR1Parser.h"
#include "../../src/syntax/TokenMapBuilder.h"

class TinyLR1FlowTest : public QObject
{
    Q_OBJECT
   private slots:
    void test_flow()
    {
        QFile fr("../tests/test_data/regex/tiny.regex");
        QVERIFY(fr.open(QIODevice::ReadOnly | QIODevice::Text));
        QString regexText = QTextStream(&fr).readAll();
        fr.close();

        Engine       engine;
        auto         rf = engine.lexFile(regexText);
        auto         pf = engine.parseFile(rf);
        QVector<int> codes;
        auto         mdfas = engine.buildAllMinDFA(pf, codes);

        QFile fs("../tests/test_data/sample/tiny/tiny1.tny");
        QVERIFY(fs.open(QIODevice::ReadOnly | QIODevice::Text));
        QString src = QTextStream(&fs).readAll();
        fs.close();
        auto tokenStr = engine.runMultiple(mdfas, codes, src);
        qWarning() << "[tiny] tokens(raw)" << tokenStr;
        auto             map = TokenMapBuilder::build(regexText, pf);
        QVector<QString> tokens;
        for (auto x : tokenStr.split(' ', Qt::SkipEmptyParts)) tokens.push_back(map.value(x, x));
        QString mapped;
        for (int i = 0; i < tokens.size() && i < 40; ++i)
        {
            mapped += tokens[i] + " ";
        }
        qWarning() << "[tiny] tokens(mapped <=40)" << mapped.trimmed();

        QFile fg("../tests/test_data/syntax/tiny_grammar.txt");
        QVERIFY(fg.open(QIODevice::ReadOnly | QIODevice::Text));
        QString gtext = QTextStream(&fg).readAll();
        fg.close();
        QString err;
        auto    g = engine.parseGrammarText(gtext, err);
        QVERIFY(err.isEmpty());
        auto gr = LR1Builder::build(g);
        auto tb = LR1Builder::computeActionTable(g, gr);
        auto r  = LR1Parser::parse(tokens, g, tb);
        QVERIFY2(r.errorPos < 0, "LR1 parse failed for tiny program");
    }
};

QTEST_MAIN(TinyLR1FlowTest)
#include "tiny_lr1_flow_test.moc"
