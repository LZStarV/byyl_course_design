/*
 * 版权信息：Copyright (c) 2023 林展星
 * 文件名称：LR1Parser.cpp
 *           支持冲突策略与优先移进终结符配置，输出解析步骤与语义树。
 *
 * 当前版本：1.0.0
 * 作    者：林展星
 * 完成日期：2023年12月7日
 *
 * 版本历史：
 * 1.0.0 2023-12-07 林展星 初始版本
 */
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

static bool parseReduction(const LR1ActionTable& t,
                           const QString&        act,
                           QString&              L,
                           QVector<QString>&     rhs)
{
    QString s   = act.mid(1).trimmed();
    bool    ok  = false;
    int     idx = s.toInt(&ok);
    if (ok)
    {
        for (const auto& pr : t.reductions)
        {
            if (pr.first == idx)
            {
                QString text  = pr.second;
                int     arrow = text.indexOf("->");
                L             = text.left(arrow).trimmed();
                QString R     = text.mid(arrow + 2).trimmed();
                rhs.clear();
                if (!R.isEmpty())
                {
                    for (auto x : R.split(' ', Qt::SkipEmptyParts)) rhs.push_back(x.trimmed());
                }
                return true;
            }
        }
    }
    int arrow = s.indexOf("->");
    if (arrow >= 0)
    {
        L         = s.left(arrow).trimmed();
        QString R = s.mid(arrow + 2).trimmed();
        rhs.clear();
        if (!R.isEmpty())
        {
            for (auto x : R.split(' ', Qt::SkipEmptyParts)) rhs.push_back(x.trimmed());
        }
        return true;
    }
    return false;
}

static int reductionIdFor(const LR1ActionTable& t, const QString& L, const QVector<QString>& rhs)
{
    QString key = L + " -> " + (rhs.isEmpty() ? QString("#") : rhs.join(" "));
    for (const auto& pr : t.reductions)
    {
        if (pr.second == key)
            return pr.first;
    }
    return -1;
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
            auto parts  = act.split('|');
            bool hasAcc = false;
            for (auto p : parts)
                if (p == QStringLiteral("acc"))
                {
                    hasAcc = true;
                    break;
                }
            if (hasAcc)
            {
                act = QStringLiteral("acc");
            }
            else
            {
                QString policy  = Config::lr1ConflictPolicy().trimmed().toLower();
                auto    prefer  = Config::lr1PreferShiftTokens();
                QString nextTok = a;
                if (!prefer.isEmpty())
                {
                    for (const auto& pt : prefer)
                    {
                        if (nextTok == pt)
                        {
                            QString pick;
                            for (auto p : parts)
                                if (p.startsWith("s"))
                                {
                                    pick = p;
                                    break;
                                }
                            if (!pick.isEmpty())
                            {
                                act = pick;
                                break;
                            }
                        }
                    }
                }
                if (act.isEmpty())
                {
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
                        QString msg =
                            QString("错误：状态=%1, 前瞻=%2, 动作冲突未配置，中止").arg(st).arg(a);
                        pushStep(res.steps, step++, stack, input, QStringLiteral("error"), msg);
                        res.errorPos = res.steps.size();
                        break;
                    }
                }
            }
        }
        if (act.isEmpty())
        {
            // 错误触发点后移：若上一步为移进，则用当前lookahead的下一符号重试一次
            if (!res.steps.isEmpty() && res.steps.back().action.startsWith("s") && input.size() > 1)
            {
                QString a2   = input[1];
                QString act2 = actionFor(t, st, a2);
                if (!act2.isEmpty())
                    act = act2;
            }
            if (act.isEmpty())
            {
                QString msg = QString("错误：状态=%1, 前瞻=%2, 无可用动作，中止").arg(st).arg(a);
                pushStep(res.steps, step++, stack, input, QStringLiteral("error"), msg);
                res.errorPos = res.steps.size();
                break;
            }
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
            // 语义过程记录：移进叶子
            pushStep(res.semanticSteps, step, stack, input, QString("shift %1").arg(a), QString());
            pushStep(res.steps, step++, stack, input, act, QString());
            input.pop_front();
            continue;
        }
        if (act.startsWith("r"))
        {
            QString          L;
            QVector<QString> rhs;
            if (!parseReduction(t, act, L, rhs))
            {
                QString msg = QString("错误：归约解析失败，动作=%1，中止").arg(act);
                pushStep(res.steps, step++, stack, input, QStringLiteral("error"), msg);
                res.errorPos = res.steps.size();
                break;
            }
            int                     k = rhs.isEmpty() ? 0 : rhs.size();
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
                QString msg =
                    QString("错误：goto 失败，状态=%1, 归约到=%2，中止").arg(stTop).arg(L);
                pushStep(res.steps, step++, stack, input, QStringLiteral("error"), msg);
                res.errorPos = res.steps.size();
                break;
            }
            stack.push_back({to, L});
            ParseTreeNode* p = new ParseTreeNode;
            p->symbol        = L;
            p->children      = kids;
            nodeStk.push_back(p);
            res.root = p;
            QString kidsStr;
            for (int i = 0; i < kids.size(); ++i)
                kidsStr += (i ? "," : "") + (kids[i] ? kids[i]->symbol : QString());
            pushStep(res.semanticSteps,
                     step,
                     stack,
                     input,
                     QString("reduce %1 -> %2, children=[%3]")
                         .arg(L)
                         .arg(rhs.isEmpty() ? QString("#") : rhs.join(" "))
                         .arg(kidsStr),
                     QString());
            pushStep(res.steps,
                     step++,
                     stack,
                     input,
                     act,
                     QString("%1 -> %2").arg(L).arg(rhs.isEmpty() ? QString("#") : rhs.join(" ")));
            continue;
        }
        pushStep(res.steps,
                 step++,
                 stack,
                 input,
                 QStringLiteral("error"),
                 QString("错误：未知动作 '%1'，中止").arg(act));
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
    QString llow = L.trimmed().toLower();
    {
        auto             names = Config::identifierTokenNames();
        QVector<QString> lowers;
        for (auto s : names) lowers.push_back(s.trimmed().toLower());
        if (lowers.contains(llow))
        {
            if (semKids.size() == 1 && semKids[0])
                return semKids[0];
            SemanticASTNode* root = makeSemNode(L);
            for (auto c : semKids)
                if (c)
                    root->children.push_back(c);
            return root;
        }
    }
    if (roles.isEmpty())
    {
        if (semKids.size() == 1 && semKids[0])
        {
            return semKids[0];
        }
        SemanticASTNode* root = makeSemNode(L);
        for (auto c : semKids)
            if (c)
                root->children.push_back(c);
        return root;
    }
    // 若同一候选存在多个 root 标注，视为聚合节点：并列附加（列表型产生式）
    int rootCount = 0;
    for (int i = 0; i < roles.size(); ++i)
        if (roleMeaning.value(roles[i]) == "root")
            rootCount++;
    if (rootCount > 1)
    {
        SemanticASTNode* root = makeSemNode(L);
        QVector<int>     idxs;
        for (int i = 0; i < roles.size(); ++i)
        {
            auto m = roleMeaning.value(roles[i]);
            if (m == "root" || m == "child" || m == "sibling")
                idxs.push_back(i);
        }
        for (int i : idxs)
        {
            if (i < semKids.size() && semKids[i])
                root->children.push_back(semKids[i]);
        }
        return root;
    }
    int          rootIdx = -1;
    QVector<int> rootIdxs;
    for (int i = 0; i < roles.size(); ++i)
    {
        auto m = roleMeaning.value(roles[i]);
        if (m == "root")
        {
            rootIdxs.push_back(i);
            if (rootIdx < 0)
                rootIdx = i;
            else if (rootPolicy == "last_1")
                rootIdx = i;
        }
    }
    QVector<int> childIdx;
    QVector<int> siblingIdx;
    for (int i = 0; i < roles.size(); ++i)
    {
        auto m = roleMeaning.value(roles[i]);
        if (m == "child")
            childIdx.push_back(i);
        else if (m == "sibling")
            siblingIdx.push_back(i);
    }
    SemanticASTNode* root = nullptr;
    if (rootIdx >= 0 && rootIdx < semKids.size() && semKids[rootIdx])
    {
        // 提升已有子树为根，保留其完整子结构
        root = semKids[rootIdx];
    }
    else
    {
        root = makeSemNode(L);
    }
    if (childOrder == "rhs_order")
    {
        for (int idx : childIdx)
        {
            if (idx < semKids.size() && semKids[idx] && idx != rootIdx)
                root->children.push_back(semKids[idx]);
        }
    }
    else
    {
        for (int i = childIdx.size() - 1; i >= 0; --i)
        {
            int idx = childIdx[i];
            if (idx < semKids.size() && semKids[idx] && idx != rootIdx)
                root->children.push_back(semKids[idx]);
        }
    }
    // 追加其它 root 标记的项为子节点
    for (int i = 0; i < rootIdxs.size(); ++i)
    {
        int idx = rootIdxs[i];
        if (idx == rootIdx)
            continue;
        if (idx < semKids.size() && semKids[idx])
            root->children.push_back(semKids[idx]);
    }
    // sibling 附加
    for (int idx : siblingIdx)
    {
        if (idx < semKids.size() && semKids[idx] && idx != rootIdx)
            root->children.push_back(semKids[idx]);
    }
    // 不再附加未标注项，避免语法树膨胀
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
            auto parts  = act.split('|');
            bool hasAcc = false;
            for (auto p : parts)
                if (p == QStringLiteral("acc"))
                {
                    hasAcc = true;
                    break;
                }
            if (hasAcc)
            {
                act = QStringLiteral("acc");
            }
            else
            {
                QString policy  = Config::lr1ConflictPolicy().trimmed().toLower();
                auto    prefer  = Config::lr1PreferShiftTokens();
                QString nextTok = a;
                if (!prefer.isEmpty())
                {
                    for (const auto& pt : prefer)
                    {
                        if (nextTok == pt)
                        {
                            QString pick;
                            for (auto p : parts)
                                if (p.startsWith("s"))
                                {
                                    pick = p;
                                    break;
                                }
                            if (!pick.isEmpty())
                            {
                                act = pick;
                                break;
                            }
                        }
                    }
                }
                if (act.isEmpty())
                {
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
                        QString msg =
                            QString("错误：状态=%1, 前瞻=%2, 动作冲突未配置，中止").arg(st).arg(a);
                        pushStep(res.steps, step++, stack, input, QStringLiteral("error"), msg);
                        pushStep(
                            res.semanticSteps, step, stack, input, QStringLiteral("error"), msg);
                        res.errorPos = res.steps.size();
                        break;
                    }
                }
            }
        }
        if (act.isEmpty())
        {
            if (!res.steps.isEmpty() && res.steps.back().action.startsWith("s") && input.size() > 1)
            {
                QString a2   = input[1];
                QString act2 = actionFor(t, st, a2);
                if (!act2.isEmpty())
                    act = act2;
            }
            if (act.isEmpty())
            {
                QString msg = QString("错误：状态=%1, 前瞻=%2, 无可用动作，中止").arg(st).arg(a);
                pushStep(res.steps, step++, stack, input, QStringLiteral("error"), msg);
                pushStep(res.semanticSteps, step, stack, input, QStringLiteral("error"), msg);
                res.errorPos = res.steps.size();
                break;
            }
        }
        if (act == "acc")
        {
            pushStep(res.steps, step++, stack, input, act, QString());
            if (!nodeStk.isEmpty())
                res.root = nodeStk.back();
            if (!semStk.isEmpty())
                res.astRoot = semStk.back();
            // 顶层包装为起始非终结符名称
            if (res.astRoot)
            {
                SemanticASTNode* top = new SemanticASTNode();
                top->tag             = g.startSymbol;
                top->children.push_back(res.astRoot);
                res.astRoot = top;
            }
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
            // 语义：移进叶子占位；默认 tag=a
            semStk.push_back(makeSemNode(a));
            pushStep(res.steps, step++, stack, input, act, QString());
            pushStep(res.semanticSteps,
                     step,
                     stack,
                     input,
                     QString("移进符号[%1]，语义栈压入终结符节点").arg(a),
                     QString());
            input.pop_front();
            continue;
        }
        if (act.startsWith("r"))
        {
            QString          L;
            QVector<QString> rhs;
            if (!parseReduction(t, act, L, rhs))
            {
                QString msg = QString("错误：归约解析失败，动作=%1，中止").arg(act);
                pushStep(res.steps, step++, stack, input, QStringLiteral("error"), msg);
                pushStep(res.semanticSteps, step, stack, input, QStringLiteral("error"), msg);
                res.errorPos = res.steps.size();
                break;
            }
            int                       k = rhs.isEmpty() ? 0 : rhs.size();
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
                QString msg =
                    QString("错误：goto 失败，状态=%1, 归约到=%2，中止").arg(stTop).arg(L);
                pushStep(res.steps, step++, stack, input, QStringLiteral("error"), msg);
                pushStep(res.semanticSteps, step, stack, input, QStringLiteral("error"), msg);
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
            int rid     = reductionIdFor(t, L, rhs);
            pushStep(res.semanticSteps,
                     step,
                     stack,
                     input,
                     QString("准备归约：产生式 %1 → %2，执行语义动作构建非终结符节点 [%3]%4")
                         .arg(rid >= 0 ? QString::number(rid) : QStringLiteral("?"))
                         .arg(rhs.isEmpty() ? QString("#") : rhs.join(" "))
                         .arg(sem ? sem->tag : L)
                         .arg(rid >= 0 ? QString("（编码:%1）").arg(rid) : QString()),
                     QString());
            pushStep(res.steps,
                     step++,
                     stack,
                     input,
                     act,
                     QString("%1 -> %2").arg(L).arg(rhs.isEmpty() ? QString("#") : rhs.join(" ")));
            continue;
        }
        pushStep(res.steps,
                 step++,
                 stack,
                 input,
                 QStringLiteral("error"),
                 QString("错误：未知动作 '%1'，中止").arg(act));
        pushStep(res.semanticSteps,
                 step,
                 stack,
                 input,
                 QStringLiteral("error"),
                 QString("错误：未知动作 '%1'，中止").arg(act));
        res.errorPos = res.steps.size();
        break;
    }
    return res;
}

ParseResult LR1Parser::parseWithSemantics(const QVector<QString>&                     tokens,
                                          const Grammar&                              g,
                                          const LR1ActionTable&                       t,
                                          const QMap<QString, QVector<QVector<int>>>& actions,
                                          const QMap<int, QString>&                   roleMeaning,
                                          const QString&                              rootPolicy,
                                          const QString&                              childOrder,
                                          const QVector<QString>&                     lexemes)
{
    ParseResult      res;
    QVector<QString> input = tokens;
    input.push_back("$");
    QVector<QPair<int, QString>> stack;
    QVector<ParseTreeNode*>      nodeStk;
    QVector<SemanticASTNode*>    semStk;
    stack.push_back({0, QString()});
    int           step = 0;
    int           ip   = 0;  // 输入位置索引
    QSet<QString> idNames;
    for (auto s : Config::identifierTokenNames()) idNames.insert(s.trimmed().toLower());
    while (!input.isEmpty())
    {
        QString a   = input[0];
        int     st  = stack.isEmpty() ? -1 : stack.back().first;
        QString act = actionFor(t, st, a);
        if (act.contains('|'))
        {
            auto parts  = act.split('|');
            bool hasAcc = false;
            for (auto p : parts)
                if (p == QStringLiteral("acc"))
                {
                    hasAcc = true;
                    break;
                }
            if (hasAcc)
            {
                act = QStringLiteral("acc");
            }
            else
            {
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
                    act = parts[0];
                }
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
            if (res.astRoot)
            {
                SemanticASTNode* top = new SemanticASTNode();
                top->tag             = g.startSymbol;
                top->children.push_back(res.astRoot);
                res.astRoot = top;
            }
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
            // 语义：对携带词素的终结符使用“token(lexeme)”作为叶子标签
            QString tag  = a;
            QString mlow = a.trimmed().toLower();
            QString shownLex;
            if (idNames.contains(mlow))
            {
                if (ip >= lexemes.size() || lexemes[ip].trimmed().isEmpty())
                {
                    QString msg = QString("缺少词素: [%1] 需要紧随词素").arg(a);
                    pushStep(res.semanticSteps, step, stack, input, QStringLiteral("error"), msg);
                    pushStep(res.steps, step++, stack, input, QStringLiteral("error"), msg);
                    res.errorPos = res.steps.size();
                    break;
                }
                QString lx = lexemes[ip].trimmed();
                tag        = QString("%1(%2)").arg(a).arg(lx);
                shownLex   = lx;
                ip++;
            }
            semStk.push_back(makeSemNode(tag));
            // 语义过程记录：移进叶子（可能替换为词素）
            pushStep(
                res.semanticSteps,
                step,
                stack,
                input,
                QString("移进符号[%1]%2，语义栈压入终结符节点")
                    .arg(a)
                    .arg(!shownLex.isEmpty() ? QString("（lexeme=%1）").arg(shownLex) : QString()),
                QString());
            pushStep(res.steps, step++, stack, input, act, QString());
            input.pop_front();
            continue;
        }
        if (act.startsWith("r"))
        {
            QString          L;
            QVector<QString> rhs;
            if (!parseReduction(t, act, L, rhs))
            {
                res.errorPos = res.steps.size();
                break;
            }
            int                       k = rhs.isEmpty() ? 0 : rhs.size();
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
            QVector<int> roles;
            if (actions.contains(L))
            {
                const auto& vec  = actions.value(L);
                int         pick = -1;
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
            // 语义过程记录：构建语义节点，根与孩子
            QString kidsStr;
            for (int i = 0; i < semKids.size(); ++i)
                kidsStr += (i ? "," : "") + (semKids[i] ? semKids[i]->tag : QString());
            int rid2 = reductionIdFor(t, L, rhs);
            pushStep(
                res.semanticSteps,
                step,
                stack,
                input,
                QString(
                    "准备归约：产生式 %1 → %2，执行语义动作构建非终结符节点 [%3]%4，子节点=[%5]")
                    .arg(rid2 >= 0 ? QString::number(rid2) : QStringLiteral("?"))
                    .arg(rhs.isEmpty() ? QString("#") : rhs.join(" "))
                    .arg(sem ? sem->tag : L)
                    .arg(rid2 >= 0 ? QString("（编码:%1）").arg(rid2) : QString())
                    .arg(kidsStr),
                QString());
            pushStep(res.steps,
                     step++,
                     stack,
                     input,
                     act,
                     QString("%1 -> %2").arg(L).arg(rhs.isEmpty() ? QString("#") : rhs.join(" ")));
            continue;
        }
        res.errorPos = res.steps.size();
        break;
    }
    return res;
}
