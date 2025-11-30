#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStatusBar>
#include <QDesktopServices>
#include <QUrl>
#include <QTabWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QLabel>
#include <QProcess>
#include <QDir>
#include <QComboBox>
#include <QDateTime>
#include <QCryptographicHash>
#include "../src/Engine.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    engine = new Engine();
    parsedPtr = nullptr;
    lastMinPtr = nullptr;
    selectedSamplePath = QString();
    currentRegexHash = QString();
    currentCodePath = QString();
    currentBinPath = QString();
    setupUiCustom();
}

MainWindow::~MainWindow()
{
    if(lastMinPtr){ delete lastMinPtr; lastMinPtr=nullptr; }
    if(parsedPtr){ delete parsedPtr; parsedPtr=nullptr; }
    delete engine;
    delete ui;
}

void MainWindow::setupUiCustom(){
    tabs = new QTabWidget(ui->centralwidget);
    auto v = new QVBoxLayout; ui->centralwidget->setLayout(v); v->addWidget(tabs);
    auto w1 = new QWidget; auto l1 = new QVBoxLayout(w1); txtInputRegex = new QTextEdit; txtInputRegex->setObjectName("txtInputRegex");
    auto h1 = new QHBoxLayout; btnLoadRegex = new QPushButton("从文件加载"); btnLoadRegex->setObjectName("btnLoadRegex"); btnSaveRegex = new QPushButton("保存正则"); btnSaveRegex->setObjectName("btnSaveRegex"); btnStartConvert = new QPushButton("转换"); btnStartConvert->setObjectName("btnStartConvert"); h1->addWidget(btnLoadRegex); h1->addWidget(btnSaveRegex); h1->addWidget(btnStartConvert);
    l1->addWidget(txtInputRegex); l1->addLayout(h1); tabs->addTab(w1, "正则编辑");
    auto w2 = new QWidget; auto l2 = new QVBoxLayout(w2);
    auto h2 = new QHBoxLayout; auto lblTok = new QLabel("选择Token"); cmbTokens = new QComboBox; cmbTokens->setObjectName("cmbTokens"); h2->addWidget(lblTok); h2->addWidget(cmbTokens); l2->addLayout(h2);
    tblNFA = new QTableWidget; tblNFA->setObjectName("tblNFA"); l2->addWidget(tblNFA); tabs->addTab(w2, "NFA状态表");
    auto w3 = new QWidget; auto l3 = new QVBoxLayout(w3);
    auto h3 = new QHBoxLayout; auto lblTokD = new QLabel("选择Token"); cmbTokensDFA = new QComboBox; cmbTokensDFA->setObjectName("cmbTokensDFA"); h3->addWidget(lblTokD); h3->addWidget(cmbTokensDFA); l3->addLayout(h3);
    tblDFA = new QTableWidget; tblDFA->setObjectName("tblDFA"); l3->addWidget(tblDFA); tabs->addTab(w3, "DFA状态表");
    auto w4 = new QWidget; auto l4 = new QVBoxLayout(w4);
    auto h4 = new QHBoxLayout; auto lblTokM = new QLabel("选择Token"); cmbTokensMin = new QComboBox; cmbTokensMin->setObjectName("cmbTokensMin"); h4->addWidget(lblTokM); h4->addWidget(cmbTokensMin); l4->addLayout(h4);
    tblMinDFA = new QTableWidget; tblMinDFA->setObjectName("tblMinDFA"); btnGenCode = new QPushButton("生成代码"); btnGenCode->setObjectName("btnGenCode"); l4->addWidget(tblMinDFA); l4->addWidget(btnGenCode); tabs->addTab(w4, "最小化DFA");
    auto w5 = new QWidget; auto l5 = new QVBoxLayout(w5); txtGeneratedCode = new QPlainTextEdit; txtGeneratedCode->setObjectName("txtGeneratedCode"); btnCompileRun = new QPushButton("编译并运行生成器"); btnCompileRun->setObjectName("btnCompileRun"); l5->addWidget(txtGeneratedCode); l5->addWidget(btnCompileRun); tabs->addTab(w5, "代码查看");
    auto w6 = new QWidget; auto l6 = new QVBoxLayout(w6);
    auto h6 = new QHBoxLayout;
    auto leftCol = new QVBoxLayout; auto rightCol = new QVBoxLayout;
    auto lblSrc = new QLabel("源程序输入");
    auto lblOut = new QLabel("Token 编码输出");
    txtSourceTiny = new QPlainTextEdit; txtSourceTiny->setObjectName("txtSourceTiny");
    txtLexResult = new QPlainTextEdit; txtLexResult->setObjectName("txtLexResult");
    btnPickSample = new QPushButton("选择样例文件"); btnPickSample->setObjectName("btnPickSample");
    leftCol->addWidget(lblSrc); leftCol->addWidget(txtSourceTiny);
    rightCol->addWidget(lblOut); rightCol->addWidget(txtLexResult);
    h6->addLayout(leftCol); h6->addLayout(rightCol);
    btnRunLexer = new QPushButton("运行词法分析"); btnRunLexer->setObjectName("btnRunLexer");
    l6->addLayout(h6); l6->addWidget(btnPickSample); l6->addWidget(btnRunLexer);
    tabs->addTab(w6, "测试与验证");
    connect(btnStartConvert,&QPushButton::clicked,this,&MainWindow::onConvertClicked);
    connect(btnGenCode,&QPushButton::clicked,this,&MainWindow::onGenCodeClicked);
    connect(btnRunLexer,&QPushButton::clicked,this,&MainWindow::onRunLexerClicked);
    connect(btnPickSample,&QPushButton::clicked,this,&MainWindow::onPickSampleClicked);
    connect(btnCompileRun,&QPushButton::clicked,this,&MainWindow::onCompileRunClicked);
    connect(btnLoadRegex,&QPushButton::clicked,this,&MainWindow::onLoadRegexClicked);
    connect(btnSaveRegex,&QPushButton::clicked,this,&MainWindow::onSaveRegexClicked);
    connect(cmbTokens, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onTokenChanged);
    connect(cmbTokensDFA, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onTokenChangedDFA);
    connect(cmbTokensMin, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onTokenChangedMin);
    connect(tabs, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
}

void MainWindow::fillTable(QTableWidget* tbl, const Tables& t){
    tbl->clear(); tbl->setColumnCount(t.columns.size()); tbl->setRowCount(t.rows.size());
    QStringList headers; for(auto c: t.columns) headers<<c; tbl->setHorizontalHeaderLabels(headers);
    for(int r=0;r<t.rows.size();++r){ auto row=t.rows[r]; for(int c=0;c<row.size();++c){ tbl->setItem(r,c,new QTableWidgetItem(row[c])); } }
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::onConvertClicked(bool){
    auto text = txtInputRegex->toPlainText(); auto rf = engine->lexFile(text); auto parsed = engine->parseFile(rf);
    if(parsed.tokens.isEmpty()){ statusBar()->showMessage("未找到Token定义"); return; }
    parsedPtr = new ParsedFile(parsed);
    currentRegexHash = computeRegexHash(text);
    currentCodePath.clear(); currentBinPath.clear();
    cmbTokens->blockSignals(true);
    cmbTokens->clear(); cmbTokens->addItem("全部"); for(const auto& t : parsed.tokens){ cmbTokens->addItem(t.rule.name); }
    cmbTokens->blockSignals(false);
    cmbTokens->setCurrentIndex(0);
    cmbTokensDFA->blockSignals(true);
    cmbTokensDFA->clear(); cmbTokensDFA->addItem("全部"); for(const auto& t : parsed.tokens){ cmbTokensDFA->addItem(t.rule.name); }
    cmbTokensDFA->blockSignals(false);
    cmbTokensDFA->setCurrentIndex(0);
    cmbTokensMin->blockSignals(true);
    cmbTokensMin->clear(); cmbTokensMin->addItem("全部"); for(const auto& t : parsed.tokens){ cmbTokensMin->addItem(t.rule.name); }
    cmbTokensMin->blockSignals(false);
    cmbTokensMin->setCurrentIndex(0);
    onTokenChanged(0); onTokenChangedDFA(0); onTokenChangedMin(0);
    statusBar()->showMessage("转换成功");
}

void MainWindow::onGenCodeClicked(bool){
    if(!parsedPtr){ statusBar()->showMessage("请先转换"); return; }
    int idx = cmbTokens ? cmbTokens->currentIndex() : -1;
    if(idx == 0){
        QVector<int> codes; auto mdfas = engine->buildAllMinDFA(*parsedPtr, codes);
        auto s = CodeGenerator::generateCombined(mdfas, codes, parsedPtr->alpha);
        txtGeneratedCode->setPlainText(s);
        QString base = ensureGenDir(); QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"); QString hash = currentRegexHash.isEmpty()? computeRegexHash(txtInputRegex->toPlainText()) : currentRegexHash;
        QString savePath = base+"/tiny_"+ts+"_"+hash.mid(0,12)+".cpp";
        QFile f(savePath); if(f.open(QIODevice::WriteOnly|QIODevice::Text)){ QTextStream o(&f); o<<s; f.close(); }
        currentCodePath = savePath; currentBinPath = base+"/bin/"+QFileInfo(savePath).completeBaseName();
        statusBar()->showMessage("组合扫描器代码已生成并保存到 generated/lex");
        return;
    }
    if(!lastMinPtr){ onTokenChanged(idx); if(!lastMinPtr){ statusBar()->showMessage("请先转换"); return; } }
    QMap<QString,int> codesMap; for(auto p: parsedPtr->tokens){ codesMap[p.rule.name]=p.rule.code; }
    auto s = engine->generateCode(*lastMinPtr, codesMap);
    txtGeneratedCode->setPlainText(s);
    statusBar()->showMessage("代码已生成");
}

void MainWindow::onRunLexerClicked(bool){
    if(!parsedPtr){ auto text = txtInputRegex->toPlainText(); auto rf = engine->lexFile(text); auto parsed = engine->parseFile(rf); if(parsed.tokens.isEmpty()){ statusBar()->showMessage("未找到Token定义"); return; } parsedPtr = new ParsedFile(parsed); }
    // 优先重用已生成代码文件；若正则改变或不存在，则生成新文件
    QString hashNow = computeRegexHash(txtInputRegex->toPlainText());
    if(currentCodePath.isEmpty() || currentRegexHash != hashNow || !QFileInfo::exists(currentCodePath)){
        QVector<int> codes; auto mdfas = engine->buildAllMinDFA(*parsedPtr, codes);
        auto s = CodeGenerator::generateCombined(mdfas, codes, parsedPtr->alpha);
        QString base = ensureGenDir(); QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"); QString savePath = base+"/tiny_"+ts+"_"+hashNow.mid(0,12)+".cpp";
        QFile f(savePath); if(f.open(QIODevice::WriteOnly|QIODevice::Text)){ QTextStream w(&f); w<<s; f.close(); }
        currentRegexHash = hashNow; currentCodePath = savePath; currentBinPath = base+"/bin/"+QFileInfo(savePath).completeBaseName();
    }
    QProcess proc; proc.start("clang++", QStringList()<<"-std=c++17"<<currentCodePath<<"-o"<<currentBinPath); proc.waitForFinished(); if(proc.exitStatus()!=QProcess::NormalExit || proc.exitCode()!=0){ txtLexResult->setPlainText(QString::fromUtf8(proc.readAllStandardError())); statusBar()->showMessage("编译失败"); return; }
    // 准备输入：优先选中的样例文件，其次文本框内容，最后使用内置示例
    QString src = txtSourceTiny->toPlainText();
    if(src.trimmed().isEmpty()){
        QString p1 = QCoreApplication::applicationDirPath()+"/../../tests/sample/tiny/tiny1.tny";
        QFile f1(p1);
        if(f1.open(QIODevice::ReadOnly|QIODevice::Text)){ QTextStream in(&f1); src = in.readAll(); f1.close(); }
        else { QString p2 = QCoreApplication::applicationDirPath()+"/tests/sample/tiny/tiny1.tny"; QFile f2(p2); if(f2.open(QIODevice::ReadOnly|QIODevice::Text)){ QTextStream in(&f2); src = in.readAll(); f2.close(); } }
        if(src.trimmed().isEmpty()){ src = QStringLiteral("read x;\n"); }
        txtSourceTiny->setPlainText(src);
    }
    QProcess run; QStringList args; if(!selectedSamplePath.isEmpty()) args<<selectedSamplePath; run.start(currentBinPath, args); if(args.isEmpty()){ run.write(src.toUtf8()); run.closeWriteChannel(); } run.waitForFinished(); auto output = QString::fromUtf8(run.readAllStandardOutput()); txtLexResult->setPlainText(output);
    QString outLex = QCoreApplication::applicationDirPath()+"/sample.lex"; QFile of(outLex); if(of.open(QIODevice::WriteOnly|QIODevice::Text)){ QTextStream o(&of); o<<output<<"\n"; of.close(); }
    if(output.contains("ERR")) statusBar()->showMessage("存在未识别的词法单元(ERR)，请检查正则与输入"); else statusBar()->showMessage("测试完成");
}

void MainWindow::onSaveRegexClicked(bool){
    auto path = QFileDialog::getSaveFileName(this, QStringLiteral("保存正则为"), QString(), QStringLiteral("Text (*.txt *.regex);;All (*)"));
    if(path.isEmpty()) return;
    QFile f(path);
    if(!f.open(QIODevice::WriteOnly | QIODevice::Text)){ statusBar()->showMessage("文件保存失败"); return; }
    QTextStream out(&f); out << txtInputRegex->toPlainText(); f.close(); statusBar()->showMessage("正则已保存");
}

void MainWindow::onLoadRegexClicked(bool){
    auto path = QFileDialog::getOpenFileName(this, QStringLiteral("选择正则文件"), QString(), QStringLiteral("Text (*.txt *.regex);;All (*)"));
    if(path.isEmpty()) return;
    QFile f(path);
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text)){ statusBar()->showMessage("文件打开失败"); return; }
    QTextStream in(&f); auto content = in.readAll(); txtInputRegex->setPlainText(content); statusBar()->showMessage("正则已加载");
}
void MainWindow::onPickSampleClicked(bool){
    QString root = QCoreApplication::applicationDirPath()+"/../../tests/sample";
    auto path = QFileDialog::getOpenFileName(this, QStringLiteral("选择样例文件"), root, QStringLiteral("Text (*.tny *.txt);;All (*)"));
    if(path.isEmpty()) return;
    QFile f(path);
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text)){ statusBar()->showMessage("样例文件打开失败"); return; }
    QTextStream in(&f); auto content = in.readAll(); txtSourceTiny->setPlainText(content); selectedSamplePath = path; statusBar()->showMessage("样例已加载");
}
void MainWindow::onCompileRunClicked(bool){
    if(!parsedPtr){ auto text = txtInputRegex->toPlainText(); auto rf = engine->lexFile(text); auto parsed = engine->parseFile(rf); if(parsed.tokens.isEmpty()){ statusBar()->showMessage("未找到Token定义"); return; } parsedPtr = new ParsedFile(parsed); }
    QVector<int> codes; auto mdfas = engine->buildAllMinDFA(*parsedPtr, codes);
    auto srcCombined = CodeGenerator::generateCombined(mdfas, codes, parsedPtr->alpha);
    QString outCpp = QCoreApplication::applicationDirPath()+"/gen_combined.cpp";
    QFile f(outCpp); if(!f.open(QIODevice::WriteOnly|QIODevice::Text)){ statusBar()->showMessage("生成代码写入失败"); return; } QTextStream w(&f); w<<srcCombined; f.close();
    QProcess proc; QString bin = QCoreApplication::applicationDirPath()+"/gen_combined_bin"; proc.start("clang++", QStringList()<<"-std=c++17"<<outCpp<<"-o"<<bin); proc.waitForFinished(); if(proc.exitStatus()!=QProcess::NormalExit || proc.exitCode()!=0){ txtLexResult->setPlainText(QString::fromUtf8(proc.readAllStandardError())); statusBar()->showMessage("编译失败"); return; }
    QStringList args; if(!selectedSamplePath.isEmpty()) args<<selectedSamplePath; QProcess run; run.start(bin, args); if(!selectedSamplePath.isEmpty()){ run.waitForFinished(); } else { run.write(txtSourceTiny->toPlainText().toUtf8()); run.closeWriteChannel(); run.waitForFinished(); }
    auto output = QString::fromUtf8(run.readAllStandardOutput()); txtLexResult->setPlainText(output);
    QString outLex = selectedSamplePath.isEmpty()? (QCoreApplication::applicationDirPath()+"/sample.lex") : (QFileInfo(selectedSamplePath).absolutePath()+"/"+QFileInfo(selectedSamplePath).completeBaseName()+".lex"); QFile lf(outLex); if(lf.open(QIODevice::WriteOnly|QIODevice::Text)){ QTextStream o(&lf); o<<output<<"\n"; lf.close(); }
    statusBar()->showMessage("生成器运行完成");
}
void MainWindow::onTokenChanged(int idx){
    if(!parsedPtr) return;
    if(idx==0){ fillAllNFA(); return; }
    if(idx-1<0 || idx-1>=parsedPtr->tokens.size()) return;
    auto pt = parsedPtr->tokens[idx-1];
    auto nfa = engine->buildNFA(pt.ast, parsedPtr->alpha);
    auto dfa = engine->buildDFA(nfa);
    auto mdfa = engine->buildMinDFA(dfa);
    if(lastMinPtr) delete lastMinPtr; lastMinPtr = new MinDFA(mdfa);
    auto tn = engine->nfaTable(nfa); fillTable(tblNFA, tn);
    auto td = engine->dfaTable(dfa); fillTable(tblDFA, td);
    auto tm = engine->minTable(mdfa); fillTable(tblMinDFA, tm);
}

void MainWindow::onTokenChangedDFA(int idx){
    if(!parsedPtr) return;
    if(idx==0){ fillAllDFA(); return; }
    if(idx-1<0 || idx-1>=parsedPtr->tokens.size()) return;
    auto pt = parsedPtr->tokens[idx-1];
    auto nfa = engine->buildNFA(pt.ast, parsedPtr->alpha);
    auto dfa = engine->buildDFA(nfa);
    auto mdfa = engine->buildMinDFA(dfa);
    auto td = engine->dfaTable(dfa); fillTable(tblDFA, td);
    auto tm = engine->minTable(mdfa); fillTable(tblMinDFA, tm);
}

void MainWindow::onTokenChangedMin(int idx){
    if(!parsedPtr) return;
    if(idx==0){ fillAllMin(); return; }
    if(idx-1<0 || idx-1>=parsedPtr->tokens.size()) return;
    auto pt = parsedPtr->tokens[idx-1];
    auto nfa = engine->buildNFA(pt.ast, parsedPtr->alpha);
    auto dfa = engine->buildDFA(nfa);
    auto mdfa = engine->buildMinDFA(dfa);
    auto tm = engine->minTable(mdfa); fillTable(tblMinDFA, tm);
}

static QVector<QString> unionSyms(const QVector<Tables>& tables, bool includeEps){ QSet<QString> s; for(const auto& t: tables){ for(int i=2;i<t.columns.size();++i){ QString c=t.columns[i]; if(includeEps || c!="#") s.insert(c); } } QVector<QString> v = QVector<QString>(s.begin(), s.end()); std::sort(v.begin(), v.end()); return v; }

void MainWindow::fillAllNFA(){
    QVector<Tables> parts; for(const auto& tok: parsedPtr->tokens){ auto nfa = engine->buildNFA(tok.ast, parsedPtr->alpha); parts.push_back(engine->nfaTable(nfa)); }
    auto syms = unionSyms(parts, true);
    Tables t; t.columns.clear(); t.columns.push_back("标记"); t.columns.push_back("状态 ID"); for(auto c: syms){ if(c!="#") t.columns.push_back(c); } t.columns.push_back("#");
    for(int k=0;k<parts.size();++k){ Tables pt = parts[k]; QVector<QString> sep; sep<<"Token"<<parsedPtr->tokens[k].rule.name; for(int i=2;i<t.columns.size();++i) sep<<QString(); t.rows.push_back(sep);
        for(const auto& row: pt.rows){ QVector<QString> newRow; newRow<<row[0]<<row[1]; for(int ci=2; ci<t.columns.size(); ++ci){ QString col=t.columns[ci]; int idx=-1; for(int j=2;j<pt.columns.size();++j){ if(pt.columns[j]==col){ idx=j; break; } } newRow<< (idx==-1?QString():row[idx]); }
            t.rows.push_back(newRow);
        }
    }
    fillTable(tblNFA, t);
}

void MainWindow::fillAllDFA(){
    QVector<Tables> parts; for(const auto& tok: parsedPtr->tokens){ auto nfa = engine->buildNFA(tok.ast, parsedPtr->alpha); auto dfa = engine->buildDFA(nfa); parts.push_back(engine->dfaTable(dfa)); }
    auto syms = unionSyms(parts, false);
    Tables t; t.columns.clear(); t.columns.push_back("标记"); t.columns.push_back("状态集合"); for(auto c: syms){ t.columns.push_back(c); }
    for(int k=0;k<parts.size();++k){ Tables pt = parts[k]; QVector<QString> sep; sep<<"Token"<<parsedPtr->tokens[k].rule.name; for(int i=2;i<t.columns.size();++i) sep<<QString(); t.rows.push_back(sep);
        for(const auto& row: pt.rows){ QVector<QString> newRow; newRow<<row[0]<<row[1]; for(int ci=2; ci<t.columns.size(); ++ci){ QString col=t.columns[ci]; int idx=-1; for(int j=2;j<pt.columns.size();++j){ if(pt.columns[j]==col){ idx=j; break; } } newRow<< (idx==-1?QString():row[idx]); }
            t.rows.push_back(newRow);
        }
    }
    fillTable(tblDFA, t);
}

void MainWindow::fillAllMin(){
    QVector<Tables> parts; for(const auto& tok: parsedPtr->tokens){ auto nfa = engine->buildNFA(tok.ast, parsedPtr->alpha); auto dfa = engine->buildDFA(nfa); auto mdfa = engine->buildMinDFA(dfa); parts.push_back(engine->minTable(mdfa)); }
    auto syms = unionSyms(parts, false);
    Tables t; t.columns.clear(); t.columns.push_back("标记"); t.columns.push_back("状态 ID"); for(auto c: syms){ t.columns.push_back(c); }
    for(int k=0;k<parts.size();++k){ Tables pt = parts[k]; QVector<QString> sep; sep<<"Token"<<parsedPtr->tokens[k].rule.name; for(int i=2;i<t.columns.size();++i) sep<<QString(); t.rows.push_back(sep);
        for(const auto& row: pt.rows){ QVector<QString> newRow; newRow<<row[0]<<row[1]; for(int ci=2; ci<t.columns.size(); ++ci){ QString col=t.columns[ci]; int idx=-1; for(int j=2;j<pt.columns.size();++j){ if(pt.columns[j]==col){ idx=j; break; } } newRow<< (idx==-1?QString():row[idx]); }
            t.rows.push_back(newRow);
        }
    }
    fillTable(tblMinDFA, t);
}
QString MainWindow::computeRegexHash(const QString& text){ auto h = QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Sha256); return QString(h.toHex()); }
QString MainWindow::ensureGenDir(){ QString base = QCoreApplication::applicationDirPath()+"/../../generated/lex"; QDir d(base); if(!d.exists()) d.mkpath("."); QDir b(base+"/bin"); if(!b.exists()) b.mkpath("."); return base; }
void MainWindow::onTabChanged(int idx){
    if(idx<0) return;
    auto w = tabs->widget(idx);
    if(!w) return;
    auto codeView = w->findChild<QPlainTextEdit*>("txtGeneratedCode");
    if(codeView && !currentCodePath.isEmpty()){
        QFile f(currentCodePath);
        if(f.open(QIODevice::ReadOnly|QIODevice::Text)){
            QTextStream in(&f);
            codeView->setPlainText(in.readAll());
            f.close();
        }
    }
}
