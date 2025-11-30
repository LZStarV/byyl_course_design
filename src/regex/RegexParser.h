#pragma once
#include <QString>
#include <QMap>
#include <QVector>
#include "RegexLexer.h"
#include "../model/Alphabet.h"
/**
 * \brief 正则表达式抽象语法树（AST）节点
 */
struct ASTNode
{
    enum Type
    {
        Concat,
        Union,
        Star,
        Plus,
        Question,
        CharSet,
        Symbol,
        Ref
    } type;
    QString           value;
    QVector<ASTNode*> children;
};
/**
 * \brief 解析后的 Token，包含词法规则与对应 AST 根节点
 */
struct ParsedToken
{
    Rule     rule;
    ASTNode* ast;
};
/**
 * \brief 解析后的词法文件结构
 */
struct ParsedFile
{
    QMap<QString, Rule>  macros;
    QVector<ParsedToken> tokens;
    Alphabet             alpha;
};
/**
 * \brief 正则解析器
 *
 * 将词法文件中的表达式转化为 AST，并构建字母表与 Token 集。
 */
class RegexParser
{
   public:
    /**
     * \brief 解析词法文件为 AST 与 Token 列表
     * \param file 词法文件结构
     * \return 解析结果，包含宏、Token 与字母表
     */
    static ParsedFile parse(const RegexFile& file);
};
