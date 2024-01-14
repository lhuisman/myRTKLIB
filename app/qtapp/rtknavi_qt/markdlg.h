//---------------------------------------------------------------------------

#ifndef markdlgH
#define markdlgH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_markdlg.h"

class KeyDialog;

//---------------------------------------------------------------------------
class QMarkDialog : public QDialog, private Ui::MarkDialog
{
    Q_OBJECT
public slots:
    void saveClose();
    void showKeyDialog();
    void btnPositionClicked();

protected:
    int nMark;
    QString stationPositionFileF;
    int positionMode;
private:
    void updateEnable(void);
    KeyDialog *keyDialog;

public:

    void setPositionMode(int);
    int getPositionMode();

    void setName(const QString&);
    QString getName();

    void setComment(const QString&);
    QString getComment();

    void setStationPositionFile(const QString&);
    QString getStationPositionFile();

    explicit QMarkDialog(QWidget *parent);
	
};
//---------------------------------------------------------------------------
#endif
