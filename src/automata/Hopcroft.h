#pragma once
#include "../model/Automata.h"
class Hopcroft {
public:
    static MinDFA minimize(const DFA& dfa);
};
