//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include <QShowEvent>

#include "maskoptdlg.h"
//---------------------------------------------------------------------------
MaskOptDialog::MaskOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &MaskOptDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &MaskOptDialog::reject);
    connect(cBMaskEnabled1, &QCheckBox::clicked, this, &MaskOptDialog::updateEnable);
    connect(cBMaskEnabled2, &QCheckBox::clicked, this, &MaskOptDialog::updateEnable);

    updateEnable();
}
//---------------------------------------------------------------------------
void MaskOptDialog::setSnrMask(snrmask_t mask)
{
    QDoubleSpinBox *widgets[][9] = {
        { sBMask_1_1, sBMask_1_2, sBMask_1_3, sBMask_1_4, sBMask_1_5, sBMask_1_6, sBMask_1_7, sBMask_1_8, sBMask_1_9 },
        { sBMask_2_1, sBMask_2_2, sBMask_2_3, sBMask_2_4, sBMask_2_5, sBMask_2_6, sBMask_2_7, sBMask_2_8, sBMask_2_9 },
        { sBMask_3_1, sBMask_3_2, sBMask_3_3, sBMask_3_4, sBMask_3_5, sBMask_3_6, sBMask_3_7, sBMask_3_8, sBMask_3_9 }
    };
    
    cBMaskEnabled1->setChecked(mask.ena[0]);
    cBMaskEnabled2->setChecked(mask.ena[1]);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 9; j++) {
            widgets[i][j]->setValue(mask.mask[i][j]);
        }
	}
    updateEnable();
}
//---------------------------------------------------------------------------
snrmask_t MaskOptDialog::getSnrMask()
{
    snrmask_t mask;
    QDoubleSpinBox *widgets[][9] = {
        { sBMask_1_1, sBMask_1_2, sBMask_1_3, sBMask_1_4, sBMask_1_5, sBMask_1_6, sBMask_1_7, sBMask_1_8, sBMask_1_9 },
        { sBMask_2_1, sBMask_2_2, sBMask_2_3, sBMask_2_4, sBMask_2_5, sBMask_2_6, sBMask_2_7, sBMask_2_8, sBMask_2_9 },
        { sBMask_3_1, sBMask_3_2, sBMask_3_3, sBMask_3_4, sBMask_3_5, sBMask_3_6, sBMask_3_7, sBMask_3_8, sBMask_3_9 }
	};
    
    mask.ena[0] = cBMaskEnabled1->isChecked();
    mask.ena[1] = cBMaskEnabled2->isChecked();
    for (int i = 0; i < 3; i++) for (int j = 0; j < 9; j++)
        mask.mask[i][j] = widgets[i][j]->value();

    return mask;
}
//---------------------------------------------------------------------------
void MaskOptDialog::updateEnable(void)
{
    QDoubleSpinBox *widgets[][9] = {
        { sBMask_1_1, sBMask_1_2, sBMask_1_3, sBMask_1_4, sBMask_1_5, sBMask_1_6, sBMask_1_7, sBMask_1_8, sBMask_1_9 },
        { sBMask_2_1, sBMask_2_2, sBMask_2_3, sBMask_2_4, sBMask_2_5, sBMask_2_6, sBMask_2_7, sBMask_2_8, sBMask_2_9 },
        { sBMask_3_1, sBMask_3_2, sBMask_3_3, sBMask_3_4, sBMask_3_5, sBMask_3_6, sBMask_3_7, sBMask_3_8, sBMask_3_9 }
	};

    for (int i = 0; i < 3; i++) for (int j = 0; j < 9; j++)
        widgets[i][j]->setEnabled(cBMaskEnabled1->isChecked() || cBMaskEnabled2->isChecked());

}
//---------------------------------------------------------------------------
