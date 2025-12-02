#include "NotificationService.h"
#include <QMainWindow>
#include <QStatusBar>
#include "../ui/ToastManager.h"

NotificationService::NotificationService(QMainWindow* mw) : mw_(mw) {}

void NotificationService::setMainWindow(QMainWindow* mw) { mw_ = mw; }

void NotificationService::info(const QString& text)
{
    if (mw_ && mw_->statusBar()) mw_->statusBar()->showMessage(text);
    ToastManager::instance().showInfo(text);
}

void NotificationService::warning(const QString& text)
{
    if (mw_ && mw_->statusBar()) mw_->statusBar()->showMessage(text);
    ToastManager::instance().showWarning(text);
}

void NotificationService::error(const QString& text)
{
    if (mw_ && mw_->statusBar()) mw_->statusBar()->showMessage(text);
    ToastManager::instance().showError(text);
}
