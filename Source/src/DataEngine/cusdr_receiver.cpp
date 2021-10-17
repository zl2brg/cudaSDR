
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

	qtwdsp = nullptr;

	setupConnections();

	highResTimer = new HResTimer();
	m_displayTime = (int)(1000000.0/set->getFramesPerSecond(m_receiver));
	m_smeterTime.start();
}

Receiver::~Receiver() {


	if (qtwdsp) {

		delete qtwdsp;
		qtwdsp = nullptr;
	}
	inBuf.clear();
	outBuf.clear();

	m_stopped = false;
}

void Receiver::setAudioBufferSize() {
	int scale=m_samplerate/48000;
	m_audiobuffersize = 1024/scale;
	RECEIVER_DEBUG << "set Audio buffer size to: " << m_audiobuffersize;
	}

void Receiver::setupConnections() {

	CHECKED_CONNECT(
		set,
		SIGNAL(systemStateChanged(
					QObject *,
					QSDR::_Error,
					QSDR::_HWInterfaceMode,
					QSDR::_ServerMode,
					QSDR::_DataEngineState)),
		this,
		SLOT(setSystemState(
					QObject *,
					QSDR::_Error,
					QSDR::_HWInterfaceMode,
					QSDR::_ServerMode,
					QSDR::_DataEngineState)));

	CHECKED_CONNECT(
		set,
		SIGNAL(mainVolumeChanged(QObject *, int, float)),
		this,
		SLOT(setAudioVolume(QObject *, int, float)));

	CHECKED_CONNECT(
		set,
		SIGNAL(sampleRateChanged(QObject *, int)),
		this,
		SLOT(setSampleRate(QObject *, int)));

	CHECKED_CONNECT(
		set,
		SIGNAL(dspModeChanged(QObject *, int, DSPMode)),
		this,
		SLOT(setDspMode(QObject *, int, DSPMode)));

	CHECKED_CONNECT(
		set,
		SIGNAL(hamBandChanged(QObject *, int, bool, HamBand)),
		this,
		SLOT(setHamBand(QObject *, int, bool, HamBand)));

	CHECKED_CONNECT(
		set,
		SIGNAL(adcModeChanged(QObject *, int, ADCMode)),
		this,
		SLOT(setADCMode(QObject *, int, ADCMode)));

	CHECKED_CONNECT(
		set,
		SIGNAL(agcModeChanged(QObject *, int, AGCMode, bool)),
		this,
		SLOT(setAGCMode(QObject *, int, AGCMode, bool)));

	CHECKED_CONNECT(
		set,
		SIGNAL(agcGainChanged(QObject *, int, int)),
		this,
		SLOT(setAGCGain(QObject *, int, int)));

//	CHECKED_CONNECT(
//		set,
//		SIGNAL(agcMaximumGain_dBmChanged(QObject *, int, int)),
//		this,
//		SLOT(setAGCMaximumGain_dBm(QObject *, int, int)));

	CHECKED_CONNECT(
		set,
		SIGNAL(agcMaximumGainChanged(QObject *, int, qreal)),
		this,
		SLOT(setAGCMaximumGain_dB(QObject *, int, qreal)));

	CHECKED_CONNECT(
		set,
		SIGNAL(agcFixedGainChanged_dB(QObject *, int, qreal)),
		this,
		SLOT(setAGCFixedGain_dB(QObject *, int, qreal)));

	CHECKED_CONNECT(
		set,
		SIGNAL(agcThresholdChanged_dB(QObject *, int, qreal)),
		this,
		SLOT(setAGCThreshold_dB(QObject *, int, qreal)));

	CHECKED_CONNECT(
		set,
		SIGNAL(agcHangThresholdChanged(QObject *, int, int)),
		this,
		SLOT(setAGCHangThreshold(QObject *, int, int)));

	CHECKED_CONNECT(
		set,
		SIGNAL(agcHangLevelChanged_dB(QObject *, int, qreal)),
		this,
		SLOT(setAGCHangLevel_dB(QObject *, int, qreal)));

	CHECKED_CONNECT(
		set,
		SIGNAL(agcVariableGainChanged_dB(QObject *, int, qreal)),
		this,
		SLOT(setAGCSlope_dB(QObject * , int, qreal)));

	CHECKED_CONNECT(
		set,
		SIGNAL(agcAttackTimeChanged(QObject *, int, qreal)),
		this,
		SLOT(setAGCAttackTime(QObject *, int, qreal)));

	CHECKED_CONNECT(
		set,
		SIGNAL(agcDecayTimeChanged(QObject *, int, qreal)),
		this,
		SLOT(setAGCDecayTime(QObject *, int, qreal)));

	CHECKED_CONNECT(
		set,
		SIGNAL(agcHangTimeChanged(QObject *, int, qreal)),
		this,
		SLOT(setAGCHangTime(QObject *, int, qreal)));

	CHECKED_CONNECT(
		set,
		SIGNAL(filterFrequenciesChanged(QObject *, int, qreal, qreal)),
		this,
		SLOT(setFilterFrequencies(QObject *, int, qreal, qreal)));

	CHECKED_CONNECT(
		set,
		SIGNAL(framesPerSecondChanged(QObject*, int, int)),
		this,
		SLOT(setFramesPerSecond(QObject*, int, int)));

	/*CHECKED_CONNECT(
		set,
		SIGNAL(receiverDataReady()),
		this,
		SLOT(dspProcessing()));*/
}

void Receiver::setReceiverData(TReceiver data) {

	m_receiverData = data;

	//m_serverMode = m_receiverData.serverMode;
	m_dspCore = m_receiverData.dspCore;
	m_sampleRate = m_receiverData.sampleRate;
	m_hamBand = m_receiverData.hamBand;
	m_dspMode = m_receiverData.dspMode;
	m_dspModeList = m_receiverData.dspModeList;
	m_adcMode = m_receiverData.adcMode;
	m_agcMode = m_receiverData.agcMode;
	m_agcMode = m_receiverData.agcMode;
	m_agcGain = m_receiverData.acgGain;
	m_agcFixedGain_dB = m_receiverData.agcFixedGain_dB;
	m_agcMaximumGain_dB = m_receiverData.agcMaximumGain_dB;
	m_agcHangThreshold = m_receiverData.agcHangThreshold;
	m_agcSlope = m_receiverData.agcSlope;

	m_audioVolume = m_receiverData.audioVolume;

	m_filterLo = m_receiverData.filterLo;
	m_filterHi = m_receiverData.filterHi;

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

    qtwdsp = new QWDSPEngine(this, m_receiver, BUFFER_SIZE);

    if (qtwdsp)
        qtwdsp->setQtDSPStatus(true);
    else {

        RECEIVER_DEBUG << "could not start QWtDSP for receiver: " << m_receiver;
        qtwdsp = nullptr;
        return false;
    }

    qtwdsp->setVolume(m_audioVolume);


    DSPMode mode = m_dspModeList.at(m_hamBand);
    RECEIVER_DEBUG << "set DSP mode to: " << set->getDSPModeString(mode);

    qtwdsp->setDSPMode(mode);
    qtwdsp->setFilter(getFilterFromDSPMode(set->getDefaultFilterList(), mode).filterLo,getFilterFromDSPMode(set->getDefaultFilterList(), mode).filterHi);

	RECEIVER_DEBUG << "QtWDSP for receiver: " << m_receiver << " started.";

	return true;
}


void Receiver::deleteDSPInterface() {

	if (m_dspCore == QSDR::QtDSP)
		deleteQtWDSP();
}

void Receiver::deleteQtWDSP() {

	if (qtwdsp) {

		delete qtwdsp;
		qtwdsp = nullptr;
	}
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
	int spectrumDataReady;
	//RECEIVER_DEBUG << "dspProcessing: " << this->thread();
    qtwdsp->processDSP(inBuf, audioOutputBuf);



	if (highResTimer->getElapsedTimeInMicroSec() >= getDisplayDelay()) {
        m_mutex.lock();
//       GetPixels(0,0,qtwdsp->spectrumBuffer.data(), &spectrumDataReady);

        if (m_state == RadioState::RX)
            GetPixels(0,0,qtwdsp->spectrumBuffer.data(), &spectrumDataReady);
         else {
            GetPixels(TX_ID, 0, qtwdsp->spectrumBuffer.data(), &spectrumDataReady);
        }


		if (spectrumDataReady) {
			memcpy(
					newSpectrum.data(),
					qtwdsp->spectrumBuffer.data(),
					4096 * sizeof(float)
			);
			emit spectrumBufferChanged(m_receiver, newSpectrum);
		}

		m_mutex.unlock();
		highResTimer->start();
	}



	if (m_receiver == set->getCurrentReceiver()) {
		// S-Meter
		if (m_smeterTime.elapsed() > 200) {

			m_sMeterValue = qtwdsp->getSMeterInstValue();
			emit sMeterValueChanged(m_receiver, m_sMeterValue);
			m_smeterTime.restart();
		}

		// process output data
		emit audioBufferSignal(m_receiver, audioOutputBuf,m_audiobuffersize);
	}
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

void Receiver::setReceiver(int value) {

	m_receiver = value;
}

void Receiver::setSampleRate(int value) {

	m_sampleRate = value;
}

void Receiver::setHamBand(QObject *sender, int rx, bool byBtn, HamBand band) {

	Q_UNUSED(sender)
	Q_UNUSED(byBtn)

	if (m_receiver == rx) {

		if (m_hamBand == band) return;
		m_hamBand = band;
	}
}

void Receiver::setDspMode(QObject *sender, int rx, DSPMode mode) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_dspMode == mode) return;

	m_dspMode = mode;

	if (qtwdsp) {

		qtwdsp->setDSPMode(mode);
        qtwdsp->setFilter(getFilterFromDSPMode(set->getDefaultFilterList(), mode).filterLo,getFilterFromDSPMode(set->getDefaultFilterList(), mode).filterHi);
	}

	QString msg = "[receiver]: set mode for receiver %1 to %2";
	emit messageEvent(msg.arg(rx).arg(set->getDSPModeString(m_dspMode)));
}

void Receiver::setADCMode(QObject *sender, int rx, ADCMode mode) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_adcMode == mode) return;

	m_adcMode = mode;

	//RECEIVER_DEBUG << "RRK setADCMode = " << m_adcMode;
}

void Receiver::setAGCMode(QObject *sender, int rx, AGCMode mode, bool hang) {

	Q_UNUSED(sender)
	Q_UNUSED(hang)

	if (m_receiver != rx) return;
	if (m_agcMode == mode) return;

	m_agcMode = mode;

	if (qtwdsp) {

		qtwdsp->setAGCMode(mode);
	}
}

void Receiver::setAGCGain(QObject *sender, int rx, int value) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_agcGain == value) return;

	m_agcGain = value;

	if (qtwdsp) {

		RECEIVER_DEBUG << "AGCThreshDB (plus offset) = " << m_agcGain - AGCOFFSET;
		qtwdsp->setAGCThreshold( m_agcGain - AGCOFFSET);
	}
}

void Receiver::setAGCFixedGain_dB(QObject *sender, int rx, qreal value) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_agcFixedGain_dB == value) return;

	m_agcFixedGain_dB = value;

}

void Receiver::setAGCMaximumGain_dB(QObject *sender, int rx, qreal value) {

	Q_UNUSED(sender)
	if (m_receiver != rx) return;
	if (m_agcMaximumGain_dB == value) return;

	m_agcMaximumGain_dB = value;

	if (qtwdsp) {
		qtwdsp->setAGCMaximumGain(m_agcMaximumGain_dB);
	}
}

void Receiver::setAGCThreshold_dB(QObject *sender, int rx, qreal value) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_agcThreshold_dBm == value) return;

	m_agcThreshold_dBm = value;

	if (qtwdsp) {
		qtwdsp->setAGCThreshold((double)value);
	}
}

void Receiver::setAGCHangThreshold(QObject *sender, int rx, int value) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_agcHangThreshold == value) return;

	m_agcHangThreshold = value;
	if (qtwdsp) {

		RECEIVER_DEBUG << "m_agcHangThreshold =" << m_agcHangThreshold/100.0;
		qtwdsp->setAGCHangThreshold(m_receiver, m_agcHangThreshold / 100.0);
	}
}

void Receiver::setAGCHangLevel_dB(QObject *sender, int rx, qreal value) {

        Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_agcHangLevel == value) return;

	m_agcHangLevel = value;

	if (qtwdsp) {

		RECEIVER_DEBUG << "m_agcHangLevel = " << m_agcHangLevel - AGCOFFSET;
		qtwdsp->setAGCHangLevel(m_agcHangLevel - AGCOFFSET);
	}
}

void Receiver::setAGCSlope_dB(QObject *sender, int rx, qreal value) {

        Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_agcSlope == value) return;

	m_agcSlope = value;

	if (qtwdsp) {

		RECEIVER_DEBUG << "m_agcSlope = " << m_agcSlope;
		qtwdsp->setAGCSlope(m_receiver, m_agcSlope);
	}
}

void Receiver::setAGCAttackTime(QObject *sender, int rx, qreal value) {

        Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_agcAttackTime == value) return;

	m_agcAttackTime = value;
	qtwdsp->setAGCAttackTime(m_receiver, value);
	RECEIVER_DEBUG << "m_agcAttackTime = " << m_agcAttackTime;
}


void Receiver::setAGCDecayTime(QObject *sender, int rx, qreal value) {

        Q_UNUSED(sender)

	if (m_agcDecayTime == value) return;
	m_agcDecayTime = value;
	qtwdsp->setAGCDecayTime(m_receiver, value);
	RECEIVER_DEBUG << "m_agcDecayTime = " << m_agcDecayTime;
}


void Receiver::setAGCHangTime(QObject *sender, int rx, qreal value) {

        Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_agcHangTime == value) return;

	m_agcHangTime = value;

	if (qtwdsp) {

		RECEIVER_DEBUG << "m_agcHangTime = " << m_agcHangTime;
		qtwdsp->setAGCHangTime(m_agcHangTime);
	}
}

void Receiver::setAudioVolume(QObject *sender, int rx, float value) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;
	//if (m_audioVolume == value) return;

	m_audioVolume = value;

	if (qtwdsp) {

		RECEIVER_DEBUG << "setAudioVolume =" << m_audioVolume;
		qtwdsp->setVolume(value);
	}
}

void Receiver::setFilterFrequencies(QObject *sender, int rx, double low, double high) {

	Q_UNUSED(sender)

	if (m_receiver == rx) {

		if (m_filterLo == low && m_filterHi == high) return;
		m_filterLo = low;
		m_filterHi = high;

		if (qtwdsp) {

			qtwdsp->setFilter(low, high);
		}
	}
}

void Receiver::setCtrFrequency(long frequency) {

	if (m_ctrFrequency == frequency) return;
	m_ctrFrequency = frequency;

	HamBand band = getBandFromFrequency(set->getBandFrequencyList(), frequency);
	m_lastCtrFrequencyList[(int) band] = m_ctrFrequency;
}

void Receiver::setVfoFrequency(long frequency) {

	if (m_vfoFrequency == frequency) return;
	m_vfoFrequency = frequency;

	HamBand band = getBandFromFrequency(set->getBandFrequencyList(), frequency);
	m_lastVfoFrequencyList[(int) band] = m_vfoFrequency;
}

void Receiver::setLastCtrFrequencyList(const QList<long> &fList) {

	m_lastCtrFrequencyList = fList;
}

void Receiver::setLastVfoFrequencyList(const QList<long> &fList) {

	m_lastVfoFrequencyList = fList;
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

	m_mercuryAttenuators = attenuators;
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
