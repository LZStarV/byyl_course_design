#include "CodeViewController.h"
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QFile>
#include <QTextStream>
#include "../../../src/config/Config.h"
#include "../../mainwindow.h"

CodeViewController::CodeViewController(MainWindow* mw) : mw_(mw) {}

void CodeViewController::bind(QTabWidget* tabs)
{
    QObject::connect(tabs, &QTabWidget::currentChanged, this, &CodeViewController::onOuterTabChanged);
}

void CodeViewController::onOuterTabChanged(int idx)
{
    if (idx < 0) return;
    auto tabs = qobject_cast<QTabWidget*>(sender());
    QWidget* w = tabs ? tabs->widget(idx) : nullptr;
    if (!w) return;
    if (auto codeView = w->findChild<QPlainTextEdit*>("txtGeneratedCode"))
    {
        QString path = mw_->codePath();
        if (!path.isEmpty())
        {
            QFile f(path);
            if (f.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QTextStream in(&f);
                codeView->setPlainText(in.readAll());
                f.close();
            }
        }
    }
    if (auto syntaxCodeView = w->findChild<QPlainTextEdit*>("txtSyntaxGeneratedCode"))
    {
        QString path = Config::generatedOutputDir() + "/syntax/syntax_parser.cpp";
        QFile f(path);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&f);
            syntaxCodeView->setPlainText(in.readAll());
            f.close();
        }
    }
}
