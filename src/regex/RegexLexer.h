#pragma once
#include <QString>
#include <QVector>
#include <QMap>
/**
 * \brief 词法文件的基本记号与规则结构
 */
struct RegexToken
{
    QString kind;
    QString text;
};
/**
 * \brief 词法规则（宏/Token）
 */
struct Rule
{
    QString name;
    QString expr;
    bool    isToken = false;
    int     code    = 0;
    bool    isGroup = false;
};
/**
 * \brief 词法文件结构，包含宏与 Token 定义
 */
struct RegexFile
{
    QMap<QString, Rule> rules;
    QVector<Rule>       tokens;
};
/**
 * \brief 正则词法分析器
 *
 * 将文本形式的规则文件解析为结构化的宏与 Token 集合。
 */
class RegexLexer
{
   public:
    /**
     * \brief 解析正则词法文件
     * \param input 文件文本内容
     * \return 结构化的规则与 Token 集合
     */
    static RegexFile lex(const QString& input);
};
