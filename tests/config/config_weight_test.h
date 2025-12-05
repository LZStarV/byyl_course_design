#pragma once
#include <QtTest/QtTest>
#include <QtCore/QProcessEnvironment>
#include "../../src/Engine.h"
class ConfigWeightTest : public QObject
{
    Q_OBJECT
   private slots:
    void test_weight_tie_break_env()
    {
        Engine    eng;
        RegexFile rf;
        rf.tokens = {
            Rule{QStringLiteral("_t100"), QStringLiteral("a"), true, 100, false},
            Rule{QStringLiteral("_u220"), QStringLiteral("a"), true, 220, false},
        };
        auto         parsed = eng.parseFile(rf);
        QVector<int> codes;
        auto         mdfas        = eng.buildAllMinDFA(parsed, codes);
        auto         out_default  = eng.runMultiple(mdfas, codes, QStringLiteral("a"));
        auto         toks_default = out_default.split(' ', Qt::SkipEmptyParts);
        QVERIFY(!toks_default.isEmpty());
        QCOMPARE(toks_default[0], QString::number(220));
        qputenv("LEXER_WEIGHTS", QByteArray("100:5,220:1,0:0"));
        auto out_env  = eng.runMultiple(mdfas, codes, QStringLiteral("a"));
        auto toks_env = out_env.split(' ', Qt::SkipEmptyParts);
        QVERIFY(!toks_env.isEmpty());
        QCOMPARE(toks_env[0], QString::number(100));
        qunsetenv("LEXER_WEIGHTS");
    }
};
