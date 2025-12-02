#pragma once
#include <QVector>
#include <QString>
#include "Grammar.h"
#include "LL1.h"
#include "AST.h"

struct SyntaxResult
{
    SyntaxASTNode* root = nullptr;
    int errorPos = -1;
};

SyntaxResult parseTokens(const QVector<QString>& tokens,
                         const Grammar& g,
                         const LL1Info& info);

