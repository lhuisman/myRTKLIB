//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include <QDialog>
#include <QShowEvent>

#include "keydlg.h"

//---------------------------------------------------------------------------
KeyDialog::KeyDialog(QWidget *parent, int flag)
    : QDialog(parent)
{
    setupUi(this);

    setFlag(flag);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &KeyDialog::close);
}
//---------------------------------------------------------------------------
void KeyDialog::setFlag(int flag)
{
    labelS->setVisible(flag != 3);
    labelSecond->setVisible(flag != 3);
    labelha->setVisible(flag != 3);
    label3H_Hour->setVisible(flag != 3);
    labelhb->setVisible(flag != 3);
    label6H_Hour->setVisible(flag != 3);
    labelhc->setVisible(flag != 3);
    label12H_Hour->setVisible(flag != 3);

    labelr->setVisible(flag >= 1);
    labelStationID->setVisible(flag >= 1);

    labels->setVisible(flag == 3);
    labelStationID_L->setVisible(flag == 3);
    labelS_2->setVisible(flag == 3);
    labelStationID_U->setVisible(flag == 3);
    labelN->setVisible(flag == 3);
    labelSequenceNumber->setVisible(flag == 3);

    labelb->setVisible(flag == 2);
    labelBaseStationID->setVisible(flag == 2);
}
//---------------------------------------------------------------------------
