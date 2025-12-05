#include "AST.h"
#include "LR1Parser.h"
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

static void emitParseNode(QTextStream&                     o,
                          const ParseTreeNode*             n,
                          int&                             id,
                          QMap<const ParseTreeNode*, int>& ids)
{
    int nid = ++id;
    ids.insert(n, nid);
    o << "  n" << nid << " [label=\"" << n->symbol << "\"]\n";
    for (auto c : n->children)
    {
        emitParseNode(o, c, id, ids);
        int cid = ids.value(c);
        o << "  n" << nid << " -> n" << cid << "\n";
    }
}

QString parseTreeToDot(ParseTreeNode* root)
{
    QString     s;
    QTextStream o(&s);
    o << "digraph G {\nrankdir=TB\n";
    int                             id = 0;
    QMap<const ParseTreeNode*, int> ids;
    if (root)
        emitParseNode(o, root, id, ids);
    o << "}\n";
    return s;
}

static void emitParseNodeWithLeafTokens(QTextStream&                     o,
                                        const ParseTreeNode*             n,
                                        int&                             id,
                                        QMap<const ParseTreeNode*, int>& ids,
                                        const QVector<QString>&          tokens,
                                        int&                             leafIdx)
{
    int nid = ++id;
    ids.insert(n, nid);
    bool    isLeaf = n->children.isEmpty();
    int     start  = leafIdx;
    QString label;
    if (isLeaf)
    {
        QString tok = (leafIdx < tokens.size()) ? tokens[leafIdx] : QString();
        label       = tok.isEmpty() ? n->symbol : QString("%1/%2").arg(n->symbol).arg(tok);
        if (leafIdx < tokens.size())
            leafIdx++;
    }
    else
    {
        for (auto c : n->children) emitParseNodeWithLeafTokens(o, c, id, ids, tokens, leafIdx);
        int     end = qMax(start, leafIdx - 1);
        QString preview;
        if (!tokens.isEmpty() && start >= 0 && start < tokens.size())
        {
            int upto = qMin(end, tokens.size() - 1);
            int cnt  = qMin(upto - start + 1, 6);
            for (int i = 0; i < cnt; ++i)
            {
                if (i)
                    preview += " ";
                preview += tokens[start + i];
            }
            if (upto - start + 1 > cnt)
                preview += " ...";
        }
        if (!preview.isEmpty())
            label = QString("%1 [%2..%3]: %4").arg(n->symbol).arg(start).arg(end).arg(preview);
        else
            label = n->symbol;
    }
    o << "  n" << nid << " [label=\"" << label << "\"]\n";
    for (auto c : n->children)
    {
        int cid = ids.value(c);
        o << "  n" << nid << " -> n" << cid << "\n";
    }
}

QString parseTreeToDotWithTokens(ParseTreeNode* root, const QVector<QString>& tokens)
{
    QString     s;
    QTextStream o(&s);
    o << "digraph G {\nrankdir=TB\n";
    if (!tokens.isEmpty())
    {
        QString preview;
        int     previewN = qMin(tokens.size(), 8);
        for (int i = 0; i < previewN; ++i)
        {
            if (i)
                preview += " ";
            preview += tokens[i];
        }
        o << "label=\"Tokens: " << preview << (tokens.size() > previewN ? " ..." : "")
          << "\"\nlabelloc=t\nfontsize=12\n";
    }
    int                             id = 0;
    QMap<const ParseTreeNode*, int> ids;
    int                             leafIdx = 0;
    if (root)
        emitParseNodeWithLeafTokens(o, root, id, ids, tokens, leafIdx);
    o << "}\n";
    return s;
}

static void emitSemanticNode(QTextStream&                       o,
                             const SemanticASTNode*             n,
                             int&                               id,
                             QMap<const SemanticASTNode*, int>& ids)
{
    int nid = ++id;
    ids.insert(n, nid);
    o << "  n" << nid << " [label=\"" << n->tag << "\"]\n";
    for (auto c : n->children)
    {
        emitSemanticNode(o, c, id, ids);
        int cid = ids.value(c);
        o << "  n" << nid << " -> n" << cid << "\n";
    }
}

QString semanticAstToDot(SemanticASTNode* root)
{
    QString     s;
    QTextStream o(&s);
    o << "digraph G {\nrankdir=TB\n";
    int                               id = 0;
    QMap<const SemanticASTNode*, int> ids;
    if (root)
        emitSemanticNode(o, root, id, ids);
    o << "}\n";
    return s;
}
