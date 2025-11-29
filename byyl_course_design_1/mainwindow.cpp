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
#include "../src/core/Engine.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    engine = new Engine();
    setupUiCustom();
}

MainWindow::~MainWindow()
{
    delete engine;
    delete ui;
}

void MainWindow::setupUiCustom(){
    tabs = new QTabWidget(ui->centralwidget);
    auto v = new QVBoxLayout; ui->centralwidget->setLayout(v); v->addWidget(tabs);
    auto w1 = new QWidget; auto l1 = new QVBoxLayout(w1); txtInputRegex = new QTextEdit; txtInputRegex->setObjectName("txtInputRegex");
    auto h1 = new QHBoxLayout; btnLoadRegex = new QPushButton("从文件加载"); btnLoadRegex->setObjectName("btnLoadRegex"); btnStartConvert = new QPushButton("转换"); btnStartConvert->setObjectName("btnStartConvert"); h1->addWidget(btnLoadRegex); h1->addWidget(btnStartConvert);
    l1->addWidget(txtInputRegex); l1->addLayout(h1); tabs->addTab(w1, "正则编辑");
    auto w2 = new QWidget; auto l2 = new QVBoxLayout(w2); tblNFA = new QTableWidget; tblNFA->setObjectName("tblNFA"); l2->addWidget(tblNFA); tabs->addTab(w2, "NFA状态表");
    auto w3 = new QWidget; auto l3 = new QVBoxLayout(w3); tblDFA = new QTableWidget; tblDFA->setObjectName("tblDFA"); l3->addWidget(tblDFA); tabs->addTab(w3, "DFA状态表");
    auto w4 = new QWidget; auto l4 = new QVBoxLayout(w4); tblMinDFA = new QTableWidget; tblMinDFA->setObjectName("tblMinDFA"); btnGenCode = new QPushButton("生成代码"); btnGenCode->setObjectName("btnGenCode"); l4->addWidget(tblMinDFA); l4->addWidget(btnGenCode); tabs->addTab(w4, "最小化DFA");
    auto w5 = new QWidget; auto l5 = new QVBoxLayout(w5); txtGeneratedCode = new QPlainTextEdit; txtGeneratedCode->setObjectName("txtGeneratedCode"); l5->addWidget(txtGeneratedCode); tabs->addTab(w5, "代码查看");
    auto w6 = new QWidget; auto l6 = new QVBoxLayout(w6); auto h6 = new QHBoxLayout; txtSourceTiny = new QPlainTextEdit; txtSourceTiny->setObjectName("txtSourceTiny"); txtLexResult = new QPlainTextEdit; txtLexResult->setObjectName("txtLexResult"); btnRunLexer = new QPushButton("运行词法分析"); btnRunLexer->setObjectName("btnRunLexer"); h6->addWidget(txtSourceTiny); h6->addWidget(txtLexResult); l6->addLayout(h6); l6->addWidget(btnRunLexer); tabs->addTab(w6, "测试与验证");
    connect(btnStartConvert,&QPushButton::clicked,this,&MainWindow::onConvertClicked);
    connect(btnGenCode,&QPushButton::clicked,this,&MainWindow::onGenCodeClicked);
    connect(btnRunLexer,&QPushButton::clicked,this,&MainWindow::onRunLexerClicked);
    connect(btnLoadRegex,&QPushButton::clicked,this,&MainWindow::onLoadRegexClicked);
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
    auto pt = parsed.tokens.first(); auto nfa = engine->buildNFA(pt.ast, parsed.alpha); auto dfa = engine->buildDFA(nfa); auto mdfa = engine->buildMinDFA(dfa); lastMinPtr = new MinDFA(mdfa);
    auto tn = engine->nfaTable(nfa); fillTable(tblNFA, tn);
    auto td = engine->dfaTable(dfa); fillTable(tblDFA, td);
    auto tm = engine->minTable(mdfa); fillTable(tblMinDFA, tm);
    statusBar()->showMessage("转换成功");
}

void MainWindow::onGenCodeClicked(bool){
    if(!lastMinPtr){ statusBar()->showMessage("请先转换"); return; }
    auto parsed = *parsedPtr; QMap<QString,int> codes; for(auto p: parsed.tokens){ codes[p.rule.name]=p.rule.code; }
    auto s = engine->generateCode(*lastMinPtr, codes); txtGeneratedCode->setPlainText(s); statusBar()->showMessage("代码已生成");
}

void MainWindow::onRunLexerClicked(bool){
    if(!lastMinPtr || !parsedPtr){ statusBar()->showMessage("请先生成"); return; }
    auto src = txtSourceTiny->toPlainText(); auto code = parsedPtr->tokens.first().rule.code; auto res = engine->run(*lastMinPtr, src, code); txtLexResult->setPlainText(res); statusBar()->showMessage("测试完成");
}

void MainWindow::onLoadRegexClicked(bool){
    auto path = QFileDialog::getOpenFileName(this, QStringLiteral("选择正则文件"), QString(), QStringLiteral("Text (*.txt *.regex);;All (*)"));
    if(path.isEmpty()) return;
    QFile f(path);
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text)){ statusBar()->showMessage("文件打开失败"); return; }
    QTextStream in(&f); auto content = in.readAll(); txtInputRegex->setPlainText(content); statusBar()->showMessage("正则已加载");
}
