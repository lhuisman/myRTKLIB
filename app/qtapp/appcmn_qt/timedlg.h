//---------------------------------------------------------------------------
#ifndef timedlgH
#define timedlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "rtklib.h"

#include "ui_timedlg.h"

class QShowEvent;

//---------------------------------------------------------------------------
class TimeDialog : public QDialog, public Ui::TimeDialog
{
    Q_OBJECT

public:

    explicit TimeDialog(QWidget *parent = nullptr);
    void setTime(gtime_t time);
};
//---------------------------------------------------------------------------
#endif
