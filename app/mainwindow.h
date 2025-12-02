#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "services/NotificationService.h"
#include "../src/syntax/Grammar.h"
#include "../src/syntax/LL1.h"
class QTabWidget;
class QTextEdit;
class QPlainTextEdit;
class QTableWidget;
class QPushButton;
class QComboBox;
class QStackedWidget;
class QLineEdit;
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
    void previewImage(const QString& pngPath, const QString& title);

   private:
    Ui::MainWindow* ui;
    QStackedWidget* stack;
    QTabWidget*     tabs;
    QTextEdit*      txtInputRegex;
    QTableWidget*   tblNFA;
    QTableWidget*   tblDFA;
    QTableWidget*   tblMinDFA;
    QComboBox*      cmbTokens;
    QComboBox*      cmbTokensDFA;
    QComboBox*      cmbTokensMin;
    QPushButton*    btnExportNFA;
    QPushButton*    btnPreviewNFA;
    QPushButton*    btnExportDFA;
    QPushButton*    btnPreviewDFA;
    QPushButton*    btnExportMin;
    QPushButton*    btnPreviewMin;
    QLineEdit*      edtGraphDpiNfa;
    QLineEdit*      edtGraphDpiDfa;
    QLineEdit*      edtGraphDpiMin;
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
    Grammar         currentGrammar;
    LL1Info         currentLL1;
    bool            hasGrammar = false;
    NotificationService notify;
    class SyntaxController* syntaxController;
    void            setupUiCustom();
    void            fillTable(QTableWidget* tbl, const Tables& t);
    void            fillAllNFA();
    void            fillAllDFA();
    void            fillAllMin();
    QString         computeRegexHash(const QString& text);
    QString         ensureGenDir();
    QString         ensureGraphDir();
    bool            renderDotWithGraphviz(const QString& dotPath,
                                          const QString& outPath,
                                          const QString& fmt,
                                          int            dpi);
    QString         pickDotSavePath(const QString& suggestedName);
    bool            renderDotFromContent(const QString& dotContent,
                                         QString&       outPngPath,
                                         int            dpi);
    bool            renderDotToFile(const QString& dotContent,
                                    const QString& outPath,
                                    const QString& fmt,
                                    int            dpi);
    void            showImagePreview(const QString& pngPath, const QString& title);
    QString         pickImageSavePath(const QString& suggestedName, const QString& fmt);
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
    void onExportNFAClicked(bool);
    void onPreviewNFAClicked(bool);
    void onExportDFAClicked(bool);
    void onPreviewDFAClicked(bool);
    void onExportMinClicked(bool);
    void onPreviewMinClicked(bool);
    void onExportNFADot();
    void onExportNFAImage();
    void onExportDFADot();
    void onExportDFAImage();
    void onExportMinDot();
    void onExportMinImage();
    void onLoadGrammarClicked(bool);
    void onParseGrammarClicked(bool);
    void onRunSyntaxAnalysisClicked(bool);
    void onExportSyntaxDotClicked(bool);
    void onPreviewSyntaxTreeClicked(bool);
    void onSaveLexResultClicked(bool);
    void onSaveLexResultAsClicked(bool);
};
#endif  // MAINWINDOW_H
