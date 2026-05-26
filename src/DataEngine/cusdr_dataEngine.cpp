#include "Models/RadioModel.h"
#include "Models/RadioTelemetry.h"
#include "Models/SliceModel.h"
#if defined(__clang__)
#pragma clang diagnostic push
#endif
/**
* @file  cusdr_dataEngine.cpp
* @brief cuSDR data engine class
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2011-02-02
*/

/*
 *   
 *   Copyright 2010 Hermann von Hasseln, DL3HVH
 *
 *	 using original C code by John Melton, G0ORX/N6LYT and Dave McQuate, WA8YWQ
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

//#define TESTING
extern double cwramp48[];		// see cwramp.c, for 48 kHz sample rate

#define LOG_DATA_ENGINE
// use DATA_ENGINE_DEBUG
//#define LOG_DATA_PROCESSOR
// use DATA_PROCESSOR_DEBUG
#define LOG_AUDIO_PROCESSOR
// use AUDIO_PROCESSOR
#define LOG_WIDEBAND_PROCESSOR
 //use WIDEBAND_PROCESSOR_DEBUG
#define RAMPLEN 250
#include "cusdr_dataEngine.h"
#include "Controllers/RadioController.h"
#include "SoapySDRDataSource.h"
#include "CProtocol1.h"
#include "CProtocol2.h"
#ifdef HAVE_CODEC2
extern "C" {
#define COMP FREEDV_COMP
#include <codec2/freedv_api.h>
#undef COMP
}
#endif


/*!
	\class DataEngine
	\brief The DataEngine class implements the main SDR functionality.
*/
/*!
	\brief Implements interfaces to the HPSDR hardware and various Server and DSP functionality.
	- set up HW interfaces to Metis or other resp.
	- initializes Metis.
	- set up parameters for HPSDR hardware.
	- implements the data receiver thread.
	- implements the data processor thread.
	- implements the wide band data processor thread.
	- implements the audio receiver thread.
	- implements the audio processor thread.
*/

DataEngine::DataEngine(RadioModel *model, QObject *parent)
	: QObject(parent)
	, set(Settings::instance())
        , m_radioModel(model)
    , m_protocol(nullptr)
    , m_internal_cw(set->isInternalCw())
	, m_cw_key_reversed(set->isCwKeyReversed())
    , m_cw_keyer_spacing(set->getCwKeyerSpacing())
	, m_cw_keyer_speed(set->getCwKeyerSpeed())
	, m_cw_keyer_mode(set->getCwKeyerMode())
    , m_cw_keyer_weight(set->getCwKeyerWeight())
	, m_cw_sidetone_volume(set->getCwSidetoneVolume())
	, m_cw_ptt_delay(set->getCwPttDelay())
	, m_cw_hang_time(set->getCwHangTime())
	, m_cw_sidetone_freq(set->getCwSidetoneFreq())
    , m_radioState(RadioState::RX)
	, m_serverMode(set->getCurrentServerMode())
	, m_hwInterface(set->getHWInterface())
	, m_dataEngineState(QSDR::DataEngineDown)
	, m_meterType(SIGNAL_STRENGTH)
	, m_restart(false)
	, m_networkDeviceRunning(false)
	, m_soundFileLoaded(false)
	//, m_wbSpectrumAveraging(set->getSpectrumAveraging())
	//, m_wbSpectrumAveraging(true)
    , m_discoveryThreadRunning(false)
	, m_dataIOThreadRunning(false)
    , m_dataProcThreadRunning(false)
	, m_audioRcvrThreadRunning(false)
	, m_audioInProcThreadRunning(false)
    , m_audioOutProcThreadRunning(false)
	, m_frequencyChange(false)
	, m_hamBandChanged(true)
	, m_hpsdrDevices(0)
	, m_configure(10)
    , m_timeout(5000)
    , m_remainingTime(0)
    , m_RxFrequencyChange(0)//(35.0f)
    , m_forwardPower(0)
    , m_rxSamples(0)
    , m_spectrumSize(set->getSpectrumSize())
    , m_sendState(0)
    , m_sMeterCalibrationOffset(0.0f)



{
	qRegisterMetaType<QAbstractSocket::SocketError>();

	this->setObjectName(QString::fromUtf8("dataEngine"));

	m_clientConnected = false;

	//currentRx = 0;
	m_discoverer= nullptr;
	m_dataIO= nullptr;
#ifdef HAVE_SOAPYSDR
	m_soapySDRSource = nullptr;
#endif
	m_dataProcessor= nullptr;
	m_wbDataProcessor= nullptr;
	m_audioReceiver= nullptr;
	m_audioOutProcessor= nullptr;
    m_audioInput= nullptr;
    m_cwIO = nullptr;
	//m_wbAverager= nullptr;
	set->setMercuryVersion(0);
	set->setPenelopeVersion(0);

	set->setPennyLaneVersion(0);
	set->setMetisVersion(0);
	set->setHermesVersion(0);
    setupConnections();

	io.metisFW = 0;
	io.hermesFW = 0;
	io.mercuryFW = 0;
    io.ccTx.use_repeaterOffset = set->get_repeaterMode();

    //m_audioBuffer.resize(0);
    //m_audiobuf.resize(IO_BUFFER_SIZE);

	m_counter = 0;

}

DataEngine::~DataEngine() {
    // m_protocol is a unique_ptr — destroyed automatically
    // Add socket cleanup
    if (sendSocket) {
        delete sendSocket;
        sendSocket = nullptr;
    }
    if (m_controlSocket) {
        delete m_controlSocket;
        m_controlSocket = nullptr;
    }

   // file->close();
    if (m_audioInput)
        delete m_audioInput;

#ifdef HAVE_SOAPYSDR
    if (m_soapySDRSource) {
        delete m_soapySDRSource;
        m_soapySDRSource = nullptr;
    }
#endif
}

void DataEngine::setupConnections() {

	if (m_radioModel && m_radioModel->telemetry()) {
		connect(this, &DataEngine::rcveIQEvent,
		        m_radioModel->telemetry(), &RadioTelemetry::setRcveIQ);
	}

	CHECKED_CONNECT(
		set,
		&Settings::systemStateChanged,
		this,
		&DataEngine::systemStateChanged);

	CHECKED_CONNECT(
		set, 
		&Settings::rxListChanged,
		this,
		&DataEngine::rxListChanged);

	CHECKED_CONNECT(
		set, 
		&Settings::numberOfRXChanged,
		this, 
		&DataEngine::setNumberOfRx);

	CHECKED_CONNECT(
		set, 
		&Settings::currentReceiverChanged,
		this, 
		&DataEngine::setCurrentReceiver);

	CHECKED_CONNECT(
		set,
		&Settings::hamBandChanged,
		this,
		&DataEngine::setHamBand);

	CHECKED_CONNECT(
		set,
		&Settings::sampleRateChanged, 
		this, 
		&DataEngine::setSampleRate);

	CHECKED_CONNECT(
		set, 
		&Settings::mercuryAttenuatorChanged,
		this, 
		&DataEngine::setMercuryAttenuator);

	CHECKED_CONNECT(
		set,
		&Settings::ditherChanged,
		this, 
        &DataEngine::setDither);

	CHECKED_CONNECT(
		set, 
		&Settings::randomChanged,
		this, 
		&DataEngine::setRandom);

	CHECKED_CONNECT(
		set, 
		&Settings::src10MhzChanged, 
		this, 
		&DataEngine::set10MhzSource);

	CHECKED_CONNECT(
		set, 
		&Settings::src122_88MhzChanged, 
		this, 
		&DataEngine::set122_88MhzSource);

	CHECKED_CONNECT(
		set, 
        &Settings::micSourceChanged,
		this, 
        &DataEngine::setMicSource);

	CHECKED_CONNECT(
		set, 
		&Settings::classChanged, 
		this, 
		&DataEngine::setMercuryClass);

	CHECKED_CONNECT(
		set, 
		&Settings::timingChanged, 
		this, 
		&DataEngine::setMercuryTiming);

	CHECKED_CONNECT(
		set, 
		&Settings::clientDisconnectedEvent, 
		this, 
		&DataEngine::setClientDisconnected);

	CHECKED_CONNECT(
		set, 
		&Settings::clientNoConnectedChanged, 
		this, 
		(qOverload<int>(&DataEngine::setClientConnected)));

	CHECKED_CONNECT(
		set, 
		&Settings::rxConnectedStatusChanged, 
		this, 
		&DataEngine::setRxConnectedStatus);

	CHECKED_CONNECT(
		set, 
		&Settings::audioRxChanged, 
		this, 
		&DataEngine::setAudioReceiver);

	CHECKED_CONNECT(
		set, 
		&Settings::framesPerSecondChanged,
		this, 
		&DataEngine::setFramesPerSecond);

	CHECKED_CONNECT(
		set,
		&Settings::searchMetisSignal,
		this,
		&DataEngine::searchHpsdrNetworkDevices);

#ifdef HAVE_SOAPYSDR
	CHECKED_CONNECT(
	set,
	&Settings::searchSoapySignal,
	this,
	&DataEngine::searchSoapyDevices);
#endif

	CHECKED_CONNECT(
		set, 
		&Settings::networkDeviceNumberChanged, 
		this, 
		&DataEngine::setHPSDRDeviceNumber);

	CHECKED_CONNECT(
		set,
		&Settings::alexConfigurationChanged,
		this,
		&DataEngine::setAlexConfiguration);

	CHECKED_CONNECT(
		set,
		&Settings::alexStateChanged,
		this,
		&DataEngine::setAlexStates);

	CHECKED_CONNECT(
		set,
		&Settings::pennyOCEnabledChanged,
		this,
		&DataEngine::setPennyOCEnabled);

	CHECKED_CONNECT(
		set,
		&Settings::rxJ6PinsChanged,
		this,
		&DataEngine::setRxJ6Pins);

	CHECKED_CONNECT(
		set,
		&Settings::txJ6PinsChanged,
		this,
		&DataEngine::setTxJ6Pins);

    CHECKED_CONNECT(
            set,
            &Settings::radioStateChanged,
            this,
            &DataEngine::radioStateChange);

    CHECKED_CONNECT(
            set,
            &Settings::driveLevelChanged,
			this,
			&DataEngine::set_tx_drivelevel);

    CHECKED_CONNECT(
            set,
            &Settings::repeaterModeChanged,
            this,
            &DataEngine::setRepeaterMode);


    // connect(set, &Settings::dspModeChanged, this, &DataEngine::dspModeChanged);


    CHECKED_CONNECT(
            set,
            &Settings::CwHangTimeChanged,
            this,
            &DataEngine::CwHangTimeChanged);

    CHECKED_CONNECT(
            set,
            &Settings::CwSidetoneFreqChanged,
            this,
            &DataEngine::CwSidetoneFreqChanged);


    connect(set, &Settings::CwKeyReversedChanged, this, &DataEngine::CwKeyReversedChanged);

    CHECKED_CONNECT(
            set,
            &Settings::CwKeyerModeChanged,
            this,
            &DataEngine::CwKeyerModeChanged);

    CHECKED_CONNECT(
               set,
               &Settings::InternalCwChanged,
               this,
               &DataEngine::InternalCwChanged);

    CHECKED_CONNECT(
               set,
               &Settings::CwKeyerSpeedChanged,
               this,
               &DataEngine::CwKeyerSpeedChanged);


    CHECKED_CONNECT(
               set,
               &Settings::CwPttDelayChanged,
               this,
               &DataEngine::CwPttDelayChanged);

    CHECKED_CONNECT(
               set,
               &Settings::CwSidetoneVolumeChanged,
               this,
               &DataEngine::CwSidetoneVolumeChanged);

    CHECKED_CONNECT(
            set,
            &Settings::CwKeyerWeightChanged,
            this,
            &DataEngine::CwKeyerWeightChanged);

    CHECKED_CONNECT(
            set,
            &Settings::CwKeyerSpacingChanged,
                    this,
            &DataEngine::CwKeyerSpacingChanged);


}

//********************************************************
// start/stop data engine
bool DataEngine::startDataEngineWithoutConnection() {

	DATA_ENGINE_DEBUG << "no HPSDR-HW interface";

	if (io.inputBuffer.length() > 0) {

        initReceivers(1);
		if (!m_dataIO)	createDataIO();
		if (!m_dataProcessor)	createDataProcessor();



		// data receiver thread
		if (!startDataIO(QThread::HighPriority)) {

			setSystemState(QSDR::DataReceiverThreadError, m_hwInterface, m_serverMode, QSDR::DataEngineDown);
			return false;
		}

				// IQ data processing thread
		if (!startDataProcessor(QThread::HighPriority)) {

			setSystemState(QSDR::DataProcessThreadError, m_hwInterface, m_serverMode, QSDR::DataEngineDown);
			return false;
		}
		setSystemState(QSDR::NoError, m_hwInterface, m_serverMode, QSDR::DataEngineUp);
        set->setRadioState(RadioState ::RX);
		return true;
	}
	else {

		DATA_ENGINE_DEBUG << "no data available - data file loaded?";
		return false;
	}
}

bool DataEngine::findHPSDRDevices() {

	if (!m_discoverer) createDiscoverer();

	// HPSDR network IO thread
	if (!startDiscoverer(QThread::NormalPriority)) {

		io.networkIOMutex.lock();
		DATA_ENGINE_DEBUG << "HPSDR device discovery thread could not be started.";
		io.networkIOMutex.unlock();
		return false;
	}

    // Invoke discovery on the discoverer thread (moved to thread via moveToThread,
    // so a queued invocation is required — a direct call would run on the wrong thread).
    QMetaObject::invokeMethod(m_discoverer, "initHPSDRDevice", Qt::QueuedConnection);

	io.networkIOMutex.lock();
	DATA_ENGINE_DEBUG << "HPSDR network device detection...please wait.";
	set->setSystemMessage("HPSDR network device detection...please wait", 0);
	io.devicefound.wait(&io.networkIOMutex);

	m_hpsdrDevices = set->getHpsdrNetworkDevices();
	if (m_hpsdrDevices == 0) {

		io.networkIOMutex.unlock();
		stopDiscoverer();
		DATA_ENGINE_DEBUG << "no device found. HPSDR hardware powered? Network connection established?";
		set->setSystemMessage("no device found. HPSDR hardware powered? Network connection established?", 10000);

		setSystemState(QSDR::HwIOError,	m_hwInterface, m_serverMode, QSDR::DataEngineDown);
	}
	else {

		emit clearSystemMessageEvent();
		if (m_hpsdrDevices > 1)
			set->showNetworkIODialog();

		QList<TNetworkDevicecard> metisList = set->getMetisCardsList();
		DATA_ENGINE_DEBUG << "found " << metisList.count() << " network device(s)";
				
		for (int i = 0; i < metisList.count(); i++) {

			DATA_ENGINE_DEBUG 	<< "Device "
								<< i << " @ "
								<< qPrintable(metisList.at(i).ip_address.toString())
								//<< " [" << qPrintable((char *) &metisList.at(i).mac_address) << "]";
								<< " [" << metisList.at(i).mac_address << "]";
		}

		io.hpsdrDeviceIPAddress = set->getCurrentMetisCard().ip_address;
		io.hpsdrDeviceName = set->getCurrentMetisCard().boardName;
		DATA_ENGINE_DEBUG << "using HPSDR network device at " << qPrintable(io.hpsdrDeviceIPAddress.toString());

		
		QThread::msleep(100);

		// stop the discovery thread
		io.networkIOMutex.unlock();
		stopDiscoverer();

		if (getFirmwareVersions()) return true;
		return false;
	}

	return false;
}

bool DataEngine::getFirmwareVersions() {

	m_fwCount = 0;

	// Create the protocol object now, before DataIO starts, so that
	// initDataReceiverSocket() binds the correct ports and readDeviceData()
	// doesn't drop packets with a null protocol check.
	if (m_hwInterface == QSDR::Metis || m_hwInterface == QSDR::Hermes) {
		if (set->getCurrentMetisCard().protocol == 2)
			m_protocol = std::make_unique<CProtocol2>();
		else
			m_protocol = std::make_unique<CProtocol1>();
	}
	io.protocol = m_protocol.get();

	// init receivers
	int rcvrs = set->getNumberOfReceivers();
	QString str = "Initializing %1 receiver(s)...please wait";
	set->setSystemMessage(str.arg(set->getNumberOfReceivers()), rcvrs * 500);
	if (!initReceivers(rcvrs)) return false;

	if (!m_dataIO) createDataIO();
		
	if (!m_dataProcessor) createDataProcessor();

	switch (m_serverMode) {

		case QSDR::SDRMode:
			
			for (int i = 0; i < set->getNumberOfReceivers(); i++) {

				RX.at(i)->setConnectedStatus(true);
			}
			break;
		default:

			DATA_ENGINE_DEBUG << "no valid server mode";
			setSystemState(QSDR::ServerModeError, m_hwInterface, m_serverMode, QSDR::DataEngineDown);

			return false;
	}

	connectDSPSlots();

//	for (int i = 0; i < set->getNumberOfReceivers(); i++)
//		RX.at(i)->setAudioVolume(i, set->getMainVolume());

	// IQ data processing thread
	if (!startDataProcessor(QThread::HighPriority)) {

		DATA_ENGINE_DEBUG << "data processor thread could not be started.";
		return false;
	}

	// data IO thread
	if (!startDataIO(QThread::HighPriority)) {//  ::NormalPriority)) {

		DATA_ENGINE_DEBUG << "data IO thread could not be started.";
		return false;
	}

	//setSampleRate(set->getSampleRate());
	QThread::msleep(100);

	// For Protocol 2: do NOT start the network device here.
	// DSP threads have not been started yet; start() is called by initDataEngine()
	// immediately after this returns and will launch DSP threads first, then
	// send the General Packet and Run=1.  Starting IQ streaming before DSP
	// threads are live causes a BlockingQueuedConnection deadlock in the
	// DataProcessor, producing persistent fexchange0 -2 errors on first start.
	// For Protocol 1: continue the existing flow (start device to collect FW
	// version response packets).
	if (set->getCurrentMetisCard().protocol != 2) {
		// pre-conditioning
		for (int i = 0; i < io.receivers; i++)
			m_dataIO->sendInitFramesToNetworkDevice(i);
		
		if (m_serverMode == QSDR::SDRMode)
			m_dataIO->networkDeviceStartStop(0x01);
		
		m_networkDeviceRunning = true;
		setSystemState(QSDR::NoError, m_hwInterface, m_serverMode, QSDR::DataEngineUp);
		QThread::msleep(300);
	}

    io.metisFW = set->getMetisVersion();
    io.mercuryFW = set->getMercuryVersion();
    io.penelopeFW = set->getPenelopeVersion();
    io.pennylaneFW = set->getPennyLaneVersion();
    io.hermesFW = set->getHermesVersion();
    io.ccTx.drivelevel = set->get_tx_drivelevel();
	if (set->getFirmwareVersionCheck())
		return checkFirmwareVersions();
	else
		return true;
}

// credits go to George Byrkit, K9TRV: the older FW checkings are shamelessly taken from the KISS Konsole!
// TODO(P2-FIRMWARE): All firmware checks below compare io.hpsdrDeviceName against
// the strings "Metis" and "Hermes".  Protocol 2 hardware reports board type as a
// numeric board ID in the discovery reply (byte 11), not a name string.  The name
// assigned to io.hpsdrDeviceName at line 544 comes from MetisCard::boardName which
// is populated during discovery and may not equal "Metis" or "Hermes" for newer P2
// boards ("Orion", "Orion2", "Angelia", etc.).  Until the board-name mapping for
// P2 boards is verified, these checks are likely silently skipped for P2 hardware.
bool DataEngine::checkFirmwareVersions() {

	if (io.metisFW != 0 &&  io.hpsdrDeviceName == "Hermes") {

		stop();

		QString msg = "Metis selected, but Hermes found!";
    //	set->showWarningDialog(msg);
		return false;
	}

	if (io.hermesFW != 0 && io.hpsdrDeviceName == "Metis") {

		stop();

		QString msg = "Hermes selected, but Metis found!";
		set->showWarningDialog(msg);
		return false;
	}

	if (io.penelopeFW == 0 && (set->getPenelopePresence() || set->getPennyLanePresence())) {

		stop();

		QString msg = "Penelope or Pennylane selected, but firmware version = 0 !";
		set->showWarningDialog(msg);
		return false;
	}

	if (io.mercuryFW < 27 && set->getNumberOfReceivers() > 4 && io.hpsdrDeviceName == "Metis") {

		stop();

		QString msg = "Mercury FW must be V2.7 or higher!";
		set->showWarningDialog(msg);
		return false;
	}

	if (io.hpsdrDeviceName == "Metis") {

		QString msg;
		switch (io.metisFW) {

			case 13:
				if (((set->getPenelopePresence() || set->getPennyLanePresence()) &&
					(io.penelopeFW == 13 || io.pennylaneFW == 13)) ||
					io.mercuryFW != 29)
				{
					stop();

					msg = "Penny[Lane] FW Version V1.3 and Mercury FW V2.7 requires Metis FW V1.3!";
					set->showWarningDialog(msg);
					return false;
				}
				break;

			case 14:
				if (((set->getPenelopePresence() || set->getPennyLanePresence()) &&
					(io.penelopeFW == 14 || io.pennylaneFW == 14)) ||
					io.mercuryFW != 29)
				{
					stop();

					msg = "Penny[Lane] FW Version V1.4 and Mercury FW V2.7 requires Metis FW V1.4!";
					set->showWarningDialog(msg);
					return false;
				}
				break;

			case 15:

				if (((set->getPenelopePresence() || set->getPennyLanePresence()) &&
					(io.penelopeFW == 15 || io.pennylaneFW == 15)) ||
					io.mercuryFW != 30)
				{
					stop();

					msg = "Penny[Lane] FW Version V1.5 and Mercury FW V3.0 requires Metis FW V1.5!";
					set->showWarningDialog(msg);
					return false;
				}
				break;

			case 16:

				if (((set->getPenelopePresence() || set->getPennyLanePresence()) &&
					(io.penelopeFW == 16 || io.pennylaneFW == 16)) ||
					io.mercuryFW != 31)
				{
					stop();

					msg = "Penny[Lane] FW Version V1.6 and Mercury FW V3.1 requires Metis FW V1.6!";
					set->showWarningDialog(msg);
					return false;
				}
				break;

			case 17:
			case 18:

				if (((set->getPenelopePresence() || set->getPennyLanePresence()) &&
					(io.penelopeFW == 17 || io.pennylaneFW == 17)) ||
					io.mercuryFW != 32)
				{
					stop();

					msg = "Penny[Lane] FW Version V1.7 and Mercury FW V3.2 requires Metis FW V1.7 or V1.8!";
					set->showWarningDialog(msg);
					return false;
				}
				break;

			case 19:
			case 20:

				stop();

				msg = "Metis FW V1.9 or V2.0 have some problems - please upgrade to Metis V2.1!";
				set->showWarningDialog(msg);
				return false;
				break;

			case 21:

				if ((set->getPenelopePresence() && io.penelopeFW != 17)	||
					(set->getPennyLanePresence() && io.pennylaneFW != 17)||
					io.mercuryFW != 33)
				{
					stop();

					msg = "Penny[Lane] FW Version V1.7 and Mercury FW V3.3 required for Metis FW V2.1!";
					set->showWarningDialog(msg);
					return false;
				}
				break;

//			case 22:
//
//				if ((set->getPenelopePresence() && m_penelopeFW != 17)	||
//					(set->getPennyLanePresence() && m_pennylaneFW != 17)||
//					m_mercuryFW != 33)
//				{
//					stop();
//
//					msg = "Penny[Lane] FW Version V1.7 and Mercury FW V3.3 required for Metis FW >= V2.1!";
//					set->showWarningDialog(msg);
//					return false;
//				}
//				break;

            case 26:

                if ((set->getPenelopePresence() && io.penelopeFW != 18)	||
                    (set->getPennyLanePresence() && io.pennylaneFW != 18)||
                    io.mercuryFW != 34)
                {
                    stop();

                    msg = "Penny[Lane] FW Version V1.8 and Mercury FW V3.4 required for Metis FW V2.6!";
                    set->showWarningDialog(msg);
                    return false;
                }
                break;

            default:

				//stop();

				msg = "Not a standard Metis FW version !";
				set->showWarningDialog(msg);
				//return false;
				return true;
		}
	}

	if (io.mercuryFW < 33 && set->getNumberOfReceivers() > 4 && io.hpsdrDeviceName == "Metis") {

		stop();

		QString msg = "Mercury FW < V3.3 has only 4 receivers!";
		set->showWarningDialog(msg);
		return false;
	}

	if (io.hermesFW < 18 && set->getNumberOfReceivers() > 2 && io.hpsdrDeviceName == "Hermes") {

		stop();

		QString msg = "Hermes FW < V1.8 has only 2 receivers!";
		set->showWarningDialog(msg);
		return false;
	}
	setWideBandBufferCount();

	return true;
}

bool DataEngine::start() {

	m_fwCount = 0;
	m_sendState = 0;

	// Only (re)create the protocol object if DataIO is not already running.
	// When getFirmwareVersions() precedes start(), it already created the correct
	// protocol and set io.protocol.  Deleting it here races with the DataIO
	// thread which may be actively calling methods on io->protocol.
	if (!m_dataIO) {
		if (m_hwInterface == QSDR::Metis || m_hwInterface == QSDR::Hermes) {
			if (set->getCurrentMetisCard().protocol == 2)
				m_protocol = std::make_unique<CProtocol2>();
			else
				m_protocol = std::make_unique<CProtocol1>();
		}
		io.protocol = m_protocol.get();
	}

	int rcvrs = set->getNumberOfReceivers();
    if (!m_audioInput) {
        createAudioInputProcessor();
    }
    m_audioInput->Setup();

	if (!m_dataIO) createDataIO();

	if (!m_dataProcessor) createDataProcessor();


	if (m_serverMode == QSDR::SDRMode && !m_wbDataProcessor)
		createWideBandDataProcessor();


	switch (m_serverMode) {

		//case QSDR::ExternalDSP:

			/*
			//CHECKED_CONNECT(
			//	set,
			//	SIGNAL(frequencyChanged(bool, int, long)),
			//	this,
			//	SLOT(setFrequency(bool, int, long)));

        //if (!m_audioProcessorRunning) {

        //	//if (!m_audioProcessor)	createAudioProcessor();
        //	if (!m_audioReceiver)	createAudioReceiver();

        //	m_audioInProcThread->start();
        //	if (m_audioInProcThread->isRunning()) {
        //
        //		m_audioInProcThreadRunning = true;
        //		DATA_ENGINE_DEBUG << "Audio processor process started.";
        //	}
        //	else {

        //		m_audioInProcThreadRunning = false;
        //		setSystemState(
        //						QSDR::AudioThreadError,
        //						m_hwInterface,
        //						m_serverMode,
        //						QSDR::DataEngineDown);
        //		return false;
        //	}
        //
        //	io.audio_rx = 0;
        //	io.clientList.append(0);

			//	m_audioProcessorRunning = true;
			//	setSystemState(
			//			QSDR::NoError,
			//			m_hwInterface,
			//			m_serverMode,
			//			QSDR::DataEngineUp);
			//}
			 */
			//return false;

        case QSDR::SDRMode:

            setTimeStamp(false);
            break;

		default:

			DATA_ENGINE_DEBUG << "no valid server mode";

			setSystemState(QSDR::ServerModeError, m_hwInterface, m_serverMode, QSDR::DataEngineDown);
			return false;
	}	// end switch (m_serverMode)

	if (RX.count() != rcvrs || m_dspThreadList.count() != rcvrs) {
		if (!m_dspThreadList.isEmpty()) {
			foreach (QThread* thread, m_dspThreadList) {
				thread->quit();
				thread->wait();
			}
			qDeleteAll(m_dspThreadList.begin(), m_dspThreadList.end());
			m_dspThreadList.clear();
		}

		if (!RX.isEmpty()) {
			qDeleteAll(RX.begin(), RX.end());
			RX.clear();
		}

		if (!initReceivers(rcvrs)) {
			DATA_ENGINE_DEBUG << "failed to initialize receivers for count" << rcvrs;
			return false;
		}
	}

	connectDSPSlots();
	const QList<qint64> ctrFrequencies = set->getCtrFrequencies();
	for (int i = 0; i < rcvrs ; i++) {

		RX.at(i)->setConnectedStatus(true);
		if (i < ctrFrequencies.count()) {
			setFrequency(true, i, ctrFrequencies.at(i));
		}


        //CHECKED_CONNECT(
		//		RX.at(i),
		//		SIGNAL(outputBufferSignal(int, const CPX &)),
		//		this, //m_dataProcessor,
		//		SLOT(setOutputBuffer(int, const CPX &)));

		CHECKED_CONNECT(
				RX.at(i),
				&SliceProcessor::outputBufferSignal,
				m_dataProcessor,
				&DataProcessor::setOutputBuffer);

        CHECKED_CONNECT(
				RX.at(i),
				&SliceProcessor::audioBufferSignal,
				m_dataProcessor,
				&DataProcessor::send_hpsdr_data);

		m_dspThreadList.at(i)->start(QThread::HighPriority);

		if (m_dspThreadList.at(i)->isRunning()) {

			//m_dataProcThreadRunning = true;
			io.networkIOMutex.lock();
			DATA_ENGINE_DEBUG << "receiver processor thread started for Rx " << i;
			io.networkIOMutex.unlock();
		}
		else {

			//m_dataProcThreadRunning = false;
			//setSystemState(QSDR::DataProcessThreadError, m_hwInterface, m_serverMode, QSDR::DataEngineDown);
			return false;
	}
		m_dataIO->set_wbBuffers(set->getWidebandBuffers());
	}

/*
    if (!startAudioInputProcessor(QThread::NormalPriority))
    {
        DATA_ENGINE_DEBUG << "Audio Input data processor thread could not be started.";
        return false;
    }
*/

	if (!startWideBandDataProcessor(QThread::NormalPriority)) {

		DATA_ENGINE_DEBUG << "wide band data processor thread could not be started.";
		return false;
	}

	// data IO thread
//	if (!startDataIO(QThread::HighPriority)) {//  ::NormalPriority)) {
//
//		DATA_ENGINE_DEBUG << "data receiver thread could not be started.";
//		return false;
//	}

	// IQ data processing thread
	if (!startDataProcessor(QThread::HighPriority)) {

		DATA_ENGINE_DEBUG << "data processor thread could not be started.";
		return false;
	}

	// data IO thread
	if (!startDataIO(QThread::HighPriority)) {//  ::NormalPriority::HighPriority)) {

		DATA_ENGINE_DEBUG << "data IO thread could not be started.";
		return false;
	}

	// Give the DataIO thread time to run initDataReceiverSocket() and bind its
	// UDP socket (m_dataIOSocket).  Without this delay, sendInitFramesToNetworkDevice()
	// and networkDeviceStartStop() both check !m_dataIOSocket and silently return
	// early, leaving the P1/P2 hardware without a start command.  This matches the
	// same pattern used in getFirmwareVersions().
	QThread::msleep(100);

	// start Sync,ADC and S-Meter timers
	//m_SyncChangedTime.start();
	//m_ADCChangedTime.start();
	//m_smeterTime.start();

		// pre-conditioning
	for (int i = 0; i < io.receivers; i++) {
		m_dataIO->sendInitFramesToNetworkDevice(i);
	}

    if (m_serverMode == QSDR::SDRMode && set->getWidebandData()) {
			m_dataIO->networkDeviceStartStop(0x03); // 0x03 for starting the device with wide band data
			QThread::msleep(100);
	    }
    else {
            DATA_ENGINE_DEBUG << "[START] calling networkDeviceStartStop(0x01) protocol="
                              << (m_protocol ? "valid" : "NULL")
                              << " hwInterface=" << m_hwInterface;
            m_dataIO->networkDeviceStartStop(0x01); // 0x01 for starting the device without wide band data
        }

	// For Protocol 2, keep startup simple and rely on the periodic control timer
	// to cycle DDC/TX/HP packets continuously, matching the known-good behavior.
	if (set->getCurrentMetisCard().protocol == 2 && m_dataProcessor) {
		io.rcveIQ_toggle = true;
		QMetaObject::invokeMethod(
			m_dataProcessor,
			&DataProcessor::startControlTimer,
			Qt::BlockingQueuedConnection);
		DATA_ENGINE_DEBUG << "[P2-START] control timer started";
	}

	m_networkDeviceRunning = true;
	setSystemState(QSDR::NoError, m_hwInterface, m_serverMode, QSDR::DataEngineUp);
	set->setSystemMessage("System running", 4000);

		DATA_ENGINE_DEBUG << "Data Engine thread: " << this->thread();

	return true;
}

void DataEngine::stop() {

	if (m_dataEngineState == QSDR::DataEngineUp) {
		
		switch (m_hwInterface) {

			case QSDR::Metis:
			case QSDR::Hermes:
				
				// turn time stamping off
				setTimeStamp(false);

				// For Protocol 2, stop periodic control traffic before issuing the
				// final stop command so the simulator does not see interleaved HP
				// packets with sequence jumps during shutdown.
				if (set->getCurrentMetisCard().protocol == 2 &&
					m_dataProcessor && m_dataProcThread && m_dataProcThread->isRunning()) {
					QMetaObject::invokeMethod(
						m_dataProcessor,
						&DataProcessor::stopControlTimer,
						Qt::BlockingQueuedConnection);
				}

				// stop the device
				m_dataIO->networkDeviceStartStop(0);
				m_networkDeviceRunning = false;
				DATA_ENGINE_DEBUG << "HPSDR device stopped";

				// stop the threads
				//QThread::msleep(100);
				stopDataIO();
				QThread::msleep(100);
				stopDataProcessor();
				if (m_wbDataProcessor)
					stopWideBandDataProcessor();

                m_protocol.reset();
                io.protocol = nullptr;
				
				// clear device list
				QThread::msleep(100);
				set->clearMetisCardList();
				DATA_ENGINE_DEBUG << "device cards list cleared.";
				break;

			case QSDR::NoInterfaceMode:

				stopDataIO();
				
				DATA_ENGINE_DEBUG << "data queue count: " << io.data_queue.count();

				stopDataProcessor();
                break;

#ifdef HAVE_SOAPYSDR
            case QSDR::SoapySDR:

                stopDataIO();
                stopDataProcessor();
                break;
#endif
        }

		while (!io.au_queue.isEmpty())
			io.au_queue.dequeue();

		// Stop all WDSP channels BEFORE killing DSP threads.
		// SetChannelState(wait=1) deadlocks if called after the DSP thread is
		// dead (nobody calls fexchange0 to release the internal semaphore).
		// Clearing run=0 here while threads are still alive lets any in-flight
		// fexchange0 call observe the flag and exit, leaving the channel in a
		// clean state for CloseChannel in the destructor.
		for (const auto &rx : RX) {
			if (rx->qtwdsp) {
				DATA_ENGINE_DEBUG << "[RX-STOP] stopping WDSP channel for rx" << rx->getReceiverNo();
				rx->qtwdsp->stopChannel();
			}
		}
		QThread::msleep(5); // let any in-flight fexchange0 observe run=0

		// clear receiver thread list
		foreach (QThread* thread, m_dspThreadList) {

			thread->quit();
			thread->wait();
		}
		qDeleteAll(m_dspThreadList.begin(), m_dspThreadList.end());
		m_dspThreadList.clear();

		// clear receiver list
        for (const auto &rx : RX) {

            rx->stop();
			rx->setConnectedStatus(false);
			disconnectDSPSlots();

			if (m_radioModel && m_radioModel->telemetry()) {
				disconnect(rx, nullptr, m_radioModel->telemetry(), nullptr);
			}

			/*disconnect(
				rx,
				SIGNAL(outputBufferSignal(int, const CPX &)),
				this,
				SLOT(setOutputBuffer(int, const CPX &)));*/

			/*disconnect(
				rx,
				SIGNAL(outputBufferSignal(int, const CPX &)),
				m_dataProcessor,
				SLOT(setOutputBuffer(int, const CPX &)));*/

			//rx->deleteDSPInterface();
			//DATA_ENGINE_DEBUG << "DSP core deleted.";
		}
		qDeleteAll(RX.begin(), RX.end());
		RX.clear();
		set->setRxList(RX);
		DATA_ENGINE_DEBUG << "receiver threads deleted, receivers deleted, receiver & thread list cleared.";
		set->setSystemMessage("Data engine shut down.", 4000);

		setSystemState(QSDR::NoError, m_hwInterface, m_serverMode, QSDR::DataEngineDown);
	}

	m_rxSamples = 0;
	m_restart = true;
	m_found = 0;
	m_hpsdrDevices = 0;

	set->setMercuryVersion(0);
	set->setPenelopeVersion(0);
	set->setMetisVersion(0);
	set->setHermesVersion(0);

	//set->setPeakHold(false);
	//set->resetWidebandSpectrumBuffer();

	/*disconnect(
		set, 
		SIGNAL(ctrFrequencyChanged(int, int, long)), 
		this, 
		SLOT(setFrequency(int, int, long)));*/

	DATA_ENGINE_DEBUG << "shut down done.";
}

bool DataEngine::initDataEngine() {

#ifdef TESTING
	qDebug() << "************************** TESTING MODUS ***********************************";
	return start();
#endif

	if (m_hwInterface == QSDR::NoInterfaceMode) {
		
		return startDataEngineWithoutConnection();
	}
#ifdef HAVE_SOAPYSDR
    else if (m_hwInterface == QSDR::SoapySDR) {
        if (!m_soapySDRSource) createDataIO();
        initReceivers(1);

        // Start DSP threads for all receivers (skipped by the normal HPSDR path)
        for (int i = 0; i < m_dspThreadList.size(); ++i) {
            m_dspThreadList.at(i)->start(QThread::HighPriority);
            DATA_ENGINE_DEBUG << "SoapySDR: started DSP thread for rx" << i;
        }

        if (!m_dataProcessor) createDataProcessor();

        if (!startDataIO(QThread::HighPriority)) {
            setSystemState(QSDR::DataReceiverThreadError, m_hwInterface, m_serverMode, QSDR::DataEngineDown);
            return false;
        }

        if (!startDataProcessor(QThread::HighPriority)) {
            setSystemState(QSDR::DataProcessThreadError, m_hwInterface, m_serverMode, QSDR::DataEngineDown);
            return false;
        }

        setSystemState(QSDR::NoError, m_hwInterface, m_serverMode, QSDR::DataEngineUp);
        set->setRadioState(RadioState::RX);
        return true;
    }
#endif
	else {
		
		if (findHPSDRDevices()) {
		
			if (io.mercuryFW > 0 || io.hermesFW > 0) {

				DATA_ENGINE_DEBUG << "got firmware versions:";
				DATA_ENGINE_DEBUG << "	Metis firmware:  " << io.metisFW;
				DATA_ENGINE_DEBUG << "	Mercury firmware:  " << io.mercuryFW;
				DATA_ENGINE_DEBUG << "	Penelope firmware:  " << io.penelopeFW;
				DATA_ENGINE_DEBUG << "	Pennylane firmware:  " << io.pennylaneFW;
				DATA_ENGINE_DEBUG << "	Hermes firmware: " << io.hermesFW;
				DATA_ENGINE_DEBUG << "stopping and restarting data engine.";

				return start();
			}
			else {

				DATA_ENGINE_DEBUG << "did not get firmware versions!";
				setSystemState(QSDR::FirmwareError, m_hwInterface, m_serverMode, QSDR::DataEngineDown);
			}
		}
	}
	return false;
}

bool DataEngine::initReceivers(int rcvrs) {

	DATA_ENGINE_DEBUG << "[RX-ADD] initReceivers: allocating" << rcvrs << "receiver(s)";

	for (int i = 0; i < rcvrs; i++) {
        if (!m_radioModel || i >= m_radioModel->slices().size()) continue;

        auto rx =  new SliceProcessor(m_radioModel->slices().at(i), nullptr);
		// init the DSP core
		DATA_ENGINE_DEBUG << "[RX-ADD] initReceivers: init DSP core for rx " << i;

		if (rx->initDSPInterface()) {

			DATA_ENGINE_DEBUG << "[RX-ADD] initReceivers: DSP core for rx" << i << " OK — QWDSPEngine constructed and WDSP channel open";

			rx->setConnectedStatus(false);
			rx->setServerMode(m_serverMode);

			// create dsp thread
			auto thread = new QThreadEx();
			rx->moveToThread(thread);

			//CHECKED_CONNECT(this, SIGNAL(doDSP()), rx, SLOT(dspProcessing()));

            connect(rx, &SliceProcessor::spectrumBufferChanged, set, &Settings::setSpectrumBuffer);
            if (RadioTelemetry* tel = m_radioModel ? m_radioModel->telemetry() : nullptr) {
                connect(rx, &SliceProcessor::sMeterValueChanged, tel,
                        [tel](int receiverId, double value) {
                            tel->setSMeterValue(receiverId, value);
                        });
            }
         //   connect(rx.get(), &SliceProcessor::outputBufferSignal, m_dataProcessor, &DataProcessor::setOutputBuffer);


			m_dspThreadList.append(thread);
			RX.append(rx);
		}
		else {

			return false;
		}
    }
    set->setRxList(RX);

	
	io.currentReceiver = 0;
	io.receivers = rcvrs;

	io.timing = 0;
	m_configure = io.receivers + 1;

	// init cc Rc parameters
	io.ccRx.devices.mercuryFWVersion = 0;
	io.ccRx.devices.penelopeFWVersion = 0;
	io.ccRx.devices.pennylaneFWVersion = 0;
	io.ccRx.devices.hermesFWVersion = 0;
	io.ccRx.devices.metisFWVersion = 0;

	io.ccRx.ptt    = false;
	io.ccRx.dash   = false;
	io.ccRx.dot    = false;
	io.ccRx.lt2208 = false;
	io.ccRx.ain1   = 0;
	io.ccRx.ain2   = 0;
	io.ccRx.ain3   = 0;
	io.ccRx.ain4   = 0;
	io.ccRx.ain5   = 0;
	io.ccRx.ain6   = 0;
	io.ccRx.hermesI01 = false;
	io.ccRx.hermesI02 = false;
	io.ccRx.hermesI03 = false;
	io.ccRx.hermesI04 = false;
	io.ccRx.mercury1_LT2208 = false;
	io.ccRx.mercury2_LT2208 = false;
	io.ccRx.mercury3_LT2208 = false;
	io.ccRx.mercury4_LT2208 = false;

	// init cc Tx parameters
	io.ccTx.currentBand = set->getCurrentHamBand(0);
	io.ccTx.mercuryAttenuators = set->getMercuryAttenuators(0);
	io.ccTx.mercuryAttenuator = io.ccTx.mercuryAttenuators.at(io.ccTx.currentBand);
	io.ccTx.dither = set->getMercuryDither();
	io.ccTx.random = set->getMercuryRandom();
	io.ccTx.duplex = 1;
	io.ccTx.mox = false;
	io.ccTx.ptt = false;
	io.ccTx.alexStates = set->getAlexStates();
	io.ccTx.vnaMode = false;
	io.ccTx.alexConfig = set->getAlexConfig();
	io.ccTx.timeStamp = 0;
	io.ccTx.commonMercuryFrequencies = 0;
	io.ccTx.pennyOCenabled = set->getPennyOCEnabled();
	io.ccTx.rxJ6pinList = set->getRxJ6Pins();
	io.ccTx.txJ6pinList = set->getTxJ6Pins();

	setAlexConfiguration(io.ccTx.alexConfig);

	io.rxClass = set->getRxClass();
	io.mic_gain = 0.26F;
	io.rx_freq_change = -1;
	io.tx_freq_change = -1;
	io.clients = 0;
	io.sendIQ_toggle = true;
	io.rcveIQ_toggle = false;
	io.alexForwardVolts = 0.0;
	io.alexReverseVolts = 0.0;
	io.alexForwardPower = 0.0;
	io.alexReversePower = 0.0;
	io.penelopeForwardVolts = 0.0;
	io.penelopeForwardPower = 0.0;
	io.ain3Volts = 0.0;
	io.ain4Volts = 0.0;
	io.supplyVolts = 0.0f;


	//*****************************
	// C&C bytes
	for (int i = 0; i < 5; i++) {

		io.control_in[i]  = 0x00;
		io.control_out[i] = 0x00;
	}

	// C0
	// 0 0 0 0 0 0 0 0
	//               |
	//               +------------ MOX (1 = active, 0 = inactive)

	io.control_out[0] |= MOX_ENABLED;

	// set C1
	//
	// 0 0 0 0 0 0 0 0
	// | | | | | | | |
	// | | | | | | + +------------ Speed (00 = 48kHz, 01 = 96kHz, 10 = 192kHz)
	// | | | | + +---------------- 10MHz Ref. (00 = Atlas/Excalibur, 01 = Penelope, 10 = Mercury)*
	// | | | +-------------------- 122.88MHz source (0 = Penelope, 1 = Mercury)*
	// | + +---------------------- Config (00 = nil, 01 = Penelope, 10 = Mercury, 11 = both)*
	// +-------------------------- Mic source (0 = Janus, 1 = Penelope)*

	// Bits 1,0
	setSampleRate(set->getSampleRate());

	// Bits 7,..,2
	setHPSDRConfig();

	io.control_out[1] &= 0x03; // 0 0 0 0 0 0 1 1
	io.control_out[1] |= io.ccTx.clockByte;

	// set C2
	//
	// 0 0 0 0 0 0 0 0
	// |           | |
	// |           | +------------ Mode (1 = Class E, 0 = All other modes)
    // +---------- +-------------- Open Collector Outputs on Penelope or Hermes (bit 6...bit 0)

	io.control_out[2] = io.control_out[2] & 0xFE; // 1 1 1 1 1 1 1 0
	io.control_out[2] = io.control_out[2] | io.rxClass;

	// set C3
	//
	// 0 0 0 0 0 0 0 0
	// | | | | | | | |
	// | | | | | | + +------------ Alex Attenuator (00 = 0dB, 01 = 10dB, 10 = 20dB, 11 = 30dB)
	// | | | | | +---------------- Preamp On/Off (0 = Off, 1 = On)
	// | | | | +------------------ LT2208 Dither (0 = Off, 1 = On)
	// | | | + ------------------- LT2208 Random (0= Off, 1 = On)
	// | + + --------------------- Alex Rx Antenna (00 = none, 01 = Rx1, 10 = Rx2, 11 = XV)
	// + ------------------------- Alex Rx out (0 = off, 1 = on). Set if Alex Rx Antenna > 00.

	io.control_out[3] = io.control_out[3] & 0xFB; // 1 1 1 1 1 0 1 1
	io.control_out[3] = io.control_out[3] | (io.ccTx.mercuryAttenuator << 2);

	io.control_out[3] = io.control_out[3] & 0xF7; // 1 1 1 1 0 1 1 1
	io.control_out[3] = io.control_out[3] | (io.ccTx.dither << 3);

	io.control_out[3] = io.control_out[3] & 0xEF; // 1 1 1 0 1 1 1 1
	io.control_out[3] = io.control_out[3] | (io.ccTx.random << 4);

	// set C4
	//
	// 0 0 0 0 0 0 0 0
	// | | | | | | | |
	// | | | | | | + + ----------- Alex Tx relay (00 = Tx1, 01= Tx2, 10 = Tx3)
	// | | | | | + --------------- Duplex (0 = off, 1 = on)
	// + + + + +------------------ Number of Receivers (00000 = 1, 11111 = 32)

	//RRK removed 4HL
        // | +------------------------ Time stamp - 1PPS on LSB of Mic data (0 = off, 1 = on)
	// +-------------------------- Common Mercury Frequency (0 = independent frequencies to Mercury
	//			                   Boards, 1 = same frequency to all Mercury boards)

	io.control_out[4] &= 0x07; // 1 1 0 0 0 1 1 1
	io.control_out[4] = (io.ccTx.duplex << 2) | ((io.receivers - 1) << 3);

	if (!m_radioController) {
		m_radioController = std::make_unique<RadioController>(this);
	}
	m_radioController->bind(m_radioModel, this);

	return true;
}

void DataEngine::setHPSDRConfig() {

	io.ccTx.clockByte = 0x0;

	// C1
	// 0 0 0 0 0 0 0 0
	// | | | | | | | |
	// | | | | | | + +------------ Speed (00 = 48kHz, 01 = 96kHz, 10 = 192kHz)
	// | | | | + +---------------- 10MHz Ref. (00 = Atlas/Excalibur, 01 = Penelope, 10 = Mercury)*
	// | | | +-------------------- 122.88MHz source (0 = Penelope, 1 = Mercury)*
	// | + +---------------------- Config (00 = nil, 01 = Penelope, 10 = Mercury, 11 = both)*
	// +-------------------------- Mic source (0 = Janus, 1 = Penelope)*
	//
	// * Ignored by Hermes

	if (
		(set->getPenelopePresence()   || set->getPennyLanePresence()) &&
		((set->get10MHzSource() == 0) || set->getExcaliburPresence())
		)
	{

		io.ccTx.clockByte = MIC_SOURCE_PENELOPE | MERCURY_PRESENT | PENELOPE_PRESENT | MERCURY_122_88MHZ_SOURCE | ATLAS_10MHZ_SOURCE;
	}
	else if ((set->getPenelopePresence() || set->getPennyLanePresence()) && (set->get10MHzSource() == 1)) {
		
		io.ccTx.clockByte = MIC_SOURCE_PENELOPE | MERCURY_PRESENT | PENELOPE_PRESENT | MERCURY_122_88MHZ_SOURCE | PENELOPE_10MHZ_SOURCE;
	}
	else if ((set->getPenelopePresence() || set->getPennyLanePresence()) && (set->get10MHzSource() == 2)) {
		
		io.ccTx.clockByte = MIC_SOURCE_PENELOPE | MERCURY_PRESENT | PENELOPE_PRESENT | MERCURY_122_88MHZ_SOURCE | MERCURY_10MHZ_SOURCE;
	}
	else if ((set->get10MHzSource() == 0) || set->getExcaliburPresence()) {
		
		io.ccTx.clockByte = MERCURY_PRESENT | MERCURY_122_88MHZ_SOURCE | ATLAS_10MHZ_SOURCE;
	}
	else {
		
		io.ccTx.clockByte = MERCURY_PRESENT | MERCURY_122_88MHZ_SOURCE | MERCURY_10MHZ_SOURCE;
	}
}

void DataEngine::connectDSPSlots() {
    connect(set, &Settings::ctrFrequencyChanged, this, &DataEngine::setFrequency);
}

void DataEngine::disconnectDSPSlots() {
    disconnect(set, &Settings::ctrFrequencyChanged, this, &DataEngine::setFrequency);
}




//********************************************************
// create, start/stop HPSDR device network IO

void DataEngine::createDiscoverer() {

	m_discoverer = new Discoverer(&io);

	m_discoveryThread = new QThreadEx();
	m_discoverer->moveToThread(m_discoveryThread);

#ifdef HAVE_SOAPYSDR
    connect(m_discoverer, &Discoverer::soapyDeviceListFound, set, &Settings::setSoapyDeviceList);
#endif
}

bool DataEngine::startDiscoverer(QThread::Priority prio) {

	m_discoveryThread->start(prio);

	if (m_discoveryThread->isRunning()) {
					
		m_discoveryThreadRunning = true;
		io.networkIOMutex.lock();
        DATA_ENGINE_DEBUG << "HPSDR device discovery thread started.";
		io.networkIOMutex.unlock();

		return true;
	}
	else {

		m_discoveryThreadRunning = false;
		return false;
	}
}

void DataEngine::stopDiscoverer() {

	if (m_discoveryThread->isRunning()) {
		
		m_discoveryThread->quit();
		m_discoveryThread->wait(1000);
		delete m_discoveryThread;
        m_discoveryThread = nullptr;
		delete m_discoverer;
		m_discoverer = nullptr;

		m_discoveryThreadRunning = false;

		DATA_ENGINE_DEBUG << "HPSDR discovery thread stopped and deleted.";
	}
	else
		DATA_ENGINE_DEBUG << "HPSDR discovery thread wasn't started.";
}

//********************************************************
// create, start/stop data receiver

void DataEngine::createDataIO() {

#ifdef HAVE_SOAPYSDR
    if (m_hwInterface == QSDR::SoapySDR) {
        m_soapySDRSource = new SoapySDRDataSource(&io);
        m_dataIOThread = new QThreadEx();
        m_soapySDRSource->moveToThread(m_dataIOThread);

        m_soapySDRSource->connect(
                    m_dataIOThread,
                    &QThread::started,
                    m_soapySDRSource,
                    &SoapySDRDataSource::init);

        m_soapySDRSource->connect(
                    m_dataIOThread,
                    &QThread::started,
                    m_soapySDRSource,
                    &SoapySDRDataSource::runStream);
        
        // We still need DataIO for local soundcard output!
        m_dataIO = new DataIO(&io);
        return;
    }
#endif

	m_dataIO = new DataIO(&io);

	switch (m_serverMode) {
		
		//case QSDR::ExternalDSP:
		//	break;

		//case QSDR::InternalDSP:
		case QSDR::SDRMode:

			io.networkIOMutex.lock();
			DATA_ENGINE_DEBUG 	<< "configured for "
								<< qPrintable(QString::number(set->getNumberOfReceivers()))
								<< " receiver(s) at "
								<< qPrintable(QString::number(set->getSampleRate()/1000))
								<< " kHz sample rate";
			io.networkIOMutex.unlock();
//			sendMessage(
//				m_message.arg(
//					QString::number(set->getNumberOfReceivers()),
//					QString::number(set->getSampleRate()/1000)));
			break;


        case QSDR::NoServerMode:
            break;
    }

	m_dataIOThread = new QThreadEx();
	m_dataIO->moveToThread(m_dataIOThread);

	switch (m_hwInterface) {

		case QSDR::NoInterfaceMode:
/*
			m_dataIO->connect(
						m_dataIOThread,
						SIGNAL(started()), 
						SLOT(readData()));
						*/
			break;
			
		case QSDR::Metis:
		case QSDR::Hermes:

			m_dataIO->connect(
						m_dataIOThread,
						&QThread::started, 
						m_dataIO,
						&DataIO::initDataReceiverSocket);
			break;
#ifdef HAVE_SOAPYSDR
        case QSDR::SoapySDR:
            break;
#endif
    }
}

bool DataEngine::startDataIO(QThread::Priority prio) {

	m_dataIOThread->start(prio);

	if (m_dataIOThread->isRunning()) {
					
		m_dataIOThreadRunning = true;
		io.networkIOMutex.lock();
		DATA_ENGINE_DEBUG << "data IO thread started.";
		io.networkIOMutex.unlock();

		return true;
	}
	else {

		m_dataIOThreadRunning = false;
		setSystemState(QSDR::DataProcessThreadError, m_hwInterface, m_serverMode, QSDR::DataEngineDown);
		return false;
	}
}

void DataEngine::stopDataIO() {

	if (m_dataIOThread && m_dataIOThread->isRunning()) {
					
#ifdef HAVE_SOAPYSDR
        if (m_soapySDRSource) {
            m_soapySDRSource->stop();
        }
#endif
        if (m_dataIO) {
            m_dataIO->stop();
        }

		m_dataIOThread->quit();

		while (!m_dataIOThread->isFinished()) {
		
			DATA_ENGINE_DEBUG << "data IO thread not yet finished...";
			if (m_dataIOThread->wait(100)) break;
		}
		m_dataIOThreadRunning = false;
		
		delete m_dataIOThread;
        m_dataIOThread = nullptr;

		delete m_dataIO;
		m_dataIO = nullptr;

#ifdef HAVE_SOAPYSDR
        delete m_soapySDRSource;
        m_soapySDRSource = nullptr;
#endif

		DATA_ENGINE_DEBUG << "data IO thread deleted.";
	}
	else
		DATA_ENGINE_DEBUG << "data IO thread wasn't started.";
}
 
//********************************************************
// create, start/stop data processor

void DataEngine::createDataProcessor() {

	m_dataProcessor = new DataProcessor(this, m_serverMode, m_hwInterface);
	sendSocket = new QUdpSocket();
    m_controlSocket = new QUdpSocket();
    if (!m_controlSocket->bind(QHostAddress::AnyIPv4, 0)) {
        DATA_ENGINE_DEBUG << "Warning: Could not bind m_controlSocket.";
    }
    connect(
			sendSocket,
        &QAbstractSocket::errorOccurred,
			m_dataProcessor,
        &DataProcessor::displayDataProcessorSocketError
        );



	switch (m_serverMode) {
		
		// The signal iqDataReady is generated by the function
		// processInputBuffer when a block of input data are
		// decoded.

		case QSDR::SDRMode:
			/*connect(
				this,
				SIGNAL(iqDataReady(int)),
				SLOT(dttSPDspProcessing(int)),
				Qt::DirectConnection);*/

			break;
			
		case QSDR::NoServerMode:
        break;
    }

	m_dataProcThread = new QThreadEx();
	m_dataProcessor->moveToThread(m_dataProcThread);
	sendSocket->moveToThread(m_dataProcThread);
    if (m_controlSocket) {
        m_controlSocket->moveToThread(m_dataProcThread);
    }

	switch (m_hwInterface) {

		case QSDR::NoInterfaceMode:
            /*
			m_dataProcessor->connect(
						m_dataProcThread, 
						&QThread::started, 
						m_dataProcessor, 
						&DataProcessor::processData);
            */
			break;
			
		case QSDR::Metis:
		case QSDR::Hermes:

		    /*
			m_dataProcessor->connect(
						m_dataProcThread, 
						SIGNAL(started()), 
						SLOT(processDeviceData()));
*/

			if (m_dataIO) {
				CHECKED_CONNECT(
						m_dataIO,
						&DataIO::readydata,
						m_dataProcessor,
						&DataProcessor::processReadData);
            } else {
				DATA_ENGINE_DEBUG << "createDataProcessor: no data source found, skipping readydata connection.";
			}

            break;

#ifdef HAVE_SOAPYSDR
        case QSDR::SoapySDR:
            if (m_soapySDRSource) {
                qDebug() << "DataEngine: Connecting m_soapySDRSource::readydata to DataProcessor";
                CHECKED_CONNECT(
                        m_soapySDRSource,
                        &SoapySDRDataSource::readydata,
                        m_dataProcessor,
                        &DataProcessor::processReadData);
                // Also trigger one immediate check in case data is already waiting
                QMetaObject::invokeMethod(m_dataProcessor, "processReadData", Qt::QueuedConnection);
            }
            break;
#endif
    }
}


bool DataEngine::startDataProcessor(QThread::Priority prio) {

	m_dataProcThread->start(prio);
				
	if (m_dataProcThread->isRunning()) {
					
		m_dataProcThreadRunning = true;
		io.networkIOMutex.lock();
		DATA_ENGINE_DEBUG << "data processor thread started.";
		io.networkIOMutex.unlock();

		return true;
	}
	else {

		m_dataProcThreadRunning = false;
		setSystemState(QSDR::DataProcessThreadError, m_hwInterface, m_serverMode, QSDR::DataEngineDown);
		return false;
	}
}

void DataEngine::stopDataProcessor() {

	if (m_dataProcThread->isRunning()) {
		if (m_dataProcessor) {
			QMetaObject::invokeMethod(m_dataProcessor,
								  &DataProcessor::stopControlTimer,
								  Qt::BlockingQueuedConnection);
			QMetaObject::invokeMethod(m_dataProcessor,
								  &DataProcessor::stop,
								  Qt::BlockingQueuedConnection);
		}
		
		if (m_serverMode == QSDR::SDRMode ) {
			
			if (io.iq_queue.isEmpty()) {
				io.iq_queue.enqueue(TIQPacket(QByteArray(BUFFER_SIZE, 0x0), 0));
			}
		}

		m_dataProcThread->quit();
		m_dataProcThread->wait();
		delete m_dataProcThread;
        m_dataProcThread = nullptr;
		delete m_dataProcessor;
		m_dataProcessor = nullptr;

		if (m_serverMode == QSDR::SDRMode ) {

			while (!io.iq_queue.isEmpty())
				io.iq_queue.dequeue();

			DATA_ENGINE_DEBUG << "iq_queue empty.";
		}

		m_dataProcThreadRunning = false;

		DATA_ENGINE_DEBUG << "data processor thread deleted.";
	}
	else
		DATA_ENGINE_DEBUG << "data processor thread wasn't started.";
}

//********************************************************
// create, start/stop audio out processor

void DataEngine::createAudioOutProcessor() {

	m_audioOutProcessor = new AudioOutProcessor(this, m_serverMode);
	m_audioOutProcThread = new QThreadEx();
	m_audioOutProcessor->moveToThread(m_audioOutProcThread);
}

__attribute__((unused)) void DataEngine::startAudioOutProcessor(QThread::Priority prio) {

	Q_UNUSED (prio)
}

void DataEngine::stopAudioOutProcessor() {
}

//********************************************************
// create, start/stop winde band data processor

void DataEngine::createWideBandDataProcessor() {

	int size;

	if (io.mercuryFW > 32 || io.hermesFW > 11)
		size = BIGWIDEBANDSIZE;
	else
		size = SMALLWIDEBANDSIZE;
	
	m_wbDataProcessor = new WideBandDataProcessor(&io, m_serverMode, size);

	connect(set, &Settings::spectrumAveragingCntChanged,
			this, &DataEngine::setWbSpectrumAveraging);

	if (RadioTelemetry* tel = m_radioModel ? m_radioModel->telemetry() : nullptr) {
		connect(m_wbDataProcessor, &WideBandDataProcessor::wbSpectrumBufferChanged,
		        tel, &RadioTelemetry::setWidebandSpectrumBuffer);
	}


	m_wbDataProcThread = new QThreadEx();
	m_wbDataProcessor->moveToThread(m_wbDataProcThread);
	m_wbDataProcessor->connect(
							m_wbDataProcThread, 
							&QThread::started, 
							m_wbDataProcessor, 
							&WideBandDataProcessor::processWideBandData);
}

bool DataEngine::startWideBandDataProcessor(QThread::Priority prio) {
	m_wbDataProcThread->start(prio);//(QThread::TimeCriticalPriority);//(QThread::HighPriority);//(QThread::LowPriority);

	if (m_wbDataProcThread->isRunning()) {
					
		m_wbDataRcvrThreadRunning = true;
		io.networkIOMutex.lock();
		DATA_ENGINE_DEBUG << "wide band data processor thread started.";
		io.networkIOMutex.unlock();

		return true;
	}
	else {

		m_wbDataRcvrThreadRunning = false;
		setSystemState(QSDR::WideBandDataProcessThreadError, m_hwInterface, m_serverMode, QSDR::DataEngineDown);
		return false;
	}
}

void DataEngine::stopWideBandDataProcessor() {

	if (m_wbDataProcThread->isRunning()) {
					
		m_wbDataProcessor->stop();
		if (io.wb_queue.isEmpty())
			io.wb_queue.enqueue(m_datagram);

		m_wbDataProcThread->quit();
		m_wbDataProcThread->wait();
		delete m_wbDataProcThread;
        m_wbDataProcThread = nullptr;
		delete m_wbDataProcessor;
		m_wbDataProcessor = nullptr;

		m_wbDataRcvrThreadRunning = false;
		
		DATA_ENGINE_DEBUG << "wide band data processor thread deleted.";
	}
	else
		DATA_ENGINE_DEBUG << "wide band data processor thread wasn't started.";
}

void DataEngine::setWideBandBufferCount()
{
	// if we have 4096 * 16 bit = 8 * 1024 raw consecutive ADC samples, m_wbBuffers = 8
	// we have 16384 * 16 bit = 32 * 1024 raw consecutive ADC samples, m_wbBuffers = 32
	int wbBuffers = 0;
	if (io.mercuryFW > 32 || io.hermesFW > 11)
		wbBuffers = BIGWIDEBANDSIZE / 512;
	else
		wbBuffers = SMALLWIDEBANDSIZE / 512;

	set->setWidebandBuffers(wbBuffers);

}
//********************************************************
// create, start/stop audio receiver

void DataEngine::createAudioReceiver() {

	m_audioReceiver = new AudioReceiver(&io);

	connect(m_audioReceiver, &AudioReceiver::rcveIQEvent,
			this, &DataEngine::setRcveIQSignal);

	connect(m_audioReceiver, &AudioReceiver::clientConnectedEvent,
			this, qOverload<bool>(&DataEngine::setClientConnected));

	
	m_AudioRcvrThread = new QThreadEx();
	m_audioReceiver->moveToThread(m_AudioRcvrThread);

	m_audioReceiver->connect(
						m_AudioRcvrThread, 
						&QThread::started, 
						m_audioReceiver, 
						&AudioReceiver::initClient);
}
 

void DataEngine::processFileBuffer(const QList<qreal> buffer) {


	int topsize = 2*BUFFER_SIZE - 1;
	//float specMax = -100.0f;
	//float specMin = 0.0f;

	Q_ASSERT(buffer.length() == 128);

	for (int i = 0; i < 64; i++) {

		cpxIn[i + m_rxSamples].re = buffer.at(2*i);
		cpxIn[i + m_rxSamples].im = buffer.at(2*i+1);

	}
	m_rxSamples += 64;

	if (m_rxSamples == 2*BUFFER_SIZE) {

		// reorder the spectrum buffer
		for (int i = 0; i < BUFFER_SIZE; i++) {

			m_spectrumBuffer[topsize - i] =
				(float)(10.0 * log10(MagCPX(cpxOut[i+BUFFER_SIZE]) + 1.5E-45));
			m_spectrumBuffer[BUFFER_SIZE - i] =
				(float)(10.0 * log10(MagCPX(cpxOut[i]) + 1.5E-45));
		}

		/*float specMean = 0.0f;
		for (int i = BUFFER_SIZE+20; i < BUFFER_SIZE+105; i++) {

			specMean += m_spectrumBuffer[i];
			if (m_spectrumBuffer[i] > specMax) specMax = m_spectrumBuffer[i];
			if (m_spectrumBuffer[i] < specMin) specMin = m_spectrumBuffer[i];
		}*/
		//specMean *= 1.0f/BUFFER_SIZE;
		//DATA_PROCESSOR_DEBUG << "pan min" << specMin << "max" << specMax << "mean" << specMean;

		QThread::usleep(42667);

		//emit spectrumBufferChanged(m_spectrumBuffer);
		//set->setSpectrumBuffer(m_spectrumBuffer);
		//set->setSpectrumBuffer(0, m_spectrumBuffer);

		m_rxSamples = 0;
	}
}


//*****************************************************************************
//

void DataEngine::systemStateChanged(
	QSDR::_Error err, 
	QSDR::_HWInterfaceMode hwmode, 
	QSDR::_ServerMode mode, 
	QSDR::_DataEngineState state)
{
	Q_UNUSED (err)

	QMutexLocker locker(&io.mutex);
	if (m_hwInterface != hwmode)
		m_hwInterface = hwmode;
		
	if (m_serverMode != mode)
		m_serverMode = mode;
		
	if (m_dataEngineState != state)
		m_dataEngineState = state;
}

void DataEngine::setSystemState(
		QSDR::_Error err,
		QSDR::_HWInterfaceMode hwmode,
		QSDR::_ServerMode statemode,
		QSDR::_DataEngineState enginestate)
{
	QMutexLocker locker(&io.networkIOMutex);
	set->setSystemState(err, hwmode, statemode, enginestate);
}

float DataEngine::getFilterSizeCalibrationOffset() {

    //int size=1024; // dspBufferSize
    float i = log10((qreal) BUFFER_SIZE);
    return 3.0f*(11.0f - i);
}

void DataEngine::searchHpsdrNetworkDevices() {

	if (!m_discoverer) createDiscoverer();

	// HPSDR network IO thread
	if (!startDiscoverer(QThread::NormalPriority)) {

		DATA_ENGINE_DEBUG << "HPSDR network discovery thread could not be started.";
		return;
	}

    QMetaObject::invokeMethod(m_discoverer, "initHPSDRDevice", Qt::QueuedConnection);

	io.networkIOMutex.lock();
	io.devicefound.wait(&io.networkIOMutex);

	//m_discoverer->findHPSDRDevices();

	// stop the discovery thread
	io.networkIOMutex.unlock();
	stopDiscoverer();
}

#ifdef HAVE_SOAPYSDR
void DataEngine::searchSoapyDevices() {
    if (!m_discoverer) createDiscoverer();
    if (!m_discoveryThread->isRunning()) {
        m_discoveryThread->start();
    }
    QMetaObject::invokeMethod(m_discoverer, "discoverSoapyDevices", Qt::QueuedConnection);
}
#endif

void DataEngine::setHPSDRDeviceNumber(int value) {

	m_hpsdrDevices = value;
}

void DataEngine::rxListChanged(QList<SliceProcessor *> list) {

	QMutexLocker locker(&io.mutex);
	RX = list;
}

void DataEngine::setCurrentReceiver(int rx) {

	QMutexLocker locker(&io.mutex);
	io.currentReceiver = rx;
}

void DataEngine::setFramesPerSecond(int rx, int value) {

	Q_UNUSED(rx)
	Q_UNUSED(value)

	/*io.mutex.lock();
	if (m_fpsList.length() > 0)
		m_fpsList[rx] = (int)(1000000.0/value);
	io.mutex.unlock();*/
}

void DataEngine::setSampleRate(int value) {

	bool applyOk = true;

	if (set && set->getSampleRate() != value) {
		DATA_ENGINE_DEBUG << "sample-rate propagation mismatch: signal=" << value
		                  << "settings=" << set->getSampleRate();
	}

	bool shouldRequestP2Update = false;
	{
	QMutexLocker locker(&io.mutex);

	switch (value) {
	
		case 48000:
			io.samplerate = value;
			io.speed = 0;
			break;
			
		case 96000:
			io.samplerate = value;
			io.speed = 1;
			break;
			
		case 192000:
			io.samplerate = value;
			io.speed = 2;
			break;
			
		case 384000:
			io.samplerate = value;
			io.speed = 3;
			break;

        case 768000:
            io.samplerate = value;
            io.speed = 4;
            break;

        case 1536000:
            io.samplerate = value;
            io.speed = 5;
            break;

		default:
			DATA_ENGINE_DEBUG << "invalid sample rate !\n";
			applyOk = false;
			break;
	}

	shouldRequestP2Update = applyOk && m_protocol && set->getCurrentMetisCard().protocol == 2 && m_dataProcessor;

	if (io.samplerate != value) {
		DATA_ENGINE_DEBUG << "samplerate apply mismatch: requested=" << value
		                  << "applied=" << io.samplerate;
	}

	} // QMutexLocker released here

	if (!applyOk) {
		stop();
		return;
	}

	if (shouldRequestP2Update) {
		// On live rate changes while RX is running, force an immediate DDC-specific
		// control packet so hardware DDC rate tracks io/settings right away.
		QMetaObject::invokeMethod(m_dataProcessor,
								  &DataProcessor::requestProtocol2DDCUpdate,
								  Qt::QueuedConnection);
	}

}

void DataEngine::setMercuryAttenuator(HamBand band, int value) {

	Q_UNUSED(band)

	QMutexLocker locker(&io.mutex);
	io.ccTx.mercuryAttenuator = value;
}

void DataEngine::setMercuryAttenuators(QList<int> attn) {

	QMutexLocker locker(&io.mutex);
	io.ccTx.mercuryAttenuators = attn;
}

void DataEngine::setDither(int value) {

	QMutexLocker locker(&io.mutex);
	io.ccTx.dither = value;
}

void DataEngine::setRandom(int value) {

	QMutexLocker locker(&io.mutex);
	io.ccTx.random = value;
}

void DataEngine::set10MhzSource(int source) {

	QMutexLocker locker(&io.mutex);
	io.control_out[1] = io.control_out[1] & 0xF3;
	io.control_out[1] = io.control_out[1] | (source << 2);
}

void DataEngine::set122_88MhzSource(int source) {

	QMutexLocker locker(&io.mutex);
	io.control_out[1] = io.control_out[1] & 0xEF;
	io.control_out[1] = io.control_out[1] | (source << 4);
}

void DataEngine::setMicSource( int source) {

	QMutexLocker locker(&io.mutex);
	io.control_out[1] = io.control_out[1] & 0x7F;
	io.control_out[1] = io.control_out[1] | (source << 7);
}

void DataEngine::setMercuryClass(int value) {

	QMutexLocker locker(&io.mutex);
	io.rxClass = value;
}

void DataEngine::setMercuryTiming(int value) {

	QMutexLocker locker(&io.mutex);
	io.timing = value;
}

void DataEngine::setAlexConfiguration(quint16 conf) {

	{
		QMutexLocker locker(&io.mutex);
		io.ccTx.alexConfig = conf;
		DATA_ENGINE_DEBUG << "Alex Configuration = " << io.ccTx.alexConfig;
	}

	if (set->getCurrentMetisCard().protocol == 2 && m_dataProcessor) {
		QMetaObject::invokeMethod(m_dataProcessor,
			&DataProcessor::requestProtocol2HPUpdate,
			Qt::QueuedConnection);
	}
}

void DataEngine::setAlexStates(HamBand band, const QList<int> &states) {

	Q_UNUSED (band)

	{
		QMutexLocker locker(&io.mutex);
		qDebug() << "setAlexStates: band=" << band << "states=" << states;
		io.ccTx.alexStates = states;
		DATA_ENGINE_DEBUG << "Alex States = " << io.ccTx.alexStates;
	}

	if (set->getCurrentMetisCard().protocol == 2 && m_dataProcessor) {
		QMetaObject::invokeMethod(m_dataProcessor,
			&DataProcessor::requestProtocol2HPUpdate,
			Qt::QueuedConnection);
	}
}

void DataEngine::setPennyOCEnabled(bool value) {

	QMutexLocker locker(&io.mutex);
	io.ccTx.pennyOCenabled = value;
}

void DataEngine::setRxJ6Pins(const QList<int> &list) {

	QMutexLocker locker(&io.mutex);
	io.ccTx.rxJ6pinList = list;

}

void DataEngine::setTxJ6Pins(const QList<int> &list) {

	QMutexLocker locker(&io.mutex);
	io.ccTx.txJ6pinList = list;
}

void DataEngine::setRcveIQSignal(int value) {

	emit rcveIQEvent(value);
}

void DataEngine::setPenelopeVersion(int version) {

	emit penelopeVersionInfoEvent(version);
}

void DataEngine::setHwIOVersion(int version) {

	emit hwIOVersionInfoEvent(version);
}

void DataEngine::setNumberOfRx(int value) {

	DATA_ENGINE_DEBUG << "[RX-ADD] setNumberOfRx: requested=" << value << "current=" << io.receivers;

	if (io.receivers == value) {
		DATA_ENGINE_DEBUG << "[RX-ADD] receiver count unchanged, no action.";
		return;
	}

    bool restart = (m_dataEngineState == QSDR::DataEngineUp);
	DATA_ENGINE_DEBUG << "[RX-ADD] engineUp=" << restart << "-> will" << (restart ? "stop/restart" : "update counts only");
    if (restart) {
		DATA_ENGINE_DEBUG << "[RX-ADD] stopping engine...";
		stop();
		// Give hardware/network stack a short settle window before re-init.
		QThread::msleep(200);
		DATA_ENGINE_DEBUG << "[RX-ADD] engine stopped, settling 200ms done.";
	}

	{
	QMutexLocker locker(&io.mutex);
	io.receivers = value;
	if (io.currentReceiver >= value) {
		DATA_ENGINE_DEBUG << "[RX-ADD] currentReceiver" << io.currentReceiver << ">= new count, resetting to 0";
		io.currentReceiver = 0;
	}
	}

	DATA_ENGINE_DEBUG << "[RX-ADD] flushing IQ queue (" << io.iq_queue.count() << " items) and WB queue (" << io.wb_queue.count() << " items)";
	while (!io.iq_queue.isEmpty())
		io.iq_queue.dequeue();
	while (!io.wb_queue.isEmpty())
		io.wb_queue.dequeue();

	if (set->getCurrentReceiver() >= value) {
		set->setCurrentReceiver(0);
	}

	DATA_ENGINE_DEBUG << "[RX-ADD] io.receivers set to" << value;

    if (restart) {
		DATA_ENGINE_DEBUG << "[RX-ADD] restarting engine with" << value << "receiver(s)...";
		QThread::msleep(100);
		if (!start()) {
			DATA_ENGINE_DEBUG << "[RX-ADD] FAILED to restart data engine after receiver-count change.";
		} else {
			DATA_ENGINE_DEBUG << "[RX-ADD] engine restart successful.";
			if (m_protocol && set->getCurrentMetisCard().protocol == 2 && m_dataProcessor) {
				// For Protocol 2: after the engine restarts, push an explicit DDC Specific
				// (port 1025) + HP Run=1 (port 1027) burst so the simulator immediately
				// receives the updated DDC enable bitmask and re-asserts Run=1.
				DATA_ENGINE_DEBUG << "[RX-ADD] P2: queuing DDC+HP Run=1 setup burst for" << value << "receiver(s)";
				QMetaObject::invokeMethod(m_dataProcessor,
				                          &DataProcessor::requestProtocol2ReceiverSetup,
				                          Qt::QueuedConnection);
			}
		}
	}
}

void DataEngine::setTimeStamp(bool value) {

	if (io.timeStamp == value) return;

	QMutexLocker locker(&io.mutex);
	io.timeStamp = value;
	//io.control_out[4] &= 0xc7;
	//RRK io.control_out[4] |= value << 6;

	if (value)
		DATA_ENGINE_DEBUG << "set time stamp on";
	else
		DATA_ENGINE_DEBUG << "set time stamp off";
}

void DataEngine::setRxSocketState(int rx, const char* prop, QString str) {

	RX[rx]->setProperty(prop, str);
}

void DataEngine::setRxPeerAddress(int rx, QHostAddress address) {

	RX[rx]->setPeerAddress(address);
}

void DataEngine::setRx(int rx) {

	QMutexLocker locker(&io.mutex);
	RX[rx]->setReceiver(rx);
}

void DataEngine::setRxClient(int rx, int client) {

	QMutexLocker locker(&io.mutex);
	RX[rx]->setClient(client);
}

void DataEngine::setClientConnected(int rx) {

	if (!io.clientList.contains(rx)) {

		io.clientList.append(rx);
		io.audio_rx = rx;

		m_AudioRcvrThread->quit();
		m_AudioRcvrThread->wait();
		m_AudioRcvrThread->start();
	}
	else {

		io.sendIQ_toggle = true;
		// For Protocol 2, clearing rcveIQ_toggle here can immediately drop
		// the Run bit after startup and prevent RX from ever starting.
		if (set->getCurrentMetisCard().protocol != 2) {
			io.rcveIQ_toggle = false;
		}
		m_AudioRcvrThread->start();
	}
}

void DataEngine::setClientConnected(bool value) {

	m_clientConnected = value;
}

void DataEngine::setClientDisconnected(int client) {

	Q_UNUSED(client)
	/*if (m_clientConnected) {

		m_AudioRcvrThread->quit();
		m_AudioRcvrThread->wait();
		if (!m_AudioRcvrThread->isRunning())
			DATA_ENGINE_DEBUG << "audio receiver thread stopped.";

		m_clientConnected = false;		
	}
	sync_toggle = true;
	adc_toggle = false;*/
}

//void DataEngine::setAudioInProcessorRunning(bool value) {
//
//	//m_audioInProcessorRunning = value;
//}

void DataEngine::setAudioReceiver(int rx) {

	QMutexLocker locker(&io.mutex);
	emit audioRxEvent(rx);
}

void DataEngine::setIQPort(int rx, int port) {

	QMutexLocker locker(&io.mutex);
	RX[rx]->setIQPort(port);
}

void DataEngine::setRxConnectedStatus(int rx, bool value) {

	QMutexLocker locker(&io.mutex);
	RX[rx]->setConnectedStatus(value);
}

void DataEngine::setHamBand(int rx, bool byBtn, HamBand band) {

	Q_UNUSED(rx)
	Q_UNUSED(byBtn)

	QMutexLocker locker(&io.mutex);
	io.ccTx.currentBand = band;

	if (set->getCurrentMetisCard().protocol == 2 && m_dataProcessor) {
		QMetaObject::invokeMethod(m_dataProcessor,
			&DataProcessor::requestProtocol2HPUpdate,
			Qt::QueuedConnection);
	}
}

void DataEngine::setFrequency(int mode, int rx, qint64 frequency) {

	Q_UNUSED(mode)
	Q_UNUSED(frequency)

	io.rx_freq_change = rx;
}

void DataEngine::applySliceDspMode(int rx, DSPMode mode)
{
	dspModeChanged(rx, mode);
}

void DataEngine::suspend() {


}

// *********************************************************************
// Data processor

DataProcessor::DataProcessor(
					DataEngine *de, 
					QSDR::_ServerMode serverMode,
					QSDR::_HWInterfaceMode hwMode)
	: QObject()
	, de(de)
	, set(Settings::instance())
        
	, m_serverMode(serverMode)
	, m_hwInterface(hwMode)
	, m_socketConnected(false)
	, m_setNetworkDeviceHeader(true)
	, m_bytes(0)
    , m_idx(IO_HEADER_SIZE)
    , m_sendState(1) // start at DDC Specific; state 0 (GP) is sent explicitly at startup
	, m_offset(0)
	, m_length(0)
	, m_sendSequence(0)
	, m_oldSendSequence(0)
	, m_stopped(false)
{
	m_IQSequence = 0L;
	m_sequenceHi = 0L;
	
	m_IQDatagram.resize(0);

	m_SyncChangedTime.start();
	m_ADCChangedTime.start();

    InitCPX(m_iq_output_buffer, DSP_SAMPLE_SIZE, 0.0f);

    //socket = new QUdpSocket();
	m_deviceAddress = set->getCurrentMetisCard().ip_address;

    m_controlTimer = new QTimer(this);
    connect(m_controlTimer, &QTimer::timeout, this, &DataProcessor::encodeCCBytes);

    file = new QFile("data.txt");
    file->open(QIODevice::ReadWrite);

#ifdef HAVE_CODEC2
	m_freeDVTxMode = 0;
	m_freeDVTx = freedv_open(m_freeDVTxMode);
	if (!m_freeDVTx) {
		DATA_PROCESSOR_DEBUG << "FreeDV TX disabled: failed to open mode" << m_freeDVTxMode;
	} else {
		m_freeDVSpeechRate = freedv_get_speech_sample_rate(m_freeDVTx);
		m_freeDVModemRate = freedv_get_modem_sample_rate(m_freeDVTx);
		m_freeDVTxNSpeech = freedv_get_n_speech_samples(m_freeDVTx);
		m_freeDVTxNModem = freedv_get_n_nom_modem_samples(m_freeDVTx);
		m_freeDVTxDecim = qMax(1, 48000 / qMax(1, m_freeDVSpeechRate));
		m_freeDVTxInterp = qMax(1, 48000 / qMax(1, m_freeDVModemRate));
		m_freeDVSpeechFrame.resize(m_freeDVTxNSpeech);
		m_freeDVModemFrame.resize(m_freeDVTxNModem);
		DATA_PROCESSOR_DEBUG << "FreeDV TX enabled: mode=" << m_freeDVTxMode
			<< " speechRate=" << m_freeDVSpeechRate
			<< " modemRate=" << m_freeDVModemRate
			<< " nSpeech=" << m_freeDVTxNSpeech
			<< " nModem=" << m_freeDVTxNModem;
	}
#endif

}

DataProcessor::~DataProcessor() {
#ifdef HAVE_CODEC2
	if (m_freeDVTx) {
		freedv_close(m_freeDVTx);
		m_freeDVTx = nullptr;
	}
#endif
    if (file) {
    file->close();
        delete file;  // Add this
        file = nullptr;
    }
}

#ifdef HAVE_CODEC2
void DataProcessor::applyCodec2ToMicBuffer(int sampleCount)
{
	if (sampleCount <= 0) return;

	// Use the selected Codec2 mode directly (0=1600, 1=1400, 2=1300, 3=700C, 4=2400, 5=3200, 6=700D)
	const int wantedMode = set->getFreeDVMode(de->io.currentReceiver);
	if (!m_freeDVTx || m_freeDVTxMode != wantedMode) {
		if (m_freeDVTx) {
			freedv_close(m_freeDVTx);
			m_freeDVTx = nullptr;
		}

		m_freeDVTx = freedv_open(wantedMode);
		m_freeDVTxMode = wantedMode;
		m_freeDVSpeechAccum.clear();
		m_freeDVModemQueue.clear();
		m_freeDVModemReadPos = 0;
		m_freeDVTxHoldCount = 0;
		m_freeDVTxHeldSample = 0;

		if (!m_freeDVTx) {
			DATA_PROCESSOR_DEBUG << "FreeDV TX reopen failed for mode" << wantedMode;
			return;
		}

		m_freeDVSpeechRate = freedv_get_speech_sample_rate(m_freeDVTx);
		m_freeDVModemRate = freedv_get_modem_sample_rate(m_freeDVTx);
		m_freeDVTxNSpeech = freedv_get_n_speech_samples(m_freeDVTx);
		m_freeDVTxNModem = freedv_get_n_nom_modem_samples(m_freeDVTx);
		m_freeDVTxDecim = qMax(1, 48000 / qMax(1, m_freeDVSpeechRate));
		m_freeDVTxInterp = qMax(1, 48000 / qMax(1, m_freeDVModemRate));
		m_freeDVSpeechFrame.assign(m_freeDVTxNSpeech, 0);
		m_freeDVModemFrame.assign(m_freeDVTxNModem, 0);
	}

	if (!m_freeDVTx || m_freeDVTxNSpeech <= 0 || m_freeDVTxNModem <= 0) return;

	for (int i = 0; i + m_freeDVTxDecim <= sampleCount; i += m_freeDVTxDecim) {
		double sum = 0.0;
		for (int k = 0; k < m_freeDVTxDecim; ++k) {
			sum += mic_buffer[(i + k) * 2];
		}
		double s = sum / static_cast<double>(m_freeDVTxDecim);
		if (s > 1.0) s = 1.0;
		if (s < -1.0) s = -1.0;
		m_freeDVSpeechAccum.push_back(static_cast<int16_t>(s * 32767.0));
	}

	quint64 txFramesThisBlock = 0;
	static quint64 txSpeechLevelLogCounter = 0;
	while (static_cast<int>(m_freeDVSpeechAccum.size()) >= m_freeDVTxNSpeech) {
		double sumSq = 0.0;
		double peak = 0.0;
		for (int i = 0; i < m_freeDVTxNSpeech; ++i) {
			m_freeDVSpeechFrame[i] = m_freeDVSpeechAccum[i];
			double norm = static_cast<double>(m_freeDVSpeechFrame[i]) / 32768.0;
			sumSq += norm * norm;
			double absNorm = std::abs(norm);
			if (absNorm > peak)
				peak = absNorm;
		}
		m_freeDVSpeechAccum.erase(m_freeDVSpeechAccum.begin(), m_freeDVSpeechAccum.begin() + m_freeDVTxNSpeech);

		if ((++txSpeechLevelLogCounter % 50) == 0) {
			double rms = std::sqrt(sumSq / static_cast<double>(m_freeDVTxNSpeech));
			DATA_PROCESSOR_DEBUG << "FreeDV TX speech level: rms=" << rms
			                     << " peak=" << peak
			                     << " mode=" << m_freeDVTxMode;
		}

		freedv_tx(m_freeDVTx, m_freeDVModemFrame.data(), m_freeDVSpeechFrame.data());
		m_freeDVModemQueue.insert(m_freeDVModemQueue.end(), m_freeDVModemFrame.begin(), m_freeDVModemFrame.end());
		txFramesThisBlock += 1;
	}

	for (int i = 0; i < sampleCount; ++i) {
		if (m_freeDVTxHoldCount <= 0) {
			if (m_freeDVModemReadPos < m_freeDVModemQueue.size()) {
				m_freeDVTxHeldSample = m_freeDVModemQueue[m_freeDVModemReadPos++];
			} else {
				m_freeDVTxHeldSample = 0;
			}
			m_freeDVTxHoldCount = m_freeDVTxInterp;
		}

		mic_buffer[i * 2] = static_cast<double>(m_freeDVTxHeldSample) / 32768.0;
		mic_buffer[i * 2 + 1] = 0.0;
		m_freeDVTxHoldCount -= 1;
	}

	if (txFramesThisBlock > 0) {
		set->addFreeDVTxFrames(de->io.currentReceiver, txFramesThisBlock);
	}

	if (m_freeDVModemReadPos > 0 && m_freeDVModemReadPos * 2 >= m_freeDVModemQueue.size()) {
		m_freeDVModemQueue.erase(m_freeDVModemQueue.begin(), m_freeDVModemQueue.begin() + m_freeDVModemReadPos);
		m_freeDVModemReadPos = 0;
	}
}
#endif

void DataProcessor::stop() {

	m_stopped = true;
}

void DataProcessor::startControlTimer() {
    if (de->set->getCurrentMetisCard().protocol == 2 && m_controlTimer) {
        m_controlTimer->start(10);
    }
}

void DataProcessor::stopControlTimer() {
	if (m_controlTimer && m_controlTimer->isActive()) {
		m_controlTimer->stop();
	}
}

void DataProcessor::requestProtocol2HPUpdate() {
	if (!de || !de->m_protocol || !de->m_controlSocket) return;
	if (de->set->getCurrentMetisCard().protocol != 2) return;

	if (!de->io.rcveIQ_toggle) return; // don't send HP mid-setup (Run not yet asserted)

	unsigned char p2CmdBuf[1444];
	quint16 port = DEVICE_PORT;
	int hpState = 3; // High Priority Data Packet

	memset(p2CmdBuf, 0, sizeof(p2CmdBuf));
	de->m_protocol->encodeCCBytes(p2CmdBuf, &de->io, hpState, port);

	if (port != 1027) return;

	m_deviceAddress = de->io.hpsdrDeviceIPAddress;
	qint64 sent = -1;
	if (de->m_dataIO) {
		QByteArray dg((const char*)p2CmdBuf, 1444);
		QMetaObject::invokeMethod(
			de->m_dataIO,
			"sendProtocol2ControlDatagram",
			Qt::BlockingQueuedConnection,
			Q_RETURN_ARG(qint64, sent),
			Q_ARG(QByteArray, dg),
			Q_ARG(QHostAddress, m_deviceAddress),
			Q_ARG(quint16, port));
	}
	if (sent < 0 && de->m_controlSocket) {
		de->m_controlSocket->writeDatagram((const char*)p2CmdBuf, 1444, m_deviceAddress, port);
	}
}

void DataProcessor::requestProtocol2DDCUpdate() {
	if (!de || !de->m_protocol || !de->m_controlSocket) return;
	if (de->set->getCurrentMetisCard().protocol != 2) return;

	unsigned char p2CmdBuf[1444];
	quint16 port = DEVICE_PORT;
	int oneShotState = 1; // DDC Specific packet

	memset(p2CmdBuf, 0, sizeof(p2CmdBuf));
	de->m_protocol->encodeCCBytes(p2CmdBuf, &de->io, oneShotState, port);

	if (port != 1025) {
		DATA_PROCESSOR_DEBUG << "P2 rate update produced unexpected control port" << port;
		return;
	}

	m_deviceAddress = de->io.hpsdrDeviceIPAddress;
	if (de->m_controlSocket->writeDatagram((const char*)p2CmdBuf, 1444, m_deviceAddress, port) < 0) {
		DATA_PROCESSOR_DEBUG << "error sending P2 DDC rate update:" << de->m_controlSocket->errorString();
	}
}

void DataProcessor::requestProtocol2ReceiverSetup() {
    if (!de || !de->m_protocol || !de->m_controlSocket) return;
    if (de->set->getCurrentMetisCard().protocol != 2) return;

	DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] enter: controlSocketLocalPort="
	                    << de->m_controlSocket->localPort()
	                    << " device=" << de->io.hpsdrDeviceIPAddress;

	// Keep Run low during setup staging to avoid early HP assertions from
	// concurrent control paths; raise it only for the explicit start HP packet.
	de->io.rcveIQ_toggle = false;

    unsigned char p2CmdBuf[1444];
    quint16 port = DEVICE_PORT;

	auto sendP2Control = [&](const char *data, int len, quint16 dstPort) -> qint64 {
		if (de->m_dataIO) {
			QByteArray dg(data, len);
			qint64 sent = -1;
			QMetaObject::invokeMethod(
				de->m_dataIO,
				"sendProtocol2ControlDatagram",
				Qt::BlockingQueuedConnection,
				Q_RETURN_ARG(qint64, sent),
				Q_ARG(QByteArray, dg),
				Q_ARG(QHostAddress, m_deviceAddress),
				Q_ARG(quint16, dstPort));
			if (sent >= 0) {
				return sent;
			}
		}
		return de->m_controlSocket->writeDatagram(data, len, m_deviceAddress, dstPort);
	};

    // 0. Send General packet (port 1024) — must arrive before HP packets so
    //    newhpsdrsim sets alex0_enable=1 (byte 59 bit 0) and processes alex0 bits.
    int genState = 0;
    memset(p2CmdBuf, 0, sizeof(p2CmdBuf));
    de->m_protocol->encodeCCBytes(p2CmdBuf, &de->io, genState, port);
    m_deviceAddress = de->io.hpsdrDeviceIPAddress;
    sendP2Control((const char*)p2CmdBuf, 60, port); // General packet is 60 bytes

    // 1. Send DDC Specific packet (port 1025) with updated receiver-count/bitmask.
	int ddcState = 1;
    memset(p2CmdBuf, 0, sizeof(p2CmdBuf));
    de->m_protocol->encodeCCBytes(p2CmdBuf, &de->io, ddcState, port);
    if (port != 1025) {
        DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] unexpected DDC port" << port << "(expected 1025)";
        return;
    }
    m_deviceAddress = de->io.hpsdrDeviceIPAddress;
	qint64 ddcSent = sendP2Control((const char*)p2CmdBuf, 1444, port);
	if (ddcSent < 0) {
        DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] DDC Specific send FAILED:" << de->m_controlSocket->errorString();
	} else {
		DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] DDC Specific sent: bytes=" << ddcSent << " port=" << port;
	}

	// 2. Send TX Specific packet (port 1026). Some simulator/device builds
	// expect the full DDC+TX+HP setup sequence before honoring Run.
	int txState = 2;
	memset(p2CmdBuf, 0, sizeof(p2CmdBuf));
	de->m_protocol->encodeCCBytes(p2CmdBuf, &de->io, txState, port);
	if (port != 1026) {
		DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] unexpected TX port" << port << "(expected 1026)";
		return;
	}
	qint64 txSent = sendP2Control((const char*)p2CmdBuf, 60, port);
	if (txSent < 0) {
		DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] TX Specific send FAILED:" << de->m_controlSocket->errorString();
	} else {
		DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] TX Specific sent: bytes=" << txSent << " port=" << port;
	}

	// 3. Send High Priority packet with Run=0 first so DDC frequencies are
	// latched before Run is asserted.
	int hpState = 3;
	de->io.rcveIQ_toggle = false;
	memset(p2CmdBuf, 0, sizeof(p2CmdBuf));
	de->m_protocol->encodeCCBytes(p2CmdBuf, &de->io, hpState, port);
	if (port != 1027) {
		DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] unexpected HP port" << port << "(expected 1027)";
		return;
	}
	qint64 hpSentRun0 = sendP2Control((const char*)p2CmdBuf, 1444, port);
	if (hpSentRun0 < 0) {
		DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] HP Run=0 send FAILED:" << de->m_controlSocket->errorString();
	} else {
		DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] HP Run=0 sent: bytes=" << hpSentRun0 << " port=" << port;
	}

	// 4. Assert Run=1 as the final startup packet.
	QThread::msleep(5);
	de->io.rcveIQ_toggle = true;
	quint16 runPort = DEVICE_PORT;
	QByteArray runDatagram = de->m_protocol->formatStartStop(1, runPort);
	qint64 runSent = sendP2Control(runDatagram.constData(), runDatagram.size(), runPort);
	if (runSent < 0) {
		DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] final Run=1 send FAILED on port" << runPort << ":" << de->m_controlSocket->errorString();
		return;
	} else {
		DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] final Run=1 sent: bytes=" << runSent << " port=" << runPort;
	}

	// The simulator starts DDC/TX receiver threads only after Run=1 is seen.
	// Re-send DDC/TX setup in a short burst so newly spawned threads reliably
	// receive and latch enable/rate config.
	for (int attempt = 0; attempt < 20; ++attempt) {
		QThread::msleep(50);

		int ddcResendState = 1;
		port = DEVICE_PORT;
		memset(p2CmdBuf, 0, sizeof(p2CmdBuf));
		de->m_protocol->encodeCCBytes(p2CmdBuf, &de->io, ddcResendState, port);
		if (port == 1025) {
			qint64 ddcSent2 = sendP2Control((const char*)p2CmdBuf, 1444, port);
			if (ddcSent2 < 0) {
				DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] post-run DDC send FAILED (attempt" << (attempt + 1) << "):" << de->m_controlSocket->errorString();
			} else {
				DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] post-run DDC sent: bytes=" << ddcSent2 << " port=" << port << " attempt=" << (attempt + 1);
			}
		}

		int txResendState = 2;
		port = DEVICE_PORT;
		memset(p2CmdBuf, 0, sizeof(p2CmdBuf));
		de->m_protocol->encodeCCBytes(p2CmdBuf, &de->io, txResendState, port);
		if (port == 1026) {
			qint64 txSent2 = sendP2Control((const char*)p2CmdBuf, 60, port);
			if (txSent2 < 0) {
				DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] post-run TX send FAILED (attempt" << (attempt + 1) << "):" << de->m_controlSocket->errorString();
			} else {
				DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] post-run TX sent: bytes=" << txSent2 << " port=" << port << " attempt=" << (attempt + 1);
			}
		}
	}

	DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] complete";
}

void DataProcessor::initDataProcessorSocket() {

}


void DataProcessor::displayDataProcessorSocketError(QAbstractSocket::SocketError error) {

	DATA_PROCESSOR_DEBUG << "data processor socket error: " << error;
}

void DataProcessor::processDeviceData() {

	DATA_PROCESSOR_DEBUG << "Data Processor thread: " << this->thread();
	forever {

		TIQPacket packet = de->io.iq_queue.dequeue();
		processInputBuffer(packet.payload, packet.sourcePort);

		if (de->io.iq_queue.isFull()) {
			DATA_PROCESSOR_DEBUG << "IQ queue full!";
		}

		QMutexLocker locker(&m_mutex);
		if (m_stopped) {
			m_stopped = false;
			break;
		}
	}
}


void DataProcessor::processInputBuffer(const QByteArray &buffer, quint16 sourcePort) {
    if (de->m_protocol) {
		de->m_protocol->processInputBuffer(buffer, de, sourcePort);
        if (de->io.ccRx.dash != de->io.ccRx.previous_dash) emit keyer_event(0, de->io.ccRx.dash);
        if (de->io.ccRx.dot != de->io.ccRx.previous_dot) emit keyer_event(1, de->io.ccRx.dot);
    }
}

void DataProcessor::decodeCCBytes(const QByteArray &buffer) {
    if (de->m_protocol) {
        de->m_protocol->decodeCCBytes(buffer, &de->io);
    }
}

void DataProcessor::setOutputBuffer(int rx, const CPX &buffer) {

    if (rx == de->io.currentReceiver) {
		processOutputBuffer(buffer);

	}
}

// full_txBuffer() fires for both P1 and P2 when the output_buffer is full (every 63 samples).
// For P2, writeData() now sends a proper 1444-byte DUC IQ packet to port 1029.
// sendAudio() handles RX audio → sound card (same for both protocols).
void DataProcessor::full_txBuffer(){

    encodeCCBytes();
    switch (m_hwInterface) {

        case QSDR::Metis:
        case QSDR::Hermes:

            de->io.audioDatagram.resize(IO_BUFFER_SIZE);
            de->io.audioDatagram = QByteArray::fromRawData((const char *)&de->io.output_buffer, IO_BUFFER_SIZE);
            de->m_dataIO->sendAudio(de->io.output_buffer); //RRK
            writeData();
            break;

        case QSDR::NoInterfaceMode:
            break;
#ifdef HAVE_SOAPYSDR
        case QSDR::SoapySDR:
            break;
#endif
    }
    m_idx = IO_HEADER_SIZE;

}

void DataProcessor::buffer_tx_data()
{
    de->io.output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
    de->io.output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
    de->io.output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
    de->io.output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];

}

void DataProcessor::add_rx_audio_sample() {
        qint16 leftRXSample;
        qint16 rightRXSample;
		const DSPMode rxMode = set->getDSPMode(de->io.currentReceiver);
		if (rxMode == FDV) {
			leftRXSample = 0;
			rightRXSample = 0;
		} else {
			leftRXSample = (qint16) (rx_audio_buffer[rx_audio_ptr].re * 32767.0f);
			rightRXSample = (qint16) (rx_audio_buffer[rx_audio_ptr].im * 32767.0f);
		}
        de->io.output_buffer[m_idx++] = leftRXSample >> 8;
        de->io.output_buffer[m_idx++] = leftRXSample;
        de->io.output_buffer[m_idx++] = rightRXSample >> 8;
        de->io.output_buffer[m_idx++] = rightRXSample;
        rx_audio_ptr++;
    }

/* Sends RX Audio and tx iq data back to hpsdr. Always at 48 KHz bandwidth */

void DataProcessor::send_hpsdr_data(int rx, const CPX &buffer, int buffersize) {
    // Only send audio for the currently selected receiver.
    if (rx != de->io.currentReceiver) return;
    rx_audio_ptr = 0;
/* buffer rx audio */
    for (int j = 0; j < buffersize; j++)
        {
        rx_audio_buffer[j].re = buffer[j].re;
        rx_audio_buffer[j].im = buffer[j].im;
        }

    if (set->is_transmitting()) {
        if (!tx_index) get_tx_iqData();
    } else memset(&m_tx_iq_Buffer, 0x0, sizeof(m_tx_iq_Buffer));
        while (rx_audio_ptr  <   buffersize) {
        add_rx_audio_sample();
        add_mic_sample();

        if (m_idx == IO_BUFFER_SIZE) {
            full_txBuffer();
        }

    }
    rx_audio_ptr=0;
}


void DataProcessor::add_audio_sample(qint16 leftRXSample, qint16 rightRXSample)
{
    Q_UNUSED(leftRXSample)
    Q_UNUSED(rightRXSample)
    buffer_tx_data();
    if (m_idx == IO_BUFFER_SIZE)
    {
        full_txBuffer();
        m_idx =8;
    }
    if (tx_index >= 4096) tx_index = 0;
}


void DataProcessor::add_tx_iq_sample(double i, double q)
{
    long   leftTXSample;
    long   rightTXSample;
    double gain = 32767.0f;

    rightTXSample = i >= 0.0 ? (long) floor(i * gain + 0.5) : (long) ceil(i * gain - 0.5);
    leftTXSample =  q >= 0.0 ? (long) floor(q * gain + 0.5) : (long) ceil(q * gain - 0.5);
    buffer_tx_iq_sample(leftTXSample,rightTXSample);

}

void DataProcessor::buffer_tx_iq_sample(int i, int q)
{
    m_tx_iq_Buffer[m_idx++] = i >> 8;
    m_tx_iq_Buffer[m_idx++] = i;
    m_tx_iq_Buffer[m_idx++] = q >> 8;
    m_tx_iq_Buffer[m_idx++] = q;
   }



void DataProcessor::processMicData() {
    
    AUDIOBUF temp_data;
    int queueCount = de->m_audioInput->m_faudioInQueue.count();
    
    if (queueCount > 0)
    {
	    qDebug() << "processMicData called, queue count:" << queueCount;

        temp_data = de->m_audioInput->m_faudioInQueue.dequeue();
        // Only process the actual number of samples in the buffer
        int numSamples = qMin((int)temp_data.size(), DSP_SAMPLE_SIZE);
        for (int s = 0; s < numSamples; s++)
        {
            mic_buffer[(s * 2)]  = temp_data[s];
            mic_buffer[(s * 2) + 1] = 0.0f;
        }

		qDebug() << "Mic buffer processed with " << numSamples << " samples." << mic_buffer[0] << mic_buffer[1];
    }
    else {
        temp_data.clear();
        memset(&mic_buffer, 0x0, sizeof(mic_buffer));
    }
    mic_buffer_index = 0;

}

void DataProcessor::add_mic_sample()
{
 //    de->io.output_buffer[m_idx++] = 0;
  //  de->io.output_buffer[m_idx++] = 0;
  //  de->io.output_buffer[m_idx++] = 0;
  //  de->io.output_buffer[m_idx++] = 0;
    de->io.output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
    de->io.output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
    de->io.output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
    de->io.output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
    if (tx_index >= 4096) tx_index = 0;
}

/* cw code from pihpsdr */
double DataProcessor::get_cwsample() {
    static int cw_not_ready =1;
    static int cw_shape;
    int cw_key_up = 0;
    int cw_key_down = 0;


//
//	We HAVE TO shape the signal to avoid hard clicks to be
//	heard way beside our frequency. The envelope (ramp function)
//      is stored in cwramp48[0::RAMPLEN], so we "move" cw_shape between these
//      values. The ramp width is RAMPLEN/48000 seconds.
//
//      In the new protocol, we use this ramp for the side tone, but
//      must use values from cwramp192 for the TX iq signal.
//
//      Note that usually, the pulse is much broader than the ramp,
//      that is, cw_key_down and cw_key_up are much larger than RAMPLEN.
//
        if (cw_not_ready)             qDebug() << QTime::currentTime().msec() <<" cw key down";

    cw_not_ready=0;
        if (de->cw_key_down > 0 ) {
            if (cw_shape < RAMPLEN) cw_shape++;	// walk up the ramp
            cw_key_down--;			// decrement key-up counter
        } else {
            // dig into this even if cw_key_up is already zero, to ensure
            // that we reach the bottom of the ramp for very small pauses
            if (cw_shape > 0) cw_shape--;	// walk down the ramp
            if (cw_key_up > 0) cw_key_up--; // decrement key-down counter
        }

        return cwramp48[cw_shape] * 100;

}

void DataProcessor::send_mic_data() {
    int error;
    long   leftTXSample;
    long   rightTXSample;
    double is,qs;
    double gain = 32767.0f;
    // double gain = 25 * 0.00392;
    static AUDIOBUF a;
    get_cwsample();

    if ( de->io.ccTx.mox ||  de->io.ccTx.ptt ) {

        fexchange0(TX_ID, a.data(), (double *) m_iq_output_buffer.data(), &error);
        Spectrum0(1, TX_ID, 0, 0, (double *) m_iq_output_buffer.data());

        for (int j = 0; j < DSP_SAMPLE_SIZE; j++) {
            qs = m_iq_output_buffer.at(j).re;
            is = m_iq_output_buffer.at(j).im;
            rightTXSample = is >= 0.0 ? (long) floor(is * gain + 0.5) : (long) ceil(is * gain - 0.5);
            leftTXSample = qs >= 0.0 ? (long) floor(qs * gain + 0.5) : (long) ceil(qs * gain - 0.5);
            buffer_tx_iq_sample(leftTXSample, rightTXSample);
        }


    }
    mic_buffer_index = 0;
}


void DataProcessor::fetch_MicData(){
	int numSamples = 0;
    AUDIOBUF temp_data;
    if (de->m_audioInput->m_faudioInQueue.count() > 0)
    {
        temp_data = de->m_audioInput->m_faudioInQueue.dequeue();

		numSamples = qMin((int)temp_data.size(), (int)DSP_SAMPLE_SIZE);
        for (int s = 0; s < numSamples; s++)
        {
            mic_buffer[(s * 2 )]  = temp_data[s] ;
            mic_buffer[(s * 2 ) + 1 ] = 0.0f;
        }
        // Zero-fill remaining if buffer was short
        for (int s = numSamples; s < DSP_SAMPLE_SIZE; s++) {
            mic_buffer[(s * 2)] = 0.0f;
            mic_buffer[(s * 2) + 1] = 0.0f;
        }
    }
    else{
        temp_data.clear();
        memset(&mic_buffer,0x0,sizeof(mic_buffer));
    }

	// Keep WSJT-X digital-input handling (DIGU/DIGL) separate.
	// DV routing is only active for the FreeDV mode button (mapped to FDV).
	const DSPMode txMode = set->getDSPMode(de->io.currentReceiver);
	if (txMode == FDV) {
#ifdef HAVE_CODEC2
		applyCodec2ToMicBuffer(numSamples);
#endif
	}

    mic_buffer_index = 0;

}

/*  processes mic samples ready to transmit */
void DataProcessor::get_tx_iqData(){
    int error;
    long int   leftTXSample;
    long int rightTXSample;
    double is,qs;
    double gain = 32767.0f;
   // double gain = 25 * 0.00392;
    fetch_MicData();

    if ( de->io.ccTx.mox ||  de->io.ccTx.ptt ) {
        fexchange0(TX_ID, mic_buffer, (double *) m_iq_output_buffer.data(), &error);

		Spectrum0(1, TX_ID, 0, 0, (double *) m_iq_output_buffer.data());


/* Queue the tx data */
        int idx = 0;
        for (int j = 0; j < DSP_SAMPLE_SIZE; j++) {
            is = m_iq_output_buffer.at(j).re;
            qs = m_iq_output_buffer.at(j).im;
            leftTXSample = is >= 0.0 ? (long) floor(is * gain + 0.5) : (long) ceil(is * gain - 0.5);
            rightTXSample =  qs >= 0.0 ? (long) floor(qs * gain + 0.5) : (long) ceil(qs * gain - 0.5);
            m_tx_iq_Buffer[idx++] = (int)leftTXSample >> 8;
            m_tx_iq_Buffer[idx++] = (int)leftTXSample;
            m_tx_iq_Buffer[idx++] = (int)rightTXSample >> 8;
            m_tx_iq_Buffer[idx++] = (int)rightTXSample;
        }
    }
}

/* copied from pihpsdr */
void DataProcessor::DumpBuffer(unsigned char *buffer,int length, const char *who) {
  QMutex dump_mutex;
  dump_mutex.lock();
  printf("%s: %s: %d\n",__FUNCTION__,who,length);
  int i=0;
  int line=0;

  while(i<length) {

    printf("%02X",buffer[i]);
    i++;
    line++;
    if(line==16) {
      printf("\n");
      line=0;
    }
  }
  if(line!=0) {
    printf("\n");
  }
  printf("\n");
  dump_mutex.unlock();
}


/* Sends rx audio data from wdsp to  hermes audio and to PC */
void DataProcessor::setAudioBuffer(int rx, const CPX &buffer, int buffersize)
{
    //DATA_PROCESSOR_DEBUG << "processOutputBuffer: " << this->thread();
    Q_UNUSED(rx)

    qint16 leftRXSample;
    qint16 rightRXSample;

	// process the output
	const DSPMode rxMode = set->getDSPMode(de->io.currentReceiver);
	const bool muteAnalogRxForCodec2 = (rxMode == FDV);
    if (tx_index == 0)  get_tx_iqData();
        for (int j = 0; j < buffersize; j++) {

			if (muteAnalogRxForCodec2) {
				leftRXSample = 0;
				rightRXSample = 0;
			} else {
				leftRXSample  = (qint16)(buffer.at(j).re * 32767.0f);
				rightRXSample = (qint16)(buffer.at(j).im * 32767.0f);
			}

            de->io.output_buffer[m_idx++] = leftRXSample  >> 8;
            de->io.output_buffer[m_idx++] = leftRXSample;
            de->io.output_buffer[m_idx++] = rightRXSample >> 8;
            de->io.output_buffer[m_idx++] = rightRXSample;
            de->io.output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
            de->io.output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
            de->io.output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
            de->io.output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];

            if (tx_index >= 4096) tx_index = 0;

 //   qDebug() << "buffer " << de->io.output_buffer[IO_HEADER_SIZE ] << de->io.output_buffer[IO_BUFFER_SIZE - 1] ;
        if (m_idx == IO_BUFFER_SIZE) {

                //if (de->m_audioBuffer.length() == 1024) {

            //	//m_audioEngine->setAudioBuffer(this, m_audioBuffer);
            //	de->m_audioBuffer.resize(0);
            //}
            // set the C&C bytes
			encodeCCBytes();

            switch (m_hwInterface) {

				case QSDR::Metis:
				case QSDR::Hermes:

					de->io.audioDatagram.resize(IO_BUFFER_SIZE);
					de->io.audioDatagram = QByteArray::fromRawData((const char *)&de->io.output_buffer, IO_BUFFER_SIZE);

					//if (m_dataIOThreadRunning) {
					//	de->m_dataIO->writeData();
					//}

                    if ( de->io.ccTx.mox ||  de->io.ccTx.ptt )
                    {
                        /*
                       int val =   ((de->io.output_buffer[3]) &0xfe) >> 1;
                       qDebug() << "command" << val;
                       qDebug() << "C[0] " << " " << bin << de->io.output_buffer[3];
                       qDebug() << "C[1] " << " " << bin <<de->io.output_buffer[4];
                       qDebug() << "C[2] " << " " << bin <<de->io.output_buffer[5];
                       qDebug() << "C[3] " << " " << bin <<de->io.output_buffer[6];
                       qDebug() << "\n";
                         */

                    }

                    de->m_dataIO->sendAudio(de->io.output_buffer); //RRK

					writeData();
					break;

				case QSDR::NoInterfaceMode:
#ifdef HAVE_SOAPYSDR
				case QSDR::SoapySDR:
#endif
					break;
            }
        m_idx = IO_HEADER_SIZE;
         }
       }
}




/* Sends rx audio data from wdsp to  hermes audio and to PC */
void DataProcessor::setAudioBuffer_old(int rx, const CPX &buffer, int buffersize)
{
    Q_UNUSED(rx)


//    qDebug() << "Buffer Size" << buffersize;
    QTextStream stream( this->file );
    qint16 leftRXSample;
    qint16 rightRXSample;
	// process the output
	const DSPMode rxMode = set->getDSPMode(de->io.currentReceiver);
	const bool muteAnalogRxForCodec2 = (rxMode == FDV);
    if (tx_index == 0)  get_tx_iqData();
    for (int j = 0; j < buffersize; j++) {

		if (muteAnalogRxForCodec2) {
			leftRXSample = 0;
			rightRXSample = 0;
		} else {
			leftRXSample  = (qint16)(buffer.at(j).re * 32767.0f);
			rightRXSample = (qint16)(buffer.at(j).im * 32767.0f);
		}
        de->io.output_buffer[m_idx++] = leftRXSample  >> 8;
        de->io.output_buffer[m_idx++] = leftRXSample;
        de->io.output_buffer[m_idx++] = rightRXSample >> 8;
        de->io.output_buffer[m_idx++] = rightRXSample;
        de->io.output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
        de->io.output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
        de->io.output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
        de->io.output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];

        if (tx_index >= 4096) tx_index = 0;





        //   qDebug() << "buffer " << de->io.output_buffer[IO_HEADER_SIZE ] << de->io.output_buffer[IO_BUFFER_SIZE - 1] ;
        if (m_idx == IO_BUFFER_SIZE) {
            encodeCCBytes();
            switch (m_hwInterface) {

                case QSDR::Metis:
                case QSDR::Hermes:

                    de->io.audioDatagram.resize(IO_BUFFER_SIZE);
                    de->io.audioDatagram = QByteArray::fromRawData((const char *)&de->io.output_buffer, IO_BUFFER_SIZE);


                    //if (m_dataIOThreadRunning) {
                    //	de->m_dataIO->writeData();
                    //}

                    if ( de->io.ccTx.mox ||  de->io.ccTx.ptt )
                    {
                        /*
                       int val =   ((de->io.output_buffer[3]) &0xfe) >> 1;
                       qDebug() << "command" << val;
                       qDebug() << "C[0] " << " " << bin << de->io.output_buffer[3];
                       qDebug() << "C[1] " << " " << bin <<de->io.output_buffer[4];
                       qDebug() << "C[2] " << " " << bin <<de->io.output_buffer[5];
                       qDebug() << "C[3] " << " " << bin <<de->io.output_buffer[6];
                       qDebug() << "\n";
                         */

                    }
                    qDebug() << "audio buffer sent";
                 //   de->m_dataIO->sendAudio(de->io.output_buffer); //RRK
                    writeData();
                    break;

                case QSDR::NoInterfaceMode:
#ifdef HAVE_SOAPYSDR
                case QSDR::SoapySDR:
#endif
                    break;
            }
            m_idx = IO_HEADER_SIZE;

        }
    }
    //   DATA_ENGINE_DEBUG << "TX QUEUE SIZE end " << m_tx_iqdata.size();
}



/* UNUSED */
void DataProcessor::processOutputBuffer(const CPX &buffer) {

	DATA_PROCESSOR_DEBUG << "processOutputBuffer: " << this->thread();

	qint16 leftRXSample;
    qint16 rightRXSample;
    qint16 leftTXSample;
    qint16 rightTXSample;

	// process the output
	for (int j = 0; j < BUFFER_SIZE; j++) {

		leftRXSample  = (qint16)(buffer.at(j).re * 32767.0f);
		rightRXSample = (qint16)(buffer.at(j).im * 32767.0f);

		leftTXSample = 0;
        rightTXSample = 0;

//    qDebug() << sizeof(de->io.output_buffer);

		de->io.output_buffer[m_idx++] = leftRXSample  >> 8;
        de->io.output_buffer[m_idx++] = leftRXSample;
        de->io.output_buffer[m_idx++] = rightRXSample >> 8;
        de->io.output_buffer[m_idx++] = rightRXSample;
        de->io.output_buffer[m_idx++] = leftTXSample  >> 8;
        de->io.output_buffer[m_idx++] = leftTXSample;
        de->io.output_buffer[m_idx++] = rightTXSample >> 8;
        de->io.output_buffer[m_idx++] = rightTXSample;

		if (m_idx == IO_BUFFER_SIZE) {

			//if (de->m_audioBuffer.length() == 1024) {

			//	//m_audioEngine->setAudioBuffer(this, m_audioBuffer);
			//	de->m_audioBuffer.resize(0);
			//}
			// set the C&C bytes
			encodeCCBytes();

			switch (m_hwInterface) {

				case QSDR::Metis:
				case QSDR::Hermes:

					de->io.audioDatagram.resize(IO_BUFFER_SIZE);
					de->io.audioDatagram = QByteArray::fromRawData((const char *)&de->io.output_buffer, IO_BUFFER_SIZE);

					//if (m_dataIOThreadRunning) {
					//	de->m_dataIO->writeData();
					//}

                 //   de->m_dataIO->sendAudio(de->io.output_buffer); //RRK
                    writeData();
					break;

				case QSDR::NoInterfaceMode:
#ifdef HAVE_SOAPYSDR
				case QSDR::SoapySDR:
#endif
					break;
			}
			m_idx = IO_HEADER_SIZE;
		}
	}
}

void DataProcessor::encodeCCBytes() {
    if (de->m_protocol) {
        quint16 port = DEVICE_PORT;

        if (de->set->getCurrentMetisCard().protocol == 2) {
            // Protocol 2 HP Data packet (port 1027) must be exactly 1444 bytes;
            // the hpsdrsim highprio_thread breaks out of its receive loop on any
            // other size, meaning run never becomes 1 and the RX threads never start.
            // Use a separate 1444-byte buffer so output_buffer (512 bytes) is not
            // overrun.  Packets for other P2 command ports (1024/1025/1026) are
            // still 60 bytes.
            static unsigned char p2CmdBuf[1444];
            memset(p2CmdBuf, 0, sizeof(p2CmdBuf));
            de->m_protocol->encodeCCBytes(p2CmdBuf, &de->io, m_sendState, port);
            m_deviceAddress = de->io.hpsdrDeviceIPAddress;
            // Per hpsdrsim packet-length checks:
            //   port 1025 (DDC Specific) : 1444 bytes
            //   port 1026 (DUC Specific) :   60 bytes
            //   port 1027 (HP Data)      : 1444 bytes
            const int sendSize = (port == 1025 || port == 1027) ? 1444 : 60;
			qint64 sent = -1;
			if (de->m_dataIO) {
				QByteArray dg((const char*)p2CmdBuf, sendSize);
				QMetaObject::invokeMethod(
					de->m_dataIO,
					"sendProtocol2ControlDatagram",
					Qt::BlockingQueuedConnection,
					Q_RETURN_ARG(qint64, sent),
					Q_ARG(QByteArray, dg),
					Q_ARG(QHostAddress, m_deviceAddress),
					Q_ARG(quint16, port));
			}
			if (sent < 0 && de->m_controlSocket) {
				sent = de->m_controlSocket->writeDatagram((const char*)p2CmdBuf, sendSize, m_deviceAddress, port);
			}
			if (sent < 0) {
				DATA_PROCESSOR_DEBUG << "error sending control data to device";
            }
        } else {
            de->m_protocol->encodeCCBytes(de->io.output_buffer, &de->io, m_sendState, port);
        }
    }
}


void DataProcessor::writeData() {
    if (!de->m_protocol) return;

    // Protocol 2: formatOutputPacket returns the complete 1444-byte DUC IQ packet.
    // Send it in a single call to port 1029; bypass the P1 two-call toggle.
    if (de->set->getCurrentMetisCard().protocol == 2) {
        QByteArray ducPkt = de->m_protocol->formatOutputPacket(de->io.audioDatagram, m_sendSequence);
        if (de->sendSocket->writeDatagram(ducPkt, m_deviceAddress, 1029) < 0) {
            DATA_PROCESSOR_DEBUG << "P2 TX: error sending DUC IQ:" << de->sendSocket->errorString();
        }
        m_oldSendSequence = m_sendSequence - 1; // keep tracking consistent
        return;
    }

	if (m_setNetworkDeviceHeader) {
        m_outDatagram = de->m_protocol->formatOutputPacket(de->io.audioDatagram, m_sendSequence);
        m_setNetworkDeviceHeader = false;
    }
	else {
		m_outDatagram += de->io.audioDatagram;

        quint16 dataPort = (de->set->getCurrentMetisCard().protocol == 2) ? 1029 : DEVICE_PORT;

		if (de->sendSocket->writeDatagram(m_outDatagram, m_deviceAddress, dataPort) < 0) {
			DATA_PROCESSOR_DEBUG << "error sending data to device: " << de->sendSocket->errorString();
		}

		if (m_sendSequence != m_oldSendSequence + 1) {
			DATA_PROCESSOR_DEBUG << "output sequence error: old = " << m_oldSendSequence << "; new =" << m_sendSequence;
		}

		m_oldSendSequence = m_sendSequence;
		m_setNetworkDeviceHeader = true;
    }
}

void 	DataEngine::setWbSpectrumAveraging(int rx, int value)
{
	if (m_wbDataProcessor) m_wbDataProcessor->setWbSpectrumAveraging(rx,value);
}


void DataEngine::setRepeaterMode(bool mode) {
        io.ccTx.use_repeaterOffset = mode;
}

void DataEngine::dspModeChanged(int rx, DSPMode mode){
    Q_UNUSED(rx);
    io.ccTx.mode = mode;
    TX.setDSPMode(1,mode);
}

void DataEngine::CwHangTimeChanged(int CwHangTime)
{
m_cw_hang_time = CwHangTime;
}

void DataEngine::CwSidetoneFreqChanged(int CwSidetoneFreq)
{
    m_cw_sidetone_freq = CwSidetoneFreq;

}

void DataEngine::CwKeyReversedChanged(int CwKeyReversed)
{
    m_cw_key_reversed = CwKeyReversed;
}

void DataEngine::CwKeyerModeChanged(int CwKeyerMode)
{
    m_cw_keyer_mode = CwKeyerMode;
}

void DataEngine::InternalCwChanged(int InternalCW)
{
    m_internal_cw = InternalCW;
}

void DataEngine::CwKeyerSpeedChanged(int CwKeyerSpeed)
{
    m_cw_keyer_speed = CwKeyerSpeed;
}

void DataEngine::CwPttDelayChanged(int CwPttDelay)
{
    m_cw_ptt_delay = CwPttDelay;
}

void DataEngine::CwSidetoneVolumeChanged(int CwSidetoneVolume)
{
    m_cw_sidetone_volume = CwSidetoneVolume;
}


void DataEngine::CwKeyerWeightChanged(int CwKeyerWeight)
{
    m_cw_keyer_weight = CwKeyerWeight;
}

void DataEngine::CwKeyerSpacingChanged(int CwKeyerSpacing)
{
    m_cw_keyer_spacing = CwKeyerSpacing;
}


// *********************************************************************
// Audio out processor

AudioOutProcessor::AudioOutProcessor(DataEngine *de, QSDR::_ServerMode serverMode)
	: QObject()
	, m_dataEngine(de)
	, m_serverMode(serverMode)
	, m_stopped(false)
{
	m_IQDatagram.resize(0);
}

AudioOutProcessor::~AudioOutProcessor() {
}

void AudioOutProcessor::stop() {

	m_stopped = true;
}

void AudioOutProcessor::processDeviceData() {
	forever {
		m_mutex.lock();
		if (m_stopped) {
			m_stopped = false;
			m_mutex.unlock();
			break;
		}
		m_mutex.unlock();
	}
}

void AudioOutProcessor::processData() {
	forever {
		m_mutex.lock();
		if (m_stopped) {
			m_stopped = false;
			m_mutex.unlock();
			break;
		}
		m_mutex.unlock();
	}
}


void DataEngine::createAudioInputProcessor() {

    m_audioInput = new TransmitAudioInput();
/*
    CHECKED_CONNECT(
            m_audioInput,
            SIGNAL(tx_mic_data_ready()),
            m_dataProcessor,
            SLOT(processMicData()));
*/

    m_cwIO = new iambic(this);

    connect(m_dataProcessor, &DataProcessor::keyer_event,
            m_cwIO, &iambic::keyer_event, Qt::DirectConnection);

/*
    CHECKED_CONNECT_OPT(
            m_dataProcessor,
            SIGNAL(keyer_event(
                    int,int)),
            m_dataProcessor,
            SLOT(key_down_test(
                    int,int)), Qt::DirectConnection);

*/
   //         m_cwIO->Start();

}

void DataEngine::set_tx_drivelevel(int value){

    qDebug() << "Drive level change" << value;
    io.ccTx.drivelevel = value;

}

void DataEngine::radioStateChange(RadioState state) {

    m_radioState = state;

    if ((state == RadioState::MOX) || (state == RadioState::TUNE))
    {
        io.ccTx.mox = true;
        m_audioInput->Start();
    }
    else{
        io.ccTx.mox = false;
        m_audioInput->Stop();
    }

	// Keep all receiver display pipelines in sync with TX/RX state.
	// SliceProcessor::dspProcessing uses m_state to choose RX or TX spectrum source.
	for (int i = 0; i < RX.size(); ++i) {
		if (RX.at(i)) {
			RX.at(i)->m_state = state;
		}
	}
}

void DataProcessor::processReadData()
{
#ifdef HAVE_SOAPYSDR
    if (set->getHWInterface() == QSDR::SoapySDR) {
        static uint32_t soapyProcCount = 0;
        while(!de->io.data_queue.isEmpty()) {
            soapyProcCount++;
            QList<double> samples = de->io.data_queue.dequeue();
            if (soapyProcCount % 100 == 0) {
                qDebug() << "DataProcessor: Processing SoapySDR samples, queue size:" << de->io.data_queue.count();
            }
            processInputBuffer(samples);
        }
        return;
    }
#endif

	static quint64 p2ReadDataPackets = 0;
	TIQPacket packet;
    while(!de->io.iq_queue.isEmpty()) {
	  packet = de->io.iq_queue.dequeue();
	  const QByteArray &buf = packet.payload;
      if (de->io.protocol && de->io.protocol->getHeaderSize() == METIS_HEADER_SIZE) {
          // Protocol 1: each UDP packet carries two 512-byte frames.
          // The payload (1024 bytes) must be split and processed separately.
		  processInputBuffer(buf.left(512), 0);
		  processInputBuffer(buf.mid(512, 512), 0);
      } else {
          // Protocol 2: each DDC port sends one continuous IQ stream per
          // packet.  Pass the entire payload as a single buffer.
		  ++p2ReadDataPackets;
		  if ((p2ReadDataPackets % 500) == 1) {
			  DATA_PROCESSOR_DEBUG << "P2 processReadData packet=" << p2ReadDataPackets
								   << " size=" << buf.size()
								   << " sourcePort=" << packet.sourcePort
								   << " iqQueueRemaining=" << de->io.iq_queue.count();
		  }
		  processInputBuffer(buf, packet.sourcePort);
      }
    }
}

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include <complex>

#ifdef HAVE_SOAPYSDR
void DataProcessor::processInputBuffer(const QList<double> &samples) {
    if (samples.isEmpty()) return;

    static uint32_t inputBufCount = 0;
    inputBufCount++;

    int rx = 0;
    if (rx < de->RX.size() && de->RX[rx]) {
        QVector<float> floatBlock;
        floatBlock.reserve(samples.size());
        for (const double s : samples)
            floatBlock.append(static_cast<float>(s));

        // Use thread-safe push
        de->RX[rx]->enqueueSoapyData(floatBlock);
        QMetaObject::invokeMethod(de->RX[rx], "dspProcessingSoapy", Qt::QueuedConnection);
    }
}
#endif

void DataProcessor::key_down(int state) {
    qDebug() << "Key Down" << state;
    if (state) {
        de->cw_key_down = 960000;    // up to 20 sec
    } else {
        de->cw_key_down = 0;
    }
}

void DataProcessor::key_down_test(int dummy,int state) {
    Q_UNUSED(dummy)
    qDebug() << QTime::currentTime().msec()  << "Key Down test" << state;
    if (state) {
        de->cw_key_down = 960000;    // up to 20 sec
    } else {
        de->cw_key_down = 0;
    }
}
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
