#pragma once
#include <QString>
#include "AST.h"
#include "LR1Parser.h"

QString syntaxAstToDot(SyntaxASTNode* root);
QString parseTreeToDot(ParseTreeNode* root);
QString parseTreeToDotWithTokens(ParseTreeNode* root, const QVector<QString>& tokens);
QString semanticAstToDot(SemanticASTNode* root);
