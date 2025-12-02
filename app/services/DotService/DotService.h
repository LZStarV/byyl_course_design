#pragma once
#include <QString>
class MainWindow;
class NotificationService;

class DotService
{
public:
    DotService(MainWindow* mw, NotificationService* notify);
    QString ensureGraphDir() const;
    QString pickDotSavePath(const QString& suggestedName) const;
    QString pickImageSavePath(const QString& suggestedName, const QString& fmt) const;
    bool renderToFile(const QString& dotContent, const QString& outPath, const QString& fmt, int dpi) const;
    bool renderToTempPng(const QString& dotContent, QString& outPngPath, int dpi) const;
    void previewPng(const QString& pngPath, const QString& title) const;
private:
    MainWindow* mw_;
    NotificationService* notify_;
};

