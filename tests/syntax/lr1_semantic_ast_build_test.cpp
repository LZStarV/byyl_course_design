#include <QtTest/QtTest>
#include "../../src/syntax/GrammarParser.h"
#include "../../src/syntax/LR1.h"
#include "../../src/syntax/LR1Parser.h"
#include "../../src/config/Config.h"

class LR1SemanticAstBuildTest : public QObject
{
    Q_OBJECT
   private slots:
    void tiny_example()
    {
        QString gram =
            "program -> stmt-sequence\n"
            "stmt-sequence -> stmt-sequence ; statement | statement\n"
            "statement -> if-stmt | repeat-stmt | assign-stmt | read-stmt | write-stmt\n"
            "if-stmt -> if exp then stmt-sequence end | if exp then stmt-sequence else "
            "stmt-sequence end\n"
            "repeat-stmt -> repeat stmt-sequence until exp\n"
            "assign-stmt -> identifier := exp\n"
            "read-stmt -> read identifier\n"
            "write-stmt -> write exp\n"
            "exp -> simple-exp comparison-op simple-exp | simple-exp\n"
            "comparison-op -> < | > | = | <= | <> | >=\n"
            "simple-exp -> simple-exp addop term | term\n"
            "addop -> + | -\n"
            "term -> term mulop factor | factor\n"
            "mulop -> * | / | % | ^\n"
            "factor -> ( exp ) | number | identifier\n";
        QString err;
        auto    g = GrammarParser::parse(gram, err);
        QVERIFY(err.isEmpty());
        auto             gr     = LR1Builder::build(g);
        auto             tbl    = LR1Builder::computeActionTable(g, gr);
        QVector<QString> tokens = {
            QStringLiteral("read"), QStringLiteral("identifier"), QStringLiteral(";")};
        QMap<QString, QVector<QVector<int>>> acts;
        acts[QStringLiteral("read-stmt")] = {QVector<int>({1, 2})};
        auto res                          = LR1Parser::parseWithSemantics(tokens,
                                                 g,
                                                 tbl,
                                                 acts,
                                                 Config::semanticRoleMeaning(),
                                                 Config::semanticRootSelectionPolicy(),
                                                 Config::semanticChildOrderPolicy());
        QCOMPARE(res.errorPos, -1);
        QVERIFY(res.astRoot != nullptr);
        QCOMPARE(res.astRoot->tag, QStringLiteral("read"));
        QVERIFY(!res.astRoot->children.isEmpty());
    }
    void assign_stmt_positive()
    {
        QString gram =
            "assign-stmt -> identifier := exp\n"
            "exp -> identifier | number\n";
        QString err;
        auto    g = GrammarParser::parse(gram, err);
        QVERIFY(err.isEmpty());
        auto             gr     = LR1Builder::build(g);
        auto             tbl    = LR1Builder::computeActionTable(g, gr);
        QVector<QString> tokens = {
            QStringLiteral("identifier"), QStringLiteral(":="), QStringLiteral("number")};
        QMap<QString, QVector<QVector<int>>> acts;
        acts[QStringLiteral("assign-stmt")] = {QVector<int>({1, 2, 3})};
        auto res                            = LR1Parser::parseWithSemantics(tokens,
                                                 g,
                                                 tbl,
                                                 acts,
                                                 Config::semanticRoleMeaning(),
                                                 Config::semanticRootSelectionPolicy(),
                                                 Config::semanticChildOrderPolicy());
        QCOMPARE(res.errorPos, -1);
        QVERIFY(res.astRoot != nullptr);
    }
    void missing_semicolon_negative()
    {
        QString gram = "stmt -> read identifier ;\n";
        QString err;
        auto    g = GrammarParser::parse(gram, err);
        QVERIFY(err.isEmpty());
        auto             gr     = LR1Builder::build(g);
        auto             tbl    = LR1Builder::computeActionTable(g, gr);
        QVector<QString> tokens = {QStringLiteral("read"), QStringLiteral("identifier")};
        QMap<QString, QVector<QVector<int>>> acts;
        acts[QStringLiteral("stmt")] = {QVector<int>({1, 2, 3})};
        auto res                     = LR1Parser::parseWithSemantics(tokens,
                                                 g,
                                                 tbl,
                                                 acts,
                                                 Config::semanticRoleMeaning(),
                                                 Config::semanticRootSelectionPolicy(),
                                                 Config::semanticChildOrderPolicy());
        QVERIFY(res.errorPos >= 0);
    }
};

QTEST_MAIN(LR1SemanticAstBuildTest)
#include "lr1_semantic_ast_build_test.moc"
