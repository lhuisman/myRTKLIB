//---------------------------------------------------------------------------
#ifndef fileoptdlgH
#define fileoptdlgH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_fileoptdlg.h"

class KeyDialog;

//---------------------------------------------------------------------------
class FileOptDialog : public QDialog, private Ui::FileOptDialog
{
    Q_OBJECT

protected:
    KeyDialog *keyDialog;

    int options, pathEnabled;
public slots:
    void filePathSelect();
    void keyDialogShow();
    void updateEnable();

public:
    explicit FileOptDialog(QWidget* parent, int options = 0, int pathEnabled = 0);

    void setPath(QString path);
    QString getPath();
    void setOptions(int);
    int getOptions() {return options;}
    void setPathEnabled(int);
    int getPathEnabled() {return pathEnabled;}
};
//---------------------------------------------------------------------------
#endif
