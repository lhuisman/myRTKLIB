//---------------------------------------------------------------------------
#include <QShowEvent>
#include <QMessageBox>
#include <QFileDialog>

#include "rtklib.h"

#include "plotmain.h"
#include "skydlg.h"

extern Plot *plot;

//---------------------------------------------------------------------------
SkyImgDialog::SkyImgDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(btnClose, &QPushButton::clicked, this, &SkyImgDialog::accept);
    connect(btnGenMask, &QPushButton::clicked, this, &SkyImgDialog::btnGenMaskClicked);
    connect(btnLoad, &QPushButton::clicked, this, &SkyImgDialog::btnLoadClicked);
    connect(btnSave, &QPushButton::clicked, this, &SkyImgDialog::btnSaveClicked);
    connect(btnUpdate, &QPushButton::clicked, this, &SkyImgDialog::updateSky);
    connect(cBSkyResample, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SkyImgDialog::updateSky);
    connect(cBSkyElevationMask, &QCheckBox::clicked, this, &SkyImgDialog::updateSky);
    connect(cBSkyDestCorrection, &QCheckBox::clicked, this, &SkyImgDialog::skyDestCorrectionClicked);
    connect(cBSkyFlip, &QCheckBox::clicked, this, &SkyImgDialog::updateSky);
    connect(cBSkyBinarize, &QCheckBox::clicked, this, &SkyImgDialog::skyBinarizeClicked);
}
//---------------------------------------------------------------------------
void SkyImgDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

	updateField();
	updateEnable();
}
//---------------------------------------------------------------------------
void SkyImgDialog::btnSaveClicked()
{
    QFile fp;
    QString file = plot->skyImageFile;

    if (file == "") return;

	updateSky();

    file = file + ".tag";
    fp.setFileName(file);
    if (QFile::exists(file))
        if (QMessageBox::question(this, file, tr("File exists. Overwrite it?")) != QMessageBox::Yes) return;
    if (!fp.open(QIODevice::WriteOnly)) return;

    QString data;
    data = QString("%% sky image tag file: rtkplot %1 %2\n\n").arg(VER_RTKLIB, PATCH_LEVEL);
    data += QString("centx   = %1\n").arg(plot->skyCenter[0], 0, 'g', 6);
    data += QString("centy   = %1\n").arg(plot->skyCenter[1], 0, 'g', 6);
    data += QString("scale   = %1\n").arg(plot->skyScale, 0, 'g', 6);
    data += QString("roll    = %1\n").arg(plot->skyFOV[0], 0, 'g', 6);
    data += QString("pitch   = %1\n").arg(plot->skyFOV[1], 0, 'g', 6);
    data += QString("yaw     = %1\n").arg(plot->skyFOV[2], 0, 'g', 6);
    data += QString("destcorr= %1\n").arg(plot->skyDistortionCorrection);
    data += QString("resample= %1\n").arg(plot->skyResample);
    data += QString("flip    = %1\n").arg(plot->skyFlip);
    data += QString("dest    = %1 %2 %3 %4 %5 %6 %7 %8 %9s\n")
                .arg(plot->skyDistortion[1], 0, 'g', 6).arg(plot->skyDistortion[2], 0, 'g', 6).arg(plot->skyDistortion[3], 0, 'g', 6).arg(plot->skyDistortion[4], 0, 'g', 6)
                .arg(plot->skyDistortion[5], 0, 'g', 6).arg(plot->skyDistortion[6], 0, 'g', 6).arg(plot->skyDistortion[7], 0, 'g', 6).arg(plot->skyDistortion[8], 0, 'g', 6)
                .arg(plot->skyDistortion[9], 0, 'g', 6);
    data += QString("elmask  = %1\n").arg(plot->skyElevationMask);
    data += QString("binarize= %1\n").arg(plot->skyBinarize);
    data += QString("binthr1 = %1\n").arg(plot->skyBinThres1, 0, 'f', 2);
    data += QString("binthr2 = %1f\n").arg(plot->skyBinThres2, 0, 'f', 2);
    fp.write(data.toLatin1());
}
//---------------------------------------------------------------------------
void SkyImgDialog::updateField(void)
{
    setWindowTitle(plot->skyImageFile);

    sBSkySize1->setValue(plot->skySize[0]);
    sBSkySize2->setValue(plot->skySize[1]);
    sBSkyCenter1->setValue(plot->skyCenter[0]);
    sBSkyCenter2->setValue(plot->skyCenter[1]);
    sBSkyScale->setValue(plot->skyScale);
    sBSkyFOV1->setValue(plot->skyFOV[0]);
    sBSkyFOV2->setValue(plot->skyFOV[1]);
    sBSkyFOV3->setValue(plot->skyFOV[2]);
    sBSkyDest1->setValue(plot->skyDistortion[1]);
    sBSkyDest2->setValue(plot->skyDistortion[2]);
    sBSkyDest3->setValue(plot->skyDistortion[3]);
    sBSkyDest4->setValue(plot->skyDistortion[4]);
    sBSkyDest5->setValue(plot->skyDistortion[5]);
    sBSkyDest6->setValue(plot->skyDistortion[6]);
    sBSkyDest7->setValue(plot->skyDistortion[7]);
    sBSkyDest8->setValue(plot->skyDistortion[8]);
    sBSkyDest9->setValue(plot->skyDistortion[9]);
    cBSkyElevationMask->setChecked(plot->skyElevationMask);
    cBSkyDestCorrection->setChecked(plot->skyDistortionCorrection);
    cBSkyResample->setCurrentIndex(plot->skyResample);
    cBSkyFlip->setChecked(plot->skyFlip);
    cBSkyBinarize->setChecked(plot->skyBinarize);
    sBSkyBinThres1->setValue(plot->skyBinThres1);
    sBSkyBinThres2->setValue(plot->skyBinThres2);
}
//---------------------------------------------------------------------------
void SkyImgDialog::updateSky(void)
{
    plot->skyCenter[0] = sBSkyCenter1->value();
    plot->skyCenter[1] = sBSkyCenter2->value();
    plot->skyScale = sBSkyScale->value();
    plot->skyFOV[0] = sBSkyFOV1->value();
    plot->skyFOV[1] = sBSkyFOV2->value();
    plot->skyFOV[2] = sBSkyFOV3->value();
    plot->skyDistortion[1] = sBSkyDest1->value();
    plot->skyDistortion[2] = sBSkyDest2->value();
    plot->skyDistortion[3] = sBSkyDest3->value();
    plot->skyDistortion[4] = sBSkyDest4->value();
    plot->skyDistortion[5] = sBSkyDest5->value();
    plot->skyDistortion[6] = sBSkyDest6->value();
    plot->skyDistortion[7] = sBSkyDest7->value();
    plot->skyDistortion[8] = sBSkyDest8->value();
    plot->skyDistortion[9] = sBSkyDest9->value();
    plot->skyElevationMask = cBSkyElevationMask->isChecked();
    plot->skyDistortionCorrection = cBSkyDestCorrection->isChecked();
    plot->skyResample = cBSkyResample->currentIndex();
    plot->skyFlip = cBSkyFlip->isChecked();
    plot->skyBinarize = cBSkyBinarize->isChecked();
    plot->skyBinThres1 = sBSkyBinThres1->value();
    plot->skyBinThres2 = sBSkyBinThres2->value();

    plot->updateSky();
}
//---------------------------------------------------------------------------
void SkyImgDialog::updateEnable(void)
{
    sBSkyDest1->setEnabled(cBSkyDestCorrection->isChecked());
    sBSkyDest2->setEnabled(cBSkyDestCorrection->isChecked());
    sBSkyDest3->setEnabled(cBSkyDestCorrection->isChecked());
    sBSkyDest4->setEnabled(cBSkyDestCorrection->isChecked());
    sBSkyDest5->setEnabled(cBSkyDestCorrection->isChecked());
    sBSkyDest6->setEnabled(cBSkyDestCorrection->isChecked());
    sBSkyDest7->setEnabled(cBSkyDestCorrection->isChecked());
    sBSkyDest8->setEnabled(cBSkyDestCorrection->isChecked());
    sBSkyDest9->setEnabled(cBSkyDestCorrection->isChecked());
    sBSkyBinThres1->setEnabled(cBSkyBinarize->isChecked());
    sBSkyBinThres2->setEnabled(cBSkyBinarize->isChecked());
    btnGenMask->setEnabled(cBSkyBinarize->isChecked());
}
//---------------------------------------------------------------------------
void SkyImgDialog::skyDestCorrectionClicked()
{
	updateSky();
	updateEnable();
}
//---------------------------------------------------------------------------
void SkyImgDialog::btnLoadClicked()
{
    plot->readSkyTag(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open Tag"), QString(), tr("Tag File (*.tag);;All (*.*)"))));
    updateField();
    plot->updateSky();
}
//---------------------------------------------------------------------------
void SkyImgDialog::btnGenMaskClicked()
{
    QImage &bm = plot->skyImageResampled;
    double r, ca, sa, el, el0;
    int x, y, w, h, az, n;

    w = bm.width();
    h = bm.height();
    if (w <= 0 || h <= 0) return;

    for (az = 0; az <= 360; az++) {
        ca = cos(az * D2R);
        sa = sin(az * D2R);
        for (el = 90.0, n = 0, el0 = 0.0; el >= 0.0; el -= 0.1) {
            r = (1.0 - el / 90.0) * plot->skyScaleR;
            x = (int)floor(w / 2.0 + sa * r + 0.5);
            y = (int)floor(h / 2.0 + ca * r + 0.5);
            if (x < 0 || x >= w || y < 0 || y >= h) continue;
            QRgb pix = bm.pixel(x, y);
            if (qRed(pix) < 255 && qGreen(pix) < 255 && qBlue(pix) < 255) {
                if (++n == 1) el0 = el;
                if (n >= 5) break;
            } else {
                n = 0;
            }
        }
        plot->elevationMaskData[az] = el0 == 90.0 ? 0.0 : el0 * D2R;
    }
    plot->updateSky();
}
//---------------------------------------------------------------------------
void SkyImgDialog::skyBinarizeClicked()
{
	updateSky();
	updateEnable();
}
//---------------------------------------------------------------------------
