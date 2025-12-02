#include "SettingsController.h"
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include "../../components/SettingsDialog/SettingsDialog.h"

SettingsController::SettingsController(QMainWindow* parent) : QObject(parent) {}

void SettingsController::bind(QMainWindow* mw)
{
    auto mbar = mw->menuBar();
    auto mSettings = mbar->addMenu(QStringLiteral("设置"));
    auto actOpenSettings = mSettings->addAction(QStringLiteral("打开设置…"));
    QObject::connect(actOpenSettings, &QAction::triggered, [mw]() {
        SettingsDialog dlg(mw);
        dlg.exec();
    });
}

