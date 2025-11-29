#include "CodeGenerator.h"
#include <QStringBuilder>
static QString genJudge(const MinDFA& m){ QString s; s += "int Judgechar(char ch){"; if(m.alpha.hasLetter){ s += " if(isalpha((unsigned char)ch)) return 1;"; } if(m.alpha.hasDigit){ s += " if(isdigit((unsigned char)ch)) return 0;"; } s += " return -1; }"; return s; }
static QString genNext(){ return "char GetNext(){ extern std::string buffer; extern size_t pos; if(pos<buffer.size()) return buffer[pos++]; return '\0'; }"; }
QString CodeGenerator::generate(const MinDFA& mdfa, const QMap<QString,int>& tokenCodes){ QString code; code += "#include <iostream>\n#include <cctype>\n#include <string>\nusing namespace std;\n"; code += "std::string buffer; size_t pos=0; char ch;\n"; code += genJudge(mdfa)+"\n"+genNext()+"\n"; code += "void GetToken(){ int state="+QString::number(mdfa.start)+"; bool done=false; ch=GetNext(); while(!done){ switch(state){";
    for(auto it=mdfa.states.begin(); it!=mdfa.states.end(); ++it){ code += " case "+QString::number(it->id)+":"; bool any=false; for(auto a: mdfa.alpha.ordered()){ int t=it->trans.value(a,-1); if(t!=-1){ any=true; if(a.compare("letter",Qt::CaseInsensitive)==0){ code += " if(isalpha((unsigned char)ch)) { state="+QString::number(t)+"; ch=GetNext(); break; }"; } else if(a.compare("digit",Qt::CaseInsensitive)==0){ code += " if(isdigit((unsigned char)ch)) { state="+QString::number(t)+"; ch=GetNext(); break; }"; } else { code += " if(ch=='"+a+"') { state="+QString::number(t)+"; ch=GetNext(); break; }"; } } }
        if(!any){ code += " done=true; break;"; } else { code += " break;"; }
    }
    code += " } } cout<<\"TOK\"; }"; return code; }
