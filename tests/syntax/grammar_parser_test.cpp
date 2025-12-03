#include <QtTest/QtTest>
#include "../../src/syntax/GrammarParser.h"

class GrammarParserTest : public QObject
{
    Q_OBJECT
   private slots:
    void parse_simple_grammar()
    {
        QString err;
        auto    g = GrammarParser::parseString("S -> a S | #\n", err);
        QVERIFY(err.isEmpty());
        QVERIFY(!g.productions.isEmpty());
        QVERIFY(g.startSymbol == "S");
    }
};

QTEST_MAIN(GrammarParserTest)
#include "grammar_parser_test.moc"
