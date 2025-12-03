#pragma once
#include <QDialog>
#include <QVector>
#include <QString>

class QLineEdit;
class QTableWidget;
class QCheckBox;
class QPushButton;

class SettingsDialog : public QDialog
{
    Q_OBJECT
   public:
    explicit SettingsDialog(QWidget* parent = nullptr);

   private:
    QLineEdit*    edtOutDir;
    QTableWidget* tblTiers;
    QCheckBox*    chkSkipBrace;
    QCheckBox*    chkSkipLine;
    QCheckBox*    chkSkipBlock;
    QCheckBox*    chkSkipHash;
    QCheckBox*    chkSkipSingle;
    QCheckBox*    chkSkipDouble;
    QCheckBox*    chkSkipTemplate;
    QPushButton*  btnAddRow;
    QPushButton*  btnDelRow;
    QPushButton*  btnDefaults;
    QPushButton*  btnSave;
    QPushButton*  btnCancel;
    void          buildUi();
    void          loadCurrent();
    bool          collectAndApply();
    QString       decideSavePath() const;
};
