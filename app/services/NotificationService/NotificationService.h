#pragma once
#include <QString>
class QMainWindow;

class NotificationService
{
   public:
    explicit NotificationService(QMainWindow* mw = nullptr);
    void setMainWindow(QMainWindow* mw);
    void info(const QString& text);
    void warning(const QString& text);
    void error(const QString& text);

   private:
    QMainWindow* mw_;
};
