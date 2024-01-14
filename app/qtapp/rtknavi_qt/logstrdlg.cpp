//---------------------------------------------------------------------------
#include <QDialog>
#include <QShowEvent>
#include <QFileDialog>
#include <QCompleter>
#include <QFileSystemModel>

#include "serioptdlg.h"
#include "fileoptdlg.h"
#include "tcpoptdlg.h"
#include "logstrdlg.h"
#include "keydlg.h"

//---------------------------------------------------------------------------
LogStrDialog::LogStrDialog(QWidget *parent)
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
    lEFilePath3->setCompleter(fileCompleter);

    // line edit actions
    QAction *aclEFilePath1Select = lEFilePath1->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    aclEFilePath1Select->setToolTip(tr("Select File"));
    QAction *aclEFilePath2Select = lEFilePath2->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    aclEFilePath2Select->setToolTip(tr("Select File"));
    QAction *aclEFilePath3Select = lEFilePath3->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    aclEFilePath3Select->setToolTip(tr("Select File"));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &LogStrDialog::saveClose);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &LogStrDialog::reject);
    connect(aclEFilePath1Select, &QAction::triggered, this, &LogStrDialog::selectFile1);
    connect(aclEFilePath2Select, &QAction::triggered, this, &LogStrDialog::selectFile2);
    connect(aclEFilePath3Select, &QAction::triggered, this, &LogStrDialog::selectFile3);
    connect(btnKey, &QPushButton::clicked, this, &LogStrDialog::showKeyDialog);
    connect(btnStream1, &QPushButton::clicked, this, &LogStrDialog::showStreamOptions1);
    connect(btnStream2, &QPushButton::clicked, this, &LogStrDialog::showStreamOptions2);
    connect(btnStream3, &QPushButton::clicked, this, &LogStrDialog::showStreamOptions3);
    connect(cBStream1, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &LogStrDialog::updateEnable);
    connect(cBStream2, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &LogStrDialog::updateEnable);
    connect(cBStream3, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &LogStrDialog::updateEnable);
    connect(cBStream1C, &QCheckBox::clicked, this, &LogStrDialog::updateEnable);
    connect(cBStream2C, &QCheckBox::clicked, this, &LogStrDialog::updateEnable);
    connect(cBStream3C, &QCheckBox::clicked, this, &LogStrDialog::updateEnable);

    cBSwapInterval->setValidator(new QDoubleValidator(this));
}
//---------------------------------------------------------------------------
void LogStrDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;
    
    cBStream1C->setChecked(streamEnabled[0]);
    cBStream2C->setChecked(streamEnabled[1]);
    cBStream3C->setChecked(streamEnabled[2]);
    cBStream1->setCurrentIndex(stream[0]);
    cBStream2->setCurrentIndex(stream[1]);
    cBStream3->setCurrentIndex(stream[2]);
    lEFilePath1->setText(getFilePath(paths[0][2]));
    lEFilePath2->setText(getFilePath(paths[1][2]));
    lEFilePath3->setText(getFilePath(paths[2][2]));
    cBSwapInterval->insertItem(0, swapInterval); cBSwapInterval->setCurrentIndex(0);
    cBTimeTag->setChecked(logTimeTag);

	updateEnable();
}
//---------------------------------------------------------------------------
void LogStrDialog::saveClose()
{
    streamEnabled[0] = cBStream1C->isChecked();
    streamEnabled[1] = cBStream2C->isChecked();
    streamEnabled[2] = cBStream3C->isChecked();
    stream[0] = cBStream1->currentIndex();
    stream[1] = cBStream2->currentIndex();
    stream[2] = cBStream3->currentIndex();
    paths[0][2] = setFilePath(lEFilePath1->text());
    paths[1][2] = setFilePath(lEFilePath2->text());
    paths[2][2] = setFilePath(lEFilePath3->text());
    swapInterval = cBSwapInterval->currentText();
    logTimeTag = cBTimeTag->isChecked();

    accept();
}
//---------------------------------------------------------------------------
void LogStrDialog::selectFile1()
{
    lEFilePath1->setText(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Open..."), lEFilePath1->text())));
}
//---------------------------------------------------------------------------
void LogStrDialog::selectFile2()
{
    lEFilePath2->setText(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Open..."), lEFilePath2->text())));
}
//---------------------------------------------------------------------------
void LogStrDialog::selectFile3()
{
    lEFilePath3->setText(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Open..."), lEFilePath3->text())));
}
//---------------------------------------------------------------------------
void LogStrDialog::showKeyDialog()
{
    keyDialog->exec();
}
//---------------------------------------------------------------------------
void LogStrDialog::showStreamOptions1()
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
void LogStrDialog::showStreamOptions2()
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
void LogStrDialog::showStreamOptions3()
{
    switch (cBStream3->currentIndex()) {
        case 0: showSerialOptions(2, 0); break;
        case 1: showTcpOptions(2, 1); break;
        case 2: showTcpOptions(2, 0); break;
        case 3: showTcpOptions(2, 2); break;
    case 4: showTcpOptions(0, 4); break;
    }
}
//---------------------------------------------------------------------------
QString LogStrDialog::getFilePath(const QString &path)
{
    return path.mid(0, path.indexOf("::"));
}
//---------------------------------------------------------------------------
QString LogStrDialog::setFilePath(const QString &p)
{
    QString path = p;
    QString str;
    bool okay;

    if (cBTimeTag->isChecked()) path += "::T";
    str = cBSwapInterval->currentText();
    str.toDouble(&okay);
    if (okay) path += "::S=" + str;
	return path;
}
//---------------------------------------------------------------------------
void LogStrDialog::showSerialOptions(int index, int opt)
{
    serialOptDialog->setPath(paths[index][0]);
    serialOptDialog->setOptions(opt);

    serialOptDialog->exec();
    if (serialOptDialog->result() != QDialog::Accepted) return;

    paths[index][0] = serialOptDialog->getPath();
}
//---------------------------------------------------------------------------
void LogStrDialog::showTcpOptions(int index, int opt)
{
    tcpOptDialog->setPath(paths[index][1]);
    tcpOptDialog->setOptions(opt);
    tcpOptDialog->setHistory(history, 10);

    tcpOptDialog->exec();
    if (tcpOptDialog->result() != QDialog::Accepted) return;

    paths[index][1] = tcpOptDialog->getPath();
    for (int i = 0; i < 10; i++) {
        history[i] = tcpOptDialog->getHistory()[i];
	}
}
//---------------------------------------------------------------------------
void LogStrDialog::updateEnable()
{
    int ena = (cBStream1C->isChecked() && cBStream1->currentIndex() == 5) ||
              (cBStream2C->isChecked() && cBStream2->currentIndex() == 5) ||
              (cBStream3C->isChecked() && cBStream3->currentIndex() == 5);

    cBStream1->setEnabled(cBStream1C->isChecked());
    cBStream2->setEnabled(cBStream2C->isChecked());
    cBStream3->setEnabled(cBStream3C->isChecked());
    btnStream1->setEnabled(cBStream1C->isChecked() && cBStream1->currentIndex() <= 4);
    btnStream2->setEnabled(cBStream2C->isChecked() && cBStream2->currentIndex() <= 4);
    btnStream3->setEnabled(cBStream3C->isChecked() && cBStream3->currentIndex() <= 4);
    lEFilePath1->setEnabled(cBStream1C->isChecked() && cBStream1->currentIndex() == 5);
    lEFilePath2->setEnabled(cBStream2C->isChecked() && cBStream2->currentIndex() == 5);
    lEFilePath3->setEnabled(cBStream3C->isChecked() && cBStream3->currentIndex() == 5);
    lblSwapInterval->setEnabled(ena);
    lblH->setEnabled(ena);
    lblF1->setEnabled(ena);
    cBTimeTag->setEnabled(ena);
    cBSwapInterval->setEnabled(ena);
    btnKey->setEnabled(ena);
}
//---------------------------------------------------------------------------
