#pragma once
#include <QWidget>
class QComboBox;
class QTableWidget;
class QLineEdit;
class QPushButton;

class NFAViewTab : public QWidget
{
    Q_OBJECT
   public:
    explicit NFAViewTab(QWidget* parent = nullptr);
    QComboBox*    cmbTokens;
    QTableWidget* tblNFA;
    QLineEdit*    edtGraphDpiNfa;
    QPushButton*  btnExportNFA;
    QPushButton*  btnPreviewNFA;
};
