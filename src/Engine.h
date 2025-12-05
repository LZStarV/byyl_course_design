#pragma once
#include <QString>
#include <QVector>
#include <QMap>
#include "regex/RegexLexer.h"
#include "regex/RegexParser.h"
#include "automata/Thompson.h"
#include "automata/SubsetConstruction.h"
#include "automata/Hopcroft.h"
#include "generator/CodeGenerator.h"
#include "syntax/Grammar.h"
#include "syntax/LL1.h"
/**
 * \brief 状态表结构
 *
 * 用于在界面展示 NFA/DFA/MinDFA 的转移与标记。
 */
struct Tables
{
    QVector<QString>          columns;
    QVector<QString>          marks;
    QVector<QString>          states;
    QVector<QVector<QString>> rows;
};
/**
 * \brief 词法处理引擎
 *
 * 负责从规则文本到最小化自动机与代码生成的完整流水线：
 * 词法 -> 解析 -> AST -> NFA -> DFA -> MinDFA -> 代码生成与运行。
 */
class Engine
{
   public:
    /** \brief 词法阶段：解析文本为规则文件结构 */
    RegexFile lexFile(const QString& text);
    /** \brief 语法阶段：将规则文件解析为 AST 与 Token 列表 */
    ParsedFile parseFile(const RegexFile& f);
    /** \brief 构建 NFA */
    NFA buildNFA(ASTNode* ast, const Alphabet& alpha);
    /** \brief 构建 DFA */
    DFA buildDFA(const NFA& nfa);
    /** \brief 最小化 DFA */
    MinDFA buildMinDFA(const DFA& dfa);
    /** \brief 生成 NFA 状态表用于展示 */
    Tables nfaTable(const NFA& nfa);
    /** \brief 生成 DFA 状态表用于展示 */
    Tables dfaTable(const DFA& dfa);
    /** \brief 生成 MinDFA 状态表用于展示 */
    Tables minTable(const MinDFA& dfa);
    /** \brief 生成单词法扫描器源码 */
    QString generateCode(const MinDFA& mdfa, const QMap<QString, int>& tokenCodes);
    /** \brief 运行单词法，返回编码结果 */
    QString run(const MinDFA& mdfa, const QString& source, int tokenCode);
    /** \brief 为多个 Token 构建各自的 MinDFA */
    QVector<MinDFA> buildAllMinDFA(const ParsedFile& pf, QVector<int>& codes);
    /** \brief 运行组合词法，返回编码序列 */
    QString                               runMultiple(const QVector<MinDFA>& mdfas,
                                                      const QVector<int>&    codes,
                                                      const QString&         source,
                                                      const QSet<int>&       identifierCodes);
    Grammar                               parseGrammarText(const QString& text, QString& error);
    LL1Info                               computeLL1(const Grammar& g);
    QMap<QString, QVector<QString>>       firstFollowAsRows(const LL1Info& info);
    QMap<QString, QMap<QString, QString>> parsingTableAsRows(const Grammar& g, const LL1Info& info);
};
