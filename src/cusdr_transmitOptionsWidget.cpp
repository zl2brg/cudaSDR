/**
* @file  cusdr_transmitOptionsWidget.h
* @brief transmit control widget class for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2012-06-16
*/

/*
 *   
 *   Copyright 2012 Hermann von Hasseln, DL3HVH
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
 
//#include <QtGui>
//#include <QMenu>
//#include <QFileDialog>
//#include <QDebug>
#include <QBoxLayout>
#include <QLabel>
#include <QSignalBlocker>

#include "cusdr_transmitOptionsWidget.h"
#include "Util/AudioDeviceService.h"

namespace {
int findDeviceComboIndex(const QList<QAudioDevice> &devices, const QString &name, int offset)
{
    for (int i = 0; i < devices.size(); ++i) {
        if (devices.at(i).description() == name)
            return i + offset;
    }
    return -1;
}
}


TransmitOptionsWidget::TransmitOptionsWidget(QWidget *parent)
	: QWidget(parent)
	, set(Settings::instance())
	, m_minimumWidgetWidth(set->getMinimumWidgetWidth())
	//, m_minimumGroupBoxWidth(set->getMinimumGroupBoxWidth())
	, m_minimumGroupBoxWidth(0)
{
	setMinimumWidth(m_minimumWidgetWidth);
	setContentsMargins(4, 8, 4, 0);
	setMouseTracking(true);
	
	// create groups
	createSourceGroup();
    createAMSettingsGroup();
	createTransmitFilterGroup();
	createPTTOptionsGroup();


	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(0,0,0,0);
	mainLayout->addSpacing(8);
    mainLayout->setSizeConstraint(QLayout::SetMaximumSize);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(0);
	hbox1->setContentsMargins(4, 0, 4, 0);
	hbox1->addWidget(sourceGroup);

    QHBoxLayout *hbox5 = new QHBoxLayout();
    hbox5->setSpacing(0);
    hbox5->setContentsMargins(4, 0, 4, 0);
    hbox5->addWidget(amTxSettingsGroup);

	QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->setSpacing(0);
	hbox2->setContentsMargins(4, 0, 4, 0);
	hbox2->addWidget(transmitFilterGroup);

	QHBoxLayout *hbox3 = new QHBoxLayout();
	hbox3->setSpacing(0);
	hbox3->setContentsMargins(4, 0, 4, 0);
	hbox3->addWidget(pttOptionsGroup);

	/*QHBoxLayout *hbox4 = new QHBoxLayout();
	hbox4->setSpacing(0);
	hbox4->setContentsMargins(4, 0, 4, 0);
	hbox4->addWidget(searchNetworkDeviceGroupBox);

	if (m_hwInterface == QSDR::NoInterfaceMode) {
		
		deviceNIGroupBox->hide();
		searchNetworkDeviceGroupBox->hide();
	}

	QHBoxLayout *hbox5 = new QHBoxLayout();
	hbox5->setSpacing(0);
	hbox5->setContentsMargins(4, 0, 4, 0);
	hbox5->addWidget(source10MhzExclusiveGroup);

	QHBoxLayout *hbox6 = new QHBoxLayout();
	hbox6->setSpacing(0);
	hbox6->setContentsMargins(4, 0, 4, 0);
	hbox6->addWidget(source122_88MhzExclusiveGroup);

	QHBoxLayout *hbox7 = new QHBoxLayout();
	hbox7->setSpacing(0);
	hbox7->setContentsMargins(4, 0, 4, 0);
	hbox7->addWidget(numberOfReceiversGroup());*/

	mainLayout->addLayout(hbox1);
    mainLayout->addLayout(hbox5);
	mainLayout->addLayout(hbox2);
	mainLayout->addLayout(hbox3);
	/*mainLayout->addLayout(hbox4);
	mainLayout->addLayout(hbox5);
	mainLayout->addLayout(hbox6);
	mainLayout->addLayout(hbox7);*/
	mainLayout->addStretch();
	setLayout(mainLayout);

	setupConnections();
}

TransmitOptionsWidget::~TransmitOptionsWidget() {

	disconnect(set, 0, this, 0);
	disconnect(0, 0, 0);
}

void TransmitOptionsWidget::setupConnections() {
	CHECKED_CONNECT(amCarrierLevelSlider, &QSlider::valueChanged,
	                this, &TransmitOptionsWidget::amCarrierLevelRequested);
	CHECKED_CONNECT(amCompressionSlider, &QSlider::valueChanged,
	                this, &TransmitOptionsWidget::audioCompressionRequested);
	CHECKED_CONNECT(highFilterSpinBox, &QSpinBox::valueChanged,
	                this, &TransmitOptionsWidget::txFilterHighRequested);
	CHECKED_CONNECT(lowFilterSpinBox, &QSpinBox::valueChanged,
	                this, &TransmitOptionsWidget::txFilterLowRequested);

	connect(micInputComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
		if (index < 0) return;
		emit micInputDevChanged(index);
		if (index == 0)
			emit micInputSourceNameChanged(QStringLiteral("hpsdr-local"));
		else
			emit micInputSourceNameChanged(micInputComboBox->itemText(index));
	});

	connect(AudioDeviceService::instance(), &AudioDeviceService::audioInputsChanged, this, [this]() {
		const QString current = micInputComboBox->currentText();
		refreshAudioDevices(current);
		emit audioDevicesRefreshRequested();
	});
}

void TransmitOptionsWidget::setAmCarrierLevel(int percent)
{
	const QSignalBlocker blocker(amCarrierLevelSlider);
	amCarrierLevelSlider->setValue(qBound(1, percent, 100));
}

void TransmitOptionsWidget::setAudioCompression(int level)
{
	const QSignalBlocker blocker(amCompressionSlider);
	amCompressionSlider->setValue(qBound(1, level, 100));
}

void TransmitOptionsWidget::setTxFilterHigh(int hz)
{
	const QSignalBlocker blocker(highFilterSpinBox);
	highFilterSpinBox->setValue(hz);
}

void TransmitOptionsWidget::setTxFilterLow(int hz)
{
	const QSignalBlocker blocker(lowFilterSpinBox);
	lowFilterSpinBox->setValue(hz);
}

void TransmitOptionsWidget::setMicInputDev(int index)
{
	if (index >= 0 && index < micInputComboBox->count()) {
		const QSignalBlocker blocker(micInputComboBox);
		micInputComboBox->setCurrentIndex(index);
	}
}

void TransmitOptionsWidget::setMicInputSourceName(const QString& name)
{
	int idx = -1;
	if (name == QLatin1String("hpsdr-local")) {
		idx = 0;
	} else if (!name.isEmpty()) {
		idx = micInputComboBox->findText(name);
	}
	if (idx >= 0 && idx < micInputComboBox->count()) {
		const QSignalBlocker blocker(micInputComboBox);
		micInputComboBox->setCurrentIndex(idx);
	}
}

void TransmitOptionsWidget::refreshAudioDevices(const QString& savedMicName)
{
	const QSignalBlocker micBlocker(micInputComboBox);
	const QString currentMic = micInputComboBox->currentText();

	micInputComboBox->clear();
	micInputComboBox->addItem(QStringLiteral("HPSDR Mic Input"));

	const QList<QAudioDevice> micInputs = AudioDeviceService::instance()->audioInputs();
	for (const QAudioDevice &deviceInfo : micInputs) {
		micInputComboBox->addItem(deviceInfo.description());
	}

	int micIndex = -1;
	if (!currentMic.isEmpty()) {
		micIndex = micInputComboBox->findText(currentMic);
	}

	if (micIndex < 0) {
		if (savedMicName == QLatin1String("hpsdr-local")) {
			micIndex = 0;
		} else {
			micIndex = findDeviceComboIndex(micInputs, savedMicName, 1);
			if (micIndex < 0) {
				const QString defaultName = AudioDeviceService::instance()->defaultInput().description();
				micIndex = findDeviceComboIndex(micInputs, defaultName, 1);
			}
			if (micIndex < 0)
				micIndex = 0;
		}
	}
	micInputComboBox->setCurrentIndex(micIndex);
}

int TransmitOptionsWidget::micInputDev() const
{
	return micInputComboBox->currentIndex();
}

QString TransmitOptionsWidget::micInputSourceName() const
{
	int idx = micInputComboBox->currentIndex();
	if (idx <= 0)
		return QStringLiteral("hpsdr-local");
	return micInputComboBox->itemText(idx);
}

void TransmitOptionsWidget::createAMSettingsGroup(){
    QLabel* amCompressionLabel = new QLabel("Compression (db):", this);
    QLabel* amCarrierLevelLabel = new QLabel("Carrier Level:", this);
    amCompressionLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    amCarrierLevelLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    amCarrierLevelSlider = new QSlider(Qt::Horizontal,this);
    amCarrierLevelSlider->setFixedSize(80, 10);
    amCarrierLevelSlider->setRange(1, 100);
    amCompressionSlider = new QSlider(Qt::Horizontal,this);
    amCompressionSlider->setFixedSize(80, 10);
    amCompressionSlider->setRange(1, 100);

    QGridLayout *grid = new QGridLayout();
    grid->addWidget(amCompressionLabel,1,1);
    grid->addWidget(amCompressionSlider,1,2);
    grid->addWidget(amCarrierLevelLabel,2,1);
    grid->addWidget(amCarrierLevelSlider,2,2);
    grid->setHorizontalSpacing(10);





    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->setSpacing(4);
    vbox->addSpacing(6);
    vbox->addLayout(grid);



    amTxSettingsGroup = new QGroupBox(tr("AM Tx Settings"), this);
    amTxSettingsGroup->setLayout(vbox);
    amTxSettingsGroup->setFont(QFont("Arial", 12));

}

void TransmitOptionsWidget::createSourceGroup() {

	QLabel* sourceLabel = new QLabel("Mic Source:", this);
    sourceLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    micInputComboBox = new QComboBox(this);
    micInputComboBox->setMinimumWidth(180);
    QHBoxLayout *hbox5 = new QHBoxLayout();
    hbox5->setSpacing(4);
    hbox5->addWidget(sourceLabel);
    hbox5->addStretch();
    hbox5->addWidget(micInputComboBox);
	
	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setSpacing(4);
	vbox->addSpacing(6);
    vbox->addLayout(hbox5);
	
	sourceGroup = new QGroupBox(tr("Mic Source"), this);
	sourceGroup->setMinimumWidth(m_minimumGroupBoxWidth);
	sourceGroup->setLayout(vbox);
    sourceGroup->setFont(QFont("Arial", 10));

    refreshAudioDevices(set ? set->getMicInputSourceName() : QString());
}

void TransmitOptionsWidget::createTransmitFilterGroup() {

	QLabel* highLabel = new QLabel("High (Hz):", this);
    highLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	QLabel* lowLabel = new QLabel("Low (Hz):", this);
    lowLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	highFilterSpinBox = new QSpinBox(this);
	highFilterSpinBox->setMinimum(1000);
	highFilterSpinBox->setMaximum(5000);
	highFilterSpinBox->setValue(set ? set->getTxFilterHigh() : 3100);

	lowFilterSpinBox = new QSpinBox(this);
	lowFilterSpinBox->setMinimum(0);
	lowFilterSpinBox->setMaximum(1000);
	lowFilterSpinBox->setValue(set ? set->getTxFilterLow() : 200);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(4);
	hbox1->addWidget(highLabel);
	hbox1->addStretch();
	hbox1->addWidget(highFilterSpinBox);
	
	QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->setSpacing(4);
	hbox2->addWidget(lowLabel);
	hbox2->addStretch();
	hbox2->addWidget(lowFilterSpinBox);
	
	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setSpacing(4);
	vbox->addSpacing(6);
	vbox->addLayout(hbox1);
	vbox->addLayout(hbox2);
	
	transmitFilterGroup = new QGroupBox(tr("Transmit Filter"), this);
	transmitFilterGroup->setMinimumWidth(m_minimumGroupBoxWidth);
	transmitFilterGroup->setLayout(vbox);
	transmitFilterGroup->setFont(QFont("Arial", 8));
}

void TransmitOptionsWidget::createPTTOptionsGroup() {

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(4);
	hbox1->addStretch();
	//hbox1->addWidget(penelopeBtn);
	//hbox1->addWidget(mercuryBtn);
	
	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setSpacing(4);
	vbox->addSpacing(6);
	vbox->addLayout(hbox1);
	
	pttOptionsGroup = new QGroupBox(tr("PTT Options"), this);
	pttOptionsGroup->setMinimumWidth(m_minimumGroupBoxWidth);
	pttOptionsGroup->setLayout(vbox);
	pttOptionsGroup->setFont(QFont("Arial", 8));
}


void TransmitOptionsWidget::closeEvent(QCloseEvent *event) {

	emit closeEvent();
	QWidget::closeEvent(event);
}

void TransmitOptionsWidget::showEvent(QShowEvent *event) {

	emit showEvent();
	QWidget::showEvent(event);
}

void TransmitOptionsWidget::enterEvent(QEvent *event) {

	Q_UNUSED(event)
}

void TransmitOptionsWidget::leaveEvent(QEvent *event) {

	Q_UNUSED(event)
}

void TransmitOptionsWidget::mouseMoveEvent(QMouseEvent *event) {

	Q_UNUSED(event)
}

void TransmitOptionsWidget::mousePressEvent(QMouseEvent *event) {

	Q_UNUSED(event)
}

void TransmitOptionsWidget::mouseReleaseEvent(QMouseEvent *event) {

	Q_UNUSED(event)
}


