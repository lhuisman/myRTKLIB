//---------------------------------------------------------------------------
#ifndef ftpoptdlgH
#define ftpoptdlgH
//---------------------------------------------------------------------------
#include "ui_ftpoptdlg.h"

#include <QDialog>

#define MAXHIST		10

class KeyDialog;

//---------------------------------------------------------------------------
class FtpOptDialog : public QDialog, private Ui::FtpOptDialog
{
    Q_OBJECT
public slots:
    void  saveClose();
    void  showKeyDialog();

private:
    void  addHistory(QComboBox *list, QString *hist);
    void  updateEnable(void);

    KeyDialog *keyDlg;
    int options;  // 0(default): FTP options; 1: HTTP options
    QString history[MAXHIST], MountpointHistory[MAXHIST];
public:
    explicit FtpOptDialog(QWidget *parent, int options = 0);

    int getOptions() {return options;}
    void setOptions(int);
    QString getPath();
    void setPath(QString);
};
//---------------------------------------------------------------------------
#endif
