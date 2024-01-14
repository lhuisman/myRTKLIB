//---------------------------------------------------------------------------
#include <QShowEvent>

#include "convmain.h"
#include "convopt.h"
#include "codeopt.h"
#include "freqdlg.h"
#include "glofcndlg.h"

extern MainWindow *mainWindow;
//---------------------------------------------------------------------------
ConvOptDialog::ConvOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    codeOptDialog = new CodeOptDialog(this);
    gloFcnDialog = new GloFcnDialog(this);
    freqDialog = new FreqDialog(this);

    int glo = MAXPRNGLO, gal = MAXPRNGAL, qzs = MAXPRNQZS, cmp = MAXPRNCMP, irn=MAXPRNIRN;;
    if (glo <= 0) cBNavSys2->setEnabled(false);
    if (gal <= 0) cBNavSys3->setEnabled(false);
    if (qzs <= 0) cBNavSys4->setEnabled(false);
    if (cmp <= 0) cBNavSys6->setEnabled(false);
    if (irn <= 0) cBNavSys7->setEnabled(false);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ConvOptDialog::saveClose);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ConvOptDialog::reject);
    connect(btnMask, &QPushButton::clicked, this, &ConvOptDialog::showMaskDialog);
    connect(cBAutoPosition, &QCheckBox::clicked, this, &ConvOptDialog::updateEnable);
    connect(cBRinexFilename, &QCheckBox::clicked, this, &ConvOptDialog::updateEnable);
    connect(cBRinexVersion, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ConvOptDialog::updateEnable);
    connect(btnFcn, &QPushButton::clicked, this, &ConvOptDialog::showFcnDialog);
    connect(btnFrequencies, &QPushButton::clicked, this, &ConvOptDialog::showFrequencyDialog);
}
//---------------------------------------------------------------------------
void ConvOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    cBRinexVersion->setCurrentIndex(mainWindow->rinexVersion);
    cBRinexFilename->setChecked(mainWindow->rinexFile);
    lERinexStationCode->setText(mainWindow->rinexStationCode);
    lERunBy->setText(mainWindow->runBy);
    lEMarker->setText(mainWindow->marker);
    lEMarkerNo->setText(mainWindow->markerNo);
    lEMarkerType->setText(mainWindow->markerType);
    lEObserver->setText(mainWindow->name[0]);
    lEAgency->setText(mainWindow->name[1]);
    lEReceiver->setText(mainWindow->receiver[0]);
    lEReceiverType->setText(mainWindow->receiver[1]);
    lEReceiverVersion->setText(mainWindow->receiver[2]);
    lEAntenna->setText(mainWindow->antenna[0]);
    lEAntennaType->setText(mainWindow->antenna[1]);
    sBApproxPosition0->setValue(mainWindow->approxPosition[0]);
    sBApproxPosition1->setValue(mainWindow->approxPosition[1]);
    sBApproxPosition2->setValue(mainWindow->approxPosition[2]);
    sBAntennaDelta0->setValue(mainWindow->antennaDelta[0]);
    sBAntennaDelta1->setValue(mainWindow->antennaDelta[1]);
    sBAntennaDelta2->setValue(mainWindow->antennaDelta[2]);
    lEComment0->setText(mainWindow->comment[0]);
    lEComment1->setText(mainWindow->comment[1]);
    lEReceiverOptions->setText(mainWindow->receiverOptions);

    for (int i = 0; i < 7; i++) codeMask[i] = mainWindow->codeMask[i];

    cBAutoPosition->setChecked(mainWindow->autoPosition);
    cBPhaseShift->setChecked(mainWindow->phaseShift);
    cBHalfCycle->setChecked(mainWindow->halfCycle);
    cBOutputIonoCorr->setChecked(mainWindow->outputIonoCorr);
    cBOutputTimeCorr->setChecked(mainWindow->outputTimeCorr);
    cBOutputLeapSecs->setChecked(mainWindow->outputLeapSeconds);
    gloFcnDialog->setGloFcnEnable(mainWindow->enableGlonassFrequency);
    for (int i = 0; i < 27; i++) {
        gloFcnDialog->setGloFcn(i, mainWindow->glonassFrequency[i]);
    }

    cBNavSys1->setChecked(mainWindow->navSys & SYS_GPS);
    cBNavSys2->setChecked(mainWindow->navSys & SYS_GLO);
    cBNavSys3->setChecked(mainWindow->navSys & SYS_GAL);
    cBNavSys4->setChecked(mainWindow->navSys & SYS_QZS);
    cBNavSys5->setChecked(mainWindow->navSys & SYS_SBS);
    cBNavSys6->setChecked(mainWindow->navSys & SYS_CMP);
    cBNavSys7->setChecked(mainWindow->navSys & SYS_IRN);
    cBObservationTypeC->setChecked(mainWindow->observationType & OBSTYPE_PR);
    cBObservationTypeL->setChecked(mainWindow->observationType & OBSTYPE_CP);
    cBObservationTypeD->setChecked(mainWindow->observationType & OBSTYPE_DOP);
    cBObservationTypeS->setChecked(mainWindow->observationType & OBSTYPE_SNR);
    cBFreq1->setChecked(mainWindow->frequencyType & FREQTYPE_L1);
    cBFreq2->setChecked(mainWindow->frequencyType & FREQTYPE_L2);
    cBFreq3->setChecked(mainWindow->frequencyType & FREQTYPE_L3);
    cBFreq4->setChecked(mainWindow->frequencyType & FREQTYPE_L4);
    cBFreq5->setChecked(mainWindow->frequencyType & FREQTYPE_L5);

    lEExcludedSatellites->setText(mainWindow->excludedSatellites);
    cBTraceLevel->setCurrentIndex(mainWindow->traceLevel);
    cBSeperateNavigation->setChecked(mainWindow->separateNavigation);
    sBTimeTolerance->setValue(mainWindow->timeTolerance);

    updateEnable();
}
//---------------------------------------------------------------------------
void ConvOptDialog::saveClose()
{
    mainWindow->rinexVersion = cBRinexVersion->currentIndex();
    mainWindow->rinexFile = cBRinexFilename->isChecked();
    mainWindow->rinexStationCode = lERinexStationCode->text();
    mainWindow->runBy = lERunBy->text();
    mainWindow->marker = lEMarker->text();
    mainWindow->markerNo = lEMarkerNo->text();
    mainWindow->markerType = lEMarkerType->text();
    mainWindow->name[0] = lEObserver->text();
    mainWindow->name[1] = lEAgency->text();
    mainWindow->receiver[0] = lEReceiver->text();
    mainWindow->receiver[1] = lEReceiverType->text();
    mainWindow->receiver[2] = lEReceiverVersion->text();
    mainWindow->antenna[0] = lEAntenna->text();
    mainWindow->antenna[1] = lEAntennaType->text();
    mainWindow->approxPosition[0] = sBApproxPosition0->value();
    mainWindow->approxPosition[1] = sBApproxPosition1->value();
    mainWindow->approxPosition[2] = sBApproxPosition2->value();
    mainWindow->antennaDelta[0] = sBAntennaDelta0->value();
    mainWindow->antennaDelta[1] = sBAntennaDelta1->value();
    mainWindow->antennaDelta[2] = sBAntennaDelta2->value();
    mainWindow->comment[0] = lEComment0->text();
    mainWindow->comment[1] = lEComment1->text();
    mainWindow->receiverOptions = lEReceiverOptions->text();

    for (int i = 0; i < 7; i++) mainWindow->codeMask[i] = codeMask[i];

    mainWindow->autoPosition = cBAutoPosition->isChecked();
    mainWindow->phaseShift = cBPhaseShift->isChecked();
    mainWindow->halfCycle = cBHalfCycle->isChecked();
    mainWindow->outputIonoCorr = cBOutputIonoCorr->isChecked();
    mainWindow->outputTimeCorr = cBOutputTimeCorr->isChecked();
    mainWindow->outputLeapSeconds = cBOutputLeapSecs->isChecked();
    mainWindow->enableGlonassFrequency = gloFcnDialog->getGloFcnEnable();
    for (int i = 0; i < 27; i++) {
        mainWindow->glonassFrequency[i] = gloFcnDialog->getGloFcn(i);
    }

    int navsys = 0, obstype = 0, freqtype = 0;

    if (cBNavSys1->isChecked()) navsys |= SYS_GPS;
    if (cBNavSys2->isChecked()) navsys |= SYS_GLO;
    if (cBNavSys3->isChecked()) navsys |= SYS_GAL;
    if (cBNavSys4->isChecked()) navsys |= SYS_QZS;
    if (cBNavSys5->isChecked()) navsys |= SYS_SBS;
    if (cBNavSys6->isChecked()) navsys |= SYS_CMP;
    if (cBNavSys7->isChecked()) navsys |= SYS_IRN;

    if (cBObservationTypeC->isChecked()) obstype |= OBSTYPE_PR;
    if (cBObservationTypeL->isChecked()) obstype |= OBSTYPE_CP;
    if (cBObservationTypeD->isChecked()) obstype |= OBSTYPE_DOP;
    if (cBObservationTypeS->isChecked()) obstype |= OBSTYPE_SNR;

    if (cBFreq1->isChecked()) freqtype |= FREQTYPE_L1;
    if (cBFreq2->isChecked()) freqtype |= FREQTYPE_L2;
    if (cBFreq3->isChecked()) freqtype |= FREQTYPE_L3;
    if (cBFreq4->isChecked()) freqtype |= FREQTYPE_L4;
    if (cBFreq5->isChecked()) freqtype |= FREQTYPE_L5;

    mainWindow->navSys = navsys;
    mainWindow->observationType = obstype;
    mainWindow->frequencyType = freqtype;
    mainWindow->excludedSatellites = lEExcludedSatellites->text();
    mainWindow->traceLevel = cBTraceLevel->currentIndex();
    mainWindow->separateNavigation = cBSeperateNavigation->isChecked();
    mainWindow->timeTolerance = sBTimeTolerance->value();

    accept();
}
//---------------------------------------------------------------------------
void ConvOptDialog::showFrequencyDialog()
{
    freqDialog->exec();
}
//---------------------------------------------------------------------------
void ConvOptDialog::showMaskDialog()
{
    int navSystem = 0;
    int frequencyType = 0;
    
    if (cBNavSys1->isChecked()) navSystem |= SYS_GPS;
    if (cBNavSys2->isChecked()) navSystem |= SYS_GLO;
    if (cBNavSys3->isChecked()) navSystem |= SYS_GAL;
    if (cBNavSys4->isChecked()) navSystem |= SYS_QZS;
    if (cBNavSys5->isChecked()) navSystem |= SYS_SBS;
    if (cBNavSys6->isChecked()) navSystem |= SYS_CMP;
    if (cBNavSys7->isChecked()) navSystem |= SYS_IRN;
    
    codeOptDialog->setNavSystem(navSystem);

    if (cBFreq1->isChecked()) frequencyType |= FREQTYPE_L1;
    if (cBFreq2->isChecked()) frequencyType |= FREQTYPE_L2;
    if (cBFreq3->isChecked()) frequencyType |= FREQTYPE_L3;
    if (cBFreq4->isChecked()) frequencyType |= FREQTYPE_L4;
    if (cBFreq5->isChecked()) frequencyType |= FREQTYPE_L5;

    codeOptDialog->setFrequencyType(frequencyType);

    for (int i = 0; i < 7; i++) codeOptDialog->setCodeMask(i, codeMask[i]);

    codeOptDialog->show();

    for (int i = 0; i < 7; i++) codeMask[i] = codeOptDialog->getCodeMask(i);
}
//---------------------------------------------------------------------------
void ConvOptDialog::updateEnable(void)
{
    sBApproxPosition0->setEnabled(cBAutoPosition->isChecked());
    sBApproxPosition1->setEnabled(cBAutoPosition->isChecked());
    sBApproxPosition2->setEnabled(cBAutoPosition->isChecked());
    cBSeperateNavigation->setEnabled(cBRinexVersion->currentIndex() >= 3);
    lbTimeTolerance->setEnabled(mainWindow->cBTimeInterval->isChecked());
    sBTimeTolerance->setEnabled(mainWindow->cBTimeInterval->isChecked());
    cBNavSys3->setEnabled(cBRinexVersion->currentIndex() >= 1);
    cBNavSys4->setEnabled(cBRinexVersion->currentIndex() >= 5);
    cBNavSys6->setEnabled(cBRinexVersion->currentIndex() == 2 || cBRinexVersion->currentIndex() >= 4);
    cBNavSys7->setEnabled(cBRinexVersion->currentIndex() >= 6);
    cBFreq3->setEnabled(cBRinexVersion->currentIndex() >=1 );
    cBFreq4->setEnabled(cBRinexVersion->currentIndex() >=1 );
    cBFreq5->setEnabled(cBRinexVersion->currentIndex() >=1 );
    cBPhaseShift->setEnabled(cBRinexVersion->currentIndex() >=4 );
}
//---------------------------------------------------------------------------
void ConvOptDialog::showFcnDialog()
{
    gloFcnDialog->exec();
}
//---------------------------------------------------------------------------
