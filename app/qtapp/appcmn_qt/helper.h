#ifndef helperH
#define helperH

#include <QColor>
class QLabel;

QString color2String(const QColor &c);
void setLabelBackgroundColor(QLabel * label, const QColor &color);
void setLabelTextColor(QLabel * label, const QColor &color);

namespace Color {
static const QColor Orange(0x00,0xAA,0xFF);
static const QColor Aqua(0xff, 0xff, 0x00);
}

#endif
