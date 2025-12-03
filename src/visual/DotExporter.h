#pragma once
#include "../model/Automata.h"
#include <QString>

class DotExporter
{
   public:
    static QString toDot(const NFA& nfa);
    static QString toDot(const DFA& dfa);
    static QString toDot(const MinDFA& mdfa);
    static bool    exportToDot(const NFA& nfa, const QString& filename);
    static bool    exportToDot(const DFA& dfa, const QString& filename);
    static bool    exportToDot(const MinDFA& mdfa, const QString& filename);
};
