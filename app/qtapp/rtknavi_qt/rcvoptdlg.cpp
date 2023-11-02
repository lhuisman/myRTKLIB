//---------------------------------------------------------------------------
// ported to Qt5 by Jens Reimann
#include "rcvoptdlg.h"

#include <QShowEvent>

//---------------------------------------------------------------------------
RcvOptDialog::RcvOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(btnOkClick()));
    connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
}
//---------------------------------------------------------------------------
void RcvOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    lEOption->setText(Option);
}
//---------------------------------------------------------------------------
void RcvOptDialog::btnOkClick()
{
    Option = lEOption->text();

    accept();
}
//---------------------------------------------------------------------------
