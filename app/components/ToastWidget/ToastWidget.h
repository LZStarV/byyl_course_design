#ifndef TOASTWIDGET_H
#define TOASTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>

class ToastWidget : public QWidget
{
    Q_OBJECT
   public:
    enum Type
    {
        Info,
        Warning,
        Error
    };
    explicit ToastWidget(Type type, QWidget* parent = nullptr);
    void setText(const QString& t);
    void setDuration(int ms);
    void setIcon(const QIcon& ico);
    void showAnimated();
   public slots:
    void closeAnimated();
   signals:
    void closed(ToastWidget* self);

   protected:
    void resizeEvent(QResizeEvent* e) override;

   private:
    void                       applyType(Type type);
    void                       ensureEffects();
    QLabel*                    icon_;
    QLabel*                    text_;
    QPushButton*               close_;
    QTimer*                    timer_;
    QPropertyAnimation*        fade_;
    QPropertyAnimation*        slide_;
    QGraphicsOpacityEffect*    opacity_;
    QGraphicsDropShadowEffect* shadow_;
    int                        durationMs_;
};

class InfoToast : public ToastWidget
{
   public:
    explicit InfoToast(QWidget* parent = nullptr) : ToastWidget(ToastWidget::Info, parent) {}
};
class WarningToast : public ToastWidget
{
   public:
    explicit WarningToast(QWidget* parent = nullptr) : ToastWidget(ToastWidget::Warning, parent) {}
};
class ErrorToast : public ToastWidget
{
   public:
    explicit ErrorToast(QWidget* parent = nullptr) : ToastWidget(ToastWidget::Error, parent) {}
};

#endif
