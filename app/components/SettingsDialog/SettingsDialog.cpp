#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QStackedWidget>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include "../../../src/config/Config.h"
#include "../ToastManager/ToastManager.h"

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("设置");
    buildUi();
    loadCurrent();
}

void SettingsDialog::buildUi()
{
    setFixedWidth(800);
    auto root = new QVBoxLayout(this);
    auto main = new QHBoxLayout;
    navList   = new QListWidget(this);
    stacked   = new QStackedWidget(this);
    navList->setFixedWidth(180);
    main->addWidget(navList);
    main->addWidget(stacked, 1);
    root->addLayout(main);

    // Paths page
    pagePaths = new QWidget(this);
    {
        auto v = new QVBoxLayout(pagePaths);
        auto lOut = new QHBoxLayout;
        lOut->addWidget(new QLabel("生成输出目录"));
        edtOutDir       = new QLineEdit;
        btnBrowseOutDir = new QPushButton("浏览...");
        lOut->addWidget(edtOutDir);
        lOut->addWidget(btnBrowseOutDir);
        v->addLayout(lOut);
        v->addWidget(new QLabel("语法输出目录"));
        edtSyntaxOutDir = new QLineEdit; v->addWidget(edtSyntaxOutDir);
        v->addWidget(new QLabel("图导出目录"));
        edtGraphsDir = new QLineEdit; v->addWidget(edtGraphsDir);
        auto lCfg = new QHBoxLayout; lCfg->addWidget(new QLabel("配置搜索路径（分号分隔）")); edtCfgSearchPaths = new QLineEdit; lCfg->addWidget(edtCfgSearchPaths); v->addLayout(lCfg);
        v->addStretch(1);
    }
    stacked->addWidget(pagePaths);
    navList->addItem("目录与输出");

    // Weights & Skip page
    pageWeightsSkip = new QWidget(this);
    {
        auto v = new QVBoxLayout(pageWeightsSkip);
        v->addWidget(new QLabel("权重层级（min_code / weight）"));
        tblTiers = new QTableWidget; tblTiers->setColumnCount(2);
        QStringList headers; headers << "min_code" << "weight";
        tblTiers->setHorizontalHeaderLabels(headers);
        tblTiers->horizontalHeader()->setStretchLastSection(true);
        tblTiers->verticalHeader()->setVisible(false);
        tblTiers->setSelectionBehavior(QAbstractItemView::SelectRows);
        v->addWidget(tblTiers);
        auto lTierBtns = new QHBoxLayout;
        btnAddRow      = new QPushButton("新增行");
        btnDelRow      = new QPushButton("删除选中行");
        lTierBtns->addWidget(btnAddRow);
        lTierBtns->addWidget(btnDelRow);
        v->addLayout(lTierBtns);
        v->addWidget(new QLabel("跳过注释与字符串"));
        chkSkipBrace    = new QCheckBox("跳过花括号注释");
        chkSkipLine     = new QCheckBox("跳过行注释");
        chkSkipBlock    = new QCheckBox("跳过块注释");
        chkSkipHash     = new QCheckBox("跳过井号注释");
        chkSkipSingle   = new QCheckBox("跳过单引号字符串");
        chkSkipDouble   = new QCheckBox("跳过双引号字符串");
        chkSkipTemplate = new QCheckBox("跳过模板字符串");
        v->addWidget(chkSkipBrace);
        v->addWidget(chkSkipLine);
        v->addWidget(chkSkipBlock);
        v->addWidget(chkSkipHash);
        v->addWidget(chkSkipSingle);
        v->addWidget(chkSkipDouble);
        v->addWidget(chkSkipTemplate);
        v->addStretch(1);
    }
    stacked->addWidget(pageWeightsSkip);
    navList->addItem("权重与跳过");

    // Lexer & Identifier page
    pageLexerId = new QWidget(this);
    {
        auto v = new QVBoxLayout(pageLexerId);
        v->addWidget(new QLabel("宏名称（letter/digit）"));
        auto lMacro = new QHBoxLayout; edtMacroLetter = new QLineEdit; edtMacroDigit = new QLineEdit;
        lMacro->addWidget(new QLabel("letter")); lMacro->addWidget(edtMacroLetter);
        lMacro->addWidget(new QLabel("digit")); lMacro->addWidget(edtMacroDigit);
        v->addLayout(lMacro);
        chkTokenMapHeur = new QCheckBox("启用 token_map 启发式"); v->addWidget(chkTokenMapHeur);
        auto lWs = new QHBoxLayout; lWs->addWidget(new QLabel("空白字符（逗号分隔，支持 \\t/\\n/\\r）"));
        edtWhitespaces = new QLineEdit; lWs->addWidget(edtWhitespaces); v->addLayout(lWs);
        v->addWidget(new QLabel("标识符设置"));
        chkEmitIdLexeme = new QCheckBox("在 identifier 后追加词素"); v->addWidget(chkEmitIdLexeme);
        auto lId = new QHBoxLayout; lId->addWidget(new QLabel("标识符规则名（逗号分隔）")); edtIdentifierNames = new QLineEdit; lId->addWidget(edtIdentifierNames); v->addLayout(lId);
        v->addStretch(1);
    }
    stacked->addWidget(pageLexerId);
    navList->addItem("词法与标识符");

    // Grammar page
    pageGrammar = new QWidget(this);
    {
        auto v = new QVBoxLayout(pageGrammar);
        auto lSyn = new QHBoxLayout; lSyn->addWidget(new QLabel("epsilon/eof/aug"));
        edtEpsilon = new QLineEdit; edtEof = new QLineEdit; edtAug = new QLineEdit;
        lSyn->addWidget(edtEpsilon); lSyn->addWidget(edtEof); lSyn->addWidget(edtAug);
        v->addLayout(lSyn);
        auto lNon = new QHBoxLayout; lNon->addWidget(new QLabel("非终结符正则（可空）")); edtNontermPat = new QLineEdit; lNon->addWidget(edtNontermPat); v->addLayout(lNon);
        auto lOps = new QHBoxLayout; lOps->addWidget(new QLabel("多/单字符操作符（逗号分隔）")); edtMultiOps = new QLineEdit; edtSingleOps = new QLineEdit; lOps->addWidget(edtMultiOps); lOps->addWidget(edtSingleOps); v->addLayout(lOps);
        auto lLr = new QHBoxLayout; lLr->addWidget(new QLabel("LR(1) 冲突策略")); edtLr1Policy = new QLineEdit; lLr->addWidget(edtLr1Policy); v->addLayout(lLr);
        v->addStretch(1);
    }
    stacked->addWidget(pageGrammar);
    navList->addItem("语法与算法");

    // I18n & Dot page
    pageI18nDot = new QWidget(this);
    {
        auto v = new QVBoxLayout(pageI18nDot);
        v->addWidget(new QLabel("表头（标记/状态ID/状态集合/ε列名）"));
        auto lTbl = new QHBoxLayout; edtTblMark = new QLineEdit; edtTblStateId = new QLineEdit; edtTblStateSet = new QLineEdit; edtTblEpsCol = new QLineEdit; lTbl->addWidget(edtTblMark); lTbl->addWidget(edtTblStateId); lTbl->addWidget(edtTblStateSet); lTbl->addWidget(edtTblEpsCol); v->addLayout(lTbl);
        v->addWidget(new QLabel("DOT 样式（rankdir/node_shape/epsilon_label）"));
        auto lDot = new QHBoxLayout; edtDotRankdir = new QLineEdit; edtDotNodeShape = new QLineEdit; edtDotEpsLabel = new QLineEdit; lDot->addWidget(edtDotRankdir); lDot->addWidget(edtDotNodeShape); lDot->addWidget(edtDotEpsLabel); v->addLayout(lDot);
        v->addStretch(1);
    }
    stacked->addWidget(pageI18nDot);
    navList->addItem("表头与DOT");

    // Graphviz page
    pageGraphviz = new QWidget(this);
    {
        auto v = new QVBoxLayout(pageGraphviz);
        v->addWidget(new QLabel("Graphviz（executable/dpi/timeout_ms）"));
        auto lGv = new QHBoxLayout; edtGraphvizExe = new QLineEdit; edtGraphvizDpi = new QLineEdit; edtGraphvizTimeout = new QLineEdit; lGv->addWidget(edtGraphvizExe); lGv->addWidget(edtGraphvizDpi); lGv->addWidget(edtGraphvizTimeout); v->addLayout(lGv);
        v->addStretch(1);
    }
    stacked->addWidget(pageGraphviz);
    navList->addItem("Graphviz");

    // Semantic page
    pageSemantic = new QWidget(this);
    {
        auto v = new QVBoxLayout(pageSemantic);
        auto lSem = new QHBoxLayout; lSem->addWidget(new QLabel("语义策略（root/child）")); edtSemRootPolicy = new QLineEdit; edtSemChildOrder = new QLineEdit; lSem->addWidget(edtSemRootPolicy); lSem->addWidget(edtSemChildOrder); v->addLayout(lSem);
        v->addStretch(1);
    }
    stacked->addWidget(pageSemantic);
    navList->addItem("语义策略");

    // bottom bar
    auto lBtns  = new QHBoxLayout;
    btnDefaults = new QPushButton("恢复默认");
    btnSave     = new QPushButton("保存");
    btnCancel   = new QPushButton("取消");
    lBtns->addStretch(1);
    lBtns->addWidget(btnDefaults);
    lBtns->addWidget(btnSave);
    lBtns->addWidget(btnCancel);
    root->addLayout(lBtns);
    connect(navList, &QListWidget::currentRowChanged, stacked, &QStackedWidget::setCurrentIndex);
    navList->setCurrentRow(0);
    connect(btnAddRow,
            &QPushButton::clicked,
            [this]()
            {
                int r = tblTiers->rowCount();
                tblTiers->insertRow(r);
            });
    connect(btnDelRow,
            &QPushButton::clicked,
            [this]()
            {
                auto rows = tblTiers->selectionModel()->selectedRows();
                for (int i = rows.size() - 1; i >= 0; --i) tblTiers->removeRow(rows[i].row());
            });
    connect(btnDefaults,
            &QPushButton::clicked,
            [this]()
            {
                auto v = QVector<Config::WeightTier>();
                v.push_back({220, 3});
                v.push_back({200, 4});
                v.push_back({100, 1});
                v.push_back({0, 0});
                edtOutDir->setText(QCoreApplication::applicationDirPath() + "/../../generated/lex");
                tblTiers->setRowCount(0);
                for (int i = 0; i < v.size(); ++i)
                {
                    tblTiers->insertRow(i);
                    tblTiers->setItem(i, 0, new QTableWidgetItem(QString::number(v[i].minCode)));
                    tblTiers->setItem(i, 1, new QTableWidgetItem(QString::number(v[i].weight)));
                }
                chkSkipBrace->setChecked(false);
                chkSkipLine->setChecked(false);
                chkSkipBlock->setChecked(false);
                chkSkipHash->setChecked(false);
                chkSkipSingle->setChecked(false);
                chkSkipDouble->setChecked(false);
                chkSkipTemplate->setChecked(false);
                chkEmitIdLexeme->setChecked(true);
                edtIdentifierNames->setText("identifier");
            });
    connect(btnSave,
            &QPushButton::clicked,
            [this]()
            {
                if (!collectAndApply())
                    return;
                auto path = decideSavePath();
                if (path.isEmpty())
                {
                    QMessageBox::warning(
                        this, QStringLiteral("保存失败"), QStringLiteral("无法确定保存路径"));
                    return;
                }
                if (!Config::saveJson(path))
                {
                    QMessageBox::warning(
                        this, QStringLiteral("保存失败"), QStringLiteral("文件写入失败"));
                    return;
                }
                ToastManager::instance().showInfo("设置已保存");
                accept();
            });
    connect(btnCancel, &QPushButton::clicked, [this]() { reject(); });
    connect(btnBrowseOutDir,
            &QPushButton::clicked,
            [this]()
            {
                QString dir =
                    QFileDialog::getExistingDirectory(this, "选择输出目录", edtOutDir->text());
                if (!dir.isEmpty())
                {
                    edtOutDir->setText(dir);
                }
            });
}

void SettingsDialog::loadCurrent()
{
    edtOutDir->setText(Config::generatedOutputDir());
    edtSyntaxOutDir->setText(Config::syntaxOutputDir());
    edtGraphsDir->setText(Config::graphsDir());
    edtMacroLetter->setText(Config::macroLetterName());
    edtMacroDigit->setText(Config::macroDigitName());
    chkTokenMapHeur->setChecked(Config::tokenMapUseHeuristics());
    {
        QString s; auto ws = Config::whitespaces();
        for (int i=0;i<ws.size();++i){ QChar c=ws[i]; if (c=='\t') s+="\\t"; else if (c=='\n') s+="\\n"; else if (c=='\r') s+="\\r"; else s+=c; if (i+1<ws.size()) s+=","; }
        edtWhitespaces->setText(s);
    }
    edtEpsilon->setText(Config::epsilonSymbol());
    edtEof->setText(Config::eofSymbol());
    edtAug->setText(Config::augSuffix());
    edtNontermPat->setText(Config::nonterminalPattern());
    if (edtLr1Policy) edtLr1Policy->setText(Config::lr1ConflictPolicy());
    {
        QString s1,s2; auto mo=Config::grammarMultiOps(); auto so=Config::grammarSingleOps();
        for (int i=0;i<mo.size();++i){ s1+=mo[i]; if (i+1<mo.size()) s1+=","; }
        for (int i=0;i<so.size();++i){ s2+=so[i]; if (i+1<so.size()) s2+=","; }
        edtMultiOps->setText(s1); edtSingleOps->setText(s2);
    }
    edtTblMark->setText(Config::tableMarkLabel());
    edtTblStateId->setText(Config::tableStateIdLabel());
    edtTblStateSet->setText(Config::tableStateSetLabel());
    edtTblEpsCol->setText(Config::epsilonColumnLabel());
    edtDotRankdir->setText(Config::dotRankdir());
    edtDotNodeShape->setText(Config::dotNodeShape());
    edtDotEpsLabel->setText(Config::dotEpsilonLabel());
    edtGraphvizExe->setText(Config::graphvizExecutable());
    edtGraphvizDpi->setText(QString::number(Config::graphvizDefaultDpi()));
    edtGraphvizTimeout->setText(QString::number(Config::graphvizTimeoutMs()));
    {
        QString s; auto arr=Config::configSearchPaths();
        for (int i=0;i<arr.size();++i){ s+=arr[i]; if (i+1<arr.size()) s+=';'; }
        edtCfgSearchPaths->setText(s);
    }
    edtSemRootPolicy->setText(Config::semanticRootSelectionPolicy());
    edtSemChildOrder->setText(Config::semanticChildOrderPolicy());
    tblTiers->setRowCount(0);
    QVector<int> probe;
    for (int i = 0; i <= 3; ++i) probe.push_back(i ? i * 100 : 0);
    QSet<int> emitted;
    for (int c : probe)
    {
        int w = Config::weightForCode(c);
        if (!emitted.contains(w))
            emitted.insert(w);
    }
    auto tiers = QVector<Config::WeightTier>();
    tiers.push_back({220, Config::weightForCode(220)});
    tiers.push_back({200, Config::weightForCode(200)});
    tiers.push_back({100, Config::weightForCode(100)});
    tiers.push_back({0, Config::weightForCode(0)});
    std::sort(tiers.begin(),
              tiers.end(),
              [](const Config::WeightTier& a, const Config::WeightTier& b)
              { return a.minCode > b.minCode; });
    tblTiers->setRowCount(tiers.size());
    for (int i = 0; i < tiers.size(); ++i)
    {
        tblTiers->setItem(i, 0, new QTableWidgetItem(QString::number(tiers[i].minCode)));
        tblTiers->setItem(i, 1, new QTableWidgetItem(QString::number(tiers[i].weight)));
    }
    chkSkipBrace->setChecked(Config::skipBraceComment());
    chkSkipLine->setChecked(Config::skipLineComment());
    chkSkipBlock->setChecked(Config::skipBlockComment());
    chkSkipHash->setChecked(Config::skipHashComment());
    chkSkipSingle->setChecked(Config::skipSingleQuoteString());
    chkSkipDouble->setChecked(Config::skipDoubleQuoteString());
    chkSkipTemplate->setChecked(Config::skipTemplateString());
    chkEmitIdLexeme->setChecked(Config::emitIdentifierLexeme());
    {
        auto names = Config::identifierTokenNames();
        QString s;
        for (int i = 0; i < names.size(); ++i)
        {
            s += names[i];
            if (i + 1 < names.size()) s += ",";
        }
        edtIdentifierNames->setText(s);
    }
}

static bool parseInt(const QString& s, int& out)
{
    bool ok = false;
    out     = s.trimmed().toInt(&ok);
    return ok;
}

bool SettingsDialog::collectAndApply()
{
    QString dir = edtOutDir->text().trimmed();
    if (dir.isEmpty())
        return false;
    QDir d(dir);
    if (!d.exists())
    {
        if (!QDir().mkpath(dir))
            return false;
    }
    QVector<Config::WeightTier> tiers;
    for (int r = 0; r < tblTiers->rowCount(); ++r)
    {
        auto mi = tblTiers->item(r, 0);
        auto wi = tblTiers->item(r, 1);
        if (!mi || !wi)
            continue;
        int m = 0, w = 0;
        if (!parseInt(mi->text(), m))
            return false;
        if (!parseInt(wi->text(), w))
            return false;
        tiers.push_back({m, w});
    }
    if (tiers.isEmpty())
        return false;
    std::sort(tiers.begin(),
              tiers.end(),
              [](const Config::WeightTier& a, const Config::WeightTier& b)
              { return a.minCode > b.minCode; });
    Config::setGeneratedOutputDir(dir);
    Config::setSyntaxOutputDir(edtSyntaxOutDir->text().trimmed());
    Config::setGraphsDir(edtGraphsDir->text().trimmed());
    Config::setMacroLetterName(edtMacroLetter->text().trimmed());
    Config::setMacroDigitName(edtMacroDigit->text().trimmed());
    Config::setTokenMapUseHeuristics(chkTokenMapHeur->isChecked());
    {
        QVector<QChar> ws;
        for (auto x : edtWhitespaces->text().split(',', Qt::SkipEmptyParts))
        {
            QString s=x.trimmed(); if (s=="\\t") ws.push_back('\t'); else if (s=="\\n") ws.push_back('\n'); else if (s=="\\r") ws.push_back('\r'); else if (!s.isEmpty()) ws.push_back(s[0]);
        }
        if (!ws.isEmpty()) Config::setWhitespaces(ws);
    }
    Config::setEpsilonSymbol(edtEpsilon->text().trimmed());
    Config::setEofSymbol(edtEof->text().trimmed());
    Config::setAugSuffix(edtAug->text().trimmed());
    Config::setNonterminalPattern(edtNontermPat->text().trimmed());
    if (edtLr1Policy) Config::setLr1ConflictPolicy(edtLr1Policy->text().trimmed());
    {
        QVector<QString> mo,so;
        for (auto x : edtMultiOps->text().split(',', Qt::SkipEmptyParts)) mo.push_back(x.trimmed());
        for (auto x : edtSingleOps->text().split(',', Qt::SkipEmptyParts)) so.push_back(x.trimmed());
        if (!mo.isEmpty()) Config::setGrammarMultiOps(mo);
        if (!so.isEmpty()) Config::setGrammarSingleOps(so);
    }
    Config::setTableMarkLabel(edtTblMark->text().trimmed());
    Config::setTableStateIdLabel(edtTblStateId->text().trimmed());
    Config::setTableStateSetLabel(edtTblStateSet->text().trimmed());
    Config::setEpsilonColumnLabel(edtTblEpsCol->text().trimmed());
    Config::setDotRankdir(edtDotRankdir->text().trimmed());
    Config::setDotNodeShape(edtDotNodeShape->text().trimmed());
    Config::setDotEpsilonLabel(edtDotEpsLabel->text().trimmed());
    Config::setGraphvizExecutable(edtGraphvizExe->text().trimmed());
    Config::setGraphvizDefaultDpi(edtGraphvizDpi->text().trimmed().toInt());
    Config::setGraphvizTimeoutMs(edtGraphvizTimeout->text().trimmed().toInt());
    {
        QVector<QString> paths;
        for (auto x : edtCfgSearchPaths->text().split(';', Qt::SkipEmptyParts)) paths.push_back(x.trimmed());
        Config::setConfigSearchPaths(paths);
    }
    Config::setSemanticRootSelectionPolicy(edtSemRootPolicy->text().trimmed());
    Config::setSemanticChildOrderPolicy(edtSemChildOrder->text().trimmed());
    Config::setWeightTiers(tiers);
    Config::setSkipBrace(chkSkipBrace->isChecked());
    Config::setSkipLine(chkSkipLine->isChecked());
    Config::setSkipBlock(chkSkipBlock->isChecked());
    Config::setSkipHash(chkSkipHash->isChecked());
    Config::setSkipSingle(chkSkipSingle->isChecked());
    Config::setSkipDouble(chkSkipDouble->isChecked());
    Config::setSkipTemplate(chkSkipTemplate->isChecked());
    Config::setEmitIdentifierLexeme(chkEmitIdLexeme->isChecked());
    {
        QVector<QString> names;
        for (auto x : edtIdentifierNames->text().split(',', Qt::SkipEmptyParts))
            names.push_back(x.trimmed());
        Config::setIdentifierTokenNames(names);
    }
    return true;
}

QString SettingsDialog::decideSavePath() const
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString p1     = appDir + "/../../config/lexer.json";
    QString p2     = appDir + "/config/lexer.json";
    if (QFileInfo(p1).exists())
        return p1;
    if (QFileInfo(QFileInfo(p1).absolutePath()).isDir())
        return p1;
    if (QFileInfo(QFileInfo(p2).absolutePath()).isDir())
        return p2;
    return p1;
}
