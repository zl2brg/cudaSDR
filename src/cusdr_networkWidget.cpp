/**
* @file cusdr_networkWidget.cpp
* @brief Network settings widget class for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2012-10-24
*/

/*
 *   
 *   Copyright 2010 - 2012 Hermann von Hasseln, DL3HVH
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

#define LOG_NETWORK_WIDGET

// use: NETWORK_WIDGET_DEBUG

//#include <QtGui>
//#include <QMenu>
//#include <QFileDialog>
//#include <QDebug>
#include <QBoxLayout>

#include "cusdr_networkWidget.h"


#define	btn_height		22
#define	btn_width		74
#define	btn_width2		52
#define	btn_widths		42


NetworkWidget::NetworkWidget(QWidget *parent)
    : QTabWidget(parent)
	, set(Settings::instance())
	, m_serverMode(set->getCurrentServerMode())
	, m_hwInterface(set->getHWInterface())
	, m_hwInterfaceTemp(set->getHWInterface())
	, m_dataEngineState(QSDR::DataEngineDown)
	, m_minimumWidgetWidth(set->getMinimumWidgetWidth())
	, m_minimumGroupBoxWidth(0)
	, m_numberOfReceivers(1)
	, m_hpsdrHardware(set->getHPSDRHardware())
    , m_discoveryPassId(0)
{
	setMinimumWidth(m_minimumWidgetWidth);
	setContentsMargins(4, 8, 4, 0);
	setMouseTracking(true);
    setFont(QFont("Arial",10));
	
	m_deviceCards = set->getMetisCardsList();

	createDeviceNetworkInterfaceGroup();
	createDeviceSearchGroup();

	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(0,0,0,0);
	mainLayout->addSpacing(8);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(0);
	hbox1->setContentsMargins(4, 0, 4, 0);
	hbox1->addWidget(hpsdrInterfaceExclusiveBtnGroup());

	QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->setSpacing(0);
	hbox2->setContentsMargins(4, 0, 4, 0);
	hbox2->addWidget(deviceNIGroupBox);

	QHBoxLayout *hbox3 = new QHBoxLayout();
	hbox3->setSpacing(0);
	hbox3->setContentsMargins(4, 0, 4, 0);
	hbox3->addWidget(searchNetworkDeviceGroupBox);

	if (m_hwInterface == QSDR::NoInterfaceMode) {
		
		deviceNIGroupBox->hide();
		searchNetworkDeviceGroupBox->hide();
	}

	mainLayout->addLayout(hbox1);
	mainLayout->addLayout(hbox2);
	mainLayout->addLayout(hbox3);
	mainLayout->addStretch();
	setLayout(mainLayout);

	setupConnections();
    addNICChangedConnection();
	setSocketBufSize(set->getSocketBufferSize());
}

NetworkWidget::~NetworkWidget() {

	disconnect(set, 0, this, 0);
	disconnect(0, 0, 0);
}

void NetworkWidget::setupConnections() {

	CHECKED_CONNECT(
		set,
		&Settings::systemStateChanged,
		this,
		&NetworkWidget::systemStateChanged);

	CHECKED_CONNECT(
		set, 
		&Settings::newHPSDRDeviceNIC, 
		this, 
		&NetworkWidget::addDeviceNICEntry);

	CHECKED_CONNECT(
		set, 
		&Settings::hpsdrDeviceNICChanged, 
		this, 
		&NetworkWidget::setDeviceNIC);

	CHECKED_CONNECT(
		set, 
		&Settings::metisCardListChanged, 
		this, 
		&NetworkWidget::setNetworkDeviceList);

	CHECKED_CONNECT(
		set, 
		&Settings::hpsdrNetworkDeviceChanged,
		this, 
		&NetworkWidget::setCurrentNetworkDevice);

	CHECKED_CONNECT(
		set, 
		&Settings::socketBufferSizeChanged, 
		this, 
		&NetworkWidget::setSocketBufSize);

#ifdef HAVE_SOAPYSDR
    CHECKED_CONNECT(
        set,
        &Settings::soapyDeviceListChanged,
        this,
        &NetworkWidget::setSoapyDeviceList);

    CHECKED_CONNECT(
        set,
        &Settings::soapyDeviceChanged,
        this,
        &NetworkWidget::setCurrentSoapyDevice);
#endif
}

void NetworkWidget::addNICChangedConnection() {

	CHECKED_CONNECT(
		networkDeviceInterfaces, 
		&QComboBox::currentIndexChanged, 
		set, 
		&Settings::setHPSDRDeviceNIC);
}

void NetworkWidget::setDeviceNIC(int index) {

	networkDeviceInterfaces->setCurrentIndex(index);
}

QGroupBox* NetworkWidget::hpsdrInterfaceExclusiveBtnGroup() {

	networkPresenceBtn = new AeroButton("Network", this);
    networkPresenceBtn->setFont(QFont("Arial",8));
	networkPresenceBtn->setRoundness(0);
	networkPresenceBtn->setFixedSize (btn_width, btn_height);

	CHECKED_CONNECT(
		networkPresenceBtn, 
		&AeroButton::clicked, 
		this, 
		&NetworkWidget::interfaceBtnClicked);
	
	noHWBtn = new AeroButton("None", this);
    noHWBtn->setFont(QFont("Arial",8));
    noHWBtn->setRoundness(0);
	noHWBtn->setFixedSize (btn_width, btn_height);
	
	CHECKED_CONNECT(
		noHWBtn, 
		&AeroButton::clicked, 
		this, 
		&NetworkWidget::interfaceBtnClicked);

#ifdef HAVE_SOAPYSDR
    soapyBtn = new AeroButton("SoapySDR", this);
    soapyBtn->setFont(QFont("Arial",8));
    soapyBtn->setRoundness(0);
    soapyBtn->setFixedSize (btn_width, btn_height);

    CHECKED_CONNECT(
        soapyBtn,
        &AeroButton::clicked,
        this,
        &NetworkWidget::interfaceBtnClicked);
#endif

	hwInterfaceChanged();

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(4);
	hbox1->addStretch();
	hbox1->addWidget(noHWBtn);
	hbox1->addWidget(networkPresenceBtn);
#ifdef HAVE_SOAPYSDR
    hbox1->addWidget(soapyBtn);
#endif

	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setSpacing(4);
	vbox->addSpacing(6);
	vbox->addLayout(hbox1);
	
	QGroupBox *groupBox = new QGroupBox(tr("Hardware Interface"), this);
	groupBox->setMinimumWidth(m_minimumGroupBoxWidth);
	groupBox->setLayout(vbox);
	groupBox->setFont(QFont("Arial", 8));

	return groupBox;
}

void NetworkWidget::createDeviceNetworkInterfaceGroup() {

	networkDeviceInterfaces = new QComboBox();

	networkDeviceInterfaces->setMinimumContentsLength(22);
	
	socketBufSizeBtn = new AeroButton("Enable", this);
    socketBufSizeBtn->setFont(QFont("Arial",8));
    socketBufSizeBtn->setRoundness(10);
	socketBufSizeBtn->setFixedSize(btn_widths, btn_height);
	socketBufSizeBtn->setBtnState(AeroButton::OFF);

	CHECKED_CONNECT(
		socketBufSizeBtn,
		&AeroButton::clicked,
		this,
		&NetworkWidget::socketBufSizeBtnClicked);

	socketBufferSizes = new QComboBox();
	socketBufferSizes->addItem("1 kB");
	socketBufferSizes->addItem("8 kB");
	socketBufferSizes->addItem("16 kB");
	socketBufferSizes->addItem("32 kB");
	socketBufferSizes->addItem("64 kB");
	socketBufferSizes->addItem("128 kB");
	socketBufferSizes->addItem("256 kB");
	socketBufferSizes->addItem("512 kB");
	socketBufferSizes->setEnabled(false);

	CHECKED_CONNECT(
		socketBufferSizes,
		&QComboBox::currentIndexChanged,
		this,
		&NetworkWidget::setSocketBufferSize);

	socketBufferSizeLabel = new QLabel("Socket Buffer Size:", this);
	socketBufferSizeLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(1);
	hbox1->addStretch();
	hbox1->addWidget(networkDeviceInterfaces);
	
	QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->setSpacing(1);
	//hbox2->addStretch();
	hbox2->addWidget(socketBufferSizeLabel);
	hbox2->addWidget(socketBufSizeBtn);
	hbox2->addSpacing(3);
	hbox2->addStretch();
	hbox2->addWidget(socketBufferSizes);

	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setSpacing(3);
	vbox->addSpacing(5);
	vbox->addLayout(hbox1);
	vbox->addSpacing(5);
	vbox->addLayout(hbox2);
	
	deviceNIGroupBox = new QGroupBox(tr("Local network interface"), this);
	deviceNIGroupBox->setMinimumWidth(m_minimumGroupBoxWidth);
	deviceNIGroupBox->setLayout(vbox);
	deviceNIGroupBox->setFont(QFont("Arial", 8));
}

void NetworkWidget::createDeviceSearchGroup() {

	searchBtn = new AeroButton("search", this);
	searchBtn->setRoundness(10);
	searchBtn->setFixedSize(btn_width2, btn_height);

	CHECKED_CONNECT(
		searchBtn,
		&AeroButton::clicked,
		this,
		&NetworkWidget::searchBtnClicked);

	deviceCombo = new QComboBox();
	deviceCombo->setMinimumContentsLength(22);

    CHECKED_CONNECT(
        deviceCombo,
        &QComboBox::currentIndexChanged,
        this,
        &NetworkWidget::deviceSelected);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(1);
	hbox1->addStretch();
	hbox1->addWidget(searchBtn);
	hbox1->addSpacing(3);
	hbox1->addWidget(deviceCombo);

	QVBoxLayout *vbox = new QVBoxLayout();

	vbox->setSpacing(3);
	vbox->addSpacing(5);
	vbox->addLayout(hbox1);
	vbox->addSpacing(5);

	searchNetworkDeviceGroupBox = new QGroupBox(tr("Discovered Devices"), this);
	searchNetworkDeviceGroupBox->setMinimumWidth(m_minimumGroupBoxWidth);
	searchNetworkDeviceGroupBox->setLayout(vbox);
	searchNetworkDeviceGroupBox->setFont(QFont("Arial", 8));
}

// ************************************************************************

void NetworkWidget::addDeviceNICEntry(QString niName, QString ipAddress) {
	Q_UNUSED(niName)

	//QString item = niName;
	QString item = ipAddress;
	//item.append(" (");
	//item.append(ipAddress);
	//item.append(")");
	networkDeviceInterfaces->addItem(item);
	//this->repaint();
}

void NetworkWidget::systemStateChanged(
	QSDR::_Error err, 
	QSDR::_HWInterfaceMode hwmode, 
	QSDR::_ServerMode mode, 
	QSDR::_DataEngineState state)
{
	Q_UNUSED (err)

	if (m_hwInterface != hwmode) {
		
		m_hwInterface = hwmode;
		hwInterfaceChanged();
	}

	//m_oldServerMode = m_serverMode;
	if (m_serverMode != mode) {

		enableButtons();
		
		m_serverMode = mode;
	}
		
	if (m_dataEngineState != state) {

		if (state == QSDR::DataEngineUp)
			disableButtons();
		else
			enableButtons();

		m_dataEngineState = state;
	}

	//if (!change) return;
	update();
}

void NetworkWidget::hwInterfaceChanged() {

	switch (m_hwInterface) {

		case QSDR::NoInterfaceMode:

			noHWBtn->setBtnState(AeroButton::ON);
			networkPresenceBtn->setBtnState(AeroButton::OFF);
#ifdef HAVE_SOAPYSDR
            soapyBtn->setBtnState(AeroButton::OFF);
#endif
            deviceNIGroupBox->hide();
            searchNetworkDeviceGroupBox->hide();
			break;

		case QSDR::Metis:
		case QSDR::Hermes:
			
			noHWBtn->setBtnState(AeroButton::OFF);
			networkPresenceBtn->setBtnState(AeroButton::ON);
#ifdef HAVE_SOAPYSDR
            soapyBtn->setBtnState(AeroButton::OFF);
#endif
            
            deviceNIGroupBox->show();
            searchNetworkDeviceGroupBox->show();
            searchNetworkDeviceGroupBox->setTitle(tr("Discovered Devices"));
			break;

#ifdef HAVE_SOAPYSDR
		case QSDR::SoapySDR:
			noHWBtn->setBtnState(AeroButton::OFF);
			networkPresenceBtn->setBtnState(AeroButton::OFF);
            soapyBtn->setBtnState(AeroButton::ON);
            
            deviceNIGroupBox->hide();
            searchNetworkDeviceGroupBox->show();
            searchNetworkDeviceGroupBox->setTitle(tr("Discovered Devices"));
			break;
#endif
	}

	m_hwInterfaceTemp = m_hwInterface;
}

void NetworkWidget::interfaceBtnClicked() {

	AeroButton *button = qobject_cast<AeroButton *>(sender());

	if (button == networkPresenceBtn) { // HPSDR modules

		noHWBtn->setBtnState(AeroButton::OFF);
		noHWBtn->update();
#ifdef HAVE_SOAPYSDR
        soapyBtn->setBtnState(AeroButton::OFF);
        soapyBtn->update();
#endif
		networkPresenceBtn->setBtnState(AeroButton::ON);

		if (m_hpsdrHardware == 0) {

			m_hwInterface = QSDR::Metis;
			NETWORK_WIDGET_DEBUG << "HW interface changed to Metis.";
		}
		else if (m_hpsdrHardware == 1) {

			m_hwInterface = QSDR::Hermes;
			NETWORK_WIDGET_DEBUG << "HW interface changed to Hermes.";
		}
	}
	else
	if (button == noHWBtn) {

		networkPresenceBtn->setBtnState(AeroButton::OFF);
		networkPresenceBtn->update();
#ifdef HAVE_SOAPYSDR
        soapyBtn->setBtnState(AeroButton::OFF);
        soapyBtn->update();
#endif
		noHWBtn->setBtnState(AeroButton::ON);
		m_hwInterface = QSDR::NoInterfaceMode;
		emit messageEvent("[hpsdr]: changed to no-interface mode.");
	}
#ifdef HAVE_SOAPYSDR
    else
    if (button == soapyBtn) {
        noHWBtn->setBtnState(AeroButton::OFF);
        noHWBtn->update();
        networkPresenceBtn->setBtnState(AeroButton::OFF);
        networkPresenceBtn->update();
        soapyBtn->setBtnState(AeroButton::ON);

        m_hwInterface = QSDR::SoapySDR;
        NETWORK_WIDGET_DEBUG << "HW interface changed to SoapySDR.";
    }
#endif

	if (m_hwInterfaceTemp != m_hwInterface) {
        hwInterfaceChanged();
        qDebug() << "HPSDRWidget::  setSystemState.";
        set->setSystemState(
                        QSDR::NoError,
                        m_hwInterface,
                        m_serverMode,
                        m_dataEngineState);
    }
}

void NetworkWidget::searchBtnClicked() {
    ++m_discoveryPassId;
    NETWORK_WIDGET_DEBUG << "discovery pass" << m_discoveryPassId << ": search clicked, clearing combo entries =" << deviceCombo->count();
    deviceCombo->clear();
	set->searchDevices();
}

#ifdef HAVE_SOAPYSDR
void NetworkWidget::setSoapyDeviceList(const QList<TSoapyDevice> &list) {
    auto sameSoapyDevice = [](const TSoapyDevice &a, const TSoapyDevice &b) {
        // Prefer stable unique key when available.
        if (!a.serial.isEmpty() || !b.serial.isEmpty())
            return a.driver == b.driver && a.serial == b.serial;
        // Some drivers leave serial empty; use broader identity fallback.
        return a.driver == b.driver
            && a.hardware == b.hardware
            && a.name == b.name
            && a.label == b.label
            && a.args == b.args;
    };

    int added = 0;
    int skipped = 0;
    const bool prevBlocked = deviceCombo->blockSignals(true);
    foreach (const TSoapyDevice &dev, list) {
        bool exists = false;
        for (int i = 0; i < deviceCombo->count(); ++i) {
            const QVariant data = deviceCombo->itemData(i);
            if (!data.canConvert<TSoapyDevice>())
                continue;
            if (sameSoapyDevice(data.value<TSoapyDevice>(), dev)) {
                exists = true;
                break;
            }
        }
        if (exists) {
            ++skipped;
            continue;
        }

        deviceCombo->addItem("[Soapy] " + dev.label, QVariant::fromValue(dev));
        ++added;
    }
    deviceCombo->blockSignals(prevBlocked);
    NETWORK_WIDGET_DEBUG << "discovery pass" << m_discoveryPassId << ": soapy list signal size =" << list.size() << "added =" << added << "skipped duplicates =" << skipped << "combo entries =" << deviceCombo->count();
}

void NetworkWidget::setCurrentSoapyDevice(TSoapyDevice device) {
    auto sameSoapyDevice = [](const TSoapyDevice &a, const TSoapyDevice &b) {
        if (!a.serial.isEmpty() || !b.serial.isEmpty())
            return a.driver == b.driver && a.serial == b.serial;
        return a.driver == b.driver
            && a.hardware == b.hardware
            && a.name == b.name
            && a.label == b.label
            && a.args == b.args;
    };

    for (int i = 0; i < deviceCombo->count(); ++i) {
        const QVariant data = deviceCombo->itemData(i);
        if (!data.canConvert<TSoapyDevice>())
            continue;
        if (sameSoapyDevice(data.value<TSoapyDevice>(), device)) {
            deviceCombo->blockSignals(true);
            deviceCombo->setCurrentIndex(i);
            deviceCombo->blockSignals(false);
            break;
        }
    }
}
#endif

void NetworkWidget::setNetworkDeviceList(QList<TNetworkDevicecard> list) {

	m_deviceCards = list;

	if (list.length() == 0 ) {
		return;
	}

    int added = 0;
    int skipped = 0;
    const bool prevBlocked = deviceCombo->blockSignals(true);
	foreach (TNetworkDevicecard device, list) {
        bool exists = false;
        const QString mac = QString::fromLatin1(device.mac_address);
        for (int i = 0; i < deviceCombo->count(); ++i) {
            const QVariant data = deviceCombo->itemData(i);
            if (!data.canConvert<TNetworkDevicecard>())
                continue;
            const TNetworkDevicecard existing = data.value<TNetworkDevicecard>();
            if (QString::fromLatin1(existing.mac_address) == mac) {
                exists = true;
                break;
            }
        }
        if (exists) {
            ++skipped;
            continue;
        }

		deviceCombo->addItem("[HPSDR] " + device.ip_address.toString(), QVariant::fromValue(device));
        ++added;
	}
    deviceCombo->blockSignals(prevBlocked);
    NETWORK_WIDGET_DEBUG << "discovery pass" << m_discoveryPassId << ": hpsdr list signal size =" << list.size() << "added =" << added << "skipped duplicates =" << skipped << "combo entries =" << deviceCombo->count();
}

void NetworkWidget::deviceSelected(int index) {
    if (index < 0) return;

    QVariant data = deviceCombo->itemData(index);
    
    if (data.canConvert<TNetworkDevicecard>()) {
        TNetworkDevicecard card = data.value<TNetworkDevicecard>();
        set->setCurrentHPSDRDevice(card);
        // Automatically switch to HPSDR mode
        QSDR::_HWInterfaceMode wanted = (card.protocol == 2) ? QSDR::Hermes : QSDR::Metis;
        if (m_hwInterface != wanted) {
            m_hwInterface = wanted;
            hwInterfaceChanged();
            set->setSystemState(QSDR::NoError, m_hwInterface, m_serverMode, m_dataEngineState);
        }
    } 
#ifdef HAVE_SOAPYSDR
    else if (data.canConvert<TSoapyDevice>()) {
        TSoapyDevice dev = data.value<TSoapyDevice>();
        set->setCurrentSoapyDevice(dev);
        // Automatically switch to SoapySDR mode
        if (m_hwInterface != QSDR::SoapySDR) {
            m_hwInterface = QSDR::SoapySDR;
            hwInterfaceChanged();
            set->setSystemState(QSDR::NoError, m_hwInterface, m_serverMode, m_dataEngineState);
        }
    }
#endif
}

void NetworkWidget::setCurrentNetworkDevice(TNetworkDevicecard card) {
    // Search in combo
    for (int i = 0; i < deviceCombo->count(); ++i) {
        QVariant data = deviceCombo->itemData(i);
        if (data.canConvert<TNetworkDevicecard>()) {
            if (data.value<TNetworkDevicecard>().ip_address == card.ip_address) {
                deviceCombo->blockSignals(true);
                deviceCombo->setCurrentIndex(i);
                deviceCombo->blockSignals(false);
                break;
            }
        }
    }
}

void NetworkWidget::socketBufSizeBtnClicked() {

	if (socketBufSizeBtn->btnState() == AeroButton::OFF) {

		socketBufferSizes->setEnabled(true);
		socketBufferSizes->blockSignals(true);
		setSocketBufSize(set->getSocketBufferSize());
		socketBufferSizes->blockSignals(false);
		//NETWORK_WIDGET_DEBUG << "getSocketBufferSize() :" << set->getSocketBufferSize();
		set->setManualSocketBufferSize(true);
		socketBufSizeBtn->setText("Disable");
		socketBufSizeBtn->setBtnState(AeroButton::ON);
	}
	else {

		socketBufferSizes->setEnabled(false);
		set->setManualSocketBufferSize(false);
		socketBufSizeBtn->setText("Enable");
		socketBufSizeBtn->setBtnState(AeroButton::OFF);
	}

	socketBufSizeBtn->update();
}

void NetworkWidget::disableButtons() {

	noHWBtn->setEnabled(false);
	networkPresenceBtn->setEnabled(false);
#ifdef HAVE_SOAPYSDR
    soapyBtn->setEnabled(false);
#endif
}

void NetworkWidget::enableButtons() {

	noHWBtn->setEnabled(true);
	networkPresenceBtn->setEnabled(true);
#ifdef HAVE_SOAPYSDR
    soapyBtn->setEnabled(true);
#endif
}

void NetworkWidget::setSocketBufSize(int size) {

	m_socketBufferSize = size;

	switch (m_socketBufferSize) {

		case 1:
			socketBufferSizes->setCurrentIndex(0);
			break;

		case 8:
			socketBufferSizes->setCurrentIndex(1);
			break;

		case 16:
			socketBufferSizes->setCurrentIndex(2);
			break;

		case 32:
			socketBufferSizes->setCurrentIndex(3);
			break;

		case 64:
			socketBufferSizes->setCurrentIndex(4);
			break;

		case 128:
			socketBufferSizes->setCurrentIndex(5);
			break;

		case 256:
			socketBufferSizes->setCurrentIndex(6);
			break;

		case 512:
			socketBufferSizes->setCurrentIndex(7);
			break;
	}
	
}

void NetworkWidget::setSocketBufferSize(int value) {

	switch (value) {

		case 0:
			set->setSocketBufferSize(1);
			break;

		case 1:
			set->setSocketBufferSize(8);
			break;

		case 2:
			set->setSocketBufferSize(16);
			break;

		case 3:
			set->setSocketBufferSize(32);
			break;

		case 4:
			set->setSocketBufferSize(64);
			break;

		case 5:
			set->setSocketBufferSize(128);
			break;

		case 6:
			set->setSocketBufferSize(256);
			break;

		case 7:
			set->setSocketBufferSize(512);
			break;
	}
}
