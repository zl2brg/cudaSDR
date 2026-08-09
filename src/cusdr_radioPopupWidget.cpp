#include "Models/RadioModel.h"
#include "Models/SliceModel.h"
/**
* @file cusdr_radioPopupWidget.cpp
* @brief Radio control popup widget class for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.2
* @date 2025-09-11
*/

/*
 * Copyright 2010-2024 Hermann von Hasseln, DL3HVH and Contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License version 2 as
 * published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "cusdr_radioPopupWidget.h"
#include "UI/eq_curve_plot.h"
#include <QGuiApplication>
#include <QScreen>
#include <QEnterEvent>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QPainter>
#include <QSizeGrip>
#include <QCoreApplication>
#include <QSettings>
#include <QSignalBlocker>
#include <wdsp.h>

#define LOG_RADIOPOPUP
// use: RADIOPOPUP_DEBUG

#define	btn_height		14
#define	btn_height1		14
#define	btn_width		50
#define	btn_widthb		66
#define	btn_widths		34

namespace {
    int s_lastRadioPopupWidth = -1;

    static const char *kRadioPopupWidthKey = "window/radioPopupWidth";

    int loadSavedPopupWidth() {
        QSettings popupSettings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
        return popupSettings.value(kRadioPopupWidthKey, -1).toInt();
    }

    void savePopupWidth(int width) {
        QSettings popupSettings(QCoreApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat);
        popupSettings.setValue(kRadioPopupWidthKey, width);
    }
}


RadioPopupWidget::RadioPopupWidget(SliceModel *model, QWidget *parent)
    : QWidget(parent)
    , m_sliceModel(model)
    , m_sticky(false)
    , m_filterSlope(1)
    , m_var1WidthA(1800.0f)
    , m_var2WidthA(4000.0f)
    , m_var1WidthB(3500.0f)
    , m_var2WidthB(12000.0f)
    , m_var1WidthC(150.0f)
    , m_var2WidthC(800.0f)
    , m_activeFilterIndex(-1)
    , m_receiver(model ? model->id() : 0)
    , m_currentRx(0)
    , m_singleAdcDevice(false)
    , m_minimumWidgetWidth(250)
    , m_minimumGroupBoxWidth(240)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    connect(qApp, &QCoreApplication::aboutToQuit, this, &QWidget::close);

    setMouseTracking(true);
    setContentsMargins(4, 4, 4, 4);
    setWindowOpacity(0.9);

    setFocusPolicy(Qt::StrongFocus);

    setStyleSheet(
        "RadioPopupWidget {"
        "  border: 1px solid;"
        "  border-left-color: rgba(220, 120, 120, 255);"
        "  border-top-color: rgba(220, 120, 120, 255);"
        "  border-right-color: rgba(0, 0, 0, 255);"
        "  border-bottom-color: rgba(0, 0, 0, 255);"
        "}");

    fonts = new CFonts(this);
    m_fonts = fonts->getFonts();

    loadReceiverState(m_receiver);

    stickyBtn = new AeroButton("Lock", this);
    stickyBtn->setRoundness(0);
    stickyBtn->setGlass(false);
    QColor col = QColor(35, 35, 35);
    stickyBtn->setColor(col);
    col = QColor(120, 120, 120);
    stickyBtn->setHighlight(col);
    col = QColor(160, 100, 100);
    stickyBtn->setColorOn(col);
    stickyBtn->setFixedSize(btn_width, btn_height1);
    stickyBtn->setBtnState(AeroButton::OFF);
    connect(stickyBtn, &AeroButton::clicked, this, &RadioPopupWidget::setSticky);

    createOptionsBtnGroup();
    createBandBtnGroup();
    createAdcBtnGroup();
    updateAdcAvailability();
    createModeBtnGroup();
    createAgcBtnGroup();
    createFilterBtnWidgetA();
    createFilterBtnWidgetB();
    createFilterBtnWidgetC();
    createSlopeBtnGroup();

    m_varFilterLabel = new QLabel("Var 1: 1800 Hz", this);
    m_varFilterLabel->setFont(m_fonts.smallFont);
    m_varFilterLabel->setStyleSheet("color: rgba(180, 180, 180, 255);");
    m_varFilterLabel->setFixedWidth(100);

    m_varFilterSlider = new QSlider(Qt::Horizontal, this);
    m_varFilterSlider->setFixedHeight(20);
    m_varFilterSlider->setRange(100, 5000);
    m_varFilterSlider->setValue(1800);
    connect(m_varFilterSlider, &QSlider::valueChanged, this, &RadioPopupWidget::varFilterSliderValueChanged);

    QHBoxLayout *varHBox = new QHBoxLayout();
    varHBox->setContentsMargins(0, 0, 0, 0);
    varHBox->setSpacing(4);
    varHBox->addWidget(m_varFilterLabel);
    varHBox->addWidget(m_varFilterSlider);

    m_varFilterContainer = new QWidget(this);
    m_varFilterContainer->setContentsMargins(0, 0, 0, 0);
    m_varFilterContainer->setLayout(varHBox);
    m_varFilterContainer->setVisible(false);

    m_popupAgcWidget = new AGCOptionsWidget(this);
    m_noiseFilterWidget = new NoiseFilterWidget(this);

    m_filterStackedWidget = new QStackedWidget(this);
    m_filterStackedWidget->setContentsMargins(0, 0, 0, 0);
    m_filterStackedWidget->setFixedHeight(30);
    m_filterStackedWidget->addWidget(filterAWidget);
    m_filterStackedWidget->addWidget(filterBWidget);
    m_filterStackedWidget->addWidget(filterCWidget);


    QString rxStr = tr(" Rx %1 ");
    QLabel* rxLabel = new QLabel(rxStr.arg(m_receiver + 1));
    rxLabel->setStyleSheet("background-color: rgba(40, 40, 40, 255);");

    QHBoxLayout* title = new QHBoxLayout();
    title->setContentsMargins(0, 0, 0, 0);
    title->setSpacing(0);
    title->addWidget(rxLabel);
    title->addStretch();
    title->addWidget(stickyBtn);

    // "Radio" tab — existing controls
    QWidget *radioTabPage = new QWidget(this);
    QVBoxLayout *radioLayout = new QVBoxLayout(radioTabPage);
    radioLayout->setSpacing(0);
    radioLayout->setContentsMargins(4, 4, 4, 4);
    radioLayout->setSizeConstraint(QLayout::SetNoConstraint);
    radioLayout->addLayout(optionsVBox);
    radioLayout->addSpacing(16);
    radioLayout->addLayout(bandVBox);
    radioLayout->addSpacing(8);
    radioLayout->addLayout(adcVBox);
    radioLayout->addSpacing(8);
    radioLayout->addLayout(modeVBox);
    radioLayout->addSpacing(8);
    radioLayout->addWidget(m_filterStackedWidget);
    radioLayout->addSpacing(4);
    radioLayout->addWidget(m_varFilterContainer);
    radioLayout->addSpacing(4);
    radioLayout->addWidget(slopeWidget);
    radioLayout->addSpacing(16);
    radioLayout->addLayout(agcVBox);
    radioLayout->addStretch();

    m_popupTabWidget = new QTabWidget(this);
    m_popupTabWidget->addTab(radioTabPage, " Radio ");
    m_popupTabWidget->addTab(m_noiseFilterWidget, " Noise Filter ");
    m_popupTabWidget->addTab(m_popupAgcWidget, " AGC ");

    QSizeGrip *sizeGrip = new QSizeGrip(this);
    sizeGrip->setFixedSize(18, 18);
    sizeGrip->setToolTip("Resize popup");
    sizeGrip->setStyleSheet("QSizeGrip { background: transparent; border: none; }");

    QWidget *gripIndicator = new QWidget(sizeGrip);
    gripIndicator->setFixedSize(9, 9);
    gripIndicator->move(9, 9);
    gripIndicator->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    gripIndicator->setStyleSheet(
        "background-color: rgba(45, 122, 148, 150);"
        "border: 1px solid rgba(166, 196, 208, 210);"
        "border-radius: 2px;");

    QHBoxLayout *gripRow = new QHBoxLayout();
    gripRow->setContentsMargins(0, 0, 0, 0);
    gripRow->addStretch();
    gripRow->addWidget(sizeGrip);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addLayout(title);
    mainLayout->addSpacing(4);
    mainLayout->addWidget(m_popupTabWidget);
    mainLayout->addLayout(gripRow);

    setLayout(mainLayout);
    setMinimumWidth(m_minimumWidgetWidth);

    // setup values from settings.ini
    bandBtnList.at(m_hamBand)->setBtnState(AeroButton::ON);
    bandBtnList.at(m_hamBand)->update();

    dspModeChanged(m_receiver, m_dspModeList.at(m_hamBand));
    adcModeChanged(m_receiver, m_adcMode);
    agcModeChanged(m_receiver, m_agcMode, false);
    filterChanged(m_receiver, m_filterLo, m_filterHi);

    DSPMode dspMode = m_dspModeList.at(m_hamBand);
    if (dspMode == LSB || dspMode == USB || dspMode == DIGU || dspMode == DIGL) {
        m_filterStackedWidget->setCurrentIndex(0);
    }
    else if (dspMode == DSB || dspMode == FMN || dspMode == AM || dspMode == SAM) {
        m_filterStackedWidget->setCurrentIndex(1);
    }
    else if (dspMode == CWL || dspMode == CWU) {
        m_filterStackedWidget->setCurrentIndex(2);
    }

    setupConnections();
    m_closeTimer = new QTimer(this);
    m_closeTimer->setSingleShot(true);
    m_closeTimer->setInterval(15000); // 15 second delay before closing
    connect(m_closeTimer, &QTimer::timeout, this, [this]() {
        if (m_sticky) return;

        QWidget *hovered = QApplication::widgetAt(QCursor::pos());
        if (hovered == this || (hovered && isAncestorOf(hovered))) return;

        close();
    });
}

RadioPopupWidget::~RadioPopupWidget() {
    disconnect(nullptr, nullptr, nullptr);
    delete m_closeTimer;
}

QSize RadioPopupWidget::minimumSizeHint() const {
    return QSize(m_minimumWidgetWidth, height());
}

void RadioPopupWidget::setupConnections() {
}

void RadioPopupWidget::createOptionsBtnGroup() {
    avgBtn = new AeroButton("Pan Avg", this);
    avgBtn->setRoundness(10);
    avgBtn->setFont(m_fonts.smallFont);
    avgBtn->setFixedHeight(btn_height);
    avgBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    if (m_spectrumAveraging)
        avgBtn->setBtnState(AeroButton::ON);
    else
        avgBtn->setBtnState(AeroButton::OFF);

    connect(avgBtn, &AeroButton::clicked, this, &RadioPopupWidget::avgBtnClicked);

    gridBtn = new AeroButton("Pan Grid", this);
    gridBtn->setRoundness(10);
    gridBtn->setFont(m_fonts.smallFont);
    gridBtn->setFixedHeight(btn_height);
    gridBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    if (m_panGrid)
        gridBtn->setBtnState(AeroButton::ON);
    else
        gridBtn->setBtnState(AeroButton::OFF);

    connect(gridBtn, &AeroButton::clicked, this, &RadioPopupWidget::gridBtnClicked);

    peakHoldBtn = new AeroButton("Peak Hold", this);
    peakHoldBtn->setRoundness(10);
    peakHoldBtn->setFont(m_fonts.smallFont);
    peakHoldBtn->setFixedHeight(btn_height);
    peakHoldBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    peakHoldBtn->setBtnState(AeroButton::OFF);

    connect(peakHoldBtn, &AeroButton::clicked, this, &RadioPopupWidget::peakHoldBtnClicked);

    lockPanBtn = new AeroButton("Lock Pan", this);
    lockPanBtn->setRoundness(10);
    lockPanBtn->setFont(m_fonts.smallFont);
    lockPanBtn->setFixedHeight(btn_height);
    lockPanBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    if (m_panLocked)
        lockPanBtn->setBtnState(AeroButton::ON);
    else
        lockPanBtn->setBtnState(AeroButton::OFF);

    connect(lockPanBtn, &AeroButton::clicked, this, &RadioPopupWidget::panLockedBtnClicked);

    clickVfoBtn = new AeroButton("Click VFO", this);
    clickVfoBtn->setRoundness(10);
    clickVfoBtn->setFont(m_fonts.smallFont);
    clickVfoBtn->setFixedHeight(btn_height);
    clickVfoBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    if (m_clickVFO)
        clickVfoBtn->setBtnState(AeroButton::ON);
    else
        clickVfoBtn->setBtnState(AeroButton::OFF);

    connect(clickVfoBtn, &AeroButton::clicked, this, &RadioPopupWidget::clickVfoBtnClicked);

    showCrossBtn = new AeroButton("Hair Cross", this);
    showCrossBtn->setRoundness(10);
    showCrossBtn->setFont(m_fonts.smallFont);
    showCrossBtn->setFixedHeight(btn_height);
    showCrossBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    if (m_showCross)
        showCrossBtn->setBtnState(AeroButton::ON);
    else
        showCrossBtn->setBtnState(AeroButton::OFF);

    connect(showCrossBtn, &AeroButton::clicked, this, &RadioPopupWidget::hairCrossBtnClicked);

    midToVfoBtn = new AeroButton("Mid = VFO", this);
    midToVfoBtn->setRoundness(10);
    midToVfoBtn->setFont(m_fonts.smallFont);
    midToVfoBtn->setFixedHeight(btn_height);
    midToVfoBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    connect(midToVfoBtn, &AeroButton::clicked, this, &RadioPopupWidget::midToVfoBtnClicked);

    vfoToMidBtn = new AeroButton("VFO = Mid", this);
    vfoToMidBtn->setRoundness(10);
    vfoToMidBtn->setFont(m_fonts.smallFont);
    vfoToMidBtn->setFixedHeight(btn_height);
    vfoToMidBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    connect(vfoToMidBtn, &AeroButton::clicked, this, &RadioPopupWidget::vfoToMidBtnClicked);

    vfoABtn = new AeroButton("VFO A", this);
    vfoABtn->setRoundness(10);
    vfoABtn->setFont(m_fonts.smallFont);
    vfoABtn->setFixedHeight(btn_height);
    vfoABtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(vfoABtn, &AeroButton::clicked, this, &RadioPopupWidget::vfoABtnClicked);

    vfoBBtn = new AeroButton("VFO B", this);
    vfoBBtn->setRoundness(10);
    vfoBBtn->setFont(m_fonts.smallFont);
    vfoBBtn->setFixedHeight(btn_height);
    vfoBBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(vfoBBtn, &AeroButton::clicked, this, &RadioPopupWidget::vfoBBtnClicked);

    vfoAtoBBtn = new AeroButton("A>B", this);
    vfoAtoBBtn->setRoundness(10);
    vfoAtoBBtn->setFont(m_fonts.smallFont);
    vfoAtoBBtn->setFixedHeight(btn_height);
    vfoAtoBBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(vfoAtoBBtn, &AeroButton::clicked, this, &RadioPopupWidget::vfoAtoBBtnClicked);

    vfoBtoABtn = new AeroButton("B>A", this);
    vfoBtoABtn->setRoundness(10);
    vfoBtoABtn->setFont(m_fonts.smallFont);
    vfoBtoABtn->setFixedHeight(btn_height);
    vfoBtoABtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(vfoBtoABtn, &AeroButton::clicked, this, &RadioPopupWidget::vfoBtoABtnClicked);

    vfoSwapBtn = new AeroButton("A<>B", this);
    vfoSwapBtn->setRoundness(10);
    vfoSwapBtn->setFont(m_fonts.smallFont);
    vfoSwapBtn->setFixedHeight(btn_height);
    vfoSwapBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(vfoSwapBtn, &AeroButton::clicked, this, &RadioPopupWidget::vfoSwapBtnClicked);

    if (m_sliceModel) {
        connect(m_sliceModel, &SliceModel::activeVfoChanged,
                this, [this](SliceModel::ActiveVfo) { updateActiveVfoButtons(); });
        updateActiveVfoButtons();
    }

    m_PanLineBtn = new AeroButton("Line", this);
    m_PanLineBtn->setRoundness(10);
    m_PanLineBtn->setFont(m_fonts.smallFont);
    m_PanLineBtn->setFixedHeight(btn_height);
    m_PanLineBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    panadapterBtnList.append(m_PanLineBtn);

    connect(m_PanLineBtn, &AeroButton::clicked, this, &RadioPopupWidget::panModeChanged);

    m_PanFilledLineBtn = new AeroButton("Filled Line", this);
    m_PanFilledLineBtn->setRoundness(10);
    m_PanFilledLineBtn->setFont(m_fonts.smallFont);
    m_PanFilledLineBtn->setFixedHeight(btn_height);
    m_PanFilledLineBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    panadapterBtnList.append(m_PanFilledLineBtn);

    connect(m_PanFilledLineBtn, &AeroButton::clicked, this, &RadioPopupWidget::panModeChanged);

    m_PanSolidBtn = new AeroButton("Solid", this);
    m_PanSolidBtn->setRoundness(10);
    m_PanSolidBtn->setFont(m_fonts.smallFont);
    m_PanSolidBtn->setFixedHeight(btn_height);
    m_PanSolidBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    panadapterBtnList.append(m_PanSolidBtn);

    connect(m_PanSolidBtn, &AeroButton::clicked, this, &RadioPopupWidget::panModeChanged);

    switch (m_panadapterMode) {
    case PanGraphicsMode::Line:
        m_PanLineBtn->setBtnState(AeroButton::ON);
        m_PanFilledLineBtn->setBtnState(AeroButton::OFF);
        m_PanSolidBtn->setBtnState(AeroButton::OFF);
        break;
    case PanGraphicsMode::FilledLine:
        m_PanFilledLineBtn->setBtnState(AeroButton::ON);
        m_PanLineBtn->setBtnState(AeroButton::OFF);
        m_PanSolidBtn->setBtnState(AeroButton::OFF);
        break;
    case PanGraphicsMode::Solid:
        m_PanSolidBtn->setBtnState(AeroButton::ON);
        m_PanLineBtn->setBtnState(AeroButton::OFF);
        m_PanFilledLineBtn->setBtnState(AeroButton::OFF);
        break;
    }


    m_WaterfallSimpleBtn = new AeroButton("Simple", this);
    m_WaterfallSimpleBtn->setRoundness(10);
    m_WaterfallSimpleBtn->setFont(m_fonts.smallFont);
    m_WaterfallSimpleBtn->setFixedHeight(btn_height);
    m_WaterfallSimpleBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    waterfallBtnList.append(m_WaterfallSimpleBtn);

    connect(m_WaterfallSimpleBtn, &AeroButton::clicked, this, &RadioPopupWidget::waterfallModeChanged);

    m_WaterfallEnhancedBtn = new AeroButton("Enhanced", this);
    m_WaterfallEnhancedBtn->setRoundness(10);
    m_WaterfallEnhancedBtn->setFont(m_fonts.smallFont);
    m_WaterfallEnhancedBtn->setFixedHeight(btn_height);
    m_WaterfallEnhancedBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    waterfallBtnList.append(m_WaterfallEnhancedBtn);

    connect(m_WaterfallEnhancedBtn, &AeroButton::clicked, this, &RadioPopupWidget::waterfallModeChanged);

    switch (m_waterfallColorMode) {
    case WaterfallColorMode::Simple:
        m_WaterfallSimpleBtn->setBtnState(AeroButton::ON);
        m_WaterfallEnhancedBtn->setBtnState(AeroButton::OFF);
        break;
    case WaterfallColorMode::Enhanced:
        m_WaterfallSimpleBtn->setBtnState(AeroButton::OFF);
        m_WaterfallEnhancedBtn->setBtnState(AeroButton::ON);
        break;
    }

    QHBoxLayout* hbox1 = new QHBoxLayout();
    hbox1->setContentsMargins(0, 0, 0, 0);
    hbox1->setSpacing(0);
    hbox1->addWidget(avgBtn);
    hbox1->addWidget(gridBtn);
    hbox1->addWidget(peakHoldBtn);

    QHBoxLayout* hbox2 = new QHBoxLayout();
    hbox2->setContentsMargins(0, 0, 0, 0);
    hbox2->setSpacing(0);
    hbox2->addWidget(lockPanBtn);
    hbox2->addWidget(clickVfoBtn);
    hbox2->addWidget(showCrossBtn);

    QHBoxLayout* hbox3 = new QHBoxLayout();
    hbox3->setContentsMargins(0, 0, 0, 0);
    hbox3->setSpacing(0);
    hbox3->addWidget(midToVfoBtn);
    hbox3->addWidget(vfoToMidBtn);

    QHBoxLayout* hboxVfo = new QHBoxLayout();
    hboxVfo->setContentsMargins(0, 0, 0, 0);
    hboxVfo->setSpacing(0);
    hboxVfo->addWidget(vfoABtn);
    hboxVfo->addWidget(vfoBBtn);
    hboxVfo->addWidget(vfoAtoBBtn);
    hboxVfo->addWidget(vfoBtoABtn);
    hboxVfo->addWidget(vfoSwapBtn);

    QHBoxLayout* hbox4 = new QHBoxLayout();
    hbox4->setContentsMargins(0, 0, 0, 0);
    hbox4->setSpacing(0);
    hbox4->addWidget(m_PanLineBtn);
    hbox4->addWidget(m_PanFilledLineBtn);
    hbox4->addWidget(m_PanSolidBtn);

    QHBoxLayout* hbox5 = new QHBoxLayout();
    hbox5->setContentsMargins(0, 0, 0, 0);
    hbox5->setSpacing(0);
    hbox5->addWidget(m_WaterfallSimpleBtn);
    hbox5->addWidget(m_WaterfallEnhancedBtn);

    optionsVBox = new QVBoxLayout;
    optionsVBox->setSpacing(1);
    optionsVBox->addLayout(hbox1);
    optionsVBox->addLayout(hbox2);
    optionsVBox->addLayout(hbox3);
    optionsVBox->addLayout(hboxVfo);
    optionsVBox->addSpacing(4);
    optionsVBox->addLayout(hbox4);
    optionsVBox->addLayout(hbox5);
}

void RadioPopupWidget::createBandBtnGroup() {
    band2200mBtn = new AeroButton("2200m", this);
    bandBtnList.append(band2200mBtn);
    connect(band2200mBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band630mBtn = new AeroButton("630 m", this);
    bandBtnList.append(band630mBtn);
    connect(band630mBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band160mBtn = new AeroButton("160 m", this);
    bandBtnList.append(band160mBtn);
    connect(band160mBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band80mBtn = new AeroButton("80 m", this);
    bandBtnList.append(band80mBtn);
    connect(band80mBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band60mBtn = new AeroButton("60 m", this);
    bandBtnList.append(band60mBtn);
    connect(band60mBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band40mBtn = new AeroButton("40 m", this);
    bandBtnList.append(band40mBtn);
    connect(band40mBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band30mBtn = new AeroButton("30 m", this);
    bandBtnList.append(band30mBtn);
    connect(band30mBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band20mBtn = new AeroButton("20 m", this);
    bandBtnList.append(band20mBtn);
    connect(band20mBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band17mBtn = new AeroButton("17 m", this);
    bandBtnList.append(band17mBtn);
    connect(band17mBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band15mBtn = new AeroButton("15 m", this);
    bandBtnList.append(band15mBtn);
    connect(band15mBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band12mBtn = new AeroButton("12 m", this);
    bandBtnList.append(band12mBtn);
    connect(band12mBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band10mBtn = new AeroButton("10 m", this);
    bandBtnList.append(band10mBtn);
    connect(band10mBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band6mBtn = new AeroButton("6 m", this);
    bandBtnList.append(band6mBtn);
    connect(band6mBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band2mBtn = new AeroButton("2 m", this);
    bandBtnList.append(band2mBtn);
    connect(band2mBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band125cmBtn = new AeroButton("125 cm", this);
    bandBtnList.append(band125cmBtn);
    connect(band125cmBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band70cmBtn = new AeroButton("70 cm", this);
    bandBtnList.append(band70cmBtn);
    connect(band70cmBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band33cmBtn = new AeroButton("33 cm", this);
    bandBtnList.append(band33cmBtn);
    connect(band33cmBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band23cmBtn = new AeroButton("23 cm", this);
    bandBtnList.append(band23cmBtn);
    connect(band23cmBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band13cmBtn = new AeroButton("13 cm", this);
    bandBtnList.append(band13cmBtn);
    connect(band13cmBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band10cmBtn = new AeroButton("10 cm", this);
    bandBtnList.append(band10cmBtn);
    connect(band10cmBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    band5cmBtn = new AeroButton("5 cm", this);
    bandBtnList.append(band5cmBtn);
    connect(band5cmBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    bandGenBtn = new AeroButton("Gen", this);
    bandBtnList.append(bandGenBtn);
    connect(bandGenBtn, &AeroButton::clicked, this, &RadioPopupWidget::bandChangedByBtn);

    for (AeroButton *btn : bandBtnList) {
        btn->setRoundness(0);
        btn->setFixedHeight(btn_height);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        QColor col = QColor(0, 255, 0);
        btn->setTextOnColor(col);
        btn->update();
    }

    QHBoxLayout *hbox1 = new QHBoxLayout();
    hbox1->setContentsMargins(0, 0, 0, 0);
    hbox1->setSpacing(0);
    hbox1->addWidget(band2200mBtn);
    hbox1->addWidget(band630mBtn);
    hbox1->addWidget(band160mBtn);
    hbox1->addWidget(band80mBtn);
    hbox1->addWidget(band60mBtn);
    hbox1->addWidget(band40mBtn);
    hbox1->addWidget(band30mBtn);

    QHBoxLayout *hbox2 = new QHBoxLayout();
    hbox2->setContentsMargins(0, 0, 0, 0);
    hbox2->setSpacing(0);
    hbox2->addWidget(band20mBtn);
    hbox2->addWidget(band17mBtn);
    hbox2->addWidget(band15mBtn);
    hbox2->addWidget(band12mBtn);
    hbox2->addWidget(band10mBtn);
    hbox2->addWidget(band6mBtn);
    hbox2->addWidget(band2mBtn);

    QHBoxLayout *hbox3 = new QHBoxLayout();
    hbox3->setContentsMargins(0, 0, 0, 0);
    hbox3->setSpacing(0);
    hbox3->addWidget(band125cmBtn);
    hbox3->addWidget(band70cmBtn);
    hbox3->addWidget(band33cmBtn);
    hbox3->addWidget(band23cmBtn);
    hbox3->addWidget(band13cmBtn);
    hbox3->addWidget(band10cmBtn);
    hbox3->addWidget(band5cmBtn);

    QHBoxLayout *hbox4 = new QHBoxLayout();
    hbox4->setContentsMargins(0, 0, 0, 0);
    hbox4->setSpacing(0);
    hbox4->addWidget(bandGenBtn);

    bandVBox = new QVBoxLayout;
    bandVBox->setSpacing(1);
    bandVBox->addLayout(hbox1);
    bandVBox->addLayout(hbox2);
    bandVBox->addLayout(hbox3);
    bandVBox->addLayout(hbox4);
}

void RadioPopupWidget::createAdcBtnGroup() {
    adc1Btn = new AeroButton("ADC1", this);
    adcModeBtnList.append(adc1Btn);
    connect(adc1Btn, &AeroButton::clicked, this, &RadioPopupWidget::adcModeChangedByBtn);

    adc2Btn = new AeroButton("ADC2", this);
    adcModeBtnList.append(adc2Btn);
    connect(adc2Btn, &AeroButton::clicked, this, &RadioPopupWidget::adcModeChangedByBtn);

    for (AeroButton *btn : adcModeBtnList) {
        btn->setRoundness(0);
        btn->setFixedHeight(btn_height);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->update();
    }

    QHBoxLayout *hbox1 = new QHBoxLayout();
    hbox1->setContentsMargins(0, 0, 0, 0);
    hbox1->setSpacing(0);
    hbox1->addWidget(adc1Btn);
    hbox1->addWidget(adc2Btn);

    adcVBox = new QVBoxLayout;
    adcVBox->setSpacing(1);
    adcVBox->addLayout(hbox1);
}

void RadioPopupWidget::updateAdcAvailability() {
    if (adc2Btn) {
        adc2Btn->setEnabled(!m_singleAdcDevice);
        if (m_singleAdcDevice && m_adcMode == adc2) {
            m_adcMode = adc1;
            emit adcModeRequested(m_receiver, adc1);
        }
    }
}

void RadioPopupWidget::createModeBtnGroup() {
    lsbBtn = new AeroButton("LSB", this);
    dspModeBtnList.append(lsbBtn);
    connect(lsbBtn, &AeroButton::clicked, this, &RadioPopupWidget::dspModeChangedByBtn);

    usbBtn = new AeroButton("USB", this);
    dspModeBtnList.append(usbBtn);
    connect(usbBtn, &AeroButton::clicked, this, &RadioPopupWidget::dspModeChangedByBtn);

    dsbBtn = new AeroButton("DSB", this);
    dspModeBtnList.append(dsbBtn);
    connect(dsbBtn, &AeroButton::clicked, this, &RadioPopupWidget::dspModeChangedByBtn);

    cwlBtn = new AeroButton("CWL", this);
    dspModeBtnList.append(cwlBtn);
    connect(cwlBtn, &AeroButton::clicked, this, &RadioPopupWidget::dspModeChangedByBtn);

    cwuBtn = new AeroButton("CWU", this);
    dspModeBtnList.append(cwuBtn);
    connect(cwuBtn, &AeroButton::clicked, this, &RadioPopupWidget::dspModeChangedByBtn);

    fmnBtn = new AeroButton("FMN", this);
    dspModeBtnList.append(fmnBtn);
    connect(fmnBtn, &AeroButton::clicked, this, &RadioPopupWidget::dspModeChangedByBtn);

    amBtn = new AeroButton("AM", this);
    dspModeBtnList.append(amBtn);
    connect(amBtn, &AeroButton::clicked, this, &RadioPopupWidget::dspModeChangedByBtn);

    diguBtn = new AeroButton("DIGU", this);
    dspModeBtnList.append(diguBtn);
    connect(diguBtn, &AeroButton::clicked, this, &RadioPopupWidget::dspModeChangedByBtn);

    diglBtn = new AeroButton("DIGL", this);
    dspModeBtnList.append(diglBtn);
    connect(diglBtn, &AeroButton::clicked, this, &RadioPopupWidget::dspModeChangedByBtn);

    specBtn = new AeroButton("SPEC", this);
    dspModeBtnList.append(specBtn);
    connect(specBtn, &AeroButton::clicked, this, &RadioPopupWidget::dspModeChangedByBtn);

    samBtn = new AeroButton("SAM", this);
    dspModeBtnList.append(samBtn);
    connect(samBtn, &AeroButton::clicked, this, &RadioPopupWidget::dspModeChangedByBtn);

	drmBtn = new AeroButton("FreeDV", this);
	dspModeBtnList.append(drmBtn);
	connect(drmBtn, &AeroButton::clicked, this, &RadioPopupWidget::dspModeChangedByBtn);

	for (AeroButton *btn : dspModeBtnList) {
        btn->setRoundness(0);
        btn->setFixedHeight(btn_height);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->update();
    }

    QHBoxLayout *hbox1 = new QHBoxLayout();
    hbox1->setContentsMargins(0, 0, 0, 0);
    hbox1->setSpacing(0);
    hbox1->addWidget(lsbBtn);
    hbox1->addWidget(usbBtn);
    hbox1->addWidget(dsbBtn);
    hbox1->addWidget(cwlBtn);
    hbox1->addWidget(cwuBtn);
    hbox1->addWidget(fmnBtn);

    QHBoxLayout *hbox2 = new QHBoxLayout();
    hbox2->setContentsMargins(0, 0, 0, 0);
    hbox2->setSpacing(0);
    hbox2->addWidget(amBtn);
    hbox2->addWidget(diguBtn);
    hbox2->addWidget(specBtn);
    hbox2->addWidget(diglBtn);
    hbox2->addWidget(samBtn);
    hbox2->addWidget(drmBtn);

    m_freeDVModeCombo = new QComboBox(this);
    m_freeDVModeCombo->addItem("FreeDV 1600", 0);
    m_freeDVModeCombo->addItem("FreeDV 700C", 6);
    m_freeDVModeCombo->addItem("FreeDV RADE v1", 100);
    m_freeDVModeCombo->setCurrentIndex(0);
    connect(m_freeDVModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &RadioPopupWidget::freeDVModeSelectionChanged);

    m_freeDVStatusLabel = new QLabel("FreeDV: inactive (select DRM)", this);
    m_freeDVStatusLabel->setMinimumWidth(200);

    modeVBox = new QVBoxLayout;
    modeVBox->setSpacing(1);
    modeVBox->addLayout(hbox1);
    modeVBox->addLayout(hbox2);
    modeVBox->addWidget(m_freeDVModeCombo);
    modeVBox->addWidget(m_freeDVStatusLabel);

    QHBoxLayout *rxEqTop = new QHBoxLayout();
    m_rxEqEnable = new QCheckBox(QStringLiteral("RX EQ"), this);
    m_rxEqEnable->setChecked(Settings::instance()->getRxEqEnabled());
    connect(m_rxEqEnable, &QCheckBox::toggled, this, [](bool on) {
        Settings::instance()->setRxEqEnabled(on);
    });
    m_rxEqCurveDeg = new QSpinBox(this);
    m_rxEqCurveDeg->setRange(0, 3);
    m_rxEqCurveDeg->setPrefix(QStringLiteral("NURBS "));
    m_rxEqCurveDeg->setToolTip(QStringLiteral("0 = classic linear; 1–3 = NURBS degree"));
    m_rxEqCurveDeg->setValue(Settings::instance()->getRxEqCurveDeg());
    connect(m_rxEqCurveDeg, QOverload<int>::of(&QSpinBox::valueChanged), this, [](int deg) {
        Settings::instance()->setRxEqCurveDeg(deg);
    });
    rxEqTop->addWidget(m_rxEqEnable);
    rxEqTop->addStretch();
    rxEqTop->addWidget(m_rxEqCurveDeg);
    modeVBox->addLayout(rxEqTop);

    static const char *const kRxEqLabels[] = {
        "Pre", "32", "63", "125", "250", "500", "1k", "2k", "4k", "8k", "16k"
    };
    QHBoxLayout *eqRow = new QHBoxLayout();
    eqRow->setSpacing(2);
    const QVector<int> bands = Settings::instance()->getRxEqBands();
    for (int i = 0; i < 11; ++i) {
        QVBoxLayout *col = new QVBoxLayout();
        QLabel *lab = new QLabel(QString::fromLatin1(kRxEqLabels[i]), this);
        lab->setAlignment(Qt::AlignHCenter);
        QSlider *slider = new QSlider(Qt::Vertical, this);
        slider->setRange(-12, 12);
        slider->setValue(i < bands.size() ? bands.at(i) : 0);
        slider->setFixedHeight(72);
        slider->setToolTip(QStringLiteral("%1 Hz gain (dB)").arg(QString::fromLatin1(kRxEqLabels[i])));
        connect(slider, &QSlider::valueChanged, this, [i](int value) {
            Settings::instance()->setRxEqBand(i, value);
        });
        m_rxEqSliders.append(slider);
        col->addWidget(lab);
        col->addWidget(slider);
        eqRow->addLayout(col);
    }
    modeVBox->addLayout(eqRow);

    m_rxEqPlot = new EqCurvePlot(this);
    modeVBox->addWidget(m_rxEqPlot);
    auto refreshRxEqPlot = [this]() {
        if (!m_rxEqPlot)
            return;
        // RX channel may not exist yet at popup construction time.
        QVector<double> X(AudioConfig::kEqDrawPoints, 0.0);
        QVector<double> Y(AudioConfig::kEqDrawPoints, 0.0);
        GetRXAEQDraw(m_receiver, X.data(), Y.data());
        m_rxEqPlot->setCurve(X, Y);
    };
    connect(Settings::instance(), &Settings::rxEqChanged, this, refreshRxEqPlot);
    // Do not call GetRXAEQDraw here — OpenChannel for this RX happens later.

    updateFreeDVControls();
}

void RadioPopupWidget::createAgcBtnGroup() {
    agcOFF = new AeroButton("Off", this);
    agcOFF->setRoundness(0);
    agcOFF->setFixedHeight(btn_height);
    agcOFF->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    agcModeBtnList.append(agcOFF);
    connect(agcOFF, &AeroButton::clicked, this, &RadioPopupWidget::agcModeChangedByBtn);

    agcLONG = new AeroButton("Long", this);
    agcLONG->setRoundness(0);
    agcLONG->setFixedHeight(btn_height);
    agcLONG->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    agcModeBtnList.append(agcLONG);
    connect(agcLONG, &AeroButton::clicked, this, &RadioPopupWidget::agcModeChangedByBtn);

    agcSLOW = new AeroButton("Slow", this);
    agcSLOW->setRoundness(0);
    agcSLOW->setFixedHeight(btn_height);
    agcSLOW->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    agcModeBtnList.append(agcSLOW);
    connect(agcSLOW, &AeroButton::clicked, this, &RadioPopupWidget::agcModeChangedByBtn);

    agcMED = new AeroButton("Med", this);
    agcMED->setRoundness(0);
    agcMED->setFixedHeight(btn_height);
    agcMED->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    agcModeBtnList.append(agcMED);
    connect(agcMED, &AeroButton::clicked, this, &RadioPopupWidget::agcModeChangedByBtn);

    agcFAST = new AeroButton("Fast", this);
    agcFAST->setRoundness(0);
    agcFAST->setFixedHeight(btn_height);
    agcFAST->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    agcModeBtnList.append(agcFAST);
    connect(agcFAST, &AeroButton::clicked, this, &RadioPopupWidget::agcModeChangedByBtn);

    agcUSER = new AeroButton("User", this);
    agcUSER->setRoundness(0);
    agcUSER->setFixedHeight(btn_height);
    agcUSER->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    agcModeBtnList.append(agcUSER);
    connect(agcUSER, &AeroButton::clicked, this, &RadioPopupWidget::agcModeChangedByBtn);

    showAGCLines = new AeroButton("Show Lines", this);
    showAGCLines->setRoundness(0);
    showAGCLines->setFixedHeight(btn_height);
    showAGCLines->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(showAGCLines, &AeroButton::clicked, this, &RadioPopupWidget::agcShowLinesChanged);

    showAGCLines->setBtnState(AeroButton::OFF);

    QHBoxLayout* hbox1 = new QHBoxLayout();
    hbox1->setContentsMargins(0, 0, 0, 0);
    hbox1->setSpacing(0);
    hbox1->addWidget(agcOFF);
    hbox1->addWidget(agcLONG);
    hbox1->addWidget(agcSLOW);
    hbox1->addWidget(agcMED);
    hbox1->addWidget(agcFAST);
    hbox1->addWidget(agcUSER);

    QHBoxLayout* hbox2 = new QHBoxLayout();
    hbox2->setContentsMargins(0, 0, 0, 0);
    hbox2->setSpacing(0);
    hbox2->addWidget(showAGCLines);

    QHBoxLayout* hbox3 = new QHBoxLayout();
    hbox3->setContentsMargins(0, 0, 0, 0);
    hbox3->setSpacing(0);
    QCheckBox* anf = new QCheckBox();
    anf->setText("ANF");
    hbox3->addWidget(anf);

    agcVBox = new QVBoxLayout;
    agcVBox->setSpacing(1);
    agcVBox->addLayout(hbox1);
    agcVBox->addLayout(hbox2);
    agcVBox->addLayout(hbox3);
}

void RadioPopupWidget::createFilterBtnWidgetA() {
    filter1kBtnA = new AeroButton("1k", this);
    filter1kBtnA->setObjectName("1k");
    filterBtnListA.append(filter1kBtnA);
    connect(filter1kBtnA, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter1k8BtnA = new AeroButton("1k8", this);
    filter1k8BtnA->setObjectName("1k8");
    filterBtnListA.append(filter1k8BtnA);
    connect(filter1k8BtnA, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter2k1BtnA = new AeroButton("2k1", this);
    filter2k1BtnA->setObjectName("2k1");
    filterBtnListA.append(filter2k1BtnA);
    connect(filter2k1BtnA, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter2k4BtnA = new AeroButton("2k4", this);
    filter2k4BtnA->setObjectName("2k4");
    filterBtnListA.append(filter2k4BtnA);
    connect(filter2k4BtnA, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter2k7BtnA = new AeroButton("2k7", this);
    filter2k7BtnA->setObjectName("2k7");
    filterBtnListA.append(filter2k7BtnA);
    connect(filter2k7BtnA, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter2k9BtnA = new AeroButton("2k9", this);
    filter2k9BtnA->setObjectName("2k9");
    filterBtnListA.append(filter2k9BtnA);
    connect(filter2k9BtnA, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter3k3BtnA = new AeroButton("3k3", this);
    filter3k3BtnA->setObjectName("3k3");
    filterBtnListA.append(filter3k3BtnA);
    connect(filter3k3BtnA, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter3k8BtnA = new AeroButton("3k8", this);
    filter3k8BtnA->setObjectName("3k8");
    filterBtnListA.append(filter3k8BtnA);
    connect(filter3k8BtnA, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter4k4BtnA = new AeroButton("4k4", this);
    filter4k4BtnA->setObjectName("4k4");
    filterBtnListA.append(filter4k4BtnA);
    connect(filter4k4BtnA, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter5kBtnA = new AeroButton("5k", this);
    filter5kBtnA->setObjectName("5k");
    filterBtnListA.append(filter5kBtnA);
    connect(filter5kBtnA, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filterVar1BtnA = new AeroButton("Var1", this);
    filterVar1BtnA->setObjectName("Var1");
    filterBtnListA.append(filterVar1BtnA);
    connect(filterVar1BtnA, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filterVar2BtnA = new AeroButton("Var2", this);
    filterVar2BtnA->setObjectName("Var2");
    filterBtnListA.append(filterVar2BtnA);
    connect(filterVar2BtnA, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    for(AeroButton *btn : filterBtnListA) {
        btn->setRoundness(0);
        btn->setFixedHeight(btn_height);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    QHBoxLayout *hbox1 = new QHBoxLayout();
    hbox1->setContentsMargins(0, 0, 0, 0);
    hbox1->setSpacing(0);
    hbox1->addWidget(filter5kBtnA);
    hbox1->addWidget(filter4k4BtnA);
    hbox1->addWidget(filter3k8BtnA);
    hbox1->addWidget(filter3k3BtnA);
    hbox1->addWidget(filter2k9BtnA);
    hbox1->addWidget(filter2k7BtnA);

    QHBoxLayout *hbox2 = new QHBoxLayout();
    hbox2->setContentsMargins(0, 0, 0, 0);
    hbox2->setSpacing(0);
    hbox2->addWidget(filter2k4BtnA);
    hbox2->addWidget(filter2k1BtnA);
    hbox2->addWidget(filter1k8BtnA);
    hbox2->addWidget(filter1kBtnA);
    hbox2->addWidget(filterVar1BtnA);
    hbox2->addWidget(filterVar2BtnA);

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->setSpacing(0);
    vbox->setContentsMargins(0,0,0,0);
    vbox->addLayout(hbox1);
    vbox->addLayout(hbox2);

    filterAWidget = new QWidget();
    filterAWidget->setContentsMargins(0, 0, 0, 0);
    filterAWidget->setLayout(vbox);
}

void RadioPopupWidget::createFilterBtnWidgetB() {
    filter16kBtnB = new AeroButton("16k", this);
    filterBtnListB.append(filter16kBtnB);
    connect(filter16kBtnB, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter12kBtnB = new AeroButton("12k", this);
    filterBtnListB.append(filter12kBtnB);
    connect(filter12kBtnB, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter10kBtnB = new AeroButton("10k", this);
    filterBtnListB.append(filter10kBtnB);
    connect(filter10kBtnB, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter8kBtnB = new AeroButton("8k", this);
    filterBtnListB.append(filter8kBtnB);
    connect(filter8kBtnB, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter6k6BtnB = new AeroButton("6k6", this);
    filterBtnListB.append(filter6k6BtnB);
    connect(filter6k6BtnB, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter5k2BtnB = new AeroButton("5k2", this);
    filterBtnListB.append(filter5k2BtnB);
    connect(filter5k2BtnB, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter4kBtnB = new AeroButton("4k", this);
    filterBtnListB.append(filter4kBtnB);
    connect(filter4kBtnB, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter3k1BtnB = new AeroButton("3k1", this);
    filterBtnListB.append(filter3k1BtnB);
    connect(filter3k1BtnB, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter2k9BtnB = new AeroButton("2k9", this);
    filterBtnListB.append(filter2k9BtnB);
    connect(filter2k9BtnB, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter2k4BtnB = new AeroButton("2k4", this);
    filterBtnListB.append(filter2k4BtnB);
    connect(filter2k4BtnB, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filterVar1BtnB = new AeroButton("Var1", this);
    filterBtnListB.append(filterVar1BtnB);
    connect(filterVar1BtnB, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filterVar2BtnB = new AeroButton("Var2", this);
    filterBtnListB.append(filterVar2BtnB);
    connect(filterVar2BtnB, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    for(AeroButton *btn : filterBtnListB) {
        btn->setRoundness(0);
        btn->setFixedHeight(btn_height);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    QHBoxLayout *hbox1 = new QHBoxLayout();
    hbox1->setContentsMargins(0, 0, 0, 0);
    hbox1->setSpacing(0);
    hbox1->addWidget(filter16kBtnB);
    hbox1->addWidget(filter12kBtnB);
    hbox1->addWidget(filter10kBtnB);
    hbox1->addWidget(filter8kBtnB);
    hbox1->addWidget(filter6k6BtnB);
    hbox1->addWidget(filter5k2BtnB);

    QHBoxLayout *hbox2 = new QHBoxLayout();
    hbox2->setContentsMargins(0, 0, 0, 0);
    hbox2->setSpacing(0);
    hbox2->addWidget(filter4kBtnB);
    hbox2->addWidget(filter3k1BtnB);
    hbox2->addWidget(filter2k9BtnB);
    hbox2->addWidget(filter2k4BtnB);
    hbox2->addWidget(filterVar1BtnB);
    hbox2->addWidget(filterVar2BtnB);

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->setSpacing(0);
    vbox->setContentsMargins(0,0,0,0);
    vbox->addLayout(hbox1);
    vbox->addLayout(hbox2);

    filterBWidget = new QWidget();
    filterBWidget->setContentsMargins(0, 0, 0, 0);
    filterBWidget->setLayout(vbox);
}

void RadioPopupWidget::createFilterBtnWidgetC() {
    filter1kBtnC = new AeroButton("1k", this);
    filterBtnListC.append(filter1kBtnC);
    connect(filter1kBtnC, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter800BtnC = new AeroButton("800", this);
    filterBtnListC.append(filter800BtnC);
    connect(filter800BtnC, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter750BtnC = new AeroButton("750", this);
    filterBtnListC.append(filter750BtnC);
    connect(filter750BtnC, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter600BtnC = new AeroButton("600", this);
    filterBtnListC.append(filter600BtnC);
    connect(filter600BtnC, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter500BtnC = new AeroButton("500", this);
    filterBtnListC.append(filter500BtnC);
    connect(filter500BtnC, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter400BtnC = new AeroButton("400", this);
    filterBtnListC.append(filter400BtnC);
    connect(filter400BtnC, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter250BtnC = new AeroButton("250", this);
    filterBtnListC.append(filter250BtnC);
    connect(filter250BtnC, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter100BtnC = new AeroButton("100", this);
    filterBtnListC.append(filter100BtnC);
    connect(filter100BtnC, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter50BtnC = new AeroButton("50", this);
    filterBtnListC.append(filter50BtnC);
    connect(filter50BtnC, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filter25BtnC = new AeroButton("25", this);
    filterBtnListC.append(filter25BtnC);
    connect(filter25BtnC, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filterVar1BtnC = new AeroButton("Var1", this);
    filterBtnListC.append(filterVar1BtnC);
    connect(filterVar1BtnC, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    filterVar2BtnC = new AeroButton("Var2", this);
    filterBtnListC.append(filterVar2BtnC);
    connect(filterVar2BtnC, &AeroButton::clicked, this, &RadioPopupWidget::filterChangedByBtn);

    for(AeroButton *btn : filterBtnListC) {
        btn->setRoundness(0);
        btn->setFixedHeight(btn_height);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    QHBoxLayout *hbox1 = new QHBoxLayout();
    hbox1->setContentsMargins(0, 0, 0, 0);
    hbox1->setSpacing(0);
    hbox1->addWidget(filter1kBtnC);
    hbox1->addWidget(filter800BtnC);
    hbox1->addWidget(filter750BtnC);
    hbox1->addWidget(filter600BtnC);
    hbox1->addWidget(filter500BtnC);
    hbox1->addWidget(filter400BtnC);

    QHBoxLayout *hbox2 = new QHBoxLayout();
    hbox2->setContentsMargins(0, 0, 0, 0);
    hbox2->setSpacing(0);
    hbox2->addWidget(filter250BtnC);
    hbox2->addWidget(filter100BtnC);
    hbox2->addWidget(filter50BtnC);
    hbox2->addWidget(filter25BtnC);
    hbox2->addWidget(filterVar1BtnC);
    hbox2->addWidget(filterVar2BtnC);

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->setSpacing(0);
    vbox->setContentsMargins(0,0,0,0);
    vbox->addLayout(hbox1);
    vbox->addLayout(hbox2);

    filterCWidget = new QWidget();
    filterCWidget->setContentsMargins(0, 0, 0, 0);
    filterCWidget->setLayout(vbox);
}

void RadioPopupWidget::createSlopeBtnGroup() {
    QLabel *slopeLabel = new QLabel("Filter Slope", this);
    slopeLabel->setFont(m_fonts.smallFont);
    slopeLabel->setStyleSheet("color: rgba(180, 180, 180, 255);");

    slopeSoftBtn = new AeroButton("Soft", this);
    slopeNormBtn = new AeroButton("Norm", this);
    slopeSteepBtn = new AeroButton("Steep", this);
    slopeBrickBtn = new AeroButton("Brick", this);

    slopeBtnList.append(slopeSoftBtn);
    slopeBtnList.append(slopeNormBtn);
    slopeBtnList.append(slopeSteepBtn);
    slopeBtnList.append(slopeBrickBtn);

    for (AeroButton *btn : slopeBtnList) {
        btn->setRoundness(0);
        btn->setFont(m_fonts.smallFont);
        btn->setFixedHeight(btn_height);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->setBtnState(AeroButton::OFF);
        connect(btn, &AeroButton::clicked, this, &RadioPopupWidget::filterSlopeChangedByBtn);
    }

    if (m_filterSlope >= 0 && m_filterSlope < slopeBtnList.size()) {
        slopeBtnList.at(m_filterSlope)->setBtnState(AeroButton::ON);
    } else {
        slopeNormBtn->setBtnState(AeroButton::ON);
    }

    QHBoxLayout *slopeHBox = new QHBoxLayout();
    slopeHBox->setContentsMargins(0, 0, 0, 0);
    slopeHBox->setSpacing(0);
    slopeHBox->addWidget(slopeSoftBtn);
    slopeHBox->addWidget(slopeNormBtn);
    slopeHBox->addWidget(slopeSteepBtn);
    slopeHBox->addWidget(slopeBrickBtn);

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(2);
    vbox->addWidget(slopeLabel);
    vbox->addLayout(slopeHBox);

    slopeWidget = new QWidget(this);
    slopeWidget->setContentsMargins(0, 0, 0, 0);
    slopeWidget->setLayout(vbox);
}

void RadioPopupWidget::filterSlopeChangedByBtn() {
    AeroButton *button = qobject_cast<AeroButton *>(sender());
    if (!button) return;

    int idx = slopeBtnList.indexOf(button);
    if (idx < 0) return;

    for (AeroButton *btn : slopeBtnList) {
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }
    button->setBtnState(AeroButton::ON);
    button->update();

    m_filterSlope = idx;
    emit filterSlopeRequested(m_receiver, m_filterSlope);
}

void RadioPopupWidget::setFilterSlope(int slope) {
    if (slope < 0 || slope >= slopeBtnList.size()) return;
    m_filterSlope = slope;

    for (AeroButton *btn : slopeBtnList) {
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }
    slopeBtnList.at(slope)->setBtnState(AeroButton::ON);
    slopeBtnList.at(slope)->update();
}

void RadioPopupWidget::ctrFrequencyChanged(int mode, int rx, qint64 frequency) {
    Q_UNUSED (mode)

    if (m_receiver != rx) return;
    setCtrFrequency(frequency);
}

void RadioPopupWidget::vfoFrequencyChanged(int mode, int rx, qint64 frequency) {
    Q_UNUSED (mode)

    if (m_receiver != rx) return;
    setVfoFrequency(frequency);
}

void RadioPopupWidget::bandChangedByBtn() {
    AeroButton *button = qobject_cast<AeroButton *>(sender());
    int btnIndex = bandBtnList.indexOf(button);
    if (btnIndex == -1) return;

    for(AeroButton *btn : bandBtnList) {
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    button->setBtnState(AeroButton::ON);
    button->update();

    // Controller restores the last VFO for this band from Settings (source of truth).
    emit hamBandRequested(m_receiver, static_cast<HamBand>(btnIndex));
}

void RadioPopupWidget::bandChanged(int rx, bool byButton, HamBand band) {
    Q_UNUSED (byButton)

    if (m_receiver != rx) return;
    m_hamBand = band;

    for(AeroButton *btn : bandBtnList) {
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    bandBtnList.at(band)->setBtnState(AeroButton::ON);
    bandBtnList.at(band)->update();

    updateFreeDVControls();
}

void RadioPopupWidget::freeDVModeSelectionChanged(int index) {
    if (index < 0 || !m_freeDVModeCombo) return;

    const int mode = m_freeDVModeCombo->itemData(index).toInt();
    emit freeDVModeRequested(m_receiver, mode);
}

void RadioPopupWidget::freeDVModeChanged(int rx, int mode) {
    if (m_receiver != rx || !m_freeDVModeCombo) return;

    const int idx = m_freeDVModeCombo->findData(mode);
    if (idx >= 0 && idx != m_freeDVModeCombo->currentIndex()) {
        m_freeDVModeCombo->setCurrentIndex(idx);
    }
    updateFreeDVControls();
}

void RadioPopupWidget::freeDVStatusChanged(int rx, bool sync, float snr, quint64 rxFrames, quint64 txFrames) {
    if (m_receiver != rx || !m_freeDVStatusLabel || !m_freeDVModeCombo) return;

    const QString syncStr = sync ? "sync" : "search";
    const QString txt = QString("FreeDV %1: %2  SNR %3 dB  RXf %4  TXf %5")
                            .arg(m_freeDVModeCombo->currentText())
                            .arg(syncStr)
                            .arg(QString::number(snr, 'f', 1))
                            .arg(rxFrames)
                            .arg(txFrames);
    m_freeDVStatusLabel->setText(txt);
}

void RadioPopupWidget::updateFreeDVControls() {
    if (!m_freeDVModeCombo || !m_freeDVStatusLabel || m_dspModeList.isEmpty()) return;

    const DSPMode mode = m_dspModeList.at(m_hamBand);
    const bool isDrm = (mode == (DSPMode) FDV);

    m_freeDVModeCombo->setVisible(isDrm);
    m_freeDVStatusLabel->setVisible(true);

    if (!isDrm) {
        m_freeDVStatusLabel->setText("FreeDV: inactive (select FDV)");
        m_freeDVStatusLabel->setStyleSheet("color: rgb(150, 150, 150);");
    }
    else {
        m_freeDVStatusLabel->setStyleSheet("");
    }
}

void RadioPopupWidget::dspModeChangedByBtn() {
    AeroButton *button = qobject_cast<AeroButton *>(sender());
    int btnIndex = dspModeBtnList.indexOf(button);
    if (btnIndex == -1) return;

    for(AeroButton *btn : dspModeBtnList) {
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    DSPMode mode = static_cast<DSPMode>(btnIndex);
    emit dspModeRequested(m_receiver, mode);

    button->setBtnState(AeroButton::ON);
    button->update();
    updateFreeDVControls();
}

void RadioPopupWidget::dspModeChanged(int rx, DSPMode mode) {
    if (m_receiver != rx) return;
    setDSPMode(mode);
}

void RadioPopupWidget::filterGroupChanged(DSPMode mode) {
    if (mode == LSB || mode == USB || mode == DIGU || mode == DIGL) {
        m_filterStackedWidget->setCurrentIndex(0);
    } else if (mode == DSB || mode == FMN || mode == AM || mode == SAM) {
        m_filterStackedWidget->setCurrentIndex(1);
    } else if (mode == CWL || mode == CWU) {
        m_filterStackedWidget->setCurrentIndex(2);
    }
}

float RadioPopupWidget::getVarWidth(int groupIdx, int varIndex) const {
    if (groupIdx == 0) return (varIndex == 10) ? m_var1WidthA : m_var2WidthA;
    if (groupIdx == 1) return (varIndex == 10) ? m_var1WidthB : m_var2WidthB;
    if (groupIdx == 2) return (varIndex == 10) ? m_var1WidthC : m_var2WidthC;
    return 2400.0f;
}

void RadioPopupWidget::setVarWidth(int groupIdx, int varIndex, float width) {
    if (groupIdx == 0) {
        if (varIndex == 10) m_var1WidthA = width; else m_var2WidthA = width;
    } else if (groupIdx == 1) {
        if (varIndex == 10) m_var1WidthB = width; else m_var2WidthB = width;
    } else if (groupIdx == 2) {
        if (varIndex == 10) m_var1WidthC = width; else m_var2WidthC = width;
    }
}

void RadioPopupWidget::varFilterSliderValueChanged(int value) {
    if (!m_varFilterContainer || !m_varFilterContainer->isVisible()) return;

    DSPMode mode = m_dspModeList.at(m_hamBand);
    int groupIdx = -1;
    if (mode == LSB || mode == USB || mode == DIGU || mode == DIGL) groupIdx = 0;
    else if (mode == DSB || mode == FMN || mode == AM || mode == SAM) groupIdx = 1;
    else if (mode == CWL || mode == CWU) groupIdx = 2;

    if (groupIdx == -1) return;

    QList<AeroButton *> *activeList = nullptr;
    if (groupIdx == 0) activeList = &filterBtnListA;
    else if (groupIdx == 1) activeList = &filterBtnListB;
    else if (groupIdx == 2) activeList = &filterBtnListC;

    if (!activeList) return;

    int varIndex = -1;
    if (activeList->at(10)->btnState() == AeroButton::ON) varIndex = 10;
    else if (activeList->at(11)->btnState() == AeroButton::ON) varIndex = 11;

    if (varIndex == -1) return;

    float filterWidth = static_cast<float>(value);
    setVarWidth(groupIdx, varIndex, filterWidth);

    m_varFilterLabel->setText(QString("Var %1: %2 Hz").arg(varIndex == 10 ? 1 : 2).arg(value));

    if (groupIdx == 0) { // Group A: SSB/Data
        if (mode == LSB || mode == DIGL) { m_filterLo = -(filterWidth + 150.0f); m_filterHi = -150.0f; }
        else { m_filterLo = 150.0f; m_filterHi = filterWidth + 150.0f; }
    } else if (groupIdx == 1) { // Group B: Wide
        if (mode == FMN) { m_filterLo = -2000.0f; m_filterHi = 2000.0f; }
        else { m_filterLo = -filterWidth / 2.0f; m_filterHi = filterWidth / 2.0f; }
    } else if (groupIdx == 2) { // Group C: CW
        if (mode == CWL) { m_filterLo = -(filterWidth + 100.0f); m_filterHi = -100.0f; }
        else { m_filterLo = 100.0f; m_filterHi = filterWidth + 100.0f; }
    }

    emit filterFrequenciesRequested(m_receiver, m_filterLo, m_filterHi);
}

void RadioPopupWidget::filterChangedByBtn() {
    AeroButton *button = qobject_cast<AeroButton *>(sender());
    if (!button) return;

    QList<AeroButton *> btnList;
    int groupIdx = -1;

    if (filterBtnListA.contains(button)) { btnList = filterBtnListA; groupIdx = 0; }
    else if (filterBtnListB.contains(button)) { btnList = filterBtnListB; groupIdx = 1; }
    else if (filterBtnListC.contains(button)) { btnList = filterBtnListC; groupIdx = 2; }

    if (groupIdx == -1) return;

    int btnIndex = btnList.indexOf(button);
    if (btnIndex == -1) return;

    for (AeroButton *btn : btnList) {
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    button->setBtnState(AeroButton::ON);
    button->update();
    m_activeFilterIndex = btnIndex;

    qreal filterWidth = 0.0f;
    if (btnIndex == 10 || btnIndex == 11) { // Var 1 or Var 2
        filterWidth = getVarWidth(groupIdx, btnIndex);

        int minW = 100, maxW = 5000;
        if (groupIdx == 1) { minW = 1000; maxW = 20000; }
        else if (groupIdx == 2) { minW = 25; maxW = 1500; }

        m_varFilterSlider->blockSignals(true);
        m_varFilterSlider->setRange(minW, maxW);
        m_varFilterSlider->setValue(static_cast<int>(filterWidth));
        m_varFilterSlider->blockSignals(false);

        m_varFilterLabel->setText(QString("Var %1: %2 Hz").arg(btnIndex == 10 ? 1 : 2).arg(static_cast<int>(filterWidth)));
        m_varFilterContainer->setVisible(true);
    } else {
        m_varFilterContainer->setVisible(false);
        float widths[] = {1000, 1800, 2100, 2400, 2700, 2900, 3300, 3800, 4400, 5000,
                          16000, 12000, 10000, 8000, 6600, 5200, 4000, 3100, 2900, 2400,
                          1000, 800, 750, 600, 500, 400, 250, 100, 50, 25};
        filterWidth = widths[groupIdx * 10 + btnIndex];
    }

    DSPMode mode = m_dspModeList.at(m_hamBand);
    if (groupIdx == 0) { // Group A: SSB/Data
        if (mode == LSB || mode == DIGL) { m_filterLo = -(filterWidth + 150.0f); m_filterHi = -150.0f; }
        else { m_filterLo = 150.0f; m_filterHi = filterWidth + 150.0f; }
    } else if (groupIdx == 1) { // Group B: Wide
        if (mode == FMN) { m_filterLo = -2000.0f; m_filterHi = 2000.0f; }
        else { m_filterLo = -filterWidth/2.0f; m_filterHi = filterWidth/2.0f; }
    } else if (groupIdx == 2) { // Group C: CW
        if (mode == CWL) { m_filterLo = -(filterWidth + 100.0f); m_filterHi = -100.0f; }
        else { m_filterLo = 100.0f; m_filterHi = filterWidth + 100.0f; }
    }
    
    emit filterFrequenciesRequested(m_receiver, m_filterLo, m_filterHi);
}

void RadioPopupWidget::filterChanged(int rx, qreal low, qreal high) {
    if (m_receiver != rx) return;
    m_filterLo = low;
    m_filterHi = high;

    DSPMode mode = m_dspModeList.at(m_hamBand);
    QList<AeroButton *> *activeList = nullptr;
    int groupIdx = -1;

    if (mode == LSB || mode == USB || mode == DIGU || mode == DIGL) { activeList = &filterBtnListA; groupIdx = 0; }
    else if (mode == DSB || mode == FMN || mode == AM || mode == SAM) { activeList = &filterBtnListB; groupIdx = 1; }
    else if (mode == CWL || mode == CWU) { activeList = &filterBtnListC; groupIdx = 2; }

    if (!activeList) return;

    for (AeroButton *btn : *activeList) {
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    if (m_activeFilterIndex == 10 || m_activeFilterIndex == 11) {
        // Var 1 or Var 2 explicitly active
        activeList->at(m_activeFilterIndex)->setBtnState(AeroButton::ON);
        activeList->at(m_activeFilterIndex)->update();

        float customW = static_cast<float>(qAbs(m_filterHi - m_filterLo));
        setVarWidth(groupIdx, m_activeFilterIndex, customW);

        int minW = 100, maxW = 5000;
        if (groupIdx == 1) { minW = 1000; maxW = 20000; }
        else if (groupIdx == 2) { minW = 25; maxW = 1500; }

        m_varFilterSlider->blockSignals(true);
        m_varFilterSlider->setRange(minW, maxW);
        m_varFilterSlider->setValue(static_cast<int>(customW));
        m_varFilterSlider->blockSignals(false);

        m_varFilterLabel->setText(QString("Var %1: %2 Hz").arg(m_activeFilterIndex == 10 ? 1 : 2).arg(static_cast<int>(customW)));
        m_varFilterContainer->setVisible(true);
    } else if (m_activeFilterIndex >= 0 && m_activeFilterIndex < 10) {
        // Fixed preset explicitly active
        activeList->at(m_activeFilterIndex)->setBtnState(AeroButton::ON);
        activeList->at(m_activeFilterIndex)->update();
        m_varFilterContainer->setVisible(false);
    } else {
        // Uninitialized m_activeFilterIndex (-1): match by frequency
        float widths[] = {1000, 1800, 2100, 2400, 2700, 2900, 3300, 3800, 4400, 5000,
                          16000, 12000, 10000, 8000, 6600, 5200, 4000, 3100, 2900, 2400,
                          1000, 800, 750, 600, 500, 400, 250, 100, 50, 25};

        bool matchedPreset = false;
        for (int i = 0; i < 10; ++i) {
            float w = widths[groupIdx * 10 + i];
            bool match = false;
            if (groupIdx == 0) {
                match = (qAbs(m_filterLo - (-(w + 150.0f))) < 2.0f && qAbs(m_filterHi - (-150.0f)) < 2.0f) ||
                        (qAbs(m_filterLo - 150.0f) < 2.0f && qAbs(m_filterHi - (w + 150.0f)) < 2.0f);
            } else if (groupIdx == 1) {
                if (mode == FMN) match = (qAbs(m_filterLo - (-2000.0f)) < 2.0f && qAbs(m_filterHi - 2000.0f) < 2.0f);
                else match = (qAbs(m_filterLo - (-w/2.0f)) < 2.0f && qAbs(m_filterHi - w/2.0f) < 2.0f);
            } else if (groupIdx == 2) {
                match = (qAbs(m_filterLo - (-(w + 100.0f))) < 2.0f && qAbs(m_filterHi - (-100.0f)) < 2.0f) ||
                        (qAbs(m_filterLo - 100.0f) < 2.0f && qAbs(m_filterHi - (w + 100.0f)) < 2.0f);
            }

            if (match) {
                m_activeFilterIndex = i;
                activeList->at(i)->setBtnState(AeroButton::ON);
                activeList->at(i)->update();
                matchedPreset = true;
                break;
            }
        }

        if (!matchedPreset) {
            m_activeFilterIndex = 10;
            activeList->at(10)->setBtnState(AeroButton::ON);
            activeList->at(10)->update();
            m_varFilterContainer->setVisible(true);
        } else {
            m_varFilterContainer->setVisible(false);
        }
    }
}

void RadioPopupWidget::adcModeChangedByBtn() {
    AeroButton *button = qobject_cast<AeroButton *>(sender());
    int btn = adcModeBtnList.indexOf(button);
    if (btn == -1) return;

    for(AeroButton *b : adcModeBtnList) {
        b->setBtnState(AeroButton::OFF);
        b->update();
    }

    ADCMode newMode = static_cast<ADCMode>(btn);
    emit adcModeRequested(m_receiver, newMode);

    button->setBtnState(AeroButton::ON);
    button->update();
}

void RadioPopupWidget::adcModeChanged(int rx, ADCMode mode) {
    if (m_receiver != rx) return;
    m_adcMode = mode;

    for(AeroButton *btn : adcModeBtnList) {
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    adcModeBtnList.at(mode)->setBtnState(AeroButton::ON);
    adcModeBtnList.at(mode)->update();
}

void RadioPopupWidget::agcModeChangedByBtn() {
    AeroButton *button = qobject_cast<AeroButton *>(sender());
    int btn = agcModeBtnList.indexOf(button);
    if (btn == -1) return;

    for(AeroButton *b : agcModeBtnList) {
        b->setBtnState(AeroButton::OFF);
        b->update();
    }

    AGCMode newMode = static_cast<AGCMode>(btn);
    emit agcModeRequested(m_receiver, newMode);

    button->setBtnState(AeroButton::ON);
    button->update();
}

void RadioPopupWidget::agcModeChanged(int rx, AGCMode mode, bool hang) {
    Q_UNUSED(hang)

    if (m_receiver != rx) return;
    m_agcMode = mode;

    for(AeroButton *btn : agcModeBtnList) {
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    agcModeBtnList.at(mode)->setBtnState(AeroButton::ON);
    agcModeBtnList.at(mode)->update();
}

void RadioPopupWidget::agcShowLinesChanged() {
    if (showAGCLines->btnState() == AeroButton::OFF) {
        showAGCLines->setBtnState(AeroButton::ON);
        emit agcShowLinesRequested(m_receiver, true);
    }
    else {
        showAGCLines->setBtnState(AeroButton::OFF);
        emit agcShowLinesRequested(m_receiver, false);
    }
}

void RadioPopupWidget::avgBtnClicked() {
    if (avgBtn->btnState() == AeroButton::OFF) {
        avgBtn->setBtnState(AeroButton::ON);
        emit spectrumAveragingRequested(m_receiver, true);
    }
    else {
        avgBtn->setBtnState(AeroButton::OFF);
        emit spectrumAveragingRequested(m_receiver, false);
    }
}

void RadioPopupWidget::gridBtnClicked() {
    if (gridBtn->btnState() == AeroButton::OFF) {
        gridBtn->setBtnState(AeroButton::ON);
        emit panGridRequested(m_receiver, true);
    }
    else {
        gridBtn->setBtnState(AeroButton::OFF);
        emit panGridRequested(m_receiver, false);
    }
}

void RadioPopupWidget::peakHoldBtnClicked() {
    if (peakHoldBtn->btnState() == AeroButton::OFF) {
        peakHoldBtn->setBtnState(AeroButton::ON);
        emit peakHoldRequested(m_receiver, true);
    }
    else {
        peakHoldBtn->setBtnState(AeroButton::OFF);
        emit peakHoldRequested(m_receiver, false);
    }
}

void RadioPopupWidget::panLockedBtnClicked() {
    if (lockPanBtn->btnState() == AeroButton::OFF) {
        lockPanBtn->setBtnState(AeroButton::ON);
        emit panLockedRequested(m_receiver, true);
    }
    else {
        lockPanBtn->setBtnState(AeroButton::OFF);
        emit panLockedRequested(m_receiver, false);
    }
}

void RadioPopupWidget::clickVfoBtnClicked() {
    if (clickVfoBtn->btnState() == AeroButton::OFF) {
        clickVfoBtn->setBtnState(AeroButton::ON);
        emit clickVFORequested(m_receiver, true);
    }
    else {
        clickVfoBtn->setBtnState(AeroButton::OFF);
        emit clickVFORequested(m_receiver, false);
    }
}

void RadioPopupWidget::hairCrossBtnClicked() {
    if (showCrossBtn->btnState() == AeroButton::OFF) {
        showCrossBtn->setBtnState(AeroButton::ON);
        emit hairCrossRequested(m_receiver, true);
    }
    else {
        showCrossBtn->setBtnState(AeroButton::OFF);
        emit hairCrossRequested(m_receiver, false);
    }
}

void RadioPopupWidget::midToVfoBtnClicked() {
    emit midToVfoBtnEvent();
}

void RadioPopupWidget::vfoToMidBtnClicked() {
    emit vfoToMidBtnEvent();
}

void RadioPopupWidget::updateActiveVfoButtons() {
    if (!vfoABtn || !vfoBBtn)
        return;
    const bool bActive = m_sliceModel && m_sliceModel->activeVfo() == SliceModel::VfoB;
    vfoABtn->setBtnState(bActive ? AeroButton::OFF : AeroButton::ON);
    vfoBBtn->setBtnState(bActive ? AeroButton::ON : AeroButton::OFF);
}

void RadioPopupWidget::vfoABtnClicked() {
    if (!m_sliceModel)
        return;
    m_sliceModel->setActiveVfo(SliceModel::VfoA);
    Settings::instance()->setVfoFrequencyVisible(m_receiver, m_sliceModel->frequency());
    updateActiveVfoButtons();
}

void RadioPopupWidget::vfoBBtnClicked() {
    if (!m_sliceModel)
        return;
    m_sliceModel->setActiveVfo(SliceModel::VfoB);
    Settings::instance()->setVfoFrequencyVisible(m_receiver, m_sliceModel->frequency());
    updateActiveVfoButtons();
}

void RadioPopupWidget::vfoAtoBBtnClicked() {
    if (!m_sliceModel)
        return;
    m_sliceModel->copyAtoB();
    if (m_sliceModel->activeVfo() == SliceModel::VfoB)
        Settings::instance()->setVfoFrequencyVisible(m_receiver, m_sliceModel->frequency());
}

void RadioPopupWidget::vfoBtoABtnClicked() {
    if (!m_sliceModel)
        return;
    m_sliceModel->copyBtoA();
    if (m_sliceModel->activeVfo() == SliceModel::VfoA)
        Settings::instance()->setVfoFrequencyVisible(m_receiver, m_sliceModel->frequency());
}

void RadioPopupWidget::vfoSwapBtnClicked() {
    if (!m_sliceModel)
        return;
    m_sliceModel->swapVfos();
    Settings::instance()->setVfoFrequencyVisible(m_receiver, m_sliceModel->frequency());
}

void RadioPopupWidget::loadReceiverState(int rx) {
    Q_UNUSED(rx)
    m_hamBand = gen;
    m_dspModeList.clear();
    for (int i = 0; i < 30; ++i) {
        m_dspModeList.append(LSB);
    }
    m_adcMode = adc1;
    m_agcMode = _agcMode::agcLONG;
    m_filterMode = filterLSB;
    m_filterLo = -2700;
    m_filterHi = -150;
    m_spectrumAveraging = false;
    m_panGrid = true;
    m_peakHold = false;
    m_panLocked = false;
    m_clickVFO = true;
    m_showCross = false;
    m_panadapterMode = Line;
    m_waterfallColorMode = Simple;
}

void RadioPopupWidget::setCurrentReceiver(int value) {
    Q_UNUSED(value)
}

void RadioPopupWidget::setSticky() {
    if (stickyBtn->btnState() == AeroButton::OFF) {
        stickyBtn->setBtnState(AeroButton::ON);
        stickyBtn->setText("Unlock");
        m_sticky = true;
    }
    else {
        stickyBtn->setBtnState(AeroButton::OFF);
        stickyBtn->setText("Lock");
        m_sticky = false;
    }
}

void RadioPopupWidget::panModeChanged() {
    AeroButton *button = qobject_cast<AeroButton *>(sender());
    int btnHit = panadapterBtnList.indexOf(button);

    for(AeroButton *btn : panadapterBtnList) {
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    button->setBtnState(AeroButton::ON);
    button->update();

    switch (btnHit) {
    case 0: m_panadapterMode = PanGraphicsMode::Line; break;
    case 1: m_panadapterMode = PanGraphicsMode::FilledLine; break;
    case 2: m_panadapterMode = PanGraphicsMode::Solid; break;
    }
    emit graphicsStateRequested(m_receiver, m_panadapterMode, m_waterfallColorMode);
}

void RadioPopupWidget::waterfallModeChanged() {
    AeroButton *button = qobject_cast<AeroButton *>(sender());
    int btnHit = waterfallBtnList.indexOf(button);

    for(AeroButton *btn : waterfallBtnList) {
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    button->setBtnState(AeroButton::ON);
    button->update();

    switch (btnHit) {
    case 0: m_waterfallColorMode = WaterfallColorMode::Simple; break;
    case 1: m_waterfallColorMode = WaterfallColorMode::Enhanced; break;
    }
    emit graphicsStateRequested(m_receiver, m_panadapterMode, m_waterfallColorMode);
}

// **********************
bool RadioPopupWidget::showPopupWidget(QPoint position) {
    if (s_lastRadioPopupWidth < 0)
        s_lastRadioPopupWidth = loadSavedPopupWidth();

    const bool hasStoredWidth = (s_lastRadioPopupWidth > 0);
    show();

    if (hasStoredWidth) {
        int targetWidth = s_lastRadioPopupWidth;
        if (targetWidth < m_minimumWidgetWidth) targetWidth = m_minimumWidgetWidth;
        resize(targetWidth, height());
    }
    else {
        const int side = qMax(height(), m_minimumWidgetWidth);
        resize(side, side);
        s_lastRadioPopupWidth = side;
    }

    // NOTE: Using QGuiApplication::primaryScreen() instead of obsolete QDesktopWidget
    QRect desktopRect = QGuiApplication::primaryScreen()->availableGeometry();

    position.setX(position.x() - frameGeometry().width() / 2);
    position.setY(position.y() - frameGeometry().height() / 2);

    move(position);

    // stop us being lost off the edge of the screen
    if (frameGeometry().right() > desktopRect.right()) move(QPoint(desktopRect.right() - frameGeometry().width(), frameGeometry().top()));
    if (frameGeometry().bottom() > desktopRect.bottom()) move(QPoint(frameGeometry().left(), desktopRect.bottom() - frameGeometry().height()));
    if (frameGeometry().left() < desktopRect.left()) move(QPoint(desktopRect.left(), frameGeometry().top()));
    if (frameGeometry().top() < desktopRect.top()) move(QPoint(frameGeometry().left(), desktopRect.top()));

    setFocus();
    return true;
}

void RadioPopupWidget::systemStateChanged(
    QSDR::_Error err,
    QSDR::_HWInterfaceMode hwmode,
    QSDR::_ServerMode mode,
    QSDR::_DataEngineState state
    ) {
    Q_UNUSED(err)
    Q_UNUSED(hwmode)
    Q_UNUSED(mode)
    Q_UNUSED(state)

    updateAdcAvailability();
}

void RadioPopupWidget::graphicModeChanged(
    int rx,
    PanGraphicsMode panMode,
    WaterfallColorMode waterfallMode)
{
    Q_UNUSED (rx)

    bool change = false;
    if (m_panadapterMode != panMode) {
        m_panadapterMode = panMode;
        change = true;
    }
    if (m_waterfallColorMode != waterfallMode) {
        m_waterfallColorMode = waterfallMode;
        change = true;
    }
    if (!change) return;
    update();
}

void RadioPopupWidget::closeEvent(QCloseEvent *event) {
    emit closeEvent();
    QWidget::closeEvent(event);
}

void RadioPopupWidget::showEvent(QShowEvent *event) {
    m_closeTimer->start();
    if (m_rxEqPlot) {
        QVector<double> X(AudioConfig::kEqDrawPoints, 0.0);
        QVector<double> Y(AudioConfig::kEqDrawPoints, 0.0);
        GetRXAEQDraw(m_receiver, X.data(), Y.data());
        m_rxEqPlot->setCurve(X, Y);
    }
    QWidget::showEvent(event);
}

void RadioPopupWidget::hideEvent(QHideEvent *event) {
    QWidget::hideEvent(event);
}

void RadioPopupWidget::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
}

void RadioPopupWidget::resizeEvent(QResizeEvent *event) {
    s_lastRadioPopupWidth = width();
    savePopupWidth(s_lastRadioPopupWidth);
    createBackground(event->size());
    QWidget::resizeEvent(event);
}

void RadioPopupWidget::mousePressEvent(QMouseEvent *event) {
    m_closeTimer->start();
    m_mouseDownPos = QCursor::pos();
    m_mouseDownWindowPos = pos();
    QWidget::mousePressEvent(event);
}

void RadioPopupWidget::mouseMoveEvent(QMouseEvent *event) {
    m_closeTimer->start();
    if (event->buttons() == Qt::LeftButton) {
        QPoint delta = QCursor::pos() - m_mouseDownPos;
        QPoint new_pos = m_mouseDownWindowPos + delta;

        // NOTE: Using QGuiApplication::primaryScreen() instead of obsolete QDesktopWidget
        QRect desktopRect = QGuiApplication::primaryScreen()->availableGeometry();
        QRect new_rect(QPoint(0, 0), size());
        new_rect.moveTopLeft(new_pos);

        // stop us being lost off the edge of the screen
        if (new_rect.right() > desktopRect.right()) new_rect.moveLeft(desktopRect.right() - (new_rect.width() - 1));
        if (new_rect.bottom() > desktopRect.bottom()) new_rect.moveTop(desktopRect.bottom() - (new_rect.height() - 1));
        if (new_rect.left() < desktopRect.left()) new_rect.moveLeft(desktopRect.left());
        if (new_rect.top() < desktopRect.top()) new_rect.moveTop(desktopRect.top());

        move(new_rect.topLeft());
    }
    QWidget::mouseMoveEvent(event);
}

void RadioPopupWidget::mouseReleaseEvent(QMouseEvent *event) {
    m_closeTimer->start();
    QWidget::mouseReleaseEvent(event);
}

void RadioPopupWidget::enterEvent(QEnterEvent *event) {
    // Stop the timer if the user moves the mouse back over the widget
    m_closeTimer->stop();
    QWidget::enterEvent(event);
}

void RadioPopupWidget::leaveEvent(QEnterEvent *event) {
    if (!m_sticky) {
        // Start the countdown to close the widget
        m_closeTimer->start();
    }
    QWidget::leaveEvent(event);
}


bool RadioPopupWidget::event(QEvent *event) {
        switch (event->type())
        {
        // When the user presses or drags the window frame,
        // just stop the auto-close timer.
        case QEvent::NonClientAreaMouseButtonPress:
        case QEvent::NonClientAreaMouseMove:
            m_closeTimer->stop();
            break;

        default:
            break;
        }
    return QWidget::event(event);
}

void RadioPopupWidget::createBackground(QSize size) {
    if (size.width() <= 0 || size.height() <= 0) return;

    QFont font(this->font());
    font.setPixelSize(12);

    QFontMetrics fm(font);
    int font_height = fm.height();

    QImage image = QImage(size, QImage::Format_ARGB32_Premultiplied);
    if (image.isNull()) return;

    image.fill(QColor(0, 0, 0, 0).rgba());

    QPainter painter(&image);
    painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    painter.setFont(font);

    painter.fillRect(image.rect(), QColor(35, 35, 35));

    QString titleStr = windowTitle();
    QRect title_bar_rect(0, 0, image.width(), font_height + 6);

    QLinearGradient title_bar_gradient(0, 0, 0, 1);
    title_bar_gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    title_bar_gradient.setSpread(QGradient::PadSpread);
    title_bar_gradient.setColorAt(0.0, QColor(100, 110, 128));
    title_bar_gradient.setColorAt(0.4, QColor(74, 80, 90));
    title_bar_gradient.setColorAt(0.6, QColor(56, 62, 70));
    title_bar_gradient.setColorAt(1.0, QColor(48, 56, 64));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(title_bar_gradient));

    title_bar_rect.adjust(1, 1, 1, 1);
    painter.setPen(Qt::black);
    painter.drawText(title_bar_rect, Qt::TextSingleLine | Qt::TextDontClip | Qt::AlignVCenter | Qt::AlignLeft, titleStr);
    title_bar_rect.adjust(-1, -1, -1, -1);
    painter.setPen(Qt::white);
    painter.drawText(title_bar_rect, Qt::TextSingleLine | Qt::TextDontClip | Qt::AlignVCenter | Qt::AlignLeft, titleStr);

    painter.setPen(QColor(255, 255, 255, 64));
    painter.drawLine(0, 0, image.width() - 1, 0);											// top line
    painter.drawLine(0, 0, 0, image.height() - 1);											// left line
    painter.setPen(QColor(0, 0, 0, 64));
    painter.drawLine(0, image.height() - 1, image.width() - 1, image.height() - 1);			// bottom line
    painter.drawLine(image.width() - 1, 0, image.width() - 1, image.height() - 1);			// right line

    painter.end();

    QPalette palette;
    palette.setBrush(backgroundRole(), QBrush(image));
    setPalette(palette);
    setAutoFillBackground(true);
}

void RadioPopupWidget::setSingleAdcDevice(bool single) {
    m_singleAdcDevice = single;
    if (adc2Btn) {
        adc2Btn->setEnabled(!single);
    }
}

void RadioPopupWidget::setBandFrequencyList(const QList<THamBandFrequencies>& list) {
    m_bandFrequencyList = list;
}

void RadioPopupWidget::setHamBand(HamBand band) {
    m_hamBand = band;
    for(AeroButton *btn : bandBtnList) {
        btn->blockSignals(true);
        btn->setBtnState(AeroButton::OFF);
        btn->blockSignals(false);
        btn->update();
    }
    if (static_cast<int>(band) >= 0 && static_cast<int>(band) < bandBtnList.size()) {
        bandBtnList.at(static_cast<int>(band))->blockSignals(true);
        bandBtnList.at(static_cast<int>(band))->setBtnState(AeroButton::ON);
        bandBtnList.at(static_cast<int>(band))->blockSignals(false);
        bandBtnList.at(static_cast<int>(band))->update();
    }
    updateFreeDVControls();
}

void RadioPopupWidget::setDSPModeList(const QList<DSPMode>& list) {
    m_dspModeList = list;
    if (static_cast<int>(m_hamBand) >= 0 && static_cast<int>(m_hamBand) < m_dspModeList.size()) {
        setDSPMode(m_dspModeList.at(static_cast<int>(m_hamBand)));
    }
}

void RadioPopupWidget::setDSPMode(DSPMode mode) {
    if (static_cast<int>(m_hamBand) >= 0 && static_cast<int>(m_hamBand) < m_dspModeList.size()) {
        m_dspModeList[m_hamBand] = mode;
    }

    for (AeroButton *btn : dspModeBtnList) {
        btn->blockSignals(true);
        btn->setBtnState(AeroButton::OFF);
        btn->blockSignals(false);
        btn->update();
    }

    const int modeIdx = static_cast<int>(mode);
    if (modeIdx >= 0 && modeIdx < dspModeBtnList.size()) {
        dspModeBtnList.at(modeIdx)->blockSignals(true);
        dspModeBtnList.at(modeIdx)->setBtnState(AeroButton::ON);
        dspModeBtnList.at(modeIdx)->blockSignals(false);
        dspModeBtnList.at(modeIdx)->update();
    }

    filterGroupChanged(mode);
    updateFreeDVControls();
}

void RadioPopupWidget::setCtrFrequency(qint64 frequency) {
    m_ctrFrequency = frequency;
    if (m_bandFrequencyList.isEmpty() || m_lastCtrFrequencyList.isEmpty()) {
        return;
    }
    HamBand band = getBandFromFrequency(m_bandFrequencyList, frequency);
    const int bandIdx = static_cast<int>(band);
    if (bandIdx >= 0 && bandIdx < m_lastCtrFrequencyList.size()) {
        m_lastCtrFrequencyList[bandIdx] = m_ctrFrequency;
    }
}

void RadioPopupWidget::setVfoFrequency(qint64 frequency) {
    m_vfoFrequency = frequency;
    if (m_bandFrequencyList.isEmpty() || m_lastVfoFrequencyList.isEmpty()) {
        return;
    }
    HamBand band = getBandFromFrequency(m_bandFrequencyList, frequency);
    const int bandIdx = static_cast<int>(band);
    if (bandIdx >= 0 && bandIdx < m_lastVfoFrequencyList.size()) {
        m_lastVfoFrequencyList[bandIdx] = m_vfoFrequency;
    }
}

void RadioPopupWidget::setADCMode(ADCMode mode) {
    m_adcMode = mode;
    for(AeroButton *btn : adcModeBtnList) {
        btn->blockSignals(true);
        btn->setBtnState(AeroButton::OFF);
        btn->blockSignals(false);
        btn->update();
    }
    if (static_cast<int>(mode) >= 0 && static_cast<int>(mode) < adcModeBtnList.size()) {
        adcModeBtnList.at(static_cast<int>(mode))->blockSignals(true);
        adcModeBtnList.at(static_cast<int>(mode))->setBtnState(AeroButton::ON);
        adcModeBtnList.at(static_cast<int>(mode))->blockSignals(false);
        adcModeBtnList.at(static_cast<int>(mode))->update();
    }
}

void RadioPopupWidget::setAGCMode(AGCMode mode) {
    m_agcMode = mode;
    for(AeroButton *btn : agcModeBtnList) {
        btn->blockSignals(true);
        btn->setBtnState(AeroButton::OFF);
        btn->blockSignals(false);
        btn->update();
    }
    if (static_cast<int>(mode) >= 0 && static_cast<int>(mode) < agcModeBtnList.size()) {
        agcModeBtnList.at(static_cast<int>(mode))->blockSignals(true);
        agcModeBtnList.at(static_cast<int>(mode))->setBtnState(AeroButton::ON);
        agcModeBtnList.at(static_cast<int>(mode))->blockSignals(false);
        agcModeBtnList.at(static_cast<int>(mode))->update();
    }
}

void RadioPopupWidget::setDefaultFilterMode(TDefaultFilterMode mode) {
    m_filterMode = mode;
}

void RadioPopupWidget::setFilterFrequencies(qreal low, qreal high) {
    filterChanged(m_receiver, low, high);
}

void RadioPopupWidget::setSpectrumAveraging(bool enabled) {
    m_spectrumAveraging = enabled;
    if (avgBtn) {
        avgBtn->blockSignals(true);
        avgBtn->setBtnState(enabled ? AeroButton::ON : AeroButton::OFF);
        avgBtn->blockSignals(false);
        avgBtn->update();
    }
}

void RadioPopupWidget::setPanGrid(bool enabled) {
    m_panGrid = enabled;
    if (gridBtn) {
        gridBtn->blockSignals(true);
        gridBtn->setBtnState(enabled ? AeroButton::ON : AeroButton::OFF);
        gridBtn->blockSignals(false);
        gridBtn->update();
    }
}

void RadioPopupWidget::setPeakHold(bool enabled) {
    m_peakHold = enabled;
    if (peakHoldBtn) {
        peakHoldBtn->blockSignals(true);
        peakHoldBtn->setBtnState(enabled ? AeroButton::ON : AeroButton::OFF);
        peakHoldBtn->blockSignals(false);
        peakHoldBtn->update();
    }
}

void RadioPopupWidget::setPanLocked(bool enabled) {
    m_panLocked = enabled;
    if (lockPanBtn) {
        lockPanBtn->blockSignals(true);
        lockPanBtn->setBtnState(enabled ? AeroButton::ON : AeroButton::OFF);
        lockPanBtn->blockSignals(false);
        lockPanBtn->update();
    }
}

void RadioPopupWidget::setClickVFO(bool enabled) {
    m_clickVFO = enabled;
    if (clickVfoBtn) {
        clickVfoBtn->blockSignals(true);
        clickVfoBtn->setBtnState(enabled ? AeroButton::ON : AeroButton::OFF);
        clickVfoBtn->blockSignals(false);
        clickVfoBtn->update();
    }
}

void RadioPopupWidget::setHairCross(bool enabled) {
    m_showCross = enabled;
    if (showCrossBtn) {
        showCrossBtn->blockSignals(true);
        showCrossBtn->setBtnState(enabled ? AeroButton::ON : AeroButton::OFF);
        showCrossBtn->blockSignals(false);
        showCrossBtn->update();
    }
}

void RadioPopupWidget::setPanadapterMode(PanGraphicsMode mode) {
    m_panadapterMode = mode;
    for(AeroButton *btn : panadapterBtnList) {
        btn->blockSignals(true);
        btn->setBtnState(AeroButton::OFF);
        btn->blockSignals(false);
        btn->update();
    }
    if (static_cast<int>(mode) >= 0 && static_cast<int>(mode) < panadapterBtnList.size()) {
        panadapterBtnList.at(static_cast<int>(mode))->blockSignals(true);
        panadapterBtnList.at(static_cast<int>(mode))->setBtnState(AeroButton::ON);
        panadapterBtnList.at(static_cast<int>(mode))->blockSignals(false);
        panadapterBtnList.at(static_cast<int>(mode))->update();
    }
}

void RadioPopupWidget::setWaterfallColorMode(WaterfallColorMode mode) {
    m_waterfallColorMode = mode;
    for(AeroButton *btn : waterfallBtnList) {
        btn->blockSignals(true);
        btn->setBtnState(AeroButton::OFF);
        btn->blockSignals(false);
        btn->update();
    }
    if (static_cast<int>(mode) >= 0 && static_cast<int>(mode) < waterfallBtnList.size()) {
        waterfallBtnList.at(static_cast<int>(mode))->blockSignals(true);
        waterfallBtnList.at(static_cast<int>(mode))->setBtnState(AeroButton::ON);
        waterfallBtnList.at(static_cast<int>(mode))->blockSignals(false);
        waterfallBtnList.at(static_cast<int>(mode))->update();
    }
}

void RadioPopupWidget::setLastFrequencies(const QList<qint64>& ctrFreqs, const QList<qint64>& vfoFreqs) {
    m_lastCtrFrequencyList = ctrFreqs;
    m_lastVfoFrequencyList = vfoFreqs;
}

void RadioPopupWidget::setFreeDVMode(int mode) {
    if (m_freeDVModeCombo) {
        const int idx = m_freeDVModeCombo->findData(mode);
        if (idx >= 0) {
            m_freeDVModeCombo->blockSignals(true);
            m_freeDVModeCombo->setCurrentIndex(idx);
            m_freeDVModeCombo->blockSignals(false);
        }
    }
}

void RadioPopupWidget::setFreeDVStatus(bool sync, float snr, quint64 rxFrames, quint64 txFrames) {
    if (!m_freeDVStatusLabel) return;
    if (sync) {
        m_freeDVStatusLabel->setText(QString("FreeDV: sync, SNR: %1 dB, rx: %2, tx: %3")
                                         .arg(snr, 0, 'f', 1)
                                         .arg(rxFrames)
                                         .arg(txFrames));
        m_freeDVStatusLabel->setStyleSheet("color: green; font-weight: bold;");
    } else {
        m_freeDVStatusLabel->setText("FreeDV: no sync");
        m_freeDVStatusLabel->setStyleSheet("");
    }
}

void RadioPopupWidget::setAGCShowLines(bool enabled) {
    if (showAGCLines) {
        showAGCLines->blockSignals(true);
        showAGCLines->setBtnState(enabled ? AeroButton::ON : AeroButton::OFF);
        showAGCLines->blockSignals(false);
        showAGCLines->update();
    }
}
