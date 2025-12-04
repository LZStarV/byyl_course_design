#include "LR1Controller.h"
#include <QFileDialog>
#include <QSpinBox>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include "../../../src/config/Config.h"
#include <QJsonDocument>
#include <QJsonObject>

LR1Controller::LR1Controller(MainWindow* mw, Engine* engine, NotificationService* notify) :
    mw_(mw), engine_(engine), notify_(notify)
{
    dotSvc_ = new DotService(mw_, notify_);
}

static QVector<QString> readLines(const QString& path)
{
    QVector<QString> v;
    QFile            f(path);
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

void LR1Controller::fillProcessTable(QTableWidget*             tbl,
                                     const QVector<QString>&   cols,
                                     const QVector<ParseStep>& steps)
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
        for (const auto& p : ps.stack) stk += QString("(%1,%2) ").arg(p.first).arg(p.second);
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
    if (!page_)
        return;
    if (auto b = page_->findChild<QPushButton*>("btnLoadDefaultLR1"))
        connect(b, &QPushButton::clicked, this, &LR1Controller::loadDefault);
    if (auto b = page_->findChild<QPushButton*>("btnPickSourceLR1"))
        connect(b, &QPushButton::clicked, this, &LR1Controller::pickSource);
    if (auto b = page_->findChild<QPushButton*>("btnRunLR1Process"))
        connect(b, &QPushButton::clicked, this, &LR1Controller::runLR1Process);
    if (auto b = page_->findChild<QPushButton*>("btnPreviewLR1Tree"))
        connect(b, &QPushButton::clicked, this, &LR1Controller::previewTree);
    setupExportButton();
}

void LR1Controller::loadDefault()
{
    QString dir     = Config::generatedOutputDir() + "/syntax";
    QString srcPath = dir + "/last_source.txt";
    QString tokPath = dir + "/last_tokens.txt";
    QFile   fs(srcPath), ft(tokPath);
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
    // 同步文法到只读视图
    if (auto txtGrammar = page_->findChild<QTextEdit*>("txtInputGrammar"))
    {
        if (auto gv = page_->findChild<QPlainTextEdit*>("txtGrammarViewLR1"))
            gv->setPlainText(txtGrammar->toPlainText());
    }
}

void LR1Controller::pickSource()
{
    auto path = QFileDialog::getOpenFileName(mw_, QStringLiteral("选择源程序文件"));
    if (path.isEmpty())
        return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        notify_->error("文件打开失败");
        return;
    }
    QTextStream in(&f);
    QString     content = in.readAll();
    f.close();
    if (auto edt = page_->findChild<QPlainTextEdit*>("txtSourceViewLR1"))
        edt->setPlainText(content);
    QString tokPath = Config::generatedOutputDir() + "/syntax/last_tokens.txt";
    QFile   ft(tokPath);
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
    if (!mw_)
        return;
    auto txtGrammar = page_->findChild<QTextEdit*>("txtInputGrammar");
    if (!txtGrammar)
    {
        notify_->warning("未找到文法输入");
        return;
    }
    QString err;
    auto    g = engine_->parseGrammarText(txtGrammar->toPlainText(), err);
    if (!err.isEmpty() || g.productions.isEmpty())
    {
        notify_->error("文法错误:" + err);
        return;
    }
    auto    gr        = LR1Builder::build(g);
    auto    tbl       = LR1Builder::computeActionTable(g, gr);
    auto    tokView   = page_->findChild<QPlainTextEdit*>("txtTokensViewLR1");
    QString tokensStr = tokView ? tokView->toPlainText().trimmed() : QString();
    if (tokensStr.isEmpty())
    {
        notify_->warning("请先提供Token序列");
        return;
    }
    auto                   tokens = splitTokens(tokensStr);
    QMap<QString, QString> tokMap;
    // 优先：转换阶段生成的映射
    QString genMap = Config::generatedOutputDir() + "/syntax/token_map.json";
    if (QFile::exists(genMap))
    {
        QFile f(genMap);
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
    else
    {
        // 次级：项目配置映射
        QString appDir = QCoreApplication::applicationDirPath();
        QString cfg1   = appDir + "/../../config/syntax_token_map.json";
        QString cfg2   = appDir + "/config/syntax_token_map.json";
        QString use;
        if (QFile::exists(cfg1))
            use = cfg1;
        else if (QFile::exists(cfg2))
            use = cfg2;
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
    int unknown = 0;
    for (auto& x : tokens)
    {
        if (tokMap.contains(x))
            x = tokMap.value(x);
        else if (x != "$")
            unknown++;
    }
    if (unknown > 0)
        notify_->warning(QString("存在未映射的Token编码数量: %1").arg(unknown));
    // 若文法不包含分隔符；则按需过滤分隔符（例如Tiny语法的分号）
    if (!g.terminals.contains(";") && tokens.contains(";"))
    {
        QVector<QString> filtered;
        for (auto& x : tokens)
            if (x != ";")
                filtered.push_back(x);
        tokens.swap(filtered);
        notify_->warning(QStringLiteral("已过滤未在文法中定义的分隔符 ';'"));
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
        QString detail;
        if (!r.steps.isEmpty())
        {
            const auto& ps   = r.steps.back();
            QString     next = ps.rest.isEmpty() ? QStringLiteral("$") : ps.rest[0];
            int         st   = ps.stack.isEmpty() ? -1 : ps.stack.back().first;
            detail = QString("(state=%1, next=%2, action=%3)").arg(st).arg(next).arg(ps.action);
        }
        notify_->error("语法分析失败 " + detail);
        return;
    }
    lastDot_ = parseTreeToDot(r.root);
    notify_->info(QString("LR(1)分析完成，共 %1 步").arg(r.steps.size()));
}

void LR1Controller::previewTree()
{
    if (lastDot_.trimmed().isEmpty())
    {
        notify_->warning("请先运行LR(1)分析生成语法树");
        return;
    }
    QString png;
    int     dpi = 150;
    if (auto le = page_->findChild<QLineEdit*>("edtGraphDpiLR1"))
    {
        bool ok = false;
        int  v  = le->text().trimmed().toInt(&ok);
        if (ok && v >= 72 && v <= 600)
            dpi = v;
    }
    if (dotSvc_ && dotSvc_->renderToTempPng(lastDot_, png, dpi))
    {
        dotSvc_->previewPng(png, "LR(1) 语法树 预览");
        QFile::remove(png);
    }
}

void LR1Controller::setupExportButton()
{
    auto btn = page_->findChild<ExportGraphButton*>("exportBtnLR1Tree");
    if (!btn)
        return;
    btn->setDotService(dotSvc_);
    btn->setSuggestedBasename("lr1_");
    btn->setDpiProvider(
        [this]
        {
            if (auto le = page_->findChild<QLineEdit*>("edtGraphDpiLR1"))
            {
                bool ok = false;
                int  v  = le->text().trimmed().toInt(&ok);
                if (ok && v >= 72 && v <= 600)
                    return v;
            }
            return 150;
        });
    btn->setDotSupplier([this] { return this->lastDot_; });
}
