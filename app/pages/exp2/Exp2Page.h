#pragma once
#include <QWidget>
class QPushButton;
class QLabel;
class QTabWidget;
class QTextEdit;
class QPlainTextEdit;
class QTableWidget;
class QGraphicsView;
class QLineEdit;
class Exp2Page : public QWidget
{
    Q_OBJECT
   public:
    explicit Exp2Page(QWidget* parent = nullptr);
   signals:
    void requestBack();
   private:
    QPushButton* btnBack;
    QTabWidget*  tabSyntax;
    // tabs 封装了控件，保留对象名用于 findChild 查询
};
