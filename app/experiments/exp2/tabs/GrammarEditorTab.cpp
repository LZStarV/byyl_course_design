#include "GrammarEditorTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>

GrammarEditorTab::GrammarEditorTab(QWidget* parent) : QWidget(parent)
{
    auto l = new QVBoxLayout(this);
    lblGrammarTitle = new QLabel("文法分析（LL(1)）");
    l->addWidget(lblGrammarTitle);
    txtInputGrammar = new QTextEdit;
    txtInputGrammar->setObjectName("txtInputGrammar");
    auto h = new QHBoxLayout;
    btnLoadGrammar = new QPushButton("加载文法文件");
    btnLoadGrammar->setObjectName("btnLoadGrammar");
    btnParseGrammar = new QPushButton("解析文法并计算");
    btnParseGrammar->setObjectName("btnParseGrammar");
    h->addWidget(btnLoadGrammar);
    h->addWidget(btnParseGrammar);
    l->addWidget(txtInputGrammar);
    l->addLayout(h);
}

