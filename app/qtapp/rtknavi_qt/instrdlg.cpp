//---------------------------------------------------------------------------

#include <QComboBox>
#include <QFileInfo>
#include <QShowEvent>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QCompleter>

#include "rtklib.h"
#include "refdlg.h"
#include "navimain.h"
#include "serioptdlg.h"
#include "cmdoptdlg.h"
#include "tcpoptdlg.h"
#include "ftpoptdlg.h"
#include "rcvoptdlg.h"

#include "instrdlg.h"

//---------------------------------------------------------------------------
InputStrDialog::InputStrDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    cBFormat1->clear();
    cBFormat2->clear();
    cBFormat3->clear();

    noFormats = 0;

    // add fill format combo box with all available formats
    for (int i = 0; i <= MAXRCVFMT; i++) {
        cBFormat1->addItem(formatstrs[i]);
        cBFormat2->addItem(formatstrs[i]);
        cBFormat3->addItem(formatstrs[i]);
        noFormats++;
	}
    cBFormat3->addItem(tr("SP3"));

    cmdOptDialog = new CmdOptDialog(this);
    rcvOptDialog = new RcvOptDialog(this);
    refDialog = new RefDialog(this);
    serialOptDialog = new SerialOptDialog(this);;
    tcpOptDialog = new TcpOptDialog(this);
    ftpOptDialog = new FtpOptDialog(this);

    // setup completers
    QCompleter *fileCompleter = new QCompleter(this);
    QFileSystemModel *fileModel = new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    lEFilePath1->setCompleter(fileCompleter);
    lEFilePath2->setCompleter(fileCompleter);
    lEFilePath3->setCompleter(fileCompleter);

    // line edit actions
    QAction *aclEFilePath1Select = lEFilePath1->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    aclEFilePath1Select->setToolTip(tr("Select File"));
    QAction *aclEFilePath2Select = lEFilePath2->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    aclEFilePath2Select->setToolTip(tr("Select File"));
    QAction *aclEFilePath3Select = lEFilePath3->addAction(QIcon(":/buttons/folder"), QLineEdit::TrailingPosition);
    aclEFilePath3Select->setToolTip(tr("Select File"));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &InputStrDialog::btnOkClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &InputStrDialog::reject);
    connect(btnCmd1, &QPushButton::clicked, this, &InputStrDialog::showCommandDialog1);
    connect(btnCmd2, &QPushButton::clicked, this, &InputStrDialog::showCommandDialog2);
    connect(btnCmd3, &QPushButton::clicked, this, &InputStrDialog::showCommandDialog3);
    connect(aclEFilePath1Select, &QAction::triggered, this, &InputStrDialog::selectFile1);
    connect(aclEFilePath2Select, &QAction::triggered, this, &InputStrDialog::selectFile2);
    connect(aclEFilePath3Select, &QAction::triggered, this, &InputStrDialog::selectFile3);
    connect(btnPosition, &QPushButton::clicked, this, &InputStrDialog::selectPosition);
    connect(btnReceiverOptions1, &QPushButton::clicked, this, &InputStrDialog::showReceiverOptions1);
    connect(btnReceiverOptions2, &QPushButton::clicked, this, &InputStrDialog::showReceiverOptions2);
    connect(btnReceiverOptions3, &QPushButton::clicked, this, &InputStrDialog::showReceiverOptions3);
    connect(btnStream1, &QPushButton::clicked, this, &InputStrDialog::showStreamOptions1);
    connect(btnStream2, &QPushButton::clicked, this, &InputStrDialog::showStreamOptions2);
    connect(btnStream3, &QPushButton::clicked, this, &InputStrDialog::showStreamOptions3);
    connect(cBStreamC1, &QCheckBox::clicked, this, &InputStrDialog::updateEnable);
    connect(cBStreamC2, &QCheckBox::clicked, this, &InputStrDialog::updateEnable);
    connect(cBStreamC3, &QCheckBox::clicked, this, &InputStrDialog::updateEnable);
    connect(cBStream1, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &InputStrDialog::updateEnable);
    connect(cBStream2, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &InputStrDialog::updateEnable);
    connect(cBStream3, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &InputStrDialog::updateEnable);
    connect(cBNmeaRequestType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &InputStrDialog::updateEnable);
    connect(cBTimeTag, &QCheckBox::clicked, this, &InputStrDialog::updateEnable);
}
//---------------------------------------------------------------------------
void InputStrDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    lEFilePath1->setText(getFilePath(paths[0][2]));
    lEFilePath2->setText(getFilePath(paths[1][2]));
    lEFilePath3->setText(getFilePath(paths[2][2]));

	updateEnable();
}
//---------------------------------------------------------------------------
void InputStrDialog::setStreamEnabled(int stream, int enabled)
{
    QCheckBox *cBStreamC[] = {cBStreamC1, cBStreamC2, cBStreamC3};
    cBStreamC[stream]->setChecked(enabled);
}
//---------------------------------------------------------------------------
int InputStrDialog::getStreamEnabled(int stream)
{
    QCheckBox *cBStreamC[] = {cBStreamC1, cBStreamC2, cBStreamC3};
    return cBStreamC[stream]->isChecked();
}
//---------------------------------------------------------------------------
void InputStrDialog::setStreamType(int stream, int type)
{
    QComboBox *cBStream[] = {cBStream1, cBStream2, cBStream3};
    cBStream[stream]->setCurrentIndex(type);
}
//---------------------------------------------------------------------------
int InputStrDialog::getStreamType(int stream)
{
    QComboBox *cBStream[] = {cBStream1, cBStream2, cBStream3};
    return cBStream[stream]->currentIndex();
}
//---------------------------------------------------------------------------
void InputStrDialog::setStreamFormat(int stream, int format)
{
    QComboBox *cBFormat[] = {cBFormat1, cBFormat2, cBFormat3};
    if (stream == 0)
        cBFormat[stream]->setCurrentIndex(format);
    else
        cBFormat[stream]->setCurrentIndex(format < noFormats ? format : noFormats + format - STRFMT_SP3);
}
//---------------------------------------------------------------------------
int InputStrDialog::getStreamFormat(int stream)
{
    QComboBox *cBFormat[] = {cBFormat1, cBFormat2, cBFormat3};
    if (stream == 0)
        return cBFormat[stream]->currentIndex();
    else
        return cBFormat[stream]->currentIndex() < noFormats ? cBFormat[stream]->currentIndex() : STRFMT_SP3 + cBFormat[stream]->currentIndex() - noFormats;
}
//---------------------------------------------------------------------------
void InputStrDialog::setReceiverOptions(int stream, const QString & options)
{
    receiverOptions[stream] = options;
}
//---------------------------------------------------------------------------
const QString &InputStrDialog::getReceiverOptions(int stream)
{
    return receiverOptions[stream];
}
//---------------------------------------------------------------------------
void InputStrDialog::setResetCommand(const QString & command)
{
    lEResetCmd->setText(command);
}
//---------------------------------------------------------------------------
QString InputStrDialog::getResetCommand()
{
    return lEResetCmd->text();
}
//---------------------------------------------------------------------------
void InputStrDialog::setNMeaPosition(int i, double value)
{
    QDoubleSpinBox *widget[] = {sBNmeaPosition1, sBNmeaPosition2, sBNmeaPosition3};
    widget[i]->setValue(value);
}
//---------------------------------------------------------------------------
double InputStrDialog::getNMeaPosition(int i)
{
    QDoubleSpinBox *widget[] = {sBNmeaPosition1, sBNmeaPosition2, sBNmeaPosition3};
    return widget[i]->value();
}
//---------------------------------------------------------------------------
void InputStrDialog::setNmeaRequestType(int nmeaMode)
{
    cBNmeaRequestType->setCurrentIndex(nmeaMode);
}
//---------------------------------------------------------------------------
int InputStrDialog::getNmeaRequestType()
{
    return cBNmeaRequestType->currentIndex();
}
//---------------------------------------------------------------------------
void InputStrDialog::setMaxBaseLine(double baseLine)
{
    sBMaxBaseLine->setValue(baseLine);
}
//---------------------------------------------------------------------------
double InputStrDialog::getMaxBaseLine()
{
    return sBMaxBaseLine->value();
}
//---------------------------------------------------------------------------
void InputStrDialog::setHistory(int i, const QString &history)
{
    this->history[i] = history;
}
//---------------------------------------------------------------------------
const QString &InputStrDialog::getHistory(int i)
{
    return history[i];
}
//---------------------------------------------------------------------------
void InputStrDialog::setTimeTagEnabled(bool tag)
{
    cBTimeTag->setChecked(tag);
}
//---------------------------------------------------------------------------
bool InputStrDialog::getTimeTagEnabled()
{
    return cBTimeTag->isChecked();
}
//---------------------------------------------------------------------------
void InputStrDialog::setTimeTag64bit(bool enabled)
{
    cB64Bit->setChecked(enabled);
}
//---------------------------------------------------------------------------
bool InputStrDialog::getTimeTag64bit()
{
    return cB64Bit->isChecked();
}
//---------------------------------------------------------------------------
void InputStrDialog::setTimeStart(const QString & time)
{
    lETimeStart->setText(time);
}
//---------------------------------------------------------------------------
QString InputStrDialog::getTimeStart()
{
    return lETimeStart->text();
}
//---------------------------------------------------------------------------
void InputStrDialog::setTimeSpeed(const QString & timeSpeed)
{
    cBTimeSpeed->setCurrentIndex(cBTimeSpeed->findText(timeSpeed));
}
//---------------------------------------------------------------------------
QString InputStrDialog::getTimeSpeed()
{
    return cBTimeSpeed->currentText();
}
//---------------------------------------------------------------------------
void InputStrDialog::setStationPositionFile(const QString & file)
{
    stationPositionFile = file;
}
//---------------------------------------------------------------------------
QString InputStrDialog::getStationPositionFile()
{
    return stationPositionFile;
}
//---------------------------------------------------------------------------
void InputStrDialog::btnOkClicked()
{
    paths[0][2] = setFilePath(lEFilePath1->text());
    paths[1][2] = setFilePath(lEFilePath2->text());
    paths[2][2] = setFilePath(lEFilePath3->text());

    accept();
}
//---------------------------------------------------------------------------
QString InputStrDialog::getFilePath(const QString &path)
{
    return path.mid(0, path.indexOf("::"));
}
//---------------------------------------------------------------------------
QString InputStrDialog::setFilePath(const QString &p)
{
    QString path = p;

    if (cBTimeTag->isChecked()) path += "::T";
    if (lETimeStart->text() != "0") path += "::+" + lETimeStart->text();
    path += "::" + cBTimeSpeed->currentText();
    if (cB64Bit->isChecked()) path += "::P=8";
    return path;
}
//---------------------------------------------------------------------------
void InputStrDialog::showStreamOptions1()
{
    switch (cBStream1->currentIndex()) {
        case 0: showSerialOptionsDialog(0, 0); break;
        case 1: showTcpOptionsDialog(0, 1); break;
        case 2: showTcpOptionsDialog(0, 0); break;
        case 3: showTcpOptionsDialog(0, 3); break;
	}
}
//---------------------------------------------------------------------------
void InputStrDialog::showStreamOptions2()
{
    switch (cBStream2->currentIndex()) {
        case 0: showSerialOptionsDialog(1, 0); break;
        case 1: showTcpOptionsDialog(1, 1); break;
        case 2: showTcpOptionsDialog(1, 0); break;
        case 3: showTcpOptionsDialog(1, 3); break;
        case 5: showFtpOptionsDialog(1, 0); break;
        case 6: showFtpOptionsDialog(1, 1); break;
	}
}
//---------------------------------------------------------------------------
void InputStrDialog::showStreamOptions3()
{
    switch (cBStream3->currentIndex()) {
        case 0: showSerialOptionsDialog(2, 0); break;
        case 1: showTcpOptionsDialog(2, 1); break;
        case 2: showTcpOptionsDialog(2, 0); break;
        case 3: showTcpOptionsDialog(2, 3); break;
        case 5: showFtpOptionsDialog(2, 0); break;
        case 6: showFtpOptionsDialog(2, 1); break;
	}
}
//---------------------------------------------------------------------------
void InputStrDialog::showCommandDialog(int streamNo)
{
    for (int i = 0; i < 3; i++) {
        if (cBStream1->currentIndex() == 0) {
            cmdOptDialog->setCommands(i, commands[streamNo][i]);
            cmdOptDialog->setCommandsEnabled(i, commandEnable[streamNo][i]);
        }
        else {
            cmdOptDialog->setCommands(i, commandsTcp[streamNo][i]);
            cmdOptDialog->setCommandsEnabled(i, commandEnableTcp[streamNo][i]);
        }
    }

    cmdOptDialog->exec();
    if (cmdOptDialog->result() != QDialog::Accepted) return;

    for (int i = 0; i < 3; i++) {
        if (cBStream1->currentIndex() == 0) {
            commands[streamNo][i] = cmdOptDialog->getCommands(i);
            commandEnable[streamNo][i] = cmdOptDialog->getCommandsEnabled(i);
        }
        else {
            commandsTcp[streamNo][i] = cmdOptDialog->getCommands(i);
            commandEnableTcp[streamNo][i] = cmdOptDialog->getCommandsEnabled(i);
        }
    }
}
//---------------------------------------------------------------------------
void InputStrDialog::showCommandDialog1()
{
    this->showCommandDialog(0);
}
//---------------------------------------------------------------------------
void InputStrDialog::showCommandDialog2()
{
    this->showCommandDialog(1);
}
//---------------------------------------------------------------------------
void InputStrDialog::showCommandDialog3()
{
    this->showCommandDialog(2);
}
//---------------------------------------------------------------------------
void InputStrDialog::showReceiverOptionDialog(int streamNo)
{
    rcvOptDialog->setOptions(receiverOptions[streamNo]);

    rcvOptDialog->exec();
    if (rcvOptDialog->result() != QDialog::Accepted) return;

    receiverOptions[streamNo] = rcvOptDialog->getOptions();
}
//---------------------------------------------------------------------------
void InputStrDialog::showReceiverOptions1()
{
    this->showReceiverOptionDialog(0);
}
//---------------------------------------------------------------------------
void InputStrDialog::showReceiverOptions2()
{
    this->showReceiverOptionDialog(1);
}
//---------------------------------------------------------------------------
void InputStrDialog::showReceiverOptions3()
{
    this->showReceiverOptionDialog(2);
}
//---------------------------------------------------------------------------
void InputStrDialog::selectPosition()
{
    refDialog->setRoverPosition(sBNmeaPosition1->value(), sBNmeaPosition2->value(), sBNmeaPosition3->value());
    refDialog->stationPositionFile = stationPositionFile;

    refDialog->exec();
    if (refDialog->result() != QDialog::Accepted) return;

    sBNmeaPosition1->setValue(refDialog->getPosition()[0]);
    sBNmeaPosition2->setValue(refDialog->getPosition()[1]);
    sBNmeaPosition3->setValue(refDialog->getPosition()[2]);
}
//---------------------------------------------------------------------------
void InputStrDialog::selectFile1()
{
    lEFilePath1->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open..."), lEFilePath1->text())));
}
//---------------------------------------------------------------------------
void InputStrDialog::selectFile2()
{
    lEFilePath2->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open..."), lEFilePath2->text())));
}
//---------------------------------------------------------------------------
void InputStrDialog::selectFile3()
{
    lEFilePath3->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open..."), lEFilePath3->text())));
}
//---------------------------------------------------------------------------
void InputStrDialog::showSerialOptionsDialog(int index, int opt)
{
    serialOptDialog->setPath(paths[index][0]);
    serialOptDialog->setOptions(opt);

    serialOptDialog->exec();
    if (serialOptDialog->result() != QDialog::Accepted) return;

    paths[index][0] = serialOptDialog->getPath();
}
//---------------------------------------------------------------------------
void InputStrDialog::showTcpOptionsDialog(int index, int opt)
{
    tcpOptDialog->setPath(paths[index][1]);
    tcpOptDialog->setOptions(opt);
    tcpOptDialog->setHistory(history, 10);

    tcpOptDialog->exec();
    if (tcpOptDialog->result() != QDialog::Accepted) return;

    paths[index][1] = tcpOptDialog->getPath();
    for (int i = 0; i < 10; i++) {
        history[i] = tcpOptDialog->getHistory()[i];
	}
}
//---------------------------------------------------------------------------
void InputStrDialog::showFtpOptionsDialog(int index, int opt)
{
    ftpOptDialog->setPath(paths[index][3]);
    ftpOptDialog->setOptions(opt);

    ftpOptDialog->exec();
    if (ftpOptDialog->result() != QDialog::Accepted) return;

    paths[index][3] = ftpOptDialog->getPath();
}
//---------------------------------------------------------------------------
void InputStrDialog::updateEnable()
{
    int enaFile = (cBStreamC1->isChecked() && (cBStream1->currentIndex() == 4)) ||
                  (cBStreamC2->isChecked() && (cBStream2->currentIndex() == 4)) ||
                  (cBStreamC3->isChecked() && (cBStream3->currentIndex() == 4));
    int enaNmea = cBStreamC2->isChecked() && (cBStream2->currentIndex() <= 3);

    cBStream1->setEnabled(cBStreamC1->isChecked());
    cBStream2->setEnabled(cBStreamC2->isChecked());
    cBStream3->setEnabled(cBStreamC3->isChecked());
    btnStream1->setEnabled(cBStreamC1->isChecked() && cBStream1->currentIndex() != 4);
    btnStream2->setEnabled(cBStreamC2->isChecked() && cBStream2->currentIndex() != 4);
    btnStream3->setEnabled(cBStreamC3->isChecked() && cBStream3->currentIndex() != 4);
    btnCmd1->setEnabled(cBStreamC1->isChecked() && cBStream1->currentIndex() != 4);
    btnCmd2->setEnabled(cBStreamC2->isChecked() && cBStream2->currentIndex() != 4);
    btnCmd3->setEnabled(cBStreamC3->isChecked() && cBStream3->currentIndex() != 4);
    cBFormat1->setEnabled(cBStreamC1->isChecked());
    cBFormat2->setEnabled(cBStreamC2->isChecked());
    cBFormat3->setEnabled(cBStreamC3->isChecked());
    btnReceiverOptions1->setEnabled(cBStreamC1->isChecked());
    btnReceiverOptions2->setEnabled(cBStreamC2->isChecked());
    btnReceiverOptions3->setEnabled(cBStreamC3->isChecked());

    lblNmea->setEnabled(enaNmea);
    cBNmeaRequestType->setEnabled(enaNmea);
    sBNmeaPosition1->setEnabled(enaNmea && cBNmeaRequestType->currentIndex() == 1);
    sBNmeaPosition2->setEnabled(enaNmea && cBNmeaRequestType->currentIndex() == 1);
    sBNmeaPosition3->setEnabled(enaNmea && cBNmeaRequestType->currentIndex() == 1);
    btnPosition->setEnabled(enaNmea && cBNmeaRequestType->currentIndex() == 1);
    lblResetCmd->setEnabled(enaNmea && cBNmeaRequestType->currentIndex() == 3);
    lEResetCmd->setEnabled(enaNmea && cBNmeaRequestType->currentIndex() == 3);
    lblMaxBaseLine->setEnabled(enaNmea && cBNmeaRequestType->currentIndex() == 3);
    sBMaxBaseLine->setEnabled(enaNmea && cBNmeaRequestType->currentIndex() == 3);

    lblF1->setEnabled(enaFile);
    lEFilePath1->setEnabled(cBStreamC1->isChecked() && cBStream1->currentIndex() == 4);
    lEFilePath2->setEnabled(cBStreamC2->isChecked() && cBStream2->currentIndex() == 4);
    lEFilePath3->setEnabled(cBStreamC3->isChecked() && cBStream3->currentIndex() == 4);
    cBTimeTag->setEnabled(enaFile);
    lETimeStart->setEnabled(enaFile && cBTimeTag->isChecked());
    cBTimeSpeed->setEnabled(enaFile && cBTimeTag->isChecked());
    lblF2->setEnabled(enaFile && cBTimeTag->isChecked());
    lblF3->setEnabled(enaFile && cBTimeTag->isChecked());
    cB64Bit->setEnabled(enaFile && cBTimeTag->isChecked());
}
//---------------------------------------------------------------------------
