#include "GeneratorController.h"
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QStatusBar>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "../../../src/generator/CodeGenerator.h"
#include "../../../src/Engine.h"
#include "../../../src/config/Config.h"
#include "../../services/FileService/FileService.h"
#include "../../services/NotificationService/NotificationService.h"
#include "../../mainwindow.h"

GeneratorController::GeneratorController(MainWindow*          mw,
                                         Engine*              engine,
                                         NotificationService* notify) :
    mw_(mw), engine_(engine), notify_(notify)
{
}

void GeneratorController::bind(QWidget* regexTab, QWidget* codeViewTab)
{
    regexTab_ = regexTab;
    codeTab_  = codeViewTab;
    if (auto btn = regexTab_->findChild<QPushButton*>("btnStartConvert"))
        QObject::connect(btn, &QPushButton::clicked, this, &GeneratorController::convert);
    if (auto genBtn = codeViewTab->findChild<QPushButton*>("btnCompileRun"))
        QObject::connect(genBtn, &QPushButton::clicked, this, &GeneratorController::compileAndRun);
    if (auto minTab = mw_->findChild<QWidget*>(QString()))
    {
        // 生成代码按钮在 MinDFA 页签
        auto btnGen = mw_->findChild<QPushButton*>("btnGenCode");
        if (btnGen)
            QObject::connect(
                btnGen, &QPushButton::clicked, this, &GeneratorController::generateCode);
    }
}

void GeneratorController::convert()
{
    auto edit   = regexTab_->findChild<QTextEdit*>("txtInputRegex");
    auto cmbNfa = mw_->findChild<QComboBox*>("cmbTokens");
    auto cmbDfa = mw_->findChild<QComboBox*>("cmbTokensDFA");
    auto cmbMin = mw_->findChild<QComboBox*>("cmbTokensMin");
    if (!edit || !cmbNfa || !cmbDfa || !cmbMin)
        return;
    auto text   = edit->toPlainText();
    auto rf     = engine_->lexFile(text);
    auto parsed = engine_->parseFile(rf);
    if (parsed.tokens.isEmpty())
    {
        notify_->warning("未找到Token定义");
        return;
    }
    mw_->setParsed(new ParsedFile(parsed));
    mw_->setRegexHash(mw_->computeRegexHashPublic(text));
    mw_->clearPaths();
    cmbNfa->blockSignals(true);
    cmbNfa->clear();
    cmbNfa->addItem("全部");
    for (const auto& t : parsed.tokens) cmbNfa->addItem(t.rule.name);
    cmbNfa->blockSignals(false);
    cmbNfa->setCurrentIndex(0);
    cmbDfa->blockSignals(true);
    cmbDfa->clear();
    cmbDfa->addItem("全部");
    for (const auto& t : parsed.tokens) cmbDfa->addItem(t.rule.name);
    cmbDfa->blockSignals(false);
    cmbDfa->setCurrentIndex(0);
    cmbMin->blockSignals(true);
    cmbMin->clear();
    cmbMin->addItem("全部");
    for (const auto& t : parsed.tokens) cmbMin->addItem(t.rule.name);
    cmbMin->blockSignals(false);
    cmbMin->setCurrentIndex(0);
    if (mw_->statusBar())
        mw_->statusBar()->showMessage("转换成功");
    notify_->info("转换成功");
}

void GeneratorController::generateCode()
{
    if (!mw_->getParsed())
    {
        notify_->warning("请先转换");
        return;
    }
    QVector<int> codes;
    auto         mdfas    = engine_->buildAllMinDFA(*mw_->getParsed(), codes);
    auto         s        = CodeGenerator::generateCombined(mdfas, codes, mw_->getParsed()->alpha);
    auto         codeView = codeTab_->findChild<QPlainTextEdit*>("txtGeneratedCode");
    if (codeView)
        codeView->setPlainText(s);
    QString base    = mw_->ensureGenDirPublic();
    QString ts      = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString curHash = mw_->computeRegexHashPublic(
        regexTab_->findChild<QTextEdit*>("txtInputRegex")->toPlainText());
    QString hash     = curHash;
    QString savePath = base + "/lex_" + ts + "_" + hash.mid(0, 12) + ".cpp";
    FileService::writeAllText(savePath, s);
    mw_->setCodePaths(savePath, base + "/bin/" + QFileInfo(savePath).completeBaseName());
    notify_->info("组合扫描器代码已生成");
}

void GeneratorController::compileAndRun()
{
    if (!mw_->getParsed())
    {
        auto edit = regexTab_->findChild<QTextEdit*>("txtInputRegex");
        if (!edit)
        {
            notify_->warning("未找到正则输入控件");
            return;
        }
        auto rf     = engine_->lexFile(edit->toPlainText());
        auto parsed = engine_->parseFile(rf);
        if (parsed.tokens.isEmpty())
        {
            notify_->warning("未找到Token定义");
            return;
        }
        mw_->setParsed(new ParsedFile(parsed));
    }
    QVector<int> codes;
    auto         mdfas   = engine_->buildAllMinDFA(*mw_->getParsed(), codes);
    auto         outPane = mw_->findChild<QPlainTextEdit*>("txtLexResult");
    auto         srcPane = mw_->findChild<QPlainTextEdit*>("txtSourceTiny");
    QString      src     = srcPane ? srcPane->toPlainText() : QString();
    if (src.trimmed().isEmpty())
        src = QStringLiteral("abc123 def456");
    auto output = engine_->runMultiple(mdfas, codes, src);
    if (outPane)
        outPane->setPlainText(output);
    notify_->info("生成器运行完成");
}
