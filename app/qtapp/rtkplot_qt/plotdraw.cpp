//---------------------------------------------------------------------------
// plotdraw.c: rtkplot draw functions
//---------------------------------------------------------------------------
#include <QColor>
#include <QPainter>
#include <QDebug>

#include "rtklib.h"
#include "plotmain.h"
#include "graph.h"
#include "mapview.h"


#define MS_FONT     "Consolas"      // monospace font name

#define COL_ELMASK  Qt::red
#define ATAN2(x, y)  ((x) * (x) + (y) * (y) > 1E-12 ? atan2(x, y) : 0.0)

// update plot --------------------------------------------------------------
void Plot::updatePlot(void)
{
    trace(3, "updatePlot\n");

    updateInfo();
    refresh();
}
// refresh plot -------------------------------------------------------------
void Plot::refresh(void)
{
    trace(3, "refresh\n");

    flush = 1;
    updateDisplay();
}
// draw plot ----------------------------------------------------------------
void Plot::updateDisplay(void)
{
    int level = dragInProgress ? 0 : 1;

    trace(3, "updateDisplay\n");

    if (flush) {
        buffer = QPixmap(lblDisplay->size());
        if (buffer.isNull()) return;
        buffer.fill(cColor[0]);

        QPainter c(&buffer);

        c.setFont(lblDisplay->font());
        c.setPen(cColor[0]);
        c.setBrush(cColor[0]);

        switch (plotType) {
            case  PLOT_TRK: drawTrack(c, level);   break;
            case  PLOT_SOLP: drawSolution(c, level, 0); break;
            case  PLOT_SOLV: drawSolution(c, level, 1); break;
            case  PLOT_SOLA: drawSolution(c, level, 2); break;
            case  PLOT_NSAT: drawNsat(c, level);   break;
            case  PLOT_OBS: drawObservation(c, level);   break;
            case  PLOT_SKY: drawSky(c, level);   break;
            case  PLOT_DOP: drawDop(c, level);   break;
            case  PLOT_RES: drawResidual(c, level);   break;
            case  PLOT_RESE: drawResidualE(c, level);   break;
            case  PLOT_SNR: drawSnr(c, level);   break;
            case  PLOT_SNRE: drawSnrE(c, level);   break;
            case  PLOT_MPS: drawMpSky(c, level);   break;
        }

        lblDisplay->setPixmap(buffer);
    }

    flush = 0;
}
// draw track-plot ----------------------------------------------------------
void Plot::drawTrack(QPainter &c, int level)
{
    QString header;
    TIMEPOS *pos, *pos1, *pos2, *vel;
    gtime_t time1 = { 0, 0 }, time2 = { 0, 0 };
    sol_t *sol;
    QPoint p1, p2;
    double xt, yt, sx, sy, opos[3], pnt[3], rr[3], enu[3]={}, cent[3];
    int sel = !btnSolution1->isChecked() && btnSolution2->isChecked() ? 1 : 0, p = 0;

    trace(3, "drawTrack: level=%d\n", level);

    if (btnShowTrack->isChecked() && btnFixCenter->isChecked()) {
        // set map position to center
        if (!btnSolution12->isChecked()) {
            pos = solutionToPosition(solutionData + sel, solutionIndex[sel], 0, 0);

            if (pos->n > 0) graphTrack->setCenter(pos->x[0], pos->y[0]);

            delete pos;
        } else {
            pos1 = solutionToPosition(solutionData, solutionIndex[0], 0, 0);
            pos2 = solutionToPosition(solutionData + 1, solutionIndex[1], 0, 0);
            pos = pos1->diff(pos2, 0);

            if (pos->n > 0) graphTrack->setCenter(pos->x[0], pos->y[0]);

            delete pos;
            delete pos1;
            delete pos2;
        }
    }
    if (!btnSolution12->isChecked() && btnShowImage->isChecked())  // background image
        drawTrackImage(c, level);

    if (btnShowMap->isChecked())  // map
        drawTrackPath(c, level);

    if (btnShowGrid->isChecked()) { // grid
        if (level) { // draw "+" at center
            graphTrack->getExtent(p1, p2);
            p1.setX((p1.x() + p2.x()) / 2);
            p1.setY((p1.y() + p2.y()) / 2);
            drawMark(graphTrack, c, p1, Graph::MarkerTypes::Plus, cColor[1], 20, 0);
        }
        if (showGridLabel >= 3) { // circles
            graphTrack->xLabelPosition = Graph::LabelPosition::Axis;
            graphTrack->yLabelPosition = Graph::LabelPosition::Axis;
            graphTrack->drawCircles(c, showGridLabel == 4);
        }
        else if (showGridLabel >= 1) { // grid
            graphTrack->xLabelPosition = Graph::LabelPosition::Inner;
            graphTrack->yLabelPosition = Graph::LabelPosition::InnerRot;
            graphTrack->drawAxis(c, showLabel, showGridLabel == 2);
        }
    }

    if (norm(originPosition, 3) > 0.0) {
        ecef2pos(originPosition, opos);
        header = tr("ORI = %1, %2 m").arg(latLonString(opos, 9)).arg(opos[2], 0, 'f', 4);
    }

    // draw tracks
    if (btnSolution1->isChecked()) {
        pos = solutionToPosition(solutionData, -1, cBQFlag->currentIndex(), 0);
        drawTrackPoint(c, pos, level, 0);
        if (btnShowMap->isChecked() && norm(solutionData[0].rb, 3) > 1E-3)
            drawTrackPosition(c, solutionData[0].rb, 0, 8, cColor[2], tr("Base Station 1"));
        drawTrackStat(c, pos, header, p++);
        header = "";
        delete pos;
    }

    if (btnSolution2->isChecked()) {
        pos = solutionToPosition(solutionData + 1, -1, cBQFlag->currentIndex(), 0);
        drawTrackPoint(c, pos, level, 1);
        if (btnShowMap->isChecked() && norm(solutionData[1].rb, 3) > 1E-3)
            drawTrackPosition(c, solutionData[1].rb, 0, 8, cColor[2], tr("Base Station 2"));
        drawTrackStat(c, pos, header, p++);
        delete pos;
    }

    if (btnSolution12->isChecked()) {
        pos1 = solutionToPosition(solutionData, -1, 0, 0);
        pos2 = solutionToPosition(solutionData + 1, -1, 0, 0);
        pos = pos1->diff(pos2, cBQFlag->currentIndex());
        drawTrackPoint(c, pos, level, 0);
        drawTrackStat(c, pos, "", p++);
        delete pos;
        delete pos1;
        delete pos2;
    }

    if (btnShowTrack->isChecked() && btnSolution1->isChecked()) {
        pos = solutionToPosition(solutionData, solutionIndex[0], 0, 0);

        if ((sol = getsol(solutionData, solutionIndex[0]))) time1 = sol->time;

        if (pos->n) {
            pos->n = 1;
            drawTrackError(c, pos, 0);
            graphTrack->toPoint(pos->x[0], pos->y[0], p1);
            graphTrack->drawMark(c, p1, Graph::MarkerTypes::Dot, cColor[0], markSize * 2 + 12, 0);
            graphTrack->drawMark(c, p1, Graph::MarkerTypes::Circle, cColor[2], markSize * 2 + 10, 0);
            graphTrack->drawMark(c, p1, Graph::MarkerTypes::Plus, cColor[2], markSize * 2 + 14, 0);
            graphTrack->drawMark(c, p1, Graph::MarkerTypes::Dot, cColor[2], markSize * 2 + 6, 0);
            graphTrack->drawMark(c, p1, Graph::MarkerTypes::Dot, markerColor[0][pos->q[0]], markSize * 2 + 4, 0);

            if (btnSolution2->isChecked()) {
                p1.rx() += markSize + 8;
                drawLabel(graphTrack, c, p1, "1", Graph::Alignment::Left, Graph::Alignment::Center);
            }
        }
        delete pos;
    }

    if (btnShowTrack->isChecked() && btnSolution2->isChecked()) {
        pos = solutionToPosition(solutionData + 1, solutionIndex[1], 0, 0);

        if ((sol = getsol(solutionData + 1, solutionIndex[1]))) time2 = sol->time;

        if (pos->n > 0 && (time1.time == 0 || fabs(timediff(time1, time2)) < DTTOL * 2.0)) {
            pos->n = 1;
            drawTrackError(c, pos, 1);
            graphTrack->toPoint(pos->x[0], pos->y[0], p1);
            graphTrack->drawMark(c, p1, Graph::MarkerTypes::Dot, cColor[0], markSize * 2 + 12, 0);
            graphTrack->drawMark(c, p1, Graph::MarkerTypes::Circle, cColor[1], markSize * 2 + 10, 0);
            graphTrack->drawMark(c, p1, Graph::MarkerTypes::Plus, cColor[1], markSize * 2 + 14, 0);
            graphTrack->drawMark(c, p1, Graph::MarkerTypes::Dot, cColor[2], markSize * 2 + 6, 0);
            graphTrack->drawMark(c, p1, Graph::MarkerTypes::Dot, markerColor[1][pos->q[0]], markSize * 2 + 4, 0);

            if (btnSolution1->isChecked()) {
                p1.setX(p1.x() + markSize + 8);
                drawLabel(graphTrack, c, p1, "2", Graph::Alignment::Left, Graph::Alignment::Center);
            }
        }
        delete pos;
    }
    if (btnShowTrack->isChecked() && btnSolution12->isChecked()) {
        pos1 = solutionToPosition(solutionData, solutionIndex[0], 0, 0);
        pos2 = solutionToPosition(solutionData + 1, solutionIndex[1], 0, 0);
        pos = pos1->diff(pos2, 0);

        if (pos->n > 0) {
            pos->n = 1;
            drawTrackError(c, pos, 1);
            graphTrack->toPoint(pos->x[0], pos->y[0], p1);
            graphTrack->drawMark(c, p1, Graph::MarkerTypes::Dot, cColor[0], markSize * 2 + 12, 0);
            graphTrack->drawMark(c, p1, Graph::MarkerTypes::Circle, cColor[2], markSize * 2 + 10, 0);
            graphTrack->drawMark(c, p1, Graph::MarkerTypes::Plus, cColor[2], markSize * 2 + 14, 0);
            graphTrack->drawMark(c, p1, Graph::MarkerTypes::Dot, cColor[2], markSize * 2 + 6, 0);
            graphTrack->drawMark(c, p1, Graph::MarkerTypes::Dot, markerColor[0][pos->q[0]], markSize * 2 + 4, 0);
        }
        delete pos;
        delete pos1;
        delete pos2;
    }
    if (level && btnShowMap->isChecked()) {
        for (int i = 0; i < nWayPoint; i++) {
            if (i == selectedWayPoint) continue;

            pnt[0] = pointPosition[i][0] * D2R;
            pnt[1] = pointPosition[i][1] * D2R;
            pnt[2] = pointPosition[i][2];
            pos2ecef(pnt, rr);
            drawTrackPosition(c, rr, 0, 8, cColor[2], pointName[i]);
        }
        if (selectedWayPoint >= 0) { // highlight selected waypoint
            pnt[0] = pointPosition[selectedWayPoint][0] * D2R;
            pnt[1] = pointPosition[selectedWayPoint][1] * D2R;
            pnt[2] = pointPosition[selectedWayPoint][2];
            pos2ecef(pnt, rr);
            drawTrackPosition(c,rr, 0, 16, Qt::red, pointName[selectedWayPoint]);
        }
    }

    if (showCompass) {
        graphTrack->getExtent(p1, p2);
        p1.rx() += SIZE_COMP / 2 + 25;
        p1.ry() += SIZE_COMP / 2 + 35;
        drawMark(graphTrack, c, p1, Graph::MarkerTypes::Compass, cColor[2], SIZE_COMP, 0);
    }

    if (showArrow && btnShowTrack->isChecked()) {
        vel = solutionToPosition(solutionData + sel, solutionIndex[sel], 0, 1);
        drawTrackVelocity(c, vel);
        delete vel;
    }

    if (showScale) {
        QString label;
        graphTrack->getExtent(p1, p2);
        graphTrack->getTick(xt, yt);
        graphTrack->getScale(sx, sy);
        p2.rx() -= 70;
        p2.ry() -= 25;
        drawMark(graphTrack, c, p2, Graph::MarkerTypes::HScale, cColor[2], static_cast<int>(xt / sx + 0.5), 0);
        p2.ry() -= 3;
        if (xt < 0.0099)
            label = QString("%1 mm").arg(xt * 1000.0, 0, 'f', 0);
        else if (xt < 0.999)
            label = QString("%1 cm").arg(xt * 100.0, 0, 'f', 0);
        else if (xt < 999.0)
            label = QString("%1 m").arg(xt, 0, 'f', 0);
        else
            label = QString("%1 km").arg(xt / 1000.0, 0, 'f', 0);
        drawLabel(graphTrack, c, p2, label, Graph::Alignment::Center, Graph::Alignment::Bottom);
    }

    if (!level) { // "+" at center
        graphTrack->getCenter(xt, yt);
        graphTrack->toPoint(xt, yt, p1);
        drawMark(graphTrack, c, p1, Graph::MarkerTypes::Plus, cColor[2], 20, 0);
    }

    // update map center
    if (level) {
        if (norm(originPosition, 3) > 0.0) {
            graphTrack->getCenter(xt, yt);
            graphTrack->toPoint(xt, yt, p1);
            graphTrack->toPos(p1, enu[0], enu[1]);
            ecef2pos(originPosition, opos);
            enu2ecef(opos, enu, rr);
            for (int i = 0; i < 3; i++) rr[i] += originPosition[i];
            ecef2pos(rr, cent);
            mapView->setCenter(cent[0] * R2D, cent[1] * R2D);
        }
        refresh_MapView();
    }
}
// draw map-image on track-plot ---------------------------------------------
void Plot::drawTrackImage(QPainter &c, int level)
{
    gtime_t time = { 0, 0 };
    QPoint p1, p2;
    double pos[3] = { 0 }, rr[3], xyz[3] = { 0 }, x1, x2, y1, y2;

    trace(3, "drawTrackImage: level=%d\n", level);

    pos[0] = mapLatitude * D2R;
    pos[1] = mapLongitude * D2R;
    pos2ecef(pos, rr);
    if (norm(originPosition, 3) > 0.0)
        positionToXyz(time, rr, 0, xyz);
    x1 = xyz[0] - mapSize[0] * 0.5 * mapScaleX;
    x2 = xyz[0] + mapSize[0] * 0.5 * mapScaleX;
    y1 = xyz[1] - mapSize[1] * 0.5 * (mapScaleEqual ? mapScaleX : mapScaleY);
    y2 = xyz[1] + mapSize[1] * 0.5 * (mapScaleEqual ? mapScaleX : mapScaleY);

    graphTrack->toPoint(x1, y2, p1);
    graphTrack->toPoint(x2, y1, p2);
    QRect r(p1, p2);
    c.drawImage(r, mapImage);
}
// check in boundary --------------------------------------------------------
#define P_IN_B(pos, bound) \
    (pos[0] >= bound[0] && pos[0] <= bound[1] && pos[1] >= bound[2] && pos[1] <= bound[3])

#define B_IN_B(bound1, bound2) \
    (bound1[0] <= bound2[1] && bound1[1] >= bound2[0] && bound1[2] <= bound2[3] && bound1[3] >= bound2[2])

// draw gis-map on track-plot ----------------------------------------------
void Plot::drawTrackPath(QPainter &c, int level)
{
    gisd_t *data;
    gis_pnt_t *pnt;
    gis_poly_t *poly;
    gis_polygon_t *polygon;
    gtime_t time = { 0, 0 };
    QColor color;
    QPoint *p, p1;
    double xyz[3], S, xl[2], yl[2], enu[8][3] = { { 0 } }, opos[3], pos[3], rr[3];
    double bound[4] = { PI / 2.0, -PI / 2.0, PI, -PI };
    int i, j, n, m;

    trace(3, "drawTrackPath: level=%d\n", level);

    // get map boundary
    graphTrack->getLimits(xl, yl);
    enu[0][0] = xl[0]; enu[0][1] = yl[0];
    enu[1][0] = xl[1]; enu[1][1] = yl[0];
    enu[2][0] = xl[0]; enu[2][1] = yl[1];
    enu[3][0] = xl[1]; enu[3][1] = yl[1];
    enu[4][0] = (xl[0] + xl[1]) / 2.0; enu[4][1] = yl[0];
    enu[5][0] = (xl[0] + xl[1]) / 2.0; enu[5][1] = yl[1];
    enu[6][0] = xl[0]; enu[6][1] = (yl[0] + yl[1]) / 2.0;
    enu[7][0] = xl[1]; enu[7][1] = (yl[0] + yl[1]) / 2.0;

    ecef2pos(originPosition, opos);

    for (i = 0; i < 8; i++) {
        if (norm(enu[i], 2) >= 1000000.0) {
            bound[0] =-PI / 2.0;
            bound[1] = PI / 2.0;
            bound[2] =-PI;
            bound[3] = PI;
            break;
        }
        enu2ecef(opos, enu[i], rr);
        for (j = 0; j < 3; j++) rr[j] += originPosition[j];
        ecef2pos(rr, pos);
        if (pos[0] < bound[0]) bound[0] = pos[0];       // min lat
        if (pos[0] > bound[1]) bound[1] = pos[0];       // max lat
        if (pos[1] < bound[2]) bound[2] = pos[1];       // min lon
        if (pos[1] > bound[3]) bound[3] = pos[1];       // max lon
    }

    for (i = MAXMAPLAYER - 1; i >= 0; i--) {
        if (!gis.flag[i]) continue;
        for (data = gis.data[i]; data; data = data->next) {
            if (data->type == 1) { // point
                pnt = static_cast<gis_pnt_t *>(data->data);
                if (!P_IN_B(pnt->pos, bound)) continue;
                positionToXyz(time, pnt->pos, 0, xyz);
                if (xyz[2] < -RE_WGS84) continue;
                graphTrack->toPoint(xyz[0], xyz[1], p1);
                drawMark(graphTrack, c, p1, Graph::MarkerTypes::Circle, cColor[2], 6, 0);
                drawMark(graphTrack, c, p1, Graph::MarkerTypes::Dot, cColor[2], 2, 0);
            } else if (level && data->type == 2) { // polyline
                poly = static_cast<gis_poly_t *>(data->data);
                if ((n = poly->npnt) <= 0 || !B_IN_B(poly->bound, bound))
                    continue;
                p = new QPoint [n];
                for (j = m = 0; j < n; j++) {
                    positionToXyz(time, poly->pos + j * 3, 0, xyz);
                    if (xyz[2] < -RE_WGS84) {
                        if (m > 1) {
                            graphTrack->drawPoly(c, p, m, mapColor[i], 0);
                            m = 0;
                        }
                        continue;
                    }
                    graphTrack->toPoint(xyz[0], xyz[1], p1);
                    if (m == 0 || p1.x() != p[m - 1].x() || p1.y() != p[m - 1].y())
                        p[m++] = p1;
                }
                graphTrack->drawPoly(c, p, m, mapColor[i], 0);
                delete [] p;
            } else if (level && data->type == 3) { // polygon
                polygon = (gis_polygon_t *)data->data;
                if ((n = polygon->npnt) <= 0 || !B_IN_B(polygon->bound, bound))
                    continue;
                p = new QPoint [n];
                for (j = m = 0; j < n; j++) {
                    positionToXyz(time, polygon->pos + j * 3, 0, xyz);
                    if (xyz[2] < -RE_WGS84) {
                        continue;
                    }
                    graphTrack->toPoint(xyz[0], xyz[1], p1);
                    if (m == 0 || p1.x() != p[m - 1].x() || p1.y() != p[m - 1].y())
                        p[m++] = p1;
                }
                // judge hole
                for (j = 0, S = 0.0; j < m - 1; j++)
                    S += static_cast<double>(p[j].x() * p[j + 1].y() - p[j + 1].x() * p[j].y());
                color = S < 0.0 ? cColor[0] : mapColorF[i];
                graphTrack->drawPatch(c, p, m, mapColor[i], color, 0);
                delete [] p;
            }
        }
    }
}
// draw track-points on track-plot ------------------------------------------
void Plot::drawTrackPoint(QPainter &c, const TIMEPOS *pos, int level, int style)
{
    int i;

    trace(3, "drawTrackPoint: level=%d style=%d\n", level, style);

    if (level) drawTrackArrow(c, pos);

    if (level && plotStyle <= 1 && !btnShowTrack->isChecked()) // error circle
        drawTrackError(c, pos, style);

    if (!(plotStyle % 2))
        graphTrack->drawPoly(c, pos->x, pos->y, pos->n, cColor[3], style);

    if (level && plotStyle < 2) {
        QColor* color = new QColor[pos->n];
        if (btnShowImage->isChecked()) {
            for (i = 0; i < pos->n; i++) color[i] = cColor[0];
            graphTrack->drawMarks(c, pos->x, pos->y, color, pos->n, Graph::MarkerTypes::Dot, markSize + 2, 0);
        }

        for (i = 0; i < pos->n; i++) color[i] = markerColor[style][pos->q[i]];
        graphTrack->drawMarks(c, pos->x, pos->y, color, pos->n, Graph::MarkerTypes::Dot, markSize, 0);
        delete[] color;
    }

}
// draw point with label on track-plot --------------------------------------
void Plot::drawTrackPosition(QPainter &c, const double *rr, int type, int siz,
              QColor color, const QString &label)
{
    gtime_t time = { 0, 0 };
    QPoint p1;
    double xyz[3], xs, ys;

    trace(3,"drawTrackPosition: type=%d rr=%.3f %.3f %.3f\n", type, rr[0], rr[1], rr[2]);

    if (norm(rr, 3) > 0.0) {
        graphTrack->getScale(xs, ys);
        positionToXyz(time, rr, type, xyz);
        graphTrack->toPoint(xyz[0], xyz[1], p1);
        drawMark(graphTrack, c, p1, Graph::MarkerTypes::Plus, color, siz + 6, 0);
        drawMark(graphTrack, c, p1, Graph::MarkerTypes::Circle, color, siz, 0);
        drawMark(graphTrack, c, p1, Graph::MarkerTypes::Circle, color, siz - 6, 0);
        p1.ry() += 10;
        drawLabel(graphTrack, c, p1, label, Graph::Alignment::Center, Graph::Alignment::Top);
    }
}
// draw statistics on track-plot --------------------------------------------
void Plot::drawTrackStat(QPainter &c, const TIMEPOS *pos, const QString &header, int p)
{
    QString s[6];
    QPoint p1, p2;
    double *d, ave[4], std[4], rms[4];
    int i, n = 0, fonth = (int)(QFontMetrics(lblDisplay->font()).height() * 1.5);

    trace(3, "drawTrackStat: p=%d\n", p);

    if (!showStats) return;

    if (p == 0 && header != "") s[n++] = header;

    if (pos->n > 0) {
        d = new double[pos->n];

        for (i = 0; i < pos->n; i++)
            d[i] = SQRT(SQR(pos->x[i]) + SQR(pos->y[i]));

        calcStats(pos->x, pos->n, 0.0, ave[0], std[0], rms[0]);
        calcStats(pos->y, pos->n, 0.0, ave[1], std[1], rms[1]);
        calcStats(pos->z, pos->n, 0.0, ave[2], std[2], rms[2]);
        calcStats(d, pos->n, 0.0, ave[3], std[3], rms[3]);

        s[n++] = QString("AVE = E:%1 m, N:%2 m, U:%3 m").arg(ave[0], 7, 'f', 4).arg(ave[1], 7, 'f', 4).arg(ave[2], 7, 'f', 4);
        s[n++] = QString("STD = E:%1 m, N:%2 m, U:%3 m").arg(std[0], 7, 'f', 4).arg(std[1], 7, 'f', 4).arg(std[2], 7, 'f', 4);
        s[n++] = QString("RMS = E:%1 m, N:%2 m, U:%3 m, 2D:%4 m").arg(rms[0], 7, 'f', 4).arg(rms[1], 7, 'f', 4).arg(rms[2], 7, 'f', 4).arg(2.0 * rms[3], 7, 'f', 4);

        delete [] d;
    }
    graphTrack->getExtent(p1, p2);
    p1.rx() = p2.x() - 10;
    p1.ry() += 8 + fonth * 4 * p;

    for (i = 0; i < n; i++, p1.ry() += fonth)
        drawLabel(graphTrack, c, p1, s[i], Graph::Alignment::Right, Graph::Alignment::Top);
}
// draw error-circle on track-plot ------------------------------------------
void Plot::drawTrackError(QPainter &c, const TIMEPOS *pos, int style)
{
    static const double sint[36] = {  // sinus table
        0.0000,	 0.1736,  0.3420,  0.5000,  0.6428,  0.7660,  0.8660,  0.9397,	0.9848,
        1.0000,	 0.9848,  0.9397,  0.8660,  0.7660,  0.6428,  0.5000,  0.3420,	0.1736,
        0.0000,	 -0.1736, -0.3420, -0.5000, -0.6428, -0.7660, -0.8660, -0.9397, -0.9848,
        -1.0000, -0.9848, -0.9397, -0.8660, -0.7660, -0.6428, -0.5000, -0.3420, -0.1736
    };
    double xc[37], yc[37], a, b, s, cc;
    int i, j;

    trace(3, "drawTrackError: style=%d\n", style);

    if (!showError) return;

    for (i = 0; i < pos->n; i++) {
        if (pos->xs[i] <= 0.0 || pos->ys[i] <= 0.0) continue;

        a = pos->xys[i] / SQRT(pos->xs[i]);

        if ((b = pos->ys[i] - a * a) >= 0.0) b = SQRT(b);
        else continue;

        for (j = 0; j < 37; j++) {
            s = sint[j % 36];
            cc = sint[(45 - j) % 36];
            xc[j] = pos->x[i] + SQRT(pos->xs[i]) * cc;
            yc[j] = pos->y[i] + a * cc + b * s;
        }
        graphTrack->drawPoly(c, xc, yc, 37, cColor[1], showError == 1 ? 0 : 1);
    }
}
// draw direction-arrow on track-plot ---------------------------------------
void Plot::drawTrackArrow(QPainter &c, const TIMEPOS *pos)
{
    QPoint p;
    double tt, d[2], dist, dt, vel;
    int i, off = 8;

    trace(3, "drawTrackArrow\n");

    if (!showArrow) return;

    for (i = 1; i < pos->n - 1; i++) {
        tt = time2gpst(pos->t[i], NULL);
        d[0] = pos->x[i + 1] - pos->x[i - 1];
        d[1] = pos->y[i + 1] - pos->y[i - 1];
        dist = norm(d, 2);
        dt = timediff(pos->t[i + 1], pos->t[i - 1]);
        vel = dt == 0.0 ? 0.0 : dist / dt;

        if (vel < 0.5 || fmod(tt + 0.005, INTARROW) >= 0.01) continue;

        graphTrack->toPoint(pos->x[i], pos->y[i], p);
        p.rx() -= static_cast<int>(off * d[1] / dist);
        p.ry() -= static_cast<int>(off * d[0] / dist);
        drawMark(graphTrack, c, p, Graph::MarkerTypes::Arrow, cColor[3], 15, static_cast<int>(ATAN2(d[1], d[0]) * R2D));
    }
}
// draw velocity-indicator on track-plot ------------------------------------
void Plot::drawTrackVelocity(QPainter &c, const TIMEPOS *vel)
{
    QString label;
    QPoint p1, p2;
    double v = 0.0, dir = 0.0;

    trace(3, "drawTrackVelocity\n");

    if (vel && vel->n > 0) {
        if ((v = SQRT(SQR(vel->x[0]) + SQR(vel->y[0]))) > 1.0)
            dir = ATAN2(vel->x[0], vel->y[0]) * R2D;
    }

    graphTrack->getExtent(p1, p2);
    p1.rx() += SIZE_VELC / 2 + 30;
    p1.ry() = p2.y() - SIZE_VELC / 2 - 30;
    drawMark(graphTrack, c, p1, Graph::MarkerTypes::Circle, cColor[2], SIZE_VELC, 0);

    p1.ry() += SIZE_VELC / 2;
    label = QString("%1 km/h").arg(v * 3600.0 / 1000.0, 0, 'f', 0);
    drawLabel(graphTrack, c, p1, label, Graph::Alignment::Center, Graph::Alignment::Top);

    p1.ry() -= SIZE_VELC / 2;
    if (v >= 1.0) drawMark(graphTrack, c, p1, Graph::MarkerTypes::Arrow, cColor[2], SIZE_VELC, 90 - static_cast<int>(dir));
    drawMark(graphTrack, c, p1, Graph::MarkerTypes::Dot, cColor[0], 8, 0);
    drawMark(graphTrack, c, p1, Graph::MarkerTypes::Circle, cColor[2], 8, 0);
}
// draw solution-plot -------------------------------------------------------
void Plot::drawSolution(QPainter &c, int level, int type)
{
    QString label[] = { tr("E-W"), tr("N-S"), tr("U-D") }, unit[] = { "m", "m/s", QString("m/s%1").arg(up2Char) };
    QPushButton *btn[] = { btnOn1, btnOn2, btnOn3 };
    TIMEPOS *pos, *pos1, *pos2;
    gtime_t time1 = { 0, 0 }, time2 = { 0, 0 };
    QPoint p1, p2;
    double xc, yc, xl[2], yl[2], off, y;
    int i, j, k, p = 0;
    int sel = !btnSolution1->isChecked() && btnSolution2->isChecked() ? 1 : 0;

    trace(3, "drawSolution: level=%d\n", level);

    if (btnShowTrack->isChecked() && (btnFixHorizontal->isChecked() || btnFixVertical->isChecked())) {
        pos = solutionToPosition(solutionData + sel, solutionIndex[sel], 0, type);

        for (i = 0; i < 3 && pos->n > 0; i++) {
            graphTriple[i]->getCenter(xc, yc);

            if (btnFixVertical->isChecked())
                yc = i == 0 ? pos->x[0] : (i == 1 ? pos->y[0] : pos->z[0]);

            if (btnFixHorizontal->isChecked()) {
                graphTriple[i]->getLimits(xl, yl);
                off = centX * (xl[1] - xl[0]) / 2.0;
                graphTriple[i]->setCenter(timePosition(pos->t[0]) - off, yc);
            } else {
                graphTriple[i]->setCenter(xc, yc);
            }
        }
        delete pos;
    }
    j = -1;

    for (i = 0; i < 3; i++)
        if (btn[i]->isChecked()) j = i;

    for (i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;
        graphTriple[i]->xLabelPosition = timeFormat ? (i == j ? Graph::LabelPosition::Time : Graph::LabelPosition::None) : \
                                        (i == j ? Graph::LabelPosition::Outer : Graph::LabelPosition::On);
        graphTriple[i]->week = week;
        graphTriple[i]->drawAxis(c, showLabel, showLabel);
    }

    if (btnSolution1->isChecked()) {
        pos = solutionToPosition(solutionData, -1, cBQFlag->currentIndex(), type);
        drawSolutionPoint(c, pos, level, 0);
        drawSolutionStat(c, pos, unit[type], p++);

        delete pos;
    }

    if (btnSolution2->isChecked()) {
        pos = solutionToPosition(solutionData + 1, -1, cBQFlag->currentIndex(), type);
        drawSolutionPoint(c, pos, level, 1);
        drawSolutionStat(c, pos, unit[type], p++);

        delete pos;
    }

    if (btnSolution12->isChecked()) {
        pos1 = solutionToPosition(solutionData, -1, 0, type);
        pos2 = solutionToPosition(solutionData + 1, -1, 0, type);
        pos = pos1->diff(pos2, cBQFlag->currentIndex());
        drawSolutionPoint(c, pos, level, 0);
        drawSolutionStat(c, pos, unit[type], p++);

        delete pos;
        delete pos1;
        delete pos2;
    }

    if (btnShowTrack->isChecked() && (btnSolution1->isChecked() || btnSolution2->isChecked() || btnSolution12->isChecked())) {
        pos = solutionToPosition(solutionData + sel, solutionIndex[sel], 0, type);
        pos1 = solutionToPosition(solutionData, solutionIndex[0], 0, type);
        pos2 = solutionToPosition(solutionData + 1, solutionIndex[1], 0, type);
        if (pos1->n > 0) time1 = pos1->t[0];
        if (pos2->n > 0) time2 = pos2->t[0];

        for (j = k = 0; j < 3 && pos->n > 0; j++) {
            if (!btn[j]->isChecked()) continue;

            graphTriple[j]->getLimits(xl, yl);
            xl[0] = xl[1] = timePosition(pos->t[0]);
            graphTriple[j]->drawPoly(c, xl, yl, 2, cColor[2], 0);

            if (btnSolution2->isChecked() && pos2->n > 0 && (time1.time == 0 || fabs(timediff(time1, time2)) < DTTOL * 2.0)) {
                xl[0] = xl[1] = timePosition(time2);
                y = j == 0 ? pos2->x[0] : (j == 1 ? pos2->y[0] : pos2->z[0]);

                graphTriple[j]->drawMark(c, xl[0], y, Graph::MarkerTypes::Dot, cColor[0], markSize * 2 + 6, 0);
                graphTriple[j]->drawMark(c, xl[0], y, Graph::MarkerTypes::Circle, cColor[1], markSize * 2 + 6, 0);
                graphTriple[j]->drawMark(c, xl[0], y, Graph::MarkerTypes::Circle, cColor[2], markSize * 2 + 2, 0);
                graphTriple[j]->drawMark(c, xl[0], y, Graph::MarkerTypes::Dot, markerColor[1][pos->q[0]], markSize * 2, 0);

                if (btnSolution1->isChecked() && pos1->n > 0 && graphTriple[j]->toPoint(xl[0], y, p1)) {
                    p1.rx() += markSize + 4;
                    drawLabel(graphTriple[j], c, p1, "2", Graph::Alignment::Left, Graph::Alignment::Center);
                }
            }
            if (btnSolution1->isChecked() && pos1->n > 0) {
                xl[0] = xl[1] = timePosition(time1);
                y = j == 0 ? pos1->x[0] : (j == 1 ? pos1->y[0] : pos1->z[0]);

                graphTriple[j]->drawMark(c, xl[0], y, Graph::MarkerTypes::Dot, cColor[0], markSize * 2 + 6, 0);
                graphTriple[j]->drawMark(c, xl[0], y, Graph::MarkerTypes::Circle, cColor[2], markSize * 2 + 6, 0);
                graphTriple[j]->drawMark(c, xl[0], y, Graph::MarkerTypes::Circle, cColor[2], markSize * 2 + 2, 0);
                graphTriple[j]->drawMark(c, xl[0], y, Graph::MarkerTypes::Dot, markerColor[0][pos->q[0]], markSize * 2, 0);

                if (btnSolution2->isChecked() && pos2->n > 0 && graphTriple[j]->toPoint(xl[0], y, p1)) {
                    p1.rx() += markSize + 4;
                    drawLabel(graphTriple[j], c, p1, "1", Graph::Alignment::Left, Graph::Alignment::Center);
                }
            }
            xl[0] = xl[1] = timePosition(pos->t[0]);
            if (k++ == 0) {
                graphTriple[j]->drawMark(c, xl[0], yl[1] - 1E-6, Graph::MarkerTypes::Dot, cColor[2], 5, 0);

                if (!btnFixHorizontal->isChecked())
                    graphTriple[j]->drawMark(c, xl[0], yl[1] - 1E-6, Graph::MarkerTypes::Circle, cColor[2], 9, 0);
            }
        }

        delete pos;
        delete pos1;
        delete pos2;
    }
    for (i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;
        graphTriple[i]->getExtent(p1, p2);
        p1.rx() += 5;
        p1.ry() += 3;
        drawLabel(graphTriple[i], c, p1, label[i] + " (" + unit[type] + ")", Graph::Alignment::Left, Graph::Alignment::Top);
    }
}
// draw points and line on solution-plot ------------------------------------
void Plot::drawSolutionPoint(QPainter &c, const TIMEPOS *pos, int level, int style)
{
    QPushButton *btn[] = { btnOn1, btnOn2, btnOn3 };
    double *x, *y, *s, xs, ys, *yy;
    int i, j;

    trace(3, "drawSolutionPoint: level=%d style=%d\n", level, style);

    x = new double [pos->n];

    for (i = 0; i < pos->n; i++)
        x[i] = timePosition(pos->t[i]);

    for (i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;

        y = i == 0 ? pos->x : (i == 1 ? pos->y : pos->z);
        s = i == 0 ? pos->xs : (i == 1 ? pos->ys : pos->zs);

        if (!level || !(plotStyle % 2))
            drawPolyS(graphTriple[i], c, x, y, pos->n, cColor[3], style);

        if (level && showError && plotType <= PLOT_SOLA && plotStyle < 2) {
            graphTriple[i]->getScale(xs, ys);

            if (showError == 1) {
                for (j = 0; j < pos->n; j++)
                    graphTriple[i]->drawMark(c, x[j], y[j], Graph::MarkerTypes::VScale, cColor[1], static_cast<int>(SQRT(s[j]) * 2.0 / ys), 0);
            } else {
                yy = new double [pos->n];

                for (j = 0; j < pos->n; j++) yy[j] = y[j] - SQRT(s[j]);
                drawPolyS(graphTriple[i], c, x, yy, pos->n, cColor[1], 1);

                for (j = 0; j < pos->n; j++) yy[j] = y[j] + SQRT(s[j]);
                drawPolyS(graphTriple[i], c, x, yy, pos->n, cColor[1], 1);

                delete [] yy;
            }
        }
        if (level && plotStyle < 2) {
            QColor * color = new QColor[pos->n];
            for (j = 0; j < pos->n; j++)
                color[i] = markerColor[style][pos->q[j]];
            graphTriple[i]->drawMarks(c, x, y, color, pos->n, Graph::MarkerTypes::Dot, markSize, 0);
            delete[] color;
        }
    }
    delete [] x;
}
// draw statistics on solution-plot -----------------------------------------
void Plot::drawSolutionStat(QPainter &c, const TIMEPOS *pos, const QString &unit, int p)
{
    QPushButton *btn[] = { btnOn1, btnOn2, btnOn3 };
    QPoint p1, p2;
    double ave, std, rms, *y, opos[3];
    int i, j = 0, k = 0, fonth = (int)(QFontMetrics(lblDisplay->font()).height() * 1.5);
    QString label, s;

    trace(3, "drawSolutionStat: p=%d\n", p);

    if (!showStats || pos->n <= 0) return;

    for (i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;

        y = i == 0 ? pos->x : (i == 1 ? pos->y : pos->z);
        calcStats(y, pos->n, 0.0, ave, std, rms);
        graphTriple[i]->getExtent(p1, p2);
        p1.rx() = p2.x() - 5;
        p1.ry() += 3 + fonth * (p + (!k++ && p > 0 ? 1 : 0));

        if (j == 0 && p == 0) {
            if (norm(originPosition, 3) > 0.0) {
                ecef2pos(originPosition, opos);
                label = QString("ORI = %1, %2 m").arg(latLonString(opos, 9) ).arg(opos[2], 0, 'f', 4);
                drawLabel(graphTriple[j], c, p1, label, Graph::Alignment::Right, Graph::Alignment::Top);
                j++; p1.ry() += fonth;
            }
        }
        s = QString("AVE = %1 %2, STD = %3 %2, RMS = %4 %2").arg(ave, 0, 'f', 4).arg(unit).arg(std, 0, 'f', 4).arg(rms, 0, 'f', 4);
        drawLabel(graphTriple[i], c, p1, s, Graph::Alignment::Right, Graph::Alignment::Top);
    }
}
// draw number-of-satellite plot --------------------------------------------
void Plot::drawNsat(QPainter &c, int level)
{
    QString label[] = {
        tr("# of Valid Satellites"),
        tr("Age of Differential (s)"),
        tr("Ratio Factor for AR Validation")
    };
    QPushButton *btn[] = { btnOn1, btnOn2, btnOn3 };
    TIMEPOS *ns;
    QPoint p1, p2;
    double xc, yc, y, xl[2], yl[2], off;
    int i, j, k, sel = !btnSolution1->isChecked() && btnSolution2->isChecked() ? 1 : 0;

    trace(3, "drawNsat: level=%d\n", level);

    if (btnShowTrack->isChecked() && btnFixHorizontal->isChecked()) {
        ns = solutionToNsat(solutionData + sel, solutionIndex[sel], 0);

        for (i = 0; i < 3; i++) {
            if (btnFixHorizontal->isChecked()) {
                graphTriple[i]->getLimits(xl, yl);
                off = centX * (xl[1] - xl[0]) / 2.0;
                graphTriple[i]->getCenter(xc, yc);
                graphTriple[i]->setCenter(timePosition(ns->t[0]) - off, yc);
            } else {
                graphTriple[i]->getRight(xc, yc);
                graphTriple[i]->setRight(timePosition(ns->t[0]), yc);
            }
        }
        delete ns;
    }
    j = -1;
    for (i = 0; i < 3; i++) if (btn[i]->isChecked()) j = i;
    for (i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;
        graphTriple[i]->xLabelPosition = timeFormat ? (i == j ? 6 : 5) : (i == j ? 1 : 0);
        graphTriple[i]->week = week;
        graphTriple[i]->drawAxis(c, showLabel, showLabel);
    }

    if (btnSolution1->isChecked()) {
        ns = solutionToNsat(solutionData, -1, cBQFlag->currentIndex());
        drawSolutionPoint(c, ns, level, 0);
        delete ns;
    }

    if (btnSolution2->isChecked()) {
        ns = solutionToNsat(solutionData + 1, -1, cBQFlag->currentIndex());
        drawSolutionPoint(c, ns, level, 1);
        delete ns;
    }

    if (btnShowTrack->isChecked() && (btnSolution1->isChecked() || btnSolution2->isChecked())) {
        ns = solutionToNsat(solutionData + sel, solutionIndex[sel], 0);

        for (j = k = 0; j < 3 && ns->n > 0; j++) {
            if (!btn[j]->isChecked()) continue;

            y = j == 0 ? ns->x[0] : (j == 1 ? ns->y[0] : ns->z[0]);
            graphTriple[j]->getLimits(xl, yl);
            xl[0] = xl[1] = timePosition(ns->t[0]);

            graphTriple[j]->drawPoly(c, xl, yl, 2, cColor[2], 0);
            graphTriple[j]->drawMark(c, xl[0], y, Graph::MarkerTypes::Dot, cColor[0], markSize * 2 + 6, 0);
            graphTriple[j]->drawMark(c, xl[0], y, Graph::MarkerTypes::Circle, cColor[2], markSize * 2 + 6, 0);
            graphTriple[j]->drawMark(c, xl[0], y, Graph::MarkerTypes::Circle, cColor[2], markSize * 2 + 2, 0);
            graphTriple[j]->drawMark(c, xl[0], y, Graph::MarkerTypes::Dot, markerColor[sel][ns->q[0]], markSize * 2, 0);

            if (k++ == 0) {
                graphTriple[j]->drawMark(c, xl[0], yl[1] - 1E-6, Graph::MarkerTypes::Dot, cColor[2], 5, 0);

                if (!btnFixHorizontal->isChecked())
                    graphTriple[j]->drawMark(c, xl[0], yl[1] - 1E-6, Graph::MarkerTypes::Circle, cColor[2], 9, 0);
            }
        }
        delete ns;
    }
    for (i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;
        graphTriple[i]->getExtent(p1, p2);
        p1.rx() += 5;
        p1.ry() += 3;
        drawLabel(graphTriple[i], c, p1, label[i], Graph::Alignment::Left, Graph::Alignment::Top);
    }
}
// draw observation-data-plot -----------------------------------------------
void Plot::drawObservation(QPainter &c, int level)
{
    QPoint p1, p2, p;
    gtime_t time;
    obsd_t *obs;
    double xs, ys, xt, xl[2], yl[2], tt[MAXSAT] = { 0 }, xp, xc, yc, yp[MAXSAT] = { 0 };
    int i, j, nSats = 0, sats[MAXSAT] = { 0 }, ind = observationIndex;
    char id[16];

    trace(3, "drawObservation: level=%d\n", level);

    for (i = 0; i < observation.n; i++) {
        if (satelliteMask[observation.data[i].sat - 1]) continue;
        sats[observation.data[i].sat - 1] = 1;
    }
    for (i = 0; i < MAXSAT; i++)
        if (sats[i]) nSats++;
    
    graphSingle->xLabelPosition = timeFormat ? Graph::LabelPosition::Time : Graph::LabelPosition::Outer;
    graphSingle->yLabelPosition = Graph::LabelPosition::On;
    graphSingle->week = week;
    graphSingle->getLimits(xl, yl);
    yl[0] = 0.5;
    yl[1] = nSats > 0 ? nSats + 0.5 : nSats + 10.5;
    graphSingle->setLimits(xl, yl);
    graphSingle->setTick(0.0, 1.0);

    if (0 <= ind && ind < nObservation && btnShowTrack->isChecked() && btnFixHorizontal->isChecked()) {
        xp = timePosition(observation.data[indexObservation[ind]].time);
        if (btnFixHorizontal->isChecked()) {
            double xl[2], yl[2], off;
            graphSingle->getLimits(xl, yl);
            off = centX * (xl[1] - xl[0]) / 2.0;
            graphSingle->getCenter(xc, yc);
            graphSingle->setCenter(xp - off, yc);
        } else {
            graphSingle->getRight(xc, yc);
            graphSingle->setRight(xp, yc);
        }
    }
    graphSingle->drawAxis(c, 1, 1);
    graphSingle->getExtent(p1, p2);

    for (i = 0, j = 0; i < MAXSAT; i++) {
        if (!sats[i]) continue;
        p.setX(p1.x());
        p.setY(p1.y() + static_cast<int>((p2.y() - p1.y()) * (j + 0.5) / nSats));
        yp[i] = nSats - (j++);
        satno2id(i + 1, id);
        graphSingle->drawText(c, p, id, cColor[2], Graph::Alignment::Right, Graph::Alignment::Center, 0);
    }
    p1.setX((int)(QFontMetrics(lblDisplay->font()).height()));
    p1.setY((p1.y() + p2.y()) / 2);
    graphSingle->drawText(c, p1, tr("SATELLITE NO"), cColor[2], Graph::Alignment::Center, Graph::Alignment::Center, 90);

    if (!btnSolution1->isChecked()) return;

    if (level && plotStyle <= 2)
        drawObservationEphemeris(c, yp);

    if (level && plotStyle <= 2) {
        graphSingle->getScale(xs, ys);
        for (i = 0; i < observation.n; i++) {
            obs = &observation.data[i];
            QColor col = observationColor(obs, azimuth[i], elevation[i]);
            if (col == Qt::black) continue;

            xt = timePosition(obs->time);
            if (fabs(xt - tt[obs->sat - 1]) / xs > 0.9) {
                graphSingle->drawMark(c, xt, yp[obs->sat - 1], Graph::MarkerTypes::Dot, plotStyle < 2 ? col : cColor[3],
                         plotStyle < 2 ? markSize : 0, 0);
                tt[obs->sat - 1] = xt;
            }
        }
    }
    if (level && plotStyle <= 2)
        drawObservationSlips(c, yp);

    if (btnShowTrack->isChecked() && 0 <= ind && ind < nObservation) {
        i = indexObservation[ind];
        time = observation.data[i].time;

        graphSingle->getLimits(xl, yl);
        xl[0] = xl[1] = timePosition(observation.data[i].time);
        graphSingle->drawPoly(c, xl, yl, 2, cColor[2], 0);

        for (; i < observation.n && timediff(observation.data[i].time, time) == 0.0; i++) {
            obs = &observation.data[i];
            QColor col = observationColor(obs, azimuth[i], elevation[i]);
            if (col == Qt::black) continue;

            graphSingle->drawMark(c, xl[0], yp[obs->sat - 1], Graph::MarkerTypes::Dot, col, markSize * 2 + 2, 0);
        }

        graphSingle->drawMark(c, xl[0], yl[1] - 1E-6, Graph::MarkerTypes::Dot, cColor[2], 5, 0);
        if (!btnFixHorizontal->isChecked())
            graphSingle->drawMark(c, xl[0], yl[1] - 1E-6, Graph::MarkerTypes::Circle, cColor[2], 9, 0);
    }
}
// generate observation-data-slips ---------------------------------------
void Plot::generateObservationSlips(int *LLI)
{
    QString obstype = cBObservationType->currentText();
    bool ok;
    double gfp[MAXSAT][NFREQ+NEXOBS]={{0}};

    for (int i = 0; i < observation.n; i++) {
        obsd_t *obs = observation.data + i;
        int j, k;

        LLI[i] = 0;
        if (elevation[i] < elevationMask * D2R || !satelliteSelection[obs->sat - 1]) continue;
        if (elevationMaskEnabled && elevation[i] < elevationMaskData[(int)(azimuth[i] * R2D + 0.5)]) continue;

        if (showSlip == 1) { // LG jump
            double freq1, freq2, gf;

            if (obstype == "ALL") {
                if ((freq1 = sat2freq(obs->sat, obs->code[0], &navigation)) == 0.0) continue;
                LLI[i] = obs->LLI[0] & 2;
                for (j = 1; j < NFREQ + NEXOBS; j++) {
                    LLI[i] |= obs->LLI[j] & 2;
                    if ((freq2 = sat2freq(obs->sat, obs->code[j], &navigation)) == 0.0) continue;
                    gf = CLIGHT * (obs->L[0] / freq1-obs->L[j] / freq2); // geometry-free
                    if (fabs(gfp[obs->sat-1][j] - gf) > THRESLIP) LLI[i] |= 1;
                    gfp[obs->sat-1][j] = gf;
                }
            }
            else {
                k = obstype.mid(1).toInt(&ok);
                if (ok) {
                    j = k > 2 ? k - 3 : k - 1;  /* L1,L2,L5,L6 ... */
                }
                else {
                    for (j = 0; j < NFREQ + NEXOBS; j++) {
                        if (!strcmp(code2obs(obs->code[j]), qPrintable(obstype))) break;
                    }
                    if (j >= NFREQ + NEXOBS) continue;
                }
                LLI[i] = obs->LLI[j] & 2;
                k = (j == 0) ? 1 : 0;
                if ((freq1 = sat2freq(obs->sat, obs->code[k], &navigation)) == 0.0) continue;
                if ((freq2 = sat2freq(obs->sat, obs->code[j], &navigation)) == 0.0) continue;
                gf = CLIGHT * (obs->L[k] / freq1-obs->L[j] / freq2);
                if (fabs(gfp[obs->sat-1][j] - gf) > THRESLIP) LLI[i] |= 1;
                gfp[obs->sat-1][j] = gf;
            }
        } else {
            if (obstype == "ALL") {
                for (j = 0; j < NFREQ + NEXOBS; j++) {
                    LLI[i] |= obs->LLI[j];
                }
            }
            else {
                k = obstype.mid(1).toInt(&ok);
                if (ok) {
                    j = k > 2 ? k - 3 : k - 1;  /* L1,L2,L5,L6 ... */
                }
                else {
                    for (j = 0; j < NFREQ + NEXOBS; j++) {
                        if (!strcmp(code2obs(obs->code[j]), qPrintable(obstype))) break;
                    }
                    if (j >= NFREQ + NEXOBS) continue;
                }
                LLI[i] = obs->LLI[j];
            }
        }
    }
}
// draw slip on observation-data-plot ---------------------------------------
void Plot::drawObservationSlips(QPainter &c, double *yp)
{
    trace(3,"drawObservationSlips\n");
    if (observation.n <= 0 || (!showSlip && !showHalfC)) return;

    int *LLI = new int [observation.n];

    generateObservationSlips(LLI);

    for (int i = 0; i < observation.n; i++) {
        if (!LLI[i]) continue;
        QPoint ps[2];
        obsd_t *obs = observation.data + i;
        if (graphSingle->toPoint(timePosition(obs->time), yp[obs->sat-1], ps[0])) {
            ps[1].setX(ps[0].x());
            ps[1].setY(ps[0].y() + markSize * 3 / 2 + 1);
            ps[0].setY(ps[0].y() - markSize * 3 / 2);
            if (showHalfC && (LLI[i] & 2)) graphSingle->drawPoly(c, ps, 2, markerColor[0][0], 0);
            if (showSlip && (LLI[i] & 1)) graphSingle->drawPoly(c, ps, 2, markerColor[0][5], 0);
        }
    }
    delete [] LLI;
}
// draw ephemeris on observation-data-plot ----------------------------------
void Plot::drawObservationEphemeris(QPainter &c, double *yp)
{
    QPoint ps[3];
    int i, j, k, in, svh, off[MAXSAT] = { 0 };

    trace(3, "drawObservationEphemeris\n");

    if (!showEphemeris) return;

    for (i = 0; i < MAXSAT; i++) {
        if (!satelliteSelection[i]) continue;
        // GPS/Galileo/... ephemeris
        for (j = 0; j < navigation.n; j++) {
            if (navigation.eph[j].sat != i + 1) continue;
            graphSingle->toPoint(timePosition(navigation.eph[j].ttr), yp[i], ps[0]);
            in = graphSingle->toPoint(timePosition(navigation.eph[j].toe), yp[i], ps[2]);
            ps[1] = ps[0];
            off[navigation.eph[j].sat - 1] = off[navigation.eph[j].sat - 1] ? 0 : 3;

            for (k = 0; k < 3; k++)
                ps[k].ry() += markSize + 2 + off[navigation.eph[j].sat - 1];
            ps[0].ry() -= 2;

            svh = navigation.eph[j].svh;
            if (satsys(i + 1, NULL) == SYS_QZS)
                svh &= 0xFE; /* mask QZS LEX health */

            graphSingle->drawPoly(c, ps, 3, svh ? markerColor[0][5] : cColor[1], 0);

            if (in) graphSingle->drawMark(c, ps[2], Graph::MarkerTypes::Dot, svh ? markerColor[0][5] : cColor[1], svh ? 4 : 3, 0);
        }
        // Glonass ephemeris
        for (j = 0; j < navigation.ng; j++) {
            if (navigation.geph[j].sat != i + 1) continue;
            graphSingle->toPoint(timePosition(navigation.geph[j].tof), yp[i], ps[0]);
            in = graphSingle->toPoint(timePosition(navigation.geph[j].toe), yp[i], ps[2]);
            ps[1] = ps[0];
            off[navigation.geph[j].sat - 1] = off[navigation.geph[j].sat - 1] ? 0 : 3;
            for (k = 0; k < 3; k++)
                ps[k].ry() += markSize + 2 + off[navigation.geph[j].sat - 1];
            ps[0].ry() -= 2;

            graphSingle->drawPoly(c, ps, 3, navigation.geph[j].svh ? markerColor[0][5] : cColor[1], 0);

            if (in) graphSingle->drawMark(c, ps[2], Graph::MarkerTypes::Dot, navigation.geph[j].svh ? markerColor[0][5] : cColor[1],
                         navigation.geph[j].svh ? 4 : 3, 0);
        }
        // SBAS ephemeris
        for (j = 0; j < navigation.ns; j++) {
            if (navigation.seph[j].sat != i + 1) continue;
            graphSingle->toPoint(timePosition(navigation.seph[j].tof), yp[i], ps[0]);
            in = graphSingle->toPoint(timePosition(navigation.seph[j].t0), yp[i], ps[2]);
            ps[1] = ps[0];
            off[navigation.seph[j].sat - 1] = off[navigation.seph[j].sat - 1] ? 0 : 3;
            for (k = 0; k < 3; k++)
                ps[k].ry() += markSize + 2 + off[navigation.seph[j].sat - 1];
            ps[0].ry() -= 2;

            graphSingle->drawPoly(c, ps, 3, navigation.seph[j].svh ? markerColor[0][5] : cColor[1], 0);

            if (in) graphSingle->drawMark(c, ps[2], Graph::MarkerTypes::Dot, navigation.seph[j].svh ? markerColor[0][5] : cColor[1],
                         navigation.seph[j].svh ? 4 : 3, 0);
        }
    }
}
// draw sky-image on sky-plot -----------------------------------------------
void Plot::drawSkyImage(QPainter &c, int level)
{
    QPoint topLeft, bottomRight;
    double xl[2], yl[2], radius, scale, mx[190], my[190];

    trace(3, "drawSkyImage: level=%d\n", level);

    if (skySize[0] <= 0 || skySize[1] <= 0) return;

    graphSky->getLimits(xl, yl);
    radius = (xl[1] - xl[0] < yl[1] - yl[0] ? xl[1] - xl[0] : yl[1] - yl[0]) * 0.45;
    scale = radius * skyImageResampled.width() / 2.0 / skyScaleR;
    graphSky->toPoint(-scale, scale, topLeft);
    graphSky->toPoint(scale, -scale, bottomRight);
    QRect rect(topLeft, bottomRight);
    c.drawImage(rect, skyImageResampled);

    if (skyElevationMask) { // elevation mask
        int n = 0;

        mx[n] = 0.0;
        my[n++] = yl[1];
        for (int i = 0; i <= 180; i++) {
            mx[n] = radius * sin(i * 2.0 * D2R);
            my[n++] = radius * cos(i * 2.0 * D2R);
        }
        mx[n] = 0.0;   my[n++] = yl[1];
        mx[n] = xl[0]; my[n++] = yl[1];
        mx[n] = xl[0]; my[n++] = yl[0];
        mx[n] = xl[1]; my[n++] = yl[0];
        mx[n] = xl[1]; my[n++] = yl[1];
        graphSky->drawPatch(c, mx, my, n, cColor[0], cColor[0], 0);
    }
}
// draw sky-plot ------------------------------------------------------------
void Plot::drawSky(QPainter &c, int level)
{
    QPoint p1, p2;
    QString s, obstype = cBObservationType->currentText();
    obsd_t *obs;
    gtime_t t[MAXSAT] = {{ 0, 0 }};
    double p[MAXSAT][2] = {{ 0 }}, p0[MAXSAT][2] = {{ 0 }};
    double x, y, xp, yp, xs, ys, dt, dx, dy, xl[2], yl[2], radius;
    int i, j, ind = observationIndex, freq;
    int hh = (int)(QFontMetrics(lblDisplay->font()).height() * 1.5);

    trace(3, "drawSky: level=%d\n", level);

    graphSky->getLimits(xl, yl);
    radius = (xl[1] - xl[0] < yl[1] - yl[0] ? xl[1] - xl[0] : yl[1] - yl[0]) * 0.45;

    if (btnShowImage->isChecked())
        drawSkyImage(c, level);
    if (btnShowSkyplot->isChecked())
        graphSky->drawSkyPlot(c, 0.0, 0.0, cColor[1], cColor[2], cColor[0], radius * 2.0);
    if (!btnSolution1->isChecked()) return;

    graphSky->getScale(xs, ys);

    if (plotStyle <= 2) {
        for (i = 0; i < observation.n; i++) {
            obs = &observation.data[i];
            if (satelliteMask[obs->sat - 1] || !satelliteSelection[obs->sat - 1] || elevation[i] <= 0.0) continue;
            QColor col = observationColor(obs, azimuth[i], elevation[i]);
            if (col == Qt::black) continue;

            x = radius * sin(azimuth[i]) * (1.0 - 2.0 * elevation[i] / PI);
            y = radius * cos(azimuth[i]) * (1.0 - 2.0 * elevation[i] / PI);
            xp = p[obs->sat - 1][0];
            yp = p[obs->sat - 1][1];

            if ((x - xp) * (x - xp) + (y - yp) * (y - yp) >= xs * xs) {
                int siz = plotStyle < 2 ? markSize : 1;
                graphSky->drawMark(c, x, y, Graph::MarkerTypes::Dot, plotStyle < 2 ? col : cColor[3], siz, 0);
                p[obs->sat - 1][0] = x;
                p[obs->sat - 1][1] = y;
            }
            if (xp == 0.0 && yp == 0.0) {
                p0[obs->sat - 1][0] = x;
                p0[obs->sat - 1][1] = y;
            }
        }
    }
    if ((plotStyle == 0 || plotStyle == 2) && !btnShowTrack->isChecked()) {
        for (i = 0; i < MAXSAT; i++) {
            if (p0[i][0] != 0.0 || p0[i][1] != 0.0) {
                QPoint pnt;
                if (graphSky->toPoint(p0[i][0], p0[i][1], pnt)) {
                    char id[16];
                    satno2id(i+1,id);
                    drawLabel(graphSky, c, pnt, QString(id), Graph::Alignment::Left, Graph::Alignment::Center);
                }
            }
        }
    }
    if (!level) return;

    if (showSlip && plotStyle <= 2) {
        int *LLI = new int [observation.n];

        generateObservationSlips(LLI);

        for (i = 0; i < observation.n; i++) {
            if (!(LLI[i] & 1)) continue;
            obs = observation.data+i;
            x = radius * sin(azimuth[i]) * (1.0 - 2.0 * elevation[i] / PI);
            y = radius * cos(azimuth[i]) * (1.0 - 2.0 * elevation[i] / PI);
            dt = timediff(obs->time, t[obs->sat - 1]);
            dx = x - p[obs->sat - 1][0];
            dy = y - p[obs->sat - 1][1];
            t[obs->sat - 1] = obs->time;
            p[obs->sat - 1][0] = x;
            p[obs->sat - 1][1] = y;
            if (fabs(dt) > 300.0) continue;
            graphSky->drawMark(c, x, y, Graph::MarkerTypes::Line, markerColor[0][5], markSize * 3 + 2, static_cast<int>(ATAN2(dy, dx) * R2D + 90));
        }
        delete [] LLI;
    }

    if (elevationMaskEnabled) {
        double *x = new double [361];
        double *y = new double [361];
        for (i = 0; i <= 360; i++) {
            x[i] = radius * sin(i * D2R) * (1.0 - 2.0 * elevationMaskData[i] / PI);
            y[i] = radius * cos(i * D2R) * (1.0 - 2.0 * elevationMaskData[i] / PI);
        }
        QPen pen = c.pen(); pen.setWidth(2); c.setPen(pen);
        graphSky->drawPoly(c, x, y, 361, COL_ELMASK, 0);
        pen.setWidth(1); c.setPen(pen);
        delete [] x;
        delete [] y;
    }

    if (btnShowTrack->isChecked() && 0 <= ind && ind < nObservation) {
        for (i = indexObservation[ind]; i < observation.n && i < indexObservation[ind + 1]; i++) {
            obs = &observation.data[i];
            if (satelliteMask[obs->sat - 1] || !satelliteSelection[obs->sat - 1] || elevation[i] <= 0.0) continue;
            QColor col = observationColor(obs, azimuth[i], elevation[i]);
            if (col == Qt::black) continue;

            x = radius * sin(azimuth[i]) * (1.0 - 2.0 * elevation[i] / PI);
            y = radius * cos(azimuth[i]) * (1.0 - 2.0 * elevation[i] / PI);

            char id[16];
            satno2id(obs->sat, id);
            int fontsize = (int)(QFontMetrics(lblDisplay->font()).height());
            graphSky->drawMark(c, x, y, Graph::MarkerTypes::Dot, col, fontsize * 2 + 5, 0);
            graphSky->drawMark(c, x, y, Graph::MarkerTypes::Circle, col == Qt::black ? markerColor[0][0] : cColor[2],
                             fontsize * 2 + 5, 0);
            graphSky->drawText(c, x, y, QString(id), cColor[0], 0, Graph::Alignment::Center, Graph::Alignment::Center);
        }
    }

    graphSky->getExtent(p1, p2);
    p1.rx() += 10; p1.ry() += 8;
    p2.rx() -= 10; p2.ry() = p1.y();

    if (showStats && !simulatedObservation) {
        s = QString(tr("MARKER: %1 %2")).arg(station.name, station.marker);
        drawLabel(graphSky, c, p1, s, Graph::Alignment::Left, Graph::Alignment::Top); p1.ry() += hh;
        s = QString(tr("REC: %1 %2 %3")).arg(station.rectype, station.recver, station.recsno);
        drawLabel(graphSky, c, p1, s, Graph::Alignment::Left, Graph::Alignment::Top); p1.ry() += hh;
        s = QString(tr("ANT: %1 %2")).arg(station.antdes, station.antsno);
        drawLabel(graphSky, c, p1, s, Graph::Alignment::Left, Graph::Alignment::Top); p1.ry() += hh;
    }
        // show statistics
    if (showStats && btnShowTrack->isChecked() && 0 <= ind && ind < nObservation && !simulatedObservation) {
        char id[16];

        if (obstype == "ALL") {
            s = QString::asprintf("%3s: %*s %*s%*s %*s","SAT",NFREQ,"PR",NFREQ,"CP",
                         NFREQ*3,"CN0",NFREQ,"LLI");
        }
        else {
            s = QString(tr("SAT: SIG  OBS   CN0 LLI"));
        }
        graphSky->drawText(c, p2, s, Qt::black, Graph::Alignment::Right, Graph::Alignment::Top, 0, QFont(MS_FONT));

        p2.ry() += 3;

        for (i = indexObservation[ind]; i < observation.n && i < indexObservation[ind + 1]; i++) {
            bool ok;
            obs = &observation.data[i];
            if (satelliteMask[obs->sat - 1] || !satelliteSelection[obs->sat - 1]) continue;
            if (hideLowSatellites && elevation[i] < elevationMask * D2R) continue;
            if (hideLowSatellites && elevationMaskEnabled && elevation[i] < elevationMaskData[static_cast<int>(azimuth[i] * R2D + 0.5)]) continue;

            satno2id(obs->sat, id);
            s = QString("%1: ").arg(id, 3, QChar('-'));

            freq=obstype.mid(1).toInt(&ok);

            if (obstype == "ALL") {
                for (j = 0; j < NFREQ; j++) s += obs->P[j] == 0.0 ? "-" : "C";
                s += " ";
                for (j = 0; j < NFREQ; j++) s += obs->L[j] == 0.0 ? "-" : "L";
                s += " ";
                for (j = 0; j < NFREQ; j++) {
                    if (obs->P[j]==0.0 && obs->L[j]==0.0) s += "-- ";
                    else s += QString("%1 ").arg(obs->SNR[j] * SNR_UNIT, 2, 'f', 0, QChar('0'));
                }
                for (j = 0; j < NFREQ; j++) {
                    if (obs->L[j] == 0.0) s += "-";
                    else s += QString("%1").arg(obs->LLI[j]);
                }
             } else if (ok) {
                freq-=freq>2?2:0;  /* L1,L2,L5,L6 ... */
                if (!obs->code[freq-1]) continue;
                s += QString("%1  %2 %3 %4  ").arg(code2obs(obs->code[freq - 1]),
                              obs->P[freq - 1] == 0.0 ? "-" : "C",
                              obs->L[freq - 1] == 0.0 ? "-" : "L",
                              obs->D[freq - 1] == 0.0 ? "-" : "D");

                if (obs->P[freq - 1] == 0.0 && obs->L[freq - 1] == 0.0) s += "---- ";
                else s += QString("%1 ").arg(obs->SNR[freq - 1] * SNR_UNIT, 4, 'f', 1);

                if (obs->L[freq-1] == 0.0) s += " -";
                else s += QString::number(obs->LLI[freq - 1]);
            } else {
                for (j = 0; j < NFREQ + NEXOBS; j++)
                    if (!strcmp(code2obs(obs->code[j]), qPrintable(obstype))) break;
                if (j >= NFREQ + NEXOBS) continue;

                s += QString("%1  %2 %3 %4  ").arg(code2obs(obs->code[j]),
                                 obs->P[j] == 0.0 ? "-" : "C",
                                 obs->L[j] == 0.0 ? "-" : "L",
                                 obs->D[j] == 0.0 ? "-" : "D");

                if (obs->P[j] == 0.0 && obs->L[j] == 0.0) s += "---- ";
                else s += QString("%1 ").arg(obs->SNR[j] * SNR_UNIT, 4, 'f', 1);

                if (obs->L[j] == 0.0) s += " -";
                else s += QString::number(obs->LLI[j]);
            }
            QColor col = observationColor(obs, azimuth[i], elevation[i]);
            p2.ry() += hh;
            graphSky->drawText(c, p2, s, col == Qt::black ? markerColor[0][0] : col, Graph::Alignment::Right, Graph::Alignment::Top, 0);
        }
    }
    if (navigation.n <= 0 && navigation.ng <= 0 && !simulatedObservation) {
        graphSky->getExtent(p1, p2);
        p2.rx() -= 10;
        p2.ry() -= 3;
        drawLabel(graphSky, c, p2, tr("No navigation data"), Graph::Alignment::Right, Graph::Alignment::Bottom);
    }
}
// draw DOP and number-of-satellite plot ------------------------------------
void Plot::drawDop(QPainter &c, int level)
{
    QString label;
    QPoint p1, p2;
    double xp, xc, yc, xl[2], yl[2], azel[MAXSAT * 2], *dop, *x, *y;
    int i, j, *ns, n = 0;
    int ind = observationIndex, doptype = cBDopType->currentIndex();

    trace(3, "drawDop: level=%d\n", level);
    
    graphSingle->xLabelPosition = timeFormat ? Graph::LabelPosition::Time : Graph::LabelPosition::Outer;
    graphSingle->yLabelPosition = Graph::LabelPosition::Outer;
    graphSingle->week = week;
    graphSingle->getLimits(xl, yl);
    yl[0] = 0.0;
    yl[1] = maxDop;
    graphSingle->setLimits(xl, yl);
    graphSingle->setTick(0.0, 0.0);

    if (0 <= ind && ind < nObservation && btnShowTrack->isChecked() && btnFixHorizontal->isChecked()) {
        double xl[2], yl[2], offset;
        graphSingle->getLimits(xl, yl);
        offset = centX * (xl[1] - xl[0]) / 2.0;
        xp = timePosition(observation.data[indexObservation[ind]].time);
        graphSingle->getCenter(xc, yc);
        graphSingle->setCenter(xp - offset, yc);
    }
    graphSingle->drawAxis(c, true, true);
    graphSingle->getExtent(p1, p2);
    p1.setX((int)(QFontMetrics(lblDisplay->font()).height()));
    p1.setY((p1.y() + p2.y()) / 2);
    if (doptype == 0)
        label = QString("# OF SATELLITES / DOP (EL>=%1%2)").arg(elevationMask, 0, 'f', 0).arg(degreeChar);
    else if (doptype == 1)
        label = QString("# OF SATELLITES (EL>=%1%2)").arg(elevationMask, 0, 'f', 0).arg(degreeChar);
    else
        label = QString("DOP (EL>=%1%2)").arg(elevationMask, 0, 'f', 0).arg(degreeChar);
    graphSingle->drawText(c, p1, label, cColor[2], Graph::Alignment::Center, Graph::Alignment::Center, 90);

    if (!btnSolution1->isChecked()) return;

    x = new double[nObservation];
    y = new double[nObservation];
    dop = new double[nObservation * 4];
    ns = new int[nObservation];

    for (i = 0; i < nObservation; i++) {
        ns[n] = 0;
        for (j = indexObservation[i]; j < observation.n && j < indexObservation[i + 1]; j++) {
            if (satelliteMask[observation.data[j].sat - 1] || !satelliteSelection[observation.data[j].sat - 1]) continue;
            if (elevation[j] < elevationMask * D2R) continue;
            if (elevationMaskEnabled && elevation[j] < elevationMaskData[static_cast<int>(azimuth[j] * R2D + 0.5)]) continue;
            azel[ns[n] * 2] = azimuth[j];
            azel[ns[n] * 2 + 2] = elevation[j];
            ns[n]++;
        }
        dops(ns[n], azel, elevationMask * D2R, dop + n * 4);
        x[n++] = timePosition(observation.data[indexObservation[i]].time);
    }
    for (i = 0; i < 4; i++) {
        if (doptype != 0 && doptype != i + 2) continue;

        for (j = 0; j < n; j++) y[j] = dop[i + j * 4] * 10;

        if (!(plotStyle % 2))
            drawPolyS(graphSingle, c, x, y, n, cColor[3], 0);
        if (level && plotStyle < 2) {
            for (j = 0; j < n; j++) {
                if (y[j] == 0.0) continue;
                graphSingle->drawMark(c, x[j], y[j], Graph::MarkerTypes::Dot, markerColor[0][i + 2], markSize, 0);
            }
        }
    }
    if (doptype == 0 || doptype == 1) {
        for (i = 0; i < n; i++) y[i] = ns[i];

        if (!(plotStyle % 2))
            drawPolyS(graphSingle, c, x, y, n, cColor[3], 1);
        if (level && plotStyle < 2) {
            for (i = 0; i < n; i++)
                graphSingle->drawMark(c, x[i], y[i], Graph::MarkerTypes::Dot, markerColor[0][1], markSize, 0);
        }
    }

    if (btnShowTrack->isChecked() && 0 <= ind && ind < nObservation) {
        graphSingle->getLimits(xl, yl);
        xl[0] = xl[1] = timePosition(observation.data[indexObservation[ind]].time);

        graphSingle->drawPoly(c, xl, yl, 2, cColor[2], 0);

        ns[0] = 0;
        for (i = indexObservation[ind]; i < observation.n && i < indexObservation[ind + 1]; i++) {
            if (satelliteMask[observation.data[i].sat - 1] || !satelliteSelection[observation.data[i].sat - 1]) continue;
            if (elevation[i] < elevationMask * D2R) continue;
            if (elevationMaskEnabled && elevation[i] < elevationMaskData[static_cast<int>(azimuth[i] * R2D + 0.5)]) continue;
            azel[ns[0] * 2] = azimuth[i];
            azel[ns[0] * 2 + 1] = elevation[i];
            ns[0]++;
        }
        dops(ns[0], azel, elevationMask * D2R, dop);

        for (i = 0; i < 4; i++) {
            if ((doptype != 0 && doptype != i + 2) || dop[i] <= 0.0) continue;
            graphSingle->drawMark(c, xl[0], dop[i]*10.0, Graph::MarkerTypes::Dot, markerColor[0][i + 2], markSize * 2 + 2, 0);
        }
        if (doptype == 0 || doptype == 1)
            graphSingle->drawMark(c, xl[0], ns[0], Graph::MarkerTypes::Dot, markerColor[0][1], markSize * 2 + 2, 0);
        graphSingle->drawMark(c, xl[0], yl[1] - 1E-6, 0, cColor[2], 5, 0);
        if (!btnFixHorizontal->isChecked())
            graphSingle->drawMark(c, xl[0], yl[1] - 1E-6, Graph::MarkerTypes::Circle, cColor[2], 9, 0);
    } else {
        drawDopStat(c, dop, ns, n);
    }

    if (navigation.n <= 0 && navigation.ng <= 0 && (doptype == 0 || doptype >= 2) && !simulatedObservation) {
        graphSingle->getExtent(p1, p2);
        p2.rx() -= 10;
        p2.ry() -= 3;
        drawLabel(graphSingle, c, p2, tr("No navigation data"), Graph::Alignment::Right, Graph::Alignment::Bottom);
    }

    delete [] x;
    delete [] y;
    delete [] dop;
    delete [] ns;
}
// draw statistics on DOP and number-of-satellite plot ----------------------
void Plot::drawDopStat(QPainter &c, double *dop, int *ns, int n)
{
    QString s0[MAXOBS + 2], s1[MAXOBS + 2], s2[MAXOBS + 2];
    double ave[4] = { 0 };
    int nsat[MAXOBS] = {0}, ndop[4] = {0}, m = 0;

    trace(3, "drawDopStat: n=%d\n", n);

    if (!showStats) return;

    for (int i = 0; i < n; i++) {
        nsat[ns[i]]++;
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < n; j++) {
            if (dop[i + j * 4] <= 0.0 || dop[i + j * 4] > maxDop) continue;
            ave[i] += dop[i + j * 4];
            ndop[i]++;
        }
        if (ndop[i] > 0) ave[i] /= ndop[i];
    }
    if (cBDopType->currentIndex() == 0 || cBDopType->currentIndex() >= 2) {
        s2[m++] = QString("AVE = GDOP: %1, PDOP: %2, HDOP: %3, VDOP: %4")
              .arg(ave[0], 4, 'f', 1).arg(ave[1], 4, 'f', 1).arg(ave[2], 4, 'f', 1).arg(ave[3], 4, 'f', 1);
        s2[m++] = QString("NDOP = %1 (%2%), %3 (%4%), %5 (%6%), %7 (%8f%)")
              .arg(ndop[0]).arg(n > 0 ? ndop[0] * 100.0 / n : 0.0, 4, 'f', 1)
              .arg(ndop[1]).arg(n > 0 ? ndop[1] * 100.0 / n : 0.0, 4, 'f', 1)
              .arg(ndop[2]).arg(n > 0 ? ndop[2] * 100.0 / n : 0.0, 4, 'f', 1)
              .arg(ndop[3]).arg(n > 0 ? ndop[3] * 100.0 / n : 0.0, 4, 'f', 1);
    }
    if (cBDopType->currentIndex() <= 1) {
        for (int i = 0, j = 0; i < MAXOBS; i++) {
            if (nsat[i] <= 0) continue;
            s0[m] = QString("%1%2:").arg(j++ == 0 ? "NSAT = " : "").arg(i, 2);
            s1[m] = QString("%1").arg(nsat[i], 7);
            s2[m++] = QString("(%1%)").arg(nsat[i] * 100.0 / n, 4, 'f', 1);
        }
    }
    QPoint p1, p2, p3;
    int fonth = (int)(QFontMetrics(lblDisplay->font()).height() * 1.2);

    graphSingle->getExtent(p1, p2);
    p1.setX(p2.x() - 10);
    p1.ry() += 8;
    p2 = p1; p2.rx() -= fonth * 3;
    p3 = p2; p3.rx() -= fonth * 4;

    for (int i = 0; i < m; i++) {
        drawLabel(graphSingle, c, p3, s0[i], Graph::Alignment::Right, Graph::Alignment::Top);
        drawLabel(graphSingle, c, p2, s1[i], Graph::Alignment::Right, Graph::Alignment::Top);
        drawLabel(graphSingle, c, p1, s2[i], Graph::Alignment::Right, Graph::Alignment::Top);
        p1.setY(p1.y() + fonth);
        p2.setY(p2.y() + fonth);
        p3.setY(p3.y() + fonth);
    }
}
// draw SNR, MP and elevation-plot ---------------------------------------------
void Plot::drawSnr(QPainter &c, int level)
{
    QPushButton *btn[] = { btnOn1, btnOn2, btnOn3 };
    QString label[] = { tr("SNR"), tr("Multipath"), tr("Elevation") };
    QString unit[] = { "dBHz", "m", degreeChar };
    gtime_t time = { 0, 0 };

    trace(3, "drawSnr: level=%d\n", level);

    if (0 <= observationIndex && observationIndex < nObservation && btnShowTrack->isChecked())
        time = observation.data[indexObservation[observationIndex]].time;
    if (0 <= observationIndex && observationIndex < nObservation && btnShowTrack->isChecked() && btnFixHorizontal->isChecked()) {
        double xc, yc, xp = timePosition(time);
        double xl[2], yl[2];

        graphTriple[0]->getLimits(xl, yl);
        xp -= centX * (xl[1] - xl[0]) / 2.0;
        for (int i=0;i<3;i++) {
            graphTriple[i]->getCenter(xc,yc);
            graphTriple[i]->setCenter(xp,yc);
        }
    }
    int j = 0;

    for (int i = 0; i < 3; i++)
        if (btn[i]->isChecked()) j = i;

    for (int i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;
        graphTriple[i]->xLabelPosition = timeFormat ? (i == j ? Graph::LabelPosition::Time : Graph::LabelPosition::None) :
                                        (i == j ? Graph::LabelPosition::Outer : Graph::LabelPosition::On);
        graphTriple[i]->week = week;
        graphTriple[i]->drawAxis(c, showLabel, showLabel);
    }

    if (nObservation > 0 && btnSolution1->isChecked()) {
        QString obstype = cBObservationType2->currentText();
        double *x = new double[nObservation];
        double *y = new double[nObservation];
        QColor *col = new QColor[nObservation];

        for (int i = 0, l = 0; i < 3; i++) {
            QColor colp[MAXSAT];
            double yp[MAXSAT], ave=0.0, rms=0.0;
            int np=0, nrms=0;

            if (!btn[i]->isChecked()) continue;

            for (int sat = 1, np = 0; sat <= MAXSAT; sat++) {
                if (satelliteMask[sat - 1] || !satelliteSelection[sat - 1]) continue;
                int n = 0;

                for (int j = n = 0; j < observation.n; j++) {
                    obsd_t *obs = observation.data + j;
                    int k, freq;
                    bool ok;

                    if (obs->sat!=sat) continue;

                    freq = obstype.mid(1).toInt(&ok);
                    if (ok) {
                        k = freq > 2 ? freq - 3 : freq - 1;
                    }
                    else {
                        for (k = 0; k < NFREQ + NEXOBS; k++) {
                            if (!strcmp(code2obs(obs->code[k]), qPrintable(obstype))) break;
                        }
                        if (k >= NFREQ + NEXOBS) continue;
                    }
                    if (obs->SNR[k] * SNR_UNIT <= 0.0) continue;

                    x[n] = timePosition(obs->time);
                    if (i == 0) {
                        y[n] = obs->SNR[k] * SNR_UNIT;
                        col[n] = markerColor[0][4];
                    } else if (i == 1) {
                        if (!multipath[k] || multipath[k][j] == 0.0) continue;
                        y[n] = multipath[k][j];
                        col[n] = markerColor[0][4];
                    } else {
                        y[n] = elevation[j] * R2D;
                        if (simulatedObservation) col[n] = sysColor(obs->sat);
                        else col[n] = snrColor(obs->SNR[k] * SNR_UNIT);

                        if (elevation[j] > 0.0 && elevation[j] < elevationMask * D2R) col[n] = markerColor[0][0];
                    }
                    if (timediff(time, obs->time) == 0.0 && np < MAXSAT) {
                        yp[np] = y[n];
                        colp[np++] = col[n];
                    }
                    if (n < nObservation) n++;
                }
                if (!level || !(plotStyle % 2)) {
                    for (int j = 0, k = 0; j < n; j = k) {
                        for (k = j + 1; k < n; k++) if (fabs(y[k - 1] - y[k]) > 30.0) break;
                        drawPolyS(graphTriple[i], c, x + j, y + j, k - j, cColor[3], 0);
                    }
                }
                if (level && plotStyle < 2) {
                    for (int j  = 0; j < n; j++) {
                        if (i != 1 && y[j] <= 0.0) continue;
                        graphTriple[i]->drawMark(c, x[j], y[j], Graph::MarkerTypes::Dot, col[j], markSize, 0);
                    }
                }
                for (int j = 0; j < n; j++) {
                    if (y[j] == 0.0) continue;
                    ave += y[j];
                    rms += SQR(y[j]);
                    nrms++;
                }
            }
            if (level && i == 1 && nrms > 0 && showStats && !btnShowTrack->isChecked()) {
                QPoint p1,p2;
                ave = ave / nrms;
                rms = SQRT(rms / nrms);

                graphTriple[i]->getExtent(p1, p2);
                p1.rx() = p2.x() - 8;
                p1.ry() += 3;
                drawLabel(graphTriple[i], c, p1, QString("AVE = %1 m, RMS = %2 m").arg(ave, 0, 'f', 4).arg(rms, 0, 'f', 4),
                          Graph::Alignment::Right, Graph::Alignment::Top);
            }
            if (btnShowTrack->isChecked() && 0 <= observationIndex && observationIndex < nObservation && btnSolution1->isChecked()) {
                if (!btn[i]->isChecked()) continue;
                QPoint p1, p2;
                double xl[2], yl[2];
                graphTriple[i]->getLimits(xl, yl);
                xl[0] = xl[1] = timePosition(time);
                graphTriple[i]->drawPoly(c, xl, yl, 2, cColor[2], 0);

                if (l++ == 0) {
                    graphTriple[i]->drawMark(c, xl[0], yl[1] - 1E-6, Graph::MarkerTypes::Dot, cColor[2], 5, 0);

                    if (!btnFixHorizontal->isChecked())
                        graphTriple[i]->drawMark(c, xl[0], yl[1] - 1E-6, Graph::MarkerTypes::Circle, cColor[2], 9, 0);
                }
                for (int k = 0; k < np; k++) {
                    if (i != 1 && yp[k] <= 0.0) continue;
                    graphTriple[i]->drawMark(c, xl[0], yp[k], Graph::MarkerTypes::Dot, cColor[0], markSize * 2 + 4, 0);
                    graphTriple[i]->drawMark(c, xl[0], yp[k], Graph::MarkerTypes::Dot, colp[k], markSize * 2 + 2, 0);
                }
                if (np <= 0 || np > 1 || (i != 1 && yp[0] <= 0.0)) continue;

                graphTriple[i]->getExtent(p1, p2);
                p1.rx() = p2.x() - 8;
                p1.ry() += 3;
                drawLabel(graphTriple[i], c, p1, QString("%1 %2").arg(yp[0], 0, 'f', i == 1 ? 4 : 1).arg(unit[i]),
                          Graph::Alignment::Right, Graph::Alignment::Top);
            }
        }
        delete [] x;
        delete [] y;
        delete [] col;
    }
    for (int i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;
        QPoint p1, p2;
        graphTriple[i]->getExtent(p1, p2);
        p1.rx() += 5;
        p1.ry() += 3;
        drawLabel(graphTriple[i], c, p1, QString("%1 (%2)").arg(label[i], unit[i]), Graph::Alignment::Left, Graph::Alignment::Top);
    }
}
// draw SNR, MP to elevation-plot ----------------------------------------------
void Plot::drawSnrE(QPainter &c, int level)
{
    QPushButton *btn[] = { btnOn1, btnOn2, btnOn3 };
    QString s;
    QString label[] = { tr("SNR (dBHz)"), tr("Multipath (m)") };
    gtime_t time = { 0, 0 };
    double ave = 0.0, rms = 0.0;

    int nrms = 0;

    trace(3, "drawSnrE: level=%d\n", level);

    int j = 0;
    for (int i = 0; i < 2; i++) if (btn[i]->isChecked()) j = i;

    for (int i = 0; i < 2; i++) {
        QPoint p1, p2;
        double xl[2] = {-0.001, 90.0}, yl[2][2]={{10.0, 65.0}, {-maxMP, maxMP}};

        if (!btn[i]->isChecked()) continue;
        graphDual[i]->xLabelPosition = i == j ? Graph::LabelPosition::Outer : Graph::LabelPosition::On;
        graphDual[i]->yLabelPosition = Graph::LabelPosition::Outer;
        graphDual[i]->setLimits(xl, yl[i]);
        graphDual[i]->setTick(0.0, 0.0);
        graphDual[i]->drawAxis(c, true, true);

        graphDual[i]->getExtent(p1, p2);
        p1.setX(0);
        p1.setY((p1.y() + p2.y()) / 2);
        graphDual[i]->drawText(c, p1, label[i], cColor[2], Graph::Alignment::Center, Graph::Alignment::Top, 90);
        if (i == j) {
            p2.rx() -= 8;
            p2.ry() -= 6;
            graphDual[i]->drawText(c, p2, tr("Elevation ( %1 )").arg(degreeChar), cColor[2],
                                Graph::Alignment::Right, Graph::Alignment::Bottom, 0);
        }
    }

    if (0 <= observationIndex && observationIndex < nObservation && btnShowTrack->isChecked())
        time = observation.data[indexObservation[observationIndex]].time;

    if (nObservation > 0 && btnSolution1->isChecked()) {
        QColor *col[2], colp[2][MAXSAT];
        QString obstype = cBObservationType2->currentText();
        double *x[2], *y[2], xp[2][MAXSAT], yp[2][MAXSAT];
        int n[2], np[2] = {0};

        for (int i = 0; i < 2; i++) {
            x[i] = new double[nObservation],
            y[i] = new double[nObservation];
            col[i] = new QColor[nObservation];
        }
        for (int sat = 1; sat <= MAXSAT; sat++) {
            if (satelliteMask[sat - 1] || !satelliteSelection[sat - 1]) continue;
            n[0] = n[1] = 0;

            for (int j = 0; j < observation.n; j++) {
                obsd_t *obs = observation.data + j;
                int k, freq;
                bool ok;

                if (obs->sat != sat || elevation[j] <= 0.0) continue;

                freq = obstype.mid(1).toInt(&ok);

                if (ok) {
                    k = freq > 2 ? freq-3 : freq-1;  /* L1,L2,L5,L6 ... */
                }
                else {
                    for (k = 0; k < NFREQ + NEXOBS; k++) {
                        if (!strcmp(code2obs(obs->code[k]), qPrintable(obstype))) break;
                    }
                    if (k >= NFREQ + NEXOBS) continue;
                }
                if (obs->SNR[k] * SNR_UNIT <= 0.0) continue;

                x[0][n[0]] = x[1][n[1]] = elevation[j] * R2D;
                y[0][n[0]] = obs->SNR[k] * SNR_UNIT;
                y[1][n[1]] = !multipath[k] ? 0.0 : multipath[k][j];

                col[0][n[0]] = col[1][n[1]] =
                    elevation[j] > 0.0 && elevation[j] < elevationMask * D2R ? markerColor[0][0] : markerColor[0][4];

                if (y[0][n[0]] > 0.0) {
                    if (timediff(time, observation.data[j].time) == 0.0) {
                        xp[0][np[0]] = x[0][n[0]];
                        yp[0][np[0]] = y[0][n[0]];
                        colp[0][np[0]] = observationColor(observation.data + j, azimuth[j], elevation[j]);
                        if (np[0] < MAXSAT && colp[0][np[0]] != Qt::black) np[0]++;
                    }
                    if (n[0] < nObservation) n[0]++;
                }
                if (y[1][n[1]] != 0.0) {
                    if (elevation[j] >= elevationMask * D2R) {
                        ave += y[1][n[1]];
                        rms += SQR(y[1][n[1]]);
                        nrms++;
                    }
                    if (timediff(time, observation.data[j].time) == 0.0) {
                        xp[1][np[1]] = x[1][n[1]];
                        yp[1][np[1]] = y[1][n[1]];
                        colp[1][np[1]] = observationColor(observation.data + j, azimuth[j], elevation[j]);
                        if (np[1] < MAXSAT && colp[1][np[1]] != Qt::black) np[1]++;
                    }
                    if (n[1] < nObservation) n[1]++;
                }
            }
            if (!level || !(plotStyle % 2)) {
                for (int i = 0; i < 2; i++) {
                    if (!btn[i]->isChecked()) continue;
                    drawPolyS(graphDual[i], c, x[i], y[i], n[i], cColor[3], 0);
                }
            }
            if (level && plotStyle < 2) {
                for (int i = 0; i < 2; i++) {
                    if (!btn[i]->isChecked()) continue;
                    for (int j = 0; j < n[i]; j++)
                        graphDual[i]->drawMark(c, x[i][j], y[i][j], Graph::MarkerTypes::Dot, col[i][j], markSize, 0);
                }
            }
        }
        for (int i = 0; i < 2; i++) {
            delete[] x[i];
            delete[] y[i];
            delete[] col[i];
        }
        if (btnShowTrack->isChecked() && 0 <= observationIndex && observationIndex < nObservation && btnSolution1->isChecked()) {
            for (int i = 0; i < 2; i++) {
                if (!btn[i]->isChecked()) continue;
                for (int j = 0; j < np[i]; j++) {
                    graphDual[i]->drawMark(c, xp[i][j], yp[i][j], Graph::MarkerTypes::Dot, cColor[0], markSize * 2 + 8, 0);
                    graphDual[i]->drawMark(c, xp[i][j], yp[i][j], Graph::MarkerTypes::Circle, cColor[2], markSize * 2 + 6, 0);
                    graphDual[i]->drawMark(c, xp[i][j], yp[i][j], Graph::MarkerTypes::Dot, colp[i][j], markSize * 2 + 2, 0);
                }
            }
        }
    }

    if (showStats) {
        int i;
        for (i = 0; i < 2; i++) if (btn[i]->isChecked()) break;
        if (i < 2) {
            QPoint p1,p2;
            int hh = (int)(QFontMetrics(lblDisplay->font()).height() * 1.5);

            graphDual[i]->getExtent(p1, p2);
            p1.rx() += 8;
            p1.ry() += 6;
            s = QString("MARKER: %1 %2").arg(station.name, station.marker);
            drawLabel(graphDual[i], c, p1, s, Graph::Alignment::Left, Graph::Alignment::Top); p1.ry() += hh;
            s = QString("REC: %1 %2 %3").arg(station.rectype, station.recver, station.recsno);
            drawLabel(graphDual[i], c, p1, s, Graph::Alignment::Left, Graph::Alignment::Top); p1.ry() += hh;
            s = QString("ANT: %1 %2").arg(station.antdes, station.antsno);
            drawLabel(graphDual[i], c, p1, s, Graph::Alignment::Left, Graph::Alignment::Top); p1.ry() += hh;
        }
        if (btn[1]->isChecked() && nrms > 0 && !btnShowTrack->isChecked()) {
            QPoint p1,p2;
            ave = ave / nrms;
            rms = SQRT(rms / nrms);
            graphDual[1]->getExtent(p1, p2);
            p1.setX(p2.x() - 8);
            p1.ry() += 6;
            drawLabel(graphDual[1], c, p1, QString("AVE = %1 m, RMS = %2 m").arg(ave, 0, 'f', 4).arg(rms, 0, 'f', 4),
                      Graph::Alignment::Right, Graph::Alignment::Top);
        }
    }
}
// draw MP-skyplot ----------------------------------------------------------
void Plot::drawMpSky(QPainter &c, int level)
{
    QString obstype = cBObservationType2->currentText();
    double radius,xl[2],yl[2],xs,ys;

    trace(3, "drawMpSky: level=%d\n", level);

    graphSky->getLimits(xl, yl);
    radius = (xl[1] - xl[0] < yl[1] - yl[0] ? xl[1] - xl[0] : yl[1] - yl[0]) * 0.45;

    if (btnShowImage->isChecked())
        drawSkyImage(c, level);

    if (btnShowSkyplot->isChecked())
        graphSky->drawSkyPlot(c, 0.0, 0.0, cColor[1], cColor[2], cColor[0], radius * 2.0);

    if (!btnSolution1->isChecked() || nObservation <= 0 || simulatedObservation) return;

    graphSky->getScale(xs, ys);

    for (int sat = 1; sat <= MAXSAT; sat++) {
        double p[MAXSAT][2] = {{0}};

        if (satelliteMask[sat - 1] || !satelliteSelection[sat - 1]) continue;

        for (int i = 0; i < observation.n; i++) {
            obsd_t *obs = observation.data+i;
            int j, freq;
            bool ok;

            if (obs->sat != sat || elevation[i] <= 0.0) continue;

            freq = obstype.mid(1).toInt(&ok);

            if (ok) {
                j = freq > 2 ? freq - 3 : freq - 1;  /* L1,L2,L5,L6 ... */
            }
            else {
                for (j = 0; j < NFREQ + NEXOBS; j++) {
                    if (!strcmp(code2obs(obs->code[j]), qPrintable(obstype))) break;
                }
                if (j >= NFREQ+NEXOBS) continue;
            }
            double x = radius * sin(azimuth[i]) * (1.0 - 2.0 * elevation[i] / PI);
            double y = radius * cos(azimuth[i]) * (1.0 - 2.0 * elevation[i] / PI);
            double xp = p[sat-1][0];
            double yp = p[sat-1][1];
            QColor col = mpColor(!multipath[j] ? 0.0 : multipath[j][i]);

            if ((x - xp) * (x - xp) + (y - yp) * (y - yp) >= xs * xs) {
                int siz = plotStyle < 2 ? markSize : 1;
                graphSky->drawMark(c, x, y, Graph::MarkerTypes::Dot, col, siz, 0);
                graphSky->drawMark(c, x, y, Graph::MarkerTypes::Dot, plotStyle < 2 ? col : cColor[3], siz, 0);
                p[sat - 1][0] = x;
                p[sat - 1][1] = y;
            }
        }
    }

    if (btnShowTrack->isChecked() && 0 <= observationIndex && observationIndex < nObservation) {
        for (int i = indexObservation[observationIndex]; i < observation.n && i < indexObservation[observationIndex + 1]; i++) {
            obsd_t *obs = observation.data+i;
            int j, freq;
            bool ok;

            freq = obstype.mid(1).toInt(&ok);

            if (ok) {
                j = freq > 2 ? freq - 3 : freq - 1;  /* L1,L2,L5,L6 ... */
            }
            else {
                for (j = 0; j < NFREQ + NEXOBS; j++) {
                    if (!strcmp(code2obs(obs->code[j]), qPrintable(obstype))) break;
                }
                if (j >= NFREQ + NEXOBS) continue;
            }
            QColor col = mpColor(!multipath[j] ? 0.0 : multipath[j][i]);
            double x = radius * sin(azimuth[i]) * (1.0 - 2.0 * elevation[i] / PI);
            double y = radius * cos(azimuth[i]) * (1.0 - 2.0 * elevation[i] / PI);

            char id[32];
            satno2id(obs->sat, id);
            int fontsize = (int)(QFontMetrics(lblDisplay->font()).height());
            graphSky->drawMark(c, x, y, Graph::MarkerTypes::Dot, col, fontsize * 2 + 5, 0);
            graphSky->drawMark(c, x, y, Graph::MarkerTypes::Circle, cColor[2], fontsize * 2 + 5, 0);
            graphSky->drawText(c, x, y, QString(id), cColor[0], 0, Graph::Alignment::Center, Graph::Alignment::Center);
        }
    }
}
// draw residuals and SNR/elevation plot ------------------------------------
void Plot::drawResidual(QPainter &c, int level)
{
    QString label[] = {
        tr("Pseudorange Residuals (m)"),
        tr("Carrier-Phase Residuals (m)"),
        tr("Elevation Angle (deg) / Signal Strength (dBHz)")
    };
    QString str;
    QPushButton *btn[] = { btnOn1, btnOn2, btnOn3 };
    int sel = !btnSolution1->isChecked() && btnSolution2->isChecked() ? 1 : 0, ind = solutionIndex[sel];
    int frq = cBFrequencyType->currentIndex() + 1, n = solutionStat[sel].n;

    trace(3, "drawResidual: level=%d\n", level);

    if (0 <= ind && ind < solutionData[sel].n && btnShowTrack->isChecked() && btnFixHorizontal->isChecked()) {
        gtime_t t = solutionData[sel].data[ind].time;

        for (int i = 0; i < 3; i++) {
            double offset, xc, yc, xl[2], yl[2];

            if (btnFixHorizontal->isChecked()) {
                graphTriple[i]->getLimits(xl, yl);
                offset = centX * (xl[1] - xl[0]) / 2.0;
                graphTriple[i]->getCenter(xc, yc);
                graphTriple[i]->getCenter(xc, yc);
                graphTriple[i]->setCenter(timePosition(t) - offset, yc);
            } else {
                graphTriple[i]->getRight(xc, yc);
                graphTriple[i]->setRight(timePosition(t), yc);
            }
        }
    }
    int j = -1;
    for (int i = 0; i < 3; i++) if (btn[i]->isChecked()) j = i;

    for (int i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;
        graphTriple[i]->xLabelPosition = timeFormat ? (i == j ? Graph::LabelPosition::Time : Graph::LabelPosition::None) :
                                        (i == j ? Graph::LabelPosition::Outer : Graph::LabelPosition::On);
        graphTriple[i]->week = week;
        graphTriple[i]->drawAxis(c, showLabel, showLabel);
    }

    if (n > 0 && ((sel == 0 && btnSolution1->isChecked()) || (sel == 1 && btnSolution2->isChecked()))) {
        QColor *col[4];
        double *x[4], *y[4], sum[2] = {0}, sum2[2] = {0};
        int ns[2] = {0};

        for (int i = 0; i < 4; i++) {
            x[i] = new double[n],
            y[i] = new double[n];
            col[i] = new QColor[n];
        }

        for (int sat = 1; sat <= MAXSAT; sat++) {
            if (satelliteMask[sat - 1] || !satelliteSelection[sat - 1]) continue;

            int m[4] = {0};

            for (int i = 0; i < n; i++) {
                solstat_t *solstat = solutionStat[sel].data + i;
                int q;

                if (solstat->sat != sat || solstat->frq != frq) continue;
                if (solstat->resp == 0.0 && solstat->resc == 0.0) continue;

                x[0][m[0]] = x[1][m[1]] = x[2][m[2]] = x[3][m[3]] = timePosition(solstat->time);
                y[0][m[0]] = solstat->resp;
                y[1][m[1]] = solstat->resc;
                y[2][m[2]] = solstat->el * R2D;
                y[3][m[3]] = solstat->snr * SNR_UNIT;

                if (!(solstat->flag >> 5)) q = 0;          // invalid
                else if ((solstat->flag & 7) <= 1) q = 2;  // float
                else if ((solstat->flag & 7) <= 3) q = 1;  // fixed
                else q = 6;                          // ppp

                col[0][m[0]] = markerColor[0][q];
                col[1][m[1]] = ((solstat->flag >> 3) & 1) ? Qt::red : markerColor[0][q];
                col[2][m[2]] = markerColor[0][1];
                col[3][m[3]] = markerColor[0][4];        // slip

                if (solstat->resp != 0.0) {
                    sum [0] += solstat->resp;
                    sum2[0] += solstat->resp * solstat->resp;
                    ns[0]++;
                }
                if (solstat->resc != 0.0) {
                    sum [1] += solstat->resc;
                    sum2[1] += solstat->resc * solstat->resc;
                    ns[1]++;
                }
                m[0]++; m[1]++; m[2]++; m[3]++;
            }
            for (int i = 0; i < 3; i++) {
                if (!btn[i]->isChecked()) continue;
                if (!level || !(plotStyle % 2)) {
                    drawPolyS(graphTriple[i], c, x[i], y[i], m[i], cColor[3], 0);
                    if (i == 2) drawPolyS(graphTriple[i], c, x[3], y[3], m[3], cColor[3], 0);
                }
                if (level && plotStyle < 2) {

                    for (int j = 0; j < m[i]; j++) {
                        graphTriple[i]->drawMark(c, x[i][j], y[i][j], Graph::MarkerTypes::Dot, col[i][j], markSize, 0);
                    }
                    if (i==2) {
                        for (int j = 0; j < m[3]; j++) {
                            graphTriple[i]->drawMark(c, x[3][j], y[3][j], Graph::MarkerTypes::Dot, col[3][j], markSize, 0);
                        }
                    }
                }
            }
        }
        for (int i = 0; i < 4; i++) {
            delete [] x[i];
            delete [] y[i];
            delete [] col[i];
        }

        if (showStats) {
            for (int i = 0; i < 2; i++) {
                if (!btn[i]->isChecked()) continue;
                QPoint p1, p2;

                double ave, std, rms;
                ave = ns[i] <= 0 ? 0.0 : sum[i] / ns[i];
                std = ns[i] <= 1 ? 0.0 : SQRT((sum2[i] - 2.0 * sum[i] * ave + ns[i] * ave * ave) / (ns[i] - 1));
                rms = ns[i] <= 0 ? 0.0 : SQRT(sum2[i] / ns[i]);
                graphTriple[i]->getExtent(p1, p2);
                p1.setX(p2.x() - 5);
                p1.ry() += 3;
                str = tr("AVE = %1 m, STD = %2 m, RMS = %3 m").arg(ave, 0, 'f', 3).arg(std, 0, 'f', 3).arg(rms, 0, 'f', 3);
                drawLabel(graphTriple[i], c, p1, str, Graph::Alignment::Right, Graph::Alignment::Top);
            }
        }
        if (btnShowTrack->isChecked() && 0 <= ind && ind < solutionData[sel].n && (btnSolution1->isChecked() || btnSolution2->isChecked())) {
            for (int i = 0, j = 0; i < 3; i++) {
                double xl[2], yl[2];

                if (!btn[i]->isChecked()) continue;

                gtime_t t = solutionData[sel].data[ind].time;

                graphTriple[i]->getLimits(xl, yl);
                xl[0] = xl[1] = timePosition(t);
                graphTriple[i]->drawPoly(c, xl, yl, 2, ind == 0 ? cColor[1] : cColor[2], 0);

                if (j++ == 0) {
                    graphTriple[i]->drawMark(c, xl[0], yl[1] - 1E-6, Graph::MarkerTypes::Dot, cColor[2], 5, 0);
                    graphTriple[i]->drawMark(c, xl[0], yl[1] - 1E-6, Graph::MarkerTypes::Circle, cColor[2], 9, 0);
                }
            }
        }
    }
    for (int i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;
        QPoint p1, p2;
        graphTriple[i]->getExtent(p1, p2);
        p1.rx() += 5;
        p1.ry() += 3;
        drawLabel(graphTriple[i], c, p1, label[i], Graph::Alignment::Left, Graph::Alignment::Top);
    }
}
// draw residuals - elevation plot ------------------------------------------
void Plot::drawResidualE(QPainter &c, int level)
{
    QPushButton *btn[] = {btnOn1, btnOn2, btnOn3};
    QString label[] = {tr("Pseudorange Residuals (m)"), tr("Carrier-Phase Residuals (m)")};
    int j, sel = !btnSolution1->isChecked() && btnSolution2->isChecked() ? 1 : 0;
    int frq = cBFrequencyType->currentIndex() + 1, n = solutionStat[sel].n;

    trace(3,"drawResidualE: level=%d\n", level);

    j = 0;
    for (int i = 0; i < 2; i++)
        if (btn[i]->isChecked()) j = i;
    for (int i = 0; i < 2; i++) {
        if (!btn[i]->isChecked()) continue;

        QPoint p1, p2;
        double xl[2] = {-0.001, 90.0};
        double yl[2][2] = {{-maxMP, maxMP}, {-maxMP / 100.0, maxMP / 100.0}};
        
        graphDual[i]->xLabelPosition = i== j ? Graph::LabelPosition::Outer : Graph::LabelPosition::On;
        graphDual[i]->yLabelPosition = Graph::LabelPosition::Outer;
        graphDual[i]->setLimits(xl, yl[i]);
        graphDual[i]->setTick(0.0, 0.0);
        graphDual[i]->drawAxis(c, 1, 1);
        graphDual[i]->getExtent(p1, p2);

        p1.setX((int)(QFontMetrics(lblDisplay->font()).height()));
        p1.setY((p1.y() + p2.y()) / 2);
        graphDual[i]->drawText(c, p1, label[i], cColor[2], Graph::Alignment::Center, Graph::Alignment::Center, 90);
        if (i == j) {
            p2.rx() -= 8;
            p2.ry() -= 6;
            graphDual[i]->drawText(c, p2, "Elevation ( ° )", cColor[2], Graph::Alignment::Right, Graph::Alignment::Bottom, 0);
        }
    }
    if (n > 0 && ((sel == 0 && btnSolution1->isChecked()) || (sel == 1 && btnSolution2->isChecked()))) {
        QColor *col[2];
        double *x[2], *y[2], sum[2] = {0}, sum2[2] = {0};
        int ns[2] = {0};

        for (int i = 0; i < 2; i++) {
            x[i] = new double[n],
            y[i] = new double[n];
            col[i] = new QColor[n];
        }
        for (int sat = 1; sat <= MAXSAT; sat++) {
            if (satelliteMask[sat - 1] || !satelliteSelection[sat - 1]) continue;
            int q, m[2] = {0};

            for (int i = 0; i < n; i++) {
                solstat_t *solstat = solutionStat[sel].data + i;
                if (solstat->sat != sat || solstat->frq != frq) continue;

                x[0][m[0]] =x [1][m[1]] = solstat->el*R2D;
                y[0][m[0]] = solstat->resp;
                y[1][m[1]] = solstat->resc;
                if (!(solstat->flag >> 5))  q = 0; // invalid
                else if ((solstat->flag & 0x7) <= 1) q = 2; // float
                else if ((solstat->flag & 0x7) <= 3) q = 1; // fixed
                else q = 6; // ppp

                col[0][m[0]] = markerColor[0][q];
                col[1][m[1]] = ((solstat->flag >> 3) & 1) ? Qt::red : markerColor[0][q];

                if (solstat->resp != 0.0) {
                    sum[0] += solstat->resp;
                    sum2[0] += solstat->resp * solstat->resp;
                    ns[0]++;
                }
                if (solstat->resc != 0.0) {
                    sum[1] += solstat->resc;
                    sum2[1] += solstat->resc * solstat->resc;
                    ns[1]++;
                }
                m[0]++;
                m[1]++;
            }
            for (int i = 0; i < 2; i++) {
                if (!btn[i]->isChecked()) continue;
                if (!level || !(plotStyle % 2)) {
                    drawPolyS(graphDual[i], c, x[i], y[i], m[i], cColor[3], 0);
                }
                if (level && plotStyle < 2) {
                    for (int j = 0; j < m[i]; j++) {
                        graphDual[i]->drawMark(c, x[i][j], y[i][j], Graph::MarkerTypes::Dot, col[i][j], markSize, 0);
                    }
                }
            }
        }
        for (int i = 0; i < 2; i++) {
            delete[] x[i];
            delete[] y[i];
            delete[] col[i];
        }
        if (showStats) {
            for (int i = 0; i < 2; i++) {
                if (!btn[i]->isChecked()) continue;

                QString str;
                QPoint p1, p2;
                double ave,std,rms;

                ave = ns[i] <= 0 ? 0.0 : sum[i] / ns[i];
                std = ns[i] <= 1 ? 0.0 : SQRT((sum2[i] - 2.0 * sum[i] * ave + ns[i] * ave * ave) / (ns[i] - 1));
                rms = ns[i] <= 0 ? 0.0 : SQRT(sum2[i] / ns[i]);
                graphDual[i]->getExtent(p1, p2);
                p1.setX(p2.x() - 5);
                p1.ry() += 3;
                str = QString("AVE = %1 m, STD = %2 m, RMS = %3 m").arg(ave, 0, 'f', 3).arg(std, 0, 'f', 3).arg(rms, 0, 'f', 3);
                drawLabel(graphTriple[i], c, p1, str, Graph::Alignment::Right, Graph::Alignment::Top);
            }
        }
    }
}
// draw polyline without time-gaps ------------------------------------------
void Plot::drawPolyS(Graph *graph, QPainter &c, double *x, double *y, int n,
                     const QColor &color, int style)
{
    int i, j;

    for (i = 0; i < n; i = j) {
        for (j = i + 1; j < n; j++)
            if (fabs(x[j] - x[j - 1]) > TBRK) break;
        graph->drawPoly(c, x + i, y + i, j - i, color, style);
    }
}
// draw label with hemming --------------------------------------------------
void Plot::drawLabel(Graph *g, QPainter &c, const QPoint &p, const QString &label, int ha, int va)
{
    g->drawText(c, p, label, cColor[2], cColor[0], ha, va, 0);
}
// draw mark with hemming ---------------------------------------------------
void Plot::drawMark(Graph *g, QPainter &c, const QPoint &p, int mark, const QColor &color,
            int size, int rot)
{
    g->drawMark(c, p, mark, color, cColor[0], size, rot);
}
// refresh map view --------------------------------------------------
void Plot::refresh_MapView(void)
{
    sol_t *sol;
    double pos[3] = { 0 };

    if (btnShowTrack->isChecked()) {
        if (btnSolution2->isChecked() && solutionData[1].n > 0 &&
                (sol = getsol(solutionData+1, solutionIndex[1]))) {
            ecef2pos(sol->rr, pos);
            mapView->setMark(2, "Solution 2", pos[0]*R2D, pos[1]*R2D);
            mapView->showMark(2);

        } else {
            mapView->hideMark(2);
        }
        if (btnSolution1->isChecked() && solutionData[0].n > 0 &&
                (sol = getsol(solutionData, solutionIndex[0]))) {
            ecef2pos(sol->rr, pos);
            mapView->setMark(1, "Solution 1", pos[0]*R2D, pos[1]*R2D);
            mapView->showMark(1);
        } else {
            mapView->hideMark(1);
        }
    } else {
        mapView->hideMark(1);
        mapView->hideMark(2);
    }

}
