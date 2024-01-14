//---------------------------------------------------------------------------
#ifndef rcvoptdlgH
#define rcvoptdlgH
//---------------------------------------------------------------------------

#include <QDialog>
#include <ui_rcvoptdlg.h>
//---------------------------------------------------------------------------
class RcvOptDialog : public QDialog, private Ui::RcvOptDialog
{
    Q_OBJECT

public:
    explicit RcvOptDialog(QWidget* parent);

    QString getOptions();
    void setOptions(const QString &);
};
//---------------------------------------------------------------------------
#endif
