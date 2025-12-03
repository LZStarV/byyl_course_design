#include "DFAViewTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>

DFAViewTab::DFAViewTab(QWidget* parent) : QWidget(parent)
{
    auto l       = new QVBoxLayout(this);
    auto hSel    = new QHBoxLayout;
    auto lbl     = new QLabel("选择Token");
    cmbTokensDFA = new QComboBox;
    cmbTokensDFA->setObjectName("cmbTokensDFA");
    hSel->addWidget(lbl);
    hSel->addWidget(cmbTokensDFA);
    auto hTools  = new QHBoxLayout;
    btnExportDFA = new QPushButton("导出(DFA)");
    btnExportDFA->setObjectName("btnExportDFA");
    btnPreviewDFA = new QPushButton("预览(DFA)");
    btnPreviewDFA->setObjectName("btnPreviewDFA");
    edtGraphDpiDfa = new QLineEdit;
    edtGraphDpiDfa->setObjectName("edtGraphDpiDfa");
    edtGraphDpiDfa->setPlaceholderText("DPI(默认150)");
    hTools->addWidget(btnExportDFA);
    hTools->addWidget(btnPreviewDFA);
    hTools->addWidget(new QLabel("分辨率DPI"));
    hTools->addWidget(edtGraphDpiDfa);
    tblDFA = new QTableWidget;
    tblDFA->setObjectName("tblDFA");
    l->addLayout(hSel);
    l->addLayout(hTools);
    l->addWidget(tblDFA);
}
