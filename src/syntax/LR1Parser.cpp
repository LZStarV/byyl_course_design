#include "LR1Parser.h"

static QString actionFor(const LR1ActionTable& t, int st, const QString& a)
{
    return t.action.value(st).value(a);
}

static int gotoFor(const LR1ActionTable& t, int st, const QString& A)
{
    return t.gotoTable.value(st).value(A, -1);
}

static QVector<QString> tokenize(const QString& s)
{
    QVector<QString> v;
    for (auto x : s.split(' ', Qt::SkipEmptyParts)) v.push_back(x.trimmed());
    return v;
}

static void pushStep(QVector<ParseStep>&                 steps,
                     int                                 stepIdx,
                     const QVector<QPair<int, QString>>& stk,
                     const QVector<QString>&             rest,
                     const QString&                      act,
                     const QString&                      prod)
{
    ParseStep ps;
    ps.step       = stepIdx;
    ps.stack      = stk;
    ps.rest       = rest;
    ps.action     = act;
    ps.production = prod;
    steps.push_back(ps);
}

ParseResult LR1Parser::parse(const QVector<QString>& tokens,
                             const Grammar&          g,
                             const LR1ActionTable&   t)
{
    ParseResult      res;
    QVector<QString> input = tokens;
    input.push_back("$");
    QVector<QPair<int, QString>> stack;
    QVector<ParseTreeNode*>      nodeStk;
    stack.push_back({0, QString()});
    int step = 0;
    while (!input.isEmpty())
    {
        QString a   = input[0];
        int     st  = stack.isEmpty() ? -1 : stack.back().first;
        QString act = actionFor(t, st, a);
        if (act.contains('|'))
        {
            auto    parts = act.split('|');
            QString pick;
            for (auto p : parts)
                if (p.startsWith("s"))
                {
                    pick = p;
                    break;
                }
            if (pick.isEmpty())
                pick = parts[0];
            act = pick;
        }
        if (act.isEmpty())
        {
            res.errorPos = res.steps.size();
            break;
        }
        if (act == "acc")
        {
            pushStep(res.steps, step++, stack, input, act, QString());
            if (!nodeStk.isEmpty())
                res.root = nodeStk.back();
            break;
        }
        if (act.startsWith("s"))
        {
            int to = act.mid(1).toInt();
            stack.push_back({to, a});
            ParseTreeNode* n = new ParseTreeNode;
            n->symbol        = a;
            res.root         = n;
            nodeStk.push_back(n);
            pushStep(res.steps, step++, stack, input, act, QString());
            input.pop_front();
            continue;
        }
        if (act.startsWith("r"))
        {
            QString                 prod  = act.mid(1).trimmed();
            int                     arrow = prod.indexOf("->");
            QString                 L     = prod.left(arrow).trimmed();
            QString                 R     = prod.mid(arrow + 2).trimmed();
            QVector<QString>        rhs   = tokenize(R);
            int                     k     = rhs[0] == "#" ? 0 : rhs.size();
            QVector<ParseTreeNode*> kids;
            for (int i = 0; i < k; ++i)
            {
                if (!stack.isEmpty())
                    stack.pop_back();
                if (!nodeStk.isEmpty())
                {
                    kids.push_back(nodeStk.back());
                    nodeStk.pop_back();
                }
            }
            std::reverse(kids.begin(), kids.end());
            int stTop = stack.isEmpty() ? -1 : stack.back().first;
            int to    = gotoFor(t, stTop, L);
            if (to < 0)
            {
                res.errorPos = res.steps.size();
                break;
            }
            stack.push_back({to, L});
            ParseTreeNode* p = new ParseTreeNode;
            p->symbol        = L;
            p->children      = kids;
            nodeStk.push_back(p);
            res.root = p;
            pushStep(res.steps, step++, stack, input, act, QString("%1 -> %2").arg(L).arg(R));
            continue;
        }
        res.errorPos = res.steps.size();
        break;
    }
    return res;
}
