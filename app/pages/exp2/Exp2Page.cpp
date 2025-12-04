#include "Exp2Page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include "../../experiments/exp2/tabs/grammar/GrammarEditorTab.h"
#include "../../experiments/exp2/tabs/grammar/FirstFollowTab.h"
#include "../../experiments/exp2/tabs/lr/LRTab.h"
#include "../../experiments/exp2/tabs/syntax/LR1ProcessTab.h"
#include "../../experiments/exp2/tabs/syntax/LR1TreeTab.h"
#include "../../experiments/exp2/tabs/syntax/LR1ProcessTab.h"
#include "../../experiments/exp2/tabs/syntax/LR1TreeTab.h"

Exp2Page::Exp2Page(QWidget* parent) : QWidget(parent)
{
    auto v   = new QVBoxLayout;
    auto bar = new QHBoxLayout;
    btnBack  = new QPushButton("← 返回");
    bar->addWidget(btnBack);
    bar->addStretch(1);
    v->addLayout(bar);
    tabSyntax     = new QTabWidget;
    auto wGrammar = new GrammarEditorTab;
    tabSyntax->addTab(wGrammar, "文法分析");
    auto wFF = new FirstFollowTab;
    tabSyntax->addTab(wFF, "First 与 Follow 集");
    auto wLR = new LRTab;
    tabSyntax->addTab(wLR, "LR分析");
    // 移除旧的“语法树”页签，使用 LR(1) 语法树页签
    auto wLR1Proc = new LR1ProcessTab;
    tabSyntax->addTab(wLR1Proc, "LR(1)分析过程");
    auto wLR1Tree = new LR1TreeTab;
    tabSyntax->addTab(wLR1Tree, "LR(1)语法树");
    // 按实验要求移除“代码查看”页签
    v->addWidget(tabSyntax);
    setLayout(v);
    connect(btnBack, &QPushButton::clicked, this, [this]() { emit requestBack(); });
}
