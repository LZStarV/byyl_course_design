#include "CodeViewTab.h"
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>

CodeViewTab::CodeViewTab(QWidget* parent) : QWidget(parent)
{
    auto l           = new QVBoxLayout(this);
    txtGeneratedCode = new QPlainTextEdit;
    txtGeneratedCode->setObjectName("txtGeneratedCode");
    btnCompileRun = new QPushButton("编译并运行生成器");
    btnCompileRun->setObjectName("btnCompileRun");
    l->addWidget(txtGeneratedCode);
    l->addWidget(btnCompileRun);
}
