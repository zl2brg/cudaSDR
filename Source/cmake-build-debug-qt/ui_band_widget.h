/********************************************************************************
** Form generated from reading UI file 'band_widget.ui'
**
** Created by: Qt User Interface Compiler version 5.13.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BAND_WIDGET_H
#define UI_BAND_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <Util/cusdr_buttons.h>

QT_BEGIN_NAMESPACE

class Ui_BandWidget
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox_3;
    QGridLayout *gridLayout;
    AeroButton *bnd_70cm;
    AeroButton *bnd_20m;
    AeroButton *bnd_40m;
    AeroButton *bnd_630m;
    AeroButton *bnd_gen;
    AeroButton *bnd_17m;
    AeroButton *bnd_2m;
    AeroButton *bnd_12m;
    AeroButton *bnd_10m;
    AeroButton *bnd_30m;
    AeroButton *bnd_23cm;
    AeroButton *bnd_6m;
    AeroButton *bnd_15m;
    AeroButton *bnd_33cm;
    AeroButton *bnd_60m;
    AeroButton *bnd_125cm;
    AeroButton *bnd_80m;
    AeroButton *bnd_160m;
    AeroButton *bnd_2200m;
    AeroButton *bnd_13cm;
    AeroButton *bnd_10cm;
    AeroButton *bnd_5cm;

    void setupUi(QWidget *BandWidget)
    {
        if (BandWidget->objectName().isEmpty())
            BandWidget->setObjectName(QString::fromUtf8("BandWidget"));
        BandWidget->resize(509, 230);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(BandWidget->sizePolicy().hasHeightForWidth());
        BandWidget->setSizePolicy(sizePolicy);
        verticalLayout = new QVBoxLayout(BandWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBox_3 = new QGroupBox(BandWidget);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
        groupBox_3->setSizePolicy(sizePolicy1);
        groupBox_3->setFlat(true);
        gridLayout = new QGridLayout(groupBox_3);
        gridLayout->setSpacing(1);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(1, 1, 1, 1);
        bnd_70cm = new AeroButton(groupBox_3);
        bnd_70cm->setObjectName(QString::fromUtf8("bnd_70cm"));

        gridLayout->addWidget(bnd_70cm, 2, 3, 1, 1);

        bnd_20m = new AeroButton(groupBox_3);
        bnd_20m->setObjectName(QString::fromUtf8("bnd_20m"));

        gridLayout->addWidget(bnd_20m, 1, 1, 1, 1);

        bnd_40m = new AeroButton(groupBox_3);
        bnd_40m->setObjectName(QString::fromUtf8("bnd_40m"));

        gridLayout->addWidget(bnd_40m, 0, 5, 1, 1);

        bnd_630m = new AeroButton(groupBox_3);
        bnd_630m->setObjectName(QString::fromUtf8("bnd_630m"));

        gridLayout->addWidget(bnd_630m, 0, 1, 1, 1);

        bnd_gen = new AeroButton(groupBox_3);
        bnd_gen->setObjectName(QString::fromUtf8("bnd_gen"));

        gridLayout->addWidget(bnd_gen, 4, 0, 1, 6);

        bnd_17m = new AeroButton(groupBox_3);
        bnd_17m->setObjectName(QString::fromUtf8("bnd_17m"));

        gridLayout->addWidget(bnd_17m, 1, 2, 1, 1);

        bnd_2m = new AeroButton(groupBox_3);
        bnd_2m->setObjectName(QString::fromUtf8("bnd_2m"));

        gridLayout->addWidget(bnd_2m, 2, 1, 1, 1);

        bnd_12m = new AeroButton(groupBox_3);
        bnd_12m->setObjectName(QString::fromUtf8("bnd_12m"));

        gridLayout->addWidget(bnd_12m, 1, 4, 1, 1);

        bnd_10m = new AeroButton(groupBox_3);
        bnd_10m->setObjectName(QString::fromUtf8("bnd_10m"));

        gridLayout->addWidget(bnd_10m, 1, 5, 1, 1);

        bnd_30m = new AeroButton(groupBox_3);
        bnd_30m->setObjectName(QString::fromUtf8("bnd_30m"));

        gridLayout->addWidget(bnd_30m, 1, 0, 1, 1);

        bnd_23cm = new AeroButton(groupBox_3);
        bnd_23cm->setObjectName(QString::fromUtf8("bnd_23cm"));

        gridLayout->addWidget(bnd_23cm, 2, 5, 1, 1);

        bnd_6m = new AeroButton(groupBox_3);
        bnd_6m->setObjectName(QString::fromUtf8("bnd_6m"));

        gridLayout->addWidget(bnd_6m, 2, 0, 1, 1);

        bnd_15m = new AeroButton(groupBox_3);
        bnd_15m->setObjectName(QString::fromUtf8("bnd_15m"));

        gridLayout->addWidget(bnd_15m, 1, 3, 1, 1);

        bnd_33cm = new AeroButton(groupBox_3);
        bnd_33cm->setObjectName(QString::fromUtf8("bnd_33cm"));

        gridLayout->addWidget(bnd_33cm, 2, 4, 1, 1);

        bnd_60m = new AeroButton(groupBox_3);
        bnd_60m->setObjectName(QString::fromUtf8("bnd_60m"));

        gridLayout->addWidget(bnd_60m, 0, 4, 1, 1);

        bnd_125cm = new AeroButton(groupBox_3);
        bnd_125cm->setObjectName(QString::fromUtf8("bnd_125cm"));

        gridLayout->addWidget(bnd_125cm, 2, 2, 1, 1);

        bnd_80m = new AeroButton(groupBox_3);
        bnd_80m->setObjectName(QString::fromUtf8("bnd_80m"));

        gridLayout->addWidget(bnd_80m, 0, 3, 1, 1);

        bnd_160m = new AeroButton(groupBox_3);
        bnd_160m->setObjectName(QString::fromUtf8("bnd_160m"));

        gridLayout->addWidget(bnd_160m, 0, 2, 1, 1);

        bnd_2200m = new AeroButton(groupBox_3);
        bnd_2200m->setObjectName(QString::fromUtf8("bnd_2200m"));

        gridLayout->addWidget(bnd_2200m, 0, 0, 1, 1);

        bnd_13cm = new AeroButton(groupBox_3);
        bnd_13cm->setObjectName(QString::fromUtf8("bnd_13cm"));

        gridLayout->addWidget(bnd_13cm, 3, 0, 1, 1);

        bnd_10cm = new AeroButton(groupBox_3);
        bnd_10cm->setObjectName(QString::fromUtf8("bnd_10cm"));

        gridLayout->addWidget(bnd_10cm, 3, 1, 1, 1);

        bnd_5cm = new AeroButton(groupBox_3);
        bnd_5cm->setObjectName(QString::fromUtf8("bnd_5cm"));

        gridLayout->addWidget(bnd_5cm, 3, 2, 1, 1);


        verticalLayout->addWidget(groupBox_3);


        retranslateUi(BandWidget);

        QMetaObject::connectSlotsByName(BandWidget);
    } // setupUi

    void retranslateUi(QWidget *BandWidget)
    {
        BandWidget->setWindowTitle(QCoreApplication::translate("BandWidget", "Band", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("BandWidget", "Band", nullptr));
        bnd_70cm->setText(QCoreApplication::translate("BandWidget", "70 cm", nullptr));
        bnd_20m->setText(QCoreApplication::translate("BandWidget", "20 m", nullptr));
        bnd_40m->setText(QCoreApplication::translate("BandWidget", "40 m", nullptr));
        bnd_630m->setText(QCoreApplication::translate("BandWidget", "630 m", nullptr));
        bnd_gen->setText(QCoreApplication::translate("BandWidget", "Gen", nullptr));
        bnd_17m->setText(QCoreApplication::translate("BandWidget", "17 m", nullptr));
        bnd_2m->setText(QCoreApplication::translate("BandWidget", "2 m", nullptr));
        bnd_12m->setText(QCoreApplication::translate("BandWidget", "12 m", nullptr));
        bnd_10m->setText(QCoreApplication::translate("BandWidget", "10 m", nullptr));
        bnd_30m->setText(QCoreApplication::translate("BandWidget", "30 m", nullptr));
        bnd_23cm->setText(QCoreApplication::translate("BandWidget", "23 cm", nullptr));
        bnd_6m->setText(QCoreApplication::translate("BandWidget", "6 m", nullptr));
        bnd_15m->setText(QCoreApplication::translate("BandWidget", "15 m", nullptr));
        bnd_33cm->setText(QCoreApplication::translate("BandWidget", "33 cm", nullptr));
        bnd_60m->setText(QCoreApplication::translate("BandWidget", "60 m", nullptr));
        bnd_125cm->setText(QCoreApplication::translate("BandWidget", "125 cm", nullptr));
        bnd_80m->setText(QCoreApplication::translate("BandWidget", "80 m", nullptr));
        bnd_160m->setText(QCoreApplication::translate("BandWidget", "160 m", nullptr));
        bnd_2200m->setText(QCoreApplication::translate("BandWidget", "2200 m", nullptr));
        bnd_13cm->setText(QCoreApplication::translate("BandWidget", "13 cm", nullptr));
        bnd_10cm->setText(QCoreApplication::translate("BandWidget", "10 cm", nullptr));
        bnd_5cm->setText(QCoreApplication::translate("BandWidget", "5 cm", nullptr));
    } // retranslateUi

};

namespace Ui {
    class BandWidget: public Ui_BandWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BAND_WIDGET_H
