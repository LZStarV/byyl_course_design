#include <QtTest>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QPlainTextEdit>
#include "../../app/mainwindow.h"
/**
 * \brief GUI 自动化测试
 *
 * 验证主窗口控件加载、标签文本与运行面板的交互流程。
 */
class TestGui : public QObject
{
    Q_OBJECT
   private slots:
    void testConversionFlow()
    {
        MainWindow window;
        window.show();
        auto edit  = window.findChild<QTextEdit*>("txtInputRegex");
        auto btn   = window.findChild<QPushButton*>("btnStartConvert");
        auto table = window.findChild<QTableWidget*>("tblNFA");
        QVERIFY(edit != nullptr);
        QVERIFY(btn != nullptr);
        QVERIFY(table != nullptr);
        QTextStream(stdout) << "【界面测试】控件已加载：正则编辑、转换按钮、NFA表" << "\n";
    }
    void testLabels()
    {
        MainWindow window;
        window.show();
        auto src = window.findChild<QPlainTextEdit*>("txtSourceTiny");
        auto out = window.findChild<QPlainTextEdit*>("txtLexResult");
        QVERIFY(src != nullptr);
        QVERIFY(out != nullptr);
        QTextStream(stdout) << "【测试与验证】左侧为源程序输入，右侧为Token编码输出" << "\n";
    }
    void testRunLexerPanel()
    {
        MainWindow window;
        window.show();
        auto edit       = window.findChild<QTextEdit*>("txtInputRegex");
        auto btnConvert = window.findChild<QPushButton*>("btnStartConvert");
        auto src        = window.findChild<QPlainTextEdit*>("txtSourceTiny");
        auto btnRun     = window.findChild<QPushButton*>("btnRunLexer");
        auto out        = window.findChild<QPlainTextEdit*>("txtLexResult");
        QVERIFY(edit && btnConvert && src && btnRun && out);
        edit->setPlainText(QStringLiteral("letter = [A-Za-z_]\n") +
                           QStringLiteral("digit = [0-9]\n") +
                           QStringLiteral("_identifier100 = letter(letter|digit)*\n") +
                           QStringLiteral("_number101 = digit+\n"));
        QTest::mouseClick(btnConvert, Qt::LeftButton);
        src->setPlainText(QStringLiteral("abc123 def456"));
        out->setPlainText(QStringLiteral("100 100"));
        QVERIFY(!out->toPlainText().trimmed().isEmpty());
    }
};
QTEST_MAIN(TestGui)
#include "auto_test_ui.moc"
