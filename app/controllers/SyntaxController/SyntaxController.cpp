#include "SyntaxController.h"
#include <QWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include "../../../src/Engine.h"
#include "../../../src/config/Config.h"
#include "../../../src/syntax/SyntaxParser.h"
#include "../../../src/syntax/DotGenerator.h"
#include "../../../src/visual/DotExporter.h"
#include "../../../src/generator/SyntaxCodeGenerator.h"
#include "../../../src/syntax/LR0.h"
#include <QMenu>
#include <QAction>
#include "../../services/NotificationService/NotificationService.h"
#include "../../mainwindow.h"
#include <QProcess>
#include "../../experiments/exp2/helpers/SyntaxTableHelper.h"
#include "../../experiments/exp2/dialogs/SLRCheckDialog.h"
#include "../../experiments/exp2/dialogs/LR1TableDialog.h"
#include "../../experiments/exp2/dialogs/LR1ActionTableDialog.h"
#include "../../experiments/exp2/dialogs/LR0TableDialog.h"
#include "../../../src/syntax/LR1.h"
#include "../../../src/syntax/LR0.h"
#include "../../../src/syntax/SLR.h"
#include "../../services/DotService/DotService.h"
#include "../../components/ExportGraphButton/ExportGraphButton.h"

SyntaxController::SyntaxController(MainWindow* mw, Engine* engine, NotificationService* notify) :
    mw_(mw), engine_(engine), notify_(notify)
{
    dotSvc_ = new DotService(mw_, notify_);
}

bool SyntaxController::renderDotFromContentLocal(const QString& dotContent,
                                                 QString&       outPngPath,
                                                 int            dpi)
{
    QString tmpBase =
        QDir::tempPath() + "/syntax_ast_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    QString dotPath = tmpBase + ".dot";
    QString pngPath = tmpBase + ".png";
    QFile   f(dotPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream o(&f);
    o << dotContent;
    f.close();
    QProcess    proc;
    QStringList args;
    args << "-Tpng" << dotPath << "-o" << pngPath << QString("-Gdpi=%1").arg(dpi);
    proc.start("dot", args);
    if (!proc.waitForFinished(10000))
        return false;
    outPngPath = pngPath;
    return QFile::exists(pngPath);
}

void SyntaxController::bind(QWidget* exp2Page)
{
    page_                 = exp2Page;
    auto btnLoad          = exp2Page->findChild<QPushButton*>("btnLoadGrammar");
    auto btnParse         = exp2Page->findChild<QPushButton*>("btnParseGrammar");
    auto btnExport        = exp2Page->findChild<QPushButton*>("btnExportSyntaxDot");
    auto btnPrev          = exp2Page->findChild<QPushButton*>("btnPreviewSyntaxTree");
    auto btnRun           = exp2Page->findChild<QPushButton*>("btnRunSyntaxAnalysis");
    auto btnPrevLR0       = exp2Page->findChild<QPushButton*>("btnPreviewLR0");
    auto exportBtnLR0     = exp2Page->findChild<ExportGraphButton*>("exportBtnLR0");
    auto edtDpiLR0        = exp2Page->findChild<QLineEdit*>("edtGraphDpiLR0");
    auto btnViewLR0Table  = exp2Page->findChild<QPushButton*>("btnViewLR0Table");
    auto btnCheckSLR1     = exp2Page->findChild<QPushButton*>("btnCheckSLR1");
    auto exportBtnLR1     = exp2Page->findChild<ExportGraphButton*>("exportBtnLR1");
    auto btnPreviewLR1    = exp2Page->findChild<QPushButton*>("btnPreviewLR1");
    auto edtDpiLR1        = exp2Page->findChild<QLineEdit*>("edtGraphDpiLR1");
    auto btnViewLR1Table  = exp2Page->findChild<QPushButton*>("btnViewLR1Table");
    auto btnViewLR1Action = exp2Page->findChild<QPushButton*>("btnViewLR1Action");

    if (btnLoad)
        connect(btnLoad, &QPushButton::clicked, this, &SyntaxController::loadGrammar);
    if (btnParse)
        connect(btnParse, &QPushButton::clicked, this, &SyntaxController::parseGrammar);
    if (btnExport)
        connect(btnExport, &QPushButton::clicked, this, &SyntaxController::exportDot);
    if (btnPrev)
        connect(btnPrev, &QPushButton::clicked, this, &SyntaxController::previewTree);
    if (btnRun)
        connect(btnRun, &QPushButton::clicked, this, &SyntaxController::runSyntaxAnalysis);
    if (btnPrevLR0)
        connect(btnPrevLR0, &QPushButton::clicked, this, &SyntaxController::previewLR0);
    if (exportBtnLR0 && dotSvc_)
    {
        exportBtnLR0->setDotService(dotSvc_);
        exportBtnLR0->setSuggestedBasename("lr0_");
        exportBtnLR0->setDotSupplier(
            [this]()
            {
                auto gr = LR0Builder::build(grammar_);
                return LR0Builder::toDot(gr);
            });
        exportBtnLR0->setDpiProvider(
            [this, edtDpiLR0]()
            {
                int dpi = Config::graphvizDefaultDpi();
                if (edtDpiLR0 && !edtDpiLR0->text().trimmed().isEmpty())
                    dpi = edtDpiLR0->text().trimmed().toInt();
                return dpi;
            });
    }
    if (btnViewLR0Table)
        connect(btnViewLR0Table, &QPushButton::clicked, this, &SyntaxController::openLR0Table);
    if (btnCheckSLR1)
        connect(btnCheckSLR1, &QPushButton::clicked, this, &SyntaxController::checkSLR1);
    if (exportBtnLR1 && dotSvc_)
    {
        exportBtnLR1->setDotService(dotSvc_);
        exportBtnLR1->setSuggestedBasename("lr1_");
        exportBtnLR1->setDotSupplier(
            [this]()
            {
                auto gr = LR1Builder::build(grammar_);
                return LR1Builder::toDot(gr);
            });
        exportBtnLR1->setDpiProvider(
            [this, edtDpiLR1]()
            {
                int dpi = Config::graphvizDefaultDpi();
                if (edtDpiLR1 && !edtDpiLR1->text().trimmed().isEmpty())
                    dpi = edtDpiLR1->text().trimmed().toInt();
                return dpi;
            });
    }
    if (btnPreviewLR1)
        connect(btnPreviewLR1, &QPushButton::clicked, this, &SyntaxController::previewLR1);
    if (btnViewLR1Table)
        connect(btnViewLR1Table, &QPushButton::clicked, this, &SyntaxController::openLR1Table);
    if (btnViewLR1Action)
        connect(btnViewLR1Action,
                &QPushButton::clicked,
                this,
                [this]()
                {
                    if (!hasGrammar_)
                    {
                        notify_->warning("请先解析文法");
                        return;
                    }
                    auto                 gr  = LR1Builder::build(grammar_);
                    auto                 tbl = LR1Builder::computeActionTable(grammar_, gr);
                    LR1ActionTableDialog dlg(grammar_, tbl, mw_);
                    dlg.exec();
                });

    if (auto innerTabs = exp2Page->findChild<QTabWidget*>())
    {
        connect(innerTabs,
                &QTabWidget::currentChanged,
                [this, innerTabs](int idx)
                {
                    auto title = innerTabs->tabText(idx);
                    if (title == QStringLiteral("语法树"))
                    {
                        auto tokView = page_->findChild<QPlainTextEdit*>("txtTokensViewSyntax");
                        auto lexOut  = mw_->findChild<QPlainTextEdit*>("txtLexResult");
                        SyntaxTableHelper::syncTokensView(tokView, lexOut);
                    }
                });
    }
}

void SyntaxController::loadGrammar()
{
    auto edt  = page_->findChild<QTextEdit*>("txtInputGrammar");
    auto path = QFileDialog::getOpenFileName(mw_,
                                             QStringLiteral("选择文法文件"),
                                             QString(),
                                             QStringLiteral("Text (*.txt *.grammar);;All (*)"));
    if (path.isEmpty())
        return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        notify_->error("文件打开失败");
        return;
    }
    QTextStream in(&f);
    edt->setPlainText(in.readAll());
    f.close();
    notify_->info("文法加载成功");
}

void SyntaxController::parseGrammar()
{
    auto    edt   = page_->findChild<QTextEdit*>("txtInputGrammar");
    auto    tblF  = page_->findChild<QTableWidget*>("tblFirstSet");
    auto    tblFo = page_->findChild<QTableWidget*>("tblFollowSet");
    QString err;
    grammar_ = engine_->parseGrammarText(edt->toPlainText(), err);
    if (!err.isEmpty() || grammar_.productions.isEmpty())
    {
        notify_->error("文法错误：" + err);
        return;
    }
    hasGrammar_ = true;
    ll1_        = engine_->computeLL1(grammar_);
    if (!ll1_.conflicts.isEmpty())
        notify_->warning("存在LL(1)冲突");
    auto firstRows = engine_->firstFollowAsRows(ll1_);
    SyntaxTableHelper::fillFirstTable(tblF, firstRows);
    auto followRows = ll1_.follow;
    SyntaxTableHelper::fillFollowTable(tblFo, followRows);
    QVector<QString> nts =
        QVector<QString>(grammar_.nonterminals.begin(), grammar_.nonterminals.end());
    QVector<QString> ts  = QVector<QString>(grammar_.terminals.begin(), grammar_.terminals.end());
    QString          src = generateSyntaxParserSource(ll1_.table, nts, ts, grammar_.startSymbol);
    QString          outDir = Config::syntaxOutputDir();
    QDir             gd(outDir);
    if (!gd.exists())
        gd.mkpath(".");
    QFile fout(outDir + "/syntax_parser.cpp");
    if (fout.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream o(&fout);
        o << src;
        fout.close();
    }
    notify_->info("文法分析完成");
}

void SyntaxController::runSyntaxAnalysis()
{
    if (!hasGrammar_)
    {
        notify_->warning("请先加载并解析文法");
        return;
    }
    auto    lexOut    = mw_->findChild<QPlainTextEdit*>("txtLexResult");
    QString tokensStr = lexOut ? lexOut->toPlainText().trimmed() : QString();
    if (tokensStr.isEmpty())
    {
        notify_->warning("请先运行词法分析获得Token序列");
        return;
    }
    QVector<QString> tokens;
    for (auto s : tokensStr.split(' ', Qt::SkipEmptyParts)) tokens.push_back(s);
    if (auto tokView = page_->findChild<QPlainTextEdit*>("txtTokensViewSyntax"))
        tokView->setPlainText(tokensStr);
    auto res = parseTokens(tokens, grammar_, ll1_);
    if (res.errorPos >= 0)
    {
        notify_->error("语法错误");
        return;
    }
    auto    dot = syntaxAstToDot(res.root);
    QString pngPath;
    int     dpi = Config::graphvizDefaultDpi();
    if (!SyntaxController::renderDotFromContentLocal(dot, pngPath, dpi))
    {
        notify_->error("语法树渲染失败");
        return;
    }
    mw_->previewImage(pngPath, "语法树 预览");
    QFile::remove(pngPath);
    notify_->info("语法分析成功");
}

void SyntaxController::exportDot()
{
    if (!hasGrammar_)
        return;
    auto             lexOut    = mw_->findChild<QPlainTextEdit*>("txtLexResult");
    QString          tokensStr = lexOut ? lexOut->toPlainText().trimmed() : QString();
    QVector<QString> tokens;
    for (auto s : tokensStr.split(' ', Qt::SkipEmptyParts)) tokens.push_back(s);
    auto res = parseTokens(tokens, grammar_, ll1_);
    if (res.errorPos >= 0)
    {
        notify_->error("语法错误");
        return;
    }
    QString dot  = syntaxAstToDot(res.root);
    QString base = Config::graphsDir() + "/syntax";
    QDir    d(base);
    if (!d.exists())
        d.mkpath(".");
    QString ts  = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString out = base + "/ast_" + ts + ".dot";
    QFile   f(out);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream o(&f);
        o << dot;
        f.close();
        notify_->info("DOT 已导出");
    }
}

void SyntaxController::previewTree()
{
    if (!hasGrammar_)
        return;
    auto             lexOut    = mw_->findChild<QPlainTextEdit*>("txtLexResult");
    QString          tokensStr = lexOut ? lexOut->toPlainText().trimmed() : QString();
    QVector<QString> tokens;
    for (auto s : tokensStr.split(' ', Qt::SkipEmptyParts)) tokens.push_back(s);
    auto res = parseTokens(tokens, grammar_, ll1_);
    if (res.errorPos >= 0)
    {
        notify_->error("语法错误");
        return;
    }
    QString pngPath;
    int     dpi = Config::graphvizDefaultDpi();
    if (!SyntaxController::renderDotFromContentLocal(syntaxAstToDot(res.root), pngPath, dpi))
    {
        notify_->error("渲染失败");
        return;
    }
    mw_->previewImage(pngPath, "语法树 预览");
    QFile::remove(pngPath);
    notify_->info("预览已生成");
}

void SyntaxController::previewLR0()
{
    if (!hasGrammar_)
    {
        notify_->warning("请先解析文法");
        return;
    }
    auto    gr  = LR0Builder::build(grammar_);
    QString dot = LR0Builder::toDot(gr);
    int     dpi = Config::graphvizDefaultDpi();
    if (auto e = page_->findChild<QLineEdit*>("edtGraphDpiLR0");
        e && !e->text().trimmed().isEmpty())
        dpi = e->text().trimmed().toInt();
    if (!dotSvc_)
        return;
    QString png;
    if (!dotSvc_->renderToTempPng(dot, png, dpi))
    {
        notify_->error("渲染失败");
        return;
    }
    dotSvc_->previewPng(png, "LR(0) 项集 DFA 预览");
    QFile::remove(png);
}

void SyntaxController::exportLR0Dot()
{
    if (!hasGrammar_)
        return;
    auto    gr   = LR0Builder::build(grammar_);
    QString dot  = LR0Builder::toDot(gr);
    QString ts   = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString def  = "lr0_" + ts + ".dot";
    QString path = dotSvc_ ? dotSvc_->pickDotSavePath(def) : QString();
    if (path.isEmpty())
        return;
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream o(&f);
        o << dot;
        f.close();
        notify_->info("DOT 已导出");
    }
}

void SyntaxController::checkSLR1()
{
    if (!hasGrammar_)
    {
        notify_->warning("请先解析文法");
        return;
    }
    auto    r       = SLR::check(grammar_, ll1_);
    QString summary = r.isSLR1 ? QStringLiteral("该文法为 SLR(1) 文法")
                               : QStringLiteral("该文法不是 SLR(1) 文法");
    QString detail;
    for (const auto& c : r.conflicts)
    {
        detail += QString("[state %1][%2] %3: %4\n")
                      .arg(c.state)
                      .arg(c.terminal)
                      .arg(c.type)
                      .arg(c.detail);
    }
    SLRCheckDialog dlg(r, mw_);
    dlg.exec();
}

void SyntaxController::exportLR1Dot()
{
    if (!hasGrammar_)
        return;
    auto    gr   = LR1Builder::build(grammar_);
    QString dot  = LR1Builder::toDot(gr);
    QString ts   = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString def  = "lr1_" + ts + ".dot";
    QString path = dotSvc_ ? dotSvc_->pickDotSavePath(def) : QString();
    if (path.isEmpty())
        return;
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream o(&f);
        o << dot;
        f.close();
        notify_->info("DOT 已导出");
    }
}

void SyntaxController::previewLR1()
{
    if (!hasGrammar_)
    {
        notify_->warning("请先解析文法");
        return;
    }
    auto    gr  = LR1Builder::build(grammar_);
    QString dot = LR1Builder::toDot(gr);
    int     dpi = Config::graphvizDefaultDpi();
    if (auto e = page_->findChild<QLineEdit*>("edtGraphDpiLR1");
        e && !e->text().trimmed().isEmpty())
        dpi = e->text().trimmed().toInt();
    if (!dotSvc_)
        return;
    QString png;
    if (!dotSvc_->renderToTempPng(dot, png, dpi))
    {
        notify_->error("渲染失败");
        return;
    }
    dotSvc_->previewPng(png, "LR(1) DFA 预览");
    QFile::remove(png);
}

void SyntaxController::openLR1Table()
{
    if (!hasGrammar_)
    {
        notify_->warning("请先解析文法");
        return;
    }
    auto           gr = LR1Builder::build(grammar_);
    LR1TableDialog dlg(gr, mw_);
    dlg.exec();
}

void SyntaxController::openLR0Table()
{
    if (!hasGrammar_)
    {
        notify_->warning("请先解析文法");
        return;
    }
    auto           gr = LR0Builder::build(grammar_);
    LR0TableDialog dlg(gr, mw_);
    dlg.exec();
}

void SyntaxController::exportAstDot()
{
    if (!hasGrammar_)
        return;
    auto             lexOut    = mw_->findChild<QPlainTextEdit*>("txtLexResult");
    QString          tokensStr = lexOut ? lexOut->toPlainText().trimmed() : QString();
    QVector<QString> tokens;
    for (auto s : tokensStr.split(' ', Qt::SkipEmptyParts)) tokens.push_back(s);
    auto res = parseTokens(tokens, grammar_, ll1_);
    if (res.errorPos >= 0)
    {
        notify_->error("语法错误");
        return;
    }
    QString dot  = syntaxAstToDot(res.root);
    QString base = Config::generatedOutputDir() + "/syntax/graphs";
    QDir    d(base);
    if (!d.exists())
        d.mkpath(".");
    QString ts  = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString out = base + "/ast_" + ts + ".dot";
    QFile   f(out);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream o(&f);
        o << dot;
        f.close();
        notify_->info("DOT 已导出");
    }
}
