//---------------------------------------------------------------------------

#include <QColorDialog>
#include <QShowEvent>

#define INHIBIT_RTK_LOCK_MACROS

#include "vmapdlg.h"
#include "plotmain.h"

extern Plot *plot;

//---------------------------------------------------------------------------
QString color2String(const QColor &c);

//---------------------------------------------------------------------------
VecMapDialog::VecMapDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(btnColor1, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor2, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor3, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor4, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor5, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor6, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor7, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor8, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor9, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor10, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor11, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor12, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor1F, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor2F, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor3F, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor4F, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor5F, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor6F, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor7F, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor8F, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor9F, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor10F, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor11F, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(btnColor12F, &QPushButton::clicked, this, &VecMapDialog::btnColorClicked);
    connect(cBVisible1, &QCheckBox::clicked, this, &VecMapDialog::visibilityClicked);
    connect(cBVisible2, &QCheckBox::clicked, this, &VecMapDialog::visibilityClicked);
    connect(cBVisible3, &QCheckBox::clicked, this, &VecMapDialog::visibilityClicked);
    connect(cBVisible4, &QCheckBox::clicked, this, &VecMapDialog::visibilityClicked);
    connect(cBVisible5, &QCheckBox::clicked, this, &VecMapDialog::visibilityClicked);
    connect(cBVisible6, &QCheckBox::clicked, this, &VecMapDialog::visibilityClicked);
    connect(cBVisible7, &QCheckBox::clicked, this, &VecMapDialog::visibilityClicked);
    connect(cBVisible8, &QCheckBox::clicked, this, &VecMapDialog::visibilityClicked);
    connect(cBVisible9, &QCheckBox::clicked, this, &VecMapDialog::visibilityClicked);
    connect(cBVisible10, &QCheckBox::clicked, this, &VecMapDialog::visibilityClicked);
    connect(cBVisible11, &QCheckBox::clicked, this, &VecMapDialog::visibilityClicked);
    connect(cBVisible12, &QCheckBox::clicked, this, &VecMapDialog::visibilityClicked);
    connect(rBLayer1, &QRadioButton::clicked, this, &VecMapDialog::updateMap);
    connect(rBLayer2, &QRadioButton::clicked, this, &VecMapDialog::updateMap);
    connect(rBLayer3, &QRadioButton::clicked, this, &VecMapDialog::updateMap);
    connect(rBLayer4, &QRadioButton::clicked, this, &VecMapDialog::updateMap);
    connect(rBLayer5, &QRadioButton::clicked, this, &VecMapDialog::updateMap);
    connect(rBLayer6, &QRadioButton::clicked, this, &VecMapDialog::updateMap);
    connect(rBLayer7, &QRadioButton::clicked, this, &VecMapDialog::updateMap);
    connect(rBLayer8, &QRadioButton::clicked, this, &VecMapDialog::updateMap);
    connect(rBLayer9, &QRadioButton::clicked, this, &VecMapDialog::updateMap);
    connect(rBLayer10, &QRadioButton::clicked, this, &VecMapDialog::updateMap);
    connect(rBLayer11, &QRadioButton::clicked, this, &VecMapDialog::updateMap);
    connect(rBLayer12, &QRadioButton::clicked, this, &VecMapDialog::updateMap);
    connect(btnApply, &QPushButton::clicked, this, &VecMapDialog::btnApplyClicked);
    connect(btnUp, &QPushButton::clicked, this, &VecMapDialog::btnUpClicked);
    connect(btnDown, &QPushButton::clicked, this, &VecMapDialog::btnDownClicked);
    connect(btnClose, &QPushButton::clicked, this, &VecMapDialog::reject);
}
//---------------------------------------------------------------------------
void VecMapDialog::btnColorClicked()
{
    QPushButton *color[] = {
        btnColor1, btnColor2, btnColor3, btnColor4, btnColor5, btnColor6, btnColor7, btnColor8, btnColor9,
        btnColor10, btnColor11, btnColor12, btnColor1F, btnColor2F, btnColor3F, btnColor4F, btnColor5F,
        btnColor6F, btnColor7F, btnColor8F, btnColor9F, btnColor10F, btnColor11F, btnColor12F, NULL
    };
    int i;

    for (i = 0; color[i]; i++) {
        if (color[i] == sender()) break;
    }
    if (color[i]) {
        QColorDialog dialog(this);

        dialog.setCurrentColor(colors[i]);

        if (dialog.exec() != QDialog::Accepted) return;

        color[i]->setStyleSheet(QString("QPushButton {background-color: %1;}").arg(color2String(dialog.currentColor())));
        colors[i] = dialog.currentColor();

        updateMap();
    }
}
//---------------------------------------------------------------------------
void VecMapDialog::btnUpClicked()
{
    QRadioButton *rBLayer[] = {
        rBLayer1, rBLayer2,  rBLayer3, rBLayer4, rBLayer5, rBLayer6, rBLayer7, rBLayer8, rBLayer9,
        rBLayer10, rBLayer11, rBLayer12
	};
    QPushButton *color[]={
               btnColor1, btnColor2, btnColor3, btnColor4, btnColor5, btnColor6, btnColor7, btnColor8, btnColor9,
               btnColor10, btnColor11, btnColor12
    };
    QString style;
    QPushButton *colorf[]={
        btnColor1F, btnColor2F, btnColor3F, btnColor4F, btnColor5F, btnColor6F, btnColor7F, btnColor8F,
        btnColor9F, btnColor10F, btnColor11F, btnColor12F
    };
	gisd_t *data;
	char name[256];
    int layer, flag;

    for (layer = 0; layer < MAXMAPLAYER; layer++)
        if (rBLayer[layer]->isChecked()) break;
    if (layer == 0 || layer >= MAXMAPLAYER) return;

    strncpy(name, gis.name[layer - 1], 255);
    strncpy(gis.name[layer - 1], gis.name[layer], 255);
    strncpy(gis.name[layer], name, 255);

    flag = gis.flag[layer - 1];
    gis.flag[layer - 1] = gis.flag[layer];
    gis.flag[layer] = flag;

    data = gis.data[layer - 1];
    gis.data[layer - 1] = gis.data[layer];
    gis.data[layer] = data;

    style = color[layer-1]->styleSheet();
    color[layer-1]->setStyleSheet(color[layer]->styleSheet());
    color[layer]->setStyleSheet(style);

    style = colorf[layer-1]->styleSheet();
    colorf[layer-1]->setStyleSheet(colorf[layer]->styleSheet());
    colorf[layer]->setStyleSheet(style);

    rBLayer[layer - 1]->setChecked(true);
    rBLayer[layer]->setChecked(false);

    updateMap();
}
//---------------------------------------------------------------------------
void VecMapDialog::btnDownClicked()
{
    QRadioButton *rBLayer[] = {
        rBLayer1, rBLayer2, rBLayer3, rBLayer4, rBLayer5, rBLayer6, rBLayer7, rBLayer8, rBLayer9,
        rBLayer10, rBLayer11, rBLayer12
	};
    QPushButton *color[]={
        btnColor1, btnColor2, btnColor3, btnColor4, btnColor5, btnColor6, btnColor7, btnColor8, btnColor9,
        btnColor10, btnColor11, btnColor12
    };
    QString style;
    QPushButton *colorf[]={
        btnColor1F, btnColor2F, btnColor3F, btnColor4F, btnColor5F, btnColor6F, btnColor7F, btnColor8F,
        btnColor9F, btnColor10F, btnColor11F, btnColor12F
    };
	gisd_t *data;
	char name[256];
    int layer, flag;

    for (layer = 0; layer < MAXMAPLAYER; layer++)
        if (rBLayer[layer]->isChecked()) break;
    if (layer == MAXMAPLAYER - 1 || layer >= MAXMAPLAYER) return;

    strncpy(name, gis.name[layer + 1], 255);
    strncpy(gis.name[layer + 1], gis.name[layer], 255);
    strncpy(gis.name[layer], name, 255);

    flag = gis.flag[layer + 1];
    gis.flag[layer + 1] = gis.flag[layer];
    gis.flag[layer] = flag;

    data = gis.data[layer + 1];
    gis.data[layer + 1] = gis.data[layer];
    gis.data[layer] = data;

    style = color[layer+1]->styleSheet();
    color[layer+1]->setStyleSheet(color[layer]->styleSheet());
    color[layer]->setStyleSheet(style);

    style = colorf[layer+1]->styleSheet();
    colorf[layer+1]->setStyleSheet(colorf[layer]->styleSheet());
    colorf[layer]->setStyleSheet(style);

    rBLayer[layer + 1]->setChecked(true);
    rBLayer[layer]->setChecked(false);

    updateMap();
}
//---------------------------------------------------------------------------
void VecMapDialog::visibilityClicked()
{
    QCheckBox *vis[] = {
        cBVisible1, cBVisible2, cBVisible3, cBVisible4, cBVisible5, cBVisible6, cBVisible7, cBVisible8, cBVisible9, cBVisible10, cBVisible11, cBVisible12
	};

    for (int i = 0; i < MAXMAPLAYER; i++)
        if (qobject_cast<QCheckBox *>(sender()) == vis[i]) gis.flag[i] = vis[i]->isChecked() ? 1 : 0;

    updateMap();
}
//---------------------------------------------------------------------------
void VecMapDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    QRadioButton *layer[] = {
        rBLayer1, rBLayer2, rBLayer3, rBLayer4, rBLayer5, rBLayer6, rBLayer7, rBLayer8, rBLayer9,
        rBLayer10, rBLayer11, rBLayer12
	};
    QPushButton *color[] = {
        btnColor1, btnColor2, btnColor3, btnColor4, btnColor5, btnColor6, btnColor7, btnColor8, btnColor9,
        btnColor10, btnColor11, btnColor12
	};
    QPushButton *colorf[]={
        btnColor1F, btnColor2F, btnColor3F, btnColor4F, btnColor5F, btnColor6F, btnColor7F, btnColor8F,
        btnColor9F, btnColor10F, btnColor11F, btnColor12F
    };

    gis = plot->gis;
    for (int i = 0; i < MAXMAPLAYER; i++) {
        layer[i]->setChecked(false);
        color[i]->setStyleSheet(QString("QPushButton {background-color: %1;}").arg(color2String(plot->mapColor[i])));
        colorf[i]->setStyleSheet(QString("QPushButton {background-color: %1;}").arg(color2String(plot->mapColor[i])));
	}

    updateMap();
}
//---------------------------------------------------------------------------
void VecMapDialog::btnApplyClicked()
{
    updateMap();

    accept();
}
//---------------------------------------------------------------------------
void VecMapDialog::updateMap(void)
{
    QRadioButton *layer[] = {
        rBLayer1, rBLayer2, rBLayer3, rBLayer4, rBLayer5, rBLayer6, rBLayer7, rBLayer8, rBLayer9,
        rBLayer10, rBLayer11, rBLayer12
	};
    QCheckBox *vis[] = {
        cBVisible1, cBVisible2, cBVisible3, cBVisible4, cBVisible5, cBVisible6, cBVisible7, cBVisible8, cBVisible9, cBVisible10, cBVisible11, cBVisible12
	};

    for (int i = 0; i < MAXMAPLAYER; i++) {
        layer[i]->setText(gis.name[i]);
        vis[i]->setChecked(gis.flag[i]);
        plot->mapColor[i] = colors[i];
        plot->mapColorF[i] = colors[i+12];
    }
    plot->gis = gis;
}
//---------------------------------------------------------------------------
