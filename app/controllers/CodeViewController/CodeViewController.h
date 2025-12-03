#pragma once
#include <QObject>
class QTabWidget;
class QWidget;
class MainWindow;

class CodeViewController : public QObject
{
    Q_OBJECT
   public:
    explicit CodeViewController(MainWindow* mw);
    void bind(QTabWidget* tabs);
   public slots:
    void onOuterTabChanged(int idx);

   private:
    MainWindow* mw_;
};
