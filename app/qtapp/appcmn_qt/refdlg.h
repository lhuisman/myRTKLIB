//---------------------------------------------------------------------------
#ifndef refdlgH
#define refdlgH
//---------------------------------------------------------------------------

#include <QDialog>

#include "ui_refdlg.h"

//---------------------------------------------------------------------------
class RefDialog : public QDialog, public Ui::RefDialog
{
    Q_OBJECT

protected:
    void  showEvent(QShowEvent *);

public slots:
    void  saveClose();
    void  stationSelected(int, int);
    void  loadStations();
    void  findList();

private:
    void  loadList(QString filename);
    void  loadSinex(QString filename);
    void  addReference(int n, double *pos, const QString code, const QString name);
    int   validReference();
    void  updateDistances();

    int options, fontScale;
    double roverPosition[3];
public:
    explicit RefDialog(QWidget* parent, int options = 0);

    void setRoverPosition(double lat, double lon, double h);
    double *getPosition();

    QString getStationId();
    QString getStationName();
    QString stationPositionFile;
};
//---------------------------------------------------------------------------
#endif
