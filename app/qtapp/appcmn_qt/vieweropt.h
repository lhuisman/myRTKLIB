//---------------------------------------------------------------------------

#ifndef vieweroptH
#define vieweroptH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_vieweropt.h"

//---------------------------------------------------------------------------
class ViewerOptDialog : public QDialog, private Ui::ViewerOptDialog
{
    Q_OBJECT

public slots:
    void selectTextColor();
    void selectBackgroundColor();
    void selectFont();

protected:
    QFont font;
    QColor colorText, colorBackground;
public:
    explicit ViewerOptDialog(QWidget* parent);

    QFont getFont() {return font;}
    void setFont(const QFont &font);
    const QColor& getTextColor() {return colorText;}
    void setTextColor(const QColor &);
    const QColor& getBackgroundColor() {return colorBackground;}
    void setBackgroundColor(const QColor &);
};
//---------------------------------------------------------------------------
#endif
