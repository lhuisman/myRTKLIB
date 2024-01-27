//---------------------------------------------------------------------------
#ifndef getmainH
#define getmainH
//---------------------------------------------------------------------------
#include <QDialog>
#include <QSettings>
#include <QTimer>
#include <QPixmap>
#include <QSystemTrayIcon>

#define INHIBIT_RTK_LOCK_MACROS
#include "rtklib.h"
#include "ui_getmain.h"

class TextViewer;
class DownloadThread;
class TimeDialog;

//---------------------------------------------------------------------------
class MainForm : public QWidget, public Ui::MainForm
{
     Q_OBJECT

protected:
    void  closeEvent(QCloseEvent *);

    void  showEvent(QShowEvent*);

    void  dragEnterEvent(QDragEnterEvent *event);
    void  dropEvent(QDropEvent * event);

public slots:
    void  showOptionsDialog();
    void  viewLogFile();
    void  download();
    void  openOutputDirectory();
    void  dataListSelectionChanged();
    void  selectOutputDirectory();
    void  localDirectoryCheckBoxClicked();
    void  showStationDialog();
    void  showKeyDialog();
    void  showAboutDialog();
    void  busyTimerTriggered();
    void  minimizeToTray();
    void  restoreFromTaskTray(QSystemTrayIcon::ActivationReason);
    void  testDownload();
    void  SelectDeselectAllStations();
    void  downloadFinished();
    void  showStartTimeDetails();
    void  showStopTimeDetails();
    void  updateEnable();
    void  updateDataListWidget();
    void  updateMessage();

private:
    QStringList types;
    QStringList urls;
    QStringList locals;
    QPixmap images[8];
    QSystemTrayIcon trayIcon;
    DownloadThread *processingThread;
    TextViewer *viewer;
    TimeDialog *timeDialog;

    void  loadOptions();
    void  saveOptions();
    void  updateStationListLabel();
    void  panelEnable(int ena);
    void  getTime(gtime_t *ts, gtime_t *te, double *ti);
    int   selectUrl(url_t *urls);
    int   selectStation(char **stas);
    void  loadUrlList(QString file);
    void  loadStationFile(QString file);
    int   execCommand(const QString &cmd, const QStringList &opt);
    void  readHistory(QSettings &, QString key, QComboBox *);
    void  writeHistory(QSettings &, QString key, QComboBox *);
    void  addHistory(QComboBox *combo);
	
public:
    QString iniFilename;
    QString urlFile;
    QString logFile;
    QString stations;
    QString proxyAddr;
    int holdErr;
    int holdList;
    int columnCnt;
    int dateFormat;
    int traceLevel;
    int logAppend;
    int timerCnt;
    QTimer busyTimer;

    explicit MainForm(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
