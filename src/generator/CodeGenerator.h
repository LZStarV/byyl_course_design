#pragma once
#include "../model/Automata.h"
#include <QString>
/**
 * \brief 代码生成器
 *
 * 基于最小化 DFA 与 Token 编码生成可编译的 C++ 扫描器源码，
 * 支持单一词法与多个词法的组合生成。
 */
class CodeGenerator
{
   public:
    /**
     * \brief 生成单一词法扫描器源码
     * \param mdfa 最小化 DFA
     * \param tokenCodes Token 名到编码的映射
     * \return 可编译的 C++ 源码字符串
     */
    static QString generate(const MinDFA& mdfa, const QMap<QString, int>& tokenCodes);
    /**
     * \brief 生成组合扫描器源码
     * \param mdfas 多个最小化 DFA 列表
     * \param codes 对应的 Token 编码列表
     * \param alpha 统一字母表
     * \return 可编译的 C++ 源码字符串
     */
    static QString generateCombined(const QVector<MinDFA>& mdfas,
                                    const QVector<int>&    codes,
                                    const Alphabet&        alpha);
};
