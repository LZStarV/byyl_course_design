#pragma once
#include <QMap>
#include <QSet>
#include <QVector>
#include <QString>
#include "Alphabet.h"
struct NFAEdge
{
    int     to;
    QString symbol;
    bool    epsilon = false;
};
struct NFAState
{
    int              id;
    bool             accept = false;
    QVector<NFAEdge> edges;
};
struct NFA
{
    int                 start = 0;
    QMap<int, NFAState> states;
    Alphabet            alpha;
};
struct DFAState
{
    QSet<int>          nfaSet;
    int                id     = -1;
    bool               accept = false;
    QMap<QString, int> trans;
};
struct DFA
{
    int                 start = 0;
    QMap<int, DFAState> states;
    Alphabet            alpha;
};
struct MinDFA
{
    int                 start = 0;
    QMap<int, DFAState> states;
    Alphabet            alpha;
};
