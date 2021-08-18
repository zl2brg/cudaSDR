/********************************************************************************
** Form generated from reading UI file 'radio_ctrl.ui'
**
** Created by: Qt User Interface Compiler version 5.15.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RADIO_CTRL_H
#define UI_RADIO_CTRL_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <Util/cusdr_buttons.h>

QT_BEGIN_NAMESPACE

class Ui_RadioCtrl
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout;
    AeroButton *bnd_125cm;
    AeroButton *bnd_33cm;
    AeroButton *bnd_gen;
    AeroButton *bnd_17m;
    AeroButton *bnd_10m;
    AeroButton *bnd_80m;
    AeroButton *bnd_70cm;
    AeroButton *bnd_6m;
    AeroButton *bnd_12m;
    AeroButton *bnd_15m;
    AeroButton *bnd_20m;
    AeroButton *bnd_40m;
    AeroButton *bnd_630m;
    AeroButton *bnd_5cm;
    AeroButton *bnd_2m;
    AeroButton *bnd_160m;
    AeroButton *bnd_60m;
    AeroButton *bnd_23cm;
    AeroButton *bnd_30m;
    AeroButton *bnd_13cm;
    AeroButton *bnd_2200m;
    AeroButton *bnd_10cm;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_3;
    AeroButton *mode_usb;
    AeroButton *mode_lsb;
    AeroButton *mode_cwu;
    AeroButton *mode_digu;
    AeroButton *mode_spec;
    AeroButton *mode_digl;
    AeroButton *mode_fm;
    AeroButton *mode_dsb;
    AeroButton *mode_cwl;
    AeroButton *mode_am;
    AeroButton *mode_sam;
    AeroButton *mode_freedv;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout_2;
    AeroButton *filter_6;
    AeroButton *filter_5;
    AeroButton *filter_9;
    AeroButton *filter_var1;
    AeroButton *filter_1;
    AeroButton *filter_3;
    AeroButton *filter_4;
    AeroButton *filter_8;
    AeroButton *filter_7;
    AeroButton *filter_10;
    AeroButton *filter_2;
    AeroButton *filter_var2;
    QSlider *Var2_Slider;
    QSlider *Var1_Slider;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *RadioCtrl)
    {
        if (RadioCtrl->objectName().isEmpty())
            RadioCtrl->setObjectName(QString::fromUtf8("RadioCtrl"));
        RadioCtrl->resize(509, 573);
        verticalLayout = new QVBoxLayout(RadioCtrl);
        verticalLayout->setSpacing(2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(2, 2, 1, 2);
        groupBox_4 = new QGroupBox(RadioCtrl);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBox_4->sizePolicy().hasHeightForWidth());
        groupBox_4->setSizePolicy(sizePolicy);
        groupBox_4->setFlat(true);
        gridLayout = new QGridLayout(groupBox_4);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setHorizontalSpacing(2);
        gridLayout->setVerticalSpacing(5);
        gridLayout->setContentsMargins(2, 5, 2, 2);
        bnd_125cm = new AeroButton(groupBox_4);
        bnd_125cm->setObjectName(QString::fromUtf8("bnd_125cm"));

        gridLayout->addWidget(bnd_125cm, 3, 2, 1, 1);

        bnd_33cm = new AeroButton(groupBox_4);
        bnd_33cm->setObjectName(QString::fromUtf8("bnd_33cm"));

        gridLayout->addWidget(bnd_33cm, 3, 4, 1, 1);

        bnd_gen = new AeroButton(groupBox_4);
        bnd_gen->setObjectName(QString::fromUtf8("bnd_gen"));

        gridLayout->addWidget(bnd_gen, 4, 3, 1, 3);

        bnd_17m = new AeroButton(groupBox_4);
        bnd_17m->setObjectName(QString::fromUtf8("bnd_17m"));

        gridLayout->addWidget(bnd_17m, 2, 2, 1, 1);

        bnd_10m = new AeroButton(groupBox_4);
        bnd_10m->setObjectName(QString::fromUtf8("bnd_10m"));

        gridLayout->addWidget(bnd_10m, 2, 5, 1, 1);

        bnd_80m = new AeroButton(groupBox_4);
        bnd_80m->setObjectName(QString::fromUtf8("bnd_80m"));

        gridLayout->addWidget(bnd_80m, 1, 3, 1, 1);

        bnd_70cm = new AeroButton(groupBox_4);
        bnd_70cm->setObjectName(QString::fromUtf8("bnd_70cm"));

        gridLayout->addWidget(bnd_70cm, 3, 3, 1, 1);

        bnd_6m = new AeroButton(groupBox_4);
        bnd_6m->setObjectName(QString::fromUtf8("bnd_6m"));

        gridLayout->addWidget(bnd_6m, 3, 0, 1, 1);

        bnd_12m = new AeroButton(groupBox_4);
        bnd_12m->setObjectName(QString::fromUtf8("bnd_12m"));

        gridLayout->addWidget(bnd_12m, 2, 4, 1, 1);

        bnd_15m = new AeroButton(groupBox_4);
        bnd_15m->setObjectName(QString::fromUtf8("bnd_15m"));

        gridLayout->addWidget(bnd_15m, 2, 3, 1, 1);

        bnd_20m = new AeroButton(groupBox_4);
        bnd_20m->setObjectName(QString::fromUtf8("bnd_20m"));

        gridLayout->addWidget(bnd_20m, 2, 1, 1, 1);

        bnd_40m = new AeroButton(groupBox_4);
        bnd_40m->setObjectName(QString::fromUtf8("bnd_40m"));

        gridLayout->addWidget(bnd_40m, 1, 5, 1, 1);

        bnd_630m = new AeroButton(groupBox_4);
        bnd_630m->setObjectName(QString::fromUtf8("bnd_630m"));

        gridLayout->addWidget(bnd_630m, 1, 1, 1, 1);

        bnd_5cm = new AeroButton(groupBox_4);
        bnd_5cm->setObjectName(QString::fromUtf8("bnd_5cm"));

        gridLayout->addWidget(bnd_5cm, 4, 2, 1, 1);

        bnd_2m = new AeroButton(groupBox_4);
        bnd_2m->setObjectName(QString::fromUtf8("bnd_2m"));

        gridLayout->addWidget(bnd_2m, 3, 1, 1, 1);

        bnd_160m = new AeroButton(groupBox_4);
        bnd_160m->setObjectName(QString::fromUtf8("bnd_160m"));

        gridLayout->addWidget(bnd_160m, 1, 2, 1, 1);

        bnd_60m = new AeroButton(groupBox_4);
        bnd_60m->setObjectName(QString::fromUtf8("bnd_60m"));

        gridLayout->addWidget(bnd_60m, 1, 4, 1, 1);

        bnd_23cm = new AeroButton(groupBox_4);
        bnd_23cm->setObjectName(QString::fromUtf8("bnd_23cm"));

        gridLayout->addWidget(bnd_23cm, 3, 5, 1, 1);

        bnd_30m = new AeroButton(groupBox_4);
        bnd_30m->setObjectName(QString::fromUtf8("bnd_30m"));

        gridLayout->addWidget(bnd_30m, 2, 0, 1, 1);

        bnd_13cm = new AeroButton(groupBox_4);
        bnd_13cm->setObjectName(QString::fromUtf8("bnd_13cm"));

        gridLayout->addWidget(bnd_13cm, 4, 0, 1, 1);

        bnd_2200m = new AeroButton(groupBox_4);
        bnd_2200m->setObjectName(QString::fromUtf8("bnd_2200m"));

        gridLayout->addWidget(bnd_2200m, 1, 0, 1, 1);

        bnd_10cm = new AeroButton(groupBox_4);
        bnd_10cm->setObjectName(QString::fromUtf8("bnd_10cm"));

        gridLayout->addWidget(bnd_10cm, 4, 1, 1, 1);


        verticalLayout->addWidget(groupBox_4);

        groupBox_2 = new QGroupBox(RadioCtrl);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        gridLayout_3 = new QGridLayout(groupBox_2);
        gridLayout_3->setSpacing(2);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        gridLayout_3->setContentsMargins(2, 5, 2, 2);
        mode_usb = new AeroButton(groupBox_2);
        mode_usb->setObjectName(QString::fromUtf8("mode_usb"));

        gridLayout_3->addWidget(mode_usb, 0, 1, 1, 1);

        mode_lsb = new AeroButton(groupBox_2);
        mode_lsb->setObjectName(QString::fromUtf8("mode_lsb"));

        gridLayout_3->addWidget(mode_lsb, 0, 0, 1, 1);

        mode_cwu = new AeroButton(groupBox_2);
        mode_cwu->setObjectName(QString::fromUtf8("mode_cwu"));

        gridLayout_3->addWidget(mode_cwu, 0, 4, 1, 1);

        mode_digu = new AeroButton(groupBox_2);
        mode_digu->setObjectName(QString::fromUtf8("mode_digu"));

        gridLayout_3->addWidget(mode_digu, 1, 2, 1, 1);

        mode_spec = new AeroButton(groupBox_2);
        mode_spec->setObjectName(QString::fromUtf8("mode_spec"));

        gridLayout_3->addWidget(mode_spec, 1, 4, 1, 1);

        mode_digl = new AeroButton(groupBox_2);
        mode_digl->setObjectName(QString::fromUtf8("mode_digl"));

        gridLayout_3->addWidget(mode_digl, 1, 3, 1, 1);

        mode_fm = new AeroButton(groupBox_2);
        mode_fm->setObjectName(QString::fromUtf8("mode_fm"));

        gridLayout_3->addWidget(mode_fm, 1, 1, 1, 1);

        mode_dsb = new AeroButton(groupBox_2);
        mode_dsb->setObjectName(QString::fromUtf8("mode_dsb"));

        gridLayout_3->addWidget(mode_dsb, 0, 2, 1, 1);

        mode_cwl = new AeroButton(groupBox_2);
        mode_cwl->setObjectName(QString::fromUtf8("mode_cwl"));

        gridLayout_3->addWidget(mode_cwl, 0, 3, 1, 1);

        mode_am = new AeroButton(groupBox_2);
        mode_am->setObjectName(QString::fromUtf8("mode_am"));

        gridLayout_3->addWidget(mode_am, 1, 0, 1, 1);

        mode_sam = new AeroButton(groupBox_2);
        mode_sam->setObjectName(QString::fromUtf8("mode_sam"));

        gridLayout_3->addWidget(mode_sam, 2, 0, 1, 1);

        mode_freedv = new AeroButton(groupBox_2);
        mode_freedv->setObjectName(QString::fromUtf8("mode_freedv"));

        gridLayout_3->addWidget(mode_freedv, 2, 1, 1, 1);


        verticalLayout->addWidget(groupBox_2);

        groupBox = new QGroupBox(RadioCtrl);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setFlat(true);
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setSpacing(2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(2, 5, 2, 2);
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        filter_6 = new AeroButton(groupBox);
        filter_6->setObjectName(QString::fromUtf8("filter_6"));

        gridLayout_2->addWidget(filter_6, 1, 0, 1, 1);

        filter_5 = new AeroButton(groupBox);
        filter_5->setObjectName(QString::fromUtf8("filter_5"));

        gridLayout_2->addWidget(filter_5, 0, 4, 1, 1);

        filter_9 = new AeroButton(groupBox);
        filter_9->setObjectName(QString::fromUtf8("filter_9"));

        gridLayout_2->addWidget(filter_9, 1, 3, 1, 1);

        filter_var1 = new AeroButton(groupBox);
        filter_var1->setObjectName(QString::fromUtf8("filter_var1"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(filter_var1->sizePolicy().hasHeightForWidth());
        filter_var1->setSizePolicy(sizePolicy1);

        gridLayout_2->addWidget(filter_var1, 2, 0, 1, 1);

        filter_1 = new AeroButton(groupBox);
        filter_1->setObjectName(QString::fromUtf8("filter_1"));

        gridLayout_2->addWidget(filter_1, 0, 0, 1, 1);

        filter_3 = new AeroButton(groupBox);
        filter_3->setObjectName(QString::fromUtf8("filter_3"));

        gridLayout_2->addWidget(filter_3, 0, 2, 1, 1);

        filter_4 = new AeroButton(groupBox);
        filter_4->setObjectName(QString::fromUtf8("filter_4"));

        gridLayout_2->addWidget(filter_4, 0, 3, 1, 1);

        filter_8 = new AeroButton(groupBox);
        filter_8->setObjectName(QString::fromUtf8("filter_8"));

        gridLayout_2->addWidget(filter_8, 1, 2, 1, 1);

        filter_7 = new AeroButton(groupBox);
        filter_7->setObjectName(QString::fromUtf8("filter_7"));

        gridLayout_2->addWidget(filter_7, 1, 1, 1, 1);

        filter_10 = new AeroButton(groupBox);
        filter_10->setObjectName(QString::fromUtf8("filter_10"));

        gridLayout_2->addWidget(filter_10, 1, 4, 1, 1);

        filter_2 = new AeroButton(groupBox);
        filter_2->setObjectName(QString::fromUtf8("filter_2"));

        gridLayout_2->addWidget(filter_2, 0, 1, 1, 1);

        filter_var2 = new AeroButton(groupBox);
        filter_var2->setObjectName(QString::fromUtf8("filter_var2"));

        gridLayout_2->addWidget(filter_var2, 3, 0, 1, 1);

        Var2_Slider = new QSlider(groupBox);
        Var2_Slider->setObjectName(QString::fromUtf8("Var2_Slider"));
        Var2_Slider->setOrientation(Qt::Horizontal);

        gridLayout_2->addWidget(Var2_Slider, 3, 1, 1, 3);

        Var1_Slider = new QSlider(groupBox);
        Var1_Slider->setObjectName(QString::fromUtf8("Var1_Slider"));
        Var1_Slider->setOrientation(Qt::Horizontal);

        gridLayout_2->addWidget(Var1_Slider, 2, 1, 1, 3);


        verticalLayout_2->addLayout(gridLayout_2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);


        verticalLayout->addWidget(groupBox);


        retranslateUi(RadioCtrl);

        QMetaObject::connectSlotsByName(RadioCtrl);
    } // setupUi

    void retranslateUi(QWidget *RadioCtrl)
    {
        RadioCtrl->setWindowTitle(QCoreApplication::translate("RadioCtrl", "Dialog", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("RadioCtrl", "Band", nullptr));
        bnd_125cm->setText(QCoreApplication::translate("RadioCtrl", "125 cm", nullptr));
        bnd_33cm->setText(QCoreApplication::translate("RadioCtrl", "33 cm", nullptr));
        bnd_gen->setText(QCoreApplication::translate("RadioCtrl", "Gen", nullptr));
        bnd_17m->setText(QCoreApplication::translate("RadioCtrl", "17 m", nullptr));
        bnd_10m->setText(QCoreApplication::translate("RadioCtrl", "10 m", nullptr));
        bnd_80m->setText(QCoreApplication::translate("RadioCtrl", "80 m", nullptr));
        bnd_70cm->setText(QCoreApplication::translate("RadioCtrl", "70 cm", nullptr));
        bnd_6m->setText(QCoreApplication::translate("RadioCtrl", "6 m", nullptr));
        bnd_12m->setText(QCoreApplication::translate("RadioCtrl", "12 m", nullptr));
        bnd_15m->setText(QCoreApplication::translate("RadioCtrl", "15 m", nullptr));
        bnd_20m->setText(QCoreApplication::translate("RadioCtrl", "20 m", nullptr));
        bnd_40m->setText(QCoreApplication::translate("RadioCtrl", "40 m", nullptr));
        bnd_630m->setText(QCoreApplication::translate("RadioCtrl", "630 m", nullptr));
        bnd_5cm->setText(QCoreApplication::translate("RadioCtrl", "5 cm", nullptr));
        bnd_2m->setText(QCoreApplication::translate("RadioCtrl", "2 m", nullptr));
        bnd_160m->setText(QCoreApplication::translate("RadioCtrl", "160 m", nullptr));
        bnd_60m->setText(QCoreApplication::translate("RadioCtrl", "60 m", nullptr));
        bnd_23cm->setText(QCoreApplication::translate("RadioCtrl", "23 cm", nullptr));
        bnd_30m->setText(QCoreApplication::translate("RadioCtrl", "30 m", nullptr));
        bnd_13cm->setText(QCoreApplication::translate("RadioCtrl", "13 cm", nullptr));
        bnd_2200m->setText(QCoreApplication::translate("RadioCtrl", "2200 m", nullptr));
        bnd_10cm->setText(QCoreApplication::translate("RadioCtrl", "10 cm", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("RadioCtrl", "Mode", nullptr));
        mode_usb->setText(QCoreApplication::translate("RadioCtrl", "USB", nullptr));
        mode_lsb->setText(QCoreApplication::translate("RadioCtrl", "LSB", nullptr));
        mode_cwu->setText(QCoreApplication::translate("RadioCtrl", "CWH", nullptr));
        mode_digu->setText(QCoreApplication::translate("RadioCtrl", "DIGU", nullptr));
        mode_spec->setText(QCoreApplication::translate("RadioCtrl", "SPEC", nullptr));
        mode_digl->setText(QCoreApplication::translate("RadioCtrl", "DIGL", nullptr));
        mode_fm->setText(QCoreApplication::translate("RadioCtrl", "FM", nullptr));
        mode_dsb->setText(QCoreApplication::translate("RadioCtrl", "DSB", nullptr));
        mode_cwl->setText(QCoreApplication::translate("RadioCtrl", "CWL", nullptr));
        mode_am->setText(QCoreApplication::translate("RadioCtrl", "AM", nullptr));
        mode_sam->setText(QCoreApplication::translate("RadioCtrl", "SAM", nullptr));
        mode_freedv->setText(QCoreApplication::translate("RadioCtrl", "FDV", nullptr));
        groupBox->setTitle(QCoreApplication::translate("RadioCtrl", "Filter", nullptr));
        filter_6->setText(QCoreApplication::translate("RadioCtrl", "2k7", nullptr));
        filter_5->setText(QCoreApplication::translate("RadioCtrl", "3k1", nullptr));
        filter_9->setText(QCoreApplication::translate("RadioCtrl", "1k8", nullptr));
        filter_var1->setText(QCoreApplication::translate("RadioCtrl", "Var1", nullptr));
        filter_1->setText(QCoreApplication::translate("RadioCtrl", "5k", nullptr));
        filter_3->setText(QCoreApplication::translate("RadioCtrl", "3k8", nullptr));
        filter_4->setText(QCoreApplication::translate("RadioCtrl", "3k3", nullptr));
        filter_8->setText(QCoreApplication::translate("RadioCtrl", "2k1", nullptr));
        filter_7->setText(QCoreApplication::translate("RadioCtrl", "2k4", nullptr));
        filter_10->setText(QCoreApplication::translate("RadioCtrl", "1k", nullptr));
        filter_2->setText(QCoreApplication::translate("RadioCtrl", "4k4", nullptr));
        filter_var2->setText(QCoreApplication::translate("RadioCtrl", "Var2", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RadioCtrl: public Ui_RadioCtrl {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RADIO_CTRL_H
