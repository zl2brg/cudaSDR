/********************************************************************************
** Form generated from reading UI file 'audio_dialog.ui'
**
** Created by: Qt User Interface Compiler version 5.13.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AUDIO_DIALOG_H
#define UI_AUDIO_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <Util/cusdr_buttons.h>

QT_BEGIN_NAMESPACE

class Ui_audio_dialog
{
public:
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_3;
    QComboBox *audiodevlist;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout;
    QSlider *inputLevelSlider;
    QGroupBox *groupBox_3;
    QHBoxLayout *horizontalLayout_2;
    QSlider *drivelevelSlider;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer;
    AeroButton *OkButton;
    AeroButton *CancelButton;

    void setupUi(QDialog *audio_dialog)
    {
        if (audio_dialog->objectName().isEmpty())
            audio_dialog->setObjectName(QString::fromUtf8("audio_dialog"));
        audio_dialog->resize(227, 364);
        verticalLayout_2 = new QVBoxLayout(audio_dialog);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupBox = new QGroupBox(audio_dialog);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        horizontalLayout_3 = new QHBoxLayout(groupBox);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        audiodevlist = new QComboBox(groupBox);
        audiodevlist->setObjectName(QString::fromUtf8("audiodevlist"));

        horizontalLayout_3->addWidget(audiodevlist);


        verticalLayout_2->addWidget(groupBox);

        groupBox_2 = new QGroupBox(audio_dialog);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        horizontalLayout = new QHBoxLayout(groupBox_2);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        inputLevelSlider = new QSlider(groupBox_2);
        inputLevelSlider->setObjectName(QString::fromUtf8("inputLevelSlider"));
        inputLevelSlider->setMaximum(100);
        inputLevelSlider->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(inputLevelSlider);


        verticalLayout_2->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(audio_dialog);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        horizontalLayout_2 = new QHBoxLayout(groupBox_3);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        drivelevelSlider = new QSlider(groupBox_3);
        drivelevelSlider->setObjectName(QString::fromUtf8("drivelevelSlider"));
        drivelevelSlider->setMaximum(100);
        drivelevelSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_2->addWidget(drivelevelSlider);


        verticalLayout_2->addWidget(groupBox_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalSpacer = new QSpacerItem(118, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer);

        OkButton = new AeroButton(audio_dialog);
        OkButton->setObjectName(QString::fromUtf8("OkButton"));

        horizontalLayout_4->addWidget(OkButton);

        CancelButton = new AeroButton(audio_dialog);
        CancelButton->setObjectName(QString::fromUtf8("CancelButton"));

        horizontalLayout_4->addWidget(CancelButton);


        verticalLayout_2->addLayout(horizontalLayout_4);


        retranslateUi(audio_dialog);

        QMetaObject::connectSlotsByName(audio_dialog);
    } // setupUi

    void retranslateUi(QDialog *audio_dialog)
    {
        audio_dialog->setWindowTitle(QCoreApplication::translate("audio_dialog", "Dialog", nullptr));
        groupBox->setTitle(QCoreApplication::translate("audio_dialog", "Audio Input", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("audio_dialog", "Input Level", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("audio_dialog", "Drive Level", nullptr));
        OkButton->setText(QCoreApplication::translate("audio_dialog", "Ok", nullptr));
        CancelButton->setText(QCoreApplication::translate("audio_dialog", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class audio_dialog: public Ui_audio_dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUDIO_DIALOG_H
