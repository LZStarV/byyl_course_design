#include "MinDFAViewTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>

MinDFAViewTab::MinDFAViewTab(QWidget* parent) : QWidget(parent)
{
    auto l       = new QVBoxLayout(this);
    auto hSel    = new QHBoxLayout;
    auto lbl     = new QLabel("选择Token");
    cmbTokensMin = new QComboBox;
    cmbTokensMin->setObjectName("cmbTokensMin");
    hSel->addWidget(lbl);
    hSel->addWidget(cmbTokensMin);
    auto hTools  = new QHBoxLayout;
    btnExportMin = new QPushButton("导出(MinDFA)");
    btnExportMin->setObjectName("btnExportMin");
    btnPreviewMin = new QPushButton("预览(MinDFA)");
    btnPreviewMin->setObjectName("btnPreviewMin");
    edtGraphDpiMin = new QLineEdit;
    edtGraphDpiMin->setObjectName("edtGraphDpiMin");
    edtGraphDpiMin->setPlaceholderText("DPI(默认150)");
    hTools->addWidget(btnExportMin);
    hTools->addWidget(btnPreviewMin);
    hTools->addWidget(new QLabel("分辨率DPI"));
    hTools->addWidget(edtGraphDpiMin);
    tblMinDFA = new QTableWidget;
    tblMinDFA->setObjectName("tblMinDFA");
    btnGenCode = new QPushButton("生成代码");
    btnGenCode->setObjectName("btnGenCode");
    l->addLayout(hSel);
    l->addLayout(hTools);
    l->addWidget(tblMinDFA);
    l->addWidget(btnGenCode);
}
