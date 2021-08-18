/********************************************************************************
** Form generated from reading UI file 'setupwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETUPWIDGET_H
#define UI_SETUPWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <Util/cusdr_buttons.h>

QT_BEGIN_NAMESPACE

class Ui_SetupWidget
{
public:
    QDialogButtonBox *buttonBox;
    QTabWidget *tabWidget;
    QWidget *general;
    QWidget *hpsdr;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout;
    AeroButton *excalabur_btn;
    AeroButton *alex_btn;
    AeroButton *mercury_btn;
    AeroButton *penelopue_btn;
    AeroButton *modules_btn;
    AeroButton *firmware_check_btn;
    AeroButton *pennylane_btn;
    AeroButton *hermes_btn;
    QLabel *label;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    AeroButton *sample_48k_btn;
    AeroButton *sample_96k_btn;
    AeroButton *sample_192k_btn;
    AeroButton *sample_384k_btn;
    QGroupBox *groupBox_3;
    QHBoxLayout *horizontalLayout_4;
    QHBoxLayout *horizontalLayout_3;
    AeroButton *hw_none_btn;
    AeroButton *hw_network_btn;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout_2;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_2;
    QComboBox *ipaddr_combo;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_3;
    AeroButton *skt_buffer_enable_btn;
    QComboBox *skt_buffer_size_combo;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_4;
    AeroButton *hpsdr_search_btn;
    QComboBox *hpsdr_ip_combo;
    QWidget *audio;

    void setupUi(QDialog *SetupWidget)
    {
        if (SetupWidget->objectName().isEmpty())
            SetupWidget->setObjectName(QString::fromUtf8("SetupWidget"));
        SetupWidget->resize(626, 611);
        buttonBox = new QDialogButtonBox(SetupWidget);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(270, 570, 341, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        tabWidget = new QTabWidget(SetupWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setGeometry(QRect(10, 10, 581, 551));
        general = new QWidget();
        general->setObjectName(QString::fromUtf8("general"));
        tabWidget->addTab(general, QString());
        hpsdr = new QWidget();
        hpsdr->setObjectName(QString::fromUtf8("hpsdr"));
        groupBox = new QGroupBox(hpsdr);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 10, 191, 181));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        excalabur_btn = new AeroButton(groupBox);
        excalabur_btn->setObjectName(QString::fromUtf8("excalabur_btn"));

        gridLayout->addWidget(excalabur_btn, 4, 1, 1, 1);

        alex_btn = new AeroButton(groupBox);
        alex_btn->setObjectName(QString::fromUtf8("alex_btn"));

        gridLayout->addWidget(alex_btn, 5, 1, 1, 1);

        mercury_btn = new AeroButton(groupBox);
        mercury_btn->setObjectName(QString::fromUtf8("mercury_btn"));

        gridLayout->addWidget(mercury_btn, 4, 0, 1, 1);

        penelopue_btn = new AeroButton(groupBox);
        penelopue_btn->setObjectName(QString::fromUtf8("penelopue_btn"));

        gridLayout->addWidget(penelopue_btn, 3, 0, 1, 1);

        modules_btn = new AeroButton(groupBox);
        modules_btn->setObjectName(QString::fromUtf8("modules_btn"));

        gridLayout->addWidget(modules_btn, 2, 0, 1, 1);

        firmware_check_btn = new AeroButton(groupBox);
        firmware_check_btn->setObjectName(QString::fromUtf8("firmware_check_btn"));

        gridLayout->addWidget(firmware_check_btn, 1, 1, 1, 1);

        pennylane_btn = new AeroButton(groupBox);
        pennylane_btn->setObjectName(QString::fromUtf8("pennylane_btn"));

        gridLayout->addWidget(pennylane_btn, 3, 1, 1, 1);

        hermes_btn = new AeroButton(groupBox);
        hermes_btn->setObjectName(QString::fromUtf8("hermes_btn"));

        gridLayout->addWidget(hermes_btn, 2, 1, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 1, 0, 1, 1);


        verticalLayout_2->addLayout(gridLayout);

        groupBox_2 = new QGroupBox(hpsdr);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(10, 340, 171, 68));
        verticalLayout = new QVBoxLayout(groupBox_2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        sample_48k_btn = new AeroButton(groupBox_2);
        sample_48k_btn->setObjectName(QString::fromUtf8("sample_48k_btn"));

        horizontalLayout_2->addWidget(sample_48k_btn);

        sample_96k_btn = new AeroButton(groupBox_2);
        sample_96k_btn->setObjectName(QString::fromUtf8("sample_96k_btn"));

        horizontalLayout_2->addWidget(sample_96k_btn);

        sample_192k_btn = new AeroButton(groupBox_2);
        sample_192k_btn->setObjectName(QString::fromUtf8("sample_192k_btn"));

        horizontalLayout_2->addWidget(sample_192k_btn);

        sample_384k_btn = new AeroButton(groupBox_2);
        sample_384k_btn->setObjectName(QString::fromUtf8("sample_384k_btn"));

        horizontalLayout_2->addWidget(sample_384k_btn);


        verticalLayout->addLayout(horizontalLayout_2);

        groupBox_3 = new QGroupBox(hpsdr);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setGeometry(QRect(10, 420, 171, 68));
        horizontalLayout_4 = new QHBoxLayout(groupBox_3);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        hw_none_btn = new AeroButton(groupBox_3);
        hw_none_btn->setObjectName(QString::fromUtf8("hw_none_btn"));

        horizontalLayout_3->addWidget(hw_none_btn);

        hw_network_btn = new AeroButton(groupBox_3);
        hw_network_btn->setObjectName(QString::fromUtf8("hw_network_btn"));

        horizontalLayout_3->addWidget(hw_network_btn);


        horizontalLayout_4->addLayout(horizontalLayout_3);

        groupBox_4 = new QGroupBox(hpsdr);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        groupBox_4->setGeometry(QRect(10, 210, 191, 121));
        gridLayout_2 = new QGridLayout(groupBox_4);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_2 = new QLabel(groupBox_4);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_5->addWidget(label_2);

        ipaddr_combo = new QComboBox(groupBox_4);
        ipaddr_combo->setObjectName(QString::fromUtf8("ipaddr_combo"));

        horizontalLayout_5->addWidget(ipaddr_combo);


        gridLayout_2->addLayout(horizontalLayout_5, 0, 0, 1, 1);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_3 = new QLabel(groupBox_4);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_6->addWidget(label_3);

        skt_buffer_enable_btn = new AeroButton(groupBox_4);
        skt_buffer_enable_btn->setObjectName(QString::fromUtf8("skt_buffer_enable_btn"));

        horizontalLayout_6->addWidget(skt_buffer_enable_btn);

        skt_buffer_size_combo = new QComboBox(groupBox_4);
        skt_buffer_size_combo->setObjectName(QString::fromUtf8("skt_buffer_size_combo"));

        horizontalLayout_6->addWidget(skt_buffer_size_combo);


        gridLayout_2->addLayout(horizontalLayout_6, 1, 0, 1, 1);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_4 = new QLabel(groupBox_4);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_7->addWidget(label_4);

        hpsdr_search_btn = new AeroButton(groupBox_4);
        hpsdr_search_btn->setObjectName(QString::fromUtf8("hpsdr_search_btn"));

        horizontalLayout_7->addWidget(hpsdr_search_btn);

        hpsdr_ip_combo = new QComboBox(groupBox_4);
        hpsdr_ip_combo->setObjectName(QString::fromUtf8("hpsdr_ip_combo"));

        horizontalLayout_7->addWidget(hpsdr_ip_combo);


        gridLayout_2->addLayout(horizontalLayout_7, 2, 0, 1, 1);

        tabWidget->addTab(hpsdr, QString());
        audio = new QWidget();
        audio->setObjectName(QString::fromUtf8("audio"));
        tabWidget->addTab(audio, QString());

        retranslateUi(SetupWidget);
        QObject::connect(buttonBox, SIGNAL(accepted()), SetupWidget, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), SetupWidget, SLOT(reject()));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(SetupWidget);
    } // setupUi

    void retranslateUi(QDialog *SetupWidget)
    {
        SetupWidget->setWindowTitle(QCoreApplication::translate("SetupWidget", "Dialog", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(general), QCoreApplication::translate("SetupWidget", "General", nullptr));
        groupBox->setTitle(QCoreApplication::translate("SetupWidget", "Hardware", nullptr));
        excalabur_btn->setText(QCoreApplication::translate("SetupWidget", "Excalabur", nullptr));
        alex_btn->setText(QCoreApplication::translate("SetupWidget", "Alex", nullptr));
        mercury_btn->setText(QCoreApplication::translate("SetupWidget", "Mercury", nullptr));
        penelopue_btn->setText(QCoreApplication::translate("SetupWidget", "Penelopie", nullptr));
        modules_btn->setText(QCoreApplication::translate("SetupWidget", "Modules", nullptr));
        firmware_check_btn->setText(QCoreApplication::translate("SetupWidget", "ON", nullptr));
        pennylane_btn->setText(QCoreApplication::translate("SetupWidget", "Pennylane", nullptr));
        hermes_btn->setText(QCoreApplication::translate("SetupWidget", "Hermes", nullptr));
        label->setText(QCoreApplication::translate("SetupWidget", "Fimrware Check", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("SetupWidget", "Sample Rate", nullptr));
        sample_48k_btn->setText(QCoreApplication::translate("SetupWidget", "48KHz", nullptr));
        sample_96k_btn->setText(QCoreApplication::translate("SetupWidget", "96kHz", nullptr));
        sample_192k_btn->setText(QCoreApplication::translate("SetupWidget", "192kHz", nullptr));
        sample_384k_btn->setText(QCoreApplication::translate("SetupWidget", "384kHz", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("SetupWidget", "Hardware Interface", nullptr));
        hw_none_btn->setText(QCoreApplication::translate("SetupWidget", "None", nullptr));
        hw_network_btn->setText(QCoreApplication::translate("SetupWidget", "Network", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("SetupWidget", "Local Network Interface", nullptr));
        label_2->setText(QCoreApplication::translate("SetupWidget", "IP Addr   ", nullptr));
        label_3->setText(QCoreApplication::translate("SetupWidget", "Socket Buffer", nullptr));
        skt_buffer_enable_btn->setText(QCoreApplication::translate("SetupWidget", "Enable", nullptr));
        label_4->setText(QCoreApplication::translate("SetupWidget", "HPSDR Addr", nullptr));
        hpsdr_search_btn->setText(QCoreApplication::translate("SetupWidget", "Search", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(hpsdr), QCoreApplication::translate("SetupWidget", "HPSDR", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(audio), QCoreApplication::translate("SetupWidget", "audio", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SetupWidget: public Ui_SetupWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETUPWIDGET_H
