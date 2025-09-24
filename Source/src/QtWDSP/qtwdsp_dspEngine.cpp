/**
* @file  qtwdsp_dspEngine.cpp
* @brief QtWDSP DSP engine class
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2012-04-07
*/

/*   
 *   Copyright (C) 2007, 2008, 2009, 2010 Philip A Covington, N8VB
 *
 *	 adapted for QtDSP by (C) 2012 Hermann von Hasseln, DL3HVH
 *
 *   The ProcessFrequencyShift method is adpated from cuteSDR by (C) Moe Wheatley, AE4JY.
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
#define LOG_WDSP_ENGINE

// use: WDSP_ENGINE_DEBUG << "debug message";

#include "qtwdsp_dspEngine.h"

#include <algorithm>
#include <cmath>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

// Add missing constants here
namespace {
    constexpr double DEFAULT_KEEP_TIME = 0.1;
    constexpr double DEFAULT_KAISER_PI = 14.0;
    constexpr int DEFAULT_PIXELS = 4096;
    constexpr int DEFAULT_FFT_SIZE = 2048;
    constexpr int QWDSPEngine_BUFFER_SIZE = 1024;
}

double wmyLog(double x, double base) {

	return log(x) / log(base);
}

QWDSPEngine::QWDSPEngine(QObject *parent, int rx, int size)
	: QObject(parent)
	, set(Settings::instance())
	, m_qtdspOn(false)
	, m_rx(rx)
	, m_size(size)
	, m_samplerate(set->getSampleRate())
	, m_fftMultiplier(1)
	, m_volume(0.0f)
{
    if (!set) {
        qCritical() << "Settings instance is null!";
        return;
    }
    
    if (rx < 0 || size <= 0) {
        qCritical() << "Invalid parameters: rx=" << rx << " size=" << size;
        return;
    }

    qRegisterMetaType<QVector<cpx>>();
    qRegisterMetaType<CPX>();
    
    m_refreshrate = set->getFramesPerSecond(m_rx);
    m_averageCount = set->getSpectrumAveragingCnt(m_rx);
    m_PanAvMode = set->getPanAveragingMode(m_rx);
    m_PanDetMode = set->getPanDetectorMode(m_rx);
    m_agcSlope = set->getAGCSlope(m_rx);
    m_agcMaximumGain = set->getAGCMaximumGain_dB(m_rx);
    spectrumBuffer.resize(QWDSPEngine_BUFFER_SIZE * 4);
    spectrumBuffer.resize(BUFFER_SIZE * 4);
    m_fftSize = getfftVal(set->getfftSize(m_rx));
    m_nr_agc = set->getNrAGC(m_rx);
    m_nr2_ae = set->getNr2ae(m_rx);
    m_nr2_gain_method = set->getNr2GainMethod(m_rx);
    m_nr2_npe_method = set->getNr2NpeMethod(m_rx);
    m_nbMode = set->getnbMode(m_rx);
    m_nrMode = set->getnrMode(m_rx);
    m_anf = set->getAnf(m_rx);
    m_snb = set->getSnb(m_rx);

    setNCOFrequency(m_rx, m_rxData.vfoFrequency - m_rxData.ctrFrequency);
    WDSP_ENGINE_DEBUG << "init DSPEngine with size: " << m_size;
    SleeperThread::msleep(100);

    setupConnections();

    WDSP_ENGINE_DEBUG << "Opening WDSP channel" << m_rx << "m_size=" << m_size << "Sample rate=" << m_samplerate;
    OpenChannel(m_rx, m_size, 2048, m_samplerate, 48000, 48000, 0, 0, 0.010, 0.025, 0.0, 0.010, 0);
    create_anbEXT(m_rx, 1, size, m_samplerate, 0.0001, 0.0001, 0.0001, 0.05, 20);
    create_nobEXT(m_rx, 1, 0, size, m_samplerate, 0.0001, 0.0001, 0.0001, 0.05, 20);
    
    qDebug() << "RXASetNC" << m_fftSize;
    RXASetNC(m_rx, m_fftSize);

    setFilterMode(m_rx);
    SetRXAFMDeviation(m_rx, 8000.0);
    SetRXAMode(m_rx, FMN);
    RXASetNC(m_rx, 4096);
    SetRXAPanelRun(m_rx, 1);
    SetRXAPanelSelect(m_rx, 3);
    
    int analyzerResult;
    XCreateAnalyzer(m_rx, &analyzerResult, 262144, 1, 1, const_cast<char*>(""));
    if (analyzerResult != 0) {
        qWarning() << "XCreateAnalyzer id=" << m_rx << "failed:" << analyzerResult;
    }
    init_analyzer(m_refreshrate);
    calcDisplayAveraging();
    SetDisplayAvBackmult(m_rx, 0, m_display_avb);
    SetDisplayNumAverage(m_rx, 0, m_display_average);
    SetDisplayDetectorMode(m_rx, 0, m_PanDetMode);
    SetDisplayAverageMode(m_rx, 0, m_PanAvMode);
    SetRXAFMSQRun(m_rx, 1);
    SetChannelState(m_rx, 1, 0);
}

QWDSPEngine::~QWDSPEngine() {

    if (SetChannelState(m_rx, 0, 0) != 0) {
        qWarning() << "Failed to stop RX channel" << m_rx;
    }
    if (SetChannelState(TX_ID, 0, 0) != 0) {
        qWarning() << "Failed to stop TX channel" << TX_ID;
    }
  //  CloseChannel(m_rx);
  //  CloseChannel(TX_ID);
    DestroyAnalyzer(m_rx);
    SetRXAFMSQRun(m_rx, 0);
    destroy_nobEXT(m_rx);
    destroy_anbEXT(m_rx);
}

void QWDSPEngine::setupConnections() {

    connect(set, &Settings::ncoFrequencyChanged,
            this, &QWDSPEngine::setNCOFrequency);

    connect(set, &Settings::sampleSizeChanged,
            this, &QWDSPEngine::setSampleSize);

    connect(set, &Settings::framesPerSecondChanged,
            this, &QWDSPEngine::setFramesPerSecond);

    connect(set, &Settings::panAveragingModeChanged,
            this, &QWDSPEngine::setPanAdaptorAveragingMode);

    connect(set, &Settings::panDetectorModeChanged,
            this, &QWDSPEngine::setPanAdaptorDetectorMode);

    connect(set, &Settings::spectrumAveragingCntChanged,
            this, &QWDSPEngine::setPanAdaptorAveragingCnt);

    connect(set, &Settings::fftSizeChanged,
            this, &QWDSPEngine::setfftSize);

    connect(set, &Settings::fmsqLevelChanged,
            this, &QWDSPEngine::setfmsqLevel);

    connect(set, &Settings::noiseBlankerChanged,
            this, &QWDSPEngine::setNoiseBlankerMode);

    connect(set, &Settings::noiseFilterChanged,
            this, &QWDSPEngine::setNoiseFilterMode);

    connect(set, &Settings::nr2AeChanged,
            this, &QWDSPEngine::setNr2Ae);

    connect(set, &Settings::nr2NpeMethodChanged,
            this, &QWDSPEngine::setNr2NpeMethod);

    connect(set, &Settings::nr2GainMethodChanged,
            this, &QWDSPEngine::setNr2GainMethod);

    connect(set, &Settings::nrAgcChanged,
            this, &QWDSPEngine::setNrAGC);

    connect(set, &Settings::anfChanged,
            this, &QWDSPEngine::setanf);

    connect(set, &Settings::snbChanged,
            this, &QWDSPEngine::setsnb);
}



void QWDSPEngine::processDSP(CPX &in, CPX &out) {
    int error;
    std::lock_guard<QMutex> lock(m_mutex);  // RAII - automatic unlock
    
    fexchange0(m_rx, reinterpret_cast<double*>(in.data()), 
               reinterpret_cast<double*>(out.data()), &error);
    if (error != 0) {
        WDSP_ENGINE_DEBUG << "WDSP channel read error" << error;
    } else {
        Spectrum0(1, m_rx, 0, 0, reinterpret_cast<double*>(in.data()));
    }
}

double QWDSPEngine::getSMeterInstValue() {

    return  GetRXAMeter(m_rx,RXA_S_AV);

}

void QWDSPEngine::setVolume(float value) {
    // Add parameter validation
    if (value < 0.0f || value > 100.0f) {
        qWarning() << "Invalid volume value:" << value << "valid range: 0.0-100.0";
        return;
    }
    
    SetRXAPanelGain1(m_rx, static_cast<double>(value));
    WDSP_ENGINE_DEBUG << "WDSP volume set to" << value;
}

void QWDSPEngine::setQtDSPStatus(bool value) { 
	
	m_qtdspOn = value; 
}

void QWDSPEngine::setDSPMode(DSPMode mode) {

	m_dspmode = mode;
	WDSP_ENGINE_DEBUG << "WDSP mode set to " << mode;
	SetRXAMode(m_rx, mode);

}

void QWDSPEngine::setAGCMode(AGCMode agc) {
		SetRXAAGCMode(m_rx, agc);
		//SetRXAAGCThresh(rx->id, agc_thresh_point, 4096.0, rx->sample_rate);
		SetRXAAGCSlope(m_rx,m_agcSlope);
	//	SetRXAAGCTop(m_rx,m_agcMaximumGain);
		switch(agc) {
			case agcOFF:
				break;
			case agcLONG:
				SetRXAAGCAttack(m_rx,2);
				SetRXAAGCHang(m_rx,2000);
				SetRXAAGCDecay(m_rx,2000);
				SetRXAAGCHangThreshold(m_rx, m_agcHangThreshold);
				break;
			case agcSLOW:
				SetRXAAGCAttack(m_rx,2);
				SetRXAAGCHang(m_rx,1000);
				SetRXAAGCDecay(m_rx,500);
				SetRXAAGCHangThreshold(m_rx,m_agcHangThreshold);
				break;
			case agcMED:
				SetRXAAGCAttack(m_rx,2);
				SetRXAAGCHang(m_rx,0);
				SetRXAAGCDecay(m_rx,250);
				SetRXAAGCHangThreshold(m_rx,100);
				break;
			case agcFAST:
				SetRXAAGCAttack(m_rx,2);
				SetRXAAGCHang(m_rx,0);
				SetRXAAGCDecay(m_rx,50);
				SetRXAAGCHangThreshold(m_rx,100);
				break;

			case agcUser:
				SetRXAAGCAttack(m_rx,m_agcAttackTime);
				SetRXAAGCHang(m_rx,0);
				SetRXAAGCDecay(m_rx,m_agcDecayTime);
				SetRXAAGCHangThreshold(m_rx,m_agcHangThreshold);
				break;
		}
	emit setAGCLineValues(m_rx);
	WDSP_ENGINE_DEBUG << "Set AGC Mode " << agc;

}

void QWDSPEngine::setAGCAttackTime(int rx, int value) {
    if (m_rx != rx) return;
	m_agcAttackTime = value;
}

void QWDSPEngine::setAGCDecayTime(int rx, int value) {
    if (m_rx != rx) return;
	m_agcDecayTime = value;
}

void QWDSPEngine::setAGCSlope(int rx, int value) {
    if (m_rx != rx) return;
	m_agcSlope = value;
}


void QWDSPEngine::setAGCMaximumGain(qreal value) {
	SetRXAAGCTop(m_rx, (double)value);
	m_agcMaximumGain = value;
    WDSP_ENGINE_DEBUG << "Set AGCMaximum gain " << value;
	emit setAGCLineValues(m_rx);
}

void QWDSPEngine::setAGCHangThreshold(int rx, double value) {
    if (m_rx != rx) return;
	m_agcHangLevel = value;
   	WDSP_ENGINE_DEBUG << "Set AGC Hang Threshold " << value;
}

void QWDSPEngine::setAGCLineValues(int rx) {
    if (m_rx != rx) return;
    double hang;
    double thresh;

    GetRXAAGCHangLevel(m_rx, &hang);
    GetRXAAGCThresh(m_rx, &thresh, 2048, (double)m_samplerate);

    if ((hang != m_agcHangLevel) || (thresh != m_agcHangThreshold))
	{
		m_agcHangLevel = hang;
		m_agcThreshold = thresh;
		emit set->agcLineLevelsChanged(this,m_rx,thresh,hang);
		WDSP_ENGINE_DEBUG << "Set AGC line value" << hang;

	}

//    qreal noiseOffset = 10.0 * log10(qAbs(filter->filterHi() - filter->filterLo()) * 2 * m_size / m_samplerate);
//    qreal threshold = 20.0 * log10(thresh) - noiseOffset + AGCOFFSET;


}


void QWDSPEngine::setAGCHangLevel(double level) {

	SetRXAAGCHangLevel(m_rx,level);
	WDSP_ENGINE_DEBUG << "Set AGC line value" << level;

}


void QWDSPEngine::setAGCThreshold(double threshold) {

	SetRXAAGCThresh(m_rx,threshold,2048,this->m_samplerate);
	emit setAGCLineValues(m_rx);
	WDSP_ENGINE_DEBUG << "Set AGC threshold " << threshold;
}

void QWDSPEngine::setAGCHangTime(int value) {

	SetRXAAGCHang(m_rx,value);
	WDSP_ENGINE_DEBUG << "Set AGC Hang time" << value;

}


void QWDSPEngine::setSampleRate(QObject *sender, int value) {
    Q_UNUSED(sender)

    if (m_samplerate == value) return;

    // Use modern validation
    static const std::set<int> validRates{48000, 96000, 192000, 384000};
    if (validRates.find(value) == validRates.end()) {
        WDSP_ENGINE_DEBUG << "Invalid sample rate:" << value 
                          << "Valid rates: 48, 96, 192, or 384 kHz";
        return;
    }

    m_samplerate = value;
    
    // Add error checking for WDSP calls
    if (SetChannelState(m_rx, 0, 1) != 0) {
        qWarning() << "Failed to stop channel" << m_rx;
    }
    
    SetInputSamplerate(m_rx, m_samplerate);

    init_analyzer(m_refreshrate);
    SetEXTANBSamplerate(m_rx, m_samplerate);
    SetEXTNOBSamplerate(m_rx, m_samplerate);
    
    if (SetChannelState(m_rx, 1, 0) != 0) {
        qWarning() << "Failed to restart channel" << m_rx;
    }
    
    WDSP_ENGINE_DEBUG << "Sample rate set to" << m_samplerate;
}


void QWDSPEngine:: setFilter(double low,double high) {


	if(m_dspmode == FMN) {
		SetRXAFMDeviation(m_rx, (double)8000.0);
		}
	RXASetPassband(m_rx,low,high);
	emit setAGCLineValues(m_rx);
    WDSP_ENGINE_DEBUG << "Set Filter:Low  " <<  low << "High " << high;
}


void QWDSPEngine::setNCOFrequency(int rx, long ncoFreq) {

	if (getQtDSPStatus() == 0 ) return;

	if (m_rx != rx) return;

	if(ncoFreq==0) {
		SetRXAShiftFreq(m_rx, (double)ncoFreq);
		RXANBPSetShiftFrequency(m_rx, (double)ncoFreq);
		SetRXAShiftRun(m_rx, 0);
	} else {
		SetRXAShiftFreq(m_rx, (double)ncoFreq);
		RXANBPSetShiftFrequency(m_rx, (double)ncoFreq);
		SetRXAShiftRun(m_rx, 1);
	}
}

void QWDSPEngine::setSampleSize(int rx, int size) {

	if (m_rx == rx) {

		m_mutex.lock();
		m_spectrumSize = size;
		WDSP_ENGINE_DEBUG <<  "Set sample size" <<  size;
		m_mutex.unlock();
	}
}

void QWDSPEngine::ProcessFrequencyShift(CPX &in, CPX &out) {


}

void QWDSPEngine::init_analyzer(int refreshrate) {
    constexpr int flp[] = {0};
    constexpr double keep_time = DEFAULT_KEEP_TIME;
    constexpr int n_pixout = 1;
    constexpr int spur_elimination_ffts = 1;
    constexpr int data_type = 1;
    const int fft_size = m_fftSize;
    constexpr int window_type = 6;
    constexpr double kaiser_pi = DEFAULT_KAISER_PI;
    constexpr int clip = 0;
    constexpr int span_clip_l = 0;
    constexpr int span_clip_h = 0;
    constexpr int pixels = DEFAULT_PIXELS;
    constexpr int stitches = 1;
    constexpr int calibration_data_set = 0;
    constexpr double span_min_freq = 0.0;
    constexpr double span_max_freq = 0.0;

    const int max_w = fft_size + static_cast<int>(
        std::min(keep_time * refreshrate, 
                keep_time * static_cast<double>(fft_size) * static_cast<double>(refreshrate))
    );

    const int overlap = static_cast<int>(
        std::max(0.0, std::ceil(fft_size - static_cast<double>(m_samplerate) / static_cast<double>(refreshrate)))
    );

    qDebug() << "SetAnalyzer id=" << m_rx << "buffer_size=" << m_size 
             << "overlap=" << overlap << "fft=" << m_fftSize;

    SetAnalyzer(m_rx, n_pixout, spur_elimination_ffts, data_type, 
                const_cast<int*>(flp), fft_size, 1024, window_type, kaiser_pi, 
                overlap, clip, span_clip_l, span_clip_h, pixels, stitches, 
                calibration_data_set, span_min_freq, span_max_freq, max_w);
}



void QWDSPEngine::setFramesPerSecond(QObject* sender, int rx, int value){

	Q_UNUSED(sender)
	if (rx != m_rx) return;
    
    std::lock_guard<QMutex> lock(m_mutex);  // RAII mutex guard
    m_refreshrate = value;
    init_analyzer(value);
    calcDisplayAveraging();
    SetDisplayAvBackmult(rx, 0, m_display_avb);
    SetDisplayNumAverage(rx, 0, m_display_average);
    WDSP_ENGINE_DEBUG << "SetFramesPerSecond" << value;
}


void QWDSPEngine::setPanAdaptorAveragingMode(int rx, int mode) {
    if (rx != m_rx) return;
    WDSP_ENGINE_DEBUG <<  "Setpan av mode" <<  mode;
    SetDisplayAverageMode(m_rx,0,mode);
    calcDisplayAveraging();
    SetDisplayAvBackmult(rx, 0, m_display_avb);
    SetDisplayNumAverage(rx, 0, m_display_average);
}


void QWDSPEngine::setPanAdaptorDetectorMode(int rx, int mode) {
    if (rx != m_rx) return;
    WDSP_ENGINE_DEBUG <<  "Setpan av det  mode" <<  mode;
    SetDisplayDetectorMode(rx,0,mode);

}

void QWDSPEngine::setPanAdaptorAveragingCnt(QObject* sender, int rx, int count){
    Q_UNUSED(sender);
    if (rx != m_rx) return;
    m_averageCount = count;
    calcDisplayAveraging();
    SetDisplayAvBackmult(rx, 0, m_display_avb);
    SetDisplayNumAverage(rx, 0, m_display_average);
    WDSP_ENGINE_DEBUG <<  "Setpan av count mode" <<  m_display_avb << " " << m_display_average;
}

void QWDSPEngine::calcDisplayAveraging() {
    const double t = 0.001 * m_averageCount;
    m_display_avb = std::exp(-1.0 / (static_cast<double>(m_refreshrate) * t));
    m_display_average = std::max(2, static_cast<int>(
        std::min(60.0, static_cast<double>(m_refreshrate) * t)
    ));
}

int QWDSPEngine::getfftVal(int size) {
    // Use modern container for better maintainability
    static const std::map<int, int> fftSizeMap = {
        {0, 2048},
        {1, 4096},
        {2, 8192},
        {3, 16384},
        {4, 32768},
        {5, 65536},   // FIX: was 655356 - typo!
        {6, 131072},
        {7, 262144}   // FIX: was 655356 - another typo!
    };
    
    auto it = fftSizeMap.find(size);
    if (it != fftSizeMap.end()) {
        return it->second;
    }
    
    WDSP_ENGINE_DEBUG << "invalid fft size set" << size << "using default 2048";
    return 2048;  // Default fallback
}


void QWDSPEngine::setfftSize(int rx, int value) {
	if (rx != m_rx) return;
    
    m_fftSize = getfftVal(value);
    WDSP_ENGINE_DEBUG << "mfftsize set" << m_fftSize;
    
    std::lock_guard<QMutex> lock(m_mutex);  // RAII mutex guard
    init_analyzer(value);
    calcDisplayAveraging();
    SetDisplayAvBackmult(rx, 0, m_display_avb);
    SetDisplayNumAverage(rx, 0, m_display_average);
}


void QWDSPEngine::setfmsqLevel(int rx, int value) {
	if (rx != m_rx) return;
	double threshold = pow(10.0,-2.0 * value/100.0);
	WDSP_ENGINE_DEBUG <<  "fmSqLevel set" <<  value;
	SetRXAFMSQThreshold(m_rx, threshold);

}

void QWDSPEngine::setFilterMode(int rx) {
    if (rx != m_rx) return;
	switch (m_nbMode) {
		case 0:
			m_nb = m_nb2 = 0;
			break;
		case 1:
			m_nb = 1;
			m_nb2 = 0;
			break;
		case 2:
			m_nb = 0;
			m_nb2 = 1;
			break;

		default:
			WDSP_ENGINE_DEBUG << "invalid nb mode" << m_nbMode;
			break;
	}

	switch (m_nrMode) {

		case 0:
			m_nr = m_nr2 = 0;
			break;
		case 1:
			m_nr = 1;
			m_nr2 = 0;
			break;
		case 2:
			m_nr = 0;
			m_nr2 = 1;
			break;

		default:
			WDSP_ENGINE_DEBUG <<  "invalid nr mode" <<  m_nrMode;
			break;
	}

	SetRXAEMNRPosition(m_rx,m_nr_agc);
	SetRXAEMNRaeRun(m_rx, m_nr2_ae);
	SetRXAEMNRnpeMethod(m_rx,m_nr2_npe_method);
	SetRXAEMNRgainMethod(m_rx,m_nr2_gain_method);
	SetEXTANBRun(rx, m_nb);
 	SetEXTNOBRun(rx, m_nb2);
  	SetRXAANRRun(rx, m_nr);
  	SetRXAEMNRRun(rx, m_nr2);
  	SetRXAANFRun(rx, m_anf);
  	SetRXASNBARun(rx, m_snb);
    WDSP_ENGINE_DEBUG <<  "nb mode" <<  m_nb;
    WDSP_ENGINE_DEBUG <<  "nb2mode" <<  m_nb2;
    WDSP_ENGINE_DEBUG <<  "nf mode" <<  m_nr;
    WDSP_ENGINE_DEBUG <<  "nr2 mode" <<  m_nr2;
    WDSP_ENGINE_DEBUG <<  "anf mode" <<  m_anf;
    WDSP_ENGINE_DEBUG <<  "snb mode" <<  m_anf;
}

void QWDSPEngine::setNoiseBlankerMode(int rx, int nb) {
	if (rx != m_rx) return;
	m_nbMode = nb;
	WDSP_ENGINE_DEBUG << "nb mode" << nb;
	setFilterMode(rx);
}


void QWDSPEngine::setNoiseFilterMode(int rx, int nr) {
	m_nrMode = nr;
	setFilterMode(rx);
}

void QWDSPEngine::setNr2Ae(int rx, bool value) {
    if (rx != m_rx) return;
    m_nr2_ae = value;
    SetRXAEMNRaeRun(m_rx, m_nr2_ae);
}

void QWDSPEngine::setNr2GainMethod(int rx, int value) {
    if (rx != m_rx) return;
    m_nr2_gain_method = value;
    SetRXAEMNRgainMethod(m_rx,m_nr2_gain_method);
}

void QWDSPEngine::setNr2NpeMethod(int rx, int value) {
    if (rx != m_rx) return;
    m_nr2_npe_method = value;
    SetRXAEMNRnpeMethod(m_rx,m_nr2_npe_method);
}

void QWDSPEngine::setNrAGC(int rx, int value) {
    if (rx != m_rx) return;
    m_nr_agc = value;
    SetRXAEMNRPosition(m_rx,m_nr_agc);
}


void QWDSPEngine::setanf(int rx, bool value) {
	if (rx != m_rx) return;
	m_anf = value;
	WDSP_ENGINE_DEBUG <<  "anf mode" <<  value;
	SetRXAANFRun(rx, m_anf);
}

void QWDSPEngine::setsnb(int rx, bool value) {
	m_snb = value;
	WDSP_ENGINE_DEBUG <<  "	snb mode" <<  value;
	SetRXASNBARun(rx, m_snb);
}

//#define POSTGEN

void QWDSPEngine::set_txrx(RadioState state) {
    bool tone = true;
    switch (state){
        case RadioState::RX:
        if (tone)
        {
#ifdef POSTGEN
            SetTXAPostGenRun(TX_ID,0);
#else
         SetTXAPreGenRun(TX_ID,0);
#endif
        }
            SetChannelState(TX_ID,0,1);
            SetChannelState(0,1,1);
            qDebug() << "RX";
        break;
        case RadioState::TUNE:
        case RadioState::MOX:

            if (tone)
            {
#ifdef POSTGEN
              SetTXAPostGenToneFreq(TX_ID,1000);
              SetTXAPostGenToneMag(TX_ID,0.5);
              SetTXAPostGenMode(TX_ID,0);

//            SetChannelState(0,0,1);

              SetTXAPostGenRun(TX_ID,1);
#else
            SetTXAPreGenToneFreq(TX_ID,1000);
            SetTXAPreGenToneMag(TX_ID,1);
            SetTXAPreGenMode(TX_ID,0);

//            SetChannelState(0,0,1);

            SetTXAPreGenRun(TX_ID,1);

#endif
            }
            SetTXAMode(TX_ID,0);

//            SetTXABandpassFreqs(TX_ID, 100,1000);
            SetTXAPanelGain1(TX_ID,pow(1.0, 10));
            SetTXAPanelRun(TX_ID, 1);
            SetTXABandpassFreqs(TX_ID,1000,1);
            SetTXABandpassWindow(TX_ID,1);
            SetTXABandpassRun(TX_ID,1);
            SetChannelState(TX_ID,1,0);
        case RadioState::DUPLEX:
        default:
         break;
    }
}
