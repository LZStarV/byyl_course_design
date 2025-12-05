#include <QtTest/QtTest>
#include "../../src/Engine.h"
#include "../../src/config/Config.h"
#include "../../src/syntax/GrammarParser.h"
#include "../../src/syntax/LR1.h"
#include "../../src/syntax/LR1Parser.h"
#include "../../src/syntax/TokenMapBuilder.h"

class TinyLR1FlowTest : public QObject
{
    Q_OBJECT
   private slots:
    void test_flow()
    {
        QFile fr("../tests/test_data/regex/tiny.regex");
        QVERIFY(fr.open(QIODevice::ReadOnly | QIODevice::Text));
        QString regexText = QTextStream(&fr).readAll();
        fr.close();

        Engine       engine;
        auto         rf = engine.lexFile(regexText);
        auto         pf = engine.parseFile(rf);
        QVector<int> codes;
        auto         mdfas = engine.buildAllMinDFA(pf, codes);
        QSet<int>    idCodes;
        {
            auto             names = Config::identifierTokenNames();
            QVector<QString> lowers;
            for (auto s : names) lowers.push_back(s.trimmed().toLower());
            for (const auto& pt : pf.tokens)
            {
                QString n = pt.rule.name.trimmed().toLower();
                for (const auto& k : lowers)
                {
                    if (!k.isEmpty() && n.contains(k))
                    {
                        idCodes.insert(pt.rule.code);
                        break;
                    }
                }
            }
        }

        QFile fs("../tests/test_data/sample/tiny/tiny1.tny");
        QVERIFY(fs.open(QIODevice::ReadOnly | QIODevice::Text));
        QString src = QTextStream(&fs).readAll();
        fs.close();
        auto tokenStr = engine.runMultiple(mdfas, codes, src, idCodes);
        qWarning() << "[tiny] tokens(raw)" << tokenStr;
        auto             map = TokenMapBuilder::build(regexText, pf);
        QVector<QString> tokens;
        {
            auto          raw = tokenStr.split(' ', Qt::SkipEmptyParts);
            QSet<QString> idNames;
            for (auto s : Config::identifierTokenNames()) idNames.insert(s.trimmed().toLower());
            for (int i = 0; i < raw.size(); ++i)
            {
                QString mapped = map.value(raw[i], raw[i]);
                QString mlow   = mapped.trimmed().toLower();
                if (idNames.contains(mlow))
                {
                    tokens.push_back(mapped);
                    if (i + 1 < raw.size())
                        i++;
                    continue;
                }
                tokens.push_back(mapped);
            }
        }
        QString mapped;
        for (int i = 0; i < tokens.size() && i < 40; ++i)
        {
            mapped += tokens[i] + " ";
        }
        qWarning() << "[tiny] tokens(mapped <=40)" << mapped.trimmed();

        QFile fg("../tests/test_data/syntax/tiny_grammar.txt");
        QVERIFY(fg.open(QIODevice::ReadOnly | QIODevice::Text));
        QString gtext = QTextStream(&fg).readAll();
        fg.close();
        QString err;
        auto    g = engine.parseGrammarText(gtext, err);
        QVERIFY(err.isEmpty());
        auto gr = LR1Builder::build(g);
        auto tb = LR1Builder::computeActionTable(g, gr);
        auto r  = LR1Parser::parse(tokens, g, tb);
        QVERIFY2(r.errorPos < 0, "LR1 parse failed for tiny program");
    }
};

QTEST_MAIN(TinyLR1FlowTest)
#include "tiny_lr1_flow_test.moc"
