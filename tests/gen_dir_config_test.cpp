#include <QtTest>
#include "../src/config/Config.h"

class GenDirConfigTest: public QObject {
    Q_OBJECT
private slots:
    void test_gen_dir_env_override(){
        qunsetenv("BYYL_GEN_DIR");
        auto dir_default = Config::generatedOutputDir();
        QVERIFY(!dir_default.isEmpty());
        // 环境变量覆盖
        qputenv("BYYL_GEN_DIR", QByteArray("/tmp/byyl_gen"));
        auto dir_env = Config::generatedOutputDir();
        QVERIFY(!dir_env.isEmpty());
        QFileInfo fi(dir_env);
        auto canon = fi.canonicalFilePath();
        QVERIFY(canon.contains("/tmp/byyl_gen"));
        qunsetenv("BYYL_GEN_DIR");
    }
};

QTEST_MAIN(GenDirConfigTest)
#include "gen_dir_config_test.moc"
