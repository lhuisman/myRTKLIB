//---------------------------------------------------------------------------

#include "getoptdlg.h"
#include "getmain.h"

#include <QFileDialog>
#include <QShowEvent>
#include <QIntValidator>
#include <QCompleter>
#include <QFileSystemModel>

extern MainForm *mainForm;

//---------------------------------------------------------------------------
DownOptDialog::DownOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    QCompleter *fileCompleter = new QCompleter(this);
    QFileSystemModel *fileModel = new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    lEUrlFilename->setCompleter(fileCompleter);
    lELogFilename->setCompleter(fileCompleter);

    QAction *acLogFilename = lELogFilename->addAction(QIcon(":/icons/folder"), QLineEdit::TrailingPosition);
    acLogFilename->setToolTip(tr("Select Log File Path"));

    QAction *acUrlFilename = lEUrlFilename->addAction(QIcon(":/icons/folder"), QLineEdit::TrailingPosition);
    acUrlFilename->setToolTip(tr("Select URL File Path"));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &DownOptDialog::btnOkClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DownOptDialog::reject);
    connect(acLogFilename, &QAction::triggered, this, &DownOptDialog::selectLogFilename);
    connect(acUrlFilename, &QAction::triggered, this, &DownOptDialog::selectUrlFile);
}
//---------------------------------------------------------------------------
void DownOptDialog::selectUrlFile()
{
    lEUrlFilename->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("GNSS Data URL File"))));
}
//---------------------------------------------------------------------------
void DownOptDialog::selectLogFilename()
{
    lELogFilename->setText(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Download Log File"))));
}
//---------------------------------------------------------------------------
void DownOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    cBKeepErr->setChecked(mainForm->holdErr);
    cBKeepList->setChecked(mainForm->holdList);
    sBTestColumnCount->setValue(mainForm->columnCnt);
    lEProxy->setText(mainForm->proxyAddr);
    lEUrlFilename->setText(mainForm->urlFile);
    lELogFilename->setText(mainForm->logFile);
    cBLogAppend->setChecked(mainForm->logAppend);
    cBDateFormat->setCurrentIndex(mainForm->dateFormat);
    cBTraceLevel->setCurrentIndex(mainForm->traceLevel);
}
//---------------------------------------------------------------------------
void DownOptDialog::btnOkClicked()
{
    mainForm->holdErr = cBKeepErr->isChecked();
    mainForm->holdList = cBKeepList->isChecked();
    mainForm->columnCnt = sBTestColumnCount->value();
    mainForm->proxyAddr = lEProxy->text();
    mainForm->urlFile = lEUrlFilename->text();
    mainForm->logFile = lELogFilename->text();
    mainForm->logAppend = cBLogAppend->isChecked();
    mainForm->dateFormat = cBDateFormat->currentIndex();
    mainForm->traceLevel = cBTraceLevel->currentIndex();

    accept();
}
//---------------------------------------------------------------------------
