#pragma once
#include <QWidget>
class QPushButton;
class QVBoxLayout;
class Exp1Page : public QWidget
{
    Q_OBJECT
   public:
    explicit Exp1Page(QWidget* parent = nullptr);
    QWidget* contentWidget() const;
   signals:
    void requestBack();

   private:
    QPushButton* btnBack;
    QWidget*     content;
    QVBoxLayout* vroot;
};
