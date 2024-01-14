//---------------------------------------------------------------------------
#ifndef aboutdlgH
#define aboutdlgH
//---------------------------------------------------------------------------
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QShowEvent>
#include <QPixmap>

#include "ui_aboutdlg.h"

//---------------------------------------------------------------------------
class AboutDialog : public QDialog, private Ui::AboutDlg
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent *);

    QPixmap icon;
    QString aboutString;
public:

    explicit AboutDialog(QWidget*, QPixmap icon, QString labelText);
};
#endif
