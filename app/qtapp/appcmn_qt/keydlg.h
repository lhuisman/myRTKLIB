//---------------------------------------------------------------------------

#ifndef keydlgH
#define keydlgH

#include <QDialog>

#include "ui_keydlg.h"

//---------------------------------------------------------------------------
class KeyDialog : public QDialog, private Ui::KeyDialog
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent*);

public:
    int flag;

    explicit KeyDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
