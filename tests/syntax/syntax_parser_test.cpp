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
        QString err;
        auto    g    = GrammarParser::parseString("s -> a s | #\n", err);
        QVERIFY(err.isEmpty());
        auto info = LL1::compute(g);
        QVector<QString> toks = {QStringLiteral("a"), QStringLiteral("a")};
        auto res = parseTokens(toks, g, info);
        QVERIFY(res.errorPos < 0);
        QVERIFY(res.root != nullptr);
        QVERIFY(res.root->symbol == QStringLiteral("s"));
        QVERIFY(res.root->children.size() >= 1);
    }
    void parse_epsilon_branch()
    {
        QString err; auto g = GrammarParser::parseString("S -> #\n", err); QVERIFY(err.isEmpty());
        auto info = LL1::compute(g);
        QVector<QString> toks; // empty input
        auto res = parseTokens(toks, g, info);
        QVERIFY(res.errorPos < 0);
        QVERIFY(res.root != nullptr);
        QVERIFY(res.root->symbol == QStringLiteral("S"));
        // ε 产生式不添加子节点
    }
};

QTEST_MAIN(SyntaxParserTest)
#include "syntax_parser_test.moc"
