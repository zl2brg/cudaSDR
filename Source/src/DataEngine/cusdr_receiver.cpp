
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
* 2023 updated by Simon Eatough ZL2BRG
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
    , m_refreshrate(set->getFramesPerSecond(rx))

    , m_fftSize ( set->getfftSize(rx))

    , m_averageCount(set->getSpectrumAveragingCnt(rx))
    , m_PanAvMode(set->getPanAveragingMode(rx))
    , m_PanDetMode ( set->getPanDetectorMode(rx))
    , m_agcSlope ( set->getAGCSlope(rx))

    , m_agcHangThreshold ( set->getAGCHangThreshold(rx))
    , m_agcMaximumGain ( set->getAGCMaximumGain_dB(rx))
    ,m_nrMode ( set->getnrMode(rx))
    , m_nr_agc ( set->getNrAGC(rx))
    , m_nr2_ae ( set->getNr2ae(rx))
    , m_nr2_gain_method ( set->getNr2GainMethod(rx))
    , m_nr2_npe_method ( set->getNr2NpeMethod(rx))
    , m_nbMode ( set->getnbMode(rx))
    , m_anf ( set->getAnf(rx))
    , m_snb( set->getSnb(rx))
	//, m_calOffset(63.0)
	//, m_calOffset(33.0)
{
    m_receiverData = set->getReceiverDataList().at(rx);

    m_sampleRate = m_receiverData.sampleRate;
    m_hamBand = m_receiverData.hamBand;
    m_dspMode = m_receiverData.dspMode;
    m_dspModeList = m_receiverData.dspModeList;
    m_adcMode = m_receiverData.adcMode;
    m_agcMode = m_receiverData.agcMode;
    m_agcMode = m_receiverData.agcMode;
    m_agcGain = m_receiverData.agcgGain;
    m_agcFixedGain_dB = m_receiverData.agcFixedGain_dB;
    m_agcHangThreshold = m_receiverData.agcHangThreshold;
    m_agcSlope = m_receiverData.agcSlope;

    m_audioVolume = m_receiverData.audioVolume;
    m_filterLo = m_receiverData.filterLo;
    m_filterHi = m_receiverData.filterHi;

    m_lastCtrFrequencyList = m_receiverData.lastCenterFrequencyList;
    m_lastVfoFrequencyList = m_receiverData.lastVfoFrequencyList;
    spectrumBuffer.resize(BUFFER_SIZE*4);
    m_mercuryAttenuators = m_receiverData.mercuryAttenuators;

	InitCPX(inBuf, BUFFER_SIZE, 0.0f);
	InitCPX(outBuf, BUFFER_SIZE, 0.0f);
    InitCPX(audioOutputBuf, BUFFER_SIZE, 0.0f);
	setAudioBufferSize();
	newSpectrum.resize(BUFFER_SIZE*4);
	highResTimer = new HResTimer();
	m_displayTime = (int)(1000000.0/set->getFramesPerSecond(m_receiver));
	m_smeterTime.start();
    setupConnections();
}

Receiver::~Receiver() {
	inBuf.clear();
	outBuf.clear();
}



void Receiver::setAudioBufferSize() {
	int scale=m_samplerate/DSP_RATE;
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
		SLOT(setDisplayFramesPerSecond(QObject * , int, int)));


    CHECKED_CONNECT(
            set,
            SIGNAL(ncoFrequencyChanged(int, long)),
            this,
            SLOT(setNCOFrequency(int, long)));

    CHECKED_CONNECT(
            set,
            SIGNAL(sampleSizeChanged(int, int)),
            this,
            SLOT(setSampleSize(int, int)));


    CHECKED_CONNECT(
            set,
            SIGNAL(panAveragingModeChanged( int, int)),
            this,
            SLOT(setDisplayrAveragegMode(int, int)));


    CHECKED_CONNECT(
            set,
            SIGNAL(panDetectorModeChanged( int, int)),
            this,
            SLOT(setDisplayDetectorMode(int, int)));

    CHECKED_CONNECT(
            set,
            SIGNAL(spectrumAveragingCntChanged(QObject*, int , int)),
            this,
            SLOT(setDisplyAveragingCount(QObject * , int, int)));

    CHECKED_CONNECT(
            set,
            SIGNAL(fftSizeChanged( int , int)),
            this,
            SLOT(setfftSize(int, int)));

    CHECKED_CONNECT(
            set,
            SIGNAL(fmsqLevelChanged( int , int)),
            this,
            SLOT(setfmsqLevel(int, int)));

    CHECKED_CONNECT(
            set,
            SIGNAL(noiseBlankerChanged( int , int)),
            this,
            SLOT(setNoiseBlankerMode(int, int )));

    CHECKED_CONNECT(
            set,
            SIGNAL(noiseFilterChanged( int , int)),
            this,
            SLOT(setNoiseFilterMode(int, int )));

    CHECKED_CONNECT(
            set,
            SIGNAL(nr2AeChanged(int, bool)),
            this,
            SLOT(setNr2Ae(int, bool )));

    CHECKED_CONNECT(
            set,
            SIGNAL(nr2NpeMethodChanged(int, int)),
            this,
            SLOT(setNr2NpeMethod(int, int )));

    CHECKED_CONNECT(
            set,
            SIGNAL(nr2GainMethodChanged(int, int)),
            this,
            SLOT(setNr2GainMethod(int, int )));

    CHECKED_CONNECT(
            set,
            SIGNAL(nrAgcChanged(int, int)),
            this,
            SLOT(setNrAGC(int, int )));

    CHECKED_CONNECT(
            set,
            SIGNAL(anfChanged(int, bool)),
            this,
            SLOT(setanf(int, bool )));

    CHECKED_CONNECT(
            set,
            SIGNAL(snbChanged(int, bool)),
            this,
            SLOT(setsnb(int, bool )));
	/*CHECKED_CONNECT(
		set,
		SIGNAL(receiverDataReady()),
		this,
		SLOT(dspProcessing()));*/
}

void Receiver::start() {
    SetChannelState(m_receiver,1,0);
    m_stopped = false;
    RECEIVER_DEBUG << "Receiver: " << m_receiver << "Started";

}

bool Receiver::initWDSPInterface() {
    int result;

    RECEIVER_DEBUG << "Open RX Channel" << m_receiver;
    OpenChannel(m_receiver,
                BUFFER_SIZE,
                DSPSIZE, // ,
                m_samplerate,
                DSP_RATE, // dsp rate
                DSP_RATE, // output rate
                0, // receive
                0, // run
                0.010, 0.025, 0.0, 0.010, 0);

    create_anbEXT(m_receiver, 1, DSP_SAMPLE_SIZE, m_samplerate, 0.0001, 0.0001, 0.0001, 0.05, 20);
    create_nobEXT(m_receiver, 1, 0, DSP_SAMPLE_SIZE, m_samplerate, 0.0001, 0.0001, 0.0001, 0.05, 20);
    fprintf(stderr, "RXASetNC %d\n", m_fftSize);
    RXASetNC(m_receiver, m_fftSize);
    setNoiseBlankerMode(m_receiver);
    SetRXAFMDeviation(m_receiver, (double)8000.0);

    RXASetNC(m_receiver, 4096);
    SetRXAPanelRun(m_receiver, 1);
    SetRXAPanelSelect(m_receiver, 3);
    XCreateAnalyzer(m_receiver, &result, 262144, 1, 1, (char *) "");
    if(result != 0) {
        RECEIVER_DEBUG <<  "XCreateAnalyzer id=%d failed: %d\n" <<  result;
    }
    setfftSize(m_receiver, 2048);
    SetDisplayDetectorMode(m_receiver, 0, m_PanDetMode);
    SetDisplayAverageMode(m_receiver, 0, m_PanAvMode);
    SetRXAFMSQRun(m_receiver,1);
//   SetChannelState(m_receiver,1,0);
    DSPMode mode = set->getDSPMode(m_receiver);
    setAudioVolume(this, m_receiver, 0.2);
    RECEIVER_DEBUG << "set DSP mode to: " << set->getDSPModeString(mode);
    SetRXAMode(m_receiver, mode);
    setFilterFrequencies(this, m_receiver, getFilterFromDSPMode(set->getDefaultFilterList(), mode).filterLo, getFilterFromDSPMode(
            set->getDefaultFilterList(), mode).filterHi);
    SetRXAAGCThresh(m_receiver, set->getAGCGain(m_receiver),this->m_fftSize,this->m_samplerate);
    set->setAGCMaximumGain_dB(this, m_receiver, set->getAGCGain(m_receiver));
    setAGC_Line(m_receiver);
    setAGCGain(this,m_receiver,set->getAGCGain(m_receiver));
    return true;

}


void Receiver::deleteDSPInterface() {


}


void Receiver::enqueueData() {

	inQueue.enqueue(inBuf);

	if (inQueue.isFull()) {
		RECEIVER_DEBUG << "inQueue full!";
	}

}

void Receiver::stop() {
    closeReceiver();
    QThread::msleep(100);
	m_mutex.lock();
	m_stopped = true;
	m_mutex.unlock();
    RECEIVER_DEBUG << "QtWDSP for receiver: " << m_receiver << "Stopped";
}

void Receiver::closeReceiver() {
    SetChannelState(m_receiver, 0, 1);
    CloseChannel(m_receiver);
    DestroyAnalyzer(m_receiver);
 //   SetRXAFMSQRun(m_receiver,0);
    destroy_nobEXT(m_receiver);
    destroy_anbEXT(m_receiver);
    inBuf.clear();
    outBuf.clear();
    if (m_receiver == 0){ // tx channel tied to RX 0
    SetChannelState(TX_ID,0,1);
    CloseChannel(TX_ID);
    }
}

void Receiver::dspProcessing() {
	int spectrumDataReady;
   int error;
      m_mutex.lock();

        fexchange0(m_receiver, (double *) inBuf.data(),  (double *) audioOutputBuf.data(), &error);
        if(error!=0) {
            RECEIVER_DEBUG << "WDSP channel read error " << error;
        }
        else  Spectrum0(1, m_receiver, 0, 0, (double *) inBuf.data());
        m_mutex.unlock();
        if (highResTimer->getElapsedTimeInMicroSec() >= getDisplayDelay()) {
            m_mutex.lock();
            if (m_state == RadioState::RX)
            {
                GetPixels(m_receiver,0,spectrumBuffer.data(), &spectrumDataReady);
            }
            else {
                GetPixels(TX_ID, 0, spectrumBuffer.data(), &spectrumDataReady);
            }

        if (spectrumDataReady) {
			memcpy(

					newSpectrum.data(),
                    spectrumBuffer.data(),
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

			m_sMeterValue = GetRXAMeter(m_receiver,RXA_S_AV);
            emit sMeterValueChanged(m_receiver, m_sMeterValue);
			m_smeterTime.restart();
		}
		// process output data
		emit audioBufferSignal(m_receiver, audioOutputBuf,m_audiobuffersize);
	}
}


void Receiver::setServerMode(QSDR::_ServerMode mode) {

	m_serverMode = mode;
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
    SetRXAMode(m_receiver, mode);
    setFilterFrequencies(this, m_receiver, getFilterFromDSPMode(set->getDefaultFilterList(), mode).filterLo,
                         getFilterFromDSPMode(set->getDefaultFilterList(), mode).filterHi);
}

void Receiver::setADCMode(QObject *sender, int rx, ADCMode mode) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_adcMode == mode) return;

	m_adcMode = mode;

	//RECEIVER_DEBUG << "RRK setADCMode = " << m_adcMode;
}



void Receiver::setAGCGain(QObject *sender, int rx, int value) {

	Q_UNUSED(sender)
    Q_UNUSED(rx)

//	if (m_receiver != rx) return;
//	if (m_agcGain == value) return;

	    m_agcGain = value;

		RECEIVER_DEBUG << "AGCThreshDB (plus offset) = " << m_agcGain - AGCOFFSET;
        SetRXAAGCTop(m_receiver, (double)value);
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
    SetRXAAGCTop(m_receiver, (double)value);
	m_agcMaximumGain_dB = value;
    setAGC_Line(m_receiver);
}

void Receiver::setAGCThreshold_dB(QObject *sender, int rx, qreal value) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_agcThreshold_dBm == value) return;

	m_agcThreshold_dBm = value;
    SetRXAAGCThresh(m_receiver,m_agcThreshold_dBm,this->m_fftSize,this->m_samplerate);
    setAGC_Line(m_receiver);
    RECEIVER_DEBUG << "Set AGC threshold " << value;

}

void Receiver::setAGCHangThreshold(QObject *sender, int rx, int value) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_agcHangThreshold == value) return;

	m_agcHangThreshold = value;
	RECEIVER_DEBUG << "m_agcHangThreshold =" << m_agcHangThreshold/100.0;

}

void Receiver::setAGCHangLevel_dB(QObject *sender, int rx, qreal value) {

     Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_agcHangLevel == value) return;

	m_agcHangLevel = value;
    RECEIVER_DEBUG << "m_agcHangLevel = " << m_agcHangLevel - AGCOFFSET;
    SetRXAAGCHangLevel(m_receiver,value);
}

void Receiver::setAGCSlope_dB(QObject *sender, int rx, qreal value) {

        Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_agcSlope == value) return;

	m_agcSlope = value;

}

void Receiver::setAGCAttackTime(QObject *sender, int rx, qreal value) {

        Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_agcAttackTime == value) return;

	m_agcAttackTime = value;
	RECEIVER_DEBUG << "m_agcAttackTime = " << m_agcAttackTime;
}


void Receiver::setAGCDecayTime(QObject *sender, int rx, qreal value) {
    if (m_receiver != rx) return;
    Q_UNUSED(sender)
	if (m_agcDecayTime == value) return;
	m_agcDecayTime = value;
	RECEIVER_DEBUG << "m_agcDecayTime = " << m_agcDecayTime;
}


void Receiver::setAGCHangTime(QObject *sender, int rx, qreal value) {

        Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_agcHangTime == value) return;
	m_agcHangTime = value;
}

void Receiver::setAudioVolume(QObject *sender, int rx, float value) {

	Q_UNUSED(sender)
	if (m_receiver != rx) return;
    RECEIVER_DEBUG << "Set RX Volume" << rx << value;
    SetRXAPanelGain1(m_receiver, value);
}

void Receiver::setFilterFrequencies(QObject *sender, int rx, double low, double high) {

	Q_UNUSED(sender)

	if (m_receiver == rx) {

		if (m_filterLo == low && m_filterHi == high) return;
		m_filterLo = low;
		m_filterHi = high;
        if(m_dspmode == FMN) {
            SetRXAFMDeviation(m_receiver, (double)8000.0);
        }
        RXASetPassband(m_receiver,low,high);
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

void Receiver::setAGC_Line(int rx) {
    if (m_receiver != rx) return;
    double hang;
    double thresh;

    GetRXAAGCHangLevel(m_receiver, &hang);
    GetRXAAGCThresh(m_receiver, &thresh, this->m_fftSize, (double)m_samplerate);

    set->setAGCLineLevels(this,m_receiver,-100,hang);

    if ((hang != m_agcHangLevel) || (thresh != m_agcHangThreshold))
	{
        printf("level %f\n",thresh);
        m_agcHangLevel = hang;
		m_agcThreshold = thresh;


        RECEIVER_DEBUG << "Set AGC line value" << thresh << hang;

	}

//    qreal noiseOffset = 10.0 * log10(qAbs(filter->filterHi() - filter->filterLo()) * 2 * BUFFER_SIZE / m_samplerate);
//    qreal threshold = 20.0 * log10(thresh) - noiseOffset + AGCOFFSET;


}


void Receiver::setSampleRate(QObject *sender, int value) {

	Q_UNUSED(sender)

	if (m_samplerate == value) return;

	switch (value) {

		case DSP_RATE:
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

    SetChannelState(m_receiver,0,1);
    setAudioBufferSize();
    SetInputSamplerate(m_receiver,m_samplerate);
    init_analyzer(m_refreshrate);
    SetEXTANBSamplerate (m_receiver, m_samplerate);
    SetEXTNOBSamplerate (m_receiver, m_samplerate);
    SetChannelState(m_receiver,1,1);
}

void Receiver::setNCOFrequency(int rx, long ncoFreq) {

	if (m_receiver != rx) return;
    RECEIVER_DEBUG << "set NCO frequency" << ncoFreq;
	if(ncoFreq==0) {
		SetRXAShiftFreq(m_receiver, (double)ncoFreq);
		RXANBPSetShiftFrequency(m_receiver, (double)ncoFreq);
		SetRXAShiftRun(m_receiver, 0);
	} else {
		SetRXAShiftFreq(m_receiver, (double)ncoFreq);
		RXANBPSetShiftFrequency(m_receiver, (double)ncoFreq);
		SetRXAShiftRun(m_receiver, 1);
	}
}

void Receiver::setSampleSize(int rx, int size) {

	if (m_receiver == rx) {

		m_mutex.lock();
		m_spectrumSize = size;
        RECEIVER_DEBUG <<  "Set sample size" <<  size;
		m_mutex.unlock();
	}
}

void Receiver::ProcessFrequencyShift(CPX &in, CPX &out) {
    Q_UNUSED(in)
    Q_UNUSED(out)


}

void Receiver::init_analyzer(int refreshrate) {
	int flp[] = {0};
	double keep_time = 0.1;
    int n_pixout = 1;
	int spur_elimination_ffts = 1;
	int data_type = 1;
    int fft_size = m_fftSize;
    int window_type = 6;
	double kaiser_pi = 14.0;
    int overlap;
	int clip = 0;
	int span_clip_l = 0;
	int span_clip_h = 0;
	int pixels = 4096;
	int stitches = 1;
	int calibration_data_set = 0;
	double span_min_freq = 0.0;
	double span_max_freq = 0.0;

    int max_w = fft_size + (int) min(keep_time * refreshrate, keep_time * (double) fft_size * (double) refreshrate);

    overlap = (int) max(0.0, ceil(fft_size - (double) m_samplerate / (double) refreshrate));

    printf("SetAnalyzer id=%d buffer_size=%d overlap=%d\n fft%d\n", m_receiver, BUFFER_SIZE, overlap,m_fftSize);


	SetAnalyzer(m_receiver,
				n_pixout,
				spur_elimination_ffts, //number of LO frequencies = number of ffts used in elimination
				data_type, //0 for real input data (I only); 1 for complex input data (I & Q)
				flp, //vector with one elt for each LO frequency, 1 if high-side LO, 0 otherwise
                fft_size, //size of the fft, i.e., number of input samples
                1024, //number of samples transferred for each OpenBuffer()/CloseBuffer()
				window_type, //integer specifying which window function to use
				kaiser_pi, //PiAlpha parameter for Kaiser window
				overlap, //number of samples each fft (other than the first) is to re-use from the previous
				clip, //number of fft output bins to be clipped from EACH side of each sub-span
				span_clip_l, //number of bins to clip from low end of entire span
				span_clip_h, //number of bins to clip from high end of entire span
				pixels, //number of pixel values to return.  may be either <= or > number of bins
				stitches, //number of sub-spans to concatenate to form a complete span
				calibration_data_set, //identifier of which set of calibration data to use
				span_min_freq, //frequency at first pixel value
				span_max_freq, //frequency at last pixel value
				max_w //max samples to hold in input ring buffers
	);
}

void Receiver::calcDisplayAveraging() {

    double t=0.001*m_averageCount;
    m_display_avb = exp(-1.0 / ((double)m_refreshrate * t));
    m_display_average = max(2, (int)min(60, (double)m_refreshrate * t));
}

void Receiver::setDisplayFramesPerSecond(QObject* sender, int rx, int value){

	Q_UNUSED(sender)
	if (rx != m_receiver) return;
	m_mutex.lock();
    SetChannelState(m_receiver,0,1);
	m_refreshrate = value;
    init_analyzer(value);
    calcDisplayAveraging();
    SetDisplayAvBackmult(rx, 0, m_display_avb);
    SetDisplayNumAverage(rx, 0, m_display_average);
    m_displayTime = (int)(1000000.0/value);
    SetChannelState(m_receiver,1,1);
    m_mutex.unlock();
}

void Receiver::setDisplayrAveragegMode(int rx, int mode) {
    if (rx != m_receiver) return;
    RECEIVER_DEBUG <<  "Setpan av mode" <<  mode;
    m_display_avb = mode;
    m_mutex.lock();
    SetChannelState(m_receiver,0,1);
    SetDisplayAverageMode(m_receiver,0,mode);
    calcDisplayAveraging();
    SetDisplayAvBackmult(rx, 0, m_display_avb);
    SetDisplayNumAverage(rx, 0, m_display_average);
    SetChannelState(m_receiver,1,1);
    m_mutex.unlock();
}

void Receiver::setDisplayDetectorMode(int rx, int mode) {
    if (rx != m_receiver) return;
    RECEIVER_DEBUG <<  "Setpan av det  mode" <<  mode;
    SetDisplayDetectorMode(rx,0,mode);

}

void Receiver::setDisplyAveragingCount(QObject* sender, int rx, int count){
    Q_UNUSED(sender);
    if (rx != m_receiver) return;
    m_averageCount = count;
    calcDisplayAveraging();
    SetDisplayAvBackmult(rx, 0, m_display_avb);
    SetDisplayNumAverage(rx, 0, m_display_average);
    RECEIVER_DEBUG <<  "Setpan av count mode" <<  m_display_avb << " " << m_display_average;
}

void Receiver::setfftSize(int rx, int value) {
    m_fftSize =  value;
    m_mutex.lock();
    SetChannelState(m_receiver,0,1);
    qDebug() << "set fft size " << m_fftSize;
	init_analyzer(m_refreshrate);
	calcDisplayAveraging();
	SetDisplayAvBackmult(rx, 0, m_display_avb);
	SetDisplayNumAverage(rx, 0, m_display_average);
    SetChannelState(m_receiver,1,1);
    m_mutex.unlock();

}

void Receiver::setfmsqLevel(int rx, int value) {
	if (rx != m_receiver) return;
	double threshold = pow(10.0,-2.0 * value/100.0);
    RECEIVER_DEBUG <<  "fmSqLevel set"<< value;
	SetRXAFMSQThreshold(m_receiver, threshold);

}

void Receiver::setNoiseBlankerMode(int rx) {
    if (rx != m_receiver) return;
    m_nb = m_nb2 = 0;
    m_nr = m_nr2 = 0;
    switch (m_nbMode) {
		case 0:
			m_nb = m_nb2 = 0;
			break;
		case 1:
			m_nb = 1;
			break;
		case 2:
			m_nb2 = 1;
			break;

		default:
            RECEIVER_DEBUG << "invalid nb mode" << m_nbMode;
			break;
	}

	switch (m_nrMode) {

		case 0:
			break;
		case 1:
			m_nr = 1;
			break;
		case 2:
			m_nr2 = 1;
			break;

		default:
            RECEIVER_DEBUG <<  "invalid nr mode" <<  m_nrMode;
			break;
	}

	SetRXAEMNRPosition(m_receiver,m_nr_agc);
	SetRXAEMNRaeRun(m_receiver, m_nr2_ae);
	SetRXAEMNRnpeMethod(m_receiver,m_nr2_npe_method);
	SetRXAEMNRgainMethod(m_receiver,m_nr2_gain_method);
	SetEXTANBRun(rx, m_nb);
 	SetEXTNOBRun(rx, m_nb2);
  	SetRXAANRRun(rx, m_nr);
  	SetRXAEMNRRun(rx, m_nr2);
  	SetRXAANFRun(rx, m_anf);
  	SetRXASNBARun(rx, m_snb);
    RECEIVER_DEBUG <<  "nb mode" <<  m_nb;
    RECEIVER_DEBUG <<  "nb2mode" <<  m_nb2;
    RECEIVER_DEBUG <<  "nf mode" <<  m_nr;
    RECEIVER_DEBUG <<  "nr2 mode" <<  m_nr2;
    RECEIVER_DEBUG <<  "anf mode" <<  m_anf;
    RECEIVER_DEBUG <<  "snb mode" <<  m_anf;
}

void Receiver::setNoiseBlankerMode(int rx, int nb) {
	if (rx != m_receiver) return;
	m_nbMode = nb;
    RECEIVER_DEBUG << "nb mode" << nb;
    setNoiseBlankerMode(rx);
}

void Receiver::setNoiseFilterMode(int rx, int nr) {
	m_nrMode = nr;
    setNoiseBlankerMode(rx);
}

void Receiver::setNr2Ae(int rx, bool value) {
    if (rx != m_receiver) return;
    m_nr2_ae = value;
    SetRXAEMNRaeRun(m_receiver, m_nr2_ae);
}

void Receiver::setNr2GainMethod(int rx, int value) {
    if (rx != m_receiver) return;
    m_nr2_gain_method = value;
    SetRXAEMNRgainMethod(m_receiver,m_nr2_gain_method);
}

void Receiver::setNr2NpeMethod(int rx, int value) {
    if (rx != m_receiver) return;
    m_nr2_npe_method = value;
    SetRXAEMNRnpeMethod(m_receiver,m_nr2_npe_method);
}

void Receiver::setNrAGC(int rx, int value) {
    if (rx != m_receiver) return;
    m_nr_agc = value;
    SetRXAEMNRPosition(m_receiver,m_nr_agc);
}

void Receiver::setanf(int rx, bool value) {
	if (rx != m_receiver) return;
	m_anf = value;
    RECEIVER_DEBUG <<  "anf mode" <<  value;
	SetRXAANFRun(rx, m_anf);
}

void Receiver::setsnb(int rx, bool value) {
	m_snb = value;
    RECEIVER_DEBUG <<  "	snb mode" <<  value;
	SetRXASNBARun(rx, m_snb);
}
