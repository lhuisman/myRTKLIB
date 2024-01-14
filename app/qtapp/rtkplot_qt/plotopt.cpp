//---------------------------------------------------------------------------

#include "plotmain.h"
#include "plotopt.h"
#include "refdlg.h"
#include "viewer.h"
#include "rtklib.h"
#include "helper.h"

#include <QShowEvent>
#include <QColorDialog>
#include <QFontDialog>
#include <QFileDialog>
#include <QPoint>
#include <QDebug>
#include <QFileSystemModel>
#include <QCompleter>

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
PlotOptDialog::PlotOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    refDialog = new RefDialog(this, 1);

    QCompleter *fileCompleter = new QCompleter(this);
    QFileSystemModel *fileModel = new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    lETLEFile->setCompleter(fileCompleter);
    lETLESatelliteFile->setCompleter(fileCompleter);
    lEShapeFile->setCompleter(fileCompleter);

    QAction * acTLEFileOpen = lETLEFile->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    acTLEFileOpen->setToolTip(tr("Select TLE file"));
    QAction * acTLEFileView = lETLEFile->addAction(QIcon(":/buttons/doc"), QLineEdit::TrailingPosition);
    acTLEFileView->setToolTip(tr("View TLE file"));

    QAction * acTLESatelliteFileOpen = lETLESatelliteFile->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    acTLESatelliteFileOpen->setToolTip(tr("Select TLE satellite file"));
    QAction * acTLESatelliteFileView = lETLESatelliteFile->addAction(QIcon(":/buttons/doc"), QLineEdit::TrailingPosition);
    acTLESatelliteFileView->setToolTip(tr("View TLE satellite file"));

    QAction * acShapeFileOpen = lEShapeFile->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    acShapeFileOpen->setToolTip(tr("Select shape file"));

    connect(btnCancel, &QPushButton::clicked, this, &PlotOptDialog::reject);
    connect(btnColor1, &QPushButton::clicked, this, &PlotOptDialog::color1Select);
    connect(btnColor2, &QPushButton::clicked, this, &PlotOptDialog::color2Select);
    connect(btnColor3, &QPushButton::clicked, this, &PlotOptDialog::color3Select);
    connect(btnColor4, &QPushButton::clicked, this, &PlotOptDialog::color4Select);
    connect(btnFont, &QPushButton::clicked, this, &PlotOptDialog::fontSelect);
    connect(btnOK, &QPushButton::clicked, this, &PlotOptDialog::btnOKClicked);
    connect(btnReferencePosition, &QPushButton::clicked, this, &PlotOptDialog::referencePositionSelect);
    connect(acTLEFileOpen, &QAction::triggered, this, &PlotOptDialog::tleFileOpen);
    connect(acTLEFileView, &QAction::triggered, this, &PlotOptDialog::tleFileView);
    connect(acTLESatelliteFileOpen, &QAction::triggered, this, &PlotOptDialog::tleSatelliteFileOpen);
    connect(acTLESatelliteFileView, &QAction::triggered, this, &PlotOptDialog::tleSatelliteFileView);
    connect(acShapeFileOpen, &QAction::triggered, this, &PlotOptDialog::shapeFileOpen);
    connect(cBOrigin, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PlotOptDialog::updateEnable);
    connect(cBAutoScale, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PlotOptDialog::updateEnable);
    connect(cBReceiverPosition, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PlotOptDialog::updateEnable);
    connect(tBMColor1, &QToolButton::clicked, this, &PlotOptDialog::markerColorSelect);
    connect(tBMColor2, &QToolButton::clicked, this, &PlotOptDialog::markerColorSelect);
    connect(tBMColor3, &QToolButton::clicked, this, &PlotOptDialog::markerColorSelect);
    connect(tBMColor4, &QToolButton::clicked, this, &PlotOptDialog::markerColorSelect);
    connect(tBMColor5, &QToolButton::clicked, this, &PlotOptDialog::markerColorSelect);
    connect(tBMColor6, &QToolButton::clicked, this, &PlotOptDialog::markerColorSelect);
    connect(tBMColor7, &QToolButton::clicked, this, &PlotOptDialog::markerColorSelect);
    connect(tBMColor8, &QToolButton::clicked, this, &PlotOptDialog::markerColorSelect);
    connect(tBMColor9, &QToolButton::clicked, this, &PlotOptDialog::markerColorSelect);
    connect(tBMColor10, &QToolButton::clicked, this, &PlotOptDialog::markerColorSelect);
    connect(tBMColor11, &QToolButton::clicked, this, &PlotOptDialog::markerColorSelect);
    connect(tBMColor12, &QToolButton::clicked, this, &PlotOptDialog::markerColorSelect);
    connect(cBTimeSync, &QCheckBox::clicked, this, &PlotOptDialog::updateEnable);
}
//---------------------------------------------------------------------------
void PlotOptDialog::showEvent(QShowEvent *event)
{
    int marks[] = { 1, 2, 3, 4, 5, 10, 15, 20 };

    if (event->spontaneous()) return;

    cBTimeFormat->setCurrentIndex(plot->timeFormat);
    cBLatLonFormat->setCurrentIndex(plot->latLonFormat);
    cBAutoScale->setCurrentIndex(plot->autoScale);
    cBShowStats->setCurrentIndex(plot->showStats);
    cBShowArrow->setCurrentIndex(plot->showArrow);
    cBShowSlip->setCurrentIndex(plot->showSlip);
    cBShowHalfC->setCurrentIndex(plot->showHalfC);
    cBShowError->setCurrentIndex(plot->showError);
    cBShowEphemeris->setCurrentIndex(plot->showEphemeris);
    cBShowLabel->setCurrentIndex(plot->showLabel);
    cBShowGridLabel->setCurrentIndex(plot->showGridLabel);
    cBShowScale->setCurrentIndex(plot->showScale);
    cBShowCompass->setCurrentIndex(plot->showCompass);
    cBPlotStyle->setCurrentIndex(plot->plotStyle);

    for (int i = 0; i < 8; i++)
        if (marks[i] == plot->markSize) cBMarkSize->setCurrentIndex(i);

    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 2; j++)
            markerColor[j][i] = plot->markerColor[j][i];

    for (int i = 0; i < 4; i++)
        cColor[i] = plot->cColor[i];

    tBMColor1->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->markerColor[0][1])));
    tBMColor2->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->markerColor[0][2])));
    tBMColor3->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->markerColor[0][3])));
    tBMColor4->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->markerColor[0][4])));
    tBMColor5->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->markerColor[0][5])));
    tBMColor6->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->markerColor[0][6])));
    tBMColor7->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->markerColor[1][1])));
    tBMColor8->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->markerColor[1][2])));
    tBMColor9->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->markerColor[1][3])));
    tBMColor10->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->markerColor[1][4])));
    tBMColor11->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->markerColor[1][5])));
    tBMColor12->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->markerColor[1][6])));
    lblColor1->setStyleSheet(QString("QLabel {background-color: %1;}").arg(color2String(plot->cColor[0])));
    lblColor2->setStyleSheet(QString("QLabel {background-color: %1;}").arg(color2String(plot->cColor[1])));
    lblColor3->setStyleSheet(QString("QLabel {background-color: %1;}").arg(color2String(plot->cColor[2])));
    lblColor4->setStyleSheet(QString("QLabel {background-color: %1;}").arg(color2String(plot->cColor[3])));

    fontOption.setFamily(plot->fontName);
    fontOption.setPixelSize(plot->fontSize);
    updateFont();

    cBElevationMask->setCurrentText(QString::number(plot->elevationMask));
    cBMaxDop->setCurrentText(QString::number(plot->maxDop));
    cBMaxMP->setCurrentText(QString::number(plot->maxMP));
    cBOrigin->setCurrentIndex(plot->origin);
    cBReceiverPosition->setCurrentIndex(plot->receiverPosition);
    sBReferencePosition1->setValue(plot->ooPosition[0] * R2D);
    sBReferencePosition2->setValue(plot->ooPosition[1] * R2D);
    sBReferencePosition3->setValue(plot->ooPosition[2]);
    cBNavSys1->setChecked(plot->navSys & SYS_GPS);
    cBNavSys2->setChecked(plot->navSys & SYS_GLO);
    cBNavSys3->setChecked(plot->navSys & SYS_GAL);
    cBNavSys4->setChecked(plot->navSys & SYS_QZS);
    cBNavSys5->setChecked(plot->navSys & SYS_SBS);
    cBNavSys6->setChecked(plot->navSys & SYS_CMP);
    cBNavSys7->setChecked(plot->navSys & SYS_IRN);
    cBAnimationCycle->setCurrentText(QString::number(plot->animationCycle));
    sBRefreshCycle->setValue(plot->refreshCycle);
    cBHideLowSatellites->setCurrentIndex(plot->hideLowSatellites);
    cBElevationMask->setCurrentText(QString::number(plot->elevationMask));
    lEExcludedSatellites->setText(plot->excludedSatellites);
    sBBufferSize->setValue(plot->rtBufferSize);
    cBTimeSync->setChecked(plot->timeSyncOut);
    sBTimeSync->setValue(plot->timeSyncPort);
    lERinexOptions->setText(plot->rinexOptions);
    lEShapeFile->setText(plot->shapeFile);
    lETLEFile->setText(plot->tleFile);
    lETLESatelliteFile->setText(plot->tleSatelliteFile);

    for (int i = 0; i < cBYRange->count(); i++) {
            double range;
            bool ok;
            QString unit;

            QString s = cBYRange->itemText(i);
            QStringList tokens = s.split(' ');

            if (tokens.length() != 2) continue;

            range = tokens.at(0).toInt(&ok);
            if (!ok) continue;

            unit = tokens.at(1);
            if (unit == "cm") range *= 0.01;
            else if (unit == "km") range *= 1000.0;

            if (range == plot->yRange) {
                cBYRange->setCurrentIndex(i);
                break;
            }
        }

    updateEnable();
}
//---------------------------------------------------------------------------
void PlotOptDialog::btnOKClicked()
{
    int marks[] = { 1, 2, 3, 4, 5, 10, 15, 20 };

    plot->timeFormat = cBTimeFormat->currentIndex();
    plot->latLonFormat = cBLatLonFormat->currentIndex();
    plot->autoScale = cBAutoScale->currentIndex();
    plot->showStats = cBShowStats->currentIndex();
    plot->showArrow = cBShowArrow->currentIndex();
    plot->showSlip = cBShowSlip->currentIndex();
    plot->showHalfC = cBShowHalfC->currentIndex();
    plot->showError = cBShowError->currentIndex();
    plot->showEphemeris = cBShowEphemeris->currentIndex();
    plot->showLabel = cBShowLabel->currentIndex();
    plot->showGridLabel = cBShowGridLabel->currentIndex();
    plot->showScale = cBShowScale->currentIndex();
    plot->showCompass = cBShowCompass->currentIndex();
    plot->plotStyle = cBPlotStyle->currentIndex();
    plot->markSize = marks[cBMarkSize->currentIndex()];

    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 2; j++)
            plot->markerColor[j][i] = markerColor[j][i];

    for (int i = 0; i < 4; i++)
        plot->cColor[i] = cColor[i];

    plot->fontName=fontOption.family();
    plot->fontSize=fontOption.pixelSize();
    plot->font = fontOption;

    plot->elevationMask = cBElevationMask->currentText().toDouble();
    plot->maxDop = cBMaxDop->currentText().toDouble();
    plot->maxMP = cBMaxMP->currentText().toDouble();
    plot->yRange = cBYRange->currentText().toDouble();
    plot->origin = cBOrigin->currentIndex();
    plot->receiverPosition = cBReceiverPosition->currentIndex();
    plot->ooPosition[0] = sBReferencePosition1->value() * D2R;
    plot->ooPosition[1] = sBReferencePosition2->value() * D2R;
    plot->ooPosition[2] = sBReferencePosition3->value();
    plot->navSys = (cBNavSys1->isChecked() ? SYS_GPS : 0) |
               (cBNavSys2->isChecked() ? SYS_GLO : 0) |
               (cBNavSys3->isChecked() ? SYS_GAL : 0) |
               (cBNavSys4->isChecked() ? SYS_QZS : 0) |
               (cBNavSys5->isChecked() ? SYS_SBS : 0) |
               (cBNavSys6->isChecked() ? SYS_CMP : 0) |
               (cBNavSys7->isChecked() ? SYS_IRN : 0);
    plot->animationCycle = cBAnimationCycle->currentText().toInt();
    plot->refreshCycle = sBRefreshCycle->value();
    plot->hideLowSatellites = cBHideLowSatellites->currentIndex();
    plot->elevationMaskEnabled = cBElevationMask->currentIndex();
    plot->rtBufferSize = sBBufferSize->value();
    plot->timeSyncOut = cBTimeSync->isChecked();
    plot->timeSyncPort = sBTimeSync->value();
    plot->excludedSatellites = lEExcludedSatellites->text();
    plot->rinexOptions = lERinexOptions->text();
    plot->shapeFile = lEShapeFile->text();
    plot->tleFile = lETLEFile->text();
    plot->tleSatelliteFile = lETLESatelliteFile->text();

    QStringList tokens = cBYRange->currentText().split(' ');
    if (tokens.length() == 2) {
        bool ok;
        int range = tokens.at(0).toInt(&ok);
        QString unit = tokens.at(1);

        if (ok) {
            if (unit == "cm") range *= 0.01;
            else if (unit == "km") range *= 1000.0;
            plot->yRange=range;
        }
    }

    accept();
}
//---------------------------------------------------------------------------
void PlotOptDialog::markerColorSelect()
{
    QToolButton *button = dynamic_cast<QToolButton *>(QObject::sender());

    if (!button) return;

    QColorDialog dialog(this);
    QColor *current;

    if (button == tBMColor1) current = &markerColor[0][1];
    else if (button == tBMColor2) current = &markerColor[0][2];
    else if (button == tBMColor3) current = &markerColor[0][3];
    else if (button == tBMColor4) current = &markerColor[0][4];
    else if (button == tBMColor5) current = &markerColor[0][5];
    else if (button == tBMColor6) current = &markerColor[0][6];
    else if (button == tBMColor7) current = &markerColor[1][1];
    else if (button == tBMColor8) current = &markerColor[1][2];
    else if (button == tBMColor9) current = &markerColor[1][3];
    else if (button == tBMColor10) current = &markerColor[1][4];
    else if (button == tBMColor11) current = &markerColor[1][5];
    else if (button == tBMColor12) current = &markerColor[1][6];
    else return;
    dialog.setCurrentColor(*current);

    dialog.exec();
    if (dialog.result() != QDialog::Accepted) return;

    button->setStyleSheet(QString("background-color: %1").arg(color2String(dialog.currentColor())));
    *current = dialog.currentColor();
}
//---------------------------------------------------------------------------
void PlotOptDialog::color1Select()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(cColor[0]);
    dialog.exec();
    if (dialog.result() != QDialog::Accepted) return;
    lblColor1->setStyleSheet(QString("QLabel {background-color: %1;}").arg(color2String(dialog.currentColor())));
    cColor[0] = dialog.currentColor();
}
//---------------------------------------------------------------------------
void PlotOptDialog::color2Select()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(cColor[1]);
    dialog.exec();
    if (dialog.result() != QDialog::Accepted) return;
    lblColor2->setStyleSheet(QString("QLabel {background-color: %1;}").arg(color2String(dialog.currentColor())));
    cColor[1] = dialog.currentColor();
}
//---------------------------------------------------------------------------
void PlotOptDialog::color3Select()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(cColor[2]);
    dialog.exec();
    if (dialog.result() != QDialog::Accepted) return;
    lblColor3->setStyleSheet(QString("QLabel {background-color: %1;}").arg(color2String(dialog.currentColor())));
    cColor[2] = dialog.currentColor();
}
//---------------------------------------------------------------------------
void PlotOptDialog::color4Select()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(cColor[3]);
    dialog.exec();
    if (dialog.result() != QDialog::Accepted) return;
    lblColor4->setStyleSheet(QString("QLabel {background-color: %1;}").arg(color2String(dialog.currentColor())));
    cColor[3] = dialog.currentColor();
}
//---------------------------------------------------------------------------
void PlotOptDialog::fontSelect()
{
    QFontDialog dialog(this);

    dialog.setCurrentFont(fontOption);
    dialog.exec();

    if (dialog.result() != QDialog::Accepted) return;
    fontOption = dialog.currentFont();

    updateFont();
}
//---------------------------------------------------------------------------
void PlotOptDialog::shapeFileOpen()
{
    lEShapeFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open"), lEShapeFile->text())));
}
//---------------------------------------------------------------------------
void PlotOptDialog::tleFileOpen()
{
    lETLEFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open"), lETLEFile->text(), tr("Text Files (*.txt);;Position Files (*.pos *.snx);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void PlotOptDialog::tleSatelliteFileOpen()
{
    lETLESatelliteFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open"), lETLESatelliteFile->text(), tr("Text Files (*.txt);;Position Files (*.pos *.snx);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void PlotOptDialog::referencePositionSelect()
{
    refDialog->setRoverPosition(sBReferencePosition1->value(), sBReferencePosition2->value(), sBReferencePosition3->value());

    refDialog->move(pos().x() + size().width() / 2 - refDialog->size().width() / 2,
            pos().y() + size().height() / 2 - refDialog->size().height() / 2);

    refDialog->exec();
    if (refDialog->result() != QDialog::Accepted) return;

    sBReferencePosition1->setValue(refDialog->getPosition()[0]);
    sBReferencePosition2->setValue(refDialog->getPosition()[1]);
    sBReferencePosition3->setValue(refDialog->getPosition()[2]);
}
//---------------------------------------------------------------------------
void PlotOptDialog::updateFont(void)
{
    lblFontName->setFont(fontOption);
    lblFontName->setText(fontOption.family() + " " + QString::number(fontOption.pointSize()) + " pt");
}
//---------------------------------------------------------------------------
void PlotOptDialog::updateEnable(void)
{
    sBReferencePosition1->setEnabled(cBOrigin->currentIndex() == 5 || cBReceiverPosition->currentIndex() == 1);
    sBReferencePosition2->setEnabled(cBOrigin->currentIndex() == 5 || cBReceiverPosition->currentIndex() == 1);
    sBReferencePosition3->setEnabled(cBOrigin->currentIndex() == 5 || cBReceiverPosition->currentIndex() == 1);
    lblReferencePosition->setEnabled(cBOrigin->currentIndex() == 5 || cBOrigin->currentIndex() == 6 || cBReceiverPosition->currentIndex() == 1);
    btnReferencePosition->setEnabled(cBOrigin->currentIndex() == 5 || cBOrigin->currentIndex() == 6 || cBReceiverPosition->currentIndex() == 1);
    sBTimeSync->setEnabled(cBTimeSync->isChecked());
}
//---------------------------------------------------------------------------
void PlotOptDialog::tleFileView()
{
    TextViewer *viewer;
    QString file = lETLEFile->text();

    if (file == "") return;
    viewer = new TextViewer(this);
    viewer->setWindowTitle(file);
    viewer->show();
    viewer->read(file);
}
//---------------------------------------------------------------------------
void PlotOptDialog::tleSatelliteFileView()
{
    TextViewer *viewer;
    QString file = lETLESatelliteFile->text();

    if (file == "") return;
    viewer = new TextViewer(this);
    viewer->setWindowTitle(file);
    viewer->show();
    viewer->read(file);
}
//---------------------------------------------------------------------------
