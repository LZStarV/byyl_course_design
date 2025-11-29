#include "CodeGenerator.h"
#include <QStringBuilder>
static QString genJudge(const MinDFA& m){ QString s; s += "int Judgechar(char ch){"; if(m.alpha.hasLetter){ s += " if(isalpha((unsigned char)ch)) return 1;"; } if(m.alpha.hasDigit){ s += " if(isdigit((unsigned char)ch)) return 0;"; } s += " return -1; }"; return s; }
static QString genNext(){ return "char GetNext(){ extern std::string buffer; extern size_t pos; if(pos<buffer.size()) return buffer[pos++]; return '\\0'; }"; }
QString CodeGenerator::generate(const MinDFA& mdfa, const QMap<QString,int>& tokenCodes){
    QString code;
    code += "#include <cctype>\n";
    code += "#include <string>\n";
    code += "using namespace std;\n\n";

    code += "int Judgechar(char ch) {\n";
    if (mdfa.alpha.hasLetter) { code += "    if (isalpha((unsigned char)ch)) return 1;\n"; }
    if (mdfa.alpha.hasDigit) { code += "    if (isdigit((unsigned char)ch)) return 0;\n"; }
    code += "    return -1;\n";
    code += "}\n\n";

    code += "bool AcceptState(int s) {\n";
    code += "    switch (s) {\n";
    for(auto it=mdfa.states.begin(); it!=mdfa.states.end(); ++it){ if(it->accept){ code += "        case "+QString::number(it->id)+": return true;\n"; } }
    code += "        default: return false;\n";
    code += "    }\n";
    code += "}\n\n";

    code += "int Step(int state, char ch) {\n";
    code += "    switch (state) {\n";
    for(auto it=mdfa.states.begin(); it!=mdfa.states.end(); ++it){
        code += "        case "+QString::number(it->id)+":\n";
        for(auto a: mdfa.alpha.ordered()){
            int t=it->trans.value(a,-1);
            if(t!=-1){
                if(a.compare("letter",Qt::CaseInsensitive)==0){ code += "            if (isalpha((unsigned char)ch)) return "+QString::number(t)+";\n"; }
                else if(a.compare("digit",Qt::CaseInsensitive)==0){ code += "            if (isdigit((unsigned char)ch)) return "+QString::number(t)+";\n"; }
                else { code += "            if (ch=='"+a+"') return "+QString::number(t)+";\n"; }
            }
        }
        code += "            return -1;\n";
    }
    code += "        default: return -1;\n";
    code += "    }\n";
    code += "}\n";
    return code;
}

static QString genAcceptStateI(const MinDFA& mdfa, int idx){
    QString s; s += "bool AcceptState_"+QString::number(idx)+"(int s){ switch(s){";
    for(auto it=mdfa.states.begin(); it!=mdfa.states.end(); ++it){ if(it->accept){ s += " case "+QString::number(it->id)+": return true;"; } }
    s += " default: return false; } }\n"; return s;
}
static QString genStepI(const MinDFA& mdfa, int idx){
    QString code; code += "int Step_"+QString::number(idx)+"(int state,char ch){ switch(state){";
    for(auto it=mdfa.states.begin(); it!=mdfa.states.end(); ++it){ code += " case "+QString::number(it->id)+":"; for(auto a: mdfa.alpha.ordered()){ int t=it->trans.value(a,-1); if(t!=-1){ if(a.compare("letter",Qt::CaseInsensitive)==0){ code += " if(isalpha((unsigned char)ch)) return "+QString::number(t)+";"; } else if(a.compare("digit",Qt::CaseInsensitive)==0){ code += " if(isdigit((unsigned char)ch)) return "+QString::number(t)+";"; } else { code += " if(ch=='"+a+"') return "+QString::number(t)+";"; } } }
        code += " return -1;";
    }
    code += " default: return -1; } }\n"; return code;
}
QString CodeGenerator::generateCombined(const QVector<MinDFA>& mdfas, const QVector<int>& codes, const Alphabet& alpha){
    QString out; out += "#include <cctype>\n#include <string>\n#include <iostream>\nusing namespace std;\n";
    out += "static inline int Judgechar(char ch){"; if(alpha.hasLetter){ out += " if(isalpha((unsigned char)ch)) return 1;"; } if(alpha.hasDigit){ out += " if(isdigit((unsigned char)ch)) return 0;"; } out += " return -1; }\n";
    for(int i=0;i<mdfas.size();++i){ out += genAcceptStateI(mdfas[i], i); out += genStepI(mdfas[i], i); }
    out += "static inline int codeWeight(int c){ if(c>=220) return 3; if(c>=200) return 4; if(c>=100) return 1; return 0; }\n";
    out += "static int matchLen(int i,const string& src,size_t pos){ int state="+QString::number(mdfas[0].start)+"; size_t p=pos; int last=-1; while(p<src.size()){ char ch=src[p]; int ns=Step_"+QString::number(0)+"(state,ch); if(ns==-1) break; state=ns; p++; if(AcceptState_"+QString::number(0)+"(state)) last=p; } return last==-1?0:(int)(last-pos); }\n";
    // Note: matchLen above uses DFA 0; we need per i. Generate per i separately
    out.chop(out.size());
    for(int i=0;i<mdfas.size();++i){ out += "static int matchLen_"+QString::number(i)+"(const string& src,size_t pos){ int state="+QString::number(mdfas[i].start)+"; size_t p=pos; int last=-1; while(p<src.size()){ char ch=src[p]; int ns=Step_"+QString::number(i)+"(state,ch); if(ns==-1) break; state=ns; p++; if(AcceptState_"+QString::number(i)+"(state)) last=(int)p; } return last==-1?0:(int)(last-pos); }\n"; }
    out += "static string runMultiple(const string& src){ string out; size_t pos=0; while(pos<src.size()){ char ch=src[pos]; if(ch==' '||ch=='\t'||ch=='\n'||ch=='\r'){ pos++; continue; } if(ch=='{'){ pos++; while(pos<src.size() && src[pos++]!='}'){} continue; } int bestLen=0; int bestIdx=-1; int bestW=-1;";
    out += " int codeList["+QString::number(codes.size())+"]={"; for(int i=0;i<codes.size();++i){ out += QString::number(codes[i]); if(i+1<codes.size()) out += ","; } out += "};\n";
    for(int i=0;i<mdfas.size();++i){ out += " int len"+QString::number(i)+"=matchLen_"+QString::number(i)+"(src,pos); int w"+QString::number(i)+"=codeWeight(codeList["+QString::number(i)+"]); if(len"+QString::number(i)+">bestLen || (len"+QString::number(i)+"==bestLen && w"+QString::number(i)+">bestW)){ bestLen=len"+QString::number(i)+"; bestIdx="+QString::number(i)+"; bestW=w"+QString::number(i)+"; }\n"; }
    out += " if(bestLen>0){ if(!out.empty()) out+=' '; out+=to_string(codeList[bestIdx]); pos+=bestLen; } else { if(!out.empty()) out+=' '; out+=string(\"ERR\"); pos++; } } return out; }\n";
    out += "int main(int argc,char** argv){ string input; if(argc>1){ FILE* f=fopen(argv[1],\"rb\"); if(f){ char buf[4096]; size_t n; while((n=fread(buf,1,sizeof(buf),f))>0){ input.append(buf,n);} fclose(f);} } else { string line; while(getline(cin,line)){ if(!input.empty()) input+='\n'; input+=line; } } string out=runMultiple(input); cout<<out; return 0; }\n";
    return out;
}
