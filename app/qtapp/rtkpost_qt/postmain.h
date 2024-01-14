//---------------------------------------------------------------------------
#ifndef postmainH
#define postmainH
//---------------------------------------------------------------------------
#include <QString>
#include <QDialog>
#include <QThread>

#include "rtklib.h"

#include "ui_postmain.h"

class QShowEvent;
class QCloseEvent;
class QSettings;
class OptDialog;
class TextViewer;
class ConvDialog;


//Helper Class ------------------------------------------------------------------

class ProcessingThread : public QThread
{
    Q_OBJECT

public:
    prcopt_t prcopt;
    solopt_t solopt;
    filopt_t filopt;
    gtime_t ts, te;
    double ti, tu;
    int n, stat;
    char *infile[6], outfile[1024];
    QString rov, base;

    explicit ProcessingThread(QObject *parent);
    ~ProcessingThread();

    void addInput(const QString &);
    static QString toList(const QString & list);

protected:
    void run();

signals:
    void done(int);
};
//---------------------------------------------------------------------------

class MainForm : public QDialog, public Ui::MainForm
{
    Q_OBJECT

public slots:
    void callRtkPlot();
    void viewOutputFile();
    void convertToKML();
    void showOptionsDialog();
    void postProcess();
    void abortProcessing();
    void showAboutDialog();
	
    void showStartTimeDialog();
    void showStopTimeDialog();
    void selectInputFile1();
    void selectInputFile3();
    void selectInputFile2();
    void selectInputFile4();
    void selectInputFile5();
    void selectOutputFile();
    void viewInputFile1();
    void viewInputFile3();
    void viewInputFile2();
    void viewInputFile4();
    void viewInputFile5();
    void viewOutputFileStat();
    void viewOutputFileTrace();
    void plotInputFile1();
    void plotInputFile2();
    void showKeyDialog();
		
    void outputDirectoryEnableClicked();
    void selectOutputDirectory();
    void selectInputFile6();
    void viewInputFile6();

    void processingFinished(int);
    void showMessage(const QString  &msg);

protected:
    void showEvent(QShowEvent*);
    void closeEvent(QCloseEvent*);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:
    OptDialog  *optDialog;
    ConvDialog *convDialog;
    TextViewer *textViewer;

    ProcessingThread *processingThread;

    void execProcessing();
    int  getOption(prcopt_t &prcopt, solopt_t &solopt, filopt_t &filopt);
    int  obsToNav (const QString &obsfile, QString &navfile);
	
    QString filePath(const QString &file);
    void readList(QComboBox *, QSettings *ini,  const QString &key);
    void writeList(QSettings *ini, const QString &key, const QComboBox *combo);
    void addHistory(QComboBox *combo);
    int execCommand(const QString &cmd, const QStringList &opt, int show);
	
    gtime_t getTimeStart();
    gtime_t getTimeStop();
    void setOutputFile();
    void setTimeStart(gtime_t time);
    void setTimeStop(gtime_t time);
    void updateEnable();
    void loadOptions();
    void saveOptions();
	
public:
    QString iniFile;
    bool abortFlag;
	
    // options
    int positionMode, frequencies, solution, dynamicModel, ionosphereOption, troposphereOption, receiverBiasEstimation;
    int ARIter, minFixSats, minHoldSats, minDropSats, ARFilter;
    int numIter, codeSmooth, tideCorrection;
    int outputCntResetAmbiguity, fixCntHoldAmbiguity, LockCntFixAmbiguity, roverPositionType, referencePositionType;
    int satelliteEphemeris, navigationSystems;
    int roverAntennaPcv, referenceAntennaPcv, ambiguityResolutionGPS, ambiguityResolutionGLO, ambiguityResolutionBDS;
    int outputHeader, outputOptions, outputVelocity, outputSingle, outputDatum;
    int outputHeight, outputGeoid, debugTrace, debugStatus, baseLineConstrain;
    int solutionFormat, timeFormat, latLonFormat, intpolateReferenceObs, netRSCorr, satelliteClockCorrection;
    int sbasCorrection, sbasCorrection1, sbasCorrection2, sbasCorrection3, sbasCorrection4, timeDecimal;
    int solutionStatic, sbasSat, mapFunction;
    int positionOption[6];
    double elevationMask, maxAgeDiff, varHoldAmb, gainHoldAmb, rejectPhase, rejectCode;
    double measurementErrorR1, measurementErrorR2, measurementErrorR5, measurementError2, measurementError3, measurementError4, measurementError5;
    double measurementError6, measurementError7, measurementError8;
    double satelliteClockStability, roverAntennaE, roverAntennaN, roverAntennaU, referenceAntennaE, referenceAntennaN, referenceAntennaU;
    double processNoise1, processNoise2, processNoise3, processNoise4, processNoise5;
    double validThresAR, elevationMaskAR, elevationMaskHold, slipThreshold, dopplerThreshold;
    double maxPositionVarAR, glonassHwBias, thresAR3, thresAR4, validThresARMin, validThresARMax;
    double roverPosition[3], referencePosition[3], baseLine[2];
    double maxSolutionStd;
    snrmask_t snrMask;
	
    QString rnxOptions1, rnxOptions2, pppOptions;
    QString fieldSeperator, roverAntenna, referenceAntenna, antennaPcvFile, stationPositionFile, PrecEphFile;
    QString netRSCorrFile1, netRSCorrFile2, satelliteClockCorrectionFile;
    QString geoidDataFile, ionosphereFile, dcbFile, eopFile, blqFile;
    QString sbasCorrectionFile, satellitePcvFile, excludedSatellites;
    QString roverList, baseList;
	
    void viewFile(const QString &file);

    explicit MainForm(QWidget *parent = 0);
};

//---------------------------------------------------------------------------
#endif
