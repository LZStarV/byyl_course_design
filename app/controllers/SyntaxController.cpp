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
#include "../../src/Engine.h"
#include "../../src/config/Config.h"
#include "../../src/syntax/SyntaxParser.h"
#include "../../src/syntax/DotGenerator.h"
#include "../../src/visual/DotExporter.h"
#include "../../src/generator/SyntaxCodeGenerator.h"
#include "../services/NotificationService.h"
#include "../mainwindow.h"
#include <QProcess>

SyntaxController::SyntaxController(MainWindow* mw, Engine* engine, NotificationService* notify)
    : mw_(mw), engine_(engine), notify_(notify) {}

bool SyntaxController::renderDotFromContentLocal(const QString& dotContent, QString& outPngPath, int dpi)
{
    QString tmpBase = QDir::tempPath() + "/syntax_ast_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    QString dotPath = tmpBase + ".dot";
    QString pngPath = tmpBase + ".png";
    QFile f(dotPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream o(&f); o << dotContent; f.close();
    QProcess proc;
    QStringList args; args << "-Tpng" << dotPath << "-o" << pngPath << QString("-Gdpi=%1").arg(dpi);
    proc.start("dot", args);
    if (!proc.waitForFinished(10000)) return false;
    outPngPath = pngPath;
    return QFile::exists(pngPath);
}

void SyntaxController::bind(QWidget* exp2Page)
{
    page_ = exp2Page;
    auto btnLoad   = exp2Page->findChild<QPushButton*>("btnLoadGrammar");
    auto btnParse  = exp2Page->findChild<QPushButton*>("btnParseGrammar");
    auto btnExport = exp2Page->findChild<QPushButton*>("btnExportSyntaxDot");
    auto btnPrev   = exp2Page->findChild<QPushButton*>("btnPreviewSyntaxTree");
    auto btnRun    = exp2Page->findChild<QPushButton*>("btnRunSyntaxAnalysis");
    if (btnLoad)  connect(btnLoad,  &QPushButton::clicked, this, &SyntaxController::loadGrammar);
    if (btnParse) connect(btnParse, &QPushButton::clicked, this, &SyntaxController::parseGrammar);
    if (btnExport)connect(btnExport,&QPushButton::clicked, this, &SyntaxController::exportDot);
    if (btnPrev)  connect(btnPrev,  &QPushButton::clicked, this, &SyntaxController::previewTree);
    if (btnRun)   connect(btnRun,   &QPushButton::clicked, this, &SyntaxController::runSyntaxAnalysis);
}

void SyntaxController::loadGrammar()
{
    auto edt = page_->findChild<QTextEdit*>("txtInputGrammar");
    auto path = QFileDialog::getOpenFileName(mw_, QStringLiteral("选择文法文件"), QString(), QStringLiteral("Text (*.txt *.grammar);;All (*)"));
    if (path.isEmpty()) return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) { notify_->error("文件打开失败"); return; }
    QTextStream in(&f);
    edt->setPlainText(in.readAll());
    f.close();
    notify_->info("文法加载成功");
}

void SyntaxController::parseGrammar()
{
    auto edt = page_->findChild<QTextEdit*>("txtInputGrammar");
    auto tblF = page_->findChild<QTableWidget*>("tblFirstSet");
    auto tblFo = page_->findChild<QTableWidget*>("tblFollowSet");
    auto tblP = page_->findChild<QTableWidget*>("tblParsingTable");
    QString err;
    grammar_ = engine_->parseGrammarText(edt->toPlainText(), err);
    if (!err.isEmpty() || grammar_.productions.isEmpty()) { notify_->error("文法错误：" + err); return; }
    hasGrammar_ = true;
    ll1_ = engine_->computeLL1(grammar_);
    if (!ll1_.conflicts.isEmpty()) notify_->warning("存在LL(1)冲突");
    tblF->clear(); tblFo->clear(); tblP->clear();
    tblF->setColumnCount(2); tblFo->setColumnCount(2);
    QStringList hf; hf << "非终结符" << "First"; tblF->setHorizontalHeaderLabels(hf);
    QStringList hfo; hfo << "非终结符" << "Follow"; tblFo->setHorizontalHeaderLabels(hfo);
    auto firstRows = engine_->firstFollowAsRows(ll1_);
    tblF->setRowCount(firstRows.size());
    int r = 0; for (auto it = firstRows.begin(); it != firstRows.end(); ++it) { tblF->setItem(r,0,new QTableWidgetItem(it.key())); tblF->setItem(r,1,new QTableWidgetItem(it.value().join(", "))); r++; }
    auto followRows = ll1_.follow; tblFo->setRowCount(followRows.size()); r=0;
    for (auto it = followRows.begin(); it != followRows.end(); ++it) { tblFo->setItem(r,0,new QTableWidgetItem(it.key())); QStringList vals = QStringList(it.value().begin(), it.value().end()); tblFo->setItem(r,1,new QTableWidgetItem(vals.join(", "))); r++; }
    QStringList terms; for (auto t : grammar_.terminals) terms << t; terms << "$";
    tblP->setColumnCount(terms.size()+1); QStringList hp; hp << "非终结符"; for (auto t:terms) hp<<t; tblP->setHorizontalHeaderLabels(hp);
    tblP->setRowCount(grammar_.nonterminals.size()); r=0;
    for (auto A : grammar_.nonterminals)
    {
        tblP->setItem(r,0,new QTableWidgetItem(A));
        for (int c=0;c<terms.size();++c)
        {
            QString a=terms[c]; int idx=ll1_.table.value(A).value(a,-1); QString val;
            if (idx>=0){ const auto& p = grammar_.productions[A][idx]; val = p.left + " -> "; for (int i=0;i<p.right.size();++i){ val+=p.right[i]; if (i+1<p.right.size()) val+=" "; } }
            tblP->setItem(r,c+1,new QTableWidgetItem(val));
        }
        r++;
    }
    QVector<QString> nts = QVector<QString>(grammar_.nonterminals.begin(), grammar_.nonterminals.end());
    QVector<QString> ts = QVector<QString>(grammar_.terminals.begin(), grammar_.terminals.end());
    QString src = generateSyntaxParserSource(ll1_.table, nts, ts, grammar_.startSymbol);
    QString outDir = Config::generatedOutputDir() + "/syntax"; QDir gd(outDir); if (!gd.exists()) gd.mkpath(".");
    QFile fout(outDir + "/syntax_parser.cpp"); if (fout.open(QIODevice::WriteOnly | QIODevice::Text)) { QTextStream o(&fout); o << src; fout.close(); }
    notify_->info("文法分析完成");
}

void SyntaxController::runSyntaxAnalysis()
{
    if (!hasGrammar_) { notify_->warning("请先加载并解析文法"); return; }
    auto lexOut = mw_->findChild<QPlainTextEdit*>("txtLexResult");
    QString tokensStr = lexOut ? lexOut->toPlainText().trimmed() : QString();
    if (tokensStr.isEmpty()) { notify_->warning("请先运行词法分析获得Token序列"); return; }
    QVector<QString> tokens; for (auto s : tokensStr.split(' ', Qt::SkipEmptyParts)) tokens.push_back(s);
    auto res = parseTokens(tokens, grammar_, ll1_);
    if (res.errorPos >= 0) { notify_->error("语法错误"); return; }
    auto dot = syntaxAstToDot(res.root); QString pngPath; int dpi=150;
    if (!SyntaxController::renderDotFromContentLocal(dot, pngPath, dpi)) { notify_->error("语法树渲染失败"); return; }
    mw_->previewImage(pngPath, "语法树 预览"); QFile::remove(pngPath); notify_->info("语法分析成功");
}

void SyntaxController::exportDot()
{
    if (!hasGrammar_) return;
    auto lexOut = mw_->findChild<QPlainTextEdit*>("txtLexResult"); QString tokensStr = lexOut ? lexOut->toPlainText().trimmed() : QString(); QVector<QString> tokens; for (auto s: tokensStr.split(' ', Qt::SkipEmptyParts)) tokens.push_back(s);
    auto res = parseTokens(tokens, grammar_, ll1_); if (res.errorPos >= 0) { notify_->error("语法错误"); return; }
    QString dot = syntaxAstToDot(res.root); QString base = Config::generatedOutputDir() + "/syntax/graphs"; QDir d(base); if (!d.exists()) d.mkpath(".");
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"); QString out = base + "/ast_" + ts + ".dot";
    QFile f(out); if (f.open(QIODevice::WriteOnly | QIODevice::Text)) { QTextStream o(&f); o << dot; f.close(); notify_->info("DOT 已导出"); }
}

void SyntaxController::previewTree()
{
    if (!hasGrammar_) return;
    auto lexOut = mw_->findChild<QPlainTextEdit*>("txtLexResult"); QString tokensStr = lexOut ? lexOut->toPlainText().trimmed() : QString(); QVector<QString> tokens; for (auto s: tokensStr.split(' ', Qt::SkipEmptyParts)) tokens.push_back(s);
    auto res = parseTokens(tokens, grammar_, ll1_); if (res.errorPos >= 0) { notify_->error("语法错误"); return; }
    QString pngPath; int dpi=150; if (!SyntaxController::renderDotFromContentLocal(syntaxAstToDot(res.root), pngPath, dpi)) { notify_->error("渲染失败"); return; }
    mw_->previewImage(pngPath, "语法树 预览"); QFile::remove(pngPath); notify_->info("预览已生成");
}
static bool renderDotFromContentLocal(const QString& dotContent, QString& outPngPath, int dpi)
{
    QString tmpBase = QDir::tempPath() + "/syntax_ast_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    QString dotPath = tmpBase + ".dot";
    QString pngPath = tmpBase + ".png";
    QFile f(dotPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream o(&f); o << dotContent; f.close();
    QProcess proc;
    QStringList args; args << "-Tpng" << dotPath << "-o" << pngPath << QString("-Gdpi=%1").arg(dpi);
    proc.start("dot", args);
    if (!proc.waitForFinished(10000)) return false;
    outPngPath = pngPath;
    return QFile::exists(pngPath);
}
