#pragma once
#include <QWidget>
class QPlainTextEdit;
class QPushButton;

class CodeViewTab : public QWidget
{
    Q_OBJECT
   public:
    explicit CodeViewTab(QWidget* parent = nullptr);
    QPlainTextEdit* txtGeneratedCode;
    QPushButton*    btnCompileRun;
};
