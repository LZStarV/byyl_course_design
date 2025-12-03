#pragma once
#include <QWidget>
class QTableWidget; class QPushButton; class QLineEdit;
class FirstFollowTab : public QWidget
{
    Q_OBJECT
public:
    explicit FirstFollowTab(QWidget* parent=nullptr);
    QTableWidget* tblFirstSet;
    QTableWidget* tblFollowSet;
    QPushButton* btnExportLR0;
    QPushButton* btnPreviewLR0;
    QLineEdit* edtGraphDpiLR0;
};

