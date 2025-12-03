#pragma once
#include <QWidget>
class QLabel; class QTextEdit; class QPushButton;
class GrammarEditorTab : public QWidget
{
    Q_OBJECT
public:
    explicit GrammarEditorTab(QWidget* parent=nullptr);
    QLabel* lblGrammarTitle;
    QTextEdit* txtInputGrammar;
    QPushButton* btnLoadGrammar;
    QPushButton* btnParseGrammar;
};

