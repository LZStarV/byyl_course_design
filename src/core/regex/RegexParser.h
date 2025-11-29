#pragma once
#include <QString>
#include <QMap>
#include <QVector>
#include "RegexLexer.h"
#include "../model/Alphabet.h"
struct ASTNode { enum Type{Concat,Union,Star,Plus,Question,CharSet,Symbol,Ref} type; QString value; QVector<ASTNode*> children; };
struct ParsedToken { Rule rule; ASTNode* ast; };
struct ParsedFile { QMap<QString,Rule> macros; QVector<ParsedToken> tokens; Alphabet alpha; };
class RegexParser {
public:
    static ParsedFile parse(const RegexFile& file);
};
