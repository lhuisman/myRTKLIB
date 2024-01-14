//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include <QShowEvent>
#include <QUrl>
#include <QIntValidator>

#include "ftpoptdlg.h"
#include "keydlg.h"

//---------------------------------------------------------------------------
FtpOptDialog::FtpOptDialog(QWidget *parent, int options)
    : QDialog(parent)
{
    setupUi(this);

    keyDlg = new KeyDialog(this);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &FtpOptDialog::saveClose);
    connect(btnKey, &QPushButton::clicked, this, &FtpOptDialog::showKeyDialog);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FtpOptDialog::reject);


    cBPathOffset->setValidator(new QIntValidator(this));
    cBInterval->setValidator(new QIntValidator(this));
    cBOffset->setValidator(new QIntValidator(this));

    setOptions(options);
}
//---------------------------------------------------------------------------
void FtpOptDialog::setOptions(int options)
{
    QString cap[] = { tr("FTP Option"), tr("HTTP Option") };
    this->options = options;

    setWindowTitle(cap[options]);

    updateEnable();
}
//---------------------------------------------------------------------------
void FtpOptDialog::setPath(QString path)
{
    int topts[4] = { 0, 3600, 0, 0 };

    QStringList tokens = path.split("::");
    if (tokens.size() > 1) {
        QString t = tokens.at(1);
        if (t.startsWith("T=")) {
            QStringList values = t.mid(2).split(",");  // remove "T=" and split list:
            for (int i = 0; (i < 4) || (i < values.size()); i++)
                topts[i] = values.at(i).toInt();
        }
    }
    QUrl url(QString("ftp://") + path);

    cBAddress->clear();
    cBAddress->addItem(url.host() + url.path());
    for (int i = 0; i < MAXHIST; i++)
        if (history[i] != "") cBAddress->addItem(history[i]);
    ;

    cBAddress->setCurrentIndex(0);
    lEUser->setText(url.userName());
    lEPassword->setText(url.password());
    cBPathOffset->insertItem(0, QString("%1 h").arg(topts[0] / 3600.0, 0, 'g', 2)); cBPathOffset->setCurrentIndex(0);
    cBInterval->insertItem(0, QString("%1 h").arg(topts[1] / 3600.0, 0, 'g', 2)); cBInterval->setCurrentIndex(0);
    cBOffset->insertItem(0, QString("%1 h").arg(topts[2] / 3600.0, 0, 'g', 2)); cBOffset->setCurrentIndex(0);
    sBRetryInterval->setValue(topts[3]);
}
//---------------------------------------------------------------------------
QString FtpOptDialog::getPath()
{
    QString pathOffset_Text = cBPathOffset->currentText().split(" ").first();
    QString interval_Text = cBInterval->currentText().split(" ").first();
    QString offset_Text = cBOffset->currentText().split(" ").first();
    QString user_Text = lEUser->text();
    QString password_Text = lEPassword->text();
    QString address_Text = cBAddress->currentText();
    int topts[4];
    bool ok;

    topts[0] = pathOffset_Text.toInt(&ok) * 3600.0;
    topts[1] = interval_Text.toInt(&ok) * 3600.0;
    topts[2] = offset_Text.toInt(&ok) * 3600.0;
    topts[3] = sBRetryInterval->value();

    return QString("%1:%2@%3::T=%4,%5,%6,%7").arg(user_Text)
           .arg(password_Text).arg(address_Text)
           .arg(topts[0]).arg(topts[1]).arg(topts[2]).arg(topts[3]);
}
//---------------------------------------------------------------------------
void FtpOptDialog::saveClose()
{
    addHistory(cBAddress, history);

    accept();
}
//---------------------------------------------------------------------------
void FtpOptDialog::showKeyDialog()
{
    keyDlg->exec();
}
//---------------------------------------------------------------------------
void FtpOptDialog::addHistory(QComboBox *list, QString *hist)
{
    for (int i = 0; i < MAXHIST; i++) {
        if (list->currentText() != hist[i]) continue;
        for (int j = i + 1; j < MAXHIST; j++) hist[j - 1] = hist[j];
        hist[MAXHIST - 1] = "";
	}
    for (int i = MAXHIST - 1; i > 0; i--) hist[i] = hist[i - 1];
    hist[0] = list->currentText();

    list->clear();
    for (int i = 0; i < MAXHIST; i++)
        if (hist[i] != "") list->addItem(hist[i]);
}
//---------------------------------------------------------------------------
void FtpOptDialog::updateEnable(void)
{
    lEUser->setEnabled(options == 0);
    lEPassword->setEnabled(options == 0);
    lbUser->setEnabled(options == 0);
    lbPassword->setEnabled(options == 0);
}
