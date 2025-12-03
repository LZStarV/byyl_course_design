#include "TestValidationController.h"
#include <QWidget>
#include <QPushButton>
#include "../../mainwindow.h"

TestValidationController::TestValidationController(MainWindow* mw) : mw_(mw) {}

void TestValidationController::bind(QWidget* testPage)
{
    auto btnPick   = testPage->findChild<QPushButton*>("btnPickSample");
    auto btnRun    = testPage->findChild<QPushButton*>("btnRunLexer");
    auto btnSaveAs = testPage->findChild<QPushButton*>("btnSaveLexResultAs");
    if (btnPick)
        QObject::connect(btnPick, &QPushButton::clicked, mw_, &MainWindow::pickSample);
    if (btnRun)
        QObject::connect(btnRun, &QPushButton::clicked, mw_, &MainWindow::runLexer);
    if (btnSaveAs)
        QObject::connect(btnSaveAs, &QPushButton::clicked, mw_, &MainWindow::saveLexAs);
}
