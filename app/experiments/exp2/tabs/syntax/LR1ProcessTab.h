#pragma once
#include <QWidget>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QTableWidget>

class LR1ProcessTab : public QWidget
{
    Q_OBJECT
   public:
    explicit LR1ProcessTab(QWidget* parent = nullptr);
    QPushButton*     btnLoadDefault;
    QPushButton*     btnPickSource;
    QPushButton*     btnRunLR1;
    QPlainTextEdit*  txtSourceView;
    QPlainTextEdit*  txtTokensView;
    QTableWidget*    tblProcess;
};

