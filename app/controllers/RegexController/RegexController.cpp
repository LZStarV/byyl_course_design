#include "RegexController.h"
#include <QWidget>
#include <QPushButton>
#include "../../mainwindow.h"

RegexController::RegexController(MainWindow* mw) : mw_(mw) {}

void RegexController::bind(QWidget* regexPage)
{
    auto btnLoad = regexPage->findChild<QPushButton*>("btnLoadRegex");
    auto btnSave = regexPage->findChild<QPushButton*>("btnSaveRegex");
    auto btnConvert = regexPage->findChild<QPushButton*>("btnStartConvert");
    if (btnLoad) QObject::connect(btnLoad, &QPushButton::clicked, mw_, &MainWindow::loadRegex);
    if (btnSave) QObject::connect(btnSave, &QPushButton::clicked, mw_, &MainWindow::saveRegex);
    if (btnConvert) QObject::connect(btnConvert, &QPushButton::clicked, mw_, &MainWindow::startConvert);
}

