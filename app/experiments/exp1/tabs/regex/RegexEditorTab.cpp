#include "RegexEditorTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>

RegexEditorTab::RegexEditorTab(QWidget* parent) : QWidget(parent)
{
    auto l        = new QVBoxLayout(this);
    txtInputRegex = new QTextEdit;
    txtInputRegex->setObjectName("txtInputRegex");
    auto h       = new QHBoxLayout;
    btnLoadRegex = new QPushButton("从文件加载");
    btnLoadRegex->setObjectName("btnLoadRegex");
    btnSaveRegex = new QPushButton("保存正则");
    btnSaveRegex->setObjectName("btnSaveRegex");
    btnStartConvert = new QPushButton("转换");
    btnStartConvert->setObjectName("btnStartConvert");
    h->addWidget(btnLoadRegex);
    h->addWidget(btnSaveRegex);
    h->addWidget(btnStartConvert);
    l->addWidget(txtInputRegex);
    l->addLayout(h);
}
