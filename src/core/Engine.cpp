#include "Engine.h"
RegexFile Engine::lexFile(const QString& text){ return RegexLexer::lex(text); }
ParsedFile Engine::parseFile(const RegexFile& f){ return RegexParser::parse(f); }
NFA Engine::buildNFA(ASTNode* ast, const Alphabet& alpha){ return Thompson::build(ast,alpha); }
DFA Engine::buildDFA(const NFA& nfa){ return SubsetConstruction::build(nfa); }
MinDFA Engine::buildMinDFA(const DFA& dfa){ return Hopcroft::minimize(dfa); }
static Tables tableFromNFA(const NFA& nfa){ Tables t; QVector<QString> cols; cols.push_back("标记"); cols.push_back("状态 ID"); auto syms=nfa.alpha.ordered(); for(auto s: syms) cols.push_back(s); cols.push_back("#"); t.columns=cols; for(auto it=nfa.states.begin(); it!=nfa.states.end(); ++it){ QString mark = it->id==nfa.start?"-":(it->accept?"+":""); QString sid=QString::number(it->id); QVector<QString> row; row.push_back(mark); row.push_back(sid); for(auto s: syms){ QString dst; for(auto e: it->edges){ if(!e.epsilon && e.symbol==s){ dst += QString::number(e.to)+","; } } if(!dst.isEmpty()) dst.chop(1); row.push_back(dst); }
        QString edst; for(auto e: it->edges){ if(e.epsilon){ edst += QString::number(e.to)+","; } } if(!edst.isEmpty()) edst.chop(1); row.push_back(edst); t.rows.push_back(row); t.marks.push_back(mark); t.states.push_back(sid); }
    return t; }
static QString setName(const QSet<int>& s){ QString r="{"; QList<int> v = QList<int>(s.begin(), s.end()); std::sort(v.begin(), v.end()); for(int i=0;i<v.size();++i){ r+=QString::number(v[i]); if(i+1<v.size()) r+=", "; } r+="}"; return r; }
static Tables tableFromDFA(const DFA& dfa){ Tables t; QVector<QString> cols; cols.push_back("标记"); cols.push_back("状态集合"); auto syms=dfa.alpha.ordered(); for(auto s: syms) cols.push_back(s); t.columns=cols; for(auto it=dfa.states.begin(); it!=dfa.states.end(); ++it){ QString mark = it->id==dfa.start?"-":(it->accept?"+":""); QString sid=setName(it->nfaSet); QVector<QString> row; row.push_back(mark); row.push_back(sid); for(auto s: syms){ int to=it->trans.value(s,-1); row.push_back(to==-1?QString():setName(dfa.states[to].nfaSet)); } t.rows.push_back(row); t.marks.push_back(mark); t.states.push_back(sid); }
    return t; }
static Tables tableFromMin(const MinDFA& dfa){ Tables t; QVector<QString> cols; cols.push_back("标记"); cols.push_back("状态 ID"); auto syms=dfa.alpha.ordered(); for(auto s: syms) cols.push_back(s); t.columns=cols; for(auto it=dfa.states.begin(); it!=dfa.states.end(); ++it){ QString mark = it->id==dfa.start?"-":(it->accept?"+":""); QString sid=QString::number(it->id); QVector<QString> row; row.push_back(mark); row.push_back(sid); for(auto s: syms){ int to=it->trans.value(s,-1); row.push_back(to==-1?QString():QString::number(to)); } t.rows.push_back(row); t.marks.push_back(mark); t.states.push_back(sid); } return t; }
Tables Engine::nfaTable(const NFA& nfa){ return tableFromNFA(nfa);} 
Tables Engine::dfaTable(const DFA& dfa){ return tableFromDFA(dfa);} 
Tables Engine::minTable(const MinDFA& dfa){ return tableFromMin(dfa);} 
QString Engine::generateCode(const MinDFA& mdfa, const QMap<QString,int>& tokenCodes){ return CodeGenerator::generate(mdfa, tokenCodes);} 
static int classify(const MinDFA& mdfa, QChar ch){ if(mdfa.alpha.hasLetter && ch.isLetter()) return 1; if(mdfa.alpha.hasDigit && ch.isDigit()) return 0; return -2; }
QString Engine::run(const MinDFA& mdfa, const QString& source, int tokenCode){ QString out; int pos=0; while(pos<source.size()){ QChar ch=source[pos++]; if(ch==' '||ch=='\t'||ch=='\n'||ch=='\r') continue; if(ch=='{'){ while(pos<source.size() && source[pos++]!='}'){} continue; } int state=mdfa.start; bool acc=false; while(true){ bool moved=false; for(auto a: mdfa.alpha.ordered()){ int t=mdfa.states[state].trans.value(a,-1); if(t==-1) continue; if(a.compare("letter",Qt::CaseInsensitive)==0){ if(ch.isLetter()){ state=t; moved=true; if(pos<source.size()) ch=source[pos++]; else break; } } else if(a.compare("digit",Qt::CaseInsensitive)==0){ if(ch.isDigit()){ state=t; moved=true; if(pos<source.size()) ch=source[pos++]; else break; } } else { if(a.size()==1 && ch==a[0]){ state=t; moved=true; if(pos<source.size()) ch=source[pos++]; else break; } } }
            if(!moved) break; acc = mdfa.states[state].accept; }
        if(acc){ out += QString::number(tokenCode)+" "; } else { out += "ERR "; }
    }
    return out.trimmed(); }
