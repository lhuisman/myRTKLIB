//---------------------------------------------------------------------------
#ifndef staoptdlgH
#define staoptdlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_staoptdlg.h"
//---------------------------------------------------------------------------
class StaListDialog : public QDialog, private Ui::StaListDialog
{
    Q_OBJECT

protected:
    void  showEvent(QShowEvent*);

public slots:
    void  loadFile();
    void  btnOkClicked();
    void  saveFile();

public:
    explicit StaListDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
