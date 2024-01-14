//---------------------------------------------------------------------------
#ifndef optdlgH
#define optdlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_convopt.h"

class QShowEvent;
class CodeOptDialog;
class GloFcnDialog;
class FreqDialog;

//---------------------------------------------------------------------------
class ConvOptDialog : public QDialog, private Ui::ConvOptDialog
{
    Q_OBJECT

public slots:
    void saveClose();
    void showMaskDialog();
    void showFrequencyDialog();
    void showFcnDialog();

protected:
    void showEvent(QShowEvent*);

private:
    void updateEnable(void);

    CodeOptDialog *codeOptDialog;
    GloFcnDialog *gloFcnDialog;
    FreqDialog *freqDialog;
public:
    QString codeMask[7];
	
    explicit ConvOptDialog(QWidget *parent);
};
//---------------------------------------------------------------------------
#endif
