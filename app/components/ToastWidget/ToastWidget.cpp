#include "ToastWidget.h"
#include <QHBoxLayout>
#include <QStyle>
#include <QScreen>
#include <QApplication>

ToastWidget::ToastWidget(Type type, QWidget* parent) : QWidget(parent), icon_(new QLabel), text_(new QLabel), close_(new QPushButton), timer_(new QTimer(this)), fade_(nullptr), slide_(nullptr), opacity_(nullptr), shadow_(nullptr), durationMs_(3000) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    auto lay = new QHBoxLayout(this);
    lay->setContentsMargins(12, 10, 8, 10);
    lay->setSpacing(8);
    icon_->setFixedSize(20, 20);
    icon_->setScaledContents(true);
    text_->setWordWrap(true);
    close_->setText("×");
    close_->setFixedSize(20, 20);
    close_->setFocusPolicy(Qt::NoFocus);
    lay->addWidget(icon_);
    lay->addWidget(text_, 1);
    lay->addWidget(close_);
    connect(close_, &QPushButton::clicked, this, &ToastWidget::closeAnimated);
    connect(timer_, &QTimer::timeout, this, &ToastWidget::closeAnimated);
    ensureEffects();
    applyType(type);
}

void ToastWidget::ensureEffects() {
    if (!opacity_) {
        opacity_ = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(opacity_);
    }
    if (!shadow_) {
        shadow_ = new QGraphicsDropShadowEffect(this);
        shadow_->setBlurRadius(16);
        shadow_->setOffset(0, 2);
        shadow_->setColor(QColor(0, 0, 0, 60));
        setGraphicsEffect(opacity_);
        opacity_->setOpacity(0.0);
    }
}

void ToastWidget::applyType(Type type) {
    QPalette pal = palette();
    QColor bg;
    QColor border;
    QColor fg;
    QIcon ico;
    if (type == Info) {
        bg = QColor("#E6F0FF");
        border = QColor("#2B78E4");
        fg = QColor("#1C3B70");
        ico = style()->standardIcon(QStyle::SP_MessageBoxInformation);
        durationMs_ = 3000;
    } else if (type == Warning) {
        bg = QColor("#FFF8E1");
        border = QColor("#F5A623");
        fg = QColor("#7A4B00");
        ico = style()->standardIcon(QStyle::SP_MessageBoxWarning);
        durationMs_ = 5000;
    } else {
        bg = QColor("#FFE9E9");
        border = QColor("#D0021B");
        fg = QColor("#680012");
        ico = style()->standardIcon(QStyle::SP_MessageBoxCritical);
        durationMs_ = 8000;
    }
    setStyleSheet(QString("QWidget{background:%1;border:1px solid %2;border-radius:8px;} QLabel{color:%3;} QPushButton{border:none;background:transparent;color:%3;}").arg(bg.name(), border.name(), fg.name()));
    setIcon(ico);
    text_->setPalette(pal);
}

void ToastWidget::setText(const QString& t) { text_->setText(t); }

void ToastWidget::setDuration(int ms) { durationMs_ = ms; }

void ToastWidget::setIcon(const QIcon& ico) { icon_->setPixmap(ico.pixmap(20, 20)); }

void ToastWidget::showAnimated() {
    opacity_->setOpacity(0.0);
    if (fade_) fade_->deleteLater();
    if (slide_) slide_->deleteLater();
    fade_ = new QPropertyAnimation(opacity_, "opacity", this);
    fade_->setDuration(220);
    fade_->setStartValue(0.0);
    fade_->setEndValue(1.0);
    fade_->setEasingCurve(QEasingCurve::OutCubic);
    slide_ = new QPropertyAnimation(this, "pos", this);
    slide_->setDuration(220);
    slide_->setEasingCurve(QEasingCurve::OutCubic);
    QPoint p = pos();
    slide_->setStartValue(QPoint(p.x(), p.y() - 12));
    slide_->setEndValue(p);
    show();
    fade_->start();
    slide_->start();
    timer_->start(durationMs_);
}

void ToastWidget::closeAnimated() {
    timer_->stop();
    if (fade_) fade_->stop();
    if (slide_) slide_->stop();
    auto f = new QPropertyAnimation(opacity_, "opacity", this);
    f->setDuration(180);
    f->setStartValue(opacity_->opacity());
    f->setEndValue(0.0);
    f->setEasingCurve(QEasingCurve::OutCubic);
    auto s = new QPropertyAnimation(this, "pos", this);
    s->setDuration(180);
    s->setEasingCurve(QEasingCurve::OutCubic);
    QPoint p = pos();
    s->setStartValue(p);
    s->setEndValue(QPoint(p.x(), p.y() - 12));
    connect(f, &QPropertyAnimation::finished, this, [this]() { emit closed(this); hide(); deleteLater(); });
    f->start();
    s->start();
}

void ToastWidget::resizeEvent(QResizeEvent* e) { QWidget::resizeEvent(e); }
