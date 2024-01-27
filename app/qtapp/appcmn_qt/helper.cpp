#include "helper.h"
#include <QLabel>


QString color2String(const QColor &c)
{
    return QString("rgb(%1,%2,%3)").arg(c.red()).arg(c.green()).arg(c.blue());
}

void setLabelBackgroundColor(QLabel * label, const QColor &color)
{
    // use palett instead of stylesheet here for speed reasons
    if (true) {
        label->setAutoFillBackground(true);
        QPalette palette = label->palette();
        palette.setColor(label->backgroundRole(), color);
        label->setPalette(palette);
    } else
        label->setStyleSheet(QString("QLabel {background-color: %1}").arg(color2String(color)));
}
void setLabelTextColor(QLabel * label, const QColor &color)
{
    // use palett instead of stylesheet here for speed reasons
    if (true) {
        QPalette palette = label->palette();
        palette.setColor(label->foregroundRole(), color);
        label->setPalette(palette);
    } else
        label->setStyleSheet(QString("QLabel {color: %1}").arg(color2String(color)));
}
