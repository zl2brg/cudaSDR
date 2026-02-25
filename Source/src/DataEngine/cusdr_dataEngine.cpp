#pragma clang diagnostic push
#pragma clang diagnostic push
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EmptyDeclOrStmt"
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
#include "cusdr_dataIO.h"             // DataIO concrete class (for factory new DataIO)
#include "Protocol1FrameDecoder.h"   // concrete Protocol1 frame decoder
#include "Protocol2Handler.h"        // stub: HPSDR Protocol 2
#include "SoapySDRHandler.h"         // stub: SoapySDR abstraction


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

#define FREQ_CONST ((2.0 * 3.14159) / 48000)
// NOTE: firstTimeRxInit, adc_rx1_4 etc. moved to Protocol1FrameDecoder as instance members.

DataEngine::DataEngine(QObject *parent)
	: QObject(parent)
	, set(Settings::instance())
    , m_internal_cw(set->isInternalCw())
	, m_cw_key_reversed(set->isCwKeyReversed())
	, m_cw_keyer_speed(set->getCwKeyerSpeed())
	, m_cw_keyer_mode(set->getCwKeyerMode())
	, m_cw_sidetone_volume(set->getCwSidetoneVolume())
	, m_cw_ptt_delay(set->getCwPttDelay())
	, m_cw_hang_time(set->getCwHangTime())
    , m_cw_keyer_spacing(set->getCwKeyerSpacing())
    , m_cw_keyer_weight(set->getCwKeyerWeight())
	, m_cw_sidetone_freq(set->getCwSidetoneFreq())
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
    , m_radioState(RadioState::RX)



{
	qRegisterMetaType<QAbstractSocket::SocketError>();

	this->setObjectName(QString::fromUtf8("dataEngine"));

	m_clientConnected = false;

	//currentRx = 0;
	m_discoverer= nullptr;
	m_dataIO= nullptr;
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
    // Add socket cleanup
    if (sendSocket) {
        delete sendSocket;
        sendSocket = nullptr;
    }

    // Add raw socket cleanup
    if (data_socket >= 0) {
        close(data_socket);
        data_socket = -1;
    }
   // file->close();
    if (m_audioInput)
        delete m_audioInput;
}

void DataEngine::setupConnections() {

	CHECKED_CONNECT(
		set,
		SIGNAL(systemStateChanged(
                QObject*,QSDR::_Error,
				QSDR::_HWInterfaceMode,
				QSDR::_ServerMode,
				QSDR::_DataEngineState)),this,
		SLOT(systemStateChanged(
				QObject*,
				QSDR::_Error,
				QSDR::_HWInterfaceMode,
				QSDR::_ServerMode,
				QSDR::_DataEngineState)))

	CHECKED_CONNECT(
		set, 
		SIGNAL(rxListChanged(QList<Receiver*>)),
		this,
		SLOT(rxListChanged(QList<Receiver*>)))

	CHECKED_CONNECT(
		set, 
		SIGNAL(numberOfRXChanged(QObject*,int)),
		this, 
		SLOT(setNumberOfRx(QObject*,int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(currentReceiverChanged(QObject*,int)),
		this, 
		SLOT(setCurrentReceiver(QObject*,int)));

	CHECKED_CONNECT(
		set,
		SIGNAL(hamBandChanged(QObject *, int, bool, HamBand)),
		this,
		SLOT(setHamBand(QObject *, int, bool, HamBand)));

	CHECKED_CONNECT(
		set,
		SIGNAL(sampleRateChanged(QObject *, int)), 
		this, 
		SLOT(setSampleRate(QObject *, int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(mercuryAttenuatorChanged(QObject *, HamBand, int)),
		this, 
		SLOT(setMercuryAttenuator(QObject *, HamBand, int)));

//	CHECKED_CONNECT(
//		set,
//		SIGNAL(mercuryAttenuatorsChanged(QObject *, QList<int>)),
//		this,
//		SLOT(setMercuryAttenuators(QObject *, QList<int>)));

	CHECKED_CONNECT(
		set,
		SIGNAL(ditherChanged(QObject*,int)),
		this, 
        SLOT(setDither(QObject*,int)))

	CHECKED_CONNECT(
		set, 
		SIGNAL(randomChanged(QObject*,int)),
		this, 
		SLOT(setRandom(QObject*,int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(src10MhzChanged(QObject *, int)), 
		this, 
		SLOT(set10MhzSource(QObject *, int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(src122_88MhzChanged(QObject *, int)), 
		this, 
		SLOT(set122_88MhzSource(QObject *, int)));

	CHECKED_CONNECT(
		set, 
        SIGNAL(micSourceChanged(int)),
		this, 
        SLOT(setMicSource(int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(classChanged(QObject *, int)), 
		this, 
		SLOT(setMercuryClass(QObject *, int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(timingChanged(QObject *, int)), 
		this, 
		SLOT(setMercuryTiming(QObject *, int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(clientDisconnectedEvent(int)), 
		this, 
		SLOT(setClientDisconnected(int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(clientNoConnectedChanged(QObject*, int)), 
		this, 
		SLOT(setClientConnected(QObject*, int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(rxConnectedStatusChanged(QObject*, int, bool)), 
		this, 
		SLOT(setRxConnectedStatus(QObject*, int, bool)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(audioRxChanged(QObject*, int)), 
		this, 
		SLOT(setAudioReceiver(QObject*, int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(framesPerSecondChanged(QObject*, int, int)),
		this, 
		SLOT(setFramesPerSecond(QObject*, int, int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(searchMetisSignal()), 
		this, 
		SLOT(searchHpsdrNetworkDevices()));

	/*CHECKED_CONNECT(
		set, 
		SIGNAL(spectrumAveragingChanged(QObject*, int, bool)), 
		this, 
		SLOT(setWbSpectrumAveraging(QObject*, int, bool)));*/

	CHECKED_CONNECT(
		set, 
		SIGNAL(networkDeviceNumberChanged(int)), 
		this, 
		SLOT(setHPSDRDeviceNumber(int)));

//	CHECKED_CONNECT(
//		set,
//		SIGNAL(alexConfigurationChanged(const QList<TAlexConfiguration> &)),
//		this,
//		SLOT(setAlexConfiguration(const QList<TAlexConfiguration> &)));

	CHECKED_CONNECT(
		set,
		SIGNAL(alexConfigurationChanged(quint16)),
		this,
		SLOT(setAlexConfiguration(quint16)));

	CHECKED_CONNECT(
		set,
		SIGNAL(alexStateChanged(HamBand, const QList<int> &)),
		this,
		SLOT(setAlexStates(HamBand, const QList<int> &)));

	CHECKED_CONNECT(
		set,
		SIGNAL(pennyOCEnabledChanged(bool)),
		this,
		SLOT(setPennyOCEnabled(bool)));

	CHECKED_CONNECT(
		set,
		SIGNAL(rxJ6PinsChanged(const QList<int> &)),
		this,
		SLOT(setRxJ6Pins(const QList<int> &)));

	CHECKED_CONNECT(
		set,
		SIGNAL(txJ6PinsChanged(const QList<int> &)),
		this,
		SLOT(setTxJ6Pins(const QList<int> &)));

    CHECKED_CONNECT(
            set,
            SIGNAL(radioStateChanged(RadioState)),
            this,
            SLOT(radioStateChange(RadioState)));

    CHECKED_CONNECT(
            set,
            SIGNAL(driveLevelChanged(QObject *, int)),
            this,
            SLOT(set_tx_drivelevel(QObject *, int)));

    CHECKED_CONNECT(
            set,
            SIGNAL(repeaterModeChanged(bool)),
            this,
            SLOT(setRepeaterMode(bool)));


    CHECKED_CONNECT(
            set,
            SIGNAL(dspModeChanged(QObject *, int, DSPMode)),
            this,
            SLOT(dspModeChanged(QObject *, int, DSPMode)));

     CHECKED_CONNECT(
            set,
            SIGNAL(dspModeChanged(QObject *, int, DSPMode)),
            this,
            SLOT(dspModeChanged(QObject *, int, DSPMode)));


    CHECKED_CONNECT(
            set,
            SIGNAL(CwHangTimeChanged(int)),
            this,
            SLOT(CwHangTimeChanged(int)));

    CHECKED_CONNECT(
            set,
            SIGNAL(CwSidetoneFreqChanged(int)),
            this,
            SLOT(CwSidetoneFreqChanged(int)));


    CHECKED_CONNECT(
            set,
            SIGNAL(CwKeyReversedChanged(int)),
            this,
            SLOT(CwKeyReversedChanged(int)));

    CHECKED_CONNECT(
            set,
            SIGNAL(CwKeyReversedChanged(int)),
            this,
            SLOT(CwKeyReversedChanged(int)));

    CHECKED_CONNECT(
            set,
            SIGNAL(CwKeyerModeChanged(int)),
            this,
            SLOT(CwKeyerModeChanged(int)));

    CHECKED_CONNECT(
               set,
               SIGNAL(InternalCwChanged(int)),
               this,
               SLOT(InternalCwChanged(int)));

    CHECKED_CONNECT(
               set,
               SIGNAL(CwKeyerSpeedChanged(int)),
               this,
               SLOT(CwKeyerSpeedChanged(int)));


    CHECKED_CONNECT(
               set,
               SIGNAL(CwPttDelayChanged(int)),
               this,
               SLOT(CwPttDelayChanged(int)));

    CHECKED_CONNECT(
               set,
               SIGNAL(CwSidetoneVolumeChanged(int)),
               this,
               SLOT(CwSidetoneVolumeChanged(int)));

    CHECKED_CONNECT(
            set,
            SIGNAL(CwKeyerWeightChanged(int)),
            this,
            SLOT(CwKeyerWeightChanged(int)));

    CHECKED_CONNECT(
            set,
            SIGNAL(CwKeyerSpacingChanged(int)),
                    this,
            SLOT(CwKeyerSpacingChanged(int)));


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
		if (!startDataIO(QThread::NormalPriority)) {

			setSystemState(QSDR::DataReceiverThreadError, m_hwInterface, m_serverMode, QSDR::DataEngineDown);
			return false;
		}

				// IQ data processing thread
		if (!startDataProcessor(QThread::NormalPriority)) {

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

		//Sleep(100);
		SleeperThread::msleep(100);

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

	// init receivers
	int rcvrs = set->getNumberOfReceivers();
	// NOTE: firstTimeRxInit set via setFirstTimeRxInit() after dataprocessor is created (below)

	QString str = "Initializing %1 receiver(s)...please wait";
	set->setSystemMessage(str.arg(set->getNumberOfReceivers()), rcvrs * 500);
	if (!initReceivers(rcvrs)) return false;

	if (!m_dataIO) createDataIO();
		
	if (!m_dataProcessor) createDataProcessor();

	// Tell the Protocol 1 frame decoder how many RX freq frames to force on startup
	if (m_dataProcessor) m_dataProcessor->setFirstTimeRxInit(rcvrs);

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

	set->setRxList(RX);
	connectDSPSlots();

//	for (int i = 0; i < set->getNumberOfReceivers(); i++)
//		RX.at(i)->setAudioVolume(this, i, set->getMainVolume());

	// IQ data processing thread
	if (!startDataProcessor(QThread::NormalPriority)) {

		DATA_ENGINE_DEBUG << "data processor thread could not be started.";
		return false;
	}

	// data IO thread
	if (!startDataIO(QThread::NormalPriority)) {//  ::NormalPriority)) {

		DATA_ENGINE_DEBUG << "data IO thread could not be started.";
		return false;
	}

	//setSampleRate(this, set->getSampleRate());
	SleeperThread::msleep(100);

	// pre-conditioning
	for (int i = 0; i < io.receivers; i++)
		m_dataIO->sendInitFrames(i);
				
	if (m_serverMode == QSDR::SDRMode)
		m_dataIO->sendCommand(0x01); // 0x01 for starting Metis without wide band data
		
	m_networkDeviceRunning = true;
	setSystemState(QSDR::NoError, m_hwInterface, m_serverMode, QSDR::DataEngineUp);
	SleeperThread::msleep(300);

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

	int rcvrs = set->getNumberOfReceivers();
    if (!m_audioInput) createAudioInputProcessor();

	if (!m_dataIO) createDataIO();

	if (!m_dataProcessor) createDataProcessor();


	if (m_serverMode == QSDR::SDRMode && !m_wbDataProcessor)
		createWideBandDataProcessor();


	switch (m_serverMode) {

		//case QSDR::ExternalDSP:

			/*
			//CHECKED_CONNECT(
			//	set,
			//	SIGNAL(frequencyChanged(QObject*, bool, int, long)),
			//	this,
			//	SLOT(setFrequency(QObject*, bool, int, long)));

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

            setTimeStamp(this, false);
            break;

		default:

			DATA_ENGINE_DEBUG << "no valid server mode";

			setSystemState(QSDR::ServerModeError, m_hwInterface, m_serverMode, QSDR::DataEngineDown);
			return false;
	}	// end switch (m_serverMode)

	set->setRxList(RX);
	connectDSPSlots();

    for (int i = 0; i < rcvrs ; i++) {

		RX.at(i)->setConnectedStatus(true);
        RX.at(i)->setAudioVolume(this, i, RX.at(i)->getAudioVolume());
		setFrequency(this, true, i, set->getCtrFrequencies().at(i));


        //CHECKED_CONNECT(
		//		RX.at(i),
		//		SIGNAL(outputBufferSignal(int, const CPX &)),
		//		this, //m_dataProcessor,
		//		SLOT(setOutputBuffer(int, const CPX &)));

		CHECKED_CONNECT(
				RX.at(i),
				SIGNAL(outputBufferSignal(int, const CPX &)),m_dataProcessor,SLOT(setOutputBuffer(int, const CPX &)));
        CHECKED_CONNECT(RX.at(i),SIGNAL(audioBufferSignal(int, const CPX &, int)),m_dataProcessor,SLOT(
                send_hpsdr_data(int, const CPX &,int)));

   //     CHECKED_CONNECT(RX.at(i),SIGNAL(audioBufferSignal(int, const CPX &, int)),m_dataProcessor,SLOT(
    ///     setAudioBuffer(int, const CPX &,int)));


		m_dspThreadList.at(i)->start(QThread::NormalPriority);//QThread::TimeCriticalPriority);

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
	if (!startDataProcessor(QThread::NormalPriority)) {

		DATA_ENGINE_DEBUG << "data processor thread could not be started.";
		return false;
	}

	// data IO thread
	if (!startDataIO(QThread::NormalPriority)) {//  ::NormalPriority::HighPriority)) {

		DATA_ENGINE_DEBUG << "data IO thread could not be started.";
		return false;
	}

	// start Sync,ADC and S-Meter timers
	//m_SyncChangedTime.start();
	//m_ADCChangedTime.start();
	//m_smeterTime.start();

		// pre-conditioning
	for (int i = 0; i < io.receivers; i++) {
			m_dataIO->sendInitFrames(i);
		}

    if (m_serverMode == QSDR::SDRMode && set->getWidebandData()) {
			m_dataIO->sendCommand(0x03); // 0x03 for starting the device with wide band data
			SleeperThread::msleep(100);
	    }
    else {
            m_dataIO->sendCommand(0x01); // 0x01 for starting the device without wide band data
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
				setTimeStamp(this, false);

				// stop the device
				m_dataIO->sendCommand(0);
				m_networkDeviceRunning = false;
				DATA_ENGINE_DEBUG << "HPSDR device stopped";

				// stop the threads
				//SleeperThread::msleep(100);
				stopDataIO();
				SleeperThread::msleep(100);
				stopDataProcessor();
				if (m_wbDataProcessor)
					stopWideBandDataProcessor();
				
				// clear device list
				SleeperThread::msleep(100);
				set->clearMetisCardList();
				DATA_ENGINE_DEBUG << "device cards list cleared.";
				break;

			case QSDR::NoInterfaceMode:

				stopDataIO();
				
				DATA_ENGINE_DEBUG << "data queue count: " << io.data_queue.count();

				stopDataProcessor();

            case QSDR::SoapySDR:
                break;
        }

		while (!io.au_queue.isEmpty())
			io.au_queue.dequeue();

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

			disconnect(
				rx,
				SIGNAL(spectrumBufferChanged(int, const qVectorFloat&)),
				set,
				SLOT(setSpectrumBuffer(int, const qVectorFloat&)));

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
		SIGNAL(ctrFrequencyChanged(QObject*, int, int, long)), 
		this, 
		SLOT(setFrequency(QObject*, int, int, long)));*/

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

	for (int i = 0; i < rcvrs; i++) {

        auto rx =  new Receiver(i);
		// init the DSP core
		DATA_ENGINE_DEBUG << "trying to init a DSP core for rx " << i;

		if (rx->initDSPInterface()) {

			DATA_ENGINE_DEBUG << "init DSP core for rx " << i << " successful !";

			rx->setConnectedStatus(false);
			rx->setServerMode(m_serverMode);

			// create dsp thread
			auto thread = new QThreadEx();
			rx->moveToThread(thread);

			//CHECKED_CONNECT(this, SIGNAL(doDSP()), rx, SLOT(dspProcessing()));

            connect(
                rx, // Connect to the raw pointer managed by the unique_ptr
                &Receiver::spectrumBufferChanged,
				set,
                // The lambda captures the 'set' pointer and calls the slot
                [this](int receiverId, const QList<float> &buffer) {
                    set->setSpectrumBuffer(receiverId, buffer);
                }
                );

            connect(rx, &Receiver::sMeterValueChanged, set, &Settings::setSMeterValue);
         //   connect(rx.get(), &Receiver::outputBufferSignal, m_dataProcessor, &DataProcessor::setOutputBuffer);


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
	io.ccTx.currentBand = RX.at(0)->getHamBand();
	io.ccTx.mercuryAttenuators = RX.at(0)->getMercuryAttenuators();
	io.ccTx.mercuryAttenuator = RX.at(0)->getMercuryAttenuators().at(io.ccTx.currentBand);
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
	setSampleRate(this, set->getSampleRate());

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

	CHECKED_CONNECT(
		set,
		SIGNAL(ctrFrequencyChanged(QObject *, int, int, long)),
		this,
		SLOT(setFrequency(QObject *, int, int, long)));
}

void DataEngine::disconnectDSPSlots() {

	disconnect(
		set,
		SIGNAL(ctrFrequencyChanged(QObject *, int, int, long)),
		this,
		SLOT(setFrequency(QObject *, int, int, long)));
}




//********************************************************
// create, start/stop HPSDR device network IO

void DataEngine::createDiscoverer() {

	m_discoverer = new Discoverer(&io);

	m_discoveryThread = new QThreadEx();
	m_discoverer->moveToThread(m_discoveryThread);

	m_discoverer->connect(
					m_discoveryThread,
					SIGNAL(started()), 
					SLOT(initDevice()));
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

	// Factory: choose hardware transport based on interface mode
	switch (m_hwInterface) {
		case QSDR::Protocol2:
			m_dataIO = new Protocol2Handler(&io);
			break;
		case QSDR::SoapySDR:
			m_dataIO = new SoapySDRHandler(&io);
			break;
		case QSDR::Metis:
		case QSDR::Hermes:
		default:
			m_dataIO = new DataIO(&io);
			break;
	}

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
		case QSDR::Protocol2:
		case QSDR::SoapySDR:

			m_dataIO->connect(
						m_dataIOThread,
						SIGNAL(started()),
						SLOT(initIO()));
			break;
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

	if (m_dataIOThread->isRunning()) {
					
		m_dataIO->stop();
		m_dataIOThread->quit();

		while (!m_dataIOThread->isFinished()) {
		
			DATA_ENGINE_DEBUG << "data IO thread not yet finished...";
			if (m_dataIOThread->wait(100)) break;
		}
		m_dataIOThreadRunning = false;
		
		delete m_dataIOThread;
		delete m_dataIO;
		m_dataIO = nullptr;

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
	Connect();
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

	switch (m_hwInterface) {

		case QSDR::NoInterfaceMode:
			m_dataProcessor->connect(
						m_dataProcThread, 
						SIGNAL(started()), 
						SLOT(processData()));
			break;
			
		case QSDR::Metis:
		case QSDR::Hermes:

		    /*
			m_dataProcessor->connect(
						m_dataProcThread, 
						SIGNAL(started()), 
						SLOT(processDeviceData()));
*/

            CHECKED_CONNECT(
                    m_dataIO,
                    SIGNAL(readydata()),
                    m_dataProcessor,
                    SLOT(processReadData()));

            break;
        case QSDR::SoapySDR:
            break;
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
					
		m_dataProcessor->stop();
		
		if (m_serverMode == QSDR::SDRMode ) {
			
			if (io.iq_queue.isEmpty()) {
				io.iq_queue.enqueue(QByteArray(BUFFER_SIZE, 0x0));
			}
		}

		m_dataProcThread->quit();
		m_dataProcThread->wait();
		delete m_dataProcThread;
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

	CHECKED_CONNECT(
			set,
			SIGNAL(spectrumAveragingCntChanged(QObject*, int, int)),
			this,
			SLOT(setWbSpectrumAveraging(QObject*, int, int)));



	CHECKED_CONNECT(
			m_wbDataProcessor,
			SIGNAL(wbSpectrumBufferChanged(const qVectorFloat&)),
			set,
			SLOT(setWidebandSpectrumBuffer(const qVectorFloat&)));


	m_wbDataProcThread = new QThreadEx();
	m_wbDataProcessor->moveToThread(m_wbDataProcThread);
	m_wbDataProcessor->connect(
							m_wbDataProcThread, 
							SIGNAL(started()), 
							SLOT(processWideBandData()));
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

	set->setWidebandBuffers(this, wbBuffers);

}
//********************************************************
// create, start/stop audio receiver

void DataEngine::createAudioReceiver() {

	m_audioReceiver = new AudioReceiver(&io);

	CHECKED_CONNECT(
		m_audioReceiver, 
		SIGNAL(rcveIQEvent(QObject *, int)), 
		this, 
		SLOT(setRcveIQSignal(QObject *, int)));

	CHECKED_CONNECT(
		m_audioReceiver, 
		SIGNAL(clientConnectedEvent(bool)), 
		this, 
		SLOT(setClientConnected(bool)));

	
	m_AudioRcvrThread = new QThreadEx();
	m_audioReceiver->moveToThread(m_AudioRcvrThread);

	m_audioReceiver->connect(
						m_AudioRcvrThread, 
						SIGNAL(started()), 
						SLOT(initClient()));
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

		SleeperThread::usleep(42667);

		//emit spectrumBufferChanged(m_spectrumBuffer);
		//set->setSpectrumBuffer(m_spectrumBuffer);
		//set->setSpectrumBuffer(0, m_spectrumBuffer);

		m_rxSamples = 0;
	}
}


//*****************************************************************************
//

void DataEngine::systemStateChanged(
	QObject *sender, 
	QSDR::_Error err, 
	QSDR::_HWInterfaceMode hwmode, 
	QSDR::_ServerMode mode, 
	QSDR::_DataEngineState state)
{
	Q_UNUSED (sender)
	Q_UNUSED (err)

	io.mutex.lock();
	if (m_hwInterface != hwmode)
		m_hwInterface = hwmode;
		
	if (m_serverMode != mode)
		m_serverMode = mode;
		
	if (m_dataEngineState != state)
		m_dataEngineState = state;

	io.mutex.unlock();
}

void DataEngine::setSystemState(
		QSDR::_Error err,
		QSDR::_HWInterfaceMode hwmode,
		QSDR::_ServerMode statemode,
		QSDR::_DataEngineState enginestate)
{
	io.networkIOMutex.lock();
	set->setSystemState(this, err, hwmode, statemode, enginestate);
	io.networkIOMutex.unlock();
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

	io.networkIOMutex.lock();
	io.devicefound.wait(&io.networkIOMutex);

	//m_discoverer->findHPSDRDevices();

	// stop the discovery thread
	io.networkIOMutex.unlock();
	stopDiscoverer();
}

void DataEngine::setHPSDRDeviceNumber(int value) {

	m_hpsdrDevices = value;
}

void DataEngine::rxListChanged(QList<Receiver *> list) {

	io.mutex.lock();
	RX = list;
	io.mutex.unlock();
}

void DataEngine::setCurrentReceiver(QObject *sender, int rx) {

	Q_UNUSED(sender)

	io.mutex.lock();
	io.currentReceiver = rx;
	io.mutex.unlock();
}

void DataEngine::setFramesPerSecond(QObject *sender, int rx, int value) {

	Q_UNUSED(sender)
	Q_UNUSED(rx)
	Q_UNUSED(value)

	/*io.mutex.lock();
	if (m_fpsList.length() > 0)
		m_fpsList[rx] = (int)(1000000.0/value);
	io.mutex.unlock();*/
}

void DataEngine::setSampleRate(QObject *sender, int value) {

	Q_UNUSED(sender)

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

		default:
			DATA_ENGINE_DEBUG << "invalid sample rate !\n";
			stop();
			break;
	}

//	io.mutex.unlock();
}

void DataEngine::setMercuryAttenuator(QObject *sender, HamBand band, int value) {

	Q_UNUSED(sender)
	Q_UNUSED(band)

	io.mutex.lock();
	io.ccTx.mercuryAttenuator = value;
	io.mutex.unlock();
}

void DataEngine::setMercuryAttenuators(QObject *sender, QList<int> attn) {

	Q_UNUSED(sender)

	io.mutex.lock();
	io.ccTx.mercuryAttenuators = attn;
	io.mutex.unlock();
}

void DataEngine::setDither(QObject *sender, int value) {

	Q_UNUSED(sender)

	io.mutex.lock();
	io.ccTx.dither = value;
	io.mutex.unlock();
}

void DataEngine::setRandom(QObject *sender, int value) {

	Q_UNUSED(sender)

	io.mutex.lock();
	io.ccTx.random = value;
	io.mutex.unlock();
}

void DataEngine::set10MhzSource(QObject *sender, int source) {

	Q_UNUSED(sender)

	io.mutex.lock();
	io.control_out[1] = io.control_out[1] & 0xF3;
	io.control_out[1] = io.control_out[1] | (source << 2);
	io.mutex.unlock();
}

void DataEngine::set122_88MhzSource(QObject *sender, int source) {

	Q_UNUSED(sender)

	io.mutex.lock();
	io.control_out[1] = io.control_out[1] & 0xEF;
	io.control_out[1] = io.control_out[1] | (source << 4);
	io.mutex.unlock();
}

void DataEngine::setMicSource( int source) {

	io.mutex.lock();
	io.control_out[1] = io.control_out[1] & 0x7F;
	io.control_out[1] = io.control_out[1] | (source << 7);
	io.mutex.unlock();
}

void DataEngine::setMercuryClass(QObject *sender, int value) {

	Q_UNUSED(sender)

	io.mutex.lock();
	io.rxClass = value;
	io.mutex.unlock();
}

void DataEngine::setMercuryTiming(QObject *sender, int value) {

	Q_UNUSED(sender)

	io.mutex.lock();
	io.timing = value;
	io.mutex.unlock();
}

void DataEngine::setAlexConfiguration(quint16 conf) {

	io.mutex.lock();
	io.ccTx.alexConfig = conf;
	DATA_ENGINE_DEBUG << "Alex Configuration = " << io.ccTx.alexConfig;
	io.mutex.unlock();
}

void DataEngine::setAlexStates(HamBand band, const QList<int> &states) {

	Q_UNUSED (band)

	io.mutex.lock();
	io.ccTx.alexStates = states;
	DATA_ENGINE_DEBUG << "Alex States = " << io.ccTx.alexStates;
	io.mutex.unlock();
}

void DataEngine::setPennyOCEnabled(bool value) {

	io.mutex.lock();
	io.ccTx.pennyOCenabled = value;
	io.mutex.unlock();
}

void DataEngine::setRxJ6Pins(const QList<int> &list) {

	io.mutex.lock();
	io.ccTx.rxJ6pinList = list;
	io.mutex.unlock();

}

void DataEngine::setTxJ6Pins(const QList<int> &list) {

	io.mutex.lock();
	io.ccTx.txJ6pinList = list;
	io.mutex.unlock();
}

void DataEngine::setRcveIQSignal(QObject *sender, int value) {

	emit rcveIQEvent(sender, value);
}

void DataEngine::setPenelopeVersion(QObject *sender, int version) {

	emit penelopeVersionInfoEvent(sender, version);
}

void DataEngine::setHwIOVersion(QObject *sender, int version) {

	emit hwIOVersionInfoEvent(sender, version);
}

void DataEngine::setNumberOfRx(QObject *sender, int value) {

	Q_UNUSED(sender)

	if (io.receivers == value) return;

	io.mutex.lock();
	io.receivers = value;
	io.mutex.unlock();
	//io.control_out[4] &= 0xc7;
	//io.control_out[4] |= (value - 1) << 3;

	DATA_ENGINE_DEBUG << "number of receivers set to " << QString::number(value);
}

void DataEngine::setTimeStamp(QObject *sender, bool value) {

	Q_UNUSED(sender)

	if (io.timeStamp == value) return;

	io.mutex.lock();
	io.timeStamp = value;
	io.mutex.unlock();
	//io.control_out[4] &= 0xc7;
	//RRK io.control_out[4] |= value << 6;

	if (value)
		DATA_ENGINE_DEBUG << "set time stamp on";
	else
		DATA_ENGINE_DEBUG << "set time stamp off";
}

void DataEngine::setRxSocketState(int rx, const char* prop, QString str) {

	RX[rx]->setProperty(prop, str);
	set->setRxList(RX);
}

void DataEngine::setRxPeerAddress(int rx, QHostAddress address) {

	RX[rx]->setPeerAddress(address);
	set->setRxList(RX);
}

void DataEngine::setRx(int rx) {

	io.mutex.lock();
	RX[rx]->setReceiver(rx);
	set->setRxList(RX);
	io.mutex.unlock();
}

void DataEngine::setRxClient(int rx, int client) {

	io.mutex.lock();
	RX[rx]->setClient(client);
	set->setRxList(RX);
	io.mutex.unlock();
}

void DataEngine::setClientConnected(QObject* sender, int rx) {

	Q_UNUSED(sender)

	if (!io.clientList.contains(rx)) {

		io.clientList.append(rx);
		io.audio_rx = rx;

		m_AudioRcvrThread->quit();
		m_AudioRcvrThread->wait();
		m_AudioRcvrThread->start();
	}
	else {

		io.sendIQ_toggle = true;
		io.rcveIQ_toggle = false;
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

void DataEngine::setAudioReceiver(QObject *sender, int rx) {

	Q_UNUSED(sender)

	io.mutex.lock();
	emit audioRxEvent(rx);
	io.mutex.unlock();
}

void DataEngine::setIQPort(int rx, int port) {

	io.mutex.lock();
	RX[rx]->setIQPort(port);
	set->setRxList(RX);
	io.mutex.unlock();
}

void DataEngine::setRxConnectedStatus(QObject* sender, int rx, bool value) {

	Q_UNUSED(sender)

	io.mutex.lock();
	RX[rx]->setConnectedStatus(value);
	set->setRxList(RX);
	io.mutex.unlock();
}

void DataEngine::setHamBand(QObject *sender, int rx, bool byBtn, HamBand band) {

	Q_UNUSED(sender)
	Q_UNUSED(rx)
	Q_UNUSED(byBtn)

	io.mutex.lock();
	io.ccTx.currentBand = band;
	io.mutex.unlock();
}

void DataEngine::setFrequency(QObject* sender, int mode, int rx, long frequency) {

	Q_UNUSED (sender)
	Q_UNUSED (mode)

	//RX[rx]->setFrequency(frequency);
	RX[rx]->setCtrFrequency(frequency);
	io.rx_freq_change = rx;
	io.tx_freq_change = rx;
    io.ccTx.txFrequency = frequency;


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
	, m_offset(0)
	, m_length(0)
    , m_idx(IO_HEADER_SIZE)
	, m_stopped(false)
{
	m_IQSequence = 0L;
	m_sequenceHi = 0L;
	
	m_IQDatagram.resize(0);

	// Protocol 1 frame decode/encode — delegated to Protocol1FrameDecoder
	m_frameDecoder = new Protocol1FrameDecoder(de, hwMode, set);
	connect(m_frameDecoder, &IFrameDecoder::keyerEvent, this, &DataProcessor::keyer_event);

	// NOTE: m_SyncChangedTime, m_ADCChangedTime, m_fwCount moved to Protocol1FrameDecoder

	m_sendSequence = 0L;
	m_oldSendSequence = 0L;

	m_deviceSendDataSignature.resize(4);
	m_deviceSendDataSignature[0] = (char)0xEF;
	m_deviceSendDataSignature[1] = (char)0xFE;
	m_deviceSendDataSignature[2] = (char)0x01;
	m_deviceSendDataSignature[3] = (char)0x02;
    InitCPX(m_iq_output_buffer, DSP_SAMPLE_SIZE, 0.0f);

    //socket = new QUdpSocket();
	m_deviceAddress = set->getCurrentMetisCard().ip_address;

    file = new QFile("data.txt");
    file->open(QIODevice::ReadWrite);

}

DataProcessor::~DataProcessor() {
    delete m_frameDecoder;
    m_frameDecoder = nullptr;
    if (file) {
    file->close();
        delete file;
        file = nullptr;
    }
}

void DataProcessor::setFirstTimeRxInit(int n) {
    if (m_frameDecoder)
        m_frameDecoder->setFirstTimeRxInit(n);
}

void DataProcessor::stop() {

	m_stopped = true;
}

void DataProcessor::initDataProcessorSocket() {

}


void DataProcessor::displayDataProcessorSocketError(QAbstractSocket::SocketError error) {

	DATA_PROCESSOR_DEBUG << "data processor socket error: " << error;
}

void DataProcessor::processDeviceData() {


	DATA_PROCESSOR_DEBUG << "Data Processor thread: " << this->thread();
	forever {

        //m_dataEngine->processInputBuffer(m_dataEngine->io.iq_queue.dequeue());

		QByteArray buf = de->io.iq_queue.dequeue();
		//de->processInputBuffer(buf.left(BUFFER_SIZE/2));
		//de->processInputBuffer(buf.right(BUFFER_SIZE/2));

		m_frameDecoder->decodeInputFrame(buf.left(BUFFER_SIZE));
	//	m_frameDecoder->decodeInputFrame(buf.right(BUFFER_SIZE/2));

		if (de->io.iq_queue.isFull()) {
			DATA_PROCESSOR_DEBUG << "IQ queue full!";
		}

		QMutexLocker locker(&m_mutex);
		if (m_stopped) {
			m_stopped = false;
			break;
		}
	}

//	if (m_serverMode == QSDR::ExternalDSP) {
//
//		disconnect(this);
//		m_dataProcessorSocket->close();
//		delete m_dataProcessorSocket;
//		m_dataProcessorSocket = NULL;
//
//		m_socketConnected = false;
//	}
}


// processInputBuffer() — moved to Protocol1FrameDecoder; implementation removed.


// decodeCCBytes() — moved to Protocol1FrameDecoder; implementation removed.

void DataProcessor::setOutputBuffer(int rx, const CPX &buffer) {

    if (rx == de->io.currentReceiver) {
		processOutputBuffer(buffer);

	}
}

void DataProcessor::full_txBuffer(){

    m_frameDecoder->encodeControlBytes();
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
        case QSDR::SoapySDR:
            break;
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
        leftRXSample = (qint16) (rx_audio_buffer[rx_audio_ptr].re * 32767.0f);
        rightRXSample = (qint16) (rx_audio_buffer[rx_audio_ptr].im * 32767.0f);
        de->io.output_buffer[m_idx++] = leftRXSample >> 8;
        de->io.output_buffer[m_idx++] = leftRXSample;
        de->io.output_buffer[m_idx++] = rightRXSample >> 8;
        de->io.output_buffer[m_idx++] = rightRXSample;
        rx_audio_ptr++;
    }

/* Sends RX Audio and tx iq data back to hpsdr. Always at 48 KHz bandwidth */

void DataProcessor::send_hpsdr_data(int rx, const CPX &buffer, int buffersize) {
    Q_UNUSED(rx);
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
    buffer_tx_data();
    if (m_idx == IO_BUFFER_SIZE)
    {
        full_txBuffer();
        m_idx =8;
    }
    if (tx_index >= sizeof(m_tx_iq_Buffer)) tx_index = 0;
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
    float cwsample;
    double mic_sample_double;
    double ramp;
    static int cw_not_ready =1;
    static int cw_shape;
    int cw_key_up = 0;
    int cw_key_down = 0;
    int updown;
    float cw_keyer_sidetone_volume=0.5;


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
            updown=1;
        } else {
            // dig into this even if cw_key_up is already zero, to ensure
            // that we reach the bottom of the ramp for very small pauses
            if (cw_shape > 0) cw_shape--;	// walk down the ramp
            if (cw_key_up > 0) cw_key_up--; // decrement key-down counter
            updown=0;
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
    double temp;
    float *sample;
    int i,q;
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
    AUDIOBUF temp_data;
    if (de->m_audioInput->m_faudioInQueue.count() > 0)
    {
        temp_data = de->m_audioInput->m_faudioInQueue.dequeue();
		qDebug() << "fetchmic data" << temp_data.size();

        for (int s = 0; s < DSP_SAMPLE_SIZE;s++)
        {
            mic_buffer[(s * 2 )]  = temp_data[s] ;//4294967295.0f;
            mic_buffer[(s * 2 ) + 1 ] = 0.0f;
        }
    }
    else{
        temp_data.clear();
        memset(&mic_buffer,0x0,sizeof(mic_buffer));
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
    int i,q;
    fetch_MicData();

    if ( de->io.ccTx.mox ||  de->io.ccTx.ptt ) {
        fexchange0(TX_ID, mic_buffer, (double *) m_iq_output_buffer.data(), &error);
     
		Spectrum0(1, TX_ID, 0, 0, (double *) m_iq_output_buffer.data());


/* Queue the tx data */
        int tx_index = 0;
        for (int j = 0; j < DSP_SAMPLE_SIZE; j++) {
            qs = m_iq_output_buffer.at(j).re;
            is = m_iq_output_buffer.at(j).im;
            rightTXSample = is >= 0.0 ? (long) floor(is * gain + 0.5) : (long) ceil(is * gain - 0.5);
            leftTXSample = qs >= 0.0 ? (long) floor(qs * gain + 0.5) : (long) ceil(qs * gain - 0.5);
            i = (int) leftTXSample;
            q = (int) rightTXSample;
            m_tx_iq_Buffer[tx_index++] = i >> 8;
            m_tx_iq_Buffer[tx_index++] = i;
            m_tx_iq_Buffer[tx_index++] = q >> 8;
            m_tx_iq_Buffer[tx_index++] = q;
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

    qint16 leftRXSample;
    qint16 rightRXSample;
    qint16 leftTXSample;
    qint16 rightTXSample;

    // process the output
    if (tx_index == 0)  get_tx_iqData();
        for (int j = 0; j < buffersize; j++) {

            leftRXSample  = (qint16)(buffer.at(j).re * 32767.0f);
            rightRXSample = (qint16)(buffer.at(j).im * 32767.0f);

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
			m_frameDecoder->encodeControlBytes();

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
					break;
            }
        m_idx = IO_HEADER_SIZE;
         }
       }
}




/* Sends rx audio data from wdsp to  hermes audio and to PC */
void DataProcessor::setAudioBuffer_old(int rx, const CPX &buffer, int buffersize)
{


//    qDebug() << "Buffer Size" << buffersize;
    QTextStream stream( this->file );
    qint16 leftRXSample;
    qint16 rightRXSample;
    char *ptr;
    // process the output
    if (tx_index == 0)  get_tx_iqData();
    for (int j = 0; j < buffersize; j++) {

        leftRXSample  = (qint16)(buffer.at(j).re * 32767.0f);
        rightRXSample = (qint16)(buffer.at(j).im * 32767.0f);
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
            m_frameDecoder->encodeControlBytes();
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
			m_frameDecoder->encodeControlBytes();

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
					break;
			}
			m_idx = IO_HEADER_SIZE;
		}
	}
}

// encodeCCBytes() — moved to Protocol1FrameDecoder; implementation removed.



void DataProcessor::writeData() {
	if (m_setNetworkDeviceHeader) {

		//RRK updated for 4byte int and network order
		quint32 outseq = qFromBigEndian(m_sendSequence);
		m_outDatagram.resize(0);
        m_outDatagram += m_deviceSendDataSignature;

		QByteArray seq(reinterpret_cast<const char*>(&outseq), sizeof(outseq));

		m_outDatagram += seq;
		m_outDatagram += de->io.audioDatagram;

		m_sendSequence++;
        m_setNetworkDeviceHeader = false;
    }
	else {

		//QUdpSocket socket;
		//DATA_PROCESSOR_DEBUG << "writeData: " << this->thread();
		m_outDatagram += de->io.audioDatagram;

		if (de->sendSocket->writeDatagram(m_outDatagram, m_deviceAddress, DEVICE_PORT) < 0) {
			DATA_PROCESSOR_DEBUG << "error sending data to device: " << de->sendSocket->errorString();
		}

		//if (m_sendSequence%100 == 0)
		//	DATAIO_DEBUG << m_sendSequence;

		if (m_sendSequence != m_oldSendSequence + 1) {
			DATA_PROCESSOR_DEBUG << "output sequence error: old = " << m_oldSendSequence << "; new =" << m_sendSequence;
		}

		m_oldSendSequence = m_sendSequence;
		m_setNetworkDeviceHeader = true;
    }
}

void 	DataEngine::setWbSpectrumAveraging(QObject* sender, int rx, int value)
{
	m_wbDataProcessor->setWbSpectrumAveraging(sender,rx,value);
}


void DataEngine::setRepeaterMode(bool mode) {
        io.ccTx.use_repeaterOffset = mode;
}

void DataEngine::dspModeChanged(QObject* sender, int rx, DSPMode mode){
    Q_UNUSED(sender);
    Q_UNUSED(rx);
    io.ccTx.mode = mode;
    TX.setDSPMode(sender,1,mode);
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
    m_cw_keyer_weight = CwKeyerSpacing;
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


void DataEngine::Connect(){
	qDebug() << "protocol1: Connect";
	data_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(data_socket< 0) {
        qDebug() << "protocol1: create socket failed for data_socket";
        return;
    }

    int optval = 1;
    if(setsockopt(data_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))<0) {
        perror("data_socket: SO_REUSEADDR");
    }
    if(setsockopt(data_socket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval))<0) {
        perror("data_socket: SO_REUSEPORT");
    }

	inet_pton(AF_INET, "192.168.1.11", &DataAddr.sin_addr);
 //   DataAddr.sin_addr.s_addr = io.hpsdrDeviceIPAddress.toIPv4Address();
    DataAddr.sin_port = htons(1024);
    DataAddr.sin_family = AF_INET;
	int result = bind(data_socket,(struct sockaddr*)&DataAddr,16);
	if (result < 0){
		perror("error");
		qDebug() << "protocol1: bind socket failed for data_socket " << result;
	}
	else qDebug() << "bind ok";
}

void DataEngine::senddata(char * buffer, int length) {

    if(sendto(data_socket,&buffer,length,0,(struct sockaddr*)&DataAddr,sizeof(DataAddr))!=length) {
        perror("sendto socket failed for metis_send_data\n");
    }

}


void DataEngine::createAudioInputProcessor() {

    m_audioInput = new PAudioInput();
/*
    CHECKED_CONNECT(
            m_audioInput,
            SIGNAL(tx_mic_data_ready()),
            m_dataProcessor,
            SLOT(processMicData()));
*/

    m_cwIO = new iambic(this);

    CHECKED_CONNECT_OPT(
            m_dataProcessor,
            SIGNAL(keyer_event(
                    int,
                    int)),
            m_cwIO,
            SLOT(keyer_event(
                    int,
                    int)),Qt::DirectConnection);

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

bool DataEngine::start_TxProcessor() {
    return false;
}


void DataEngine::stop_TxProcessor() {

}

void DataEngine::set_tx_drivelevel(QObject* sender, int value){

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
    RX.at(0)->m_state = state;
}

void DataProcessor::processReadData()
{
    QByteArray buf;
    while (!de->io.iq_queue.isEmpty()) {
        buf = de->io.iq_queue.dequeue();
        m_frameDecoder->decodeInputFrame(buf.left(BUFFER_SIZE / 2));
        m_frameDecoder->decodeInputFrame(buf.right(BUFFER_SIZE / 2));
    }
}

void DataProcessor::key_down(int state) {
qDebug() << "Key Down" << state;
if (state) {
  de->cw_key_down = 960000;    // up to 20 sec
} else {
  de->cw_key_down = 0;
}
}

void DataProcessor::key_down_test(int dummy,int state) {
    qDebug() << QTime::currentTime().msec()  << "Key Down test" << state;
    if (state) {
        de->cw_key_down = 960000;    // up to 20 sec
    } else {
        de->cw_key_down = 0;
    }
}
#pragma clang diagnostic pop
