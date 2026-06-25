/**
* @file cusdr_radioWidget.cpp
* @brief Radio control widget class for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2011-02-10
*/

/*
 *   Copyright 2010, 2011 Hermann von Hasseln, DL3HVH
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
 
#include <QtGui>
#include <QDebug>
#include <QGroupBox>
#include <QBoxLayout>

#include "cusdr_radioWidget.h"

#define	btn_height		14
#define	btn_width		60
#define	btn_widthb		70
#define	btn_widths		34


RadioWidget::RadioWidget(QWidget *parent) 
	: QWidget(parent)
	, set(Settings::instance())
	, m_currentRx(set->getCurrentReceiver())
    , m_minimumWidgetWidth(set->getMinimumWidgetWidth())
	, m_minimumGroupBoxWidth(set->getMinimumGroupBoxWidth())
{
	//setMinimumWidth(m_minimumWidgetWidth);
	setContentsMargins(4, 0, 4, 0);
	
	m_hamBand = set->getCurrentHamBand(0);
	m_dspModeList = set->getDSPModeList(0);
	if (m_hamBand >= 0 && m_hamBand < m_dspModeList.size())
		m_dspModeList[m_hamBand] = set->getDSPMode(0);
	m_filterMode = set->getDefaultFilterMode(0);
	m_filterLo = set->getFilterLo(0);
	m_filterHi = set->getFilterHi(0);

	m_lastCtrFrequencyList = set->getLastCenterFrequencyList(0);
	m_lastVfoFrequencyList = set->getLastVfoFrequencyList(0);

	createBandBtnGroup();
	createModeBtnGroup();
	createFilterBtnGroupA();
	createFilterBtnGroupB();
	createFilterBtnGroupC();

	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(0,0,0,0);
	mainLayout->addSpacing(8);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(0);
    hbox1->setContentsMargins(0,0,0,0);
	hbox1->addStretch();
	hbox1->addWidget(bandGroupBox);

	QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->setSpacing(0);
    hbox2->setContentsMargins(0,0,0,0);
	hbox2->addStretch();
	hbox2->addWidget(modeGroupBox);

	QHBoxLayout *hbox3 = new QHBoxLayout();
	hbox3->setSpacing(0);
    hbox3->setContentsMargins(0,0,0,0);
	hbox3->addStretch();
	hbox3->addWidget(filterGroupABox);

	QHBoxLayout *hbox4 = new QHBoxLayout();
	hbox4->setSpacing(0);
    hbox4->setContentsMargins(0,0,0,0);
	hbox4->addStretch();
	hbox4->addWidget(filterGroupBBox);

	QHBoxLayout *hbox5 = new QHBoxLayout();
	hbox5->setSpacing(0);
    hbox5->setContentsMargins(0,0,0,0);
	hbox5->addStretch();
	hbox5->addWidget(filterGroupCBox);

	QHBoxLayout *hbox6 = new QHBoxLayout();
	hbox6->setSpacing(0);
    hbox6->setContentsMargins(0,0,0,0);
	hbox6->addStretch();
	hbox6->addWidget(mercuryBtnGroup());

	mainLayout->addLayout(hbox1);
	mainLayout->addLayout(hbox2);
	mainLayout->addLayout(hbox3);
	mainLayout->addLayout(hbox4);
	mainLayout->addLayout(hbox5);
	mainLayout->addLayout(hbox6);
	mainLayout->addStretch();
	
	// setup values from settings.ini
	bandBtnList.at(m_hamBand)->setBtnState(AeroButton::ON);
	bandBtnList.at(m_hamBand)->update();

	dspModeChanged(0, m_dspModeList.at(m_hamBand));
	filterChanged(0, m_filterLo, m_filterHi);

	DSPMode dspMode = m_dspModeList.at(m_hamBand);

	if (dspMode == (DSPMode) LSB || dspMode == (DSPMode) USB || dspMode == (DSPMode) DIGU || dspMode == (DSPMode) DIGL) {

		filterGroupBBox->hide();
		filterGroupCBox->hide();
	}
	else 
	if (dspMode == (DSPMode) DSB || dspMode == (DSPMode) FMN || dspMode == (DSPMode) AM || dspMode == (DSPMode) SAM) {

		filterGroupABox->hide();
		filterGroupCBox->hide();
	}
	else 
	if (dspMode == (DSPMode) CWL || dspMode == (DSPMode) CWU) {

		filterGroupABox->hide();
		filterGroupBBox->hide();
	}

	setLayout(mainLayout);
	setupConnections();
}

RadioWidget::~RadioWidget() {

	disconnect(set, 0, this, 0);
	disconnect(0, 0, 0);
}

QSize RadioWidget::sizeHint() const {
	
	return QSize(m_minimumWidgetWidth, height());
}

QSize RadioWidget::minimumSizeHint() const {

	return QSize(m_minimumWidgetWidth, height());
}

void RadioWidget::setupConnections() {

	CHECKED_CONNECT(
		set,
		&Settings::systemStateChanged,
		this,
		&RadioWidget::systemStateChanged);

	CHECKED_CONNECT(
		set, 
		&Settings::currentReceiverChanged,
		this, 
		&RadioWidget::setCurrentReceiver);

	CHECKED_CONNECT(
		set,
		&Settings::vfoFrequencyChanged,
		this,
		&RadioWidget::vfoFrequencyChanged);
	
	CHECKED_CONNECT(
		set, 
		&Settings::hamBandChanged,
		this,
		&RadioWidget::bandChanged);

	CHECKED_CONNECT(
		set,
		&Settings::dspModeChanged, 
		this, 
		&RadioWidget::dspModeChanged);
	CHECKED_CONNECT(
		set,
		&Settings::freeDVModeChanged,
		this,
		&RadioWidget::freeDVModeChanged);
	CHECKED_CONNECT(
		set,
		&Settings::freeDVStatusChanged,
		this,
		&RadioWidget::freeDVStatusChanged);

//	CHECKED_CONNECT(
//		set,
//		SIGNAL(agcModeChanged(int, AGCMode)),
//		this,
//		SLOT(agcModeChanged(int, AGCMode)));

	CHECKED_CONNECT(
		set, 
		&Settings::filterFrequenciesChanged, 
		this, 
		&RadioWidget::filterChanged);

	CHECKED_CONNECT(
		set,
		&Settings::mercuryAttenuatorChanged,
		this,
		&RadioWidget::setMercuryAttenuator);
}

void RadioWidget::createBandBtnGroup() {

	band2200mBtn = new AeroButton("2200m", this);
	band2200mBtn->setRoundness(0);
	//band2200mBtn->setGlass(false);
	band2200mBtn->setFixedSize(btn_widths, btn_height);
	//band2200mBtn->setTextColor(QColor(200, 200, 200));
	bandBtnList.append(band2200mBtn);
	CHECKED_CONNECT(band2200mBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band630mBtn = new AeroButton("630 m", this);
	band630mBtn->setRoundness(0);
	band630mBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band630mBtn);
	CHECKED_CONNECT(band630mBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band160mBtn = new AeroButton("160 m", this);
	band160mBtn->setRoundness(0);
	band160mBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band160mBtn);
	CHECKED_CONNECT(band160mBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band80mBtn = new AeroButton("80 m", this);
	band80mBtn->setRoundness(0);
	band80mBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band80mBtn);
	CHECKED_CONNECT(band80mBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band60mBtn = new AeroButton("60 m", this);
	band60mBtn->setRoundness(0);
	band60mBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band60mBtn);
	CHECKED_CONNECT(band60mBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band40mBtn = new AeroButton("40 m", this);
	band40mBtn->setRoundness(0);
	band40mBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band40mBtn);
	CHECKED_CONNECT(band40mBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band30mBtn = new AeroButton("30 m", this);
	band30mBtn->setRoundness(0);
	band30mBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band30mBtn);
	CHECKED_CONNECT(band30mBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band20mBtn = new AeroButton("20 m", this);
	band20mBtn->setRoundness(0);
	band20mBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band20mBtn);
	CHECKED_CONNECT(band20mBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band17mBtn = new AeroButton("17 m", this);
	band17mBtn->setRoundness(0);
	band17mBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band17mBtn);
	CHECKED_CONNECT(band17mBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band15mBtn = new AeroButton("15 m", this);
	band15mBtn->setRoundness(0);
	band15mBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band15mBtn);
	CHECKED_CONNECT(band15mBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band12mBtn = new AeroButton("12 m", this);
	band12mBtn->setRoundness(0);
	band12mBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band12mBtn);
	CHECKED_CONNECT(band12mBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band10mBtn = new AeroButton("10 m", this);
	band10mBtn->setRoundness(0);
	band10mBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band10mBtn);
	CHECKED_CONNECT(band10mBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band6mBtn = new AeroButton("6 m", this);
	band6mBtn->setRoundness(0);
	band6mBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band6mBtn);
	CHECKED_CONNECT(band6mBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band2mBtn = new AeroButton("2 m", this);
	band2mBtn->setRoundness(0);
	band2mBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band2mBtn);
	CHECKED_CONNECT(band2mBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band125cmBtn = new AeroButton("125 cm", this);
	band125cmBtn->setRoundness(0);
	band125cmBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band125cmBtn);
	CHECKED_CONNECT(band125cmBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band70cmBtn = new AeroButton("70 cm", this);
	band70cmBtn->setRoundness(0);
	band70cmBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band70cmBtn);
	CHECKED_CONNECT(band70cmBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band33cmBtn = new AeroButton("33 cm", this);
	band33cmBtn->setRoundness(0);
	band33cmBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band33cmBtn);
	CHECKED_CONNECT(band33cmBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band23cmBtn = new AeroButton("23 cm", this);
	band23cmBtn->setRoundness(0);
	band23cmBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band23cmBtn);
	CHECKED_CONNECT(band23cmBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band13cmBtn = new AeroButton("13 cm", this);
	band13cmBtn->setRoundness(0);
	band13cmBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band13cmBtn);
	CHECKED_CONNECT(band13cmBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band10cmBtn = new AeroButton("10 cm", this);
	band10cmBtn->setRoundness(0);
	band10cmBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band10cmBtn);
	CHECKED_CONNECT(band10cmBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	band5cmBtn = new AeroButton("5 cm", this);
	band5cmBtn->setRoundness(0);
	band5cmBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(band5cmBtn);
	CHECKED_CONNECT(band5cmBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);
	bandGenBtn = new AeroButton("Gen", this);
	bandGenBtn->setRoundness(0);
	bandGenBtn->setFixedSize(btn_widths, btn_height);
	bandBtnList.append(bandGenBtn);
	CHECKED_CONNECT(bandGenBtn, &AeroButton::clicked, this, &RadioWidget::bandChangedByBtn);

	/*bandxxBtn = new AeroButton("", this);
	bandxxBtn->setRoundness(0);
	bandxxBtn->setGlass(false);
	bandxxBtn->setFixedSize(btn_widths, btn_height);
	bandxxBtn->setHighlight(QColor(90, 90, 90));
	bandxxBtn->setTextColor(QColor(200, 200, 200));
	bandxxBtn->setEnabled(false);*/
	
	// only for testing
	//current_band = 1; // 80 m
	//band80mBtn->setBtnState(AeroButton::ON);

	QGridLayout *layout = new QGridLayout();
	layout->setVerticalSpacing(1);
	layout->setHorizontalSpacing(1);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(band2200mBtn, 0, 0);
	layout->addWidget(band630mBtn, 0, 1);
	layout->addWidget(band160mBtn, 0, 2);
	layout->addWidget(band80mBtn, 0, 3);
	layout->addWidget(band60mBtn, 0, 4);
	layout->addWidget(band40mBtn, 0, 5);
	layout->addWidget(band30mBtn, 1, 0);
	layout->addWidget(band20mBtn, 1, 1);
	layout->addWidget(band17mBtn, 1, 2);
	layout->addWidget(band15mBtn, 1, 3);
	layout->addWidget(band12mBtn, 1, 4);
	layout->addWidget(band10mBtn, 1, 5);
	layout->addWidget(band6mBtn,  2, 0);
	layout->addWidget(band2mBtn,  2, 1);
	layout->addWidget(band125cmBtn,  2, 2);
	layout->addWidget(band70cmBtn,  2, 3);
	layout->addWidget(band33cmBtn,  2, 4);
	layout->addWidget(band23cmBtn,  2, 5);
	layout->addWidget(band13cmBtn,  3, 0);
	layout->addWidget(band10cmBtn,  3, 1);
	layout->addWidget(band5cmBtn,  3, 2);
	layout->addWidget(bandGenBtn, 3, 3);
	//layout->addWidget(bandxxBtn, 2, 3);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(4);
	hbox1->addStretch(10);
	hbox1->addLayout(layout);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setSpacing(1);
	//vbox->addLayout(layout);
	vbox->addLayout(hbox1);

	bandGroupBox = new QGroupBox(tr("Band"), this);
	bandGroupBox->setMinimumWidth(m_minimumGroupBoxWidth);
	bandGroupBox->setLayout(vbox);
	//bandGroupBox->setMinimumWidth(100);
	bandGroupBox->setFont(QFont("Arial", 8));
}

void RadioWidget::createModeBtnGroup() {

	lsbBtn = new AeroButton("LSB", this);
	lsbBtn->setRoundness(0);
	//lsbBtn->setGlass(false);
	lsbBtn->setFixedSize(btn_widths, btn_height);
	//lsbBtn->setTextColor(QColor(200, 200, 200));
	dspModeBtnList.append(lsbBtn);
	CHECKED_CONNECT(lsbBtn, &AeroButton::clicked, this, &RadioWidget::dspModeChangedByBtn);

	usbBtn = new AeroButton("USB", this);
	usbBtn->setRoundness(0);
	//usbBtn->setGlass(false);
	usbBtn->setFixedSize(btn_widths, btn_height);
	//usbBtn->setTextColor(QColor(200, 200, 200));
	dspModeBtnList.append(usbBtn);
	CHECKED_CONNECT(usbBtn, &AeroButton::clicked, this, &RadioWidget::dspModeChangedByBtn);

	dsbBtn = new AeroButton("DSB", this);
	dsbBtn->setRoundness(0);
	//dsbBtn->setGlass(false);
	dsbBtn->setFixedSize(btn_widths, btn_height);
	//dsbBtn->setTextColor(QColor(200, 200, 200));
	dspModeBtnList.append(dsbBtn);
	CHECKED_CONNECT(dsbBtn, &AeroButton::clicked, this, &RadioWidget::dspModeChangedByBtn);

	cwlBtn = new AeroButton("CWL", this);
	cwlBtn->setRoundness(0);
	//cwlBtn->setGlass(false);
	cwlBtn->setFixedSize(btn_widths, btn_height);
	//cwlBtn->setTextColor(QColor(200, 200, 200));
	dspModeBtnList.append(cwlBtn);
	CHECKED_CONNECT(cwlBtn, &AeroButton::clicked, this, &RadioWidget::dspModeChangedByBtn);

	cwuBtn = new AeroButton("CWU", this);
	cwuBtn->setRoundness(0);
	//cwuBtn->setGlass(false);
	cwuBtn->setFixedSize(btn_widths, btn_height);
	//cwuBtn->setTextColor(QColor(200, 200, 200));
	dspModeBtnList.append(cwuBtn);
	CHECKED_CONNECT(cwuBtn, &AeroButton::clicked, this, &RadioWidget::dspModeChangedByBtn);

	fmnBtn = new AeroButton("FMN", this);
	fmnBtn->setRoundness(0);
	//fmnBtn->setGlass(false);
	fmnBtn->setFixedSize(btn_widths, btn_height);
	//fmnBtn->setTextColor(QColor(200, 200, 200));
	dspModeBtnList.append(fmnBtn);
	CHECKED_CONNECT(fmnBtn, &AeroButton::clicked, this, &RadioWidget::dspModeChangedByBtn);

	amBtn = new AeroButton("AM", this);
	amBtn->setRoundness(0);
	//amBtn->setGlass(false);
	amBtn->setFixedSize(btn_widths, btn_height);
	//amBtn->setTextColor(QColor(200, 200, 200));
	dspModeBtnList.append(amBtn);
	CHECKED_CONNECT(amBtn, &AeroButton::clicked, this, &RadioWidget::dspModeChangedByBtn);

	diguBtn = new AeroButton("DIGU", this);
	diguBtn->setRoundness(0);
	//diguBtn->setGlass(false);
	diguBtn->setFixedSize(btn_widths, btn_height);
	//diguBtn->setTextColor(QColor(200, 200, 200));
	dspModeBtnList.append(diguBtn);
	CHECKED_CONNECT(diguBtn, &AeroButton::clicked, this, &RadioWidget::dspModeChangedByBtn);

	diglBtn = new AeroButton("DIGL", this);
	diglBtn->setRoundness(0);
	//diglBtn->setGlass(false);
	diglBtn->setFixedSize(btn_widths, btn_height);
	//diglBtn->setTextColor(QColor(200, 200, 200));
	dspModeBtnList.append(diglBtn);
	CHECKED_CONNECT(diglBtn, &AeroButton::clicked, this, &RadioWidget::dspModeChangedByBtn);

	specBtn = new AeroButton("SPEC", this);
	specBtn->setRoundness(0);
	//specBtn->setGlass(false);
	specBtn->setFixedSize(btn_widths, btn_height);
	//specBtn->setTextColor(QColor(200, 200, 200));
	dspModeBtnList.append(specBtn);
	CHECKED_CONNECT(specBtn, &AeroButton::clicked, this, &RadioWidget::dspModeChangedByBtn);

	samBtn = new AeroButton("SAM", this);
	samBtn->setRoundness(0);
	//samBtn->setGlass(false);
	samBtn->setFixedSize(btn_widths, btn_height);
	//samBtn->setTextColor(QColor(200, 200, 200));
	dspModeBtnList.append(samBtn);
	CHECKED_CONNECT(samBtn, &AeroButton::clicked, this, &RadioWidget::dspModeChangedByBtn);

	drmBtn = new AeroButton("FreeDV", this);
	drmBtn->setRoundness(0);
	//drmBtn->setGlass(false);
	drmBtn->setFixedSize(btn_widths, btn_height);
	//drmBtn->setTextColor(QColor(200, 200, 200));
	dspModeBtnList.append(drmBtn);
	CHECKED_CONNECT(drmBtn, &AeroButton::clicked, this, &RadioWidget::dspModeChangedByBtn);

	/*foreach(AeroButton *btn, dspModeBtnList) {

		btn->setBtnState(AeroButton::OFF);
		btn->update();
	}
	m_dspMode = set->getCurrentDSPMode();
	dspModeBtnList.at(m_dspMode)->setBtnState(AeroButton::ON);*/

	QGridLayout *layout = new QGridLayout();
	layout->setVerticalSpacing(1);
	layout->setHorizontalSpacing(1);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(lsbBtn, 0, 0);
	layout->addWidget(usbBtn, 0, 1);
	layout->addWidget(dsbBtn, 0, 2);
	layout->addWidget(cwlBtn, 0, 3);
	layout->addWidget(cwuBtn, 0, 4);
	layout->addWidget(fmnBtn, 0, 5);
	layout->addWidget(amBtn,  1, 0);
	layout->addWidget(diguBtn, 1, 1);
	layout->addWidget(specBtn, 1, 2);
	layout->addWidget(diglBtn, 1, 3);
	layout->addWidget(samBtn,  1, 4);
	layout->addWidget(drmBtn,  1, 5);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(4);
	hbox1->addStretch();
	hbox1->addLayout(layout);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setSpacing(1);
	vbox->addLayout(hbox1);
	m_freeDVModeCombo = new QComboBox(this);
	m_freeDVModeCombo->addItem("FreeDV 1600", 0);
	m_freeDVModeCombo->addItem("FreeDV 700C", 6);
	m_freeDVModeCombo->addItem("FreeDV RADE v1", 100);
	m_freeDVModeCombo->setCurrentIndex(qMax(0, m_freeDVModeCombo->findData(set->getFreeDVMode(m_currentRx))));
	CHECKED_CONNECT(m_freeDVModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(freeDVModeSelectionChanged(int)));

	m_freeDVStatusLabel = new QLabel("FreeDV: no sync", this);
	m_freeDVStatusLabel->setMinimumWidth(200);

	vbox->addWidget(m_freeDVModeCombo);
	vbox->addWidget(m_freeDVStatusLabel);

	modeGroupBox = new QGroupBox(tr("Mode"), this);
	modeGroupBox->setMinimumWidth(m_minimumGroupBoxWidth);
	modeGroupBox->setLayout(vbox);
	//modeGroupBox->setMinimumWidth(100);
	modeGroupBox->setFont(QFont("Arial", 8));
	updateFreeDVControls();
}

void RadioWidget::createFilterBtnGroupA() {

	filter1kBtnA = new AeroButton("1k", this);
	filter1kBtnA->setObjectName("1k");
	filter1kBtnA->setRoundness(0);
	//filter1kBtnA->setGlass(false);
	filter1kBtnA->setFixedSize(btn_widths, btn_height);
	//filter1kBtnA->setTextColor(QColor(200, 200, 200));
	filterBtnListA.append(filter1kBtnA);
	CHECKED_CONNECT(filter1kBtnA, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter1k8BtnA = new AeroButton("1k8", this);
	filter1kBtnA->setObjectName("1k8");
	filter1k8BtnA->setRoundness(0);
	//filter1k8BtnA->setGlass(false);
	filter1k8BtnA->setFixedSize(btn_widths, btn_height);
	//filter1k8BtnA->setTextColor(QColor(200, 200, 200));
	filterBtnListA.append(filter1k8BtnA);
	CHECKED_CONNECT(filter1k8BtnA, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter2k1BtnA = new AeroButton("2k1", this);
	filter1kBtnA->setObjectName("2k1");
	filter2k1BtnA->setRoundness(0);
	//filter2k1BtnA->setGlass(false);
	filter2k1BtnA->setFixedSize(btn_widths, btn_height);
	//filter2k1BtnA->setTextColor(QColor(200, 200, 200));
	filterBtnListA.append(filter2k1BtnA);
	CHECKED_CONNECT(filter2k1BtnA, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter2k4BtnA = new AeroButton("2k4", this);
	filter1kBtnA->setObjectName("2k4");
	filter2k4BtnA->setRoundness(0);
	//filter2k4BtnA->setGlass(false);
	filter2k4BtnA->setFixedSize(btn_widths, btn_height);
	//filter2k4BtnA->setTextColor(QColor(200, 200, 200));
	filterBtnListA.append(filter2k4BtnA);
	CHECKED_CONNECT(filter2k4BtnA, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter2k7BtnA = new AeroButton("2k7", this);
	filter1kBtnA->setObjectName("2k7");
	filter2k7BtnA->setRoundness(0);
	//filter2k7BtnA->setGlass(false);
	filter2k7BtnA->setFixedSize(btn_widths, btn_height);
	//filter2k7BtnA->setTextColor(QColor(200, 200, 200));
	filterBtnListA.append(filter2k7BtnA);
	CHECKED_CONNECT(filter2k7BtnA, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter2k9BtnA = new AeroButton("2k9", this);
	filter1kBtnA->setObjectName("2k9");
	filter2k9BtnA->setRoundness(0);
	//filter2k9BtnA->setGlass(false);
	filter2k9BtnA->setFixedSize(btn_widths, btn_height);
	//filter2k9BtnA->setTextColor(QColor(200, 200, 200));
	filterBtnListA.append(filter2k9BtnA);
	CHECKED_CONNECT(filter2k9BtnA, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter3k3BtnA = new AeroButton("3k3", this);
	filter1kBtnA->setObjectName("3k3");
	filter3k3BtnA->setRoundness(0);
	//filter3k3BtnA->setGlass(false);
	filter3k3BtnA->setFixedSize(btn_widths, btn_height);
	//filter3k3BtnA->setTextColor(QColor(200, 200, 200));
	filterBtnListA.append(filter3k3BtnA);
	CHECKED_CONNECT(filter3k3BtnA, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter3k8BtnA = new AeroButton("3k8", this);
	filter1kBtnA->setObjectName("3k8");
	filter3k8BtnA->setRoundness(0);
	//filter3k8BtnA->setGlass(false);
	filter3k8BtnA->setFixedSize(btn_widths, btn_height);
	//filter3k8BtnA->setTextColor(QColor(200, 200, 200));
	filterBtnListA.append(filter3k8BtnA);
	CHECKED_CONNECT(filter3k8BtnA, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter4k4BtnA = new AeroButton("4k4", this);
	filter1kBtnA->setObjectName("4k4");
	filter4k4BtnA->setRoundness(0);
	//filter4k4BtnA->setGlass(false);
	filter4k4BtnA->setFixedSize(btn_widths, btn_height);
	//filter4k4BtnA->setTextColor(QColor(200, 200, 200));
	filterBtnListA.append(filter4k4BtnA);
	CHECKED_CONNECT(filter4k4BtnA, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter5kBtnA = new AeroButton("5k", this);
	filter1kBtnA->setObjectName("5k");
	filter5kBtnA->setRoundness(0);
	//filter5kBtnA->setGlass(false);
	filter5kBtnA->setFixedSize(btn_widths, btn_height);
	//filter5kBtnA->setTextColor(QColor(200, 200, 200));
	filterBtnListA.append(filter5kBtnA);
	CHECKED_CONNECT(filter5kBtnA, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filterVar1BtnA = new AeroButton("Var1", this);
	filter1kBtnA->setObjectName("Var1");
	filterVar1BtnA->setRoundness(0);
	//filterVar1BtnA->setGlass(false);
	filterVar1BtnA->setFixedSize(btn_widths, btn_height);
	//filterVar1BtnA->setTextColor(QColor(200, 200, 200));
	filterBtnListA.append(filterVar1BtnA);
	CHECKED_CONNECT(filterVar1BtnA, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filterVar2BtnA = new AeroButton("Var2", this);
	filter1kBtnA->setObjectName("Var2");
	filterVar2BtnA->setRoundness(0);
	//filterVar2BtnA->setGlass(false);
	filterVar2BtnA->setFixedSize(btn_widths, btn_height);
	//filterVar2BtnA->setTextColor(QColor(200, 200, 200));
	filterBtnListA.append(filterVar2BtnA);
	CHECKED_CONNECT(filterVar2BtnA, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	foreach(AeroButton *btn, filterBtnListA) {

		btn->setBtnState(AeroButton::OFF);
		btn->update();
	}
	//filterBtnListA.at(set->getCurrentDSPMode())->setBtnState(AeroButton::ON);

	QGridLayout *layout = new QGridLayout;
	layout->setVerticalSpacing(1);
	layout->setHorizontalSpacing(1);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(filter5kBtnA,  0, 0);
	layout->addWidget(filter4k4BtnA, 0, 1);
	layout->addWidget(filter3k8BtnA, 0, 2);
	layout->addWidget(filter3k3BtnA, 0, 3);
	layout->addWidget(filter2k9BtnA, 0, 4);
	layout->addWidget(filter2k7BtnA, 0, 5);
	layout->addWidget(filter2k4BtnA, 1, 0);
	layout->addWidget(filter2k1BtnA, 1, 1);
	layout->addWidget(filter1k8BtnA, 1, 2);
	layout->addWidget(filter1kBtnA,  1, 3);	
	layout->addWidget(filterVar1BtnA, 1, 4);
	layout->addWidget(filterVar2BtnA, 1, 5);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(4);
	hbox1->addStretch();
	hbox1->addLayout(layout);

	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setSpacing(1);
	vbox->addLayout(hbox1);

	filterGroupABox = new QGroupBox(tr("Filter"), this);
	filterGroupABox->setMinimumWidth(m_minimumGroupBoxWidth);
	filterGroupABox->setLayout(vbox);
	filterGroupABox->setFont(QFont("Arial", 8));
	updateFreeDVControls();
}

void RadioWidget::freeDVModeChanged(int rx, int mode) {
	if (m_currentRx != rx) return;

	const int idx = m_freeDVModeCombo->findData(mode);
	if (idx >= 0 && idx != m_freeDVModeCombo->currentIndex())
		m_freeDVModeCombo->setCurrentIndex(idx);

	updateFreeDVControls();
}

void RadioWidget::freeDVStatusChanged(int rx, bool sync, float snr, quint64 rxFrames, quint64 txFrames) {
	if (m_currentRx != rx) return;

	QString syncStr = sync ? "sync" : "search";
	QString txt = QString("FreeDV %1: %2  SNR %3 dB  RXf %4  TXf %5")
		.arg(m_freeDVModeCombo->currentText())
		.arg(syncStr)
		.arg(QString::number(snr, 'f', 1))
		.arg(rxFrames)
		.arg(txFrames);
	m_freeDVStatusLabel->setText(txt);
}

void RadioWidget::freeDVModeSelectionChanged(int index) {
	if (index < 0) return;
	const int mode = m_freeDVModeCombo->itemData(index).toInt();
	set->setFreeDVMode(m_currentRx, mode);
	updateFreeDVControls();
}

void RadioWidget::updateFreeDVControls() {
	const DSPMode mode = m_dspModeList.at(m_hamBand);
	const bool isDrm = (mode == (DSPMode) FDV);

	m_freeDVModeCombo->setVisible(isDrm);
	m_freeDVStatusLabel->setVisible(isDrm);

	if (!isDrm)
		m_freeDVStatusLabel->setText("FreeDV: inactive (select FDV)");
}

void RadioWidget::createFilterBtnGroupB() {

	filter16kBtnB = new AeroButton("16k", this);
	filter16kBtnB->setRoundness(0);
	//filter16kBtnB->setGlass(false);
	filter16kBtnB->setFixedSize(btn_widths, btn_height);
	//filter16kBtnB->setTextColor(QColor(200, 200, 200));
	filterBtnListB.append(filter16kBtnB);
	CHECKED_CONNECT(filter16kBtnB, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter12kBtnB = new AeroButton("12k", this);
	filter12kBtnB->setRoundness(0);
	//filter12kBtnB->setGlass(false);
	filter12kBtnB->setFixedSize(btn_widths, btn_height);
	//filter12kBtnB->setTextColor(QColor(200, 200, 200));
	filterBtnListB.append(filter12kBtnB);
	CHECKED_CONNECT(filter12kBtnB, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter10kBtnB = new AeroButton("10k", this);
	filter10kBtnB->setRoundness(0);
	//filter10kBtnB->setGlass(false);
	filter10kBtnB->setFixedSize(btn_widths, btn_height);
	//filter10kBtnB->setTextColor(QColor(200, 200, 200));
	filterBtnListB.append(filter10kBtnB);
	CHECKED_CONNECT(filter10kBtnB, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter8kBtnB = new AeroButton("8k", this);
	filter8kBtnB->setRoundness(0);
	//filter8kBtnB->setGlass(false);
	filter8kBtnB->setFixedSize(btn_widths, btn_height);
	//filter8kBtnB->setTextColor(QColor(200, 200, 200));
	filterBtnListB.append(filter8kBtnB);
	CHECKED_CONNECT(filter8kBtnB, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter6k6BtnB = new AeroButton("6k6", this);
	filter6k6BtnB->setRoundness(0);
	//filter6k6BtnB->setGlass(false);
	filter6k6BtnB->setFixedSize(btn_widths, btn_height);
	//filter6k6BtnB->setTextColor(QColor(200, 200, 200));
	filterBtnListB.append(filter6k6BtnB);
	CHECKED_CONNECT(filter6k6BtnB, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter5k2BtnB = new AeroButton("5k2", this);
	filter5k2BtnB->setRoundness(0);
	//filter5k2BtnB->setGlass(false);
	filter5k2BtnB->setFixedSize(btn_widths, btn_height);
	//filter5k2BtnB->setTextColor(QColor(200, 200, 200));
	filterBtnListB.append(filter5k2BtnB);
	CHECKED_CONNECT(filter5k2BtnB, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter4kBtnB = new AeroButton("4k", this);
	filter4kBtnB->setRoundness(0);
	//filter4kBtnB->setGlass(false);
	filter4kBtnB->setFixedSize(btn_widths, btn_height);
	//filter4kBtnB->setTextColor(QColor(200, 200, 200));
	filterBtnListB.append(filter4kBtnB);
	CHECKED_CONNECT(filter4kBtnB, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter3k1BtnB = new AeroButton("3k1", this);
	filter3k1BtnB->setRoundness(0);
	//filter3k1BtnB->setGlass(false);
	filter3k1BtnB->setFixedSize(btn_widths, btn_height);
	//filter3k1BtnB->setTextColor(QColor(200, 200, 200));
	filterBtnListB.append(filter3k1BtnB);
	CHECKED_CONNECT(filter3k1BtnB, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter2k9BtnB = new AeroButton("2k9", this);
	filter2k9BtnB->setRoundness(0);
	//filter2k9BtnB->setGlass(false);
	filter2k9BtnB->setFixedSize(btn_widths, btn_height);
	//filter2k9BtnB->setTextColor(QColor(200, 200, 200));
	filterBtnListB.append(filter2k9BtnB);
	CHECKED_CONNECT(filter2k9BtnB, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter2k4BtnB = new AeroButton("2k4", this);
	filter2k4BtnB->setRoundness(0);
	//filter2k4BtnB->setGlass(false);
	filter2k4BtnB->setFixedSize(btn_widths, btn_height);
	//filter2k4BtnB->setTextColor(QColor(200, 200, 200));
	filterBtnListB.append(filter2k4BtnB);
	CHECKED_CONNECT(filter2k4BtnB, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filterVar1BtnB = new AeroButton("Var1", this);
	filterVar1BtnB->setRoundness(0);
	//filterVar1BtnB->setGlass(false);
	filterVar1BtnB->setFixedSize(btn_widths, btn_height);
	//filterVar1BtnB->setTextColor(QColor(200, 200, 200));
	filterBtnListB.append(filterVar1BtnB);
	CHECKED_CONNECT(filterVar1BtnB, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filterVar2BtnB = new AeroButton("Var2", this);
	filterVar2BtnB->setRoundness(0);
	//filterVar2BtnB->setGlass(false);
	filterVar2BtnB->setFixedSize(btn_widths, btn_height);
	//filterVar2BtnB->setTextColor(QColor(200, 200, 200));
	filterBtnListB.append(filterVar2BtnB);
	CHECKED_CONNECT(filterVar2BtnB, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	foreach(AeroButton *btn, filterBtnListB) {

		btn->setBtnState(AeroButton::OFF);
		btn->update();
	}
	//filterBtnListA.at(set->getCurrentDSPMode())->setBtnState(AeroButton::ON);

	QGridLayout *layout = new QGridLayout;
	layout->setVerticalSpacing(1);
	layout->setHorizontalSpacing(1);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(filter16kBtnB, 0, 0);
	layout->addWidget(filter12kBtnB, 0, 1);
	layout->addWidget(filter10kBtnB, 0, 2);
	layout->addWidget(filter8kBtnB,  0, 3);
	layout->addWidget(filter6k6BtnB, 0, 4);
	layout->addWidget(filter5k2BtnB, 0, 5);
	layout->addWidget(filter4kBtnB,  1, 0);
	layout->addWidget(filter3k1BtnB, 1, 1);
	layout->addWidget(filter2k9BtnB, 1, 2);
	layout->addWidget(filter2k4BtnB, 1, 3);	
	layout->addWidget(filterVar1BtnB, 1, 4);
	layout->addWidget(filterVar2BtnB, 1, 5);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(4);
	hbox1->addStretch();
	hbox1->addLayout(layout);

	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setSpacing(1);
	vbox->addLayout(hbox1);

	filterGroupBBox = new QGroupBox(tr("Filter"), this);
	filterGroupBBox->setMinimumWidth(m_minimumGroupBoxWidth);
	filterGroupBBox->setLayout(vbox);
	filterGroupBBox->setFont(QFont("Arial", 8));
}

void RadioWidget::createFilterBtnGroupC() {

	filter1kBtnC = new AeroButton("1k", this);
	filter1kBtnC->setRoundness(0);
	//filter1kBtnC->setGlass(false);
	filter1kBtnC->setFixedSize(btn_widths, btn_height);
	//filter1kBtnC->setTextColor(QColor(200, 200, 200));
	filterBtnListC.append(filter1kBtnC);
	CHECKED_CONNECT(filter1kBtnC, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter800BtnC = new AeroButton("800", this);
	filter800BtnC->setRoundness(0);
	//filter800BtnC->setGlass(false);
	filter800BtnC->setFixedSize(btn_widths, btn_height);
	//filter800BtnC->setTextColor(QColor(200, 200, 200));
	filterBtnListC.append(filter800BtnC);
	CHECKED_CONNECT(filter800BtnC, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter750BtnC = new AeroButton("750", this);
	filter750BtnC->setRoundness(0);
	//filter750BtnC->setGlass(false);
	filter750BtnC->setFixedSize(btn_widths, btn_height);
	//filter750BtnC->setTextColor(QColor(200, 200, 200));
	filterBtnListC.append(filter750BtnC);
	CHECKED_CONNECT(filter750BtnC, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter600BtnC = new AeroButton("600", this);
	filter600BtnC->setRoundness(0);
	//filter600BtnC->setGlass(false);
	filter600BtnC->setFixedSize(btn_widths, btn_height);
	//filter600BtnC->setTextColor(QColor(200, 200, 200));
	filterBtnListC.append(filter600BtnC);
	CHECKED_CONNECT(filter600BtnC, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter500BtnC = new AeroButton("500", this);
	filter500BtnC->setRoundness(0);
	//filter500BtnC->setGlass(false);
	filter500BtnC->setFixedSize(btn_widths, btn_height);
	//filter500BtnC->setTextColor(QColor(200, 200, 200));
	filterBtnListC.append(filter500BtnC);
	CHECKED_CONNECT(filter500BtnC, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter400BtnC = new AeroButton("400", this);
	filter400BtnC->setRoundness(0);
	//filter400BtnC->setGlass(false);
	filter400BtnC->setFixedSize(btn_widths, btn_height);
	//filter400BtnC->setTextColor(QColor(200, 200, 200));
	filterBtnListC.append(filter400BtnC);
	CHECKED_CONNECT(filter400BtnC, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter250BtnC = new AeroButton("250", this);
	filter250BtnC->setRoundness(0);
	//filter250BtnC->setGlass(false);
	filter250BtnC->setFixedSize(btn_widths, btn_height);
	//filter250BtnC->setTextColor(QColor(200, 200, 200));
	filterBtnListC.append(filter250BtnC);
	CHECKED_CONNECT(filter250BtnC, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter100BtnC = new AeroButton("100", this);
	filter100BtnC->setRoundness(0);
	//filter100BtnC->setGlass(false);
	filter100BtnC->setFixedSize(btn_widths, btn_height);
	//filter100BtnC->setTextColor(QColor(200, 200, 200));
	filterBtnListC.append(filter100BtnC);
	CHECKED_CONNECT(filter100BtnC, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter50BtnC = new AeroButton("50", this);
	filter50BtnC->setRoundness(0);
	//filter50BtnC->setGlass(false);
	filter50BtnC->setFixedSize(btn_widths, btn_height);
	//filter50BtnC->setTextColor(QColor(200, 200, 200));
	filterBtnListC.append(filter50BtnC);
	CHECKED_CONNECT(filter50BtnC, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filter25BtnC = new AeroButton("25", this);
	filter25BtnC->setRoundness(0);
	//filter25BtnC->setGlass(false);
	filter25BtnC->setFixedSize(btn_widths, btn_height);
	//filter25BtnC->setTextColor(QColor(200, 200, 200));
	filterBtnListC.append(filter25BtnC);
	CHECKED_CONNECT(filter25BtnC, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filterVar1BtnC = new AeroButton("Var1", this);
	filterVar1BtnC->setRoundness(0);
	//filterVar1BtnC->setGlass(false);
	filterVar1BtnC->setFixedSize(btn_widths, btn_height);
	//filterVar1BtnC->setTextColor(QColor(200, 200, 200));
	filterBtnListC.append(filterVar1BtnC);
	CHECKED_CONNECT(filterVar1BtnC, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	filterVar2BtnC = new AeroButton("Var2", this);
	filterVar2BtnC->setRoundness(0);
	//filterVar2BtnC->setGlass(false);
	filterVar2BtnC->setFixedSize(btn_widths, btn_height);
	//filterVar2BtnC->setTextColor(QColor(200, 200, 200));
	filterBtnListC.append(filterVar2BtnC);
	CHECKED_CONNECT(filterVar2BtnC, &AeroButton::clicked, this, &RadioWidget::filterChangedByBtn);

	foreach(AeroButton *btn, filterBtnListC) {

		btn->setBtnState(AeroButton::OFF);
		btn->update();
	}
	//filterBtnListC.at(set)->getCurrentDSPMode())->setBtnState(AeroButton::ON);

	QGridLayout *layout = new QGridLayout();
	layout->setVerticalSpacing(1);
	layout->setHorizontalSpacing(1);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(filter1kBtnC,  0, 0);
	layout->addWidget(filter800BtnC, 0, 1);
	layout->addWidget(filter750BtnC, 0, 2);
	layout->addWidget(filter600BtnC, 0, 3);
	layout->addWidget(filter500BtnC, 0, 4);
	layout->addWidget(filter400BtnC, 0, 5);
	layout->addWidget(filter250BtnC, 1, 0);
	layout->addWidget(filter100BtnC, 1, 1);
	layout->addWidget(filter50BtnC,  1, 2);
	layout->addWidget(filter25BtnC,  1, 3);	
	layout->addWidget(filterVar1BtnC, 1, 4);
	layout->addWidget(filterVar2BtnC, 1, 5);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(4);
	hbox1->addStretch();
	hbox1->addLayout(layout);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setSpacing(1);
	vbox->addLayout(hbox1);

	filterGroupCBox = new QGroupBox(tr("Filter"), this);
	filterGroupCBox->setMinimumWidth(m_minimumGroupBoxWidth);
	filterGroupCBox->setLayout(vbox);
	filterGroupCBox->setFont(QFont("Arial", 8));
}

QGroupBox *RadioWidget::mercuryBtnGroup() {

	attenuatorBtn = new AeroButton("Attn.", this);
	attenuatorBtn->setRoundness(0);
	attenuatorBtn->setFixedSize (50, btn_height);
	connect(attenuatorBtn, &AeroButton::released, this, &RadioWidget::attenuatorChanged);

	ditherBtn = new AeroButton("Dither", this);
	ditherBtn->setRoundness(0);
	//ditherBtn->setGlass(false);
	ditherBtn->setFixedSize (50, btn_height);
	//ditherBtn->setTextColor(QColor(200, 200, 200));
	connect(ditherBtn, &AeroButton::released, this, &RadioWidget::ditherChanged);

	randomBtn = new AeroButton("Rand", this);
	randomBtn->setRoundness(0);
	//randomBtn->setGlass(false);
	randomBtn->setFixedSize(50, btn_height);
	//randomBtn->setTextColor(QColor(200, 200, 200));
	CHECKED_CONNECT(randomBtn, &AeroButton::released, this, &RadioWidget::randomChanged);
	
	HamBand band = set->getCurrentHamBand(0);

	if (set->getMercuryAttenuators(0).at(band)) {

		attenuatorBtn->setBtnState(AeroButton::OFF);
		attenuatorBtn->setText("Att 0dB");
	}
	else {

		attenuatorBtn->setBtnState(AeroButton::ON);
		attenuatorBtn->setText("Att -20dB");
	}

	if(set->getMercuryDither())
		ditherBtn->setBtnState(AeroButton::ON);

	if(set->getMercuryRandom())
		randomBtn->setBtnState(AeroButton::ON);
		
	QGridLayout *layout = new QGridLayout();
	layout->setVerticalSpacing(1);
	layout->setHorizontalSpacing(1);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(attenuatorBtn, 0, 0);
	layout->addWidget(ditherBtn, 0, 1);
	layout->addWidget(randomBtn, 0, 2);
	
	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(4);
	hbox1->addStretch();
	hbox1->addLayout(layout);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setSpacing(1);
	vbox->addLayout(hbox1);
	
	QGroupBox *groupBox = new QGroupBox(tr("Mercury"), this);
	groupBox->setMinimumWidth(m_minimumGroupBoxWidth);
	groupBox->setLayout(vbox);
	//groupBox->setMinimumWidth(175);
	groupBox->setFont(QFont("Arial", 8));

	return groupBox;
}

QLabel *RadioWidget::createLabel(const QString &text) {

	QLabel *label = new QLabel(text, this);
	label->setFrameStyle(QFrame::Box | QFrame::Raised);
	return label;
}

 
//******************************************
void RadioWidget::ctrFrequencyChanged(int mode, int rx, qint64 frequency) {

	Q_UNUSED(mode)

	if (m_currentRx != rx) return;
	m_ctrFrequency = frequency;

	HamBand band = getBandFromFrequency(set->getBandFrequencyList(), frequency);
	m_lastCtrFrequencyList[(int) band] = m_ctrFrequency;
}

void RadioWidget::vfoFrequencyChanged(int mode, int rx, qint64 frequency) {

	Q_UNUSED(mode)

	if (m_currentRx != rx) return;
	m_vfoFrequency = frequency;

	HamBand band = getBandFromFrequency(set->getBandFrequencyList(), frequency);
	m_lastVfoFrequencyList[(int) band] = m_vfoFrequency;
}

void RadioWidget::bandChangedByBtn() {
    AeroButton *button = qobject_cast<AeroButton *>(sender());
    int btnIndex = bandBtnList.indexOf(button);
    if (btnIndex == -1) return;

    for(AeroButton *btn : bandBtnList) {
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    button->setBtnState(AeroButton::ON);
    button->update();

    HamBand band = static_cast<HamBand>(btnIndex);
    set->setHamBand(m_currentRx, true, band);

    if (btnIndex >= 0 && btnIndex < m_lastVfoFrequencyList.size()) {
        set->setVFOFrequency(2, m_currentRx, m_lastVfoFrequencyList.at(btnIndex));
    }
}

void RadioWidget::bandChanged(int rx, bool byButton, HamBand band) {

	Q_UNUSED (byButton)

	if (m_currentRx != rx) return;
	m_hamBand = band;

	foreach(AeroButton *btn, bandBtnList) {

		btn->setBtnState(AeroButton::OFF);
		btn->update();
	}

	//m_lastFrequencyList[(int) band] = m_frequency;
	bandBtnList.at(band)->setBtnState(AeroButton::ON);
	bandBtnList.at(band)->update();
}

void RadioWidget::dspModeChangedByBtn() {
    AeroButton *button = qobject_cast<AeroButton *>(sender());
    int btnIndex = dspModeBtnList.indexOf(button);
    if (btnIndex == -1) return;
    
    for(AeroButton *btn : dspModeBtnList) {
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    DSPMode mode = static_cast<DSPMode>(btnIndex);
    set->setDSPMode(m_currentRx, mode);
    m_dspModeList[m_hamBand] = mode;
    filterChanged(m_currentRx, m_filterLo, m_filterHi);
    filterGroupChanged(mode);

    button->setBtnState(AeroButton::ON);
    button->update();
    updateFreeDVControls();
}

void RadioWidget::dspModeChanged(int rx, DSPMode mode) {

	if (m_currentRx != rx) return;
	m_dspModeList[m_hamBand] = mode;

	foreach(AeroButton *btn, dspModeBtnList) {

		btn->setBtnState(AeroButton::OFF);
		btn->update();
	}

	dspModeBtnList.at(mode)->setBtnState(AeroButton::ON);
	dspModeBtnList.at(mode)->update();
	updateFreeDVControls();
}

void RadioWidget::filterGroupChanged(DSPMode mode) {
    // A-group: SSB/Data (LSB, USB, DIGU, DIGL)
    // B-group: Wide (DSB, FMN, AM, SAM)
    // C-group: Narrow (CWL, CWU)
    
    bool showA = (mode == LSB || mode == USB || mode == DIGU || mode == DIGL);
    bool showB = (mode == DSB || mode == FMN || mode == AM || mode == SAM);
    bool showC = (mode == CWL || mode == CWU);

    filterGroupABox->setVisible(showA);
    filterGroupBBox->setVisible(showB);
    filterGroupCBox->setVisible(showC);
}

void RadioWidget::filterChangedByBtn() {
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

    qreal filterWidth = 0.0f;
    if (btnIndex < 10) {
        float widths[] = {1000, 1800, 2100, 2400, 2700, 2900, 3300, 3800, 4400, 5000,
                          2400, 2900, 3100, 4000, 5200, 6600, 8000, 10000, 12000, 16000,
                          25, 50, 100, 250, 400, 500, 600, 750, 800, 1000};
        filterWidth = widths[groupIdx * 10 + btnIndex];
    } else {
        filterWidth = qAbs(m_filterHi - m_filterLo);
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
    
    set->setRXFilter(m_currentRx, m_filterLo, m_filterHi);
    update(); 
}

void RadioWidget::filterChanged(int rx, qreal low, qreal high) {
    if (m_currentRx != rx) return;
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

    float widths[] = {1000, 1800, 2100, 2400, 2700, 2900, 3300, 3800, 4400, 5000,
                      2400, 2900, 3100, 4000, 5200, 6600, 8000, 10000, 12000, 16000,
                      25, 50, 100, 250, 400, 500, 600, 750, 800, 1000};
    
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
            activeList->at(i)->setBtnState(AeroButton::ON);
            activeList->at(i)->update();
            break;
        }
    }
}

void RadioWidget::attenuatorChanged() {

	if (attenuatorBtn->btnState() == AeroButton::OFF) {
		
		set->setMercuryAttenuator(0);
		attenuatorBtn->setBtnState(AeroButton::ON);
		attenuatorBtn->setText("Att -20dB");
		emit newMessage("[hpsdr]: attenuator -20 dB.");
	}
	else {

		set->setMercuryAttenuator(1);
		attenuatorBtn->setBtnState(AeroButton::OFF);
		attenuatorBtn->setText("Att 0dB");
		emit newMessage("[hpsdr]: attenuator 0 dB.");
	}
}

void RadioWidget::setMercuryAttenuator(HamBand band, int value) {

	Q_UNUSED(band)

	if (value) {

		attenuatorBtn->setBtnState(AeroButton::OFF);
		attenuatorBtn->setText("Att 0dB");
		emit newMessage("[hpsdr]: attenuator 0 dB.");
	}
	else {

		attenuatorBtn->setBtnState(AeroButton::ON);
		attenuatorBtn->setText("Att -20dB");
		emit newMessage("[hpsdr]: attenuator -20 dB.");
	}
}

void RadioWidget::ditherChanged() {

	if (ditherBtn->btnState() == AeroButton::OFF) {
		
		set->setDither(true);
		ditherBtn->setBtnState(AeroButton::ON);
		emit newMessage("[hpsdr]: dither on.");

	} else {

		set->setDither(false);
		ditherBtn->setBtnState(AeroButton::OFF);
		emit newMessage("[hpsdr]: dither off.");
	}
}

void RadioWidget::randomChanged() {

	if (randomBtn->btnState() == AeroButton::OFF) {
		
		set->setRandom(true);
		randomBtn->setBtnState(AeroButton::ON);
		emit newMessage("[hpsdr]: random on.");

	} else {

		set->setRandom(false);
		randomBtn->setBtnState(AeroButton::OFF);
		emit newMessage("[hpsdr]: random off.");
	}
}

void RadioWidget::setCurrentReceiver(int value) {

	if (m_currentRx == value) return;
	m_currentRx = value;

	if (m_hamBand != set->getCurrentHamBand(m_currentRx)) {
		m_hamBand = set->getCurrentHamBand(m_currentRx);

		foreach(AeroButton *btn, bandBtnList) {
			
			btn->setBtnState(AeroButton::OFF);
			btn->update();
		}
		AeroButton *button = bandBtnList.at(m_hamBand);
		button->setBtnState(AeroButton::ON);
		button->update();
		
	}

	const int modeIdx = m_freeDVModeCombo->findData(set->getFreeDVMode(m_currentRx));
	if (modeIdx >= 0)
		m_freeDVModeCombo->setCurrentIndex(modeIdx);

	updateFreeDVControls();

	const DSPMode mode = set->getDSPMode(m_currentRx);
	if (m_hamBand >= 0 && m_hamBand < m_dspModeList.size()) {
		m_dspModeList[m_hamBand] = mode;
	}
	dspModeChanged(m_currentRx, mode);

//	DSPMode mode = m_dspModeList.at(m_hamBand);
//	if (mode != rxData.dspModeList.at(m_hamBand)) {
//
//		m_dspModeList[m_hamBand] = rxData.dspModeList.at(m_hamBand);
//
//		foreach(AeroButton *btn, dspModeBtnList) {
//
//			btn->setBtnState(AeroButton::OFF);
//			btn->update();
//		}
//		qDebug() << "***********************    RadioWidget: DSPmode changed by changed receiver!";
//		AeroButton *button = dspModeBtnList.at(mode);
//		button->setBtnState(AeroButton::ON);
//		button->update();
//
//		filterGroupChanged(mode);
//		filterChanged(m_currentRx, m_filterLo, m_filterHi);
//	}

//	if (m_agcMode != rxData.agcMode) {
//		m_agcMode = rxData.agcMode;
//	}

	if (m_filterLo != set->getFilterLo(m_currentRx) || m_filterHi != set->getFilterHi(m_currentRx)) {

		m_filterLo = set->getFilterLo(m_currentRx);
		m_filterHi = set->getFilterHi(m_currentRx);
		filterChanged(m_currentRx, m_filterLo, m_filterHi);
	}
}
 
// **********************

void RadioWidget::systemStateChanged(
		QSDR::_Error err, 
		QSDR::_HWInterfaceMode hwmode, 
		QSDR::_ServerMode mode, 
		QSDR::_DataEngineState state
) {
	Q_UNUSED(err)
	Q_UNUSED(hwmode)
	Q_UNUSED(mode)
	Q_UNUSED(state)


}

void RadioWidget::closeEvent(QCloseEvent *event) {

	emit closeEvent();
	QWidget::closeEvent(event);
}

void RadioWidget::showEvent(QShowEvent *event) {

	emit showEvent();
	QWidget::showEvent(event);
}


