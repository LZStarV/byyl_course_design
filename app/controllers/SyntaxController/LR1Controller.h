#pragma once
#include <QObject>
#include <QWidget>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include "../../mainwindow.h"
#include "../../services/NotificationService/NotificationService.h"
#include "../../services/DotService/DotService.h"
#include "../../../src/Engine.h"
#include "../../../src/syntax/GrammarParser.h"
#include "../../../src/syntax/LR1.h"
#include "../../../src/syntax/LR1Parser.h"
#include "../../../src/syntax/DotGenerator.h"
#include "../../components/ExportGraphButton/ExportGraphButton.h"

class LR1Controller : public QObject
{
    Q_OBJECT
   public:
    explicit LR1Controller(MainWindow* mw, Engine* engine, NotificationService* notify);
    void bind(QWidget* exp2Page);

   private:
    MainWindow*          mw_;
    Engine*              engine_;
    NotificationService* notify_;
    DotService*          dotSvc_;
    QWidget*             page_ = nullptr;
    QString              lastSourcePath_;
    QString              lastDot_;

    void                    loadDefault();
    void                    pickSource();
    void                    runLR1Process();
    void                    previewTree();
    void                    setupExportButton();
    static QVector<QString> splitTokens(const QString& s);
    static void             fillProcessTable(QTableWidget*             tbl,
                                             const QVector<QString>&   cols,
                                             const QVector<ParseStep>& steps);
};
