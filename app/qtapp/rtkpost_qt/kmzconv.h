//---------------------------------------------------------------------------

#ifndef kmzconvH
#define kmzconvH
//---------------------------------------------------------------------------
#include <QDialog>
#include <QSettings>
#include "ui_kmzconv.h"

class TextViewer;
//---------------------------------------------------------------------------
class ConvDialog : public QDialog, public Ui::ConvDialog
{
    Q_OBJECT

public slots:
    void convert();
    void viewOutputFile();
    void selectInputFile();
    void selectGoogleEarthFile();
    void callGoogleEarth();
    void formatChanged();
    void updateEnable();

private:
    int execCommand(const QString &cmd, const QStringList &opt);
    void showMessage(const QString &msg);
    void updateOutputFile();

protected:
    TextViewer *viewer;

public:
    explicit ConvDialog(QWidget *parent);

    void setInput(const QString &File);

    void loadOptions(QSettings&);
    void saveOptions(QSettings&);
};
//---------------------------------------------------------------------------
#endif
