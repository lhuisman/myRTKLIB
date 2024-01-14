//---------------------------------------------------------------------------
#ifndef codeoptH
#define codeoptH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_codeopt.h"

class QShowEvent;
class ConvOptDialog;

//---------------------------------------------------------------------------
class CodeOptDialog : public QDialog, private Ui::CodeOptDialog
{
    Q_OBJECT

public slots:
    void setUnsetAll();

protected:
    int navSystem, frequencyType;

private:
    void updateEnable(void);

public:
    CodeOptDialog(QWidget *parent);

    void setNavSystem(int navSystem);
    int getNavSystem() {return navSystem;}
    void setFrequencyType(int frequencyType);
    int getFrequencyType() {return frequencyType;}
    void setCodeMask(int i, const QString& mask);
    QString getCodeMask(int i);
};
//----------------------------------------------------------------------
#endif
