#define LOG_ANTENNA_WIDGET

#include <QLabel>
#include <QGroupBox>
#include <QBoxLayout>

#include "cusdr_alexAntennaWidget.h"

#define	btn_height		22
#define	btn_width		20
#define	btn_width2		28
#define	btn_width3		32

AlexAntennaWidget::AlexAntennaWidget(QWidget *parent)
	: QWidget(parent)
	, m_serverMode(QSDR::SDRMode)
	, m_hwInterface(QSDR::NoInterfaceMode)
	, m_dataEngineState(QSDR::DataEngineDown)
	, m_alexConfig(0)
	, m_numberOfBands(11)
	, m_minimumWidgetWidth(500)
	, m_minimumGroupBoxWidth(0)
{
	setMinimumWidth(m_minimumWidgetWidth);
	setContentsMargins(4, 8, 4, 0);
	setMouseTracking(true);

	// Pre-initialize m_alexStates
	for (int i = 0; i < m_numberOfBands; ++i) {
		m_alexStates.append(0);
	}

	createAntennasGroup();

	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(0,0,0,0);
	mainLayout->addSpacing(8);

	QHBoxLayout *hbox1 = new QHBoxLayout;
	hbox1->setSpacing(0);
	hbox1->setContentsMargins(4, 0, 4, 0);
	hbox1->addWidget(antennaGroup);

	mainLayout->addLayout(hbox1);
	mainLayout->addStretch();
		
	setLayout(mainLayout);
}

AlexAntennaWidget::~AlexAntennaWidget() {
	disconnect(0, 0, 0);
}

void AlexAntennaWidget::createAntennasGroup() {
	QLabel *emptyLabel1 = new QLabel("  ", this);
    emptyLabel1->setFrameStyle(QFrame::Box | QFrame::Raised);

	QLabel *emptyLabel2 = new QLabel(" ", this);
    emptyLabel2->setFrameStyle(QFrame::Box | QFrame::Raised);

	QLabel *emptyLabel3 = new QLabel(" ", this);
	emptyLabel3->setFrameStyle(QFrame::Box | QFrame::Raised);

	// band names
	QStringList bandNames;
    bandNames << "160 m" << "80 m" << "60 m" << "40 m" << "30 m" << "20 m" << "17 m" << "15 m" << "12 m" << "10 m" << "6 m";

	QList<QLabel *> bandLabelList;

	for (int i = 0; i < m_numberOfBands; i++) {
		QLabel *label = new QLabel(bandNames.at(i), this);
		label->setFrameStyle(QFrame::Box | QFrame::Raised);
		bandLabelList << label;
	}

	// rx antenna names
	QList<QLabel *> rxAntLabelList;
	for (int i = 0; i < 3; i++) {
		QLabel *label = new QLabel(QString("ANT%1").arg(i+1), this);
		label->setFrameStyle(QFrame::Box | QFrame::Raised);
		rxAntLabelList << label;
	}

	// rx aux names
	QList<QLabel *> rxAuxLabelList;
	rxAuxLabelList << new QLabel("RX1", this) << new QLabel("RX2", this) << new QLabel("XV", this);
	for (int i = 0; i < 3; i++)
		rxAuxLabelList.at(i)->setFrameStyle(QFrame::Box | QFrame::Raised);

	// tx antenna names
	QList<QLabel *> txAntLabelList;
	for (int i = 0; i < 3; i++) {
		QLabel *label = new QLabel(QString("ANT%1").arg(i+1), this);
		label->setFrameStyle(QFrame::Box | QFrame::Raised);
		txAntLabelList << label;
	}

	QGridLayout *gridLayout = new QGridLayout;
	gridLayout->setSpacing(1);

	// header row 0
	gridLayout->addWidget(emptyLabel1, 0, 0);

	QLabel *rxLabel = new QLabel("Rx ANT Selection", this);
	rxLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
	gridLayout->addWidget(rxLabel, 0, 1, 1, 3, Qt::AlignCenter);

	gridLayout->addWidget(emptyLabel2, 0, 4);

	QLabel *rxAuxLabel = new QLabel("Rx AUX Selection", this);
	rxAuxLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
	gridLayout->addWidget(rxAuxLabel, 0, 5, 1, 3, Qt::AlignCenter);

	gridLayout->addWidget(emptyLabel3, 0, 8);

	QLabel *txLabel = new QLabel("Tx ANT Selection", this);
	txLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
	gridLayout->addWidget(txLabel, 0, 9, 1, 3, Qt::AlignCenter);

	// header row 1
	for (int i = 0; i < rxAntLabelList.count(); i++)
		gridLayout->addWidget(rxAntLabelList.at(i), 1, i+1, Qt::AlignCenter);

	for (int i = 0; i < rxAuxLabelList.count(); i++)
		gridLayout->addWidget(rxAuxLabelList.at(i), 1, i+5, Qt::AlignCenter);

	for (int i = 0; i < txAntLabelList.count(); i++)
		gridLayout->addWidget(txAntLabelList.at(i), 1, i+9, Qt::AlignCenter);

	// band labels row 2 - 12
	for (int i = 0; i < bandLabelList.count(); i++)
		gridLayout->addWidget(bandLabelList.at(i), i+2, 0);

	// separators
	for (int i = 0; i < bandLabelList.count(); i++) {
		QLabel *sep1 = new QLabel(" ", this);
		sep1->setFrameStyle(QFrame::Box | QFrame::Raised);
		sep1->setFixedWidth(10);
		gridLayout->addWidget(sep1, i+2, 4);

		QLabel *sep2 = new QLabel(" ", this);
		sep2->setFrameStyle(QFrame::Box | QFrame::Raised);
		sep2->setFixedWidth(10);
		gridLayout->addWidget(sep2, i+2, 8);
	}

	for (int i = 0; i < bandLabelList.count(); i++) { // bands
		QList<AeroButton *> rxAntList;
		for (int j = 0; j < 3; j++) { // pins
			AeroButton *btn = new AeroButton("", this);
			btn->setRoundness(0);
			btn->setFixedSize (btn_width, btn_height);
			btn->setBtnState(AeroButton::OFF);
			gridLayout->addWidget(btn, i+2, j+1);
			rxAntList << btn;

			CHECKED_CONNECT(
				btn, 
				&AeroButton::clicked, 
				this, 
				&AlexAntennaWidget::antBtnClicked);
		}
		bandBtnMatrix << rxAntList;

		QList<AeroButton *> rxAuxList;
		for (int j = 0; j < 3; j++) { // pins
			AeroButton *btn = new AeroButton("", this);
			btn->setRoundness(0);
			btn->setFixedSize (btn_width, btn_height);
			btn->setBtnState(AeroButton::OFF);
			gridLayout->addWidget(btn, i+2, j+5);
			rxAuxList << btn;

			CHECKED_CONNECT(
				btn, 
				&AeroButton::clicked, 
				this, 
				&AlexAntennaWidget::rxAuxBtnClicked);
		}
		bandBtnRxMatrix << rxAuxList;
		rx1BtnList << rxAuxList.at(0);
		rx2BtnList << rxAuxList.at(1);
		xvBtnList << rxAuxList.at(2);

		QList<AeroButton *> txAntList;
		for (int j = 0; j < 3; j++) { // pins
			AeroButton *btn = new AeroButton("", this);
			btn->setRoundness(0);
			btn->setFixedSize (btn_width, btn_height);
			btn->setBtnState(AeroButton::OFF);
			gridLayout->addWidget(btn, i+2, j+9);
			txAntList << btn;

			CHECKED_CONNECT(
				btn, 
				&AeroButton::clicked, 
				this, 
				&AlexAntennaWidget::txAntBtnClicked);
		}
		bandBtnTxMatrix << txAntList;
		tx1BtnList << txAntList.at(0);
		tx2BtnList << txAntList.at(1);
		tx3BtnList << txAntList.at(2);
	}

	antennaGroup = new QGroupBox(tr("Alex RF Routing Selection"), this);
	antennaGroup->setMinimumWidth(m_minimumGroupBoxWidth);
	antennaGroup->setLayout(gridLayout);
	antennaGroup->setFont(QFont("Arial", 8));
}

void AlexAntennaWidget::setAlexConfig(quint16 config) {
	m_alexConfig = config;
}

void AlexAntennaWidget::setAlexStates(const QList<int>& states) {
	m_alexStates = states;
	setAlexValues();
}

void AlexAntennaWidget::setAlexValues() {
	for (int i = 0; i < qMin(m_numberOfBands, m_alexStates.size()); i++) {
		// Clear buttons first
		foreach(AeroButton *btn, bandBtnMatrix.at(i)) {
			btn->blockSignals(true);
			btn->setBtnState(AeroButton::OFF);
			btn->blockSignals(false);
			btn->update();
		}
		foreach(AeroButton *btn, bandBtnRxMatrix.at(i)) {
			btn->blockSignals(true);
			btn->setBtnState(AeroButton::OFF);
			btn->blockSignals(false);
			btn->update();
		}
		foreach(AeroButton *btn, bandBtnTxMatrix.at(i)) {
			btn->blockSignals(true);
			btn->setBtnState(AeroButton::OFF);
			btn->blockSignals(false);
			btn->update();
		}

		int rxAnt = m_alexStates.at(i) & 0x03;
		if (rxAnt > 0 && rxAnt <= 3) {
			bandBtnMatrix.at(i).at(rxAnt-1)->blockSignals(true);
			bandBtnMatrix.at(i).at(rxAnt-1)->setBtnState(AeroButton::ON);
			bandBtnMatrix.at(i).at(rxAnt-1)->blockSignals(false);
			bandBtnMatrix.at(i).at(rxAnt-1)->update();
		}

		int rxAux = (m_alexStates.at(i) >> 2) & 0x07;
		if (rxAux > 0 && rxAux <= 3) {
			bandBtnRxMatrix.at(i).at(rxAux-1)->blockSignals(true);
			bandBtnRxMatrix.at(i).at(rxAux-1)->setBtnState(AeroButton::ON);
			bandBtnRxMatrix.at(i).at(rxAux-1)->blockSignals(false);
			bandBtnRxMatrix.at(i).at(rxAux-1)->update();
		}

		int txAnt = (m_alexStates.at(i) >> 5) & 0x03;
		if (txAnt > 0 && txAnt <= 3) {
			bandBtnTxMatrix.at(i).at(txAnt-1)->blockSignals(true);
			bandBtnTxMatrix.at(i).at(txAnt-1)->setBtnState(AeroButton::ON);
			bandBtnTxMatrix.at(i).at(txAnt-1)->blockSignals(false);
			bandBtnTxMatrix.at(i).at(txAnt-1)->update();
		}
	}
}

void AlexAntennaWidget::antBtnClicked() {
	AeroButton *button = qobject_cast<AeroButton *>(sender());
	int btnHit = -1;
	int antenna = -1;
	for (int i = 0; i < bandBtnMatrix.size(); i++) {
		int idx = bandBtnMatrix.at(i).indexOf(button);
		if (idx >= 0) {
			btnHit = i;
			antenna = idx + 1;
			break;
		}
	}

	if (btnHit >= 0) {
		int nextState = m_alexStates.at(btnHit);
		nextState &= 0x1FC;
		nextState |= antenna;
		emit alexStateRequested(btnHit, nextState);
	}
}

void AlexAntennaWidget::rxAuxBtnClicked() {
	AeroButton *button = qobject_cast<AeroButton *>(sender());
	int btnHit = -1;
	int aux = -1;
	int btnHit1 = rx1BtnList.indexOf(button);
	int btnHit2 = rx2BtnList.indexOf(button);
	int btnHit3 = xvBtnList.indexOf(button);

	if (btnHit1 >= 0) {
		btnHit = btnHit1;
		aux = 1;
	} else if (btnHit2 >= 0) {
		btnHit = btnHit2;
		aux = 2;
	} else if (btnHit3 >= 0) {
		btnHit = btnHit3;
		aux = 3;
	} else {
		return;
	}

	if (btnHit >= 0) {
		int nextState = m_alexStates.at(btnHit);
		nextState &= 0x1E3;
		if (button->btnState() == AeroButton::OFF) {
			nextState |= aux << 2;
		}
		emit alexStateRequested(btnHit, nextState);
	}
}

void AlexAntennaWidget::txAntBtnClicked() {
	AeroButton *button = qobject_cast<AeroButton *>(sender());
	int btnHit = -1;
	int antenna = -1;
	int btnHit1 = tx1BtnList.indexOf(button);
	int btnHit2 = tx2BtnList.indexOf(button);
	int btnHit3 = tx3BtnList.indexOf(button);

	if (btnHit1 >= 0) {
		btnHit = btnHit1;
		antenna = 1;
	} else if (btnHit2 >= 0) {
		btnHit = btnHit2;
		antenna = 2;
	} else if (btnHit3 >= 0) {
		btnHit = btnHit3;
		antenna = 3;
	} else {
		return;
	}

	if (btnHit >= 0) {
		int nextState = m_alexStates.at(btnHit);
		nextState &= 0x19F;
		nextState |= antenna << 5;
		emit alexStateRequested(btnHit, nextState);
	}
}
