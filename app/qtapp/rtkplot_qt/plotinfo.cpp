//---------------------------------------------------------------------------
// plotinfo.c: rtkplot info functions
//---------------------------------------------------------------------------
#include "plotmain.h"

#include "rtklib.h"

#define ATAN2(x, y)  ((x) * (x) + (y) * (y) > 1E-12 ? atan2(x, y) : 0.0)

// update information on status-bar -----------------------------------------
void Plot::updateInfo(void)
{
    int showObs = (PLOT_OBS <= plotType && plotType <= PLOT_DOP) ||
              plotType == PLOT_SNR || plotType == PLOT_SNRE || plotType == PLOT_MPS;

    trace(3, "updateInfo:\n");

    if (btnShowTrack->isChecked()) {
        if (showObs) updateTimeObservation();
        else updateTimeSolution();
    } else {
        if (showObs) updateInfoObservation();
        else updateInfoSolution();
    }
}
// update time-information for observation-data plot ------------------------
void Plot::updateTimeObservation(void)
{
    QString msgs1[] = { " #FRQ=5 ", " 4 ", " 3 ", " 2 ", " 1", "", "" };
    QString msgs2[] = { " SNR=...45.", "..40.", "..35.", "..30.", "..25 ", "", " <25 " };
    QString msgs3[] = { " SYS=GPS ", "GLO ", "GAL ", "QZS ", "BDS ", "IRN ", "SBS" };
    QString msgs4[] = { " MP=..0.6", "..0.3", "..0.0..", "-0.3..", "-0.6..", "", "" };
    QList<QString> msgs;
    QString msg;
    double azel[MAXOBS * 2], dop[4] = { 0 };
    int i, ns = 0, no = 0;
    QString tstr;

    trace(3, "updateTimeObservation\n");

    if (btnSolution1->isChecked() && observationIndex >= 0 && observationIndex < nObservation) {
        for (i = indexObservation[observationIndex]; i < observation.n && i < indexObservation[observationIndex + 1]; i++, no++) {
            if (satelliteMask[observation.data[i].sat - 1] || !satelliteSelection[observation.data[i].sat - 1]) continue;
            if (elevation[i] < elevationMask * D2R) continue;
            if (elevationMaskEnabled && elevation[i] < elevationMaskData[static_cast<int>(azimuth[i] * R2D + 0.5)]) continue;
            azel[ns * 2] = azimuth[i];
            azel[1 + ns * 2] = elevation[i];
            ns++;
        }
    }
    if (ns >= 0) {
        dops(ns, azel, elevationMask * D2R, dop);

        timeString(observation.data[indexObservation[observationIndex]].time, 3, 1, tstr);
        msg = QString("[1] %1 : N = %2 ").arg(tstr).arg(no);

        if (plotType == PLOT_DOP) {
            msgs.append(QString("NSAT = %1").arg(ns));
            msgs.append(QString(" GDOP = %1").arg(dop[0], 0, 'f', 1));
            msgs.append(QString(" PDOP = %1").arg(dop[1], 0, 'f', 1));
            msgs.append(QString(" HDOP = %1").arg(dop[2], 0, 'f', 1));
            msgs.append(QString(" VDOP = %1").arg(dop[3], 0, 'f', 1));
        } else if (plotType <= PLOT_SKY && cBObservationType->currentIndex() == 0) {
            msg += QString("NSAT = %1 ").arg(ns);
            for (i = 0; i < 7; i++) msgs.append(simulatedObservation ? msgs3[i] : msgs1[i]);
        } else if (plotType == PLOT_MPS) {
            msg += QString("NSAT = %1 ").arg(ns);
            for (i = 0; i < 7; i++) msgs.append(msgs4[i]);
        } else {
            msg += QString("NSAT = %1 ").arg(ns);
            for (i = 0; i < 7; i++) msgs.append(simulatedObservation ? msgs3[i] : msgs2[i]);
        }
    }

    showMessage(msg);
    showLegend(msgs);
}
// update time-information for solution plot --------------------------------
void Plot::updateTimeSolution(void)
{
    const char *unit[] = { "m", "m/s", "m/s²" };
    const QString sol[] = { tr(""), tr("FIX"), tr("FLOAT"), tr("SBAS"), tr("DGPS"), tr("Single"), tr("PPP") };
    QString msg;
    QList<QString> msgs;
    sol_t *data;
    double pos[3], r, az, el;
    int sel = btnSolution1->isChecked() || !btnSolution2->isChecked() ? 0 : 1, solIndex = solutionIndex[sel];
    QString tstr;

    trace(3, "updateTimeSolution\n");

    if ((btnSolution1->isChecked() || btnSolution2->isChecked() || btnSolution12->isChecked()) &&
        (data = getsol(solutionData + sel, solIndex))) {
        if (!connectState) msg = QString("[%1] ").arg(sel + 1);
        else msg = "[R]";

        timeString(data->time, 2, 1, tstr);
        msg += tstr;
        msg += " : ";

        if (PLOT_SOLP <= plotType && plotType <= PLOT_SOLA) {
            TIMEPOS *p = solutionToPosition(solutionData+sel, solIndex, 0, plotType-PLOT_SOLP);
            msg += QString("E = %1%2, N = %3%2, U = %4%2, Q = ")
                   .arg(p->x[0], 7, 'f', 4).arg(unit[plotType - PLOT_SOLP]).arg(p->y[0], 7, 'f', 4).arg(p->z[0], 7, 'f', 4);
            delete p;
        } else if (plotType == PLOT_NSAT) {
            msg += QString("NS = %1, AGE = %2, RATIO = %3, Q = ").arg(data->ns).arg(data->age, 0, 'f', 1)
                   .arg(data->ratio, 0, 'f', 1);
        } else if (!data->type) {
            ecef2pos(data->rr, pos);
            msg += latLonString(pos, 9) + QString(", %1 m,  Q = ").arg(pos[2], 9, 'f', 4);
        } else {
            r = norm(data->rr, 3);
            az = norm(data->rr, 2) <= 1E-12 ? 0.0 : atan2(data->rr[0], data->rr[1]) * R2D;
            el = r <= 1E-12 ? 0.0 : asin(data->rr[2] / r) * R2D;
            msg += QString("B = %1 m, D = %2%3, %4%3,  Q = ")
                   .arg(r, 0, 'f', 3).arg(az < 0.0 ? az + 360.0 : az, 6, 'f', 2).arg(degreeChar).arg(el, 5, 'f', 2);
        }
        if (1 <= data->stat && data->stat <= 6)
            msgs.append(QString("%1:%2").arg(data->stat).arg(sol[data->stat]));
    }
    showMessage(msg);
    showLegend(msgs);
}
// update statistics-information for observation-data plot ------------------
void Plot::updateInfoObservation(void)
{
    QString msgs0[] = { " NSAT", " GDOP", " PDOP", " HDOP", " VDOP", "", "" };
    QString msgs1[] = { " #FRQ=5 ", " 4 ", " 3 ", " 2 ", " 1 ", "", "" };
    QString msgs2[] = { " SNR=...45.", "..40.", "..35.", "..30.", "..25 ", "", " <25 " };
    QString msgs3[] = { " SYS=GPS ", "GLO ", "GAL ", "QZS ", "BDS ", "IRN ", "SBS" };
    QString msgs4[] = { " MP=..0.6", "..0.3", "..0.0..", "-0.3..", "-0.6..", "", "" };
    QString msg;
    QList<QString> msgs;
    gtime_t timeStart = { 0, 0 }, timeEnd = { 0, 0 }, time, previousTime = { 0, 0 };
    int i, n = 0, ne = 0, p;
    QString timeStartString, timeEndString;

    trace(3, "updateInfoObservation:\n");

    if (btnSolution1->isChecked()) {
        for (i = 0; i < observation.n; i++) {
            time = observation.data[i].time;
            if (timeStart.time == 0) timeStart = time; // save start time
            timeEnd = time;  // update end time
            if (previousTime.time == 0 || timediff(time, previousTime) > TTOL) ne++;
            n++;
            previousTime = time;
        }
    }
    if (n > 0) {
        timeString(timeStart, 0, 0, timeStartString);
        timeString(timeEnd, 0, 1, timeEndString);
        if (timeFormat && (p = timeStartString.indexOf(' '))) timeStartString = timeStartString.left(p);
        timeEndString = timeEndString.mid(timeFormat ? 5 : 0);
        msg = QString("[1] %1-%2 : EP = %3, N = %4").arg(timeStartString).arg(timeEndString).arg(ne).arg(n);

        for (i = 0; i < 7; i++) {
            if (plotType == PLOT_DOP) {
                msgs.append(msgs0[i]);
            } else if (plotType <= PLOT_SKY && cBObservationType->currentIndex() == 0) {
                msgs.append(simulatedObservation ? msgs3[i] : msgs1[i]);
            } else if (plotType == PLOT_MPS) {
                msgs.append(msgs4[i]);
            } else if (plotType == PLOT_SNRE) {
                msgs.append("");
            } else {
                msgs.append(simulatedObservation ? msgs3[i] : msgs2[i]);
            }
        }
    }
    showMessage(msg);
    showLegend(msgs);
}
// update statistics-information for solution plot --------------------------
void Plot::updateInfoSolution(void)
{
    QString msg, s;
    QList<QString> msgs;
    TIMEPOS *pos = NULL, *pos1, *pos2;
    sol_t *data;
    gtime_t timeStart = { 0, 0 }, timeEnd = { 0, 0 };
    double r[3], baseline, baselineMinMax[2] = { 1E9, 0.0 };
    int i, j,p, n = 0, nq[8] = { 0 }, sel = btnSolution1->isChecked() || !btnSolution2->isChecked() ? 0 : 1;
    QString timeStartString, timeEndString;

    trace(3, "updateInfoSolution:\n");

    if (btnSolution1->isChecked() || btnSolution2->isChecked()) {
        pos = solutionToPosition(solutionData + sel, -1, 0, 0);
    } else if (btnSolution12->isChecked()) {
        pos1 = solutionToPosition(solutionData, -1, 0, 0);
        pos2 = solutionToPosition(solutionData + 1, -1, 0, 0);
        pos = pos1->diff(pos2, 0);
        delete pos1;
        delete pos2;
    }
    if (pos) {
        for (i = 0; i < pos->n; i++) {
            if (timeStart.time == 0) timeStart = pos->t[i];
            timeEnd = pos->t[i];
            nq[pos->q[i]]++;
            n++;
        }
        delete pos;
    }
    for (i = 0; (data = getsol(solutionData + sel, i)) != NULL; i++) {
        if (data->type) {
            baseline = norm(data->rr, 3);
        } else if (norm(solutionData[sel].rb, 3) > 0.0) {
            for (j = 0; j < 3; j++) r[j] = data->rr[j] - solutionData[sel].rb[j];
            baseline = norm(r, 3);
        } else {
            baseline = 0.0;
        }
        if (baseline < baselineMinMax[0]) baselineMinMax[0] = baseline;
        if (baseline > baselineMinMax[1]) baselineMinMax[1] = baseline;
    }
    if (n > 0) {
        if (!connectState) msg = QString("[%1] ").arg(sel + 1);
        else msg = "[R]";

        timeString(timeStart, 0, 0, timeStartString);
        timeString(timeEnd, 0, 1, timeEndString);
        if (timeFormat && (p = timeStartString.indexOf(' '))) timeStartString = timeStartString.left(p);
        timeEndString = timeEndString.mid(timeFormat ? 5 : 0);
        msg += QString("%1-%2 : N = %3").arg(timeStartString).arg(timeEndString).arg(n);

        if (baselineMinMax[0] - baselineMinMax[1] > 100)
            msg += QString(", B = %1-%2 km").arg(baselineMinMax[0] / 1E3, 0, 'f', 1).arg(baselineMinMax[1] / 1E3, 0, 'f', 1);
        else
            msg += QString(", B = %1 km").arg(baselineMinMax[0] / 1E3, 0, 'f', 1);
        msg += ", Q = ";

        for (i = 1; i <= 6; i++) {
            if (nq[i] <= 0) msgs.append("");
            else msgs.append(QString("%1: %2 (%3%) ").arg(i).arg(nq[i]).arg(static_cast<double>(nq[i]) / n * 100.0, 0, 'f', 1));
        }
    }
    showMessage(msg);
    showLegend(msgs);
}
// update plot-type pull-down menu ------------------------------------------
void Plot::updatePlotType(void)
{
    int i;

    trace(3, "updatePlotType\n");
    
    cBPlotTypeSelection->blockSignals(true);
    cBPlotTypeSelection->clear();
    if (solutionData[0].n > 0 || solutionData[1].n > 0 ||
        (nObservation <= 0 && solutionStat[0].n <= 0 && solutionStat[1].n <= 0)) {
        cBPlotTypeSelection->addItem(PTypes[PLOT_TRK]);
        cBPlotTypeSelection->addItem(PTypes[PLOT_SOLP]);
        cBPlotTypeSelection->addItem(PTypes[PLOT_SOLV]);
        cBPlotTypeSelection->addItem(PTypes[PLOT_SOLA]);
        cBPlotTypeSelection->addItem(PTypes[PLOT_NSAT]);
    }
    if (nObservation > 0) {
        cBPlotTypeSelection->addItem(PTypes[PLOT_OBS]);
        cBPlotTypeSelection->addItem(PTypes[PLOT_SKY]);
        cBPlotTypeSelection->addItem(PTypes[PLOT_DOP]);
    }
    if (solutionStat[0].n > 0 || solutionStat[1].n > 0) {
        cBPlotTypeSelection->addItem(PTypes[PLOT_RES]);
        cBPlotTypeSelection->addItem(PTypes[PLOT_RESE]);
    }
    if (nObservation > 0) {
        cBPlotTypeSelection->addItem(PTypes[PLOT_SNR]);
        cBPlotTypeSelection->addItem(PTypes[PLOT_SNRE]);
        cBPlotTypeSelection->addItem(PTypes[PLOT_MPS]);
    }
    for (i = 0; i < cBPlotTypeSelection->count(); i++) {
        if (cBPlotTypeSelection->itemText(i) != PTypes[plotType]) continue;
        cBPlotTypeSelection->setCurrentIndex(i);
        cBPlotTypeSelection->blockSignals(false);
        return;
    }
    cBPlotTypeSelection->setCurrentIndex(0);
    
    cBPlotTypeSelection->blockSignals(false);
}
// update satellite-list pull-down menu -------------------------------------
void Plot::updateSatelliteList(void)
{
    int i, j, sys, previousSys = 0, sat, satMask[MAXSAT] = { 0 };
    char s[8];

    trace(3, "updateSatelliteList\n");

    for (i = 0; i < 2; i++)
        for (j = 0; j < solutionStat[i].n; j++) {
            sat = solutionStat[i].data[j].sat;
            if (1 <= sat && sat <= MAXSAT) satMask[sat - 1] = 1;
        }
    for (j = 0; j < observation.n; j++) {
        sat = observation.data[j].sat;
        if (1 <= sat && sat <= MAXSAT) satMask[sat - 1] = 1;
    }
    cBSatelliteList->clear();
    cBSatelliteList->addItem("ALL");

    // add satellite systems
    for (sat = 1; sat <= MAXSAT; sat++) {
        if (satelliteMask[sat - 1] || !satMask[sat - 1]) continue;
        if ((sys = satsys(sat, NULL)) == previousSys) continue;
        switch (sys) {
            case SYS_GPS: cBSatelliteList->addItem("G"); break;
            case SYS_GLO: cBSatelliteList->addItem("R"); break;
            case SYS_GAL: cBSatelliteList->addItem("E"); break;
            case SYS_QZS: cBSatelliteList->addItem("J"); break;
            case SYS_CMP: cBSatelliteList->addItem("C"); break;
            case SYS_IRN: cBSatelliteList->addItem("I"); break;
            case SYS_SBS: cBSatelliteList->addItem("S"); break;
        };
        previousSys = sys;
    }

    // add individual satellites
    for (sat = 1; sat <= MAXSAT; sat++) {
        if (satelliteMask[sat - 1] || !satMask[sat - 1]) continue;
        satno2id(sat, s);
        cBSatelliteList->addItem(s);
    }
    cBSatelliteList->setCurrentIndex(0);

    updateSatelliteSelection();
}
// string compare --------------------------------------------------------------
static int _strcmp(const void *str1, const void *str2)
{
    return strcmp(*(const char **)str1, *(const char **)str2);
}
// update observation type pull-down menu --------------------------------------
void Plot::updateObservationType(void)
{
    char *codes[MAXCODE + 1], *obs;
    int i, j, n = 0, codeMask[MAXCODE + 1] = { 0 };

    trace(3, "updateObservationType\n");

    for (i = 0; i < observation.n; i++)
        for (j = 0; j < NFREQ + NEXOBS; j++)
            codeMask[observation.data[i].code[j]] = 1;

    for (unsigned char c = 1; c <= MAXCODE; c++) {
        if (!codeMask[c]) continue;
        if (!*(obs = code2obs((uint8_t)c))) continue;
        codes[n++] = obs;
    }
    qsort(codes, n, sizeof(char *), _strcmp);
    cBObservationType->clear();
    cBObservationType2->clear();
    cBObservationType->addItem(tr("ALL"));

    for (i = 0; i < NFREQ; i++) {
        cBObservationType->addItem(QString("L%1").arg(i+1));
        cBObservationType2->addItem(QString("L%1").arg(i+1));
    }
    for (i = 0; i < n; i++) {
        cBObservationType->addItem(QString("L%1").arg(codes[i]));
        cBObservationType2->addItem(QString("L%1").arg(codes[i]));
    }
    cBObservationType->setCurrentIndex(0);
    cBObservationType2->setCurrentIndex(0);
}
// update information for current-cursor position ---------------------------
void Plot::updatePoint(int x, int y)
{
    gtime_t time;
    QPoint p =lblDisplay->mapFromGlobal(QPoint(x, y));
    double enu[3] = { 0 }, rr[3], pos[3], xx, yy, r, xl[2], yl[2], q[2], az, el;
    int i;
    QString tstr;
    QString msg;

    trace(4, "updatePoint: x=%d y=%d\n", x, y);

    if (plotType == PLOT_TRK) { // track-plot
        graphTrack->toPos(p, enu[0], enu[1]);

        if (pointType == 1 || norm(originPosition, 3) <= 0.0) {
            msg = QString("E: %1 m, N: %2 m").arg(enu[0], 0, 'f', 4).arg(enu[1], 0, 'f', 4);
        } else if (pointType == 2) {
            r = norm(enu, 2);
            az = r <= 0.0 ? 0.0 : ATAN2(enu[0], enu[1]) * R2D;
            if (az < 0.0) az += 360.0;
            msg = QString("R: %1 m, D: %2%3").arg(r, 0, 'f', 4).arg(az, 5, 'f', 1).arg(degreeChar);
        } else {
            ecef2pos(originPosition, pos);
            enu2ecef(pos, enu, rr);
            for (i = 0; i < 3; i++) rr[i] += originPosition[i];
            ecef2pos(rr, pos);
            msg = latLonString(pos, 8);
        }
    } else if (plotType == PLOT_SKY || plotType == PLOT_MPS) { // sky-plot
        graphSky->getLimits(xl, yl);
        graphSky->toPos(p, q[0], q[1]);
        r = (xl[1] - xl[0] < yl[1] - yl[0] ? xl[1] - xl[0] : yl[1] - yl[0]) * 0.45;

        if ((el = 90.0 - 90.0 * norm(q, 2) / r) > 0.0) {
            az = el >= 90.0 ? 0.0 : ATAN2(q[0], q[1]) * R2D;
            if (az < 0.0) az += 360.0;
            msg = QString("AZ=%1%2, EL=%3%4").arg(az, 5, 'f', 1).arg(degreeChar).arg(el, 4, 'f', 1).arg(degreeChar);
        }
    } else if (plotType == PLOT_SNRE||plotType==PLOT_RESE) { // snr-el-plot
        graphDual[0]->toPos(p, q[0], q[1]);
        msg = QString("EL=%1%2").arg(q[0], 4, 'f', 1).arg(degreeChar);
    } else {
        graphTriple[0]->toPos(p, xx, yy);
        time = gpst2time(week, xx);
        if (timeFormat == 2) time = utc2gpst(time);                              // UTC
        else if (timeFormat == 3) time = timeadd(gpst2utc(time), -9 * 3600.0);   // JST
        timeString(time, 0, 1, tstr);
        msg = tstr;
    }
    lblMessage2->setVisible(true);
    lblMessage2->setText(msg);
}
//---------------------------------------------------------------------------
