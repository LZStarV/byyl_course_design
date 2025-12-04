#include "LR1ProcessTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

LR1ProcessTab::LR1ProcessTab(QWidget* parent) : QWidget(parent)
{
    auto v = new QVBoxLayout(this);
    auto h = new QHBoxLayout;
    btnLoadDefault = new QPushButton("加载默认源程序");
    btnLoadDefault->setObjectName("btnLoadDefaultLR1");
    btnPickSource = new QPushButton("选择源程序文件");
    btnPickSource->setObjectName("btnPickSourceLR1");
    btnRunLR1 = new QPushButton("运行LR(1)分析（每句）");
    btnRunLR1->setObjectName("btnRunLR1Process");
    h->addWidget(btnLoadDefault);
    h->addWidget(btnPickSource);
    h->addWidget(btnRunLR1);
    v->addLayout(h);
    auto l1 = new QLabel("源程序");
    txtSourceView = new QPlainTextEdit;
    txtSourceView->setObjectName("txtSourceViewLR1");
    v->addWidget(l1);
    v->addWidget(txtSourceView);
    auto l2 = new QLabel("Token 序列");
    txtTokensView = new QPlainTextEdit;
    txtTokensView->setObjectName("txtTokensViewLR1");
    txtTokensView->setReadOnly(true);
    v->addWidget(l2);
    v->addWidget(txtTokensView);
    tblProcess = new QTableWidget;
    tblProcess->setObjectName("tblLR1Process");
    v->addWidget(tblProcess);
}

