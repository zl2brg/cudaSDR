/********************************************************************************
** Form generated from reading UI file 'tx_settings_dialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TX_SETTINGS_DIALOG_H
#define UI_TX_SETTINGS_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_tx_settings_dialog
{
public:
    QVBoxLayout *verticalLayout_5;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_4;
    QComboBox *audiodevlist;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QSlider *audioCompression;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout;
    QLabel *label_3;
    QSlider *amCarrierLevel;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    QCheckBox *repeaterMode;
    QCheckBox *fmPre;
    QLabel *label;
    QSpinBox *repeater_offset;
    QLabel *label_5;
    QSpinBox *fm_deviation;
    QLabel *label_4;
    QSpinBox *ctcss_tone;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout_2;
    QLabel *label_13;
    QLabel *label_7;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *internal_keyer;
    QCheckBox *keyer_spacing;
    QCheckBox *keyer_reverse;
    QSlider *weight;
    QLabel *label_8;
    QLabel *label_6;
    QLabel *label_14;
    QLabel *label_12;
    QLabel *label_15;
    QSpinBox *sidetone_freq;
    QSpinBox *keyer_speed;
    QComboBox *KeyerMode;
    QSpinBox *sidetone_volume;
    QSpinBox *ptt_delay;
    QSpinBox *cw_hangtime;
    QListView *listView;

    void setupUi(QWidget *tx_settings_dialog)
    {
        if (tx_settings_dialog->objectName().isEmpty())
            tx_settings_dialog->setObjectName("tx_settings_dialog");
        tx_settings_dialog->resize(280, 702);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(tx_settings_dialog->sizePolicy().hasHeightForWidth());
        tx_settings_dialog->setSizePolicy(sizePolicy);
        tx_settings_dialog->setMaximumSize(QSize(16777215, 747));
        QFont font;
        font.setPointSize(10);
        tx_settings_dialog->setFont(font);
        verticalLayout_5 = new QVBoxLayout(tx_settings_dialog);
        verticalLayout_5->setObjectName("verticalLayout_5");
        groupBox = new QGroupBox(tx_settings_dialog);
        groupBox->setObjectName("groupBox");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy1);
        groupBox->setFlat(false);
        verticalLayout_4 = new QVBoxLayout(groupBox);
        verticalLayout_4->setSpacing(9);
        verticalLayout_4->setObjectName("verticalLayout_4");
        audiodevlist = new QComboBox(groupBox);
        audiodevlist->setObjectName("audiodevlist");

        verticalLayout_4->addWidget(audiodevlist);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        label_2 = new QLabel(groupBox);
        label_2->setObjectName("label_2");

        horizontalLayout_2->addWidget(label_2);

        audioCompression = new QSlider(groupBox);
        audioCompression->setObjectName("audioCompression");
        audioCompression->setEnabled(true);
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(audioCompression->sizePolicy().hasHeightForWidth());
        audioCompression->setSizePolicy(sizePolicy2);
        audioCompression->setMinimumSize(QSize(100, 0));
        audioCompression->setMaximum(20);
        audioCompression->setPageStep(5);
        audioCompression->setOrientation(Qt::Horizontal);

        horizontalLayout_2->addWidget(audioCompression);


        verticalLayout_4->addLayout(horizontalLayout_2);


        verticalLayout_5->addWidget(groupBox);

        groupBox_3 = new QGroupBox(tx_settings_dialog);
        groupBox_3->setObjectName("groupBox_3");
        sizePolicy.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
        groupBox_3->setSizePolicy(sizePolicy);
        groupBox_3->setMaximumSize(QSize(16777215, 65));
        groupBox_3->setLayoutDirection(Qt::LeftToRight);
        verticalLayout_3 = new QVBoxLayout(groupBox_3);
        verticalLayout_3->setObjectName("verticalLayout_3");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(2);
        horizontalLayout->setObjectName("horizontalLayout");
        label_3 = new QLabel(groupBox_3);
        label_3->setObjectName("label_3");

        horizontalLayout->addWidget(label_3);

        amCarrierLevel = new QSlider(groupBox_3);
        amCarrierLevel->setObjectName("amCarrierLevel");
        QSizePolicy sizePolicy3(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(amCarrierLevel->sizePolicy().hasHeightForWidth());
        amCarrierLevel->setSizePolicy(sizePolicy3);
        amCarrierLevel->setMinimumSize(QSize(100, 0));
        amCarrierLevel->setSingleStep(5);
        amCarrierLevel->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(amCarrierLevel);


        verticalLayout_3->addLayout(horizontalLayout);


        verticalLayout_5->addWidget(groupBox_3);

        groupBox_2 = new QGroupBox(tx_settings_dialog);
        groupBox_2->setObjectName("groupBox_2");
        groupBox_2->setAutoFillBackground(true);
        groupBox_2->setFlat(false);
        gridLayout = new QGridLayout(groupBox_2);
        gridLayout->setObjectName("gridLayout");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        repeaterMode = new QCheckBox(groupBox_2);
        repeaterMode->setObjectName("repeaterMode");
        repeaterMode->setFont(font);
        repeaterMode->setLayoutDirection(Qt::LeftToRight);
        repeaterMode->setStyleSheet(QString::fromUtf8(""));

        verticalLayout->addWidget(repeaterMode);

        fmPre = new QCheckBox(groupBox_2);
        fmPre->setObjectName("fmPre");
        fmPre->setLayoutDirection(Qt::LeftToRight);

        verticalLayout->addWidget(fmPre);


        gridLayout->addLayout(verticalLayout, 0, 0, 1, 2);

        label = new QLabel(groupBox_2);
        label->setObjectName("label");

        gridLayout->addWidget(label, 1, 0, 1, 1);

        repeater_offset = new QSpinBox(groupBox_2);
        repeater_offset->setObjectName("repeater_offset");
        repeater_offset->setMaximumSize(QSize(80, 16777215));
        repeater_offset->setMaximum(1);
        repeater_offset->setSingleStep(100);

        gridLayout->addWidget(repeater_offset, 1, 1, 1, 1);

        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName("label_5");
        label_5->setContextMenuPolicy(Qt::NoContextMenu);

        gridLayout->addWidget(label_5, 2, 0, 1, 1);

        fm_deviation = new QSpinBox(groupBox_2);
        fm_deviation->setObjectName("fm_deviation");
        fm_deviation->setMaximumSize(QSize(80, 16777215));

        gridLayout->addWidget(fm_deviation, 2, 1, 1, 1);

        label_4 = new QLabel(groupBox_2);
        label_4->setObjectName("label_4");

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        ctcss_tone = new QSpinBox(groupBox_2);
        ctcss_tone->setObjectName("ctcss_tone");
        ctcss_tone->setMaximumSize(QSize(80, 16777215));
        ctcss_tone->setMaximum(1000);
        ctcss_tone->setSingleStep(100);

        gridLayout->addWidget(ctcss_tone, 3, 1, 1, 1);


        verticalLayout_5->addWidget(groupBox_2);

        groupBox_4 = new QGroupBox(tx_settings_dialog);
        groupBox_4->setObjectName("groupBox_4");
        groupBox_4->setFlat(false);
        gridLayout_2 = new QGridLayout(groupBox_4);
        gridLayout_2->setObjectName("gridLayout_2");
        label_13 = new QLabel(groupBox_4);
        label_13->setObjectName("label_13");

        gridLayout_2->addWidget(label_13, 6, 0, 1, 1);

        label_7 = new QLabel(groupBox_4);
        label_7->setObjectName("label_7");

        gridLayout_2->addWidget(label_7, 2, 0, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        internal_keyer = new QCheckBox(groupBox_4);
        internal_keyer->setObjectName("internal_keyer");
        internal_keyer->setLayoutDirection(Qt::LeftToRight);
        internal_keyer->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_3->addWidget(internal_keyer);

        keyer_spacing = new QCheckBox(groupBox_4);
        keyer_spacing->setObjectName("keyer_spacing");

        horizontalLayout_3->addWidget(keyer_spacing);

        keyer_reverse = new QCheckBox(groupBox_4);
        keyer_reverse->setObjectName("keyer_reverse");
        keyer_reverse->setLayoutDirection(Qt::LeftToRight);

        horizontalLayout_3->addWidget(keyer_reverse);


        gridLayout_2->addLayout(horizontalLayout_3, 0, 0, 1, 2);

        weight = new QSlider(groupBox_4);
        weight->setObjectName("weight");
        weight->setOrientation(Qt::Horizontal);
        weight->setInvertedAppearance(false);
        weight->setInvertedControls(false);
        weight->setTickPosition(QSlider::TicksAbove);
        weight->setTickInterval(5);

        gridLayout_2->addWidget(weight, 8, 1, 1, 1);

        label_8 = new QLabel(groupBox_4);
        label_8->setObjectName("label_8");

        gridLayout_2->addWidget(label_8, 3, 0, 1, 1);

        label_6 = new QLabel(groupBox_4);
        label_6->setObjectName("label_6");
        label_6->setContextMenuPolicy(Qt::NoContextMenu);

        gridLayout_2->addWidget(label_6, 4, 0, 1, 1);

        label_14 = new QLabel(groupBox_4);
        label_14->setObjectName("label_14");

        gridLayout_2->addWidget(label_14, 7, 0, 1, 1);

        label_12 = new QLabel(groupBox_4);
        label_12->setObjectName("label_12");

        gridLayout_2->addWidget(label_12, 5, 0, 1, 1);

        label_15 = new QLabel(groupBox_4);
        label_15->setObjectName("label_15");

        gridLayout_2->addWidget(label_15, 8, 0, 1, 1);

        sidetone_freq = new QSpinBox(groupBox_4);
        sidetone_freq->setObjectName("sidetone_freq");
        sidetone_freq->setMinimum(500);
        sidetone_freq->setMaximum(4000);
        sidetone_freq->setSingleStep(100);

        gridLayout_2->addWidget(sidetone_freq, 5, 1, 1, 1);

        keyer_speed = new QSpinBox(groupBox_4);
        keyer_speed->setObjectName("keyer_speed");
        keyer_speed->setMinimum(10);
        keyer_speed->setMaximum(50);
        keyer_speed->setSingleStep(5);

        gridLayout_2->addWidget(keyer_speed, 3, 1, 1, 1);

        KeyerMode = new QComboBox(groupBox_4);
        KeyerMode->addItem(QString());
        KeyerMode->addItem(QString());
        KeyerMode->addItem(QString());
        KeyerMode->setObjectName("KeyerMode");
        QSizePolicy sizePolicy4(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(KeyerMode->sizePolicy().hasHeightForWidth());
        KeyerMode->setSizePolicy(sizePolicy4);
        KeyerMode->setMaximumSize(QSize(80, 16777215));

        gridLayout_2->addWidget(KeyerMode, 2, 1, 1, 1);

        sidetone_volume = new QSpinBox(groupBox_4);
        sidetone_volume->setObjectName("sidetone_volume");
        sidetone_volume->setMaximum(100);
        sidetone_volume->setSingleStep(5);
        sidetone_volume->setValue(5);

        gridLayout_2->addWidget(sidetone_volume, 6, 1, 1, 1);

        ptt_delay = new QSpinBox(groupBox_4);
        ptt_delay->setObjectName("ptt_delay");
        ptt_delay->setMaximum(1000);
        ptt_delay->setSingleStep(100);

        gridLayout_2->addWidget(ptt_delay, 7, 1, 1, 1);

        cw_hangtime = new QSpinBox(groupBox_4);
        cw_hangtime->setObjectName("cw_hangtime");

        gridLayout_2->addWidget(cw_hangtime, 4, 1, 1, 1);

        listView = new QListView(groupBox_4);
        listView->setObjectName("listView");

        gridLayout_2->addWidget(listView, 1, 1, 1, 1);


        verticalLayout_5->addWidget(groupBox_4);


        retranslateUi(tx_settings_dialog);

        QMetaObject::connectSlotsByName(tx_settings_dialog);
    } // setupUi

    void retranslateUi(QWidget *tx_settings_dialog)
    {
        tx_settings_dialog->setWindowTitle(QCoreApplication::translate("tx_settings_dialog", "Dialog", nullptr));
        groupBox->setTitle(QCoreApplication::translate("tx_settings_dialog", "Mic Input", nullptr));
        label_2->setText(QCoreApplication::translate("tx_settings_dialog", "Compression (db)", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("tx_settings_dialog", "AM", nullptr));
        label_3->setText(QCoreApplication::translate("tx_settings_dialog", "Carrier Level %", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("tx_settings_dialog", "FM", nullptr));
        repeaterMode->setText(QCoreApplication::translate("tx_settings_dialog", "Repeater Mode", nullptr));
        fmPre->setText(QCoreApplication::translate("tx_settings_dialog", "FM Pre-emphasize", nullptr));
        label->setText(QCoreApplication::translate("tx_settings_dialog", "RepeaterOffset (Khz)", nullptr));
        label_5->setText(QCoreApplication::translate("tx_settings_dialog", "FM Deviation (Khz)", nullptr));
        label_4->setText(QCoreApplication::translate("tx_settings_dialog", "CTCSS Enable (Hz)", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("tx_settings_dialog", "CW", nullptr));
        label_13->setText(QCoreApplication::translate("tx_settings_dialog", "Sidetone Volume", nullptr));
        label_7->setText(QCoreApplication::translate("tx_settings_dialog", "Keyer Mode", nullptr));
        internal_keyer->setText(QCoreApplication::translate("tx_settings_dialog", "Internal", nullptr));
        keyer_spacing->setText(QCoreApplication::translate("tx_settings_dialog", "Spacing", nullptr));
        keyer_reverse->setText(QCoreApplication::translate("tx_settings_dialog", "Reverse ", nullptr));
        label_8->setText(QCoreApplication::translate("tx_settings_dialog", "Keyer Speed", nullptr));
        label_6->setText(QCoreApplication::translate("tx_settings_dialog", "Hang Time", nullptr));
        label_14->setText(QCoreApplication::translate("tx_settings_dialog", "PTT Delay", nullptr));
        label_12->setText(QCoreApplication::translate("tx_settings_dialog", "Sidetone Frequency", nullptr));
        label_15->setText(QCoreApplication::translate("tx_settings_dialog", "Weight", nullptr));
        KeyerMode->setItemText(0, QCoreApplication::translate("tx_settings_dialog", "Straight", nullptr));
        KeyerMode->setItemText(1, QCoreApplication::translate("tx_settings_dialog", "Mode A", nullptr));
        KeyerMode->setItemText(2, QCoreApplication::translate("tx_settings_dialog", "Mode B", nullptr));

    } // retranslateUi

};

namespace Ui {
    class tx_settings_dialog: public Ui_tx_settings_dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TX_SETTINGS_DIALOG_H
