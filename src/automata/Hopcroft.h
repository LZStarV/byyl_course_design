#pragma once
#include "../model/Automata.h"
/**
 * \brief Hopcroft 最小化算法
 *
 * 对 DFA 进行状态划分与合并，得到最小化的 MinDFA。
 */
class Hopcroft
{
   public:
    /**
     * \brief 最小化 DFA
     * \param dfa 输入 DFA
     * \return 最小化后的 MinDFA
     */
    static MinDFA minimize(const DFA& dfa);
};
