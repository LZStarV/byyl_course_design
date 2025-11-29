#pragma once
#include "../model/Automata.h"
#include <QString>
class CodeGenerator {
public:
    static QString generate(const MinDFA& mdfa, const QMap<QString,int>& tokenCodes);
};
