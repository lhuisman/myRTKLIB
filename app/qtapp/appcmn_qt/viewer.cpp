//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann
#include <QShowEvent>
#include <QFileDialog>
#include <QPalette>
#include <QDebug>

#include "rtklib.h"
#include "viewer.h"
#include "vieweropt.h"

QColor TextViewer::colorText = Qt::black, TextViewer::colorBackground = Qt::white;
QFont TextViewer::font;

//---------------------------------------------------------------------------
TextViewer::TextViewer(QWidget *parent, int option)
    : QDialog(parent)
{
    setupUi(this);

    viewerOptDialog = new ViewerOptDialog(this);

    connect(btnClose, &QPushButton::clicked, this, &TextViewer::accept);
    connect(btnFind, &QPushButton::clicked, this, &TextViewer::findText);
    connect(btnOptions, &QPushButton::clicked, this, &TextViewer::showOptions);
    connect(btnReadSave, &QPushButton::clicked, this, &TextViewer::readSaveFile);
    connect(btnReload, &QPushButton::clicked, this, &TextViewer::reloadText);
    connect(findStr, &QLineEdit::editingFinished, this, &TextViewer::findText);

    setOption(option);
}
//---------------------------------------------------------------------------
void TextViewer::setOption(int option)
{
    if (option == 0) {
        btnReload->setVisible(false);
        btnReadSave->setVisible(false);
    } else if (option == 2) {
        btnReload->setVisible(false);
        btnReadSave->setText(tr("Save..."));
    }
}
//---------------------------------------------------------------------------
void TextViewer::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

	updateText();
}
//---------------------------------------------------------------------------
void TextViewer::reloadText()
{
	read(file);
}
//---------------------------------------------------------------------------
void TextViewer::readSaveFile()
{
    if (btnReadSave->text() == tr("Save..."))
        save(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, QString(), file)));
    else
        read(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, QString(), file)));
}
//---------------------------------------------------------------------------
void TextViewer::showOptions()
{
    viewerOptDialog->setFont(font);
    viewerOptDialog->setTextColor(colorText);
    viewerOptDialog->setBackgroundColor(colorBackground);

    viewerOptDialog->move(this->size().width() / 2 - viewerOptDialog->size().width() / 2,
                          this->size().height() / 2 - viewerOptDialog->size().height() / 2);
    viewerOptDialog->exec();

    if (viewerOptDialog->result() != QDialog::Accepted) return;

    font = viewerOptDialog->getFont();
    colorText = viewerOptDialog->getTextColor();
    colorBackground = viewerOptDialog->getBackgroundColor();

	updateText();
}
//---------------------------------------------------------------------------
void TextViewer::findText()
{
    textEdit->find(findStr->text());
}
//---------------------------------------------------------------------------
bool TextViewer::read(const QString &path)
{
    char filename[1024], *p[] = { filename };

    if (expath(qPrintable(path), p, 1) < 1) return false;

    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) return false;
    textEdit->setPlainText("");

    QString TextStr = f.readAll();
    textEdit->appendPlainText(TextStr);

    setWindowTitle(filename);
    file = filename;

    return true;
}
//---------------------------------------------------------------------------
bool TextViewer::save(const QString &filename)
{
    QFile f(filename);

    if (!f.open(QIODevice::WriteOnly)) return false;

    f.write(textEdit->toPlainText().toLocal8Bit());
    file = filename;

    return true;
}
//---------------------------------------------------------------------------
void TextViewer::updateText()
{
    QPalette pal;

    textEdit->setFont(font);
    pal = textEdit->palette();
    pal.setColor(QPalette::Text, colorText);
    pal.setColor(QPalette::Base, colorBackground);
    textEdit->setPalette(pal);
}
//---------------------------------------------------------------------------
