#include "LR1TreeTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

LR1TreeTab::LR1TreeTab(QWidget* parent) : QWidget(parent)
{
    auto v     = new QVBoxLayout(this);
    auto h     = new QHBoxLayout;
    btnPreview = new QPushButton("预览语法树(LR1)");
    btnPreview->setObjectName("btnPreviewLR1Tree");
    exportBtn = new ExportGraphButton("导出语法树");
    exportBtn->setObjectName("exportBtnLR1Tree");
    auto lblDpi = new QLabel("分辨率DPI");
    edtGraphDpi = new QLineEdit;
    edtGraphDpi->setObjectName("edtGraphDpiLR1");
    edtGraphDpi->setPlaceholderText("DPI(默认150)");
    h->addWidget(btnPreview);
    h->addWidget(exportBtn);
    h->addWidget(lblDpi);
    h->addWidget(edtGraphDpi);
    v->addLayout(h);
    tblProcess = new QTableWidget;
    tblProcess->setObjectName("tblLR1Process");
    tblProcess->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tblProcess->setSelectionBehavior(QAbstractItemView::SelectRows);
    v->addWidget(tblProcess);
}
