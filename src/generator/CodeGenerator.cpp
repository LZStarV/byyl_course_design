#include "CodeGenerator.h"
#include <QStringBuilder>
static QString genJudge(const MinDFA& m)
{
    QString s;
    s += "int Judgechar(char ch){";
    if (m.alpha.hasLetter)
    {
        s += " if(isalpha((unsigned char)ch)) return 1;";
    }
    if (m.alpha.hasDigit)
    {
        s += " if(isdigit((unsigned char)ch)) return 0;";
    }
    s += " return -1; }";
    return s;
}
static QString genNext()
{
    return "char GetNext(){ extern std::string buffer; extern size_t pos; if(pos<buffer.size()) "
           "return buffer[pos++]; return '\\0'; }";
}
QString CodeGenerator::generate(const MinDFA& mdfa, const QMap<QString, int>& tokenCodes)
{
    QString code;
    code += "#include <cctype>\n";
    code += "#include <string>\n";
    code += "using namespace std;\n\n";

    code += "int Judgechar(char ch) {\n";
    if (mdfa.alpha.hasLetter)
    {
        code += "    if (isalpha((unsigned char)ch)) return 1;\n";
    }
    if (mdfa.alpha.hasDigit)
    {
        code += "    if (isdigit((unsigned char)ch)) return 0;\n";
    }
    code += "    return -1;\n";
    code += "}\n\n";

    code += "bool AcceptState(int s) {\n";
    code += "    switch (s) {\n";
    for (auto it = mdfa.states.begin(); it != mdfa.states.end(); ++it)
    {
        if (it->accept)
        {
            code += "        case " + QString::number(it->id) + ": return true;\n";
        }
    }
    code += "        default: return false;\n";
    code += "    }\n";
    code += "}\n\n";

    code += "int Step(int state, char ch) {\n";
    code += "    switch (state) {\n";
    for (auto it = mdfa.states.begin(); it != mdfa.states.end(); ++it)
    {
        code += "        case " + QString::number(it->id) + ":\n";
        for (auto a : mdfa.alpha.ordered())
        {
            int t = it->trans.value(a, -1);
            if (t != -1)
            {
                if (a.compare("letter", Qt::CaseInsensitive) == 0)
                {
                    code += "            if (isalpha((unsigned char)ch)";
                    code += mdfa.alpha.allowUnderscoreInLetter ? " || ch=='_'" : "";
                    code += mdfa.alpha.allowDollarInLetter ? " || ch=='$'" : "";
                    code += ") return " + QString::number(t) + ";\n";
                }
                else if (a.compare("digit", Qt::CaseInsensitive) == 0)
                {
                    code += "            if (isdigit((unsigned char)ch)) return " +
                            QString::number(t) + ";\n";
                }
                else
                {
                    code += "            if (ch=='" + a + "') return " + QString::number(t) + ";\n";
                }
            }
        }
        code += "            return -1;\n";
    }
    code += "        default: return -1;\n";
    code += "    }\n";
    code += "}\n";
    return code;
}

static QString genAcceptStateI(const MinDFA& mdfa, int idx)
{
    QString s;
    s += "bool AcceptState_" + QString::number(idx) + "(int s)\n";
    s += "{\n";
    s += "    switch (s)\n";
    s += "    {\n";
    for (auto it = mdfa.states.begin(); it != mdfa.states.end(); ++it)
    {
        if (it->accept)
        {
            s += "        case " + QString::number(it->id) + ": return true;\n";
        }
    }
    s += "        default: return false;\n";
    s += "    }\n";
    s += "}\n\n";
    return s;
}
static QString genStepI(const MinDFA& mdfa, int idx)
{
    QString code;
    code += "int Step_" + QString::number(idx) + "(int state, char ch)\n";
    code += "{\n";
    code += "    switch (state)\n";
    code += "    {\n";
    for (auto it = mdfa.states.begin(); it != mdfa.states.end(); ++it)
    {
        code += "        case " + QString::number(it->id) + ":\n";
        for (auto a : mdfa.alpha.ordered())
        {
            int t = it->trans.value(a, -1);
            if (t != -1)
            {
                if (a.compare("letter", Qt::CaseInsensitive) == 0)
                {
                    code += "            if (isalpha((unsigned char)ch)";
                    code += mdfa.alpha.allowUnderscoreInLetter ? " || ch=='_'" : "";
                    code += mdfa.alpha.allowDollarInLetter ? " || ch=='$'" : "";
                    code += ") return " + QString::number(t) + ";\n";
                }
                else if (a.compare("digit", Qt::CaseInsensitive) == 0)
                {
                    code += "            if (isdigit((unsigned char)ch)) return " +
                            QString::number(t) + ";\n";
                }
                else
                {
                    code += "            if (ch=='" + a + "') return " + QString::number(t) + ";\n";
                }
            }
        }
        code += "            return -1;\n";
    }
    code += "        default: return -1;\n";
    code += "    }\n";
    code += "}\n\n";
    return code;
}
QString CodeGenerator::generateCombined(const QVector<MinDFA>& mdfas,
                                        const QVector<int>&    codes,
                                        const Alphabet&        alpha)
{
    QString out;
    out += "#include <cctype>\n";
    out += "#include <string>\n";
    out += "#include <iostream>\n";
    out += "#include <cstdlib>\n";
    out += "using namespace std;\n\n";
    out += "// 自动生成的组合词法分析器\n";
    out += "// 程序根据提供的正则规则构建多个 DFA，并对输入进行扫描。\n";

    out += "// 按字母表配置将字符划分为 letter/digit 类别\n";
    out += "static inline int Judgechar(char ch)\n";
    out += "{\n";
    if (alpha.hasLetter)
    {
        out += "    if (isalpha((unsigned char)ch)) return 1;\n";
    }
    if (alpha.hasDigit)
    {
        out += "    if (isdigit((unsigned char)ch)) return 0;\n";
    }
    out += "    return -1;\n";
    out += "}\n\n";

    for (int i = 0; i < mdfas.size(); ++i)
    {
        out += genAcceptStateI(mdfas[i], i);
        out += genStepI(mdfas[i], i);
    }

    out += "// 每个 Token 的 DFA 函数分发表\n";
    out += "typedef int (*StepFn)(int,char);\n";
    out += "typedef bool (*AcceptFn)(int);\n";
    out += "static StepFn STEPS[" + QString::number(mdfas.size()) + "]={";
    for (int i = 0; i < mdfas.size(); ++i)
    {
        out += "Step_" + QString::number(i);
        if (i + 1 < mdfas.size())
            out += ",";
    }
    out += "};\n";
    out += "static AcceptFn ACCEPTS[" + QString::number(mdfas.size()) + "]={";
    for (int i = 0; i < mdfas.size(); ++i)
    {
        out += "AcceptState_" + QString::number(i);
        if (i + 1 < mdfas.size())
            out += ",";
    }
    out += "};\n";
    out += "static int STARTS[" + QString::number(mdfas.size()) + "]={";
    for (int i = 0; i < mdfas.size(); ++i)
    {
        out += QString::number(mdfas[i].start);
        if (i + 1 < mdfas.size())
            out += ",";
    }
    out += "};\n\n";

    out += "// 权重函数：用于在匹配长度相同的情况下进行优先级决策\n";
    out += "static inline int codeWeight(int c)\n";
    out += "{\n";
    out += "    static bool inited=false;\n";
    out += "    static int mins[16]; static int ws[16]; static int cnt=0;\n";
    out += "    if(!inited){\n";
    out += "        const char* env=getenv(\"LEXER_WEIGHTS\");\n";
    out += "        if(env){\n";
    out += "            // 格式: 220:3,200:4,100:1,0:0\n";
    out += "            const char* p=env; while(*p){\n";
    out +=
        "                int a=0,b=0; while(*p && *p>='0' && *p<='9'){ a=a*10+(*p-'0'); p++; }\n";
    out +=
        "                if(*p==':'){ p++; while(*p && *p>='0' && *p<='9'){ b=b*10+(*p-'0'); p++; "
        "} }\n";
    out +=
        "                mins[cnt]=a; ws[cnt]=b; cnt++; if(*p==',') p++; else while(*p && *p!=',') "
        "p++;\n";
    out += "            }\n";
    out += "        }\n";
    out += "        if(cnt>1){ // 按 minCode 降序排序\n";
    out +=
        "            for(int i=0;i<cnt;i++){ for(int j=i+1;j<cnt;j++){ if(mins[j]>mins[i]){ int "
        "tm=mins[i]; mins[i]=mins[j]; mins[j]=tm; int tw=ws[i]; ws[i]=ws[j]; ws[j]=tw; } } }\n";
    out += "        }\n";
    out +=
        "        if(cnt==0){ mins[cnt]=220; ws[cnt++]=3; mins[cnt]=200; ws[cnt++]=4; "
        "mins[cnt]=100; ws[cnt++]=1; mins[cnt]=0; ws[cnt++]=0; }\n";
    out += "        inited=true;\n";
    out += "    }\n";
    out += "    for(int i=0;i<cnt;i++){ if(c>=mins[i]) return ws[i]; }\n";
    out += "    return 0;\n";
    out += "}\n\n";

    out += "// 计算从位置 pos 开始，DFA[idx] 可接受的最长前缀长度\n";
    out += "static inline int matchLen(int idx, const string& src, size_t pos)\n";
    out += "{\n";
    out += "    int state=STARTS[idx];\n";
    out += "    size_t p=pos;\n";
    out += "    int last=-1;\n";
    out += "    StepFn step=STEPS[idx]; AcceptFn acc=ACCEPTS[idx];\n";
    out += "    while (p<src.size())\n";
    out += "    {\n";
    out += "        char ch=src[p];\n";
    out += "        int ns=step(state,ch);\n";
    out += "        if (ns==-1) break;\n";
    out += "        state=ns;\n";
    out += "        p++;\n";
    out += "        if (acc(state)) last=(int)p;\n";
    out += "    }\n";
    out += "    return last==-1?0:(int)(last-pos);\n";
    out += "}\n\n";

    out += "// 主扫描流程：按配置跳过注释/空白，选择最长匹配（权重用于并列决策）\n";
    out += "static string runMultiple(const string& src)\n";
    out += "{\n";
    out += "    string out;\n";
    out += "    size_t pos=0;\n";
    out += "    while (pos<src.size())\n";
    out += "    {\n";
    out += "        char ch=src[pos];\n";
    out += "        const char* envSkip = getenv(\"LEXER_SKIP_BRACE_COMMENT\");\n";
    out += "        bool skipBrace = false;\n";
    out += "        if (envSkip) {\n";
    out += "            string v(envSkip);\n";
    out += "            for (auto &c : v) c = tolower(c);\n";
    out += "            skipBrace = (v==\"1\" || v==\"true\" || v==\"yes\");\n";
    out += "        }\n";

    out += "        const char* envLine = getenv(\"LEXER_SKIP_LINE_COMMENT\");\n";
    out += "        bool skipLine = true;\n";
    out += "        if (envLine) {\n";
    out += "            string v(envLine);\n";
    out += "            for (auto &c : v) c = tolower(c);\n";
    out += "            skipLine = (v==\"1\" || v==\"true\" || v==\"yes\");\n";
    out += "        }\n";

    out += "        const char* envHash = getenv(\"LEXER_SKIP_HASH_COMMENT\");\n";
    out += "        bool skipHash = true;\n";
    out += "        if (envHash) {\n";
    out += "            string v(envHash);\n";
    out += "            for (auto &c : v) c = tolower(c);\n";
    out += "            skipHash = (v==\"1\" || v==\"true\" || v==\"yes\");\n";
    out += "        }\n";

    out += "        const char* envBlock = getenv(\"LEXER_SKIP_BLOCK_COMMENT\");\n";
    out += "        bool skipBlock = true;\n";
    out += "        if (envBlock) {\n";
    out += "            string v(envBlock);\n";
    out += "            for (auto &c : v) c = tolower(c);\n";
    out += "            skipBlock = (v==\"1\" || v==\"true\" || v==\"yes\");\n";
    out += "        }\n";
    out += "        if (ch==' '||ch=='\\t'||ch=='\\n'||ch=='\\r'){ pos++; continue; }\n";
    out +=
        "        if (skipBrace && ch=='{'){ pos++; while(pos<src.size() && src[pos++]!='}'){} "
        "continue; }\n";
    out +=
        "        if (skipLine && ch=='/' && pos+1<src.size() && src[pos+1]=='/'){ pos+=2; "
        "while(pos<src.size() && src[pos++]!='\\n'){} continue; }\n";
    out +=
        "        if (skipHash && ch==35){ pos++; while(pos<src.size() && src[pos++]!='\\n'){} "
        "continue; }\n";
    out +=
        "        if (skipBlock && ch=='/' && pos+1<src.size() && src[pos+1]=='*'){ pos+=2; "
        "while(pos+1<src.size()){ if(src[pos]=='*' && src[pos+1]=='/'){ pos+=2; break; } pos++; } "
        "continue; }\n";
    out += "        const char* envSq = getenv(\"LEXER_SKIP_SQ_STRING\");\n";
    out += "        bool skipSq = true;\n";
    out += "        if (envSq) {\n";
    out += "            string v(envSq);\n";
    out += "            for (auto &c : v) c = tolower(c);\n";
    out += "            skipSq = (v==\"1\" || v==\"true\" || v==\"yes\");\n";
    out += "        }\n";

    out += "        const char* envDq = getenv(\"LEXER_SKIP_DQ_STRING\");\n";
    out += "        bool skipDq = true;\n";
    out += "        if (envDq) {\n";
    out += "            string v(envDq);\n";
    out += "            for (auto &c : v) c = tolower(c);\n";
    out += "            skipDq = (v==\"1\" || v==\"true\" || v==\"yes\");\n";
    out += "        }\n";

    out += "        const char* envTpl = getenv(\"LEXER_SKIP_TPL_STRING\");\n";
    out += "        bool skipTpl = true;\n";
    out += "        if (envTpl) {\n";
    out += "            string v(envTpl);\n";
    out += "            for (auto &c : v) c = tolower(c);\n";
    out += "            skipTpl = (v==\"1\" || v==\"true\" || v==\"yes\");\n";
    out += "        }\n";
    out += "        if (skipSq && ch==39)\n";
    out += "        {\n";
    out += "            pos++;\n";
    out += "            while (pos < src.size())\n";
    out += "            {\n";
    out += "                char c = src[pos++];\n";
    out += "                if (c == 92)\n";
    out += "                {\n";
    out += "                    if (pos < src.size()) pos++;\n";
    out += "                    continue;\n";
    out += "                }\n";
    out += "                if (c == 39) break;\n";
    out += "            }\n";
    out += "            continue;\n";
    out += "        }\n";

    out += "        if (skipDq && ch==34)\n";
    out += "        {\n";
    out += "            pos++;\n";
    out += "            while (pos < src.size())\n";
    out += "            {\n";
    out += "                char c = src[pos++];\n";
    out += "                if (c == 92)\n";
    out += "                {\n";
    out += "                    if (pos < src.size()) pos++;\n";
    out += "                    continue;\n";
    out += "                }\n";
    out += "                if (c == 34) break;\n";
    out += "            }\n";
    out += "            continue;\n";
    out += "        }\n";

    out += "        if (skipTpl && ch==96)\n";
    out += "        {\n";
    out += "            pos++;\n";
    out += "            while (pos < src.size())\n";
    out += "            {\n";
    out += "                char c = src[pos++];\n";
    out += "                if (c == 92)\n";
    out += "                {\n";
    out += "                    if (pos < src.size()) pos++;\n";
    out += "                    continue;\n";
    out += "                }\n";
    out += "                if (c == 96) break;\n";
    out += "                if (c == 36 && pos < src.size() && src[pos] == 123)\n";
    out += "                {\n";
    out += "                    pos++;\n";
    out += "                    int depth = 1;\n";
    out += "                    while (pos < src.size() && depth > 0)\n";
    out += "                    {\n";
    out += "                        char c2 = src[pos++];\n";
    out += "                        if (c2 == 92)\n";
    out += "                        {\n";
    out += "                            if (pos < src.size()) pos++;\n";
    out += "                            continue;\n";
    out += "                        }\n";
    out += "                        if (c2 == 123) depth++;\n";
    out += "                        else if (c2 == 125) depth--;\n";
    out += "                    }\n";
    out += "                }\n";
    out += "            }\n";
    out += "            continue;\n";
    out += "        }\n";
    out += "        int bestLen=0; int bestIdx=-1; int bestW=-1;\n";
    out += "        int codeList[" + QString::number(codes.size()) + "]={";
    for (int i = 0; i < codes.size(); ++i)
    {
        out += QString::number(codes[i]);
        if (i + 1 < codes.size())
            out += ",";
    }
    out += "};\n";
    out += "        for (int i = 0; i < " + QString::number(mdfas.size()) + "; ++i)\n";
    out += "        {\n";
    out += "            int len = matchLen(i, src, pos);\n";
    out += "            int w   = codeWeight(codeList[i]);\n";
    out += "            if (len > bestLen || (len == bestLen && w > bestW))\n";
    out += "            {\n";
    out += "                bestLen = len;\n";
    out += "                bestIdx = i;\n";
    out += "                bestW   = w;\n";
    out += "            }\n";
    out += "        }\n";
    out += "        if (bestLen > 0)\n";
    out += "        {\n";
    out += "            if (!out.empty()) out += ' ';\n";
    out += "            out += to_string(codeList[bestIdx]);\n";
    out += "            pos += bestLen;\n";
    out += "        }\n";
    out += "        else\n";
    out += "        {\n";
    out += "            if (!out.empty()) out += ' ';\n";
    out += "            out += string(\"ERR\");\n";
    out += "            pos++;\n";
    out += "        }\n";
    out += "    }\n";
    out += "    return out;\n";
    out += "}\n\n";

    out += "int main(int argc, char** argv)\n";
    out += "{\n";
    out += "    string input;\n";
    out += "    if (argc>1)\n";
    out += "    {\n";
    out += "        FILE* f=fopen(argv[1],\"rb\");\n";
    out +=
        "        if (f){ char buf[4096]; size_t n; while((n=fread(buf,1,sizeof(buf),f))>0){ "
        "input.append(buf,n);} fclose(f);}\n";
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
