//---------------------------------------------------------------------------
#ifndef navioptH
#define navioptH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_naviopt.h"
#include "rtklib.h"

class TextViewer;
class FreqDialog;
//---------------------------------------------------------------------------
class OptDialog : public QDialog, private Ui::OptDialog
{
    Q_OBJECT
protected:
    void showEvent(QShowEvent*);

public slots:
    void saveClose();
    void selectAntennaPcvFile();
    void viewAntennaPcvFile();
    void loadOptions();
    void saveOptions();
    void selectReferencePosition();
    void selectRoverPosition();
    void viewStationPositionFile();
    void selectStationPositionFile();
    void referencePositionTypeChanged(int);
    void roverPositionTypeChanged(int);
    void getPosition(int type, QLineEdit **edit, double *pos);
    void setPosition(int type, QLineEdit **edit, double *pos);
    void selectPanelFont();
    void selectSolutionFont();
    void selectGeoidDataFile();
    void viewSatellitePcvFile();
    void selectSatellitePcvFile();
    void selectLocalDirectory();
    void selectIonosphereFile();
    void selectDCBFile();
    void viewDCBFile();
    void selectEOPFile();
    void viewEOPFile();
    void selectBLQFile();
    void viewBLQFile();
    void showSnrMaskDialog();

private:
    void getOptions(void);
    void setOptions(void);
    void load(const QString &file);
    void save(const QString &file);
    void readAntennaList(void);
    void updateEnable(void);
    void showFrequenciesDialog();
    void showKeyDialog();
    int options;

public:
    enum OptionsType {NaviOptions = 0, PostOptions = 1};
    prcopt_t processOptions;
    solopt_t solutionOption;
    QFont panelFont, positionFont;
    TextViewer *textViewer;
    FreqDialog * freqDialog;

    int serverCycle, serverBufferSize, solutionBufferSize, navSelect, savedSolution;
    int nmeaRequest, nmeaCycle, timeoutTime, reconnectTime, dgpsCorrection, sbasCorrection;
    int debugTrace, debugStatus;
    int roverPositionType, referencePositionType, roverAntennaPcv, referenceAntennaPcv, baselineC;
    int monitorPort, fileSwapMargin, panelStack;

    QString excludedSatellites, localDirectory;
    QString roverAntenna, referenceAntenna, satellitePcvFile, antennaPcvFile, stationPositionFile;
    QString geoidDataFile, dcbFile, eopFile, blqFile, ionosphereFile, tleFile, tleSatFile;
    QString proxyAddr;

    double roverAntennaDelta[3], referenceAntennaDelta[3], roverPosition[3], referencePosition[3];
    double baseline[2], nmeaInterval[2];

    QString roverList, baseList;
    QString rnxOptions1, rnxOptions2, pppOptions;
    int sbasSat, intpolateReferenceObs;

    explicit OptDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
