//---------------------------------------------------------------------------

#include <QShowEvent>
#include <QDateTime>

#include "rtklib.h"
#include "plotmain.h"
#include "tspandlg.h"
#include "timedlg.h"

//---------------------------------------------------------------------------
SpanDialog::SpanDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    for (int i = 0; i < 3; i++) {
        timeEnabled[i] = true;
        timeValid[i] = true;
	}

    connect(cBTimeEndEnabled, &QCheckBox::clicked, this, &SpanDialog::updateEnable);
    connect(cBTimeIntervalEnabled, &QCheckBox::clicked, this, &SpanDialog::updateEnable);
    connect(cBTimeStartEnabled, &QCheckBox::clicked, this, &SpanDialog::updateEnable);
    connect(btnOk, &QPushButton::clicked, this, &SpanDialog::btnOkClicked);
    connect(btnCancel, &QPushButton::clicked, this, &SpanDialog::reject);
    connect(btnTimeStart, &QPushButton::clicked, this, &SpanDialog::btnTimeStartClicked);
    connect(btnTimeEnd, &QPushButton::clicked, this, &SpanDialog::btnTimeEndClicked);
}
//---------------------------------------------------------------------------
void SpanDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    cBTimeStartEnabled->setChecked(timeEnabled[0]);
    cBTimeEndEnabled->setChecked(timeEnabled[1]);
    cBTimeIntervalEnabled->setChecked(timeEnabled[2]);

    QDateTime start = QDateTime::fromSecsSinceEpoch(timeStart.time); start = start.addMSecs(timeStart.sec * 1000);
    QDateTime end = QDateTime::fromSecsSinceEpoch(timeEnd.time); start = start.addMSecs(timeEnd.sec * 1000);

    dTTimeStart->setTime(start.time());
    dTTimeStart->setDate(start.date());
    dTTimeEnd->setTime(end.time());
    dTTimeEnd->setDate(end.date());

    cBTimeInterval->setCurrentText(QString::number(timeInterval));

    updateEnable();
}
//---------------------------------------------------------------------------
void SpanDialog::btnOkClicked()
{
    timeEnabled[0] = cBTimeStartEnabled->isChecked();
    timeEnabled[1] = cBTimeEndEnabled->isChecked();
    timeEnabled[2] = cBTimeIntervalEnabled->isChecked();

    QDateTime start(dTTimeStart->dateTime());
    QDateTime end(dTTimeEnd->dateTime());

    timeStart.time = start.toSecsSinceEpoch(); timeStart.sec = start.time().msec() / 1000;
    timeEnd.time = end.toSecsSinceEpoch(); timeEnd.sec = end.time().msec() / 1000;
    timeInterval = cBTimeInterval->currentText().toDouble();

    accept();
}
//---------------------------------------------------------------------------
void SpanDialog::updateEnable(void)
{
    dTTimeStart->setEnabled(cBTimeStartEnabled->isChecked() && timeValid[0]);
    dTTimeEnd->setEnabled(cBTimeEndEnabled->isChecked() && timeValid[1]);
    cBTimeInterval->setEnabled(cBTimeIntervalEnabled->isChecked() && timeValid[2]);
    cBTimeStartEnabled->setEnabled(timeValid[0] == 1);
    cBTimeEndEnabled->setEnabled(timeValid[1] == 1);
    cBTimeIntervalEnabled->setEnabled(timeValid[2] == 1);
}
//---------------------------------------------------------------------------
void SpanDialog::btnTimeStartClicked(void)
{
    TimeDialog * timeDialog = new TimeDialog(this);

    timeDialog->time.time = dTTimeStart->dateTime().toSecsSinceEpoch();
    timeDialog->time.sec = dTTimeStart->dateTime().time().msec() / 1000;
    timeDialog->exec();
}
//---------------------------------------------------------------------------
void SpanDialog::btnTimeEndClicked(void)
{
    TimeDialog * timeDialog = new TimeDialog(this);

    timeDialog->time.time = dTTimeEnd->dateTime().toSecsSinceEpoch();
    timeDialog->time.sec = dTTimeEnd->dateTime().time().msec() / 1000;
    timeDialog->exec();
}
//---------------------------------------------------------------------------
