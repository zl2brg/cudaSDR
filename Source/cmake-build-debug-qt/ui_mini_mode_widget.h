/********************************************************************************
** Form generated from reading UI file 'mini_mode_widget.ui'
**
** Created by: Qt User Interface Compiler version 5.13.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MINI_MODE_WIDGET_H
#define UI_MINI_MODE_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <Util/cusdr_buttons.h>

QT_BEGIN_NAMESPACE

class Ui_MiniModeWidget
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    AeroButton *mode_lsb;
    AeroButton *mode_usb;
    AeroButton *mode_dsb;
    AeroButton *mode_cwl;
    AeroButton *mode_cwu;
    AeroButton *mode_fm;
    AeroButton *mode_am;
    AeroButton *mode_digu;
    AeroButton *mode_digl;
    AeroButton *mode_spec;
    AeroButton *mode_sam;
    AeroButton *mode_freedv;

    void setupUi(QWidget *MiniModeWidget)
    {
        if (MiniModeWidget->objectName().isEmpty())
            MiniModeWidget->setObjectName(QString::fromUtf8("MiniModeWidget"));
        MiniModeWidget->resize(1073, 46);
        MiniModeWidget->setAutoFillBackground(false);
        verticalLayout = new QVBoxLayout(MiniModeWidget);
        verticalLayout->setSpacing(1);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(1, 1, 0, 1);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(10);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setSizeConstraint(QLayout::SetMinimumSize);
        horizontalLayout->setContentsMargins(1, 1, 1, 1);
        mode_lsb = new AeroButton(MiniModeWidget);
        mode_lsb->setObjectName(QString::fromUtf8("mode_lsb"));

        horizontalLayout->addWidget(mode_lsb);

        mode_usb = new AeroButton(MiniModeWidget);
        mode_usb->setObjectName(QString::fromUtf8("mode_usb"));

        horizontalLayout->addWidget(mode_usb);

        mode_dsb = new AeroButton(MiniModeWidget);
        mode_dsb->setObjectName(QString::fromUtf8("mode_dsb"));

        horizontalLayout->addWidget(mode_dsb);

        mode_cwl = new AeroButton(MiniModeWidget);
        mode_cwl->setObjectName(QString::fromUtf8("mode_cwl"));

        horizontalLayout->addWidget(mode_cwl);

        mode_cwu = new AeroButton(MiniModeWidget);
        mode_cwu->setObjectName(QString::fromUtf8("mode_cwu"));

        horizontalLayout->addWidget(mode_cwu);

        mode_fm = new AeroButton(MiniModeWidget);
        mode_fm->setObjectName(QString::fromUtf8("mode_fm"));

        horizontalLayout->addWidget(mode_fm);

        mode_am = new AeroButton(MiniModeWidget);
        mode_am->setObjectName(QString::fromUtf8("mode_am"));

        horizontalLayout->addWidget(mode_am);

        mode_digu = new AeroButton(MiniModeWidget);
        mode_digu->setObjectName(QString::fromUtf8("mode_digu"));

        horizontalLayout->addWidget(mode_digu);

        mode_digl = new AeroButton(MiniModeWidget);
        mode_digl->setObjectName(QString::fromUtf8("mode_digl"));

        horizontalLayout->addWidget(mode_digl);

        mode_spec = new AeroButton(MiniModeWidget);
        mode_spec->setObjectName(QString::fromUtf8("mode_spec"));

        horizontalLayout->addWidget(mode_spec);

        mode_sam = new AeroButton(MiniModeWidget);
        mode_sam->setObjectName(QString::fromUtf8("mode_sam"));

        horizontalLayout->addWidget(mode_sam);

        mode_freedv = new AeroButton(MiniModeWidget);
        mode_freedv->setObjectName(QString::fromUtf8("mode_freedv"));

        horizontalLayout->addWidget(mode_freedv);

        horizontalLayout->setStretch(0, 5);
        horizontalLayout->setStretch(1, 5);
        horizontalLayout->setStretch(2, 5);
        horizontalLayout->setStretch(3, 5);
        horizontalLayout->setStretch(4, 5);
        horizontalLayout->setStretch(5, 5);
        horizontalLayout->setStretch(6, 5);
        horizontalLayout->setStretch(7, 5);
        horizontalLayout->setStretch(8, 5);
        horizontalLayout->setStretch(9, 5);
        horizontalLayout->setStretch(10, 5);
        horizontalLayout->setStretch(11, 5);

        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(MiniModeWidget);

        QMetaObject::connectSlotsByName(MiniModeWidget);
    } // setupUi

    void retranslateUi(QWidget *MiniModeWidget)
    {
        MiniModeWidget->setWindowTitle(QCoreApplication::translate("MiniModeWidget", "Dialog", nullptr));
        mode_lsb->setText(QCoreApplication::translate("MiniModeWidget", "LSB", nullptr));
        mode_usb->setText(QCoreApplication::translate("MiniModeWidget", "USB", nullptr));
        mode_dsb->setText(QCoreApplication::translate("MiniModeWidget", "DSB", nullptr));
        mode_cwl->setText(QCoreApplication::translate("MiniModeWidget", "CWL", nullptr));
        mode_cwu->setText(QCoreApplication::translate("MiniModeWidget", "CWH", nullptr));
        mode_fm->setText(QCoreApplication::translate("MiniModeWidget", "FM", nullptr));
        mode_am->setText(QCoreApplication::translate("MiniModeWidget", "AM", nullptr));
        mode_digu->setText(QCoreApplication::translate("MiniModeWidget", "DIGU", nullptr));
        mode_digl->setText(QCoreApplication::translate("MiniModeWidget", "DIGL", nullptr));
        mode_spec->setText(QCoreApplication::translate("MiniModeWidget", "SPEC", nullptr));
        mode_sam->setText(QCoreApplication::translate("MiniModeWidget", "SAM", nullptr));
        mode_freedv->setText(QCoreApplication::translate("MiniModeWidget", "FDV", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MiniModeWidget: public Ui_MiniModeWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MINI_MODE_WIDGET_H
