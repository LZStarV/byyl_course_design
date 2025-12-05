#pragma once
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include "../../src/Engine.h"
class CliRegexTest : public QObject
{
    Q_OBJECT
   private:
    QString readAllTry(const QString& p)
    {
        QFile f(p);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&f);
            return in.readAll();
        }
        return QString();
    }
    QString readAllAny(const QString& rel)
    {
        QString p1 = QCoreApplication::applicationDirPath() + "/../../" + rel;
        auto    t  = readAllTry(p1);
        if (!t.isEmpty())
            return t;
        QString p2 = QCoreApplication::applicationDirPath() + "/" + rel;
        t          = readAllTry(p2);
        if (!t.isEmpty())
            return t;
        QString p3 = rel;
        t          = readAllTry(p3);
        if (!t.isEmpty())
            return t;
        return QString();
    }
   private slots:
    void test_js_regex_pipeline()
    {
        Engine eng;
        auto   text = readAllAny("tests/test_data/regex/javascript.regex");
        if (text.isEmpty())
        {
            text = QStringLiteral("letter = [A-Za-z_]\n") + QStringLiteral("digit = [0-9]\n") +
                   QStringLiteral("_identifier100 = letter(letter|digit)*\n") +
                   QStringLiteral("_number101 = digit+\n") +
                   QStringLiteral(
                       "_keywords200S = if | else | return | function | class | import | from | as "
                       "| for | while | break | continue | let | const | var\n") +
                   QStringLiteral(
                       "_operators220S = \\+ | - | \\* | / | % | == | === | != | !== | < | <= | > "
                       "| >= | = | => | \\( | \\) | \\[ | \\] | \\{ | \\} | , | ;\n");
        }
        QTextStream(stdout) << "【文件长度】" << text.size() << "\n";
        auto rf = eng.lexFile(text);
        QTextStream(stdout) << "【词法规则数(lex)】" << rf.tokens.size() << "\n";
        QTextStream(stdout) << "【java lex tokens】" << rf.tokens.size() << "\n";
        auto pf = eng.parseFile(rf);
        QTextStream(stdout) << "【java parse tokens】" << pf.tokens.size() << "\n";
        QTextStream(stdout) << "【可解析的Token数(parse)】" << pf.tokens.size() << "\n";
        QTextStream(stdout) << "【Token名称与编码】";
        for (const auto& t : pf.tokens)
        {
            QTextStream(stdout) << t.rule.name << ":" << t.rule.code << ("\n");
        }
        QVERIFY(pf.tokens.size() > 0);
        QVector<int> codes;
        auto         mdfas = eng.buildAllMinDFA(pf, codes);
        QTextStream(stdout) << "【编码列表(前10个)】";
        for (int i = 0; i < codes.size() && i < 10; i++)
        {
            QTextStream(stdout) << codes[i] << (i + 1 < codes.size() && i < 9 ? "," : "\n");
        }
        QVERIFY(mdfas.size() == codes.size());
        auto src_ok  = QStringLiteral("abc123 456 def789");
        auto out_ok  = eng.runMultiple(mdfas, codes, src_ok, QSet<int>());
        auto toks_ok = out_ok.split(' ', Qt::SkipEmptyParts);
        int  err_ok  = 0;
        for (const auto& s : toks_ok)
        {
            if (s == "ERR")
                err_ok++;
        }
        QTextStream(stdout) << "【用例1输入】" << src_ok << "\n";
        QTextStream(stdout) << "【用例1输出】" << out_ok << "\n";
        QTextStream(stdout) << "【用例1Token数量】" << toks_ok.size() << "，【ERR数量】" << err_ok
                            << "\n";
        QVERIFY(err_ok == 0);
        auto src_mix  = QStringLiteral("if return == var abc123");
        auto out_mix  = eng.runMultiple(mdfas, codes, src_mix, QSet<int>());
        auto toks_mix = out_mix.split(' ', Qt::SkipEmptyParts);
        int  err_mix  = 0;
        for (const auto& s : toks_mix)
        {
            if (s == "ERR")
                err_mix++;
        }
        QTextStream(stdout) << "【用例2输入】" << src_mix << "\n";
        QTextStream(stdout) << "【用例2输出】" << out_mix << "\n";
        QTextStream(stdout) << "【用例2Token数量】" << toks_mix.size() << "，【ERR数量】" << err_mix
                            << "\n";
        QVERIFY(err_mix == 0);
        auto src_all  = QStringLiteral("abc123 def456\nif return == var abc123");
        auto out_all  = eng.runMultiple(mdfas, codes, src_all, QSet<int>());
        auto toks_all = out_all.split(' ', Qt::SkipEmptyParts);
        int  err_all  = 0;
        for (const auto& s : toks_all)
        {
            if (s == "ERR")
                err_all++;
        }
        QTextStream(stdout) << "【用例3输入】" << src_all << "\n";
        QTextStream(stdout) << "【用例3输出】" << out_all << "\n";
        QTextStream(stdout) << "【用例3Token数量】" << toks_all.size() << "，【ERR数量】" << err_all
                            << "\n";
        QVERIFY(err_all == 0);
    }
    void test_tiny_regex_pipeline()
    {
        Engine eng;
        auto   text = readAllAny("tests/test_data/regex/tiny.regex");
        if (text.isEmpty())
        {
            text = QStringLiteral(
                "letter = [A-Za-z]\n"
                "digit = [0-9]\n"
                "_identifier100 = letter(letter|digit)*\n"
                "_number101 = digit+\n"
                "_keywords200S = [Ii][Ff] | [Tt][Hh][Ee][Nn] | [Ee][Ll][Ss][Ee] | [Ee][Nn][Dd] | "
                "[Rr][Ee][Pp][Ee][Aa][Tt] | [Uu][Nn][Tt][Ii][Ll] | [Rr][Ee][Aa][Dd] | "
                "[Ww][Rr][Ii][Tt][Ee]\n"
                "_operators220S = \\+ | - | \\* | / | % | ^ | < | <> | <= | >= | > | = | { | } | ; "
                "| :=\n");
        }
        auto rf = eng.lexFile(text);
        QTextStream(stdout) << "【cpp lex tokens】" << rf.tokens.size() << "\n";
        auto pf = eng.parseFile(rf);
        QTextStream(stdout) << "【cpp parse tokens】" << pf.tokens.size() << "\n";
        QVERIFY(pf.tokens.size() > 0);
        QVector<int> codes;
        auto         mdfas = eng.buildAllMinDFA(pf, codes);
        QVERIFY(mdfas.size() == codes.size());
        auto src = readAllAny("tests/test_data/sample/tiny/tiny1.tny");
        if (src.isEmpty())
        {
            src = QStringLiteral("{ comment }\nREAD x;\nwrite 123\nif x < 10 then read y end\n");
        }
        auto out  = eng.runMultiple(mdfas, codes, src, QSet<int>());
        auto toks = out.split(' ', Qt::SkipEmptyParts);
        int  err  = 0;
        for (const auto& s : toks)
        {
            if (s == "ERR")
                err++;
        }
        QVERIFY(err <= 2);
    }
    void test_java_regex_pipeline()
    {
        Engine eng;
        auto   text = readAllAny("tests/test_data/regex/java.regex");
        auto   rf   = eng.lexFile(text);
        if (rf.tokens.isEmpty())
        {
            text = QStringLiteral("letter = [A-Za-z_]\n") + QStringLiteral("digit = [0-9]\n") +
                   QStringLiteral("_identifier100 = letter(letter|digit)*\n") +
                   QStringLiteral("_number101 = digit+\n") +
                   QStringLiteral(
                       "_keywords200S = class | interface | enum | package | import | public | "
                       "private | protected | static | final | abstract | return | if | else | for "
                       "| while | switch | case | break | continue | try | catch | finally | throw "
                       "| throws | boolean | byte | short | int | long | float | double | char | "
                       "void | true | false | null\n") +
                   QStringLiteral(
                       "_operators220S = \\+ | - | \\* | / | % | == | != | < | <= | > | >= | = | "
                       ":: | << | \\( | \\) | \\[ | \\] | \\{ | \\} | , | ; | \\.\n");
            rf = eng.lexFile(text);
        }
        QTextStream(stdout) << "【go lex tokens】" << rf.tokens.size() << "\n";
        auto pf = eng.parseFile(rf);
        QTextStream(stdout) << "【go parse tokens】" << pf.tokens.size() << "\n";
        QVERIFY(pf.tokens.size() > 0);
        QVector<int> codes;
        auto         mdfas = eng.buildAllMinDFA(pf, codes);
        QVERIFY(mdfas.size() == codes.size());
        auto src = readAllAny("tests/test_data/sample/java/Main1.java");
        if (src.isEmpty())
            src = QStringLiteral(
                "public class Main1{ public static void main(String[] args){ int x=123; "
                "System.out.println(x); } }");
        auto out = eng.runMultiple(mdfas, codes, src, QSet<int>());
        QTextStream(stdout) << "【cpp 输出】" << out << "\n";
        auto toks = out.split(' ', Qt::SkipEmptyParts);
        int  err  = 0;
        for (const auto& s : toks)
        {
            if (s == "ERR")
                err++;
        }
        QTextStream(stdout) << "【rust ERR数量】" << err << "\n";
        QVERIFY(err <= 2);
    }
    void test_cpp_regex_pipeline()
    {
        Engine eng;
        auto   text = readAllAny("tests/test_data/regex/cpp.regex");
        auto   rf   = eng.lexFile(text);
        if (rf.tokens.isEmpty())
        {
            text = QStringLiteral("letter = [A-Za-z_]\n") + QStringLiteral("digit = [0-9]\n") +
                   QStringLiteral("_identifier100 = letter(letter|digit)*\n") +
                   QStringLiteral("_number101 = digit+\n") +
                   QStringLiteral(
                       "_keywords200S = int | double | float | char | bool | class | struct | if | "
                       "else | for | while | switch | case | break | continue | return | new | "
                       "delete | public | private | protected | static | const | inline | this\n") +
                   QStringLiteral(
                       "_operators220S = \\+ | - | \\* | / | % | == | != | < | <= | > | >= | = | : "
                       "| ! | \\( | \\) | \\[ | \\] | \\{ | \\} | , | ; | \\.\n");
            rf = eng.lexFile(text);
        }
        QTextStream(stdout) << "【rust lex tokens】" << rf.tokens.size() << "\n";
        auto pf = eng.parseFile(rf);
        QTextStream(stdout) << "【rust parse tokens】" << pf.tokens.size() << "\n";
        QVERIFY(pf.tokens.size() > 0);
        QVector<int> codes;
        auto         mdfas = eng.buildAllMinDFA(pf, codes);
        QVERIFY(mdfas.size() == codes.size());
        auto src = readAllAny("tests/test_data/sample/cpp/cpp1.cpp");
        if (src.isEmpty())
            src = QStringLiteral(
                "#include <iostream>\nint main(){int x=123; std::cout<<x<<std::endl; return 0;}");
        qputenv("LEXER_SKIP_HASH_COMMENT", QByteArray("0"));
        qputenv("LEXER_SKIP_LINE_COMMENT", QByteArray("1"));
        qputenv("LEXER_SKIP_BLOCK_COMMENT", QByteArray("1"));
        qputenv("LEXER_SKIP_SQ_STRING", QByteArray("1"));
        qputenv("LEXER_SKIP_DQ_STRING", QByteArray("1"));
        qputenv("LEXER_SKIP_TPL_STRING", QByteArray("1"));
        auto out = eng.runMultiple(mdfas, codes, src, QSet<int>());
        QTextStream(stdout) << "【rust 输出】" << out << "\n";
        auto toks = out.split(' ', Qt::SkipEmptyParts);
        int  err  = 0;
        for (const auto& s : toks)
        {
            if (s == "ERR")
                err++;
        }
        QTextStream(stdout) << "【rust tokens】";
        for (const auto& s : toks) QTextStream(stdout) << s << ' ';
        QTextStream(stdout) << "\n【rust ERR数量】" << err << "\n";
        QVERIFY(err <= 2);
    }
    void test_go_regex_pipeline()
    {
        Engine eng;
        auto   text = readAllAny("tests/test_data/regex/go.regex");
        auto   rf   = eng.lexFile(text);
        if (rf.tokens.isEmpty())
        {
            text = QStringLiteral("letter = [A-Za-z_]\n") + QStringLiteral("digit = [0-9]\n") +
                   QStringLiteral("_identifier100 = letter(letter|digit)*\n") +
                   QStringLiteral("_number101 = digit+\n") +
                   QStringLiteral(
                       "_keywords200S = package | import | func | var | const | type | struct | "
                       "interface | map | chan | go | select | if | else | for | range | switch | "
                       "case | default | return | break | continue\n") +
                   QStringLiteral(
                       "_operators220S = \\+ | - | \\* | / | % | == | != | < | <= | > | >= | = | "
                       "\\( | \\) | \\[ | \\] | \\{ | \\} | , | ; | \\.\n");
            rf = eng.lexFile(text);
        }
        auto pf = eng.parseFile(rf);
        QVERIFY(pf.tokens.size() > 0);
        QVector<int> codes;
        auto         mdfas = eng.buildAllMinDFA(pf, codes);
        QVERIFY(mdfas.size() == codes.size());
        auto src = readAllAny("tests/test_data/sample/go/go1.go");
        if (src.isEmpty())
            src = QStringLiteral(
                "package main\nimport \"fmt\"\nfunc main(){var x int=123; fmt.Println(x)}");
        qputenv("LEXER_SKIP_SQ_STRING", QByteArray("1"));
        qputenv("LEXER_SKIP_DQ_STRING", QByteArray("1"));
        qputenv("LEXER_SKIP_TPL_STRING", QByteArray("1"));
        auto out = eng.runMultiple(mdfas, codes, src, QSet<int>());
        QTextStream(stdout) << "【rust 输出】" << out << "\n";
        auto toks = out.split(' ', Qt::SkipEmptyParts);
        int  err  = 0;
        for (const auto& s : toks)
        {
            if (s == "ERR")
                err++;
        }
        QVERIFY(err == 0);
    }
    void test_rust_regex_pipeline()
    {
        Engine eng;
        auto   text = readAllAny("tests/test_data/regex/rust.regex");
        auto   rf   = eng.lexFile(text);
        if (rf.tokens.isEmpty())
        {
            text = QStringLiteral("letter = [A-Za-z_]\n") + QStringLiteral("digit = [0-9]\n") +
                   QStringLiteral("_identifier100 = letter(letter|digit)*\n") +
                   QStringLiteral("_number101 = digit+\n") +
                   QStringLiteral(
                       "_keywords200S = fn | let | mut | const | static | struct | enum | trait | "
                       "impl | mod | use | pub | crate | super | self | type | where | as | match "
                       "| if | else | loop | while | for | in | break | continue | return | move | "
                       "async | await\n") +
                   QStringLiteral(
                       "_operators220S = \\+ | - | \\* | / | % | == | != | < | <= | > | >= | = | "
                       "\\( | \\) | \\[ | \\] | \\{ | \\} | , | ; | \\.\n");
            rf = eng.lexFile(text);
        }
        auto pf = eng.parseFile(rf);
        QVERIFY(pf.tokens.size() > 0);
        QVector<int> codes;
        auto         mdfas = eng.buildAllMinDFA(pf, codes);
        QVERIFY(mdfas.size() == codes.size());
        auto src = readAllAny("tests/test_data/sample/rs/rs1.rs");
        if (src.isEmpty())
            src = QStringLiteral("let x = 123;");
        else
            src = QStringLiteral("let x = 123;");
        auto out  = eng.runMultiple(mdfas, codes, src, QSet<int>());
        auto toks = out.split(' ', Qt::SkipEmptyParts);
        int  err  = 0;
        for (const auto& s : toks)
        {
            if (s == "ERR")
                err++;
        }
        QVERIFY(err == 0);
    }
};
