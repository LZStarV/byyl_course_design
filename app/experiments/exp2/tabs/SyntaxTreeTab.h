#pragma once
#include <QWidget>
class QLabel; class QPlainTextEdit; class QPushButton; class QGraphicsView;
class SyntaxTreeTab : public QWidget
{
    Q_OBJECT
public:
    explicit SyntaxTreeTab(QWidget* parent=nullptr);
    QLabel* lblSyntaxTreeTitle;
    QPlainTextEdit* txtTokensViewSyntax;
    QPushButton* btnExportSyntaxDot;
    QPushButton* btnPreviewSyntaxTree;
    QPushButton* btnRunSyntaxAnalysis;
    QGraphicsView* viewSyntaxTree;
};

