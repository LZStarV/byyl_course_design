#include "ToastManager.h"
#include <QApplication>
#include <QScreen>
#include <QStyle>
#include <QWindow>

ToastManager& ToastManager::instance() {
    static ToastManager inst;
    return inst;
}

ToastManager::ToastManager() : anchor_(nullptr), limit_(3), marginRight_(24), marginTop_(24), spacing_(10), width_(360) {}

void ToastManager::setAnchor(QWidget* w) { anchor_ = w; }

void ToastManager::setLimit(int n) { limit_ = qMax(1, n); }

void ToastManager::setMargins(int right, int top) { marginRight_ = right; marginTop_ = top; }

void ToastManager::setSpacing(int s) { spacing_ = s; }

void ToastManager::showInfo(const QString& text) { enqueue(ToastWidget::Info, text, 3000); }

void ToastManager::showWarning(const QString& text) { enqueue(ToastWidget::Warning, text, 5000); }

void ToastManager::showError(const QString& text) { enqueue(ToastWidget::Error, text, 8000); }

void ToastManager::enqueue(ToastWidget::Type t, const QString& text, int ms) {
    queue_.enqueue({t, text, ms});
    tryShowNext();
}

QRect ToastManager::anchorAvailableGeometry() const {
    QScreen* sc = nullptr;
    if (anchor_ && anchor_->windowHandle()) sc = anchor_->windowHandle()->screen();
    if (!sc) sc = QApplication::primaryScreen();
    return sc ? sc->availableGeometry() : QRect(0, 0, 1280, 800);
}

void ToastManager::tryShowNext() {
    while (!queue_.isEmpty() && active_.size() < limit_) {
        auto it = queue_.dequeue();
        auto* tw = new ToastWidget(it.type);
        tw->setText(it.text);
        tw->setDuration(it.duration);
        QIcon ico;
        if (it.type == ToastWidget::Info) ico = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
        else if (it.type == ToastWidget::Warning) ico = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
        else ico = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
        tw->setIcon(ico);
        connect(tw, &ToastWidget::closed, this, [this](ToastWidget* self) {
            active_.removeAll(self);
            layoutToasts();
            tryShowNext();
        });
        tw->setFixedWidth(width_);
        active_.append(tw);
        layoutToasts();
        tw->showAnimated();
    }
}

void ToastManager::layoutToasts() {
    QRect geo = anchorAvailableGeometry();
    int x = geo.right() - marginRight_ - width_;
    int y = geo.top() + marginTop_;
    int curY = y;
    for (auto* t : active_) {
        QSize sz = t->sizeHint();
        int h = qMax(40, sz.height());
        t->setGeometry(x, curY, width_, h);
        curY += h + spacing_;
    }
}
