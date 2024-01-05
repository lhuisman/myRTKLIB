//---------------------------------------------------------------------------
#ifndef mapoptdlgH
#define mapoptdlgH
//---------------------------------------------------------------------------
#include "ui_mapoptdlg.h"

#include <QDialog>

class QKeyEvent;
class QShowEvent;

//---------------------------------------------------------------------------
class MapOptDialog : public QDialog, private Ui::MapAreaDialog
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent*);

public slots:
    void btnSaveClicked();

    void btnCenterClicked();
    void scaleEqualClicked();

private slots:
    void updateMap();
    void updateEnable();
	
public:
    MapOptDialog(QWidget* parent);
    void updateField();
};

//---------------------------------------------------------------------------
#endif
