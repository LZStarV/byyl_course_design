#include "Exp2Page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QGraphicsView>

Exp2Page::Exp2Page(QWidget* parent) : QWidget(parent)
{
    auto v = new QVBoxLayout(this);
    auto bar = new QHBoxLayout;
    btnBack = new QPushButton("← 返回");
    bar->addWidget(btnBack);
    bar->addStretch(1);
    v->addLayout(bar);
    tabSyntax = new QTabWidget;
    auto wGrammar = new QWidget;
    auto lGrammar = new QVBoxLayout(wGrammar);
    lblGrammarTitle = new QLabel("文法分析（LL(1)）");
    lGrammar->addWidget(lblGrammarTitle);
    txtInputGrammar = new QTextEdit;
    txtInputGrammar->setObjectName("txtInputGrammar");
    auto hG = new QHBoxLayout;
    btnLoadGrammar = new QPushButton("加载文法文件");
    btnLoadGrammar->setObjectName("btnLoadGrammar");
    btnParseGrammar = new QPushButton("解析文法并计算");
    btnParseGrammar->setObjectName("btnParseGrammar");
    hG->addWidget(btnLoadGrammar);
    hG->addWidget(btnParseGrammar);
    tblFirstSet = new QTableWidget;
    tblFirstSet->setObjectName("tblFirstSet");
    tblFollowSet = new QTableWidget;
    tblFollowSet->setObjectName("tblFollowSet");
    tblParsingTable = new QTableWidget;
    tblParsingTable->setObjectName("tblParsingTable");
    lGrammar->addWidget(txtInputGrammar);
    lGrammar->addLayout(hG);
    lGrammar->addWidget(tblFirstSet);
    lGrammar->addWidget(tblFollowSet);
    lGrammar->addWidget(tblParsingTable);
    tabSyntax->addTab(wGrammar, "文法分析");
    auto wTree = new QWidget;
    auto lTree = new QVBoxLayout(wTree);
    lblSyntaxTreeTitle = new QLabel("语法树预览与导出");
    lTree->addWidget(lblSyntaxTreeTitle);
    auto hT = new QHBoxLayout;
    btnExportSyntaxDot = new QPushButton("导出DOT");
    btnExportSyntaxDot->setObjectName("btnExportSyntaxDot");
    btnPreviewSyntaxTree = new QPushButton("预览语法树");
    btnPreviewSyntaxTree->setObjectName("btnPreviewSyntaxTree");
    btnRunSyntaxAnalysis = new QPushButton("运行语法分析");
    btnRunSyntaxAnalysis->setObjectName("btnRunSyntaxAnalysis");
    hT->addWidget(btnExportSyntaxDot);
    hT->addWidget(btnPreviewSyntaxTree);
    hT->addWidget(btnRunSyntaxAnalysis);
    viewSyntaxTree = new QGraphicsView;
    viewSyntaxTree->setObjectName("viewSyntaxTree");
    lTree->addLayout(hT);
    lTree->addWidget(viewSyntaxTree);
    tabSyntax->addTab(wTree, "语法树");
    auto wCode = new QWidget;
    auto lCode = new QVBoxLayout(wCode);
    lblSyntaxCodeTitle = new QLabel("语法分析器代码查看");
    lCode->addWidget(lblSyntaxCodeTitle);
    txtSyntaxGeneratedCode = new QPlainTextEdit;
    txtSyntaxGeneratedCode->setObjectName("txtSyntaxGeneratedCode");
    txtSyntaxGeneratedCode->setReadOnly(true);
    lCode->addWidget(txtSyntaxGeneratedCode);
    tabSyntax->addTab(wCode, "代码查看");
    v->addWidget(tabSyntax);
    connect(btnBack, &QPushButton::clicked, this, [this]() { emit requestBack(); });
}
