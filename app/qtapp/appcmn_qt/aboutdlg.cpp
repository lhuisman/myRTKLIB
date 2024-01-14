//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include "aboutdlg.h"
#include "rtklib.h"

//---------------------------------------------------------------------------
AboutDialog::AboutDialog(QWidget *parent, QPixmap icon, QString labelText)
    : QDialog(parent)
{
    setupUi(this);
    this->icon = icon.scaled(128, 128);
    aboutString = labelText;

    connect(btnOkay, &QPushButton::clicked, this, &AboutDialog::accept);
}

void AboutDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    lbIcon->setPixmap(icon);
    lbAbout->setText(aboutString);
    lbVersion->setText(tr("with RTKLIB ver.%1 %2").arg(VER_RTKLIB).arg(PATCH_LEVEL));
    lbCopyright->setText(QString("%1\nQt Version\n2016-2024 Jens Reimann").arg(COPYRIGHT_RTKLIB));
}
