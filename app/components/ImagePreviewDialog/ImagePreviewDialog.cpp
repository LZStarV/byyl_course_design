#include "ImagePreviewDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>

ImagePreviewDialog::ImagePreviewDialog(QWidget* parent) : QDialog(parent)
{
    auto v      = new QVBoxLayout(this);
    auto h      = new QHBoxLayout;
    btnZoomIn_  = new QPushButton("缩放+");
    btnZoomOut_ = new QPushButton("缩放-");
    btnFit_     = new QPushButton("适应窗口");
    btnReset_   = new QPushButton("100%");
    h->addWidget(btnZoomIn_);
    h->addWidget(btnZoomOut_);
    h->addWidget(btnFit_);
    h->addWidget(btnReset_);
    v->addLayout(h);
    scene_ = new QGraphicsScene(this);
    view_  = new QGraphicsView(scene_);
    view_->setDragMode(QGraphicsView::ScrollHandDrag);
    v->addWidget(view_);
    connect(btnZoomIn_, &QPushButton::clicked, [this]() { view_->scale(1.2, 1.2); });
    connect(btnZoomOut_, &QPushButton::clicked, [this]() { view_->scale(1.0 / 1.2, 1.0 / 1.2); });
    connect(btnFit_,
            &QPushButton::clicked,
            [this]()
            {
                view_->resetTransform();
                view_->fitInView(scene_->itemsBoundingRect(), Qt::KeepAspectRatio);
            });
    connect(btnReset_, &QPushButton::clicked, [this]() { view_->resetTransform(); });
    resize(900, 700);
}

bool ImagePreviewDialog::loadImage(const QString& pngPath)
{
    QPixmap px(pngPath);
    if (px.isNull())
    {
        return false;
    }
    scene_->clear();
    scene_->addPixmap(px);
    return true;
}
