//---------------------------------------------------------------------------

#ifndef launchoptdlgH
#define launchoptdlgH
//---------------------------------------------------------------------------
#include "ui_launchoptdlg.h"
#include <QDialog>

class QShowEvent;
//---------------------------------------------------------------------------
class LaunchOptDialog : public QDialog, private Ui::LaunchOptDialog
{
    Q_OBJECT

public:
    LaunchOptDialog(QWidget* parent);

    void setMinimize(int minimized);
    int getMinimize();
    void setOption(int option);
    int getOption();
};
//---------------------------------------------------------------------------
#endif
