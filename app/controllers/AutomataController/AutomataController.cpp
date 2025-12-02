#include "AutomataController.h"
#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QMenu>
#include <QAction>
#include <QTableWidget>
#include <QLineEdit>
#include <QDateTime>
#include <QFile>
#include <QStatusBar>
#include "../../mainwindow.h"
#include "../../services/DotService/DotService.h"
#include "../../../src/Engine.h"
#include "../../../src/visual/DotExporter.h"

AutomataController::AutomataController(MainWindow* mw) : mw_(mw) {}

void AutomataController::bind(QWidget* root)
{
    root_ = root;
    dot_ = new DotService(mw_, mw_->notifyPtr());
    auto btnExpNfa = root->findChild<QPushButton*>("btnExportNFA");
    auto btnPrevNfa= root->findChild<QPushButton*>("btnPreviewNFA");
    auto btnExpDfa = root->findChild<QPushButton*>("btnExportDFA");
    auto btnPrevDfa= root->findChild<QPushButton*>("btnPreviewDFA");
    auto btnExpMin = root->findChild<QPushButton*>("btnExportMin");
    auto btnPrevMin= root->findChild<QPushButton*>("btnPreviewMin");
    cmbTok_    = root->findChild<QComboBox*>("cmbTokens");
    cmbTokDfa_ = root->findChild<QComboBox*>("cmbTokensDFA");
    cmbTokMin_ = root->findChild<QComboBox*>("cmbTokensMin");
    tblNfa_ = root->findChild<QTableWidget*>("tblNFA");
    tblDfa_ = root->findChild<QTableWidget*>("tblDFA");
    tblMin_ = root->findChild<QTableWidget*>("tblMinDFA");
    if (btnExpNfa)
    {
        auto menu = new QMenu(btnExpNfa);
        auto actDot = menu->addAction("导出DOT...");
        auto actImg = menu->addAction("导出图片...");
        btnExpNfa->setMenu(menu);
        QObject::connect(actDot, &QAction::triggered, this, &AutomataController::exportNfaDot);
        QObject::connect(actImg, &QAction::triggered, this, &AutomataController::exportNfaImage);
    }
    if (btnPrevNfa) QObject::connect(btnPrevNfa, &QPushButton::clicked, this, &AutomataController::previewNfa);
    if (btnExpDfa)
    {
        auto menu = new QMenu(btnExpDfa);
        auto actDot = menu->addAction("导出DOT...");
        auto actImg = menu->addAction("导出图片...");
        btnExpDfa->setMenu(menu);
        QObject::connect(actDot, &QAction::triggered, this, &AutomataController::exportDfaDot);
        QObject::connect(actImg, &QAction::triggered, this, &AutomataController::exportDfaImage);
    }
    if (btnPrevDfa) QObject::connect(btnPrevDfa, &QPushButton::clicked, this, &AutomataController::previewDfa);
    if (btnExpMin)
    {
        auto menu = new QMenu(btnExpMin);
        auto actDot = menu->addAction("导出DOT...");
        auto actImg = menu->addAction("导出图片...");
        btnExpMin->setMenu(menu);
        QObject::connect(actDot, &QAction::triggered, this, &AutomataController::exportMinDot);
        QObject::connect(actImg, &QAction::triggered, this, &AutomataController::exportMinImage);
    }
    if (btnPrevMin) QObject::connect(btnPrevMin, &QPushButton::clicked, this, &AutomataController::previewMin);
    if (cmbTok_)    QObject::connect(cmbTok_,    QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AutomataController::onTokenChanged);
    if (cmbTokDfa_) QObject::connect(cmbTokDfa_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AutomataController::onTokenChangedDFA);
    if (cmbTokMin_) QObject::connect(cmbTokMin_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AutomataController::onTokenChangedMin);
}

void AutomataController::fillTable(QTableWidget* tbl, const Tables& t)
{
    if (!tbl) return;
    tbl->clear(); tbl->setColumnCount(t.columns.size()); tbl->setRowCount(t.rows.size());
    QStringList headers; for (auto c: t.columns) headers << c; tbl->setHorizontalHeaderLabels(headers);
    for (int r = 0; r < t.rows.size(); ++r) { auto row = t.rows[r]; for (int c = 0; c < row.size(); ++c) { tbl->setItem(r, c, new QTableWidgetItem(row[c])); } }
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

static QVector<QString> unionSyms(const QVector<Tables>& tables, bool includeEps)
{
    QSet<QString> s; for (const auto& t : tables) { for (int i = 2; i < t.columns.size(); ++i) { QString c = t.columns[i]; if (includeEps || c != "#") s.insert(c); } }
    QVector<QString> v = QVector<QString>(s.begin(), s.end()); std::sort(v.begin(), v.end()); return v;
}

void AutomataController::onTokenChanged(int idx)
{
    auto parsed = mw_->getParsed(); if (!parsed) return; auto eng = mw_->getEngine();
    if (idx == 0) { fillAllNFA(); return; }
    if (idx - 1 < 0 || idx - 1 >= parsed->tokens.size()) return;
    auto pt = parsed->tokens[idx - 1]; auto nfa = eng->buildNFA(pt.ast, parsed->alpha); auto dfa = eng->buildDFA(nfa); auto mdfa = eng->buildMinDFA(dfa);
    auto tn = eng->nfaTable(nfa); fillTable(tblNfa_, tn);
    auto td = eng->dfaTable(dfa); fillTable(tblDfa_, td);
    auto tm = eng->minTable(mdfa); fillTable(tblMin_, tm);
}

void AutomataController::onTokenChangedDFA(int idx)
{
    auto parsed = mw_->getParsed(); if (!parsed) return; auto eng = mw_->getEngine();
    if (idx == 0) { fillAllDFA(); return; }
    if (idx - 1 < 0 || idx - 1 >= parsed->tokens.size()) return;
    auto pt = parsed->tokens[idx - 1]; auto nfa = eng->buildNFA(pt.ast, parsed->alpha); auto dfa = eng->buildDFA(nfa); auto mdfa = eng->buildMinDFA(dfa);
    auto td = eng->dfaTable(dfa); fillTable(tblDfa_, td);
    auto tm = eng->minTable(mdfa); fillTable(tblMin_, tm);
}

void AutomataController::onTokenChangedMin(int idx)
{
    auto parsed = mw_->getParsed(); if (!parsed) return; auto eng = mw_->getEngine();
    if (idx == 0) { fillAllMin(); return; }
    if (idx - 1 < 0 || idx - 1 >= parsed->tokens.size()) return;
    auto pt = parsed->tokens[idx - 1]; auto nfa = eng->buildNFA(pt.ast, parsed->alpha); auto dfa = eng->buildDFA(nfa); auto mdfa = eng->buildMinDFA(dfa);
    auto tm = eng->minTable(mdfa); fillTable(tblMin_, tm);
}

void AutomataController::fillAllNFA()
{
    auto parsed = mw_->getParsed(); if (!parsed) return; auto eng = mw_->getEngine();
    QVector<Tables> parts; for (const auto& tok : parsed->tokens) { auto nfa = eng->buildNFA(tok.ast, parsed->alpha); parts.push_back(eng->nfaTable(nfa)); }
    auto syms = unionSyms(parts, true); Tables t; t.columns.clear(); t.columns.push_back("标记"); t.columns.push_back("状态 ID"); for (auto c : syms) { if (c != "#") t.columns.push_back(c); } t.columns.push_back("#");
    for (int k = 0; k < parts.size(); ++k) { Tables pt = parts[k]; QVector<QString> sep; sep << "Token" << parsed->tokens[k].rule.name; for (int i = 2; i < t.columns.size(); ++i) sep << QString(); t.rows.push_back(sep);
        for (const auto& row : pt.rows) { QVector<QString> newRow; newRow << row[0] << row[1]; for (int ci = 2; ci < t.columns.size(); ++ci) { QString col = t.columns[ci]; int idx = -1; for (int j = 2; j < pt.columns.size(); ++j) { if (pt.columns[j] == col) { idx = j; break; } } newRow << (idx == -1 ? QString() : row[idx]); } t.rows.push_back(newRow); } }
    fillTable(tblNfa_, t);
}

void AutomataController::fillAllDFA()
{
    auto parsed = mw_->getParsed(); if (!parsed) return; auto eng = mw_->getEngine();
    QVector<Tables> parts; for (const auto& tok : parsed->tokens) { auto nfa = eng->buildNFA(tok.ast, parsed->alpha); auto dfa = eng->buildDFA(nfa); parts.push_back(eng->dfaTable(dfa)); }
    auto syms = unionSyms(parts, false); Tables t; t.columns.clear(); t.columns.push_back("标记"); t.columns.push_back("状态集合"); for (auto c : syms) { t.columns.push_back(c); }
    for (int k = 0; k < parts.size(); ++k) { Tables pt = parts[k]; QVector<QString> sep; sep << "Token" << parsed->tokens[k].rule.name; for (int i = 2; i < t.columns.size(); ++i) sep << QString(); t.rows.push_back(sep);
        for (const auto& row : pt.rows) { QVector<QString> newRow; newRow << row[0] << row[1]; for (int ci = 2; ci < t.columns.size(); ++ci) { QString col = t.columns[ci]; int idx = -1; for (int j = 2; j < pt.columns.size(); ++j) { if (pt.columns[j] == col) { idx = j; break; } } newRow << (idx == -1 ? QString() : row[idx]); } t.rows.push_back(newRow); } }
    fillTable(tblDfa_, t);
}

void AutomataController::fillAllMin()
{
    auto parsed = mw_->getParsed(); if (!parsed) return; auto eng = mw_->getEngine();
    QVector<Tables> parts; for (const auto& tok : parsed->tokens) { auto nfa = eng->buildNFA(tok.ast, parsed->alpha); auto dfa = eng->buildDFA(nfa); auto mdfa = eng->buildMinDFA(dfa); parts.push_back(eng->minTable(mdfa)); }
    auto syms = unionSyms(parts, false); Tables t; t.columns.clear(); t.columns.push_back("标记"); t.columns.push_back("状态 ID"); for (auto c : syms) { t.columns.push_back(c); }
    for (int k = 0; k < parts.size(); ++k) { Tables pt = parts[k]; QVector<QString> sep; sep << "Token" << parsed->tokens[k].rule.name; for (int i = 2; i < t.columns.size(); ++i) sep << QString(); t.rows.push_back(sep);
        for (const auto& row : pt.rows) { QVector<QString> newRow; newRow << row[0] << row[1]; for (int ci = 2; ci < t.columns.size(); ++ci) { QString col = t.columns[ci]; int idx = -1; for (int j = 2; j < pt.columns.size(); ++j) { if (pt.columns[j] == col) { idx = j; break; } } newRow << (idx == -1 ? QString() : row[idx]); } t.rows.push_back(newRow); } }
    fillTable(tblMin_, t);
}

void AutomataController::exportNfaDot()
{
    auto parsed = mw_->getParsed(); if (!parsed) return; int idx = cmbTok_ ? cmbTok_->currentIndex() : -1; if (idx <= 0 || idx - 1 >= parsed->tokens.size()) return;
    auto pt = parsed->tokens[idx - 1]; auto nfa = mw_->getEngine()->buildNFA(pt.ast, parsed->alpha);
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"); QString suggest = "nfa_" + pt.rule.name + "_" + ts + ".dot";
    QString outPath = dot_->pickDotSavePath(suggest); if (outPath.isEmpty()) return;
    if (!DotExporter::exportToDot(nfa, outPath)) { mw_->statusBar()->showMessage("DOT文件写入失败"); return; }
    mw_->statusBar()->showMessage("NFA DOT已导出: " + outPath);
}

void AutomataController::exportNfaImage()
{
    auto parsed = mw_->getParsed(); if (!parsed) return; int idx = cmbTok_ ? cmbTok_->currentIndex() : -1; if (idx <= 0 || idx - 1 >= parsed->tokens.size()) return;
    auto pt = parsed->tokens[idx - 1]; auto nfa = mw_->getEngine()->buildNFA(pt.ast, parsed->alpha);
    auto dpiEdit = root_->findChild<QLineEdit*>("edtGraphDpiNfa"); int dpi = (dpiEdit && !dpiEdit->text().trimmed().isEmpty()) ? dpiEdit->text().trimmed().toInt() : 150;
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"); QString suggest = "nfa_" + pt.rule.name + "_" + ts + ".png";
    QString outPath = dot_->pickImageSavePath(suggest, "png"); if (outPath.isEmpty()) return;
    if (!dot_->renderToFile(DotExporter::toDot(nfa), outPath, "png", dpi)) { mw_->statusBar()->showMessage("图片导出失败"); return; }
    mw_->statusBar()->showMessage("NFA 图片已导出: " + outPath);
}

void AutomataController::previewNfa()
{
    auto parsed = mw_->getParsed(); if (!parsed) return; int idx = cmbTok_ ? cmbTok_->currentIndex() : -1; if (idx <= 0 || idx - 1 >= parsed->tokens.size()) { mw_->statusBar()->showMessage("请选择具体Token后预览NFA"); return; }
    auto pt = parsed->tokens[idx - 1]; auto nfa = mw_->getEngine()->buildNFA(pt.ast, parsed->alpha); auto dpiEdit = root_->findChild<QLineEdit*>("edtGraphDpiNfa"); int dpi = (dpiEdit && !dpiEdit->text().trimmed().isEmpty()) ? dpiEdit->text().trimmed().toInt() : 150;
    QString pngPath; if (!dot_->renderToTempPng(DotExporter::toDot(nfa), pngPath, dpi)) { mw_->statusBar()->showMessage("Graphviz渲染失败，请确认已安装dot"); return; }
    dot_->previewPng(pngPath, "NFA 预览"); QFile::remove(pngPath);
}

void AutomataController::exportDfaDot()
{
    auto parsed = mw_->getParsed(); if (!parsed) return; int idx = cmbTokDfa_ ? cmbTokDfa_->currentIndex() : -1; if (idx <= 0 || idx - 1 >= parsed->tokens.size()) return;
    auto pt = parsed->tokens[idx - 1]; auto nfa = mw_->getEngine()->buildNFA(pt.ast, parsed->alpha); auto dfa = mw_->getEngine()->buildDFA(nfa);
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"); QString suggest = "dfa_" + pt.rule.name + "_" + ts + ".dot";
    QString outPath = dot_->pickDotSavePath(suggest); if (outPath.isEmpty()) return;
    if (!DotExporter::exportToDot(dfa, outPath)) { mw_->statusBar()->showMessage("DOT文件写入失败"); return; }
    mw_->statusBar()->showMessage("DFA DOT已导出: " + outPath);
}

void AutomataController::exportDfaImage()
{
    auto parsed = mw_->getParsed(); if (!parsed) return; int idx = cmbTokDfa_ ? cmbTokDfa_->currentIndex() : -1; if (idx <= 0 || idx - 1 >= parsed->tokens.size()) return;
    auto pt = parsed->tokens[idx - 1]; auto nfa = mw_->getEngine()->buildNFA(pt.ast, parsed->alpha); auto dfa = mw_->getEngine()->buildDFA(nfa);
    auto dpiEdit = root_->findChild<QLineEdit*>("edtGraphDpiDfa"); int dpi = (dpiEdit && !dpiEdit->text().trimmed().isEmpty()) ? dpiEdit->text().trimmed().toInt() : 150;
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"); QString suggest = "dfa_" + pt.rule.name + "_" + ts + ".png";
    QString outPath = dot_->pickImageSavePath(suggest, "png"); if (outPath.isEmpty()) return;
    if (!dot_->renderToFile(DotExporter::toDot(dfa), outPath, "png", dpi)) { mw_->statusBar()->showMessage("图片导出失败"); return; }
    mw_->statusBar()->showMessage("DFA 图片已导出: " + outPath);
}

void AutomataController::previewDfa()
{
    auto parsed = mw_->getParsed(); if (!parsed) return; int idx = cmbTokDfa_ ? cmbTokDfa_->currentIndex() : -1; if (idx <= 0 || idx - 1 >= parsed->tokens.size()) { mw_->statusBar()->showMessage("请选择具体Token后预览DFA"); return; }
    auto pt = parsed->tokens[idx - 1]; auto nfa = mw_->getEngine()->buildNFA(pt.ast, parsed->alpha); auto dfa = mw_->getEngine()->buildDFA(nfa); auto dpiEdit = root_->findChild<QLineEdit*>("edtGraphDpiDfa"); int dpi = (dpiEdit && !dpiEdit->text().trimmed().isEmpty()) ? dpiEdit->text().trimmed().toInt() : 150;
    QString pngPath; if (!dot_->renderToTempPng(DotExporter::toDot(dfa), pngPath, dpi)) { mw_->statusBar()->showMessage("Graphviz渲染失败，请确认已安装dot"); return; }
    dot_->previewPng(pngPath, "DFA 预览"); QFile::remove(pngPath);
}

void AutomataController::exportMinDot()
{
    auto parsed = mw_->getParsed(); if (!parsed) return; int idx = cmbTokMin_ ? cmbTokMin_->currentIndex() : -1; if (idx <= 0 || idx - 1 >= parsed->tokens.size()) return;
    auto pt = parsed->tokens[idx - 1]; auto nfa = mw_->getEngine()->buildNFA(pt.ast, parsed->alpha); auto dfa = mw_->getEngine()->buildDFA(nfa); auto mdfa = mw_->getEngine()->buildMinDFA(dfa);
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"); QString suggest = "mindfa_" + pt.rule.name + "_" + ts + ".dot";
    QString outPath = dot_->pickDotSavePath(suggest); if (outPath.isEmpty()) return;
    if (!DotExporter::exportToDot(mdfa, outPath)) { mw_->statusBar()->showMessage("DOT文件写入失败"); return; }
    mw_->statusBar()->showMessage("MinDFA DOT已导出: " + outPath);
}

void AutomataController::exportMinImage()
{
    auto parsed = mw_->getParsed(); if (!parsed) return; int idx = cmbTokMin_ ? cmbTokMin_->currentIndex() : -1; if (idx <= 0 || idx - 1 >= parsed->tokens.size()) return;
    auto pt = parsed->tokens[idx - 1]; auto nfa = mw_->getEngine()->buildNFA(pt.ast, parsed->alpha); auto dfa = mw_->getEngine()->buildDFA(nfa); auto mdfa = mw_->getEngine()->buildMinDFA(dfa);
    auto dpiEdit = root_->findChild<QLineEdit*>("edtGraphDpiMin"); int dpi = (dpiEdit && !dpiEdit->text().trimmed().isEmpty()) ? dpiEdit->text().trimmed().toInt() : 150;
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"); QString suggest = "mindfa_" + pt.rule.name + "_" + ts + ".png";
    QString outPath = dot_->pickImageSavePath(suggest, "png"); if (outPath.isEmpty()) return;
    if (!dot_->renderToFile(DotExporter::toDot(mdfa), outPath, "png", dpi)) { mw_->statusBar()->showMessage("图片导出失败"); return; }
    mw_->statusBar()->showMessage("MinDFA 图片已导出: " + outPath);
}

void AutomataController::previewMin()
{
    auto parsed = mw_->getParsed(); if (!parsed) return; int idx = cmbTokMin_ ? cmbTokMin_->currentIndex() : -1; if (idx <= 0 || idx - 1 >= parsed->tokens.size()) { mw_->statusBar()->showMessage("请选择具体Token后预览MinDFA"); return; }
    auto pt = parsed->tokens[idx - 1]; auto nfa = mw_->getEngine()->buildNFA(pt.ast, parsed->alpha); auto dfa = mw_->getEngine()->buildDFA(nfa); auto mdfa = mw_->getEngine()->buildMinDFA(dfa); auto dpiEdit = root_->findChild<QLineEdit*>("edtGraphDpiMin"); int dpi = (dpiEdit && !dpiEdit->text().trimmed().isEmpty()) ? dpiEdit->text().trimmed().toInt() : 150;
    QString pngPath; if (!dot_->renderToTempPng(DotExporter::toDot(mdfa), pngPath, dpi)) { mw_->statusBar()->showMessage("Graphviz渲染失败，请确认已安装dot"); return; }
    dot_->previewPng(pngPath, "MinDFA 预览"); QFile::remove(pngPath);
}

