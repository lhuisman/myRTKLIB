//---------------------------------------------------------------------------
#ifndef startdlgH
#define startdlgH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_startdlg.h"

#include "rtklib.h"
class QShowEvent;

//---------------------------------------------------------------------------
class StartDialog : public QDialog, private Ui::StartDialog
{
    Q_OBJECT

protected:
    QString filename;

public slots:
    void setTimeFromFile();

public:

    explicit StartDialog(QWidget *parent = nullptr);

    gtime_t getTime();
    void setTime(gtime_t);
    QString getFileName() {return filename;}
    void setFileName(QString filename);
};
//---------------------------------------------------------------------------
#endif
