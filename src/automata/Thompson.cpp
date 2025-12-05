#include "Thompson.h"
#include <QPair>
static QPair<int, int> newFrag(NFA& nfa)
{
    int      s = nfa.states.size() + 1;
    int      t = s + 1;
    NFAState ss;
    ss.id = s;
    NFAState tt;
    tt.id     = t;
    tt.accept = true;
    nfa.states.insert(s, ss);
    nfa.states.insert(t, tt);
    return {s, t};
}
static QPair<int, int> buildAtom(NFA& nfa, ASTNode* ast)
{
    auto    p = newFrag(nfa);
    NFAEdge e;
    e.to      = p.second;
    e.epsilon = false;
    e.symbol  = ast->value;
    nfa.states[p.first].edges.push_back(e);
    return p;
}
static QPair<int, int> buildFrom(NFA& nfa, ASTNode* ast)
{
    if (!ast)
        return {0, 0};
    // 处理单一符号：直接从起始到终止加一条边
    if (ast->type == ASTNode::Symbol)
    {
        return buildAtom(nfa, ast);
    }
    // 处理字符集：为集合中每个字符添加一条至终止的边
    if (ast->type == ASTNode::CharSet)
    {
        auto p = newFrag(nfa);
        for (auto ch : ast->value)
        {
            NFAEdge e;
            e.to      = p.second;
            e.epsilon = false;
            e.symbol  = QString(ch);
            nfa.states[p.first].edges.push_back(e);
        }
        return p;
    }
    // 引用应在解析阶段被展开，此处不再特殊处理
    // 连接：a 的终止通过 ε 边指向 b 的起始
    if (ast->type == ASTNode::Concat)
    {
        auto    a = buildFrom(nfa, ast->children[0]);
        auto    b = buildFrom(nfa, ast->children[1]);
        NFAEdge e;
        e.to      = b.first;
        e.epsilon = true;
        nfa.states[a.second].edges.push_back(e);
        nfa.states[a.second].accept = false;
        return {a.first, b.second};
    }
    // 并集：新增起止片段，起始 ε 到 a/b 起点，a/b 终点 ε 到新终点
    if (ast->type == ASTNode::Union)
    {
        auto    a = buildFrom(nfa, ast->children[0]);
        auto    b = buildFrom(nfa, ast->children[1]);
        auto    p = newFrag(nfa);
        NFAEdge e1;
        e1.epsilon = true;
        e1.to      = a.first;
        nfa.states[p.first].edges.push_back(e1);
        NFAEdge e2;
        e2.epsilon = true;
        e2.to      = b.first;
        nfa.states[p.first].edges.push_back(e2);
        NFAEdge e3;
        e3.epsilon = true;
        e3.to      = p.second;
        nfa.states[a.second].edges.push_back(e3);
        NFAEdge e4;
        e4.epsilon = true;
        e4.to      = p.second;
        nfa.states[b.second].edges.push_back(e4);
        nfa.states[a.second].accept = false;
        nfa.states[b.second].accept = false;
        return p;
    }
    // 闭包（*）：新增起止片段，起始 ε 到 a 起点与新终点，a 终点 ε 到新终点与 a 起点
    if (ast->type == ASTNode::Star)
    {
        auto    a = buildFrom(nfa, ast->children[0]);
        auto    p = newFrag(nfa);
        NFAEdge e1;
        e1.epsilon = true;
        e1.to      = a.first;
        nfa.states[p.first].edges.push_back(e1);
        NFAEdge e2;
        e2.epsilon = true;
        e2.to      = p.second;
        nfa.states[p.first].edges.push_back(e2);
        NFAEdge e3;
        e3.epsilon = true;
        e3.to      = p.second;
        nfa.states[a.second].edges.push_back(e3);
        NFAEdge e4;
        e4.epsilon = true;
        e4.to      = a.first;
        nfa.states[a.second].edges.push_back(e4);
        nfa.states[a.second].accept = false;
        return p;
    }
    // 正闭包（+）：至少一次 a，a 终点 ε 回到 a 起点，并 ε 到新终点
    if (ast->type == ASTNode::Plus)
    {
        auto    a = buildFrom(nfa, ast->children[0]);
        auto    p = newFrag(nfa);
        NFAEdge e1;
        e1.epsilon = true;
        e1.to      = a.first;
        nfa.states[p.first].edges.push_back(e1);
        NFAEdge e2;
        e2.epsilon = true;
        e2.to      = p.second;
        nfa.states[a.second].edges.push_back(e2);
        NFAEdge e3;
        e3.epsilon = true;
        e3.to      = a.first;
        nfa.states[a.second].edges.push_back(e3);
        nfa.states[a.second].accept = false;
        return {p.first, p.second};
    }
    // 可选（?）：新增起止片段，起始 ε 到 a 起点与新终点，a 终点 ε 到新终点
    if (ast->type == ASTNode::Question)
    {
        auto    a = buildFrom(nfa, ast->children[0]);
        auto    p = newFrag(nfa);
        NFAEdge e1;
        e1.epsilon = true;
        e1.to      = a.first;
        nfa.states[p.first].edges.push_back(e1);
        NFAEdge e2;
        e2.epsilon = true;
        e2.to      = p.second;
        nfa.states[p.first].edges.push_back(e2);
        NFAEdge e3;
        e3.epsilon = true;
        e3.to      = p.second;
        nfa.states[a.second].edges.push_back(e3);
        nfa.states[a.second].accept = false;
        return p;
    }
    return newFrag(nfa);
}
NFA Thompson::build(ASTNode* ast, Alphabet alpha)
{
    NFA nfa;
    nfa.alpha = alpha;
    auto p    = buildFrom(nfa, ast);
    nfa.start = p.first;
    for (auto it = nfa.states.begin(); it != nfa.states.end(); ++it)
    {
        it.value().accept = false;
    }
    nfa.states[p.second].accept = true;
    return nfa;
}
