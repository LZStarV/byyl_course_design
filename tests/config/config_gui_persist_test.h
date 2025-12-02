#pragma once
#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "../../src/config/Config.h"
class ConfigGuiPersistTest : public QObject
{
    Q_OBJECT
  private slots:
    void test_save_and_reload()
    {
        QTemporaryDir tmp;
        QString path = tmp.path() + "/lexer.json";
        QVector<Config::WeightTier> tiers;
        tiers.push_back({300, 9});
        tiers.push_back({100, 2});
        tiers.push_back({0, 0});
        Config::setGeneratedOutputDir(tmp.path() + "/gen");
        Config::setWeightTiers(tiers);
        Config::setSkipBrace(true);
        Config::setSkipLine(false);
        Config::setSkipBlock(false);
        Config::setSkipHash(false);
        Config::setSkipSingle(false);
        Config::setSkipDouble(false);
        Config::setSkipTemplate(false);
        QVERIFY(Config::saveJson(path));
        qunsetenv("BYYL_GEN_DIR");
        qunsetenv("LEXER_WEIGHTS");
        qunsetenv("LEXER_SKIP_BRACE_COMMENT");
        qunsetenv("LEXER_SKIP_LINE_COMMENT");
        qunsetenv("LEXER_SKIP_BLOCK_COMMENT");
        qunsetenv("LEXER_SKIP_HASH_COMMENT");
        qunsetenv("LEXER_SKIP_SQ_STRING");
        qunsetenv("LEXER_SKIP_DQ_STRING");
        qunsetenv("LEXER_SKIP_TPL_STRING");
        Config::clearGeneratedOutputDirOverride();
        Config::clearWeightTiersOverride();
        Config::clearSkipOverrides();
        Config::reload();
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly | QIODevice::Text));
        auto data = f.readAll();
        f.close();
        QFile cfg(QCoreApplication::applicationDirPath() + "/../../config/lexer.json");
        QFileInfo fi(cfg);
        bool ok = true;
        QJsonParseError err;
        auto doc = QJsonDocument::fromJson(data, &err);
        ok = ok && err.error == QJsonParseError::NoError && doc.isObject();
        QVERIFY(ok);
        auto obj = doc.object();
        auto arr = obj.value("weight_tiers").toArray();
        QVERIFY(arr.size() >= 1);
    }
};
