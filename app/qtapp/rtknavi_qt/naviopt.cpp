//---------------------------------------------------------------------------
#include <QFileDialog>
#include <QLineEdit>
#include <QPoint>
#include <QFontDialog>
#include <QFont>
#include <QShowEvent>
#include <QFileSystemModel>
#include <QCompleter>

#include "rtklib.h"
#include "naviopt.h"
#include "viewer.h"
#include "refdlg.h"
#include "navimain.h"
#include "maskoptdlg.h"
#include "freqdlg.h"
#include "keydlg.h"

//---------------------------------------------------------------------------
#define MAXSTR      1024                /* max length of a string */

// receiver options table ---------------------------------------------------
static int strtype[] = {                  /* stream types */
    STR_NONE, STR_NONE, STR_NONE, STR_NONE, STR_NONE, STR_NONE, STR_NONE, STR_NONE
};
static char strpath[8][MAXSTR] = { "" };        /* stream paths */
static int strfmt[] = {                         /* stream formats */
    STRFMT_RTCM3, STRFMT_RTCM3, STRFMT_SP3, SOLF_LLH, SOLF_NMEA, 0, 0, 0
};
static int svrcycle = 10;               /* server cycle (ms) */
static int timeout = 10000;             /* timeout time (ms) */
static int reconnect = 10000;           /* reconnect interval (ms) */
static int nmeacycle = 5000;            /* nmea request cycle (ms) */
static int fswapmargin = 30;            /* file swap marign (s) */
static int buffsize = 32768;            /* input buffer size (bytes) */
static int navmsgsel = 0;               /* navigation mesaage select */
static int nmeareq = 0;                 /* nmea request type (0:off,1:lat/lon,2:single) */
static double nmeapos[] = { 0, 0 };     /* nmea position (lat/lon) (deg) */
static char proxyaddr[MAXSTR] = "";     /* proxy address */

#define TIMOPT  "0:gpst,1:utc,2:jst,3:tow"
#define CONOPT  "0:dms,1:deg,2:xyz,3:enu,4:pyl"
#define FLGOPT  "0:off,1:std+2:age/ratio/ns"
#define ISTOPT  "0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,7:ntripcli,8:ftp,9:http"
#define OSTOPT  "0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,6:ntripsvr"
#define FMTOPT  "0:rtcm2,1:rtcm3,2:oem4,3:oem3,4:ubx,5:ss2,6:hemis,7:skytraq,8:gw10,9:javad,10:nvs,11:binex,12:rt17,13:sbf,14:cmr,17:sp3"
#define NMEOPT  "0:off,1:latlon,2:single"
#define SOLOPT  "0:llh,1:xyz,2:enu,3:nmea"
#define MSGOPT  "0:all,1:rover,2:base,3:corr"

static opt_t rcvopts[] = {
    { "inpstr1-type",     3, (void *)&strtype[0],  ISTOPT  },
    { "inpstr2-type",     3, (void *)&strtype[1],  ISTOPT  },
    { "inpstr3-type",     3, (void *)&strtype[2],  ISTOPT  },
    { "inpstr1-path",     2, (void *)strpath [0],  ""      },
    { "inpstr2-path",     2, (void *)strpath [1],  ""      },
    { "inpstr3-path",     2, (void *)strpath [2],  ""      },
    { "inpstr1-format",   3, (void *)&strfmt [0],  FMTOPT  },
    { "inpstr2-format",   3, (void *)&strfmt [1],  FMTOPT  },
    { "inpstr3-format",   3, (void *)&strfmt [2],  FMTOPT  },
    { "inpstr2-nmeareq",  3, (void *)&nmeareq,     NMEOPT  },
    { "inpstr2-nmealat",  1, (void *)&nmeapos[0],  "deg"   },
    { "inpstr2-nmealon",  1, (void *)&nmeapos[1],  "deg"   },
    { "outstr1-type",     3, (void *)&strtype[3],  OSTOPT  },
    { "outstr2-type",     3, (void *)&strtype[4],  OSTOPT  },
    { "outstr1-path",     2, (void *)strpath [3],  ""      },
    { "outstr2-path",     2, (void *)strpath [4],  ""      },
    { "outstr1-format",   3, (void *)&strfmt [3],  SOLOPT  },
    { "outstr2-format",   3, (void *)&strfmt [4],  SOLOPT  },
    { "logstr1-type",     3, (void *)&strtype[5],  OSTOPT  },
    { "logstr2-type",     3, (void *)&strtype[6],  OSTOPT  },
    { "logstr3-type",     3, (void *)&strtype[7],  OSTOPT  },
    { "logstr1-path",     2, (void *)strpath [5],  ""      },
    { "logstr2-path",     2, (void *)strpath [6],  ""      },
    { "logstr3-path",     2, (void *)strpath [7],  ""      },

    { "misc-svrcycle",    0, (void *)&svrcycle,    "ms"    },
    { "misc-timeout",     0, (void *)&timeout,     "ms"    },
    { "misc-reconnect",   0, (void *)&reconnect,   "ms"    },
    { "misc-nmeacycle",   0, (void *)&nmeacycle,   "ms"    },
    { "misc-buffsize",    0, (void *)&buffsize,    "bytes" },
    { "misc-navmsgsel",   3, (void *)&navmsgsel,   MSGOPT  },
    { "misc-proxyaddr",   2, (void *)proxyaddr,    ""      },
    { "misc-fswapmargin", 0, (void *)&fswapmargin, "s"     },

    { "",		      0, NULL,		       ""      }
};
//---------------------------------------------------------------------------
OptDialog::OptDialog(QWidget *parent)
    : QDialog(parent)
{
    QString label;
    int freq[]={1,2,5,6,7,8,9};
    int nglo = MAXPRNGLO, ngal = MAXPRNGAL, nqzs = MAXPRNQZS;
    int ncmp = MAXPRNCMP, nirn = MAXPRNIRN;

    setupUi(this);

    processOptions = prcopt_default;
    solutionOption = solopt_default;
    options = NaviOptions;

    textViewer = new TextViewer(this);
    freqDialog = new FreqDialog(this);
    
    cBFrequencies->clear();
    for (int i = 0; i < NFREQ; i++) {
        label = label + (i > 0 ? "+" : "") + QString("L%1").arg(freq[i]);
        cBFrequencies->addItem(label);
	}
    QCompleter *fileCompleter = new QCompleter(this);
    QFileSystemModel *fileModel = new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    lEStationPositionFile->setCompleter(fileCompleter);
    lEAntennaPcvFile->setCompleter(fileCompleter);
    lESatellitePcvFile->setCompleter(fileCompleter);
    lEDCBFile->setCompleter(fileCompleter);
    lEGeoidDataFile->setCompleter(fileCompleter);
    lEEOPFile->setCompleter(fileCompleter);
    lEBLQFile->setCompleter(fileCompleter);

    QCompleter *dirCompleter = new QCompleter(this);
    QFileSystemModel *dirModel = new QFileSystemModel(dirCompleter);
    dirModel->setRootPath("");
    dirModel->setFilter(QDir::AllDirs | QDir::Drives | QDir::NoDotAndDotDot);
    dirCompleter->setModel(dirModel);
    lELocalDirectory->setCompleter(dirCompleter);

    // station position file line edit actions
    QAction *acStationPositionFileSelect = lEStationPositionFile->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    acStationPositionFileSelect->setToolTip(tr("Select File"));
    QAction *acStationPositionFileView = lEStationPositionFile->addAction(QIcon(":/buttons/doc"), QLineEdit::TrailingPosition);
    acStationPositionFileView->setToolTip(tr("View File"));
    acStationPositionFileView->setEnabled(false);

    // satllite PCV line edit actions
    QAction *acSatellitePcvFileSelect = lESatellitePcvFile->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    acSatellitePcvFileSelect->setToolTip(tr("Select File"));
    QAction *acSatellitePcvFileView = lESatellitePcvFile->addAction(QIcon(":/buttons/doc"), QLineEdit::TrailingPosition);
    acSatellitePcvFileView->setToolTip(tr("View File"));
    acSatellitePcvFileView->setEnabled(false);

    // antenna PCV line edit actions
    QAction *acAntennaPcvFileSelect = lEAntennaPcvFile->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    acAntennaPcvFileSelect->setToolTip(tr("Select File"));
    QAction *acAntennaPcvFileView = lEAntennaPcvFile->addAction(QIcon(":/buttons/doc"), QLineEdit::TrailingPosition);
    acAntennaPcvFileView->setToolTip(tr("View File"));
    acAntennaPcvFileView->setEnabled(false);

    // DCB file line edit actions
    QAction *acDCBFileSelect = lEDCBFile->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    acDCBFileSelect->setToolTip(tr("Select File"));
    QAction *acDCBFileView = lEDCBFile->addAction(QIcon(":/buttons/doc"), QLineEdit::TrailingPosition);
    acDCBFileView->setToolTip(tr("View File"));
    acDCBFileView->setEnabled(false);

    // BLQ file line edit actions
    QAction *acBLQFileSelect = lEBLQFile->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    acBLQFileSelect->setToolTip(tr("Select File"));
    QAction *acBLQFileView = lEBLQFile->addAction(QIcon(":/buttons/doc"), QLineEdit::TrailingPosition);
    acBLQFileView->setToolTip(tr("View File"));
    acBLQFileView->setEnabled(false);

    // EOP file line edit actions
    QAction *acEOPFileSelect = lEEOPFile->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    acEOPFileSelect->setToolTip(tr("Select File"));
    QAction *acEOPFileView = lEEOPFile->addAction(QIcon(":/buttons/doc"), QLineEdit::TrailingPosition);
    acEOPFileView->setToolTip(tr("View File"));
    acEOPFileView->setEnabled(false);

    // geoid data file line edit actions
    QAction *acGeoidDataFileSelect = lEGeoidDataFile->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    acGeoidDataFileSelect->setToolTip(tr("Select File"));

    // BLQ file line edit actions
    QAction *acLocalDirectorySelect = lELocalDirectory->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    acLocalDirectorySelect->setToolTip(tr("Select Directory"));

    // Ionosphere file line edit actions
    QAction *acIonosphereFileSelect = lEIonosphereFile->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    acIonosphereFileSelect->setToolTip(tr("Select File"));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &OptDialog::saveClose);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &OptDialog::reject);
    connect(btnLoad, &QPushButton::clicked, this, &OptDialog::loadOptions);
    connect(btnSave, &QPushButton::clicked, this, &OptDialog::saveOptions);
    connect(acAntennaPcvFileSelect, &QAction::triggered, this, &OptDialog::selectAntennaPcvFile);
    connect(acAntennaPcvFileView, &QAction::triggered, this, &OptDialog::viewAntennaPcvFile);
    connect(lEAntennaPcvFile, &QLineEdit::textChanged, this, [acAntennaPcvFileView, this]()
            {acAntennaPcvFileView->setEnabled(QFile::exists(this->lEAntennaPcvFile->text()));});
    connect(acSatellitePcvFileSelect, &QAction::triggered, this, &OptDialog::selectSatellitePcvFile);
    connect(acSatellitePcvFileView, &QAction::triggered, this, &OptDialog::viewSatellitePcvFile);
    connect(lESatellitePcvFile, &QLineEdit::textChanged, this, [acSatellitePcvFileView, this]()
            {acSatellitePcvFileView->setEnabled(QFile::exists(this->lESatellitePcvFile->text()));});
    connect(acDCBFileSelect, &QAction::triggered, this, &OptDialog::selectDCBFile);
    connect(acDCBFileView, &QAction::triggered, this, &OptDialog::viewDCBFile);
    connect(lEDCBFile, &QLineEdit::textChanged, this, [acDCBFileView, this]()
            {acDCBFileView->setEnabled(QFile::exists(this->lEDCBFile->text()));});
    connect(acEOPFileSelect, &QAction::triggered, this, &OptDialog::selectEOPFile);
    connect(acEOPFileView, &QAction::triggered, this, &OptDialog::viewEOPFile);
    connect(lEEOPFile, &QLineEdit::textChanged, this, [acEOPFileView, this]()
            {acEOPFileView->setEnabled(QFile::exists(this->lEEOPFile->text()));});
    connect(acGeoidDataFileSelect, &QAction::triggered, this, &OptDialog::selectGeoidDataFile);
    connect(acLocalDirectorySelect, &QAction::triggered, this, &OptDialog::selectLocalDirectory);
    connect(acIonosphereFileSelect, &QAction::triggered, this, &OptDialog::selectIonosphereFile);
    connect(acBLQFileSelect, &QAction::triggered, this, &OptDialog::selectBLQFile);
    connect(acBLQFileView, &QAction::triggered, this, &OptDialog::viewBLQFile);
    connect(lEBLQFile, &QLineEdit::textChanged, this, [acBLQFileView, this]()
            {acBLQFileView->setEnabled(QFile::exists(this->lEBLQFile->text()));});
    connect(acStationPositionFileSelect, &QAction::triggered, this, &OptDialog::selectStationPositionFile);
    connect(acStationPositionFileView, &QAction::triggered, this, &OptDialog::viewStationPositionFile);
    connect(lEStationPositionFile, &QLineEdit::textChanged, this, [acStationPositionFileView, this]()
            {acStationPositionFileView->setEnabled(QFile::exists(this->lEStationPositionFile->text()));});
    connect(btnReferencePosition, &QPushButton::clicked, this, &OptDialog::selectReferencePosition);
    connect(btnRoverPosition, &QPushButton::clicked, this, &OptDialog::selectRoverPosition);
    connect(btnSnrMask, &QPushButton::clicked, this, &OptDialog::showSnrMaskDialog);
    connect(btnFontPanel, &QPushButton::clicked, this, &OptDialog::selectPanelFont);
    connect(btnFontSolution, &QPushButton::clicked, this, &OptDialog::selectSolutionFont);
    connect(cBPositionMode, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OptDialog::updateEnable);
    connect(cBSolutionFormat, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OptDialog::updateEnable);
    connect(cBReferencePositionType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OptDialog::referencePositionTypeChanged);
    connect(cBRoverPositionType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OptDialog::roverPositionTypeChanged);
    connect(cBAmbiguityResolutionGPS, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OptDialog::updateEnable);
    connect(cBRoverAntennaPcv, &QCheckBox::clicked, this, &OptDialog::updateEnable);
    connect(cBReferenceAntennaPcv, &QCheckBox::clicked, this, &OptDialog::updateEnable);
    connect(cBOutputHeight, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OptDialog::updateEnable);
    connect(cBNavSys1, &QCheckBox::clicked, this, &OptDialog::updateEnable);
    connect(cBNavSys2, &QCheckBox::clicked, this, &OptDialog::updateEnable);
    connect(cBNavSys3, &QCheckBox::clicked, this, &OptDialog::updateEnable);
    connect(cBNavSys4, &QCheckBox::clicked, this, &OptDialog::updateEnable);
    connect(cBNavSys5, &QCheckBox::clicked, this, &OptDialog::updateEnable);
    connect(cBNavSys6, &QCheckBox::clicked, this, &OptDialog::updateEnable);
    connect(cBBaselineConstrain, &QCheckBox::clicked, this, &OptDialog::updateEnable);
    connect(btnFrequencies, &QPushButton::clicked, this, &OptDialog::showFrequenciesDialog);
    connect(cBReferenceAntenna, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OptDialog::updateEnable);
    connect(cBRoverAntenna, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OptDialog::updateEnable);
    connect(cBIonosphereOption, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OptDialog::updateEnable);
    connect(cBTroposphereOption, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OptDialog::updateEnable);
    connect(cBDynamicModel, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OptDialog::updateEnable);
    connect(cBSatelliteEphemeris, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OptDialog::updateEnable);
    connect(btnHelp, &QPushButton::clicked, this, &OptDialog::showKeyDialog);
    connect(cBFrequencies, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OptDialog::updateEnable);

    if (nglo <= 0) cBNavSys2->setEnabled(false);
    if (ngal <= 0) cBNavSys3->setEnabled(false);
    if (nqzs <= 0) cBNavSys4->setEnabled(false);
    if (ncmp <= 0) cBNavSys6->setEnabled(false);
    if (nirn <= 0) cBNavSys7->setEnabled(false);

    updateEnable();

}
//---------------------------------------------------------------------------
void OptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    getOptions();

    tWOptions->setCurrentIndex(0);
}
//---------------------------------------------------------------------------
void OptDialog::saveClose()
{
    setOptions();

    accept();
}
//---------------------------------------------------------------------------
void OptDialog::loadOptions()
{
    QString fileName;

    fileName = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Load Options..."), QString(), tr("Options File (*.conf);;All (*.*)")));

    load(fileName);
}
//---------------------------------------------------------------------------
void OptDialog::saveOptions()
{
    QString file;

    file = QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Save Options..."), QString(), tr("Options File (*.conf);;All (*.*)")));

    QFileInfo f(file);
    if (f.suffix() == "") file = file + ".conf";
    save(file);
}
//---------------------------------------------------------------------------
void OptDialog::viewStationPositionFile()
{
    if (lEStationPositionFile->text() == "") return;

    TextViewer *viewer = new TextViewer(this);
    viewer->show();

    viewer->read(lEStationPositionFile->text());
}
//---------------------------------------------------------------------------
void OptDialog::selectStationPositionFile()
{
    QString fileName;

    fileName = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Station Position File"), lEStationPositionFile->text(), tr("Position File (*.pos);;All (*.*)")));

    lEStationPositionFile->setText(fileName);
}
//---------------------------------------------------------------------------
void OptDialog::showSnrMaskDialog()
{
    MaskOptDialog maskOptDialog(this);
    
    maskOptDialog.setSnrMask(processOptions.snrmask);

    maskOptDialog.exec();
    if (maskOptDialog.result() != QDialog::Accepted) return;
    processOptions.snrmask = maskOptDialog.getSnrMask();
}
//---------------------------------------------------------------------------
void OptDialog::roverPositionTypeChanged(int)
{
    QLineEdit *edit[] = { lERoverPosition1, lERoverPosition2, lERoverPosition3 };
	double pos[3];

    getPosition(roverPositionType, edit, pos);
    setPosition(cBRoverPositionType->currentIndex(), edit, pos);
    roverPositionType = cBRoverPositionType->currentIndex();

	updateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::referencePositionTypeChanged(int)
{
    QLineEdit *edit[] = { lEReferencePosition1, lEReferencePosition2, lEReferencePosition3 };
	double pos[3];

    getPosition(referencePositionType, edit, pos);
    setPosition(cBReferencePositionType->currentIndex(), edit, pos);
    referencePositionType = cBReferencePositionType->currentIndex();

	updateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::selectRoverPosition()
{
    RefDialog refDialog(this);
    QLineEdit *edit[] = { lERoverPosition1, lERoverPosition2, lERoverPosition3 };
    double p[3], posi[3];

    getPosition(cBRoverPositionType->currentIndex(), edit, p);
    ecef2pos(p, posi);
    
    refDialog.setRoverPosition(posi[0] * R2D, posi[1] * R2D, posi[2]);
    refDialog.stationPositionFile = lEStationPositionFile->text();
    refDialog.move(pos().x() + size().width() / 2 - refDialog.size().width() / 2,
               pos().y() + size().height() / 2 - refDialog.size().height() / 2);

    refDialog.exec();
    if (refDialog.result() != QDialog::Accepted) return;

    posi[0] = refDialog.getPosition()[0] * D2R;
    posi[1] = refDialog.getPosition()[1] * D2R;
    posi[2] = refDialog.getPosition()[2];

    pos2ecef(posi, p);
    setPosition(cBRoverPositionType->currentIndex(), edit, p);
}
//---------------------------------------------------------------------------
void OptDialog::selectReferencePosition()
{
    RefDialog refDialog(this);
    QLineEdit *edit[] = { lEReferencePosition1, lEReferencePosition2, lEReferencePosition3 };
    double p[3], posi[3];

    getPosition(cBReferencePositionType->currentIndex(), edit, p);
    ecef2pos(p, posi);
    refDialog.setRoverPosition(posi[0] * R2D, posi[1] * R2D, posi[2]);
    refDialog.stationPositionFile = lEStationPositionFile->text();
    refDialog.move(pos().x() + size().width() / 2 - refDialog.size().width() / 2,
               pos().y() + size().height() / 2 - refDialog.size().height() / 2);

    refDialog.exec();
    if (refDialog.result() != QDialog::Accepted) return;

    posi[0] = refDialog.getPosition()[0] * D2R;
    posi[1] = refDialog.getPosition()[1] * D2R;
    posi[2] = refDialog.getPosition()[2];

    pos2ecef(posi, p);
    setPosition(cBReferencePositionType->currentIndex(), edit, p);
}
//---------------------------------------------------------------------------
void OptDialog::viewSatellitePcvFile()
{
    if (lESatellitePcvFile->text() == "") return;

    textViewer->show();

    textViewer->read(lESatellitePcvFile->text());
}
//---------------------------------------------------------------------------
void OptDialog::selectSatellitePcvFile()
{
    lESatellitePcvFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Satellite Antenna PCV File"), lESatellitePcvFile->text(), tr("PCV File (*.pcv *.atx);Position File (*.pcv *.snx);All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::viewAntennaPcvFile()
{
    if (lEAntennaPcvFile->text() == "") return;

    textViewer->show();

    textViewer->read(lEAntennaPcvFile->text());
}
//---------------------------------------------------------------------------
void OptDialog::selectAntennaPcvFile()
{
    lEAntennaPcvFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Receiver Antenna PCV File"), lEAntennaPcvFile->text(), tr("PCV File (*.pcv *.atx);Position File (*.pcv *.snx);All (*.*)"))));
    readAntennaList();
}
//---------------------------------------------------------------------------
void OptDialog::selectGeoidDataFile()
{
    QString fileName = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Geoid Data File"), lEGeoidDataFile->text(), tr("All (*.*)")));

    lEGeoidDataFile->setText(fileName);
}
//---------------------------------------------------------------------------
void OptDialog::selectDCBFile()
{
    lEDCBFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("DCB Data File"), lEDCBFile->text(), tr("DCB Data File (*.dcb *.DCB);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::viewDCBFile()
{
    if (lEDCBFile->text() == "") return;

    textViewer->show();

    textViewer->read(lEDCBFile->text());
}
//---------------------------------------------------------------------------
void OptDialog::selectEOPFile()
{
    lEEOPFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("EOP Date File"), lEEOPFile->text(), tr("EOP Data File (*.eop *.erp);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::viewEOPFile()
{
    if (lEEOPFile->text() == "") return;

    textViewer->show();

    textViewer->read(lEEOPFile->text());
}
//---------------------------------------------------------------------------
void OptDialog::selectBLQFile()
{
    lEBLQFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Ocean Tide Loading BLQ File"), lEBLQFile->text(), tr("OTL BLQ File (*.blq);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::viewBLQFile()
{
    QString BLQFile_Text = lEBLQFile->text();

    if (BLQFile_Text == "") return;

    TextViewer *viewer = new TextViewer(this);
    viewer->show();
    viewer->read(BLQFile_Text);
}
//---------------------------------------------------------------------------
void OptDialog::selectLocalDirectory()
{
    QString dir = lELocalDirectory->text();

    dir = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("FTP/HTTP Local Directory"), dir));
    lELocalDirectory->setText(dir);
}
//---------------------------------------------------------------------------
void OptDialog::selectIonosphereFile()
{
    lEIonosphereFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Ionosphere Data File"), lEIonosphereFile->text(), tr("Ionosphere Data File (*.*i,*stec);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::selectPanelFont()
{
    QFontDialog dialog(this);

    dialog.setCurrentFont(fontLabelPanel->font());
    dialog.exec();

    fontLabelPanel->setFont(dialog.selectedFont());
    fontLabelPanel->setText(fontLabelPanel->font().family() + QString::number(fontLabelPanel->font().pointSize()) + " pt");
}
//---------------------------------------------------------------------------
void OptDialog::selectSolutionFont()
{
    QFontDialog dialog(this);

    dialog.setCurrentFont(fontLabelSolution->font());
    dialog.exec();

    fontLabelSolution->setFont(dialog.selectedFont());
    fontLabelSolution->setText(fontLabelSolution->font().family() + QString::number(fontLabelSolution->font().pointSize()) + " pt");
}
//---------------------------------------------------------------------------
void OptDialog::getOptions(void)
{
    QLineEdit *editu[] = { lERoverPosition1, lERoverPosition2, lERoverPosition3 };
    QLineEdit *editr[] = { lEReferencePosition1, lEReferencePosition2, lEReferencePosition3 };

    cBPositionMode->setCurrentIndex(processOptions.mode);
    cBFrequencies->setCurrentIndex(processOptions.nf - 1 > NFREQ - 1 ? NFREQ - 1 : processOptions.nf - 1);
    cBElevationMask->setCurrentIndex(cBElevationMask->findText(QString::number(processOptions.elmin * R2D)));
    cBDynamicModel->setCurrentIndex(processOptions.dynamics);
    cBTideCorrection->setCurrentIndex(processOptions.tidecorr);
    cBIonosphereOption->setCurrentIndex(processOptions.ionoopt);
    cBTroposphereOption->setCurrentIndex(processOptions.tropopt);
    cBSatelliteEphemeris->setCurrentIndex(processOptions.sateph);
    cBAmbiguityResolutionGPS->setCurrentIndex(processOptions.modear);
    cBAmbiguityResolutionGLO->setCurrentIndex(processOptions.glomodear);
    cBAmbiguityResolutionBDS->setCurrentIndex(processOptions.bdsmodear);
    sBValidThresAR->setValue(processOptions.thresar[0]);
    sBMaxPositionVarAR->setValue(processOptions.thresar[1]);
    sBGlonassHwBias->setValue(processOptions.thresar[2]);
    sBValidThresARMin->setValue(processOptions.thresar[5]);
    sBValidThresARMax->setValue(processOptions.thresar[6]);
    sBOutageCountResetAmbiguity->setValue(processOptions.maxout);
    sBLockCountFixAmbiguity->setValue(processOptions.minlock);
    sBFixCountHoldAmbiguity->setValue(processOptions.minfix);
    sBElevationMaskAR->setValue(processOptions.elmaskar * R2D);
    sBElevationMaskHold->setValue(processOptions.elmaskhold * R2D);
    sBMaxAgeDifferences->setValue(processOptions.maxtdiff);
    sBRejectCode->setValue(processOptions.maxinno[0]);
    sBRejectPhase->setValue(processOptions.maxinno[1]);
    sBVarHoldAmb->setValue(processOptions.varholdamb);
    sBGainHoldAmb->setValue(processOptions.gainholdamb);
    sBSlipThreshold->setValue(processOptions.thresslip);
    sBDopplerThreshold->setValue(processOptions.thresdop);
    //sBARIter->setValue(prcopt.armaxiter);
    sBNumIteration->setValue(processOptions.niter);
    sBMinFixSats->setValue(processOptions.minfixsats);
    sBMinHoldSats->setValue(processOptions.minholdsats);
    sBMinDropSats->setValue(processOptions.mindropsats);
    if (options == NaviOptions) {
        cBSyncSolution->setCurrentIndex(processOptions.syncsol);
    }
    cBARFilter->setCurrentIndex(processOptions.arfilter);
    lEExcludedSatellites->setText(excludedSatellites);

    cBNavSys1->setChecked(processOptions.navsys & SYS_GPS);
    cBNavSys2->setChecked(processOptions.navsys & SYS_GLO);
    cBNavSys3->setChecked(processOptions.navsys & SYS_GAL);
    cBNavSys4->setChecked(processOptions.navsys & SYS_QZS);
    cBNavSys5->setChecked(processOptions.navsys & SYS_SBS);
    cBNavSys6->setChecked(processOptions.navsys & SYS_CMP);
    cBNavSys7->setChecked(processOptions.navsys & SYS_IRN);
    cBPositionOption1->setChecked(processOptions.posopt[0]);
    cBPositionOption2->setChecked(processOptions.posopt[1]);
    cBPositionOption3->setChecked(processOptions.posopt[2]);
    cBPositionOption4->setChecked(processOptions.posopt[3]);
    cBPositionOption5->setChecked(processOptions.posopt[4]);
    cBPositionOption6->setChecked(processOptions.posopt[5]);

    cBSolutionFormat->setCurrentIndex(solutionOption.posf);
    cBTimeFormat->setCurrentIndex(solutionOption.timef == 0 ? 0 : solutionOption.times + 1);
    sBTimeDecimal->setValue(solutionOption.timeu);
    cBLatLonFormat->setCurrentIndex(solutionOption.degf);
    lEFieldSeperator->setText(solutionOption.sep);
    cBOutputHeader->setCurrentIndex(solutionOption.outhead);
    cBOutputOptions->setCurrentIndex(solutionOption.outopt);
    cBOutputVelocity->setCurrentIndex(solutionOption.outvel);
    cBOutputSingle->setCurrentIndex(processOptions.outsingle);
    sBMaxSolutionStd->setValue(solutionOption.maxsolstd);
    cBOutputDatum->setCurrentIndex(solutionOption.datum);
    cBOutputHeight->setCurrentIndex(solutionOption.height);
    cBOutputGeoid->setCurrentIndex(solutionOption.geoid);
    if (options == NaviOptions) {
        sBNmeaInterval1->setValue(solutionOption.nmeaintv[0]);
        sBNmeaInterval2->setValue(solutionOption.nmeaintv[1]);
    }
    cBSolutionStatic->setCurrentIndex(solutionOption.solstatic);
    cBDebugStatus->setCurrentIndex(debugStatus);
    cBDebugTrace->setCurrentIndex(debugTrace);

    cBBaselineConstrain->setChecked(baselineC);
    sBBaselineLen->setValue(baseline[0]);
    sBBaselineSig->setValue(baseline[1]);

    sBMeasurementErrorR1->setValue(processOptions.eratio[0]);
    sBMeasurementErrorR2->setValue(processOptions.eratio[1]);
    sBMeasurementErrorR5->setValue(processOptions.eratio[2]);
    sBMeasurementError2->setValue(processOptions.err[1]);
    sBMeasurementError3->setValue(processOptions.err[2]);
    sBMeasurementError4->setValue(processOptions.err[3]);
    sBMeasurementError5->setValue(processOptions.err[4]);
    sBProcessNoise1->setValue(processOptions.prn[0]);
    sBProcessNoise2->setValue(processOptions.prn[1]);
    sBProcessNoise3->setValue(processOptions.prn[2]);
    sBProcessNoise4->setValue(processOptions.prn[3]);
    sBProcessNoise5->setValue(processOptions.prn[4]);
    sBSatelliteClockStability->setValue(processOptions.sclkstab);
    sBMaxAveEp->setValue(processOptions.maxaveep);
    cBInitRestart->setChecked(processOptions.initrst);

    cBRoverPositionType->setCurrentIndex(roverPositionType);
    cBReferencePositionType->setCurrentIndex(referencePositionType);
    cBRoverAntennaPcv->setChecked(roverAntennaPcv);
    cBReferenceAntennaPcv->setChecked(referenceAntennaPcv);
    sBRoverAntennaE->setValue(roverAntennaDelta[0]);
    sBRoverAntennaN->setValue(roverAntennaDelta[1]);
    sBRoverAntennaU->setValue(roverAntennaDelta[2]);
    sBReferenceAntennaE->setValue(referenceAntennaDelta[0]);
    sBReferenceAntennaN->setValue(referenceAntennaDelta[1]);
    sBReferenceAntennaU->setValue(referenceAntennaDelta[2]);
    setPosition(cBRoverPositionType->currentIndex(), editu, roverPosition);
    setPosition(cBReferencePositionType->currentIndex(), editr, referencePosition);

    lESatellitePcvFile->setText(satellitePcvFile);
    lEAntennaPcvFile->setText(antennaPcvFile);
    lEStationPositionFile->setText(stationPositionFile);
    lEGeoidDataFile->setText(geoidDataFile);
    lEDCBFile->setText(dcbFile);
    lEEOPFile->setText(eopFile);
    lEBLQFile->setText(blqFile);
    readAntennaList();

    cBRoverAntenna->setCurrentIndex(cBRoverAntenna->findText(roverAntenna));
    cBReferenceAntenna->setCurrentIndex(cBReferenceAntenna->findText(referenceAntenna));

    if (options == NaviOptions) {
        lELocalDirectory->setText(localDirectory);

        sBServerCycle->setValue(serverCycle);
        sBTimeoutTime->setValue(timeoutTime);
        sBReconnectTime->setValue(reconnectTime);
        sBNmeaCycle->setValue(nmeaCycle);
        sBFileSwapMargin->setValue(fileSwapMargin);
        sBServerBufferSize->setValue(serverBufferSize);
        sBSolutionBufferSize->setValue(solutionBufferSize);
        sBSavedSolution->setValue(savedSolution);
        cBNavSelect->setCurrentIndex(navSelect);
        sBSbasSatellite->setValue(processOptions.sbassatsel);
        lEProxyAddress->setText(proxyAddr);
        sBMonitorPort->setValue(monitorPort);
        sBSolutionBufferSize->setValue(solutionBufferSize);
        cBPanelStack->setCurrentIndex(panelStack);

        fontLabelPanel->setFont(panelFont);
        fontLabelPanel->setText(fontLabelPanel->font().family() + QString::number(fontLabelPanel->font().pointSize()) + "pt");
        fontLabelSolution->setFont(positionFont);
        fontLabelSolution->setText(fontLabelSolution->font().family() + QString::number(fontLabelSolution->font().pointSize()) + "pt");
    } else if (options == PostOptions) {
        lERnxOptions1->setText(rnxOptions1);
        lERnxOptions2->setText(rnxOptions2);
        lEPPPOptions->setText(pppOptions);

        cBIntputReferenceObservation->setCurrentIndex(intpolateReferenceObs);
        lESbasSat->setText(QString::number(sbasSat));
        lEIonosphereFile->setText(ionosphereFile);


        tERoverList->setPlainText(roverList);
        tEBaseList->setPlainText(baseList);
    }

	updateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::setOptions(void)
{
    QString FieldSep_Text = lEFieldSeperator->text();
    QLineEdit *editu[] = { lERoverPosition1, lERoverPosition2, lERoverPosition3 };
    QLineEdit *editr[] = { lEReferencePosition1, lEReferencePosition2, lEReferencePosition3 };

    processOptions.mode = cBPositionMode->currentIndex();
    processOptions.nf = cBFrequencies->currentIndex() + 1;
    processOptions.soltype = cBSolution->currentIndex();
    processOptions.elmin = cBElevationMask->currentText().toDouble() * D2R;
    processOptions.dynamics = cBDynamicModel->currentIndex();
    processOptions.tidecorr = cBTideCorrection->currentIndex();
    processOptions.ionoopt = cBIonosphereOption->currentIndex();
    processOptions.tropopt = cBTroposphereOption->currentIndex();
    processOptions.sateph = cBSatelliteEphemeris->currentIndex();
    excludedSatellites = lEExcludedSatellites->text();
    processOptions.modear = cBAmbiguityResolutionGPS->currentIndex();
    processOptions.glomodear = cBAmbiguityResolutionGLO->currentIndex();
    processOptions.bdsmodear = cBAmbiguityResolutionBDS->currentIndex();
    processOptions.thresar[0] = sBValidThresAR->value();
    processOptions.thresar[1] = sBMaxPositionVarAR->value();
    processOptions.thresar[2] = sBGlonassHwBias->value();
    processOptions.thresar[5] = sBValidThresARMin->value();
    processOptions.thresar[6] = sBValidThresARMax->value();
    processOptions.maxout = sBOutageCountResetAmbiguity->value();
    processOptions.minlock = sBLockCountFixAmbiguity->value();
    processOptions.minfix = sBFixCountHoldAmbiguity->value();
    processOptions.elmaskar = sBElevationMaskAR->value() * D2R;
    processOptions.elmaskhold = sBElevationMaskHold->value() * D2R;
    processOptions.maxtdiff = sBMaxAgeDifferences->value();
    processOptions.maxinno[0] = sBRejectPhase->value();
    processOptions.maxinno[1] = sBRejectCode->value();
    processOptions.varholdamb = sBVarHoldAmb->value();
    processOptions.gainholdamb = sBGainHoldAmb->value();
    processOptions.thresslip = sBSlipThreshold->value();
    processOptions.thresdop = sBDopplerThreshold->value();
    //processOptions.arfilter
    processOptions.niter = sBNumIteration->value();
    processOptions.minfixsats = sBMinFixSats->value();
    processOptions.minholdsats = sBMinHoldSats->value();
    processOptions.mindropsats = sBMinDropSats->value();
    if (options == NaviOptions) {
        processOptions.syncsol = cBSyncSolution->currentIndex();
    }
    processOptions.arfilter = cBARFilter->currentIndex();
    baselineC = cBBaselineConstrain->isChecked();
    baseline[0] = sBBaselineLen->value();
    baseline[1] = sBBaselineSig->value();

    processOptions.navsys = 0;
    if (cBNavSys1->isChecked()) processOptions.navsys |= SYS_GPS;
    if (cBNavSys2->isChecked()) processOptions.navsys |= SYS_GLO;
    if (cBNavSys3->isChecked()) processOptions.navsys |= SYS_GAL;
    if (cBNavSys4->isChecked()) processOptions.navsys |= SYS_QZS;
    if (cBNavSys5->isChecked()) processOptions.navsys |= SYS_SBS;
    if (cBNavSys6->isChecked()) processOptions.navsys |= SYS_CMP;
    if (cBNavSys7->isChecked()) processOptions.navsys |= SYS_IRN;
    processOptions.posopt[0] = cBPositionOption1->isChecked();
    processOptions.posopt[1] = cBPositionOption2->isChecked();
    processOptions.posopt[2] = cBPositionOption3->isChecked();
    processOptions.posopt[3] = cBPositionOption4->isChecked();
    processOptions.posopt[4] = cBPositionOption5->isChecked();
    processOptions.posopt[5] = cBPositionOption6->isChecked();

    solutionOption.posf = cBSolutionFormat->currentIndex();
    solutionOption.timef = cBTimeFormat->currentIndex() == 0 ? 0 : 1;
    solutionOption.times = cBTimeFormat->currentIndex() == 0 ? 0 : cBTimeFormat->currentIndex() - 1;
    solutionOption.timeu = static_cast<int>(sBTimeDecimal->value());
    solutionOption.degf = cBLatLonFormat->currentIndex();
    strncpy(solutionOption.sep, qPrintable(FieldSep_Text), 63);
    solutionOption.outhead = cBOutputHeader->currentIndex();
    solutionOption.outopt = cBOutputOptions->currentIndex();
    solutionOption.outvel = cBOutputVelocity->currentIndex();
    processOptions.outsingle = cBOutputSingle->currentIndex();
    solutionOption.maxsolstd = sBMaxSolutionStd->value();
    solutionOption.datum = cBOutputDatum->currentIndex();
    solutionOption.height = cBOutputHeight->currentIndex();
    solutionOption.geoid = cBOutputGeoid->currentIndex();
    if (options == NaviOptions) {
        solutionOption.nmeaintv[0] = sBNmeaInterval1->value();
        solutionOption.nmeaintv[1] = sBNmeaInterval2->value();
    }
    solutionOption.solstatic = cBSolutionStatic->currentIndex();
    debugTrace = cBDebugTrace->currentIndex();
    debugStatus = cBDebugStatus->currentIndex();

    processOptions.eratio[0] = sBMeasurementErrorR1->value();
    processOptions.eratio[1] = sBMeasurementErrorR2->value();
    processOptions.eratio[2] = sBMeasurementErrorR5->value();
    processOptions.err[1] = sBMeasurementError2->value();
    processOptions.err[2] = sBMeasurementError3->value();
    processOptions.err[3] = sBMeasurementError4->value();
    processOptions.err[4] = sBMeasurementError5->value();
    processOptions.prn[0] = sBProcessNoise1->value();
    processOptions.prn[1] = sBProcessNoise2->value();
    processOptions.prn[2] = sBProcessNoise3->value();
    processOptions.prn[3] = sBProcessNoise4->value();
    processOptions.prn[4] = sBProcessNoise5->value();
    processOptions.sclkstab = sBSatelliteClockStability->value();
    processOptions.maxaveep = sBMaxAveEp->value();
    processOptions.initrst = cBInitRestart->isChecked();

    roverPositionType = cBRoverPositionType->currentIndex();
    referencePositionType = cBReferencePositionType->currentIndex();
    roverAntennaPcv = cBRoverAntennaPcv->isChecked();
    referenceAntennaPcv = cBReferenceAntennaPcv->isChecked();
    roverAntenna = cBRoverAntenna->currentText();
    referenceAntenna = cBReferenceAntenna->currentText();
    roverAntennaDelta[0] = sBRoverAntennaE->value();
    roverAntennaDelta[1] = sBRoverAntennaN->value();
    roverAntennaDelta[2] = sBRoverAntennaU->value();
    referenceAntennaDelta[0] = sBReferenceAntennaE->value();
    referenceAntennaDelta[1] = sBReferenceAntennaN->value();
    referenceAntennaDelta[2] = sBReferenceAntennaU->value();
    getPosition(cBRoverPositionType->currentIndex(), editu, roverPosition);
    getPosition(cBReferencePositionType->currentIndex(), editr, referencePosition);

    satellitePcvFile = lESatellitePcvFile->text();
    antennaPcvFile = lEAntennaPcvFile->text();
    stationPositionFile = lEStationPositionFile->text();
    geoidDataFile = lEGeoidDataFile->text();
    dcbFile = lEDCBFile->text();
    eopFile = lEEOPFile->text();
    blqFile = lEBLQFile->text();

    if (options == NaviOptions) {
        serverCycle = sBServerCycle->value();
        timeoutTime = sBTimeoutTime->value();
        reconnectTime = sBReconnectTime->value();
        nmeaCycle = sBNmeaCycle->value();
        fileSwapMargin = sBFileSwapMargin->value();
        serverBufferSize = sBServerBufferSize->value();
        solutionBufferSize = sBSolutionBufferSize->value();
        savedSolution = sBSavedSolution->value();
        navSelect = cBNavSelect->currentIndex();
        processOptions.sbassatsel = sBSbasSatellite->value();
        proxyAddr = lEProxyAddress->text();
        monitorPort = sBMonitorPort->value();
        panelStack = cBPanelStack->currentIndex();

        panelFont = fontLabelPanel->font();
        positionFont = fontLabelSolution->font();
        localDirectory = lELocalDirectory->text();
    } else if (options == PostOptions) {
        rnxOptions1 = lERnxOptions1->text();
        rnxOptions2 = lERnxOptions2->text();
        pppOptions = lEPPPOptions->text();

        intpolateReferenceObs = cBIntputReferenceObservation->currentIndex();
        sbasSat = lESbasSat->text().toInt();
        ionosphereFile = lEIonosphereFile->text();

        roverList = tERoverList->toPlainText();
        baseList = tEBaseList->toPlainText();
    }

	updateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::load(const QString &file)
{
    int itype[] = { STR_SERIAL, STR_TCPCLI, STR_TCPSVR, STR_NTRIPCLI, STR_FILE, STR_FTP, STR_HTTP };
    int otype[] = { STR_SERIAL, STR_TCPCLI, STR_TCPSVR, STR_NTRIPSVR, STR_NTRIPCAS, STR_FILE };
    QLineEdit *editu[] = { lERoverPosition1, lERoverPosition2, lERoverPosition3 };
    QLineEdit *editr[] = { lEReferencePosition1, lEReferencePosition2, lEReferencePosition3 };
    QString buff;
    char id[32];
	int sat;
    prcopt_t prcopt = prcopt_default;
    solopt_t solopt = solopt_default;
    filopt_t filopt;

    memset(&filopt, 0, sizeof(filopt_t));

	resetsysopts();
    if (!loadopts(qPrintable(file), sysopts) ||
        !loadopts(qPrintable(file), rcvopts)) return;
    getsysopts(&prcopt, &solopt, &filopt);

    if (options == NaviOptions) {
        for (int i = 0; i < 8; i++) {
            mainForm->streamEnabled[i] = strtype[i] != STR_NONE;
            mainForm->streamType[i] = STR_NONE;
            for (int j = 0; j < (i < 3 ? 7 : 5); j++) {
                if (strtype[i] != (i < 3 ? itype[j] : otype[j])) continue;
                mainForm->streamType[i] = j;
                break;
            }
            if (i < 5) mainForm->inputFormat[i] = strfmt[i];

            if (strtype[i] == STR_SERIAL)
                mainForm->paths[i][0] = strpath[i];
            else if (strtype[i] == STR_FILE)
                mainForm->paths[i][2] = strpath[i];
            else if (strtype[i] <= STR_NTRIPCLI)
                mainForm->paths[i][1] = strpath[i];
            else if (strtype[i] <= STR_HTTP)
                mainForm->paths[i][3] = strpath[i];
        }
        mainForm->nmeaRequestType = nmeareq;
        mainForm->nmeaPosition[0] = nmeapos[0];
        mainForm->nmeaPosition[1] = nmeapos[1];
    }

    sBSbasSatellite->setValue(prcopt.sbassatsel);

    cBPositionMode->setCurrentIndex(prcopt.mode);
    cBFrequencies->setCurrentIndex(prcopt.nf > NFREQ - 1 ? NFREQ - 1 : prcopt.nf - 1);
    cBSolution->setCurrentIndex(prcopt.soltype);
    cBElevationMask->setCurrentIndex(cBElevationMask->findText(QString::number(prcopt.elmin * R2D, 'f', 0)));
    processOptions.snrmask = prcopt.snrmask;
    cBDynamicModel->setCurrentIndex(prcopt.dynamics);
    cBTideCorrection->setCurrentIndex(prcopt.tidecorr);
    cBIonosphereOption->setCurrentIndex(prcopt.ionoopt);
    cBTroposphereOption->setCurrentIndex(prcopt.tropopt);
    cBSatelliteEphemeris->setCurrentIndex(prcopt.sateph);
    lEExcludedSatellites->setText("");
    for (sat = 1; sat <= MAXSAT; sat++) {
        if (!prcopt.exsats[sat - 1]) continue;
        satno2id(sat, id);
        buff += QString("%1%2%3").arg(buff.isEmpty() ? "" : " ").arg(prcopt.exsats[sat - 1] == 2 ? "+" : "").arg(id);
	}
    lEExcludedSatellites->setText(buff);
    cBNavSys1->setChecked(prcopt.navsys & SYS_GPS);
    cBNavSys2->setChecked(prcopt.navsys & SYS_GLO);
    cBNavSys3->setChecked(prcopt.navsys & SYS_GAL);
    cBNavSys4->setChecked(prcopt.navsys & SYS_QZS);
    cBNavSys5->setChecked(prcopt.navsys & SYS_SBS);
    cBNavSys6->setChecked(prcopt.navsys & SYS_CMP);
    cBNavSys7->setChecked(prcopt.navsys & SYS_IRN);
    cBPositionOption1->setChecked(prcopt.posopt[0]);
    cBPositionOption2->setChecked(prcopt.posopt[1]);
    cBPositionOption3->setChecked(prcopt.posopt[2]);
    cBPositionOption4->setChecked(prcopt.posopt[3]);
    cBPositionOption5->setChecked(prcopt.posopt[4]);
    cBPositionOption6->setChecked(prcopt.posopt[5]);

    cBAmbiguityResolutionGPS->setCurrentIndex(prcopt.modear);
    cBAmbiguityResolutionGLO->setCurrentIndex(prcopt.glomodear);
    cBAmbiguityResolutionBDS->setCurrentIndex(prcopt.bdsmodear);
    sBValidThresAR->setValue(prcopt.thresar[0]);
    sBMaxPositionVarAR->setValue(prcopt.thresar[1]);
    sBGlonassHwBias->setValue(prcopt.thresar[2]);
    sBValidThresARMin->setValue(prcopt.thresar[5]);
    sBValidThresARMax->setValue(prcopt.thresar[6]);
    sBOutageCountResetAmbiguity->setValue(prcopt.maxout);
    sBFixCountHoldAmbiguity->setValue(prcopt.minfix);
    sBLockCountFixAmbiguity->setValue(prcopt.minlock);
    sBElevationMaskAR->setValue(prcopt.elmaskar * R2D);
    sBElevationMaskHold->setValue(prcopt.elmaskhold * R2D);
    sBMaxAgeDifferences->setValue(prcopt.maxtdiff);
    sBRejectCode->setValue(prcopt.maxinno[0]);
    sBRejectPhase->setValue(prcopt.maxinno[1]);
    sBVarHoldAmb->setValue(prcopt.varholdamb);
    sBGainHoldAmb->setValue(prcopt.gainholdamb);
    sBSlipThreshold->setValue(prcopt.thresslip);
    sBDopplerThreshold->setValue(prcopt.thresdop);
    //sBARIter->setValue(prcopt.armaxiter);
    sBMinFixSats->setValue(prcopt.minfixsats);
    sBMinHoldSats->setValue(prcopt.minholdsats);
    sBMinDropSats->setValue(prcopt.mindropsats);
    sBNumIteration->setValue(prcopt.niter);
    cBSyncSolution->setCurrentIndex(prcopt.syncsol);
    cBARFilter->setCurrentIndex(prcopt.arfilter);
    sBBaselineLen->setValue(prcopt.baseline[0]);
    sBBaselineSig->setValue(prcopt.baseline[1]);
    cBBaselineConstrain->setChecked(prcopt.baseline[0] > 0.0);

    cBSolutionFormat->setCurrentIndex(solopt.posf);
    cBTimeFormat->setCurrentIndex(solopt.timef == 0 ? 0 : solopt.times + 1);
    sBTimeDecimal->setValue(solopt.timeu);
    cBLatLonFormat->setCurrentIndex(solopt.degf);
    lEFieldSeperator->setText(solopt.sep);
    cBOutputHeader->setCurrentIndex(solopt.outhead);
    cBOutputOptions->setCurrentIndex(solopt.outopt);
    cBOutputVelocity->setCurrentIndex(solopt.outvel);
    cBOutputSingle->setCurrentIndex(prcopt.outsingle);
    sBMaxSolutionStd->setValue(solopt.maxsolstd);
    cBOutputDatum->setCurrentIndex(solopt.datum);
    cBOutputHeight->setCurrentIndex(solopt.height);
    cBOutputGeoid->setCurrentIndex(solopt.geoid);
    cBSolutionStatic->setCurrentIndex(solopt.solstatic);
    sBNmeaInterval1->setValue(solopt.nmeaintv[0]);
    sBNmeaInterval2->setValue(solopt.nmeaintv[1]);
    cBDebugTrace->setCurrentIndex(solopt.trace);
    cBDebugStatus->setCurrentIndex(solopt.sstat);

    sBMeasurementErrorR1->setValue(prcopt.eratio[0]);
    sBMeasurementErrorR2->setValue(prcopt.eratio[1]);
    sBMeasurementErrorR5->setValue(prcopt.eratio[2]);
    sBMeasurementError2->setValue(prcopt.err[1]);
    sBMeasurementError3->setValue(prcopt.err[2]);
    sBMeasurementError4->setValue(prcopt.err[3]);
    sBMeasurementError5->setValue(prcopt.err[4]);
    sBSatelliteClockStability->setValue(prcopt.sclkstab);
    sBProcessNoise1->setValue(prcopt.prn[0]);
    sBProcessNoise2->setValue(prcopt.prn[1]);
    sBProcessNoise3->setValue(prcopt.prn[2]);
    sBProcessNoise4->setValue(prcopt.prn[3]);
    sBProcessNoise5->setValue(prcopt.prn[4]);

    cBRoverAntennaPcv->setChecked(*prcopt.anttype[0]);
    cBReferenceAntennaPcv->setChecked(*prcopt.anttype[1]);
    cBRoverAntenna->setCurrentIndex(cBRoverAntenna->findText(prcopt.anttype[0]));
    cBReferenceAntenna->setCurrentIndex(cBReferenceAntenna->findText(prcopt.anttype[1]));
    sBRoverAntennaE->setValue(prcopt.antdel[0][0]);
    sBRoverAntennaN->setValue(prcopt.antdel[0][1]);
    sBRoverAntennaU->setValue(prcopt.antdel[0][2]);
    sBReferenceAntennaE->setValue(prcopt.antdel[1][0]);
    sBReferenceAntennaN->setValue(prcopt.antdel[1][1]);
    sBReferenceAntennaU->setValue(prcopt.antdel[1][2]);
    sBMaxAveEp->setValue(prcopt.maxaveep);
    cBInitRestart->setChecked(prcopt.initrst);

    lERnxOptions1->setText(prcopt.rnxopt[0]);
    lERnxOptions2->setText(prcopt.rnxopt[1]);
    lEPPPOptions->setText(prcopt.pppopt);

    if (options == NaviOptions) {
        cBRoverPositionType->setCurrentIndex(0);
        cBReferencePositionType->setCurrentIndex(0);
        if      (prcopt.refpos==POSOPT_RTCM  ) cBReferencePositionType->setCurrentIndex(3);
        else if (prcopt.refpos==POSOPT_SINGLE) cBReferencePositionType->setCurrentIndex(4);
    } else if (options == PostOptions) {
        cBIntputReferenceObservation->setCurrentIndex(prcopt.intpref);
        lESbasSat->setText(QString::number(prcopt.sbassatsel));
        cBRoverPositionType->setCurrentIndex(prcopt.rovpos == 0 ? 0 : prcopt.rovpos + 2);
        cBReferencePositionType->setCurrentIndex(prcopt.refpos == 0 ? 0 : prcopt.refpos + 2);
    }

    roverPositionType = cBRoverPositionType->currentIndex();
    referencePositionType = cBReferencePositionType->currentIndex();
    setPosition(cBRoverPositionType->currentIndex(), editu, prcopt.ru);
    setPosition(cBReferencePositionType->currentIndex(), editr, prcopt.rb);

    lESatellitePcvFile->setText(filopt.satantp);
    lEAntennaPcvFile->setText(filopt.rcvantp);
    lEStationPositionFile->setText(filopt.stapos);
    lEGeoidDataFile->setText(filopt.geoid);
    lEEOPFile->setText(filopt.eop);
    lEDCBFile->setText(filopt.dcb);
    lEBLQFile->setText(filopt.blq);
    lELocalDirectory->setText(filopt.tempdir);
    lEIonosphereFile->setText(filopt.iono);

	readAntennaList();
	updateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::save(const QString &file)
{
    QString ProxyAddrE_Text = lEProxyAddress->text();
    QString ExSats_Text = lEExcludedSatellites->text();
    QString FieldSep_Text = lEFieldSeperator->text();
    QString RovAnt_Text = cBRoverAntenna->currentText(), RefAnt_Text = cBReferenceAntenna->currentText();
    QString SatPcvFile_Text = lESatellitePcvFile->text();
    QString AntPcvFile_Text = lEAntennaPcvFile->text();
    QString lEStationPositionFile_Text = lEStationPositionFile->text();
    QString GeoidDataFile_Text = lEGeoidDataFile->text();
    QString EOPFile_Text = lEEOPFile->text();
    QString DCBFile_Text = lEDCBFile->text();
    QString BLQFile_Text = lEBLQFile->text();
    QString IonoFile_Text = lEIonosphereFile->text();
    QString RnxOpts1_Text = lERnxOptions1->text();
    QString RnxOpts2_Text = lERnxOptions2->text();
    QString PPPOpts_Text = lEPPPOptions->text();
    QString LocalDir_Text = lELocalDirectory->text();
    int itype[] = { STR_SERIAL, STR_TCPCLI, STR_TCPSVR, STR_NTRIPCLI, STR_FILE, STR_FTP, STR_HTTP };
    int otype[] = { STR_SERIAL, STR_TCPCLI, STR_TCPSVR, STR_NTRIPSVR, STR_NTRIPCAS, STR_FILE };
    QLineEdit *editu[] = { lERoverPosition1, lERoverPosition2, lERoverPosition3 };
    QLineEdit *editr[] = { lEReferencePosition1, lEReferencePosition2, lEReferencePosition3 };
    char buff[1024], *p, *q, comment[256], s[64];
    int sat, ex;
    prcopt_t prcopt = prcopt_default;
    solopt_t solopt = solopt_default;
    filopt_t filopt;

    memset(&filopt, 0, sizeof(filopt_t));

    if (options == NaviOptions) {
        for (int i = 0; i < 8; i++) {
            strtype[i] = i < 3 ? itype[mainForm->streamType[i]] : otype[mainForm->streamType[i]];
            strfmt[i] = mainForm->inputFormat[i];

            if (!mainForm->streamEnabled[i]) {
                strtype[i] = STR_NONE;
                strncpy(strpath[i], "", MAXSTR-1);
            } else if (strtype[i] == STR_SERIAL) {
                strncpy(strpath[i], qPrintable(mainForm->paths[i][0]), MAXSTR-1);
            } else if (strtype[i] == STR_FILE) {
                strncpy(strpath[i], qPrintable(mainForm->paths[i][2]), MAXSTR-1);
            } else if (strtype[i] == STR_TCPSVR) {
                strncpy(buff, qPrintable(mainForm->paths[i][1]), MAXSTR-1);
                if ((p = strchr(buff, '/'))) *p = '\0'; // TODO
                if ((p = strrchr(buff, ':'))) {
                    strncpy(strpath[i], p, MAXSTR-1);
                }
                else {
                    strncpy(strpath[i], "", MAXSTR-1);
                }
            }
            else if (strtype[i] == STR_TCPCLI) {
                strncpy(buff, qPrintable(mainForm->paths[i][1]), 1023);
                if ((p = strchr(buff, '/'))) *p = '\0';
                if ((p = strrchr(buff, '@'))) {
                    strncpy(strpath[i], p+1, MAXSTR-1);
                }
                else {
                    strncpy(strpath[i], buff, MAXSTR-1);
                }
            }
            else if (strtype[i] == STR_NTRIPSVR) {
                strncpy(buff, qPrintable(mainForm->paths[i][1]), 1023);
                if ((p = strchr(buff, ':')) && strchr(p + 1, '@')) {
                    strncpy(strpath[i], p, MAXSTR-1);
                }
                else {
                    strncpy(strpath[i], buff, MAXSTR-1);
                }
            }
            else if (strtype[i] == STR_NTRIPCLI) {
                strncpy(buff, qPrintable(mainForm->paths[i][1]), 1023);
                if ((p = strchr(buff, '/')) && (q = strchr(p + 1, ':'))) *q = '\0';
                strncpy(strpath[i], buff, MAXSTR);
            }
            else if (strtype[i] == STR_NTRIPCAS) {
                strncpy(buff, qPrintable(mainForm->paths[i][1]), 1023);
                if ((p = strchr(buff, '/')) && (q = strchr(p + 1, ':'))) *q = '\0';
                if ((p = strchr(buff, '@'))) {
                    *(p + 1) = '\0';
                    strncpy(strpath[i], buff, MAXSTR-1);
                }
                if ((p = strchr(p ? p + 2 : buff, ':'))) {
                    strncat(strpath[i], p, MAXSTR-1);
                }
            } else if (strtype[i] == STR_FTP || strtype[i] == STR_HTTP)
            {
                strncpy(strpath[i], qPrintable(mainForm->paths[i][3]), MAXSTR-1);
            }
        }
        nmeareq = mainForm->nmeaRequestType;
        nmeapos[0] = mainForm->nmeaPosition[0];
        nmeapos[1] = mainForm->nmeaPosition[1];

        svrcycle = sBServerCycle->value();
        timeout = sBTimeoutTime->value();
        reconnect = sBReconnectTime->value();
        nmeacycle = sBNmeaCycle->value();
        buffsize = sBServerBufferSize->value();
        navmsgsel = cBNavSelect->currentIndex();
        strncpy(proxyaddr, qPrintable(ProxyAddrE_Text), MAXSTR-1);
        fswapmargin = sBFileSwapMargin->value();
    }

    prcopt.mode = cBPositionMode->currentIndex();
    prcopt.nf = cBFrequencies->currentIndex() + 1;
    prcopt.soltype = cBSolution->currentIndex();
    prcopt.elmin = cBElevationMask->currentText().toDouble() * D2R;
    prcopt.snrmask = processOptions.snrmask;
    prcopt.dynamics = cBDynamicModel->currentIndex();
    prcopt.tidecorr = cBTideCorrection->currentIndex();
    prcopt.ionoopt = cBIonosphereOption->currentIndex();
    prcopt.tropopt = cBTroposphereOption->currentIndex();
    prcopt.sateph = cBSatelliteEphemeris->currentIndex();
    if (lEExcludedSatellites->text() != "") {
        strncpy(buff, qPrintable(ExSats_Text), 1023);
        for (p = strtok(buff, " "); p; p = strtok(NULL, " ")) {
            if (*p == '+') {
                ex = 2; p++;
            } else {
                ex = 1;
            }
            if (!(sat = satid2no(p))) continue;
            prcopt.exsats[sat - 1] = (unsigned char)ex;
        }
    }
    prcopt.navsys = (cBNavSys1->isChecked() ? SYS_GPS : 0) |
            (cBNavSys2->isChecked() ? SYS_GLO : 0) |
            (cBNavSys3->isChecked() ? SYS_GAL : 0) |
            (cBNavSys4->isChecked() ? SYS_QZS : 0) |
            (cBNavSys5->isChecked() ? SYS_SBS : 0) |
            (cBNavSys6->isChecked() ? SYS_CMP : 0) |
            (cBNavSys7->isChecked() ? SYS_IRN : 0);
    prcopt.posopt[0] = cBPositionOption1->isChecked();
    prcopt.posopt[1] = cBPositionOption2->isChecked();
    prcopt.posopt[2] = cBPositionOption3->isChecked();
    prcopt.posopt[3] = cBPositionOption4->isChecked();
    prcopt.posopt[4] = cBPositionOption5->isChecked();
    prcopt.posopt[5] = cBPositionOption6->isChecked();
    //	prcopt.mapfunc=MapFunc->currentIndex();

    prcopt.modear = cBAmbiguityResolutionGPS->currentIndex();
    prcopt.glomodear = cBAmbiguityResolutionGLO->currentIndex();
    prcopt.bdsmodear = cBAmbiguityResolutionBDS->currentIndex();
    prcopt.thresar[0] = sBValidThresAR->value();
    prcopt.thresar[1] = sBMaxPositionVarAR->value();
    prcopt.thresar[2] = sBGlonassHwBias->value();
    prcopt.thresar[5] = sBValidThresARMin->value();
    prcopt.thresar[6] = sBValidThresARMax->value();
    prcopt.maxout = sBOutageCountResetAmbiguity->value();
    prcopt.minfix = sBFixCountHoldAmbiguity->value();
    prcopt.minlock = sBLockCountFixAmbiguity->value();
    prcopt.elmaskar = sBElevationMaskAR->value() * D2R;
    prcopt.elmaskhold = sBElevationMaskHold->value() * D2R;
    prcopt.maxtdiff = sBMaxAgeDifferences->value();
    prcopt.maxinno[0] = sBRejectCode->value();
    prcopt.maxinno[1] = sBRejectPhase->value();
    prcopt.varholdamb = sBVarHoldAmb->value();
    prcopt.gainholdamb = sBGainHoldAmb->value();
    prcopt.thresslip = sBSlipThreshold->value();
    prcopt.thresdop = sBDopplerThreshold->value();
    //prcopt.armaxiter = sBARIter->value();
    prcopt.niter = sBNumIteration->value();
    prcopt.minfix = sBMinFixSats->value();
    prcopt.minholdsats = sBMinHoldSats->value();
    prcopt.mindropsats = sBMinDropSats->value();
    prcopt.syncsol = cBSyncSolution->currentIndex();
    prcopt.arfilter = cBARFilter->currentIndex();
    if (prcopt.mode == PMODE_MOVEB && cBBaselineConstrain->isChecked()) {
        prcopt.baseline[0] = sBBaselineLen->value();
        prcopt.baseline[1] = sBBaselineSig->value();
    }
    solopt.posf = cBSolutionFormat->currentIndex();
    solopt.timef = cBTimeFormat->currentIndex() == 0 ? 0 : 1;
    solopt.times = cBTimeFormat->currentIndex() == 0 ? 0 : cBTimeFormat->currentIndex() - 1;
    solopt.timeu = sBTimeDecimal->value();
    solopt.degf = cBLatLonFormat->currentIndex();
    strncpy(solopt.sep, qPrintable(FieldSep_Text), 63);
    solopt.outhead = cBOutputHeader->currentIndex();
    solopt.outopt = cBOutputOptions->currentIndex();
    solopt.outvel = cBOutputVelocity->currentIndex();
    prcopt.outsingle = cBOutputSingle->currentIndex();
    solopt.maxsolstd = sBMaxSolutionStd->value();
    solopt.datum = cBOutputDatum->currentIndex();
    solopt.height = cBOutputHeight->currentIndex();
    solopt.geoid = cBOutputGeoid->currentIndex();
    solopt.solstatic = cBSolutionFormat->currentIndex();
    solopt.nmeaintv[0] = sBNmeaInterval1->value();
    solopt.nmeaintv[1] = sBNmeaInterval2->value();
    solopt.trace = cBDebugTrace->currentIndex();
    solopt.sstat = cBDebugStatus->currentIndex();

    prcopt.eratio[0] = sBMeasurementErrorR1->value();
    prcopt.eratio[1] = sBMeasurementErrorR2->value();
    prcopt.eratio[2] = sBMeasurementErrorR5->value();
    prcopt.err[1] = sBMeasurementError2->value();
    prcopt.err[2] = sBMeasurementError3->value();
    prcopt.err[3] = sBMeasurementError4->value();
    prcopt.err[4] = sBMeasurementError5->value();
    prcopt.sclkstab = sBSatelliteClockStability->value();
    prcopt.prn[0] = sBProcessNoise1->value();
    prcopt.prn[1] = sBProcessNoise2->value();
    prcopt.prn[2] = sBProcessNoise3->value();
    prcopt.prn[3] = sBProcessNoise4->value();
    prcopt.prn[4] = sBProcessNoise5->value();

    if (cBRoverAntennaPcv->isChecked()) strncpy(prcopt.anttype[0], qPrintable(RovAnt_Text), MAXANT-1);
    if (cBReferenceAntennaPcv->isChecked()) strncpy(prcopt.anttype[1], qPrintable(RefAnt_Text), MAXANT-1);
    prcopt.antdel[0][0] = sBRoverAntennaE->value();
    prcopt.antdel[0][1] = sBRoverAntennaN->value();
    prcopt.antdel[0][2] = sBRoverAntennaU->value();
    prcopt.antdel[1][0] = sBReferenceAntennaE->value();
    prcopt.antdel[1][1] = sBReferenceAntennaN->value();
    prcopt.antdel[1][2] = sBReferenceAntennaU->value();
    prcopt.maxaveep = sBMaxAveEp->value();
    prcopt.initrst = cBInitRestart->isChecked();

    if (options == NaviOptions) {
        prcopt.sbassatsel = sBSbasSatellite->value();
        prcopt.rovpos = POSOPT_POS;
        prcopt.refpos = POSOPT_POS;
        if      (cBReferencePositionType->currentIndex() == 3) prcopt.refpos = POSOPT_RTCM;
        else if (cBReferencePositionType->currentIndex() == 4) prcopt.refpos = POSOPT_SINGLE;
    } else if (options == PostOptions) {
        prcopt.sbassatsel = lESbasSat->text().toInt();
        prcopt.rovpos = cBRoverPositionType->currentIndex() < 3 ? 0 : cBRoverPositionType->currentIndex() - 2;
        prcopt.refpos = cBReferencePositionType->currentIndex() < 3 ? 0 : cBReferencePositionType->currentIndex() - 2;
    }

    if (prcopt.rovpos == POSOPT_POS) getPosition(cBRoverPositionType->currentIndex(), editu, prcopt.ru);
    if (prcopt.refpos == POSOPT_POS) getPosition(cBReferencePositionType->currentIndex(), editr, prcopt.rb);

    if (options == PostOptions) {
        prcopt.intpref = cBIntputReferenceObservation->currentIndex();
        strncpy(prcopt.rnxopt[0], qPrintable(RnxOpts1_Text), 255);
        strncpy(prcopt.rnxopt[1], qPrintable(RnxOpts2_Text), 255);
        strncpy(prcopt.pppopt, qPrintable(PPPOpts_Text), 255);
        strncpy(filopt.iono, qPrintable(IonoFile_Text), MAXSTRPATH-1);
    }

    strncpy(filopt.satantp, qPrintable(SatPcvFile_Text), MAXSTRPATH-1);
    strncpy(filopt.rcvantp, qPrintable(AntPcvFile_Text), MAXSTRPATH-1);
    strncpy(filopt.stapos, qPrintable(lEStationPositionFile_Text), MAXSTRPATH-1);
    strncpy(filopt.geoid, qPrintable(GeoidDataFile_Text), MAXSTRPATH-1);
    strncpy(filopt.eop, qPrintable(EOPFile_Text), MAXSTRPATH-1);
    strncpy(filopt.dcb, qPrintable(DCBFile_Text), MAXSTRPATH-1);
    strncpy(filopt.tempdir, qPrintable(LocalDir_Text), MAXSTRPATH-1);
    strncpy(filopt.blq, qPrintable(BLQFile_Text), MAXSTRPATH-1);

    time2str(utc2gpst(timeget()), s, 0);
    sprintf(comment, qPrintable(tr("RTKNavi_Qt options (%s, v.%s %s)")), s, VER_RTKLIB, PATCH_LEVEL);  //TODO
    setsysopts(&prcopt, &solopt, &filopt);
    if (!saveopts(qPrintable(file), "w", comment, sysopts) ||
        !saveopts(qPrintable(file), "a", "", rcvopts)) return;
}
//---------------------------------------------------------------------------
void OptDialog::updateEnable(void)
{
    bool rel = PMODE_DGPS <= cBPositionMode->currentIndex() && cBPositionMode->currentIndex() <= PMODE_FIXED;
    bool rtk = PMODE_KINEMA <= cBPositionMode->currentIndex() && cBPositionMode->currentIndex() <= PMODE_FIXED;
    bool ppp = cBPositionMode->currentIndex() >= PMODE_PPP_KINEMA;
    bool ar = rtk || ppp;
    
    cBFrequencies->setEnabled(rel || ppp);
    cBSolution->setEnabled(options == PostOptions ? rel || ppp : false);
    cBDynamicModel->setEnabled(rel);
    cBTideCorrection->setEnabled(rel || ppp);
    cBPositionOption1->setEnabled(ppp);
    cBPositionOption2->setEnabled(ppp);
    cBPositionOption3->setEnabled(ppp);
    cBPositionOption4->setEnabled(ppp);
    cBPositionOption5->setEnabled(ppp);
    cBPositionOption6->setEnabled(ppp);

    cBAmbiguityResolutionGPS->setEnabled(ar);
    cBAmbiguityResolutionGLO->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() > 0 && cBNavSys2->isChecked());
    cBAmbiguityResolutionBDS->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() > 0 && cBNavSys6->isChecked());
    sBValidThresAR->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() >= 1 && cBAmbiguityResolutionGPS->currentIndex() < 4);
    sBValidThresARMin->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() >= 1 && cBAmbiguityResolutionGPS->currentIndex() < 4);
    sBValidThresARMax->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() >= 1 && cBAmbiguityResolutionGPS->currentIndex() < 4);
    sBMaxPositionVarAR->setEnabled(ar && !ppp);
    sBGlonassHwBias->setEnabled(ar && cBAmbiguityResolutionGLO->currentIndex() == 2);
    sBLockCountFixAmbiguity->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() >= 1);
    sBElevationMaskAR->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() >= 1);
    sBOutageCountResetAmbiguity->setEnabled(ar || ppp);
    sBFixCountHoldAmbiguity->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() == 3);
    sBElevationMaskHold->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() == 3);
    sBSlipThreshold->setEnabled(ar || ppp);
    sBDopplerThreshold->setEnabled(ar || ppp);
    sBMaxAgeDifferences->setEnabled(rel);
    sBRejectCode->setEnabled(rel || ppp);
    sBRejectPhase->setEnabled(rel || ppp);
    sBVarHoldAmb->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() == 3);
    sBGainHoldAmb->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() == 3);
     //sBARIter->setEnabled(ppp);
    sBMinFixSats->setEnabled(ar || ppp);
    sBMinHoldSats->setEnabled(ar || ppp);
    sBMinDropSats->setEnabled(ar || ppp);
    sBMaxPositionVarAR->setEnabled(ar || ppp);
    sBNumIteration->setEnabled(rel || ppp);
    cBSyncSolution->setEnabled(options == NaviOptions ? rel || ppp : false);
    cBARFilter->setEnabled(ar || ppp);
    cBBaselineConstrain->setEnabled(cBPositionMode->currentIndex() == PMODE_MOVEB);
    sBBaselineLen->setEnabled(cBBaselineConstrain->isChecked() && cBPositionMode->currentIndex() == PMODE_MOVEB);
    sBBaselineSig->setEnabled(cBBaselineConstrain->isChecked() && cBPositionMode->currentIndex() == PMODE_MOVEB);

    cBOutputHeader->setEnabled(cBSolutionFormat->currentIndex() < 3);
    cBOutputOptions->setEnabled(options == PostOptions ? cBSolutionFormat->currentIndex() < 3: false);
    cBTimeFormat->setEnabled(cBSolutionFormat->currentIndex() < 3);
    sBTimeDecimal->setEnabled(cBSolutionFormat->currentIndex() < 3);
    cBLatLonFormat->setEnabled(cBSolutionFormat->currentIndex() == 0);
    lEFieldSeperator->setEnabled(cBSolutionFormat->currentIndex() < 3);
    cBOutputSingle->setEnabled(cBPositionMode->currentIndex() != 0);
    cBOutputDatum->setEnabled(cBSolutionFormat->currentIndex() == 0);
    cBOutputHeight->setEnabled(cBSolutionFormat->currentIndex() == 0);
    cBOutputGeoid->setEnabled(cBSolutionFormat->currentIndex() == 0 && cBOutputHeight->currentIndex() == 1);
    cBSolutionStatic->setEnabled(cBPositionMode->currentIndex() == PMODE_STATIC ||
                                 cBPositionMode->currentIndex() == PMODE_PPP_STATIC);

    cBRoverAntennaPcv->setEnabled(rel || ppp);
    cBRoverAntenna->setEnabled((rel || ppp) && cBRoverAntennaPcv->isChecked());
    sBRoverAntennaE->setEnabled((rel || ppp) && cBRoverAntennaPcv->isChecked()&&cBRoverAntenna->currentText()!="*");
    sBRoverAntennaN->setEnabled((rel || ppp) && cBRoverAntennaPcv->isChecked()&&cBRoverAntenna->currentText()!="*");
    sBRoverAntennaU->setEnabled((rel || ppp) && cBRoverAntennaPcv->isChecked()&&cBRoverAntenna->currentText()!="*");
    lblRoverAntennaD->setEnabled((rel || ppp) && cBRoverAntennaPcv->isChecked()&&cBRoverAntenna->currentText()!="*");
    cBReferenceAntennaPcv->setEnabled(rel);
    cBReferenceAntenna->setEnabled(rel && cBReferenceAntennaPcv->isChecked());
    sBReferenceAntennaE->setEnabled(rel && cBReferenceAntennaPcv->isChecked()&&cBReferenceAntenna->currentText()!="*");
    sBReferenceAntennaN->setEnabled(rel && cBReferenceAntennaPcv->isChecked()&&cBReferenceAntenna->currentText()!="*");
    sBReferenceAntennaU->setEnabled(rel && cBReferenceAntennaPcv->isChecked()&&cBReferenceAntenna->currentText()!="*");
    lblReferenceAntennaD->setEnabled(rel && cBReferenceAntennaPcv->isChecked()&&cBReferenceAntenna->currentText()!="*");

    cBRoverPositionType->setEnabled(cBPositionMode->currentIndex() == PMODE_FIXED || cBPositionMode->currentIndex() == PMODE_PPP_FIXED);
    lERoverPosition1->setEnabled(cBRoverPositionType->isEnabled() && cBRoverPositionType->currentIndex() <= 2);
    lERoverPosition2->setEnabled(cBRoverPositionType->isEnabled() && cBRoverPositionType->currentIndex() <= 2);
    lERoverPosition3->setEnabled(cBRoverPositionType->isEnabled() && cBRoverPositionType->currentIndex() <= 2);
    btnRoverPosition->setEnabled(cBRoverPositionType->isEnabled() && cBRoverPositionType->currentIndex() <= 2);

    cBReferencePositionType->setEnabled(rel && cBPositionMode->currentIndex() != PMODE_MOVEB);
    lEReferencePosition1->setEnabled(cBReferencePositionType->isEnabled() && cBReferencePositionType->currentIndex() <= 2);
    lEReferencePosition2->setEnabled(cBReferencePositionType->isEnabled() && cBReferencePositionType->currentIndex() <= 2);
    lEReferencePosition3->setEnabled(cBReferencePositionType->isEnabled() && cBReferencePositionType->currentIndex() <= 2);
    btnReferencePosition->setEnabled(cBReferencePositionType->isEnabled() && cBReferencePositionType->currentIndex() <= 2);

    lblMaxAveEp->setVisible(cBReferencePositionType->currentIndex() == 4);
    sBMaxAveEp->setVisible(cBReferencePositionType->currentIndex() == 4);
    cBInitRestart->setVisible(cBReferencePositionType->currentIndex() == 4);

    //sBSbasSatellite->setEnabled(cBPositionMode->currentIndex() == 0);
    if (options == NaviOptions) {
        lEIonosphereFile->setVisible(false);
        lblIonosphereFile->setVisible(false);
        lELocalDirectory->setVisible(true);
        lblLocalDirectory->setVisible(true);

        tWOptions->setTabVisible(6, true);
        tWOptions->setTabVisible(7, false);
    } else if (options == PostOptions) {
        lEIonosphereFile->setVisible(true);
        lblIonosphereFile->setVisible(false);
        lELocalDirectory->setVisible(false);
        lblLocalDirectory->setVisible(false);

        tWOptions->setTabVisible(6, false);
        tWOptions->setTabVisible(7, true);
    }
}
//---------------------------------------------------------------------------
void OptDialog::getPosition(int type, QLineEdit **edit, double *pos)
{
    double p[3] = { 0 }, dms1[3] = { 0 }, dms2[3] = { 0 };

    if (type == 1) { /* lat/lon/height dms/m */
        QStringList tokens = edit[0]->text().split(" ");
        for (int i = 0; i < 3 || i < tokens.size(); i++)
            dms1[i] = tokens.at(i).toDouble();

        tokens = edit[1]->text().split(" ");
        for (int i = 0; i < 3 || i < tokens.size(); i++)
            dms2[i] = tokens.at(i).toDouble();

        p[0] = (dms1[0] < 0 ? -1 : 1) * (fabs(dms1[0]) + dms1[1] / 60 + dms1[2] / 3600) * D2R;
        p[1] = (dms2[0] < 0 ? -1 : 1) * (fabs(dms2[0]) + dms2[1] / 60 + dms2[2] / 3600) * D2R;
        p[2] = edit[2]->text().toDouble();
        pos2ecef(p, pos);
    } else if (type == 2) { /* x/y/z-ecef */
        pos[0] = edit[0]->text().toDouble();
        pos[1] = edit[1]->text().toDouble();
        pos[2] = edit[2]->text().toDouble();
    } else {   /* lat/lon/hight decimal */
        p[0] = edit[0]->text().toDouble() * D2R;
        p[1] = edit[1]->text().toDouble() * D2R;
        p[2] = edit[2]->text().toDouble();
        pos2ecef(p, pos);
	}
}
//---------------------------------------------------------------------------
void OptDialog::setPosition(int type, QLineEdit **edit, double *pos)
{
    double p[3], dms1[3], dms2[3], s1, s2;

    if (type == 1) { /* lat/lon/height dms/m */
        ecef2pos(pos, p);
        s1 = p[0] < 0 ? -1 : 1;
        s2 = p[1] < 0 ? -1 : 1;
        p[0] = fabs(p[0]) * R2D + 1E-12;
        p[1] = fabs(p[1]) * R2D + 1E-12;
        dms1[0] = floor(p[0]); p[0] = (p[0] - dms1[0]) * 60.0;
        dms1[1] = floor(p[0]); dms1[2] = (p[0] - dms1[1]) * 60.0;
        dms2[0] = floor(p[1]); p[1] = (p[1] - dms2[0]) * 60.0;
        dms2[1] = floor(p[1]); dms2[2] = (p[1] - dms2[1]) * 60.0;

        edit[0]->setText(QString("%1 %2 %3").arg(s1 * dms1[0], 0, 'f', 0).arg(dms1[1], 2, 'f', 0, '0').arg(dms1[2], 9, 'f', 6, '0'));
        edit[1]->setText(QString("%1 %2 %3").arg(s2 * dms2[0], 0, 'f', 0).arg(dms2[1], 2, 'f', 0, '0').arg(dms2[2], 9, 'f', 6, '0'));
        edit[2]->setText(QString::number(p[2], 'f', 4));
    } else if (type == 2) { /* x/y/z-ecef */
        edit[0]->setText(QString::number(pos[0], 'f', 4));
        edit[1]->setText(QString::number(pos[1], 'f', 4));
        edit[2]->setText(QString::number(pos[2], 'f', 4));
    } else {   /* lat/lon/hight decimal */
        ecef2pos(pos, p);
        edit[0]->setText(QString::number(p[0] * R2D, 'f', 9));
        edit[1]->setText(QString::number(p[1] * R2D, 'f', 9));
        edit[2]->setText(QString::number(p[2], 'f', 4));
	}
}
//---------------------------------------------------------------------------
void OptDialog::readAntennaList(void)
{
    QString antennaPcvFile_Text = lEAntennaPcvFile->text();
    pcvs_t pcvs = { 0, 0, 0 };
    char *p;
    QString roverAntenna_Text, referenceAntenna_Text;
    int i;

    if (!readpcv(qPrintable(antennaPcvFile_Text), &pcvs)) return;

    /* Save currently defined antennas */
    roverAntenna_Text = cBRoverAntenna->currentText();
    referenceAntenna_Text = cBReferenceAntenna->currentText();

    /* Clear and add antennas from ANTEX file */
    cBRoverAntenna->clear();
    cBReferenceAntenna->clear();

    cBRoverAntenna->addItem(""); cBReferenceAntenna->addItem("");
    cBRoverAntenna->addItem("*"); cBReferenceAntenna->addItem("*");

    for (int i = 0; i < pcvs.n; i++) {
        if (pcvs.pcv[i].sat) continue;
        if ((p = strchr(pcvs.pcv[i].type, ' '))) *p = '\0';
        if (i > 0 && !strcmp(pcvs.pcv[i].type, pcvs.pcv[i - 1].type)) continue;
        cBRoverAntenna->addItem(pcvs.pcv[i].type);
        cBReferenceAntenna->addItem(pcvs.pcv[i].type);
    }

    /* Restore previously defined antennas */
    i = cBRoverAntenna->findText(roverAntenna_Text);
    cBRoverAntenna->setCurrentIndex(i == -1 ? 0 : i);
    i = cBReferenceAntenna->findText(referenceAntenna_Text);
    cBReferenceAntenna->setCurrentIndex(i == -1 ? 0 : i);

    free(pcvs.pcv);
}
//---------------------------------------------------------------------------
void OptDialog::showKeyDialog()
{
    KeyDialog *keyDialog = new KeyDialog(this);

    keyDialog->setFlag(2);
    keyDialog->exec();

    delete keyDialog;
}

//---------------------------------------------------------------------------
void OptDialog::showFrequenciesDialog()
{
    FreqDialog *freqDialog = new FreqDialog(this);

    freqDialog->exec();

    delete freqDialog;
}



