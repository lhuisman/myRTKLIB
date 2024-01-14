//---------------------------------------------------------------------------

#ifndef glofcndlgH
#define glofcndlgH
//---------------------------------------------------------------------------

#include <QDialog>
#include <QLineEdit>
#include "ui_glofcndlg.h"

//---------------------------------------------------------------------------
class GloFcnDialog : public QDialog, private Ui::GloFcnDialog
{
    Q_OBJECT

public slots:
    void readRinex();
    void clearFrequencies();
    void updateEnable(void);

private:
    QSpinBox * getFcn(int prn);

public:
    GloFcnDialog(QWidget *parent);

    int getGloFcnEnable();
    void setGloFcnEnable(int enable);
    int getGloFcn(int i);
    void setGloFcn(int i, int fcn);
};

//---------------------------------------------------------------------------
#endif
