#pragma once
#include <QWidget>
class QPushButton;
class QLabel;
class QTabWidget;
class QTextEdit;
class QPlainTextEdit;
class QTableWidget;
class QGraphicsView;
class QLineEdit;
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
    QLabel*      lblSyntaxTreeTitle;
    QGraphicsView* viewSyntaxTree;
    QPushButton* btnExportSyntaxDot;
    QPushButton* btnPreviewSyntaxTree;
    QPushButton* btnRunSyntaxAnalysis;
    QLabel*      lblSyntaxCodeTitle;
    QPlainTextEdit* txtSyntaxGeneratedCode;
    QWidget*     wLR0;
    QPushButton* btnExportLR0;
    QPushButton* btnPreviewLR0;
    QLineEdit*   edtGraphDpiLR0;
    QPlainTextEdit* txtTokensViewSyntax;
};
