#include "FirstFollowTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

FirstFollowTab::FirstFollowTab(QWidget* parent) : QWidget(parent)
{
    auto l = new QVBoxLayout(this);
    auto hFF = new QHBoxLayout;
    tblFirstSet = new QTableWidget; tblFirstSet->setObjectName("tblFirstSet");
    tblFollowSet = new QTableWidget; tblFollowSet->setObjectName("tblFollowSet");
    hFF->addWidget(tblFirstSet);
    hFF->addWidget(tblFollowSet);
    l->addLayout(hFF);
    auto hLR0 = new QHBoxLayout;
    btnExportLR0 = new QPushButton("导出"); btnExportLR0->setObjectName("btnExportLR0");
    btnPreviewLR0 = new QPushButton("预览LR(0)项集DFA"); btnPreviewLR0->setObjectName("btnPreviewLR0");
    edtGraphDpiLR0 = new QLineEdit; edtGraphDpiLR0->setObjectName("edtGraphDpiLR0"); edtGraphDpiLR0->setPlaceholderText("DPI(默认150)");
    hLR0->addWidget(btnExportLR0);
    hLR0->addWidget(btnPreviewLR0);
    hLR0->addWidget(edtGraphDpiLR0);
    l->addLayout(hLR0);
}

