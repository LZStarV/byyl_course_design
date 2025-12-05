#include "LR1ProcessTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

LR1ProcessTab::LR1ProcessTab(QWidget* parent) : QWidget(parent)
{
    auto v         = new QVBoxLayout(this);
    auto hBtns     = new QHBoxLayout;
    btnLoadDefault = new QPushButton("加载默认正则表达式");
    btnLoadDefault->setObjectName("btnLoadDefaultLR1");
    cmbPickSource = new QComboBox();
    cmbPickSource->addItem(QStringLiteral("导入正则表达式"));
    cmbPickSource->addItem(QStringLiteral("导入Token序列"));
    // 语义动作通过独立按钮导入，不再放在下拉中
    cmbPickSource->setObjectName("cmbPickSourceLR1");
    btnRunLR1 = new QPushButton("运行LR(1)分析（每句）");
    btnRunLR1->setObjectName("btnRunLR1Process");

    // 设置按钮最小宽度并使其可扩展
    btnLoadDefault->setMinimumWidth(150);
    btnRunLR1->setMinimumWidth(150);
    btnLoadDefault->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btnRunLR1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    hBtns->addWidget(cmbPickSource);
    hBtns->addWidget(btnLoadDefault);
    // 新增导入语义动作按钮
    auto btnLoadSemantic = new QPushButton("导入语义动作");
    btnLoadSemantic->setObjectName("btnLoadSemanticActions");
    hBtns->addWidget(btnLoadSemantic);
    hBtns->addWidget(btnRunLR1);
    hBtns->setSpacing(10);  // 设置按钮间间距
    v->addLayout(hBtns);
    auto h = new QHBoxLayout;
    // 左：源程序
    auto left     = new QVBoxLayout;
    auto l1       = new QLabel("正则表达式");
    txtSourceView = new QPlainTextEdit;
    txtSourceView->setObjectName("txtSourceViewLR1");
    txtSourceView->setReadOnly(true);
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
    // 右：语义动作（只读）
    auto right           = new QVBoxLayout;
    auto l3              = new QLabel("当前语义动作");
    auto txtSemanticView = new QPlainTextEdit;
    txtSemanticView->setObjectName("txtSemanticViewLR1");
    txtSemanticView->setReadOnly(true);
    right->addWidget(l3);
    right->addWidget(txtSemanticView);
    h->addLayout(left);
    h->addLayout(mid);
    h->addLayout(right);
    v->addLayout(h);
}
