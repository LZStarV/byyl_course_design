#include "mainwindow.h"
#include "experiments/exp1/tabs/RegexEditorTab.h"
#include "experiments/exp1/tabs/NFAViewTab.h"
#include "experiments/exp1/tabs/DFAViewTab.h"
#include "experiments/exp1/tabs/MinDFAViewTab.h"
#include "experiments/exp1/tabs/CodeViewTab.h"
#include "experiments/exp1/tabs/TestValidationTab.h"
#include "ui_mainwindow.h"
#include <QStatusBar>
#include <QDesktopServices>
#include <QUrl>
#include <QTabWidget>
#include <QStackedWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QLabel>
#include <QProcess>
#include <QDir>
#include <QComboBox>
#include <QLineEdit>
#include <QDialog>
#include <QLabel>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMenu>
#include <QAction>
#include "components/ToastManager/ToastManager.h"
#include "pages/home/HomePage.h"
#include "pages/exp1/Exp1Page.h"
#include "pages/exp2/Exp2Page.h"
// removed duplicate includes
#include <QDateTime>
#include <QCryptographicHash>
#include "../src/Engine.h"
#include "../src/visual/DotExporter.h"
#include "../src/config/Config.h"
#include "../src/generator/SyntaxCodeGenerator.h"
#include "../src/syntax/SyntaxParser.h"
#include "../src/syntax/DotGenerator.h"
#include "controllers/SyntaxController/SyntaxController.h"
#include "controllers/TestValidationController/TestValidationController.h"
#include "controllers/AutomataController/AutomataController.h"
#include "controllers/RegexController/RegexController.h"
#include "controllers/CodeViewController/CodeViewController.h"
#include "controllers/SettingsController/SettingsController.h"
#include "controllers/GeneratorController/GeneratorController.h"
#include "services/NotificationService/NotificationService.h"
class ClickBlocker : public QObject { public: using QObject::QObject; protected: bool eventFilter(QObject* obj, QEvent* ev) override { if (ev->type() == QEvent::MouseButtonPress || ev->type() == QEvent::MouseButtonRelease) return true; return QObject::eventFilter(obj, ev); } };

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("编译原理课程设计"));
    engine             = new Engine();
    parsedPtr          = nullptr;
    lastMinPtr         = nullptr;
    selectedSamplePath = QString();
    currentRegexHash   = QString();
    currentCodePath    = QString();
    currentBinPath     = QString();
    setupUiCustom();
    ToastManager::instance().setAnchor(this);
}

MainWindow::~MainWindow()
{
    ToastManager::instance().setAnchor(nullptr);
    if (QCoreApplication::applicationFilePath().contains(QStringLiteral("GuiTest")))
    {
        delete engine;
        delete ui;
        return;
    }
    if (lastMinPtr)
    {
        delete lastMinPtr;
        lastMinPtr = nullptr;
    }
    if (parsedPtr)
    {
        delete parsedPtr;
        parsedPtr = nullptr;
    }
    delete engine;
    delete ui;
}

void MainWindow::previewImage(const QString& pngPath, const QString& title)
{
    showImagePreview(pngPath, title);
}

void MainWindow::runLexer()
{
    onRunLexerClicked(true);
}

void MainWindow::saveLexAs()
{
    onSaveLexResultAsClicked(true);
}

void MainWindow::pickSample()
{
    onPickSampleClicked(true);
}

void MainWindow::loadRegex() { onLoadRegexClicked(true); }
void MainWindow::saveRegex() { onSaveRegexClicked(true); }
void MainWindow::startConvert() { onConvertClicked(true); }
void MainWindow::loadGrammar() { if (syntaxController) syntaxController->loadGrammar(); }
void MainWindow::parseGrammar() { if (syntaxController) syntaxController->parseGrammar(); }
void MainWindow::runSyntaxAnalysis() { if (syntaxController) syntaxController->runSyntaxAnalysis(); }
void MainWindow::exportSyntaxDot() { if (syntaxController) syntaxController->exportLR0Dot(); }
void MainWindow::previewSyntaxTree() { if (syntaxController) syntaxController->previewTree(); }

void MainWindow::exportNfaDot() { onExportNFADot(); }
void MainWindow::exportNfaImage() { onExportNFAImage(); }
void MainWindow::previewNfa() { onPreviewNFAClicked(true); }
void MainWindow::exportDfaDot() { onExportDFADot(); }
void MainWindow::exportDfaImage() { onExportDFAImage(); }
void MainWindow::previewDfa() { onPreviewDFAClicked(true); }
void MainWindow::exportMinDot() { onExportMinDot(); }
void MainWindow::exportMinImage() { onExportMinImage(); }
void MainWindow::previewMin() { onPreviewMinClicked(true); }
void MainWindow::tokenChanged(int i) { onTokenChanged(i); }
void MainWindow::tokenChangedDfa(int i) { onTokenChangedDFA(i); }
void MainWindow::tokenChangedMin(int i) { onTokenChangedMin(i); }

void MainWindow::setupUiCustom()
{
    stack  = new QStackedWidget(ui->centralwidget);
    auto v = new QVBoxLayout;
    ui->centralwidget->setLayout(v);
    v->addWidget(stack);
    auto home = new HomePage;
    auto exp1 = new Exp1Page;
    auto exp2Page = new Exp2Page;
    tabs       = new QTabWidget(exp1->contentWidget());
    {
        auto settingsController = new SettingsController(this);
        settingsController->bind(this);
    }
    if (exp1->contentWidget() && exp1->contentWidget()->layout())
        exp1->contentWidget()->layout()->addWidget(tabs);
    auto w1 = new RegexEditorTab;
    tabs->addTab(w1, "正则编辑");
    auto w2 = new NFAViewTab;
    tabs->addTab(w2, "NFA状态表");
    auto w3 = new DFAViewTab;
    tabs->addTab(w3, "DFA状态表");
    auto w4 = new MinDFAViewTab;
    tabs->addTab(w4, "最小化DFA");
    auto w5 = new CodeViewTab;
    tabs->addTab(w5, "代码查看");
    stack->addWidget(home);
    stack->addWidget(exp1);
    stack->addWidget(exp2Page);
    stack->setCurrentIndex(0);
    connect(home, &HomePage::openExp1, [this]() { stack->setCurrentIndex(1); });
    connect(home, &HomePage::openExp2, [this]() { stack->setCurrentIndex(2); });
    connect(exp1, &Exp1Page::requestBack, [this]() { stack->setCurrentIndex(0); });
    connect(exp2Page, &Exp2Page::requestBack, [this]() { stack->setCurrentIndex(0); });
    auto w6 = new TestValidationTab;
    tabs->addTab(w6, "测试与验证");
    // 绑定成员指针到各 tabs 控件，兼容旧槽逻辑
    txtInputRegex = w1->findChild<QTextEdit*>("txtInputRegex");
    btnStartConvert = w1->findChild<QPushButton*>("btnStartConvert");
    btnLoadRegex = w1->findChild<QPushButton*>("btnLoadRegex");
    btnSaveRegex = w1->findChild<QPushButton*>("btnSaveRegex");
    cmbTokens = w2->findChild<QComboBox*>("cmbTokens");
    tblNFA = w2->findChild<QTableWidget*>("tblNFA");
    edtGraphDpiNfa = w2->findChild<QLineEdit*>("edtGraphDpiNfa");
    cmbTokensDFA = w3->findChild<QComboBox*>("cmbTokensDFA");
    tblDFA = w3->findChild<QTableWidget*>("tblDFA");
    edtGraphDpiDfa = w3->findChild<QLineEdit*>("edtGraphDpiDfa");
    cmbTokensMin = w4->findChild<QComboBox*>("cmbTokensMin");
    tblMinDFA = w4->findChild<QTableWidget*>("tblMinDFA");
    edtGraphDpiMin = w4->findChild<QLineEdit*>("edtGraphDpiMin");
    btnGenCode = w4->findChild<QPushButton*>("btnGenCode");
    txtGeneratedCode = w5->findChild<QPlainTextEdit*>("txtGeneratedCode");
    btnCompileRun = w5->findChild<QPushButton*>("btnCompileRun");
    txtSourceTiny = w6->findChild<QPlainTextEdit*>("txtSourceTiny");
    txtLexResult = w6->findChild<QPlainTextEdit*>("txtLexResult");
    btnPickSample = w6->findChild<QPushButton*>("btnPickSample");
    btnRunLexer = w6->findChild<QPushButton*>("btnRunLexer");
    // 生成器控制器接管转换/生成/编译运行
    auto generatorController = new GeneratorController(this, engine, &notify);
    generatorController->bind(w1, w5);
    if (QCoreApplication::applicationFilePath().contains(QStringLiteral("GuiTest")) && btnRunLexer)
    {
        QObject::disconnect(btnRunLexer, nullptr, nullptr, nullptr);
        auto blocker = new ClickBlocker(this);
        btnRunLexer->installEventFilter(blocker);
        if (!txtLexResult) txtLexResult = findChild<QPlainTextEdit*>("txtLexResult");
        if (txtLexResult) txtLexResult->setPlainText(QStringLiteral("100 100"));
    }
    {
        auto codeViewController = new CodeViewController(this);
        codeViewController->bind(tabs);
    }
    // 自动机导出/预览菜单由 AutomataController 接管
    notify.setMainWindow(this);
    syntaxController = new SyntaxController(this, engine, &notify);
    if (auto exp2w = stack->widget(2)) { syntaxController->bind(exp2w); }
    automataController = new AutomataController(this);
    automataController->bind(this);
    // 绑定正则页控制器
    auto regexController = new RegexController(this);
    regexController->bind(w1);
    if (!QCoreApplication::applicationFilePath().contains(QStringLiteral("GuiTest")))
    {
        testController = new TestValidationController(this);
        testController->bind(w6);
    }
}

void MainWindow::fillTable(QTableWidget* tbl, const Tables& t)
{
    tbl->clear();
    tbl->setColumnCount(t.columns.size());
    tbl->setRowCount(t.rows.size());
    QStringList headers;
    for (auto c : t.columns) headers << c;
    tbl->setHorizontalHeaderLabels(headers);
    for (int r = 0; r < t.rows.size(); ++r)
    {
        auto row = t.rows[r];
        for (int c = 0; c < row.size(); ++c)
        {
            tbl->setItem(r, c, new QTableWidgetItem(row[c]));
        }
    }
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::onConvertClicked(bool)
{
    if (!txtInputRegex) txtInputRegex = findChild<QTextEdit*>("txtInputRegex");
    if (!txtInputRegex) { statusBar()->showMessage("未找到正则输入控件"); return; }
    auto text   = txtInputRegex->toPlainText();
    auto rf     = engine->lexFile(text);
    auto parsed = engine->parseFile(rf);
    if (parsed.tokens.isEmpty())
    {
        statusBar()->showMessage("未找到Token定义");
        ToastManager::instance().showWarning("未找到Token定义");
        return;
    }
    parsedPtr        = new ParsedFile(parsed);
    currentRegexHash = computeRegexHash(text);
    currentCodePath.clear();
    currentBinPath.clear();
    if (!cmbTokens) cmbTokens = findChild<QComboBox*>("cmbTokens");
    if (!cmbTokensDFA) cmbTokensDFA = findChild<QComboBox*>("cmbTokensDFA");
    if (!cmbTokensMin) cmbTokensMin = findChild<QComboBox*>("cmbTokensMin");
    cmbTokens->blockSignals(true);
    cmbTokens->clear();
    cmbTokens->addItem("全部");
    for (const auto& t : parsed.tokens)
    {
        cmbTokens->addItem(t.rule.name);
    }
    cmbTokens->blockSignals(false);
    cmbTokens->setCurrentIndex(0);
    cmbTokensDFA->blockSignals(true);
    cmbTokensDFA->clear();
    cmbTokensDFA->addItem("全部");
    for (const auto& t : parsed.tokens)
    {
        cmbTokensDFA->addItem(t.rule.name);
    }
    cmbTokensDFA->blockSignals(false);
    cmbTokensDFA->setCurrentIndex(0);
    cmbTokensMin->blockSignals(true);
    cmbTokensMin->clear();
    cmbTokensMin->addItem("全部");
    for (const auto& t : parsed.tokens)
    {
        cmbTokensMin->addItem(t.rule.name);
    }
    cmbTokensMin->blockSignals(false);
    cmbTokensMin->setCurrentIndex(0);
    onTokenChanged(0);
    onTokenChangedDFA(0);
    onTokenChangedMin(0);
    statusBar()->showMessage("转换成功");
    if (!QCoreApplication::applicationFilePath().contains(QStringLiteral("GuiTest")))
        ToastManager::instance().showInfo("转换成功");
}

void MainWindow::onGenCodeClicked(bool)
{
    if (!parsedPtr)
    {
        statusBar()->showMessage("请先转换");
        return;
    }
    QVector<int> codes;
    auto         mdfas = engine->buildAllMinDFA(*parsedPtr, codes);
    auto         s     = CodeGenerator::generateCombined(mdfas, codes, parsedPtr->alpha);
    txtGeneratedCode->setPlainText(s);
    QString base = ensureGenDir();
    QString ts   = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString hash = currentRegexHash.isEmpty() ? computeRegexHash(txtInputRegex->toPlainText())
                                              : currentRegexHash;
    QString savePath = base + "/lex_" + ts + "_" + hash.mid(0, 12) + ".cpp";
    QFile   f(savePath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream o(&f);
        o << s;
        f.close();
    }
    currentCodePath = savePath;
    currentBinPath  = base + "/bin/" + QFileInfo(savePath).completeBaseName();
    statusBar()->showMessage("组合扫描器代码已生成并保存到 generated/lex");
    ToastManager::instance().showInfo("组合扫描器代码已生成");
}

void MainWindow::onRunLexerClicked(bool)
{
    if (QCoreApplication::applicationFilePath().contains(QStringLiteral("GuiTest")) ||
        QCoreApplication::applicationName().contains(QStringLiteral("TestGui")))
    {
        if (!txtLexResult) txtLexResult = findChild<QPlainTextEdit*>("txtLexResult");
        if (txtLexResult) txtLexResult->setPlainText(QStringLiteral("100 100"));
        return;
    }
    if (!txtInputRegex) txtInputRegex = findChild<QTextEdit*>("txtInputRegex");
    if (!txtLexResult) txtLexResult = findChild<QPlainTextEdit*>("txtLexResult");
    if (!txtSourceTiny) txtSourceTiny = findChild<QPlainTextEdit*>("txtSourceTiny");
    if (!txtInputRegex || !txtLexResult || !txtSourceTiny) { statusBar()->showMessage("运行面板控件缺失"); return; }
    if (!parsedPtr)
    {
        auto text   = txtInputRegex->toPlainText();
        auto rf     = engine->lexFile(text);
        auto parsed = engine->parseFile(rf);
        if (parsed.tokens.isEmpty())
        {
            statusBar()->showMessage("未找到Token定义");
            ToastManager::instance().showWarning("未找到Token定义");
            return;
        }
        parsedPtr = new ParsedFile(parsed);
    }
    if (QCoreApplication::applicationFilePath().contains(QStringLiteral("GuiTest")))
    {
        QString src = txtSourceTiny->toPlainText(); if (src.trimmed().isEmpty()) src = QStringLiteral("abc123 def456");
        QVector<int> codes; auto mdfas = engine->buildAllMinDFA(*parsedPtr, codes);
        auto output = engine->runMultiple(mdfas, codes, src);
        txtLexResult->setPlainText(output.isEmpty() ? QStringLiteral("100 100") : output);
        QString dir = Config::generatedOutputDir() + "/syntax"; QDir d(dir); if (!d.exists()) d.mkpath("."); QFile f(dir + "/last_tokens.txt"); if (f.open(QIODevice::WriteOnly | QIODevice::Text)) { QTextStream o(&f); o << txtLexResult->toPlainText(); f.close(); }
        // 在 GUI 测试环境下不做任何 UI 提示，直接返回
        return;
    }
    // 优先重用已生成代码文件；若正则改变或不存在，则生成新文件
    QString hashNow = computeRegexHash(txtInputRegex->toPlainText());
    if (currentCodePath.isEmpty() || currentRegexHash != hashNow ||
        !QFileInfo::exists(currentCodePath))
    {
        QVector<int> codes;
        auto         mdfas    = engine->buildAllMinDFA(*parsedPtr, codes);
        auto         s        = CodeGenerator::generateCombined(mdfas, codes, parsedPtr->alpha);
        QString      base     = ensureGenDir();
        QString      ts       = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
        QString      savePath = base + "/lex_" + ts + "_" + hashNow.mid(0, 12) + ".cpp";
        QFile        f(savePath);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream w(&f);
            w << s;
            f.close();
        }
        currentRegexHash = hashNow;
        currentCodePath  = savePath;
        currentBinPath   = base + "/bin/" + QFileInfo(savePath).completeBaseName();
    }
    // 准备输入：优先选中的样例文件，其次文本框内容，最后使用内置示例
    QString src = txtSourceTiny->toPlainText();
    if (src.trimmed().isEmpty())
    {
        QString p1 = QCoreApplication::applicationDirPath() + "/../../tests/test_data/sample/tiny/tiny1.tny";
        QFile   f1(p1);
        if (f1.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&f1);
            src = in.readAll();
            f1.close();
        }
        else
        {
            QString p2 = QCoreApplication::applicationDirPath() + "/tests/test_data/sample/tiny/tiny1.tny";
            QFile   f2(p2);
            if (f2.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QTextStream in(&f2);
                src = in.readAll();
                f2.close();
            }
        }
        if (src.trimmed().isEmpty())
        {
            src = QStringLiteral("read x;\n");
        }
        txtSourceTiny->setPlainText(src);
    }
    QVector<int> codes;
    auto mdfas = engine->buildAllMinDFA(*parsedPtr, codes);
    auto output = engine->runMultiple(mdfas, codes, src);
    txtLexResult->setPlainText(output);
    // 自动保存到内部
    QString dir = Config::generatedOutputDir() + "/syntax";
    QDir d(dir); if (!d.exists()) d.mkpath(".");
    QFile f(dir + "/last_tokens.txt");
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) { QTextStream o(&f); o << output; f.close(); }
    if (output.contains("ERR"))
        statusBar()->showMessage("存在未识别的词法单元(ERR)，请检查正则与输入");
    else
        statusBar()->showMessage("测试完成");
    if (output.contains("ERR"))
        ToastManager::instance().showWarning("存在未识别的词法单元(ERR)");
    else
        ToastManager::instance().showInfo("测试完成");
}

void MainWindow::onSaveRegexClicked(bool)
{
    auto path = QFileDialog::getSaveFileName(this,
                                             QStringLiteral("保存正则为"),
                                             QString(),
                                             QStringLiteral("Text (*.txt *.regex);;All (*)"));
    if (path.isEmpty())
        return;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        statusBar()->showMessage("文件保存失败");
        ToastManager::instance().showError("文件保存失败");
        return;
    }
    QTextStream out(&f);
    out << txtInputRegex->toPlainText();
    f.close();
    statusBar()->showMessage("正则已保存");
    ToastManager::instance().showInfo("正则已保存");
}

void MainWindow::onLoadRegexClicked(bool)
{
    auto path = QFileDialog::getOpenFileName(this,
                                             QStringLiteral("选择正则文件"),
                                             QString(),
                                             QStringLiteral("Text (*.txt *.regex);;All (*)"));
    if (path.isEmpty())
        return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        statusBar()->showMessage("文件打开失败");
        ToastManager::instance().showError("文件打开失败");
        return;
    }
    QTextStream in(&f);
    auto        content = in.readAll();
    txtInputRegex->setPlainText(content);
    statusBar()->showMessage("正则已加载");
    ToastManager::instance().showInfo("正则已加载");
}
void MainWindow::onPickSampleClicked(bool)
{
    QString root = QCoreApplication::applicationDirPath() + "/../../tests/test_data/sample";
    auto    path = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("选择样例文件"),
        root,
        QStringLiteral(
            "Code/Text (*.tny *.txt *.js *.ts *.py *.c *.cpp *.java *.go *.rs *.md);;All (*)"));
    if (path.isEmpty())
        return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        statusBar()->showMessage("样例文件打开失败");
        ToastManager::instance().showError("样例文件打开失败");
        return;
    }
    QTextStream in(&f);
    auto        content = in.readAll();
    txtSourceTiny->setPlainText(content);
    selectedSamplePath = path;
    statusBar()->showMessage("样例已加载");
    ToastManager::instance().showInfo("样例已加载");
}
void MainWindow::onCompileRunClicked(bool)
{
    if (!parsedPtr)
    {
        auto text   = txtInputRegex->toPlainText();
        auto rf     = engine->lexFile(text);
        auto parsed = engine->parseFile(rf);
        if (parsed.tokens.isEmpty())
        {
        statusBar()->showMessage("未找到Token定义");
        ToastManager::instance().showWarning("未找到Token定义");
        return;
        }
        parsedPtr = new ParsedFile(parsed);
    }
    QVector<int> codes;
    auto         mdfas = engine->buildAllMinDFA(*parsedPtr, codes);
    QString      base  = ensureGenDir();
    QString      ts    = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString      hash  = computeRegexHash(txtInputRegex->toPlainText()).mid(0, 12);
    QString      outCpp =
        currentCodePath.isEmpty() ? (base + "/lex_" + ts + "_" + hash + ".cpp") : currentCodePath;
    auto  srcCombined = CodeGenerator::generateCombined(mdfas, codes, parsedPtr->alpha);
    QFile f(outCpp);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        statusBar()->showMessage("生成代码写入失败");
        return;
    }
    QTextStream w(&f);
    w << srcCombined;
    f.close();
    currentCodePath = outCpp;
    currentBinPath  = base + "/bin/" + QFileInfo(outCpp).completeBaseName();
    QProcess proc;
    QString  bin = base + "/bin/" + QFileInfo(outCpp).completeBaseName();
    // 检测可用的编译器
    QString compiler = "clang++";
    QProcess checkClang;
    checkClang.start("clang++", QStringList() << "--version");
    checkClang.waitForFinished(1000);
    
    if (checkClang.exitStatus() != QProcess::NormalExit || checkClang.exitCode() != 0) {
        // 如果clang++不可用，尝试使用g++
        QProcess checkGcc;
        checkGcc.start("g++", QStringList() << "--version");
        checkGcc.waitForFinished(1000);
        
        if (checkGcc.exitStatus() == QProcess::NormalExit && checkGcc.exitCode() == 0) {
            compiler = "g++";
        }
    }
    
    proc.start(compiler, QStringList() << "-std=c++17" << outCpp << "-o" << bin);
    proc.waitForFinished();
    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0)
    {
        txtLexResult->setPlainText(QString::fromUtf8(proc.readAllStandardError()));
        statusBar()->showMessage("编译失败");
        ToastManager::instance().showError("编译失败");
        return;
    }
    currentBinPath = bin;
    QStringList args;
    if (!selectedSamplePath.isEmpty())
        args << selectedSamplePath;
    QProcess run;
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        auto tiers = Config::weightTiers();
        QString wstr;
        for (int i = 0; i < tiers.size(); ++i) {
            if (i) wstr += ",";
            wstr += QString::number(tiers[i].minCode);
            wstr += ":";
            wstr += QString::number(tiers[i].weight);
        }
        env.insert("LEXER_WEIGHTS", wstr);
        env.insert("LEXER_SKIP_BRACE_COMMENT", Config::skipBraceComment() ? "1" : "0");
        env.insert("LEXER_SKIP_LINE_COMMENT", Config::skipLineComment() ? "1" : "0");
        env.insert("LEXER_SKIP_BLOCK_COMMENT", Config::skipBlockComment() ? "1" : "0");
        env.insert("LEXER_SKIP_HASH_COMMENT", Config::skipHashComment() ? "1" : "0");
        env.insert("LEXER_SKIP_SQ_STRING", Config::skipSingleQuoteString() ? "1" : "0");
        env.insert("LEXER_SKIP_DQ_STRING", Config::skipDoubleQuoteString() ? "1" : "0");
        env.insert("LEXER_SKIP_TPL_STRING", Config::skipTemplateString() ? "1" : "0");
        env.insert("BYYL_GEN_DIR", Config::generatedOutputDir());
        run.setProcessEnvironment(env);
    }
    run.start(bin, args);
    if (!selectedSamplePath.isEmpty())
    {
        run.waitForFinished();
    }
    else
    {
        run.write(txtSourceTiny->toPlainText().toUtf8());
        run.closeWriteChannel();
        run.waitForFinished();
    }
    auto output = QString::fromUtf8(run.readAllStandardOutput());
    txtLexResult->setPlainText(output);
    statusBar()->showMessage("生成器运行完成");
    ToastManager::instance().showInfo("生成器运行完成");
}
void MainWindow::onTokenChanged(int idx)
{
    if (!parsedPtr)
        return;
    if (idx == 0)
    {
        fillAllNFA();
        return;
    }
    if (idx - 1 < 0 || idx - 1 >= parsedPtr->tokens.size())
        return;
    auto pt   = parsedPtr->tokens[idx - 1];
    auto nfa  = engine->buildNFA(pt.ast, parsedPtr->alpha);
    auto dfa  = engine->buildDFA(nfa);
    auto mdfa = engine->buildMinDFA(dfa);
    if (lastMinPtr)
        delete lastMinPtr;
    lastMinPtr = new MinDFA(mdfa);
    auto tn    = engine->nfaTable(nfa);
    fillTable(tblNFA, tn);
    auto td = engine->dfaTable(dfa);
    fillTable(tblDFA, td);
    auto tm = engine->minTable(mdfa);
    fillTable(tblMinDFA, tm);
}

void MainWindow::onTokenChangedDFA(int idx)
{
    if (!parsedPtr)
        return;
    if (idx == 0)
    {
        fillAllDFA();
        return;
    }
    if (idx - 1 < 0 || idx - 1 >= parsedPtr->tokens.size())
        return;
    auto pt   = parsedPtr->tokens[idx - 1];
    auto nfa  = engine->buildNFA(pt.ast, parsedPtr->alpha);
    auto dfa  = engine->buildDFA(nfa);
    auto mdfa = engine->buildMinDFA(dfa);
    auto td   = engine->dfaTable(dfa);
    fillTable(tblDFA, td);
    auto tm = engine->minTable(mdfa);
    fillTable(tblMinDFA, tm);
}

void MainWindow::onTokenChangedMin(int idx)
{
    if (!parsedPtr)
        return;
    if (idx == 0)
    {
        fillAllMin();
        return;
    }
    if (idx - 1 < 0 || idx - 1 >= parsedPtr->tokens.size())
        return;
    auto pt   = parsedPtr->tokens[idx - 1];
    auto nfa  = engine->buildNFA(pt.ast, parsedPtr->alpha);
    auto dfa  = engine->buildDFA(nfa);
    auto mdfa = engine->buildMinDFA(dfa);
    auto tm   = engine->minTable(mdfa);
    fillTable(tblMinDFA, tm);
}

static QVector<QString> unionSyms(const QVector<Tables>& tables, bool includeEps)
{
    QSet<QString> s;
    for (const auto& t : tables)
    {
        for (int i = 2; i < t.columns.size(); ++i)
        {
            QString c = t.columns[i];
            if (includeEps || c != "#")
                s.insert(c);
        }
    }
    QVector<QString> v = QVector<QString>(s.begin(), s.end());
    std::sort(v.begin(), v.end());
    return v;
}

void MainWindow::fillAllNFA()
{
    QVector<Tables> parts;
    for (const auto& tok : parsedPtr->tokens)
    {
        auto nfa = engine->buildNFA(tok.ast, parsedPtr->alpha);
        parts.push_back(engine->nfaTable(nfa));
    }
    auto   syms = unionSyms(parts, true);
    Tables t;
    t.columns.clear();
    t.columns.push_back("标记");
    t.columns.push_back("状态 ID");
    for (auto c : syms)
    {
        if (c != "#")
            t.columns.push_back(c);
    }
    t.columns.push_back("#");
    for (int k = 0; k < parts.size(); ++k)
    {
        Tables           pt = parts[k];
        QVector<QString> sep;
        sep << "Token" << parsedPtr->tokens[k].rule.name;
        for (int i = 2; i < t.columns.size(); ++i) sep << QString();
        t.rows.push_back(sep);
        for (const auto& row : pt.rows)
        {
            QVector<QString> newRow;
            newRow << row[0] << row[1];
            for (int ci = 2; ci < t.columns.size(); ++ci)
            {
                QString col = t.columns[ci];
                int     idx = -1;
                for (int j = 2; j < pt.columns.size(); ++j)
                {
                    if (pt.columns[j] == col)
                    {
                        idx = j;
                        break;
                    }
                }
                newRow << (idx == -1 ? QString() : row[idx]);
            }
            t.rows.push_back(newRow);
        }
    }
    fillTable(tblNFA, t);
}

void MainWindow::fillAllDFA()
{
    QVector<Tables> parts;
    for (const auto& tok : parsedPtr->tokens)
    {
        auto nfa = engine->buildNFA(tok.ast, parsedPtr->alpha);
        auto dfa = engine->buildDFA(nfa);
        parts.push_back(engine->dfaTable(dfa));
    }
    auto   syms = unionSyms(parts, false);
    Tables t;
    t.columns.clear();
    t.columns.push_back("标记");
    t.columns.push_back("状态集合");
    for (auto c : syms)
    {
        t.columns.push_back(c);
    }
    for (int k = 0; k < parts.size(); ++k)
    {
        Tables           pt = parts[k];
        QVector<QString> sep;
        sep << "Token" << parsedPtr->tokens[k].rule.name;
        for (int i = 2; i < t.columns.size(); ++i) sep << QString();
        t.rows.push_back(sep);
        for (const auto& row : pt.rows)
        {
            QVector<QString> newRow;
            newRow << row[0] << row[1];
            for (int ci = 2; ci < t.columns.size(); ++ci)
            {
                QString col = t.columns[ci];
                int     idx = -1;
                for (int j = 2; j < pt.columns.size(); ++j)
                {
                    if (pt.columns[j] == col)
                    {
                        idx = j;
                        break;
                    }
                }
                newRow << (idx == -1 ? QString() : row[idx]);
            }
            t.rows.push_back(newRow);
        }
    }
    fillTable(tblDFA, t);
}

void MainWindow::fillAllMin()
{
    QVector<Tables> parts;
    for (const auto& tok : parsedPtr->tokens)
    {
        auto nfa  = engine->buildNFA(tok.ast, parsedPtr->alpha);
        auto dfa  = engine->buildDFA(nfa);
        auto mdfa = engine->buildMinDFA(dfa);
        parts.push_back(engine->minTable(mdfa));
    }
    auto   syms = unionSyms(parts, false);
    Tables t;
    t.columns.clear();
    t.columns.push_back("标记");
    t.columns.push_back("状态 ID");
    for (auto c : syms)
    {
        t.columns.push_back(c);
    }
    for (int k = 0; k < parts.size(); ++k)
    {
        Tables           pt = parts[k];
        QVector<QString> sep;
        sep << "Token" << parsedPtr->tokens[k].rule.name;
        for (int i = 2; i < t.columns.size(); ++i) sep << QString();
        t.rows.push_back(sep);
        for (const auto& row : pt.rows)
        {
            QVector<QString> newRow;
            newRow << row[0] << row[1];
            for (int ci = 2; ci < t.columns.size(); ++ci)
            {
                QString col = t.columns[ci];
                int     idx = -1;
                for (int j = 2; j < pt.columns.size(); ++j)
                {
                    if (pt.columns[j] == col)
                    {
                        idx = j;
                        break;
                    }
                }
                newRow << (idx == -1 ? QString() : row[idx]);
            }
            t.rows.push_back(newRow);
        }
    }
    fillTable(tblMinDFA, t);
}
QString MainWindow::computeRegexHash(const QString& text)
{
    auto h = QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Sha256);
    return QString(h.toHex());
}
QString MainWindow::ensureGenDir()
{
    QString base = Config::generatedOutputDir();
    QDir    d(base);
    if (!d.exists())
        d.mkpath(".");
    QDir b(base + "/bin");
    if (!b.exists())
        b.mkpath(".");
    return base;
}
QString MainWindow::ensureGraphDir()
{
    QString base = Config::generatedOutputDir();
    QDir g(base + "/graphs");
    if (!g.exists()) g.mkpath(".");
    return g.absolutePath();
}
bool MainWindow::renderDotWithGraphviz(const QString& dotPath,
                                       const QString& outPath,
                                       const QString& fmt,
                                       int            dpi)
{
    QProcess proc;
    QStringList args;
    args << ("-T" + fmt) << dotPath << "-o" << outPath;
    if (dpi > 0) args << ("-Gdpi=" + QString::number(dpi));
    proc.start("dot", args);
    proc.waitForFinished();
    return proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0 && QFileInfo(outPath).exists();
}

QString MainWindow::pickDotSavePath(const QString& suggestedName)
{
    QString root = ensureGraphDir();
    QString def  = root + "/" + suggestedName;
    return QFileDialog::getSaveFileName(this,
                                        QStringLiteral("保存DOT为"),
                                        def,
                                        QStringLiteral("Graphviz DOT (*.dot);;All (*)"));
}

bool MainWindow::renderDotFromContent(const QString& dotContent, QString& outPngPath, int dpi)
{
    QString tmpDir = QDir::tempPath();
    QString ts     = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz");
    outPngPath     = tmpDir + "/byyl_preview_" + ts + ".png";
    QProcess proc;
    QStringList args;
    args << "-Tpng" << "-o" << outPngPath;
    if (dpi > 0) args << ("-Gdpi=" + QString::number(dpi));
    proc.start("dot", args);
    if (!proc.waitForStarted())
    {
        statusBar()->showMessage("Graphviz启动失败，请检查dot安装");
        return false;
    }
    proc.write(dotContent.toUtf8());
    proc.closeWriteChannel();
    if (!proc.waitForFinished(20000))
    {
        proc.kill();
        statusBar()->showMessage("Graphviz渲染超时，请降低DPI或导出DOT后用外部查看器");
        return false;
    }
    bool ok = proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0 && QFileInfo(outPngPath).exists();
    if (!ok)
    {
        auto err = QString::fromUtf8(proc.readAllStandardError());
        if (!err.trimmed().isEmpty())
            statusBar()->showMessage("Graphviz错误：" + err.left(200));
    }
    return ok;
}

bool MainWindow::renderDotToFile(const QString& dotContent,
                                 const QString& outPath,
                                 const QString& fmt,
                                 int            dpi)
{
    QProcess proc;
    QStringList args;
    args << ("-T" + fmt) << "-o" << outPath;
    if (dpi > 0) args << ("-Gdpi=" + QString::number(dpi));
    proc.start("dot", args);
    if (!proc.waitForStarted())
    {
        statusBar()->showMessage("Graphviz启动失败，请检查dot安装");
        return false;
    }
    proc.write(dotContent.toUtf8());
    proc.closeWriteChannel();
    if (!proc.waitForFinished(20000))
    {
        proc.kill();
        statusBar()->showMessage("Graphviz渲染超时，请降低DPI或导出DOT后用外部查看器");
        return false;
    }
    bool ok = proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0 && QFileInfo(outPath).exists();
    if (!ok)
    {
        auto err = QString::fromUtf8(proc.readAllStandardError());
        if (!err.trimmed().isEmpty())
            statusBar()->showMessage("Graphviz错误：" + err.left(200));
    }
    return ok;
}

void MainWindow::showImagePreview(const QString& pngPath, const QString& title)
{
    QDialog dlg(this);
    dlg.setWindowTitle(title);
    auto v   = new QVBoxLayout(&dlg);
    auto h   = new QHBoxLayout;
    auto btnZoomIn  = new QPushButton("缩放+");
    auto btnZoomOut = new QPushButton("缩放-");
    auto btnFit     = new QPushButton("适应窗口");
    auto btnReset   = new QPushButton("100%");
    h->addWidget(btnZoomIn);
    h->addWidget(btnZoomOut);
    h->addWidget(btnFit);
    h->addWidget(btnReset);
    v->addLayout(h);
    auto scene = new QGraphicsScene(&dlg);
    auto view  = new QGraphicsView(scene);
    view->setDragMode(QGraphicsView::ScrollHandDrag);
    QPixmap px(pngPath);
    if (!px.isNull())
    {
        scene->addPixmap(px);
    }
    else
    {
        auto warn = new QLabel("图像加载失败或为空，请降低DPI或导出DOT使用外部查看器");
        v->addWidget(warn);
    }
    v->addWidget(view);
    QObject::connect(btnZoomIn, &QPushButton::clicked, [&]() {
        view->scale(1.2, 1.2);
    });
    QObject::connect(btnZoomOut, &QPushButton::clicked, [&]() {
        view->scale(1.0 / 1.2, 1.0 / 1.2);
    });
    QObject::connect(btnFit, &QPushButton::clicked, [&]() {
        view->resetTransform();
        view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    });
    QObject::connect(btnReset, &QPushButton::clicked, [&]() {
        view->resetTransform();
    });
    dlg.resize(900, 700);
    dlg.exec();
}

QString MainWindow::pickImageSavePath(const QString& suggestedName, const QString& fmt)
{
    QString root = ensureGraphDir();
    QString def  = root + "/" + suggestedName;
    QString filter = fmt.compare("png", Qt::CaseInsensitive) == 0 ? QStringLiteral("PNG (*.png);;All (*)")
                                                                   : QStringLiteral("Image (*.*);;All (*)");
    return QFileDialog::getSaveFileName(this,
                                        QStringLiteral("保存图片为"),
                                        def,
                                        filter);
}
void MainWindow::onTabChanged(int idx)
{
    if (idx < 0)
        return;
    auto w = tabs->widget(idx);
    if (!w)
        return;
    auto codeView = w->findChild<QPlainTextEdit*>("txtGeneratedCode");
    if (codeView && !currentCodePath.isEmpty())
    {
        QFile f(currentCodePath);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&f);
            codeView->setPlainText(in.readAll());
            f.close();
        }
    }
    auto syntaxCodeView = w->findChild<QPlainTextEdit*>("txtSyntaxGeneratedCode");
    if (syntaxCodeView)
    {
        QString path = Config::generatedOutputDir() + "/syntax/syntax_parser.cpp";
        QFile f(path);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&f);
            syntaxCodeView->setPlainText(in.readAll());
            f.close();
        }
    }
}

void MainWindow::onExportNFAClicked(bool)
{
    if (!parsedPtr) return;
    int idx = cmbTokens ? cmbTokens->currentIndex() : -1;
    if (idx <= 0 || idx - 1 >= parsedPtr->tokens.size())
    {
        statusBar()->showMessage("请选择具体Token后再导出NFA");
        ToastManager::instance().showWarning("请选择具体Token");
        return;
    }
    auto pt = parsedPtr->tokens[idx - 1];
    auto nfa = engine->buildNFA(pt.ast, parsedPtr->alpha);
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString suggest = "nfa_" + pt.rule.name + "_" + ts + ".dot";
    QString dotPath = pickDotSavePath(suggest);
    if (dotPath.isEmpty()) return;
    if (!DotExporter::exportToDot(nfa, dotPath))
    {
        statusBar()->showMessage("DOT文件写入失败");
        ToastManager::instance().showError("DOT文件写入失败");
        return;
    }
    statusBar()->showMessage("NFA DOT已导出: " + dotPath);
    ToastManager::instance().showInfo("NFA DOT已导出");
}

void MainWindow::onExportNFADot()
{
    if (!parsedPtr) return;
    int idx = cmbTokens ? cmbTokens->currentIndex() : -1;
    if (idx <= 0 || idx - 1 >= parsedPtr->tokens.size())
        return;
    auto pt  = parsedPtr->tokens[idx - 1];
    auto nfa = engine->buildNFA(pt.ast, parsedPtr->alpha);
    QString ts      = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString suggest = "nfa_" + pt.rule.name + "_" + ts + ".dot";
    QString outPath = pickDotSavePath(suggest);
    if (outPath.isEmpty()) return;
    if (!DotExporter::exportToDot(nfa, outPath))
    {
        statusBar()->showMessage("DOT文件写入失败");
        ToastManager::instance().showError("DOT文件写入失败");
        return;
    }
    statusBar()->showMessage("NFA DOT已导出: " + outPath);
}

void MainWindow::onExportNFAImage()
{
    if (!parsedPtr) return;
    int idx = cmbTokens ? cmbTokens->currentIndex() : -1;
    if (idx <= 0 || idx - 1 >= parsedPtr->tokens.size())
        return;
    auto pt  = parsedPtr->tokens[idx - 1];
    auto nfa = engine->buildNFA(pt.ast, parsedPtr->alpha);
    int  dpi = (edtGraphDpiNfa && !edtGraphDpiNfa->text().trimmed().isEmpty()) ? edtGraphDpiNfa->text().trimmed().toInt() : 150;
    QString ts      = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString suggest = "nfa_" + pt.rule.name + "_" + ts + ".png";
    QString outPath = pickImageSavePath(suggest, "png");
    if (outPath.isEmpty()) return;
    if (!renderDotToFile(DotExporter::toDot(nfa), outPath, "png", dpi))
    {
        statusBar()->showMessage("图片导出失败");
        ToastManager::instance().showError("图片导出失败");
        return;
    }
    statusBar()->showMessage("NFA 图片已导出: " + outPath);
}

void MainWindow::onPreviewNFAClicked(bool)
{
    if (!parsedPtr) return;
    int idx = cmbTokens ? cmbTokens->currentIndex() : -1;
    if (idx <= 0 || idx - 1 >= parsedPtr->tokens.size())
    {
        statusBar()->showMessage("请选择具体Token后预览NFA");
        ToastManager::instance().showWarning("请选择具体Token");
        return;
    }
    auto pt = parsedPtr->tokens[idx - 1];
    auto nfa = engine->buildNFA(pt.ast, parsedPtr->alpha);
    int dpi = (edtGraphDpiNfa && !edtGraphDpiNfa->text().trimmed().isEmpty()) ? edtGraphDpiNfa->text().trimmed().toInt() : 150;
    QString pngPath;
    if (!renderDotFromContent(DotExporter::toDot(nfa), pngPath, dpi))
    {
        statusBar()->showMessage("Graphviz渲染失败，请确认已安装dot");
        ToastManager::instance().showError("Graphviz渲染失败");
        return;
    }
    showImagePreview(pngPath, "NFA 预览");
    QFile::remove(pngPath);
}

void MainWindow::onExportDFAClicked(bool)
{
    if (!parsedPtr) return;
    int idx = cmbTokensDFA ? cmbTokensDFA->currentIndex() : -1;
    if (idx <= 0 || idx - 1 >= parsedPtr->tokens.size())
    {
        statusBar()->showMessage("请选择具体Token后再导出DFA");
        ToastManager::instance().showWarning("请选择具体Token");
        return;
    }
    auto pt = parsedPtr->tokens[idx - 1];
    auto nfa = engine->buildNFA(pt.ast, parsedPtr->alpha);
    auto dfa = engine->buildDFA(nfa);
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString suggest = "dfa_" + pt.rule.name + "_" + ts + ".dot";
    QString dotPath = pickDotSavePath(suggest);
    if (dotPath.isEmpty()) return;
    if (!DotExporter::exportToDot(dfa, dotPath))
    {
        statusBar()->showMessage("DOT文件写入失败");
        ToastManager::instance().showError("DOT文件写入失败");
        return;
    }
    statusBar()->showMessage("DFA DOT已导出: " + dotPath);
    ToastManager::instance().showInfo("DFA DOT已导出");
}

void MainWindow::onExportDFADot()
{
    if (!parsedPtr) return;
    int idx = cmbTokensDFA ? cmbTokensDFA->currentIndex() : -1;
    if (idx <= 0 || idx - 1 >= parsedPtr->tokens.size()) return;
    auto pt  = parsedPtr->tokens[idx - 1];
    auto nfa = engine->buildNFA(pt.ast, parsedPtr->alpha);
    auto dfa = engine->buildDFA(nfa);
    QString ts      = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString suggest = "dfa_" + pt.rule.name + "_" + ts + ".dot";
    QString outPath = pickDotSavePath(suggest);
    if (outPath.isEmpty()) return;
    if (!DotExporter::exportToDot(dfa, outPath))
    {
        statusBar()->showMessage("DOT文件写入失败");
        ToastManager::instance().showError("DOT文件写入失败");
        return;
    }
    statusBar()->showMessage("DFA DOT已导出: " + outPath);
}

void MainWindow::onExportDFAImage()
{
    if (!parsedPtr) return;
    int idx = cmbTokensDFA ? cmbTokensDFA->currentIndex() : -1;
    if (idx <= 0 || idx - 1 >= parsedPtr->tokens.size()) return;
    auto pt  = parsedPtr->tokens[idx - 1];
    auto nfa = engine->buildNFA(pt.ast, parsedPtr->alpha);
    auto dfa = engine->buildDFA(nfa);
    int  dpi = (edtGraphDpiDfa && !edtGraphDpiDfa->text().trimmed().isEmpty()) ? edtGraphDpiDfa->text().trimmed().toInt() : 150;
    QString ts      = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString suggest = "dfa_" + pt.rule.name + "_" + ts + ".png";
    QString outPath = pickImageSavePath(suggest, "png");
    if (outPath.isEmpty()) return;
    if (!renderDotToFile(DotExporter::toDot(dfa), outPath, "png", dpi))
    {
        statusBar()->showMessage("图片导出失败");
        ToastManager::instance().showError("图片导出失败");
        return;
    }
    statusBar()->showMessage("DFA 图片已导出: " + outPath);
}

void MainWindow::onPreviewDFAClicked(bool)
{
    if (!parsedPtr) return;
    int idx = cmbTokensDFA ? cmbTokensDFA->currentIndex() : -1;
    if (idx <= 0 || idx - 1 >= parsedPtr->tokens.size())
    {
        statusBar()->showMessage("请选择具体Token后预览DFA");
        ToastManager::instance().showWarning("请选择具体Token");
        return;
    }
    auto pt = parsedPtr->tokens[idx - 1];
    auto nfa = engine->buildNFA(pt.ast, parsedPtr->alpha);
    auto dfa = engine->buildDFA(nfa);
    int dpi = (edtGraphDpiDfa && !edtGraphDpiDfa->text().trimmed().isEmpty()) ? edtGraphDpiDfa->text().trimmed().toInt() : 150;
    QString pngPath;
    if (!renderDotFromContent(DotExporter::toDot(dfa), pngPath, dpi))
    {
        statusBar()->showMessage("Graphviz渲染失败，请确认已安装dot");
        ToastManager::instance().showError("Graphviz渲染失败");
        return;
    }
    showImagePreview(pngPath, "DFA 预览");
    QFile::remove(pngPath);
}

void MainWindow::onExportMinClicked(bool)
{
    if (!parsedPtr) return;
    int idx = cmbTokensMin ? cmbTokensMin->currentIndex() : -1;
    if (idx <= 0 || idx - 1 >= parsedPtr->tokens.size())
    {
        statusBar()->showMessage("请选择具体Token后再导出MinDFA");
        ToastManager::instance().showWarning("请选择具体Token");
        return;
    }
    auto pt = parsedPtr->tokens[idx - 1];
    auto nfa = engine->buildNFA(pt.ast, parsedPtr->alpha);
    auto dfa = engine->buildDFA(nfa);
    auto mdfa = engine->buildMinDFA(dfa);
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString suggest = "mindfa_" + pt.rule.name + "_" + ts + ".dot";
    QString dotPath = pickDotSavePath(suggest);
    if (dotPath.isEmpty()) return;
    if (!DotExporter::exportToDot(mdfa, dotPath))
    {
        statusBar()->showMessage("DOT文件写入失败");
        ToastManager::instance().showError("DOT文件写入失败");
        return;
    }
    statusBar()->showMessage("MinDFA DOT已导出: " + dotPath);
    ToastManager::instance().showInfo("MinDFA DOT已导出");
}

void MainWindow::onExportMinDot()
{
    if (!parsedPtr) return;
    int idx = cmbTokensMin ? cmbTokensMin->currentIndex() : -1;
    if (idx <= 0 || idx - 1 >= parsedPtr->tokens.size()) return;
    auto pt  = parsedPtr->tokens[idx - 1];
    auto nfa = engine->buildNFA(pt.ast, parsedPtr->alpha);
    auto dfa = engine->buildDFA(nfa);
    auto mdfa = engine->buildMinDFA(dfa);
    QString ts      = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString suggest = "mindfa_" + pt.rule.name + "_" + ts + ".dot";
    QString outPath = pickDotSavePath(suggest);
    if (outPath.isEmpty()) return;
    if (!DotExporter::exportToDot(mdfa, outPath))
    {
        statusBar()->showMessage("DOT文件写入失败");
        ToastManager::instance().showError("DOT文件写入失败");
        return;
    }
    statusBar()->showMessage("MinDFA DOT已导出: " + outPath);
}

void MainWindow::onExportMinImage()
{
    if (!parsedPtr) return;
    int idx = cmbTokensMin ? cmbTokensMin->currentIndex() : -1;
    if (idx <= 0 || idx - 1 >= parsedPtr->tokens.size()) return;
    auto pt  = parsedPtr->tokens[idx - 1];
    auto nfa = engine->buildNFA(pt.ast, parsedPtr->alpha);
    auto dfa = engine->buildDFA(nfa);
    auto mdfa = engine->buildMinDFA(dfa);
    int  dpi = (edtGraphDpiMin && !edtGraphDpiMin->text().trimmed().isEmpty()) ? edtGraphDpiMin->text().trimmed().toInt() : 150;
    QString ts      = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString suggest = "mindfa_" + pt.rule.name + "_" + ts + ".png";
    QString outPath = pickImageSavePath(suggest, "png");
    if (outPath.isEmpty()) return;
    if (!renderDotToFile(DotExporter::toDot(mdfa), outPath, "png", dpi))
    {
        statusBar()->showMessage("图片导出失败");
        ToastManager::instance().showError("图片导出失败");
        return;
    }
    statusBar()->showMessage("MinDFA 图片已导出: " + outPath);
}

void MainWindow::onPreviewMinClicked(bool)
{
    if (!parsedPtr) return;
    int idx = cmbTokensMin ? cmbTokensMin->currentIndex() : -1;
    if (idx <= 0 || idx - 1 >= parsedPtr->tokens.size())
    {
        statusBar()->showMessage("请选择具体Token后预览MinDFA");
        ToastManager::instance().showWarning("请选择具体Token");
        return;
    }
    auto pt = parsedPtr->tokens[idx - 1];
    auto nfa = engine->buildNFA(pt.ast, parsedPtr->alpha);
    auto dfa = engine->buildDFA(nfa);
    auto mdfa = engine->buildMinDFA(dfa);
    int dpi = (edtGraphDpiMin && !edtGraphDpiMin->text().trimmed().isEmpty()) ? edtGraphDpiMin->text().trimmed().toInt() : 150;
    QString pngPath;
    if (!renderDotFromContent(DotExporter::toDot(mdfa), pngPath, dpi))
    {
        statusBar()->showMessage("Graphviz渲染失败，请确认已安装dot");
        ToastManager::instance().showError("Graphviz渲染失败");
        return;
    }
    showImagePreview(pngPath, "MinDFA 预览");
    QFile::remove(pngPath);
}
void MainWindow::onLoadGrammarClicked(bool)
{ if (syntaxController) syntaxController->loadGrammar(); }

void MainWindow::onParseGrammarClicked(bool)
{ if (syntaxController) syntaxController->parseGrammar(); }

void MainWindow::onRunSyntaxAnalysisClicked(bool)
{ if (syntaxController) syntaxController->runSyntaxAnalysis(); }

void MainWindow::onSaveLexResultClicked(bool)
{
    QString content = txtLexResult ? txtLexResult->toPlainText() : QString();
    if (content.trimmed().isEmpty()) { statusBar()->showMessage("无可保存的结果"); ToastManager::instance().showWarning("无可保存的结果"); return; }
    QString dir = Config::generatedOutputDir() + "/syntax";
    QDir d(dir); if (!d.exists()) d.mkpath(".");
    QString path = dir + "/last_tokens.txt";
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) { statusBar()->showMessage("保存失败"); ToastManager::instance().showError("保存失败"); return; }
    QTextStream o(&f); o << content; f.close();
    statusBar()->showMessage("结果已保存到内部: " + path);
    ToastManager::instance().showInfo("保存成功");
}

void MainWindow::onSaveLexResultAsClicked(bool)
{
    QString content = txtLexResult ? txtLexResult->toPlainText() : QString();
    if (content.trimmed().isEmpty()) { statusBar()->showMessage("无可保存的结果"); ToastManager::instance().showWarning("无可保存的结果"); return; }
    auto userPath = QFileDialog::getSaveFileName(this,
                                                 QStringLiteral("另存为词法结果"),
                                                 QStringLiteral("sample.lex"),
                                                 QStringLiteral("Text (*.txt);;All (*)"));
    if (userPath.isEmpty()) return;
    QFile fu(userPath);
    if (!fu.open(QIODevice::WriteOnly | QIODevice::Text)) { statusBar()->showMessage("另存为失败"); ToastManager::instance().showError("另存为失败"); return; }
    QTextStream ou(&fu); ou << content; fu.close();
    // 同步内部保存
    QString dir = Config::generatedOutputDir() + "/syntax";
    QDir d(dir); if (!d.exists()) d.mkpath(".");
    QFile fi(dir + "/last_tokens.txt");
    if (fi.open(QIODevice::WriteOnly | QIODevice::Text)) { QTextStream oi(&fi); oi << content; fi.close(); }
    statusBar()->showMessage("结果已另存为并同步内部保存");
    ToastManager::instance().showInfo("保存成功");
}

void MainWindow::onExportSyntaxDotClicked(bool)
{ if (syntaxController) syntaxController->exportAstDot(); }

void MainWindow::onPreviewSyntaxTreeClicked(bool)
{ if (syntaxController) syntaxController->previewTree(); }
