//---------------------------------------------------------------------------
// rtkconv : rinex converter
//
//          Copyright (C) 2007-2020 by T.TAKASU, All rights reserved.
//          ported to Qt by Jens Reimann
//
// options : rtkconv [-t title][-i file]
//
//           -t title   window title
//           -i file    ini file path
//
// version : $Revision:$ $Date:$
// history : 2008/07/14  1.0 new
//			 2010/07/18  1.1 rtklib 2.4.0
//			 2011/06/10  1.2 rtklib 2.4.1
//			 2013/04/01  1.3 rtklib 2.4.2
//			 2020/11/30  1.4 support RINEX 3.04
//			                 support NavIC/IRNSS
//                           support SBF for auto format recognition
//                           support "Phase Shift" option
//                           no support "Scan Obs" option
//---------------------------------------------------------------------------
#include <clocale>

#include <QShowEvent>
#include <QTimer>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QDoubleValidator>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QMimeData>
#include <QDebug>
#include <QCompleter>
#include <QFileSystemModel>

#include "convmain.h"
#include "timedlg.h"
#include "aboutdlg.h"
#include "startdlg.h"
#include "keydlg.h"
#include "convopt.h"
#include "viewer.h"
#include "rtklib.h"

//---------------------------------------------------------------------------

MainWindow *mainWindow;

#define PRGNAME     "RTKConv-Qt"        // program name
#define MAXHIST     20                  // max number of histories
#define TSTARTMARGIN 60.0               // time margin for file name replacement
#define TRACEFILE   "rtkconv_qt.trace"     // trace file

static int abortf = 0;

// show message in message area ---------------------------------------------
extern "C" {
    extern int showmsg(const char *format, ...)
    {
        va_list arg;
        char buff[1024];

        va_start(arg, format); vsprintf(buff, format, arg); va_end(arg);

        QMetaObject::invokeMethod(mainWindow->message, "setText", Qt::QueuedConnection, Q_ARG(QString, QString(buff)));

        return abortf;
    }
    extern void settime(gtime_t) {}
    extern void settspan(gtime_t, gtime_t) {}
}

// constructor --------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(this);
    mainWindow = this;
    rinexTime.sec = rinexTime.time = 0;

    setlocale(LC_NUMERIC, "C");
    setWindowIcon(QIcon(":/icons/rtkconv_Icon.ico"));
    setAcceptDrops(true);

    QString app_filename = QApplication::applicationFilePath();
    QFileInfo fi(app_filename);
    iniFile = fi.absolutePath() + "/" + fi.baseName() + ".ini";

    convOptDialog = new ConvOptDialog(this);
    timeDialog = new TimeDialog(this);
    keyDialog = new KeyDialog(this);
    aboutDialog = new AboutDialog(this, QPixmap(":/icons/rtkconv"), PRGNAME);
    startDialog = new StartDialog(this);
    viewer = new TextViewer(this);

    // populate format combo box
    cBFormat->clear();
    cBFormat->addItem(tr("Auto"));
    for (int i = 0; i <= MAXRCVFMT; i++)
        cBFormat->addItem(formatstrs[i], i);
    cBFormat->addItem(formatstrs[STRFMT_RINEX], STRFMT_RINEX);

    QCompleter *fileCompleter = new QCompleter(this);
    QFileSystemModel *fileModel = new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    lEOutputFile1->setCompleter(fileCompleter);
    lEOutputFile2->setCompleter(fileCompleter);
    lEOutputFile3->setCompleter(fileCompleter);
    lEOutputFile4->setCompleter(fileCompleter);
    lEOutputFile5->setCompleter(fileCompleter);
    lEOutputFile6->setCompleter(fileCompleter);
    lEOutputFile7->setCompleter(fileCompleter);
    lEOutputFile8->setCompleter(fileCompleter);
    lEOutputFile9->setCompleter(fileCompleter);
    cBInputFile->setCompleter(fileCompleter);

    comboTimeInterval->setValidator(new QDoubleValidator());

    QCompleter *dirCompleter = new QCompleter(this);
    QFileSystemModel *dirModel = new QFileSystemModel(dirCompleter);
    dirModel->setRootPath("");
    dirModel->setFilter(QDir::AllDirs | QDir::Drives | QDir::NoDotAndDotDot);
    dirCompleter->setModel(dirModel);
    lEOutputDirectory->setCompleter(dirCompleter);

    QAction *acOutputFileSelect[9], *acOutputFileView[9];
    QLineEdit *lEOutputs[9] = {lEOutputFile1, lEOutputFile2, lEOutputFile3, lEOutputFile4,
                               lEOutputFile5, lEOutputFile6, lEOutputFile7, lEOutputFile8, lEOutputFile9};
    for (int i = 0; i < 9; i++) {
        acOutputFileSelect[i] = lEOutputs[i]->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
        acOutputFileSelect[i]->setToolTip(tr("Select File"));
        acOutputFileView[i] = lEOutputs[i]->addAction(QIcon(":/buttons/doc"), QLineEdit::TrailingPosition);
        acOutputFileView[i]->setToolTip(tr("View File"));
    }

    connect(btnPlot, &QPushButton::clicked, this, &MainWindow::callRtkPlot);
    connect(btnConvert, &QPushButton::clicked, this, &MainWindow::convertFile);
    connect(btnOptions, &QPushButton::clicked, this, &MainWindow::showOptions);
    connect(btnExit, &QPushButton::clicked, this, &MainWindow::close);
    connect(btnAbout, &QPushButton::clicked, this, &MainWindow::showAboutDialog);
    connect(btnTimeStart, &QPushButton::clicked, this, &MainWindow::showStartTimeDialog);
    connect(btnTimeStop, &QPushButton::clicked, this, &MainWindow::showStopTimeDialog);
    connect(btnInputFile, &QPushButton::clicked, this, &MainWindow::selectInputFile);
    connect(btnInputFileView, &QPushButton::clicked, this, &MainWindow::viewInputFile);
    connect(acOutputFileSelect[0], &QAction::triggered, this, &MainWindow::selectOutputFile1);
    connect(acOutputFileSelect[1], &QAction::triggered, this, &MainWindow::selectOutputFile2);
    connect(acOutputFileSelect[2], &QAction::triggered, this, &MainWindow::selectOutputFile3);
    connect(acOutputFileSelect[3], &QAction::triggered, this, &MainWindow::selectOutputFile4);
    connect(acOutputFileSelect[4], &QAction::triggered, this, &MainWindow::selectOutputFile5);
    connect(acOutputFileSelect[5], &QAction::triggered, this, &MainWindow::selectOutputFile6);
    connect(acOutputFileSelect[6], &QAction::triggered, this, &MainWindow::selectOutputFile7);
    connect(acOutputFileSelect[7], &QAction::triggered, this, &MainWindow::selectOutputFile8);
    connect(acOutputFileSelect[8], &QAction::triggered, this, &MainWindow::selectOutputFile9);
    connect(acOutputFileView[0], &QAction::triggered, this, &MainWindow::viewOutputFile1);
    connect(acOutputFileView[1], &QAction::triggered, this, &MainWindow::viewOutputFile2);
    connect(acOutputFileView[2], &QAction::triggered, this, &MainWindow::viewOutputFile3);
    connect(acOutputFileView[3], &QAction::triggered, this, &MainWindow::viewOutputFile4);
    connect(acOutputFileView[4], &QAction::triggered, this, &MainWindow::viewOutputFile5);
    connect(acOutputFileView[5], &QAction::triggered, this, &MainWindow::viewOutputFile6);
    connect(acOutputFileView[6], &QAction::triggered, this, &MainWindow::viewOutputFile7);
    connect(acOutputFileView[7], &QAction::triggered, this, &MainWindow::viewOutputFile8);
    connect(acOutputFileView[8], &QAction::triggered, this, &MainWindow::viewOutputFile9);
    connect(btnAbort, &QCheckBox::clicked, this, &MainWindow::abort);
    connect(cBTimeStart, &QCheckBox::clicked, this, &MainWindow::updateEnable);
    connect(cBTimeEnd, &QCheckBox::clicked, this, &MainWindow::updateEnable);
    connect(cBTimeInterval, &QCheckBox::clicked, this, &MainWindow::updateEnable);
    connect(cBTimeUnit, &QCheckBox::clicked, this, &MainWindow::updateEnable);
    connect(cBOutputDirectoryEnable, &QCheckBox::clicked, this, &MainWindow::outputDirectoryEnableClicked);
    connect(cBInputFile, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::inputFileChanged);
    connect(cBInputFile, &QComboBox::editTextChanged, this, &MainWindow::inputFileChanged);
    connect(cBFormat, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::updateEnable);
    connect(lEOutputDirectory, &QLineEdit::editingFinished, this, &MainWindow::outputDirectoryChanged);
    connect(btnOutputDirectory, &QPushButton::clicked, this, &MainWindow::selectOutputDirectory);
    connect(btnKey, &QPushButton::clicked, this, &MainWindow::showKeyDialog);
    connect(btnPost, &QPushButton::clicked, this, &MainWindow::callRtkPost);
    connect(cBOutputFileEnable1, &QCheckBox::clicked, this, &MainWindow::updateEnable);
    connect(cBOutputFileEnable2, &QCheckBox::clicked, this, &MainWindow::updateEnable);
    connect(cBOutputFileEnable3, &QCheckBox::clicked, this, &MainWindow::updateEnable);
    connect(cBOutputFileEnable4, &QCheckBox::clicked, this, &MainWindow::updateEnable);
    connect(cBOutputFileEnable5, &QCheckBox::clicked, this, &MainWindow::updateEnable);
    connect(cBOutputFileEnable6, &QCheckBox::clicked, this, &MainWindow::updateEnable);
    connect(cBOutputFileEnable7, &QCheckBox::clicked, this, &MainWindow::updateEnable);
    connect(cBOutputFileEnable8, &QCheckBox::clicked, this, &MainWindow::updateEnable);
    connect(cBOutputFileEnable9, &QCheckBox::clicked, this, &MainWindow::updateEnable);

    setWindowTitle(QString(tr("%1 ver.%2 %3")).arg(PRGNAME).arg(VER_RTKLIB, PATCH_LEVEL));
    btnAbort->setVisible(false);
}
// callback on form show ----------------------------------------------------
void MainWindow::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    QCommandLineParser parser;
    parser.setApplicationDescription("RTK conv");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption iniFileOption(QStringList() << "i",
                     QCoreApplication::translate("main", "ini file to use"),
                     QCoreApplication::translate("main", "ini file"));
    parser.addOption(iniFileOption);

    QCommandLineOption titleOption(QStringList() << "t",
                       QCoreApplication::translate("main", "window title"),
                       QCoreApplication::translate("main", "title"));
    parser.addOption(titleOption);

    parser.process(*QApplication::instance());

    if (parser.isSet(iniFileOption))
        iniFile = parser.value(iniFileOption);

    loadOptions();

    if (parser.isSet(titleOption))
        setWindowTitle(parser.value(titleOption));
}
// callback on form close ---------------------------------------------------
void MainWindow::closeEvent(QCloseEvent *)
{
    saveOptions();
}
// set output file paths ----------------------------------------------------
void MainWindow::setOutputFiles(const QString &infile)
{
    QLineEdit *edit[] = {
        lEOutputFile1, lEOutputFile2, lEOutputFile3, lEOutputFile4, lEOutputFile5, lEOutputFile6, lEOutputFile7, lEOutputFile8, lEOutputFile9
    };
    QDir outputDirectory(lEOutputDirectory->text());
    QString ofile[10];

    if (infile.isEmpty()) return;

    if (cBOutputDirectoryEnable->isChecked()) {
        QFileInfo info(infile);

        ofile[0] = outputDirectory.filePath(info.fileName());
    } else {
        ofile[0] = infile;
    }
    ofile[0].replace('*', '0');
    ofile[0].replace('?', '0');

    if (!rinexFile) {
        QFileInfo info(ofile[0]);
        ofile[0] = info.absoluteDir().filePath(info.baseName());
        ofile[1] = ofile[0] + ".obs";
        ofile[2] = ofile[0] + ".nav";
        ofile[3] = ofile[0] + ".gnav";
        ofile[4] = ofile[0] + ".hnav";
        ofile[5] = ofile[0] + ".qnav";
        ofile[6] = ofile[0] + ".lnav";
        ofile[7] = ofile[0] + ".cnav";
        ofile[8] = ofile[0] + ".inav";
        ofile[9] = ofile[0] + ".sbs";
    } else {
        QFileInfo info(ofile[0]);
        ofile[0] = info.filePath() + "/";
        ofile[1] += ofile[0] + QString("%%r%%n0.%%yO");
        if (rinexVersion >= 3 && navSys && (navSys != SYS_GPS)) /* ver.3 and mixed system */
            ofile[2] += ofile[0] + "%%r%%n0.%%yP";
        else
            ofile[2] += ofile[0] + "%%r%%n0.%%yN";
        ofile[3] += ofile[0] + "%%r%%n0.%%yG";
        ofile[4] += ofile[0] + "%%r%%n0.%%yH";
        ofile[5] += ofile[0] + "%%r%%n0.%%yQ";
        ofile[6] += ofile[0] + "%%r%%n0.%%yL";
        ofile[7] += ofile[0] + "%%r%%n0.%%yC";
        ofile[8] += ofile[0] + "%%r%%n0.%%yI";
        ofile[9] += ofile[0] + "%%r%%n0_%%y.sbs";
    }
    for (int i = 0; i < 9; i++) {
        if (ofile[i + 1] == infile) ofile[i + 1] += "_";
        ofile[i + 1] = QDir::toNativeSeparators(ofile[i + 1]);
        edit[i]->setText(ofile[i + 1]);
    }
}
// callback on file drag and drop -------------------------------------------
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    trace(3, "dragEnterEvent\n");

    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

// callback on file drag and drop -------------------------------------------
void MainWindow::dropEvent(QDropEvent *event)
{
    trace(3, "dropEvent\n");

    if (!event->mimeData()->hasFormat("text/uri-list")) return;

    QString file = QDir::toNativeSeparators(QUrl(event->mimeData()->text()).toLocalFile());

    cBInputFile->setCurrentText(file);
    setOutputFiles(cBInputFile->currentText());
}
// add history --------------------------------------------------------------
void MainWindow::addHistory(QComboBox *combo)
{
    QString history = combo->currentText();

    if (history == "") return;

    int idx = combo->currentIndex();
    if (idx >= 0) combo->removeItem(idx);

    combo->insertItem(0, history);
    for (int i = combo->count() - 1; i >= MAXHIST; i--) combo->removeItem(i);

    combo->setCurrentIndex(0);
}
// read history -------------------------------------------------------------
void MainWindow::readHistory(QComboBox *combo, QSettings *ini, const QString &key)
{
    QString item;
    for (int i = 0; i < 100; i++) {
        item = ini->value(QString("%1_%2").arg(key).arg(i, 3), "").toString();
        if (item != "" && combo->findText(item)==-1) combo->addItem(item);
    }
}
// write history ------------------------------------------------------------
void MainWindow::writeHistory(QSettings *ini, const QString &key, const QComboBox *combo)
{
    for (int i = 0; i < combo->count(); i++)
        ini->setValue(QString("%1_%2").arg(key).arg(i, 3), combo->itemText(i));
}
// callback on button-plot --------------------------------------------------
void MainWindow::callRtkPlot()
{
    int i, ena[8];
    QString file1 = lEOutputFile1->text();
    QString file2 = lEOutputFile2->text();
    QString file3 = lEOutputFile3->text();
    QString file4 = lEOutputFile4->text();
    QString file5 = lEOutputFile5->text();
    QString file6 = lEOutputFile6->text();
    QString file7 = lEOutputFile7->text();
    QString file8 = lEOutputFile8->text();
    QString file[] = {file1, file2, file3, file4, file5, file6, file7, file8};
    QCheckBox *cb[] = {
        cBOutputFileEnable1, cBOutputFileEnable2, cBOutputFileEnable3, cBOutputFileEnable4, cBOutputFileEnable5, cBOutputFileEnable6, cBOutputFileEnable7, cBOutputFileEnable8
    };
    QString cmd[] = {"rtkplot_qt", "..\\..\\..\\bin\\rtkplot_qt", "..\\rtkplot_qt\\rtkplot_qt"};
    QStringList opts;

    opts << " -r";

    for (i = 0; i < 8; i++) ena[i] = cb[i]->isEnabled() && cb[i]->isChecked();

    for (i = 0; i < 8; i++)
        if (ena[i]) opts << " \"" + repPath(file[i]) + "\"";

    if (opts.size() == 1) return;

    if (!execCommand(cmd[0], opts) && !execCommand(cmd[1], opts) && !execCommand(cmd[2], opts))
        message->setText(tr("error : rtkplot_qt execution"));
}
// callback on button-post-proc ---------------------------------------------
void MainWindow::callRtkPost()
{
    QString cmd[] = {commandPostExe, QString("..\\..\\..\\bin\\") + commandPostExe, QString("..\\rtkpost_qt\\") + commandPostExe};
    QStringList opts;

    if (!cBOutputFileEnable1->isChecked()) return;

    opts << " -r \"" + lEOutputFile1->text() + "\"";
    opts << " -n \"\" -n \"\"";

    if (cBOutputFileEnable9->isChecked())
        opts << " -n \"" + lEOutputFile9->text() + "\"";

    if (cBTimeStart->isChecked()) opts << + " -ts " + dateTime1->dateTime().toString("yyyy/MM/dd hh:mm:ss");
    if (cBTimeEnd->isChecked()) opts << " -te " + dateTime2->dateTime().toString("yyyy/MM/dd hh:mm:ss");
    if (cBTimeInterval->isChecked()) opts << " -ti " + comboTimeInterval->currentText();
    if (cBTimeUnit->isChecked()) opts << " -tu " + cBTimeUnit->text();

    if (!execCommand(cmd[0], opts) && !execCommand(cmd[1], opts) && !execCommand(cmd[2], opts))
        message->setText(tr("error : rtkpost_qt execution"));
}
// callback on button-options -----------------------------------------------
void MainWindow::showOptions()
{
    int rnxfile_old = rinexFile;

    convOptDialog->exec();
    if (convOptDialog->result() != QDialog::Accepted) return;

    if (rinexFile != rnxfile_old)
        setOutputFiles(cBInputFile->currentText());

    updateEnable();
}
//---------------------------------------------------------------------------
void MainWindow::abort()
{
    abortf = 1;
}
// callbck on button-time-1 -------------------------------------------------
void MainWindow::showStartTimeDialog()
{
    gtime_t ts = { 0, 0 }, te = { 0, 0 };
    double tint = 0.0, tunit = 0.0;

    getTime(&ts, &te, &tint, &tunit);
    timeDialog->setTime(ts);
    timeDialog->exec();
}
// callbck on button-time-2 -------------------------------------------------
void MainWindow::showStopTimeDialog()
{
    gtime_t ts = { 0, 0 }, te = { 0, 0 };
    double tint = 0.0, tunit = 0.0;

    getTime(&ts, &te, &tint, &tunit);
    timeDialog->setTime(te);
    timeDialog->exec();
}
// callback on button-input-file --------------------------------------------
void MainWindow::selectInputFile()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Input RTCM, RCV RAW or RINEX File"), cBInputFile->currentText(),
                                                    tr("All (*.*);;RTCM 2 (*.rtcm2);;RTCM 3 (*.rtcm3);;NovtAtel (*.gps);;ublox (*.ubx);;SuperStart II (*.log);;"
                                                       "Hemisphere (*.bin);;Javad (*.jps);;RINEX OBS (*.obs *.*O);Septentrio (*.sbf)"));

    if (!filename.isEmpty()) {
        cBInputFile->setCurrentText(QDir::toNativeSeparators(filename));
        setOutputFiles(cBInputFile->currentText());
    }
}
// callback on output-directory change --------------------------------------
void MainWindow::outputDirectoryChanged()
{
    setOutputFiles(cBInputFile->currentText());
}
// callback on button-output-directory --------------------------------------
void MainWindow::selectOutputDirectory()
{
    lEOutputDirectory->setText(QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Output Directory"), lEOutputDirectory->text())));
    setOutputFiles(cBInputFile->currentText());
}
// callback on button-keyword -----------------------------------------------
void MainWindow::showKeyDialog()
{
    keyDialog->setFlag(1);
    keyDialog->show();
}
// callback on button-output-file-1 -----------------------------------------
void MainWindow::selectOutputFile1()
{
    QString selectedFilter = tr("RINEX OBS (*.obs *.*O)");

    QString filename = QFileDialog::getOpenFileName(this, tr("Output RINEX OBS File"), lEOutputFile1->text(),
                                                    tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                                       "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                                       "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter);

    if (!filename.isEmpty())
        lEOutputFile1->setText(QDir::toNativeSeparators(filename));
}
// callback on button-output-file-2 -----------------------------------------
void MainWindow::selectOutputFile2()
{
    QString selectedFilter = tr("RINEX NAV (*.nav *.*N *.*P)");
    QString filename = QFileDialog::getOpenFileName(this, tr("Output RINEX NAV File"), lEOutputFile2->text(),
                                        tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                           "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                           "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter);

    if (!filename.isEmpty())
        lEOutputFile2->setText(QDir::toNativeSeparators(filename));
}
// callback on button-output-file-3 -----------------------------------------
void MainWindow::selectOutputFile3()
{
    QString selectedFilter = tr("RINEX GNAV (*.gnav *.*G)");

    lEOutputFile3->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output RINEX GNAV File"), lEOutputFile3->text(),
                                        tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                           "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                           "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter)));
}
// callback on button-output-file-4 -----------------------------------------
void MainWindow::selectOutputFile4()
{
    QString selectedFilter = tr("RINEX HNAV (*.hnav *.*H)");

    lEOutputFile4->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output RINEX HNAV File"), lEOutputFile4->text(),
                                        tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                           "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                           "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter)));
}
// callback on button-output-file-5 -----------------------------------------
void MainWindow::selectOutputFile5()
{
    QString selectedFilter = tr("RINEX QNAV (*.qnav *.*Q)");

    lEOutputFile5->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output RINEX QNAV File"), lEOutputFile5->text(),
                                        tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                           "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                           "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter)));
}
// callback on button-output-file-6 -----------------------------------------
void MainWindow::selectOutputFile6()
{
    QString selectedFilter = tr("RINEX LNAV (*.lnav *.*L)");

    lEOutputFile6->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output RINEX LNAV File"), lEOutputFile6->text(),
                                        tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                           "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                           "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter)));
}
// callback on button-output-file-7 -----------------------------------------
void MainWindow::selectOutputFile7()
{
    QString selectedFilter = tr("RINEX CNAV (*.cnav *.*C)");

    lEOutputFile7->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output SRINEX CNAVFile"), lEOutputFile7->text(),
                                        tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                           "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                           "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter)));
}
// callback on button-output-file-8 -----------------------------------------
void MainWindow::selectOutputFile8()
{
    QString selectedFilter = tr("RINEX INAV (*.inav *.*I)");

    lEOutputFile8->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output SBAS/LEX Log File"), lEOutputFile8->text(),
                                        tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                           "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                           "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter)));
}
// callback on button-output-file-9 -----------------------------------------
void MainWindow::selectOutputFile9()
{
    QString selectedFilter = tr("SBAS Log (*.sbs)");

    lEOutputFile9->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output SBAS/LEX Log File"), lEOutputFile9->text(),
                                        tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                           "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                           "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter)));
}
// callback on button-view-input-file ----------------------------------------
void MainWindow::viewInputFile()
{
    QString inputFilename = cBInputFile->currentText();
    QString ext = QFileInfo(inputFilename).suffix();

    if (ext.length() < 3) return;

    if ((ext.toLower() == "obs") || (ext.toLower() == "nav") ||
        (ext.mid(1).toLower() == "nav") ||
        (ext.at(2) == 'o') || (ext.at(2) == 'O') || (ext.at(2) == 'n') ||
        (ext.at(2) == 'N') || (ext.at(2) == 'p') || (ext.at(2) == 'P') ||
        (ext.at(2) == 'g') || (ext.at(2) == 'G') || (ext.at(2) == 'h') ||
        (ext.at(2) == 'H') || (ext.at(2) == 'q') || (ext.at(2) == 'Q') ||
        (ext.at(2) == 'l') || (ext.at(2) == 'L') || (ext.at(2) == 'c') ||
        (ext.at(2) == 'C') || (ext.at(2) == 'I') || (ext.at(2) == 'i') ||
            (ext == "rnx") || (ext == "RNX")) {
        viewer->show();
        viewer->read(repPath(inputFilename));
    }
}
// callback on button-view-file-1 -------------------------------------------
void MainWindow::viewOutputFile1()
{
    QString outputFile1_Text = lEOutputFile1->text();

    viewer->read(repPath(outputFile1_Text));
    viewer->show();
}
// callback on button-view-file-2 -------------------------------------------
void MainWindow::viewOutputFile2()
{
    QString outputFile2_Text = lEOutputFile2->text();

    viewer->read(repPath(outputFile2_Text));
    viewer->show();
}
// callback on button-view-file-3 -------------------------------------------
void MainWindow::viewOutputFile3()
{
    QString outputFile3_Text = lEOutputFile3->text();

    viewer->read(repPath(outputFile3_Text));
    viewer->show();
}
// callback on button-view-file-4 -------------------------------------------
void MainWindow::viewOutputFile4()
{
    QString outputFile4_Text = lEOutputFile4->text();

    viewer->read(repPath(outputFile4_Text));
    viewer->show();
}
// callback on button-view-file-5 -------------------------------------------
void MainWindow::viewOutputFile5()
{
    QString outputFile5_Text = lEOutputFile5->text();

    viewer->read(repPath(outputFile5_Text));
    viewer->show();
}
// callback on button-view-file-6 -------------------------------------------
void MainWindow::viewOutputFile6()
{
    QString outputFile6_Text = lEOutputFile6->text();

    viewer->read(repPath(outputFile6_Text));
    viewer->show();
}
// callback on button-view-file-7 -------------------------------------------
void MainWindow::viewOutputFile7()
{
    QString outputFile7_Text = lEOutputFile7->text();

    viewer->read(repPath(outputFile7_Text));
    viewer->show();
}
// callback on button-view-file-8 -------------------------------------------
void MainWindow::viewOutputFile8()
{
    QString outputFile8_Text = lEOutputFile8->text();

    viewer->read(repPath(outputFile8_Text));
    viewer->show();
}
// callback on button-view-file-9 -------------------------------------------
void MainWindow::viewOutputFile9()
{
    QString outputFile9_Text = lEOutputFile9->text();

    viewer->read(repPath(outputFile9_Text));
    viewer->show();
}
// callback on button-about -------------------------------------------------
void MainWindow::showAboutDialog()
{
    aboutDialog->exec();
}
// callback on output-file check/uncheck ------------------------------------
void MainWindow::outputDirectoryEnableClicked()
{
    setOutputFiles(cBInputFile->currentText());
    updateEnable();
}
// callback on input-file-change --------------------------------------------
void MainWindow::inputFileChanged()
{
    setOutputFiles(cBInputFile->currentText());
}
// get time -----------------------------------------------------------------
void MainWindow::getTime(gtime_t *ts, gtime_t *te, double *tint,
             double *tunit)
{
    if (cBTimeStart->isChecked()) {
        QDateTime start(dateTime1->dateTime());
        ts->time = start.toSecsSinceEpoch();
        ts->sec = start.time().msec() / 1000;
    } else {
        ts->time = 0;
        ts->sec = 0;
    }

    if (cBTimeEnd->isChecked()) {
        QDateTime end(dateTime2->dateTime());
        te->time = end.toSecsSinceEpoch();
        te->sec = end.time().msec() / 1000;
    } else {
        te->time = 0;
        te->sec = 0;
    }

    if (cBTimeInterval->isChecked())
        *tint = comboTimeInterval->currentText().toDouble();
    else *tint = 0;

    if (cBTimeUnit->isChecked())
        *tunit = sBTimeUnit->value() * 3600;
    else *tunit = 0;
}
// replace keywords in file path --------------------------------------------
QString MainWindow::repPath(const QString &File)
{
    char path[1024];

    reppath(qPrintable(File), path, timeadd(rinexTime, TSTARTMARGIN), qPrintable(rinexStationCode), "");
    return QString(path);
}
// execute command ----------------------------------------------------------
int MainWindow::execCommand(const QString &cmd, QStringList &opt)
{
    return QProcess::startDetached(cmd, opt);
}
// update enable/disable of widgets -----------------------------------------
void MainWindow::updateEnable(void)
{
    QString formatText = cBFormat->currentText();
    int rnx = (formatText == "RINEX");
    int sep_nav = (rinexVersion < 3 || separateNavigation);

    dateTime1->setEnabled(cBTimeStart->isChecked());
    btnTimeStart->setEnabled(cBTimeStart->isChecked());
    dateTime2->setEnabled(cBTimeEnd->isChecked());
    btnTimeStop->setEnabled(cBTimeEnd->isChecked());
    comboTimeInterval->setEnabled(cBTimeInterval->isChecked());
    lbTimeInterval->setEnabled(cBTimeInterval->isEnabled());
    cBTimeUnit->setEnabled(cBTimeStart->isChecked() && cBTimeEnd->isChecked());
    sBTimeUnit->setEnabled(cBTimeStart->isChecked() && cBTimeEnd->isChecked() && cBTimeUnit->isChecked());
    cBOutputFileEnable3->setEnabled(sep_nav && (navSys & SYS_GLO));
    cBOutputFileEnable3->setChecked(cBOutputFileEnable3->isEnabled());
    cBOutputFileEnable4->setEnabled(sep_nav && (navSys & SYS_SBS));
    cBOutputFileEnable4->setChecked(cBOutputFileEnable4->isEnabled());
    cBOutputFileEnable5->setEnabled(sep_nav && (navSys & SYS_QZS) && rinexVersion >= 5);
    cBOutputFileEnable5->setChecked(cBOutputFileEnable5->isEnabled());
    cBOutputFileEnable6->setEnabled(sep_nav && (navSys & SYS_GAL) && rinexVersion >= 2);
    cBOutputFileEnable6->setChecked(cBOutputFileEnable6->isEnabled());
    cBOutputFileEnable7->setEnabled(sep_nav && (navSys & SYS_CMP) && rinexVersion >= 4);
    cBOutputFileEnable7->setChecked(cBOutputFileEnable7->isEnabled());
    cBOutputFileEnable8->setEnabled(sep_nav && (navSys & SYS_IRN) && rinexVersion >= 6);
    cBOutputFileEnable8->setChecked(cBOutputFileEnable8->isEnabled());
    cBOutputFileEnable9->setEnabled(!rnx);
    cBOutputFileEnable9->setChecked(cBOutputFileEnable9->isEnabled());
    lEOutputDirectory->setEnabled(cBOutputDirectoryEnable->isChecked());
    lEOutputFile1->setEnabled(cBOutputFileEnable1->isChecked());
    lEOutputFile2->setEnabled(cBOutputFileEnable2->isChecked());
    lEOutputFile3->setEnabled(cBOutputFileEnable3->isChecked() && cBOutputFileEnable3->isEnabled());
    lEOutputFile4->setEnabled(cBOutputFileEnable4->isChecked() && cBOutputFileEnable4->isEnabled());
    lEOutputFile5->setEnabled(cBOutputFileEnable5->isChecked() && cBOutputFileEnable5->isEnabled());
    lEOutputFile6->setEnabled(cBOutputFileEnable6->isChecked() && cBOutputFileEnable6->isEnabled());
    lEOutputFile7->setEnabled(cBOutputFileEnable7->isChecked() && cBOutputFileEnable7->isEnabled());
    lEOutputFile8->setEnabled(cBOutputFileEnable8->isChecked() && cBOutputFileEnable8->isEnabled());
    lEOutputFile9->setEnabled(cBOutputFileEnable9->isChecked() && !rnx);
    btnOutputDirectory->setEnabled(cBOutputDirectoryEnable->isChecked());
}
// convert file -------------------------------------------------------------
void MainWindow::convertFile(void)
{
    QString inputFile_Text = cBInputFile->currentText();
    QString outputFile1_Text = lEOutputFile1->text(), outputFile2_Text = lEOutputFile2->text();
    QString outputFile3_Text = lEOutputFile3->text(), outputFile4_Text = lEOutputFile4->text();
    QString outputFile5_Text = lEOutputFile5->text(), outputFile6_Text = lEOutputFile6->text();
    QString outputFile7_Text = lEOutputFile7->text(), outputFile8_Text = lEOutputFile8->text();
    QString outputFile9_Text = lEOutputFile9->text();
    int i;
    double RNXVER[] = { 210, 211, 212, 300, 301, 302, 303, 304 };

    conversionThread = new ConversionThread(this);

    // recognize input file format
    strncpy(conversionThread->ifile, qPrintable(inputFile_Text), 1023);
    QFileInfo fi(inputFile_Text);
    if (cBFormat->currentIndex() == 0) { // auto
        if (fi.completeSuffix() == "rtcm2") {
            conversionThread->format = STRFMT_RTCM2;
        } else if (fi.completeSuffix() == "rtcm3") {
            conversionThread->format = STRFMT_RTCM3;
        } else if (fi.completeSuffix() == "gps") {
            conversionThread->format = STRFMT_OEM4;
        } else if (fi.completeSuffix() == "ubx") {
            conversionThread->format = STRFMT_UBX;
        } else if (fi.completeSuffix() == "sbp") {
            conversionThread->format = STRFMT_SBP;
        } else if (fi.completeSuffix() == "bin") {
            conversionThread->format = STRFMT_CRES;
        } else if (fi.completeSuffix() == "jps") {
            conversionThread->format = STRFMT_JAVAD;
        } else if (fi.completeSuffix() == "bnx") {
            conversionThread->format = STRFMT_BINEX;
        } else if (fi.completeSuffix() == "binex") {
            conversionThread->format = STRFMT_BINEX;
        } else if (fi.completeSuffix() == "rt17") {
            conversionThread->format = STRFMT_RT17;
        } else if (fi.completeSuffix() == "sbf") {
            conversionThread->format = STRFMT_SEPT;
        } else if (fi.completeSuffix().toLower() == "obs") {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().toLower().contains("nav")) {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'o') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'O') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'n') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'N') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'p') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'P') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'g') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'G') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'h') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'H') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'q') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'Q') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'l') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'L') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'c') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'C') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'i') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'I') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix() == "rnx") {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix() == "RNX") {
            conversionThread->format = STRFMT_RINEX;
        } else {
            showmsg("file format can not be recognized");
            return;
        }
    } else {
        conversionThread->format = cBFormat->currentData().toInt();
        if (conversionThread->format<0)
            return;
    }
    conversionThread->rnxopt.rnxver = RNXVER[rinexVersion];

    if (conversionThread->format == STRFMT_RTCM2 || conversionThread->format == STRFMT_RTCM3 || conversionThread->format == STRFMT_RT17) {
        // input start date/time for rtcm 2, rtcm 3, RT17 or CMR
        startDialog->exec();
        if (startDialog->result() != QDialog::Accepted) return;
        conversionThread->rnxopt.trtcm = startDialog->getTime();
    }
    if (lEOutputFile1->isEnabled() && cBOutputFileEnable1->isChecked()) strncpy(conversionThread->ofile[0], qPrintable(outputFile1_Text), 1023);
    if (lEOutputFile2->isEnabled() && cBOutputFileEnable2->isChecked()) strncpy(conversionThread->ofile[1], qPrintable(outputFile2_Text), 1023);
    if (lEOutputFile3->isEnabled() && cBOutputFileEnable3->isChecked()) strncpy(conversionThread->ofile[2], qPrintable(outputFile3_Text), 1023);
    if (lEOutputFile4->isEnabled() && cBOutputFileEnable4->isChecked()) strncpy(conversionThread->ofile[3], qPrintable(outputFile4_Text), 1023);
    if (lEOutputFile5->isEnabled() && cBOutputFileEnable5->isChecked()) strncpy(conversionThread->ofile[4], qPrintable(outputFile5_Text), 1023);
    if (lEOutputFile6->isEnabled() && cBOutputFileEnable6->isChecked()) strncpy(conversionThread->ofile[5], qPrintable(outputFile6_Text), 1023);
    if (lEOutputFile7->isEnabled() && cBOutputFileEnable7->isChecked()) strncpy(conversionThread->ofile[6], qPrintable(outputFile7_Text), 1023);
    if (lEOutputFile8->isEnabled() && cBOutputFileEnable8->isChecked()) strncpy(conversionThread->ofile[7], qPrintable(outputFile8_Text), 1023);
    if (lEOutputFile9->isEnabled() && cBOutputFileEnable9->isChecked()) strncpy(conversionThread->ofile[8], qPrintable(outputFile9_Text), 1023);

    // check overwrite output file
    for (i = 0; i < 9; i++) {
        if (!QFile(conversionThread->ofile[i]).exists()) continue;
        if (QMessageBox::question(this, tr("Overwrite"), QString(tr("%1 exists. Do you want to overwrite?")).arg(conversionThread->ofile[i])) != QMessageBox::Yes) return;
    }
    getTime(&conversionThread->rnxopt.ts, &conversionThread->rnxopt.te, &conversionThread->rnxopt.tint, &conversionThread->rnxopt.tunit);
    strncpy(conversionThread->rnxopt.staid, qPrintable(rinexStationCode), 31);
    sprintf(conversionThread->rnxopt.prog, "%s %s %s", PRGNAME, VER_RTKLIB, PATCH_LEVEL);
    strncpy(conversionThread->rnxopt.runby, qPrintable(runBy), 31);
    strncpy(conversionThread->rnxopt.marker, qPrintable(marker), 63);
    strncpy(conversionThread->rnxopt.markerno, qPrintable(markerNo), 31);
    strncpy(conversionThread->rnxopt.markertype, qPrintable(markerType), 31);
    for (i = 0; i < 2; i++) strncpy(conversionThread->rnxopt.name[i], qPrintable(name[i]), 31);
    for (i = 0; i < 3; i++) strncpy(conversionThread->rnxopt.rec[i], qPrintable(receiver [i]), 31);
    for (i = 0; i < 3; i++) strncpy(conversionThread->rnxopt.ant[i], qPrintable(antenna [i]), 31);
    if (autoPosition)
        for (i = 0; i < 3; i++) conversionThread->rnxopt.apppos[i] = approxPosition[i];
    for (i = 0; i < 3; i++) conversionThread->rnxopt.antdel[i] = antennaDelta[i];
    strncpy(conversionThread->rnxopt.rcvopt, qPrintable(receiverOptions), 255);
    conversionThread->rnxopt.navsys = navSys;
    conversionThread->rnxopt.obstype = observationType;
    conversionThread->rnxopt.freqtype = frequencyType;
    for (i = 0; i < 2; i++) sprintf(conversionThread->rnxopt.comment[i], "%.63s", qPrintable(comment[i]));
    for (i = 0; i < 7; i++) strncpy(conversionThread->rnxopt.mask[i], qPrintable(codeMask[i]), 63);
    conversionThread->rnxopt.autopos = autoPosition;
    conversionThread->rnxopt.phshift = phaseShift;
    conversionThread->rnxopt.halfcyc = halfCycle;
    conversionThread->rnxopt.outiono = outputIonoCorr;
    conversionThread->rnxopt.outtime = outputTimeCorr;
    conversionThread->rnxopt.outleaps = outputLeapSeconds;
    conversionThread->rnxopt.sep_nav = separateNavigation;
    conversionThread->rnxopt.ttol = timeTolerance;
    if (enableGlonassFrequency) {
        for (i = 0; i < MAXPRNGLO; i++) conversionThread->rnxopt.glofcn[i] = glonassFrequency[i];
    }

    QStringList exsatsLst = excludedSatellites.split(" ");
    foreach(const QString &sat, exsatsLst){
        int satid;

        if (!(satid = satid2no(qPrintable(sat)))) continue;
        conversionThread->rnxopt.exsats[satid - 1] = 1;
    }

    abortf = 0;
    btnConvert->setVisible(false);
    btnAbort->setVisible(true);
    panel1->setEnabled(false);
    panel2->setEnabled(false);
    btnPlot->setEnabled(false);
    btnPost->setEnabled(false);
    btnOptions->setEnabled(false);
    btnExit->setEnabled(false);
    cBFormat->setEnabled(false);
    btnKey->setEnabled(false);
    lbInputFile->setEnabled(false);
    cBOutputDirectoryEnable->setEnabled(false);
    lbOutputFile->setEnabled(false);
    lbFormat->setEnabled(false);
    message->setText("");

    if (traceLevel > 0) {
        traceopen(TRACEFILE);
        tracelevel(traceLevel);
    }
    setCursor(Qt::WaitCursor);

    // post processing positioning
    connect(conversionThread, &QThread::finished, this, &MainWindow::conversionFinished);

    conversionThread->start();
}
// conversion done -------------------------------------------------------------
void MainWindow::conversionFinished()
{
    setCursor(Qt::ArrowCursor);

    if (traceLevel > 0)
        traceclose();

    btnConvert->setVisible(true);
    btnAbort->setVisible(false);
    panel1->setEnabled(true);
    panel2->setEnabled(true);
    btnPlot->setEnabled(true);
    btnPost->setEnabled(true);
    btnOptions->setEnabled(true);
    btnExit->setEnabled(true);
    cBFormat->setEnabled(true);
    btnKey->setEnabled(true);
    lbInputFile->setEnabled(true);
    cBOutputDirectoryEnable->setEnabled(true);
    lbOutputFile->setEnabled(true);
    lbFormat->setEnabled(true);

#if 0
    // set time-start/end if time not specified
    if (!TimeStartF->Checked && rnxopt.tstart.time != 0) {
        time2str(rnxopt.tstart, tstr, 0);
        tstr[10] = '\0';
        TimeY1->Text = tstr;
        TimeH1->Text = tstr + 11;
    }
    if (!TimeEndF->Checked && rnxopt.tend.time != 0) {
        time2str(rnxopt.tend, tstr, 0);
        tstr[10] = '\0';
        TimeY2->Text = tstr;
        TimeH2->Text = tstr + 11;
    }
#endif
    rinexTime = conversionThread->rnxopt.tstart;

    conversionThread->deleteLater();

    addHistory(cBInputFile);
}
// load options -------------------------------------------------------------
void MainWindow::loadOptions(void)
{
    QSettings ini(iniFile, QSettings::IniFormat);
    QString mask = "11111111111111111111111111111111111111111111111111111111111111111111";

    rinexVersion = ini.value("opt/rnxver", 6).toInt();
    rinexFile = ini.value("opt/rnxfile", 0).toInt();
    rinexStationCode = ini.value("opt/rnxcode", "0000").toString();
    runBy = ini.value("opt/runby", "").toString();
    marker = ini.value("opt/marker", "").toString();
    markerNo = ini.value("opt/markerno", "").toString();
    markerType = ini.value("opt/markertype", "").toString();
    name[0] = ini.value("opt/name0", "").toString();
    name[1] = ini.value("opt/name1", "").toString();
    receiver[0] = ini.value("opt/rec0", "").toString();
    receiver[1] = ini.value("opt/rec1", "").toString();
    receiver[2] = ini.value("opt/rec2", "").toString();
    antenna[0] = ini.value("opt/ant0", "").toString();
    antenna[1] = ini.value("opt/ant1", "").toString();
    antenna[2] = ini.value("opt/ant2", "").toString();
    approxPosition[0] = ini.value("opt/apppos0", 0.0).toDouble();
    approxPosition[1] = ini.value("opt/apppos1", 0.0).toDouble();
    approxPosition[2] = ini.value("opt/apppos2", 0.0).toDouble();
    antennaDelta[0] = ini.value("opt/antdel0", 0.0).toDouble();
    antennaDelta[1] = ini.value("opt/antdel1", 0.0).toDouble();
    antennaDelta[2] = ini.value("opt/antdel2", 0.0).toDouble();
    comment[0] = ini.value("opt/comment0", "").toString();
    comment[1] = ini.value("opt/comment1", "").toString();
    receiverOptions = ini.value("opt/rcvoption", "").toString();
    navSys = ini.value("opt/navsys", 0xff).toInt();
    observationType = ini.value("opt/obstype", 0xF).toInt();
    frequencyType = ini.value("opt/freqtype", 0x7).toInt();
    excludedSatellites = ini.value("opt/exsats", "").toString();
    traceLevel = ini.value("opt/tracelevel", 0).toInt();
    rinexTime.time = ini.value("opt/rnxtime", 0).toInt();
    codeMask[0] = ini.value("opt/codemask_1", mask).toString();
    codeMask[1] = ini.value("opt/codemask_2", mask).toString();
    codeMask[2] = ini.value("opt/codemask_3", mask).toString();
    codeMask[3] = ini.value("opt/codemask_4", mask).toString();
    codeMask[4] = ini.value("opt/codemask_5", mask).toString();
    codeMask[5] = ini.value("opt/codemask_6", mask).toString();
    codeMask[6] = ini.value("opt/codemask_7", mask).toString();
    autoPosition = ini.value("opt/autopos", 0).toInt();
    phaseShift = ini.value("opt/phaseShift", 0).toInt();
    halfCycle = ini.value("opt/halfcyc", 0).toInt();
    outputIonoCorr = ini.value("opt/outiono", 0).toInt();
    outputTimeCorr = ini.value("opt/outtime", 0).toInt();
    outputLeapSeconds = ini.value("opt/outleaps", 0).toInt();
    separateNavigation = ini.value("opt/sepnav", 0).toInt();
    timeTolerance	= ini.value("opt/timetol", 0.005).toInt();
    enableGlonassFrequency = ini.value("opt/glofcnena", 0).toInt();
    for (int i = 0; i < 27; i++)
        glonassFrequency[i] = ini.value(QString("opt/glofcn%1").arg(i + 1, 2, 10, QLatin1Char('0')), 0).toInt();

    cBTimeStart->setChecked(ini.value("set/timestartf", 0).toBool());
    cBTimeEnd->setChecked(ini.value("set/timeendf", 0).toBool());
    cBTimeInterval->setChecked(ini.value("set/timeintf", 0).toBool());
    dateTime1->setDate(ini.value("set/timey1", "2020/01/01").value<QDate>());
    dateTime1->setTime(ini.value("set/timeh1", "00:00:00").value<QTime>());
    dateTime2->setDate(ini.value("set/timey2", "2020/01/01").value<QDate>());
    dateTime2->setTime(ini.value("set/timeh2", "00:00:00").value<QTime>());
    comboTimeInterval->setCurrentText(ini.value("set/timeint", "1").toString());
    cBTimeUnit->setChecked(ini.value("set/timeunitf", 0).toBool());
    sBTimeUnit->setValue(ini.value("set/timeunit", "24").toDouble());
    cBInputFile->setCurrentText(ini.value("set/infile", "").toString());
    lEOutputDirectory->setText(ini.value("set/outdir", "").toString());
    lEOutputFile1->setText(ini.value("set/outfile1", "").toString());
    lEOutputFile2->setText(ini.value("set/outfile2", "").toString());
    lEOutputFile3->setText(ini.value("set/outfile3", "").toString());
    lEOutputFile4->setText(ini.value("set/outfile4", "").toString());
    lEOutputFile5->setText(ini.value("set/outfile5", "").toString());
    lEOutputFile6->setText(ini.value("set/outfile6", "").toString());
    lEOutputFile7->setText(ini.value("set/outfile7", "").toString());
    lEOutputFile8->setText(ini.value("set/outfile8", "").toString());
    lEOutputFile9->setText(ini.value("set/outfile9", "").toString());
    cBOutputDirectoryEnable->setChecked(ini.value("set/outdirena", false).toBool());
    cBOutputFileEnable1->setChecked(ini.value("set/outfileena1", true).toBool());
    cBOutputFileEnable2->setChecked(ini.value("set/outfileena2", true).toBool());
    cBOutputFileEnable3->setChecked(ini.value("set/outfileena3", true).toBool());
    cBOutputFileEnable4->setChecked(ini.value("set/outfileena4", true).toBool());
    cBOutputFileEnable5->setChecked(ini.value("set/outfileena5", true).toBool());
    cBOutputFileEnable6->setChecked(ini.value("set/outfileena6", true).toBool());
    cBOutputFileEnable7->setChecked(ini.value("set/outfileena7", true).toBool());
    cBOutputFileEnable8->setChecked(ini.value("set/outfileena8", true).toBool());
    cBOutputFileEnable9->setChecked(ini.value("set/outfileena9", true).toBool());
    cBFormat->setCurrentIndex(ini.value("set/format", 0).toInt());

    readHistory(cBInputFile, &ini, "hist/inputfile");

    viewer->loadOptions(ini);

    commandPostExe = ini.value("set/cmdpostexe", "rtkpost_qt").toString();

    updateEnable();
}
// save options -------------------------------------------------------------
void MainWindow::saveOptions(void)
{
    QSettings ini(iniFile, QSettings::IniFormat);

    ini.setValue("opt/rnxver", rinexVersion);
    ini.setValue("opt/rnxfile", rinexFile);
    ini.setValue("opt/rnxcode", rinexStationCode);
    ini.setValue("opt/runby", runBy);
    ini.setValue("opt/marker", marker);
    ini.setValue("opt/markerno", markerNo);
    ini.setValue("opt/markertype", markerType);
    ini.setValue("opt/name0", name[0]);
    ini.setValue("opt/name1", name[1]);
    ini.setValue("opt/rec0", receiver[0]);
    ini.setValue("opt/rec1", receiver[1]);
    ini.setValue("opt/rec2", receiver[2]);
    ini.setValue("opt/ant0", antenna[0]);
    ini.setValue("opt/ant1", antenna[1]);
    ini.setValue("opt/ant2", antenna[2]);
    ini.setValue("opt/apppos0", approxPosition[0]);
    ini.setValue("opt/apppos1", approxPosition[1]);
    ini.setValue("opt/apppos2", approxPosition[2]);
    ini.setValue("opt/antdel0", antennaDelta[0]);
    ini.setValue("opt/antdel1", antennaDelta[1]);
    ini.setValue("opt/antdel2", antennaDelta[2]);
    ini.setValue("opt/comment0", comment[0]);
    ini.setValue("opt/comment1", comment[1]);
    ini.setValue("opt/rcvoption", receiverOptions);
    ini.setValue("opt/navsys", navSys);
    ini.setValue("opt/obstype", observationType);
    ini.setValue("opt/freqtype", frequencyType);
    ini.setValue("opt/exsats", excludedSatellites);
    ini.setValue("opt/tracelevel", traceLevel);
    ini.setValue("opt/rnxtime", (int)(rinexTime.time));
    ini.setValue("opt/codemask_1", codeMask[0]);
    ini.setValue("opt/codemask_2", codeMask[1]);
    ini.setValue("opt/codemask_3", codeMask[2]);
    ini.setValue("opt/codemask_4", codeMask[3]);
    ini.setValue("opt/codemask_5", codeMask[4]);
    ini.setValue("opt/codemask_6", codeMask[5]);
    ini.setValue("opt/codemask_7", codeMask[6]);
    ini.setValue("opt/autopos", autoPosition);
    ini.setValue("opt/phasewhift", phaseShift);
    ini.setValue("opt/halfcyc", halfCycle);
    ini.setValue("opt/outiono", outputIonoCorr);
    ini.setValue("opt/outtime", outputTimeCorr);
    ini.setValue("opt/outleaps", outputLeapSeconds);
    ini.setValue("opt/sepnav", separateNavigation);
    ini.setValue("opt/timetol", timeTolerance);
    ini.setValue("opt/glofcnena",  enableGlonassFrequency);
    for (int i = 0; i < 27; i++) {
        ini.setValue(QString("opt/glofcn%1").arg(i + 1, 2, 10, QLatin1Char('0')), glonassFrequency[i]);
    }

    ini.setValue("set/timestartf", cBTimeStart->isChecked());
    ini.setValue("set/timeendf", cBTimeEnd->isChecked());
    ini.setValue("set/timeintf", cBTimeInterval->isChecked());
    ini.setValue("set/timey1", dateTime1->date());
    ini.setValue("set/timeh1", dateTime1->time());
    ini.setValue("set/timey2", dateTime2->date());
    ini.setValue("set/timeh2", dateTime2->time());
    ini.setValue("set/timeint", comboTimeInterval->currentText());
    ini.setValue("set/timeunitf", cBTimeUnit->isChecked());
    ini.setValue("set/timeunit", sBTimeUnit->value());
    ini.setValue("set/infile", cBInputFile->currentText());
    ini.setValue("set/outdir", lEOutputDirectory->text());
    ini.setValue("set/outfile1", lEOutputFile1->text());
    ini.setValue("set/outfile2", lEOutputFile2->text());
    ini.setValue("set/outfile3", lEOutputFile3->text());
    ini.setValue("set/outfile4", lEOutputFile4->text());
    ini.setValue("set/outfile5", lEOutputFile5->text());
    ini.setValue("set/outfile6", lEOutputFile6->text());
    ini.setValue("set/outfile7", lEOutputFile7->text());
    ini.setValue("set/outfile8", lEOutputFile8->text());
    ini.setValue("set/outfile9", lEOutputFile9->text());
    ini.setValue("set/outdirena", cBOutputDirectoryEnable->isChecked());
    ini.setValue("set/outfileena1", cBOutputFileEnable1->isChecked());
    ini.setValue("set/outfileena2", cBOutputFileEnable2->isChecked());
    ini.setValue("set/outfileena3", cBOutputFileEnable3->isChecked());
    ini.setValue("set/outfileena4", cBOutputFileEnable4->isChecked());
    ini.setValue("set/outfileena5", cBOutputFileEnable5->isChecked());
    ini.setValue("set/outfileena6", cBOutputFileEnable6->isChecked());
    ini.setValue("set/outfileena7", cBOutputFileEnable7->isChecked());
    ini.setValue("set/outfileena8", cBOutputFileEnable8->isChecked());
    ini.setValue("set/outfileena9", cBOutputFileEnable9->isChecked());
    ini.setValue("set/format", cBFormat->currentIndex());

    writeHistory(&ini, "hist/inputfile", cBInputFile);

    viewer->saveOptions(ini);
};
//---------------------------------------------------------------------------
