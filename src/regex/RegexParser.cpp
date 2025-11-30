#include "RegexParser.h"
#include <QStack>
#include <QSet>
static ASTNode* parseExpr(const QString& s,int& i,const QMap<QString,Rule>& macros);
static bool isMeta(QChar c){ return c=='|'||c=='*'||c=='+'||c=='?'||c=='('||c==')'||c=='['||c==']'; }
static ASTNode* make(ASTNode::Type t,const QString& v=QString()){ auto n=new ASTNode(); n->type=t; n->value=v; return n; }
static ASTNode* concat(ASTNode* a,ASTNode* b){ auto n=make(ASTNode::Concat); n->children={a,b}; return n; }
static ASTNode* unary(ASTNode::Type t,ASTNode* a){ auto n=make(t); n->children={a}; return n; }
static ASTNode* parseCharset(const QString& s,int& i){ QSet<QChar> set; i++; bool negate=false; if(i<s.size() && s[i]=='^'){ negate=true; i++; }
    while(i<s.size() && s[i]!=']'){ if(i+2<s.size() && s[i+1]=='-' && s[i+2]!=']'){ QChar a=s[i], b=s[i+2]; for(int c=a.unicode(); c<=b.unicode(); ++c) set.insert(QChar(c)); i+=3; } else { set.insert(s[i]); i++; } }
    QString v; for(auto ch: set) v.append(ch); return make(ASTNode::CharSet,v);
}
static ASTNode* makeSeqFromString(const QString& text){ if(text.isEmpty()) return nullptr; ASTNode* left = make(ASTNode::Symbol, QString(text[0])); for(int k=1;k<text.size();++k){ auto right = make(ASTNode::Symbol, QString(text[k])); left = concat(left, right); } return left; }
static ASTNode* parseAtom(const QString& s,int& i,const QMap<QString,Rule>& macros){ if(i>=s.size()) return nullptr; QChar c=s[i];
    while(i<s.size() && (s[i]==' '||s[i]=='\t'||s[i]=='\n'||s[i]=='\r')){ i++; }
    if(i>=s.size()) return nullptr; c=s[i];
    if(c=='('){ i++; auto e=parseExpr(s,i,macros); if(i<s.size() && s[i]==')') i++; return e; }
    if(c=='['){ return parseCharset(s,i); }
    if(c=='\\'){ if(i+1<s.size()){ i++; QChar d=s[i++]; return make(ASTNode::Symbol,QString(d)); } }
    if(c.isLetter()){ QString ident; while(i<s.size() && (s[i].isLetter()||s[i].isDigit()||s[i]=='_')){ ident.append(s[i]); i++; }
        if(macros.contains(ident)){ auto ref=make(ASTNode::Ref,ident); return ref; } else { return makeSeqFromString(ident); } }
    if(!isMeta(c)){ i++; return make(ASTNode::Symbol,QString(c)); }
    return nullptr;
}
static ASTNode* parseFactor(const QString& s,int& i,const QMap<QString,Rule>& macros){ auto a=parseAtom(s,i,macros); if(!a) return nullptr; bool done=false; while(!done && i<s.size()){
        QChar c=s[i]; if(c=='*'){ a=unary(ASTNode::Star,a); i++; } else if(c=='+'){ a=unary(ASTNode::Plus,a); i++; } else if(c=='?'){ a=unary(ASTNode::Question,a); i++; } else done=true; }
    return a;
}
static ASTNode* parseConcat(const QString& s,int& i,const QMap<QString,Rule>& macros){ auto left=parseFactor(s,i,macros); while(i<s.size()){
        if(s[i]=='|'||s[i]==')') break; auto right=parseFactor(s,i,macros); if(!right) break; left=concat(left,right); }
    return left;
}
ASTNode* parseExpr(const QString& s,int& i,const QMap<QString,Rule>& macros){ auto left=parseConcat(s,i,macros); while(i<s.size() && s[i]=='|'){ i++; auto right=parseConcat(s,i,macros); auto u=make(ASTNode::Union,"|"); u->children={left,right}; left=u; }
    return left;
}
static void collectAlphabet(ASTNode* n,Alphabet& a,const QMap<QString,Rule>& macros){ if(!n) return; switch(n->type){
        case ASTNode::Symbol: if(!n->value.isEmpty()) a.add(QString(n->value[0])); break;
        case ASTNode::CharSet: for(auto ch: n->value) a.add(QString(ch)); break;
        case ASTNode::Ref:
            if(n->value.compare("letter",Qt::CaseInsensitive)==0){ a.hasLetter=true; a.add("letter"); }
            else if(n->value.compare("digit",Qt::CaseInsensitive)==0){ a.hasDigit=true; a.add("digit"); }
            break;
        default: break;
    }
    for(auto c: n->children) collectAlphabet(c,a,macros);
}
ParsedFile RegexParser::parse(const RegexFile& file){ ParsedFile out; out.macros=file.rules; for(auto r: file.tokens){ int i=0; auto ast=parseExpr(r.expr,i,out.macros); ParsedToken pt; pt.rule=r; pt.ast=ast; out.tokens.push_back(pt); Alphabet a; collectAlphabet(ast,a,out.macros); for(auto s: a.symbols) out.alpha.add(s); if(a.hasLetter) out.alpha.hasLetter=true; if(a.hasDigit) out.alpha.hasDigit=true; }
    return out;
}
