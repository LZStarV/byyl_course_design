#include "Grammar.h"

bool Grammar::hasEpsilon(const QVector<QString>& rhs) const
{
    return rhs.size() == 1 && rhs[0] == "#";
}
