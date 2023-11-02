//---------------------------------------------------------------------------
#ifndef instrdlgH
#define instrdlgH
//---------------------------------------------------------------------------

#include <QDialog>

#include "ui_instrdlg.h"

class CmdOptDialog;
class RcvOptDialog;
class RefDialog;
class SerialOptDialog;
class TcpOptDialog;
class FtpOptDialog;

//---------------------------------------------------------------------------
class InputStrDialog : public QDialog, private Ui::InputStrDialog
{
    Q_OBJECT

protected:
    void  showEvent(QShowEvent*);

public slots:
    void  btnStream1Clicked();
    void  btnStream2Clicked();
    void  btnStream3Clicked();
    void  btnCmd1Clicked();
    void  btnCmd2Clicked();
    void  btnCmd3Clicked();
    void  btnFile1Clicked();
    void  btnFile2Clicked();
    void  btnFile3Clicked();
    void  btnReceiverOptions1Clicked();
    void  btnReceiverOptions2Click();
    void  btnReceiverOptions3Click();
    void  btnOkClicked();
    void  btnPositionClicked();
    void  updateEnable(void);

private:
    QString  getFilePath(const QString &path);
    QString  setFilePath(const QString &path);
    void  serialOptions(int index, int opt);
    void  tcpOptions(int index, int opt);
    void  ftpOptions(int index, int opt);

    CmdOptDialog *cmdOptDialog;
    RcvOptDialog *rcvOptDialog;
    RefDialog *refDialog;
    SerialOptDialog *serialOptDialog;
    TcpOptDialog *tcpOptDialog;
    FtpOptDialog *ftpOptDialog;
public:
    bool streamC[3], timeTag;
    int stream[3], format[3], commandEnable[3][3], commandEnableTcp[3][3];
    int nmeaReq, NReceivers, time64Bit;
    double nmeaPosition[3], maxBaseLine;
    QString paths[3][4], commands[3][3], commandsTcp[3][3], timeStart, timeSpeed;
    QString receiverOptions[3], resetCommand;
    QString history[10];

    explicit InputStrDialog(QWidget* parent);
};
#endif
