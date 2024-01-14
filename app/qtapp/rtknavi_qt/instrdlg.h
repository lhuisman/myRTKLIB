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
    void  showStreamOptions1();
    void  showStreamOptions2();
    void  showStreamOptions3();
    void  showCommandDialog1();
    void  showCommandDialog2();
    void  showCommandDialog3();
    void  selectFile1();
    void  selectFile2();
    void  selectFile3();
    void  showReceiverOptions1();
    void  showReceiverOptions2();
    void  showReceiverOptions3();
    void  btnOkClicked();
    void  selectPosition();
    void  updateEnable();

private:
    QString  getFilePath(const QString &path);
    QString  setFilePath(const QString &path);
    void  showSerialOptionsDialog(int index, int opt);
    void  showTcpOptionsDialog(int index, int opt);
    void  showFtpOptionsDialog(int index, int opt);
    void showCommandDialog(int streamNo);
    void showReceiverOptionDialog(int streamNo);

    CmdOptDialog *cmdOptDialog;
    RcvOptDialog *rcvOptDialog;
    RefDialog *refDialog;
    SerialOptDialog *serialOptDialog;
    TcpOptDialog *tcpOptDialog;
    FtpOptDialog *ftpOptDialog;

    QString receiverOptions[3];
    QString history[10];
    int noFormats;
    QString stationPositionFile;
public:
    void setStreamEnabled(int stream, int enabled);
    int getStreamEnabled(int stream);

    void setStreamType(int stream, int type);
    int getStreamType(int stream);

    void setStreamFormat(int stream, int format);
    int getStreamFormat(int stream);

    void setReceiverOptions(int stream, const QString &options);
    const QString &getReceiverOptions(int stream);

    void setFilePath(int stream, const QString &path);
    const QString &getFilePath(int stream);

    void setResetCommand(const QString &command);
    QString getResetCommand();

    void setNMeaPosition(int i, double value);
    double getNMeaPosition(int i);

    void setNmeaRequestType(int);
    int getNmeaRequestType();

    void setMaxBaseLine(double baseline);
    double getMaxBaseLine();

    void setHistory(int i, const QString &history);
    const QString &getHistory(int i);

    void setTimeTagEnabled(bool);
    bool getTimeTagEnabled();

    void setTimeTag64bit(bool);
    bool getTimeTag64bit();

    void setTimeStart(const QString &time);
    QString getTimeStart();

    void setTimeSpeed(const QString &speed);
    QString getTimeSpeed();

    void setStationPositionFile(const QString &file);
    QString getStationPositionFile();

    int commandEnable[3][3], commandEnableTcp[3][3];
    QString paths[3][4], commands[3][3], commandsTcp[3][3];

    explicit InputStrDialog(QWidget* parent);
};
#endif
