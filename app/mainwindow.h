#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class QTabWidget;
class QTextEdit;
class QPlainTextEdit;
class QTableWidget;
class QPushButton;
class QComboBox;
class Engine;
struct ParsedFile;
struct MinDFA;
struct Tables;

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

/**
 * \brief 应用主窗口
 * 承载词法处理引擎的可视化管线与交互：编辑规则、构建与展示
 * NFA/DFA/MinDFA，
 * 生成与编译运行扫描器，并提供样例加载与执行。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

   public:
    /** \brief 构造主窗口并初始化界面与信号槽 */
    MainWindow(QWidget* parent = nullptr);
    /** \brief 释放界面资源与关联对象 */
    ~MainWindow();

   private:
    Ui::MainWindow* ui;
    QTabWidget*     tabs;
    QTextEdit*      txtInputRegex;
    QTableWidget*   tblNFA;
    QTableWidget*   tblDFA;
    QTableWidget*   tblMinDFA;
    QComboBox*      cmbTokens;
    QComboBox*      cmbTokensDFA;
    QComboBox*      cmbTokensMin;
    QPlainTextEdit* txtGeneratedCode;
    QPlainTextEdit* txtSourceTiny;
    QPlainTextEdit* txtLexResult;
    QPushButton*    btnStartConvert;
    QPushButton*    btnGenCode;
    QPushButton*    btnCompileRun;
    QPushButton*    btnRunLexer;
    QPushButton*    btnLoadRegex;
    QPushButton*    btnSaveRegex;
    QPushButton*    btnPickSample;
    QString         selectedSamplePath;
    QString         currentRegexHash;
    QString         currentCodePath;
    QString         currentBinPath;
    Engine*         engine;
    ParsedFile*     parsedPtr;
    MinDFA*         lastMinPtr;
    void            setupUiCustom();
    void            fillTable(QTableWidget* tbl, const Tables& t);
    void            fillAllNFA();
    void            fillAllDFA();
    void            fillAllMin();
    QString         computeRegexHash(const QString& text);
    QString         ensureGenDir();
   private slots:
    void onConvertClicked(bool);
    void onGenCodeClicked(bool);
    void onCompileRunClicked(bool);
    void onRunLexerClicked(bool);
    void onLoadRegexClicked(bool);
    void onSaveRegexClicked(bool);
    void onTokenChanged(int);
    void onTokenChangedDFA(int);
    void onTokenChangedMin(int);
    void onPickSampleClicked(bool);
    void onTabChanged(int);
};
#endif  // MAINWINDOW_H
