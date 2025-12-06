#include <QtTest/QtTest>
#include "../../src/Engine.h"

class FACoreTest : public QObject {
    Q_OBJECT
  private:
    QString rule_union() {
        return QStringLiteral("_tok100 = a | b\n");
    }
    QString rule_star() {
        return QStringLiteral("_tok101 = (a|b)*\n");
    }
    QString rule_concat() {
        return QStringLiteral("_tok102 = a b\n");
    }
  private slots:
    void nfa_thompson_structure_union() {
        Engine eng; auto rf = eng.lexFile(rule_union()); auto pf = eng.parseFile(rf);
        auto nfa = eng.buildNFA(pf.tokens[0].ast, pf.alpha);
        QVERIFY(nfa.states.size() > 0);
        bool hasEpsilon = false; for (auto it = nfa.states.begin(); it != nfa.states.end(); ++it)
            for (const auto& e : it.value().edges) if (e.epsilon) { hasEpsilon = true; break; }
        QVERIFY(hasEpsilon);
        int acceptCount = 0; for (auto it = nfa.states.begin(); it != nfa.states.end(); ++it)
            if (it.value().accept) acceptCount++;
        QVERIFY(acceptCount >= 1);
    }
    void dfa_subset_determinism_union() {
        Engine eng; auto rf = eng.lexFile(rule_union()); auto pf = eng.parseFile(rf);
        auto dfa = eng.buildDFA(eng.buildNFA(pf.tokens[0].ast, pf.alpha));
        QVERIFY(dfa.states.size() > 0);
        // no epsilon edges in DFA
        for (auto it = dfa.states.begin(); it != dfa.states.end(); ++it) {
            for (auto tit = it.value().trans.begin(); tit != it.value().trans.end(); ++tit)
                QVERIFY(!tit.key().contains(QChar(0))); // symbol is normal
        }
    }
    void mindfa_minimization_star() {
        Engine eng; auto rf = eng.lexFile(rule_star()); auto pf = eng.parseFile(rf);
        auto dfa = eng.buildDFA(eng.buildNFA(pf.tokens[0].ast, pf.alpha));
        auto mdf = eng.buildMinDFA(dfa);
        QVERIFY(mdf.states.size() > 0);
        // start state must exist and be reachable
        QVERIFY(mdf.states.contains(mdf.start));
    }
    void accept_concat_with_codegen() {
        Engine eng; auto rf = eng.lexFile(rule_concat()); auto pf = eng.parseFile(rf);
        auto dfa = eng.buildDFA(eng.buildNFA(pf.tokens[0].ast, pf.alpha));
        auto mdf = eng.buildMinDFA(dfa);
        QMap<QString,int> codes; codes.insert("_tok102", 102);
        auto srcCore = CodeGenerator::generate(mdf, codes);
        QVERIFY(!srcCore.trimmed().isEmpty());
        // 简单运行：用 Engine::run 验证状态机可接受 "ab"
        auto out = eng.run(mdf, QStringLiteral("ab"), 102);
        QVERIFY(out.contains(QString::number(102)));
    }
};

QTEST_MAIN(FACoreTest)
#include "fa_core_test.moc"
