#define LOG_DISPLAYOPTIONS_WIDGET

#include <QStringList>
#include <QVBoxLayout>
#include <QHBoxLayout>
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
	m_wbPanadapterMode = Line;
	m_waterColorMode = Simple;
	m_panAvMode = AV_MODE_NONE;
	m_panDetMode = DET_MODE_AVERAGE;
	m_fftSize = 1; // index into 2k/4k/8k/16k/32k (matches Settings default)

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
	int fontMaxWidth = m_fonts.smallFontMetrics->boundingRect(" 200 ").width();

	m_fpsSlider = new QSlider(Qt::Horizontal, this);
	m_fpsSlider->setTickPosition(QSlider::NoTicks);
	m_fpsSlider->setFixedSize(190, 12);
	m_fpsSlider->setSingleStep(1);
	m_fpsSlider->setRange(1, 80);
	m_fpsSlider->setValue(m_framesPerSecond);

	CHECKED_CONNECT(m_fpsSlider, &QSlider::valueChanged, this, &DisplayOptionsWidget::fpsValueChanged);

	QString str = "%1 ";
	m_fpsLevelLabel = new QLabel(str.arg(m_framesPerSecond, 2, 10, QLatin1Char(' ')), this);
	m_fpsLevelLabel->setFont(m_fonts.smallFont);
	m_fpsLevelLabel->setFixedSize(fontMaxWidth, 12);
	m_fpsLevelLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->addStretch();
	hbox->addWidget(m_fpsSlider);
	hbox->addWidget(m_fpsLevelLabel);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setSpacing(6);
	vbox->addSpacing(6);
	vbox->addLayout(hbox);

	m_fpsGroupBox = new QGroupBox(tr("Frames per Second"), this);
	m_fpsGroupBox->setMinimumWidth(m_minimumGroupBoxWidth);
	m_fpsGroupBox->setLayout(vbox);
	m_fpsGroupBox->setFont(QFont("Arial", 12));
}

void DisplayOptionsWidget::createPanSpectrumOptions() {
	m_PanLineBtn = new AeroButton("Line", this);
	m_PanLineBtn->setRoundness(0);
	m_PanLineBtn->setFixedSize(btn_width, btn_height);
	m_panadapterBtnList.append(m_PanLineBtn);
	CHECKED_CONNECT(m_PanLineBtn, &AeroButton::clicked, this, &DisplayOptionsWidget::panModeChanged);

	m_PanFilledLineBtn = new AeroButton("Filled Line", this);
	m_PanFilledLineBtn->setRoundness(0);
	m_PanFilledLineBtn->setFixedSize(btn_width, btn_height);
	m_panadapterBtnList.append(m_PanFilledLineBtn);
	CHECKED_CONNECT(m_PanFilledLineBtn, &AeroButton::clicked, this, &DisplayOptionsWidget::panModeChanged);

	m_PanSolidBtn = new AeroButton("Solid", this);
	m_PanSolidBtn->setRoundness(0);
	m_PanSolidBtn->setFixedSize(btn_width, btn_height);
	m_panadapterBtnList.append(m_PanSolidBtn);
	CHECKED_CONNECT(m_PanSolidBtn, &AeroButton::clicked, this, &DisplayOptionsWidget::panModeChanged);

	switch (m_panadapterMode) {
		case Line:
			m_PanLineBtn->setBtnState(AeroButton::ON);
			m_PanFilledLineBtn->setBtnState(AeroButton::OFF);
			m_PanSolidBtn->setBtnState(AeroButton::OFF);
			break;
		case FilledLine:
			m_PanFilledLineBtn->setBtnState(AeroButton::ON);
			m_PanLineBtn->setBtnState(AeroButton::OFF);
			m_PanSolidBtn->setBtnState(AeroButton::OFF);
			break;
		case Solid:
			m_PanSolidBtn->setBtnState(AeroButton::ON);
			m_PanLineBtn->setBtnState(AeroButton::OFF);
			m_PanFilledLineBtn->setBtnState(AeroButton::OFF);
			break;
	}

	int fontMaxWidth = m_fonts.smallFontMetrics->boundingRect(" 200 ").width();

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

	QString str = "%1 ";
	m_avgLevelLabel = new QLabel(str.arg(m_avgValue, 2, 10, QLatin1Char(' ')), this);
	m_avgLevelLabel->setFont(m_fonts.smallFont);
	m_avgLevelLabel->setFixedSize(fontMaxWidth, 12);
	m_avgLevelLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	m_sqlabel = new QLabel("FM Thresh:", this);
	m_sqlabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	m_avgLabel = new QLabel("Avg Filter:", this);
	m_avgLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	m_panAvgModeLabel = new QLabel("Avg Mode:", this);
	m_panAvgModeLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	m_panDetModeLabel = new QLabel("Avg Det:", this);
	m_panDetModeLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	m_fftLabel = new QLabel("FFT Size:", this);
	m_fftLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	m_fftSizeCombo = new QComboBox(this);
	m_fftSizeCombo->addItems(QStringList() << "2k" << "4k" << "8k" << "16k" << "32k");
	m_fftSizeCombo->setFont(m_fonts.normalFont);
	m_fftSizeCombo->setCurrentIndex(m_fftSize);
	CHECKED_CONNECT(m_fftSizeCombo, &QComboBox::currentIndexChanged, this, &DisplayOptionsWidget::fftSizeChanged);

	m_panAverageCombo = new QComboBox(this);
	m_panAverageCombo->addItems(QStringList() << "None" << "Recursive" << "TimeWindow" << "LogRecursive");
	m_panAverageCombo->setFont(m_fonts.normalFont);
	m_panAverageCombo->setCurrentIndex(m_panAvMode);
	CHECKED_CONNECT(m_panAverageCombo, &QComboBox::currentIndexChanged, this, &DisplayOptionsWidget::panAverageModeChanged);

	m_panDetectorCombo = new QComboBox(this);
	m_panDetectorCombo->addItems(QStringList() << "Peak" << "Rosenfall" << "Average" << "Sample");
	m_panDetectorCombo->setFont(m_fonts.normalFont);
	m_panDetectorCombo->setCurrentIndex(m_panDetMode);
	CHECKED_CONNECT(m_panDetectorCombo, &QComboBox::currentIndexChanged, this, &DisplayOptionsWidget::panDetectorModeChanged);

	QHBoxLayout *hbox1 = new QHBoxLayout;
	hbox1->setSpacing(4);
	hbox1->addStretch();
	hbox1->addWidget(m_PanLineBtn);
	hbox1->addWidget(m_PanFilledLineBtn);
	hbox1->addWidget(m_PanSolidBtn);

	QHBoxLayout *hbox2 = new QHBoxLayout;
	hbox2->setSpacing(0);
	hbox2->setContentsMargins(0, 0, 0, 0);
	hbox2->addWidget(m_avgLabel);
	hbox2->addStretch();
	hbox2->addWidget(m_avgSlider);
	hbox2->addWidget(m_avgLevelLabel);

	QHBoxLayout *hbox3 = new QHBoxLayout;
	hbox3->setSpacing(0);
	hbox3->setContentsMargins(0, 0, 0, 0);
	hbox3->addWidget(m_panAvgModeLabel);
	hbox3->addWidget(m_panAverageCombo);

	QHBoxLayout *hbox4 = new QHBoxLayout;
	hbox4->setSpacing(0);
	hbox4->setContentsMargins(0, 0, 0, 0);
	hbox4->addWidget(m_panDetModeLabel);
	hbox4->addWidget(m_panDetectorCombo);

	QHBoxLayout *hbox5 = new QHBoxLayout;
	hbox5->setSpacing(0);
	hbox5->setContentsMargins(0, 0, 0, 0);
	hbox5->addWidget(m_fftLabel);
	hbox5->addWidget(m_fftSizeCombo);

	QHBoxLayout *hbox6 = new QHBoxLayout;
	hbox6->setSpacing(0);
	hbox6->setContentsMargins(0, 0, 0, 0);
	hbox6->addWidget(m_sqlabel);
	hbox6->addWidget(m_fmSqlevel);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setSpacing(6);
	vbox->addSpacing(6);
	vbox->addLayout(hbox1);
	vbox->addLayout(hbox2);
	vbox->addLayout(hbox3);
	vbox->addLayout(hbox4);
	vbox->addLayout(hbox5);
	vbox->addLayout(hbox6);

	m_panSpectrumOptions = new QGroupBox(tr("Panadapter Spectrum"), this);
	m_panSpectrumOptions->setMinimumWidth(m_minimumGroupBoxWidth);
	m_panSpectrumOptions->setLayout(vbox);
	m_panSpectrumOptions->setFont(QFont("Arial", 12));
}

void DisplayOptionsWidget::createWidebandPanOptions() {
	m_wbPanLineBtn = new AeroButton("Line", this);
	m_wbPanLineBtn->setRoundness(0);
	m_wbPanLineBtn->setFixedSize(btn_width, btn_height);
	m_wbpanadapterBtnList.append(m_wbPanLineBtn);
	CHECKED_CONNECT(m_wbPanLineBtn, &AeroButton::clicked, this, &DisplayOptionsWidget::wbPanModeChanged);

	m_wbPanFilledLineBtn = new AeroButton("Filled Line", this);
	m_wbPanFilledLineBtn->setRoundness(0);
	m_wbPanFilledLineBtn->setFixedSize(btn_width, btn_height);
	m_wbpanadapterBtnList.append(m_wbPanFilledLineBtn);
	CHECKED_CONNECT(m_wbPanFilledLineBtn, &AeroButton::clicked, this, &DisplayOptionsWidget::wbPanModeChanged);

	m_wbPanSolidBtn = new AeroButton("Solid", this);
	m_wbPanSolidBtn->setRoundness(0);
	m_wbPanSolidBtn->setFixedSize(btn_width, btn_height);
	m_wbpanadapterBtnList.append(m_wbPanSolidBtn);
	CHECKED_CONNECT(m_wbPanSolidBtn, &AeroButton::clicked, this, &DisplayOptionsWidget::wbPanModeChanged);

	switch (m_wbPanadapterMode) {
		case Line:
			m_wbPanLineBtn->setBtnState(AeroButton::ON);
			m_wbPanFilledLineBtn->setBtnState(AeroButton::OFF);
			m_wbPanSolidBtn->setBtnState(AeroButton::OFF);
			break;
		case FilledLine:
			m_wbPanFilledLineBtn->setBtnState(AeroButton::ON);
			m_wbPanLineBtn->setBtnState(AeroButton::OFF);
			m_wbPanSolidBtn->setBtnState(AeroButton::OFF);
			break;
		case Solid:
			m_wbPanSolidBtn->setBtnState(AeroButton::ON);
			m_wbPanLineBtn->setBtnState(AeroButton::OFF);
			m_wbPanFilledLineBtn->setBtnState(AeroButton::OFF);
			break;
	}

	int fontMaxWidth = m_fonts.smallFontMetrics->boundingRect(" 200 ").width();

	m_wbAvgValue = 100;

	m_wbAvgSlider = new QSlider(Qt::Horizontal, this);
	m_wbAvgSlider->setTickPosition(QSlider::NoTicks);
	m_wbAvgSlider->setFixedSize(130, 12);
	m_wbAvgSlider->setSingleStep(1);
	m_wbAvgSlider->setRange(1, 1000);
	m_wbAvgSlider->setValue(m_wbAvgValue);
	CHECKED_CONNECT(m_wbAvgSlider, &QSlider::valueChanged, this, &DisplayOptionsWidget::setWidebandAveragingCnt);

	QString str = "%1 ";
	m_wbAvgLevelLabel = new QLabel(str.arg(m_wbAvgValue, 2, 10, QLatin1Char(' ')), this);
	m_wbAvgLevelLabel->setFont(m_fonts.smallFont);
	m_wbAvgLevelLabel->setFixedSize(fontMaxWidth, 12);
	m_wbAvgLevelLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	m_wbAvgLabel = new QLabel("Avg Filter:", this);
	m_wbAvgLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	QHBoxLayout *hbox1 = new QHBoxLayout;
	hbox1->setSpacing(4);
	hbox1->addStretch();
	hbox1->addWidget(m_wbPanLineBtn);
	hbox1->addWidget(m_wbPanFilledLineBtn);
	hbox1->addWidget(m_wbPanSolidBtn);

	QHBoxLayout *hbox2 = new QHBoxLayout;
	hbox2->setSpacing(0);
	hbox2->setContentsMargins(0, 0, 0, 0);
	hbox2->addWidget(m_wbAvgLabel);
	hbox2->addStretch();
	hbox2->addWidget(m_wbAvgSlider);
	hbox2->addWidget(m_wbAvgLevelLabel);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setSpacing(6);
	vbox->addSpacing(6);
	vbox->addLayout(hbox1);
	vbox->addLayout(hbox2);

	m_widebandPanOptions = new QGroupBox(tr("Wideband Panadapter Spectrum"), this);
	m_widebandPanOptions->setMinimumWidth(m_minimumGroupBoxWidth);
	m_widebandPanOptions->setLayout(vbox);
	m_widebandPanOptions->setFont(QFont("Arial", 12));
}

void DisplayOptionsWidget::createWaterfallSpectrumOptions() {
	m_waterfallSimpleBtn = new AeroButton("Simple", this);
	m_waterfallSimpleBtn->setRoundness(0);
	m_waterfallSimpleBtn->setFixedSize(btn_width, btn_height);
	m_waterfallColorBtnList.append(m_waterfallSimpleBtn);
	CHECKED_CONNECT(m_waterfallSimpleBtn, &AeroButton::clicked, this, &DisplayOptionsWidget::waterfallColorChanged);

	m_waterfallEnhancedBtn = new AeroButton("Enhanced", this);
	m_waterfallEnhancedBtn->setRoundness(0);
	m_waterfallEnhancedBtn->setFixedSize(btn_width, btn_height);
	m_waterfallColorBtnList.append(m_waterfallEnhancedBtn);
	CHECKED_CONNECT(m_waterfallEnhancedBtn, &AeroButton::clicked, this, &DisplayOptionsWidget::waterfallColorChanged);

	switch (m_waterColorMode) {
		case Simple:
			m_waterfallSimpleBtn->setBtnState(AeroButton::ON);
			m_waterfallEnhancedBtn->setBtnState(AeroButton::OFF);
			break;
		case Enhanced:
			m_waterfallEnhancedBtn->setBtnState(AeroButton::ON);
			m_waterfallSimpleBtn->setBtnState(AeroButton::OFF);
			break;
	}

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

	m_waterfallTimeSpinBox = nullptr;

	QHBoxLayout *hbox1 = new QHBoxLayout;
	hbox1->setSpacing(4);
	hbox1->addStretch();
	hbox1->addWidget(m_waterfallSimpleBtn);
	hbox1->addWidget(m_waterfallEnhancedBtn);

	QHBoxLayout *hbox3 = new QHBoxLayout;
	hbox3->setSpacing(4);
	hbox3->addWidget(m_waterfallLoOffsetLabel);
	hbox3->addStretch();
	hbox3->addWidget(m_waterfallLoOffsetSpinBox);

	QHBoxLayout *hbox4 = new QHBoxLayout;
	hbox4->setSpacing(4);
	hbox4->addWidget(m_waterfallHiOffsetLabel);
	hbox4->addStretch();
	hbox4->addWidget(m_waterfallHiOffsetSpinBox);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setSpacing(6);
	vbox->addSpacing(6);
	vbox->addLayout(hbox1);
	vbox->addLayout(hbox3);
	vbox->addLayout(hbox4);

	m_waterfallSpectrumOptions = new QGroupBox(tr("Waterfall Spectrum"), this);
	m_waterfallSpectrumOptions->setMinimumWidth(m_minimumGroupBoxWidth);
	m_waterfallSpectrumOptions->setLayout(vbox);
	m_waterfallSpectrumOptions->setFont(QFont("Arial", 12));
}

void DisplayOptionsWidget::createSMeterOptions() {
	m_sMeterHoldTimeSpinBox = new QSpinBox(this);
	m_sMeterHoldTimeSpinBox->setMinimum(500);
	m_sMeterHoldTimeSpinBox->setMaximum(10000);
	m_sMeterHoldTimeSpinBox->setSingleStep(500);
	m_sMeterHoldTimeSpinBox->setValue(m_sMeterHoldTime);
	CHECKED_CONNECT(m_sMeterHoldTimeSpinBox, &QSpinBox::valueChanged, this, &DisplayOptionsWidget::sMeterHoldTimeChanged);

	m_sMeterHoldTimeLabel = new QLabel("Hold Time (ms):", this);
	m_sMeterHoldTimeLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	QHBoxLayout *hbox2 = new QHBoxLayout;
	hbox2->setSpacing(4);
	hbox2->addWidget(m_sMeterHoldTimeLabel);
	hbox2->addStretch();
	hbox2->addWidget(m_sMeterHoldTimeSpinBox);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setSpacing(6);
	vbox->addSpacing(6);
	vbox->addLayout(hbox2);

	m_sMeterOptions = new QGroupBox(tr("S-Meter"), this);
	m_sMeterOptions->setMinimumWidth(m_minimumGroupBoxWidth);
	m_sMeterOptions->setLayout(vbox);
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
	hbox1->addStretch();
	hbox1->addWidget(m_setCallSignBtn);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setSpacing(6);
	vbox->addLayout(hbox1);

	m_callSignEditor = new QGroupBox(tr("Call Sign Editor"), this);
	m_callSignEditor->setMinimumWidth(m_minimumGroupBoxWidth);
	m_callSignEditor->setLayout(vbox);
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
	if (m_waterfallTimeSpinBox) {
		const QSignalBlocker blocker(m_waterfallTimeSpinBox);
		m_waterfallTimeSpinBox->setValue(val);
	}
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
	// Settings/WDSP use a combo index (0=2k … 4=32k), not the FFT bin count.
	m_fftSize = size;
	const QSignalBlocker blocker(m_fftSizeCombo);
	if (size >= 0 && size < m_fftSizeCombo->count()) {
		m_fftSizeCombo->setCurrentIndex(size);
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
	if (btnHit < 0 || !button) return;

	foreach (AeroButton *btn, m_panadapterBtnList) {
		btn->setBtnState(AeroButton::OFF);
		btn->update();
	}
	button->setBtnState(AeroButton::ON);
	button->update();

	m_panadapterMode = static_cast<PanGraphicsMode>(btnHit);
	emit graphicsStateRequested(m_currentReceiver, m_panadapterMode, m_waterColorMode);
}

void DisplayOptionsWidget::wbPanModeChanged() {
	AeroButton *button = qobject_cast<AeroButton *>(sender());
	int btnHit = m_wbpanadapterBtnList.indexOf(button);
	if (btnHit < 0 || !button) return;

	foreach (AeroButton *btn, m_wbpanadapterBtnList) {
		btn->setBtnState(AeroButton::OFF);
		btn->update();
	}
	button->setBtnState(AeroButton::ON);
	button->update();

	m_wbPanadapterMode = static_cast<PanGraphicsMode>(btnHit);
	emit graphicsStateRequested(-1, m_wbPanadapterMode, m_waterColorMode);
}

void DisplayOptionsWidget::waterfallColorChanged() {
	AeroButton *button = qobject_cast<AeroButton *>(sender());
	int btnHit = m_waterfallColorBtnList.indexOf(button);
	if (btnHit < 0 || !button) return;

	foreach (AeroButton *btn, m_waterfallColorBtnList) {
		btn->setBtnState(AeroButton::OFF);
		btn->update();
	}
	button->setBtnState(AeroButton::ON);
	button->update();

	m_waterColorMode = static_cast<WaterfallColorMode>(btnHit);
	emit graphicsStateRequested(m_currentReceiver, m_panadapterMode, m_waterColorMode);
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
	m_fftSize = index;
	emit fftSizeRequested(m_currentReceiver, index);
}

void DisplayOptionsWidget::sqLevelChanged(int val) {
	emit fmsqLevelRequested(m_currentReceiver, val);
}
