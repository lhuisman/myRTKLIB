//---------------------------------------------------------------------------

#ifndef plotoptH
#define plotoptH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_plotopt.h"

class Plot;
class RefDialog;

//---------------------------------------------------------------------------
class PlotOptDialog : public QDialog, private Ui::PlotOptDialog
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent*);

public slots:
    void btnOKClicked();
    void color1Select();
    void color2Select();
    void color3Select();
    void color4Select();
    void referencePositionSelect();
    void markerColorSelect();
    void fontSelect();
    void tleFileOpen();
    void shapeFileOpen();
    void tleSatelliteFileOpen();
    void tleFileView();
    void tleSatelliteFileView();

private:
    void updateFont(void);
    void updateEnable(void);
    QColor markerColor[2][8]; // {{mark1 0-7},{mark2 0-7}}
    QColor cColor[4];    // {background,grid,text,line}
    QFont fontOption;

public:
    Plot *plot;
    RefDialog *refDialog;

    explicit PlotOptDialog(QWidget *parent=NULL);
};
//---------------------------------------------------------------------------
#endif
