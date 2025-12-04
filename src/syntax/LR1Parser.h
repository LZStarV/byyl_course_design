#pragma once
#include <QString>
#include <QVector>
#include <QMap>
#include "Grammar.h"
#include "LR1.h"

struct ParseTreeNode
{
    QString                 symbol;
    QVector<ParseTreeNode*> children;
};

struct ParseStep
{
    int                          step = 0;
    QVector<QPair<int, QString>> stack;
    QVector<QString>             rest;
    QString                      action;
    QString                      production;
};

struct ParseResult
{
    QVector<ParseStep> steps;
    ParseTreeNode*     root     = nullptr;
    int                errorPos = -1;
};

class LR1Parser
{
   public:
    static ParseResult parse(const QVector<QString>& tokens,
                             const Grammar&          g,
                             const LR1ActionTable&   t);
};
