#define LOG_DISPLAYOPTIONS_WIDGET

#include <QStringList>
#include <QRadioButton>
#include <QButtonGroup>
#include "Models/RadioModel.h"
#include "Models/SliceModel.h"
#include "cusdr_displayWidget.h"

#define	btn_height		15
#define	btn_width		70
#define	btn_widths		38
#define	btn_width2		52
#define	btn_width3		60

DisplayOptionsWidget::DisplayOptionsWidget(RadioModel *model, QWidget *parent)
	: QWidget(parent)
        , m_radioModel(model)
	, m_serverMode(QSDR::SDRMode)
	, m_hwInterface(QSDR::NoInterfaceMode)
	, m_dataEngineState(QSDR::DataEngineDown)
	, m_minimumWidgetWidth(250)
	, m_minimumGroupBoxWidth(240)
	, m_currentReceiver(0)
	, m_btnChooserHit(0)
	, m_framesPerSecond(30)
	, m_sampleRate(48000)
	, m_sMeterHoldTime(1000)
{
	setContentsMargins(4, 0, 4, 0);
	setMouseTracking(true);
	m_panadapterMode = Line;
	m_waterColorMode = Simple;
	m_panAvMode = AV_MODE_NONE;
    m_panDetMode = DET_MODE_AVERAGE;
    m_fftSize = 4096;

    fonts = new CFonts(this);
	m_fonts = fonts->getFonts();

	createFPSGroupBox();
	createPanSpectrumOptions();
	createWidebandPanOptions();
	createWaterfallSpectrumOptions();
	createSMeterOptions();
	createCallSignEditor();

	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(0,0,0,0);
	mainLayout->addSpacing(8);

	QHBoxLayout *hbox1 = new QHBoxLayout;
	hbox1->setSpacing(0);
    hbox1->setContentsMargins(0,0,0,0);
	hbox1->addStretch();
	hbox1->addWidget(m_fpsGroupBox);

	QHBoxLayout *hbox2 = new QHBoxLayout;
	hbox2->setSpacing(0);
    hbox2->setContentsMargins(0,0,0,0);
	hbox2->addStretch();
	hbox2->addWidget(m_panSpectrumOptions);

	QHBoxLayout *hbox3 = new QHBoxLayout;
	hbox3->setSpacing(0);
    hbox3->setContentsMargins(0,0,0,0);
	hbox3->addStretch();
	hbox3->addWidget(m_widebandPanOptions);

	QHBoxLayout *hbox4 = new QHBoxLayout;
	hbox4->setSpacing(0);
    hbox4->setContentsMargins(0,0,0,0);
	hbox4->addStretch();
	hbox4->addWidget(m_waterfallSpectrumOptions);

	QHBoxLayout *hbox5 = new QHBoxLayout;
	hbox5->setSpacing(0);
    hbox5->setContentsMargins(0,0,0,0);
	hbox5->addStretch();
	hbox5->addWidget(m_sMeterOptions);

	QHBoxLayout *hbox6 = new QHBoxLayout;
	hbox6->setSpacing(0);
    hbox6->setContentsMargins(0,0,0,0);
	hbox6->addStretch();
	hbox6->addWidget(m_callSignEditor);

	mainLayout->addLayout(hbox1);
	mainLayout->addLayout(hbox2);
	mainLayout->addLayout(hbox3);
	mainLayout->addLayout(hbox4);
	mainLayout->addLayout(hbox5);
	mainLayout->addLayout(hbox6);
	mainLayout->addStretch();

	setLayout(mainLayout);
}

DisplayOptionsWidget::~DisplayOptionsWidget() {
	disconnect(0, 0, 0);
}

QSize DisplayOptionsWidget::sizeHint() const {
	return QSize(m_minimumWidgetWidth, height());
}

QSize DisplayOptionsWidget::minimumSizeHint() const {
	return QSize(m_minimumWidgetWidth, height());
}

void DisplayOptionsWidget::createFPSGroupBox() {
	m_fpsLevelLabel = new QLabel(" 30 ", this);
	m_fpsLevelLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
	m_fpsLevelLabel->setFixedWidth(30);

	QLabel *lbl = new QLabel("Frames/sec:", this);
	lbl->setFont(m_fonts.normalFont);

	m_fpsSlider = new QSlider(Qt::Horizontal, this);
	m_fpsSlider->setTickPosition(QSlider::NoTicks);
	m_fpsSlider->setFixedSize(130, 12);
	m_fpsSlider->setSingleStep(1);
	m_fpsSlider->setRange(1, 120);
	m_fpsSlider->setValue(m_framesPerSecond);

	CHECKED_CONNECT(m_fpsSlider, &QSlider::valueChanged, this, &DisplayOptionsWidget::fpsValueChanged);

	QGridLayout *gridLayout = new QGridLayout;
	gridLayout->setSpacing(4);

	gridLayout->addWidget(lbl, 0, 0);
	gridLayout->addWidget(m_fpsSlider, 0, 1, Qt::AlignVCenter);
	gridLayout->addWidget(m_fpsLevelLabel, 0, 2);

	m_fpsGroupBox = new QGroupBox(tr("Spectrum Refresh Rate Options"), this);
	m_fpsGroupBox->setMinimumWidth(m_minimumGroupBoxWidth);
	m_fpsGroupBox->setLayout(gridLayout);
	m_fpsGroupBox->setFont(QFont("Arial", 8));
}

void DisplayOptionsWidget::createPanSpectrumOptions() {
	m_avgLevelLabel = new QLabel(" 100 ", this);
	m_avgLevelLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
	m_avgLevelLabel->setFixedWidth(30);

	QList<QString> btnList;
	btnList << "Line" << "Fill" << "Solid";

	m_panadapterMode = Line;

	for (int i = 0; i < 3; i++) {
		AeroButton *btn = new AeroButton(btnList.at(i), this);
		btn->setRoundness(0);
		btn->setFixedSize(btn_width2, btn_height);
		btn->setBtnState(AeroButton::OFF);
		m_panadapterBtnList.append(btn);

		CHECKED_CONNECT(btn, &AeroButton::clicked, this, &DisplayOptionsWidget::panModeChanged);
	}

	switch (m_panadapterMode) {
		case Line:
			m_panadapterBtnList.at(0)->setBtnState(AeroButton::ON);
			break;
		case FilledLine:
			m_panadapterBtnList.at(1)->setBtnState(AeroButton::ON);
			break;
		case Solid:
			m_panadapterBtnList.at(2)->setBtnState(AeroButton::ON);
			break;
	}

	m_avgValue = 100;

	m_fmSqlevel = new QSlider(Qt::Horizontal, this);
	m_fmSqlevel->setTickPosition(QSlider::NoTicks);
	m_fmSqlevel->setFixedSize(130, 12);
	m_fmSqlevel->setSingleStep(1);
	m_fmSqlevel->setValue(80);
	m_fmSqlevel->setRange(1, 100);

	CHECKED_CONNECT(m_fmSqlevel, &QSlider::valueChanged, this, &DisplayOptionsWidget::sqLevelChanged);

	m_avgSlider = new QSlider(Qt::Horizontal, this);
	m_avgSlider->setTickPosition(QSlider::NoTicks);
	m_avgSlider->setFixedSize(130, 12);
	m_avgSlider->setSingleStep(1);
	m_avgSlider->setRange(1, 1000);
	m_avgSlider->setValue(m_avgValue);

	CHECKED_CONNECT(m_avgSlider, &QSlider::valueChanged, this, &DisplayOptionsWidget::averagingFilterCntChanged);

	m_panAverageCombo = new QComboBox(this);
	m_panAverageCombo->setObjectName("panAverageCombo");
	m_panAverageCombo->addItem("Off");
	m_panAverageCombo->addItem("Simple");
	m_panAverageCombo->addItem("Enhanced");
	m_panAverageCombo->setFixedSize(btn_width3, 20);
	m_panAverageCombo->setCurrentIndex(0);

	CHECKED_CONNECT(m_panAverageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DisplayOptionsWidget::panAverageModeChanged);

	m_panDetectorCombo = new QComboBox(this);
	m_panDetectorCombo->setObjectName("panDetectorCombo");
	m_panDetectorCombo->addItem("Average");
	m_panDetectorCombo->addItem("Peak");
	m_panDetectorCombo->addItem("Sample");
	m_panDetectorCombo->addItem("RMS");
	m_panDetectorCombo->setFixedSize(btn_width3, 20);
	m_panDetectorCombo->setCurrentIndex(0);

	CHECKED_CONNECT(m_panDetectorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DisplayOptionsWidget::panDetectorModeChanged);

	m_fftSizeCombo = new QComboBox(this);
	m_fftSizeCombo->setObjectName("fftSizeCombo");
	m_fftSizeCombo->addItem("512", 512);
	m_fftSizeCombo->addItem("1024", 1024);
	m_fftSizeCombo->addItem("2048", 2048);
	m_fftSizeCombo->addItem("4096", 4096);
	m_fftSizeCombo->addItem("8192", 8192);
	m_fftSizeCombo->addItem("16384", 16384);
	m_fftSizeCombo->addItem("32768", 32768);
	m_fftSizeCombo->setFixedSize(btn_width3, 20);
	m_fftSizeCombo->setCurrentIndex(3);

	CHECKED_CONNECT(m_fftSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DisplayOptionsWidget::fftSizeChanged);

	QLabel *avgLabel = new QLabel("Averaging:", this);
	avgLabel->setFont(m_fonts.normalFont);

	QLabel *avgMethodLabel = new QLabel("Method:", this);
	avgMethodLabel->setFont(m_fonts.normalFont);

	QLabel *detMethodLabel = new QLabel("Detector:", this);
	detMethodLabel->setFont(m_fonts.normalFont);

	QLabel *fftLabel = new QLabel("FFT Size:", this);
	fftLabel->setFont(m_fonts.normalFont);

	QLabel *fmSqLabel = new QLabel("FM SQL:", this);
	fmSqLabel->setFont(m_fonts.normalFont);

	QGridLayout *gridLayout = new QGridLayout;
	gridLayout->setSpacing(4);

	for (int i = 0; i < 3; i++) {
		gridLayout->addWidget(m_panadapterBtnList.at(i), 0, i, Qt::AlignCenter);
	}

	QHBoxLayout *hbox1 = new QHBoxLayout;
	hbox1->setSpacing(4);
	hbox1->addWidget(avgLabel);
	hbox1->addWidget(m_avgSlider, Qt::AlignVCenter);
	hbox1->addWidget(m_avgLevelLabel);

	QHBoxLayout *hbox2 = new QHBoxLayout;
	hbox2->setSpacing(4);
	hbox2->addWidget(avgMethodLabel);
	hbox2->addWidget(m_panAverageCombo);
	hbox2->addWidget(detMethodLabel);
	hbox2->addWidget(m_panDetectorCombo);

	QHBoxLayout *hbox3 = new QHBoxLayout;
	hbox3->setSpacing(4);
	hbox3->addWidget(fftLabel);
	hbox3->addWidget(m_fftSizeCombo);
	hbox3->addWidget(fmSqLabel);
	hbox3->addWidget(m_fmSqlevel);

	gridLayout->addLayout(hbox1, 1, 0, 1, 5);
	gridLayout->addLayout(hbox2, 2, 0, 1, 5);
	gridLayout->addLayout(hbox3, 3, 0, 1, 5);

	m_panSpectrumOptions = new QGroupBox(tr("Panadapter Spectrum Options"), this);
	m_panSpectrumOptions->setMinimumWidth(m_minimumGroupBoxWidth);
	m_panSpectrumOptions->setLayout(gridLayout);
	m_panSpectrumOptions->setFont(QFont("Arial", 8));
}

void DisplayOptionsWidget::createWidebandPanOptions() {
	m_wbAvgLevelLabel = new QLabel(" 100 ", this);
	m_wbAvgLevelLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
	m_wbAvgLevelLabel->setFixedWidth(30);

	QList<QString> btnList;
	btnList << "Line" << "Fill" << "Solid";

	m_wbPanadapterMode = Line;

	for (int i = 0; i < 3; i++) {
		AeroButton *btn = new AeroButton(btnList.at(i), this);
		btn->setRoundness(0);
		btn->setFixedSize(btn_width2, btn_height);
		btn->setBtnState(AeroButton::OFF);
		m_wbpanadapterBtnList.append(btn);

		CHECKED_CONNECT(btn, &AeroButton::clicked, this, &DisplayOptionsWidget::wbPanModeChanged);
	}

	switch (m_wbPanadapterMode) {
		case Line:
			m_wbpanadapterBtnList.at(0)->setBtnState(AeroButton::ON);
			break;
		case FilledLine:
			m_wbpanadapterBtnList.at(1)->setBtnState(AeroButton::ON);
			break;
		case Solid:
			m_wbpanadapterBtnList.at(2)->setBtnState(AeroButton::ON);
			break;
	}

	m_wbAvgValue = 100;

	m_wbAvgSlider = new QSlider(Qt::Horizontal, this);
	m_wbAvgSlider->setTickPosition(QSlider::NoTicks);
	m_wbAvgSlider->setFixedSize(130, 12);
	m_wbAvgSlider->setSingleStep(1);
	m_wbAvgSlider->setRange(1, 1000);
	m_wbAvgSlider->setValue(m_wbAvgValue);

	CHECKED_CONNECT(m_wbAvgSlider, &QSlider::valueChanged, this, &DisplayOptionsWidget::setWidebandAveragingCnt);

	QLabel *avgLabel = new QLabel("Averaging:", this);
	avgLabel->setFont(m_fonts.normalFont);

	QGridLayout *gridLayout = new QGridLayout;
	gridLayout->setSpacing(4);

	for (int i = 0; i < 3; i++) {
		gridLayout->addWidget(m_wbpanadapterBtnList.at(i), 0, i, Qt::AlignCenter);
	}

	QHBoxLayout *hbox1 = new QHBoxLayout;
	hbox1->setSpacing(4);
	hbox1->addWidget(avgLabel);
	hbox1->addWidget(m_wbAvgSlider, Qt::AlignVCenter);
	hbox1->addWidget(m_wbAvgLevelLabel);

	gridLayout->addLayout(hbox1, 1, 0, 1, 3);

	m_widebandPanOptions = new QGroupBox(tr("Wideband Spectrum Options"), this);
	m_widebandPanOptions->setMinimumWidth(m_minimumGroupBoxWidth);
	m_widebandPanOptions->setLayout(gridLayout);
	m_widebandPanOptions->setFont(QFont("Arial", 8));
}

void DisplayOptionsWidget::createWaterfallSpectrumOptions() {
	QList<QString> btnList;
	btnList << "Mono" << "Enhanced";

	m_waterColorMode = Simple;

	for (int i = 0; i < 2; i++) {
		AeroButton *btn = new AeroButton(btnList.at(i), this);
		btn->setRoundness(0);
		btn->setFixedSize(btn_width, btn_height);
		btn->setBtnState(AeroButton::OFF);
		m_waterfallColorBtnList.append(btn);

		CHECKED_CONNECT(btn, &AeroButton::clicked, this, &DisplayOptionsWidget::waterfallColorChanged);
	}

	m_waterfallColorBtnList.at(m_waterColorMode)->setBtnState(AeroButton::ON);

	m_waterfallLoOffsetSpinBox = new QSpinBox(this);
	m_waterfallLoOffsetSpinBox->setMinimum(-200);
	m_waterfallLoOffsetSpinBox->setMaximum(200);
	m_waterfallLoOffsetSpinBox->setSingleStep(1);
	m_waterfallLoOffsetSpinBox->setValue(0);

	CHECKED_CONNECT(m_waterfallLoOffsetSpinBox, &QSpinBox::valueChanged, this, &DisplayOptionsWidget::waterfallLoOffsetChanged);

	m_waterfallHiOffsetSpinBox = new QSpinBox(this);
	m_waterfallHiOffsetSpinBox->setMinimum(-200);
	m_waterfallHiOffsetSpinBox->setMaximum(200);
	m_waterfallHiOffsetSpinBox->setSingleStep(1);
	m_waterfallHiOffsetSpinBox->setValue(0);

	CHECKED_CONNECT(m_waterfallHiOffsetSpinBox, &QSpinBox::valueChanged, this, &DisplayOptionsWidget::waterfallHiOffsetChanged);

	m_waterfallLoOffsetLabel = new QLabel("Offset Low (dBm):", this);
    m_waterfallLoOffsetLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	m_waterfallHiOffsetLabel = new QLabel("Offset High (dBm):", this);
    m_waterfallHiOffsetLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	m_waterfallTimeSpinBox = new QSpinBox(this);
	m_waterfallTimeSpinBox->setMinimum(1);
	m_waterfallTimeSpinBox->setMaximum(1000);
	m_waterfallTimeSpinBox->setSingleStep(1);
	m_waterfallTimeSpinBox->setValue(5);

	CHECKED_CONNECT(m_waterfallTimeSpinBox, &QSpinBox::valueChanged, this, &DisplayOptionsWidget::waterfallTimeChanged);

	QLabel *waterfallTimeLabel = new QLabel("Timing (ms):", this);
    waterfallTimeLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	QGridLayout *gridLayout = new QGridLayout;
	gridLayout->setSpacing(4);

	for (int i = 0; i < 2; i++) {
		gridLayout->addWidget(m_waterfallColorBtnList.at(i), 0, i, Qt::AlignCenter);
	}

	gridLayout->addWidget(waterfallTimeLabel, 1, 0);
	gridLayout->addWidget(m_waterfallTimeSpinBox, 1, 1);

	gridLayout->addWidget(m_waterfallLoOffsetLabel, 2, 0);
	gridLayout->addWidget(m_waterfallLoOffsetSpinBox, 2, 1);

	gridLayout->addWidget(m_waterfallHiOffsetLabel, 3, 0);
	gridLayout->addWidget(m_waterfallHiOffsetSpinBox, 3, 1);

	m_waterfallSpectrumOptions = new QGroupBox(tr("Waterfall Options"), this);
	m_waterfallSpectrumOptions->setMinimumWidth(m_minimumGroupBoxWidth);
	m_waterfallSpectrumOptions->setLayout(gridLayout);
	m_waterfallSpectrumOptions->setFont(QFont("Arial", 8));
}

void DisplayOptionsWidget::createSMeterOptions() {
	m_sMeterHoldTimeSpinBox = new QSpinBox(this);
	m_sMeterHoldTimeSpinBox->setMinimum(0);
	m_sMeterHoldTimeSpinBox->setMaximum(5000);
	m_sMeterHoldTimeSpinBox->setSingleStep(10);
	m_sMeterHoldTimeSpinBox->setValue(m_sMeterHoldTime);

	CHECKED_CONNECT(m_sMeterHoldTimeSpinBox, &QSpinBox::valueChanged, this, &DisplayOptionsWidget::sMeterHoldTimeChanged);

	QLabel *holdTimeLabel = new QLabel("Hold Time (ms):", this);
	holdTimeLabel->setFont(m_fonts.normalFont);

	QList<QString> radioBtnList;
	radioBtnList << "Analog" << "Digital";

	QButtonGroup *btnGroup = new QButtonGroup(this);
	for (int i = 0; i < 2; i++) {
		QRadioButton *btn = new QRadioButton(radioBtnList.at(i), this);
		btnGroup->addButton(btn, i);
		m_sMeterTypeRadioBtnList.append(btn);

		CHECKED_CONNECT(btn, &QRadioButton::clicked, this, &DisplayOptionsWidget::sMeterChanged);
	}

	m_sMeterTypeRadioBtnList.at(0)->setChecked(true);

	QGridLayout *gridLayout = new QGridLayout;
	gridLayout->setSpacing(4);

	gridLayout->addWidget(holdTimeLabel, 0, 0);
	gridLayout->addWidget(m_sMeterHoldTimeSpinBox, 0, 1);

	gridLayout->addWidget(m_sMeterTypeRadioBtnList.at(0), 1, 0);
	gridLayout->addWidget(m_sMeterTypeRadioBtnList.at(1), 1, 1);

	m_sMeterOptions = new QGroupBox(tr("S-Meter Options"), this);
	m_sMeterOptions->setMinimumWidth(m_minimumGroupBoxWidth);
	m_sMeterOptions->setLayout(gridLayout);
	m_sMeterOptions->setFont(QFont("Arial", 12));
}

void DisplayOptionsWidget::createCallSignEditor() {
	callSignLineEdit = new QLineEdit(this);
	callSignLineEdit->setText("");

	CHECKED_CONNECT(callSignLineEdit, &QLineEdit::textEdited, this, &DisplayOptionsWidget::callSignTextChanged);

	m_setCallSignBtn = new AeroButton("Set", this);
	m_setCallSignBtn->setRoundness(0);
	m_setCallSignBtn->setFixedSize(btn_width, btn_height);
	
	CHECKED_CONNECT(m_setCallSignBtn, &AeroButton::clicked, this, &DisplayOptionsWidget::callSignChanged);

	QHBoxLayout *hbox1 = new QHBoxLayout;
	hbox1->setSpacing(4);
	hbox1->addWidget(callSignLineEdit);
	hbox1->addWidget(m_setCallSignBtn);

	m_callSignEditor = new QGroupBox(tr("Call Sign Editor"), this);
	m_callSignEditor->setMinimumWidth(m_minimumGroupBoxWidth);
	m_callSignEditor->setLayout(hbox1);
	m_callSignEditor->setFont(QFont("Arial", 12));
}

void DisplayOptionsWidget::setFramesPerSecond(int fps) {
	m_framesPerSecond = fps;
	const QSignalBlocker blocker(m_fpsSlider);
	m_fpsSlider->setValue(fps);
	m_fpsLevelLabel->setText(QString("%1 ").arg(fps, 2, 10, QLatin1Char(' ')));
}

void DisplayOptionsWidget::setSpectrumAveragingCnt(int avg) {
	m_avgValue = avg;
	const QSignalBlocker blocker(m_avgSlider);
	m_avgSlider->setValue(avg);
	m_avgLevelLabel->setText(QString("%1 ").arg(avg, 2, 10, QLatin1Char(' ')));
}

void DisplayOptionsWidget::setWaterfallTime(int val) {
	m_waterfallTime = val;
	const QSignalBlocker blocker(m_waterfallTimeSpinBox);
	m_waterfallTimeSpinBox->setValue(val);
}

void DisplayOptionsWidget::setWaterfallOffsetLo(int val) {
	const QSignalBlocker blocker(m_waterfallLoOffsetSpinBox);
	m_waterfallLoOffsetSpinBox->setValue(val);
}

void DisplayOptionsWidget::setWaterfallOffsetHi(int val) {
	const QSignalBlocker blocker(m_waterfallHiOffsetSpinBox);
	m_waterfallHiOffsetSpinBox->setValue(val);
}

void DisplayOptionsWidget::setSMeterHoldTime(int val) {
	m_sMeterHoldTime = val;
	const QSignalBlocker blocker(m_sMeterHoldTimeSpinBox);
	m_sMeterHoldTimeSpinBox->setValue(val);
}

void DisplayOptionsWidget::setCallsign(const QString& callsign) {
	m_callSingText = callsign;
	const QSignalBlocker blocker(callSignLineEdit);
	callSignLineEdit->setText(callsign);
}

void DisplayOptionsWidget::setPanadapterMode(PanGraphicsMode mode) {
	if (mode != m_panadapterMode) {
		foreach(AeroButton *btn, m_panadapterBtnList) {
			btn->blockSignals(true);
			btn->setBtnState(AeroButton::OFF);
			btn->blockSignals(false);
			btn->update();
		}
		if (static_cast<int>(mode) >= 0 && static_cast<int>(mode) < m_panadapterBtnList.size()) {
			m_panadapterBtnList.at(static_cast<int>(mode))->blockSignals(true);
			m_panadapterBtnList.at(static_cast<int>(mode))->setBtnState(AeroButton::ON);
			m_panadapterBtnList.at(static_cast<int>(mode))->blockSignals(false);
			m_panadapterBtnList.at(static_cast<int>(mode))->update();
		}
		m_panadapterMode = mode;
	}
}

void DisplayOptionsWidget::setWaterfallColorMode(WaterfallColorMode mode) {
	if (mode != m_waterColorMode) {
		foreach(AeroButton *btn, m_waterfallColorBtnList) {
			btn->blockSignals(true);
			btn->setBtnState(AeroButton::OFF);
			btn->blockSignals(false);
			btn->update();
		}
		if (static_cast<int>(mode) >= 0 && static_cast<int>(mode) < m_waterfallColorBtnList.size()) {
			m_waterfallColorBtnList.at(static_cast<int>(mode))->blockSignals(true);
			m_waterfallColorBtnList.at(static_cast<int>(mode))->setBtnState(AeroButton::ON);
			m_waterfallColorBtnList.at(static_cast<int>(mode))->blockSignals(false);
			m_waterfallColorBtnList.at(static_cast<int>(mode))->update();
		}
		m_waterColorMode = mode;
	}
}

void DisplayOptionsWidget::setPanAveragingMode(PanAveragingMode mode) {
	m_panAveragingMode = mode;
	const QSignalBlocker blocker(m_panAverageCombo);
	m_panAverageCombo->setCurrentIndex(static_cast<int>(mode));
}

void DisplayOptionsWidget::setPanDetectorMode(PanDetectorMode mode) {
	m_panDetectorMode = mode;
	const QSignalBlocker blocker(m_panDetectorCombo);
	m_panDetectorCombo->setCurrentIndex(static_cast<int>(mode));
}

void DisplayOptionsWidget::setfftSize(int size) {
	m_fftSize = size;
	const QSignalBlocker blocker(m_fftSizeCombo);
	int idx = m_fftSizeCombo->findData(size);
	if (idx >= 0) {
		m_fftSizeCombo->setCurrentIndex(idx);
	}
}

void DisplayOptionsWidget::setfmsqLevel(int val) {
	const QSignalBlocker blocker(m_fmSqlevel);
	m_fmSqlevel->setValue(val);
}

void DisplayOptionsWidget::setCurrentReceiver(int rx) {
	if (m_currentReceiver == rx) return;
	m_currentReceiver = rx;
	emit receiverChanged(rx);
}

void DisplayOptionsWidget::panModeChanged() {
	AeroButton *button = qobject_cast<AeroButton *>(sender());
	int btnHit = m_panadapterBtnList.indexOf(button);
	if (btnHit >= 0) {
		m_panadapterMode = static_cast<PanGraphicsMode>(btnHit);
		emit graphicsStateRequested(m_currentReceiver, m_panadapterMode, m_waterColorMode);
	}
}

void DisplayOptionsWidget::wbPanModeChanged() {
	AeroButton *button = qobject_cast<AeroButton *>(sender());
	int btnHit = m_wbpanadapterBtnList.indexOf(button);
	if (btnHit >= 0) {
		m_wbPanadapterMode = static_cast<PanGraphicsMode>(btnHit);
		emit graphicsStateRequested(-1, m_wbPanadapterMode, m_waterColorMode);
	}
}

void DisplayOptionsWidget::waterfallColorChanged() {
	AeroButton *button = qobject_cast<AeroButton *>(sender());
	int btnHit = m_waterfallColorBtnList.indexOf(button);
	if (btnHit >= 0) {
		m_waterColorMode = static_cast<WaterfallColorMode>(btnHit);
		emit graphicsStateRequested(m_currentReceiver, m_panadapterMode, m_waterColorMode);
	}
}

void DisplayOptionsWidget::sMeterChanged() {
}

void DisplayOptionsWidget::waterfallTimeChanged(int value) {
	emit waterfallTimeRequested(m_currentReceiver, value);
}

void DisplayOptionsWidget::waterfallLoOffsetChanged(int value) {
	emit waterfallOffsetLoRequested(m_currentReceiver, value);
}

void DisplayOptionsWidget::waterfallHiOffsetChanged(int value) {
	emit waterfallOffsetHiRequested(m_currentReceiver, value);
}

void DisplayOptionsWidget::sMeterHoldTimeChanged(int value) {
	emit sMeterHoldTimeRequested(value);
}

void DisplayOptionsWidget::fpsValueChanged(int value) {
	m_framesPerSecond = value;
	m_fpsLevelLabel->setText(QString("%1 ").arg(m_framesPerSecond, 2, 10, QLatin1Char(' ')));
	emit framesPerSecondRequested(m_currentReceiver, value);
}

void DisplayOptionsWidget::averagingFilterCntChanged(int value) {
	m_avgValue = value;
	m_avgLevelLabel->setText(QString("%1 ").arg(m_avgValue, 2, 10, QLatin1Char(' ')));
	emit spectrumAveragingCntRequested(m_currentReceiver, value);
}

void DisplayOptionsWidget::setWidebandAveragingCnt(int value) {
	m_wbAvgValue = value;
	m_wbAvgLevelLabel->setText(QString("%1 ").arg(m_wbAvgValue, 2, 10, QLatin1Char(' ')));
	emit spectrumAveragingCntRequested(-1, value);
}

void DisplayOptionsWidget::callSignTextChanged(const QString& text) {
	m_callSingText = text;
}

void DisplayOptionsWidget::callSignChanged() {
	emit callsignRequested(m_callSingText);
}

void DisplayOptionsWidget::panAverageModeChanged(int mode)  {
	emit panAveragingModeRequested(m_currentReceiver, mode);
}

void DisplayOptionsWidget::panDetectorModeChanged(int mode)  {
	emit panDetectorModeRequested(m_currentReceiver, mode);
}

void DisplayOptionsWidget::fftSizeChanged(int index)  {
	int size = m_fftSizeCombo->itemData(index).toInt();
	emit fftSizeRequested(m_currentReceiver, size);
}

void DisplayOptionsWidget::sqLevelChanged(int val) {
	emit fmsqLevelRequested(m_currentReceiver, val);
}
