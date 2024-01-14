//---------------------------------------------------------------------------

#include "launchmain.h"
#include "launchoptdlg.h"

#include <QShowEvent>

//---------------------------------------------------------------------------
LaunchOptDialog::LaunchOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &LaunchOptDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &LaunchOptDialog::reject);
}
//---------------------------------------------------------------------------
void LaunchOptDialog::setOption(int option)
{

    if (option == 1) {
        rBOptionMkl->setChecked(true);
	}
    else if (option == 2) {
        rBOptionWin64->setChecked(true);
	}
	else {
        rBOptionNormal->setChecked(true);
	}
}
//---------------------------------------------------------------------------
int LaunchOptDialog::getOption()
{
    if (rBOptionMkl->isChecked()) {
        return 1;
	}
    else if (rBOptionWin64->isChecked()) {
        return 2;
	}
	else {
        return 0;
	}
}
//---------------------------------------------------------------------------
void LaunchOptDialog::setMinimize(int minimize)
{
    cBMinimize->setChecked(minimize);
}
//---------------------------------------------------------------------------
int LaunchOptDialog::getMinimize()
{
    return cBMinimize->isChecked();
}
//---------------------------------------------------------------------------
