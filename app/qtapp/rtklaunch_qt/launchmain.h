//---------------------------------------------------------------------------
#ifndef launchmainH
#define launchmainH
//---------------------------------------------------------------------------
#include <QDialog>
#include <QSystemTrayIcon>
#include <QMenu>

#include "ui_launchmain.h"

class QCloseEvent;
class QCloseEvent;
class LaunchOptDialog;

//---------------------------------------------------------------------------
class MainForm : public QDialog, private Ui::MainForm
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent *);

public slots:
    void launchRTKPlot();
    void launchRTKConv();
    void launchStrSvr();
    void launchRTKPost();
    void launchSrcTblBrows();
    void launchRTKNavi();
    void launchRTKGet();
    void moveToTray();
    void showOptions();
    void restoreFromTaskTray(QSystemTrayIcon::ActivationReason);
    void expandWindow();

private:
    QString iniFile;
    QSystemTrayIcon trayIcon;
    QMenu *trayMenu;
    LaunchOptDialog *launchOptDlg;
    bool tray;
    
    int execCommand(const QString &cmd, const QStringList &opt);
    void updatePanel();

public:
    int option, minimize;
    explicit MainForm(QWidget *parent = 0);
};
//---------------------------------------------------------------------------
#endif
