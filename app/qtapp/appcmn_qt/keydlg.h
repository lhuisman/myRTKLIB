//---------------------------------------------------------------------------

#ifndef keydlgH
#define keydlgH

#include <QDialog>

#include "ui_keydlg.h"

//---------------------------------------------------------------------------
class KeyDialog : public QDialog, private Ui::KeyDialog
{
    Q_OBJECT

protected:
public:

    explicit KeyDialog(QWidget* parent, int flags = 0);

    // 0 (default): hide station id key;
    // 1: show hour keys, hide station sequence keys
    // 2: show base station key, hide station sequence keys;
    // 3: show station sequence keys (no hour keys)
    void setFlag(int flag); };
//---------------------------------------------------------------------------
#endif
