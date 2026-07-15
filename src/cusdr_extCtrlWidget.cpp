#define LOG_PENNY_WIDGET

#include <QLabel>
#include <QGroupBox>
#include <QBoxLayout>

#include "cusdr_extCtrlWidget.h"

#define	btn_height		22
#define	btn_width		20
#define	btn_width2		28
#define	btn_width3		32

ExtCtrlWidget::ExtCtrlWidget(QWidget *parent)
	: QWidget(parent)
	, m_pennyOCEnabled(false)
	, m_minimumWidgetWidth(500)
	, m_minimumGroupBoxWidth(0)
{
	setMinimumWidth(m_minimumWidgetWidth);
	setContentsMargins(4, 8, 4, 0);
	setMouseTracking(true);

	// Pre-initialize lists
	for (int i = 0; i < MAX_BANDS - 1; ++i) {
		m_rxPins.append(0);
		m_txPins.append(0);
	}

	createReceivePinsGroup();
	createTransmitPinsGroup();

	enableBtn = new AeroButton("Enable", this);
	enableBtn->setRoundness(0);
	enableBtn->setFixedSize (65, btn_height);
	enableBtn->setBtnState(AeroButton::OFF);
	
	CHECKED_CONNECT(
		enableBtn, 
		&AeroButton::clicked, 
		this, 
		&ExtCtrlWidget::enable);

	// disable all buttons initially
	foreach(QList<AeroButton *> btnList, receivePinsBtnMatrix) {
		foreach(AeroButton *btn, btnList) {
			btn->setEnabled(false);
			btn->update();
		}
	}

	foreach(QList<AeroButton *> btnList, transmitPinsBtnMatrix) {
		foreach(AeroButton *btn, btnList) {
			btn->setEnabled(false);
			btn->update();
		}
	}

	// set main layout
	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(0,0,0,0);
	mainLayout->addSpacing(8);

	QHBoxLayout *hbox1 = new QHBoxLayout;
	hbox1->setSpacing(0);
	hbox1->setContentsMargins(4, 0, 4, 0);
	hbox1->addWidget(enableBtn);
	hbox1->addStretch();

	QHBoxLayout *hbox2 = new QHBoxLayout;
	hbox2->setSpacing(0);
	hbox2->setContentsMargins(4, 0, 4, 0);
	hbox2->addWidget(receivePinsGroup);

	QHBoxLayout *hbox3 = new QHBoxLayout;
	hbox3->setSpacing(0);
	hbox3->setContentsMargins(4, 0, 4, 0);
	hbox3->addWidget(transmitPinsGroup);

	mainLayout->addLayout(hbox1);
	mainLayout->addLayout(hbox2);
	mainLayout->addLayout(hbox3);
	mainLayout->addStretch();
		
	setLayout(mainLayout);
}

ExtCtrlWidget::~ExtCtrlWidget() {
	disconnect(0, 0, 0);
}

void ExtCtrlWidget::createReceivePinsGroup() {
	QLabel* emptyLabel = new QLabel(" ", this);
    emptyLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	QStringList bandNames;
    bandNames << "2200m" << "630m" << "160 m" << "80 m" << "60 m" << "40 m" << "30 m" << "20 m" << "17 m" << "15 m" << "12 m" << "10 m" << "6 m" << "2 m" << "125 cm" << "70 cm" << "33 cm" << "23 cm" << "13 cm" << "10 cm" << "5 cm" << "gen";

	QList<QLabel *> bandLabelList;

	for (int i = 0; i < MAX_BANDS-1; i++) {
		QLabel *label = new QLabel(bandNames.at(i), this);
		label->setFrameStyle(QFrame::Box | QFrame::Raised);
		bandLabelList << label;
	}

	QList<QLabel *> pinLabelList;

	for (int i = 0; i < 7; i++) {
		QLabel *label = new QLabel(QString("Pin %1").arg(i+1), this);
		label->setFrameStyle(QFrame::Box | QFrame::Raised);
		pinLabelList << label;
	}

	QGridLayout *gridLayout = new QGridLayout;
	gridLayout->setSpacing(1);

	gridLayout->addWidget(emptyLabel, 0, 0);

	for (int i = 0; i < pinLabelList.count(); i++)
		gridLayout->addWidget(pinLabelList.at(i), 0, i+1, Qt::AlignCenter);

	for (int i = 0; i < bandLabelList.count(); i++)
		gridLayout->addWidget(bandLabelList.at(i), i+1, 0);

	for (int i = 0; i < bandLabelList.count(); i++) { // bands
		QList<AeroButton *> btnList;
		for (int j = 0; j < pinLabelList.count(); j++) { // pins
			AeroButton *btn = new AeroButton("", this);
			btn->setRoundness(0);
			btn->setFixedSize (btn_width, btn_height);
			btn->setBtnState(AeroButton::OFF);
			gridLayout->addWidget(btn, i+1, j+1);
			btnList << btn;

			CHECKED_CONNECT(
				btn, 
				&AeroButton::clicked, 
				this, 
				&ExtCtrlWidget::receivePinsBtnClicked);
		}
		receivePinsBtnMatrix << btnList;
	}

	receivePinsGroup = new QGroupBox(tr("PennyLane Open Collector Outputs - Receive State"), this);
	receivePinsGroup->setMinimumWidth(m_minimumGroupBoxWidth);
	receivePinsGroup->setLayout(gridLayout);
	receivePinsGroup->setFont(QFont("Arial", 8));
}

void ExtCtrlWidget::createTransmitPinsGroup() {
	QLabel* emptyLabel = new QLabel(" ", this);
    emptyLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	QStringList bandNames;
    bandNames << "2200m" << "630m" << "160 m" << "80 m" << "60 m" << "40 m" << "30 m" << "20 m" << "17 m" << "15 m" << "12 m" << "10 m" << "6 m" << "2 m" << "125 cm" << "70 cm" << "33 cm" << "23 cm" << "13 cm" << "10 cm" << "5 cm" << "gen";

	QList<QLabel *> bandLabelList;

	for (int i = 0; i < MAX_BANDS-1; i++) {
		QLabel *label = new QLabel(bandNames.at(i), this);
		label->setFrameStyle(QFrame::Box | QFrame::Raised);
		bandLabelList << label;
	}

	QList<QLabel *> pinLabelList;

	for (int i = 0; i < 7; i++) {
		QLabel *label = new QLabel(QString("Pin %1").arg(i+1), this);
		label->setFrameStyle(QFrame::Box | QFrame::Raised);
		pinLabelList << label;
	}

	QGridLayout *gridLayout = new QGridLayout;
	gridLayout->setSpacing(1);

	gridLayout->addWidget(emptyLabel, 0, 0);

	for (int i = 0; i < pinLabelList.count(); i++)
		gridLayout->addWidget(pinLabelList.at(i), 0, i+1, Qt::AlignCenter);

	for (int i = 0; i < bandLabelList.count(); i++)
		gridLayout->addWidget(bandLabelList.at(i), i+1, 0);

	for (int i = 0; i < bandLabelList.count(); i++) { // bands
		QList<AeroButton *> btnList;
		for (int j = 0; j < pinLabelList.count(); j++) { // pins
			AeroButton *btn = new AeroButton("", this);
			btn->setRoundness(0);
			btn->setFixedSize (btn_width, btn_height);
			btn->setBtnState(AeroButton::OFF);
			gridLayout->addWidget(btn, i+1, j+1);
			btnList << btn;

			CHECKED_CONNECT(
				btn, 
				&AeroButton::clicked, 
				this, 
				&ExtCtrlWidget::transmitPinsBtnClicked);
		}
		transmitPinsBtnMatrix << btnList;
	}

	transmitPinsGroup = new QGroupBox(tr("PennyLane Open Collector Outputs - Transmit State"), this);
	transmitPinsGroup->setMinimumWidth(m_minimumGroupBoxWidth);
	transmitPinsGroup->setLayout(gridLayout);
	transmitPinsGroup->setFont(QFont("Arial", 8));
}

void ExtCtrlWidget::setPennyOCEnabled(bool enabled) {
	m_pennyOCEnabled = enabled;
	enableBtn->blockSignals(true);
	enableBtn->setBtnState(enabled ? AeroButton::ON : AeroButton::OFF);
	enableBtn->blockSignals(false);
	enableBtn->update();

	foreach(QList<AeroButton *> btnList, receivePinsBtnMatrix) {
		foreach(AeroButton *btn, btnList) {
			btn->setEnabled(enabled);
			btn->update();
		}
	}

	foreach(QList<AeroButton *> btnList, transmitPinsBtnMatrix) {
		foreach(AeroButton *btn, btnList) {
			btn->setEnabled(enabled);
			btn->update();
		}
	}
}

void ExtCtrlWidget::setRxPins(const QList<int>& pins) {
	m_rxPins = pins;
	setValues();
}

void ExtCtrlWidget::setTxPins(const QList<int>& pins) {
	m_txPins = pins;
	setValues();
}

void ExtCtrlWidget::setValues() {
	for (int i = 0; i < qMin(m_rxPins.size(), receivePinsBtnMatrix.size()); i++) { // bands
		for (int j = 0; j < 7; j++) { // pins
			const bool rxOn = (0x1 & (m_rxPins.at(i) >> (j+1)));
			receivePinsBtnMatrix.at(i).at(j)->blockSignals(true);
			receivePinsBtnMatrix.at(i).at(j)->setBtnState(rxOn ? AeroButton::ON : AeroButton::OFF);
			receivePinsBtnMatrix.at(i).at(j)->blockSignals(false);
			receivePinsBtnMatrix.at(i).at(j)->update();
		}
	}
	for (int i = 0; i < qMin(m_txPins.size(), transmitPinsBtnMatrix.size()); i++) { // bands
		for (int j = 0; j < 7; j++) { // pins
			const bool txOn = (0x1 & (m_txPins.at(i) >> (j+1)));
			transmitPinsBtnMatrix.at(i).at(j)->blockSignals(true);
			transmitPinsBtnMatrix.at(i).at(j)->setBtnState(txOn ? AeroButton::ON : AeroButton::OFF);
			transmitPinsBtnMatrix.at(i).at(j)->blockSignals(false);
			transmitPinsBtnMatrix.at(i).at(j)->update();
		}
	}
}

uchar ExtCtrlWidget::getMask(int value) {
	uchar mask = 0x0;
	switch (value) {
		case 0:  mask = 0xFD; break;
		case 1:  mask = 0xFB; break;
		case 2:  mask = 0xF7; break;
		case 3:  mask = 0xEF; break;
		case 4:  mask = 0xDF; break;
		case 5:  mask = 0xBF; break;
		case 6:  mask = 0x7F; break;
		default: mask = 0x0;  break;
	}
	return mask;
}

void ExtCtrlWidget::enable() {
	bool target = (enableBtn->btnState() == AeroButton::OFF);
	emit pennyOCEnabledRequested(target);
}

void ExtCtrlWidget::receivePinsBtnClicked() {
	AeroButton *button = qobject_cast<AeroButton *>(sender());
	int band = -1;
	int pin = -1;
	for (int i = 0; i < m_rxPins.size(); i++) {
		pin = receivePinsBtnMatrix.at(i).indexOf(button);
		if (pin >= 0) {
			band = i;
			break;
		}
	}

	if (band >= 0 && pin >= 0) {
		QList<int> targetPins = m_rxPins;
		if (button->btnState() == AeroButton::ON) {
			targetPins[band] &= getMask(pin);
		} else {
			targetPins[band] &= getMask(pin);
			targetPins[band] |= (1 << (pin+1));
		}
		emit rxPinsRequested(targetPins);
	}
}

void ExtCtrlWidget::transmitPinsBtnClicked() {
	AeroButton *button = qobject_cast<AeroButton *>(sender());
	int band = -1;
	int pin = -1;
	for (int i = 0; i < m_txPins.size(); i++) {
		pin = transmitPinsBtnMatrix.at(i).indexOf(button);
		if (pin >= 0) {
			band = i;
			break;
		}
	}

	if (band >= 0 && pin >= 0) {
		QList<int> targetPins = m_txPins;
		if (button->btnState() == AeroButton::ON) {
			targetPins[band] &= getMask(pin);
		} else {
			targetPins[band] &= getMask(pin);
			targetPins[band] |= (1 << (pin+1));
		}
		emit txPinsRequested(targetPins);
	}
}
