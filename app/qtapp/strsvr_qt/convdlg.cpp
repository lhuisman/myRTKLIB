//---------------------------------------------------------------------------
#include <QShowEvent>

#include "rtklib.h"
#include "convdlg.h"

//---------------------------------------------------------------------------
ConvDialog::ConvDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    for (int i = 0; i <= MAXRCVFMT; i++)
        cBInputFormat->addItem(formatstrs[i]);
    cBInputFormat->setCurrentIndex(0);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ConvDialog::saveClose);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ConvDialog::reject);
    connect(cBConversion, &QCheckBox::toggled, this, &ConvDialog::updateEnable);
}
//---------------------------------------------------------------------------
void ConvDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    cBConversion->setChecked(conversionEnabled);
    cBInputFormat->setCurrentIndex(conversionInputFormat);
    cBOutputFormat->setCurrentIndex(conversionOutputFormat);
    lEOutputMessages->setText(conversionMessage);
    lEOptions->setText(conversionOptions);

    updateEnable();
}
//---------------------------------------------------------------------------
void ConvDialog::saveClose()
{
    conversionEnabled = cBConversion->isChecked();
    conversionInputFormat = cBInputFormat->currentIndex();
    conversionOutputFormat = cBOutputFormat->currentIndex();
    conversionMessage = lEOutputMessages->text();
    conversionOptions = lEOptions->text();

    accept();
}
//---------------------------------------------------------------------------
void ConvDialog::updateEnable(void)
{
    cBInputFormat->setEnabled(cBConversion->isChecked());
    cBOutputFormat->setEnabled(cBConversion->isChecked());
    lEOutputMessages->setEnabled(cBConversion->isChecked());
    lEOptions->setEnabled(cBConversion->isChecked());
}
//---------------------------------------------------------------------------
