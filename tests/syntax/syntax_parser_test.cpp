#include <QtTest/QtTest>
#include "../../src/syntax/GrammarParser.h"
#include "../../src/syntax/LL1.h"
#include "../../src/syntax/SyntaxParser.h"

class SyntaxParserTest : public QObject
{
    Q_OBJECT
   private slots:
    void parse_tokens_basic()
    {
        QString        err;
        auto           g    = GrammarParser::parseString("s -> a s | #\n", err);
        auto           info = LL1::compute(g);
        SyntaxASTNode* root = new SyntaxASTNode;
        QVERIFY(root != nullptr);
    }
};

QTEST_MAIN(SyntaxParserTest)
#include "syntax_parser_test.moc"
