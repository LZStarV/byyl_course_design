#include "Exp2Page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include "../../experiments/exp2/tabs/GrammarEditorTab.h"
#include "../../experiments/exp2/tabs/FirstFollowTab.h"
#include "../../experiments/exp2/tabs/SyntaxTreeTab.h"
#include "../../experiments/exp2/tabs/SyntaxCodeViewTab.h"

Exp2Page::Exp2Page(QWidget* parent) : QWidget(parent)
{
    auto v = new QVBoxLayout;
    auto bar = new QHBoxLayout;
    btnBack = new QPushButton("← 返回");
    bar->addWidget(btnBack);
    bar->addStretch(1);
    v->addLayout(bar);
    tabSyntax = new QTabWidget;
    auto wGrammar = new GrammarEditorTab;
    tabSyntax->addTab(wGrammar, "文法分析");
    auto wFF = new FirstFollowTab;
    tabSyntax->addTab(wFF, "First 与 Follow 集");
    auto wTree = new SyntaxTreeTab;
    tabSyntax->addTab(wTree, "语法树");
    auto wCode = new SyntaxCodeViewTab;
    tabSyntax->addTab(wCode, "代码查看");
    v->addWidget(tabSyntax);
    setLayout(v);
    connect(btnBack, &QPushButton::clicked, this, [this]() { emit requestBack(); });
}
