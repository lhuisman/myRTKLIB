//---------------------------------------------------------------------------
#ifndef tcpoptdlgH
#define tcpoptdlgH
//---------------------------------------------------------------------------
#include <QDialog>

#define MAXHIST		10

namespace Ui {
class TcpOptDialog;
}

class MntpOptDialog;
class QComboBox;

//---------------------------------------------------------------------------
class TcpOptDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TcpOptDialog(QWidget* parent, int options = 0);
    void setOptions(int options);  // 0: TCP Server, 1: TCP Client, 2: NTRIP Server, 3: NTRIP Client, 4: NTRIP Caster Client, 5: NTRIP Caster Server, 6: UDP Server, 7: UDP Client,

    QString getPath();
    void setPath(QString path);
    QString* getHistory();
    void setHistory(QString history[], int size);

protected:
    MntpOptDialog *mntpOptDialog;
    QString history[MAXHIST];
    int showOptions;

protected slots:
    void btnOkClicked();
    void btnNtripClicked();
    void btnMountpointClicked();
    void btnBrowseClicked();

private:
    void addHistory(QComboBox *list, QString *hist);
    int  execCommand(const QString &cmd, const QStringList &opt, int show);
    Ui::TcpOptDialog *ui;

};
//---------------------------------------------------------------------------
#endif
