#include "Exp1Page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

Exp1Page::Exp1Page(QWidget* parent) : QWidget(parent)
{
    vroot    = new QVBoxLayout(this);
    auto bar = new QHBoxLayout;
    btnBack  = new QPushButton("← 返回");
    bar->addWidget(btnBack);
    bar->addStretch(1);
    vroot->addLayout(bar);
    content = new QWidget;
    auto v  = new QVBoxLayout(content);
    v->setContentsMargins(0, 0, 0, 0);
    vroot->addWidget(content);
    connect(btnBack, &QPushButton::clicked, this, [this]() { emit requestBack(); });
}

QWidget* Exp1Page::contentWidget() const
{
    return content;
}
