#include "FileService.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDir>

QString FileService::openFile(QWidget* parent, const QString& title, const QString& filter)
{
    return QFileDialog::getOpenFileName(parent, title, QString(), filter);
}

QString FileService::saveFile(QWidget* parent, const QString& title, const QString& suggested, const QString& filter)
{
    return QFileDialog::getSaveFileName(parent, title, suggested, filter);
}

bool FileService::readAllText(const QString& path, QString& out)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream in(&f);
    out = in.readAll();
    f.close();
    return true;
}

bool FileService::writeAllText(const QString& path, const QString& content)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream o(&f);
    o << content;
    f.close();
    return true;
}

QString FileService::ensureDir(const QString& dir)
{
    QDir d(dir);
    if (!d.exists()) d.mkpath(".");
    return d.absolutePath();
}

