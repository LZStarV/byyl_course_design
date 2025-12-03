#include "SyntaxTreeTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QGraphicsView>

SyntaxTreeTab::SyntaxTreeTab(QWidget* parent) : QWidget(parent)
{
    auto l             = new QVBoxLayout(this);
    lblSyntaxTreeTitle = new QLabel("语法树预览与导出");
    l->addWidget(lblSyntaxTreeTitle);
    auto tokensLabel = new QLabel("当前 Token 序列");
    tokensLabel->setObjectName("lblTokensViewSyntax");
    txtTokensViewSyntax = new QPlainTextEdit;
    txtTokensViewSyntax->setObjectName("txtTokensViewSyntax");
    txtTokensViewSyntax->setReadOnly(true);
    l->addWidget(tokensLabel);
    l->addWidget(txtTokensViewSyntax);
    auto hT            = new QHBoxLayout;
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
    l->addLayout(hT);
    l->addWidget(viewSyntaxTree);
}
