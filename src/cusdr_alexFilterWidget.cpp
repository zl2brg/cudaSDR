#define LOG_ALEX_WIDGET

#include <QPen>
#include <QDebug>
#include <QScopedPointer>

#include "cusdr_alexFilterWidget.h"

#define	btn_height		15
#define	btn_width		22
#define	btn_width2		24
#define	btn_width3		45

AlexFilterWidget::AlexFilterWidget(QWidget *parent)
	: QWidget(parent)
	, m_frequency(14000000)
	, m_minimumWidgetWidth(0)
	, m_minimumGroupBoxWidth(0)
	, m_hpfFilters(6)
	, m_lpfFilters(7)
	, m_receiver(0)
	, bypassAll(false)
	, lowNoise6m(false)
	, hpf13MHz(false)
	, hpf20MHz(false)
	, hpf9_5MHz(false)
	, hpf6_5MHz(false)
	, hpf1_5MHz(false)
	, m_alexConfig(0)
{
	setObjectName("AlexFilterWidget");
	setContentsMargins(4, 8, 4, 0);
	setMouseTracking(true);

	// Pre-populate m_alexStates
	for (int i = 0; i < 11; ++i) {
		m_alexStates.append(0);
	}

	fonts = new CFonts(this);
	m_fonts = fonts->getFonts();

	btnOff = QColor(68, 68, 68, 255);
	btnOn = QColor(56, 242, 115, 255);

	manualFilterBtn = new AeroButton("Auto", this);
	manualFilterBtn->setRoundness(0);
	manualFilterBtn->setFixedSize (btn_width3, btn_height);
	manualFilterBtn->setBtnState(AeroButton::OFF);

	CHECKED_CONNECT(manualFilterBtn, &AeroButton::clicked, this, &AlexFilterWidget::manualFilterBtnClicked);

	defaultValuesBtn = new AeroButton("Default", this);
	defaultValuesBtn->setRoundness(0);
	defaultValuesBtn->setFixedSize (btn_width3, btn_height);
	defaultValuesBtn->setBtnState(AeroButton::OFF);

	CHECKED_CONNECT(defaultValuesBtn, &AeroButton::clicked, this, &AlexFilterWidget::defaultValuesBtnClicked);

	setFilterValues();

	createHPFGroup();
	createLPFGroup();

	// set main layout
	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(0,0,0,0);
	mainLayout->addSpacing(8);
	mainLayout->setContentsMargins(1, 0, 4, 0);

	QHBoxLayout *hbox1 = new QHBoxLayout;
	hbox1->setSpacing(0);
	hbox1->setContentsMargins(4, 0, 12, 0);
	hbox1->addStretch();
	hbox1->addWidget(manualFilterBtn);
	hbox1->addWidget(defaultValuesBtn);

	QHBoxLayout *hbox2 = new QHBoxLayout;
	hbox2->setSpacing(0);
	hbox2->setContentsMargins(2, 0, 10, 0);
	hbox2->addWidget(HPFGroup);

	QHBoxLayout *hbox3 = new QHBoxLayout;
	hbox3->setSpacing(0);
	hbox3->setContentsMargins(2, 0, 10, 0);
	hbox3->addWidget(LPFGroup);

	mainLayout->addLayout(hbox1);
	mainLayout->addLayout(hbox2);
	mainLayout->addLayout(hbox3);
	mainLayout->addStretch();

	setLayout(mainLayout);

	m_HPFActiveBtnList.at(6)->setColors(btnOn, btnOn);

	setAlexConfiguration((double)(m_frequency/1000.0));
}

AlexFilterWidget::~AlexFilterWidget() {
	disconnect(0, 0, 0);
}

void AlexFilterWidget::setAlexConfig(quint16 config) {
	m_alexConfig = config;
}

void AlexFilterWidget::setAlexStates(const QList<int>& states) {
	m_alexStates = states;
}

void AlexFilterWidget::setAlexManualState(bool manual) {
	manualFilterBtn->blockSignals(true);
	if (manual) {
		m_alexConfig |= 0x01;
		manualFilterBtn->setText("Manual");
		manualFilterBtn->setBtnState(AeroButton::ON);
	} else {
		m_alexConfig &= 0xFFFE;
		foreach(QHLed *led, m_HPFActiveBtnList)
			led->setColors(btnOff, btnOff);
		m_HPFActiveBtnList.at(6)->setColors(btnOn, btnOn);

		manualFilterBtn->setText("Auto");
		manualFilterBtn->setBtnState(AeroButton::OFF);
	}
	manualFilterBtn->blockSignals(false);
	manualFilterBtn->update();
}

void AlexFilterWidget::setFrequencies(const QList<long>& hpfLo, const QList<long>& hpfHi, const QList<long>& lpfLo, const QList<long>& lpfHi) {
	for (int i = 0; i < qMin(hpfLo.size(), m_HPFLoSpinBoxList.size()); ++i) {
		m_HPFLoSpinBoxList.at(i)->blockSignals(true);
		m_HPFLoSpinBoxList.at(i)->setValue(hpfLo.at(i) / 1000.0);
		m_HPFLoSpinBoxList.at(i)->blockSignals(false);
	}
	for (int i = 0; i < qMin(hpfHi.size(), m_HPFHiSpinBoxList.size()); ++i) {
		m_HPFHiSpinBoxList.at(i)->blockSignals(true);
		m_HPFHiSpinBoxList.at(i)->setValue(hpfHi.at(i) / 1000.0);
		m_HPFHiSpinBoxList.at(i)->blockSignals(false);
	}
	for (int i = 0; i < qMin(lpfLo.size(), m_LPFLoSpinBoxList.size()); ++i) {
		m_LPFLoSpinBoxList.at(i)->blockSignals(true);
		m_LPFLoSpinBoxList.at(i)->setValue(lpfLo.at(i) / 1000.0);
		m_LPFLoSpinBoxList.at(i)->blockSignals(false);
	}
	for (int i = 0; i < qMin(lpfHi.size(), m_LPFHiSpinBoxList.size()); ++i) {
		m_LPFHiSpinBoxList.at(i)->blockSignals(true);
		m_LPFHiSpinBoxList.at(i)->setValue(lpfHi.at(i) / 1000.0);
		m_LPFHiSpinBoxList.at(i)->blockSignals(false);
	}
}

void AlexFilterWidget::setFrequency(int mode, int rx, qint64 frequency) {
	Q_UNUSED(mode);
	if (rx != m_receiver) return;
	m_frequency = frequency;
	setAlexConfiguration((double)(frequency / 1000.0));
}

void AlexFilterWidget::createHPFGroup() {
	QLabel *byPassLabel = new QLabel("byPass", this);
	byPassLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	QLabel *emptyLabel = new QLabel(" ", this);
	emptyLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	QLabel *hpfLabel = new QLabel("HPF (kHz)", this);
	hpfLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	QLabel *mHz1_5Label = new QLabel("1.5 MHz", this);
	mHz1_5Label->setFrameStyle(QFrame::Box | QFrame::Raised);
	m_HPFLabelList.append(mHz1_5Label);

	QLabel *mHz6_5Label = new QLabel("6.5 MHz", this);
	mHz6_5Label->setFrameStyle(QFrame::Box | QFrame::Raised);
	m_HPFLabelList.append(mHz6_5Label);

	QLabel *mHz9_5Label = new QLabel("9.5 MHz", this);
	mHz9_5Label->setFrameStyle(QFrame::Box | QFrame::Raised);
	m_HPFLabelList.append(mHz9_5Label);

	QLabel *mHz13Label = new QLabel("13 MHz", this);
	mHz13Label->setFrameStyle(QFrame::Box | QFrame::Raised);
	m_HPFLabelList.append(mHz13Label);

	QLabel *mHz20Label = new QLabel("20 MHz", this);
	mHz20Label->setFrameStyle(QFrame::Box | QFrame::Raised);
	m_HPFLabelList.append(mHz20Label);

	QLabel *mhz55Label = new QLabel("6 m LNA", this);
	mhz55Label->setFrameStyle(QFrame::Box | QFrame::Raised);
	m_HPFLabelList.append(mhz55Label);

	mhz55HPFLabel = new QLabel("6 m LNA", this);
	mhz55HPFLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
	m_HPFLabelList.append(mhz55HPFLabel);

	// create spinboxes
	for (int i = 0; i < m_hpfFilters; i++) {
		QDoubleSpinBox *loSpinBox = new QDoubleSpinBox(this);
		loSpinBox->setRange(m_HPFFrequencyRangeLoList.at(i).first, m_HPFFrequencyRangeLoList.at(i).second);
		loSpinBox->setDecimals(3);
		loSpinBox->setSingleStep(0.005);
		loSpinBox->setValue(m_HPFLoDefaultFrequencyList.at(i));
		loSpinBox->setFixedWidth(64);
		loSpinBox->setFont(m_fonts.normalFont);

		CHECKED_CONNECT(loSpinBox, SIGNAL(valueChanged(double)), this, SLOT(hpfLoSpinBoxValueChanged(double)));

		m_HPFLoSpinBoxList.append(loSpinBox);

		QDoubleSpinBox *hiSpinBox = new QDoubleSpinBox(this);
		hiSpinBox->setRange(m_HPFFrequencyRangeHiList.at(i).first, m_HPFFrequencyRangeHiList.at(i).second);
		hiSpinBox->setDecimals(3);
		hiSpinBox->setSingleStep(0.005);
		hiSpinBox->setValue(m_HPFHiDefaultFrequencyList.at(i));
		hiSpinBox->setFixedWidth(64);
		hiSpinBox->setFont(m_fonts.normalFont);

		CHECKED_CONNECT(hiSpinBox, SIGNAL(valueChanged(double)), this, SLOT(hpfHiSpinBoxValueChanged(double)));

		m_HPFHiSpinBoxList.append(hiSpinBox);
	}

	bypassAllHPFBtn = new AeroButton("", this);
	bypassAllHPFBtn->setRoundness(0);
	bypassAllHPFBtn->setFixedSize(btn_width, btn_height);
	bypassAllHPFBtn->setBtnState(AeroButton::OFF);

	CHECKED_CONNECT(bypassAllHPFBtn, &AeroButton::clicked, this, &AlexFilterWidget::bypassAllHPFBtnClicked);

	hpf1_5MHzBtn = new AeroButton("", this);
	hpf1_5MHzBtn->setRoundness(0);
	hpf1_5MHzBtn->setFixedSize(btn_width, btn_height);
	hpf1_5MHzBtn->setBtnState(AeroButton::OFF);
	m_HPFBtnList.append(hpf1_5MHzBtn);

	CHECKED_CONNECT(hpf1_5MHzBtn, &AeroButton::clicked, this, &AlexFilterWidget::hpf1_5MHzBtnClicked);

	hpf6_5MHzBtn = new AeroButton("", this);
	hpf6_5MHzBtn->setRoundness(0);
	hpf6_5MHzBtn->setFixedSize(btn_width, btn_height);
	hpf6_5MHzBtn->setBtnState(AeroButton::OFF);
	m_HPFBtnList.append(hpf6_5MHzBtn);

	CHECKED_CONNECT(hpf6_5MHzBtn, &AeroButton::clicked, this, &AlexFilterWidget::hpf6_5MHzBtnClicked);

	hpf9_5MHzBtn = new AeroButton("", this);
	hpf9_5MHzBtn->setRoundness(0);
	hpf9_5MHzBtn->setFixedSize(btn_width, btn_height);
	hpf9_5MHzBtn->setBtnState(AeroButton::OFF);
	m_HPFBtnList.append(hpf9_5MHzBtn);

	CHECKED_CONNECT(hpf9_5MHzBtn, &AeroButton::clicked, this, &AlexFilterWidget::hpf9_5MHzBtnClicked);

	hpf13MHzBtn = new AeroButton("", this);
	hpf13MHzBtn->setRoundness(0);
	hpf13MHzBtn->setFixedSize(btn_width, btn_height);
	hpf13MHzBtn->setBtnState(AeroButton::OFF);
	m_HPFBtnList.append(hpf13MHzBtn);

	CHECKED_CONNECT(hpf13MHzBtn, &AeroButton::clicked, this, &AlexFilterWidget::hpf13MHzBtnClicked);

	hpf20MHzBtn = new AeroButton("", this);
	hpf20MHzBtn->setRoundness(0);
	hpf20MHzBtn->setFixedSize(btn_width, btn_height);
	hpf20MHzBtn->setBtnState(AeroButton::OFF);
	m_HPFBtnList.append(hpf20MHzBtn);

	CHECKED_CONNECT(hpf20MHzBtn, &AeroButton::clicked, this, &AlexFilterWidget::hpf20MHzBtnClicked);

	lowNoise6mAmpBtn = new AeroButton("", this);
	lowNoise6mAmpBtn->setRoundness(0);
	lowNoise6mAmpBtn->setFixedSize(btn_width, btn_height);
	lowNoise6mAmpBtn->setBtnState(AeroButton::OFF);
	m_HPFBtnList.append(lowNoise6mAmpBtn);

	CHECKED_CONNECT(lowNoise6mAmpBtn, &AeroButton::clicked, this, &AlexFilterWidget::lowNoise6mAmpBtnClicked);

	// create LEDs
	for (int i = 0; i < 7; i++) {
		QHLed *led = new QHLed("", this);
		led->setFixedSize (10, 10);
		led->setColors(btnOff, btnOff);
		m_HPFActiveBtnList.append(led);
	}

	QGridLayout *gridLayout = new QGridLayout;
	gridLayout->setSpacing(2);

	gridLayout->addWidget(hpfLabel, 0, 0, 1, 4, Qt::AlignCenter);
	gridLayout->addWidget(emptyLabel, 0, 4);
	gridLayout->addWidget(byPassLabel, 0, 5, Qt::AlignCenter);

	gridLayout->addWidget(mHz1_5Label, 1, 0, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFLoSpinBoxList.at(0), 1, 1, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFHiSpinBoxList.at(0), 1, 2, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFActiveBtnList.at(0), 1, 3, Qt::AlignCenter);
	gridLayout->addWidget(hpf1_5MHzBtn, 1, 5, Qt::AlignCenter);

	gridLayout->addWidget(mHz6_5Label, 2, 0, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFLoSpinBoxList.at(1), 2, 1, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFHiSpinBoxList.at(1), 2, 2, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFActiveBtnList.at(1), 2, 3, Qt::AlignCenter);
	gridLayout->addWidget(hpf6_5MHzBtn, 2, 5, Qt::AlignCenter);

	gridLayout->addWidget(mHz9_5Label, 3, 0, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFLoSpinBoxList.at(2), 3, 1, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFHiSpinBoxList.at(2), 3, 2, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFActiveBtnList.at(2), 3, 3, Qt::AlignCenter);
	gridLayout->addWidget(hpf9_5MHzBtn, 3, 5, Qt::AlignCenter);

	gridLayout->addWidget(mHz13Label, 4, 0, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFLoSpinBoxList.at(3), 4, 1, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFHiSpinBoxList.at(3), 4, 2, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFActiveBtnList.at(3), 4, 3, Qt::AlignCenter);
	gridLayout->addWidget(hpf13MHzBtn, 4, 5, Qt::AlignCenter);

	gridLayout->addWidget(mHz20Label, 5, 0, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFLoSpinBoxList.at(4), 5, 1, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFHiSpinBoxList.at(4), 5, 2, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFActiveBtnList.at(4), 5, 3, Qt::AlignCenter);
	gridLayout->addWidget(hpf20MHzBtn, 5, 5, Qt::AlignCenter);

	gridLayout->addWidget(mhz55Label, 6, 0, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFLoSpinBoxList.at(5), 6, 1, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFHiSpinBoxList.at(5), 6, 2, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFActiveBtnList.at(5), 6, 3, Qt::AlignCenter);
	gridLayout->addWidget(lowNoise6mAmpBtn, 6, 5, Qt::AlignCenter);

	gridLayout->addWidget(mhz55HPFLabel, 7, 0, Qt::AlignCenter);
	gridLayout->addWidget(m_HPFActiveBtnList.at(6), 7, 3, Qt::AlignCenter);
	gridLayout->addWidget(bypassAllHPFBtn, 7, 5, Qt::AlignCenter);

	HPFGroup = new QGroupBox(tr("High Pass Filter Cutoff Frequency Options (MHz)"), this);
	HPFGroup->setMinimumWidth(m_minimumGroupBoxWidth);
	HPFGroup->setLayout(gridLayout);
	HPFGroup->setFont(QFont("Arial", 8));
}

void AlexFilterWidget::createLPFGroup() {
	QLabel *lpfLabel = new QLabel("LPF (kHz)", this);
	lpfLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	QLabel *emptyLabel = new QLabel(" ", this);
	emptyLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	QLabel *activeLabel = new QLabel("active", this);
	activeLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	QLabel *mHz1_8Label = new QLabel("1.8 MHz", this);
	mHz1_8Label->setFrameStyle(QFrame::Box | QFrame::Raised);
	m_LPFLabelList.append(mHz1_8Label);

	QLabel *mHz3_5Label = new QLabel("3.5 MHz", this);
	mHz3_5Label->setFrameStyle(QFrame::Box | QFrame::Raised);
	m_LPFLabelList.append(mHz3_5Label);

	QLabel *mHz7Label = new QLabel("7.0 MHz", this);
	mHz7Label->setFrameStyle(QFrame::Box | QFrame::Raised);
	m_LPFLabelList.append(mHz7Label);

	QLabel *mHz10Label = new QLabel("10 MHz", this);
	mHz10Label->setFrameStyle(QFrame::Box | QFrame::Raised);
	m_LPFLabelList.append(mHz10Label);

	QLabel *mHz14Label = new QLabel("14-18 MHz", this);
	mHz14Label->setFrameStyle(QFrame::Box | QFrame::Raised);
	m_LPFLabelList.append(mHz14Label);

	QLabel *mHz21Label = new QLabel("21-29 MHz", this);
	mHz21Label->setFrameStyle(QFrame::Box | QFrame::Raised);
	m_LPFLabelList.append(mHz21Label);

	QLabel *mHz50Label = new QLabel("50 MHz", this);
	mHz50Label->setFrameStyle(QFrame::Box | QFrame::Raised);
	m_LPFLabelList.append(mHz50Label);

	// create spinboxes
	for (int i = 0; i < m_lpfFilters; i++) {
		QDoubleSpinBox *loSpinBox = new QDoubleSpinBox(this);
		loSpinBox->setRange(m_LPFFrequencyRangeLoList.at(i).first, m_LPFFrequencyRangeLoList.at(i).second);
		loSpinBox->setDecimals(3);
		loSpinBox->setSingleStep(0.005);
		loSpinBox->setValue(m_LPFLoDefaultFrequencyList.at(i));
		loSpinBox->setFixedWidth(64);
		loSpinBox->setFont(m_fonts.normalFont);

		CHECKED_CONNECT(loSpinBox, SIGNAL(valueChanged(double)), this, SLOT(lpfLoSpinBoxValueChanged(double)));

		m_LPFLoSpinBoxList.append(loSpinBox);

		QDoubleSpinBox *hiSpinBox = new QDoubleSpinBox(this);
		hiSpinBox->setRange(m_LPFFrequencyRangeHiList.at(i).first, m_LPFFrequencyRangeHiList.at(i).second);
		hiSpinBox->setDecimals(3);
		hiSpinBox->setSingleStep(0.005);
		hiSpinBox->setValue(m_LPFHiDefaultFrequencyList.at(i));
		hiSpinBox->setFixedWidth(64);
		hiSpinBox->setFont(m_fonts.normalFont);

		CHECKED_CONNECT(hiSpinBox, SIGNAL(valueChanged(double)), this, SLOT(lpfHiSpinBoxValueChanged(double)));

		m_LPFHiSpinBoxList.append(hiSpinBox);
	}

	// create LEDs
	for (int i = 0; i < 7; i++) {
		QHLed *led = new QHLed("", this);
		led->setFixedSize (10, 10);
		led->setColors(btnOff, btnOff);
		m_LPFActiveBtnList.append(led);
	}

	QGridLayout *gridLayout = new QGridLayout;
	gridLayout->setSpacing(2);

	gridLayout->addWidget(lpfLabel, 0, 0, 1, 3, Qt::AlignCenter);
	gridLayout->addWidget(activeLabel, 0, 3, Qt::AlignCenter);

	gridLayout->addWidget(mHz1_8Label, 1, 0, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFLoSpinBoxList.at(0), 1, 1, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFHiSpinBoxList.at(0), 1, 2, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFActiveBtnList.at(0), 1, 3, Qt::AlignCenter);

	gridLayout->addWidget(mHz3_5Label, 2, 0, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFLoSpinBoxList.at(1), 2, 1, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFHiSpinBoxList.at(1), 2, 2, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFActiveBtnList.at(1), 2, 3, Qt::AlignCenter);

	gridLayout->addWidget(mHz7Label, 3, 0, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFLoSpinBoxList.at(2), 3, 1, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFHiSpinBoxList.at(2), 3, 2, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFActiveBtnList.at(2), 3, 3, Qt::AlignCenter);

	gridLayout->addWidget(mHz10Label, 4, 0, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFLoSpinBoxList.at(3), 4, 1, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFHiSpinBoxList.at(3), 4, 2, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFActiveBtnList.at(3), 4, 3, Qt::AlignCenter);

	gridLayout->addWidget(mHz14Label, 5, 0, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFLoSpinBoxList.at(4), 5, 1, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFHiSpinBoxList.at(4), 5, 2, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFActiveBtnList.at(4), 5, 3, Qt::AlignCenter);

	gridLayout->addWidget(mHz21Label, 6, 0, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFLoSpinBoxList.at(5), 6, 1, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFHiSpinBoxList.at(5), 6, 2, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFActiveBtnList.at(5), 6, 3, Qt::AlignCenter);

	gridLayout->addWidget(mHz50Label, 7, 0, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFLoSpinBoxList.at(6), 7, 1, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFHiSpinBoxList.at(6), 7, 2, Qt::AlignCenter);
	gridLayout->addWidget(m_LPFActiveBtnList.at(6), 7, 3, Qt::AlignCenter);

	LPFGroup = new QGroupBox(tr("Low Pass Filter Cutoff Frequency Options (MHz)"), this);
	LPFGroup->setMinimumWidth(m_minimumGroupBoxWidth);
	LPFGroup->setLayout(gridLayout);
	LPFGroup->setFont(QFont("Arial", 8));
}

void AlexFilterWidget::initAlexValues() {
}

void AlexFilterWidget::setFilterValues() {
	m_HPFFrequencyRangeLoList << qMakePair(1.0, 2.0) << qMakePair(5.0, 7.5) << qMakePair(8.0, 10.5) << qMakePair(11.0, 14.5) << qMakePair(18.0, 22.0) << qMakePair(45.0, 55.0);
	m_HPFFrequencyRangeHiList << qMakePair(1.0, 2.0) << qMakePair(5.0, 7.5) << qMakePair(8.0, 10.5) << qMakePair(11.0, 14.5) << qMakePair(18.0, 22.0) << qMakePair(45.0, 55.0);
	m_LPFFrequencyRangeLoList << qMakePair(1.5, 2.5) << qMakePair(2.5, 4.5) << qMakePair(5.0, 8.5) << qMakePair(8.0, 12.0) << qMakePair(12.0, 19.5) << qMakePair(19.0, 31.0) << qMakePair(45.0, 55.0);
	m_LPFFrequencyRangeHiList << qMakePair(1.5, 2.5) << qMakePair(2.5, 4.5) << qMakePair(5.0, 8.5) << qMakePair(8.0, 12.0) << qMakePair(12.0, 19.5) << qMakePair(19.0, 31.0) << qMakePair(45.0, 55.0);

	m_HPFLoDefaultFrequencyList << 1.5 << 6.5 << 9.5 << 13.0 << 20.0 << 50.0;
	m_HPFHiDefaultFrequencyList << 1.5 << 6.5 << 9.5 << 13.0 << 20.0 << 50.0;
	m_LPFLoDefaultFrequencyList << 2.1 << 4.0 << 7.6 << 11.0 << 19.0 << 30.5 << 54.0;
	m_LPFHiDefaultFrequencyList << 2.1 << 4.0 << 7.6 << 11.0 << 19.0 << 30.5 << 54.0;
}

void AlexFilterWidget::setAlexConfiguration(double frequency) {
	if (m_alexConfig & 0x01) { // Manual Mode
		foreach(QHLed *led, m_HPFActiveBtnList)
			led->setColors(btnOff, btnOff);

		if (bypassAll)      m_HPFActiveBtnList.at(6)->setColors(btnOn, btnOn);
		else if (lowNoise6m) m_HPFActiveBtnList.at(5)->setColors(btnOn, btnOn);
		else if (hpf20MHz)   m_HPFActiveBtnList.at(4)->setColors(btnOn, btnOn);
		else if (hpf13MHz)   m_HPFActiveBtnList.at(3)->setColors(btnOn, btnOn);
		else if (hpf9_5MHz)  m_HPFActiveBtnList.at(2)->setColors(btnOn, btnOn);
		else if (hpf6_5MHz)  m_HPFActiveBtnList.at(1)->setColors(btnOn, btnOn);
		else if (hpf1_5MHz)  m_HPFActiveBtnList.at(0)->setColors(btnOn, btnOn);

	} else { // Auto Mode
		foreach(QHLed *led, m_HPFActiveBtnList)
			led->setColors(btnOff, btnOff);

		int hpfActive = 6; // default bypass
		if (frequency >= m_HPFLoSpinBoxList.at(5)->value() && frequency <= m_HPFHiSpinBoxList.at(5)->value()) {
			hpfActive = 5;
		} else if (frequency >= m_HPFLoSpinBoxList.at(4)->value() && frequency <= m_HPFHiSpinBoxList.at(4)->value()) {
			hpfActive = 4;
		} else if (frequency >= m_HPFLoSpinBoxList.at(3)->value() && frequency <= m_HPFHiSpinBoxList.at(3)->value()) {
			hpfActive = 3;
		} else if (frequency >= m_HPFLoSpinBoxList.at(2)->value() && frequency <= m_HPFHiSpinBoxList.at(2)->value()) {
			hpfActive = 2;
		} else if (frequency >= m_HPFLoSpinBoxList.at(1)->value() && frequency <= m_HPFHiSpinBoxList.at(1)->value()) {
			hpfActive = 1;
		} else if (frequency >= m_HPFLoSpinBoxList.at(0)->value() && frequency <= m_HPFHiSpinBoxList.at(0)->value()) {
			hpfActive = 0;
		}
		m_HPFActiveBtnList.at(hpfActive)->setColors(btnOn, btnOn);

		foreach(QHLed *led, m_LPFActiveBtnList)
			led->setColors(btnOff, btnOff);

		int lpfActive = 6; // default LPF
		if (frequency <= m_LPFLoSpinBoxList.at(0)->value()) {
			lpfActive = 0;
		} else if (frequency > m_LPFLoSpinBoxList.at(0)->value() && frequency <= m_LPFLoSpinBoxList.at(1)->value()) {
			lpfActive = 1;
		} else if (frequency > m_LPFLoSpinBoxList.at(1)->value() && frequency <= m_LPFLoSpinBoxList.at(2)->value()) {
			lpfActive = 2;
		} else if (frequency > m_LPFLoSpinBoxList.at(2)->value() && frequency <= m_LPFLoSpinBoxList.at(3)->value()) {
			lpfActive = 3;
		} else if (frequency > m_LPFLoSpinBoxList.at(3)->value() && frequency <= m_LPFLoSpinBoxList.at(4)->value()) {
			lpfActive = 4;
		} else if (frequency > m_LPFLoSpinBoxList.at(4)->value() && frequency <= m_LPFLoSpinBoxList.at(5)->value()) {
			lpfActive = 5;
		} else if (frequency > m_LPFLoSpinBoxList.at(5)->value()) {
			lpfActive = 6;
		}
		m_LPFActiveBtnList.at(lpfActive)->setColors(btnOn, btnOn);
	}
}

void AlexFilterWidget::hpfLoSpinBoxValueChanged(double value) {
	QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *>(sender());
	int filter = m_HPFLoSpinBoxList.indexOf(spinBox);
	if (filter >= 0) {
		emit hpfLoFrequencyRequested(filter, (long)(value * 1000));
	}
}

void AlexFilterWidget::hpfHiSpinBoxValueChanged(double value) {
	QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *>(sender());
	int filter = m_HPFHiSpinBoxList.indexOf(spinBox);
	if (filter >= 0) {
		emit hpfHiFrequencyRequested(filter, (long)(value * 1000));
	}
}

void AlexFilterWidget::lpfLoSpinBoxValueChanged(double value) {
	QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *>(sender());
	int filter = m_LPFLoSpinBoxList.indexOf(spinBox);
	if (filter >= 0) {
		emit lpfLoFrequencyRequested(filter, (long)(value * 1000));
	}
}

void AlexFilterWidget::lpfHiSpinBoxValueChanged(double value) {
	QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *>(sender());
	int filter = m_LPFHiSpinBoxList.indexOf(spinBox);
	if (filter >= 0) {
		emit lpfHiFrequencyRequested(filter, (long)(value * 1000));
	}
}

void AlexFilterWidget::setCurrentReceiver(int rx) {
	m_receiver = rx;
}

void AlexFilterWidget::manualFilterBtnClicked() {
	bool target = (manualFilterBtn->btnState() == AeroButton::OFF);
	emit manualFilterRequested(target);
}

void AlexFilterWidget::defaultValuesBtnClicked() {
	for (int i = 0; i < m_HPFLoSpinBoxList.size(); ++i) {
		m_HPFLoSpinBoxList[i]->setValue(m_HPFLoDefaultFrequencyList.at(i));
		m_HPFHiSpinBoxList[i]->setValue(m_HPFHiDefaultFrequencyList.at(i));
	}
	for (int i = 0; i < m_LPFLoSpinBoxList.size(); ++i) {
		m_LPFLoSpinBoxList[i]->setValue(m_LPFLoDefaultFrequencyList.at(i));
		m_LPFHiSpinBoxList[i]->setValue(m_LPFHiDefaultFrequencyList.at(i));
	}
}

void AlexFilterWidget::bypassAllHPFBtnClicked() {
	bypassAll = true;
	lowNoise6m = false;
	hpf20MHz = false;
	hpf13MHz = false;
	hpf9_5MHz = false;
	hpf6_5MHz = false;
	hpf1_5MHz = false;

	m_alexConfig &= 0xFF01;
	m_alexConfig |= 0x80;
	emit alexConfigurationRequested(m_alexConfig);
}

void AlexFilterWidget::lowNoise6mAmpBtnClicked() {
	bypassAll = false;
	lowNoise6m = true;
	hpf20MHz = false;
	hpf13MHz = false;
	hpf9_5MHz = false;
	hpf6_5MHz = false;
	hpf1_5MHz = false;

	m_alexConfig &= 0xFF01;
	m_alexConfig |= 0x40;
	emit alexConfigurationRequested(m_alexConfig);
}

void AlexFilterWidget::hpf20MHzBtnClicked() {
	bypassAll = false;
	lowNoise6m = false;
	hpf20MHz = true;
	hpf13MHz = false;
	hpf9_5MHz = false;
	hpf6_5MHz = false;
	hpf1_5MHz = false;

	m_alexConfig &= 0xFF01;
	m_alexConfig |= 0x20;
	emit alexConfigurationRequested(m_alexConfig);
}

void AlexFilterWidget::hpf13MHzBtnClicked() {
	bypassAll = false;
	lowNoise6m = false;
	hpf20MHz = false;
	hpf13MHz = true;
	hpf9_5MHz = false;
	hpf6_5MHz = false;
	hpf1_5MHz = false;

	m_alexConfig &= 0xFF01;
	m_alexConfig |= 0x10;
	emit alexConfigurationRequested(m_alexConfig);
}

void AlexFilterWidget::hpf9_5MHzBtnClicked() {
	bypassAll = false;
	lowNoise6m = false;
	hpf20MHz = false;
	hpf13MHz = false;
	hpf9_5MHz = true;
	hpf6_5MHz = false;
	hpf1_5MHz = false;

	m_alexConfig &= 0xFF01;
	m_alexConfig |= 0x08;
	emit alexConfigurationRequested(m_alexConfig);
}

void AlexFilterWidget::hpf6_5MHzBtnClicked() {
	bypassAll = false;
	lowNoise6m = false;
	hpf20MHz = false;
	hpf13MHz = false;
	hpf9_5MHz = false;
	hpf6_5MHz = true;
	hpf1_5MHz = false;

	m_alexConfig &= 0xFF01;
	m_alexConfig |= 0x04;
	emit alexConfigurationRequested(m_alexConfig);
}

void AlexFilterWidget::hpf1_5MHzBtnClicked() {
	bypassAll = false;
	lowNoise6m = false;
	hpf20MHz = false;
	hpf13MHz = false;
	hpf9_5MHz = false;
	hpf6_5MHz = false;
	hpf1_5MHz = true;

	m_alexConfig &= 0xFF01;
	m_alexConfig |= 0x02;
	emit alexConfigurationRequested(m_alexConfig);
}
