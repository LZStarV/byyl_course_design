#include "TestValidationTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>

TestValidationTab::TestValidationTab(QWidget* parent) : QWidget(parent)
{
    auto l        = new QVBoxLayout(this);
    auto h        = new QHBoxLayout;
    auto left     = new QVBoxLayout;
    auto right    = new QVBoxLayout;
    auto lblSrc   = new QLabel("源程序输入");
    auto lblOut   = new QLabel("Token 编码输出");
    txtSourceTiny = new QPlainTextEdit;
    txtSourceTiny->setObjectName("txtSourceTiny");
    txtLexResult = new QPlainTextEdit;
    txtLexResult->setObjectName("txtLexResult");
    left->addWidget(lblSrc);
    left->addWidget(txtSourceTiny);
    right->addWidget(lblOut);
    right->addWidget(txtLexResult);
    h->addLayout(left);
    h->addLayout(right);
    btnPickSample = new QPushButton("选择样例文件");
    btnPickSample->setObjectName("btnPickSample");
    btnRunLexer = new QPushButton("运行词法分析");
    btnRunLexer->setObjectName("btnRunLexer");
    btnSaveLexResultAs = new QPushButton("另存为...");
    btnSaveLexResultAs->setObjectName("btnSaveLexResultAs");
    l->addLayout(h);
    auto hBtns = new QHBoxLayout;
    hBtns->addWidget(btnPickSample);
    hBtns->addWidget(btnRunLexer);
    hBtns->addWidget(btnSaveLexResultAs);
    l->addLayout(hBtns);
}
