#pragma once
#include <QObject>
#include <QString>
#include <QVector>
#include <QLineEdit>
#include <QTableWidget>
#include <QGraphicsView>
#include "../../../src/syntax/Grammar.h"
#include "../../../src/syntax/LL1.h"
class QWidget;
class QTableWidget;
class QTextEdit;
class QPushButton;
class MainWindow;
class Engine;
struct LL1Info;
class NotificationService;

class SyntaxController : public QObject
{
    Q_OBJECT
public:
    SyntaxController(MainWindow* mw, Engine* engine, NotificationService* notify);
    void bind(QWidget* exp2Page);
    void loadGrammar();
    void parseGrammar();
    void runSyntaxAnalysis();
    void exportDot();
    void previewTree();
    void previewLR0();
    void exportLR0Dot();
    void exportAstDot();
private:
    MainWindow* mw_;
    Engine* engine_;
    NotificationService* notify_;
    QWidget* page_ = nullptr;
    struct Grammar grammar_;
    struct LL1Info ll1_;
    bool hasGrammar_ = false;
    static bool renderDotFromContentLocal(const QString& dotContent, QString& outPngPath, int dpi);
};

