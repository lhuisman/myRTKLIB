//---------------------------------------------------------------------------
// ported to Qt5 by Jens Reimann
#include "rcvoptdlg.h"

#include <QShowEvent>

//---------------------------------------------------------------------------
RcvOptDialog::RcvOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &RcvOptDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &RcvOptDialog::reject);
}
//---------------------------------------------------------------------------
void RcvOptDialog::setOptions(const QString &options)
{
    lEOption->setText(options);
}
//---------------------------------------------------------------------------
QString RcvOptDialog::getOptions()
{
    return lEOption->text();
}
//---------------------------------------------------------------------------
