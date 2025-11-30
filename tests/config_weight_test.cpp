#include <QtTest>
#include <QProcessEnvironment>
#include "../src/Engine.h"

class ConfigWeightTest: public QObject {
    Q_OBJECT
private slots:
    void test_weight_tie_break_env(){
        // 两个相同长度的匹配，权重决定优先级
        Engine eng; Alphabet alpha; alpha.hasLetter=true; alpha.hasDigit=true;
        // token1: 单个字母
        ASTNode* t1 = new ASTNode{ASTNode::Ref, QString("letter"), {}};
        // token2: 单个字母（相同长度）
        ASTNode* t2 = new ASTNode{ASTNode::Ref, QString("letter"), {}};

        auto nfa1 = eng.buildNFA(t1, alpha); auto dfa1 = eng.buildDFA(nfa1); auto mdfa1 = eng.buildMinDFA(dfa1);
        auto nfa2 = eng.buildNFA(t2, alpha); auto dfa2 = eng.buildDFA(nfa2); auto mdfa2 = eng.buildMinDFA(dfa2);
        QVector<MinDFA> mdfas{mdfa1, mdfa2}; QVector<int> codes{100, 220};

        // 默认：220 的权重为 3，高于 100 的权重 1 → 应选 220
        auto out_default = eng.runMultiple(mdfas, codes, QStringLiteral("a"));
        auto toks_default = out_default.split(' ', Qt::SkipEmptyParts);
        QVERIFY(!toks_default.isEmpty());
        QCOMPARE(toks_default[0], QString::number(220));

        // 设置环境变量，交换优先级：100:5,220:1 → 应选 100
        qputenv("LEXER_WEIGHTS", QByteArray("100:5,220:1,0:0"));
        auto out_env = eng.runMultiple(mdfas, codes, QStringLiteral("a"));
        auto toks_env = out_env.split(' ', Qt::SkipEmptyParts);
        QVERIFY(!toks_env.isEmpty());
        QCOMPARE(toks_env[0], QString::number(100));

        // 清除环境影响
        qunsetenv("LEXER_WEIGHTS");
    }
};

QTEST_MAIN(ConfigWeightTest)
#include "config_weight_test.moc"

