#define LOG_HPSDR_WIDGET

#include <QBoxLayout>

#include "cusdr_hpsdrWidget.h"

#define	btn_height		22
#define	btn_width		74
#define	btn_width2		52
#define	btn_widths		42

HPSDRWidget::HPSDRWidget(QWidget *parent) 
	: QWidget(parent)
	, m_serverMode(QSDR::SDRMode)
	, m_hwInterface(QSDR::NoInterfaceMode)
	, m_hwInterfaceTemp(QSDR::NoInterfaceMode)
	, m_dataEngineState(QSDR::DataEngineDown)
	, m_minimumWidgetWidth(0)
	, m_minimumGroupBoxWidth(0)
	, m_numberOfReceivers(1)
	, m_hpsdrHardware(0)
{
	setContentsMargins(4, 8, 4, 0);
	setMouseTracking(true);
	
	m_firmwareCheck = false;

	createSource10MhzExclusiveGroup();
	createSource122_88MhzExclusiveGroup();

	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(0,0,0,0);
	mainLayout->addSpacing(8);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(0);
	hbox1->setContentsMargins(4, 0, 4, 0);
	hbox1->addWidget(hpsdrHardwareBtnGroup());

	QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->setSpacing(0);
	hbox2->setContentsMargins(4, 0, 4, 0);
	hbox2->addWidget(source10MhzExclusiveGroup);

	QHBoxLayout *hbox3 = new QHBoxLayout();
	hbox3->setSpacing(0);
	hbox3->setContentsMargins(4, 0, 4, 0);
	hbox3->addWidget(source122_88MhzExclusiveGroup);

	QHBoxLayout *hbox4 = new QHBoxLayout();
	hbox4->setSpacing(0);
	hbox4->setContentsMargins(4, 0, 4, 0);
	hbox4->addWidget(sampleRateExclusiveGroup());

	QHBoxLayout *hbox5 = new QHBoxLayout();
	hbox5->setSpacing(0);
	hbox5->setContentsMargins(4, 0, 4, 0);
	hbox5->addWidget(numberOfReceiversGroup());

	mainLayout->addLayout(hbox1);
	mainLayout->addLayout(hbox2);
	mainLayout->addLayout(hbox3);
	mainLayout->addLayout(hbox4);
	mainLayout->addLayout(hbox5);
	mainLayout->addStretch();
	setLayout(mainLayout);

	m_receiverComboBox->blockSignals(true);
	m_receiverComboBox->setCurrentIndex(m_numberOfReceivers - 1);
	m_receiverComboBox->blockSignals(false);

	setHPSDRHardware();
}

HPSDRWidget::~HPSDRWidget() {
	disconnect(0, 0, 0);
}

QGroupBox* HPSDRWidget::hpsdrHardwareBtnGroup() {
	modulesPresenceBtn = new AeroButton("Modules", this);
	modulesPresenceBtn->setRoundness(0);
	modulesPresenceBtn->setFixedSize(btn_width, btn_height);
	modulesPresenceBtn->setBtnState(AeroButton::OFF);
	hardwareBtnList.append(modulesPresenceBtn);
	
	CHECKED_CONNECT(
		modulesPresenceBtn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::hpsdrHardwareChanged);
	
	hermesPresenceBtn = new AeroButton("Hermes", this);
	hermesPresenceBtn->setRoundness(0);
	hermesPresenceBtn->setFixedSize(btn_width, btn_height);
	hermesPresenceBtn->setBtnState(AeroButton::OFF);
	hardwareBtnList.append(hermesPresenceBtn);

	CHECKED_CONNECT(
		hermesPresenceBtn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::hpsdrHardwareChanged);

	penelopePresenceBtn = new AeroButton("Penelope", this);
	penelopePresenceBtn->setRoundness(0);
	penelopePresenceBtn->setFixedSize(btn_width, btn_height);
	penelopePresenceBtn->setBtnState(AeroButton::OFF);
	
	CHECKED_CONNECT(
		penelopePresenceBtn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::penelopePresenceChanged);

	pennyPresenceBtn = new AeroButton("PennyLane", this);
	pennyPresenceBtn->setRoundness(0);
	pennyPresenceBtn->setFixedSize(btn_width, btn_height);
	pennyPresenceBtn->setBtnState(AeroButton::OFF);
	
	CHECKED_CONNECT(
		pennyPresenceBtn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::pennyPresenceChanged);

	mercuryPresenceBtn = new AeroButton("Mercury", this);
	mercuryPresenceBtn->setRoundness(0);
	mercuryPresenceBtn->setFixedSize(btn_width, btn_height);
	mercuryPresenceBtn->setBtnState(AeroButton::OFF);
	
	CHECKED_CONNECT(
		mercuryPresenceBtn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::mercuryPresenceChanged);

	alexPresenceBtn = new AeroButton("Alex", this);
	alexPresenceBtn->setRoundness(0);
	alexPresenceBtn->setFixedSize(btn_width, btn_height);
	alexPresenceBtn->setBtnState(AeroButton::OFF);
	
	CHECKED_CONNECT(
		alexPresenceBtn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::alexPresenceChanged);

	excaliburPresenceBtn = new AeroButton("Excalibur", this);
	excaliburPresenceBtn->setRoundness(0);
	excaliburPresenceBtn->setFixedSize(btn_width, btn_height);
	excaliburPresenceBtn->setBtnState(AeroButton::OFF);
	
	CHECKED_CONNECT(
		excaliburPresenceBtn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::excaliburPresenceChanged);
	
	firmwareCheckBtn = new AeroButton("On", this);
	firmwareCheckBtn->setRoundness(0);
	firmwareCheckBtn->setFixedSize(btn_widths, btn_height);

	if (m_firmwareCheck) {
		firmwareCheckBtn->setBtnState(AeroButton::ON);
		firmwareCheckBtn->setText("On");
	} else {
		firmwareCheckBtn->setBtnState(AeroButton::OFF);
		firmwareCheckBtn->setText("Off");
	}

	CHECKED_CONNECT(
		firmwareCheckBtn,
		&AeroButton::released,
		this,
		&HPSDRWidget::hpsdrHardwareChanged);

	m_fwCheckLabel = new QLabel("Firmware Check:", this);
	m_fwCheckLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	m_detectedBoardLabel = new QLabel("HPSDR Board: unknown", this);
	m_detectedBoardLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(4);
	hbox1->addStretch();
	hbox1->addWidget(modulesPresenceBtn);
	hbox1->addWidget(hermesPresenceBtn);
	
	QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->setSpacing(4);
	hbox2->addStretch();
	hbox2->addWidget(penelopePresenceBtn);
	hbox2->addWidget(pennyPresenceBtn);
	hbox2->addWidget(mercuryPresenceBtn);
	hbox2->addWidget(excaliburPresenceBtn);
	hbox2->addWidget(alexPresenceBtn);

	QHBoxLayout *hbox3 = new QHBoxLayout();
	hbox3->setSpacing(4);
	hbox3->addStretch();
	hbox3->addWidget(m_detectedBoardLabel);
	hbox3->addSpacing(4);
	hbox3->addWidget(m_fwCheckLabel);
	hbox3->addWidget(firmwareCheckBtn);

	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setSpacing(4);
	vbox->addSpacing(6);
	vbox->addLayout(hbox1);
	vbox->addSpacing(6);
	vbox->addLayout(hbox2);
	vbox->addSpacing(6);
	vbox->addLayout(hbox3);
	
	m_hpsdrHardwareGroupBox = new QGroupBox(tr("HPSDR Hardware Selection"), this);
	m_hpsdrHardwareGroupBox->setMinimumWidth(m_minimumGroupBoxWidth);
	m_hpsdrHardwareGroupBox->setLayout(vbox);
	m_hpsdrHardwareGroupBox->setFont(QFont("Arial", 8));

	return m_hpsdrHardwareGroupBox;
}

void HPSDRWidget::createSource10MhzExclusiveGroup() {
	atlasBtn = new AeroButton("Atlas", this);
	atlasBtn->setRoundness(0);
	atlasBtn->setFixedSize(btn_width, btn_height);
	atlasBtn->setBtnState(AeroButton::OFF);
	source10MhzBtnList.append(atlasBtn);

	CHECKED_CONNECT(
		atlasBtn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::source10MhzChanged);

	penelopeBtn = new AeroButton("Penelope", this);
	penelopeBtn->setRoundness(0);
	penelopeBtn->setFixedSize(btn_width, btn_height);
	penelopeBtn->setBtnState(AeroButton::OFF);
	source10MhzBtnList.append(penelopeBtn);

	CHECKED_CONNECT(
		penelopeBtn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::source10MhzChanged);

	mercuryBtn = new AeroButton("Mercury", this);
	mercuryBtn->setRoundness(0);
	mercuryBtn->setFixedSize(btn_width, btn_height);
	mercuryBtn->setBtnState(AeroButton::OFF);
	source10MhzBtnList.append(mercuryBtn);

	CHECKED_CONNECT(
		mercuryBtn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::source10MhzChanged);

	AeroButton *noneBtn = new AeroButton("None", this);
	noneBtn->setRoundness(0);
	noneBtn->setFixedSize(btn_width, btn_height);
	noneBtn->setBtnState(AeroButton::OFF);
	source10MhzBtnList.append(noneBtn);

	CHECKED_CONNECT(
		noneBtn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::source10MhzChanged);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(4);
	hbox1->addStretch();
	hbox1->addWidget(atlasBtn);
	hbox1->addWidget(penelopeBtn);
	hbox1->addWidget(mercuryBtn);
	hbox1->addWidget(noneBtn);

	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setSpacing(4);
	vbox->addSpacing(6);
	vbox->addLayout(hbox1);
	
	source10MhzExclusiveGroup = new QGroupBox(tr("10 MHz Clock Reference Source Selection"), this);
	source10MhzExclusiveGroup->setMinimumWidth(m_minimumGroupBoxWidth);
	source10MhzExclusiveGroup->setLayout(vbox);
	source10MhzExclusiveGroup->setFont(QFont("Arial", 8));
}

void HPSDRWidget::createSource122_88MhzExclusiveGroup() {
	penelope2Btn = new AeroButton("Penelope", this);
	penelope2Btn->setRoundness(0);
	penelope2Btn->setFixedSize(btn_width, btn_height);
	penelope2Btn->setBtnState(AeroButton::OFF);

	CHECKED_CONNECT(
		penelope2Btn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::source122_88MhzChanged);

	mercury2Btn = new AeroButton("Mercury", this);
	mercury2Btn->setRoundness(0);
	mercury2Btn->setFixedSize(btn_width, btn_height);
	mercury2Btn->setBtnState(AeroButton::OFF);

	CHECKED_CONNECT(
		mercury2Btn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::source122_88MhzChanged);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(4);
	hbox1->addStretch();
	hbox1->addWidget(penelope2Btn);
	hbox1->addWidget(mercury2Btn);

	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setSpacing(4);
	vbox->addSpacing(6);
	vbox->addLayout(hbox1);
	
	source122_88MhzExclusiveGroup = new QGroupBox(tr("122.88 MHz Clock Reference Source Selection"), this);
	source122_88MhzExclusiveGroup->setMinimumWidth(m_minimumGroupBoxWidth);
	source122_88MhzExclusiveGroup->setLayout(vbox);
	source122_88MhzExclusiveGroup->setFont(QFont("Arial", 8));
}

QGroupBox *HPSDRWidget::sampleRateExclusiveGroup() {
	samplerate48Btn = new AeroButton("48 kHz", this);
	samplerate48Btn->setRoundness(0);
	samplerate48Btn->setFixedSize(btn_width, btn_height);
	samplerate48Btn->setBtnState(AeroButton::OFF);
	samplerateBtnList.append(samplerate48Btn);

	CHECKED_CONNECT(
		samplerate48Btn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::sampleRateChanged);

	samplerate96Btn = new AeroButton("96 kHz", this);
	samplerate96Btn->setRoundness(0);
	samplerate96Btn->setFixedSize(btn_width, btn_height);
	samplerate96Btn->setBtnState(AeroButton::OFF);
	samplerateBtnList.append(samplerate96Btn);

	CHECKED_CONNECT(
		samplerate96Btn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::sampleRateChanged);

	samplerate192Btn = new AeroButton("192 kHz", this);
	samplerate192Btn->setRoundness(0);
	samplerate192Btn->setFixedSize(btn_width, btn_height);
	samplerate192Btn->setBtnState(AeroButton::OFF);
	samplerateBtnList.append(samplerate192Btn);

	CHECKED_CONNECT(
		samplerate192Btn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::sampleRateChanged);

	samplerate384Btn = new AeroButton("384 kHz", this);
	samplerate384Btn->setRoundness(0);
	samplerate384Btn->setFixedSize(btn_width, btn_height);
	samplerate384Btn->setBtnState(AeroButton::OFF);
	samplerateBtnList.append(samplerate384Btn);

	CHECKED_CONNECT(
		samplerate384Btn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::sampleRateChanged);

    samplerate768Btn = new AeroButton("768 kHz", this);
    samplerate768Btn->setRoundness(0);
    samplerate768Btn->setFixedSize(btn_width, btn_height);
    samplerate768Btn->setBtnState(AeroButton::OFF);
    samplerate768Btn->setEnabled(false); // P2 / Soapy only
    samplerateBtnList.append(samplerate768Btn);

    CHECKED_CONNECT(
        samplerate768Btn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::sampleRateChanged);

    samplerate1536Btn = new AeroButton("1.536 MHz", this);
    samplerate1536Btn->setRoundness(0);
    samplerate1536Btn->setFixedSize(btn_width, btn_height);
    samplerate1536Btn->setBtnState(AeroButton::OFF);
    samplerate1536Btn->setEnabled(false); // P2 / Soapy only
    samplerateBtnList.append(samplerate1536Btn);

    CHECKED_CONNECT(
        samplerate1536Btn, 
		&AeroButton::released, 
		this, 
		&HPSDRWidget::sampleRateChanged);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(4);
	hbox1->addStretch();
	hbox1->addWidget(samplerate48Btn);
	hbox1->addWidget(samplerate96Btn);
	hbox1->addWidget(samplerate192Btn);
	hbox1->addWidget(samplerate384Btn);
	hbox1->addWidget(samplerate768Btn);
	hbox1->addWidget(samplerate1536Btn);

	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setSpacing(4);
	vbox->addSpacing(6);
	vbox->addLayout(hbox1);
	
	m_sampleRateGroupBox = new QGroupBox(tr("Sample Rate Selection"), this);
	m_sampleRateGroupBox->setMinimumWidth(m_minimumGroupBoxWidth);
	m_sampleRateGroupBox->setLayout(vbox);
	m_sampleRateGroupBox->setFont(QFont("Arial", 8));

	return m_sampleRateGroupBox;
}

QGroupBox *HPSDRWidget::numberOfReceiversGroup() {
	m_receiverComboBox = new QComboBox(this);
	m_receiverComboBox->setMinimumContentsLength(4);

	QString str = "%1";
	for (int i = 0; i < MAX_RECEIVERS; i++)
		m_receiverComboBox->addItem(str.arg(i+1));

	CHECKED_CONNECT(
		m_receiverComboBox,
		&QComboBox::currentIndexChanged,
		this,
		&HPSDRWidget::receiverComboBoxChanged);

	m_receiversLabel = new QLabel("Receivers:", this);
    m_receiversLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(5);
	hbox1->addWidget(m_receiversLabel);
	hbox1->addWidget(m_receiverComboBox);
	hbox1->addStretch();

	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setSpacing(4);
	vbox->addSpacing(6);
	vbox->addLayout(hbox1);
	
	m_numberOfReceiversGroupBox = new QGroupBox(tr("Receiver Count Selection"), this);
	m_numberOfReceiversGroupBox->setMinimumWidth(m_minimumGroupBoxWidth);
	m_numberOfReceiversGroupBox->setLayout(vbox);
	m_numberOfReceiversGroupBox->setFont(QFont("Arial", 8));

	return m_numberOfReceiversGroupBox;
}

void HPSDRWidget::setHwInterface(QSDR::_HWInterfaceMode mode) {
	if (m_hwInterface != mode) {
		m_hwInterface = mode;
		hwInterfaceChanged();
	}
	updateExtendedSampleRates();
	update();
}

void HPSDRWidget::hwInterfaceChanged() {
	// HPSDR-specific controls are irrelevant in Soapy mode.
	const bool isHpsdr = (m_hwInterface == QSDR::Metis || m_hwInterface == QSDR::Hermes);
	if (m_hpsdrHardwareGroupBox)
		m_hpsdrHardwareGroupBox->setVisible(isHpsdr);
	if (source10MhzExclusiveGroup)
		source10MhzExclusiveGroup->setVisible(isHpsdr && m_hpsdrHardware == 0);
	if (source122_88MhzExclusiveGroup)
		source122_88MhzExclusiveGroup->setVisible(isHpsdr && m_hpsdrHardware == 0);

	m_hwInterfaceTemp = m_hwInterface;
}

void HPSDRWidget::setHpsdrHardware(int hw) {
	m_hpsdrHardware = hw;
	setHPSDRHardware();
}

void HPSDRWidget::setHPSDRHardware() {
	foreach(AeroButton *btn, hardwareBtnList) {
		btn->setBtnState(AeroButton::OFF);
		btn->update();
	}

	AeroButton *button = nullptr;
	if (m_hpsdrHardware >= 0 && m_hpsdrHardware < hardwareBtnList.size()) {
		button = hardwareBtnList.at(m_hpsdrHardware);
		button->setBtnState(AeroButton::ON);
		button->update();
	}

	// Atlas modules expose clock-source controls; Hermes/Soapy do not.
	const bool showClocks =
		(m_hwInterface == QSDR::Metis || m_hwInterface == QSDR::Hermes) && m_hpsdrHardware == 0;
	if (source10MhzExclusiveGroup)
		source10MhzExclusiveGroup->setVisible(showClocks);
	if (source122_88MhzExclusiveGroup)
		source122_88MhzExclusiveGroup->setVisible(showClocks);

	applyHardwarePresenceEnablement();
}

void HPSDRWidget::setNumberOfReceivers(int count) {
	m_numberOfReceivers = count;
	const bool prev = m_receiverComboBox->blockSignals(true);
	m_receiverComboBox->setCurrentIndex(count - 1);
	m_receiverComboBox->blockSignals(prev);
}

void HPSDRWidget::setFirmwareCheck(bool check) {
	m_firmwareCheck = check;
	firmwareCheckBtn->blockSignals(true);
	if (m_firmwareCheck) {
		firmwareCheckBtn->setBtnState(AeroButton::ON);
		firmwareCheckBtn->setText("On");
	} else {
		firmwareCheckBtn->setBtnState(AeroButton::OFF);
		firmwareCheckBtn->setText("Off");
	}
	firmwareCheckBtn->blockSignals(false);
}

void HPSDRWidget::set10MhzSource(int src) {
	foreach(AeroButton *btn, source10MhzBtnList) {
		btn->setBtnState(AeroButton::OFF);
		btn->update();
	}
	if (src >= 0 && src < source10MhzBtnList.size()) {
		source10MhzBtnList.at(src)->setBtnState(AeroButton::ON);
		source10MhzBtnList.at(src)->update();
	}
}

void HPSDRWidget::set122_88MhzSource(int src) {
	penelope2Btn->blockSignals(true);
	mercury2Btn->blockSignals(true);
	if (src == 0) {
		penelope2Btn->setBtnState(AeroButton::ON);
		mercury2Btn->setBtnState(AeroButton::OFF);
	} else {
		penelope2Btn->setBtnState(AeroButton::OFF);
		mercury2Btn->setBtnState(AeroButton::ON);
	}
	penelope2Btn->blockSignals(false);
	mercury2Btn->blockSignals(false);
	penelope2Btn->update();
	mercury2Btn->update();
}

void HPSDRWidget::setSampleRate(int rate) {
	foreach(AeroButton *btn, samplerateBtnList) {
		btn->setBtnState(AeroButton::OFF);
		btn->update();
	}
	int idx = 0;
	switch (rate) {
		case 48000:   idx = 0; break;
		case 96000:   idx = 1; break;
		case 192000:  idx = 2; break;
		case 384000:  idx = 3; break;
		case 768000:  idx = 4; break;
		case 1536000: idx = 5; break;
	}
	if (idx >= 0 && idx < samplerateBtnList.size()) {
		samplerateBtnList.at(idx)->setBtnState(AeroButton::ON);
		samplerateBtnList.at(idx)->update();
	}
}

void HPSDRWidget::setMercuryPresence(bool pres) {
	mercuryPresenceBtn->blockSignals(true);
	mercuryPresenceBtn->setBtnState(pres ? AeroButton::ON : AeroButton::OFF);
	mercuryPresenceBtn->blockSignals(false);
	mercuryPresenceBtn->update();
}

void HPSDRWidget::setPenelopePresence(bool pres) {
	penelopePresenceBtn->blockSignals(true);
	penelopePresenceBtn->setBtnState(pres ? AeroButton::ON : AeroButton::OFF);
	penelopePresenceBtn->blockSignals(false);
	penelopePresenceBtn->update();
}

void HPSDRWidget::setPennyLanePresence(bool pres) {
	pennyPresenceBtn->blockSignals(true);
	pennyPresenceBtn->setBtnState(pres ? AeroButton::ON : AeroButton::OFF);
	pennyPresenceBtn->blockSignals(false);
	pennyPresenceBtn->update();
}

void HPSDRWidget::setAlexPresence(bool pres) {
	alexPresenceBtn->blockSignals(true);
	alexPresenceBtn->setBtnState(pres ? AeroButton::ON : AeroButton::OFF);
	alexPresenceBtn->blockSignals(false);
	alexPresenceBtn->update();
}

void HPSDRWidget::setExcaliburPresence(bool pres) {
	excaliburPresenceBtn->blockSignals(true);
	excaliburPresenceBtn->setBtnState(pres ? AeroButton::ON : AeroButton::OFF);
	excaliburPresenceBtn->blockSignals(false);
	excaliburPresenceBtn->update();
}

void HPSDRWidget::setCurrentMetisCard(const TNetworkDevicecard& card) {
	m_deviceProtocol = card.protocol;
	updateDetectedBoardLabel(card);
	updateExtendedSampleRates();
}

void HPSDRWidget::updateExtendedSampleRates() {
	// 768 / 1536 kHz are Protocol 2 (and Soapy) only — never enable for P1 Metis/Hermes.
	const bool isP2 = (m_deviceProtocol == 2);
#ifdef HAVE_SOAPYSDR
	const bool enabled = (m_hwInterface == QSDR::SoapySDR) || isP2;
#else
	const bool enabled = isP2;
#endif
	HPSDR_WIDGET_DEBUG << "updateExtendedSampleRates: protocol =" << m_deviceProtocol
	                   << " hw =" << m_hwInterface << " enabled =" << enabled;
	if (samplerate768Btn)
		samplerate768Btn->setEnabled(enabled);
	if (samplerate1536Btn)
		samplerate1536Btn->setEnabled(enabled);
}

void HPSDRWidget::updateDetectedBoardLabel(TNetworkDevicecard card) {
	if (!m_detectedBoardLabel) return;
	if (card.boardName.isEmpty()) {
		m_detectedBoardLabel->setText("Detected: none");
		return;
	}

	m_detectedBoardLabel->setText(
		QString("Detected: %1 (ID 0x%2, P%3)")
			.arg(card.boardName)
			.arg(card.boardID, 2, 16, QLatin1Char('0'))
			.arg(card.protocol));
}

void HPSDRWidget::hpsdrHardwareChanged() {
	AeroButton *button = qobject_cast<AeroButton *>(sender());
	int btn = hardwareBtnList.indexOf(button);
	if (btn >= 0) {
		emit hpsdrHardwareRequested(btn);
	}
}

void HPSDRWidget::penelopePresenceChanged() {
	bool target = (penelopePresenceBtn->btnState() == AeroButton::OFF);
	emit penelopePresenceRequested(target);
}

void HPSDRWidget::pennyPresenceChanged() {
	bool target = (pennyPresenceBtn->btnState() == AeroButton::OFF);
	emit pennyLanePresenceRequested(target);
}

void HPSDRWidget::mercuryPresenceChanged() {
	bool target = (mercuryPresenceBtn->btnState() == AeroButton::OFF);
	emit mercuryPresenceRequested(target);
}

void HPSDRWidget::alexPresenceChanged() {
	bool target = (alexPresenceBtn->btnState() == AeroButton::OFF);
	emit alexPresenceRequested(target);
}

void HPSDRWidget::excaliburPresenceChanged() {
	bool target = (excaliburPresenceBtn->btnState() == AeroButton::OFF);
	emit excaliburPresenceRequested(target);
}

void HPSDRWidget::source10MhzChanged() {
	AeroButton *button = qobject_cast<AeroButton *>(sender());
	int btn = source10MhzBtnList.indexOf(button);
	if (btn >= 0) {
		emit src10MhzRequested(btn);
	}
}

void HPSDRWidget::source122_88MhzChanged() {
    int current = 0;
    if (penelope2Btn->btnState() == AeroButton::ON) current = 0;
    else if (mercury2Btn->btnState() == AeroButton::ON) current = 1;
    int wanted = (current == 0) ? 1 : 0;
    emit src122_88MhzRequested(wanted);
}

void HPSDRWidget::sampleRateChanged() {
	AeroButton *button = qobject_cast<AeroButton *>(sender());
	int btnHit = samplerateBtnList.indexOf(button);
	if (btnHit < 0) return;
	int rate = 48000;
	switch (btnHit) {
		case 0: rate = 48000;   break;
		case 1: rate = 96000;   break;
		case 2: rate = 192000;  break;
		case 3: rate = 384000;  break;
		case 4: rate = 768000;  break;
		case 5: rate = 1536000; break;
	}
	emit sampleRateRequested(rate);
}

void HPSDRWidget::receiverComboBoxChanged(int index) {
	emit numberOfReceiversRequested(index + 1);
}

void HPSDRWidget::setDataEngineRunning(bool running) {
	m_dataEngineState = running ? QSDR::DataEngineUp : QSDR::DataEngineDown;
	if (running)
		disableButtons();
	else
		enableButtons();
}

void HPSDRWidget::disableButtons() {
	modulesPresenceBtn->setEnabled(false);
	hermesPresenceBtn->setEnabled(false);
	penelopePresenceBtn->setEnabled(false);
	pennyPresenceBtn->setEnabled(false);
	mercuryPresenceBtn->setEnabled(false);
	excaliburPresenceBtn->setEnabled(false);
	alexPresenceBtn->setEnabled(false);
	atlasBtn->setEnabled(false);
	penelopeBtn->setEnabled(false);
	mercuryBtn->setEnabled(false);
	penelope2Btn->setEnabled(false);
	mercury2Btn->setEnabled(false);
	m_receiverComboBox->setEnabled(false);
}

void HPSDRWidget::enableButtons() {
	modulesPresenceBtn->setEnabled(true);
	hermesPresenceBtn->setEnabled(true);
	alexPresenceBtn->setEnabled(true);
	penelope2Btn->setEnabled(true);
	mercury2Btn->setEnabled(true);
	m_receiverComboBox->setEnabled(true);
	applyHardwarePresenceEnablement();
}

void HPSDRWidget::applyHardwarePresenceEnablement() {
	// Hermes is integrated — Penelope/PennyLane/Mercury/Excalibur presence N/A.
	const bool hermes = (m_hpsdrHardware == 1);
	const bool locked = (m_dataEngineState == QSDR::DataEngineUp);

	if (hermes) {
		penelopePresenceBtn->blockSignals(true);
		pennyPresenceBtn->blockSignals(true);
		mercuryPresenceBtn->blockSignals(true);
		excaliburPresenceBtn->blockSignals(true);

		penelopePresenceBtn->setBtnState(AeroButton::OFF);
		pennyPresenceBtn->setBtnState(AeroButton::OFF);
		mercuryPresenceBtn->setBtnState(AeroButton::OFF);
		excaliburPresenceBtn->setBtnState(AeroButton::OFF);

		penelopePresenceBtn->blockSignals(false);
		pennyPresenceBtn->blockSignals(false);
		mercuryPresenceBtn->blockSignals(false);
		excaliburPresenceBtn->blockSignals(false);

		penelopePresenceBtn->setEnabled(false);
		pennyPresenceBtn->setEnabled(false);
		mercuryPresenceBtn->setEnabled(false);
		excaliburPresenceBtn->setEnabled(false);
		atlasBtn->setEnabled(false);
		penelopeBtn->setEnabled(false);
		mercuryBtn->setEnabled(false);
	} else if (!locked) {
		penelopePresenceBtn->setEnabled(true);
		pennyPresenceBtn->setEnabled(true);
		mercuryPresenceBtn->setEnabled(true);
		excaliburPresenceBtn->setEnabled(true);
		atlasBtn->setEnabled(true);
		penelopeBtn->setEnabled(true);
		mercuryBtn->setEnabled(true);
	}

	penelopePresenceBtn->update();
	pennyPresenceBtn->update();
	mercuryPresenceBtn->update();
	excaliburPresenceBtn->update();
}
