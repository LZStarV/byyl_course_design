#include "HomePage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFont>
#include <QFrame>
#include <QSpacerItem>
#include <QScrollArea>

HomePage::HomePage(QWidget* parent) : QWidget(parent)
{
    auto main = new QVBoxLayout(this);
    main->setContentsMargins(0, 0, 0, 0);
    auto scroll = new QScrollArea(this);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidgetResizable(true);
    auto content = new QWidget;
    auto v       = new QVBoxLayout(content);
    v->setContentsMargins(24, 24, 24, 24);
    scroll->setWidget(content);
    main->addWidget(scroll);
    banner = new QFrame;
    banner->setObjectName("banner");
    auto vb = new QVBoxLayout(banner);
    vb->setContentsMargins(24, 24, 24, 24);
    lblTitle    = new QLabel("编译原理课程设计程序");
    lblSubtitle = new QLabel("Compiler Principles Course Design Program");
    QFont f1    = lblTitle->font();
    f1.setPointSize(f1.pointSize() + 8);
    f1.setBold(true);
    lblTitle->setFont(f1);
    lblTitle->setStyleSheet("color:#222");
    lblSubtitle->setStyleSheet("color:#666");
    vb->addWidget(lblTitle);
    vb->addWidget(lblSubtitle);
    banner->setStyleSheet("#banner{background:transparent;}");
    v->addWidget(banner);

    authorCard = new QFrame;
    authorCard->setObjectName("authorCard");
    auto ha = new QHBoxLayout(authorCard);
    ha->setContentsMargins(16, 16, 16, 16);
    auto col1      = new QVBoxLayout;
    auto col2      = new QVBoxLayout;
    auto authTitle = new QLabel("作者");
    auto stuTitle  = new QLabel("学号");
    lblAuthor      = new QLabel("林展星");
    lblStuId       = new QLabel("20232131082");
    QFont fti      = authTitle->font();
    fti.setBold(true);
    authTitle->setFont(fti);
    stuTitle->setFont(fti);
    QFont fval = lblAuthor->font();
    fval.setPointSize(fval.pointSize() + 2);
    lblAuthor->setFont(fval);
    lblStuId->setFont(fval);
    authTitle->setStyleSheet("color:#888;");
    stuTitle->setStyleSheet("color:#888;");
    col1->addWidget(authTitle);
    col1->addWidget(lblAuthor);
    col2->addWidget(stuTitle);
    col2->addWidget(lblStuId);
    ha->addLayout(col1);
    ha->addSpacing(32);
    ha->addLayout(col2);
    authorCard->setStyleSheet("#authorCard{border:1px solid #c9cdd4;}");
    v->addSpacing(12);
    v->addWidget(authorCard);
    auto buttons = new QHBoxLayout;
    btnExp1      = new QPushButton("实验一");
    btnExp2      = new QPushButton("实验二");
    btnExp1->setMinimumHeight(44);
    btnExp2->setMinimumHeight(44);
    buttons->addStretch(1);
    buttons->addWidget(btnExp1);
    buttons->addSpacing(16);
    buttons->addWidget(btnExp2);
    buttons->addStretch(1);
    v->addSpacing(16);
    v->addLayout(buttons);
    v->addStretch(2);
    connect(btnExp1, &QPushButton::clicked, this, [this]() { emit openExp1(); });
    connect(btnExp2, &QPushButton::clicked, this, [this]() { emit openExp2(); });
}
