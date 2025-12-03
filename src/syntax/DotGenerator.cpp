#include "AST.h"
#include <QString>
#include <QTextStream>
#include <QFile>
#include <QMap>

static void emitNode(QTextStream&                     o,
                     const SyntaxASTNode*             n,
                     int&                             id,
                     QMap<const SyntaxASTNode*, int>& ids)
{
    int nid = ++id;
    ids.insert(n, nid);
    o << "  n" << nid << " [label=\"" << n->symbol << "\"]\n";
    for (auto c : n->children)
    {
        emitNode(o, c, id, ids);
        int cid = ids.value(c);
        o << "  n" << nid << " -> n" << cid << "\n";
    }
}

QString syntaxAstToDot(SyntaxASTNode* root)
{
    QString     s;
    QTextStream o(&s);
    o << "digraph G {\nrankdir=TB\n";
    int                             id = 0;
    QMap<const SyntaxASTNode*, int> ids;
    if (root)
        emitNode(o, root, id, ids);
    o << "}\n";
    return s;
}
