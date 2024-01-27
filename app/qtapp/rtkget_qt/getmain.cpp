//---------------------------------------------------------------------------
// rtkget : gnss data downloader
//
//          Copyright (C) 2012 by T.TAKASU, All rights reserved.
//          ported to Qt by Jens Reimann
//
// options : rtkget [-t title][-i file]
//
//           -t title   window title
//           -i file    ini file path
//
// version : $Revision:$ $Date:$
// history : 2012/12/28  1.0 new
//---------------------------------------------------------------------------
#include <clocale>

#include <QShowEvent>
#include <QDesktopServices>
#include <QCloseEvent>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QFileDialog>
#include <QProcess>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>
#include <QFuture>
#include <QThread>
#include <QCompleter>
#include <QFileSystemModel>
#include <QStringList>

//---------------------------------------------------------------------------
#include "rtklib.h"
#include "keydlg.h"
#include "aboutdlg.h"
#include "getmain.h"
#include "getoptdlg.h"
#include "staoptdlg.h"
#include "timedlg.h"
#include "viewer.h"

#include <cstdio>

#define PRGNAME     "RTKGet-Qt"  // program name

#define URL_FILE    "../../../data/URL_LIST.txt"
#define TEST_FILE   "rtkget_test.txt"
#define TRACE_FILE  "rtkget_qt.trace"

#define MAX_URL     2048
#define MAX_URL_SEL 64
#define MAX_STA     2048
#define MAX_HIST    16

#define MAX(x, y)    ((x) > (y) ? (x) : (y))

static int abortf = 0;          // abort flag

MainForm *mainForm;

// show message in message area ---------------------------------------------
extern "C" {
    extern int showmsg(const char *format, ...)
    {
        va_list arg;
        QString str;
        static QString buff2;
        char buff[1024], *p;

        va_start(arg, format);
        vsprintf(buff, format, arg);
        va_end(arg);

        if ((p = strstr(buff, "STAT="))) {
            buff2.append(str.right(66));
            if (buff2.endsWith('_')) buff2.remove(buff2.length() - 1, 1);
            buff2.append(p + 5);
            QMetaObject::invokeMethod(mainForm->messageLabel3, "setText", Qt::QueuedConnection, Q_ARG(QString, QString(buff2)));
        } else if ((p = strstr(buff, "->"))) {
            *p = '\0';
            QMetaObject::invokeMethod(mainForm->messageLabel1, "setText", Qt::QueuedConnection, Q_ARG(QString, QString(buff)));
            QMetaObject::invokeMethod(mainForm->messageLabel2, "setText", Qt::QueuedConnection, Q_ARG(QString, QString(p + 2)));
        }
        return abortf;
    }
    void settspan(gtime_t, gtime_t)
    {
    }
    void settime(gtime_t)
    {
    }
}

class DownloadThread : public QThread
{
    public:
        QString usr, pwd, proxy;
        FILE *fp;
        url_t urls[MAX_URL_SEL];
        gtime_t ts, te;
        double ti;
        char *stas[MAX_STA], dir[1024], msg[1024], path[1024];
        int nsta, nurl, seqnos, seqnoe, opts;
        QString logFile;
        bool test, append;
        int columnCnt;
        int dateFormat;

        explicit DownloadThread(QObject *parent, const QString lf, bool a, bool t) : QThread(parent)
        {
            seqnos = 0;
            seqnoe = 0;
            opts = 0;
            ts.time = 0;
            te.time = 0;
            fp = NULL;
            test = t;
            logFile = lf;
            append = a;
            columnCnt = 0;
            dateFormat = 0;

            for (int i = 0; i < MAX_STA; i++) {
                stas[i] = new char [16];
                stas[i][0] = '\0';
            }
            ;
            for (int i = 0; i < MAX_URL_SEL; i++) memset(&urls[i], 0, sizeof(url_t));
            dir[0] = msg[0] = path[0] = '\0';
            nurl = nsta = 0; ti = 0;
        }
        ~DownloadThread()
        {
            for (int i = 0; i < MAX_STA; i++) delete [] stas[i];
        }

    protected:
        void run()
        {
            if (logFile != "") {
                reppath(qPrintable(logFile), path, utc2gpst(timeget()), "", "");
                fp = fopen(path, append ? "a" : "w");
            }
            if (test)
                dl_test(ts, te, ti, urls, nurl, stas, nsta, dir, columnCnt, dateFormat, fp);
            else
                dl_exec(ts, te, ti, seqnos, seqnoe, urls, nurl, stas, nsta, dir, qPrintable(usr),
                    qPrintable(pwd), qPrintable(proxy), opts, msg, fp);
            if (fp)
                fclose(fp);
        }
};

//---------------------------------------------------------------------------
MainForm::MainForm(QWidget *parent)
    : QWidget(parent)
{
    mainForm = this;
    setupUi(this);

    setlocale(LC_NUMERIC, "C");

    viewer = new TextViewer(this);
    timeDialog = new TimeDialog(this);

    QCompleter *dirCompleter = new QCompleter(this);
    QFileSystemModel *dirModel = new QFileSystemModel(dirCompleter);
    dirModel->setRootPath("");
    dirModel->setFilter(QDir::AllDirs | QDir::Drives | QDir::NoDotAndDotDot);
    dirCompleter->setModel(dirModel);
    cBDirectory->setCompleter(dirCompleter);

    connect(btnAll, &QPushButton::clicked, this, &MainForm::SelectDeselectAllStations);
    connect(btnDir, &QPushButton::clicked, this, &MainForm::selectOutputDirectory);
    connect(btnDownload, &QPushButton::clicked, this, &MainForm::download);
    connect(btnExit, &QPushButton::clicked, this, &MainForm::close);
    connect(btnFile, &QPushButton::clicked, this, &MainForm::openOutputDirectory);
    connect(btnAbout, &QPushButton::clicked, this, &MainForm::showAboutDialog);
    connect(btnKeywords, &QPushButton::clicked, this, &MainForm::showKeyDialog);
    connect(btnLog, &QPushButton::clicked, this, &MainForm::viewLogFile);
    connect(btnOptions, &QPushButton::clicked, this, &MainForm::showOptionsDialog);
    connect(btnStations, &QPushButton::clicked, this, &MainForm::showStationDialog);
    connect(btnTest, &QPushButton::clicked, this, &MainForm::testDownload);
    connect(btnTray, &QPushButton::clicked, this, &MainForm::minimizeToTray);
    connect(cBDataType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainForm::updateDataListWidget);
    connect(cBSubType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainForm::updateDataListWidget);
    connect(cBDirectory, &QComboBox::currentTextChanged, this, &MainForm::updateMessage);
    connect(cBLocalDirectory, &QCheckBox::clicked, this, &MainForm::localDirectoryCheckBoxClicked);
    connect(cBHidePasswd, &QCheckBox::clicked, this, &MainForm::updateEnable);
    connect(dataListWidget, &QListWidget::clicked, this, &MainForm::dataListSelectionChanged);
    connect(stationListWidget, &QListWidget::clicked, this, &MainForm::updateStationListLabel);
    connect(&trayIcon, &QSystemTrayIcon::activated, this, &MainForm::restoreFromTaskTray);
    connect(&busyTimer, &QTimer::timeout, this, &MainForm::busyTimerTriggered);
    connect(btnTimeStart, &QPushButton::clicked, this, &MainForm::showStartTimeDetails);
    connect(btnTimeStop, &QPushButton::clicked, this, &MainForm::showStopTimeDetails);
    connect(cBTimeInterval, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainForm::updateEnable);

    for (int i = 0; i < 8; i++) {
        images[i].load(QString(":/buttons/wait%1").arg(i + 1));
        images[i] = images[i].scaledToHeight(16);
    };

    timerCnt = 0;

    trayIcon.setIcon(QPixmap(":/icons/rtkget"));
    setWindowIcon(QIcon(":/icons/rtkget"));

    setAcceptDrops(true);
}
//---------------------------------------------------------------------------
void MainForm::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    QString appFilename = QApplication::applicationFilePath();
    QFileInfo fi(appFilename);

    iniFilename = fi.absoluteDir().filePath(fi.baseName()) + ".ini";

    setWindowTitle(QString("%1 v.%2 %3").arg(PRGNAME, VER_RTKLIB, PATCH_LEVEL));

    QCommandLineParser parser;
    parser.setApplicationDescription("RTK Get Qt");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption iniFileOption(QStringList() << "i" << "ini-file",
                     QCoreApplication::translate("main", "ini file to use"),
                     QCoreApplication::translate("main", "ini file"));
    parser.addOption(iniFileOption);

    QCommandLineOption titleOption(QStringList() << "t" << "title",
                       QCoreApplication::translate("main", "window title"),
                       QCoreApplication::translate("main", "title"));
    parser.addOption(titleOption);

    parser.process(*QApplication::instance());

    if (parser.isSet(iniFileOption))
        iniFilename = parser.value(iniFileOption);

    if (parser.isSet(titleOption))
        setWindowTitle(parser.value(titleOption));

    loadOptions();
    loadUrlList(urlFile);
    updateDataListWidget();
    updateEnable();

    if (traceLevel > 0) {
        traceopen(TRACE_FILE);
        tracelevel(traceLevel);
    }
}
//---------------------------------------------------------------------------
void MainForm::closeEvent(QCloseEvent *)
{
    traceclose();
    saveOptions();
}
//---------------------------------------------------------------------------
void MainForm::openOutputDirectory()
{
    QString str;
    gtime_t ts, te;
    double ti;
    char path[1024] = ".";

    str = cBLocalDirectory->isChecked() ? cBDirectory->currentText() : messageLabel2->text();
    getTime(&ts, &te, &ti);
    if (str != "") reppath(qPrintable(str), path, ts, "", "");

    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}
//---------------------------------------------------------------------------
void MainForm::viewLogFile()
{
    if (logFile == "") return;

    viewer->setWindowTitle(logFile);
    viewer->read(logFile);
    viewer->exec();
}
//---------------------------------------------------------------------------
void MainForm::testDownload()
{
    if (btnTest->text().remove('&') == tr("Abort")) {
        btnTest->setEnabled(false);
        abortf = 1;
        return;
    }
    
    processingThread = new DownloadThread(this, TEST_FILE, false, true);
    getTime(&processingThread->ts, &processingThread->te, &processingThread->ti);
    processingThread->nurl = selectUrl(processingThread->urls);
    if (timediff(processingThread->ts, processingThread->te) > 0.0 || processingThread->nurl <= 0) {
        messageLabel3->setText(tr("no local data"));
        return;
    }
    
    processingThread->nsta = selectStation(processingThread->stas);
    processingThread->columnCnt = columnCnt;
    processingThread->dateFormat = dateFormat;

    if (cBLocalDirectory->isChecked())
        strncpy(processingThread->dir, qPrintable(cBDirectory->currentText()), 1023);

    panelEnable(0);

    btnTest->setEnabled(true);
    btnTest->setText(tr("&Abort"));
    messageLabel1->setStyleSheet("QLabel { color: gray;}");
    messageLabel3->setText("");
    abortf = 0;
    
    connect(processingThread, &QThread::finished, this, &MainForm::downloadFinished);
    
    processingThread->start();
}
//---------------------------------------------------------------------------
void MainForm::showOptionsDialog()
{
    QString urlfile = urlFile;

    DownOptDialog downOptDialog(this);

    downOptDialog.exec();

    if (downOptDialog.result() != QDialog::Accepted) return;

    if (urlFile == urlfile) return;

    loadUrlList(urlFile);
    updateDataListWidget();
}
//---------------------------------------------------------------------------
void MainForm::download()
{
    QString str;

    if (btnDownload->text().remove('&') == tr("Abort")) {
        btnDownload->setEnabled(false);
        abortf = 1;
        return;
    }
    
    processingThread = new DownloadThread(this, logFile, logAppend, false);
    getTime(&processingThread->ts, &processingThread->te, &processingThread->ti);

    str = lENumber->text();
    QStringList tokens = str.split('-');
    if (tokens.size() == 2) {
        processingThread->seqnos = tokens.at(0).toInt();
        processingThread->seqnoe = tokens.at(1).toInt();
    } else if (tokens.size() == 1) {
        processingThread->seqnos = processingThread->seqnoe = tokens.at(0).toInt();
    } else {
        return;
    }
    
    processingThread->nurl = selectUrl(processingThread->urls);
    if (timediff(processingThread->ts, processingThread->te) > 0.0 || processingThread->nurl <= 0) {
        messageLabel3->setText(tr("No download data"));
        return;
    }
    
    processingThread->nsta = selectStation(processingThread->stas);
    processingThread->usr = lEFtpLogin->text();
    processingThread->pwd = lEFtpPasswd->text();
    processingThread->proxy = proxyAddr;
    
    if (!cBSkipExist->isChecked()) processingThread->opts |= DLOPT_FORCE;
    if (!cBUnzip->isChecked()) processingThread->opts |= DLOPT_KEEPCMP;
    if (holdErr) processingThread->opts |= DLOPT_HOLDERR;
    if (holdList) processingThread->opts |= DLOPT_HOLDLST;

    if (cBLocalDirectory->isChecked())
        strncpy(processingThread->dir, qPrintable(cBDirectory->currentText()), 1023);
    abortf = 0;
    panelEnable(0);
    btnDownload->setEnabled(true);
    btnDownload->setText(tr("&Abort"));
    messageLabel3->setText("");
    
    connect(processingThread, &QThread::finished, this, &MainForm::downloadFinished);

    busyTimer.start(200);
    
    processingThread->start();
}
//---------------------------------------------------------------------------
void MainForm::downloadFinished()
{
    panelEnable(1);
    updateEnable();
    busyTimer.stop();
    lbImage->setPixmap(QPixmap());

    if (cBDirectory->isEnabled())
        addHistory(cBDirectory);
    
    if (processingThread->test) {
        btnTest->setText(tr("&Test..."));
        messageLabel1->setStyleSheet("QLabel { color: bloack;}");
        messageLabel3->setText("");

        viewer->setOption(2);  // allow to save file
        viewer->read(TEST_FILE);
        viewer->setWindowTitle(tr("Local File Test"));
        viewer->exec();

        remove(TEST_FILE);
    } else {
        btnDownload->setText(tr("&Download"));
        messageLabel3->setText(processingThread->msg);
    }

    updateMessage();
    updateEnable();
    
    delete processingThread;
}

//---------------------------------------------------------------------------
void MainForm::showStationDialog()
{
    StaListDialog staListDialog(this);

    staListDialog.exec();

    if (staListDialog.result() != QDialog::Accepted) return;
    updateStationListLabel();
    btnAll->setText("&All");
}
//---------------------------------------------------------------------------
void MainForm::selectOutputDirectory()
{
    QString dir = cBDirectory->currentText();

    dir = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Output Directory"), dir));
    cBDirectory->insertItem(0, dir);
    cBDirectory->setCurrentIndex(0);
}
//---------------------------------------------------------------------------
void MainForm::showStartTimeDetails()
{
    QDateTime time(dateTimeStart->dateTime());
    gtime_t t1;

    t1.time = static_cast<time_t>(time.toSecsSinceEpoch());
    t1.sec = time.time().msec() / 1000;
    timeDialog->setTime(t1);
    timeDialog->exec();
}
//---------------------------------------------------------------------------
void MainForm::showStopTimeDetails()
{
    QDateTime time(dateTimeStop->dateTime());
    gtime_t t2;

    t2.time = static_cast<time_t>(time.toSecsSinceEpoch());
    t2.sec = time.time().msec() / 1000;
    timeDialog->setTime(t2);
    timeDialog->exec();
}
//---------------------------------------------------------------------------
void MainForm::SelectDeselectAllStations()
{
    int i, station_cnt = 0;

    for (i = stationListWidget->count() - 1; i >= 0; i--) {
        stationListWidget->item(i)->setSelected(btnAll->text() == tr("&All"));
        if (stationListWidget->item(i)->isSelected())
            station_cnt++;
    }

    btnAll->setText(btnAll->text() == tr("&All") ? tr("&Clear") : tr("&All"));
    lbStation->setText(QString(tr("Stations (%1)")).arg(station_cnt));
}
//---------------------------------------------------------------------------
void MainForm::showKeyDialog()
{
    KeyDialog keyDialog(this);

    keyDialog.setFlag(3);
    keyDialog.exec();
}
//---------------------------------------------------------------------------
void MainForm::showAboutDialog()
{
    AboutDialog aboutDialog(this, QPixmap(":/icons/rtkget"), PRGNAME);

    aboutDialog.exec();
}
//---------------------------------------------------------------------------
void MainForm::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}
//---------------------------------------------------------------------------
void MainForm::dropEvent(QDropEvent *event)
{
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    QPoint pos = event->position().toPoint();
#else
    QPoint pos = event->pos();
#endif
    if (stationListWidget == childAt(pos))
        loadStationFile(event->mimeData()->text());
    event->acceptProposedAction();
}
//---------------------------------------------------------------------------
void MainForm::minimizeToTray()
{
    setVisible(false);
    trayIcon.setVisible(true);
}
//---------------------------------------------------------------------------
void MainForm::restoreFromTaskTray(QSystemTrayIcon::ActivationReason reason)
{
    if (reason != QSystemTrayIcon::DoubleClick &&
            reason != QSystemTrayIcon::Trigger) return;

    setVisible(true);
    trayIcon.setVisible(false);
}
//---------------------------------------------------------------------------
void MainForm::localDirectoryCheckBoxClicked()
{
    updateMessage();
    updateEnable();
}
//---------------------------------------------------------------------------
void MainForm::dataListSelectionChanged()
{
    updateMessage();
    messageLabel3->setText("");
}
//---------------------------------------------------------------------------
void MainForm::busyTimerTriggered()
{
    lbImage->setPixmap(images[timerCnt % 8]);
    qApp->processEvents();
    timerCnt++;
}
//---------------------------------------------------------------------------
void MainForm::loadOptions()
{
    QSettings setting(iniFilename, QSettings::IniFormat);
    QStringList stas;

    dateTimeStart->setDate(setting.value("opt/startd", "2020/01/01").toDate());
    dateTimeStart->setTime(setting.value("opt/starth", "00:00").toTime());
    dateTimeStop->setDate(setting.value("opt/endd", "2020/01/01").toDate());
    dateTimeStop->setTime(setting.value("opt/endh", "00:00").toTime());
    cBTimeInterval->setCurrentText(setting.value("opt/timeint", "24 H").toString());
    lENumber->setText(setting.value("opt/number", 0).toString());
    urlFile = setting.value("opt/urlfile", "").toString();
    logFile = setting.value("opt/logfile", "").toString();
    stations = setting.value("opt/stations", "").toString();
    proxyAddr = setting.value("opt/proxyaddr", "").toString();
    lEFtpLogin->setText(setting.value("opt/login", "anonymous").toString());
    lEFtpPasswd->setText(setting.value("opt/passwd", "user@").toString());
    cBUnzip->setChecked(setting.value("opt/unzip", 1).toBool());
    cBSkipExist->setChecked(setting.value("opt/skipexist", 1).toBool());
    cBHidePasswd->setChecked(setting.value("opt/hidepasswd", 0).toBool());
    holdErr = setting.value("opt/holderr", 0).toInt();
    holdList = setting.value("opt/holdlist", 0).toInt();
    columnCnt = setting.value("opt/ncol", 35).toInt();
    logAppend = setting.value("opt/logappend", 0).toInt();
    dateFormat = setting.value("opt/dateformat", 0).toInt();
    traceLevel = setting.value("opt/tracelevel", 0).toInt();
    cBLocalDirectory->setChecked(setting.value("opt/localdirena", 0).toBool());

    stationListWidget->clear();

    for (int i = 0; i < 10; i++) {
        stas = setting.value(QString("sta/station%1").arg(i), "").toString().split(",");
        foreach(const QString &s, stas) {
            if (s != "")
                stationListWidget->addItem(s);
        }
    }
    readHistory(setting, "dir", cBDirectory);

    cBDirectory->insertItem(0, setting.value("opt/localdir", "").toString()); cBDirectory->setCurrentIndex(0);
    cBDataType->insertItem(0, setting.value("opt/datatype", "").toString()); cBDataType->setCurrentIndex(0);

    viewer->loadOptions(setting);
}
//---------------------------------------------------------------------------
void MainForm::saveOptions()
{
    QSettings setting(iniFilename, QSettings::IniFormat);

    setting.setValue("opt/startd", dateTimeStart->date());
    setting.setValue("opt/starth", dateTimeStart->time());
    setting.setValue("opt/endd", dateTimeStop->date());
    setting.setValue("opt/endh", dateTimeStop->time());
    setting.setValue("opt/timeint", cBTimeInterval->currentText());
    setting.setValue("opt/number", lENumber->text());
    setting.setValue("opt/urlfile", urlFile);
    setting.setValue("opt/logfile", logFile);
    setting.setValue("opt/stations", stations);
    setting.setValue("opt/proxyaddr", proxyAddr);
    setting.setValue("opt/login", lEFtpLogin->text());
    setting.setValue("opt/passwd", lEFtpPasswd->text());
    setting.setValue("opt/unzip", cBUnzip->isChecked());
    setting.setValue("opt/skipexist", cBSkipExist->isChecked());
    setting.setValue("opt/hidepasswd", cBHidePasswd->isChecked());
    setting.setValue("opt/holderr", holdErr);
    setting.setValue("opt/holdlist", holdList);
    setting.setValue("opt/ncol", columnCnt);
    setting.setValue("opt/logappend", logAppend);
    setting.setValue("opt/dateformat", dateFormat);
    setting.setValue("opt/tracelevel", traceLevel);
    setting.setValue("opt/localdirena", cBLocalDirectory->isChecked());
    setting.setValue("opt/localdir", cBDirectory->currentText());
    setting.setValue("opt/datatype", cBDataType->currentText());

    for (int i = 0, j = 0; i < 10; i++)
    {
        QString buffer;
        for (int k = 0; k < 256 && j < stationListWidget->count(); k++) {
            buffer.append(QStringLiteral("%1%2").arg(k == 0 ? "" : ",", stationListWidget->item(j++)->text()));
        };
        setting.setValue(QString("sta/station%1").arg(i), buffer);
    }

    writeHistory(setting, "dir", cBDirectory);

    viewer->saveOptions(setting);
}
//---------------------------------------------------------------------------
void MainForm::loadUrlList(QString file)
{
    url_t *urls_list;
    QString subtype, basetype;
    const char *sel[1];
    int i, n;

    sel[0] = (char*)"*";

    urls_list = new url_t [MAX_URL];

    types.clear();
    urls.clear();
    locals.clear();
    cBDataType->clear();
    cBSubType->clear();
    dataListWidget->clear();
    cBDataType->addItem(tr("ALL"));
    cBSubType->addItem("");

    if (file == "") file = URL_FILE; // default url

    n = dl_readurls(qPrintable(file), (char **)sel, 1, urls_list, MAX_URL);

    for (i = 0; i < n; i++) {
        int p;
        types.append(urls_list[i].type);
        urls.append(urls_list[i].path);
        locals.append(urls_list[i].dir);

        p = types.last().indexOf('_');
        if (p == -1) continue;
        basetype = types.last().mid(0, p);
        if (cBDataType->findText(basetype) == -1)
            cBDataType->addItem(basetype);

        subtype = types.last().mid(p + 1);
        if ((p = subtype.indexOf('_')) != -1) subtype = subtype.mid(0, p);

        if (cBSubType->findText(subtype) == -1)
            cBSubType->addItem(subtype);
    }
    cBDataType->setCurrentIndex(0);
    cBSubType->setCurrentIndex(0);

    delete [] urls_list;
}
//---------------------------------------------------------------------------
void MainForm::loadStationFile(QString file)
{
    QFile f(file);
    QByteArray buff;

    if (!f.open(QIODevice::ReadOnly)) return;

    stationListWidget->clear();

    while (!f.atEnd()) {
        buff = f.readLine();
        buff = buff.mid(buff.indexOf('#'));
        stationListWidget->addItem(buff);
    }

    updateStationListLabel();
    btnAll->setText("&All");
}
//---------------------------------------------------------------------------
void MainForm::getTime(gtime_t *ts, gtime_t *te, double *ti)
{
    QString interval_str;
    double eps[6] = { 2010, 1, 1 }, epe[6] = { 2010, 1, 1 }, val;

    eps[0] = dateTimeStart->date().year(); eps[1] = dateTimeStart->date().month(); eps[2] = dateTimeStart->date().day();
    eps[3] = dateTimeStart->time().hour(); eps[4] = dateTimeStart->time().minute(); eps[5] = 0;
    epe[0] = dateTimeStop->date().year(); epe[1] = dateTimeStop->date().month(); epe[2] = dateTimeStop->date().day();
    epe[3] = dateTimeStop->time().hour(); epe[4] = dateTimeStop->time().minute(); epe[5] = 0;

    *ts = epoch2time(eps);
    *te = epoch2time(epe);
    *ti = 86400.0;

    interval_str = cBTimeInterval->currentText();

    if (interval_str == "-") {
        *te=*ts;
    }
    QStringList tokens = interval_str.split(" ");

    if (tokens.size() == 0) return;

    val = tokens.at(0).toDouble();
    if (tokens.size() != 2) { // assume unit of hours
        *ti = val * 3600.0;
    } else {
        if (tokens.at(1) == tr("day")) *ti = val * 86400.0;
        else if (tokens.at(1) == tr("min")) *ti = val * 60.0;
        else *ti = val * 3600.0;
    }
}
//---------------------------------------------------------------------------
// prepare the selected urls for download
//
int MainForm::selectUrl(url_t *urls)
{
    QString str, file = urlFile;
    char *types[MAX_URL_SEL];
    int i, nurl = 0;

    for (i = 0; i < MAX_URL_SEL; i++) types[i] = new char [64];

    for (i = 0; i < dataListWidget->count() && nurl < MAX_URL_SEL; i++) {
        if (!dataListWidget->item(i)->isSelected()) continue;
        str = dataListWidget->item(i)->text();
        types[nurl++] = new char[str.length()];
        strncpy(types[nurl++], qPrintable(str), str.length());
    }
    if (urlFile == "") file = URL_FILE;

    nurl = dl_readurls(qPrintable(file), types, nurl, urls, MAX_URL_SEL);

    for (i = 0; i < MAX_URL_SEL; i++) delete [] types[i];

    return nurl;
}
//---------------------------------------------------------------------------
// prepare selected stations for download
//
int MainForm::selectStation(char **stas)
{
    QString str;
    int i, nsta = 0, len;

    for (i = 0; i < stationListWidget->count() && nsta < MAX_STA; i++) {
        if (!stationListWidget->item(i)->isSelected()) continue;
        str = stationListWidget->item(i)->text();
        len = str.length();
        if (str.indexOf(' ') != -1) len = str.indexOf(' ');
        if (len > 15) len = 15;
        strncpy(stas[nsta], qPrintable(str), static_cast<unsigned int>(len));
        stas[nsta++][len] = '\0';
    }
    return nsta;
}
//---------------------------------------------------------------------------
void MainForm::updateDataListWidget()
{
    QString str;
    QString type, subtype;
    int i;

    dataListWidget->clear();

    for (i = 0; i < types.size(); i++) {
        str = types.at(i);
        QStringList tokens = str.split('_');

        type = subtype = "";
        if (tokens.size() > 1) {
            type = tokens.at(0);
            subtype = tokens.at(1);
        } else {
            type = tokens.at(0);
        }

        if (cBDataType->currentText() != tr("ALL") && cBDataType->currentText() != type) continue;
        if (cBSubType->currentText() != "" && cBSubType->currentText() != subtype) continue;
        dataListWidget->addItem(types.at(i));
    }
    messageLabel1->setText("");
    messageLabel2->setText("");
}
//---------------------------------------------------------------------------
void MainForm::updateMessage()
{
    int i, j, n = 0;

    for (i = 0; i < dataListWidget->count(); i++) {
        if (!dataListWidget->item(i)->isSelected()) continue;
        for (j = 0; j < types.count(); j++) {
            if (dataListWidget->item(i)->text() != types.at(j)) continue;
            messageLabel1->setText(urls.at(j));
            messageLabel2->setText(cBLocalDirectory->isChecked() ? cBDirectory->currentText() : locals.at(j));
            messageLabel1->setToolTip(messageLabel1->text());
            messageLabel2->setToolTip(messageLabel2->text());
            n++;
            break;
        }
    }
    if (n >= 2) {
        messageLabel1->setText(messageLabel1->text() + " ...");
        if (!cBLocalDirectory->isChecked())
            messageLabel2->setText(messageLabel2->text() + " ...");
    }
}
//---------------------------------------------------------------------------
void MainForm::updateStationListLabel()
{
    int i, n = 0;

    for (i = 0; i < stationListWidget->count(); i++)
        if (stationListWidget->item(i)->isSelected()) n++;
    lbStation->setText(QString(tr("Stations (%1)")).arg(n));
}
//---------------------------------------------------------------------------
void MainForm::updateEnable()
{
    cBDirectory->setEnabled(cBLocalDirectory->isChecked());
    btnDir->setEnabled(cBLocalDirectory->isChecked());
    if (cBHidePasswd->isChecked())
        lEFtpPasswd->setEchoMode(QLineEdit::Password);
    else
        lEFtpPasswd->setEchoMode(QLineEdit::Normal);
    lbEnd->setEnabled(cBTimeInterval->currentText() != "-");
    dateTimeStop->setEnabled(cBTimeInterval->currentText() != "-");
    btnTimeStop->setEnabled(cBTimeInterval->currentText() != "-");
}
//---------------------------------------------------------------------------
void MainForm::panelEnable(int ena)
{
    timePanel->setEnabled(ena);
    stationPanel->setEnabled(ena);
    btnFile->setEnabled(ena);
    btnLog->setEnabled(ena);
    btnOptions->setEnabled(ena);
    btnTest->setEnabled(ena);
    btnDownload->setEnabled(ena);
    btnExit->setEnabled(ena);
}
// --------------------------------------------------------------------------
void MainForm::readHistory(QSettings &setting, QString key, QComboBox *combo)
{
    QString item;
    int i;

    combo->clear();

    for (i = 0; i < MAX_HIST; i++) {
        item = setting.value(QString("history/%1_%2").arg(key).arg(i, 3), "").toString();
        if (item != "") combo->addItem(item);
    }
}
// --------------------------------------------------------------------------
void MainForm::writeHistory(QSettings &setting, QString key, QComboBox *combo)
{
    int i;

    for (i = 0; i < combo->count(); i++)
        setting.setValue(QString("history/%1_%2").arg(key).arg(i), combo->itemText(i));
}
// --------------------------------------------------------------------------
void MainForm::addHistory(QComboBox *combo)
{
    QString hist = combo->currentText();

    if (hist == "") return;
    int i = combo->currentIndex();
    combo->removeItem(i);
    combo->insertItem(0, hist);
    for (int i = combo->count() - 1; i >= MAX_HIST; i--) combo->removeItem(i);
    combo->setCurrentIndex(0);
}
//---------------------------------------------------------------------------
int MainForm::execCommand(const QString &cmd, const QStringList &opt)
{
    return QProcess::startDetached(cmd, opt);
}
//---------------------------------------------------------------------------
