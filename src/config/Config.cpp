#include "Config.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

bool Config::s_loaded = false;
QVector<Config::WeightTier> Config::s_tiers;
QString Config::s_outDir;

static QVector<Config::WeightTier> defaultTiers(){
    QVector<Config::WeightTier> v; v.push_back({220,3}); v.push_back({200,4}); v.push_back({100,1}); v.push_back({0,0}); return v;
}

void Config::load(){
    if(s_loaded) return;
    s_loaded = true;
    s_tiers = defaultTiers();
    s_outDir = QString();

    // env override for output dir
    QByteArray genDirEnv = qgetenv("BYYL_GEN_DIR");
    if(!genDirEnv.isEmpty()) s_outDir = QString::fromUtf8(genDirEnv);

    // read config file
    QString appDir = QCoreApplication::applicationDirPath();
    QString cfgPath1 = appDir + "/../../config/lexer.json";
    QString cfgPath2 = appDir + "/config/lexer.json";
    QString usePath;
    if(QFile::exists(cfgPath1)) usePath = cfgPath1; else if(QFile::exists(cfgPath2)) usePath = cfgPath2;
    if(!usePath.isEmpty()){
        QFile f(usePath);
        if(f.open(QIODevice::ReadOnly|QIODevice::Text)){
            auto data = f.readAll(); f.close();
            auto doc = QJsonDocument::fromJson(data);
            if(doc.isObject()){
                auto obj = doc.object();
                if(obj.contains("generated_output_dir") && s_outDir.isEmpty()){
                    s_outDir = obj.value("generated_output_dir").toString();
                }
                if(obj.contains("weight_tiers") && obj.value("weight_tiers").isArray()){
                    QVector<Config::WeightTier> tiers;
                    auto arr = obj.value("weight_tiers").toArray();
                    for(auto v : arr){
                        if(!v.isObject()) continue; auto o=v.toObject();
                        int min = o.value("min_code").toInt(); int w = o.value("weight").toInt();
                        tiers.push_back({min,w});
                    }
                    if(!tiers.isEmpty()) s_tiers = tiers;
                }
            }
        }
    }
    if(s_outDir.isEmpty()) s_outDir = QCoreApplication::applicationDirPath()+"/../../generated/lex";
}

int Config::weightForCode(int c){
    load();
    // env override for weight tiers (LEXER_WEIGHTS)
    QByteArray wenv = qgetenv("LEXER_WEIGHTS");
    if(!wenv.isEmpty()){
        QVector<Config::WeightTier> tiers; int a=0,b=0; const char* p=wenv.constData();
        while(*p){
            a=0; b=0; while(*p && *p>='0' && *p<='9'){ a=a*10+(*p-'0'); p++; }
            if(*p==':'){ p++; while(*p && *p>='0' && *p<='9'){ b=b*10+(*p-'0'); p++; } }
            tiers.push_back({a,b}); if(*p==',') p++; else while(*p && *p!=',') p++;
        }
        if(!tiers.isEmpty()){
            std::sort(tiers.begin(), tiers.end(), [](const WeightTier& x, const WeightTier& y){ return x.minCode>y.minCode; });
            s_tiers = tiers;
        }
    }
    for(const auto& t : s_tiers){ if(c>=t.minCode) return t.weight; }
    return 0;
}

QString Config::generatedOutputDir(){
    load();
    // env override can change at runtime; re-read if present
    QByteArray genDirEnv = qgetenv("BYYL_GEN_DIR");
    if(!genDirEnv.isEmpty()) s_outDir = QString::fromUtf8(genDirEnv);
    QDir d(s_outDir); if(!d.exists()) d.mkpath(".");
    return s_outDir;
}
