#pragma once
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QVector>
#include <QtCore/QProcessEnvironment>

class EnvGuard {
public:
    explicit EnvGuard(const QMap<QString, QString>& setPairs) {
        for (auto it = setPairs.begin(); it != setPairs.end(); ++it) {
            QString key = it.key();
            QByteArray kba = key.toUtf8();
            QByteArray prev = qgetenv(kba.constData());
            original_[key] = QString::fromUtf8(prev);
            qputenv(kba.constData(), it.value().toUtf8());
            keys_.push_back(key);
        }
    }
    ~EnvGuard() {
        for (const auto& key : keys_) {
            QByteArray kba = key.toUtf8();
            const QString& prev = original_.value(key);
            if (prev.isEmpty()) qunsetenv(kba.constData());
            else qputenv(kba.constData(), prev.toUtf8());
        }
    }
private:
    QMap<QString, QString> original_;
    QVector<QString> keys_;
};

