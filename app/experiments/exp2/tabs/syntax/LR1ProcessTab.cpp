#include "LR1ProcessTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

LR1ProcessTab::LR1ProcessTab(QWidget* parent) : QWidget(parent)
{
    auto v         = new QVBoxLayout(this);
    auto hBtns     = new QHBoxLayout;
    btnLoadDefault = new QPushButton("加载默认源程序");
    btnLoadDefault->setObjectName("btnLoadDefaultLR1");
    btnPickSource = new QPushButton("选择源程序文件");
    btnPickSource->setObjectName("btnPickSourceLR1");
    btnRunLR1 = new QPushButton("运行LR(1)分析（每句）");
    btnRunLR1->setObjectName("btnRunLR1Process");
    hBtns->addWidget(btnLoadDefault);
    hBtns->addWidget(btnPickSource);
    hBtns->addWidget(btnRunLR1);
    v->addLayout(hBtns);
    auto h = new QHBoxLayout;
    // 左：源程序
    auto left     = new QVBoxLayout;
    auto l1       = new QLabel("源程序");
    txtSourceView = new QPlainTextEdit;
    txtSourceView->setObjectName("txtSourceViewLR1");
    left->addWidget(l1);
    left->addWidget(txtSourceView);
    // 中：Token 序列
    auto mid      = new QVBoxLayout;
    auto l2       = new QLabel("Token 序列");
    txtTokensView = new QPlainTextEdit;
    txtTokensView->setObjectName("txtTokensViewLR1");
    txtTokensView->setReadOnly(true);
    mid->addWidget(l2);
    mid->addWidget(txtTokensView);
    // 右：文法（只读）
    auto right     = new QVBoxLayout;
    auto l3        = new QLabel("当前文法");
    txtGrammarView = new QPlainTextEdit;
    txtGrammarView->setObjectName("txtGrammarViewLR1");
    txtGrammarView->setReadOnly(true);
    right->addWidget(l3);
    right->addWidget(txtGrammarView);
    h->addLayout(left);
    h->addLayout(mid);
    h->addLayout(right);
    v->addLayout(h);
}
