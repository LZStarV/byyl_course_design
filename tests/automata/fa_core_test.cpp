/*
 * 版权信息：Copyright (c) 2023 林展星
 * 文件名称：fa_core_test.cpp
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

class FACoreTest : public QObject
{
    Q_OBJECT
   private:
    QString rule_union()
    {
        return QStringLiteral("_tok100 = a | b\n");
    }
    QString rule_star()
    {
        return QStringLiteral("_tok101 = (a|b)*\n");
    }
    QString rule_concat()
    {
        return QStringLiteral("_tok102 = a b\n");
    }
   private slots:
    void nfa_thompson_structure_union()
    {
        Engine eng;
        qInfo() << "输入规则:" << rule_union().trimmed();
        qInfo() << "关键步骤:" << "lexFile -> parseFile -> buildNFA";
        auto rf  = eng.lexFile(rule_union());
        auto pf  = eng.parseFile(rf);
        auto nfa = eng.buildNFA(pf.tokens[0].ast, pf.alpha);
        QVERIFY(nfa.states.size() > 0);
        bool hasEpsilon   = false;
        int  epsilonEdges = 0;
        int  edgesCount   = 0;
        for (auto it = nfa.states.begin(); it != nfa.states.end(); ++it)
            for (const auto& e : it.value().edges)
            {
                edgesCount++;
                if (e.epsilon)
                {
                    hasEpsilon = true;
                    epsilonEdges++;
                }
            }
        QVERIFY(hasEpsilon);
        int acceptCount = 0;
        for (auto it = nfa.states.begin(); it != nfa.states.end(); ++it)
            if (it.value().accept)
                acceptCount++;
        QVERIFY(acceptCount >= 1);
        qInfo() << "输出:" << "states=" << nfa.states.size() << ", edges=" << edgesCount
                << ", epsilonEdges=" << epsilonEdges << ", acceptCount=" << acceptCount;
    }
    void dfa_subset_determinism_union()
    {
        Engine eng;
        qInfo() << "输入规则:" << rule_union().trimmed();
        qInfo() << "关键步骤:" << "lexFile -> parseFile -> buildNFA -> buildDFA";
        auto rf  = eng.lexFile(rule_union());
        auto pf  = eng.parseFile(rf);
        auto dfa = eng.buildDFA(eng.buildNFA(pf.tokens[0].ast, pf.alpha));
        QVERIFY(dfa.states.size() > 0);
        int  transCount = 0;
        bool allNormal  = true;
        for (auto it = dfa.states.begin(); it != dfa.states.end(); ++it)
        {
            for (auto tit = it.value().trans.begin(); tit != it.value().trans.end(); ++tit)
            {
                transCount++;
                if (tit.key().contains(QChar(0)))
                    allNormal = false;
            }
        }
        QVERIFY(allNormal);
        qInfo() << "输出:" << "states=" << dfa.states.size() << ", transitions=" << transCount
                << ", symbolsNormal=" << allNormal;
    }
    void mindfa_minimization_star()
    {
        Engine eng;
        qInfo() << "输入规则:" << rule_star().trimmed();
        qInfo() << "关键步骤:" << "buildNFA -> buildDFA -> buildMinDFA";
        auto rf  = eng.lexFile(rule_star());
        auto pf  = eng.parseFile(rf);
        auto dfa = eng.buildDFA(eng.buildNFA(pf.tokens[0].ast, pf.alpha));
        auto mdf = eng.buildMinDFA(dfa);
        QVERIFY(mdf.states.size() > 0);
        // start state must exist and be reachable
        QVERIFY(mdf.states.contains(mdf.start));
        qInfo() << "输出:" << "minStates=" << mdf.states.size() << ", start=" << mdf.start;
    }
    void accept_concat_with_codegen()
    {
        Engine             eng;
        auto               rf  = eng.lexFile(rule_concat());
        auto               pf  = eng.parseFile(rf);
        auto               dfa = eng.buildDFA(eng.buildNFA(pf.tokens[0].ast, pf.alpha));
        auto               mdf = eng.buildMinDFA(dfa);
        QMap<QString, int> codes;
        codes.insert("_tok102", 102);
        auto srcCore = CodeGenerator::generate(mdf, codes);
        QVERIFY(!srcCore.trimmed().isEmpty());
        // 简单运行：用 Engine::run 验证状态机可接受 "ab"
        auto out = eng.run(mdf, QStringLiteral("ab"), 102);
        QVERIFY(out.contains(QString::number(102)));
    }
};

QTEST_MAIN(FACoreTest)
#include "fa_core_test.moc"
