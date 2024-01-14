//---------------------------------------------------------------------------

#include "getmain.h"
#include "staoptdlg.h"

#include <QShowEvent>
#include <QFileDialog>

extern MainForm *mainForm;

//---------------------------------------------------------------------------
StaListDialog::StaListDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &StaListDialog::btnOkClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &StaListDialog::reject);
    connect(btnLoad, &QPushButton::clicked, this, &StaListDialog::loadFile);
    connect(btnSave, &QPushButton::clicked, this, &StaListDialog::saveFile);
}
//---------------------------------------------------------------------------
void StaListDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    stationListWidget->clear();

    for (int i = 0; i < mainForm->stationListWidget->count(); i++)
        stationListWidget->addItem(mainForm->stationListWidget->item(i));
}
//---------------------------------------------------------------------------
void StaListDialog::btnOkClicked()
{
    mainForm->stationListWidget->clear();

    for (int i = 0; i < stationListWidget->count(); i++)
        mainForm->stationListWidget->addItem(new QListWidgetItem(*stationListWidget->item(i)));

    accept();
}
//---------------------------------------------------------------------------
void StaListDialog::loadFile()
{
    QString filename;
    QFile fp;
    QByteArray buff;

    filename = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open...")));

    fp.setFileName(filename);
    if (!fp.open(QIODevice::ReadOnly)) return;

    stationListWidget->clear();

    while (!fp.atEnd()) {
        buff = fp.readLine();
        buff = buff.mid(buff.indexOf('#'));
        stationListWidget->addItem(buff.replace("\n", ""));
    }
}
//---------------------------------------------------------------------------
void StaListDialog::saveFile()
{
    QString file = QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Save...")));
    QFile fp;

    fp.setFileName(file);
    if (!fp.open(QIODevice::WriteOnly)) return;

    for (int i = 0; i < stationListWidget->count(); i++)
        fp.write((stationListWidget->item(i)->text() + "\n").toLatin1());
}
//---------------------------------------------------------------------------
