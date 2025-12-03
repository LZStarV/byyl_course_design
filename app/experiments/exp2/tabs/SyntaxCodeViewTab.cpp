#include "SyntaxCodeViewTab.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>

SyntaxCodeViewTab::SyntaxCodeViewTab(QWidget* parent) : QWidget(parent)
{
    auto l = new QVBoxLayout(this);
    lblSyntaxCodeTitle = new QLabel("语法分析器代码查看");
    l->addWidget(lblSyntaxCodeTitle);
    txtSyntaxGeneratedCode = new QPlainTextEdit; txtSyntaxGeneratedCode->setObjectName("txtSyntaxGeneratedCode"); txtSyntaxGeneratedCode->setReadOnly(true);
    l->addWidget(txtSyntaxGeneratedCode);
}

