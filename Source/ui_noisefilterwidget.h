/********************************************************************************
** Form generated from reading UI file 'noisefilterwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.16
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NOISEFILTERWIDGET_H
#define UI_NOISEFILTERWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_NoiseFilterWidget
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QComboBox *nrModeComboBox;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label;
    QComboBox *nbModeComboBox;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_3;
    QComboBox *nr2GainComboBox;
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *anfCheckBox;
    QCheckBox *snbCheckBox;
    QCheckBox *nr2aeCheckBox;
    QFrame *frame_5;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_4;
    QCheckBox *preAGCCheckBox;
    QCheckBox *postAGCCheckBox;
    QFrame *frame_4;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_5;
    QCheckBox *mmseCheckBox;
    QCheckBox *omsCheckBox;

    void setupUi(QWidget *NoiseFilterWidget)
    {
        if (NoiseFilterWidget->objectName().isEmpty())
            NoiseFilterWidget->setObjectName(QString::fromUtf8("NoiseFilterWidget"));
        NoiseFilterWidget->resize(211, 202);
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(NoiseFilterWidget->sizePolicy().hasHeightForWidth());
        NoiseFilterWidget->setSizePolicy(sizePolicy);
        verticalLayout = new QVBoxLayout(NoiseFilterWidget);
        verticalLayout->setSpacing(2);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetFixedSize);
        verticalLayout->setContentsMargins(2, 2, 2, 2);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_2 = new QLabel(NoiseFilterWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        nrModeComboBox = new QComboBox(NoiseFilterWidget);
        nrModeComboBox->addItem(QString());
        nrModeComboBox->addItem(QString());
        nrModeComboBox->addItem(QString());
        nrModeComboBox->setObjectName(QString::fromUtf8("nrModeComboBox"));

        horizontalLayout->addWidget(nrModeComboBox);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label = new QLabel(NoiseFilterWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_6->addWidget(label);

        nbModeComboBox = new QComboBox(NoiseFilterWidget);
        nbModeComboBox->addItem(QString());
        nbModeComboBox->addItem(QString());
        nbModeComboBox->addItem(QString());
        nbModeComboBox->setObjectName(QString::fromUtf8("nbModeComboBox"));

        horizontalLayout_6->addWidget(nbModeComboBox);


        verticalLayout->addLayout(horizontalLayout_6);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_3 = new QLabel(NoiseFilterWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_3->addWidget(label_3);

        nr2GainComboBox = new QComboBox(NoiseFilterWidget);
        nr2GainComboBox->addItem(QString());
        nr2GainComboBox->addItem(QString());
        nr2GainComboBox->addItem(QString());
        nr2GainComboBox->setObjectName(QString::fromUtf8("nr2GainComboBox"));

        horizontalLayout_3->addWidget(nr2GainComboBox);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        anfCheckBox = new QCheckBox(NoiseFilterWidget);
        anfCheckBox->setObjectName(QString::fromUtf8("anfCheckBox"));
        anfCheckBox->setLayoutDirection(Qt::RightToLeft);

        horizontalLayout_2->addWidget(anfCheckBox);

        snbCheckBox = new QCheckBox(NoiseFilterWidget);
        snbCheckBox->setObjectName(QString::fromUtf8("snbCheckBox"));
        snbCheckBox->setLayoutDirection(Qt::RightToLeft);

        horizontalLayout_2->addWidget(snbCheckBox);

        nr2aeCheckBox = new QCheckBox(NoiseFilterWidget);
        nr2aeCheckBox->setObjectName(QString::fromUtf8("nr2aeCheckBox"));
        nr2aeCheckBox->setLayoutDirection(Qt::RightToLeft);

        horizontalLayout_2->addWidget(nr2aeCheckBox);


        verticalLayout->addLayout(horizontalLayout_2);

        frame_5 = new QFrame(NoiseFilterWidget);
        frame_5->setObjectName(QString::fromUtf8("frame_5"));
        frame_5->setFrameShape(QFrame::StyledPanel);
        frame_5->setFrameShadow(QFrame::Raised);
        horizontalLayout_5 = new QHBoxLayout(frame_5);
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_4 = new QLabel(frame_5);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_5->addWidget(label_4);

        preAGCCheckBox = new QCheckBox(frame_5);
        preAGCCheckBox->setObjectName(QString::fromUtf8("preAGCCheckBox"));
        preAGCCheckBox->setLayoutDirection(Qt::RightToLeft);
        preAGCCheckBox->setAutoExclusive(true);

        horizontalLayout_5->addWidget(preAGCCheckBox);

        postAGCCheckBox = new QCheckBox(frame_5);
        postAGCCheckBox->setObjectName(QString::fromUtf8("postAGCCheckBox"));
        postAGCCheckBox->setLayoutDirection(Qt::RightToLeft);
        postAGCCheckBox->setAutoExclusive(true);

        horizontalLayout_5->addWidget(postAGCCheckBox);


        verticalLayout->addWidget(frame_5);

        frame_4 = new QFrame(NoiseFilterWidget);
        frame_4->setObjectName(QString::fromUtf8("frame_4"));
        frame_4->setFrameShape(QFrame::StyledPanel);
        frame_4->setFrameShadow(QFrame::Raised);
        horizontalLayout_4 = new QHBoxLayout(frame_4);
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_5 = new QLabel(frame_4);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_4->addWidget(label_5);

        mmseCheckBox = new QCheckBox(frame_4);
        mmseCheckBox->setObjectName(QString::fromUtf8("mmseCheckBox"));
        mmseCheckBox->setLayoutDirection(Qt::RightToLeft);
        mmseCheckBox->setAutoExclusive(true);

        horizontalLayout_4->addWidget(mmseCheckBox);

        omsCheckBox = new QCheckBox(frame_4);
        omsCheckBox->setObjectName(QString::fromUtf8("omsCheckBox"));
        omsCheckBox->setLayoutDirection(Qt::RightToLeft);
        omsCheckBox->setStyleSheet(QString::fromUtf8(""));
        omsCheckBox->setAutoExclusive(true);

        horizontalLayout_4->addWidget(omsCheckBox);


        verticalLayout->addWidget(frame_4);


        retranslateUi(NoiseFilterWidget);

        QMetaObject::connectSlotsByName(NoiseFilterWidget);
    } // setupUi

    void retranslateUi(QWidget *NoiseFilterWidget)
    {
        NoiseFilterWidget->setWindowTitle(QCoreApplication::translate("NoiseFilterWidget", "NoiseFilterWidget", nullptr));
        label_2->setText(QCoreApplication::translate("NoiseFilterWidget", "Noise Reduction ", nullptr));
        nrModeComboBox->setItemText(0, QCoreApplication::translate("NoiseFilterWidget", "Off", nullptr));
        nrModeComboBox->setItemText(1, QCoreApplication::translate("NoiseFilterWidget", "NR1", nullptr));
        nrModeComboBox->setItemText(2, QCoreApplication::translate("NoiseFilterWidget", "NR2", nullptr));

        label->setText(QCoreApplication::translate("NoiseFilterWidget", "Noise Blanker ", nullptr));
        nbModeComboBox->setItemText(0, QCoreApplication::translate("NoiseFilterWidget", "Off", nullptr));
        nbModeComboBox->setItemText(1, QCoreApplication::translate("NoiseFilterWidget", "NB1", nullptr));
        nbModeComboBox->setItemText(2, QCoreApplication::translate("NoiseFilterWidget", "NB2", nullptr));

        label_3->setText(QCoreApplication::translate("NoiseFilterWidget", "NR2 Gain Mode ", nullptr));
        nr2GainComboBox->setItemText(0, QCoreApplication::translate("NoiseFilterWidget", "Linear", nullptr));
        nr2GainComboBox->setItemText(1, QCoreApplication::translate("NoiseFilterWidget", "Log", nullptr));
        nr2GainComboBox->setItemText(2, QCoreApplication::translate("NoiseFilterWidget", "Method", nullptr));

        anfCheckBox->setText(QCoreApplication::translate("NoiseFilterWidget", "ANF", nullptr));
        snbCheckBox->setText(QCoreApplication::translate("NoiseFilterWidget", "SNB", nullptr));
        nr2aeCheckBox->setText(QCoreApplication::translate("NoiseFilterWidget", "NR2AE", nullptr));
        label_4->setText(QCoreApplication::translate("NoiseFilterWidget", "Proc", nullptr));
        preAGCCheckBox->setText(QCoreApplication::translate("NoiseFilterWidget", "PreAGC", nullptr));
        postAGCCheckBox->setText(QCoreApplication::translate("NoiseFilterWidget", "PostAGC", nullptr));
        label_5->setText(QCoreApplication::translate("NoiseFilterWidget", "NR2 NPE", nullptr));
        mmseCheckBox->setText(QCoreApplication::translate("NoiseFilterWidget", "MMSE", nullptr));
        omsCheckBox->setText(QCoreApplication::translate("NoiseFilterWidget", "OMS", nullptr));
    } // retranslateUi

};

namespace Ui {
    class NoiseFilterWidget: public Ui_NoiseFilterWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NOISEFILTERWIDGET_H
