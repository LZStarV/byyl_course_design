#include "Config.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>

bool                        Config::s_loaded            = false;
QVector<Config::WeightTier> Config::s_tiers;
QString                     Config::s_outDir;
bool                        Config::s_hasOutDirOverride = false;
QString                     Config::s_outDirOverride;
bool                        Config::s_hasTiersOverride  = false;
QVector<Config::WeightTier> Config::s_tiersOverride;
bool                        Config::s_hasSkipBrace      = false;
bool                        Config::s_skipBrace         = false;
bool                        Config::s_hasSkipLine       = false;
bool                        Config::s_skipLine          = false;
bool                        Config::s_hasSkipBlock      = false;
bool                        Config::s_skipBlock         = false;
bool                        Config::s_hasSkipHash       = false;
bool                        Config::s_skipHash          = false;
bool                        Config::s_hasSkipSingle     = false;
bool                        Config::s_skipSingle        = false;
bool                        Config::s_hasSkipDouble     = false;
bool                        Config::s_skipDouble        = false;
bool                        Config::s_hasSkipTemplate   = false;
bool                        Config::s_skipTemplate      = false;

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
    s_loaded = true;
    s_tiers  = defaultTiers();
    s_outDir = QString();
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
                if (obj.contains("generated_output_dir") && s_outDir.isEmpty())
                {
                    s_outDir = obj.value("generated_output_dir").toString();
                }
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
            }
        }
    }
    if (s_outDir.isEmpty())
        s_outDir = QCoreApplication::applicationDirPath() + "/../../generated/lex";
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
    if (s_hasTiersOverride && !s_tiersOverride.isEmpty()) return s_tiersOverride;
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
    obj.insert("generated_output_dir", s_hasOutDirOverride && !s_outDirOverride.isEmpty() ? s_outDirOverride : s_outDir);
    QJsonArray tiersArr;
    const auto& tiersUse = s_hasTiersOverride && !s_tiersOverride.isEmpty() ? s_tiersOverride : s_tiers;
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
    obj.insert("skip_single_quote_string", s_hasSkipSingle ? s_skipSingle : skipSingleQuoteString());
    obj.insert("skip_double_quote_string", s_hasSkipDouble ? s_skipDouble : skipDoubleQuoteString());
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
