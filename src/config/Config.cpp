#include "Config.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>

bool                        Config::s_loaded = false;
QVector<Config::WeightTier> Config::s_tiers;
QString                     Config::s_outDir;
QString                     Config::s_syntaxDir;
QString                     Config::s_graphsDir;
QString                     Config::s_macroLetter;
QString                     Config::s_macroDigit;
QVector<QChar>              Config::s_ws;
bool                        Config::s_tokenMapUseHeuristics = true;
QString                     Config::s_graphvizExe;
int                         Config::s_graphvizDpi;
int                         Config::s_graphvizTimeout;
QString                     Config::s_epsilon;
QString                     Config::s_eof;
QString                     Config::s_augSuffix;
QString                     Config::s_lr1Policy;
QString                     Config::s_nontermPat;
QVector<QString>            Config::s_multiOps;
QVector<QString>            Config::s_singleOps;
QString                     Config::s_tblMark;
QString                     Config::s_tblStateId;
QString                     Config::s_tblStateSet;
QString                     Config::s_tblEpsilonCol;
QString                     Config::s_dotRankdir;
QString                     Config::s_dotNodeShape;
QString                     Config::s_dotEpsLabel;
QVector<QString>            Config::s_cfgSearchPaths;
bool                        Config::s_hasOutDirOverride = false;
QString                     Config::s_outDirOverride;
bool                        Config::s_hasTiersOverride = false;
QVector<Config::WeightTier> Config::s_tiersOverride;
bool                        Config::s_hasSkipBrace    = false;
bool                        Config::s_skipBrace       = false;
bool                        Config::s_hasSkipLine     = false;
bool                        Config::s_skipLine        = false;
bool                        Config::s_hasSkipBlock    = false;
bool                        Config::s_skipBlock       = false;
bool                        Config::s_hasSkipHash     = false;
bool                        Config::s_skipHash        = false;
bool                        Config::s_hasSkipSingle   = false;
bool                        Config::s_skipSingle      = false;
bool                        Config::s_hasSkipDouble   = false;
bool                        Config::s_skipDouble      = false;
bool                        Config::s_hasSkipTemplate = false;
bool                        Config::s_skipTemplate    = false;
QMap<int, QString>          Config::s_semRoleMeaning;
QString                     Config::s_semRootPolicy;
QString                     Config::s_semChildOrder;

static QVector<Config::WeightTier> defaultTiers()
{
    QVector<Config::WeightTier> v;
    v.push_back({220, 3});
    v.push_back({200, 4});
    v.push_back({100, 1});
    v.push_back({0, 0});
    return v;
}

void Config::load()
{
    if (s_loaded)
        return;
    s_loaded      = true;
    s_tiers       = defaultTiers();
    s_outDir      = QString();
    s_syntaxDir   = QString();
    s_graphsDir   = QString();
    s_macroLetter = QString("letter");
    s_macroDigit  = QString("digit");
    s_ws.clear();
    s_tokenMapUseHeuristics = true;
    s_graphvizExe           = QString("dot");
    s_graphvizDpi           = 150;
    s_graphvizTimeout       = 20000;
    s_epsilon               = QString("#");
    s_eof                   = QString("$");
    s_augSuffix             = QString("'");
    s_lr1Policy             = QString("prefer_shift");
    s_nontermPat.clear();
    s_multiOps.clear();
    s_singleOps.clear();
    s_tblMark          = QStringLiteral("标记");
    s_tblStateId       = QStringLiteral("状态 ID");
    s_tblStateSet      = QStringLiteral("状态集合");
    s_tblEpsilonCol    = QStringLiteral("#");
    s_dotRankdir       = QStringLiteral("LR");
    s_dotNodeShape     = QStringLiteral("circle");
    s_dotEpsLabel      = QStringLiteral("ε");
    s_cfgSearchPaths.clear();
    s_hasOutDirOverride = false;
    s_hasTiersOverride  = false;
    s_hasSkipBrace      = false;
    s_hasSkipLine       = false;
    s_hasSkipBlock      = false;
    s_hasSkipHash       = false;
    s_hasSkipSingle     = false;
    s_hasSkipDouble     = false;
    s_hasSkipTemplate   = false;

    // env override for output dir
    QByteArray genDirEnv = qgetenv("BYYL_GEN_DIR");
    if (!genDirEnv.isEmpty())
        s_outDir = QString::fromUtf8(genDirEnv);

    // read config file
    QString          appDir = QCoreApplication::applicationDirPath();
    QString          usePath;
    QVector<QString> candidates;
    for (const auto& p : s_cfgSearchPaths) candidates.push_back(p);
    candidates.push_back(appDir + "/../../config/lexer.json");
    candidates.push_back(appDir + "/config/lexer.json");
    for (const auto& c : candidates)
    {
        if (QFile::exists(c))
        {
            usePath = c;
            break;
        }
    }
    if (!usePath.isEmpty())
    {
        QFile f(usePath);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            auto data = f.readAll();
            f.close();
            auto doc = QJsonDocument::fromJson(data);
            if (doc.isObject())
            {
                auto obj = doc.object();
                if (obj.contains("generated_output_dir") && s_outDir.isEmpty())
                {
                    s_outDir = obj.value("generated_output_dir").toString();
                }
                if (obj.contains("syntax_output_dir"))
                    s_syntaxDir = obj.value("syntax_output_dir").toString();
                if (obj.contains("graphs_dir"))
                    s_graphsDir = obj.value("graphs_dir").toString();
                if (obj.contains("weight_tiers") && obj.value("weight_tiers").isArray())
                {
                    QVector<Config::WeightTier> tiers;
                    auto                        arr = obj.value("weight_tiers").toArray();
                    for (auto v : arr)
                    {
                        if (!v.isObject())
                            continue;
                        auto o   = v.toObject();
                        int  min = o.value("min_code").toInt();
                        int  w   = o.value("weight").toInt();
                        tiers.push_back({min, w});
                    }
                    if (!tiers.isEmpty())
                        s_tiers = tiers;
                }
                if (obj.contains("macro_names") && obj.value("macro_names").isObject())
                {
                    auto mo = obj.value("macro_names").toObject();
                    auto ml = mo.value("letter").toString();
                    auto md = mo.value("digit").toString();
                    if (!ml.trimmed().isEmpty())
                        s_macroLetter = ml.trimmed();
                    if (!md.trimmed().isEmpty())
                        s_macroDigit = md.trimmed();
                }
                if (obj.contains("whitespaces") && obj.value("whitespaces").isArray())
                {
                    s_ws.clear();
                    auto arr = obj.value("whitespaces").toArray();
                    for (auto v : arr)
                    {
                        QString s = v.toString();
                        if (s == "\\t")
                            s_ws.push_back('\t');
                        else if (s == "\\n")
                            s_ws.push_back('\n');
                        else if (s == "\\r")
                            s_ws.push_back('\r');
                        else if (!s.isEmpty())
                            s_ws.push_back(s[0]);
                    }
                }
                if (obj.contains("token_map") && obj.value("token_map").isObject())
                {
                    auto to = obj.value("token_map").toObject();
                    if (to.contains("use_heuristics"))
                        s_tokenMapUseHeuristics = to.value("use_heuristics").toBool(true);
                }
                if (obj.contains("graphviz") && obj.value("graphviz").isObject())
                {
                    auto go = obj.value("graphviz").toObject();
                    auto ex = go.value("executable").toString();
                    if (!ex.trimmed().isEmpty())
                        s_graphvizExe = ex.trimmed();
                    int dpi = go.value("default_dpi").toInt();
                    if (dpi > 0)
                        s_graphvizDpi = dpi;
                    int to = go.value("timeout_ms").toInt();
                    if (to > 0)
                        s_graphvizTimeout = to;
                }
                if (obj.contains("epsilon_symbol"))
                    s_epsilon = obj.value("epsilon_symbol").toString("#");
                if (obj.contains("eof_symbol"))
                    s_eof = obj.value("eof_symbol").toString("$");
                if (obj.contains("aug_suffix"))
                    s_augSuffix = obj.value("aug_suffix").toString("'");
                if (obj.contains("lr1_conflict_policy"))
                    s_lr1Policy = obj.value("lr1_conflict_policy").toString("prefer_shift");
                if (obj.contains("nonterminal_pattern"))
                    s_nontermPat = obj.value("nonterminal_pattern").toString();
                if (obj.contains("i18n") && obj.value("i18n").isObject())
                {
                    auto io         = obj.value("i18n").toObject();
                    s_tblMark       = io.value("table_mark").toString(s_tblMark);
                    s_tblStateId    = io.value("table_state_id").toString(s_tblStateId);
                    s_tblStateSet   = io.value("table_state_set").toString(s_tblStateSet);
                    s_tblEpsilonCol = io.value("epsilon_column_label").toString(s_tblEpsilonCol);
                }
                if (obj.contains("semantic_actions") && obj.value("semantic_actions").isObject())
                {
                    auto so = obj.value("semantic_actions").toObject();
                    s_semRoleMeaning.clear();
                    if (so.contains("role_meaning") && so.value("role_meaning").isObject())
                    {
                        auto rm = so.value("role_meaning").toObject();
                        for (auto k : rm.keys())
                        {
                            bool ok = false;
                            int  ki = k.toInt(&ok);
                            if (ok) s_semRoleMeaning[ki] = rm.value(k).toString();
                        }
                    }
                    s_semRootPolicy  = so.value("root_selection_policy").toString();
                    s_semChildOrder  = so.value("child_order_policy").toString();
                }
                if (obj.contains("dot") && obj.value("dot").isObject())
                {
                    auto d         = obj.value("dot").toObject();
                    s_dotRankdir   = d.value("rankdir").toString(s_dotRankdir);
                    s_dotNodeShape = d.value("node_shape").toString(s_dotNodeShape);
                    s_dotEpsLabel  = d.value("epsilon_label").toString(s_dotEpsLabel);
                }
                if (obj.contains("config_search_paths") &&
                    obj.value("config_search_paths").isArray())
                {
                    s_cfgSearchPaths.clear();
                    for (auto v : obj.value("config_search_paths").toArray())
                        s_cfgSearchPaths.push_back(v.toString());
                }
                if (obj.contains("grammar_tokens") && obj.value("grammar_tokens").isObject())
                {
                    auto gt = obj.value("grammar_tokens").toObject();
                    s_multiOps.clear();
                    s_singleOps.clear();
                    if (gt.contains("multi_ops") && gt.value("multi_ops").isArray())
                    {
                        for (auto v : gt.value("multi_ops").toArray())
                            s_multiOps.push_back(v.toString());
                    }
                    if (gt.contains("single_ops") && gt.value("single_ops").isArray())
                    {
                        for (auto v : gt.value("single_ops").toArray())
                            s_singleOps.push_back(v.toString());
                    }
                }
            }
        }
    }
    if (s_outDir.isEmpty())
        s_outDir = QCoreApplication::applicationDirPath() + "/../../generated/lex";
    if (s_syntaxDir.isEmpty())
        s_syntaxDir = s_outDir + "/syntax";
    if (s_graphsDir.isEmpty())
        s_graphsDir = s_outDir + "/graphs";
    if (s_ws.isEmpty())
        s_ws = QVector<QChar>({' ', '\t', '\n', '\r'});
    if (s_multiOps.isEmpty())
        s_multiOps = QVector<QString>({"<=", ">=", "<>", ":="});
    if (s_singleOps.isEmpty())
        s_singleOps =
            QVector<QString>({"(", ")", ";", "<", ">", "=", "+", "-", "*", "/", "%", "^"});
    if (s_semRoleMeaning.isEmpty())
    {
        s_semRoleMeaning[0] = "discard";
        s_semRoleMeaning[1] = "root";
        s_semRoleMeaning[2] = "child";
    }
    if (s_semRootPolicy.trimmed().isEmpty()) s_semRootPolicy = "first_1";
    if (s_semChildOrder.trimmed().isEmpty()) s_semChildOrder = "rhs_order";
}

void Config::reload()
{
    s_loaded = false;
    s_tiers.clear();
    s_outDir.clear();
    s_hasOutDirOverride = false;
    s_outDirOverride.clear();
    s_hasTiersOverride = false;
    s_tiersOverride.clear();
    clearSkipOverrides();
}

int Config::weightForCode(int c)
{
    load();
    if (s_hasTiersOverride && !s_tiersOverride.isEmpty())
    {
        for (const auto& t : s_tiersOverride)
        {
            if (c >= t.minCode)
                return t.weight;
        }
        return 0;
    }
    QByteArray wenv = qgetenv("LEXER_WEIGHTS");
    if (!wenv.isEmpty())
    {
        QVector<Config::WeightTier> tiers;
        int                         a = 0, b = 0;
        const char*                 p = wenv.constData();
        while (*p)
        {
            a = 0;
            b = 0;
            while (*p && *p >= '0' && *p <= '9')
            {
                a = a * 10 + (*p - '0');
                p++;
            }
            if (*p == ':')
            {
                p++;
                while (*p && *p >= '0' && *p <= '9')
                {
                    b = b * 10 + (*p - '0');
                    p++;
                }
            }
            tiers.push_back({a, b});
            if (*p == ',')
                p++;
            else
                while (*p && *p != ',') p++;
        }
        if (!tiers.isEmpty())
        {
            std::sort(tiers.begin(),
                      tiers.end(),
                      [](const WeightTier& x, const WeightTier& y)
                      { return x.minCode > y.minCode; });
            s_tiers = tiers;
        }
    }
    for (const auto& t : s_tiers)
    {
        if (c >= t.minCode)
            return t.weight;
    }
    return 0;
}

QVector<Config::WeightTier> Config::weightTiers()
{
    load();
    if (s_hasTiersOverride && !s_tiersOverride.isEmpty())
        return s_tiersOverride;
    return s_tiers;
}

QString Config::generatedOutputDir()
{
    load();
    if (s_hasOutDirOverride && !s_outDirOverride.isEmpty())
        s_outDir = s_outDirOverride;
    else
    {
        QByteArray genDirEnv = qgetenv("BYYL_GEN_DIR");
        if (!genDirEnv.isEmpty())
            s_outDir = QString::fromUtf8(genDirEnv);
    }
    QDir d(s_outDir);
    if (!d.exists())
        d.mkpath(".");
    return s_outDir;
}

QString Config::syntaxOutputDir()
{
    load();
    return s_syntaxDir.isEmpty() ? (generatedOutputDir() + "/syntax") : s_syntaxDir;
}

QString Config::graphsDir()
{
    load();
    return s_graphsDir.isEmpty() ? (generatedOutputDir() + "/graphs") : s_graphsDir;
}

QString Config::macroLetterName()
{
    load();
    return s_macroLetter;
}

QString Config::macroDigitName()
{
    load();
    return s_macroDigit;
}

QVector<QChar> Config::whitespaces()
{
    load();
    return s_ws;
}

bool Config::isWhitespace(QChar ch)
{
    load();
    for (auto c : s_ws)
        if (ch == c)
            return true;
    return false;
}

bool Config::tokenMapUseHeuristics()
{
    load();
    return s_tokenMapUseHeuristics;
}

QString Config::graphvizExecutable()
{
    load();
    return s_graphvizExe;
}

int Config::graphvizDefaultDpi()
{
    load();
    return s_graphvizDpi;
}

int Config::graphvizTimeoutMs()
{
    load();
    return s_graphvizTimeout;
}

QString Config::epsilonSymbol()
{
    load();
    return s_epsilon;
}

QString Config::eofSymbol()
{
    load();
    return s_eof;
}

QString Config::augSuffix()
{
    load();
    return s_augSuffix;
}

QString Config::lr1ConflictPolicy()
{
    load();
    return s_lr1Policy;
}

QString Config::nonterminalPattern()
{
    load();
    return s_nontermPat;
}

QVector<QString> Config::grammarMultiOps()
{
    load();
    return s_multiOps;
}

QVector<QString> Config::grammarSingleOps()
{
    load();
    return s_singleOps;
}


QString Config::tableMarkLabel()
{
    load();
    return s_tblMark;
}

QString Config::tableStateIdLabel()
{
    load();
    return s_tblStateId;
}

QString Config::tableStateSetLabel()
{
    load();
    return s_tblStateSet;
}

QString Config::epsilonColumnLabel()
{
    load();
    return s_tblEpsilonCol;
}

QString Config::dotRankdir()
{
    load();
    return s_dotRankdir;
}

QString Config::dotNodeShape()
{
    load();
    return s_dotNodeShape;
}

QString Config::dotEpsilonLabel()
{
    load();
    return s_dotEpsLabel;
}

QVector<QString> Config::configSearchPaths()
{
    load();
    return s_cfgSearchPaths;
}

bool Config::skipBraceComment()
{
    load();
    if (s_hasSkipBrace)
        return s_skipBrace;
    QByteArray env = qgetenv("LEXER_SKIP_BRACE_COMMENT");
    if (!env.isEmpty())
    {
        auto v = QString::fromUtf8(env).trimmed().toLower();
        if (v == "1" || v == "true" || v == "yes")
            return true;
        if (v == "0" || v == "false" || v == "no")
            return false;
    }
    QString appDir   = QCoreApplication::applicationDirPath();
    QString cfgPath1 = appDir + "/../../config/lexer.json";
    QString cfgPath2 = appDir + "/config/lexer.json";
    QString usePath;
    if (QFile::exists(cfgPath1))
        usePath = cfgPath1;
    else if (QFile::exists(cfgPath2))
        usePath = cfgPath2;
    if (!usePath.isEmpty())
    {
        QFile f(usePath);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            auto data = f.readAll();
            f.close();
            auto doc = QJsonDocument::fromJson(data);
            if (doc.isObject())
            {
                auto obj = doc.object();
                if (obj.contains("skip_brace_comment"))
                    return obj.value("skip_brace_comment").toBool(false);
            }
        }
    }
    return false;
}

static bool envFlag(const char* name, bool defv)
{
    QByteArray env = qgetenv(name);
    if (env.isEmpty())
        return defv;
    auto v = QString::fromUtf8(env).trimmed().toLower();
    if (v == "1" || v == "true" || v == "yes")
        return true;
    if (v == "0" || v == "false" || v == "no")
        return false;
    return defv;
}

bool Config::skipLineComment()
{
    load();
    if (s_hasSkipLine)
        return s_skipLine;
    if (envFlag("LEXER_SKIP_LINE_COMMENT", false))
        return true;
    QString appDir   = QCoreApplication::applicationDirPath();
    QString cfgPath1 = appDir + "/../../config/lexer.json";
    QString cfgPath2 = appDir + "/config/lexer.json";
    QString usePath;
    if (QFile::exists(cfgPath1))
        usePath = cfgPath1;
    else if (QFile::exists(cfgPath2))
        usePath = cfgPath2;
    if (!usePath.isEmpty())
    {
        QFile f(usePath);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            auto data = f.readAll();
            f.close();
            auto doc = QJsonDocument::fromJson(data);
            if (doc.isObject())
            {
                auto obj = doc.object();
                return obj.value("skip_line_comment").toBool(false);
            }
        }
    }
    return false;
}

bool Config::skipBlockComment()
{
    load();
    if (s_hasSkipBlock)
        return s_skipBlock;
    if (envFlag("LEXER_SKIP_BLOCK_COMMENT", false))
        return true;
    QString appDir   = QCoreApplication::applicationDirPath();
    QString cfgPath1 = appDir + "/../../config/lexer.json";
    QString cfgPath2 = appDir + "/config/lexer.json";
    QString usePath;
    if (QFile::exists(cfgPath1))
        usePath = cfgPath1;
    else if (QFile::exists(cfgPath2))
        usePath = cfgPath2;
    if (!usePath.isEmpty())
    {
        QFile f(usePath);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            auto data = f.readAll();
            f.close();
            auto doc = QJsonDocument::fromJson(data);
            if (doc.isObject())
            {
                auto obj = doc.object();
                return obj.value("skip_block_comment").toBool(false);
            }
        }
    }
    return false;
}

bool Config::skipSingleQuoteString()
{
    load();
    if (s_hasSkipSingle)
        return s_skipSingle;
    if (envFlag("LEXER_SKIP_SQ_STRING", false))
        return true;
    QString appDir   = QCoreApplication::applicationDirPath();
    QString cfgPath1 = appDir + "/../../config/lexer.json";
    QString cfgPath2 = appDir + "/config/lexer.json";
    QString usePath;
    if (QFile::exists(cfgPath1))
        usePath = cfgPath1;
    else if (QFile::exists(cfgPath2))
        usePath = cfgPath2;
    if (!usePath.isEmpty())
    {
        QFile f(usePath);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            auto data = f.readAll();
            f.close();
            auto doc = QJsonDocument::fromJson(data);
            if (doc.isObject())
            {
                auto obj = doc.object();
                return obj.value("skip_single_quote_string").toBool(false);
            }
        }
    }
    return false;
}

bool Config::skipDoubleQuoteString()
{
    load();
    if (s_hasSkipDouble)
        return s_skipDouble;
    if (envFlag("LEXER_SKIP_DQ_STRING", false))
        return true;
    QString appDir   = QCoreApplication::applicationDirPath();
    QString cfgPath1 = appDir + "/../../config/lexer.json";
    QString cfgPath2 = appDir + "/config/lexer.json";
    QString usePath;
    if (QFile::exists(cfgPath1))
        usePath = cfgPath1;
    else if (QFile::exists(cfgPath2))
        usePath = cfgPath2;
    if (!usePath.isEmpty())
    {
        QFile f(usePath);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            auto data = f.readAll();
            f.close();
            auto doc = QJsonDocument::fromJson(data);
            if (doc.isObject())
            {
                auto obj = doc.object();
                return obj.value("skip_double_quote_string").toBool(false);
            }
        }
    }
    return false;
}

bool Config::skipTemplateString()
{
    load();
    if (s_hasSkipTemplate)
        return s_skipTemplate;
    if (envFlag("LEXER_SKIP_TPL_STRING", false))
        return true;
    QString appDir   = QCoreApplication::applicationDirPath();
    QString cfgPath1 = appDir + "/../../config/lexer.json";
    QString cfgPath2 = appDir + "/config/lexer.json";
    QString usePath;
    if (QFile::exists(cfgPath1))
        usePath = cfgPath1;
    else if (QFile::exists(cfgPath2))
        usePath = cfgPath2;
    if (!usePath.isEmpty())
    {
        QFile f(usePath);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            auto data = f.readAll();
            f.close();
            auto doc = QJsonDocument::fromJson(data);
            if (doc.isObject())
            {
                auto obj = doc.object();
                return obj.value("skip_template_string").toBool(false);
            }
        }
    }
    return false;
}

bool Config::skipHashComment()
{
    load();
    if (s_hasSkipHash)
        return s_skipHash;
    if (envFlag("LEXER_SKIP_HASH_COMMENT", false))
        return true;
    QString appDir   = QCoreApplication::applicationDirPath();
    QString cfgPath1 = appDir + "/../../config/lexer.json";
    QString cfgPath2 = appDir + "/config/lexer.json";
    QString usePath;
    if (QFile::exists(cfgPath1))
        usePath = cfgPath1;
    else if (QFile::exists(cfgPath2))
        usePath = cfgPath2;
    if (!usePath.isEmpty())
    {
        QFile f(usePath);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            auto data = f.readAll();
            f.close();
            auto doc = QJsonDocument::fromJson(data);
            if (doc.isObject())
            {
                auto obj = doc.object();
                return obj.value("skip_hash_comment").toBool(false);
            }
        }
    }
    return false;
}

void Config::setGeneratedOutputDir(const QString& dir)
{
    load();
    s_hasOutDirOverride = true;
    s_outDirOverride    = dir;
}

void Config::clearGeneratedOutputDirOverride()
{
    s_hasOutDirOverride = false;
    s_outDirOverride.clear();
}

void Config::setWeightTiers(const QVector<WeightTier>& tiers)
{
    load();
    s_hasTiersOverride = true;
    s_tiersOverride    = tiers;
}

void Config::clearWeightTiersOverride()
{
    s_hasTiersOverride = false;
    s_tiersOverride.clear();
}

void Config::setSkipBrace(bool v)
{
    load();
    s_hasSkipBrace = true;
    s_skipBrace    = v;
}

void Config::setSkipLine(bool v)
{
    load();
    s_hasSkipLine = true;
    s_skipLine    = v;
}

void Config::setSkipBlock(bool v)
{
    load();
    s_hasSkipBlock = true;
    s_skipBlock    = v;
}

void Config::setSkipHash(bool v)
{
    load();
    s_hasSkipHash = true;
    s_skipHash    = v;
}

void Config::setSkipSingle(bool v)
{
    load();
    s_hasSkipSingle = true;
    s_skipSingle    = v;
}

void Config::setSkipDouble(bool v)
{
    load();
    s_hasSkipDouble = true;
    s_skipDouble    = v;
}

void Config::setSkipTemplate(bool v)
{
    load();
    s_hasSkipTemplate = true;
    s_skipTemplate    = v;
}

void Config::clearSkipOverrides()
{
    s_hasSkipBrace    = false;
    s_hasSkipLine     = false;
    s_hasSkipBlock    = false;
    s_hasSkipHash     = false;
    s_hasSkipSingle   = false;
    s_hasSkipDouble   = false;
    s_hasSkipTemplate = false;
}

bool Config::saveJson(const QString& path)
{
    load();
    QJsonObject obj;
    obj.insert("generated_output_dir",
               s_hasOutDirOverride && !s_outDirOverride.isEmpty() ? s_outDirOverride : s_outDir);
    QJsonArray  tiersArr;
    const auto& tiersUse =
        s_hasTiersOverride && !s_tiersOverride.isEmpty() ? s_tiersOverride : s_tiers;
    for (const auto& t : tiersUse)
    {
        QJsonObject o;
        o.insert("min_code", t.minCode);
        o.insert("weight", t.weight);
        tiersArr.append(o);
    }
    obj.insert("weight_tiers", tiersArr);
    obj.insert("skip_brace_comment", s_hasSkipBrace ? s_skipBrace : skipBraceComment());
    obj.insert("skip_line_comment", s_hasSkipLine ? s_skipLine : skipLineComment());
    obj.insert("skip_block_comment", s_hasSkipBlock ? s_skipBlock : skipBlockComment());
    obj.insert("skip_hash_comment", s_hasSkipHash ? s_skipHash : skipHashComment());
    obj.insert("skip_single_quote_string",
               s_hasSkipSingle ? s_skipSingle : skipSingleQuoteString());
    obj.insert("skip_double_quote_string",
               s_hasSkipDouble ? s_skipDouble : skipDoubleQuoteString());
    obj.insert("skip_template_string", s_hasSkipTemplate ? s_skipTemplate : skipTemplateString());
    QJsonDocument doc(obj);
    QFile         f(path);
    if (!QDir(QFileInfo(path).absolutePath()).exists())
        QDir().mkpath(QFileInfo(path).absolutePath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    f.write(doc.toJson(QJsonDocument::Compact));
    f.close();
    return true;
}
QMap<int, QString> Config::semanticRoleMeaning()
{
    load();
    return s_semRoleMeaning;
}

QString Config::semanticRootSelectionPolicy()
{
    load();
    return s_semRootPolicy;
}

QString Config::semanticChildOrderPolicy()
{
    load();
    return s_semChildOrder;
}
