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

Receiver::Receiver(int rx)
	: QObject()
	, set(Settings::instance())
	, m_filterMode(set->getCurrentFilterMode())
	, m_stopped(false)
	, m_receiver(rx)
	, m_samplerate(set->getSampleRate())
	, m_audioMode(1)
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
    #ifdef USE_INTERNAL_AUDIO
        m_audioOutput->stop();
    #endif
    delete m_audioOutput;
    m_stopped = false;
    delete qtwdsp;
}

void Receiver::setAudioBufferSize() {
    int scale=m_samplerate/48000;
    m_audiobuffersize = 1024/scale;
    RECEIVER_DEBUG << "set Audio buffer size to: " << m_audiobuffersize;
    }

void Receiver::setupConnections() {
    // Basic state changes
    connect(set, &Settings::systemStateChanged, this, &Receiver::setSystemState);
    connect(set, &Settings::sampleRateChanged, this, &Receiver::setSampleRate);
    connect(set, &Settings::framesPerSecondChanged, this, &Receiver::setFramesPerSecond);

    // Group all config-related signals to one dirty flag
    auto markDirty = [this](QObject*, int rx, ...) {
        if (rx == m_receiver) onConfigChanged();
    };

    connect(set, &Settings::hamBandChanged, this, markDirty);
    connect(set, &Settings::dspModeChanged, this, markDirty);
    connect(set, &Settings::adcModeChanged, this, markDirty);
    connect(set, &Settings::agcModeChanged, this, markDirty);
    connect(set, &Settings::agcGainChanged, this, markDirty);
    connect(set, &Settings::agcMaximumGainChanged, this, markDirty);
    connect(set, &Settings::agcFixedGainChanged_dB, this, markDirty);
    connect(set, &Settings::agcThresholdChanged_dB, this, markDirty);
    connect(set, &Settings::agcHangThresholdChanged, this, markDirty);
    connect(set, &Settings::agcHangLevelChanged_dB, this, markDirty);
    connect(set, &Settings::agcVariableGainChanged_dB, this, markDirty);
    connect(set, &Settings::agcAttackTimeChanged, this, markDirty);
    connect(set, &Settings::agcDecayTimeChanged, this, markDirty);
    connect(set, &Settings::agcHangTimeChanged, this, markDirty);
    connect(set, &Settings::filterFrequenciesChanged, this, markDirty);
    
    // Volume is special but we can include it in the batch
    connect(set, &Settings::mainVolumeChanged, this, [this](QObject*, int rx, float) {
        if (rx == m_receiver) onConfigChanged();
    });

    // Also listen for the new unified signal if available
    connect(set, &Settings::receiverConfigChanged, this, [this](int rx) {
        if (rx == m_receiver) onConfigChanged();
    });
}

void Receiver::onConfigChanged() {
    TReceiver data = set->getReceiverData(m_receiver);
    QMutexLocker locker(&m_mutex);
    m_pendingConfig = data;
    m_configDirty.store(true);
}

void Receiver::applyNewConfig() {
    QMutexLocker locker(&m_mutex);
    
    // 1. DSP Mode & Filter
    if (m_config.dspMode != m_pendingConfig.dspMode) {
        qtwdsp->setDSPMode(m_pendingConfig.dspMode);
    }
    
    if (m_config.filterLo != m_pendingConfig.filterLo || m_config.filterHi != m_pendingConfig.filterHi) {
        qtwdsp->setFilter(m_pendingConfig.filterLo, m_pendingConfig.filterHi);
    }
    
    // 2. AGC Settings
    if (m_config.agcMode != m_pendingConfig.agcMode) {
        qtwdsp->setAGCMode(m_pendingConfig.agcMode);
    }

    // Always check/update AGC parameters if any changed
    if (m_config.acgGain != m_pendingConfig.acgGain)
        qtwdsp->setAGCThreshold(m_pendingConfig.acgGain - AGCOFFSET);
        
    if (m_config.agcMaximumGain_dB != m_pendingConfig.agcMaximumGain_dB)
        qtwdsp->setAGCMaximumGain(m_pendingConfig.agcMaximumGain_dB);
        
    if (m_config.agcFixedGain_dB != m_pendingConfig.agcFixedGain_dB)
        qtwdsp->setAGCFixedGain(m_pendingConfig.agcFixedGain_dB);

    if (m_config.agcHangThreshold != m_pendingConfig.agcHangThreshold)
        qtwdsp->setAGCHangThreshold(m_receiver, m_pendingConfig.agcHangThreshold / 100.0);

    if (m_config.agcHangLevel != m_pendingConfig.agcHangLevel)
        qtwdsp->setAGCHangLevel(m_pendingConfig.agcHangLevel - AGCOFFSET);

    if (m_config.agcSlope != m_pendingConfig.agcSlope)
        qtwdsp->setAGCSlope(m_receiver, m_pendingConfig.agcSlope);

    if (m_config.agcAttackTime != m_pendingConfig.agcAttackTime)
        qtwdsp->setAGCAttackTime(m_receiver, m_pendingConfig.agcAttackTime);

    if (m_config.agcDecayTime != m_pendingConfig.agcDecayTime)
        qtwdsp->setAGCDecayTime(m_receiver, m_pendingConfig.agcDecayTime);

    if (m_config.agcHangTime != m_pendingConfig.agcHangTime)
        qtwdsp->setAGCHangTime(m_pendingConfig.agcHangTime);

    // 3. Audio & UI State
    if (m_config.audioVolume != m_pendingConfig.audioVolume)
        qtwdsp->setVolume(m_pendingConfig.audioVolume);
    
    // Update local cached values used for getters
    m_hamBand = m_pendingConfig.hamBand;
    m_audioVolume = m_pendingConfig.audioVolume;
    m_ctrFrequency = m_pendingConfig.ctrFrequency;
    m_vfoFrequency = m_pendingConfig.vfoFrequency;

    m_config = m_pendingConfig;
}

void Receiver::setReceiverData(TReceiver data) {
    QMutexLocker locker(&m_mutex);
	m_receiverData = data;
    m_config = data;
    m_pendingConfig = data;

	m_dspCore = m_receiverData.dspCore;
	m_sampleRate = m_receiverData.sampleRate;
	m_hamBand = m_receiverData.hamBand;
	m_dspModeList = m_receiverData.dspModeList;
	m_agcGain = m_receiverData.acgGain;

	m_audioVolume = m_receiverData.audioVolume;

	m_lastCtrFrequencyList = m_receiverData.lastCenterFrequencyList;
	m_lastVfoFrequencyList = m_receiverData.lastVfoFrequencyList;
	m_mercuryAttenuators = m_receiverData.mercuryAttenuators;
	m_refreshrate = m_receiverData.framesPerSecond;
}

bool Receiver::initDSPInterface() {

	if (m_dspCore == QSDR::QtDSP) {

        if (!initQtWDSPInterface()) return false;

	}
	return true;
}



bool Receiver::initQtWDSPInterface() {

//    qtwdsp = std::make_unique<QWDSPEngine>(this, m_receiver, BUFFER_SIZE);
    qtwdsp = new QWDSPEngine(this, m_receiver, BUFFER_SIZE);

    if (!qtwdsp || !qtwdsp->isValid()) {  // Add validity check
        RECEIVER_DEBUG << "could not start QWtDSP for receiver: " << m_receiver;
        return false;
    }

    qtwdsp->setQtDSPStatus(true);
    qtwdsp->setVolume(m_audioVolume);

    DSPMode mode = m_dspModeList.at(m_hamBand);
    RECEIVER_DEBUG << "set DSP mode to: " << set->getDSPModeString(mode);

    qtwdsp->setDSPMode(mode);
    
    auto filter = getFilterFromDSPMode(set->getDefaultFilterList(), mode);
    qtwdsp->setFilter(filter.filterLo, filter.filterHi);

    RECEIVER_DEBUG << "QtWDSP for receiver: " << m_receiver << " started.";
    return true;
}

void Receiver::enqueueData() {

	inQueue.enqueue(inBuf);

	if (inQueue.isFull()) {
		RECEIVER_DEBUG << "inQueue full!";
	}

}

void Receiver::stop() {

	m_mutex.lock();
	m_stopped = true;
	m_mutex.unlock();
}

void Receiver::dspProcessing() {
    // 1. Check for batch config updates at a single sync point
    if (m_configDirty.load()) {
        applyNewConfig();
        m_configDirty.store(false);
    }

    int spectrumDataReady;
    m_mutex.lock(); // Use consolidated m_mutex
    qtwdsp->processDSP(inBuf, audioOutputBuf);
    m_mutex.unlock();

      if (highResTimer->getElapsedTimeInMicroSec() >= getDisplayDelay()) {

        
        if (m_state == RadioState::RX)
            GetPixels(0,0,qtwdsp->spectrumBuffer.data(), &spectrumDataReady);
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
        m_audioOutput->writeAudio(interleaveFromCPX(audioOutputBuf));
#endif
        emit audioBufferSignal(m_receiver, audioOutputBuf, m_audiobuffersize);
    }
}

QVector<float> Receiver::interleaveFromCPX(const CPX& in) {
    QVector<float> out;

    out.reserve(in.size() * 2);
    for (const cpx& val : in) {
        out.append(val.re);
        out.append(val.im);
    }
    return out;
}

void Receiver::setSampleRate(QObject *sender, int value) {
	Q_UNUSED(sender)

	if (m_samplerate == value) return;

	switch (value) {

		case 48000:
			m_samplerate = value;
			break;

		case 96000:
			m_samplerate = value;
			break;

		case 192000:
			m_samplerate = value;
			break;

		case 384000:
			m_samplerate = value;
			break;

		default:
			RECEIVER_DEBUG << "invalid sample rate (possible values are: 48, 96, 192, or 384 kHz)!\n";
			break;
	}

	if (qtwdsp) {
        m_mutex.lock();
		setAudioBufferSize();
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

	return m_dspCore;
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

void Receiver::setConnectedStatus(bool value) {

	m_connected = value;
}
