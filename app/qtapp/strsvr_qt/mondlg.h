//---------------------------------------------------------------------------
#ifndef mondlgH
#define mondlgH
//---------------------------------------------------------------------------
#include <QDialog>
#include <QStringList>

#include "rtklib.h"

#include "ui_mondlg.h"

#define MAX_MSG_BUFF	4096

//---------------------------------------------------------------------------
class StrMonDialog : public QDialog, private Ui::StrMonDialog
{
    Q_OBJECT
public slots:
    void clearConsole();
    void scrollDown();
    void changeFormat();

protected:

private:
    QStringList consoleBuffer;
    rtcm_t rtcm;
    raw_t raw;

    void addConsole(unsigned char *msg, int len, int mode, bool newline);

public:
    explicit StrMonDialog(QWidget *parent);
    void addMessage(unsigned char *buff, int n);

    int getStreamFormat();
};
//---------------------------------------------------------------------------
#endif
