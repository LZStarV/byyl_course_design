#include <QtTest>
#include "../../src/config/Config.h"

/**
 * \brief 生成目录配置测试
 *
 * 验证默认生成目录与环境变量覆盖效果，确保路径解析正确。
 */
class GenDirConfigTest : public QObject
{
    Q_OBJECT
   private slots:
    void test_gen_dir_env_override()
    {
        qunsetenv("BYYL_GEN_DIR");
        auto dir_default = Config::generatedOutputDir();
        QVERIFY(!dir_default.isEmpty());
        qputenv("BYYL_GEN_DIR", QByteArray("/tmp/byyl_gen"));
        auto dir_env = Config::generatedOutputDir();
        QVERIFY(!dir_env.isEmpty());
        QFileInfo fi(dir_env);
        auto      canon = fi.canonicalFilePath();
        QVERIFY(canon.contains("/tmp/byyl_gen"));
        qunsetenv("BYYL_GEN_DIR");
    }
};

QTEST_MAIN(GenDirConfigTest)
#include "gen_dir_config_test.moc"
