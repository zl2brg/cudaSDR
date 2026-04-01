/**
* @file cusdr_receiver.cpp
* @brief cuSDR receiver class
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2010-11-12
*/

/* Copyright (C)
*
* 2010 - Hermann von Hasseln, DL3HVH
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.tw
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/
#define LOG_RECEIVER

// use: RECEIVER_DEBUG

#include "cusdr_receiver.h"

namespace {
constexpr int HIGH_RATE_TRANSITION_DROP_BUFFERS = 12;
}

Receiver::Receiver(int rx)
	: QObject()
	, set(Settings::instance())
	, m_stopped(false)
	, m_receiver(rx)
	, m_samplerate(set->getSampleRate())
	, m_audioMode(1)
	, m_rateTransitionDropBuffers(0)
	//, m_calOffset(63.0)
	//, m_calOffset(33.0)
{
	setReceiverData(set->getReceiverDataList().at(m_receiver));

	InitCPX(inBuf, BUFFER_SIZE, 0.0f);
	InitCPX(outBuf, BUFFER_SIZE, 0.0f);
    InitCPX(audioOutputBuf, BUFFER_SIZE, 0.0f);
    setAudioBufferSize();
    newSpectrum.resize(BUFFER_SIZE*4);
	highResTimer = std::make_unique<HResTimer>();
#ifdef USE_INTERNAL_AUDIO
    m_audioOutput = new ReceiverAudioOutput(this);
    m_audioOutput->start();
#endif
	setupConnections();
    m_displayTime = (int)(1000000.0/set->getFramesPerSecond(m_receiver));
	m_smeterTime.start();
}

Receiver::~Receiver() {
    qDebug() << "Destroy Receiver " << m_receiver;
    inBuf.clear();
    outBuf.clear();
	if (m_audioOutput) {
#ifdef USE_INTERNAL_AUDIO
		m_audioOutput->stop();
#endif
		delete m_audioOutput;
		m_audioOutput = nullptr;
	}
    m_stopped = false;
    delete qtwdsp;
}

void Receiver::setAudioBufferSize() {
    int scale=m_samplerate/48000;
    m_audiobuffersize = 1024/scale;
    RECEIVER_DEBUG << "set Audio buffer size to: " << m_audiobuffersize;
    }

void Receiver::setupConnections() {
    connect(set, &Settings::systemStateChanged,
            this, &Receiver::setSystemState);
    
    connect(set, &Settings::mainVolumeChanged,
            this, &Receiver::setAudioVolume);
    
    connect(set, &Settings::sampleRateChanged,
            this, &Receiver::setSampleRate);
    
    connect(set, &Settings::dspModeChanged,
            this, &Receiver::setDspMode);
    
    connect(set, &Settings::hamBandChanged,
            this, &Receiver::setHamBand);
    
    connect(set, &Settings::adcModeChanged,
            this, &Receiver::setADCMode);
    
    connect(set, &Settings::agcFixedGainChanged_dB,
            this, &Receiver::setAGCFixedGain_dB);
    
    connect(set, &Settings::filterFrequenciesChanged,
            this, &Receiver::setFilterFrequencies);
    
    connect(set, &Settings::framesPerSecondChanged,
            this, &Receiver::setFramesPerSecond);
}

void Receiver::setReceiverData(TReceiver data) {

	m_receiverData = data;
}

bool Receiver::initDSPInterface() {

	if (m_receiverData.dspCore == QSDR::QtDSP) {

        if (!initQtWDSPInterface()) return false;

	}
	return true;
}



bool Receiver::initQtWDSPInterface() {

    RECEIVER_DEBUG << "[RX-ADD] initQtWDSPInterface: rx=" << m_receiver << "BUFFER_SIZE=" << BUFFER_SIZE;
//    qtwdsp = std::make_unique<QWDSPEngine>(this, m_receiver, BUFFER_SIZE);
    qtwdsp = new QWDSPEngine(this, m_receiver, BUFFER_SIZE);

    if (!qtwdsp || !qtwdsp->isValid()) {  // Add validity check
        RECEIVER_DEBUG << "[RX-ADD] ERROR: could not start QWtDSP for receiver: " << m_receiver;
        return false;
    }
    RECEIVER_DEBUG << "[RX-ADD] QWDSPEngine constructed for rx=" << m_receiver << "(isValid=true)";

    qtwdsp->setQtDSPStatus(true);
    qtwdsp->setVolume(m_receiverData.audioVolume);

    DSPMode mode = m_receiverData.dspModeList.at(m_receiverData.hamBand);
    RECEIVER_DEBUG << "[RX-ADD] rx=" << m_receiver << "set DSP mode to:" << set->getDSPModeString(mode);

    qtwdsp->setDSPMode(mode);
    
    auto filter = getFilterFromDSPMode(set->getDefaultFilterList(), mode);
    qtwdsp->setFilter(filter.filterLo, filter.filterHi);

    RECEIVER_DEBUG << "[RX-ADD] initQtWDSPInterface: rx=" << m_receiver << "complete (filter lo=" << filter.filterLo << "hi=" << filter.filterHi << ")";
    return true;
}

void Receiver::enqueueRawData() {
    QVector<int32_t> rawBlock;
    rawBlock.reserve(BUFFER_SIZE * 2);
    for (int i = 0; i < BUFFER_SIZE * 2; ++i) {
        rawBlock.append(m_rawIQ[i]);
    }

    if (m_iqQueue.isFull()) {
        RECEIVER_DEBUG << "iqQueue full! dropping oldest packet";
        m_iqQueue.dequeue();
    }
    m_iqQueue.enqueue(rawBlock);
}

void Receiver::enqueueData() {
    // Legacy support or internal use
    if (m_iqQueue.isFull()) {
        RECEIVER_DEBUG << "iqQueue full! dropping oldest packet";
        m_iqQueue.dequeue();
    }
    // Convert CPX to raw int for now if this is ever called, or just do nothing
}

void Receiver::stop() {

	m_mutex.lock();
	m_stopped = true;
	m_mutex.unlock();
}

void Receiver::dspProcessing() {
	static quint64 dspEntryCount = 0;
	static quint64 dspEmptyQueueCount = 0;

	++dspEntryCount;
	if ((dspEntryCount % 100) == 1) {
		RECEIVER_DEBUG << "dspProcessing entry rx=" << m_receiver
					   << " count=" << dspEntryCount
					   << " iqQueue=" << m_iqQueue.count();
	}

	if (m_iqQueue.isEmpty()) {
		++dspEmptyQueueCount;
		if ((dspEmptyQueueCount % 100) == 1) {
			RECEIVER_DEBUG << "dspProcessing empty iqQueue rx=" << m_receiver
						   << " emptyCount=" << dspEmptyQueueCount;
		}
		return;
	}

	{
		QMutexLocker locker(&m_mutex);
		if (m_rateTransitionDropBuffers > 0) {
			m_iqQueue.dequeue();
			--m_rateTransitionDropBuffers;
			return;
		}
	}
    
    QVector<int32_t> rawIQ = m_iqQueue.dequeue();
    
    // Perform 24-bit integer to double conversion in this thread
    // This offloads work from the bottleneck DataProcessor thread.
    const double scale = 1.0 / 8388607.0;
    cpx* inPtr = inBuf.data(); // Trigger detach once
    const int32_t* rawPtr = rawIQ.constData();
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        inPtr[i].re = (double)rawPtr[2*i] * scale;
        inPtr[i].im = (double)rawPtr[2*i+1] * scale;
    }

    int spectrumDataReady;
    
    m_dspMutex.lock();
    m_dspCallTimer.start();
    qtwdsp->processDSP(inBuf, audioOutputBuf);
    double dspUs = m_dspCallTimer.nsecsElapsed() / 1000.0;
    m_dspMutex.unlock();

    ++m_dspCallCount;
    if (dspUs < m_dspTimeMin) m_dspTimeMin = dspUs;
    if (dspUs > m_dspTimeMax) m_dspTimeMax = dspUs;
    m_dspTimeAccum += dspUs;

    static constexpr quint64 DSP_REPORT_INTERVAL = 500;
    if ((m_dspCallCount % DSP_REPORT_INTERVAL) == 0) {
        double mean = m_dspTimeAccum / DSP_REPORT_INTERVAL;
        RECEIVER_DEBUG << "DSP perf rx=" << m_receiver
                       << " calls=" << m_dspCallCount
                       << " mean=" << QString::number(mean, 'f', 1) << "µs"
                       << " min="  << QString::number(m_dspTimeMin, 'f', 1) << "µs"
                       << " max="  << QString::number(m_dspTimeMax, 'f', 1) << "µs"
                       << " budget=" << QString::number(getDisplayDelay(), 'f', 0) << "µs"
                       << " iqQ=" << m_iqQueue.count();
        m_dspTimeAccum = 0.0;
        m_dspTimeMin = 1e9;
        m_dspTimeMax = 0.0;
    }

      if (highResTimer->getElapsedTimeInMicroSec() >= getDisplayDelay()) {

        
        if (m_state == RadioState::RX)
            GetPixels(m_receiver,0,qtwdsp->spectrumBuffer.data(), &spectrumDataReady);
        else {
            GetPixels(TX_ID, 0, qtwdsp->spectrumBuffer.data(), &spectrumDataReady);
            if (!spectrumDataReady) qDebug() << "Tx spectrum fetch fail";
        }

        if (spectrumDataReady) {
            newSpectrum = qtwdsp->spectrumBuffer;  // Direct assignment
            emit spectrumBufferChanged(m_receiver, newSpectrum);
        }
        
        highResTimer->start();
    }

    if (m_receiver == set->getCurrentReceiver()) {
        // Consider if this needs mutex protection too
        if (m_smeterTime.elapsed() > 200) {
            m_sMeterValue = qtwdsp->getSMeterInstValue();
            emit sMeterValueChanged(m_receiver, m_sMeterValue);
            m_smeterTime.restart();
        }
#ifdef USE_INTERNAL_AUDIO
        m_audioOutput->writeAudio(interleaveFromCPX(audioOutputBuf, m_audiobuffersize));
#endif
        emit audioBufferSignal(m_receiver, audioOutputBuf, m_audiobuffersize);
    }
}

QVector<float> Receiver::interleaveFromCPX(const CPX& in, int size) {
    int limit = (size < 0 || size > in.size()) ? in.size() : size;
    QVector<float> out(limit * 2); 
    float* outData = out.data();
    const cpx* inData = in.constData();

    for (int i = 0; i < limit; i++) {
        *outData++ = (float)inData[i].re;
        *outData++ = (float)inData[i].im;
    }
    return out;
}

void Receiver::setSampleRate(QObject *sender, int value) {
	Q_UNUSED(sender)

	if (m_samplerate == value) return;
    const int previousRate = m_samplerate;

	switch (value) {
		case 48000:
		case 96000:
		case 192000:
		case 384000:
		case 768000:
		case 1536000:
			m_samplerate = value;
			break;
		default:
			RECEIVER_DEBUG << "invalid sample rate (possible values are: 48, 96, 192, 384, 768, or 1536 kHz)!\n";
			break;
	}

	if (qtwdsp) {
        m_mutex.lock();

		// Queue flush and drop-counter are now handled unconditionally below.
		const bool highRateTransition = (previousRate >= 768000 || m_samplerate >= 768000);
		(void)highRateTransition;

		setAudioBufferSize();

		// Flush the queue and drop a few buffers after any rate transition so
		// fexchange0 is not called on the channel while it is being rebuilt.
		while (!m_iqQueue.isEmpty())
			m_iqQueue.dequeue();
		m_rateTransitionDropBuffers = HIGH_RATE_TRANSITION_DROP_BUFFERS;

        qtwdsp->setSampleRate(this, m_samplerate);
        m_mutex.unlock();

    }
	else
		RECEIVER_DEBUG << "qtdsp down: cannot set sample rate!\n";
}

void Receiver::setServerMode(QSDR::_ServerMode mode) {

	m_serverMode = mode;
}

QSDR::_ServerMode Receiver::getServerMode()	const {

	return m_serverMode;
}

QSDR::_DSPCore Receiver::getDSPCoreMode() const {

	return m_receiverData.dspCore;
}

//void Receiver::setSocketState(SocketState state) {
//
//	m_socketState = state;
//}

//Receiver::SocketState Receiver::socketState() const {
//
//	return m_socketState;
//}

void Receiver::setSystemState(
	QObject *sender,
	QSDR::_Error err,
	QSDR::_HWInterfaceMode hwmode,
	QSDR::_ServerMode mode,
	QSDR::_DataEngineState state)
{
	Q_UNUSED (sender)
	Q_UNUSED (err)

	if (m_hwInterface != hwmode)
		m_hwInterface = hwmode;

	if (m_serverMode != mode)
		m_serverMode = mode;

	if (m_dataEngineState != state)
		m_dataEngineState = state;
}

void Receiver::setAudioMode(QObject* sender, int mode) {

	if (sender != this && m_audioMode == mode) return;

	m_audioMode = mode;
}

//void Receiver::setID(int value) {
//
//	m_receiverID = value;
//	RECEIVER_DEBUG << "This is receiver " << m_receiverID;
//}

void Receiver::setReceiver(int value) {

	m_receiver = value;
}


void Receiver::setHamBand(QObject *sender, int rx, bool byBtn, HamBand band) {

	Q_UNUSED(sender)
	Q_UNUSED(byBtn)

	if (m_receiver == rx) {

		if (m_receiverData.hamBand == band) return;
		m_receiverData.hamBand = band;
	}
}

void Receiver::setDspMode(QObject *sender, int rx, DSPMode mode) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_receiverData.dspMode == mode) return;

	m_receiverData.dspMode = mode;

	QString msg = "[receiver]: set mode for receiver %1 to %2";
	emit messageEvent(msg.arg(rx).arg(set->getDSPModeString(m_receiverData.dspMode)));
}

void Receiver::setADCMode(QObject *sender, int rx, ADCMode mode) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_receiverData.adcMode == mode) return;

	m_receiverData.adcMode = mode;

	//RECEIVER_DEBUG << "RRK setADCMode = " << m_receiverData.adcMode;
}

void Receiver::setAGCFixedGain_dB(QObject *sender, int rx, qreal value) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_receiverData.agcFixedGain_dB == value) return;

	m_receiverData.agcFixedGain_dB = value;

}

void Receiver::setAudioVolume(QObject *sender, int rx, float value) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;

	m_receiverData.audioVolume = value;
}

void Receiver::setFilterFrequencies(QObject *sender, int rx, double low, double high) {

	Q_UNUSED(sender)

	if (m_receiver == rx) {

		if (m_receiverData.filterLo == low && m_receiverData.filterHi == high) return;
		m_receiverData.filterLo = low;
		m_receiverData.filterHi = high;
	}
}

void Receiver::setCtrFrequency(long frequency) {

	if (m_receiverData.ctrFrequency == frequency) return;
	m_receiverData.ctrFrequency = frequency;

	HamBand band = getBandFromFrequency(set->getBandFrequencyList(), frequency);
	m_receiverData.lastCenterFrequencyList[(int) band] = m_receiverData.ctrFrequency;
}

void Receiver::setVfoFrequency(long frequency) {

	if (m_receiverData.vfoFrequency == frequency) return;
	m_receiverData.vfoFrequency = frequency;

	HamBand band = getBandFromFrequency(set->getBandFrequencyList(), frequency);
	m_receiverData.lastVfoFrequencyList[(int) band] = m_receiverData.vfoFrequency;
}

void Receiver::setLastCtrFrequencyList(const QList<long> &fList) {

	m_receiverData.lastCenterFrequencyList = fList;
}

void Receiver::setLastVfoFrequencyList(const QList<long> &fList) {

	m_receiverData.lastVfoFrequencyList = fList;
}

void Receiver::setdBmPanScaleMin(qreal value) {

	if (m_dBmPanScaleMin == value) return;
	m_dBmPanScaleMin = value;
}

void Receiver::setdBmPanScaleMax(qreal value) {

	if (m_dBmPanScaleMax == value) return;
	m_dBmPanScaleMax = value;
}

void Receiver::setMercuryAttenuators(const QList<int> &attenuators) {

	m_receiverData.mercuryAttenuators = attenuators;
}

void Receiver::setFramesPerSecond(QObject *sender, int rx, int value) {

	Q_UNUSED(sender)

	if (m_receiver == rx)
		m_displayTime = (int)(1000000.0/value);
}

void Receiver::setPeerAddress(QHostAddress addr) {

	m_peerAddress = addr;
}

void Receiver::setSocketDescriptor(int value) {

	m_socketDescriptor = value;
}

void Receiver::setClient(int value) {

	m_client = value;
}

void Receiver::setIQPort(int value) {

	m_iqPort = value;
}

void Receiver::setBSPort(int value) {

	m_bsPort = value;
}

void Receiver::setConnectedStatus(bool value) {

	m_connected = value;
}
