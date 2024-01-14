//---------------------------------------------------------------------------
#ifndef serioptdlgH
#define serioptdlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_serioptdlg.h"

class CmdOptDialog;
//---------------------------------------------------------------------------
class SerialOptDialog : public QDialog, private Ui::SerialOptDialog
{
    Q_OBJECT

protected:
    void updatePortList();

    CmdOptDialog *cmdOptDialog;

public slots:
    void updateEnable();

public:
    QString commands[2];
    int commandsEnabled[2];

    explicit SerialOptDialog(QWidget*, int options = 0);

    void setOptions(int options);
    QString getPath();
    void setPath(QString path);
};
//---------------------------------------------------------------------------
#endif
