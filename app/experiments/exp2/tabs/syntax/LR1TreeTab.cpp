#include "LR1TreeTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

LR1TreeTab::LR1TreeTab(QWidget* parent) : QWidget(parent)
{
    auto v = new QVBoxLayout(this);
    auto h = new QHBoxLayout;
    btnPreview = new QPushButton("预览语法树(LR1)");
    btnPreview->setObjectName("btnPreviewLR1Tree");
    btnExportDot = new QPushButton("导出DOT(LR1)");
    btnExportDot->setObjectName("btnExportLR1TreeDot");
    h->addWidget(btnPreview);
    h->addWidget(btnExportDot);
    v->addLayout(h);
    view = new QGraphicsView;
    view->setObjectName("viewLR1Tree");
    v->addWidget(view);
}

