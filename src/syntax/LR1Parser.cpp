#include "LR1Parser.h"
#include "../config/Config.h"

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
            auto    parts  = act.split('|');
            QString policy = Config::lr1ConflictPolicy().trimmed().toLower();
            if (policy == "prefer_shift")
            {
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
            else if (policy == "prefer_reduce")
            {
                QString pick;
                for (auto p : parts)
                    if (p.startsWith("r"))
                    {
                        pick = p;
                        break;
                    }
                if (pick.isEmpty())
                    pick = parts[0];
                act = pick;
            }
            else
            {
                res.errorPos = res.steps.size();
                break;
            }
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

static SemanticASTNode* makeSemNode(const QString& tag)
{
    auto n = new SemanticASTNode();
    n->tag = tag;
    return n;
}

static SemanticASTNode* buildSemantic(const QString&                   L,
                                      const QVector<SemanticASTNode*>& semKids,
                                      const QVector<int>&              roles,
                                      const QMap<int, QString>&        roleMeaning,
                                      const QString&                   rootPolicy,
                                      const QString&                   childOrder)
{
    int rootIdx = -1;
    for (int i = 0; i < roles.size(); ++i)
    {
        auto m = roleMeaning.value(roles[i]);
        if (m == "root")
        {
            if (rootIdx < 0)
                rootIdx = i;
            else if (rootPolicy == "last_1")
                rootIdx = i;
        }
    }
    QVector<int> childIdx;
    for (int i = 0; i < roles.size(); ++i)
        if (roleMeaning.value(roles[i]) == "child")
            childIdx.push_back(i);
    SemanticASTNode* root = nullptr;
    if (rootIdx >= 0)
    {
        QString tag = (rootIdx < semKids.size() && semKids[rootIdx]) ? semKids[rootIdx]->tag : L;
        root        = makeSemNode(tag);
    }
    else
    {
        root = makeSemNode(L);
    }
    if (childOrder == "rhs_order")
    {
        for (int idx : childIdx)
        {
            if (idx < semKids.size() && semKids[idx])
                root->children.push_back(semKids[idx]);
        }
    }
    else
    {
        for (int idx : childIdx)
        {
            if (idx < semKids.size() && semKids[idx])
                root->children.push_back(semKids[idx]);
        }
    }
    return root;
}

ParseResult LR1Parser::parseWithSemantics(const QVector<QString>&                     tokens,
                                          const Grammar&                              g,
                                          const LR1ActionTable&                       t,
                                          const QMap<QString, QVector<QVector<int>>>& actions,
                                          const QMap<int, QString>&                   roleMeaning,
                                          const QString&                              rootPolicy,
                                          const QString&                              childOrder)
{
    ParseResult      res;
    QVector<QString> input = tokens;
    input.push_back("$");
    QVector<QPair<int, QString>> stack;
    QVector<ParseTreeNode*>      nodeStk;
    QVector<SemanticASTNode*>    semStk;
    stack.push_back({0, QString()});
    int step = 0;
    while (!input.isEmpty())
    {
        QString a   = input[0];
        int     st  = stack.isEmpty() ? -1 : stack.back().first;
        QString act = actionFor(t, st, a);
        if (act.contains('|'))
        {
            auto    parts  = act.split('|');
            QString policy = Config::lr1ConflictPolicy().trimmed().toLower();
            if (policy == "prefer_shift")
            {
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
            else if (policy == "prefer_reduce")
            {
                QString pick;
                for (auto p : parts)
                    if (p.startsWith("r"))
                    {
                        pick = p;
                        break;
                    }
                if (pick.isEmpty())
                    pick = parts[0];
                act = pick;
            }
            else
            {
                res.errorPos = res.steps.size();
                break;
            }
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
            if (!semStk.isEmpty())
                res.astRoot = semStk.back();
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
            // 语义：移进叶子占位（在归约时统一组装）；这里先压入一个叶子节点（tag=a）
            semStk.push_back(makeSemNode(a));
            pushStep(res.steps, step++, stack, input, act, QString());
            input.pop_front();
            continue;
        }
        if (act.startsWith("r"))
        {
            QString                   prod  = act.mid(1).trimmed();
            int                       arrow = prod.indexOf("->");
            QString                   L     = prod.left(arrow).trimmed();
            QString                   R     = prod.mid(arrow + 2).trimmed();
            QVector<QString>          rhs   = tokenize(R);
            int                       k     = rhs[0] == "#" ? 0 : rhs.size();
            QVector<ParseTreeNode*>   kids;
            QVector<SemanticASTNode*> semKids;
            for (int i = 0; i < k; ++i)
            {
                if (!stack.isEmpty())
                    stack.pop_back();
                if (!nodeStk.isEmpty())
                {
                    kids.push_back(nodeStk.back());
                    nodeStk.pop_back();
                }
                if (!semStk.isEmpty())
                {
                    semKids.push_back(semStk.back());
                    semStk.pop_back();
                }
            }
            std::reverse(kids.begin(), kids.end());
            std::reverse(semKids.begin(), semKids.end());
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
            // 语义：按配置与角色位组合
            QVector<int> roles;
            if (actions.contains(L))
            {
                const auto& vec = actions.value(L);
                // 根据实际规约候选（RHS 完整匹配）选择角色位
                int pick = -1;
                if (g.productions.contains(L))
                {
                    const auto& alts = g.productions.value(L);
                    for (int i = 0; i < alts.size(); ++i)
                    {
                        const auto& alt = alts[i];
                        if (alt.right.size() == rhs.size())
                        {
                            bool eq = true;
                            for (int j = 0; j < rhs.size(); ++j)
                            {
                                if (alt.right[j] != rhs[j])
                                {
                                    eq = false;
                                    break;
                                }
                            }
                            if (eq)
                            {
                                pick = i;
                                break;
                            }
                        }
                    }
                }
                if (pick >= 0 && pick < vec.size())
                    roles = vec[pick];
            }
            auto sem = buildSemantic(L, semKids, roles, roleMeaning, rootPolicy, childOrder);
            semStk.push_back(sem);
            res.astRoot = sem;
            pushStep(res.steps, step++, stack, input, act, QString("%1 -> %2").arg(L).arg(R));
            continue;
        }
        res.errorPos = res.steps.size();
        break;
    }
    return res;
}
