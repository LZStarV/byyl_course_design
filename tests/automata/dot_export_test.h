#pragma once
#include <QtTest/QtTest>
#include "../../src/Engine.h"
#include "../../src/visual/DotExporter.h"

class DotExportTest : public QObject
{
    Q_OBJECT
   private:
    QString sampleRegexUnion()
    {
        return QStringLiteral("_tok200 = a | b\n");
    }
    QString sampleRegexStar()
    {
        return QStringLiteral("_tok100 = (a|b)*\n");
    }
   private slots:
    void test_nfa_dot_contains_epsilon()
    {
        Engine eng;
        auto   rf = eng.lexFile(sampleRegexUnion());
        auto   pf = eng.parseFile(rf);
        QVERIFY(pf.tokens.size() == 1);
        auto nfa = eng.buildNFA(pf.tokens[0].ast, pf.alpha);
        auto dot = DotExporter::toDot(nfa);
        QVERIFY(dot.contains("digraph NFA"));
        QVERIFY(dot.contains("__start -> "));
        QVERIFY(dot.contains("label=\"ε\""));
    }
    void test_dfa_dot_contains_labels()
    {
        Engine eng;
        auto   rf  = eng.lexFile(sampleRegexUnion());
        auto   pf  = eng.parseFile(rf);
        auto   nfa = eng.buildNFA(pf.tokens[0].ast, pf.alpha);
        auto   dfa = eng.buildDFA(nfa);
        auto   dot = DotExporter::toDot(dfa);
        QVERIFY(dot.contains("digraph DFA"));
        QVERIFY(dot.contains("label=\"a\""));
        QVERIFY(dot.contains("label=\"b\""));
        QVERIFY(dot.contains("label=\"{"));
    }
    void test_mindfa_dot_accept_shape()
    {
        Engine eng;
        auto   rf   = eng.lexFile(sampleRegexStar());
        auto   pf   = eng.parseFile(rf);
        auto   nfa  = eng.buildNFA(pf.tokens[0].ast, pf.alpha);
        auto   dfa  = eng.buildDFA(nfa);
        auto   mdfa = eng.buildMinDFA(dfa);
        auto   dot  = DotExporter::toDot(mdfa);
        QVERIFY(dot.contains("digraph MinDFA"));
        QVERIFY(dot.contains("shape=doublecircle") || dot.contains("shape=circle"));
    }
};
