#ifndef TOASTMANAGER_H
#define TOASTMANAGER_H

#include <QObject>
#include <QQueue>
#include <QPointer>
#include <QScreen>
#include <QWidget>
#include "ToastWidget.h"

struct ToastItem {
    ToastWidget::Type type;
    QString text;
    int duration;
};

class ToastManager : public QObject {
    Q_OBJECT
public:
    static ToastManager& instance();
    void setAnchor(QWidget* w);
    void showInfo(const QString& text);
    void showWarning(const QString& text);
    void showError(const QString& text);
    void setLimit(int n);
    void setMargins(int right, int top);
    void setSpacing(int s);
private:
    ToastManager();
    void enqueue(ToastWidget::Type t, const QString& text, int ms);
    void tryShowNext();
    void layoutToasts();
    QRect anchorAvailableGeometry() const;
private:
    QPointer<QWidget> anchor_;
    QList<ToastWidget*> active_;
    QQueue<ToastItem> queue_;
    int limit_;
    int marginRight_;
    int marginTop_;
    int spacing_;
    int width_;
};

#endif

