#pragma once
#include <QString>
class QWidget;

class FileService
{
public:
    static QString openFile(QWidget* parent, const QString& title, const QString& filter);
    static QString saveFile(QWidget* parent, const QString& title, const QString& suggested, const QString& filter);
    static bool readAllText(const QString& path, QString& out);
    static bool writeAllText(const QString& path, const QString& content);
    static QString ensureDir(const QString& dir);
};

