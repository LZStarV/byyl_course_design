#pragma once
#include <QString>
#include <QMap>
#include <QVector>

QString generateSyntaxParserSource(const QMap<QString, QMap<QString, int>>& table,
                                   const QVector<QString>&                  nonterms,
                                   const QVector<QString>&                  terms,
                                   const QString&                           start);
