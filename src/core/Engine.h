#pragma once
#include <QString>
#include <QVector>
#include <QMap>
#include "regex/RegexLexer.h"
#include "regex/RegexParser.h"
#include "automata/Thompson.h"
#include "automata/SubsetConstruction.h"
#include "automata/Hopcroft.h"
#include "generator/CodeGenerator.h"
struct Tables { QVector<QString> columns; QVector<QString> marks; QVector<QString> states; QVector<QVector<QString>> rows; };
class Engine {
public:
    RegexFile lexFile(const QString& text);
    ParsedFile parseFile(const RegexFile& f);
    NFA buildNFA(ASTNode* ast, const Alphabet& alpha);
    DFA buildDFA(const NFA& nfa);
    MinDFA buildMinDFA(const DFA& dfa);
    Tables nfaTable(const NFA& nfa);
    Tables dfaTable(const DFA& dfa);
    Tables minTable(const MinDFA& dfa);
    QString generateCode(const MinDFA& mdfa, const QMap<QString,int>& tokenCodes);
    QString run(const MinDFA& mdfa, const QString& source, int tokenCode);
};
