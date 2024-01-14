//---------------------------------------------------------------------------

#include <QShowEvent>

#include "rtklib.h"
#include "keydlg.h"
#include "markdlg.h"
#include "refdlg.h"
#include "naviopt.h"
#include "navimain.h"

extern rtksvr_t rtksvr;

//---------------------------------------------------------------------------
QMarkDialog::QMarkDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    nMark = 1;
    lblMarker->setText(QString("%%r=%1").arg(nMark, 3, 10, QLatin1Char('0')));

    connect(btnKeyDlg, &QPushButton::clicked, this, &QMarkDialog::showKeyDialog);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QMarkDialog::saveClose);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QMarkDialog::close);
    connect(cBMarkerNameC, &QCheckBox::clicked, this, &QMarkDialog::updateEnable);
    connect(rBGo, &QRadioButton::toggled, this, &QMarkDialog::updateEnable);
    connect(rBStop, &QRadioButton::toggled, this, &QMarkDialog::updateEnable);
    connect(rBFix, &QRadioButton::toggled, this, &QMarkDialog::updateEnable);
    connect(btnPosition, &QPushButton::clicked, this, &QMarkDialog::btnPositionClicked);

    keyDialog = new KeyDialog(this);
}
//---------------------------------------------------------------------------
void QMarkDialog::saveClose()
{
    if (cBMarkerNameC->isChecked()) {
        rtksvrmark(&rtksvr, qPrintable(getName()), qPrintable(getComment()));
        nMark++;
        lblMarker->setText(QString("%%r=%1").arg(nMark, 3, 10, QLatin1Char('0')));
    }
}
//---------------------------------------------------------------------------
void QMarkDialog::updateEnable(void)
{
    int positionMode = getPositionMode();
    bool ena = positionMode == PMODE_STATIC || positionMode == PMODE_PPP_STATIC ||
               positionMode == PMODE_KINEMA || positionMode == PMODE_PPP_KINEMA ||
               positionMode == PMODE_FIXED || positionMode == PMODE_PPP_FIXED;

    rBStop->setEnabled(ena);
    rBGo->setEnabled(ena);
    rBFix->setEnabled(ena);
    gBPositionMode->setEnabled(ena);
    sBLatitude->setEnabled(rBFix->isChecked());
    sBLongitude->setEnabled(rBFix->isChecked());
    sBHeight->setEnabled(rBFix->isChecked());
    btnPosition->setEnabled(rBFix->isChecked());
    lblPosition->setEnabled(rBFix->isChecked());
    cBMarkerName->setEnabled(cBMarkerNameC->isChecked());
}
//---------------------------------------------------------------------------
void QMarkDialog::showKeyDialog()
{
    keyDialog->setWindowTitle(tr("Key Replacement in Marker Name"));
    keyDialog->show();
}
//---------------------------------------------------------------------------
void QMarkDialog::btnPositionClicked()
{
    RefDialog *refDialog =  new RefDialog(this);

    refDialog->setRoverPosition(sBLatitude->value(), sBLongitude->value(), sBHeight->value());
    refDialog->stationPositionFile = stationPositionFileF;

    if (refDialog->result() != QDialog::Accepted) return;

    sBLatitude->setValue(refDialog->getPosition()[0]);
    sBLongitude->setValue(refDialog->getPosition()[1]);
    sBHeight->setValue(refDialog->getPosition()[2]);
}
//---------------------------------------------------------------------------
void QMarkDialog::setPositionMode(int mode)
{
    positionMode = mode;

    if (positionMode == PMODE_STATIC || positionMode == PMODE_PPP_STATIC) {
        rBStop->setChecked(true);
    } else if (positionMode == PMODE_KINEMA || positionMode == PMODE_PPP_KINEMA) {
        rBGo->setChecked(true);
    } else {
        rBStop->setChecked(false);
        rBGo->setChecked(false);
    }
    updateEnable();
}
//---------------------------------------------------------------------------
int QMarkDialog::getPositionMode()
{
    if (rBGo->isChecked()) {
        if (positionMode == PMODE_STATIC || positionMode == PMODE_FIXED)
            positionMode = PMODE_KINEMA;
        else if (positionMode == PMODE_PPP_STATIC || positionMode == PMODE_PPP_FIXED)
            positionMode = PMODE_PPP_KINEMA;
    } else if (rBStop->isChecked()) {
        if (positionMode == PMODE_KINEMA || positionMode == PMODE_FIXED)
            positionMode = PMODE_STATIC;
        else if (positionMode == PMODE_PPP_KINEMA || positionMode == PMODE_PPP_FIXED)
            positionMode = PMODE_PPP_STATIC;
    } else if (rBFix->isChecked()) {
        if (positionMode == PMODE_KINEMA || positionMode == PMODE_STATIC) {
            positionMode = PMODE_FIXED;
        }
        else if (positionMode == PMODE_PPP_KINEMA || positionMode == PMODE_PPP_STATIC) {
            positionMode = PMODE_PPP_FIXED;
        }
    }
    return positionMode;
}
//---------------------------------------------------------------------------
void QMarkDialog::setName(const QString &name)
{
    cBMarkerName->setCurrentText(name);
}
//---------------------------------------------------------------------------
QString QMarkDialog::getName()
{
    QString mrkr = cBMarkerName->currentText();
    char str2[1024];

    reppath(qPrintable(mrkr), str2, utc2gpst(timeget()), qPrintable(QString("%1").arg(nMark, 3, 10, QChar('0'))), "");
    return mrkr;
}
//---------------------------------------------------------------------------
void QMarkDialog::setComment(const QString &comment)
{
    lEMarkerComment->setText(comment);
}
//---------------------------------------------------------------------------
QString QMarkDialog::getComment()
{
    return lEMarkerComment->text();
}
//---------------------------------------------------------------------------
void QMarkDialog::setStationPositionFile(const QString &file)
{
    stationPositionFileF = file;
}
//---------------------------------------------------------------------------
QString QMarkDialog::getStationPositionFile()
{
    return stationPositionFileF;
}
//---------------------------------------------------------------------------
