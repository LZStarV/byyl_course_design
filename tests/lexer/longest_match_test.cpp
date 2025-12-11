/*
 * 版权信息：Copyright (c) 2023 林展星
 * 文件名称：longest_match_test.cpp
 *
 * 当前版本：1.0.0
 * 作    者：林展星
 * 完成日期：2023年12月07日
 *
 * 版本历史：
 * 1.0.0 2023-12-07 林展星 初始版本
 */
#include <QtTest/QtTest>
#include "../../src/Engine.h"
#include "../../src/syntax/TokenMapBuilder.h"

class LongestMatchTest : public QObject
{
    Q_OBJECT
   private:
    QString rules()
    {
        return QStringLiteral(
            "letter=[A-Za-z_]\n"
            "digit=[0-9]\n"
            "_identifier100=letter(letter|digit)*\n"
            "_keywords200S= if | else | read | write | then | end \n");
    }
   private slots:
    void longest_match_keywords_vs_identifier()
    {
        Engine eng;
        qInfo() << "输入规则:" << rules().trimmed();
        qInfo() << "关键步骤:" << "lexFile -> parseFile -> buildAllMinDFA -> runMultiple";
        auto         rf = eng.lexFile(rules());
        auto         pf = eng.parseFile(rf);
        QVector<int> codes;
        auto         mdfs = eng.buildAllMinDFA(pf, codes);
        auto         src  = QStringLiteral("if abc");
        auto         out  = eng.runMultiple(mdfs, codes, src, QSet<int>());
        qInfo() << "源文本:" << src;
        qInfo() << "输出编码序列:" << out;
        auto toks = out.split(' ', Qt::SkipEmptyParts);
        QVERIFY(!toks.isEmpty());
        // 第一个 token 应为关键字编码200系列之一，而不是标识符100
        QVERIFY(toks[0] != QString::number(100));
    }
};

QTEST_MAIN(LongestMatchTest)
#include "longest_match_test.moc"
