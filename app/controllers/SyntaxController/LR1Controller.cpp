#include "LR1Controller.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include "../../../src/config/Config.h"
#include <QJsonDocument>
#include <QJsonObject>

LR1Controller::LR1Controller(MainWindow* mw, Engine* engine, NotificationService* notify) : mw_(mw), engine_(engine), notify_(notify)
{
    dotSvc_ = new DotService(mw_, notify_);
}

static QVector<QString> readLines(const QString& path)
{
    QVector<QString> v;
    QFile f(path);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&f);
        while (!in.atEnd()) v.push_back(in.readLine());
        f.close();
    }
    return v;
}

QVector<QString> LR1Controller::splitTokens(const QString& s)
{
    QVector<QString> v;
    for (auto x : s.split(' ', Qt::SkipEmptyParts)) v.push_back(x.trimmed());
    return v;
}

void LR1Controller::fillProcessTable(QTableWidget* tbl, const QVector<QString>& cols, const QVector<ParseStep>& steps)
{
    tbl->clear();
    tbl->setColumnCount(cols.size());
    tbl->setRowCount(steps.size());
    QStringList headers;
    for (auto c : cols) headers << c;
    tbl->setHorizontalHeaderLabels(headers);
    for (int r = 0; r < steps.size(); ++r)
    {
        const auto& ps = steps[r];
        tbl->setItem(r, 0, new QTableWidgetItem(QString::number(ps.step)));
        QString stk;
        for (const auto& p : ps.stack)
            stk += QString("(%1,%2) ").arg(p.first).arg(p.second);
        tbl->setItem(r, 1, new QTableWidgetItem(stk.trimmed()));
        tbl->setItem(r, 2, new QTableWidgetItem(ps.rest.join(" ")));
        tbl->setItem(r, 3, new QTableWidgetItem(ps.action));
        tbl->setItem(r, 4, new QTableWidgetItem(ps.production));
    }
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void LR1Controller::bind(QWidget* exp2Page)
{
    page_ = exp2Page;
    if (!page_) return;
    if (auto b = page_->findChild<QPushButton*>("btnLoadDefaultLR1"))
        connect(b, &QPushButton::clicked, this, &LR1Controller::loadDefault);
    if (auto b = page_->findChild<QPushButton*>("btnPickSourceLR1"))
        connect(b, &QPushButton::clicked, this, &LR1Controller::pickSource);
    if (auto b = page_->findChild<QPushButton*>("btnRunLR1Process"))
        connect(b, &QPushButton::clicked, this, &LR1Controller::runLR1Process);
}

void LR1Controller::loadDefault()
{
    QString dir = Config::generatedOutputDir() + "/syntax";
    QString srcPath = dir + "/last_source.txt";
    QString tokPath = dir + "/last_tokens.txt";
    QFile fs(srcPath), ft(tokPath);
    if (fs.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&fs);
        if (auto edt = page_->findChild<QPlainTextEdit*>("txtSourceViewLR1"))
            edt->setPlainText(in.readAll());
        fs.close();
        lastSourcePath_ = srcPath;
    }
    if (ft.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&ft);
        if (auto edt = page_->findChild<QPlainTextEdit*>("txtTokensViewLR1"))
            edt->setPlainText(in.readAll());
        ft.close();
    }
}

void LR1Controller::pickSource()
{
    auto path = QFileDialog::getOpenFileName(mw_, QStringLiteral("选择源程序文件"));
    if (path.isEmpty()) return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        notify_->error("文件打开失败");
        return;
    }
    QTextStream in(&f);
    QString content = in.readAll();
    f.close();
    if (auto edt = page_->findChild<QPlainTextEdit*>("txtSourceViewLR1"))
        edt->setPlainText(content);
    QString tokPath = Config::generatedOutputDir() + "/syntax/last_tokens.txt";
    QFile ft(tokPath);
    if (ft.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&ft);
        if (auto tokv = page_->findChild<QPlainTextEdit*>("txtTokensViewLR1"))
            tokv->setPlainText(in.readAll());
        ft.close();
    }
}

void LR1Controller::runLR1Process()
{
    if (!mw_) return;
    auto txtGrammar = page_->findChild<QTextEdit*>("txtInputGrammar");
    if (!txtGrammar)
    {
        notify_->warning("未找到文法输入");
        return;
    }
    QString err;
    auto g = engine_->parseGrammarText(txtGrammar->toPlainText(), err);
    if (!err.isEmpty() || g.productions.isEmpty())
    {
        notify_->error("文法错误:" + err);
        return;
    }
    auto gr  = LR1Builder::build(g);
    auto tbl = LR1Builder::computeActionTable(g, gr);
    auto tokView = page_->findChild<QPlainTextEdit*>("txtTokensViewLR1");
    QString tokensStr = tokView ? tokView->toPlainText().trimmed() : QString();
    if (tokensStr.isEmpty())
    {
        notify_->warning("请先提供Token序列");
        return;
    }
    auto tokens = splitTokens(tokensStr);
    QMap<QString, QString> tokMap;
    {
        QString appDir = QCoreApplication::applicationDirPath();
        QString cfg1 = appDir + "/../../config/syntax_token_map.json";
        QString cfg2 = appDir + "/config/syntax_token_map.json";
        QString use;
        if (QFile::exists(cfg1)) use = cfg1; else if (QFile::exists(cfg2)) use = cfg2;
        if (!use.isEmpty())
        {
            QFile f(use);
            if (f.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                auto doc = QJsonDocument::fromJson(f.readAll());
                f.close();
                if (doc.isObject())
                {
                    auto obj = doc.object();
                    for (auto k : obj.keys()) tokMap[k] = obj.value(k).toString();
                }
            }
        }
    }
    if (!tokMap.isEmpty())
    {
        for (auto& x : tokens)
            if (tokMap.contains(x)) x = tokMap.value(x);
    }
    auto r = LR1Parser::parse(tokens, g, tbl);
    if (auto tblw = page_->findChild<QTableWidget*>("tblLR1Process"))
    {
        QVector<QString> cols;
        cols << "步" << "栈" << "输入" << "动作" << "产生式";
        fillProcessTable(tblw, cols, r.steps);
    }
    if (r.errorPos >= 0)
    {
        notify_->error("语法分析失败");
        return;
    }
    QString dot = parseTreeToDot(r.root);
    QString png;
    int dpi = 150;
    if (dotSvc_ && dotSvc_->renderToTempPng(dot, png, dpi))
    {
        dotSvc_->previewPng(png, "LR(1) 语法树 预览");
        QFile::remove(png);
    }
}
