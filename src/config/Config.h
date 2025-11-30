#pragma once
#include <QString>
#include <QVector>

class Config {
public:
    struct WeightTier { int minCode; int weight; };

    static void load();
    static int weightForCode(int c);
    static QString generatedOutputDir();

private:
    static bool s_loaded;
    static QVector<WeightTier> s_tiers;
    static QString s_outDir;
};

