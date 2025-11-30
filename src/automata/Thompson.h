#pragma once
#include "../model/Automata.h"
#include "../regex/RegexParser.h"
/**
 * \brief Thompson 构造法
 *
 * 根据正则 AST 直接构造等价的 NFA。
 */
class Thompson
{
   public:
    /**
     * \brief 构建 NFA
     * \param ast 正则表达式 AST 根节点
     * \param alpha 字母表
     * \return 构造得到的 NFA
     */
    static NFA build(ASTNode* ast, Alphabet alpha);
};
