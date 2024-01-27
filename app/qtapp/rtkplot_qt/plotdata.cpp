//---------------------------------------------------------------------------
// plotdata : rtkplot data functions
//---------------------------------------------------------------------------
#include <QString>
#include <QStringList>
#include <QFile>
#include <QImage>
#include <QColor>
#include <QDir>
#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFileInfo>

#include "rtklib.h"
#include "plotmain.h"
#include "mapoptdlg.h"
#include "pntdlg.h"

#define MAX_SIMOBS   16384              // max generated obs epochs
#define MAX_SKYIMG_R 2048               // max size of resampled sky image

static char path_str[MAXNFILE][1024];
static const char *XMLNS = "http://www.topografix.com/GPX/1/1";

// read solutions -----------------------------------------------------------
void Plot::readSolution(const QStringList &files, int sel)
{
    solbuf_t sol;
    gtime_t ts, te;
    double tint, ep[6];
    int i, n = 0;
    char *paths[MAXNFILE];

    trace(3, "readSolution: sel=%d\n", sel);

    setlocale(LC_NUMERIC, "C"); // use point as decimal separator in formatted output

    memset(&sol, 0, sizeof(solbuf_t));

    for (i = 0; i < MAXNFILE; i++) paths[i] = path_str[i];

    if (files.count() <= 0) return;

    readWaitStart();
    /* Set default to current date in case no date info in solution  (e.g. GGA msgs)*/
    QFileInfo fi(files.first());
    QDateTime st = fi.birthTime();
    ep[0] = st.date().year();
    ep[1] = st.date().month();
    ep[2] = st.date().day();
    ep[3] = st.time().hour();
    ep[4] = st.time().minute();
    ep[5] = st.time().second();
    sol.time = utc2gpst(epoch2time(ep));

    for (i = 0; i < files.count() && n < MAXNFILE; i++)
        strncpy(paths[n++], qPrintable(files.at(i)), 1023);
    timeSpan(&ts, &te, &tint);

    showMessage(tr("Reading %1...").arg(files.first()));
    showLegend(QList<QString>());

    if (!readsolt(paths, n, ts, te, tint, 0, &sol)) {
        showMessage(tr("No solution data: %1...").arg(files.first()));
        showLegend(QStringList());
        readWaitEnd();
        return;
    }
    freesolbuf(solutionData + sel);
    solutionData[sel] = sol;

    if (solutionFiles[sel] != files)
        solutionFiles[sel] = files;
    setWindowTitle("");

    readSolutionStat(files, sel);

    for (i = 0; i < 2; i++) {
        if (solutionFiles[i].length() == 0) continue;
        setWindowTitle(windowTitle() + solutionFiles[i].first() + (solutionFiles[i].count() > 1 ? "... " : " "));
    }
    btnSolution12->setChecked(false);
    if (sel == 0) btnSolution1->setChecked(true);
    else btnSolution2->setChecked(true);

    if (sel == 0 || solutionData[0].n <= 0) {
        time2gpst(solutionData[sel].data[0].time, &week);
        updateOrigin();
    }
    solutionIndex[0] = solutionIndex[1] = observationIndex = 0;
    sBTime->setValue(0);

    GEDataState[sel] = 0;

    if (plotType > PLOT_NSAT)
        updateType(PLOT_TRK);
    else
        updatePlotType();
    fitTime();
    if (autoScale && plotType <= PLOT_SOLA)
        fitRange(1);
    else
        setRange(1, yRange);
    readWaitEnd();

    updateTime();
    updatePlot();
    updateEnable();
}
// read solution status -----------------------------------------------------
void Plot::readSolutionStat(const QStringList &files, int sel)
{
    gtime_t ts, te;
    double tint;
    int i, n = 0;
    char *paths[MAXNFILE];

    trace(3, "readSolutionStat\n");

    setlocale(LC_NUMERIC, "C"); // use point as decimal separator in formatted output

    freesolstatbuf(solutionStat + sel);

    for (i = 0; i < MAXNFILE; i++) paths[i] = path_str[i];

    timeSpan(&ts, &te, &tint);

    for (i = 0; i < files.count() && n < MAXNFILE; i++)
        strncpy(paths[n++], qPrintable(files.at(i)), 1023);
    showMessage(tr("Reading %1...").arg(files.first()));
    showLegend(QStringList());

    readsolstatt(paths, n, ts, te, tint, solutionStat + sel);

    updateSatelliteList();
}
// read observation data ----------------------------------------------------
void Plot::readObservation(const QStringList &files)
{
    obs_t obs = {};
    nav_t nav = {};
    sta_t sta = {};
    int nobs;

    trace(3, "readObservation\n");

    setlocale(LC_NUMERIC, "C"); // use point as decimal separator in formatted output

    if (files.size() <= 0) return;

    readWaitStart();
    showLegend(QStringList());

    if ((nobs = readObservationRinex(files, &obs, &nav, &sta)) <= 0) {
        readWaitEnd();
        return;
    }
    clearObservation();

    observation = obs;
    navigation = nav;
    station = sta;
    simulatedObservation = 0;

    updateObservation(nobs);
    updateMp();

    if (observationFiles != files)
        observationFiles = files;
    navigationFiles.clear();

    setWindowTitle(files.first() + (files.size() > 1 ? "..." : ""));

    btnSolution1->setChecked(true);

    time2gpst(observation.data[0].time, &week);
    solutionIndex[0] = solutionIndex[1] = observationIndex = 0;

    if (plotType < PLOT_OBS || PLOT_DOP < plotType)
        updateType(PLOT_OBS);
    else
        updatePlotType();
    fitTime();

    readWaitEnd();

    updateObservationType();
    updateTime();
    updatePlot();
    updateEnable();
}
// read observation data rinex ----------------------------------------------
int Plot::readObservationRinex(const QStringList &files, obs_t *obs, nav_t *nav, sta_t *sta)
{
    gtime_t ts, te;
    double tint;
    int i, n;
    char navfile[1024] = "", *p, *q, opt[2048];

    strncpy(opt, qPrintable(rinexOptions), 2047);

    trace(3, "readObservationRinex\n");

    setlocale(LC_NUMERIC, "C"); // use point as decimal separator in formatted output

    timeSpan(&ts, &te, &tint);

    for (i = 0; i < files.count(); i++) {
        showMessage(tr("Reading observation data... %1").arg(QDir::toNativeSeparators(files.at(i))));
        qApp->processEvents();

        if (readrnxt(qPrintable(QDir::toNativeSeparators(files.at(i))), 1, ts, te, tint, opt, obs, nav, sta) < 0) {
            showMessage(tr("Error: insufficient memory"));
            return -1;
        }
    }
    showMessage(tr("Reading navigation data..."));
    qApp->processEvents();

    for (i = 0; i < files.count(); i++) {
        strncpy(navfile, qPrintable(QDir::toNativeSeparators(files.at(i))), 1023);

        if (!(p = strrchr(navfile, '.'))) continue;

        if (!strcmp(p, ".obs") || !strcmp(p, ".OBS")) {
            strncpy(p, ".nav", 5); readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);
            strncpy(p, ".gnav", 6); readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);
            strncpy(p, ".hnav", 6); readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);
            strncpy(p, ".qnav", 6); readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);
            strncpy(p, ".lnav", 6); readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);
            strncpy(p, ".cnav", 6); readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);
            strncpy(p, ".inav", 6); readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);
        } else if (!strcmp(p + 3, "o") || !strcmp(p + 3, "d") ||
               !strcmp(p + 3, "O") || !strcmp(p + 3, "D")) {
            n = nav->n;

            strncpy(p + 3, "N", 2); readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);
            strncpy(p + 3, "G", 2); readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);
            strncpy(p + 3, "H", 2); readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);
            strncpy(p + 3, "Q", 2); readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);
            strncpy(p + 3, "L", 2); readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);
            strncpy(p + 3, "C", 2); readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);
            strncpy(p + 3, "I", 2); readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);
            strncpy(p + 3, "P", 2); readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);

            if (nav->n > n || !(q = strrchr(navfile, '\\'))) continue;

            // read brdc navigation data
            memcpy(q + 1, "BRDC", 4);
            strncpy(p + 3, "N", 2);
            readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);
        } else if (!strcmp(p - 1, "O.rnx" ) && (p = strrchr(navfile, '_'))) { /* rinex 3 */
            *p = '\0';
            if (!(p = strrchr(navfile, '_'))) continue;
            *p = '\0';
            if (!(p = strrchr(navfile, '_'))) continue;
            strncpy(p, "_*_*N.rnx", 10);

            n = nav->n;
            readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);

            if (nav->n > n || !(q = strrchr(navfile, '\\'))) continue;

            // read brdc navigation data
            memcpy(q + 1, "BRDC", 4);
            readrnxt(navfile, 1, ts, te, tint, opt, NULL, nav, NULL);
        }
    }
    if (obs->n <= 0) {
        showMessage(QString(tr("No observation data: %1...")).arg(files.at(0)));
        qApp->processEvents();
        freenav(nav, 0xFF);
        return 0;
    }
    uniqnav(nav);
    return sortobs(obs);
}
// read navigation data -----------------------------------------------------
void Plot::readNavigation(const QStringList &files)
{
    gtime_t ts, te;
    double tint;
    int i;

    trace(3, "readNavigation\n");

    if (files.size() <= 0) return;

    setlocale(LC_NUMERIC, "C"); // use point as decimal separator in formatted output

    readWaitStart();
    showLegend(QStringList());

    timeSpan(&ts, &te, &tint);

    freenav(&navigation, 0xFF);

    showMessage(tr("Reading navigation data..."));

    qApp->processEvents();

    for (i = 0; i < files.size(); i++) {
        readrnxt(qPrintable(QDir::toNativeSeparators(files.at(i))), 1, ts, te, tint,
                 qPrintable(rinexOptions), NULL, &navigation, NULL);
    }
    uniqnav(&navigation);

    if (navigation.n <= 0 && navigation.ng <= 0 && navigation.ns <= 0) {
        showMessage(QString(tr("No navigation message: %1...")).arg(QDir::toNativeSeparators(files.at(i))));
        readWaitEnd();
        return;
    }
    if (navigationFiles != files)
        navigationFiles = files;
    for (i = 0; i < navigationFiles.size(); i++)
        navigationFiles[i] = QDir::toNativeSeparators(navigationFiles.at(i));

    updateObservation(nObservation);
    updateMp();
    readWaitEnd();

    updatePlot();
    updateEnable();
}
// read elevation mask data -------------------------------------------------
void Plot::readElevationMaskData(const QString &file)
{
    QFile fp(file);
    double az0 = 0.0, el0 = 0.0, az1, el1;
    int i, j;
    QByteArray buff;

    trace(3, "readElevationMaskData\n");

    for (i = 0; i <= 360; i++)
        elevationMaskData[i] = 0.0;

    if (!fp.open(QIODevice::ReadOnly)) {
        showMessage(QString(tr("No elevation mask data: %1...")).arg(file));
        showLegend(QStringList());
        return;
    }
    while (!fp.atEnd()) {
        buff = fp.readLine();

        if (buff.at(0) == '%') continue;
        QList<QByteArray> tokens = buff.split(' ');
        if (tokens.size() != 2) continue;
        bool okay;
        az1 = tokens.at(0).toDouble(&okay); if (!okay) continue;
        el1 = tokens.at(1).toDouble(&okay); if (!okay) continue;

        if (az0 < az1 && az1 <= 360.0 && 0.0 <= el1 && el1 <= 90.0) {
            for (j = static_cast<int>(az0); j < static_cast<int>(az1); j++)
                elevationMaskData[j] = el0 * D2R;
            elevationMaskData[j] = el1 * D2R;
        }
        az0 = az1; el0 = el1;
    }
    updatePlot();
    updateEnable();
}
// generate visibility data ----------------------------------------------------
void Plot::generateVisibilityData(void)
{
    gtime_t time, ts, te;
    obsd_t data = {};
    double tint, pos[3], rr[3], rs[6], e[3], azel[2];
    unsigned char i, j;
    int nobs = 0;
    char name[16];

    trace(3, "generateVisibilityData\n");

    clearObservation();
    simulatedObservation = 1;

    ts = timeStart;
    te = timeEnd;
    tint = timeInterval;
    matcpy(pos, ooPosition, 3, 1);
    pos2ecef(pos, rr);

    readWaitStart();
    showLegend(QStringList());
    showMessage(tr("Generating satellite visibility..."));
    qApp->processEvents();

    for (time = ts; timediff(time, te) <= 0.0; time = timeadd(time, tint)) {
        for (i = 0; i < MAXSAT; i++) {
            satno2id(i + 1, name);
            if (!tle_pos(time, name, "", "", &tleData, NULL, rs)) continue;
            if ((geodist(rs, rr, e)) <= 0.0) continue;
            if (satazel(pos, e, azel) <= 0.0) continue;
            if (observation.n >= observation.nmax) {
                observation.nmax = observation.nmax <= 0 ? 4096 : observation.nmax * 2;
                observation.data = static_cast<obsd_t *>(realloc(observation.data, sizeof(obsd_t) * observation.nmax));
                if (!observation.data) {
                    observation.n = observation.nmax = 0;
                    break;
                }
            }
            data.time = time;
            data.sat = i + 1;

            for (j = 0; j < NFREQ; j++) {
                data.P[j] = data.L[j] = 0.0;
                data.code[j] = CODE_NONE;
            }
            data.code[0] = CODE_L1C;
            observation.data[observation.n++] = data;
        }
        if (++nobs >= MAX_SIMOBS) break;
    }
    if (observation.n <= 0) {
        readWaitEnd();
        showMessage(tr("No satellite visibility"));
        return;
    }
    updateObservation(nobs);

    setWindowTitle(tr("Satellite Visibility (Predicted)"));
    btnSolution1->setChecked(true);
    time2gpst(observation.data[0].time, &week);
    solutionIndex[0] = solutionIndex[1] = observationIndex = 0;
    if (plotType < PLOT_OBS || PLOT_DOP < plotType)
        updateType(PLOT_OBS);
    else
        updatePlotType();

    fitTime();

    readWaitEnd();

    updateObservationType();
    updateTime();
    updatePlot();
    updateEnable();
}
// read map image data ------------------------------------------------------
void Plot::readMapData(const QString &file)
{
    QImage image;
    double pos[3];

    trace(3, "readMapData\n");

    showMessage(tr("Reading map image... %1").arg(file));

    if (!image.load(file)) {
        showMessage(tr("Map file read error: %1").arg(file));
        showLegend(QStringList());
        return;
    }
    mapImage = image;
    mapImageFile = file;
    mapSize[0] = mapImage.width();
    mapSize[1] = mapImage.height();

    readMapTag(file);

    if (norm(originPosition, 3) <= 0.0 && (mapLatitude != 0.0 || mapLongitude != 0.0)) {
        pos[0] = mapLatitude * D2R;
        pos[1] = mapLongitude * D2R;
        pos[2] = 0.0;
        pos2ecef(pos, originPosition);
    }

    btnShowImage->setChecked(true);

    mapOptDialog->updateField();
    updatePlot();
    updateOrigin();
    updateEnable();
    showMessage("");
}
// resample image pixel -----------------------------------------------------
#define ResPixelNN(img1, x, y, b1, pix) { \
        int ix = static_cast<int>((x) + 0.5); \
        int iy = static_cast<int>((y) + 0.5); \
        pix = img1.pixel(ix, iy); \
}
#define ResPixelBL(img1, x, y, b1, pix) { \
        int ix = static_cast<int>(x), iy = static_cast<int>(y); \
        double dx1 = (x) - ix, dy1 = (y) - iy; \
        double dx2 = 1.0 - dx1, dy2 = 1.0 - dy1; \
        double a1 = dx2 * dy2, a2 = dx2 * dy1, a3 = dx1 * dy2, a4 = dx1 * dy1; \
        QRgb p1 = img1.pixel(ix, iy); \
        QRgb p2 = img1.pixel(ix, iy + 1); \
        pix = qRgb(static_cast<quint8>(a1 * qRed(p1) + a2 * qRed(p2) + a3 * qRed(p1) + a4 * qRed(p2)), \
               static_cast<quint8>(a1 * qRed(p1) + a2 * qGreen(p2) + a3 * qGreen(p1) + a4 * qGreen(p2)), \
               static_cast<quint8>(a1 * qRed(p1) + a2 * qBlue(p2) + a3 * qBlue(p1) + a4 * qBlue(p2))); \
}
// rotate coordinates roll-pitch-yaw ---------------------------------------
static void RPY(const double *rpy, double *R)
{
    double sr = sin(-rpy[0] * D2R), cr = cos(-rpy[0] * D2R);
    double sp = sin(-rpy[1] * D2R), cp = cos(-rpy[1] * D2R);
    double sy = sin(-rpy[2] * D2R), cy = cos(-rpy[2] * D2R);

    R[0] = cy * cr - sy * sp * sr; R[1] = -sy * cp; R[2] = cy * sr + sy * sp * cr;
    R[3] = sy * cr + cy * sp * sr; R[4] = cy * cp;  R[5] = sy * sr - cy * sp * cr;
    R[6] = -cp * sr;               R[7] = sp;       R[8] = cp * cr;
}
// RGB -> YCrCb (ITU-R BT.601) ----------------------------------------------
static void YCrCb(const QRgb *pix, double *Y)
{
    //         R(0-255)     G(0-255)     B(0-255)
    Y[0] = ( 0.299 * qRed(*pix) + 0.587 * qGreen(*pix) + 0.114 * qBlue(*pix)) / 255;         // Y  (0-1)
    Y[1] = ( 0.500 * qRed(*pix) - 0.419 * qGreen(*pix) + 0.081 * qBlue(*pix)) / 255;         // Cr (-.5-.5)
    Y[2] = (-0.169 * qRed(*pix) - 0.331 * qGreen(*pix) + 0.500 * qBlue(*pix)) / 255;        // Cb (-.5-.5)
}
// update sky image ---------------------------------------------------------
void Plot::updateSky(void)
{
    QImage &bm1 = skyImageOriginal, &bm2 = skyImageResampled;
    QRgb pix;
    double x, y, xp, yp, radius, a, p[3], q[3], R[9] = { 0 }, dr, dist, Yz[3] = { 0 }, Y[3];
    int i, j, k, w1, h1, w2, h2, wz, nz = 0;


    w1 = bm1.width(); h1 = bm1.height();
    w2 = bm2.width(); h2 = bm2.height();

    if (w1 <= 0 || h1 <= 0 || w2 <= 0 || h2 <= 0) return;

    bm2.fill(QColor("silver")); // fill bitmap by silver

    if (norm(skyFOV, 3) > 1e-12)
        RPY(skyFOV, R);
    if (skyBinarize) {      // average of zenith image
        wz = h1 / 16;   // sky area size
        for (i = w1 / 2 - wz; i <= w1 / 2 + wz; i++)
            for (j = h1 / 2 - wz; j <= h1 / 2 + wz; j++) {
                pix = bm1.pixel(i, j);
                YCrCb(&pix, Y);
                for (k = 0; k < 3; k++) Yz[k] += Y[k];
                nz++;
            }
        if (nz > 0)
            for (k = 0; k < 3; k++) Yz[k] /= nz;
    }
    for (j = 0; j < h2; j++)
        for (i = 0; i < w2; i++) {
            xp = (w2 / 2.0 - i) / skyScaleR;
            yp = (j - h2 / 2.0) / skyScaleR;
            radius = SQRT(SQR(xp) + SQR(yp));
            if (skyElevationMask && radius > 1.0) continue;

            // rotate coordinates roll-pitch-yaw
            if (norm(skyFOV, 3) > 1e-12) {
                if (radius < 1e-12) {
                    p[0] = p[1] = 0.0;
                    p[2] = 1.0;
                } else {
                    a = sin(radius * PI / 2.0);
                    p[0] = a * xp / radius;
                    p[1] = a * yp / radius;
                    p[2] = cos(radius * PI / 2.0);
                }
                q[0] = R[0] * p[0] + R[3] * p[1] + R[6] * p[2];
                q[1] = R[1] * p[0] + R[4] * p[1] + R[7] * p[2];
                q[2] = R[2] * p[0] + R[5] * p[1] + R[8] * p[2];
                if (q[2] >= 1.0) {
                    xp = yp = radius = 0.0;
                } else {
                    radius = acos(q[2]) / (PI / 2.0);
                    a = sqrt(SQR(q[0]) + SQR(q[1]));
                    xp = radius * q[0] / a;
                    yp = radius * q[1] / a;
                }
            }
            // correct lense distortion
            if (skyDistortionCorrection) {
                if (radius <= 0.0 || radius >= 1.0) continue;
                k = static_cast<int>(radius * 9.0);
                dr = radius * 9.0 - k;
                dist = k > 8 ? skyDistortion[9] : (1.0 - dr) * skyDistortion[k] + dr * skyDistortion[k + 1];
                xp *= dist / radius;
                yp *= dist / radius;
            } else {
                xp *= skyScale;
                yp *= skyScale;
            }
            if (skyFlip) xp = -xp;
            x = skyCenter[0] + xp;
            y = skyCenter[1] + yp;
            if (x < 0.0 || x >= w1 - 1 || y < 0.0 || y >= h1 - 1) continue;
            if (!skyResample)
                ResPixelNN(bm1, x, y, b1, pix)
            else
                ResPixelBL(bm1, x, y, b1, pix)
            bm2.setPixel(i, j, pix);
            if (skyBinarize) {
                YCrCb(&pix, Y);
                for (k = 1; k < 3; k++) Y[k] -= Yz[k];

                // threshold by brightness and color-distance
                if (Y[0] > skyBinThres1 && norm(Y + 1, 2) < skyBinThres2)
                    bm2.setPixel(i, j, qRgb(255, 255, 255));        // sky
                else
                    bm2.setPixel(i, j, qRgb(96, 96, 96));           // others
            }
        }
    updatePlot();
}
// read sky tag data --------------------------------------------------------
void Plot::readSkyTag(const QString &file)
{
    QFile fp(file);
    QByteArray buff;

    trace(3, "readSkyTag\n");

    if (!fp.open(QIODevice::ReadOnly)) return;

    while (!fp.atEnd()) {
        buff = fp.readLine();
        if (buff.at(0) == '\0' || buff.at(0) == '%' || buff.at(0) == '#') continue;
        QList<QByteArray> tokens = buff.split('=');
        if (tokens.size() < 2) continue;

        if (tokens.at(0) == "centx") {
            skyCenter[0] = tokens.at(1).toDouble();
        } else if (tokens.at(0) == "centy") {
            skyCenter[1] = tokens.at(1).toDouble();
        } else if (tokens.at(0) == "scale") {
            skyScale = tokens.at(1).toDouble();
        } else if (tokens.at(0) == "roll") {
            skyFOV[0] = tokens.at(1).toDouble();
        } else if (tokens.at(0) == "pitch") {
            skyFOV[1] = tokens.at(1).toDouble();
        } else if (tokens.at(0) == "yaw") {
            skyFOV[2] = tokens.at(1).toDouble();
        } else if (tokens.at(0) == "destcorr") {
            skyDistortionCorrection = tokens.at(1).toInt();
        } else if (tokens.at(0) == "elmask") {
            skyElevationMask = tokens.at(1).toInt();
        } else if (tokens.at(0) == "resample") {
            skyResample = tokens.at(1).toInt();
        } else if (tokens.at(0) == "flip") {
            skyFlip = tokens.at(1).toInt();
        } else if (tokens.at(0) == "dest") {
            QList<QByteArray> t = tokens.at(1).split(' ');
            for (int i = 0; i < 9 && i < t.size(); i++)
                skyDistortion[i] = t.at(i).toDouble();
        } else if (tokens.at(0) == "binarize") {
            skyBinarize = tokens.at(1).toInt();
        } else if (tokens.at(0) == "binthr1") {
            skyBinThres1 = tokens.at(1).toInt();
        } else if (tokens.at(0) == "binthr2") {
            skyBinThres2 = tokens.at(1).toInt();
        }
    }
}
// read sky image data ------------------------------------------------------
void Plot::readSkyData(const QString &file)
{
    QImage image;
    int i, w, h, wr;

    trace(3, "readSkyData\n");

    showMessage(tr("Reading sky data... %1").arg(file));

    if (!image.load(file)) {
        showMessage(QString(tr("Sky image file read error: %1")).arg(file));
        showLegend(QStringList());
        return;
    }
    skyImageOriginal = image;
    w = MAX(skyImageOriginal.width(), skyImageOriginal.height());
    h = MIN(skyImageOriginal.width(), skyImageOriginal.height());
    wr = MIN(w, MAX_SKYIMG_R);
    skyImageResampled = QImage(wr, wr, QImage::Format_ARGB32);
    skyImageFile = file;
    skySize[0] = skyImageOriginal.width();
    skySize[1] = skyImageOriginal.height();
    skyCenter[0] = skySize[0] / 2.0;
    skyCenter[1] = skySize[1] / 2.0;
    skyFOV[0] = skyFOV[1] = skyFOV[2] = 0.0;
    skyScale = h / 2.0;
    skyScaleR = skyScale * wr / w;
    skyDistortionCorrection = skyResample = skyFlip = 0;
    skyElevationMask = 1;
    for (i = 0; i < 10; i++) skyDistortion[i] = 0.0;

    readSkyTag(file + ".tag");

    showMessage("");
    btnShowImage->setChecked(true);

    updateSky();
}
// read map tag data --------------------------------------------------------
void Plot::readMapTag(const QString &file)
{
    QFile fp(file + ".tag");
    QByteArray buff;

    trace(3, "readMapTag\n");

    if (!(fp.open(QIODevice::ReadOnly))) return;

    mapScaleX = mapScaleY = 1.0;
    mapScaleEqual = 0;
    mapLatitude = mapLongitude = 0.0;

    while (!fp.atEnd()) {
        buff = fp.readLine();
        if (buff.at(0) == '\0' || buff.at(0) == '%' || buff.at(0) == '#') continue;
        QList<QByteArray> tokens = buff.split('=');
        if (tokens.size() < 2) continue;

        if (tokens.at(0) == "scalex") mapScaleX = tokens.at(1).toDouble();
        else if (tokens.at(0) == "scaley") mapScaleY = tokens.at(1).toDouble();
        else if (tokens.at(0) == "scaleeq") mapScaleEqual = tokens.at(1).toInt();
        else if (tokens.at(0) == "lat") mapLatitude = tokens.at(1).toDouble();
        else if (tokens.at(0) == "lon") mapLongitude = tokens.at(1).toDouble();
    }
}
// read shapefile -----------------------------------------------------------
void Plot::readShapeFile(const QStringList &files)
{
    int i;
    double pos[3];

    readWaitStart();

    gis_free(&gis);

    for (i = 0; i < files.count() && i < MAXMAPLAYER; i++) {
        showMessage(tr("Reading shapefile... %1").arg(files.at(i)));
        gis_read(qPrintable(files.at(i)), &gis, i);

        QFileInfo fi(files.at(i));
        strncpy(gis.name[i], qPrintable(fi.baseName()), 255);
    }

    readWaitEnd();
    showMessage("");
    btnShowMap->setChecked(true);

    if (norm(originPosition, 3) <= 0.0) {
        pos[0] = (gis.bound[0] + gis.bound[1]) / 2.0;
        pos[1] = (gis.bound[2] + gis.bound[3]) / 2.0;
        pos[2] = 0.0;
        pos2ecef(pos, originPosition);
    }

    updateOrigin();
    updatePlot();
    updateEnable();
}
// read GPX file ------------------------------------------------------------
void Plot::readGpxFile(const QString &file)
{
    QFile fp(file);
    QString name;
    double pos[3] = { 0 };

    if (!fp.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    nWayPoint = 0;

    QXmlStreamReader inputStream(&fp);
    while (!inputStream.atEnd() && !inputStream.hasError() && nWayPoint < MAXWAYPNT) {
        inputStream.readNext();
        if (inputStream.isStartElement()) {
            QString tag = inputStream.name().toString();
            if (tag.toLower() == "wpt") {
                pos[0] = inputStream.attributes().value("lat").toFloat();
                pos[1] = inputStream.attributes().value("lon").toFloat();
            } else if ((tag.toLower() == "ele") && norm(pos, 2) > 0.0) {
                pos[2] = inputStream.text().toFloat();
            } else if ((tag.toLower() == "name") && norm(pos, 3) > 0.0) {
                inputStream.readNext();
                name = inputStream.text().toString();
            } else if ((tag.startsWith("ogr:", Qt::CaseInsensitive)) && norm(pos, 3) > 0.0 && name.isEmpty()) {
                inputStream.readNext();
                name = inputStream.text().toString();
            }
        } else if (inputStream.isEndElement()) {
            QString tag = inputStream.name().toString();
            if (tag.toLower() == "wpt") {
                pointPosition[nWayPoint][0] = pos[0];
                pointPosition[nWayPoint][1] = pos[1];
                pointPosition[nWayPoint][2] = pos[2];
                pointName[nWayPoint++] = name;
                pos[0] = pos[1] = pos[2] = 0.0;
                name.clear();
            }
        }
    }
}
// read pos file ------------------------------------------------------------
void Plot::readPositionFile(const QString &file)
{
    QFile fp(file);
    QString id, name;
    double pos[3] = {0};
    int n, p;
    bool ok;

    if (!fp.open(QIODevice::ReadOnly | QIODevice::Text))
            return;

    nWayPoint=0;

    while (!fp.atEnd() && nWayPoint<MAXWAYPNT) {
        QByteArray line = fp.readLine();

        // remove comments
        if ((p=line.indexOf("#"))!=-1) line = line.left(p);

        QList<QByteArray> tokens = line.split(' ');

        n = tokens.size();

        if (n < 4) continue;

        pos[0] = tokens.at(0).toDouble(&ok);
        if (!ok) continue;
        pos[1] = tokens.at(1).toDouble(&ok);
        if (!ok) continue;
        pos[2] = tokens.at(2).toDouble(&ok);
        if (!ok) continue;
        id = tokens.at(3);
        if (n>4)
            name = tokens.at(4);

        if (n>=4) {
            pointPosition[nWayPoint][0] = pos[0];
            pointPosition[nWayPoint][1] = pos[1];
            pointPosition[nWayPoint][2] = pos[2];
            pointName[nWayPoint++] = (n>4) ? name : id;
        }
    }
    fp.close();
}
// read waypoint ------------------------------------------------------------
void Plot::readWaypoint(const QString &file)
{
    int type = 0;

    if (file.endsWith(".gpx")) type = 1;

    readWaitStart();
    showMessage(tr("Reading waypoint... %1").arg(file));

    if (type) readGpxFile(file);
    else readPositionFile(file);

    readWaitEnd();
    showMessage("");

    btnShowMap->setChecked(true);

    updatePlot();
    updateEnable();
    pntDialog->setPoint();
}
// save GPX file ------------------------------------------------------------
void Plot::saveGpxFile(const QString &file)
{
    QFile fp(file);
    int i;

    if (!fp.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QXmlStreamWriter stream(&fp);

    stream.setAutoFormatting(true);
    stream.writeStartDocument("1.0");

    stream.writeStartElement("gpx");
    stream.writeAttribute("version", "1.1");
    stream.writeAttribute("creator", "RTKLIB");
    stream.writeAttribute("xmlns", XMLNS);

    for (i = 0; i < nWayPoint; i++) {
        stream.writeStartElement("wpt");
        stream.writeAttribute("lat", QString::number(pointPosition[i][0], 'f', 9));
        stream.writeAttribute("lon", QString::number(pointPosition[i][1], 'f', 9));
        if (pointPosition[i][2] != 0.0)
            stream.writeTextElement("ele", QString::number(pointPosition[i][2], 'f', 4));
        stream.writeTextElement("name", pointName[i]);
        stream.writeEndElement();
    }
    stream.writeEndElement();

    stream.writeEndDocument();
}
// save pos file ------------------------------------------------------------
void Plot::savePositionFile(const QString &file)
{
    QFile fp(file);
    int i;

    if (!fp.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream stream(&fp);

    stream << QString("# WAYPOINTS by RTKLIB %1\n").arg(VER_RTKLIB);

    for (i = 0; i < nWayPoint; i++) {
        QString str(pointName[i]);
        stream << QString("%1 %2 %3 %4\n").arg(pointPosition[i][0], 13, 'f', 9)
                      .arg(pointPosition[i][1], 14, 'f', 9)
                      .arg(pointPosition[i][2], 10, 'f', 4).arg(str);
    }
    fp.close();
}
// save waypoint ------------------------------------------------------------
void Plot::saveWaypoint(const QString &file)
{
    int type = 0;

    if (file.endsWith(".gpx")) type = 1;

    readWaitStart();
    showMessage(tr("Saving waypoint... %1").arg(file));

    if (type) saveGpxFile(file);
    else savePositionFile(file);

    readWaitEnd();
    showMessage("");
}
// read station position data -----------------------------------------------
void Plot::readStationPosition(const QString &file, const QString &sta,
              double *rr)
{
    QFile fp(file);
    QByteArray buff;
    QString code;
    double pos[3];
    int sinex = 0;

    if (!(fp.open(QIODevice::ReadOnly))) return;

    while (!fp.atEnd()) {
        buff = fp.readLine();
        if (buff.indexOf("%=SNX")) sinex = 1;
        if (buff.at(0) == '%' || buff.at(1) == '#') continue;
        if (sinex) {
            if ((buff.length() < 68) || (buff.mid(14, 4) != sta)) continue;
            if (buff.mid(7, 4) == "STAX") rr[0] = buff.mid(47, 21).toDouble();
            if (buff.mid(7, 4) == "STAY") rr[1] = buff.mid(47, 21).toDouble();
            if (buff.mid(7, 4) == "STAZ") {
                rr[2] = buff.mid(47, 21).toDouble(); break;
            }
            ;
        } else {
            QList<QByteArray> tokens = buff.split(' ');
            if (tokens.size() < 4) continue;
            for (int i = 0; i < 3; i++) pos[i] = tokens.at(i).toDouble();
            code = "";
            for (int i = 3; i < tokens.size(); i++) code += tokens.at(i) + ' ';
            code = code.trimmed();

            if (code != sta) continue;

            pos[0] *= D2R;
            pos[1] *= D2R;
            pos2ecef(pos, rr);
            break;
        }
    }
}
// save dop -----------------------------------------------------------------
void Plot::saveDop(const QString &file)
{
    QFile fp(file);
    gtime_t time;
    QString data;
    double azel[MAXOBS * 2], dop[4], tow;
    int i, j, ns, week;
    char tstr[64];
    QString tlabel;

    trace(3, "saveDop: file=%s\n", qPrintable(file));

    if (!(fp.open(QIODevice::WriteOnly))) return;

    tlabel = timeFormat <= 1 ? tr("TIME (GPST)") : (timeFormat <= 2 ? tr("TIME (UTC)") : tr("TIME (JST))"));

    data = QString(tr("%% %1 %2 %3 %4 %5 %6 (EL>=%7deg)\n"))
           .arg(tlabel, timeFormat == 0 ? 13 : 19).arg("NSAT", 6).arg("GDOP", 8).arg("PDOP", 8)
               .arg("HDOP", 8).arg("VDOP", 8).arg(elevationMask, 0, 'f', 0);
    fp.write(data.toLatin1());

    for (i = 0; i < nObservation; i++) {
        ns = 0;
        for (j = indexObservation[i]; j < observation.n && j < indexObservation[i + 1]; j++) {
            if (satelliteMask[observation.data[j].sat - 1]) continue;
            if (elevation[j] < elevationMask * D2R) continue;
            if (elevationMaskEnabled && elevation[j] < elevationMaskData[static_cast<int>(azimuth[j] * R2D + 0.5)]) continue;
            azel[ns * 2] = azimuth[j];
            azel[ns * 2 + 1] = elevation[j];
            ns++;
        }
        if (ns <= 0) continue;

        dops(ns, azel, elevationMask * D2R, dop);

        time = observation.data[indexObservation[i]].time;
        if (timeFormat == 0) {
            tow = time2gpst(time, &week);
            sprintf(tstr, "%4d %8.1f ", week, tow);
        } else if (timeFormat == 1) {
            time2str(time, tstr, 1);
        } else if (timeFormat == 2) {
            time2str(gpst2utc(time), tstr, 1);
        } else {
            time2str(timeadd(gpst2utc(time), 9 * 3600.0), tstr, 1);
        }
        data = QString("%1 %2 %3 %4 %5 %6\n").arg(tstr).arg(ns, 6).arg(dop[0], 8, 'f', 1).arg(dop[1], 8, 'f', 1)
               .arg(dop[2], 8, 'f', 1).arg(dop[3], 8, 'f', 1);
        fp.write(data.toLatin1());
    }
}
// save snr and mp -------------------------------------------------------------
void Plot::saveSnrMp(const QString &file)
{
    QFile fp(file);
    QString obsTypeText = cBObservationType2->currentText();
    gtime_t time;
    double tow;
    char sat[32], tstr[64], code[64];
    QString mp, data, tlabel;
    int i, j, k, week;

    strncpy(code, qPrintable(obsTypeText.mid(1)), 63);

    trace(3, "saveSnrMp: file=%s\n", qPrintable(file));

    if (!(fp.open(QIODevice::WriteOnly))) return;

    tlabel = timeFormat <= 1 ? tr("TIME (GPST)") : (timeFormat <= 2 ? tr("TIME (UTC)") : tr("TIME (JST)"));

    mp = obsTypeText + " MP(m)";
    data = QString("%% %1 %2 %3 %4 %5 %6\n").arg(tlabel, timeFormat == 0 ? 13 : 19).arg("SAT", 6)
           .arg("AZ(deg)", 8).arg("EL(deg)", 8).arg("SNR(dBHz)", 9).arg(mp, 10);
    fp.write(data.toLatin1());

    for (i = 0; i < MAXSAT; i++) {
        if (satelliteMask[i] || !satelliteSelection[i]) continue;
        satno2id(i + 1, sat);

        for (j = 0; j < observation.n; j++) {
            if (observation.data[j].sat != i + 1) continue;

            for (k = 0; k < NFREQ + NEXOBS; k++)
                if (strchr(code2obs(observation.data[j].code[k]), code[0])) break;
            if (k >= NFREQ + NEXOBS) continue;

            time = observation.data[j].time;

            if (timeFormat == 0) {
                tow = time2gpst(time, &week);
                sprintf(tstr, "%4d %9.1f ", week, tow);
            } else if (timeFormat == 1) {
                time2str(time, tstr, 1);
            } else if (timeFormat == 2) {
                time2str(gpst2utc(time), tstr, 1);
            } else {
                time2str(timeadd(gpst2utc(time), 9 * 3600.0), tstr, 1);
            }
            data = QString("%1 %2 %3 %4 %5 %6f\n").arg(tstr).arg(sat, 6).arg(azimuth[j] * R2D, 8, 'f', 1)
                       .arg(elevation[j] * R2D, 8, 'f', 1).arg(observation.data[j].SNR[k] * SNR_UNIT, 9, 'f', 2).arg(!multipath[k] ? 0.0 : multipath[k][j], 10, 'f', 4);
            fp.write(data.toLatin1());
        }
    }
}
// save elev mask --------------------------------------------------------------
void Plot::saveElevationMask(const QString &file)
{
    QFile fp(file);
    QString data;
    double el, el0 = 0.0;
    int az;

    trace(3, "saveElevationMask: file=%s\n", qPrintable(file));

    if (!(fp.open(QIODevice::WriteOnly))) return;

    fp.write("%% Elevation Mask\n");
    fp.write("%% AZ(deg) EL(deg)\n");

    for (az = 0; az <= 360; az++) {
        el = floor(elevationMaskData[az] * R2D / 0.1 + 0.5) * 0.1;
        if (qFuzzyCompare(el, el0)) continue;
        data = QString("%1 %2\n").arg(static_cast<double>(az), 9, 'f', 1).arg(el, 6, 'f', 1);
        fp.write(data.toLatin1());
        el0 = el;
    }
}
// connect to external sources ----------------------------------------------
void Plot::connectStream(void)
{
    char cmd[1024];
    QString path, name[2];
    int i, p,  mode = STR_MODE_R;

    trace(3, "connectStream\n");

    if (connectState) return;

    for (i = 0; i < 2; i++) {
        if (rtStream[i] == STR_NONE) continue;
        else if (rtStream[i] == STR_SERIAL) path = streamPaths[i][0];
        else if (rtStream[i] == STR_FILE) path = streamPaths[i][2];
        else if (rtStream[i] <= STR_NTRIPCLI) path = streamPaths[i][1];
        else continue;

        if (rtStream[i] == STR_FILE || !solutionData[i].cyclic || solutionData[i].nmax != rtBufferSize + 1) {
            clear();
            initsolbuf(solutionData + i, 1, rtBufferSize + 1);
        }
        if (rtStream[i] == STR_SERIAL) mode |= STR_MODE_W;

        if ((p = path.indexOf("::"))) path = path.left(p);
        if ((p = path.indexOf("/:")))  path = path.left(p);
        if ((p = path.indexOf("@"))) name[i] = path.mid(p+1); else name[i] = path;

        if (!stropen(stream + i, rtStream[i], mode, qPrintable(path))) {
            showMessage(tr("Connect error: %1").arg(name[0]));
            showLegend(QStringList());
            trace(1, "stream open error: ch=%d type=%d path=%s\n", i + 1, rtStream[i], qPrintable(path));
            continue;
        }
        strsettimeout(stream + i, rtTimeoutTime, rtReconnectTime);

        if (streamCommandEnabled[i][0]) {
            strncpy(cmd, qPrintable(streamCommands[i][0]), 1023);
            strwrite(stream + i, (uint8_t *)cmd, strlen(cmd));
        }
        connectState = 1;
    }
    if (!connectState) return;

    if (title != "") setWindowTitle(title);
    else setWindowTitle(tr("CONNECT %1 %2").arg(name[0], name[1]));

    btnConnect->setChecked(true);
    btnSolution1->setChecked(!name[0].isEmpty());
    btnSolution2->setChecked(!name[1].isEmpty());
    btnSolution12->setChecked(false);
    btnShowTrack->setChecked(true);
    btnFixHorizontal->setChecked(true);
    btnFixCenter->setChecked(true);

    btnReload->setVisible(false);
    wgStreamStatus->setVisible(true);

    updateTime();
    updatePlot();
    updateEnable();
}
// disconnect from external sources -----------------------------------------
void Plot::disconnectStream(void)
{
    char cmd[1024];
    int i;

    trace(3, "Disconnect\n");

    if (!connectState) return;

    connectState = 0;

    for (i = 0; i < 2; i++) {
        if (streamCommandEnabled[i][1]) {
            strncpy(cmd, qPrintable(streamCommands[i][1]), 1023);
            strwrite(stream + i, (uint8_t *)cmd, strlen(cmd));
        }
        strclose(stream + i);
    }

    if (windowTitle().indexOf(tr("CONNECT")))
        setWindowTitle(QString(tr("DISCONNECT %1")).arg(windowTitle().mid(7)));

    lblStreamStatus1->setStyleSheet(QStringLiteral("QLabel {color: gray;}"));
    lblStreamStatus2->setStyleSheet(QStringLiteral("QLabel {color: gray;}"));
    lblConnectMessage->setText("");

    btnReload->setVisible(true);
    wgStreamStatus->setVisible(false);

    updateTime();
    updatePlot();
    updateEnable();
}
// check observation data types ---------------------------------------------
int Plot::checkObservation(const QString &file)
{
    trace(3,"checkObservation\n");

    if (!file.indexOf('.')) return 0;
    int p = file.lastIndexOf('.');
    if (file.indexOf(".z") || file.indexOf(".gz") || file.indexOf(".zip") ||
        file.indexOf(".Z") || file.indexOf(".GZ") || file.indexOf(".ZIP"))
        return file.at(p - 1) == 'o' || file.at(p - 1) == 'O' || file.at(p - 1) == 'd' || file.at(p - 1) == 'D';
    return file.indexOf(".obs") || file.indexOf(".OBS") ||
           file.mid(p + 3) == "o" || file.mid(p + 3) == "O" ||
           file.mid(p + 3) == "d" || file.mid(p + 3) == "D";
}
// update observation data index, azimuth/elevation, satellite list ---------
void Plot::updateObservation(int nobs)
{
    prcopt_t opt = prcopt_default;
    double rr[3] = {0};
    int per, per_ = -1;

    trace(3, "updateObservation\n");

    delete [] indexObservation; indexObservation = NULL;
    delete [] azimuth; azimuth = NULL;
    delete [] elevation; elevation = NULL;
    nObservation = 0;
    if (nobs <= 0) return;

    indexObservation = new int[nobs + 1];
    azimuth = new double[observation.n];
    elevation = new double[observation.n];

    readWaitStart();
    showLegend(QStringList());

    for (int i = 0, j = 0; i < observation.n; i = j) {
        gtime_t time = observation.data[i].time;
        double pos[3], azel[2*MAXOBS];
        int svh;

        for (j = i; j < observation.n; j++)
            if (timediff(observation.data[j].time, time) > TTOL) break;
        indexObservation[nObservation++] = i;

        if (receiverPosition==0) { // single point position
            sol_t sol = {};
            char msg[128];

            opt.err[0] = 900.0;
            pntpos(observation.data + i, j - i, &navigation, &opt, &sol, azel, NULL, msg);
            matcpy(rr, sol.rr, 3, 1);
            ecef2pos(rr, pos);
        } else if (receiverPosition==1) { // lat/lon/height
            matcpy(pos, ooPosition, 3, 1);
            pos2ecef(pos, rr);
        } else { // RINEX header position
            matcpy(rr, station.pos, 3, 1);
            ecef2pos(rr, pos);
        }
        for (int k = 0; k < j - i; k++) {
            double e[3], rs[6], dts[2], var;
            int sat = observation.data[i + k].sat;
            if (simulatedObservation) {
                char name[16];
                satno2id(sat, name);
                if (!tle_pos(time, name, "", "", &tleData, NULL, rs)) continue;
            }
            else {
                if (!satpos(time, time, sat, EPHOPT_BRDC, &navigation, rs, dts, &var, &svh)) {
                    continue;
                }
            }
            if (geodist(rs, rr, e) > 0.0) {
                satazel(pos, e, azel);
                if (azel[0] < 0.0) azel[0] += 2.0*PI;
            }
            else {
                azel[0] = azel[1] = 0.0;
            }
            azimuth[i+k] = azel[0];
            elevation[i+k] = azel[1];
        }
        per = (i + 1) * 100 / observation.n;
        if (per != per_) {
            showMessage(tr("Updating azimuth/elevation... (%1%)").arg(per_ = per));
            qApp->processEvents();
        }
    }
    indexObservation[nObservation] = observation.n;

    updateSatelliteList();

    readWaitEnd();
}
// update Multipath ------------------------------------------------------------
void Plot::updateMp(void)
{
    obsd_t *data;
    double freq1,freq2,freq,I,B;
    int i, j, k, m, n, sat, per, per_ = -1;

    trace(3, "updateMp\n");

    for (i = 0; i < NFREQ + NEXOBS; i++) {
        delete [] multipath[i]; multipath[i] = NULL;
    }
    if (observation.n <= 0) return;

    for (i = 0; i < NFREQ + NEXOBS; i++) {
        multipath[i] = new double[observation.n];
        for (j = 0; j < observation.n; j++) multipath[i][j] = 0.0;
    }
    readWaitStart();
    showLegend(QStringList());

    for (i = 0; i < observation.n; i++) {
        data = observation.data + i;
        freq1 = sat2freq(data->sat, data->code[0], &navigation);
        freq2 = sat2freq(data->sat, data->code[1], &navigation);
        if (data->L[0] == 0.0 || data->L[1] == 0.0 || freq1 == 0.0 || freq2 == 0.0) continue;
        I = -CLIGHT * (data->L[0] / freq1-data->L[1] / freq2) / (1.0 - SQR(freq1 / freq2));

        for (j = 0; j < NFREQ + NEXOBS; j++) {
            freq = sat2freq(data->sat, data->code[j], &navigation);
            if (data->P[j] == 0.0 || data->L[j] == 0.0 || freq == 0.0) continue;
            multipath[j][i] = data->P[j] - CLIGHT * data->L[j] / freq - 2.0 * SQR(freq1 / freq) * I;

        }
    }
    for (sat = 1; sat <= MAXSAT; sat++) {
        for (j = 0; j < NFREQ + NEXOBS; j++) {
            for (i = n = m = 0, B = 0.0; i < observation.n; i++) {
                data = observation.data + i;
                if (data->sat != sat) continue;
                if ((data->LLI[j] & 1) || (data->LLI[0] & 1) || (data->LLI[1] & 1)||
                    fabs(multipath[j][i] - B) > 5.0) {
                    for (k = m; k < i; k++) {
                        if (observation.data[k].sat == sat && multipath[j][k] != 0.0) multipath[j][k] -= B;
                    }
                    n = 0; m = i; B = 0.0;
                }
                if (multipath[j][i] != 0.0) B += (multipath[j][i] - B) / ++n;
            }
            for (k = m; k < observation.n; k++) {
                if (observation.data[k].sat == sat && multipath[j][k] != 0.0) multipath[j][k] -= B;
            }
        }
        per = sat * 100 / MAXSAT;
        if (per != per_) {
            showMessage(tr("Updating multipath... (%1%)").arg(per));
            per_ = per;
            qApp->processEvents();
        }
    }
    readWaitEnd();
}
// set connect path ---------------------------------------------------------
void Plot::connectPath(const QString &path, int ch)
{
    trace(3, "ConnectPath: path=%s ch=%d\n", qPrintable(path), ch);

    rtStream[ch] = STR_NONE;

    if (!path.indexOf("://")) return;
    if (path.indexOf("serial") != -1) rtStream[ch] = STR_SERIAL;
    else if (path.indexOf("tcpsvr") != -1) rtStream[ch] = STR_TCPSVR;
    else if (path.indexOf("tcpcli") != -1) rtStream[ch] = STR_TCPCLI;
    else if (path.indexOf("ntrip") != -1) rtStream[ch] = STR_NTRIPCLI;
    else if (path.indexOf("file") != -1) rtStream[ch] = STR_FILE;
    else return;

    streamPaths[ch][1] = path.mid(path.indexOf("://") + 3);
    rtFormat[ch] = SOLF_LLH;
    rtTimeFormat = 0;
    rtDegFormat = 0;
    rtFieldSeperator = " ";
    rtTimeoutTime = 0;
    rtReconnectTime = 10000;

    btnShowTrack->setChecked(true);
    btnFixHorizontal->setChecked(true);
    btnFixVertical->setChecked(true);
    btnFixCenter->setChecked(true);
}
// clear obs data --------------------------------------------------------------
void Plot::clearObservation(void)
{
    sta_t sta0 = {};
    int i;

    freeobs(&observation);
    freenav(&navigation, 0xFF);

    delete [] indexObservation; indexObservation = NULL;
    delete [] azimuth; azimuth = NULL;
    delete [] elevation; elevation = NULL;

    for (i = 0; i < NFREQ + NEXOBS; i++) {
        delete [] multipath[i]; multipath[i] = NULL;
    }

    observationFiles.clear();
    navigationFiles.clear();
    nObservation = 0;
    station = sta0;
    observationIndex = 0;
    simulatedObservation = 0;
}
// clear solution --------------------------------------------------------------
void Plot::clearSolution(void)
{
    int i;

    for (i = 0; i < 2; i++) {
        freesolbuf(solutionData + i);
        free(solutionStat[i].data);

        solutionStat[i].n = 0;
        solutionStat[i].data = NULL;
    }
    solutionFiles[0].clear();
    solutionFiles[1].clear();
    solutionIndex[0] = solutionIndex[1] = 0;
}
// clear data ------------------------------------------------------------------
void Plot::clear(void)
{
    double ep[] = { 2010, 1, 1, 0, 0, 0 };
    int i;

    trace(3, "clear\n");

    week = 0;

    clearObservation();
    clearSolution();
    gis_free(&gis);

    mapImage = QImage();
    mapImageFile = "";
    mapSize[0] = mapSize[1] = 0;

    for (i = 0; i < 3; i++)
        timeEnabled[i] = 0;
    timeStart = timeEnd = epoch2time(ep);
    btnAnimate->setChecked(false);

    if (plotType > PLOT_NSAT)
        updateType(PLOT_TRK);
    if (!connectState) {
        initsolbuf(solutionData, 0, 0);
        initsolbuf(solutionData + 1, 0, 0);
        setWindowTitle(title != "" ? title : tr("%1 ver.%2 %3").arg(PRGNAME).arg(VER_RTKLIB, PATCH_LEVEL));
    } else {
        initsolbuf(solutionData, 1, rtBufferSize + 1);
        initsolbuf(solutionData + 1, 1, rtBufferSize + 1);
    }

    for (i = 0; i <= 360; i++) elevationMaskData[i] = 0.0;

    nWayPoint = 0;
    selectedWayPoint = -1;

    updateTime();
    updatePlot();
    updateEnable();
}
// reload data --------------------------------------------------------------
void Plot::reload(void)
{
    QStringList obsfiles, navfiles;

    trace(3, "reload\n");

    if (simulatedObservation) {
        generateVisibilityData();
        return;
    }
    obsfiles = observationFiles;
    navfiles = navigationFiles;

    readObservation(obsfiles);
    readNavigation(navfiles);
    readSolution(solutionFiles[0], 0);
    readSolution(solutionFiles[1], 1);
}
// read wait start ----------------------------------------------------------
void Plot::readWaitStart(void)
{
    menuFile->setEnabled(false);
    menuEdit->setEnabled(false);
    menuView->setEnabled(false);
    menuHelp->setEnabled(false);
    toolPanel->setEnabled(false);
    lblDisplay->setEnabled(false);
    setCursor(Qt::WaitCursor);
}
// read wait end ------------------------------------------------------------
void Plot::readWaitEnd(void)
{
    menuFile->setEnabled(true);
    menuEdit->setEnabled(true);
    menuView->setEnabled(true);
    menuHelp->setEnabled(true);
    toolPanel->setEnabled(true);
    lblDisplay->setEnabled(true);
    setCursor(Qt::ArrowCursor);
}
// --------------------------------------------------------------------------
