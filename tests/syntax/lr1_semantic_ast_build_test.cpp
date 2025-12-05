#include <gtest/gtest.h>
#include "../../src/syntax/GrammarParser.h"
#include "../../src/syntax/LR1.h"
#include "../../src/syntax/LR1Parser.h"
#include "../../src/config/Config.h"

TEST(LR1SemanticAstBuild, TinyExample)
{
    QString gram =
        "program -> stmt-sequence\n"
        "stmt-sequence -> stmt-sequence ; statement | statement\n"
        "statement -> if-stmt | repeat-stmt | assign-stmt | read-stmt | write-stmt\n"
        "if-stmt -> if exp then stmt-sequence end | if exp then stmt-sequence else stmt-sequence "
        "end\n"
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
    ASSERT_TRUE(err.isEmpty());
    auto gr  = LR1Builder::build(g);
    auto tbl = LR1Builder::computeActionTable(g, gr);

    // tokens after mapping
    QVector<QString>                     tokens = {"read", "identifier", ";"};
    QMap<QString, QVector<QVector<int>>> acts;
    acts["read-stmt"] = {QVector<int>({1, 2})};
    auto res          = LR1Parser::parseWithSemantics(tokens,
                                             g,
                                             tbl,
                                             acts,
                                             Config::semanticRoleMeaning(),
                                             Config::semanticRootSelectionPolicy(),
                                             Config::semanticChildOrderPolicy());
    ASSERT_EQ(res.errorPos, -1);
    ASSERT_TRUE(res.astRoot != nullptr);
    ASSERT_EQ(res.astRoot->tag, "read");
    ASSERT_FALSE(res.astRoot->children.isEmpty());
}
