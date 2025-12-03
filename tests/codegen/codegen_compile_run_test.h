#pragma once
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QProcess>
#include <QtCore/QDir>
#include "../../src/Engine.h"
class CodegenTest : public QObject
{
    Q_OBJECT
   private:
    QString readAll(const QString& path)
    {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            return QString();
        QTextStream in(&f);
        return in.readAll();
    }
    QString readAllAny(const QString& rel)
    {
        QString p1 = QCoreApplication::applicationDirPath() + "/../../" + rel;
        auto    t  = readAll(p1);
        if (!t.isEmpty())
            return t;
        QString p2 = QCoreApplication::applicationDirPath() + "/" + rel;
        t          = readAll(p2);
        if (!t.isEmpty())
            return t;
        QString p3 = rel;
        t          = readAll(p3);
        if (!t.isEmpty())
            return t;
        return QString();
    }
   private slots:
    void test_generate_compile_run()
    {
        Engine   eng;
        Alphabet alpha;
        alpha.hasLetter = true;
        alpha.hasDigit  = true;
        RegexFile rf;
        Rule      m1;
        m1.name    = "letter";
        m1.expr    = "[A-Za-z_]";
        m1.isToken = false;
        rf.rules.insert(m1.name, m1);
        Rule m2;
        m2.name    = "digit";
        m2.expr    = "[0-9]";
        m2.isToken = false;
        rf.rules.insert(m2.name, m2);
        ASTNode* refLetter = new ASTNode{ASTNode::Ref, QString("letter"), {}};
        ASTNode* refDigit  = new ASTNode{ASTNode::Ref, QString("digit"), {}};
        ASTNode* unionLD   = new ASTNode{ASTNode::Union, QString("|"), {refLetter, refDigit}};
        ASTNode* starUnion = new ASTNode{ASTNode::Star, QString(), {unionLD}};
        ASTNode* concatId =
            new ASTNode{ASTNode::Concat,
                        QString(),
                        {new ASTNode{ASTNode::Ref, QString("letter"), {}}, starUnion}};
        auto               nfa  = eng.buildNFA(concatId, alpha);
        auto               dfa  = eng.buildDFA(nfa);
        auto               mdfa = eng.buildMinDFA(dfa);
        QMap<QString, int> codes;
        codes.insert("_identifier100", 100);
        auto    srcCore = CodeGenerator::generate(mdfa, codes);
        int     codeNum = codes.value("_identifier100", 100);
        QString src =
            QString("#include <iostream>\n") + QString("%1\n").arg(srcCore) +
            QString(
                "int main(){ std::string input; std::string line; "
                "while(std::getline(std::cin,line)){ if(!input.empty()) input+='\\n'; input+=line; "
                "} std::string out; size_t pos=0; while(pos<input.size()){ char c=input[pos++]; "
                "if(c==' '||c=='\\t'||c=='\\n'||c=='\\r'){ continue; } if(c=='{'){ "
                "while(pos<input.size() && input[pos++]!='}'){} continue; } int state=") +
            QString::number(mdfa.start) +
            QString(
                "; bool moved=true; while(moved){ int ns=Step(state,c); if(ns==-1){ moved=false; "
                "break; } state=ns; if(pos<input.size()) c=input[pos++]; else c='\\0'; "
                "if(c=='\\0') break; } if(AcceptState(state)){ if(!out.empty()) out+=' '; "
                "out+=std::to_string(") +
            QString::number(codeNum) +
            QString(
                "); } else { if(!out.empty()) out+=' '; out+=std::string(\"ERR\"); } } "
                "std::cout<<out; return 0; }\n");
        QString outPath = QDir::currentPath() + "/gen_lex.cpp";
        QFile   of(outPath);
        of.open(QIODevice::WriteOnly | QIODevice::Text);
        of.write(src.toUtf8());
        of.close();
        QProcess proc;
        proc.start("clang++",
                   QStringList() << "-std=c++17" << outPath << "-o"
                                 << QDir::currentPath() + "/gen_lex_bin");
        proc.waitForFinished();
        QTextStream(stdout) << "【生成器编译退出码】" << proc.exitCode() << "\n";
        QTextStream(stdout) << "【编译器错误输出】"
                            << QString::fromUtf8(proc.readAllStandardError()) << "\n";
        QVERIFY(proc.exitStatus() == QProcess::NormalExit);
        QProcess run;
        run.start(QDir::currentPath() + "/gen_lex_bin");
        run.write("abc123 def456\n");
        run.closeWriteChannel();
        run.waitForFinished();
        auto output = run.readAllStandardOutput();
        QTextStream(stdout) << "【生成器运行输出】" << QString::fromUtf8(output) << "\n";
        QVERIFY(!output.isEmpty());
    }
};
