#pragma once
#include <QWidget>
#include <QPushButton>
#include <QGraphicsView>

class LR1TreeTab : public QWidget
{
    Q_OBJECT
   public:
    explicit LR1TreeTab(QWidget* parent = nullptr);
    QPushButton*  btnPreview;
    QPushButton*  btnExportDot;
    QGraphicsView* view;
};

