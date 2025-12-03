#pragma once
#include <QWidget>
class QComboBox;
class QTableWidget;
class QLineEdit;
class QPushButton;

class DFAViewTab : public QWidget
{
    Q_OBJECT
   public:
    explicit DFAViewTab(QWidget* parent = nullptr);
    QComboBox*    cmbTokensDFA;
    QTableWidget* tblDFA;
    QLineEdit*    edtGraphDpiDfa;
    QPushButton*  btnExportDFA;
    QPushButton*  btnPreviewDFA;
};
