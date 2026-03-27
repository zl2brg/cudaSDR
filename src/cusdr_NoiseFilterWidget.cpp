/*
*   Copyright 2018 Simon Eatough, Zl2BRG
        *
        *   This program is free software; you can redistribute it and/or modify
        *   it under the terms of the GNU Library General Public License version 2 as
        *   published by the Free Software Foundation
        *
        *   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
        *   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <QBoxLayout>
#include "cusdr_NoiseFilterWidget.h"

#define	btn_height		15
#define	btn_width		70
#define	btn_widths		32
#define	btn_width2		52
#define	btn_width3		60

NoiseFilterWidget::NoiseFilterWidget(QWidget *parent)
        : QWidget(parent)
        , set(Settings::instance())
        , m_serverMode(set->getCurrentServerMode())
        , m_hwInterface(set->getHWInterface())
        , m_dataEngineState(set->getDataEngineState())
        //, m_panadapterMode(set->getPanadapterMode())
        //, m_waterColorScheme(set->getWaterfallColorScheme())
        , m_minimumWidgetWidth(set->getMinimumWidgetWidth())
        , m_minimumGroupBoxWidth(set->getMinimumGroupBoxWidth())
        , m_btnSpacing(5)
        , m_currentReceiver(set->getCurrentReceiver())
        , m_mouseOver(false)
{
    //setMinimumWidth(m_minimumWidgetWidth);
    setContentsMargins(4, 0, 4, 0);
    setMouseTracking(true);

    m_rxDataList = set->getReceiverDataList();

    createNoiseBlankerGroup();
    createNoiseFilterGroup();
    createDspGroup();

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    mainLayout->setSpacing(5);
    mainLayout->setMargin(0);
    mainLayout->addSpacing(8);

    QHBoxLayout *hbox1 = new QHBoxLayout;
    hbox1->setSpacing(0);
    hbox1->setMargin(0);
    hbox1->addStretch();
    hbox1->addWidget(noiseBlankerModeGroupBox);

    QHBoxLayout *hbox2 = new QHBoxLayout;
    hbox2->setSpacing(0);
    hbox2->setMargin(0);
    hbox2->addStretch();
    hbox2->addWidget(noiseFilterModeGroupBox);

    QHBoxLayout *hbox3 = new QHBoxLayout;
    hbox3->setSpacing(0);
    hbox3->setMargin(0);
    hbox3->addStretch();
    hbox3->addWidget(dspCombo);

    mainLayout->addLayout(hbox1);
    mainLayout->addLayout(hbox2);
    mainLayout->addLayout(hbox3);

    mainLayout->addStretch();
    setLayout(mainLayout);

    setupConnections();
}


QSize NoiseFilterWidget::sizeHint() const {

    return QSize(m_minimumWidgetWidth, height());
}

QSize NoiseFilterWidget::minimumSizeHint() const {

    return QSize(m_minimumWidgetWidth, height());
}

NoiseFilterWidget::~NoiseFilterWidget() {

    // disconnect all signals
    disconnect(set, 0, this, 0);
    disconnect(this, 0, 0, 0);
}

void NoiseFilterWidget::setupConnections() {

}


void NoiseFilterWidget::createNoiseBlankerGroup() {

    m_nbModelabel = new QLabel("Noise Blanker Mode:", this);
    m_nbModelabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    m_nbModelabel->setStyleSheet(set->getLabelStyle());
    nbModeCombo = new QComboBox(this);
    nbModeCombo->addItems(QStringList() << "Off" << "NB1" <<  "NB2");
    nbModeCombo->setFont(m_fonts.normalFont);
    nbModeCombo->setCurrentIndex(set->getnbMode(m_currentReceiver));
    nbModeCombo->setStyleSheet(set->getComboBoxStyle());
    nbModeCombo->setCurrentIndex(set->getnbMode(m_currentReceiver));

    snbCheckBox = new QCheckBox(this);
    snbCheckBox->setText("SNB");
    snbCheckBox->setFont(QFont("Arial", 8));
    snbCheckBox->setStyleSheet(set->getLabelStyle());
    snbCheckBox->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
    snbCheckBox->setChecked(set->getSnb(m_currentReceiver));

    anfCheckBox = new QCheckBox(this);
    anfCheckBox->setText("ANF");
    anfCheckBox->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
    anfCheckBox->setFont(QFont("Arial", 8));
    anfCheckBox->setStyleSheet(set->getLabelStyle());
    anfCheckBox->setChecked(set->getAnf(m_currentReceiver));

    QHBoxLayout *hbox1 = new QHBoxLayout();
    hbox1->setSpacing(4);
    hbox1->setMargin(0);
    hbox1->addStretch();
    hbox1->addWidget(m_nbModelabel);
    hbox1->addStretch();
    hbox1->addWidget(nbModeCombo);

    QHBoxLayout *hbox2 = new QHBoxLayout();
    hbox2->setSpacing(4);
    hbox2->setMargin(0);
    hbox2->addStretch();
    hbox2->addWidget(anfCheckBox);
    hbox2->addStretch();
    hbox2->addWidget(snbCheckBox);


    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addLayout(hbox1);
    vbox->setSpacing(10);
    vbox->addLayout(hbox2);
    vbox->addSpacing(1);

    noiseBlankerModeGroupBox = new QGroupBox(tr("Noise Blanker Mode"), this);
    noiseBlankerModeGroupBox->setMinimumWidth(m_minimumGroupBoxWidth);
    noiseBlankerModeGroupBox->setLayout(vbox);
    noiseBlankerModeGroupBox->setStyleSheet(set->getWidgetStyle());
    noiseBlankerModeGroupBox->setFont(QFont("Arial", 8));
}


void NoiseFilterWidget::createNoiseFilterGroup() {

    m_nbFilterlabel = new QLabel("Noise Blanker:", this);
    m_nbFilterlabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    m_nbFilterlabel->setStyleSheet(set->getLabelStyle());
    nbFilterModeCombo= new QComboBox(this);
    nbFilterModeCombo->addItems(QStringList() << "Off" << "NB1" <<  "NB2");
    nbFilterModeCombo->setFont(m_fonts.normalFont);
    nbFilterModeCombo->setCurrentIndex(0);
    nbFilterModeCombo->setStyleSheet(set->getComboBoxStyle());
    nbFilterModeCombo->setCurrentIndex(set->getnbMode(m_currentReceiver));


    QHBoxLayout *hbox1 = new QHBoxLayout();
    hbox1->setSpacing(4);
    hbox1->setMargin(0);
    hbox1->addStretch();
    hbox1->addWidget(m_nbFilterlabel);
    hbox1->addStretch();
    hbox1->addWidget(nbFilterModeCombo);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addLayout(hbox1);
    vbox->setSpacing(6);
    vbox->addSpacing(6);

    noiseFilterModeGroupBox = new QGroupBox(tr("Noise Reduction"), this);
    noiseFilterModeGroupBox->setMinimumWidth(m_minimumGroupBoxWidth);
    noiseFilterModeGroupBox->setLayout(vbox);
    noiseFilterModeGroupBox->setStyleSheet(set->getWidgetStyle());
    noiseFilterModeGroupBox->setFont(QFont("Arial", 8));

}

void NoiseFilterWidget::createDspGroup(){

    nrGainMethodlabel = new QLabel("NR2 Gain Method:", this);
    nrGainMethodlabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    nrGainMethodlabel->setStyleSheet(set->getLabelStyle());

    nr2NPElabel = new QLabel("NR2 NPE:", this);
    nr2NPElabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    nr2NPElabel->setStyleSheet(set->getLabelStyle());

    nrprepostlabel = new QLabel("Process:", this);
    nrprepostlabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    nrprepostlabel->setStyleSheet(set->getLabelStyle());

    nr2GainMethodCombo= new QComboBox(this);
    nr2GainMethodCombo->addItems(QStringList() << "Linear" << "Log" <<  "Gamma");
    nr2GainMethodCombo->setFont(m_fonts.normalFont);
    nr2GainMethodCombo->setCurrentIndex(0);
    nr2GainMethodCombo->setStyleSheet(set->getComboBoxStyle());
    nr2GainMethodCombo->setCurrentIndex(set->getNr2GainMethod(m_currentReceiver));



    nrPreAGC = new QRadioButton(this);
    nrPreAGC->setText("Pre AGC");
    nrPreAGC->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
    nrPreAGC->setFont(QFont("Arial", 8));
    nrPreAGC->setStyleSheet(set->getLabelStyle());

    nrPostAGC = new QRadioButton(this);
    nrPostAGC->setText("Pre AGC");
    nrPostAGC->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
    nrPostAGC->setFont(QFont("Arial", 8));
    nrPostAGC->setStyleSheet(set->getLabelStyle());

    nrMMSE = new QRadioButton(this);
    nrMMSE->setText("MMSE");
    nrMMSE->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
    nrMMSE->setFont(QFont("Arial", 8));
    nrMMSE->setStyleSheet(set->getLabelStyle());

    nrOSMS = new QRadioButton(this);
    nrOSMS->setText("OSMS");
    nrOSMS->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
    nrOSMS->setFont(QFont("Arial", 8));
    nrOSMS->setStyleSheet(set->getLabelStyle());

    nr2AE = new QCheckBox(this);
    nr2AE->setText("NR2AE");
    nr2AE->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
    nr2AE->setFont(QFont("Arial", 8));
    nr2AE->setStyleSheet(set->getLabelStyle());
    nr2AE->setChecked(set->getNr2ae(m_currentReceiver));




    QHBoxLayout *hbox1 = new QHBoxLayout();
    hbox1->setSpacing(1);
    hbox1->setMargin(0);
    hbox1->addStretch();
    hbox1->addWidget(nrGainMethodlabel);
    hbox1->addStretch();
    hbox1->addWidget(nr2GainMethodCombo);

    QHBoxLayout *hbox2 = new QHBoxLayout();
    hbox2->setSpacing(1);
    hbox2->setMargin(0);
    hbox2->addStretch();
    hbox2->addWidget(nrprepostlabel);
    hbox2->addStretch();
    hbox2->addWidget(nrPreAGC);
    hbox2->addStretch();
    hbox2->addWidget(nrPostAGC);

    QHBoxLayout *hbox3 = new QHBoxLayout();
    hbox3->setSpacing(1);
    hbox3->setMargin(0);
    hbox3->addStretch();
    hbox3->addWidget(nr2NPElabel);
    hbox3->addStretch();
    hbox3->addWidget(nrMMSE);
    hbox3->addStretch();
    hbox3->addWidget(nrOSMS);

    QHBoxLayout *hbox4 = new QHBoxLayout();
    hbox4->setSpacing(1);
    hbox4->setMargin(0);
    hbox4->addStretch();
    hbox4->addWidget(nr2AE);


    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(6);
    vbox->addLayout(hbox1);
    vbox->addLayout(hbox2);
    vbox->addLayout(hbox3);
    vbox->addLayout(hbox4);


    dspCombo = new QGroupBox(tr("DSP"), this);
    dspCombo->setMinimumWidth(m_minimumGroupBoxWidth);
    dspCombo->setLayout(vbox);
    dspCombo->setStyleSheet(set->getWidgetStyle());
    dspCombo->setFont(QFont("Arial", 8));
}



void NoiseFilterWidget::setCurrentReceiver(QObject *sender, int rx) {

}

void NoiseFilterWidget::systemStateChanged(
        QObject *sender,
        QSDR::_Error err,
        QSDR::_HWInterfaceMode hwmode,
        QSDR::_ServerMode mode,
        QSDR::_DataEngineState state)
{
    Q_UNUSED (sender)
    Q_UNUSED (err)

    if (m_hwInterface != hwmode)
        m_hwInterface = hwmode;


    if (m_serverMode != mode)
        m_serverMode = mode;

    if (m_dataEngineState != state)
        m_dataEngineState = state;

    update();
}