//---------------------------------------------------------------------------
#ifndef mntpoptdlgH
#define mntpoptdlgH
//---------------------------------------------------------------------------

#define MAXHIST		10
#include "ui_mntpoptdlg.h"


//---------------------------------------------------------------------------
class MntpOptDialog : public QDialog, private Ui::MntpOptDialog
{
    Q_OBJECT

public:
    MntpOptDialog(QWidget* parent = nullptr, int option = 0);

    QString getMountPoint();
    void setMountPoint(QString mountPoint);
    QString getMountPointString();
    void setMountPointString(QString mountPointString);

    void setOption(int option);
};

//---------------------------------------------------------------------------
#endif
