#pragma once
#include <QObject>
class QWidget;
class MainWindow;

class RegexController : public QObject
{
    Q_OBJECT
   public:
    explicit RegexController(MainWindow* mw);
    void bind(QWidget* regexPage);

   private:
    MainWindow* mw_;
};
