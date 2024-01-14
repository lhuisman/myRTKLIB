//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann
#include <stdio.h>

#include "cmdoptdlg.h"
#include "serioptdlg.h"
#include "cmdoptdlg.h"

#include <QShowEvent>
#include <QWidget>
#include <QComboBox>
#include <QSerialPortInfo>

//---------------------------------------------------------------------------
SerialOptDialog::SerialOptDialog(QWidget *parent, int options)
    : QDialog(parent)
{
    setupUi(this);

    cmdOptDialog = new CmdOptDialog(this);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &SerialOptDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SerialOptDialog::reject);
    connect(cBOutputTcpPort, &QCheckBox::clicked, this, &SerialOptDialog::updateEnable);

    updatePortList();

    updateEnable();
    setOptions(options);
}
//---------------------------------------------------------------------------
void SerialOptDialog::setOptions(int options)
{
    cBOutputTcpPort->setEnabled(options);
    cBTcpPort->setEnabled(options);
}
//---------------------------------------------------------------------------
void SerialOptDialog::setPath(QString path)
{
    QStringList tokens = path.split(':');

    cBPort->setCurrentIndex(cBPort->findText(tokens.first()));

    if (tokens.size() < 2) return;
    cBBitRate->setCurrentIndex(cBBitRate->findText(tokens.at(1)));

    if (tokens.size() < 3) return;
    cBByteSize->setCurrentIndex(tokens.at(2) == "7" ? 0 : 1);

    if (tokens.size() < 4) return;
    cBParity->setCurrentIndex(tokens.at(3) == "n" ? 0 : tokens.at(3) == "e" ? 1 : 2);

    if (tokens.size() < 5) return;
    cBStopBits->setCurrentIndex(tokens.at(4) == "1" ? 0 : 1);

    if (tokens.size() < 6) return;
    cBFlowControl->setCurrentIndex(tokens.at(5).contains("off") ? 0 : tokens.at(5).contains("rts") ? 1 : 2);

    QStringList tokens2 = tokens.at(5).split('#');

    bool okay;
    if (tokens2.size() == 2) {
        int port = tokens2.at(1).toInt(&okay);
        if (okay) {
            cBOutputTcpPort->setChecked(true);
            cBTcpPort->setValue(port);
        }
    } else
    {
        cBOutputTcpPort->setChecked(false);
        cBTcpPort->setValue(-1);
    }
}
//---------------------------------------------------------------------------
QString SerialOptDialog::getPath()
{
    const char *parity[] = {"n", "e", "o"}, *fctr[] = {"off", "rts", "xon"};
    QString Port_Text = cBPort->currentText(), BitRate_Text = cBBitRate->currentText().split(" ").first();
    QString path;

    path = QString("%1:%2:%3:%4:%5:%6").arg(Port_Text).arg(BitRate_Text)
           .arg(cBByteSize->currentIndex() ? 8 : 7).arg(parity[cBParity->currentIndex()])
           .arg(cBStopBits->currentIndex() ? 2 : 1).arg(fctr[cBFlowControl->currentIndex()]);

    if (cBOutputTcpPort->isChecked())
        path += QString("#%1").arg(cBTcpPort->value());

    return path;
}
//---------------------------------------------------------------------------
void SerialOptDialog::updatePortList(void)
{
    cBPort->clear();
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    for (int i = 0; i < ports.size(); i++)
        cBPort->addItem(ports.at(i).portName());
}
//---------------------------------------------------------------------------
void SerialOptDialog::updateEnable(void)
{
    cBTcpPort->setEnabled(cBOutputTcpPort->isChecked());
}

