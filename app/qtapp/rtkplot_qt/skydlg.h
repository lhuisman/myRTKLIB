//---------------------------------------------------------------------------
#ifndef skydlgH
#define skydlgH
//---------------------------------------------------------------------------
#include "ui_skydlg.h"
#include <QDialog>

class QShowEvent;

//---------------------------------------------------------------------------
class SkyImgDialog : public QDialog, private Ui::SkyImgDialog
{
    Q_OBJECT
protected:
    void showEvent(QShowEvent*);

public slots:
    void btnSaveClicked();
    void btnLoadClicked();
    void btnGenMaskClicked();
    void skyDestCorrectionClicked();
    void skyBinarizeClicked();
    void updateEnable(void);

private:
    void updateSky(void);
	
public:
    explicit SkyImgDialog(QWidget *parent = NULL);
    void updateField(void);
};
//---------------------------------------------------------------------------
#endif
