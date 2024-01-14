//---------------------------------------------------------------------------
#ifndef maskoptdlgH
#define maskoptdlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "rtklib.h"

#include "ui_maskoptdlg.h"

//---------------------------------------------------------------------------
class MaskOptDialog : public QDialog, private Ui::MaskOptDialog
{
    Q_OBJECT

public slots:
    void  updateEnable(void);

public:
    explicit MaskOptDialog(QWidget* parent = nullptr);

    snrmask_t getSnrMask();
    void setSnrMask(snrmask_t);
};
//---------------------------------------------------------------------------
#endif
