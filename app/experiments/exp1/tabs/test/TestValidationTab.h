#pragma once
#include <QWidget>
class QPlainTextEdit;
class QPushButton;

class TestValidationTab : public QWidget
{
    Q_OBJECT
   public:
    explicit TestValidationTab(QWidget* parent = nullptr);
    QPlainTextEdit* txtSourceTiny;
    QPlainTextEdit* txtLexResult;
    QPushButton*    btnPickSample;
    QPushButton*    btnRunLexer;
    QPushButton*    btnSaveLexResultAs;
};
