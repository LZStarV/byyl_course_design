#pragma once
#include <QWidget>
class QLabel;
class QPlainTextEdit;
class SyntaxCodeViewTab : public QWidget
{
    Q_OBJECT
   public:
    explicit SyntaxCodeViewTab(QWidget* parent = nullptr);
    QLabel*         lblSyntaxCodeTitle;
    QPlainTextEdit* txtSyntaxGeneratedCode;
};
