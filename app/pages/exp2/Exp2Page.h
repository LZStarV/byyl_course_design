#pragma once
#include <QWidget>
class QPushButton;
class QLabel;
class QTabWidget;
class QTextEdit;
class QPlainTextEdit;
class QTableWidget;
class QGraphicsView;
class Exp2Page : public QWidget
{
    Q_OBJECT
   public:
    explicit Exp2Page(QWidget* parent = nullptr);
   signals:
    void requestBack();
   private:
    QPushButton* btnBack;
    QTabWidget*  tabSyntax;
    QLabel*      lblGrammarTitle;
    QTextEdit*   txtInputGrammar;
    QPushButton* btnLoadGrammar;
    QPushButton* btnParseGrammar;
    QTableWidget* tblFirstSet;
    QTableWidget* tblFollowSet;
    QTableWidget* tblParsingTable;
    QLabel*      lblSyntaxTreeTitle;
    QGraphicsView* viewSyntaxTree;
    QPushButton* btnExportSyntaxDot;
    QPushButton* btnPreviewSyntaxTree;
    QPushButton* btnRunSyntaxAnalysis;
    QLabel*      lblSyntaxCodeTitle;
    QPlainTextEdit* txtSyntaxGeneratedCode;
};
