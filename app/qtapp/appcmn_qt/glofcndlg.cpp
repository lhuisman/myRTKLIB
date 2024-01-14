//---------------------------------------------------------------------------

#include <QFileDialog>
#include <QDir>

#include "rtklib.h"
#include "glofcndlg.h"

//---------------------------------------------------------------------------
GloFcnDialog::GloFcnDialog(QWidget* parent) : QDialog(parent)
{
    setupUi(this);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &GloFcnDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &GloFcnDialog::reject);
    connect(btnClear, &QPushButton::clicked, this, &GloFcnDialog::clearFrequencies);
    connect(btnRead, &QPushButton::clicked, this, &GloFcnDialog::readRinex);
    connect(cBEnableFcn, &QCheckBox::stateChanged, this, &GloFcnDialog::updateEnable);

    updateEnable();
}
//---------------------------------------------------------------------------
void GloFcnDialog::readRinex()
{
    QString filename, text;
    nav_t nav;
	int prn;

    memset(&nav, 0, sizeof(nav_t));

    filename = QFileDialog::getOpenFileName(this, tr("Open RINEX file"), "", "*.*");
    
    if (!readrnx(qPrintable(filename), 0, "", NULL, &nav, NULL)) return;
	
    for (int i = 0; i < nav.ng; i++) {
        if (satsys(nav.geph[i].sat, &prn) != SYS_GLO) continue;
        getFcn(prn)->setValue(nav.geph[i].frq);
	}
    freenav(&nav, 0xFF);
}
//---------------------------------------------------------------------------
void GloFcnDialog::clearFrequencies()
{
    for (int i = 0; i < 27; i++) {
        getFcn(i + 1)->setValue(getFcn(i + 1)->minimum());
	}
}
//---------------------------------------------------------------------------
void GloFcnDialog::updateEnable(void)
{
    btnClear->setEnabled(cBEnableFcn->isChecked());
    btnRead ->setEnabled(cBEnableFcn->isChecked());
    lblDescription1->setEnabled(cBEnableFcn->isChecked());
    lblDescription1->setEnabled(cBEnableFcn->isChecked());
	
    for (int i = 0; i < 27; i++) {
        getFcn(i + 1)->setEnabled(cBEnableFcn->isChecked());
	}
}
//---------------------------------------------------------------------------
QSpinBox * GloFcnDialog::getFcn(int prn)
{
    QSpinBox *fcn[] = {
        sBFcn01, sBFcn02, sBFcn03, sBFcn04, sBFcn05, sBFcn06, sBFcn07, sBFcn08, sBFcn09, sBFcn10,
        sBFcn11, sBFcn12, sBFcn13, sBFcn14, sBFcn15, sBFcn16, sBFcn17, sBFcn18, sBFcn19, sBFcn20,
        sBFcn21, sBFcn22, sBFcn23, sBFcn24, sBFcn25, sBFcn26, sBFcn27
	};
    return fcn[prn - 1];
}
//---------------------------------------------------------------------------
int GloFcnDialog::getGloFcnEnable()
{
    return cBEnableFcn->isChecked();
}
//---------------------------------------------------------------------------
void GloFcnDialog::setGloFcnEnable(int enable)
{
    cBEnableFcn->setChecked(enable);

}
//---------------------------------------------------------------------------
int GloFcnDialog::getGloFcn(int i)
{
    int no = getFcn(i + 1)->value();
    if (no >= -7 && no <= 6) {
        return no + 8;
    }
    else return 0; // GLONASS FCN+8 (0:none)
}
//---------------------------------------------------------------------------
void GloFcnDialog::setGloFcn(int i, int fcn)
{
    if (fcn)
        getFcn(i + 1)->setValue(fcn - 8);
    else
        getFcn(i + 1)->setValue(getFcn(i + 1)->minimum());
}
//---------------------------------------------------------------------------
