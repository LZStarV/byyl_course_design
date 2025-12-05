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
    static void reload();
    /** \brief 获取指定编码的权重 */
    static int                 weightForCode(int c);
    static QVector<WeightTier> weightTiers();
    /** \brief 词法生成输出目录路径 */
    static QString        generatedOutputDir();
    static QString        syntaxOutputDir();
    static QString        graphsDir();
    static QString        macroLetterName();
    static QString        macroDigitName();
    static bool           isWhitespace(QChar ch);
    static QVector<QChar> whitespaces();
    static bool           tokenMapUseHeuristics();
    static QString        graphvizExecutable();
    static int            graphvizDefaultDpi();
    static int            graphvizTimeoutMs();
    // semantic actions config (externalized, no hardcoding)
    static QMap<int, QString> semanticRoleMeaning();
    static QString            semanticRootSelectionPolicy();
    static QString            semanticChildOrderPolicy();
    // syntax config
    static QString          epsilonSymbol();
    static QString          eofSymbol();
    static QString          augSuffix();
    static QString          lr1ConflictPolicy();
    static QString          nonterminalPattern();
    static QVector<QString> grammarMultiOps();
    static QVector<QString> grammarSingleOps();
    static bool             emitIdentifierLexeme();
    static QVector<QString> identifierTokenNames();
    static void             setEmitIdentifierLexeme(bool v);
    static void             setIdentifierTokenNames(const QVector<QString>& names);
    // i18n for tables
    static QString tableMarkLabel();
    static QString tableStateIdLabel();
    static QString tableStateSetLabel();
    static QString epsilonColumnLabel();
    // dot style
    static QString dotRankdir();
    static QString dotNodeShape();
    static QString dotEpsilonLabel();
    // config search paths
    static QVector<QString> configSearchPaths();
    /** \brief 是否跳过花括号注释块 */
    static bool skipBraceComment();
    /** \brief 是否跳过行注释（//）*/
    static bool skipLineComment();
    /** \brief 是否跳过块注释（slash-star … star-slash）*/
    static bool skipBlockComment();
    /** \brief 是否跳过井号注释（#）*/
    static bool skipHashComment();
    /** \brief 是否跳过单引号字符串 */
    static bool skipSingleQuoteString();
    /** \brief 是否跳过双引号字符串 */
    static bool skipDoubleQuoteString();
    /** \brief 是否跳过模板字符串（例如 JS 的 `...`）*/
    static bool skipTemplateString();

    static void setGeneratedOutputDir(const QString& dir);
    static void clearGeneratedOutputDirOverride();
    static void setWeightTiers(const QVector<WeightTier>& tiers);
    static void clearWeightTiersOverride();
    static void setSkipBrace(bool v);
    static void setSkipLine(bool v);
    static void setSkipBlock(bool v);
    static void setSkipHash(bool v);
    static void setSkipSingle(bool v);
    static void setSkipDouble(bool v);
    static void setSkipTemplate(bool v);
    static void clearSkipOverrides();
    static bool saveJson(const QString& path);
    // setters for settings dialog
    static void setSyntaxOutputDir(const QString& dir);
    static void setGraphsDir(const QString& dir);
    static void setMacroLetterName(const QString& name);
    static void setMacroDigitName(const QString& name);
    static void setWhitespaces(const QVector<QChar>& ws);
    static void setTokenMapUseHeuristics(bool v);
    static void setGraphvizExecutable(const QString& exe);
    static void setGraphvizDefaultDpi(int dpi);
    static void setGraphvizTimeoutMs(int ms);
    static void setEpsilonSymbol(const QString& s);
    static void setEofSymbol(const QString& s);
    static void setAugSuffix(const QString& s);
    static void setLr1ConflictPolicy(const QString& p);
    static void setNonterminalPattern(const QString& pat);
    static void setGrammarMultiOps(const QVector<QString>& ops);
    static void setGrammarSingleOps(const QVector<QString>& ops);
    static void setTableMarkLabel(const QString& s);
    static void setTableStateIdLabel(const QString& s);
    static void setTableStateSetLabel(const QString& s);
    static void setEpsilonColumnLabel(const QString& s);
    static void setDotRankdir(const QString& s);
    static void setDotNodeShape(const QString& s);
    static void setDotEpsilonLabel(const QString& s);
    static void setConfigSearchPaths(const QVector<QString>& paths);
    static void setSemanticRootSelectionPolicy(const QString& p);
    static void setSemanticChildOrderPolicy(const QString& p);

   private:
    static bool                s_loaded;
    static QVector<WeightTier> s_tiers;
    static QString             s_outDir;
    static QString             s_syntaxDir;
    static QString             s_graphsDir;
    static QString             s_macroLetter;
    static QString             s_macroDigit;
    static QVector<QChar>      s_ws;
    static bool                s_tokenMapUseHeuristics;
    static QString             s_graphvizExe;
    static int                 s_graphvizDpi;
    static int                 s_graphvizTimeout;
    static QString             s_epsilon;
    static QString             s_eof;
    static QString             s_augSuffix;
    static QString             s_lr1Policy;
    static QString             s_nontermPat;
    static QVector<QString>    s_multiOps;
    static QVector<QString>    s_singleOps;
    static QString             s_tblMark;
    static QString             s_tblStateId;
    static QString             s_tblStateSet;
    static QString             s_tblEpsilonCol;
    static QString             s_dotRankdir;
    static QString             s_dotNodeShape;
    static QString             s_dotEpsLabel;
    static QVector<QString>    s_cfgSearchPaths;
    static bool                s_emitIdentifierLexeme;
    static QVector<QString>    s_identifierNames;
    // semantics
    static QMap<int, QString>  s_semRoleMeaning;
    static QString             s_semRootPolicy;
    static QString             s_semChildOrder;
    static bool                s_hasOutDirOverride;
    static QString             s_outDirOverride;
    static bool                s_hasTiersOverride;
    static QVector<WeightTier> s_tiersOverride;
    static bool                s_hasSkipBrace;
    static bool                s_skipBrace;
    static bool                s_hasSkipLine;
    static bool                s_skipLine;
    static bool                s_hasSkipBlock;
    static bool                s_skipBlock;
    static bool                s_hasSkipHash;
    static bool                s_skipHash;
    static bool                s_hasSkipSingle;
    static bool                s_skipSingle;
    static bool                s_hasSkipDouble;
    static bool                s_skipDouble;
    static bool                s_hasSkipTemplate;
    static bool                s_skipTemplate;
};
