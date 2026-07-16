#include <QBoxLayout>
#include <QHeaderView>

#include "cusdr_serverWidget.h"

#define	btn_height		15
#define	btn_width		80
#define	btn_width2		52

ServerWidget::ServerWidget(QWidget *parent) 
	: QWidget(parent)
	, m_minimumWidgetWidth(250)
	, m_minimumGroupBoxWidth(240)
	, m_btnSpacing(5)
{
	setContentsMargins(4, 0, 4, 0);
	setMouseTracking(true);

	createServerNIGroup();
	
	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(0,0,0,0);
	mainLayout->addSpacing(8);

	QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->setSpacing(0);
    hbox1->setContentsMargins(0,0,0,0);
	hbox1->addStretch();
	hbox1->addWidget(serverNIGroupBox);

	QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->setSpacing(0);
    hbox2->setContentsMargins(0,0,0,0);
	hbox2->addStretch();
	hbox2->addWidget(portAddressesGroup());

	QHBoxLayout *hbox3 = new QHBoxLayout();
	hbox3->setSpacing(0);
    hbox3->setContentsMargins(0,0,0,0);
	hbox3->addStretch();
	hbox3->addWidget(tciServerGroup());

	mainLayout->addLayout(hbox1);
	mainLayout->addLayout(hbox2);
	mainLayout->addLayout(hbox3);
	mainLayout->addStretch();
		
	setLayout(mainLayout);

	setupConnections();
}

ServerWidget::~ServerWidget() {
	disconnect(nullptr, nullptr, nullptr);
}

QSize ServerWidget::sizeHint() const {
	return QSize(m_minimumWidgetWidth, height());
}

QSize ServerWidget::minimumSizeHint() const {
	return QSize(m_minimumWidgetWidth, height());
}

void ServerWidget::setupConnections() {
	connect(
		serverNetworkInterfaces, 
		&QComboBox::currentIndexChanged, 
		this,
		&ServerWidget::serverNICIndexChanged);
}

void ServerWidget::addNICChangedConnection() {
}

void ServerWidget::createServerNIGroup() {
	serverNetworkInterfaces = new QComboBox();
	serverNetworkInterfaces->setMinimumContentsLength(22);

	QHBoxLayout *hbox1 = new QHBoxLayout;
	hbox1->setSpacing(1);
	hbox1->addWidget(serverNetworkInterfaces);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setSpacing(5);
	vbox->addSpacing(5);
	vbox->addLayout(hbox1);
	vbox->addSpacing(5);
	
	serverNIGroupBox = new QGroupBox(tr("Server network interface"));
	serverNIGroupBox->setMinimumWidth(m_minimumGroupBoxWidth);
	serverNIGroupBox->setLayout(vbox);
	serverNIGroupBox->setFont(QFont("Arial", 8));
}

void ServerWidget::addServerNIEntry(QString niName, QString ipAddress) {
	Q_UNUSED(niName)
	QString item = ipAddress;
	serverNetworkInterfaces->blockSignals(true);
	serverNetworkInterfaces->addItem(item);
	serverNetworkInterfaces->blockSignals(false);
}

QGroupBox *ServerWidget::portAddressesGroup() {
	portGridBox = new QGridLayout;
	portGridBox->setVerticalSpacing(3);

	labelServerPortLabel = new QLabel("Command Server Port:");
    labelServerPortLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
	portGridBox->addWidget(labelServerPortLabel, 0, 0);

	labelServerPortText = new QLabel("");
    labelServerPortText->setFrameStyle(QFrame::Box | QFrame::Raised);
	portGridBox->addWidget(labelServerPortText, 0, 1);

	labelListenerPortLabel = new QLabel("Listener Port:");
    labelListenerPortLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
	portGridBox->addWidget(labelListenerPortLabel, 1, 0);

	labelListenerPortText = new QLabel("");
    labelListenerPortText->setFrameStyle(QFrame::Box | QFrame::Raised);
	portGridBox->addWidget(labelListenerPortText, 1, 1);

	labelAudioPortLabel = new QLabel("Audio Port:");
    labelAudioPortLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
	portGridBox->addWidget(labelAudioPortLabel, 2, 0);

	labelAudioPortText = new QLabel("");
    labelAudioPortText->setFrameStyle(QFrame::Box | QFrame::Raised);
	portGridBox->addWidget(labelAudioPortText, 2, 1);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setSpacing(1);
	vbox->addLayout(portGridBox);

	QGroupBox *groupBox = new QGroupBox(tr("Port Addresses"));
	groupBox->setMinimumWidth(m_minimumGroupBoxWidth);
	groupBox->setLayout(vbox);
	groupBox->setFont(QFont("Arial", 8));

	return groupBox;
}

QGroupBox *ServerWidget::tciServerGroup() {
	tciEnableCheckBox = new QCheckBox(tr("Enable TCI server"));
	tciEnableCheckBox->setFont(QFont("Arial", 8));

	tciPortLabel = new QLabel(tr("WebSocket port: 50001"));
	tciPortLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

	connect(
		tciEnableCheckBox,
		&QCheckBox::toggled,
		this,
		&ServerWidget::tciEnabledToggled);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setSpacing(4);
	vbox->addSpacing(4);
	vbox->addWidget(tciEnableCheckBox);
	vbox->addWidget(tciPortLabel);
	vbox->addSpacing(4);

	QGroupBox *groupBox = new QGroupBox(tr("TCI Server"));
	groupBox->setMinimumWidth(m_minimumGroupBoxWidth);
	groupBox->setLayout(vbox);
	groupBox->setFont(QFont("Arial", 8));

	return groupBox;
}

void ServerWidget::setServerNIC(int index) {
	serverNetworkInterfaces->blockSignals(true);
	serverNetworkInterfaces->setCurrentIndex(index);
	serverNetworkInterfaces->blockSignals(false);
}

void ServerWidget::setTciServerEnabled(bool enabled) {
	if (tciEnableCheckBox) {
		tciEnableCheckBox->blockSignals(true);
		tciEnableCheckBox->setChecked(enabled);
		tciEnableCheckBox->blockSignals(false);
	}
}

void ServerWidget::setTciServerPort(quint16 port) {
	if (tciPortLabel) {
		tciPortLabel->setText(tr("WebSocket port: %1").arg(port));
	}
}

void ServerWidget::setPorts(quint16 serverPort, quint16 listenPort, quint16 audioPort) {
	labelServerPortText->setText(QString::number(serverPort));
	labelListenerPortText->setText(QString::number(listenPort));
	labelAudioPortText->setText(QString::number(audioPort));
}

void ServerWidget::closeEvent(QCloseEvent *event) {
	emit closeEvent();
	QWidget::closeEvent(event);
}

void ServerWidget::showEvent(QShowEvent *event) {
	QWidget::showEvent(event);
}

void ServerWidget::serverNICIndexChanged(int index) {
	emit serverNICRequested(index);
}

void ServerWidget::tciEnabledToggled(bool enabled) {
	emit tciServerEnabledRequested(enabled);
}
