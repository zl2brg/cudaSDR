/********************************************************************************
** Form generated from reading UI file 'display_ctrl.ui'
**
** Created by: Qt User Interface Compiler version 5.13.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DISPLAY_CTRL_H
#define UI_DISPLAY_CTRL_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <aerobutton.h>

QT_BEGIN_NAMESPACE

class Ui_Dialog
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout_2;
    AeroButton *pushButton_11;
    AeroButton *pushButton_12;
    AeroButton *pushButton_13;
    AeroButton *pushButton_14;
    AeroButton *pushButton_15;
    AeroButton *pushButton_16;
    AeroButton *pushButton_17;
    AeroButton *pushButton_18;
    AeroButton *pushButton_19;
    AeroButton *pushButton_20;
    QFrame *frame;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    AeroButton *pushButton;
    AeroButton *pushButton_2;
    AeroButton *pushButton_3;
    AeroButton *pushButton_4;
    AeroButton *pushButton_5;
    AeroButton *pushButton_9;
    AeroButton *pushButton_6;
    AeroButton *pushButton_7;
    AeroButton *pushButton_10;
    AeroButton *pushButton_8;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QString::fromUtf8("Dialog"));
        Dialog->resize(332, 333);
        layoutWidget = new QWidget(Dialog);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(90, 10, 82, 287));
        verticalLayout_2 = new QVBoxLayout(layoutWidget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        pushButton_11 = new AeroButton(layoutWidget);
        pushButton_11->setObjectName(QString::fromUtf8("pushButton_11"));

        verticalLayout_2->addWidget(pushButton_11);

        pushButton_12 = new AeroButton(layoutWidget);
        pushButton_12->setObjectName(QString::fromUtf8("pushButton_12"));

        verticalLayout_2->addWidget(pushButton_12);

        pushButton_13 = new AeroButton(layoutWidget);
        pushButton_13->setObjectName(QString::fromUtf8("pushButton_13"));

        verticalLayout_2->addWidget(pushButton_13);

        pushButton_14 = new AeroButton(layoutWidget);
        pushButton_14->setObjectName(QString::fromUtf8("pushButton_14"));

        verticalLayout_2->addWidget(pushButton_14);

        pushButton_15 = new AeroButton(layoutWidget);
        pushButton_15->setObjectName(QString::fromUtf8("pushButton_15"));

        verticalLayout_2->addWidget(pushButton_15);

        pushButton_16 = new AeroButton(layoutWidget);
        pushButton_16->setObjectName(QString::fromUtf8("pushButton_16"));

        verticalLayout_2->addWidget(pushButton_16);

        pushButton_17 = new AeroButton(layoutWidget);
        pushButton_17->setObjectName(QString::fromUtf8("pushButton_17"));

        verticalLayout_2->addWidget(pushButton_17);

        pushButton_18 = new AeroButton(layoutWidget);
        pushButton_18->setObjectName(QString::fromUtf8("pushButton_18"));

        verticalLayout_2->addWidget(pushButton_18);

        pushButton_19 = new AeroButton(layoutWidget);
        pushButton_19->setObjectName(QString::fromUtf8("pushButton_19"));

        verticalLayout_2->addWidget(pushButton_19);

        pushButton_20 = new AeroButton(layoutWidget);
        pushButton_20->setObjectName(QString::fromUtf8("pushButton_20"));

        verticalLayout_2->addWidget(pushButton_20);

        frame = new QFrame(Dialog);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setGeometry(QRect(180, 90, 141, 111));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        widget = new QWidget(Dialog);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(10, 10, 82, 287));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        pushButton = new AeroButton(widget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        verticalLayout->addWidget(pushButton);

        pushButton_2 = new AeroButton(widget);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));

        verticalLayout->addWidget(pushButton_2);

        pushButton_3 = new AeroButton(widget);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));

        verticalLayout->addWidget(pushButton_3);

        pushButton_4 = new AeroButton(widget);
        pushButton_4->setObjectName(QString::fromUtf8("pushButton_4"));

        verticalLayout->addWidget(pushButton_4);

        pushButton_5 = new AeroButton(widget);
        pushButton_5->setObjectName(QString::fromUtf8("pushButton_5"));

        verticalLayout->addWidget(pushButton_5);

        pushButton_9 = new AeroButton(widget);
        pushButton_9->setObjectName(QString::fromUtf8("pushButton_9"));

        verticalLayout->addWidget(pushButton_9);

        pushButton_6 = new AeroButton(widget);
        pushButton_6->setObjectName(QString::fromUtf8("pushButton_6"));

        verticalLayout->addWidget(pushButton_6);

        pushButton_7 = new AeroButton(widget);
        pushButton_7->setObjectName(QString::fromUtf8("pushButton_7"));

        verticalLayout->addWidget(pushButton_7);

        pushButton_10 = new AeroButton(widget);
        pushButton_10->setObjectName(QString::fromUtf8("pushButton_10"));

        verticalLayout->addWidget(pushButton_10);

        pushButton_8 = new AeroButton(widget);
        pushButton_8->setObjectName(QString::fromUtf8("pushButton_8"));

        verticalLayout->addWidget(pushButton_8);


        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QCoreApplication::translate("Dialog", "Dialog", nullptr));
        pushButton_11->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_12->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_13->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_14->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_15->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_16->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_17->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_18->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_19->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_20->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_2->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_3->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_4->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_5->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_9->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_6->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_7->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_10->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
        pushButton_8->setText(QCoreApplication::translate("Dialog", "PushButton", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Dialog: public Ui_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DISPLAY_CTRL_H
