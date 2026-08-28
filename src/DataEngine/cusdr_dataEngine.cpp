#include "Models/RadioModel.h"
#include "Models/RadioTelemetry.h"
#include "Models/SliceModel.h"
#include "Models/TransmitModel.h"
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
#include "DataEngineThreadFactory.h"
#include "DataEngineFirmware.h"
#include "DataEngineLifecycle.h"
#include "DataEngineSoapy.h"
#include "Util/cusdr_tciserver.h"
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

#ifdef HAVE_RADE
#include "rade_api.h"
extern "C" {
#include "lpcnet.h"
}
#if defined(Q_OS_LINUX)
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#endif
#endif

namespace {
bool txDiagEnabled() {
    static const bool enabled = qEnvironmentVariableIntValue("CUSDR_TX_DIAG") != 0;
    return enabled;
}

bool radeVerboseEnabled() {
    static const bool enabled = qEnvironmentVariableIntValue("CUSDR_RADE_VERBOSE") != 0;
    return enabled;
}

float radeTxSpeechGain() {
    static const float gain = []() {
        bool ok = false;
        const float envGain = qEnvironmentVariable("CUSDR_RADE_TX_GAIN").toFloat(&ok);
        if (ok && envGain > 0.0f && envGain <= 20.0f) {
            return envGain;
        }
        // RADE v1 TX path benefits from a higher speech drive than legacy FreeDV modes.
        return 2.0f;
    }();
    return gain;
}

#if defined(Q_OS_LINUX)
template <typename Fn>
auto runWithSuppressedCStdio(Fn&& fn) -> decltype(fn()) {
    if (radeVerboseEnabled())
        return fn();

    fflush(stdout);
    fflush(stderr);
    const int savedOut = dup(STDOUT_FILENO);
    const int savedErr = dup(STDERR_FILENO);
    const int devNull = open("/dev/null", O_WRONLY);
    if (devNull < 0 || savedOut < 0 || savedErr < 0) {
        if (devNull >= 0) close(devNull);
        if (savedOut >= 0) close(savedOut);
        if (savedErr >= 0) close(savedErr);
        return fn();
    }

    dup2(devNull, STDOUT_FILENO);
    dup2(devNull, STDERR_FILENO);
    close(devNull);

    auto result = fn();

    fflush(stdout);
    fflush(stderr);
    dup2(savedOut, STDOUT_FILENO);
    dup2(savedErr, STDERR_FILENO);
    close(savedOut);
    close(savedErr);
    return result;
}
#else
template <typename Fn>
auto runWithSuppressedCStdio(Fn&& fn) -> decltype(fn()) {
    return fn();
}
#endif

struct TxIqStats {
    double micRms = 0.0;
    double micPeak = 0.0;
    double iqRms = 0.0;
    double iqPeak = 0.0;
};

static TxIqStats computeTxIqStats(const double* micInterleaved, const CPX& iq) {
    TxIqStats st;
    double micSumSq = 0.0;
    for (int i = 0; i < DSP_SAMPLE_SIZE; ++i) {
        const double mRaw = micInterleaved[i * 2];
        const double m = std::isfinite(mRaw) ? mRaw : 0.0;
        micSumSq += m * m;
        const double a = std::abs(m);
        if (a > st.micPeak) st.micPeak = a;
    }
    st.micRms = std::sqrt(micSumSq / static_cast<double>(DSP_SAMPLE_SIZE));

    double iqSumSq = 0.0;
    for (int i = 0; i < iq.size(); ++i) {
        const double reRaw = iq.at(i).re;
        const double imRaw = iq.at(i).im;
        const double re = std::isfinite(reRaw) ? reRaw : 0.0;
        const double im = std::isfinite(imRaw) ? imRaw : 0.0;
        const double p = re * re + im * im;
        iqSumSq += p;
        const double a = std::sqrt(p);
        if (a > st.iqPeak) st.iqPeak = a;
    }
    if (!iq.isEmpty())
        st.iqRms = std::sqrt(iqSumSq / static_cast<double>(iq.size()));
    return st;
}
} // namespace


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
	, m_serverMode(set->getCurrentServerMode())
	, m_hwInterface(set->getHWInterface())
	, m_dataEngineState(QSDR::DataEngineDown)
	, m_meterType(SIGNAL_STRENGTH)
	, m_restart(false)
	, m_networkDeviceRunning(false)
	, m_soundFileLoaded(false)
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

    cw_key_down = 0;
    cw_sidetone_down = 0;
    TX.setSidetoneFrequency(static_cast<double>(m_cw_sidetone_freq));

	m_clientConnected = false;

	//currentRx = 0;
	m_discoverer= nullptr;
	m_dataIO = new DataIO();
	m_dataIO->setDataEngine(this);
	m_dataIO->setRadioModel(m_radioModel);
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

	metisFW = 0;
	hermesFW = 0;
	mercuryFW = 0;
    txParams().use_repeaterOffset = set->get_repeaterMode();

    //m_audioBuffer.resize(0);
    //m_audiobuf.resize(IO_BUFFER_SIZE);

	m_counter = 0;
	soapyInputSampleRate = set->getSampleRate();
	samplerate = set->getSampleRate();
	m_dataIO->setSampleRate(samplerate);
	if (m_radioModel)
		m_radioModel->setSampleRate(samplerate);

	m_threadFactory = new DataEngineThreadFactory(this);
	m_firmware = new DataEngineFirmware(this);
	m_lifecycle = new DataEngineLifecycle(this);
	m_soapy = new DataEngineSoapy(this);
}


TCCParameterTx& DataEngine::txParams()
{
	Q_ASSERT(m_radioModel);
	return m_radioModel->txParams();
}

const TCCParameterTx& DataEngine::txParams() const
{
	Q_ASSERT(m_radioModel);
	return m_radioModel->txParams();
}

int DataEngine::receivers() const
{
	return m_radioModel ? m_radioModel->activeReceivers() : 1;
}

void DataEngine::setReceiversCount(int count)
{
	if (m_radioModel)
		m_radioModel->setActiveReceivers(count);
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

    delete m_dataIO;
    m_dataIO = nullptr;

    delete m_threadFactory;
    m_threadFactory = nullptr;
    delete m_firmware;
    m_firmware = nullptr;
    delete m_lifecycle;
    m_lifecycle = nullptr;
    delete m_soapy;
    m_soapy = nullptr;
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

    CHECKED_CONNECT(
            set,
            &Settings::txFullDuplexChanged,
            this,
            &DataEngine::setTxFullDuplex);

    // CW / TX DSP live on TransmitModel; Settings getters forward there.
    if (TransmitModel* tx = m_radioModel ? m_radioModel->transmit() : nullptr) {
        connect(tx, &TransmitModel::cwHangTimeChanged, this, &DataEngine::CwHangTimeChanged);
        connect(tx, &TransmitModel::cwSidetoneFreqChanged, this, &DataEngine::CwSidetoneFreqChanged);
        connect(tx, &TransmitModel::cwKeyReversedChanged, this, [this](bool rev) {
            CwKeyReversedChanged(rev ? 1 : 0);
        });
        connect(tx, &TransmitModel::cwKeyerModeChanged, this, &DataEngine::CwKeyerModeChanged);
        connect(tx, &TransmitModel::internalCwChanged, this, [this](bool on) {
            InternalCwChanged(on ? 1 : 0);
        });
        connect(tx, &TransmitModel::cwKeyerSpeedChanged, this, &DataEngine::CwKeyerSpeedChanged);
        connect(tx, &TransmitModel::cwPttDelayChanged, this, &DataEngine::CwPttDelayChanged);
        connect(tx, &TransmitModel::cwSidetoneVolumeChanged, this, &DataEngine::CwSidetoneVolumeChanged);
        connect(tx, &TransmitModel::cwKeyerWeightChanged, this, &DataEngine::CwKeyerWeightChanged);
        connect(tx, &TransmitModel::cwKeyerSpacingChanged, this, [this](bool on) {
            CwKeyerSpacingChanged(on ? 1 : 0);
        });
    } else {
        CHECKED_CONNECT(set, &Settings::CwHangTimeChanged, this, &DataEngine::CwHangTimeChanged);
        CHECKED_CONNECT(set, &Settings::CwSidetoneFreqChanged, this, &DataEngine::CwSidetoneFreqChanged);
        connect(set, &Settings::CwKeyReversedChanged, this, &DataEngine::CwKeyReversedChanged);
        CHECKED_CONNECT(set, &Settings::CwKeyerModeChanged, this, &DataEngine::CwKeyerModeChanged);
        CHECKED_CONNECT(set, &Settings::InternalCwChanged, this, &DataEngine::InternalCwChanged);
        CHECKED_CONNECT(set, &Settings::CwKeyerSpeedChanged, this, &DataEngine::CwKeyerSpeedChanged);
        CHECKED_CONNECT(set, &Settings::CwPttDelayChanged, this, &DataEngine::CwPttDelayChanged);
        CHECKED_CONNECT(set, &Settings::CwSidetoneVolumeChanged, this, &DataEngine::CwSidetoneVolumeChanged);
        CHECKED_CONNECT(set, &Settings::CwKeyerWeightChanged, this, &DataEngine::CwKeyerWeightChanged);
        CHECKED_CONNECT(set, &Settings::CwKeyerSpacingChanged, this, &DataEngine::CwKeyerSpacingChanged);
    }


}

//********************************************************
// start/stop data engine
bool DataEngine::startDataEngineWithoutConnection() {
	return m_lifecycle->startDataEngineWithoutConnection();
}

bool DataEngine::findHPSDRDevices() {
	return m_lifecycle->findHPSDRDevices();
}

bool DataEngine::getFirmwareVersions() {
	return m_firmware->getFirmwareVersions();
}

bool DataEngine::checkFirmwareVersions() {
	return m_firmware->checkFirmwareVersions();
}

bool DataEngine::start() {
	return m_lifecycle->start();
}

void DataEngine::stop() {
	m_lifecycle->stop();
}

bool DataEngine::initDataEngine() {
	return m_lifecycle->initDataEngine();
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
            if (TciServer *tci = set->tciServer()) {
                connect(rx, &SliceProcessor::rxAudioSamples, tci, &TciServer::onRxAudioSamples,
                        Qt::QueuedConnection);
                connect(rx, &SliceProcessor::rxIqSamples, tci, &TciServer::onRxIqSamples,
                        Qt::QueuedConnection);
            }
         //   connect(rx.get(), &SliceProcessor::outputBufferSignal, m_dataProcessor, &DataProcessor::setOutputBuffer);

            if (m_cwIO) {
                connect(m_cwIO, &iambic::key_down,
                        rx, &SliceProcessor::cwKeyDown, Qt::DirectConnection);
            }

			m_dspThreadList.append(thread);
			RX.append(rx);
		}
		else {

			return false;
		}
    }
    set->setRxList(RX);

	currentReceiver = 0;
	setReceiversCount(rcvrs);

	timing = 0;
	m_configure = receivers() + 1;

	// init cc Rc parameters
	ccRx.devices.mercuryFWVersion = 0;
	ccRx.devices.penelopeFWVersion = 0;
	ccRx.devices.pennylaneFWVersion = 0;
	ccRx.devices.hermesFWVersion = 0;
	ccRx.devices.metisFWVersion = 0;

	ccRx.ptt    = false;
	ccRx.dash   = false;
	ccRx.dot    = false;
	ccRx.lt2208 = false;
	ccRx.ain1   = 0;
	ccRx.ain2   = 0;
	ccRx.ain3   = 0;
	ccRx.ain4   = 0;
	ccRx.ain5   = 0;
	ccRx.ain6   = 0;
	ccRx.hermesI01 = false;
	ccRx.hermesI02 = false;
	ccRx.hermesI03 = false;
	ccRx.hermesI04 = false;
	ccRx.mercury1_LT2208 = false;
	ccRx.mercury2_LT2208 = false;
	ccRx.mercury3_LT2208 = false;
	ccRx.mercury4_LT2208 = false;

	// init cc Tx parameters
	txParams().currentBand = set->getCurrentHamBand(0);
	txParams().mercuryAttenuators = set->getMercuryAttenuators(0);
	txParams().mercuryAttenuator = txParams().mercuryAttenuators.at(txParams().currentBand);
	txParams().dither = set->getMercuryDither();
	txParams().random = set->getMercuryRandom();
	txParams().duplex = set->getTxFullDuplex() ? 1 : 0;
	txParams().mox = false;
	txParams().ptt = false;
	txParams().alexStates = set->getAlexStates();
	txParams().vnaMode = false;
	txParams().alexConfig = set->getAlexConfig();
	txParams().timeStamp = 0;
	txParams().commonMercuryFrequencies = 0;
	txParams().pennyOCenabled = set->getPennyOCEnabled();
	txParams().rxJ6pinList = set->getRxJ6Pins();
	txParams().txJ6pinList = set->getTxJ6Pins();
	// Protocol 1 TX control (C0/C1..C4 state 1) uses txParams().txFrequency.
	// Keep it initialized to the currently selected receiver center frequency.
	txParams().txFrequency = set->getCtrFrequency(set->getCurrentReceiver());

	setAlexConfiguration(txParams().alexConfig);

	rxClass = set->getRxClass();
	mic_gain = 0.26F;
	rx_freq_change = -1;
	tx_freq_change = -1;
	clients = 0;
	sendIQ_toggle = true;
	rcveIQ_toggle = false;


	//*****************************
	// C&C bytes
	for (int i = 0; i < 5; i++) {
		control_out[i] = 0x00;
	}

	// C0
	// 0 0 0 0 0 0 0 0
	//               |
	//               +------------ MOX (1 = active, 0 = inactive)

	control_out[0] |= MOX_DISABLED;

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

	control_out[1] &= 0x03; // 0 0 0 0 0 0 1 1
	control_out[1] |= txParams().clockByte;

	// set C2
	//
	// 0 0 0 0 0 0 0 0
	// |           | |
	// |           | +------------ Mode (1 = Class E, 0 = All other modes)
    // +---------- +-------------- Open Collector Outputs on Penelope or Hermes (bit 6...bit 0)

	control_out[2] = control_out[2] & 0xFE; // 1 1 1 1 1 1 1 0
	control_out[2] = control_out[2] | rxClass;

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

	control_out[3] = control_out[3] & 0xFB; // 1 1 1 1 1 0 1 1
	control_out[3] = control_out[3] | (txParams().mercuryAttenuator << 2);

	control_out[3] = control_out[3] & 0xF7; // 1 1 1 1 0 1 1 1
	control_out[3] = control_out[3] | (txParams().dither << 3);

	control_out[3] = control_out[3] & 0xEF; // 1 1 1 0 1 1 1 1
	control_out[3] = control_out[3] | (txParams().random << 4);

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

	control_out[4] &= 0x07; // 1 1 0 0 0 1 1 1
	// Protocol 1 FPGA duplex must stay on for independent RX/TX NCOs (CTUN).
	control_out[4] = (1 << 2) | ((receivers() - 1) << 3);

	if (!m_radioController) {
		m_radioController = std::make_unique<RadioController>(this);
	}
	m_radioController->bind(m_radioModel, this);

	return true;
}

void DataEngine::setHPSDRConfig() {

	txParams().clockByte = 0x0;

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

		txParams().clockByte = MIC_SOURCE_PENELOPE | MERCURY_PRESENT | PENELOPE_PRESENT | MERCURY_122_88MHZ_SOURCE | ATLAS_10MHZ_SOURCE;
	}
	else if ((set->getPenelopePresence() || set->getPennyLanePresence()) && (set->get10MHzSource() == 1)) {
		
		txParams().clockByte = MIC_SOURCE_PENELOPE | MERCURY_PRESENT | PENELOPE_PRESENT | MERCURY_122_88MHZ_SOURCE | PENELOPE_10MHZ_SOURCE;
	}
	else if ((set->getPenelopePresence() || set->getPennyLanePresence()) && (set->get10MHzSource() == 2)) {
		
		txParams().clockByte = MIC_SOURCE_PENELOPE | MERCURY_PRESENT | PENELOPE_PRESENT | MERCURY_122_88MHZ_SOURCE | MERCURY_10MHZ_SOURCE;
	}
	else if ((set->get10MHzSource() == 0) || set->getExcaliburPresence()) {
		
		txParams().clockByte = MERCURY_PRESENT | MERCURY_122_88MHZ_SOURCE | ATLAS_10MHZ_SOURCE;
	}
	else {
		
		txParams().clockByte = MERCURY_PRESENT | MERCURY_122_88MHZ_SOURCE | MERCURY_10MHZ_SOURCE;
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
	m_threadFactory->createDiscoverer();
}

bool DataEngine::startDiscoverer(QThread::Priority prio) {
	return m_threadFactory->startDiscoverer(prio);
}

void DataEngine::stopDiscoverer() {
	m_threadFactory->stopDiscoverer();
}

//********************************************************
// create, start/stop data receiver

void DataEngine::createDataIO() {
	m_threadFactory->createDataIO();
}

bool DataEngine::startDataIO(QThread::Priority prio) {
	return m_threadFactory->startDataIO(prio);
}

void DataEngine::stopDataIO() {
	m_threadFactory->stopDataIO();
}
 
//********************************************************
// create, start/stop data processor

void DataEngine::createDataProcessor() {
	m_threadFactory->createDataProcessor();
}

bool DataEngine::startDataProcessor(QThread::Priority prio) {
	return m_threadFactory->startDataProcessor(prio);
}

void DataEngine::stopDataProcessor() {
	m_threadFactory->stopDataProcessor();
}

//********************************************************
// create, start/stop audio out processor

void DataEngine::createAudioOutProcessor() {
	m_threadFactory->createAudioOutProcessor();
}

__attribute__((unused)) void DataEngine::startAudioOutProcessor(QThread::Priority prio) {

	Q_UNUSED (prio)
}

void DataEngine::stopAudioOutProcessor() {
	m_threadFactory->stopAudioOutProcessor();
}

//********************************************************
// create, start/stop winde band data processor

void DataEngine::createWideBandDataProcessor() {
	m_threadFactory->createWideBandDataProcessor();
}

bool DataEngine::startWideBandDataProcessor(QThread::Priority prio) {
	return m_threadFactory->startWideBandDataProcessor(prio);
}

void DataEngine::stopWideBandDataProcessor() {
	m_threadFactory->stopWideBandDataProcessor();
}

void DataEngine::setWideBandBufferCount()
{
	// if we have 4096 * 16 bit = 8 * 1024 raw consecutive ADC samples, m_wbBuffers = 8
	// we have 16384 * 16 bit = 32 * 1024 raw consecutive ADC samples, m_wbBuffers = 32
	int wbBuffers = 0;
	if (mercuryFW > 32 || hermesFW > 11)
		wbBuffers = BIGWIDEBANDSIZE / 512;
	else
		wbBuffers = SMALLWIDEBANDSIZE / 512;

	set->setWidebandBuffers(wbBuffers);

}
//********************************************************
// create, start/stop audio receiver

void DataEngine::createAudioReceiver() {
	m_threadFactory->createAudioReceiver();
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

	QMutexLocker locker(&mutex);
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
	// Do not hold networkIOMutex across this emit: DataIO logs/enqueues under
	// that mutex, and UI slots may re-enter Settings. QMutex is not recursive.
	set->setSystemState(err, hwmode, statemode, enginestate);
}

void DataEngine::onDataIoReady()
{
	if (!m_dataIOThreadRunning) {
		DATA_ENGINE_DEBUG << "onDataIoReady: DataIO already stopped; ignoring";
		return;
	}
	m_networkDeviceRunning = true;
	setSystemState(QSDR::NoError, m_hwInterface, m_serverMode, QSDR::DataEngineUp);
	set->setSystemMessage("System running", 4000);
	DATA_ENGINE_DEBUG << "Data Engine thread: " << thread();
}

void DataEngine::onDataIoStartupFailed()
{
	if (!m_dataIOThreadRunning) {
		DATA_ENGINE_DEBUG << "onDataIoStartupFailed: DataIO already stopped; ignoring";
		return;
	}
	DATA_ENGINE_DEBUG << "DataIO startup failed (no bound receiver socket)";
	m_networkDeviceRunning = false;
	setSystemState(QSDR::DataReceiverThreadError, m_hwInterface, m_serverMode, QSDR::DataEngineDown);
	set->setMainPower(false);
	set->setSystemMessage("Failed to bind receiver socket", 8000);
}

float DataEngine::getFilterSizeCalibrationOffset() {

    //int size=1024; // dspBufferSize
    float i = log10((qreal) BUFFER_SIZE);
    return 3.0f*(11.0f - i);
}

void DataEngine::searchHpsdrNetworkDevices() {
	m_lifecycle->searchHpsdrNetworkDevices();
}

#ifdef HAVE_SOAPYSDR
void DataEngine::searchSoapyDevices() {
	m_soapy->searchSoapyDevices();
}

bool DataEngine::startSoapyEngine() {
	return m_soapy->startSoapyEngine();
}
#endif

void DataEngine::setHPSDRDeviceNumber(int value) {

	m_hpsdrDevices = value;
}

void DataEngine::rxListChanged(QList<SliceProcessor *> list) {

	QMutexLocker locker(&mutex);
	RX = list;
}

void DataEngine::setCurrentReceiver(int rx) {

	QMutexLocker locker(&mutex);
	currentReceiver = rx;
	txParams().txFrequency = set->getCtrFrequency(rx);
}

void DataEngine::setFramesPerSecond(int rx, int value) {

	Q_UNUSED(rx)
	Q_UNUSED(value)

	/*mutex.lock();
	if (m_fpsList.length() > 0)
		m_fpsList[rx] = (int)(1000000.0/value);
	mutex.unlock();*/
}

void DataEngine::setSampleRate(int value) {

	bool applyOk = true;

	if (set && set->getSampleRate() != value) {
		DATA_ENGINE_DEBUG << "sample-rate propagation mismatch: signal=" << value
		                  << "settings=" << set->getSampleRate();
	}

	bool shouldRequestP2Update = false;
	{
	QMutexLocker locker(&mutex);

	switch (value) {
	
		case 48000:
			samplerate = value;
			speed = 0;
			break;
			
		case 96000:
			samplerate = value;
			speed = 1;
			break;
			
		case 192000:
			samplerate = value;
			speed = 2;
			break;
			
		case 384000:
			samplerate = value;
			speed = 3;
			break;

        case 768000:
            samplerate = value;
            speed = 4;
            break;

        case 1536000:
            samplerate = value;
            speed = 5;
            break;

		default:
			DATA_ENGINE_DEBUG << "invalid sample rate !\n";
			applyOk = false;
			break;
	}
	if (applyOk && m_dataIO)
		m_dataIO->setSampleRate(samplerate);
	if (applyOk && m_radioModel)
		m_radioModel->setSampleRate(samplerate);

	shouldRequestP2Update = applyOk && m_protocol && set->getCurrentMetisCard().protocol == 2 && m_dataProcessor;

	if (samplerate != value) {
		DATA_ENGINE_DEBUG << "samplerate apply mismatch: requested=" << value
		                  << "applied=" << samplerate;
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

	QMutexLocker locker(&mutex);
	txParams().mercuryAttenuator = value;
}

void DataEngine::setMercuryAttenuators(QList<int> attn) {

	QMutexLocker locker(&mutex);
	txParams().mercuryAttenuators = attn;
}

void DataEngine::setDither(int value) {

	QMutexLocker locker(&mutex);
	txParams().dither = value;
}

void DataEngine::setRandom(int value) {

	QMutexLocker locker(&mutex);
	txParams().random = value;
}

void DataEngine::set10MhzSource(int source) {

	QMutexLocker locker(&mutex);
	control_out[1] = control_out[1] & 0xF3;
	control_out[1] = control_out[1] | (source << 2);
}

void DataEngine::set122_88MhzSource(int source) {

	QMutexLocker locker(&mutex);
	control_out[1] = control_out[1] & 0xEF;
	control_out[1] = control_out[1] | (source << 4);
}

void DataEngine::setMicSource( int source) {

	QMutexLocker locker(&mutex);
	control_out[1] = control_out[1] & 0x7F;
	control_out[1] = control_out[1] | (source << 7);
}

void DataEngine::setMercuryClass(int value) {

	QMutexLocker locker(&mutex);
	rxClass = value;
}

void DataEngine::setMercuryTiming(int value) {

	QMutexLocker locker(&mutex);
	timing = value;
}

void DataEngine::setAlexConfiguration(quint16 conf) {

	{
		QMutexLocker locker(&mutex);
		txParams().alexConfig = conf;
		DATA_ENGINE_DEBUG << "Alex Configuration = " << txParams().alexConfig;
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
		QMutexLocker locker(&mutex);
		qDebug() << "setAlexStates: band=" << band << "states=" << states;
		txParams().alexStates = states;
		DATA_ENGINE_DEBUG << "Alex States = " << txParams().alexStates;
	}

	if (set->getCurrentMetisCard().protocol == 2 && m_dataProcessor) {
		QMetaObject::invokeMethod(m_dataProcessor,
			&DataProcessor::requestProtocol2HPUpdate,
			Qt::QueuedConnection);
	}
}

void DataEngine::setPennyOCEnabled(bool value) {

	QMutexLocker locker(&mutex);
	txParams().pennyOCenabled = value;
}

void DataEngine::setRxJ6Pins(const QList<int> &list) {

	QMutexLocker locker(&mutex);
	txParams().rxJ6pinList = list;

}

void DataEngine::setTxJ6Pins(const QList<int> &list) {

	QMutexLocker locker(&mutex);
	txParams().txJ6pinList = list;
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

	DATA_ENGINE_DEBUG << "[RX-ADD] setNumberOfRx: requested=" << value << "current=" << receivers() << "allocated=" << RX.count();

	if (value < 1) value = 1;
	if (value > MAX_RECEIVERS) value = MAX_RECEIVERS;

	if (receivers() == value && RX.count() == value) {
		DATA_ENGINE_DEBUG << "[RX-ADD] receiver count unchanged, no action.";
		return;
	}

	if (m_dataEngineState != QSDR::DataEngineUp) {
		// Engine is down; update receiver count in model for next start
		QMutexLocker locker(&mutex);
		setReceiversCount(value);
		if (currentReceiver >= value) {
			currentReceiver = 0;
		}
		return;
	}

	// Engine is UP: dynamically scale SliceProcessors and DSP threads without tearing down network IO
	const QList<qint64> ctrFrequencies = set->getCtrFrequencies();

	// 1. Add new receiver slices if needed
	while (RX.count() < value) {
		int i = RX.count();
		if (!m_radioModel || i >= m_radioModel->slices().size()) break;

		DATA_ENGINE_DEBUG << "[RX-ADD] dynamically allocating slice processor for rx" << i;
		auto rx = new SliceProcessor(m_radioModel->slices().at(i), nullptr);
		if (!rx->initDSPInterface()) {
			DATA_ENGINE_DEBUG << "[RX-ADD] ERROR: failed to init DSP interface for rx" << i;
			delete rx;
			break;
		}

		rx->setConnectedStatus(true);
		rx->setServerMode(m_serverMode);

		auto thread = new QThreadEx();
		rx->moveToThread(thread);

		connect(rx, &SliceProcessor::spectrumBufferChanged, set, &Settings::setSpectrumBuffer);
		if (RadioTelemetry* tel = m_radioModel ? m_radioModel->telemetry() : nullptr) {
			connect(rx, &SliceProcessor::sMeterValueChanged, tel,
					[tel](int receiverId, double v) { tel->setSMeterValue(receiverId, v); });
		}
		if (TciServer *tci = set->tciServer()) {
			connect(rx, &SliceProcessor::rxAudioSamples, tci, &TciServer::onRxAudioSamples, Qt::QueuedConnection);
			connect(rx, &SliceProcessor::rxIqSamples, tci, &TciServer::onRxIqSamples, Qt::QueuedConnection);
		}
		if (m_cwIO) {
			connect(m_cwIO, &iambic::key_down, rx, &SliceProcessor::cwKeyDown, Qt::DirectConnection);
		}
		if (m_dataProcessor) {
			CHECKED_CONNECT(rx, &SliceProcessor::outputBufferSignal, m_dataProcessor, &DataProcessor::setOutputBuffer);
			CHECKED_CONNECT(rx, &SliceProcessor::audioBufferSignal, m_dataProcessor, &DataProcessor::send_hpsdr_data);
		}

		if (i < ctrFrequencies.count()) {
			setFrequency(true, i, ctrFrequencies.at(i));
		}

		thread->start(QThread::HighPriority);
		m_dspThreadList.append(thread);
		RX.append(rx);
	}

	// 2. Remove excess receiver slices if needed
	while (RX.count() > value) {
		int i = RX.count() - 1;
		DATA_ENGINE_DEBUG << "[RX-ADD] dynamically tearing down slice processor for rx" << i;
		SliceProcessor* rx = RX.takeAt(i);
		QThread* thread = (i < m_dspThreadList.count()) ? m_dspThreadList.takeAt(i) : nullptr;

		if (rx) {
			rx->stop();
			if (rx->qtwdsp) {
				rx->qtwdsp->stopChannel();
			}
			rx->disconnect();
		}
		if (thread) {
			thread->quit();
			thread->wait(500);
			delete thread;
		}
		if (rx) {
			delete rx;
		}
	}

	set->setRxList(RX);

	{
		QMutexLocker locker(&mutex);
		setReceiversCount(value);
		if (currentReceiver >= value) {
			currentReceiver = 0;
		}
		m_configure = receivers() + 1;
	}

	if (set->getCurrentReceiver() >= value) {
		set->setCurrentReceiver(0);
	}

	DATA_ENGINE_DEBUG << "[RX-ADD] dynamic receiver update complete: active=" << value << "allocated=" << RX.count();

	if (m_protocol && set->getCurrentMetisCard().protocol == 2 && m_dataProcessor) {
		DATA_ENGINE_DEBUG << "[RX-ADD] P2: queuing DDC+HP Run=1 setup burst for" << value << "receiver(s)";
		QMetaObject::invokeMethod(m_dataProcessor,
								  &DataProcessor::requestProtocol2ReceiverSetup,
								  Qt::QueuedConnection);
	}
}

void DataEngine::setTimeStamp(bool value) {

	if (timeStamp == value) return;

	QMutexLocker locker(&mutex);
	timeStamp = value;
	//control_out[4] &= 0xc7;
	//RRK control_out[4] |= value << 6;

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

	QMutexLocker locker(&mutex);
	RX[rx]->setReceiver(rx);
}

void DataEngine::setRxClient(int rx, int client) {

	QMutexLocker locker(&mutex);
	RX[rx]->setClient(client);
}

void DataEngine::setClientConnected(int rx) {

	if (!clientList.contains(rx)) {

		clientList.append(rx);
		audio_rx = rx;

		m_AudioRcvrThread->quit();
		m_AudioRcvrThread->wait();
		m_AudioRcvrThread->start();
	}
	else {

		sendIQ_toggle = true;
		// For Protocol 2, clearing rcveIQ_toggle here can immediately drop
		// the Run bit after startup and prevent RX from ever starting.
		if (set->getCurrentMetisCard().protocol != 2) {
			rcveIQ_toggle = false;
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

	QMutexLocker locker(&mutex);
	emit audioRxEvent(rx);
}

void DataEngine::setIQPort(int rx, int port) {

	QMutexLocker locker(&mutex);
	RX[rx]->setIQPort(port);
}

void DataEngine::setRxConnectedStatus(int rx, bool value) {

	QMutexLocker locker(&mutex);
	RX[rx]->setConnectedStatus(value);
}

void DataEngine::setHamBand(int rx, bool byBtn, HamBand band) {

	Q_UNUSED(rx)
	Q_UNUSED(byBtn)

	QMutexLocker locker(&mutex);
	txParams().currentBand = band;

	if (set->getCurrentMetisCard().protocol == 2 && m_dataProcessor) {
		QMetaObject::invokeMethod(m_dataProcessor,
			&DataProcessor::requestProtocol2HPUpdate,
			Qt::QueuedConnection);
	}
}

void DataEngine::setFrequency(int mode, int rx, qint64 frequency) {

	Q_UNUSED(mode)

	rx_freq_change = rx;
	if (rx == currentReceiver) {
		txParams().txFrequency = frequency;
	}

	// Protocol 2 includes DDC frequencies in the High Priority packet — push one
	// immediately so CAT/WSJT-X band changes retune without waiting for the cycle.
	if (set->getCurrentMetisCard().protocol == 2 && m_dataProcessor) {
		QMetaObject::invokeMethod(m_dataProcessor,
			&DataProcessor::requestProtocol2HPUpdate,
			Qt::QueuedConnection);
	}
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

#ifdef HAVE_SOAPYSDR
    m_soapyTxIqTimer = new QTimer(this);
    m_soapyTxIqTimer->setTimerType(Qt::PreciseTimer);
    connect(m_soapyTxIqTimer, &QTimer::timeout, this, &DataProcessor::pumpSoapyTxIqTimer);
#endif

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
#ifdef HAVE_RADE
	if (m_radeTx) {
		rade_close(m_radeTx);
		m_radeTx = nullptr;
	}
	if (m_lpcnetTx) {
		lpcnet_encoder_destroy(m_lpcnetTx);
		m_lpcnetTx = nullptr;
	}
#endif
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
    const int inputSamples = qMax(0, sampleCount);
    // Keep TX modem output continuous across brief mic underruns by reusing the
    // previously held FreeDV sample when no new mic block is available.
    const int outputSamples = (inputSamples > 0) ? inputSamples : DSP_SAMPLE_SIZE;
    if (inputSamples == 0) {
        static quint64 underrunCount = 0;
        if (++underrunCount % 200 == 1) {
            qDebug() << "FreeDV TX: mic underrun, reusing held modem sample"
                     << "count=" << underrunCount;
        }
    }

	// Use the selected Codec2 mode directly (0=1600, 1=1400, 2=1300, 3=700C, 4=2400, 5=3200, 6=700D, 100=RADE v1)
	const int wantedMode = set->getFreeDVMode(de->currentReceiver);
	if (m_freeDVTxMode != wantedMode) {
		if (m_freeDVTx) {
			freedv_close(m_freeDVTx);
			m_freeDVTx = nullptr;
		}
#ifdef HAVE_RADE
		if (m_radeTx) {
			rade_close(m_radeTx);
			m_radeTx = nullptr;
		}
		if (m_lpcnetTx) {
			lpcnet_encoder_destroy(m_lpcnetTx);
			m_lpcnetTx = nullptr;
		}
#endif
		m_freeDVTxMode = wantedMode;

		if (wantedMode == 100) {
#ifdef HAVE_RADE
			char emptyModel[1] = {0};
			m_radeTx = rade_open(emptyModel, RADE_USE_C_ENCODER);
			if (!m_radeTx) {
				DATA_PROCESSOR_DEBUG << "RADE TX failed to open";
				return;
			}
			m_lpcnetTx = lpcnet_encoder_create();
			m_freeDVSpeechRate = 16000;
			m_freeDVModemRate = 8000;
			m_freeDVTxNSpeech = LPCNET_FRAME_SIZE; // 160
			m_freeDVTxNModem = rade_n_tx_out(m_radeTx);
			m_radeTxFeaturesIn = rade_n_features_in_out(m_radeTx);
			m_freeDVTxDecim = 48000 / 16000; // 3
			m_freeDVTxInterp = 48000 / 8000; // 6
			m_radeTxSpeechAccum.clear();
			m_radeTxFeatureAccum.clear();
			m_radeTxFeatureAccum.reserve(qMax(m_radeTxFeaturesIn, NB_TOTAL_FEATURES) * 2);
			m_radeTxModemQueue.clear();
			m_radeTxModemReadPos = 0;
			m_freeDVTxHoldCount = 0;
			m_radeTxHeldSampleReal = 0.0f;
			m_radeTxHeldSampleImag = 0.0f;
#endif
		} else {
			m_freeDVTx = freedv_open(wantedMode);
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
			m_freeDVSpeechAccum.clear();
			m_freeDVModemQueue.clear();
			m_freeDVModemReadPos = 0;
			m_freeDVTxHoldCount = 0;
			m_freeDVTxHeldSample = 0;
		}
	}

	if (wantedMode == 100) {
#ifdef HAVE_RADE
		if (!m_radeTx) return;

		// Decimate input mic buffer to 16 kHz speech
		for (int i = 0; i + m_freeDVTxDecim <= inputSamples; i += m_freeDVTxDecim) {
			double sum = 0.0;
			for (int k = 0; k < m_freeDVTxDecim; ++k) {
				sum += mic_buffer[(i + k) * 2];
			}
			double s = sum / static_cast<double>(m_freeDVTxDecim);
            s *= static_cast<double>(radeTxSpeechGain());
            s = std::clamp(s, -1.0, 1.0);
			float speechVal = static_cast<float>(s * 32767.0);
			m_radeTxSpeechAccum.push_back(speechVal);
		}

		quint64 txFramesThisBlock = 0;
		while (static_cast<int>(m_radeTxSpeechAccum.size()) >= m_freeDVTxNSpeech) {
			// Convert speech accumulated floats to int16 for lpcnet encoder
			std::vector<int16_t> speech16k(m_freeDVTxNSpeech);
			for (int i = 0; i < m_freeDVTxNSpeech; ++i) {
				speech16k[i] = static_cast<int16_t>(m_radeTxSpeechAccum[i]);
			}
			m_radeTxSpeechAccum.erase(m_radeTxSpeechAccum.begin(), m_radeTxSpeechAccum.begin() + m_freeDVTxNSpeech);

			float features[NB_TOTAL_FEATURES];
			// Use generic LPCNet path to avoid a link-time dependency on opus_select_arch().
			lpcnet_compute_single_frame_features(m_lpcnetTx, speech16k.data(), features, 0);
			m_radeTxFeatureAccum.insert(
				m_radeTxFeatureAccum.end(), features, features + NB_TOTAL_FEATURES);

			while (m_radeTxFeaturesIn > 0
			       && static_cast<int>(m_radeTxFeatureAccum.size()) >= m_radeTxFeaturesIn) {
				std::vector<float> featuresIn(
					m_radeTxFeatureAccum.begin(),
					m_radeTxFeatureAccum.begin() + m_radeTxFeaturesIn);
				m_radeTxFeatureAccum.erase(
					m_radeTxFeatureAccum.begin(),
					m_radeTxFeatureAccum.begin() + m_radeTxFeaturesIn);

				std::vector<RADE_COMP> modemFrame(m_freeDVTxNModem);
				const int nTxOut = runWithSuppressedCStdio([&]() {
					return rade_tx(m_radeTx, modemFrame.data(), featuresIn.data());
				});

				for (int i = 0; i < nTxOut; ++i) {
					const float re = std::isfinite(modemFrame[i].real) ? modemFrame[i].real : 0.0f;
					// Feed RADE v1 into the existing TX audio->WDSP chain as mono modem audio.
					// This preserves TX shaping and avoids wideband splatter.
					m_radeTxModemQueue.push_back(re);
				}
				txFramesThisBlock += 1;
			}
		}

		for (int i = 0; i < outputSamples; ++i) {
			if (m_freeDVTxHoldCount <= 0) {
				if (m_radeTxModemReadPos < m_radeTxModemQueue.size()) {
					m_radeTxHeldSampleReal = m_radeTxModemQueue[m_radeTxModemReadPos++];
                    if (!std::isfinite(m_radeTxHeldSampleReal)) m_radeTxHeldSampleReal = 0.0f;
				} else {
					m_radeTxHeldSampleReal = 0.0f;
				}
				m_freeDVTxHoldCount = m_freeDVTxInterp;
			}

			mic_buffer[i * 2] = m_radeTxHeldSampleReal;
			mic_buffer[i * 2 + 1] = 0.0f;
			m_freeDVTxHoldCount -= 1;
		}

		if (txFramesThisBlock > 0) {
			set->addFreeDVTxFrames(de->currentReceiver, txFramesThisBlock);
		}

		if (m_radeTxModemReadPos > 0 && m_radeTxModemReadPos >= m_radeTxModemQueue.size()) {
			m_radeTxModemQueue.clear();
			m_radeTxModemReadPos = 0;
		}
#endif
		return;
	}

	if (!m_freeDVTx || m_freeDVTxNSpeech <= 0 || m_freeDVTxNModem <= 0) return;

	for (int i = 0; i + m_freeDVTxDecim <= inputSamples; i += m_freeDVTxDecim) {
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

	for (int i = 0; i < outputSamples; ++i) {
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
		set->addFreeDVTxFrames(de->currentReceiver, txFramesThisBlock);
	}

	if (m_freeDVModemReadPos > 0 && m_freeDVModemReadPos * 2 >= m_freeDVModemQueue.size()) {
		m_freeDVModemQueue.erase(m_freeDVModemQueue.begin(), m_freeDVModemQueue.begin() + m_freeDVModemReadPos);
		m_freeDVModemReadPos = 0;
	}
}
#endif

void DataProcessor::stop() {

	m_stopped = true;
#ifdef HAVE_SOAPYSDR
	stopSoapyTxIqTimer();
#endif
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

	if (!de->rcveIQ_toggle) return; // don't send HP mid-setup (Run not yet asserted)

	unsigned char p2CmdBuf[1444];
	quint16 port = DEVICE_PORT;
	int hpState = 3; // High Priority Data Packet

	memset(p2CmdBuf, 0, sizeof(p2CmdBuf));
	de->m_protocol->encodeCCBytes(p2CmdBuf, de, de->m_radioModel, hpState, port);

	if (port != 1027) return;

	m_deviceAddress = de->m_dataIO->hpsdrDeviceIPAddress;
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
	if (sent < 0) {
		DATA_PROCESSOR_DEBUG << "P2 HP update send via DataIO failed";
	}
}

void DataProcessor::requestProtocol2DDCUpdate() {
	if (!de || !de->m_protocol || !de->m_controlSocket) return;
	if (de->set->getCurrentMetisCard().protocol != 2) return;

	unsigned char p2CmdBuf[1444];
	quint16 port = DEVICE_PORT;
	int oneShotState = 1; // DDC Specific packet

	memset(p2CmdBuf, 0, sizeof(p2CmdBuf));
	de->m_protocol->encodeCCBytes(p2CmdBuf, de, de->m_radioModel, oneShotState, port);

	if (port != 1025) {
		DATA_PROCESSOR_DEBUG << "P2 rate update produced unexpected control port" << port;
		return;
	}

	m_deviceAddress = de->m_dataIO->hpsdrDeviceIPAddress;
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
	if (sent < 0) {
		DATA_PROCESSOR_DEBUG << "error sending P2 DDC rate update via DataIO";
	}
}

void DataProcessor::requestProtocol2ReceiverSetup() {
    if (!de || !de->m_protocol || !de->m_controlSocket) return;
    if (de->set->getCurrentMetisCard().protocol != 2) return;

	DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] enter: receivers=" << de->receivers()
	                     << " allocated=" << de->RX.count()
	                     << " device=" << de->m_dataIO->hpsdrDeviceIPAddress;

	// Always ensure Run is asserted
	de->rcveIQ_toggle = true;

    unsigned char p2CmdBuf[1444];
    quint16 port = DEVICE_PORT;

	auto sendP2Control = [&](const char *data, int len, quint16 dstPort) -> qint64 {
		if (de->m_dataIO) {
			QByteArray dg(data, len);
			qint64 sent = -1;
			const bool invoked = QMetaObject::invokeMethod(
				de->m_dataIO,
				"sendProtocol2ControlDatagram",
				Qt::BlockingQueuedConnection,
				Q_RETURN_ARG(qint64, sent),
				Q_ARG(QByteArray, dg),
				Q_ARG(QHostAddress, m_deviceAddress),
				Q_ARG(quint16, dstPort));
			if (invoked && sent >= 0) {
				return sent;
			}
			DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] DataIO control send failed invoked="
			                     << invoked << " sent=" << sent;
			return -1;
		}
		return -1;
	};

    m_deviceAddress = de->m_dataIO->hpsdrDeviceIPAddress;

    // 0. Send General packet (port 1024)
    int genState = 0;
    memset(p2CmdBuf, 0, sizeof(p2CmdBuf));
    de->m_protocol->encodeCCBytes(p2CmdBuf, de, de->m_radioModel, genState, port);
    sendP2Control((const char*)p2CmdBuf, 60, port);

    // 1. Send DDC Specific packet (port 1025) with updated receiver-count/bitmask
	int ddcState = 1;
    memset(p2CmdBuf, 0, sizeof(p2CmdBuf));
    de->m_protocol->encodeCCBytes(p2CmdBuf, de, de->m_radioModel, ddcState, port);
    if (port == 1025) {
		qint64 ddcSent = sendP2Control((const char*)p2CmdBuf, 1444, port);
		DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] DDC Specific sent: bytes=" << ddcSent << " port=" << port;
    }

	// 2. Send TX Specific packet (port 1026)
	int txState = 2;
	memset(p2CmdBuf, 0, sizeof(p2CmdBuf));
	de->m_protocol->encodeCCBytes(p2CmdBuf, de, de->m_radioModel, txState, port);
	if (port == 1026) {
		qint64 txSent = sendP2Control((const char*)p2CmdBuf, 60, port);
		DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] TX Specific sent: bytes=" << txSent << " port=" << port;
	}

	// 3. Send High Priority packet (port 1027) with Run=1 and all active DDC frequencies
	int hpState = 3;
	port = DEVICE_PORT;
	memset(p2CmdBuf, 0, sizeof(p2CmdBuf));
	de->m_protocol->encodeCCBytes(p2CmdBuf, de, de->m_radioModel, hpState, port);
	if (port == 1027) {
		qint64 runSent = sendP2Control((const char*)p2CmdBuf, 1444, port);
		DATA_PROCESSOR_DEBUG << "[P2-RXSETUP] HP Run=1 sent: bytes=" << runSent << " port=" << port;
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

		TIQPacket packet = de->m_dataIO->iq_queue.dequeue();
		processInputBuffer(packet.payload, packet.sourcePort);

		if (de->m_dataIO->iq_queue.isFull()) {
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
    }
}

void DataProcessor::decodeCCBytes(const QByteArray &buffer) {
    if (de->m_protocol) {
        de->m_protocol->decodeCCBytes(buffer, de);
    }
}

void DataProcessor::setOutputBuffer(int rx, const CPX &buffer) {

    if (rx == de->currentReceiver) {
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

            de->m_dataIO->audioDatagram.resize(IO_BUFFER_SIZE);
            de->m_dataIO->audioDatagram = QByteArray::fromRawData((const char *)de->output_buffer, IO_BUFFER_SIZE);
            de->m_dataIO->sendAudio(de->output_buffer); //RRK
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
    de->output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
    de->output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
    de->output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
    de->output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];

}

void DataProcessor::add_rx_audio_sample() {
        qint16 leftRXSample;
        qint16 rightRXSample;
		const DSPMode rxMode = set->getDSPMode(de->currentReceiver);
		if (rxMode == FDV) {
			leftRXSample = 0;
			rightRXSample = 0;
		} else {
			leftRXSample = (qint16) (rx_audio_buffer[rx_audio_ptr].re * 32767.0f);
			rightRXSample = (qint16) (rx_audio_buffer[rx_audio_ptr].im * 32767.0f);
		}

        // Host CW sidetone — zero latency, mixed directly into the RX audio stream.
        if (de->cw_sidetone_down > 0) {
            if (m_sidetoneShape < RAMPLEN) ++m_sidetoneShape;
            --de->cw_sidetone_down;
        } else if (m_sidetoneShape > 0) {
            --m_sidetoneShape;
        }
        if (m_sidetoneShape > 0) {
            const double sidevol = de->m_cw_sidetone_volume * (32767.0 / 127.0);
            const double tone = sidevol * cwramp48[m_sidetoneShape]
                                * de->TX.getNextInternalSideToneSample();
            const qint16 st = static_cast<qint16>(qBound(-32768.0, tone, 32767.0));
            leftRXSample  = static_cast<qint16>(qBound(-32768, (int)leftRXSample  + st, 32767));
            rightRXSample = static_cast<qint16>(qBound(-32768, (int)rightRXSample + st, 32767));
        }

        de->output_buffer[m_idx++] = leftRXSample >> 8;
        de->output_buffer[m_idx++] = leftRXSample;
        de->output_buffer[m_idx++] = rightRXSample >> 8;
        de->output_buffer[m_idx++] = rightRXSample;
        rx_audio_ptr++;
    }

/* Sends RX Audio and tx iq data back to hpsdr. Always at 48 KHz bandwidth */

void DataProcessor::send_hpsdr_data(int rx, const CPX &buffer, int buffersize) {
#ifdef HAVE_SOAPYSDR
    // Soapy TX IQ is paced by m_soapyTxIqTimer (half- and full-duplex).
    if (m_hwInterface == QSDR::SoapySDR) {
        Q_UNUSED(buffer)
        Q_UNUSED(buffersize)
        Q_UNUSED(rx)
        return;
    }
#endif

    // Only send audio for the currently selected receiver.
    if (rx != de->currentReceiver) return;

    rx_audio_ptr = 0;
/* buffer rx audio */
    for (int j = 0; j < buffersize; j++)
        {
        rx_audio_buffer[j].re = buffer[j].re;
        rx_audio_buffer[j].im = buffer[j].im;
        }

    if (set->is_transmitting()) {
        if (!tx_index) get_tx_iqData();
    } else if (p1SoftwareCwActive() && de->cw_key_down > 0) {
        if (!tx_index) get_tx_iqData();
    } else {
        memset(&m_tx_iq_Buffer, 0x0, sizeof(m_tx_iq_Buffer));
    }
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
        static quint64 micTraceCounter = 0;
        if (txDiagEnabled() && (++micTraceCounter % 200) == 1) {
            qDebug() << "processMicData queue count:" << queueCount;
        }

        temp_data = de->m_audioInput->m_faudioInQueue.dequeue();
        // Only process the actual number of samples in the buffer
        int numSamples = qMin((int)temp_data.size(), DSP_SAMPLE_SIZE);
        for (int s = 0; s < numSamples; s++)
        {
            mic_buffer[(s * 2)]  = temp_data[s];
            mic_buffer[(s * 2) + 1] = 0.0f;
        }

        if (txDiagEnabled() && (micTraceCounter % 200) == 1) {
            qDebug() << "Mic buffer processed with" << numSamples
                     << "samples." << mic_buffer[0] << mic_buffer[1];
        }
    }
    else {
        temp_data.clear();
        memset(&mic_buffer, 0x0, sizeof(mic_buffer));
    }
    mic_buffer_index = 0;

}

void DataProcessor::add_mic_sample()
{
 //    de->output_buffer[m_idx++] = 0;
  //  de->output_buffer[m_idx++] = 0;
  //  de->output_buffer[m_idx++] = 0;
  //  de->output_buffer[m_idx++] = 0;
    de->output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
    de->output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
    de->output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
    de->output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
    if (tx_index >= 4096) tx_index = 0;
}

bool DataProcessor::p1SoftwareCwActive() const
{
    if (!set || !de)
        return false;
    if (set->getCurrentMetisCard().protocol == 2)
        return false;
    if (m_hwInterface != QSDR::Metis && m_hwInterface != QSDR::Hermes)
        return false;
    const DSPMode txMode = set->getDSPMode(de->currentReceiver);
    return (txMode == DSPMode::CWU || txMode == DSPMode::CWL) && !set->isInternalCw();
}

void DataProcessor::fillP1CwTxIqBuffer()
{
    const double gain = 32767.0 * (static_cast<double>(de->txParams().drivelevel) / 255.0);
    int idx = 0;
    for (int j = 0; j < DSP_SAMPLE_SIZE; ++j) {
        if (de->cw_key_down > 0) {
            if (m_cwShape < RAMPLEN)
                ++m_cwShape;
            --de->cw_key_down;
        } else if (m_cwShape > 0) {
            --m_cwShape;
        }

        const double ramp = cwramp48[m_cwShape];
        const long isample = 0;
        const long qsample = static_cast<long>(floor(gain * ramp + 0.5));

        m_tx_iq_Buffer[idx++] = static_cast<uchar>((isample >> 8) & 0xff);
        m_tx_iq_Buffer[idx++] = static_cast<uchar>(isample & 0xff);
        m_tx_iq_Buffer[idx++] = static_cast<uchar>((qsample >> 8) & 0xff);
        m_tx_iq_Buffer[idx++] = static_cast<uchar>(qsample & 0xff);
    }
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

    if ( de->txParams().mox ||  de->txParams().ptt ) {

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
    // Network TX audio (remote TCI/browser client mic) takes over the TX mic
    // input whenever frames are arriving; the local soundcard mic is the
    // fallback — except while TCI TX_CHRONO is active, when ExpertSDR-style
    // clients are the sole mic source and local capture must not leak in.
    QHQueue<AUDIOBUF> *srcQueue = nullptr;
    if (de->m_audioInput) {
        if (de->m_audioInput->m_netAudioInQueue.count() > 0)
            srcQueue = &de->m_audioInput->m_netAudioInQueue;
        else {
            const TciServer *tci = set ? set->tciServer() : nullptr;
            const bool networkMicOnly = tci && tci->isTxChronoActive();
            if (!networkMicOnly && de->m_audioInput->m_faudioInQueue.count() > 0)
                srcQueue = &de->m_audioInput->m_faudioInQueue;
        }
    }
    if (srcQueue)
    {
        temp_data = srcQueue->dequeue();

		numSamples = qMin((int)temp_data.size(), (int)DSP_SAMPLE_SIZE);
        for (int s = 0; s < numSamples; s++)
        {
            mic_buffer[(s * 2 )]  = temp_data[s] ;
            mic_buffer[(s * 2 ) + 1 ] = 0.0f;
        }

        // Diagnostic: Log if we are actually getting audio data
        static int nonZeroCount = 0;
        bool hasSignal = false;
        for (int i = 0; i < numSamples; ++i) {
            if (std::abs(temp_data[i]) > 1e-5) {
                hasSignal = true;
                break;
            }
        }
        if (hasSignal && txDiagEnabled()) {
            if (++nonZeroCount % 100 == 1) {
                qDebug() << "fetch_MicData: Dequeued block with signal. RMS approx:" << temp_data[0];
            }
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
        
        static int emptyCount = 0;
        if (txDiagEnabled() && ++emptyCount % 200 == 1) {
            const int micIndex = set->getMicInputDev();
            const QString micName = set->getMicInputSourceName();
            const int digitalIndex = set->getDigitalAudioInputDev();
            const QString digitalName = set->getDigitalInputSourceName();
            qDebug().nospace()
                << "fetch_MicData: Audio queue empty"
                << " txMode=" << set->getDSPMode(de->currentReceiver)
                << " micIndex=" << micIndex
                << " micSource=\"" << micName << "\""
                << " digIndex=" << digitalIndex
                << " digSource=\"" << digitalName << "\""
                << " audioInputObj=" << (de->m_audioInput ? "yes" : "no");
        }
    }

	// Keep WSJT-X digital-input handling (DIGU/DIGL) separate.
	// DV routing is only active for the FreeDV mode button (mapped to FDV).
	const DSPMode txMode = set->getDSPMode(de->currentReceiver);
	if (txMode == FDV) {
#ifdef HAVE_CODEC2
		applyCodec2ToMicBuffer(numSamples);
#endif
	}

    mic_buffer_index = 0;

}

/*  processes mic samples ready to transmit */
void DataProcessor::get_tx_iqData(){
    QMutexLocker pumpLock(&m_txIqPumpMutex);
    static quint64 txDiagCounter = 0;
    static quint64 txNanBlocks = 0;
    int error;
    long int   leftTXSample;
    long int rightTXSample;
    double is,qs;
    double gain = 32767.0f;
   // double gain = 25 * 0.00392;
    fetch_MicData();
    int micNonFinite = 0;
    for (int i = 0; i < DSP_SAMPLE_SIZE * 2; ++i) {
        if (!std::isfinite(mic_buffer[i])) {
            mic_buffer[i] = 0.0;
            ++micNonFinite;
        }
    }

    if (p1SoftwareCwActive() && (set->is_transmitting() || de->cw_key_down > 0)) {
        fillP1CwTxIqBuffer();
        return;
    }

    if (set->is_transmitting()) {
        fexchange0(TX_ID, mic_buffer, (double *) m_iq_output_buffer.data(), &error);
        int iqNonFinite = 0;
        for (int i = 0; i < m_iq_output_buffer.size(); ++i) {
            if (!std::isfinite(m_iq_output_buffer[i].re)) {
                m_iq_output_buffer[i].re = 0.0;
                ++iqNonFinite;
            }
            if (!std::isfinite(m_iq_output_buffer[i].im)) {
                m_iq_output_buffer[i].im = 0.0;
                ++iqNonFinite;
            }
        }
        if (micNonFinite > 0 || iqNonFinite > 0) {
            ++txNanBlocks;
            if (txNanBlocks % 20 == 1) {
                qWarning() << "TX stream: non-finite samples sanitized"
                           << "mic=" << micNonFinite
                           << "iq=" << iqNonFinite
                           << "mode=" << set->getDSPMode(de->currentReceiver);
            }
        }

		Spectrum0(1, TX_ID, 0, 0, (double *) m_iq_output_buffer.data());

        if (error != 0) {
            qWarning() << "TX stream: fexchange0(TX_ID) error=" << error
                       << "mode=" << set->getDSPMode(de->currentReceiver)
                       << "state=" << set->getRadioState();
        }

        if (txDiagEnabled() && (++txDiagCounter % 50) == 1) {
            const TxIqStats st = computeTxIqStats(mic_buffer, m_iq_output_buffer);
            qDebug().nospace() << "[TX-DIAG] mode=" << set->getDSPMode(de->currentReceiver)
                               << " state=" << set->getRadioState()
                               << " micQ=" << (de->m_audioInput ? de->m_audioInput->m_faudioInQueue.count() : -1)
                               << " micRms=" << st.micRms << " micPeak=" << st.micPeak
                               << " iqRms=" << st.iqRms << " iqPeak=" << st.iqPeak
                               << " fexchange=" << error;
        }

#ifdef HAVE_SOAPYSDR
        if (m_hwInterface == QSDR::SoapySDR && !set->getTxFullDuplex())
            publishTxSpectrumForPanadapter();
#endif

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
#ifdef HAVE_SOAPYSDR
        if (m_hwInterface == QSDR::SoapySDR && !de->m_dataIO->soapy_tx_iq_queue.isFull()) {
            QVector<float> soapyTxIq(DSP_SAMPLE_SIZE * 2);
            // LimeSDR needs Q conjugation to correct HPSDR-legacy sideband inversion.
            // Pluto/other Soapy devices should keep native IQ polarity.
            const bool negateQ = set->getSoapyHardwareKey().contains(QStringLiteral("Lime"), Qt::CaseInsensitive);
            for (int j = 0; j < DSP_SAMPLE_SIZE; ++j) {
                soapyTxIq[j * 2]     = static_cast<float>(m_iq_output_buffer.at(j).re);
                soapyTxIq[j * 2 + 1] = static_cast<float>(
                    negateQ ? -m_iq_output_buffer.at(j).im : m_iq_output_buffer.at(j).im);
            }
            de->m_dataIO->soapy_tx_iq_queue.enqueue(soapyTxIq);
        }
#endif
    }
}

#ifdef HAVE_SOAPYSDR
void DataProcessor::publishTxSpectrumForPanadapter() {
    if (!set->is_transmitting())
        return;

    constexpr int kTxPanPixels = 4096;
    if (m_txSpectrumBuffer.size() != kTxPanPixels)
        m_txSpectrumBuffer.resize(kTxPanPixels);

    int flag = 0;
    GetPixels(TX_ID, 0, m_txSpectrumBuffer.data(), &flag);
    if (!flag) {
        // TX analyzer runs asynchronously; a single immediate GetPixels() can miss,
        // especially in FDV where TX framing cadence is bursty.
        for (int i = 0; i < 12 && !flag; ++i) {
            QThread::usleep(500);
            GetPixels(TX_ID, 0, m_txSpectrumBuffer.data(), &flag);
        }
    }

    if (flag) {
        m_txSpectrumSeen = true;
        m_txSpectrumMissCount = 0;
        prepareTxPanadapterSpectrum(m_txSpectrumBuffer, set->getSampleRate());
        set->setSpectrumBuffer(de->currentReceiver, m_txSpectrumBuffer);
        return;
    }

    if (m_txSpectrumSeen) {
        // Keep the last valid TX trace visible instead of blanking the pane.
        set->setSpectrumBuffer(de->currentReceiver, m_txSpectrumBuffer);
    }

    if (txDiagEnabled() && ++m_txSpectrumMissCount % 200 == 1) {
        qDebug() << "TX panadapter: GetPixels miss count=" << m_txSpectrumMissCount
                 << "mode=" << set->getDSPMode(de->currentReceiver)
                 << "state=" << set->getRadioState();
    }
}

void DataProcessor::startSoapyTxIqTimer(int intervalMs) {
    if (!m_soapyTxIqTimer || m_hwInterface != QSDR::SoapySDR)
        return;
    m_soapyTxIqTimer->setInterval(qMax(1, intervalMs));
    if (!m_soapyTxIqTimer->isActive())
        m_soapyTxIqTimer->start();
    // Prime one block immediately so MOX does not wait for the first tick.
    pumpSoapyTxIqTimer();
}

void DataProcessor::stopSoapyTxIqTimer() {
    if (m_soapyTxIqTimer)
        m_soapyTxIqTimer->stop();
}

void DataProcessor::pumpSoapyTxIqTimer() {
    if (m_hwInterface == QSDR::SoapySDR && set->is_transmitting()) {
        const RadioState state = set->getRadioState();
        // Timer drives TX IQ for all mic input modes (TUNE and MOX).
        // fetch_MicData() drains whatever the soundcard has placed in m_faudioInQueue;
        // if nothing is available it substitutes zeros, which is correct for local mic.
        if (state == RadioState::TUNE || state == RadioState::MOX) {
            get_tx_iqData();
        }
    }
}

void DataProcessor::processSoapyMicData() {
    // TX IQ is timer-driven (pumpSoapyTxIqTimer) for all mic input modes.
    // The soundcard fills m_faudioInQueue; the timer drains it via get_tx_iqData().
    // Nothing to do here — the slot is kept to preserve the signal connection.
}
#endif

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
	const DSPMode rxMode = set->getDSPMode(de->currentReceiver);
	const bool muteAnalogRxForCodec2 = (rxMode == FDV);
    if (tx_index == 0) {
        if (set->is_transmitting() || (p1SoftwareCwActive() && de->cw_key_down > 0))
            get_tx_iqData();
    }
        for (int j = 0; j < buffersize; j++) {

			if (muteAnalogRxForCodec2) {
				leftRXSample = 0;
				rightRXSample = 0;
			} else {
				leftRXSample  = (qint16)(buffer.at(j).re * 32767.0f);
				rightRXSample = (qint16)(buffer.at(j).im * 32767.0f);
			}

            de->output_buffer[m_idx++] = leftRXSample  >> 8;
            de->output_buffer[m_idx++] = leftRXSample;
            de->output_buffer[m_idx++] = rightRXSample >> 8;
            de->output_buffer[m_idx++] = rightRXSample;
            de->output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
            de->output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
            de->output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
            de->output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];

            if (tx_index >= 4096) tx_index = 0;

 //   qDebug() << "buffer " << de->output_buffer[IO_HEADER_SIZE ] << de->output_buffer[IO_BUFFER_SIZE - 1] ;
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

					de->m_dataIO->audioDatagram.resize(IO_BUFFER_SIZE);
					de->m_dataIO->audioDatagram = QByteArray::fromRawData((const char *)de->output_buffer, IO_BUFFER_SIZE);

					//if (m_dataIOThreadRunning) {
					//	de->m_dataIO->writeData();
					//}

                    if ( de->txParams().mox ||  de->txParams().ptt )
                    {
                        /*
                       int val =   ((de->output_buffer[3]) &0xfe) >> 1;
                       qDebug() << "command" << val;
                       qDebug() << "C[0] " << " " << bin << de->output_buffer[3];
                       qDebug() << "C[1] " << " " << bin <<de->output_buffer[4];
                       qDebug() << "C[2] " << " " << bin <<de->output_buffer[5];
                       qDebug() << "C[3] " << " " << bin <<de->output_buffer[6];
                       qDebug() << "\n";
                         */

                    }

                    de->m_dataIO->sendAudio(de->output_buffer); //RRK

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
	const DSPMode rxMode = set->getDSPMode(de->currentReceiver);
	const bool muteAnalogRxForCodec2 = (rxMode == FDV);
    if (tx_index == 0) {
        if (set->is_transmitting() || (p1SoftwareCwActive() && de->cw_key_down > 0))
            get_tx_iqData();
    }
    for (int j = 0; j < buffersize; j++) {

		if (muteAnalogRxForCodec2) {
			leftRXSample = 0;
			rightRXSample = 0;
		} else {
			leftRXSample  = (qint16)(buffer.at(j).re * 32767.0f);
			rightRXSample = (qint16)(buffer.at(j).im * 32767.0f);
		}
        de->output_buffer[m_idx++] = leftRXSample  >> 8;
        de->output_buffer[m_idx++] = leftRXSample;
        de->output_buffer[m_idx++] = rightRXSample >> 8;
        de->output_buffer[m_idx++] = rightRXSample;
        de->output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
        de->output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
        de->output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];
        de->output_buffer[m_idx++] = m_tx_iq_Buffer[tx_index++];

        if (tx_index >= 4096) tx_index = 0;





        //   qDebug() << "buffer " << de->output_buffer[IO_HEADER_SIZE ] << de->output_buffer[IO_BUFFER_SIZE - 1] ;
        if (m_idx == IO_BUFFER_SIZE) {
            encodeCCBytes();
            switch (m_hwInterface) {

                case QSDR::Metis:
                case QSDR::Hermes:

                    de->m_dataIO->audioDatagram.resize(IO_BUFFER_SIZE);
                    de->m_dataIO->audioDatagram = QByteArray::fromRawData((const char *)de->output_buffer, IO_BUFFER_SIZE);


                    //if (m_dataIOThreadRunning) {
                    //	de->m_dataIO->writeData();
                    //}

                    if ( de->txParams().mox ||  de->txParams().ptt )
                    {
                        /*
                       int val =   ((de->output_buffer[3]) &0xfe) >> 1;
                       qDebug() << "command" << val;
                       qDebug() << "C[0] " << " " << bin << de->output_buffer[3];
                       qDebug() << "C[1] " << " " << bin <<de->output_buffer[4];
                       qDebug() << "C[2] " << " " << bin <<de->output_buffer[5];
                       qDebug() << "C[3] " << " " << bin <<de->output_buffer[6];
                       qDebug() << "\n";
                         */

                    }
                    qDebug() << "audio buffer sent";
                 //   de->m_dataIO->sendAudio(de->output_buffer); //RRK
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

//    qDebug() << sizeof(de->output_buffer);

		de->output_buffer[m_idx++] = leftRXSample  >> 8;
        de->output_buffer[m_idx++] = leftRXSample;
        de->output_buffer[m_idx++] = rightRXSample >> 8;
        de->output_buffer[m_idx++] = rightRXSample;
        de->output_buffer[m_idx++] = leftTXSample  >> 8;
        de->output_buffer[m_idx++] = leftTXSample;
        de->output_buffer[m_idx++] = rightTXSample >> 8;
        de->output_buffer[m_idx++] = rightTXSample;

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

					de->m_dataIO->audioDatagram.resize(IO_BUFFER_SIZE);
					de->m_dataIO->audioDatagram = QByteArray::fromRawData((const char *)de->output_buffer, IO_BUFFER_SIZE);

					//if (m_dataIOThreadRunning) {
					//	de->m_dataIO->writeData();
					//}

                 //   de->m_dataIO->sendAudio(de->output_buffer); //RRK
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
            de->m_protocol->encodeCCBytes(p2CmdBuf, de, de->m_radioModel, m_sendState, port);
            m_deviceAddress = de->m_dataIO->hpsdrDeviceIPAddress;
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
			if (sent < 0) {
				DATA_PROCESSOR_DEBUG << "error sending control data to device via DataIO";
            }
        } else {
            de->m_protocol->encodeCCBytes(de->output_buffer, de, de->m_radioModel, m_sendState, port);
        }
    }
}


void DataProcessor::writeData() {
    if (!de->m_protocol) return;

    // Protocol 2: formatOutputPacket returns the complete 1444-byte DUC IQ packet.
    // Send it in a single call to port 1029; bypass the P1 two-call toggle.
    if (de->set->getCurrentMetisCard().protocol == 2) {
        QByteArray ducPkt = de->m_protocol->formatOutputPacket(de->m_dataIO->audioDatagram, m_sendSequence);
        if (de->sendSocket->writeDatagram(ducPkt, m_deviceAddress, 1029) < 0) {
            DATA_PROCESSOR_DEBUG << "P2 TX: error sending DUC IQ:" << de->sendSocket->errorString();
        }
        m_oldSendSequence = m_sendSequence - 1; // keep tracking consistent
        return;
    }

	if (m_setNetworkDeviceHeader) {
        m_outDatagram = de->m_protocol->formatOutputPacket(de->m_dataIO->audioDatagram, m_sendSequence);
        m_setNetworkDeviceHeader = false;
    }
	else {
		m_outDatagram += de->m_dataIO->audioDatagram;

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
        txParams().use_repeaterOffset = mode;
}

void DataEngine::setTxFullDuplex(bool fullDuplex) {
    // Soapy / host-side RX-during-TX preference. Protocol 1 FPGA duplex is
    // forced ON in CProtocol1::encodeCCBytes (CTUN needs independent RX/TX NCOs).
    txParams().duplex = fullDuplex ? 1 : 0;

#ifdef HAVE_SOAPYSDR
    if (m_hwInterface != QSDR::SoapySDR)
        return;

    const RadioState txState = m_radioState;
    for (int i = 0; i < RX.size(); ++i) {
        if (!RX.at(i))
            continue;
        if (fullDuplex && (txState == RadioState::MOX || txState == RadioState::TUNE))
            RX.at(i)->m_state = RadioState::RX;
        else if (!fullDuplex && (txState == RadioState::MOX || txState == RadioState::TUNE))
            RX.at(i)->m_state = txState;
        else
            RX.at(i)->m_state = RadioState::RX;
    }
#endif
}

void DataEngine::dspModeChanged(int rx, DSPMode mode){
    Q_UNUSED(rx);
    txParams().mode = mode;
    TX.setDSPMode(1,mode);
}

void DataEngine::CwHangTimeChanged(int CwHangTime)
{
m_cw_hang_time = CwHangTime;
}

void DataEngine::CwSidetoneFreqChanged(int CwSidetoneFreq)
{
    m_cw_sidetone_freq = CwSidetoneFreq;
    TX.setSidetoneFrequency(static_cast<double>(CwSidetoneFreq));
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
	m_threadFactory->createAudioInputProcessor();
}

void DataEngine::decodeCCBytes(const QByteArray &buffer) {
    if (m_dataProcessor)
        m_dataProcessor->decodeCCBytes(buffer);
    else if (m_protocol)
        m_protocol->decodeCCBytes(buffer, this);
}

void DataEngine::set_tx_drivelevel(int value){

    qDebug() << "Drive level change" << value;
    txParams().drivelevel = value;

}

void DataEngine::radioStateChange(RadioState state) {

    m_radioState = state;

    if ((state == RadioState::MOX) || (state == RadioState::TUNE)) {
        txParams().mox = true;
        if (m_audioInput) {
            m_audioInput->clearTxQueues();
            m_audioInput->Start();
        }
    } else {
        txParams().mox = false;
        if (m_audioInput) {
            m_audioInput->Stop();
            m_audioInput->clearTxQueues();
        }
#ifdef HAVE_SOAPYSDR
        while (!m_dataIO->soapy_tx_iq_queue.isEmpty())
            m_dataIO->soapy_tx_iq_queue.dequeue();
#endif
    }

#ifdef HAVE_SOAPYSDR
	if (m_hwInterface == QSDR::SoapySDR && m_dataProcessor) {
		if (state == RadioState::MOX || state == RadioState::TUNE) {
			const int intervalMs = qMax(1,
			    static_cast<int>((static_cast<long long>(DSP_SAMPLE_SIZE) * 1000LL) / 48000LL));
			QMetaObject::invokeMethod(m_dataProcessor, "startSoapyTxIqTimer",
			                          Qt::QueuedConnection,
			                          Q_ARG(int, intervalMs));
		} else {
			QMetaObject::invokeMethod(m_dataProcessor, "stopSoapyTxIqTimer",
			                          Qt::QueuedConnection);
		}
	}
#endif

	// SliceProcessor::dspProcessing uses m_state for spectrum source.
	// Soapy full-duplex (txParams().duplex): stay in RX during MOX so panadapter/audio continue.
	// Soapy half-duplex: follow MOX/TUNE like HPSDR (RX WDSP path idles without IQ pump).
	for (int i = 0; i < RX.size(); ++i) {
		if (RX.at(i)) {
#ifdef HAVE_SOAPYSDR
			if (m_hwInterface == QSDR::SoapySDR && set->getTxFullDuplex())
				RX.at(i)->m_state = RadioState::RX;
			else
#endif
				RX.at(i)->m_state = state;
		}
	}
}

void DataProcessor::processReadData()
{
#ifdef HAVE_SOAPYSDR
    if (this->m_hwInterface == QSDR::SoapySDR) {
        while (!de->m_dataIO->soapy_iq_queue.isEmpty()) {
            QVector<float> samples = de->m_dataIO->soapy_iq_queue.dequeue();

            // TCI IQ is now tapped per-receiver in SliceProcessor::dspProcessingCore
            // (covers both HPSDR and SoapySDR) and delivered via a queued signal,
            // so it is no longer forwarded from this worker thread.

            for (int rx = 0; rx < de->RX.size(); ++rx) {
                if (de->RX[rx]) {
                    int soapyInputRate = 0;
                    {
                        QMutexLocker lock(&de->mutex);
                        soapyInputRate = de->soapyInputSampleRate;
                    }
                    if (soapyInputRate > 0) {
                        de->RX[rx]->setSoapyInputSampleRate(soapyInputRate);
                    }

                    // Use thread-safe push
                    de->RX[rx]->enqueueSoapyData(samples);
                    if (de->RX[rx]->trySetSoapyDspPending()) {
                        QMetaObject::invokeMethod(de->RX[rx], "dspProcessingSoapy", Qt::QueuedConnection);
                    }
                }
            }
        }
    }
#endif

	static quint64 p2ReadDataPackets = 0;
	TIQPacket packet;
    while(!de->m_dataIO->iq_queue.isEmpty()) {
	  packet = de->m_dataIO->iq_queue.dequeue();
	  const QByteArray &buf = packet.payload;
      if (de->m_protocol.get() && de->m_protocol.get()->getHeaderSize() == METIS_HEADER_SIZE) {
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
								   << " iqQueueRemaining=" << de->m_dataIO->iq_queue.count();
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

void DataProcessor::key_down(int state) {
    // cw_sidetone_down drives host sidetone regardless of internal/software mode.
    if (state) {
        de->cw_sidetone_down = 960000;
    } else {
        de->cw_sidetone_down = 0;
    }
    // cw_key_down drives software TX envelope — only needed when not using internal keyer.
    if (!set->isInternalCw()) {
        de->cw_key_down = de->cw_sidetone_down;
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
