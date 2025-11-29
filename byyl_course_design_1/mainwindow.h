#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class QTabWidget;
class QTextEdit;
class QPlainTextEdit;
class QTableWidget;
class QPushButton;
class Engine;
struct ParsedFile;
struct MinDFA;
struct Tables;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTabWidget* tabs;
    QTextEdit* txtInputRegex;
    QTableWidget* tblNFA;
    QTableWidget* tblDFA;
    QTableWidget* tblMinDFA;
    QPlainTextEdit* txtGeneratedCode;
    QPlainTextEdit* txtSourceTiny;
    QPlainTextEdit* txtLexResult;
    QPushButton* btnStartConvert;
    QPushButton* btnGenCode;
    QPushButton* btnRunLexer;
    QPushButton* btnLoadRegex;
    Engine* engine;
    ParsedFile* parsedPtr;
    MinDFA* lastMinPtr;
    void setupUiCustom();
    void fillTable(QTableWidget* tbl, const Tables& t);
private slots:
    void onConvertClicked(bool);
    void onGenCodeClicked(bool);
    void onRunLexerClicked(bool);
    void onLoadRegexClicked(bool);
};
#endif // MAINWINDOW_H
