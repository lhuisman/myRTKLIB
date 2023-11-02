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
#include "fileoptdlg.h"
#include "ftpoptdlg.h"
#include "rcvoptdlg.h"

#include "instrdlg.h"

//---------------------------------------------------------------------------
InputStrDialog::InputStrDialog(QWidget *parent)
    : QDialog(parent)
{
    int i;

    setupUi(this);

    cBFormat1->clear();
    cBFormat2->clear();
    cBFormat3->clear();

    NReceivers = 0;

    for (i = 0; i <= MAXRCVFMT; i++) {
        cBFormat1->addItem(formatstrs[i]);
        cBFormat2->addItem(formatstrs[i]);
        cBFormat3->addItem(formatstrs[i]);
		NReceivers++;
	}
    cBFormat3->addItem(tr("SP3"));

    cmdOptDialog = new CmdOptDialog(this);
    rcvOptDialog = new RcvOptDialog(this);
    refDialog = new RefDialog(this);
    serialOptDialog = new SerialOptDialog(this);;
    tcpOptDialog = new TcpOptDialog(this);
    ftpOptDialog = new FtpOptDialog(this);

    QCompleter *fileCompleter = new QCompleter(this);
    QFileSystemModel *fileModel = new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    lEFilePath1->setCompleter(fileCompleter);
    lEFilePath2->setCompleter(fileCompleter);
    lEFilePath3->setCompleter(fileCompleter);

    connect(cBStream1, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(cBStream2, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(cBStream3, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(cBNmeaReqL, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(btnOkClicked()));
    connect(btnCmd1, SIGNAL(clicked(bool)), this, SLOT(btnCmd1Clicked()));
    connect(btnCmd2, SIGNAL(clicked(bool)), this, SLOT(btnCmd2Clicked()));
    connect(btnCmd3, SIGNAL(clicked(bool)), this, SLOT(btnCmd3Clicked()));
    connect(btnFile1, SIGNAL(clicked(bool)), this, SLOT(btnFile1Clicked()));
    connect(btnFile2, SIGNAL(clicked(bool)), this, SLOT(btnFile2Clicked()));
    connect(btnFile3, SIGNAL(clicked(bool)), this, SLOT(btnFile3Clicked()));
    connect(btnPosition, SIGNAL(clicked(bool)), this, SLOT(btnPositionClicked()));
    connect(btnReceiverOptions1, SIGNAL(clicked(bool)), this, SLOT(btnReceiverOptions1Clicked()));
    connect(btnReceiverOptions2, SIGNAL(clicked(bool)), this, SLOT(btnReceiverOptions2Click()));
    connect(btnReceiverOptions3, SIGNAL(clicked(bool)), this, SLOT(btnReceiverOptions3Click()));
    connect(btnStream1, SIGNAL(clicked(bool)), this, SLOT(btnStream1Clicked()));
    connect(btnStream2, SIGNAL(clicked(bool)), this, SLOT(btnStream2Clicked()));
    connect(btnStream3, SIGNAL(clicked(bool)), this, SLOT(btnStream3Clicked()));
    connect(cBStreamC1, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(cBStreamC2, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(cBStreamC3, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(cBTimeTagC, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
}
//---------------------------------------------------------------------------
void InputStrDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    cBStreamC1->setChecked(streamC[0]);
    cBStreamC2->setChecked(streamC[1]);
    cBStreamC3->setChecked(streamC[2]);
    cBStream1->setCurrentIndex(stream[0]);
    cBStream2->setCurrentIndex(stream[1]);
    cBStream3->setCurrentIndex(stream[2]);
    cBFormat1->setCurrentIndex(format[0]);
    cBFormat2->setCurrentIndex(format[1] < NReceivers ? format[1] : NReceivers + format[1] - STRFMT_SP3);
    cBFormat3->setCurrentIndex(format[2] < NReceivers ? format[2] : NReceivers + format[2] - STRFMT_SP3);
    lEFilePath1->setText(getFilePath(paths[0][2]));
    lEFilePath2->setText(getFilePath(paths[1][2]));
    lEFilePath3->setText(getFilePath(paths[2][2]));
    cBNmeaReqL->setCurrentIndex(nmeaReq);
    cBTimeTagC->setChecked(timeTag);
    cBTimeSpeedL->setCurrentIndex(cBTimeSpeedL->findText(timeSpeed));
    lETimeStartE->setText(timeStart);
    cB64Bit->setChecked(time64Bit);
    sBNmeaPosition1->setValue(nmeaPosition[0]);
    sBNmeaPosition2->setValue(nmeaPosition[1]);
    sBNmeaPosition3->setValue(nmeaPosition[2]);
    sBMaxBaseLine->setValue(maxBaseLine);
    lEResetCmd->setText(resetCommand);

	updateEnable();
}
//---------------------------------------------------------------------------
void InputStrDialog::btnOkClicked()
{
    streamC[0] = cBStreamC1->isChecked();
    streamC[1] = cBStreamC2->isChecked();
    streamC[2] = cBStreamC3->isChecked();
    stream[0] = cBStream1->currentIndex();
    stream[1] = cBStream2->currentIndex();
    stream[2] = cBStream3->currentIndex();
    format[0] = cBFormat1->currentIndex();
    format[1] = cBFormat2->currentIndex() < NReceivers ? cBFormat2->currentIndex() : STRFMT_SP3 + cBFormat2->currentIndex() - NReceivers;
    format[2] = cBFormat3->currentIndex() < NReceivers ? cBFormat3->currentIndex() : STRFMT_SP3 + cBFormat3->currentIndex() - NReceivers;
    paths[0][2] = setFilePath(lEFilePath1->text());
    paths[1][2] = setFilePath(lEFilePath2->text());
    paths[2][2] = setFilePath(lEFilePath3->text());
    nmeaReq = cBNmeaReqL->currentIndex();
    timeTag = cBTimeTagC->isChecked();
    timeSpeed = cBTimeSpeedL->currentText();
    timeStart = lETimeStartE->text();
    time64Bit  = cB64Bit->isChecked();
    nmeaPosition[0] = sBNmeaPosition1->value();
    nmeaPosition[1] = sBNmeaPosition2->value();
    nmeaPosition[2] = sBNmeaPosition3->value();
    maxBaseLine      = sBMaxBaseLine->value();
    resetCommand   = lEResetCmd->text();

    accept();
}
//---------------------------------------------------------------------------
QString InputStrDialog::getFilePath(const QString &path)
{
    QString file;

    file = path.mid(0, path.indexOf("::"));

    return file;
}
//---------------------------------------------------------------------------
QString InputStrDialog::setFilePath(const QString &p)
{
    QString path = p;

    if (cBTimeTagC->isChecked()) path += "::T";
    if (lETimeStartE->text() != "0") path += "::+" + lETimeStartE->text();
    path += "::" + cBTimeSpeedL->currentText();
    if (cB64Bit->isChecked()) path += "::P=8";
    return path;
}
//---------------------------------------------------------------------------
void InputStrDialog::btnStream1Clicked()
{
    switch (cBStream1->currentIndex()) {
    case 0: serialOptions(0, 0); break;
    case 1: tcpOptions(0, 1); break;
    case 2: tcpOptions(0, 0); break;
    case 3: tcpOptions(0, 3); break;
	}
}
//---------------------------------------------------------------------------
void InputStrDialog::btnStream2Clicked()
{
    switch (cBStream2->currentIndex()) {
    case 0: serialOptions(1, 0); break;
    case 1: tcpOptions(1, 1); break;
    case 2: tcpOptions(1, 0); break;
    case 3: tcpOptions(1, 3); break;
    case 5: ftpOptions(1, 0); break;
    case 6: ftpOptions(1, 1); break;
	}
}
//---------------------------------------------------------------------------
void InputStrDialog::btnStream3Clicked()
{
    switch (cBStream3->currentIndex()) {
    case 0: serialOptions(2, 0); break;
    case 1: tcpOptions(2, 1); break;
    case 2: tcpOptions(2, 0); break;
    case 3: tcpOptions(2, 3); break;
    case 5: ftpOptions(2, 0); break;
    case 6: ftpOptions(2, 1); break;
	}
}
//---------------------------------------------------------------------------
void InputStrDialog::btnCmd1Clicked()
{
    for (int i = 0;i<3;i++) {
        if (cBStream1->currentIndex() == 0) {
            cmdOptDialog->commands  [i] = commands  [0][i];
            cmdOptDialog->commandsEnabled[i] = commandEnable[0][i];
        }
        else {
            cmdOptDialog->commands  [i] = commandsTcp  [0][i];
            cmdOptDialog->commandsEnabled[i] = commandEnableTcp[0][i];
        }
    }

    cmdOptDialog->exec();
    if (cmdOptDialog->result() != QDialog::Accepted) return;

    for (int i = 0; i < 3; i++) {
        if (cBStream1->currentIndex() == 0) {
            commands  [0][i] = cmdOptDialog->commands  [i];
            commandEnable[0][i] = cmdOptDialog->commandsEnabled[i];
        }
        else {
            commandsTcp  [0][i] = cmdOptDialog->commands  [i];
            commandEnableTcp[0][i] = cmdOptDialog->commandsEnabled[i];
        }
    }
}
//---------------------------------------------------------------------------
void InputStrDialog::btnCmd2Clicked()
{
    for (int i = 0;i<3;i++) {
        if (cBStream2->currentIndex() == 0) {
            cmdOptDialog->commands  [i] = commands  [1][i];
            cmdOptDialog->commandsEnabled[i] = commandEnable[1][i];
        }
        else {
            cmdOptDialog->commands  [i] = commandsTcp  [1][i];
            cmdOptDialog->commandsEnabled[i] = commandEnableTcp[1][i];
        }
    }

    cmdOptDialog->exec();
    if (cmdOptDialog->result() != QDialog::Accepted) return;

    for (int i = 0; i < 3; i++) {
        if (cBStream2->currentIndex() == 0) {
            commands  [1][i] = cmdOptDialog->commands  [i];
            commandEnable[1][i] = cmdOptDialog->commandsEnabled[i];
        }
        else {
            commandsTcp  [1][i] = cmdOptDialog->commands  [i];
            commandEnableTcp[1][i] = cmdOptDialog->commandsEnabled[i];
        }
    }}
//---------------------------------------------------------------------------
void InputStrDialog::btnCmd3Clicked()
{
    for (int i = 0;i<3;i++) {
        if (cBStream3->currentIndex() == 0) {
            cmdOptDialog->commands  [i] = commands  [2][i];
            cmdOptDialog->commandsEnabled[i] = commandEnable[2][i];
        }
        else {
            cmdOptDialog->commands  [i] = commandsTcp  [2][i];
            cmdOptDialog->commandsEnabled[i] = commandEnableTcp[2][i];
        }
    }

    cmdOptDialog->exec();
    if (cmdOptDialog->result() != QDialog::Accepted) return;

    for (int i = 0; i < 3; i++) {
        if (cBStream3->currentIndex() == 0) {
            commands  [2][i] = cmdOptDialog->commands  [i];
            commandEnable[2][i] = cmdOptDialog->commandsEnabled[i];
        }
        else {
            commandsTcp  [2][i] = cmdOptDialog->commands  [i];
            commandEnableTcp[2][i] = cmdOptDialog->commandsEnabled[i];
        }
    }}
//---------------------------------------------------------------------------
void InputStrDialog::btnReceiverOptions1Clicked()
{
    rcvOptDialog->Option = receiverOptions[0];

    rcvOptDialog->exec();
    if (rcvOptDialog->result() != QDialog::Accepted) return;

    receiverOptions[0] = rcvOptDialog->Option;
}
//---------------------------------------------------------------------------
void InputStrDialog::btnReceiverOptions2Click()
{
    rcvOptDialog->Option = receiverOptions[1];

    rcvOptDialog->exec();
    if (rcvOptDialog->result() != QDialog::Accepted) return;

    receiverOptions[1] = rcvOptDialog->Option;
}
//---------------------------------------------------------------------------
void InputStrDialog::btnReceiverOptions3Click()
{
    rcvOptDialog->Option = receiverOptions[2];

    rcvOptDialog->exec();
    if (rcvOptDialog->result() != QDialog::Accepted) return;
    receiverOptions[2] = rcvOptDialog->Option;
}
//---------------------------------------------------------------------------
void InputStrDialog::btnPositionClicked()
{
    refDialog->RoverPosition[0] = sBNmeaPosition1->value();
    refDialog->RoverPosition[1] = sBNmeaPosition2->value();
    refDialog->RoverPosition[2] = sBNmeaPosition3->value();
    refDialog->stationPositionFile = mainForm->stationPositionFileF;

    refDialog->exec();
    if (refDialog->result() != QDialog::Accepted) return;

    sBNmeaPosition1->setValue(refDialog->position[0]);
    sBNmeaPosition2->setValue(refDialog->position[1]);
    sBNmeaPosition3->setValue(refDialog->position[2]);
}
//---------------------------------------------------------------------------
void InputStrDialog::serialOptions(int index, int opt)
{
    serialOptDialog->path = paths[index][0];
    serialOptDialog->options = opt;

    serialOptDialog->exec();
    if (serialOptDialog->result() != QDialog::Accepted) return;

    paths[index][0] = serialOptDialog->path;
}
//---------------------------------------------------------------------------
void InputStrDialog::btnFile1Clicked()
{
    lEFilePath1->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open..."), lEFilePath1->text())));
}
//---------------------------------------------------------------------------
void InputStrDialog::btnFile2Clicked()
{
    lEFilePath2->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open..."), lEFilePath2->text())));
}
//---------------------------------------------------------------------------
void InputStrDialog::btnFile3Clicked()
{
    lEFilePath3->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open..."), lEFilePath3->text())));
}
//---------------------------------------------------------------------------
void InputStrDialog::tcpOptions(int index, int opt)
{
    tcpOptDialog->path = paths[index][1];
    tcpOptDialog->options = opt;
    for (int i = 0; i < 10; i++) {
        tcpOptDialog->history[i] = history[i];
	}

    tcpOptDialog->exec();
    if (tcpOptDialog->result() != QDialog::Accepted) return;

    paths[index][1] = tcpOptDialog->path;
    for (int i = 0; i < 10; i++) {
        history[i] = tcpOptDialog->history[i];
	}
}
//---------------------------------------------------------------------------
void InputStrDialog::ftpOptions(int index, int opt)
{
    ftpOptDialog->path = paths[index][3];
    ftpOptDialog->options = opt;

    ftpOptDialog->exec();
    if (ftpOptDialog->result() != QDialog::Accepted) return;

    paths[index][3] = ftpOptDialog->path;
}
//---------------------------------------------------------------------------
void InputStrDialog::updateEnable(void)
{
    int ena1 = (cBStreamC1->isChecked() && (cBStream1->currentIndex() == 4)) ||
           (cBStreamC2->isChecked() && (cBStream2->currentIndex() == 4)) ||
           (cBStreamC3->isChecked() && (cBStream3->currentIndex() == 4));
    int ena2 = cBStreamC2->isChecked() && (cBStream2->currentIndex() <= 3);

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

    lblNmea->setEnabled(ena2);
    cBNmeaReqL->setEnabled(ena2);
    sBNmeaPosition1->setEnabled(ena2 && cBNmeaReqL->currentIndex() == 1);
    sBNmeaPosition2->setEnabled(ena2 && cBNmeaReqL->currentIndex() == 1);
    sBNmeaPosition3->setEnabled(ena2 && cBNmeaReqL->currentIndex() == 1);
    btnPosition->setEnabled(ena2 && cBNmeaReqL->currentIndex() == 1);
    lblResetCmd->setEnabled(ena2 && cBNmeaReqL->currentIndex() == 3);
    lEResetCmd->setEnabled(ena2 && cBNmeaReqL->currentIndex() == 3);
    lblMaxBaseLine->setEnabled(ena2 && cBNmeaReqL->currentIndex() == 3);
    sBMaxBaseLine->setEnabled(ena2 && cBNmeaReqL->currentIndex() == 3);

    lblF1->setEnabled(ena1);
    lEFilePath1->setEnabled(cBStreamC1->isChecked() && cBStream1->currentIndex() == 4);
    lEFilePath2->setEnabled(cBStreamC2->isChecked() && cBStream2->currentIndex() == 4);
    lEFilePath3->setEnabled(cBStreamC3->isChecked() && cBStream3->currentIndex() == 4);
    btnFile1->setEnabled(cBStreamC1->isChecked() && cBStream1->currentIndex() == 4);
    btnFile2->setEnabled(cBStreamC2->isChecked() && cBStream2->currentIndex() == 4);
    btnFile3->setEnabled(cBStreamC3->isChecked() && cBStream3->currentIndex() == 4);
    cBTimeTagC->setEnabled(ena1);
    lETimeStartE->setEnabled(ena1 && cBTimeTagC->isChecked());
    cBTimeSpeedL->setEnabled(ena1 && cBTimeTagC->isChecked());
    lblF2->setEnabled(ena1 && cBTimeTagC->isChecked());
    lblF3->setEnabled(ena1 && cBTimeTagC->isChecked());
    cB64Bit->setEnabled(ena1 && cBTimeTagC->isChecked());
}
//---------------------------------------------------------------------------
