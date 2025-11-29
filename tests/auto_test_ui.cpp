#include <QtTest>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include "../byyl_course_design_1/mainwindow.h"
class TestGui: public QObject {
    Q_OBJECT
private slots:
    void testConversionFlow(){
        MainWindow window; window.show();
        auto edit = window.findChild<QTextEdit*>("txtInputRegex");
        auto btn = window.findChild<QPushButton*>("btnStartConvert");
        auto table = window.findChild<QTableWidget*>("tblNFA");
        QVERIFY(edit != nullptr);
        QVERIFY(btn != nullptr);
        QVERIFY(table != nullptr);
    }
};
QTEST_MAIN(TestGui)
#include "auto_test_ui.moc"
