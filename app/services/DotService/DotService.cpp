#include "DotService.h"
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QDateTime>
#include <QProcess>
#include "../../mainwindow.h"
#include "../NotificationService/NotificationService.h"
#include "../../../src/config/Config.h"

DotService::DotService(MainWindow* mw, NotificationService* notify) : mw_(mw), notify_(notify) {}

QString DotService::ensureGraphDir() const
{
    QString base = Config::generatedOutputDir();
    QDir g(base + "/graphs"); if (!g.exists()) g.mkpath(".");
    return g.absolutePath();
}

QString DotService::pickDotSavePath(const QString& suggestedName) const
{
    QString root = ensureGraphDir();
    QString def = root + "/" + suggestedName;
    return QFileDialog::getSaveFileName(mw_, QStringLiteral("保存DOT为"), def, QStringLiteral("Graphviz DOT (*.dot);;All (*)"));
}

QString DotService::pickImageSavePath(const QString& suggestedName, const QString& fmt) const
{
    QString root = ensureGraphDir();
    QString def = root + "/" + suggestedName;
    QString filter = fmt.compare("png", Qt::CaseInsensitive) == 0 ? QStringLiteral("PNG (*.png);;All (*)") : QStringLiteral("Image (*.*);;All (*)");
    return QFileDialog::getSaveFileName(mw_, QStringLiteral("保存图片为"), def, filter);
}

bool DotService::renderToFile(const QString& dotContent, const QString& outPath, const QString& fmt, int dpi) const
{
    QProcess proc; QStringList args; args << ("-T" + fmt) << "-o" << outPath; if (dpi > 0) args << ("-Gdpi=" + QString::number(dpi));
    proc.start("dot", args);
    if (!proc.waitForStarted()) { if (notify_) notify_->error("Graphviz启动失败，请检查dot安装"); return false; }
    proc.write(dotContent.toUtf8()); proc.closeWriteChannel();
    if (!proc.waitForFinished(20000)) { proc.kill(); if (notify_) notify_->error("Graphviz渲染超时"); return false; }
    bool ok = proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0 && QFileInfo(outPath).exists();
    if (!ok) { auto err = QString::fromUtf8(proc.readAllStandardError()); if (!err.trimmed().isEmpty() && notify_) notify_->error("Graphviz错误：" + err.left(200)); }
    return ok;
}

bool DotService::renderToTempPng(const QString& dotContent, QString& outPngPath, int dpi) const
{
    QString tmpDir = QDir::tempPath(); QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz"); outPngPath = tmpDir + "/byyl_preview_" + ts + ".png";
    QProcess proc; QStringList args; args << "-Tpng" << "-o" << outPngPath; if (dpi > 0) args << ("-Gdpi=" + QString::number(dpi));
    proc.start("dot", args);
    if (!proc.waitForStarted()) { if (notify_) notify_->error("Graphviz启动失败，请检查dot安装"); return false; }
    proc.write(dotContent.toUtf8()); proc.closeWriteChannel();
    if (!proc.waitForFinished(20000)) { proc.kill(); if (notify_) notify_->error("Graphviz渲染超时"); return false; }
    bool ok = proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0 && QFileInfo(outPngPath).exists();
    if (!ok) { auto err = QString::fromUtf8(proc.readAllStandardError()); if (!err.trimmed().isEmpty() && notify_) notify_->error("Graphviz错误：" + err.left(200)); }
    return ok;
}

void DotService::previewPng(const QString& pngPath, const QString& title) const
{
    if (mw_) mw_->previewImage(pngPath, title);
}

