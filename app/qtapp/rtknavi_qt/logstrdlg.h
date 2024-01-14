//---------------------------------------------------------------------------
#ifndef logstrdlgH
#define logstrdlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_logstrdlg.h"

class KeyDialog;
class SerialOptDialog;
class TcpOptDialog;

//---------------------------------------------------------------------------
class LogStrDialog : public QDialog, private Ui::LogStrDialog
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent *);

    KeyDialog *keyDialog;
    SerialOptDialog *serialOptDialog;
    TcpOptDialog *tcpOptDialog;

public slots:
    void saveClose();
    void showStreamOptions1();
    void showStreamOptions2();
    void showStreamOptions3();
    void selectFile1();
    void selectFile2();
    void showKeyDialog();
    void selectFile3();
    void updateEnable();

private:
    QString getFilePath(const QString &path);
    QString setFilePath(const QString &path);

    void showSerialOptions(int index, int opt);
    void showTcpOptions(int index, int opt);

public:
    int streamEnabled[3], stream[3], logTimeTag, logAppend;
    QString paths[3][4], swapInterval;
    QString history[10];

    explicit LogStrDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
