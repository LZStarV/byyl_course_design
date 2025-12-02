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
#include <QLineEdit>

Exp2Page::Exp2Page(QWidget* parent) : QWidget(parent)
{
    auto v = new QVBoxLayout;
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
    lGrammar->addWidget(txtInputGrammar);
    lGrammar->addLayout(hG);
    tabSyntax->addTab(wGrammar, "文法分析");
    auto wFF = new QWidget;
    auto lFF = new QVBoxLayout(wFF);
    auto hFF = new QHBoxLayout;
    tblFirstSet = new QTableWidget; tblFirstSet->setObjectName("tblFirstSet");
    tblFollowSet = new QTableWidget; tblFollowSet->setObjectName("tblFollowSet");
    hFF->addWidget(tblFirstSet);
    hFF->addWidget(tblFollowSet);
    lFF->addLayout(hFF);
    // 预测分析表页签移除（课程任务未要求展示预测表）
    // LR(0) DFA 页签
    // LR(0) 控件放到 First & Follow 页签底部
    auto hLR0 = new QHBoxLayout;
    btnExportLR0 = new QPushButton("导出");
    btnExportLR0->setObjectName("btnExportLR0");
    btnPreviewLR0 = new QPushButton("预览LR(0)项集DFA");
    btnPreviewLR0->setObjectName("btnPreviewLR0");
    edtGraphDpiLR0 = new QLineEdit;
    edtGraphDpiLR0->setObjectName("edtGraphDpiLR0");
    edtGraphDpiLR0->setPlaceholderText("DPI(默认150)");
    hLR0->addWidget(btnExportLR0);
    hLR0->addWidget(btnPreviewLR0);
    hLR0->addWidget(edtGraphDpiLR0);
    // 将 LR(0) 导出/预览控件置于 First & Follow 页签底部
    lFF->addLayout(hLR0);
    tabSyntax->addTab(wFF, "First 与 Follow 集");
    // 语法树页签置于 LR(0) 之后，并显示当前 Token 序列
    auto wTree = new QWidget;
    auto lTree = new QVBoxLayout(wTree);
    lblSyntaxTreeTitle = new QLabel("语法树预览与导出");
    lTree->addWidget(lblSyntaxTreeTitle);
    auto tokensLabel = new QLabel("当前 Token 序列");
    tokensLabel->setObjectName("lblTokensViewSyntax");
    txtTokensViewSyntax = new QPlainTextEdit;
    txtTokensViewSyntax->setObjectName("txtTokensViewSyntax");
    txtTokensViewSyntax->setReadOnly(true);
    lTree->addWidget(tokensLabel);
    lTree->addWidget(txtTokensViewSyntax);
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
    setLayout(v);
    connect(btnBack, &QPushButton::clicked, this, [this]() { emit requestBack(); });
}
