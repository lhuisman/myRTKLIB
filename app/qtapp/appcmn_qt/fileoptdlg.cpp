//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include <stdio.h>

#include "keydlg.h"
#include "fileoptdlg.h"

#include <QShowEvent>
#include <QFileDialog>
#include <QIntValidator>
#include <QFileSystemModel>
#include <QCompleter>

//---------------------------------------------------------------------------
FileOptDialog::FileOptDialog(QWidget *parent, int options, int pathEnabled)
    : QDialog(parent)
{
    setupUi(this);

    keyDialog = new KeyDialog(this);

    QCompleter *fileCompleter = new QCompleter(this);
    QFileSystemModel *fileModel = new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    lEFilePath->setCompleter(fileCompleter);

    QAction *acFilePath = lEFilePath->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    acFilePath->setToolTip(tr("Select file path"));

    connect(btnCancel, &QPushButton::clicked, this, &FileOptDialog::reject);
    connect(btnOk, &QPushButton::clicked, this, &FileOptDialog::accept);
    connect(btnKey, &QPushButton::clicked, this, &FileOptDialog::keyDialogShow);
    connect(acFilePath, &QAction::triggered, this, &FileOptDialog::filePathSelect);
    connect(cBTimeTag, &QCheckBox::clicked, this, &FileOptDialog::updateEnable);
    connect(cBPathEnable, &QCheckBox::clicked, this, &FileOptDialog::updateEnable);

    cBSwapInterval->setValidator(new QIntValidator());

    setOptions(options);
    setPathEnabled(pathEnabled);
    updateEnable();
}

//---------------------------------------------------------------------------
void FileOptDialog::setOptions(int options)
{
    this->options = options;

    cBTimeTag->setText(options ? tr("TimeTag") : tr("Time"));
    lbFilePath->setVisible(options != 2);
    cBPathEnable->setVisible(options == 2);
    cBPathEnable->setChecked(options != 2 || pathEnabled);
    cBTimeSpeed->setVisible(!options);
    sBTimeStart->setVisible(!options);
    lbFilePath->setText(options ? tr("Output File Path") : tr("Input File Path"));
    lbPlus->setVisible(!options);
    lbSwapInterval->setVisible(options);
    cBSwapInterval->setVisible(options);
    btnKey->setVisible(options);
    cBTimeTag->setChecked(false);

}
//---------------------------------------------------------------------------
void FileOptDialog::setPathEnabled(int pathEnabled)
{
    this->pathEnabled = pathEnabled;

    cBPathEnable->setChecked(options != 2 || pathEnabled);
}
//---------------------------------------------------------------------------
void FileOptDialog::setPath(QString path)
{
    if (!options) { // input
        double speed = 1.0, start = 0.0;
        int size_fpos = 4;

        QStringList tokens = path.split("::");

        QString token;
        foreach(token, tokens){
            if (token == "T") cBTimeTag->setChecked(true);
            if (token.contains("+")) start = token.toDouble();
            if (token.contains("x")) speed = token.mid(1).toDouble();
            if (token.contains('P')) size_fpos = token.mid(2).toInt();
        }

        if (start <= 0.0) start = 0.0;
        if (speed <= 0.0) speed = 1.0;

        int index = cBTimeSpeed->findText(QString("x%1").arg(speed));
        if (index != -1) {
            cBTimeSpeed->setCurrentIndex(index);
        } else {
            cBTimeSpeed->addItem(QString("x%1").arg(speed), speed);
            cBTimeSpeed->setCurrentIndex(cBTimeSpeed->count());
        }
        sBTimeStart->setValue(start);
        cB64Bit->setChecked(size_fpos == 8);

        lEFilePath->setText(tokens.at(0));
    } else { // output
        double intv = 0.0;

        QStringList tokens = path.split("::");

        QString token;
        foreach(token, tokens){
            if (token == "T") cBTimeTag->setChecked(true);
            if (token.contains("S=")) intv = token.mid(2).toDouble();
        };
        int index = cBSwapInterval->findText(QString("%1 h").arg(intv, 0, 'g', 3));
        if (index != -1) {
            cBSwapInterval->setCurrentIndex(index);
        } else {
            if (intv == 0) {
                cBSwapInterval->setCurrentIndex(0);
            } else {
                cBSwapInterval->addItem(QString("%1 h").arg(intv, 0, 'g', 3), intv);
                cBSwapInterval->setCurrentIndex(cBTimeSpeed->count());
            }
        }

        lEFilePath->setText(tokens.at(0));
        setPathEnabled(cBPathEnable->isChecked());
	}
}
//---------------------------------------------------------------------------
QString FileOptDialog::getPath()
{
    QString str, path;
    bool okay;

    if (!options) {  // input
        path = lEFilePath->text();
        if (cBTimeTag->isChecked())
            path = path + "::T" + "::" + cBTimeSpeed->currentText() + "::+" + sBTimeStart->text();
        if (cB64Bit->isChecked()) {
            path = path + "::P=8";
        }
    } else { // output
        path = lEFilePath->text();
        if (cBTimeTag->isChecked()) path += "::T";
        str = cBSwapInterval->currentText();
        str = str.split(" ").first();
        str.toDouble(&okay);
        if (okay)
            path += "::S=" + str;
    }
    return path;
}
//---------------------------------------------------------------------------
void FileOptDialog::filePathSelect()
{
    if (!options)
        lEFilePath->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, QString(), lEFilePath->text())));
    else
        lEFilePath->setText(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, QString(), lEFilePath->text())));
}
//---------------------------------------------------------------------------
void FileOptDialog::keyDialogShow()
{
    keyDialog->exec();
}
//---------------------------------------------------------------------------
void FileOptDialog::updateEnable(void)
{
    lEFilePath->setEnabled(cBPathEnable->isChecked());
    cBTimeSpeed->setEnabled(cBTimeTag->isChecked());
    sBTimeStart->setEnabled(cBTimeTag->isChecked());
    cB64Bit ->setEnabled(cBTimeTag->isChecked());
    lbSwapInterval->setEnabled(cBTimeTag->isChecked());
    cBSwapInterval->setEnabled(cBPathEnable->isChecked());
    lbFilePath->setEnabled(cBPathEnable->isChecked());
    cBTimeTag->setEnabled(cBPathEnable->isChecked());
    btnKey->setEnabled(cBPathEnable->isChecked());
}
