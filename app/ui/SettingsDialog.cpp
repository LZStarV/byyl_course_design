#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include "../../src/config/Config.h"
#include "ToastManager.h"

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("设置");
    buildUi();
    loadCurrent();
}

void SettingsDialog::buildUi()
{
    auto v = new QVBoxLayout(this);
    auto lOut = new QHBoxLayout;
    lOut->addWidget(new QLabel("生成输出目录"));
    edtOutDir = new QLineEdit;
    lOut->addWidget(edtOutDir);
    v->addLayout(lOut);
    v->addWidget(new QLabel("权重层级（min_code / weight）"));
    tblTiers = new QTableWidget;
    tblTiers->setColumnCount(2);
    QStringList headers;
    headers << "min_code" << "weight";
    tblTiers->setHorizontalHeaderLabels(headers);
    tblTiers->horizontalHeader()->setStretchLastSection(true);
    tblTiers->verticalHeader()->setVisible(false);
    tblTiers->setSelectionBehavior(QAbstractItemView::SelectRows);
    v->addWidget(tblTiers);
    auto lTierBtns = new QHBoxLayout;
    btnAddRow = new QPushButton("新增行");
    btnDelRow = new QPushButton("删除选中行");
    lTierBtns->addWidget(btnAddRow);
    lTierBtns->addWidget(btnDelRow);
    v->addLayout(lTierBtns);
    v->addWidget(new QLabel("跳过注释与字符串"));
    chkSkipBrace = new QCheckBox("跳过花括号注释");
    chkSkipLine = new QCheckBox("跳过行注释");
    chkSkipBlock = new QCheckBox("跳过块注释");
    chkSkipHash = new QCheckBox("跳过井号注释");
    chkSkipSingle = new QCheckBox("跳过单引号字符串");
    chkSkipDouble = new QCheckBox("跳过双引号字符串");
    chkSkipTemplate = new QCheckBox("跳过模板字符串");
    v->addWidget(chkSkipBrace);
    v->addWidget(chkSkipLine);
    v->addWidget(chkSkipBlock);
    v->addWidget(chkSkipHash);
    v->addWidget(chkSkipSingle);
    v->addWidget(chkSkipDouble);
    v->addWidget(chkSkipTemplate);
    auto lBtns = new QHBoxLayout;
    btnDefaults = new QPushButton("恢复默认");
    btnSave = new QPushButton("保存");
    btnCancel = new QPushButton("取消");
    lBtns->addWidget(btnDefaults);
    lBtns->addStretch(1);
    lBtns->addWidget(btnSave);
    lBtns->addWidget(btnCancel);
    v->addLayout(lBtns);
    connect(btnAddRow, &QPushButton::clicked, [this]() {
        int r = tblTiers->rowCount();
        tblTiers->insertRow(r);
    });
    connect(btnDelRow, &QPushButton::clicked, [this]() {
        auto rows = tblTiers->selectionModel()->selectedRows();
        for (int i = rows.size() - 1; i >= 0; --i) tblTiers->removeRow(rows[i].row());
    });
    connect(btnDefaults, &QPushButton::clicked, [this]() {
        auto v = QVector<Config::WeightTier>();
        v.push_back({220, 3});
        v.push_back({200, 4});
        v.push_back({100, 1});
        v.push_back({0, 0});
        edtOutDir->setText(QCoreApplication::applicationDirPath() + "/../../generated/lex");
        tblTiers->setRowCount(0);
        for (int i = 0; i < v.size(); ++i) {
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
    });
    connect(btnSave, &QPushButton::clicked, [this]() {
        if (!collectAndApply()) return;
        auto path = decideSavePath();
        if (path.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("保存失败"), QStringLiteral("无法确定保存路径"));
            return;
        }
        if (!Config::saveJson(path)) {
            QMessageBox::warning(this, QStringLiteral("保存失败"), QStringLiteral("文件写入失败"));
            return;
        }
        ToastManager::instance().showInfo("设置已保存");
        accept();
    });
    connect(btnCancel, &QPushButton::clicked, [this]() { reject(); });
}

void SettingsDialog::loadCurrent()
{
    edtOutDir->setText(Config::generatedOutputDir());
    tblTiers->setRowCount(0);
    QVector<int> probe;
    for (int i = 0; i <= 3; ++i) probe.push_back(i ? i * 100 : 0);
    QSet<int> emitted;
    for (int c : probe) {
        int w = Config::weightForCode(c);
        if (!emitted.contains(w)) emitted.insert(w);
    }
    auto tiers = QVector<Config::WeightTier>();
    tiers.push_back({220, Config::weightForCode(220)});
    tiers.push_back({200, Config::weightForCode(200)});
    tiers.push_back({100, Config::weightForCode(100)});
    tiers.push_back({0, Config::weightForCode(0)});
    std::sort(tiers.begin(), tiers.end(), [](const Config::WeightTier& a, const Config::WeightTier& b){return a.minCode>b.minCode;});
    tblTiers->setRowCount(tiers.size());
    for (int i = 0; i < tiers.size(); ++i) {
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
}

static bool parseInt(const QString& s, int& out)
{
    bool ok = false;
    out = s.trimmed().toInt(&ok);
    return ok;
}

bool SettingsDialog::collectAndApply()
{
    QString dir = edtOutDir->text().trimmed();
    if (dir.isEmpty()) return false;
    QDir d(dir);
    if (!d.exists()) {
        if (!QDir().mkpath(dir)) return false;
    }
    QVector<Config::WeightTier> tiers;
    for (int r = 0; r < tblTiers->rowCount(); ++r) {
        auto mi = tblTiers->item(r, 0);
        auto wi = tblTiers->item(r, 1);
        if (!mi || !wi) continue;
        int m = 0, w = 0;
        if (!parseInt(mi->text(), m)) return false;
        if (!parseInt(wi->text(), w)) return false;
        tiers.push_back({m, w});
    }
    if (tiers.isEmpty()) return false;
    std::sort(tiers.begin(), tiers.end(), [](const Config::WeightTier& a, const Config::WeightTier& b){return a.minCode>b.minCode;});
    Config::setGeneratedOutputDir(dir);
    Config::setWeightTiers(tiers);
    Config::setSkipBrace(chkSkipBrace->isChecked());
    Config::setSkipLine(chkSkipLine->isChecked());
    Config::setSkipBlock(chkSkipBlock->isChecked());
    Config::setSkipHash(chkSkipHash->isChecked());
    Config::setSkipSingle(chkSkipSingle->isChecked());
    Config::setSkipDouble(chkSkipDouble->isChecked());
    Config::setSkipTemplate(chkSkipTemplate->isChecked());
    return true;
}

QString SettingsDialog::decideSavePath() const
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString p1 = appDir + "/../../config/lexer.json";
    QString p2 = appDir + "/config/lexer.json";
    if (QFileInfo(p1).exists()) return p1;
    if (QFileInfo(QFileInfo(p1).absolutePath()).isDir()) return p1;
    if (QFileInfo(QFileInfo(p2).absolutePath()).isDir()) return p2;
    return p1;
}
