//---------------------------------------------------------------------------

#include "freqdlg.h"
//---------------------------------------------------------------------------
FreqDialog::FreqDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &FreqDialog::accept);

}
//---------------------------------------------------------------------------
