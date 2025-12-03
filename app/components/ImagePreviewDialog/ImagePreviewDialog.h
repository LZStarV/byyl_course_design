#pragma once
#include <QDialog>
class QGraphicsView;
class QGraphicsScene;
class QPushButton;

class ImagePreviewDialog : public QDialog
{
    Q_OBJECT
   public:
    explicit ImagePreviewDialog(QWidget* parent = nullptr);
    bool loadImage(const QString& pngPath);

   private:
    QGraphicsView*  view_;
    QGraphicsScene* scene_;
    QPushButton*    btnZoomIn_;
    QPushButton*    btnZoomOut_;
    QPushButton*    btnFit_;
    QPushButton*    btnReset_;
};
