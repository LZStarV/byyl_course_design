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
    QString s;
    s += "bool AcceptState_"+QString::number(idx)+"(int s)\n";
    s += "{\n";
    s += "    switch (s)\n";
    s += "    {\n";
    for(auto it=mdfa.states.begin(); it!=mdfa.states.end(); ++it){ if(it->accept){ s += "        case "+QString::number(it->id)+": return true;\n"; } }
    s += "        default: return false;\n";
    s += "    }\n";
    s += "}\n\n";
    return s;
}
static QString genStepI(const MinDFA& mdfa, int idx){
    QString code;
    code += "int Step_"+QString::number(idx)+"(int state, char ch)\n";
    code += "{\n";
    code += "    switch (state)\n";
    code += "    {\n";
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
    code += "}\n\n";
    return code;
}
QString CodeGenerator::generateCombined(const QVector<MinDFA>& mdfas, const QVector<int>& codes, const Alphabet& alpha){
    QString out;
    out += "#include <cctype>\n";
    out += "#include <string>\n";
    out += "#include <iostream>\n";
    out += "using namespace std;\n\n";

    out += "static inline int Judgechar(char ch)\n";
    out += "{\n";
    if(alpha.hasLetter){ out += "    if (isalpha((unsigned char)ch)) return 1;\n"; }
    if(alpha.hasDigit){ out += "    if (isdigit((unsigned char)ch)) return 0;\n"; }
    out += "    return -1;\n";
    out += "}\n\n";

    for(int i=0;i<mdfas.size();++i){ out += genAcceptStateI(mdfas[i], i); out += genStepI(mdfas[i], i); }

    out += "static inline int codeWeight(int c)\n";
    out += "{\n";
    out += "    if (c>=220) return 3;\n";
    out += "    if (c>=200) return 4;\n";
    out += "    if (c>=100) return 1;\n";
    out += "    return 0;\n";
    out += "}\n\n";

    for(int i=0;i<mdfas.size();++i){
        out += "static int matchLen_"+QString::number(i)+"(const string& src, size_t pos)\n";
        out += "{\n";
        out += "    int state="+QString::number(mdfas[i].start)+";\n";
        out += "    size_t p=pos;\n";
        out += "    int last=-1;\n";
        out += "    while (p<src.size())\n";
        out += "    {\n";
        out += "        char ch=src[p];\n";
        out += "        int ns=Step_"+QString::number(i)+"(state,ch);\n";
        out += "        if (ns==-1) break;\n";
        out += "        state=ns;\n";
        out += "        p++;\n";
        out += "        if (AcceptState_"+QString::number(i)+"(state)) last=(int)p;\n";
        out += "    }\n";
        out += "    return last==-1?0:(int)(last-pos);\n";
        out += "}\n\n";
    }

    out += "static string runMultiple(const string& src)\n";
    out += "{\n";
    out += "    string out;\n";
    out += "    size_t pos=0;\n";
    out += "    while (pos<src.size())\n";
    out += "    {\n";
    out += "        char ch=src[pos];\n";
    out += "        if (ch==' '||ch=='\\t'||ch=='\\n'||ch=='\\r'){ pos++; continue; }\n";
    out += "        if (ch=='{'){ pos++; while(pos<src.size() && src[pos++]!='}'){} continue; }\n";
    out += "        int bestLen=0; int bestIdx=-1; int bestW=-1;\n";
    out += "        int codeList["+QString::number(codes.size())+"]={";
    for(int i=0;i<codes.size();++i){ out += QString::number(codes[i]); if(i+1<codes.size()) out += ","; }
    out += "};\n";
    for(int i=0;i<mdfas.size();++i){
        out += "        { int len=matchLen_"+QString::number(i)+"(src,pos); int w=codeWeight(codeList["+QString::number(i)+"]); if(len>bestLen || (len==bestLen && w>bestW)){ bestLen=len; bestIdx="+QString::number(i)+"; bestW=w; } }\n";
    }
    out += "        if (bestLen>0){ if(!out.empty()) out+=' '; out+=to_string(codeList[bestIdx]); pos+=bestLen; } else { if(!out.empty()) out+=' '; out+=string(\"ERR\"); pos++; }\n";
    out += "    }\n";
    out += "    return out;\n";
    out += "}\n\n";

    out += "int main(int argc, char** argv)\n";
    out += "{\n";
    out += "    string input;\n";
    out += "    if (argc>1)\n";
    out += "    {\n";
    out += "        FILE* f=fopen(argv[1],\"rb\");\n";
    out += "        if (f){ char buf[4096]; size_t n; while((n=fread(buf,1,sizeof(buf),f))>0){ input.append(buf,n);} fclose(f);}\n";
    out += "    }\n";
    out += "    else\n";
    out += "    {\n";
    out += "        string line;\n";
    out += "        while(getline(cin,line)){ if(!input.empty()) input+='\\n'; input+=line; }\n";
    out += "    }\n";
    out += "    string out=runMultiple(input);\n";
    out += "    cout<<out;\n";
    out += "    return 0;\n";
    out += "}\n";
    return out;
}
