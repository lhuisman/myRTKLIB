//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include <QDialog>
#include <QShowEvent>
#include <QFont>
#include <QPalette>
#include <QColorDialog>
#include <QFontDialog>

#include "helper.h"
#include "viewer.h"
#include "vieweropt.h"

//---------------------------------------------------------------------------
ViewerOptDialog::ViewerOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ViewerOptDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ViewerOptDialog::reject);
    connect(btnFont, &QPushButton::clicked, this, &ViewerOptDialog::selectFont);
    connect(btnColorText, &QPushButton::clicked, this, &ViewerOptDialog::selectTextColor);
    connect(btnColorBackground, &QPushButton::clicked, this, &ViewerOptDialog::selectBackgroundColor);
}
//---------------------------------------------------------------------------
void ViewerOptDialog::setFont(const QFont &font)
{
    fontLabel->setFont(font);
    fontLabel->setText(QString("%1 %2 px").arg(font.family()).arg(font.pointSize()));
}
//---------------------------------------------------------------------------
void ViewerOptDialog::setTextColor(const QColor &color)
{
    colorText = color;
    lbColorText->setStyleSheet(QString("background-color: %1").arg(color2String(colorText)));
}
//---------------------------------------------------------------------------
void ViewerOptDialog::setBackgroundColor(const QColor &color)
{
    colorBackground = color;
    lbColorBackground->setStyleSheet(QString("background-color: %1").arg(color2String(colorBackground)));
}
//---------------------------------------------------------------------------
void ViewerOptDialog::selectTextColor()
{
    QColorDialog d;

    d.setCurrentColor(colorText);
    d.exec();
    colorText = d.selectedColor();

    lbColorText->setStyleSheet(QString("background-color: %1").arg(color2String(colorText)));
}
//---------------------------------------------------------------------------
void ViewerOptDialog::selectBackgroundColor()
{
    QColorDialog d;

    d.setCurrentColor(colorBackground);
    d.exec();
    colorBackground = d.selectedColor();

    lbColorBackground->setStyleSheet(QString("background-color: %1").arg(color2String(colorBackground)));
}
//---------------------------------------------------------------------------
void ViewerOptDialog::selectFont()
{
    QFontDialog d;

    d.setCurrentFont(font);
    d.exec();

    font = d.selectedFont();

    fontLabel->setFont(font);
    fontLabel->setText(font.family() + QString::number(font.pointSize()) + "pt");
}
//---------------------------------------------------------------------------
