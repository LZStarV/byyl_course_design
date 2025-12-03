#pragma once
#include <QWidget>
class QTextEdit;
class QPushButton;

class RegexEditorTab : public QWidget
{
    Q_OBJECT
   public:
    explicit RegexEditorTab(QWidget* parent = nullptr);
    QTextEdit*   txtInputRegex;
    QPushButton* btnLoadRegex;
    QPushButton* btnSaveRegex;
    QPushButton* btnStartConvert;
};
