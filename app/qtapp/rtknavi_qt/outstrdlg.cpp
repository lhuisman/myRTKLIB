//---------------------------------------------------------------------------
#include <QShortcutEvent>
#include <QCompleter>
#include <QFileSystemModel>
#include <QFileDialog>

#include "rtklib.h"
#include "serioptdlg.h"
#include "fileoptdlg.h"
#include "tcpoptdlg.h"
#include "outstrdlg.h"
#include "keydlg.h"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
OutputStrDialog::OutputStrDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    keyDialog = new KeyDialog(this);
    serialOptDialog = new SerialOptDialog(this);
    tcpOptDialog = new TcpOptDialog(this);

    QCompleter *fileCompleter = new QCompleter(this);
    QFileSystemModel *fileModel = new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    lEFilePath1->setCompleter(fileCompleter);
    lEFilePath2->setCompleter(fileCompleter);

    // line edit actions
    QAction *aclEFilePath1Select = lEFilePath1->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    aclEFilePath1Select->setToolTip(tr("Select File"));
    QAction *aclEFilePath2Select = lEFilePath2->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    aclEFilePath2Select->setToolTip(tr("Select File"));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &OutputStrDialog::saveClose);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &OutputStrDialog::reject);
    connect(aclEFilePath1Select, &QAction::triggered, this, &OutputStrDialog::selectFile1);
    connect(aclEFilePath2Select, &QAction::triggered, this, &OutputStrDialog::selectFile2);
    connect(btnKey, &QPushButton::clicked, this, &OutputStrDialog::showKeyDialog);
    connect(btnStream1, &QPushButton::clicked, this, &OutputStrDialog::showStream1Options);
    connect(btnStream2, &QPushButton::clicked, this, &OutputStrDialog::showStream2Options);
    connect(cBStream1, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OutputStrDialog::updateEnable);
    connect(cBStream2, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OutputStrDialog::updateEnable);
    connect(cBStream1C, &QCheckBox::clicked, this, &OutputStrDialog::updateEnable);
    connect(cBStream2C, &QCheckBox::clicked, this, &OutputStrDialog::updateEnable);

    cBSwapInterval->setValidator(new QDoubleValidator(this));
}
//---------------------------------------------------------------------------
void OutputStrDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;
    
    cBStream1C->setChecked(streamEnabled[0]);
    cBStream2C->setChecked(streamEnabled[1]);
    cBStream1->setCurrentIndex(stream[0]);
    cBStream2->setCurrentIndex(stream[1]);
    cBFormat1->setCurrentIndex(format[0]);
    cBFormat2->setCurrentIndex(format[1]);
    lEFilePath1->setText(getFilePath(paths[0][2]));
    lEFilePath2->setText(getFilePath(paths[1][2]));
    cBSwapInterval->insertItem(0, swapInterval); cBSwapInterval->setCurrentIndex(0);
    cBTimeTag->setChecked(outputTimeTag);

    updateEnable();
}
//---------------------------------------------------------------------------
void OutputStrDialog::saveClose()
{
    streamEnabled[0] = cBStream1C->isChecked();
    streamEnabled[1] = cBStream2C->isChecked();
    stream[0] = cBStream1->currentIndex();
    stream[1] = cBStream2->currentIndex();
    format[0] = cBFormat1->currentIndex();
    format[1] = cBFormat2->currentIndex();
    paths [0][2] = setFilePath(lEFilePath1->text());
    paths [1][2] = setFilePath(lEFilePath2->text());
    swapInterval = cBSwapInterval->currentText();
    outputTimeTag = cBTimeTag->isChecked();

    accept();
}
//---------------------------------------------------------------------------
void OutputStrDialog::selectFile1()
{
    lEFilePath1->setText(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Load..."), lEFilePath1->text())));
}
//---------------------------------------------------------------------------
void OutputStrDialog::selectFile2()
{
    lEFilePath2->setText(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Load..."), lEFilePath2->text())));
}
//---------------------------------------------------------------------------
void OutputStrDialog::showKeyDialog()
{
    keyDialog->exec();
}
//---------------------------------------------------------------------------
void OutputStrDialog::showStream1Options()
{
    switch (cBStream1->currentIndex()) {
        case 0: showSerialOptions(0, 0); break;
        case 1: showTcpOptions(0, 1); break;
        case 2: showTcpOptions(0, 0); break;
        case 3: showTcpOptions(0, 2); break;
        case 4: showTcpOptions(0, 4); break;
    }
}
//---------------------------------------------------------------------------
void OutputStrDialog::showStream2Options()
{
    switch (cBStream2->currentIndex()) {
        case 0: showSerialOptions(1, 0); break;
        case 1: showTcpOptions(1, 1); break;
        case 2: showTcpOptions(1, 0); break;
        case 3: showTcpOptions(1, 2); break;
        case 4: showTcpOptions(0, 4); break;
    }
}
//---------------------------------------------------------------------------
QString OutputStrDialog::getFilePath(const QString path)
{
    return path.mid(0, path.indexOf("::"));
}
//---------------------------------------------------------------------------
QString OutputStrDialog::setFilePath(const QString p)
{
    QString path = p;
    QString str;
    bool okay;

    if (cBTimeTag->isChecked()) path += "::T";
    str = cBSwapInterval->currentText();
    str.toDouble(&okay);
    if (okay)
        path += "::S=" + str;
    return path;
}
//---------------------------------------------------------------------------
void OutputStrDialog::showSerialOptions(int index, int opt)
{
    serialOptDialog->setPath(paths[index][0]);
    serialOptDialog->setOptions(opt);

    serialOptDialog->exec();
    if (serialOptDialog->result() != QDialog::Accepted) return;

    paths[index][0] = serialOptDialog->getPath();
}
//---------------------------------------------------------------------------
void OutputStrDialog::showTcpOptions(int index, int opt)
{
    tcpOptDialog->setPath(paths[index][1]);
    tcpOptDialog->setOptions(opt);
    tcpOptDialog->setHistory(history, 10);

    tcpOptDialog->exec();
    if (tcpOptDialog->exec() != QDialog::Accepted) return;

    paths[index][1] = tcpOptDialog->getPath();
    for (int i = 0; i < 10; i++) {
        history[i] = tcpOptDialog->getHistory()[i];
	}
}
//---------------------------------------------------------------------------
void OutputStrDialog::updateEnable(void)
{
    int ena = (cBStream1C->isChecked() && (cBStream1->currentIndex() == 5)) ||
              (cBStream2C->isChecked() && (cBStream2->currentIndex() == 5));

    cBStream1->setEnabled(cBStream1C->isChecked());
    cBStream2->setEnabled(cBStream2C->isChecked());
    btnStream1->setEnabled(cBStream1C->isChecked() && cBStream1->currentIndex() <= 4);
    btnStream2->setEnabled(cBStream2C->isChecked() && cBStream2->currentIndex() <= 4);
    lEFilePath1->setEnabled(cBStream1C->isChecked() && cBStream1->currentIndex() == 5);
    lEFilePath2->setEnabled(cBStream2C->isChecked() && cBStream2->currentIndex() == 5);
    lblF1->setEnabled(ena);
    lblSwapInterval->setEnabled(ena);
    lblH->setEnabled(ena);
    cBTimeTag->setEnabled(ena);
    cBSwapInterval->setEnabled(ena);
    btnKey->setEnabled(ena);
}
//---------------------------------------------------------------------------
