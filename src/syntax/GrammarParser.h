#pragma once
#include "Grammar.h"
#include <QString>

namespace GrammarParser
{
    Grammar parseFile(const QString& path, QString& error);
    Grammar parseString(const QString& text, QString& error);
}

