#pragma once
#include <QWidget>
class QComboBox;
class QTableWidget;
class QLineEdit;
class QPushButton;

class MinDFAViewTab : public QWidget
{
    Q_OBJECT
   public:
    explicit MinDFAViewTab(QWidget* parent = nullptr);
    QComboBox*    cmbTokensMin;
    QTableWidget* tblMinDFA;
    QLineEdit*    edtGraphDpiMin;
    QPushButton*  btnExportMin;
    QPushButton*  btnPreviewMin;
    QPushButton*  btnGenCode;
};
