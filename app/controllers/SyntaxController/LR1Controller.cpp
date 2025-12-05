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
#include <QTreeWidget>

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
    tbl->setColumnCount(2);
    tbl->setRowCount(steps.size());
    tbl->setHorizontalHeaderLabels(QStringList({QStringLiteral("步骤"), QStringLiteral("描述")}));
    for (int r = 0; r < steps.size(); ++r)
    {
        const auto& ps = steps[r];
        tbl->setItem(r, 0, new QTableWidgetItem(QString::number(ps.step)));
        QString desc;
        if (ps.action.startsWith("s"))
            desc = QString("shift 到 s%1，读入 '%2'")
                       .arg(ps.stack.isEmpty() ? -1 : ps.stack.back().first)
                       .arg(ps.rest.isEmpty() ? QStringLiteral("$") : ps.rest[0]);
        else if (ps.action.startsWith("r"))
            desc = QString("reduce %1").arg(ps.production);
        else if (ps.action == "acc")
            desc = QStringLiteral("acc，分析完成");
        else
            desc = QString("动作 %1").arg(ps.action);
        tbl->setItem(r, 1, new QTableWidgetItem(desc));
    }
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

static void addTreeNode(QTreeWidgetItem* parent, const SemanticASTNode* n)
{
    if (!n) return;
    auto item = new QTreeWidgetItem(QStringList(n->tag));
    if (parent)
        parent->addChild(item);
    else
        ;
    for (auto c : n->children) addTreeNode(item, c);
}

void LR1Controller::fillSemanticTree(QTreeWidget* tree, const SemanticASTNode* root)
{
    if (!tree) return;
    tree->clear();
    auto item = new QTreeWidgetItem(QStringList(root ? root->tag : QString("(empty)")));
    tree->addTopLevelItem(item);
    if (root)
    {
        for (auto c : root->children) addTreeNode(item, c);
    }
    tree->expandAll();
}

void LR1Controller::bind(QWidget* exp2Page)
{
    page_ = exp2Page;
    if (!page_)
        return;
    if (auto b = page_->findChild<QPushButton*>("btnLoadDefaultLR1"))
        connect(b, &QPushButton::clicked, this, &LR1Controller::loadDefault);
    if (auto cmb = page_->findChild<QComboBox*>("cmbPickSourceLR1"))
        connect(cmb,
                QOverload<int>::of(&QComboBox::activated),
                this,
                &LR1Controller::onPickSourceActivated);
    if (auto b = page_->findChild<QPushButton*>("btnRunLR1Process"))
        connect(b, &QPushButton::clicked, this, &LR1Controller::runLR1Process);
    if (auto b = page_->findChild<QPushButton*>("btnLoadSemanticActions"))
        connect(b, &QPushButton::clicked, this, &LR1Controller::loadSemanticActions);
    if (auto b = page_->findChild<QPushButton*>("btnPreviewLR1Tree"))
        connect(b, &QPushButton::clicked, this, &LR1Controller::previewTree);
    setupExportButton();
}

void LR1Controller::onPickSourceActivated(int index)
{
    // 索引从0开始：0=正则表达式，1=Token序列，2=当前文法
    QString filePath = QFileDialog::getOpenFileName(
        mw_, QStringLiteral("选择文件"), "", QStringLiteral("所有文件 (*)"));

    if (filePath.isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        notify_->error(QStringLiteral("无法打开文件: ") + filePath);
        return;
    }

    QTextStream in(&file);
    QString     content = in.readAll();
    file.close();

    if (auto txtSourceView = page_->findChild<QPlainTextEdit*>("txtSourceViewLR1"))
    {
        if (index == 0)
        {  // 正则表达式
            txtSourceView->setPlainText(content);
        }
        else if (index == 1)
        {  // Token序列
            if (auto txtTokensView = page_->findChild<QPlainTextEdit*>("txtTokensViewLR1"))
            {
                txtTokensView->setPlainText(content);
            }
        }
        else if (index == 2)
        {  // 当前文法
            if (auto txtGrammarView = page_->findChild<QPlainTextEdit*>("txtGrammarViewLR1"))
            {
                txtGrammarView->setPlainText(content);
            }
        }
    }
}

void LR1Controller::loadDefault()
{
    QString dir     = Config::syntaxOutputDir();
    QString srcPath = dir + "/last_regex.txt";
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
    // 语义动作不参与默认加载
}

void LR1Controller::pickSource()
{
    auto path = QFileDialog::getOpenFileName(mw_, QStringLiteral("选择正则表达式文件"));
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
    auto                   tokensCodes = splitTokens(tokensStr);
    auto                   tokens      = tokensCodes;
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
    // 语义动作策略（外部配置）
    auto roleMeaning = Config::semanticRoleMeaning();
    auto rootPolicy  = Config::semanticRootSelectionPolicy();
    auto childOrder  = Config::semanticChildOrderPolicy();
    auto r           = LR1Parser::parseWithSemantics(tokens, g, tbl, semanticActions_, roleMeaning, rootPolicy, childOrder);
    if (auto tblw = page_->findChild<QTableWidget*>("tblLR1Process"))
    {
        QVector<QString> cols;
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
    if (r.astRoot)
        lastDot_ = semanticAstToDot(r.astRoot);
    else
        lastDot_ = parseTreeToDotWithTokens(r.root, tokens);
    if (auto tree = page_->findChild<QTreeWidget*>("treeSemanticLR1"))
        fillSemanticTree(tree, r.astRoot);
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
    if (auto tree = page_->findChild<QTreeWidget*>("treeSemanticLR1"))
    {
        // 同步文本树视图
        // 使用最新的语义 AST：需在 runLR1Process 之后通过 parseWithSemantics 的结果传入
        // 这里从 DOT 无法恢复 AST，仅在 runLR1Process 内部填充
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
void LR1Controller::loadSemanticActions()
{
    auto path = QFileDialog::getOpenFileName(mw_,
                                             QStringLiteral("选择语义动作文件"),
                                             QString(),
                                             QStringLiteral("Text (*.txt *.sem);;All (*)"));
    if (path.isEmpty())
        return;
    auto lines = readLines(path);
    if (lines.size() % 2 != 0)
    {
        notify_->error(QStringLiteral("语义动作文件行数必须为偶数"));
        return;
    }
    QMap<QString, QVector<QVector<int>>> m;
    for (int i = 0; i < lines.size(); i += 2)
    {
        QString prod = lines[i].trimmed();
        QString acts = lines[i + 1].trimmed();
        if (prod.isEmpty())
            continue;
        // 产生式行：A -> α1 | α2 | ...
        int arrow = prod.indexOf("->");
        if (arrow < 0)
        {
            notify_->error(QStringLiteral("产生式格式错误: ") + prod);
            return;
        }
        QString L    = prod.left(arrow).trimmed();
        QString Rall = prod.mid(arrow + 2).trimmed();
        auto    rhss = Rall.split('|');
        auto    actc = acts.split('|');
        if (rhss.size() != actc.size())
        {
            notify_->error(QStringLiteral("候选数与动作数不匹配: ") + L);
            return;
        }
        QVector<QVector<int>> seqs;
        for (int k = 0; k < rhss.size(); ++k)
        {
            auto rhs    = rhss[k].trimmed();
            auto actstr = actc[k].trimmed();
            auto syms   = rhs == "#" ? QVector<QString>() : rhs.split(' ', Qt::SkipEmptyParts);
            auto bits   = actstr.split(' ', Qt::SkipEmptyParts);
            QVector<int> vs;
            for (auto b : bits) vs.push_back(b.toInt());
            if (vs.size() != syms.size())
            {
                notify_->error(QStringLiteral("动作位数与候选符号数不匹配: ") + L);
                return;
            }
            seqs.push_back(vs);
        }
        m[L] = seqs;
    }
    semanticActions_ = m;
    if (auto v = page_->findChild<QPlainTextEdit*>("txtSemanticViewLR1"))
    {
        QString text;
        for (int i = 0; i < lines.size(); ++i)
        {
            text += lines[i];
            text += '\n';
        }
        v->setPlainText(text);
    }
    notify_->info(QStringLiteral("语义动作导入成功"));
}
