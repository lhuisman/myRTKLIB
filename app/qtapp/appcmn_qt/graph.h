//---------------------------------------------------------------------------
#ifndef graphH
#define graphH

#include <QString>
#include <QColor>
#include <QPoint>
#include <QLabel>

//---------------------------------------------------------------------------
class Graph // graph class
{
private:
        double autoTick(double scale);
        double autoTickTime(double scale);
        void drawBox(QPainter &c);
        void drawLabel(QPainter &c);
        void drawGrid(QPainter &c,double xt, double yt);
        void drawGridLabel(QPainter &c, double xt, double yt);
        void rotatePoint(QPoint *ps, int n, const QPoint &pc, int rot, QPoint *pr);
        int clipPoint(QPoint *p0, int area, QPoint *p1);

        QPoint p_;
        QColor color_;
        QPaintDevice *parent;
        int mark_, size_, rot_;
        int x, y, width, height;
        double xCenter, yCenter, xScale, yScale, xTick, yTick;

public:
        explicit Graph(QPaintDevice *parent);

        int isInArea(QPoint &p);
        int toPoint(const double &x, const double &y, QPoint &p);
        int onAxis(const QPoint &p);
        QString numText(double x, double dx);
        QString timeText(double x, double dx);
        void toPos(const QPoint &p, double &x, double &y);
        void setSize(int width, int height);
        void resize();
        void setPosition(const QPoint &p1, const QPoint &p2);
        void getPosition(QPoint &p1, QPoint &p2);
        void setCenter(double x, double y);
        void getCenter(double &x, double &y);
        void setRight(double x, double y);
        void getRight(double &x, double &y);
        void setScale(double xs, double ys);
        void getScale(double &xs, double &ys);
        void setLimits(const double *xl, const double *yl);
        void getLimits(double *xl, double *yl);
        void setTick(double xt, double yt);
        void getTick(double &xt, double &yt);
        void drawAxis(QPainter &c,int label, int glabel);
        void drawMark(QPainter &c,const QPoint &p, int mark, const QColor &color, int size, int rot);
        void drawMark(QPainter &c,double x, double y, int mark, const QColor &color, int size, int rot);
        void drawMark(QPainter &c,const QPoint &p, int mark, const QColor &color, const QColor &bgcolor, int size,int rot);
        void drawMark(QPainter &c,double x, double y, int mark, const QColor &color, const QColor &bgcolor,int size, int rot);
        void drawMarks(QPainter &c,const double *x, const double *y,const QVector<QColor> &colors, int n,
				   int mark, int size, int rot);
        void drawCircle(QPainter &c,const QPoint &p, const QColor &color, int rx, int ry, int style);
        void drawCircle(QPainter &c,double x, double y, const QColor &color, double rx, double ry, int style);
        void drawCircles(QPainter &c,int label);
        void drawText(QPainter &c,double x, double y, const QString &str, const QColor &color, int ha, int va, int rot);
        void drawText(QPainter &c,double x, double y, const QString &str, const QColor &color, int ha, int va, int rot, const QFont &font);
        void drawText(QPainter &c,const QPoint &p, const QString &str, const QColor &color, int ha, int va,int rot);
        void drawText(QPainter &c,const QPoint &p, const QString &str, const QColor &color, int ha, int va,int rot, const QFont &font);
        void drawText(QPainter &c,double x, double y, const QString &str, const QColor &color, const QColor &bgcolor,int ha, int va, int rot);
        void drawText(QPainter &c,const QPoint &p, const QString &str, const QColor &color, const QColor &bgcolor,int ha, int va, int rot);
        void drawPoly(QPainter &c,QPoint *p, int n, const QColor &color, int style);
        void drawPoly(QPainter &c,double *x, double *y, int n, const QColor &color, int style);
        void drawPolyline(QPainter &c,QPoint *p, int n);
        void drawPatch(QPainter &c,QPoint *p, int n, const QColor &color1, const QColor &color2, int style);
        void drawPatch(QPainter &c,double *x, double *y, int n, const QColor &color1, const QColor &color2, int style);
        void drawSkyPlot(QPainter &c,const QPoint &p, const QColor &color1, const QColor &color2, int size);
        void drawSkyPlot(QPainter &c,double x, double y, const QColor &color1, const QColor &color2, double size);
        void drawSkyPlot(QPainter &c,const QPoint &p, const QColor &color1, const QColor &color2, const QColor &bgcolor, int size);
        void drawSkyPlot(QPainter &c,double x, double y, const QColor &color1, const QColor &color2, const QColor &bgcolor,double size);

        int box, fit, xGrid, yGrid, xLPosition, yLPosition, week;
        QString title, xLabel, yLabel;
        QColor color[3];
};
#endif
