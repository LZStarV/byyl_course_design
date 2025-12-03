#pragma once
#include <QWidget>
class QPushButton;
class QLabel;
class QFrame;
class QHBoxLayout;
class QVBoxLayout;
class HomePage : public QWidget
{
    Q_OBJECT
   public:
    explicit HomePage(QWidget* parent = nullptr);
   signals:
    void openExp1();
    void openExp2();

   private:
    QFrame*      banner;
    QLabel*      lblTitle;
    QLabel*      lblSubtitle;
    QFrame*      authorCard;
    QLabel*      lblAuthor;
    QLabel*      lblStuId;
    QPushButton* btnExp1;
    QPushButton* btnExp2;
};
