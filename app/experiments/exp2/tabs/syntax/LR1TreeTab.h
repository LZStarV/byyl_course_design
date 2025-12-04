#pragma once
#include <QWidget>
#include <QPushButton>
#include <QGraphicsView>
#include <QTableWidget>
#include <QLineEdit>
#include "../../../../components/ExportGraphButton/ExportGraphButton.h"

class LR1TreeTab : public QWidget
{
    Q_OBJECT
   public:
    explicit LR1TreeTab(QWidget* parent = nullptr);
    QPushButton*       btnPreview;
    ExportGraphButton* exportBtn;
    QTableWidget*      tblProcess;
    QLineEdit*         edtGraphDpi;
};
