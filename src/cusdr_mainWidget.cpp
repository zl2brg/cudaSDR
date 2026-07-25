#include "Models/RadioModel.h"
#include "Models/SliceModel.h"
/**
* @file  cusdr_mainWidget.cpp
* @brief main window widget class for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2011-01-06
*/

/*   
 *   Copyright 2010, 2011, 2012 Hermann von Hasseln, DL3HVH
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

#define LOG_MAIN
#define DOCK_WIDTH  420

#ifdef LOG_NETWORKDIALOG
#define NETWORKDIALOG_DEBUG qDebug().nospace() << "NetworkDialog::\t"
#else
#define NETWORKDIALOG_DEBUG nullDebug()
#endif

//#include <QtGui>
//#include <QHBoxLayout>
//#include <QVBoxLayout>
//#include <QtNetwork>
//#include <QElapsedTimerr>
#include <QScrollArea>
#include <QCoreApplication>
#include <QEventLoop>
#include "cusdr_audio_settingsdialog.h"
#include "cusdr_mainWidget.h"
#include "Controllers/ServerSettingsController.h"
#include "UI/DeviceSelectionDialog.h"
#include "UI/MainWindow/MainWindowUI.h"
#include "Util/device_identity.h"
#include "Util/cusdr_tciserver.h"

extern "C" int GetWDSPVersion();

#define window_height1		600
#define window_height2		750
#define window_width1		800
#define window_width2		1030
#define btn_width1			75
#define btn_width2			54
#define btn_width3			48
#define btn_height1			21
#define btn_height2			13
#define btn_height3			16



/*!
	\class MainWindow
	\brief This class implements main window of the application.
*/
/*!
	\brief Creates a new #MainWindow with the given \a parent.
	This function does following steps:
	- set up GUI.
	- set up connections.
	- find network connections.
*/
MainWindow::MainWindow(RadioModel *model, Settings* settingsModel, QWidget *parent)
	: QMainWindow(parent)
	, set(settingsModel)
	, m_serverMode(set->getCurrentServerMode())
	, m_hwInterface(set->getHWInterface())
	, m_dataEngineState(QSDR::DataEngineDown)
	, m_mover(false)
        , m_resizePosition(0)
        , m_radioModel(model)
{
    ui = new MainWindowUI(this, settingsModel);
    setupWidget = new QDialog(this);
    setupWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

	QPalette palette;
	QColor color = Qt::black;
	color.setAlpha(255);
	palette.setColor(QPalette::Window, color);
	setPalette(palette);

	setAutoFillBackground(true);
	setMouseTracking(true);
	setContentsMargins(0, 0, 0, 0);

	m_fullScreen = false;

	// save and reload the windows size and state
	m_windowsSettingsFilename = "windowsSettings.ini";
	QSettings settings(QCoreApplication::applicationDirPath() +  "/" + m_windowsSettingsFilename, QSettings::IniFormat);
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());

	// Dock windows options
	setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks);
	setMinimumSize(QSize(window_width1, window_height1));

	m_oldSampleRate = set->getSampleRate();
	m_numberOfReceivers = set->getNumberOfReceivers();
	m_alexConfig = set->getAlexConfig();
	m_alexStates = set->getAlexStates();
	m_mercuryAttn = set->getMercuryAttenuators(0);
	m_currentHamBand = set->getCurrentHamBand(0);

	m_alexAttnState = 0x03 & (m_alexStates[m_currentHamBand] >> 7);
	m_mercuryAttnState = m_mercuryAttn.at(m_currentHamBand);

	fonts = new CFonts(this);
	m_fonts = fonts->getFonts();

	// the SDR data engine
	m_dataEngine = new DataEngine(m_radioModel, this);

	// control widgets
    m_serverWidget = new ServerWidget(this);
    m_serverSettingsController = new ServerSettingsController(this);
    m_serverSettingsController->bind(m_serverWidget, set);
    m_hpsdrTabWidget = new cusdr_SetupWidget(m_radioModel, this);

	m_wbDisplay = 0;

    m_serverWidget->hide();
    m_hpsdrTabWidget->hide();
	MAIN_DEBUG << "main window init done";
}

/*!
	\brief MainWindow Destructor
*/
MainWindow::~MainWindow() {
    qDebug() <<  "MainWindow delete";
    m_shuttingDown = true;
    // closeEvent normally stops/deletes the engine; if we get here without that, stop first.
    if (m_dataEngine && m_dataEngineState == QSDR::DataEngineUp)
	    m_dataEngine->stop();
    disconnect(set, 0, this, 0);
    disconnect(0, 0, 0);
    delete fonts;
    // the SDR data engine
    delete  m_dataEngine;
    m_dataEngine = nullptr;
    // control widgets
    delete m_serverWidget;
    delete m_hpsdrTabWidget;
    delete ui;
}



/*!
	\brief set up connections.
*/
void MainWindow::setupConnections() {

	CHECKED_CONNECT_OPT(
		set,
		&Settings::systemMessageEvent,
		this,
		&MainWindow::showStatusBarMessage,
		Qt::DirectConnection);

	CHECKED_CONNECT(
		m_dataEngine,
		&DataEngine::clearSystemMessageEvent,
		this,
		&MainWindow::clearStatusBarMessage);

	CHECKED_CONNECT(
	        set,
	        &Settings::radioStateChanged,
	        this,
	        &MainWindow::radioStateChange);





	CHECKED_CONNECT(
		set,
		&Settings::cpuLoadChanged, 
		this, 
		&MainWindow::updateStatusBar);

	CHECKED_CONNECT(
		set,
		&Settings::masterSwitchChanged, 
		this, 
		&MainWindow::masterSwitchChanged);

	if (TciServer *tci = set->tciServer()) {
		CHECKED_CONNECT(
			tci,
			&TciServer::startRequested,
			this,
			&MainWindow::onTciStartRequested);
		CHECKED_CONNECT(
			tci,
			&TciServer::stopRequested,
			this,
			&MainWindow::onTciStopRequested);
	}

	CHECKED_CONNECT(
	set,
	&Settings::hpsdrNetworkDeviceChanged,
	this,
	&MainWindow::checkStartButtonState);

	CHECKED_CONNECT(
	set,
	&Settings::metisCardListChanged,
	this,
	&MainWindow::handleDeviceListChanged);

	#ifdef HAVE_SOAPYSDR
	CHECKED_CONNECT(
	set,
	&Settings::soapyDeviceChanged,
	this,
	&MainWindow::checkStartButtonState);

	CHECKED_CONNECT(
	set,
	&Settings::soapyDeviceListChanged,
	this,
	&MainWindow::handleSoapyDeviceListChanged);
	#endif

	CHECKED_CONNECT(
	set,
	&Settings::systemStateChanged,
	this,
	&MainWindow::checkStartButtonState);

	CHECKED_CONNECT(
		set, 

		&Settings::numberOfRXChanged, 
		this, 
		&MainWindow::setNumberOfReceivers);

	CHECKED_CONNECT(
		set,
		&Settings::systemStateChanged,
		this,
		&MainWindow::systemStateChanged);

	CHECKED_CONNECT(
		set,
		&Settings::currentReceiverChanged,
		this,
		&MainWindow::setCurrentReceiver);

	CHECKED_CONNECT(
		set,
		&Settings::callsignChanged,
		this,
		&MainWindow::updateTitle);

	CHECKED_CONNECT(
		set,
		&Settings::showNetworkIO,
		this,
		&MainWindow::showNetworkIODialog);

	CHECKED_CONNECT(
		set,
        &Settings::showWarning,
		this,
        &MainWindow::showWarningDialog);

    CHECKED_CONNECT(
        set,
        &Settings::clearDiscoveredDevicesSignal,
        this,
        &MainWindow::clearDiscoveredDevices);
		
	CHECKED_CONNECT(
		set,
		&Settings::showRadioPopupChanged,
		this,
		&MainWindow::showRadioPopup);

	CHECKED_CONNECT(
		set,
        &Settings::txAllowedChanged,
		this,
        &MainWindow::setTxAllowed);

	CHECKED_CONNECT(
		set,
		&Settings::adcModeChanged,
		this,
		&MainWindow::setADCMode);

	if (m_radioModel) {
		for (SliceModel* slice : m_radioModel->slices()) {
			if (!slice) continue;
			const int rx = slice->id();
			connect(slice, &SliceModel::volumeChanged, this, [this, rx](float vol) {
				if (set->getCurrentReceiver() != rx) return;
				ui->volumeSlider->blockSignals(true);
				ui->volumeSlider->setValue(static_cast<int>(vol * 100.0f));
				ui->volumeSlider->blockSignals(false);
				ui->volLevelLabel->setText(QString("%1 %").arg(static_cast<int>(vol * 100.0f), 2, 10, QLatin1Char(' ')));
			});
			connect(slice, &SliceModel::agcModeChanged, this,
			        [this, rx](AGCMode mode) {
				        if (set->getCurrentReceiver() == rx) setAGCMode(rx, mode, false);
			        });
			connect(slice, &SliceModel::agcMaxGainChanged, this,
			        [this, rx](int gain) {
				        if (set->getCurrentReceiver() == rx) setAGCGain(rx, static_cast<qreal>(gain));
			        });
			connect(slice, &SliceModel::agcFixedGainChanged, this,
			        [this, rx](int gain) {
				        if (set->getCurrentReceiver() == rx) setAGCGain(rx, static_cast<qreal>(gain));
			        });
		}
	} else {
		connect(set, &Settings::agcModeChanged, this, &MainWindow::setAGCMode);
		connect(set, &Settings::agcMaximumGainChanged_dB, this,
		        qOverload<int, qreal>(&MainWindow::setAGCGain));
		connect(set, &Settings::agcFixedGainChanged_dB, this,
		        qOverload<int, qreal>(&MainWindow::setAGCGain));
	}

	CHECKED_CONNECT(
		set,
		&Settings::mercuryAttenuatorChanged,
		this,
		&MainWindow::mercuryAttenuatorChanged);

	CHECKED_CONNECT(
		set,
		&Settings::alexPresenceChanged,
		this,
		&MainWindow::alexPresenceChanged);

	CHECKED_CONNECT(
		set,
		&Settings::alexConfigurationChanged,
		this,
		&MainWindow::alexConfigurationChanged);

	CHECKED_CONNECT(
		set,
        &Settings::alexStateChanged,
		this,
        &MainWindow::alexStateChanged);
}

/*!
	\brief setup the main window (called in main.cpp):
	- create main button group
	- create dock widgets
	- create mode menus
	- setup main layout
	- setup connections
	- setup dialog windows
	- get network interfaces
*/
void MainWindow::setup() {
	
	//runFFTWWisdom();

	// create the big display panel at the top of the application.	
    ui->setup();
	
	// the wideband display
    m_wbDisplay = new QGLWidebandPanel(this);

	// create the receiver panels
    createReceiverPanels(MAX_RECEIVERS);
	
	// setup the layout for the control widgets, the wideband panel and the receiver panels
    setupLayout();

	// sync dock visibility to the persisted receiver count (no signal fires when count
	// is restored from INI with blockSignals, so we do it explicitly here)
	setNumberOfReceivers(set->getNumberOfReceivers());

	// set the main window title
	updateTitle();

	// show the wideband data panel as specified in the settings.ini
	if (set->getWidebandData()) {

		ui->wideBandBtn->setEnabled(true);

		if (set->getWidebandStatus()) {
			wideBandBtnClickedEvent();
		}
	}
	
	// get the network interfaces
	getNetworkInterfaces();

	// init network IO dialog to HPSDR components

	// init warning dialog
    m_warningDialog = new WarningDialog(this);
    m_warningDialog->hide();
        // setup connection for the NIC lists of the server and hpsdr widgets.
	// We need to add these connections after detecting the network interfaces.
	m_serverWidget->addNICChangedConnection();
    //m_hpsdrTabWidget->addNICChangedConnection();
	
	// experimental:
	// check for OpenCL devices
	//QList<QCLDevice> clDevices = QCLDevice::allDevices();
	//if (clDevices.length() == 0)
	//	showMessage("[main]: no OpenCL devices found.");

	//else {

	//	m_message = "[main]: found %1 OpenCL device(s).";
	//	showStatusBarMessage(m_message.arg(clDevices.length()), 5000);
	//	//QString clNo = QString::number(m_clDevices.length());
	//}
	//set->setOpenCLDevices(clDevices);

	// set the centralwidget as the central widget of the main window,
	// i.e., we have a second QMainWindow as the central widget.
	setCentralWidget(centralwidget);

	// update the display panel
  //  ui->m_oglDisplayPanel->update();

	// set the Alex configuration
	alexConfigurationChanged(m_alexConfig);

	// set the value of the attenuator(s)
	setAttenuatorButton();

	m_agcMode = set->getAGCMode(0);
	m_adcMode = set->getADCMode(0);

	// initialize all Signal/Slot connections
	setupConnections();

    m_isStartupDiscovery = true;
	updateFromSettings();
    checkStartButtonState();

    m_discoveryTimer.setSingleShot(true);
    connect(&m_discoveryTimer, &QTimer::timeout, this, &MainWindow::processDiscoveryResults);

    QTimer::singleShot(1000, set, &Settings::searchDevices);
    m_discoveryTimer.start(2500); // Wait 2.5s for discovery results
}

void MainWindow::cusdr_setup()
{
    setupWidget->show();
  //bandwidget->show();
    rxDock->show();
  //miniModeWidget->show();

}


 
/*!
	\brief updates the OpenGL widget.
*/
void MainWindow::update() {

	//if (m_oglWidget)
	//	m_oglWidget->update();
}

/*!
	\brief set up the main layout.
*/
void MainWindow::setupLayout() {

	centralwidget = new QMainWindow(this);
	centralwidget->setWindowFlags(Qt::Widget);
	centralwidget->setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks);
	centralwidget->setContextMenuPolicy(Qt::NoContextMenu);  //setStyleSheet(set->getMenuStyle());
    

	// server control widget
	QDockWidget *dock = new QDockWidget(tr("Server Ctrl"), this);
	dock->setObjectName("ServerCtrl");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setFeatures(QDockWidget::DockWidgetFloatable);
    dock->setMaximumWidth(1200);
    dock->setMinimumWidth(DOCK_WIDTH);
    dock->setWidget(m_serverWidget);
    dockWidgetList.append(dock);

    addDockWidget(Qt::RightDockWidgetArea, dock);
    dock->hide();


    // CUDR Setup control widget
    dock = new QDockWidget(tr("CUSDR Ctrl"), this);
	dock->setObjectName("HPSDRCtrl");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    dock->setMaximumWidth(1200);
    dock->setMinimumWidth(DOCK_WIDTH);
	dock->setWidget(m_hpsdrTabWidget);
	dockWidgetList.append(dock);

    addDockWidget(Qt::RightDockWidgetArea, dock);
	dock->hide();

    rxDock = new QDockWidget(tr("Band Ctrl"), this);
    rxDock->setObjectName("bBandWidget");
    rxDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
//	dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    rxDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    rxDock->setMaximumWidth(1200);
    rxDock->setMinimumWidth(DOCK_WIDTH);
//    rxDock->setWidget(filterwidget);
//    rxDock->setWidget(m_radioCtrl);
    dockWidgetList.append(rxDock);

    addDockWidget(Qt::LeftDockWidgetArea, rxDock);
    rxDock->hide();


	// receiver and wideband panel docks;
	// set receiver 0 as the main receiver
	centralwidget->setCentralWidget(rxWidgetList.at(0));

	// wideband panel dock window
	widebandDock = new QDockWidget(tr("Wideband"), this);
	widebandDock->setObjectName("Wideband");
	widebandDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    widebandDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    widebandDock->setWidget(m_wbDisplay);
	
    centralwidget->addDockWidget(Qt::TopDockWidgetArea, widebandDock);
    widebandDock->hide();

	CHECKED_CONNECT(
		widebandDock,
		SIGNAL(visibilityChanged(bool)),
		this,
		SLOT(widebandVisibilityChanged(bool)));

	// receiver dock windows
    for (int i = 1; i < MAX_RECEIVERS; i++) {

		QString str = "Receiver ";
		QString num = QString::number(i+1);
		str.append(num);
		dock = new QDockWidget(str, this);
		widebandDock->setObjectName(str);
		dock->setWidget(rxWidgetList.at(i));
		rxDockWidgetList.append(dock);

		centralwidget->addDockWidget(Qt::BottomDockWidgetArea, dock);
		dock->hide();

		rxVolumeList << 0.0f;
		//ui->viewMenu->addAction(dock->toggleViewAction());
	}

	// the outline of the receiver panels
	for (int i = 0; i < (MAX_RECEIVERS-1)/2; i++) {
        if (i + 4 < rxDockWidgetList.size()) {
            centralwidget->splitDockWidget(rxDockWidgetList.at(i), rxDockWidgetList.at(i+4), Qt::Vertical);
        }
	}

	// Create and add 3D Panadapter dock widget if display tab widget exists in setup widget
	// Note: m_hpsdrTabWidget is actually a cusdr_SetupWidget that contains the DisplayTabWidget
	if (m_hpsdrTabWidget) {
		cusdr_SetupWidget* setupWidget = qobject_cast<cusdr_SetupWidget*>(m_hpsdrTabWidget);
		if (setupWidget) {
			DisplayTabWidget* displayTabWidget = setupWidget->getDisplayTabWidget();
			if (displayTabWidget) {
				displayTabWidget->create3DDockWidget(centralwidget);
				m_3DPanDock = displayTabWidget->get3DDockWidget();
				if (m_3DPanDock) {
					centralwidget->addDockWidget(Qt::BottomDockWidgetArea, m_3DPanDock);
					m_3DPanDock->hide();
				}
			}
		}
	}

	//ui->viewMenu->addAction(dock->toggleViewAction());
}

/*!
	\brief create the receiver panels.
*/
void MainWindow::createReceiverPanels(int rx) {

	rxWidgetList.clear();
	
	for (int i = 0; i < rx; i++) {
        if (!m_radioModel || i >= m_radioModel->slices().size()) continue;
	
                QGLReceiverPanel* rxPanel = new QGLReceiverPanel(m_radioModel->slices().at(i), this);
                rxPanel->setObjectName(QString("RxPanel_%1").arg(i));
		rxWidgetList.append(rxPanel);
    }
}


/*!
	\brief update the status tool bar content with
	the CPU load and the local date & time.
*/
void MainWindow::updateStatusBar(short load) {
	ui->updateStatusBar(load);
}

//*******************************************************************************


//*******************************************************************************
 
/*!
	\brief the master switch functionality.
	- starts/stops the data engine.
	- starts/stops the server if in \a QSDR::ExternalDSP mode.
*/
void MainWindow::masterSwitchChanged(
		/*!<[in] the of the signal. */
		bool power						/*!<[in] power on or off*/
) {
	if (power) {

		if (m_dataEngine->initDataEngine()) { // start data engine

			//if (m_serverMode == QSDR::ExternalDSP && !m_hpsdrServer->startServer())
			//	m_hpsdrServer->stopServer();
			return;
		}
		else {

			set->setMainPower(false);
			startButtonClickedEvent();
			return;
		}
	}
	else {

		m_dataEngine->stop();

		//if (m_serverMode == QSDR::ExternalDSP)
		//	m_hpsdrServer->stopServer();
		set->setMainPower(false);
	}
}

/*!
	\brief set the system state according to
	- \a QSDR::_ServerMode,
	- \a QSDR::_HWInterfaceMode,
	- \a QSDR::_DataEngineState.
*/
void MainWindow::systemStateChanged(
	/*!<[in] the of the signal. */
	QSDR::_Error err,					/*!<[in] error state. */
	QSDR::_HWInterfaceMode hwmode,		/*!<[in] HPSDR interface (Metis, Hermes, none). */
	QSDR::_ServerMode mode,				/*!<[in] server mode. */
	QSDR::_DataEngineState state		/*!<[in] data engine state. */
) {
	Q_UNUSED (err)

	//	if (m_hwInterface != hwmode)
		m_hwInterface = hwmode;

	if (m_serverMode != mode)
		m_serverMode = mode;

	if (m_dataEngineState != state)
		m_dataEngineState = state;

	//if (!ui->modeBtn->isEnabled() && m_dataEngineState == QSDR::DataEngineDown)
	//	ui->modeBtn->setEnabled(true);
	ui->modeBtn->setEnabled(m_dataEngineState == QSDR::DataEngineDown);
	ui->moxBtn->setEnabled(m_hwInterface == QSDR::Hermes);
	ui->tunBtn->setEnabled(m_hwInterface == QSDR::Hermes);
    ui->plusRxBtn->setEnabled(m_dataEngineState == QSDR::DataEngineUp);


	if (state == QSDR::DataEngineUp) {

		m_dataEngineState = QSDR::DataEngineUp;
		ui->modeBtn->setEnabled(false);
	}
	else
	if (state == QSDR::DataEngineDown) {

		m_dataEngineState = QSDR::DataEngineDown;
		ui->modeBtn->setEnabled(true);
		//setCurrentReceiver(0);
	}

}

void MainWindow::setSystemState(
			QSDR::_Error err,
			QSDR::_HWInterfaceMode hwmode,
			QSDR::_ServerMode mode,
			QSDR::_DataEngineState state)
{
	m_dataEngine->io.networkIOMutex.lock();
	set->setSystemState(err, hwmode, mode, state);
	m_dataEngine->io.networkIOMutex.unlock();
}

/*!
	\brief show a temporary message on the status bar.
*/
void MainWindow::showStatusBarMessage(const QString &msg, int timeout) {

	statusBar()->showMessage(msg, timeout);
}

/*!
	\brief clear the temporary status message
*/
void MainWindow::clearStatusBarMessage() {

	statusBar()->clearMessage();
}

/*!
	\brief Stop data engine / radio before exit. Uses an explicit stop path
	(not the Start/Stop toggle) so a UI/state desync cannot start the radio.
*/
void MainWindow::ensureEngineStoppedForShutdown()
{
	if (!ui || !set)
		return;

	const bool engineUp = (m_dataEngineState == QSDR::DataEngineUp);
	const bool powerOn = set->getMainPower();
	const bool startOn = (ui->startBtn && ui->startBtn->btnState() == AeroButton::ON);

	if (!powerOn && !engineUp && !startOn)
		return;

	MAIN_DEBUG << "shutdown: stopping radio (power=" << powerOn
		   << " engineUp=" << engineUp << " startBtnOn=" << startOn << ")";

	if (startOn) {
		ui->startBtn->setBtnState(AeroButton::OFF);
		ui->startBtn->setHighlight(QColor(0x91, 0xeb, 0xff));
		ui->startBtn->setText(QStringLiteral("Start"));
	}

	if (ui->muteBtn && ui->muteBtn->btnState() == AeroButton::ON) {
		ui->muteBtn->setBtnState(AeroButton::OFF);
		ui->muteBtn->update();
		if (ui->volumeSlider)
			ui->volumeSlider->setEnabled(true);
		for (int i = 0; i < set->getNumberOfReceivers(); i++)
			set->setMainVolume(i, rxVolumeList.at(i));
	}

	// Prefer the normal power-off path so masterSwitchChanged runs stop().
	if (powerOn) {
		set->setMainPower(false);
	} else if (m_dataEngine && engineUp) {
		m_dataEngine->stop();
	}

	// Let queued stop-side signals settle before widgets/engine are deleted.
	QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

/*!
	\brief closes the application and shut down all engines.
*/
void MainWindow::closeMainWindow() {

	ensureEngineStoppedForShutdown();
	close();
}
 
/*!
	\brief maximizes/minimizes the main window.
*/
void MainWindow::maximizeMainWindow() {

	if (isMaximized()) {

		//m_titlebar->maxBtn->setIcon(QIcon(QString::fromUtf8(":/img/maximize_high.png")));//, QSize(), QIcon::Normal, QIcon::Off);
		//m_titlebar->maxBtn->update();
		showNormal();
	}
	else {

		//m_titlebar->maxBtn->setIcon(QIcon(QString::fromUtf8(":/img/minimize_high.png")));//, QSize(), QIcon::Normal, QIcon::Off);
		//m_titlebar->maxBtn->update();
		showMaximized();
	}
}
 
/*!
	\brief maximizes/minimizes the main window and hide/show titlebar.
*/
void MainWindow::setMainWindowGeometry() {

	if (isMaximized()) {

		setWindowFlags(Qt::Widget);
		//this->move(m_oldPosition);
		this->setGeometry(m_oldPosition.x(), m_oldPosition.y(), m_oldSize.width(), m_oldSize.height());
		showNormal();
	}
	else {

		setWindowFlags(Qt::FramelessWindowHint);

		m_oldSize = this->size();
		m_oldPosition = this->pos();
		QPoint pos = m_oldPosition;

		if (pos.x() < 0) 
			pos.setX(0);
		if (pos.y() < 0) 
			pos.setY(0);

		this->move(pos);
		this->show();
		showMaximized();
	}
}
 
/*!
	\brief sets full screen operation.
*/
void MainWindow::setFullScreen() {

	if (m_fullScreen)
		this->showNormal();
	else
		this->showFullScreen();
}
 
void MainWindow::showRadioPopup(bool value) {

	Q_UNUSED (value)

	//m_radioPopupWidget->showPopupWidget(QCursor::pos());
}

void MainWindow::showAboutDialog() {

	const QString appName = QApplication::applicationName();
	const QString appVersion = QApplication::applicationVersion();
	const int wdspVersion = GetWDSPVersion();

	const QString aboutText =
		tr("<b>%1</b><br>"
		   "Version: %2<br><br>"
		   "Authors:<br>"
		   "Hermann von Hasseln (DL3HVH)<br>"
		   "Philip A. Covington (N8VB) - WDSP<br>"
		   "Contributors: ZL2BRG and project contributors<br><br>"
		   "WDSP Version: %3")
			.arg(appName)
			.arg(appVersion)
			.arg(wdspVersion);

	QMessageBox::about(this, tr("About %1").arg(appName), aboutText);
}

/*!
	\brief updates entries from the ini-file.
*/
void MainWindow::updateFromSettings() {

}
 
/*!
	\brief implements the Radio-display button functionality which opens/closes the radio control widget.
*/
void MainWindow::ctrlDisplayBtnClickedEvent() {

        if (ui->m_oglDisplayPanel->isVisible()) {

                ui->m_oglDisplayPanel->setVisible(false);
        } 
        else {

                ui->m_oglDisplayPanel->setVisible(true);
        }
}
/*!
	\brief implements various Button functionalities.
*/
void MainWindow::widgetBtnClickedEvent() {

	AeroButton *button = qobject_cast<AeroButton *>(sender());
	int on = ui->mainBtnList.indexOf(button);

	//foreach(QWidget *widget, widgetList) {
	foreach(QDockWidget *dockWidget, dockWidgetList) {

		//int off = widgetList.indexOf(widget);
		int off = dockWidgetList.indexOf(dockWidget);

		if (dockWidget->isVisible()) {
				
			dockWidget->hide();
			ui->mainBtnList.at(off)->setBtnState(AeroButton::OFF);
			ui->mainBtnList.at(off)->update();
		}
		else if (on == off) {

			button->setBtnState(AeroButton::ON);
			button->update();
			QDockWidget *dock = dockWidgetList.at(on);
			dock->show();

			// Size the CUSDR settings dock to the content so pages are fully
			// visible without dragging the dock edge / horizontal scrollbar.
			if (dock->objectName() == QLatin1String("HPSDRCtrl") && dock->widget()) {
				const int chrome = 24;
				const int need = qBound(DOCK_WIDTH,
				                       dock->widget()->sizeHint().width() + chrome,
				                       dock->maximumWidth());
				if (dock->width() < need) {
					resizeDocks({dock}, {need}, Qt::Horizontal);
				}
			}
		}
	}
}

/*!
	\brief implements the Start/Stop functionality of the Start/Stop button.
*/
void MainWindow::startButtonClickedEvent() {

	if (ui->startBtn->btnState() == AeroButton::OFF) {

		ui->startBtn->setBtnState(AeroButton::ON);

		QColor col = QColor(180, 0, 0);
		ui->startBtn->setColorOn(col);

		col = QColor(250, 0, 0);
		ui->startBtn->setHighlight(col);
		ui->startBtn->setText("Stop");
    		set->setMainPower(true);
	}
	else if (ui->startBtn->btnState() == AeroButton::ON) {

		ui->startBtn->setBtnState(AeroButton::OFF);

		QColor col = QColor(0x91, 0xeb, 0xff);
		ui->startBtn->setHighlight(col);
		ui->startBtn->setText("Start");
		set->setMainPower(false);

		if (ui->muteBtn->btnState() == AeroButton::ON) {

			ui->muteBtn->setBtnState(AeroButton::OFF);
			ui->muteBtn->update();

			ui->volumeSlider->setEnabled(true);
			for (int i = 0; i < set->getNumberOfReceivers(); i++)
				set->setMainVolume(i, rxVolumeList.at(i));
		}
	}
}

/*!
	\brief TCI START — same data-engine path as the UI Start button.
*/
void MainWindow::onTciStartRequested()
{
	if (m_dataEngineState == QSDR::DataEngineUp || set->getMainPower())
		return;

	ui->startBtn->setBtnState(AeroButton::ON);

	QColor col = QColor(180, 0, 0);
	ui->startBtn->setColorOn(col);

	col = QColor(250, 0, 0);
	ui->startBtn->setHighlight(col);
	ui->startBtn->setText("Stop");
	set->setMainPower(true);
}

/*!
	\brief TCI STOP — same data-engine path as the UI Stop button (not app exit).
*/
void MainWindow::onTciStopRequested()
{
	if (!set->getMainPower() && ui->startBtn->btnState() == AeroButton::OFF)
		return;

	ui->startBtn->setBtnState(AeroButton::OFF);

	QColor col = QColor(0x91, 0xeb, 0xff);
	ui->startBtn->setHighlight(col);
	ui->startBtn->setText("Start");
	set->setMainPower(false);

	if (ui->muteBtn->btnState() == AeroButton::ON) {
		ui->muteBtn->setBtnState(AeroButton::OFF);
		ui->muteBtn->update();

		ui->volumeSlider->setEnabled(true);
		for (int i = 0; i < set->getNumberOfReceivers(); i++)
			set->setMainVolume(i, rxVolumeList.at(i));
	}
}

/*!
	\brief implements the Show/Hide functionality of the wide band data widget.
*/
void MainWindow::wideBandBtnClickedEvent() {

	if (ui->wideBandBtn->btnState() == AeroButton::OFF) {

		ui->wideBandBtn->setBtnState(AeroButton::ON);
		set->setWidebandStatus(true);
		widebandDock->show();
		//showMessage("[server]: wide band data on.");
	}
	else if (ui->wideBandBtn->btnState() == AeroButton::ON) {

		ui->wideBandBtn->setBtnState(AeroButton::OFF);
		set->setWidebandStatus(false);
		widebandDock->hide();
		//showMessage("[server]: wide band data off.");
	}
}

void MainWindow::widebandVisibilityChanged(bool value) {

	if (value)
		ui->wideBandBtn->setBtnState(AeroButton::ON);
	else
		ui->wideBandBtn->setBtnState(AeroButton::OFF);

	ui->wideBandBtn->update();
}

void MainWindow::alexBtnClickedEvent() {

	if (ui->alexBtn->btnState() == AeroButton::OFF) {

		ui->alexBtn->setBtnState(AeroButton::ON);
		ui->alexBtn->setText("Alex Auto");

		//m_alexConfiguration[0].value = false;
		m_alexConfig &= 0xFFFE;

		set->setAlexToManual(false);
	}
	else if (ui->alexBtn->btnState() == AeroButton::ON) {

		ui->alexBtn->setBtnState(AeroButton::OFF);
		ui->alexBtn->setText("Alex Man");

		//m_alexConfiguration[0].value = true;
		m_alexConfig |= 0x01;

		set->setAlexToManual(true);
	}
	//set->setAlexConfiguration(m_alexConfiguration);
}

void MainWindow::alexStateChanged(HamBand band, const QList<int> &states) {

        Q_UNUSED(band)

	m_currentHamBand = set->getCurrentHamBand(0);
	m_alexStates[m_currentHamBand] = states.at(m_currentHamBand);
	m_alexAttnState = 0x03 & (m_alexStates[m_currentHamBand] >> 7);

	setAttenuatorButton();
}

void MainWindow::alexConfigurationChanged(quint16 conf) {

	if (conf & 0x01) {

		ui->alexBtn->setBtnState(AeroButton::OFF);
		ui->alexBtn->setText("Alex Man");
	}
	else {

		ui->alexBtn->setBtnState(AeroButton::ON);
		ui->alexBtn->setText("Alex Auto");
	}

	ui->alexBtn->update();
}

void MainWindow::alexPresenceChanged(bool value) {

	if (value) {

		//if (m_alexConfiguration[0].value)
		if (m_alexConfig & 0x01)
			ui->alexBtn->setText(tr("Alex Man"));
		else
			ui->alexBtn->setText(tr("Alex Auto"));
	}
	else
		ui->alexBtn->setText(tr("Alex"));

	ui->alexBtn->setEnabled(value);

	if (ui->alexAttnActionList.size() >= 4) {
		ui->alexAttnActionList.at(0)->setCheckable(value);
		ui->alexAttnActionList.at(1)->setCheckable(value);
		ui->alexAttnActionList.at(2)->setCheckable(value);
		ui->alexAttnActionList.at(3)->setCheckable(value);
	}

	ui->alexBtn->update();
}

void MainWindow::addReceiver() {
    int num = set->getNumberOfReceivers();
    if (num < MAX_RECEIVERS) {
        set->setReceivers(num + 1);
    }
}

/*!
	\brief tune to last frequency.
*/
void MainWindow::getLastFrequency() {
}

/*!
	\brief updates the main window title.
*/
void MainWindow::updateTitle() {

	setWindowTitle(
		QApplication::applicationName() + "  " + \
		QApplication::applicationVersion() + "         " + \
		set->getCallsign());
}
 
/*!
	\brief show widget event.
*/
void MainWindow::showWidgetEvent(
		/*!<[in] the of the event. */
) {
}

/*!
	\brief hide widget event.
*/
void MainWindow::closeWidgetEvent(
		/*!<[in] the of the event. */
)
{
}

void MainWindow::setCurrentReceiver(int rx) {

	MAIN_DEBUG << "setCurrentReceiver: " << rx;
	ui->volumeSlider->setValue(static_cast<int>(set->getMainVolume(rx) * 100));
	m_agcMode = set->getAGCMode(rx);
	setAGCMode(rx, m_agcMode, false);
}

/*!
	\brief enable the receiver buttons according to the actual number of available receivers.
*/
void MainWindow::setNumberOfReceivers(
		/*!<[in] the of the event. */
		int value					/*!<[in] the number of receivers. */
) {
	ui->viewMenu->clear();
	if (m_3DPanDock)
		ui->viewMenu->addAction(m_3DPanDock->toggleViewAction());
	if (widebandDock)
		ui->viewMenu->addAction(widebandDock->toggleViewAction());
	if (m_3DPanDock || widebandDock)
		ui->viewMenu->addSeparator();
	for (int i = 0; i < value-1; i++) {
		
		ui->viewMenu->addAction(rxDockWidgetList.at(i)->toggleViewAction());
		if (!rxDockWidgetList.at(i)->isVisible())
			rxDockWidgetList.at(i)->show();
	}

	for (int i = value-1; i < MAX_RECEIVERS-1; i++) {
		
		if (rxDockWidgetList.at(i)->isVisible())
			rxDockWidgetList.at(i)->hide();
	}
}


void MainWindow::setMicLevel(int value)
{
    if (value < 0 ) value = 0;
    if (value > 100 ) value = 100;
    if (value < 0 ) value = 0;
    if (value > 100 ) value = 100;
    set->setMicInputLevel(value);

}


void MainWindow::setDriveLevel(int value)
{
    if (value < 0 ) value = 0;
    if (value > 100 ) value = 100;
    set->setDriveLevel(value);
}

/*!
	\brief set the main volume.
*/
void MainWindow::setMainVolume(int value) {
	if (value < 0) value = 0;
	if (value > 100) value = 100;

	const int rx = set->getCurrentReceiver();
	if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) {
		m_radioModel->slices()[rx]->setVolume(value / 100.0f);
		return;
	}

	set->setMainVolume(rx, value / 100.0f);
	ui->volLevelLabel->setText(QString("%1 %").arg(value, 2, 10, QLatin1Char(' ')));
}

/*!
	\brief mute Volume.
*/
void MainWindow::muteBtnClickedEvent() {

	int rcvr = set->getNumberOfReceivers();
	if (ui->muteBtn->btnState() == AeroButton::OFF) {

		ui->muteBtn->setBtnState(AeroButton::ON);
		ui->volumeSlider->setEnabled(false);

		for (int i = 0; i < rcvr; i++) {
			if (m_radioModel && i < m_radioModel->slices().size() && m_radioModel->slices()[i])
				m_radioModel->slices()[i]->setMute(true);
			else {
				rxVolumeList[i] = set->getMainVolume(i);
				set->setMainVolume(i, 0.0f);
			}
		}
	}
	else if (ui->muteBtn->btnState() == AeroButton::ON) {

		ui->muteBtn->setBtnState(AeroButton::OFF);
		ui->volumeSlider->setEnabled(true);

		for (int i = 0; i < rcvr; i++) {
			if (m_radioModel && i < m_radioModel->slices().size() && m_radioModel->slices()[i])
				m_radioModel->slices()[i]->setMute(false);
			else
				set->setMainVolume(i, rxVolumeList.at(i));
		}
	}
}

void MainWindow::setTxAllowed(bool value) {
    if (m_hwInterface == QSDR::SoapySDR) {
        // Soapy TX availability is handled by Soapy source/runtime checks.
        // Keep buttons enabled in Soapy mode so TUNE/MOX control remains usable.
        ui->moxBtn->setEnabled(true);
        ui->tunBtn->setEnabled(true);
        return;
    }

	if (!value) {

		ui->moxBtn->setEnabled(false);
		ui->tunBtn->setEnabled(false);
	}
	else if (set->getPenelopePresence() || set->getPennyLanePresence()
             || (m_hwInterface == QSDR::Hermes)
             || (m_hwInterface == QSDR::SoapySDR)) {

		ui->moxBtn->setEnabled(true);
		ui->tunBtn->setEnabled(true);
	}
}

void MainWindow::setADCMode(int rx, ADCMode mode) {

	Q_UNUSED(rx)

	m_adcMode = mode;
}

void MainWindow::setAGCMode(int rx, AGCMode mode, bool hang) {

	Q_UNUSED(hang)

	m_agcMode = mode;

	if(m_agcMode == (AGCMode) agcOFF) {

		int gain = (int) set->getAGCFixedGain_dB(rx);
		ui->agcGainLabel->setText("AGC-F:");
		ui->agcGainSlider->blockSignals(true);
		ui->agcGainSlider->setValue(gain);
		ui->agcGainSlider->blockSignals(false);

		QString str = " %1 dB";
		ui->agcGainLevelLabel->setText(str.arg(gain, 2, 10, QLatin1Char(' ')));
	}
	else {

		int gain = (int) set->getAGCMaximumGain_dB(rx);
		ui->agcGainLabel->setText("AGC-G:");
		ui->agcGainSlider->blockSignals(true);
		ui->agcGainSlider->setValue(gain);
		ui->agcGainSlider->blockSignals(false);

		QString str = " %1 dB";
		ui->agcGainLevelLabel->setText(str.arg(gain, 2, 10, QLatin1Char(' ')));
	}
}

void MainWindow::setAGCGain(int value) {

	QString str = " %1 dB";
	ui->agcGainLevelLabel->setText(str.arg(value + 0, 2, 10, QLatin1Char(' ')));

	int rx = set->getCurrentReceiver();
	if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) {
		SliceModel* slice = m_radioModel->slices()[rx];
		if (m_agcMode == (AGCMode) agcOFF)
			slice->setAgcFixedGain(value);
		else
			slice->setAgcMaxGain(value);
		return;
	}
	if (m_agcMode == (AGCMode) agcOFF)
		set->setAGCFixedGain_dB(rx, static_cast<qreal>(value));
	else
		set->setAGCMaximumGain_dB(rx, static_cast<qreal>(value));
}

void MainWindow::setAGCGain(int rx, qreal value) {

	Q_UNUSED(rx)

	ui->agcGainSlider->blockSignals(true);
	ui->agcGainSlider->setValue((int) value);

	QString str = " %1 dB";
	ui->agcGainLevelLabel->setText(str.arg((int) value + 0, 2, 10, QLatin1Char(' ')));

	ui->agcGainSlider->blockSignals(false);
}

void MainWindow::setSDRMode(bool) {

	MAIN_DEBUG << "setSDRMode";
	setServerMode(QSDR::SDRMode);

	//if (m_oldSampleRate == 48000 || m_oldSampleRate == 96000 || m_oldSampleRate == 192000)
	//	set->setSampleRate(m_oldSampleRate);
	
	//showMessage("[server]: switched to SDR mode.");
}

void MainWindow::setAttenuator() {

	QAction *action = qobject_cast<QAction *>(sender());

	int mercuryPos = ui->mercuryAttnActionList.indexOf(action);
	int alexPos = ui->alexAttnActionList.indexOf(action);

	if (mercuryPos > -1) {

		foreach(QAction *act, ui->mercuryAttnActionList)
			act->setChecked(false);
		ui->mercuryAttnActionList.at(mercuryPos)->setChecked(true);
		// Direct mapping: list index == step-attenuator value (0=0dB, 1=10dB, 2=20dB, 3=30dB)
		set->setMercuryAttenuator(mercuryPos);
	}

	if (alexPos > -1) {

		foreach(QAction *act, ui->alexAttnActionList)
			act->setChecked(false);
		ui->alexAttnActionList.at(alexPos)->setChecked(true);

		int state = 0;
		state &= 0x7F;
		state |= alexPos << 7;

		set->setAlexState(state);
	}

	// Compute total attenuation (step att + Alex att) for button label
	{
		int stepDb = 0;
		for (int i = 0; i < ui->mercuryAttnActionList.size(); i++)
			if (ui->mercuryAttnActionList.at(i)->isChecked()) { stepDb = i * 10; break; }
		int alexDb = 0;
		for (int i = 0; i < ui->alexAttnActionList.size(); i++)
			if (ui->alexAttnActionList.at(i)->isChecked()) { alexDb = i * 10; break; }
		int totalDb = stepDb + alexDb;
		if (totalDb == 0) {
			ui->attenuatorBtn->setText(tr("Attn 0 dB"));
			ui->attenuatorBtn->setBtnState(AeroButton::OFF);
		} else {
			ui->attenuatorBtn->setText(tr("Attn -%1 dB").arg(totalDb));
			ui->attenuatorBtn->setBtnState(AeroButton::ON);
		}
	}
}

void MainWindow::checkStartButtonState() {
    bool enable = false;
    QSDR::_HWInterfaceMode hw = set->getHWInterface();

    if (hw == QSDR::NoInterfaceMode) {
        // Only if we have some file loaded?
        enable = false; // for now
    } else if (hw == QSDR::Metis || hw == QSDR::Hermes) {
        QHostAddress addr = set->getCurrentMetisCard().ip_address;
        if (!addr.isNull() && addr != QHostAddress::Any && addr != QHostAddress::AnyIPv4) {
            enable = true;
        }
    } 
#ifdef HAVE_SOAPYSDR
    else if (hw == QSDR::SoapySDR) {
        if (!set->getCurrentSoapyDevice().driver.isEmpty()) {
            enable = true;
        }
    }
#endif

    ui->startBtn->setEnabled(enable);

    const TSDRDevice active = set->getLastConnectedDevice();
    QString activeText = "Active device: none";
    if (active.deviceClass == DeviceClass_HPSDR && !active.label.isEmpty()) {
        activeText = QString("Active device: [HPSDR] %1").arg(active.label);
    }
#ifdef HAVE_SOAPYSDR
    else if (active.deviceClass == DeviceClass_SoapySDR && !active.label.isEmpty()) {
        activeText = QString("Active device: [Soapy] %1").arg(active.label);
    }
#endif
    if (ui->activeDeviceLabel)
        ui->activeDeviceLabel->setText(activeText);

    MAIN_DEBUG << "Check start button: hw=" << set->getHWInterfaceModeString(hw) << " enable=" << enable;
}

void MainWindow::setAttenuatorButton() {

	foreach(QAction *act, ui->mercuryAttnActionList) act->setChecked(false);
	foreach(QAction *act, ui->alexAttnActionList) act->setChecked(false);

	// m_mercuryAttnState: 0=0dB, 1=10dB, 2=20dB, 3=30dB  (direct step-att value)
	int stepDb = qBound(0, m_mercuryAttnState, 3) * 10;
	int alexDb  = qBound(0, m_alexAttnState,   3) * 10;
	int totalDb = stepDb + alexDb;

	if (m_mercuryAttnState >= 0 && m_mercuryAttnState < ui->mercuryAttnActionList.size())
		ui->mercuryAttnActionList.at(m_mercuryAttnState)->setChecked(true);

	if (m_alexAttnState >= 0 && m_alexAttnState < ui->alexAttnActionList.size())
		ui->alexAttnActionList.at(m_alexAttnState)->setChecked(true);

	if (totalDb == 0) {
		ui->attenuatorBtn->setText(tr("Attn 0 dB"));
		ui->attenuatorBtn->setBtnState(AeroButton::OFF);
	} else {
		ui->attenuatorBtn->setText(tr("Attn -%1 dB").arg(totalDb));
		ui->attenuatorBtn->setBtnState(AeroButton::ON);
	}
}

void MainWindow::mercuryAttenuatorChanged(HamBand band, int value) {

	Q_UNUSED(value)
    Q_UNUSED(band)

	m_currentHamBand = set->getCurrentHamBand(0);
	m_mercuryAttn[m_currentHamBand] = value;
	m_mercuryAttnState = m_mercuryAttn.at(m_currentHamBand);

	setAttenuatorButton();
}

/*!
	\brief set \a QSDR::_ServerMode to \a mode.
*/
void MainWindow::setServerMode(
		QSDR::_ServerMode mode			/*!<[in] server mode. */
) {
    /*
	if (mode != QSDR::ChirpWSPR && chirpBtn->btnState() == AeroButton::ON)
		m_chirpWidget->hide();

	if (mode == QSDR::ChirpWSPR && ui->wideBandBtn->btnState() == AeroButton::ON) {

		//set->setWideBandStatus(false);
		ui->wideBandBtn->setBtnState(AeroButton::OFF);
		ui->wideBandBtn->setEnabled(false);
	}

	chirpBtn->setEnabled(QSDR::ChirpWSPR == mode);
    */

	setSystemState(QSDR::NoError, m_hwInterface, mode, m_dataEngineState);
}

/*!
	\brief get all network interfaces and IP addresses.
*/
void MainWindow::getNetworkInterfaces() {

	// find out which IP to connect to
	QString localIP = QHostAddress(QHostAddress::LocalHost).toString();

	// ip addresses from ini file
	QString serverIpAddress = set->getServerAddr();
	QString hpsdrDeviceLocalIpAddress = set->getHPSDRDeviceLocalAddr();

	MAIN_DEBUG << "server ip from ini-file: " << serverIpAddress;
	MAIN_DEBUG << "HPSDR device local ip from ini-file: " << hpsdrDeviceLocalIpAddress;
	
	QList<QHostAddress> broadcastAddresses;
	QList<QHostAddress> ipAddresses;
	QList<QNetworkInterface> nics;

	m_ipList = QNetworkInterface::allAddresses();
	m_niList = QNetworkInterface::allInterfaces();
	
	foreach (QNetworkInterface ni, m_niList) {

		//MAIN_DEBUG << "network interface " << ni.humanReadableName() << " :";
		//foreach (QNetworkAddressEntry entry, ni.addressEntries()) {

		//	QHostAddress broadcastAddress = entry.broadcast();
		//	MAIN_DEBUG << "  -> broadcast address: " << qPrintable(broadcastAddress.toString());
		//	//if (broadcastAddress != QHostAddress::Null && entry.ip() != QHostAddress::LocalHost) {
		//	if (entry.ip().toIPv4Address() && entry.ip() != QHostAddress::LocalHost) {
		//	//if (entry.ip() != QHostAddress::LocalHost) {
		//		broadcastAddresses << broadcastAddress;
		//		ipAddresses << entry.ip();
		//		nics << ni;
		//		MAIN_DEBUG << "  -> ip address: " << qPrintable(entry.ip().toString());
		//		set->m_ipAddressesList.append(entry.ip());
		//		set->m_networkInterfaces.append(ni);
		//
		//		set->addServerNetworkInterface(ni.humanReadableName(), entry.ip().toString());
		//		set->addHPSDRDeviceNIC(ni.humanReadableName(), entry.ip().toString());
		//	}
		//}

		foreach (QNetworkAddressEntry entry, ni.addressEntries()) {

			QHostAddress broadcastAddress = entry.broadcast();
			
			//if (broadcastAddress != QHostAddress::Null && entry.ip() != QHostAddress::LocalHost) {
			if (entry.ip().toIPv4Address() && entry.ip() != QHostAddress::LocalHost) {
			//if (entry.ip() != QHostAddress::LocalHost) {

				MAIN_DEBUG << "network interface " << ni.humanReadableName() << " :";
				MAIN_DEBUG << "  -> broadcast address: " << qPrintable(broadcastAddress.toString());
				broadcastAddresses << broadcastAddress;
				ipAddresses << entry.ip();
				nics << ni;
				MAIN_DEBUG << "  -> ip address: " << qPrintable(entry.ip().toString());
				set->m_ipAddressesList.append(entry.ip());
				set->m_networkInterfaces.append(ni);
				
				set->addServerNetworkInterface(ni.humanReadableName(), entry.ip().toString());
				set->addHPSDRDeviceNIC(ni.humanReadableName(), entry.ip().toString());
			}
		}
	}

	set->setNumberOfNetworkInterfaces(nics.size());
	MAIN_DEBUG << "found " << nics.size() << " valid ip addresses.";

	//emit serverWidgetEvent();
	//emit metisWidgetEvent();

	int serverIdx = -1;
	int metisIdx = -1;

	if (ipAddresses.isEmpty()) {

		// if we did not find one, use IPv4 localhost
		set->setServerAddr(localIP);
		set->setHPSDRDeviceLocalAddr(localIP);
	}
	else {

		//int serverIdx = -1;
		//int metisIdx = -1;
		
		for (int i = 0; i < nics.size(); ++i) {
			if (broadcastAddresses.at(i) != QHostAddress::Null) {

				if (ipAddresses.at(i).toString() == serverIpAddress) {

					set->setServerWidgetNIC(i);
					serverIdx = i;
				}
				if (ipAddresses.at(i).toString() == hpsdrDeviceLocalIpAddress) {

					set->setHPSDRWidgetNIC(i);
					metisIdx = i;
				}
			}
		}
	}

	QString message;
	for (int i = 0; i < nics.size(); ++i) {
		if (broadcastAddresses.at(i) != QHostAddress::Null) {
				
			if (serverIdx < 0) {
				set->setServerWidgetNIC(i);
				set->setServerAddr(ipAddresses.at(i).toString());
			}
			else {

				/*message = "[server]: network interface set to: %1 (%2).";
				showMessage(
					message.arg(
						nics.at(serverIdx).humanReadableName(),
						ipAddresses.at(serverIdx).toString() ));*/
			}

			if (metisIdx < 0) {

				set->setHPSDRWidgetNIC(i);
				set->setHPSDRDeviceLocalAddr(ipAddresses.at(i).toString());

				MAIN_DEBUG 	<< "HPSDR network device interface set to: "
							<< nics.at(i).humanReadableName()
							<< "(" << ipAddresses.at(i).toString() << ")";
			}
			else {

				MAIN_DEBUG 	<< "HPSDR network device interface set to: "
							<< nics.at(metisIdx).humanReadableName()
							<< "(" << ipAddresses.at(metisIdx).toString() << ")";
			}
		}

		/*for (int i = 0; i < nics.size(); ++i) {
			if (serverIdx != i) {
				set->setServerWidgetNIC(i);
				break;
			}
		}

		for (int i = 0; i < nics.size(); ++i) {
			if (metisIdx != i) {
				set->setHPSDRWidgetNIC(i);
				break;
			}
		
		}*/
	}

	//emit serverWidgetEvent();
	//emit metisWidgetEvent();

	//set->setServerWidgetNIC(1);
	MAIN_DEBUG << "using ip address " << qPrintable(set->getServerAddr()) << " for the server.";
	MAIN_DEBUG << "using ip address " << qPrintable(set->getHPSDRDeviceLocalAddr()) << " for connecting to a HPSDR device.";

	
}


 
/*!
	\brief suspend playing wav-file.
*/
void MainWindow::suspendSignal(
		/*!<[in] the of the event. */
) {
	m_dataEngine->suspend();
}


void MainWindow::showNetworkIODialog() {
	// Formerly this re-ran searchDevices(), which deadlocked Start when >1
	// HPSDR device was found (findHPSDRDevices held networkIOMutex while this
	// slot re-entered searchHpsdrNetworkDevices). Device picking belongs in the
	// Network panel / DeviceSelectionDialog before Start.
	MAIN_DEBUG << "multiple HPSDR devices present; select one in Network settings before Start";
}

void MainWindow::showWarningDialog(const QString &str) {

	m_warningDialog->setWarningMessage(str);
	m_warningDialog->exec();
}

/*!
	\brief generates an initial message for the logging widget-
*/
//void MainWindow::initialMessage() {
//
//	QString str = set->versionStr();
//	str.prepend("cuSDR ");
//	str.append(". \nOpenGL enabled HPSDR Front end \n(C) 2010-2012 Hermann von Hasseln, DL3HVH.\n");
//	m_msgBrowser->appendPlainText(str);
//	m_msgBrowser->appendPlainText("");
//}
 
//*******************************************************************************
// Application's window stuff

/*!
	\brief get the QRegion of the window borders.
*/
void MainWindow::getRegion() {

	/*m_topBorderFrame    = QRegion(QRect(8, 0, width() - 16, 2));
	m_topFrame 			= QRegion(QRect(8, 2, width() - 16, 3));
	m_leftFrame 		= QRegion(QRect(0, 8, 8, height() - 16));
	m_rightFrame 		= QRegion(QRect(width() - 8, 8, 8, height() - 16));
	m_bottomFrame 		= QRegion(QRect(8, height() - 8, width() - 16, 8));
	m_topLeftFrame 		= QRegion(QRect(0, 0, 8, 8));
	m_bottomLeftFrame	= QRegion(QRect(0, height() - 8, 8, 8));
	m_topRightFrame 	= QRegion(QRect(width() - 8, 0, 8, 8));
	m_bottomRightFrame	= QRegion(QRect(width() - 8, height() - 8, 8, 8));*/
}
 
/*!
	\brief mouse wheel event implementation.
*/
void MainWindow::wheelEvent(
		QWheelEvent *event			/*!<[in] event */
) {
	event->accept();
	QWidget::wheelEvent(event);
}
 
/*!
	\brief enter event implementation.
*/
void MainWindow::enterEvent(QEnterEvent *event) {
	Q_UNUSED(event);
}
 
/*!
	\brief leave event implementation.
*/
void MainWindow::leaveEvent(QEvent *event) {
	Q_UNUSED(event);
}
 
/*!
	\brief resize event implementation.
*/
void MainWindow::resizeEvent(
		QResizeEvent *event			/*!<[in] event */
) {
	//Q_UNUSED(event);
	
    //QElapsedTimerr::singleShot(10, this, SLOT(getRegion()));
	//m_resizeFrame = true;
	//ui->m_displayPanelToolBar->updateGeometry();
    ui->m_oglDisplayPanel->update();
	QWidget::resizeEvent(event);
}
 
/*!
	\brief close event implementation.
*/
void MainWindow::closeEvent(
		QCloseEvent *event			/*!<[in] event */
) {
	if (m_shuttingDown) {
		QMainWindow::closeEvent(event);
		return;
	}
	m_shuttingDown = true;

	// Always stop the radio before tearing down OpenGL / engine objects.
	ensureEngineStoppedForShutdown();

	if (TciServer *tci = set->tciServer())
		tci->stopListening();

	// Persist last VFO/center frequencies even if the user never toggled main power.
	set->saveSettings();

	QSettings settings(QCoreApplication::applicationDirPath() +  "/" + m_windowsSettingsFilename, QSettings::IniFormat);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());

	ui->mainBtnList.clear();

	if (m_serverWidget) {

		disconnect(m_serverWidget, 0, 0, 0);
		delete m_serverWidget;
		m_serverWidget = NULL;
	}

	if (m_dataEngine) {

		disconnect(m_dataEngine, 0, 0, 0);
		// Belt-and-braces: stop again if state raced past the earlier check.
		if (m_dataEngineState == QSDR::DataEngineUp)
			m_dataEngine->stop();
		delete m_dataEngine;
		m_dataEngine = NULL;
	}

	if (ui->m_oglDisplayPanel) {
		
        disconnect(ui->m_oglDisplayPanel, 0, 0, 0);
        delete ui->m_oglDisplayPanel;
		ui->m_oglDisplayPanel = NULL;
	}

	/*if (m_hpsdrWidget) {
		
		disconnect(m_hpsdrWidget, 0, 0, 0);
		delete m_hpsdrWidget;
		m_hpsdrWidget = NULL;
	}*/

	if (m_hpsdrTabWidget) {
		
		disconnect(m_hpsdrTabWidget, 0, 0, 0);
		delete m_hpsdrTabWidget;
		m_hpsdrTabWidget = NULL;
	}


	if (m_displayTabWidget) {
        qDebug() << "tab widget delete";
        delete m_displayTabWidget;
        m_displayTabWidget = NULL;
	}

	/*if (m_hpsdrServer) {

		disconnect(m_hpsdrServer, 0, 0, 0);
		delete m_hpsdrServer;
		m_hpsdrServer = NULL;
	}*/
	QMainWindow::closeEvent(event);
}
 
/*!
	\brief show event implementation.
*/
void MainWindow::showEvent(
		QShowEvent *event			/*!<[in] event */
) {
	QWidget::showEvent(event);
}
 
/*!
	\brief key pressed event implementation.
*/
void MainWindow::keyPressEvent(
		QKeyEvent *event			/*!<[in] key event */
) {
	switch (event->key()) {

		case Qt::Key_Escape:

			setMainWindowGeometry();
			return;

		case Qt::Key_1:

			return;
    }
    
    QWidget::keyPressEvent(event);
}


void MainWindow::radioStateChange(RadioState state) {

  qDebug() << "Radio State Change" << state;
    ui->moxBtn->setBtnState((AeroButton::OFF));
    ui->tunBtn->setBtnState(AeroButton::OFF);


    switch (state){
        case RadioState::RX:
        ui->moxBtn->setBtnState(AeroButton::OFF);
        ui->tunBtn->setBtnState(AeroButton::OFF);
        break;
        case RadioState::MOX:
            ui->moxBtn->setBtnState(AeroButton::ON);
            ui->tunBtn->setBtnState(AeroButton::OFF);
        break;
        case RadioState::TUNE:
            ui->tunBtn->setBtnState(AeroButton::ON);
            ui->moxBtn->setBtnState(AeroButton::OFF);

            break;
        default:
        break;
    }
    ui->tunBtn->repaint();
    ui->moxBtn->repaint();

}

void MainWindow::moxBtnClickedEvent() {
    if (set->getRadioState() == RadioState::MOX)
    {
        set->setRadioState(RadioState::RX);
    }
    else set->setRadioState(RadioState::MOX);
}

void MainWindow::tunBtnClickedEvent() {
    if (set->getRadioState() == RadioState::TUNE)
    {
        set->setRadioState(RadioState::RX);
    }
    else set->setRadioState(RadioState::TUNE);
}


//***************************************************************************
// NetworkIODialog class

//***************************************************************************
// WarningDialog class

WarningDialog::WarningDialog(QWidget *parent)
    :   QDialog(parent)
	,	set(Settings::instance())
	,	m_btnWidth(74)
	,	m_btnHeight(18)
{
	setWindowModality(Qt::NonModal);
	setWindowTitle("Warning");
	setWindowOpacity(0.9);
//	setStyleSheet(set->getDialogStyle());

	setMouseTracking(true);

	m_titleFont.setStyleStrategy(QFont::PreferAntialias);
	m_titleFont.setFixedPitch(true);
	m_titleFont.setBold(true);
	m_titleFont.setPixelSize(13);
	m_titleFont.setFamily("Arial");
	m_titleFont.setBold(true);
	
	//m_warningIcon.QPixmap::fromImage(QImage(QLatin1String(":/img/warning.png")), Qt::ColorOnly);

	m_warningLabel = new QLabel("", this);
		
	okBtn = new AeroButton("Ok", this);
	okBtn->setRoundness(10);
	okBtn->setFixedSize(m_btnWidth, m_btnHeight);

	CHECKED_CONNECT(
		okBtn, 
		SIGNAL(clicked()), 
		this, 
		SLOT(okBtnClicked()));
}

WarningDialog::~WarningDialog() {
}

void WarningDialog::paintEvent(QPaintEvent *) {

	QPainter p(this);
	p.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing | QPainter::TextAntialiasing, true);

	//QRect titlebar_rect(0, 0, width(), height());

	/*QLinearGradient titlebarGrad(0, 0, 0, 1);
	titlebarGrad.setCoordinateMode(QGradient::ObjectBoundingMode);
	titlebarGrad.setSpread(QGradient::PadSpread);
	titlebarGrad.setColorAt(0, QColor(110, 110, 110));
	titlebarGrad.setColorAt(0.45, QColor(80, 80, 80));
	titlebarGrad.setColorAt(0.55, QColor(56, 56, 65));
	titlebarGrad.setColorAt(1, QColor(40, 40, 40));*/

	// draw background rect
	/*p.setPen(Qt::NoPen);
	p.setBrush(QBrush(titlebarGrad));
	p.drawRect(titlebar_rect);
	p.setPen(QColor(255, 255, 255, 140));
	p.drawLine(1, titlebar_rect.top(), width() - 2, titlebar_rect.top());
	p.setPen(QColor(255, 255, 255, 30));
	p.drawLine(1, titlebar_rect.bottom() - 2, width() - 2, titlebar_rect.bottom() - 2);
	p.setPen(QColor(0, 0, 0, 255));
	p.drawLine(0, titlebar_rect.bottom(), width(), titlebar_rect.bottom());*/

	QPixmap warningIcon = QPixmap::fromImage(QImage(QLatin1String(":/img/warning.png")), Qt::ColorOnly);
	if (!warningIcon.isNull()) p.drawPixmap(13, 5, 16, 16, warningIcon);
		
	// draw text
	p.setFont(m_titleFont);
	p.setPen(QColor(95, 95, 95, 255));

	// warning msg
	p.drawText(
		40, 6, 
		m_msgFontWidth, m_msgFontHeight, 
		Qt::TextSingleLine | Qt::TextDontClip | Qt::AlignVCenter | Qt::AlignLeft, 
		m_message);

	p.setPen(QColor(235, 235, 235, 255));
	p.drawText(
		39, 5, 
		m_msgFontWidth, m_msgFontHeight, 
		Qt::TextSingleLine | Qt::TextDontClip | Qt::AlignVCenter | Qt::AlignLeft, 
		m_message);

	okBtn->move((width() - m_btnWidth)/2, 30);

	p.end();
}

void WarningDialog::setWarningMessage(const QString &msg) {

	m_message = msg;
	
	QFontMetrics tfm(m_titleFont);
    m_msgFontWidth = tfm.horizontalAdvance(m_message);
	m_msgFontHeight = tfm.height();

	this->setFixedWidth(m_msgFontWidth + 60);
	this->setFixedHeight(60);
}

void WarningDialog::okBtnClicked() {

        accept();
}

void MainWindow::handleDeviceListChanged(const QList<TNetworkDevicecard> &list) {
    if (set->getMainPower()) return;

    for (const TNetworkDevicecard &card : list) {
        QVariant v = QVariant::fromValue(card);
        bool found = false;
        for (const QVariant &existing : m_discoveredDevices) {
            if (existing.canConvert<TNetworkDevicecard>()) {
                if (sameHpsdrDeviceByMac(existing.value<TNetworkDevicecard>(), card)) {
                    found = true;
                    break;
                }
            }
        }
        if (!found) m_discoveredDevices.append(v);
    }
}

#ifdef HAVE_SOAPYSDR
void MainWindow::handleSoapyDeviceListChanged(const QList<TSoapyDevice> &list) {
    if (set->getMainPower()) return;

    for (const TSoapyDevice &dev : list) {
        QVariant v = QVariant::fromValue(dev);
        bool found = false;
        for (const QVariant &existing : m_discoveredDevices) {
            if (existing.canConvert<TSoapyDevice>()) {
                if (sameSoapyDevice(existing.value<TSoapyDevice>(), dev)) {
                    found = true;
                    break;
                }
            }
        }
        if (!found) m_discoveredDevices.append(v);
    }
}
#endif

void MainWindow::processDiscoveryResults() {
    if (set->getMainPower()) return;
    if (m_discoveredDevices.isEmpty()) return;

    TSDRDevice lastDevice = set->getLastConnectedDevice();

    // If more than one device is found, OR if this is a manual search, show the selection dialog.
    // If exactly one device is found during startup, auto-select it if it matches the last used hardware.

    bool showDialog = (m_discoveredDevices.size() > 1) || !m_isStartupDiscovery;

    if (m_isStartupDiscovery && m_discoveredDevices.size() > 1) {
        const QVariant matched = findLastConnectedMatch(m_discoveredDevices, lastDevice);
        if (matched.isValid()) {
            if (matched.canConvert<TNetworkDevicecard>()) {
                set->setCurrentHPSDRDevice(matched.value<TNetworkDevicecard>());
            }
#ifdef HAVE_SOAPYSDR
            else if (matched.canConvert<TSoapyDevice>()) {
                set->setCurrentSoapyDevice(matched.value<TSoapyDevice>());
            }
#endif
            showDialog = false;
        }
    }

    if (m_discoveredDevices.size() == 1 && m_isStartupDiscovery) {
        QVariant dev = m_discoveredDevices.first();
        if (dev.canConvert<TNetworkDevicecard>()) {
            TNetworkDevicecard card = dev.value<TNetworkDevicecard>();
            if (lastDevice.deviceClass == DeviceClass_HPSDR && lastDevice.serialNumber == QString::fromLatin1(card.mac_address)) {
                set->setCurrentHPSDRDevice(card);
                showDialog = false;
            } else if (lastDevice.deviceClass == DeviceClass_None) {
                set->setCurrentHPSDRDevice(card);
                showDialog = false;
            }
        }
#ifdef HAVE_SOAPYSDR
        else if (dev.canConvert<TSoapyDevice>()) {
            TSoapyDevice soapy = dev.value<TSoapyDevice>();
            if (lastDevice.deviceClass == DeviceClass_SoapySDR && lastDevice.serialNumber == soapy.serial && lastDevice.deviceType == soapy.driver) {
                set->setCurrentSoapyDevice(soapy);
                showDialog = false;
            } else if (lastDevice.deviceClass == DeviceClass_None) {
                set->setCurrentSoapyDevice(soapy);
                showDialog = false;
            }
        }
#endif
    }

    if (showDialog) {
        DeviceSelectionDialog dlg(m_discoveredDevices, this);
        if (dlg.exec() == QDialog::Accepted) {
            QVariant selected = dlg.selectedDevice();
            if (selected.canConvert<TNetworkDevicecard>()) {
                set->setCurrentHPSDRDevice(selected.value<TNetworkDevicecard>());
            }
#ifdef HAVE_SOAPYSDR
            else if (selected.canConvert<TSoapyDevice>()) {
                set->setCurrentSoapyDevice(selected.value<TSoapyDevice>());
            }
#endif
        }
    }

    m_isStartupDiscovery = false;
    m_discoveredDevices.clear();
}

void MainWindow::clearDiscoveredDevices() {
    m_discoveredDevices.clear();
    m_discoveryTimer.stop();
    m_discoveryTimer.start(2500);
}


