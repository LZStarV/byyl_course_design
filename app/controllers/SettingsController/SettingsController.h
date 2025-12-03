#pragma once
#include <QObject>
class QMainWindow;

class SettingsController : public QObject
{
    Q_OBJECT
   public:
    explicit SettingsController(QMainWindow* parent = nullptr);
    void bind(QMainWindow* mw);
};
