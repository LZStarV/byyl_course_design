#pragma once
#include <QString>
#include <QVector>

/**
 * \brief 运行时配置中心
 *
 * 提供权重分层、生成目录、以及代码扫描时的跳过规则配置。
 */
class Config
{
   public:
    /**
     * \brief 权重分层配置
     */
    struct WeightTier
    {
        int minCode;
        int weight;
    };

    /** \brief 加载配置（环境变量与默认值）*/
    static void load();
    /** \brief 获取指定编码的权重 */
    static int weightForCode(int c);
    /** \brief 词法生成输出目录路径 */
    static QString generatedOutputDir();
    /** \brief 是否跳过花括号注释块 */
    static bool skipBraceComment();
    /** \brief 是否跳过行注释（//）*/
    static bool                                           skipLineComment();
    /** \brief 是否跳过块注释（/* ... */）* / static bool skipBlockComment();
    /** \brief 是否跳过井号注释（#）*/
    static bool skipHashComment();
    /** \brief 是否跳过单引号字符串 */
    static bool skipSingleQuoteString();
    /** \brief 是否跳过双引号字符串 */
    static bool skipDoubleQuoteString();
    /** \brief 是否跳过模板字符串（例如 JS 的 `...`）*/
    static bool skipTemplateString();

   private:
    static bool                s_loaded;
    static QVector<WeightTier> s_tiers;
    static QString             s_outDir;
};
