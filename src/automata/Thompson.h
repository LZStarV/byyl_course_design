#pragma once
#include "../model/Automata.h"
#include "../regex/RegexParser.h"
class Thompson {
public:
    static NFA build(ASTNode* ast, Alphabet alpha);
};
