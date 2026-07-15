#define LOG_NETWORK_WIDGET

#include <QBoxLayout>

#include "cusdr_networkWidget.h"
#include "Util/device_identity.h"

#define	btn_height		22
#define	btn_width		74
#define	btn_width2		52
#define	btn_widths		42

NetworkWidget::NetworkWidget(QWidget *parent)
    : QTabWidget(parent)
	, m_serverMode(QSDR::SDRMode)
	, m_hwInterface(QSDR::NoInterfaceMode)
	, m_hwInterfaceTemp(QSDR::NoInterfaceMode)
	, m_dataEngineState(QSDR::DataEngineDown)
	, m_minimumWidgetWidth(500)
	, m_minimumGroupBoxWidth(0)
	, m_numberOfReceivers(1)
	, m_hpsdrHardware(0)
    , m_discoveryPassId(0)
{
	setMinimumWidth(m_minimumWidgetWidth);
	setContentsMargins(4, 8, 4, 0);
	setMouseTracking(true);
    setFont(QFont("Arial",10));
	
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

    addNICChangedConnection();
}

NetworkWidget::~NetworkWidget() {
	disconnect(0, 0, 0);
}

void NetworkWidget::addNICChangedConnection() {
	CHECKED_CONNECT(
		networkDeviceInterfaces, 
		&QComboBox::currentIndexChanged, 
		this, 
		&NetworkWidget::onDeviceNICChanged);
}

void NetworkWidget::onDeviceNICChanged(int index) {
	emit nicInterfaceSelected(index);
}

void NetworkWidget::setDeviceNIC(int index) {
	const bool prev = networkDeviceInterfaces->blockSignals(true);
	networkDeviceInterfaces->setCurrentIndex(index);
	networkDeviceInterfaces->blockSignals(prev);
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
		&NetworkWidget::onSocketBufferSizeChanged);

	socketBufferSizeLabel = new QLabel("Socket Buffer Size:", this);
	socketBufferSizeLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(1);
	hbox1->addStretch();
	hbox1->addWidget(networkDeviceInterfaces);
	
	QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->setSpacing(1);
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

void NetworkWidget::addDeviceNICEntry(QString niName, QString ipAddress) {
	Q_UNUSED(niName)
	networkDeviceInterfaces->addItem(ipAddress);
}

void NetworkWidget::setHwInterface(QSDR::_HWInterfaceMode mode) {
	if (m_hwInterface != mode) {
		m_hwInterface = mode;
		hwInterfaceChanged();
	}
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
	QSDR::_HWInterfaceMode wanted = m_hwInterface;

	if (button == networkPresenceBtn) {
		if (m_hpsdrHardware == 0) wanted = QSDR::Metis;
		else wanted = QSDR::Hermes;
	} else if (button == noHWBtn) {
		wanted = QSDR::NoInterfaceMode;
	}
#ifdef HAVE_SOAPYSDR
	else if (button == soapyBtn) {
		wanted = QSDR::SoapySDR;
	}
#endif

	if (wanted != m_hwInterface) {
		emit hwInterfaceModeRequested(wanted);
	}
}

void NetworkWidget::searchBtnClicked() {
    ++m_discoveryPassId;
    deviceCombo->clear();
	emit searchDevicesRequested();
}

#ifdef HAVE_SOAPYSDR
void NetworkWidget::setSoapyDevicesList(const QList<TSoapyDevice>& list, const TSoapyDevice& active) {
    const bool prevBlocked = deviceCombo->blockSignals(true);
    deviceCombo->clear();
    int activeIdx = -1;
    for (int i = 0; i < list.size(); ++i) {
        const TSoapyDevice &dev = list.at(i);
        deviceCombo->addItem("[Soapy] " + dev.label, QVariant::fromValue(dev));
        if (sameSoapyDevice(dev, active)) {
            activeIdx = i;
        }
    }
    if (activeIdx >= 0) {
        deviceCombo->setCurrentIndex(activeIdx);
    }
    deviceCombo->blockSignals(prevBlocked);
}

void NetworkWidget::setCurrentSoapyDevice(TSoapyDevice device) {
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

void NetworkWidget::setMetisCardsList(const QList<TNetworkDevicecard>& list, const TNetworkDevicecard& active) {
    const bool prevBlocked = deviceCombo->blockSignals(true);
    deviceCombo->clear();
    int activeIdx = -1;
    for (int i = 0; i < list.size(); ++i) {
        const TNetworkDevicecard &device = list.at(i);
        deviceCombo->addItem("[HPSDR] " + device.ip_address.toString(), QVariant::fromValue(device));
        if (sameHpsdrDeviceByMac(device, active)) {
            activeIdx = i;
        }
    }
    if (activeIdx >= 0) {
        deviceCombo->setCurrentIndex(activeIdx);
    }
    deviceCombo->blockSignals(prevBlocked);
}

void NetworkWidget::deviceSelected(int index) {
    if (index < 0) return;

    QVariant data = deviceCombo->itemData(index);
    if (data.canConvert<TNetworkDevicecard>()) {
        TNetworkDevicecard card = data.value<TNetworkDevicecard>();
        emit currentHpsdrDeviceSelected(card);
    } 
#ifdef HAVE_SOAPYSDR
    else if (data.canConvert<TSoapyDevice>()) {
        TSoapyDevice dev = data.value<TSoapyDevice>();
        emit currentSoapyDeviceSelected(dev);
    }
#endif
}

void NetworkWidget::setCurrentNetworkDevice(TNetworkDevicecard card) {
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
		emit manualSocketBufferSizeRequested(true);
	} else {
		emit manualSocketBufferSizeRequested(false);
	}
}

void NetworkWidget::setManualSocketBufferSize(bool manual) {
	socketBufSizeBtn->blockSignals(true);
	if (manual) {
		socketBufferSizes->setEnabled(true);
		socketBufSizeBtn->setText("Disable");
		socketBufSizeBtn->setBtnState(AeroButton::ON);
	} else {
		socketBufferSizes->setEnabled(false);
		socketBufSizeBtn->setText("Enable");
		socketBufSizeBtn->setBtnState(AeroButton::OFF);
	}
	socketBufSizeBtn->blockSignals(false);
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
	const bool prev = socketBufferSizes->blockSignals(true);
	switch (m_socketBufferSize) {
		case 1:   socketBufferSizes->setCurrentIndex(0); break;
		case 8:   socketBufferSizes->setCurrentIndex(1); break;
		case 16:  socketBufferSizes->setCurrentIndex(2); break;
		case 32:  socketBufferSizes->setCurrentIndex(3); break;
		case 64:  socketBufferSizes->setCurrentIndex(4); break;
		case 128: socketBufferSizes->setCurrentIndex(5); break;
		case 256: socketBufferSizes->setCurrentIndex(6); break;
		case 512: socketBufferSizes->setCurrentIndex(7); break;
	}
	socketBufferSizes->blockSignals(prev);
}

void NetworkWidget::onSocketBufferSizeChanged(int index) {
	int size = 32;
	switch (index) {
		case 0: size = 1;   break;
		case 1: size = 8;   break;
		case 2: size = 16;  break;
		case 3: size = 32;  break;
		case 4: size = 64;  break;
		case 5: size = 128; break;
		case 6: size = 256; break;
		case 7: size = 512; break;
	}
	emit socketBufferSizeRequested(size);
}
