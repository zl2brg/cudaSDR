/********************************************************************************
** Form generated from reading UI file 'cusdr_display.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CUSDR_DISPLAY_H
#define UI_CUSDR_DISPLAY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <Util/cusdr_buttons.h>

QT_BEGIN_NAMESPACE

class Ui_cusdr_Display
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_3;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_10;
    QLabel *label_7;
    QSlider *horizontalSlider_3;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_2;
    QComboBox *comboBox_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QComboBox *comboBox;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_3;
    QComboBox *comboBox_3;
    QHBoxLayout *horizontalLayout;
    AeroButton *pushButton_3;
    AeroButton *pushButton_2;
    AeroButton *pushButton;
    QGroupBox *groupBox_4;
    QVBoxLayout *verticalLayout_5;
    QHBoxLayout *horizontalLayout_7;
    QPushButton *pushButton_8;
    QPushButton *pushButton_7;
    QHBoxLayout *horizontalLayout_8;
    QLabel *label_5;
    QSpinBox *spinBox;
    QHBoxLayout *horizontalLayout_9;
    QLabel *label_6;
    QSpinBox *spinBox_2;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *pushButton_4;
    QPushButton *pushButton_5;
    QPushButton *pushButton_6;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_4;
    QComboBox *comboBox_4;

    void setupUi(QWidget *cusdr_Display)
    {
        if (cusdr_Display->objectName().isEmpty())
            cusdr_Display->setObjectName("cusdr_Display");
        cusdr_Display->resize(336, 546);
        verticalLayoutWidget = new QWidget(cusdr_Display);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(10, 0, 311, 532));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        groupBox_2 = new QGroupBox(verticalLayoutWidget);
        groupBox_2->setObjectName("groupBox_2");
        verticalLayout_3 = new QVBoxLayout(groupBox_2);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName("verticalLayout_2");
        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName("horizontalLayout_10");
        label_7 = new QLabel(groupBox_2);
        label_7->setObjectName("label_7");

        horizontalLayout_10->addWidget(label_7);

        horizontalSlider_3 = new QSlider(groupBox_2);
        horizontalSlider_3->setObjectName("horizontalSlider_3");
        horizontalSlider_3->setOrientation(Qt::Horizontal);

        horizontalLayout_10->addWidget(horizontalSlider_3);


        verticalLayout_2->addLayout(horizontalLayout_10);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName("label_2");

        horizontalLayout_3->addWidget(label_2);

        comboBox_2 = new QComboBox(groupBox_2);
        comboBox_2->setObjectName("comboBox_2");

        horizontalLayout_3->addWidget(comboBox_2);


        verticalLayout_2->addLayout(horizontalLayout_3);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        label = new QLabel(groupBox_2);
        label->setObjectName("label");

        horizontalLayout_2->addWidget(label);

        comboBox = new QComboBox(groupBox_2);
        comboBox->setObjectName("comboBox");

        horizontalLayout_2->addWidget(comboBox);


        verticalLayout_2->addLayout(horizontalLayout_2);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName("label_3");

        horizontalLayout_4->addWidget(label_3);

        comboBox_3 = new QComboBox(groupBox_2);
        comboBox_3->setObjectName("comboBox_3");

        horizontalLayout_4->addWidget(comboBox_3);


        verticalLayout_2->addLayout(horizontalLayout_4);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        pushButton_3 = new AeroButton(groupBox_2);
        pushButton_3->setObjectName("pushButton_3");

        horizontalLayout->addWidget(pushButton_3);

        pushButton_2 = new AeroButton(groupBox_2);
        pushButton_2->setObjectName("pushButton_2");

        horizontalLayout->addWidget(pushButton_2);

        pushButton = new AeroButton(groupBox_2);
        pushButton->setObjectName("pushButton");

        horizontalLayout->addWidget(pushButton);


        verticalLayout_2->addLayout(horizontalLayout);


        verticalLayout_3->addLayout(verticalLayout_2);

        groupBox_4 = new QGroupBox(groupBox_2);
        groupBox_4->setObjectName("groupBox_4");
        verticalLayout_5 = new QVBoxLayout(groupBox_4);
        verticalLayout_5->setObjectName("verticalLayout_5");
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName("horizontalLayout_7");
        pushButton_8 = new QPushButton(groupBox_4);
        pushButton_8->setObjectName("pushButton_8");

        horizontalLayout_7->addWidget(pushButton_8);

        pushButton_7 = new QPushButton(groupBox_4);
        pushButton_7->setObjectName("pushButton_7");

        horizontalLayout_7->addWidget(pushButton_7);


        verticalLayout_5->addLayout(horizontalLayout_7);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName("horizontalLayout_8");
        label_5 = new QLabel(groupBox_4);
        label_5->setObjectName("label_5");

        horizontalLayout_8->addWidget(label_5);

        spinBox = new QSpinBox(groupBox_4);
        spinBox->setObjectName("spinBox");

        horizontalLayout_8->addWidget(spinBox);


        verticalLayout_5->addLayout(horizontalLayout_8);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName("horizontalLayout_9");
        label_6 = new QLabel(groupBox_4);
        label_6->setObjectName("label_6");

        horizontalLayout_9->addWidget(label_6);

        spinBox_2 = new QSpinBox(groupBox_4);
        spinBox_2->setObjectName("spinBox_2");

        horizontalLayout_9->addWidget(spinBox_2);


        verticalLayout_5->addLayout(horizontalLayout_9);


        verticalLayout_3->addWidget(groupBox_4);

        groupBox_3 = new QGroupBox(groupBox_2);
        groupBox_3->setObjectName("groupBox_3");
        verticalLayout_4 = new QVBoxLayout(groupBox_3);
        verticalLayout_4->setObjectName("verticalLayout_4");
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName("horizontalLayout_5");
        pushButton_4 = new QPushButton(groupBox_3);
        pushButton_4->setObjectName("pushButton_4");

        horizontalLayout_5->addWidget(pushButton_4);

        pushButton_5 = new QPushButton(groupBox_3);
        pushButton_5->setObjectName("pushButton_5");

        horizontalLayout_5->addWidget(pushButton_5);

        pushButton_6 = new QPushButton(groupBox_3);
        pushButton_6->setObjectName("pushButton_6");

        horizontalLayout_5->addWidget(pushButton_6);


        verticalLayout_4->addLayout(horizontalLayout_5);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName("horizontalLayout_6");
        label_4 = new QLabel(groupBox_3);
        label_4->setObjectName("label_4");

        horizontalLayout_6->addWidget(label_4);

        comboBox_4 = new QComboBox(groupBox_3);
        comboBox_4->setObjectName("comboBox_4");

        horizontalLayout_6->addWidget(comboBox_4);


        verticalLayout_4->addLayout(horizontalLayout_6);


        verticalLayout_3->addWidget(groupBox_3);


        verticalLayout->addWidget(groupBox_2);


        retranslateUi(cusdr_Display);

        QMetaObject::connectSlotsByName(cusdr_Display);
    } // setupUi

    void retranslateUi(QWidget *cusdr_Display)
    {
        cusdr_Display->setWindowTitle(QCoreApplication::translate("cusdr_Display", "Form", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("cusdr_Display", "Panadapter Spectrum", nullptr));
        label_7->setText(QCoreApplication::translate("cusdr_Display", "Refresh Rate ", nullptr));
        label_2->setText(QCoreApplication::translate("cusdr_Display", "Averaging Det", nullptr));
        label->setText(QCoreApplication::translate("cusdr_Display", "Averaging Mode", nullptr));
        label_3->setText(QCoreApplication::translate("cusdr_Display", "FFT Size", nullptr));
        pushButton_3->setText(QCoreApplication::translate("cusdr_Display", "Solid", nullptr));
        pushButton_2->setText(QCoreApplication::translate("cusdr_Display", "Filled Line", nullptr));
        pushButton->setText(QCoreApplication::translate("cusdr_Display", "Line", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("cusdr_Display", "Wideband Spectrum", nullptr));
        pushButton_8->setText(QCoreApplication::translate("cusdr_Display", "Enhanced", nullptr));
        pushButton_7->setText(QCoreApplication::translate("cusdr_Display", "Simple", nullptr));
        label_5->setText(QCoreApplication::translate("cusdr_Display", "Offset Low (dbm)", nullptr));
        label_6->setText(QCoreApplication::translate("cusdr_Display", "Offset High (dbm)", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("cusdr_Display", "Wideband Panadaptor Spectrum", nullptr));
        pushButton_4->setText(QCoreApplication::translate("cusdr_Display", "Solid", nullptr));
        pushButton_5->setText(QCoreApplication::translate("cusdr_Display", "Filled Line", nullptr));
        pushButton_6->setText(QCoreApplication::translate("cusdr_Display", "Line", nullptr));
        label_4->setText(QCoreApplication::translate("cusdr_Display", "Averaging Filter", nullptr));
    } // retranslateUi

};

namespace Ui {
    class cusdr_Display: public Ui_cusdr_Display {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CUSDR_DISPLAY_H
