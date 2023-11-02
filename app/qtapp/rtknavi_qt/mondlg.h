//---------------------------------------------------------------------------
#ifndef mondlgH
#define mondlgH
//---------------------------------------------------------------------------
#include <QStringList>
#include <QDialog>
#include <QTimer>

#include "rtklib.h"

#include "ui_mondlg.h"

//---------------------------------------------------------------------------
class MonitorDialog : public QDialog, private Ui::MonitorDialog
{
    Q_OBJECT
protected:
    void showEvent(QShowEvent*);
    void closeEvent(QCloseEvent*);

public slots:
    void btnCloseClicked();
    void btnClearClicked();
    void btnDownClicked();
    void TypeChange(int);
    void Timer1Timer();
    void Timer2Timer();
    void SelObsChange(int);
    void SelFmtChange(int);
    void SelStrChange();
    void SelStr2Change();

private:
    int typeF, consoleFormat, stream1, stream2, fontScale, observationMode;
    QStringList consoleBuffer;
    QStringList header;
	rtcm_t rtcm;
	raw_t raw;
    QTimer timer1,timer2;
	
    void clearTable(void);
    void setRtk(void);
    void setSat(void);
    void setEst(void);
    void setCov(void);
    void setObs(void);
    void setNav(void);
    void setGnav(void);
    void setStream(void);
    void setSbsMsg(void);
    void setSbsLong(void);
    void setSbsIono(void);
    void setSbsFast(void);
    void setSbsNav(void);
    void setIonUtc(void);
    void setRtcm(void);
    void setRtcmDgps(void);
    void setRtcmSsr(void);
    void setReferenceStation(void);
    void showRtk(void);
    void showSat(void);
    void showEst(void);
    void showCov(void);
    void showObs(void);
    void showNav(void);
    void showGnav(void);
    void showSbsMsg(void);
    void showIonUtc(void);
    void showStream(void);
    void showSbsLong(void);
    void showSbsIono(void);
    void showSbsFast(void);
    void showSbsNav(void);
    void showRtcm(void);
    void showRtcmDgps(void);
    void showRtcmSsr(void);
    void showReferenceStation(void);

    void addConsole(const unsigned char *msg, int n, int mode);
    void viewConsole(void);

public:
    explicit MonitorDialog(QWidget* parent);
    ~MonitorDialog();
};
//---------------------------------------------------------------------------
#endif
