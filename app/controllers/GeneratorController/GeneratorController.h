#pragma once
#include <QObject>
class QWidget;
class MainWindow;
class Engine;
class NotificationService;

class GeneratorController : public QObject
{
    Q_OBJECT
public:
    explicit GeneratorController(MainWindow* mw, Engine* engine, NotificationService* notify);
    void bind(QWidget* regexTab, QWidget* codeViewTab);
    void convert();
    void generateCode();
    void compileAndRun();
private:
    MainWindow* mw_;
    Engine* engine_;
    NotificationService* notify_;
    QWidget* regexTab_ = nullptr;
    QWidget* codeTab_ = nullptr;
};

