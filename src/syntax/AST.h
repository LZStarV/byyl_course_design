#pragma once
#include <QString>
#include <QVector>

struct SyntaxASTNode
{
    QString                 symbol;
    QVector<SyntaxASTNode*> children;
};

struct SemanticASTNode
{
    QString                   tag;
    QVector<SemanticASTNode*> children;
};
