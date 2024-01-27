//---------------------------------------------------------------------------
#include <string.h>

#include "postmain.h"
#include "kmzconv.h"
#include "rtklib.h"
#include "viewer.h"

#include <QShowEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QProcess>
#include <QtGlobal>
#include <QFileSystemModel>
#include <QCompleter>

#ifdef Q_OS_WIN
#define GOOGLE_EARTH "C:\\Program Files\\Google\\Google Earth Pro\\client\\googleearth.exe"
#endif
#ifdef Q_OS_LINUX
#define GOOGLE_EARTH "google-earth"
#endif
#ifndef GOOGLE_EARTH
#define GOOGLE_EARTH ""
#endif

//---------------------------------------------------------------------------
ConvDialog::ConvDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    viewer = new TextViewer(this);

    QCompleter *fileCompleter = new QCompleter(this);
    QFileSystemModel *fileModel = new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    lEInputFile->setCompleter(fileCompleter);
    lEOutputFile->setCompleter(fileCompleter);
    lEGoogleEarthFile->setCompleter(fileCompleter);

    // Google Earth file line edit actions
    QAction *acGoogleEarthFileSelect = lEGoogleEarthFile->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    acGoogleEarthFileSelect->setToolTip(tr("Select Google Earth Executable"));

    // input file line edit actions
    QAction *acInputFileSelect = lEInputFile->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    acInputFileSelect->setToolTip(tr("Select File"));

    connect(btnClose, &QPushButton::clicked, this, &ConvDialog::accept);
    connect(btnConvert, &QPushButton::clicked, this, &ConvDialog::convert);
    connect(acGoogleEarthFileSelect, &QAction::triggered, this, &ConvDialog::selectGoogleEarthFile);
    connect(btnView, &QPushButton::clicked, this, &ConvDialog::viewOutputFile);
    connect(btnGoogleEarth, &QPushButton::clicked, this, &ConvDialog::callGoogleEarth);
    connect(cBAddOffset, &QCheckBox::clicked, this, &ConvDialog::updateEnable);
    connect(cBTimeSpan, &QCheckBox::clicked, this, &ConvDialog::updateEnable);
    connect(sBTimeInterval, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &ConvDialog::updateEnable);
    connect(lEInputFile, &QLineEdit::editingFinished, this, &ConvDialog::updateEnable);
    connect(acInputFileSelect, &QAction::triggered, this, &ConvDialog::selectInputFile);
    connect(cBCompress, &QCheckBox::clicked, this, &ConvDialog::updateOutputFile);
    connect(rBFormatKML, &QRadioButton::clicked, this, &ConvDialog::formatChanged);
    connect(rBFormatGPX, &QRadioButton::clicked, this, &ConvDialog::formatChanged);

    rBFormatGPX->setChecked(!rBFormatKML->isChecked());

    updateEnable();
}
//---------------------------------------------------------------------------
void ConvDialog::setInput(const QString &filename)
{
    lEInputFile->setText(filename);
	updateOutputFile();
}

//---------------------------------------------------------------------------
void ConvDialog::selectInputFile()
{
    lEInputFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open..."), lEInputFile->text(), tr("All (*.*)"))));
}
//---------------------------------------------------------------------------
void ConvDialog::convert()
{
    QString OutputFilename = lEOutputFile->text();
	int stat;
    QString cmd;
    QStringList opt;
    char file[1024], kmlfile[1024], *p;
    double offset[3] = { 0 }, tint = 0.0;
    gtime_t ts = { 0, 0 }, te = { 0, 0 };

	showMessage("");

    if (lEInputFile->text().isEmpty() || lEOutputFile->text().isEmpty()) return;

    showMessage(tr("Converting ..."));

    if (cBTimeSpan->isChecked()) {
        QDateTime start(dateTimeStart->dateTime());
        QDateTime end(dateTimeStop->dateTime());
        ts.time = start.toSecsSinceEpoch(); ts.sec = start.time().msec() / 1000;
        te.time = end.toSecsSinceEpoch(); te.sec = end.time().msec() / 1000;
  }

    if (cBAddOffset->isChecked()) {
        offset[0] = sBOffset1->value();
        offset[1] = sBOffset2->value();
        offset[2] = sBOffset3->value();
	}

    if (cBTimeInterval->isChecked())
        tint = sBTimeInterval->value();

    strncpy(file, qPrintable(lEInputFile->text()), 1023);

    if (rBFormatKML->isChecked()) {
        if (cBCompress->isChecked()) { // generate .kml file and compress it afterwards
            strncpy(kmlfile, file, 1020);
            if (!(p = strrchr(kmlfile, '.'))) p = kmlfile + strlen(kmlfile);
            strncpy(p, ".kml", 5);
        }
        stat = convkml(file, cBCompress->isChecked() ? kmlfile : qPrintable(OutputFilename),
                   ts, te, tint, cBQFlags->currentIndex(), offset,
                   cBTrackColor->currentIndex(), cBPointColor->currentIndex(),
                   cBOutputAltitude->currentIndex(), cBOutputTime->currentIndex());
    } else {
        stat = convgpx(file, cBCompress->isChecked() ? kmlfile : qPrintable(OutputFilename),
                   ts, te, tint, cBQFlags->currentIndex(), offset,
                   cBTrackColor->currentIndex(), cBPointColor->currentIndex(),
                   cBOutputAltitude->currentIndex(), cBOutputTime->currentIndex());
    }

    if (stat < 0) {
        if (stat == -1) showMessage(tr("Error : Reading input file"));
        else if (stat == -2) showMessage(tr("Error : Input file format"));
        else if (stat == -3) showMessage(tr("Error : No data in input file"));
        else showMessage(tr("Error : Writing kml file"));
		return;
	}
    if (rBFormatKML->isChecked() && cBCompress->isChecked()) {
#ifdef Q_OS_WIN
        cmd = "zip.exe";
        opt << "-j"
        opt << QString("-m %1").arg(lEOutputFile->text()); //TODO: zip for other platforms
#endif
#ifdef Q_OS_LINUX
        cmd = "gzip";
        opt << QString("-3 %1 %2").arg(lEOutputFile->text());
#endif
        opt << kmlfile;
        if (!execCommand(cmd, opt)) {
            showMessage(tr("Error : zip execution"));
			return;
		}
	}
	showMessage("done");
}

//---------------------------------------------------------------------------
void ConvDialog::updateEnable()
{
    sBOffset1->setEnabled(cBAddOffset->isChecked());
    sBOffset2->setEnabled(cBAddOffset->isChecked());
    sBOffset3->setEnabled(cBAddOffset->isChecked());
    dateTimeStart->setEnabled(cBTimeSpan->isChecked());
    dateTimeStop->setEnabled(cBTimeSpan->isChecked());
    sBTimeInterval->setEnabled(cBTimeInterval->isChecked());

    lEGoogleEarthFile->setEnabled(rBFormatKML->isChecked());
    btnGoogleEarth->setEnabled(rBFormatKML->isChecked());

    if (rBFormatKML->isChecked()) {
        cBCompress->setEnabled(false);
#ifdef Q_OS_WIN
        cBCompress->setEnabled(true);
#endif
#ifdef Q_OS_LINUX
        cBCompress->setEnabled(true);
#endif
    } else
        cBCompress->setEnabled(false);
}
//---------------------------------------------------------------------------
int ConvDialog::execCommand(const QString &cmd, const QStringList &opt)
{
    return QProcess::startDetached(cmd, opt);
}
//---------------------------------------------------------------------------
void ConvDialog::showMessage(const QString &msg)
{
    lblMessage->setText(msg);
    if (msg.contains("error", Qt::CaseInsensitive)) lblMessage->setStyleSheet("QLabel {color : red}");
    else lblMessage->setStyleSheet("QLabel {color : blue}");
}
//---------------------------------------------------------------------------
void ConvDialog::updateOutputFile()
{
    QString inputFilename = lEInputFile->text();

    if (inputFilename.isEmpty()) return;

    QFileInfo fi(inputFilename);
    if (fi.suffix().isNull()) return;

    QString filename = fi.baseName() + (rBFormatGPX->isChecked() ? ".gpx" : (cBCompress->isChecked() ? ".kmz" : ".kml"));

    lEOutputFile->setText( fi.absoluteDir().absoluteFilePath(filename));
}
//---------------------------------------------------------------------------
void ConvDialog::selectGoogleEarthFile()
{
    lEGoogleEarthFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Google Earth Exe File"), lEGoogleEarthFile->text())));
}
//---------------------------------------------------------------------------
void ConvDialog::callGoogleEarth()
{
    QString cmd;
    QStringList opt;

    cmd = lEGoogleEarthFile->text();
    opt << lEOutputFile->text();
    
    if (!execCommand(cmd, opt))
        showMessage(tr("Error : Google Earth execution"));

    return;
}
void ConvDialog::formatChanged()
{
    updateOutputFile();
    updateEnable();
}
//---------------------------------------------------------------------------
void ConvDialog::viewOutputFile()
{
    QString file = lEOutputFile->text();

    if (file.isEmpty()) return;

    viewer->setWindowTitle(file);
    viewer->show();
    viewer->read(file);
}
//---------------------------------------------------------------------------
void ConvDialog::loadOptions(QSettings &ini)
{
    cBTimeSpan->setChecked(ini.value("conv/timespan", 0).toInt());
    cBTimeInterval->setChecked(ini.value("conv/timeintf", 0).toInt());
    dateTimeStart->setDate(ini.value("conv/timey1", QDate(2000, 1, 1)).toDate());
    dateTimeStart->setTime(ini.value("conv/timeh1", QTime(0, 0, 0)).toTime());
    dateTimeStop->setDate(ini.value("conv/timey2", QDate(2000, 1, 1)).toDate());
    dateTimeStop->setTime(ini.value("conv/timeh2", QTime(0, 0, 0)).toTime());
    sBTimeInterval->setValue(ini.value ("conv/timeint", 0.0).toDouble());
    cBTrackColor->setCurrentIndex(ini.value("conv/trackcolor", 5).toInt());
    cBPointColor->setCurrentIndex(ini.value("conv/pointcolor", 5).toInt());
    cBOutputAltitude->setCurrentIndex(ini.value("conv/outputalt", 0).toInt());
    cBOutputTime->setCurrentIndex(ini.value("conv/outputtime",0).toInt());
    cBAddOffset->setChecked(ini.value("conv/addoffset", 0).toInt());
    sBOffset1->setValue(ini.value("conv/offset1", 0).toDouble());
    sBOffset2->setValue(ini.value("conv/offset2", 0).toDouble());
    sBOffset3->setValue(ini.value("conv/offset3", 0).toDouble());
    cBCompress->setChecked(ini.value("conv/compress", 0).toInt());
    rBFormatKML->setChecked(ini.value("conv/format", 0).toInt());

    lEGoogleEarthFile->setText(ini.value("opt/googleearthfile", GOOGLE_EARTH).toString());
}
//---------------------------------------------------------------------------
void ConvDialog::saveOptions(QSettings &ini)
{
    ini.setValue ("conv/timespan", cBTimeSpan->isChecked());
    ini.setValue ("conv/timey1", dateTimeStart->date());
    ini.setValue ("conv/timeh1", dateTimeStart->time());
    ini.setValue ("conv/timey2", dateTimeStop->date());
    ini.setValue ("conv/timeh2", dateTimeStop->time());
    ini.setValue ("conv/timeintf", cBTimeInterval->isChecked());
    ini.setValue ("conv/timeint", sBTimeInterval->text());
    ini.setValue ("conv/trackcolor", cBTrackColor->currentIndex());
    ini.setValue ("conv/pointcolor", cBPointColor->currentIndex());
    ini.setValue ("conv/outputalt", cBOutputAltitude->currentIndex());
    ini.setValue ("conv/outputtime", cBOutputTime->currentIndex());
    ini.setValue ("conv/addoffset", cBAddOffset ->isChecked());
    ini.setValue ("conv/offset1", sBOffset1->text());
    ini.setValue ("conv/offset2", sBOffset2->text());
    ini.setValue ("conv/offset3", sBOffset3->text());
    ini.setValue ("conv/compress", cBCompress->isChecked());
    ini.setValue ("conv/format", rBFormatKML->isChecked());

    ini.setValue("opt/googleearthfile", lEGoogleEarthFile->text());
}
//---------------------------------------------------------------------------
