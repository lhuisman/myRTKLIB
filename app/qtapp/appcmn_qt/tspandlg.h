//---------------------------------------------------------------------------
#ifndef tspandlgH
#define tspandlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_tspandlg.h"

#include "rtklib.h"

class QShowEvent;

//---------------------------------------------------------------------------
class SpanDialog : public QDialog, public Ui::SpanDialog
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent*);

public slots:
    void btnOkClicked();
    void btnTimeStartClicked();
    void btnTimeEndClicked();

private:
    void updateEnable(void);

public:
    int timeEnabled[3], timeValid[3];
    gtime_t timeStart, timeEnd;
    double timeInterval;

    explicit SpanDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
