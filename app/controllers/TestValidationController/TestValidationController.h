#pragma once
#include <QObject>
class QWidget;
class MainWindow;

class TestValidationController : public QObject
{
    Q_OBJECT
   public:
    explicit TestValidationController(MainWindow* mw);
    void bind(QWidget* testPage);

   private:
    MainWindow* mw_;
};
