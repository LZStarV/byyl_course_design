#include <QtTest>
#include <QFile>
#include <QTextStream>
#include "../src/core/Engine.h"
class CliRegexTest: public QObject {
    Q_OBJECT
private:
    QString readAllTry(const QString& p){ QFile f(p); if(f.open(QIODevice::ReadOnly|QIODevice::Text)){ QTextStream in(&f); return in.readAll(); } return QString(); }
    QString readAllAny(const QString& rel){
        QString p1 = QCoreApplication::applicationDirPath()+"/../../"+rel;
        auto t = readAllTry(p1); if(!t.isEmpty()) return t;
        QString p2 = QCoreApplication::applicationDirPath()+"/"+rel;
        t = readAllTry(p2); if(!t.isEmpty()) return t;
        QString p3 = rel; t = readAllTry(p3); if(!t.isEmpty()) return t;
        return QString();
    }
private slots:
    void test_js_regex_pipeline(){
        Engine eng; auto text = readAllAny("tests/regex/javascript.regex");
        if(text.isEmpty()){
            text = QStringLiteral("letter = [A-Za-z_]\n")+
                   QStringLiteral("digit = [0-9]\n")+
                   QStringLiteral("_identifier100 = letter(letter|digit)*\n")+
                   QStringLiteral("_number101 = digit+\n")+
                   QStringLiteral("_keywords200S = if | else | return | function | class | import | from | as | for | while | break | continue | let | const | var\n")+
                   QStringLiteral("_operators220S = \\+ | - | \\* | / | % | == | === | != | !== | < | <= | > | >= | = | => | \\( | \\) | \\[ | \\] | \\{ | \\} | , | ;\n");
        }
        QTextStream(stdout) << "TEXT_LEN=" << text.size() << "\n";
        auto rf = eng.lexFile(text);
        QTextStream(stdout) << "TOKENS_LEX=" << rf.tokens.size() << "\n";
        auto pf = eng.parseFile(rf);
        QTextStream(stdout) << "TOKENS_PARSE=" << pf.tokens.size() << "\n";
        QVERIFY(pf.tokens.size() > 0);
        auto ast = pf.tokens[0].ast; auto nfa = eng.buildNFA(ast, pf.alpha); auto dfa = eng.buildDFA(nfa); auto mdfa = eng.buildMinDFA(dfa);
        auto tn = eng.nfaTable(nfa); auto td = eng.dfaTable(dfa); auto tm = eng.minTable(mdfa);
        QVERIFY(tn.columns.size() > 2);
        QVERIFY(td.columns.size() > 2);
        QVERIFY(tm.columns.size() > 2);
        auto src = QStringLiteral("if return == === var"); auto out = eng.run(mdfa, src, pf.tokens[0].rule.code);
        QVERIFY(!out.isEmpty());
        QTextStream(stdout) << "NFA cols=" << tn.columns.size() << "\n";
        QTextStream(stdout) << "DFA cols=" << td.columns.size() << "\n";
        QTextStream(stdout) << "MinDFA cols=" << tm.columns.size() << "\n";
        QTextStream(stdout) << "OUT=" << out << "\n";
    }
};
QTEST_MAIN(CliRegexTest)
#include "cli_regex_test.moc"
