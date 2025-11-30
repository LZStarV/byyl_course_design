#pragma once
#include "../model/Automata.h"
/**
 * \brief 子集构造算法
 *
 * 将 NFA 通过子集构造法转化为等价的 DFA。
 */
class SubsetConstruction
{
   public:
    /**
     * \brief 构建 DFA
     * \param nfa 源 NFA
     * \return 等价的确定性有限自动机
     */
    static DFA build(const NFA& nfa);
};
