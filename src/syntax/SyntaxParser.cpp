#include "SyntaxParser.h"

static SyntaxASTNode* makeNode(const QString& sym)
{
    auto n = new SyntaxASTNode();
    n->symbol = sym;
    return n;
}

static bool isTerminal(const QSet<QString>& terms, const QString& s)
{
    return terms.contains(s) || s == "$";
}

SyntaxResult parseTokens(const QVector<QString>& tokens,
                         const Grammar&         g,
                         const LL1Info&         info)
{
    SyntaxResult r;
    QVector<QString> input = tokens;
    input.push_back("$");
    QVector<QString> st;
    st.push_back("$");
    st.push_back(g.startSymbol);
    QVector<SyntaxASTNode*> nodes;
    auto root = makeNode(g.startSymbol);
    nodes.push_back(makeNode("$"));
    nodes.push_back(root);
    int ip = 0;
    while (!st.isEmpty())
    {
        auto X = st.back();
        auto N = nodes.back();
        st.pop_back();
        nodes.pop_back();
        QString a = ip < input.size() ? input[ip] : "$";
        if (isTerminal(g.terminals, X) || X == "$")
        {
            if (X == a)
                ip++;
            else
            {
                r.errorPos = ip;
                break;
            }
        }
        else
        {
            int idx = info.table.value(X).value(a, -1);
            if (idx < 0)
            {
                r.errorPos = ip;
                break;
            }
            QVector<QString> rhs = g.productions[X][idx].right;
            if (rhs.size() == 1 && rhs[0] == "#")
            {
            }
            else
            {
                for (int i = rhs.size() - 1; i >= 0; --i)
                {
                    st.push_back(rhs[i]);
                    auto c = makeNode(rhs[i]);
                    N->children.push_back(c);
                    nodes.push_back(c);
                }
            }
        }
    }
    if (!root) root = makeNode(g.startSymbol);
    r.root = root;
    return r;
}
