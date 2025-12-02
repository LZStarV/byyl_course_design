#pragma once
#include <QString>
#include <QVector>

struct SyntaxASTNode
{
    QString symbol;
    QVector<SyntaxASTNode*> children;
};
