#include "Models/SliceModel.h"
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

QMutex QWDSPEngine::s_wdspMutex;

double wmyLog(double x, double base) {

	return log(x) / log(base);
}

QWDSPEngine::QWDSPEngine(SliceModel *model, QObject *parent, int size)
	: QObject(parent)
        , m_sliceModel(model)
	, set(Settings::instance())
	, m_qtdspOn(false)
	, m_firstExchangeDone(false)
	, m_rx(model ? model->id() : 0)
	, m_size(size)
	, m_samplerate(set->getSampleRate())
    , m_inputSampleRate(set->getSampleRate())
	, m_fftMultiplier(1)
	, m_volume(0.0f)
    , m_filterLo(-4000.0)
    , m_filterHi(4000.0)
{
    if (!set) {
        qCritical() << "Settings instance is null!";
        return;
    }
    
    if (m_rx < 0 || size <= 0) {
        qCritical() << "Invalid parameters: rx=" << m_rx << " size=" << size;
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
    // spectrumBuffer.resize(BUFFER_SIZE * 4);
    m_fftSize = getfftVal(set->getfftSize(m_rx));
    m_nr_agc = set->getNrAGC(m_rx);
    m_nr2_ae = set->getNr2ae(m_rx);
    m_nr2_gain_method = set->getNr2GainMethod(m_rx);
    m_nr2_npe_method = set->getNr2NpeMethod(m_rx);
    m_nbMode = set->getnbMode(m_rx);
    m_nrMode = set->getnrMode(m_rx);
    m_anf = set->getAnf(m_rx);
    m_snb = set->getSnb(m_rx);

    setNCOFrequency(m_rx, 0);
    WDSP_ENGINE_DEBUG << "init DSPEngine with size: " << m_size;
    QThread::msleep(100);

    setupConnections();

    WDSP_ENGINE_DEBUG << "[WDSP-INIT] rx=" << m_rx << "size=" << m_size << "inputRate=" << m_inputSampleRate << "dspRate=" << m_samplerate << "-> calling OpenChannel";
    OpenChannel(m_rx, m_size, 2048, m_inputSampleRate, m_samplerate, 48000, 0, 0, 0.010, 0.025, 0.0, 0.010, 0);
    WDSP_ENGINE_DEBUG << "[WDSP-INIT] rx=" << m_rx << "OpenChannel done -> create_anbEXT";
    create_anbEXT(m_rx, 1, size, m_inputSampleRate, 0.0001, 0.0001, 0.0001, 0.05, 20);
    WDSP_ENGINE_DEBUG << "[WDSP-INIT] rx=" << m_rx << "create_anbEXT done -> create_nobEXT";
    create_nobEXT(m_rx, 1, 0, size, m_inputSampleRate, 0.0001, 0.0001, 0.0001, 0.05, 20);
    WDSP_ENGINE_DEBUG << "[WDSP-INIT] rx=" << m_rx << "create_nobEXT done";
    
    qDebug() << "[WDSP-INIT] rx=" << m_rx << "RXASetNC(" << m_fftSize << ")";
    RXASetNC(m_rx, m_fftSize);

    setFilterMode(m_rx);
    SetRXAFMDeviation(m_rx, 8000.0);
    SetRXAMode(m_rx, FMN);
    RXASetNC(m_rx, 4096);
    SetRXAPanelRun(m_rx, 1);
    SetRXAPanelSelect(m_rx, 3);
    
    int analyzerResult;
    WDSP_ENGINE_DEBUG << "[WDSP-INIT] rx=" << m_rx << "-> XCreateAnalyzer";
    XCreateAnalyzer(m_rx, &analyzerResult, 32768, 1, 1, const_cast<char*>(""));
    if (analyzerResult != 0) {
        qWarning() << "[WDSP-INIT] XCreateAnalyzer id=" << m_rx << "failed:" << analyzerResult;
    } else {
        WDSP_ENGINE_DEBUG << "[WDSP-INIT] rx=" << m_rx << "XCreateAnalyzer OK";
    }
    init_analyzer(m_refreshrate);
    calcDisplayAveraging();
    SetDisplayAvBackmult(m_rx, 0, m_display_avb);
    SetDisplayNumAverage(m_rx, 0, m_display_average);
    SetDisplayDetectorMode(m_rx, 0, m_PanDetMode);
    SetDisplayAverageMode(m_rx, 0, m_PanAvMode);
    SetRXAFMSQRun(m_rx, 1);
    WDSP_ENGINE_DEBUG << "[WDSP-INIT] rx=" << m_rx << "-> SetChannelState(1,0) (start channel)";
    SetChannelState(m_rx, 1, 0);
    WDSP_ENGINE_DEBUG << "[WDSP-INIT] rx=" << m_rx << "WDSP channel fully initialised.";
}

void QWDSPEngine::stopChannel() {
    // Set run=0 immediately (no wait).  Called from DataEngine::stop() while
    // the DSP processing thread is still alive so any in-flight fexchange0
    // can observe the flag and exit cleanly.  This avoids the semaphore
    // deadlock that occurs when SetChannelState(wait=1) is called after the
    // DSP thread is already dead.
    WDSP_ENGINE_DEBUG << "[WDSP-STOP] rx=" << m_rx << "-> SetChannelState(0,0) (signal stop, no wait)";
    SetChannelState(m_rx, 0, 0);
}

QWDSPEngine::~QWDSPEngine() {

    // Channel run flag was already cleared by stopChannel() before the DSP
    // thread was killed.  Just tear down the WDSP resources in order.
    WDSP_ENGINE_DEBUG << "[WDSP-DESTROY] rx=" << m_rx << "-> DestroyAnalyzer";
    DestroyAnalyzer(m_rx);
    SetRXAFMSQRun(m_rx, 0);
    WDSP_ENGINE_DEBUG << "[WDSP-DESTROY] rx=" << m_rx << "-> destroy_nobEXT/anbEXT";
    destroy_nobEXT(m_rx);
    destroy_anbEXT(m_rx);
    WDSP_ENGINE_DEBUG << "[WDSP-DESTROY] rx=" << m_rx << "-> CloseChannel";
    CloseChannel(m_rx);
    WDSP_ENGINE_DEBUG << "[WDSP-DESTROY] rx=" << m_rx << "done.";
}

void QWDSPEngine::setupConnections() {

    connect(m_sliceModel, &SliceModel::anfChanged, this, [this](bool enabled){ setanf(m_rx, enabled); });
    connect(m_sliceModel, &SliceModel::snbChanged, this, [this](bool enabled){ setsnb(m_rx, enabled); });
    connect(m_sliceModel, &SliceModel::agcModeChanged, this, [this](AGCMode mode){ setAGCMode(mode); });
    connect(m_sliceModel, &SliceModel::agcMaxGainChanged, this, [this](int gain){ setAGCMaximumGain((qreal)gain); });
    connect(m_sliceModel, &SliceModel::agcGainChanged, this, [this](int gain) { setAGCThreshold(gain - AGCOFFSET); });
    connect(m_sliceModel, &SliceModel::agcFixedGainChanged, this, [this](int gain) { SetRXAAGCFixed(m_rx, static_cast<double>(gain)); });
    connect(m_sliceModel, &SliceModel::agcHangThresholdChanged, this, [this](int thresh){ setAGCHangThreshold(m_rx, (double)thresh); });
    connect(m_sliceModel, &SliceModel::agcSlopeChanged, this, [this](int slope){ setAGCSlope(m_rx, slope); });
    connect(m_sliceModel, &SliceModel::filterChanged, this, [this](){ setFilter((double)m_sliceModel->filterLow(), (double)m_sliceModel->filterHigh()); });
    connect(m_sliceModel, &SliceModel::dspModeChanged, this, [this](DSPMode mode) { setDSPMode(mode); });
    connect(m_sliceModel, &SliceModel::nbModeChanged, this, [this](int mode){ setNoiseBlankerMode(m_rx, mode); });
    connect(m_sliceModel, &SliceModel::fftSizeChanged, this, [this](int size){ setfftSize(m_rx, size); });
    connect(m_sliceModel, &SliceModel::spectrumAveragingCntChanged, this, [this](int count){ setPanAdaptorAveragingCnt(m_rx, count); });
    connect(m_sliceModel, &SliceModel::panAveragingModeChanged, this, [this](PanAveragingMode mode){ setPanAdaptorAveragingMode(m_rx, (int)mode); });
    connect(m_sliceModel, &SliceModel::nrModeChanged, this, [this](int mode){ setNoiseFilterMode(m_rx, mode); });
    connect(m_sliceModel, &SliceModel::nr2GainMethodChanged, this, [this](int mode){ setNr2GainMethod(m_rx, mode); });
    connect(m_sliceModel, &SliceModel::nr2NpeMethodChanged, this, [this](int mode){ setNr2NpeMethod(m_rx, mode); });
    connect(m_sliceModel, &SliceModel::nr2AeChanged, this, [this](bool enabled){ setNr2Ae(m_rx, enabled); });
    connect(m_sliceModel, &SliceModel::nrAgcChanged, this, [this](int mode){ setNrAGC(m_rx, mode); });
    connect(m_sliceModel, &SliceModel::volumeChanged, this, [this](float value){ setVolume(value); });
    connect(m_sliceModel, &SliceModel::muteChanged, this, [this](bool muted){ setVolume(muted ? 0.0f : m_sliceModel->volume()); });

    connect(set, &Settings::ncoFrequencyChanged,
            this, &QWDSPEngine::setNCOFrequency);

    connect(set, &Settings::sampleSizeChanged,
            this, &QWDSPEngine::setSampleSize);

    connect(set, &Settings::framesPerSecondChanged,
            this, &QWDSPEngine::setFramesPerSecond);

    // connect(set, &Settings::panAveragingModeChanged,
            // this, &QWDSPEngine::setPanAdaptorAveragingMode);

    if (!m_sliceModel) {
        connect(set, &Settings::panDetectorModeChanged,
                this, &QWDSPEngine::setPanAdaptorDetectorMode);
    }

    // connect(set, &Settings::spectrumAveragingCntChanged,
            // this, &QWDSPEngine::setPanAdaptorAveragingCnt);

    // connect(set, &Settings::fftSizeChanged,
            // this, &QWDSPEngine::setfftSize);

    connect(set, &Settings::fmsqLevelChanged,
            this, &QWDSPEngine::setfmsqLevel);









    // Signals routed directly here instead of relaying through SliceProcessor
    // connect(set, &Settings::mainVolumeChanged,
            // this, [this](int rx, float value) {
        // if (rx == m_rx) setVolume(value);
    // });
    // DSP mode: SliceModel::dspModeChanged (above). Legacy Settings::dspModeChanged only when no slice model.
    if (!m_sliceModel) {
        connect(set, &Settings::dspModeChanged,
                this, [this](int rx, DSPMode mode) {
            if (rx != m_rx) return;
            setDSPMode(mode);
            auto filter = getFilterFromDSPMode(set->getDefaultFilterList(),
                                               resolveWDSPMode(mode, centerFrequencyHz()));
            setFilter(filter.filterLo, filter.filterHi);
        });
    }
    if (m_sliceModel) {
        connect(m_sliceModel, &SliceModel::centerFrequencyChanged,
                this, &QWDSPEngine::updateFreeDvSideband);
    } else {
        connect(set, &Settings::ctrFrequencyChanged,
                this, [this](int /*mode*/, int rx, qint64 frequency) {
            if (rx != m_rx) return;
            updateFreeDvSideband(frequency);
        });
    }
    if (!m_sliceModel) {
        connect(set, &Settings::agcModeChanged,
                this, [this](int rx, AGCMode mode, bool) {
            if (rx == m_rx) setAGCMode(mode);
        });
        connect(set, &Settings::agcGainChanged,
                this, [this](int rx, int value) {
            if (rx == m_rx) setAGCThreshold(value - AGCOFFSET);
        });
        connect(set, &Settings::agcMaximumGainChanged_dB,
                this, [this](int rx, qreal value) {
            if (rx == m_rx) setAGCMaximumGain(value);
        });
        connect(set, &Settings::agcThresholdChanged_dB,
                this, [this](int rx, qreal value) {
            if (rx == m_rx) setAGCThreshold((double)value);
        });
        connect(set, &Settings::agcHangThresholdChanged,
                this, [this](int rx, int value) {
            if (rx == m_rx) setAGCHangThreshold(rx, value / 100.0);
        });
        connect(set, &Settings::agcHangLevelChanged_dB,
                this, [this](int rx, qreal value) {
            if (rx == m_rx) setAGCHangLevel(value - AGCOFFSET);
        });
        connect(set, &Settings::agcVariableGainChanged_dB,
                this, [this](int rx, qreal value) {
            if (rx == m_rx) setAGCSlope(rx, (int)value);
        });
        connect(set, &Settings::agcAttackTimeChanged,
                this, [this](int rx, qreal value) {
            if (rx == m_rx) setAGCAttackTime(rx, (int)value);
        });
        connect(set, &Settings::agcDecayTimeChanged,
                this, [this](int rx, qreal value) {
            if (rx == m_rx) setAGCDecayTime(rx, (int)value);
        });
        connect(set, &Settings::agcHangTimeChanged,
                this, [this](int rx, qreal value) {
            if (rx == m_rx) setAGCHangTime((int)value);
        });
    } else {
        connect(set, &Settings::agcThresholdChanged_dB,
                this, [this](int rx, qreal value) {
            if (rx == m_rx) setAGCThreshold((double)value);
        });
        connect(set, &Settings::agcHangLevelChanged_dB,
                this, [this](int rx, qreal value) {
            if (rx == m_rx) setAGCHangLevel(value - AGCOFFSET);
        });
        connect(set, &Settings::agcAttackTimeChanged,
                this, [this](int rx, qreal value) {
            if (rx == m_rx) setAGCAttackTime(rx, (int)value);
        });
        connect(set, &Settings::agcDecayTimeChanged,
                this, [this](int rx, qreal value) {
            if (rx == m_rx) setAGCDecayTime(rx, (int)value);
        });
        connect(set, &Settings::agcHangTimeChanged,
                this, [this](int rx, qreal value) {
            if (rx == m_rx) setAGCHangTime((int)value);
        });
    }
    if (!m_sliceModel) {
        connect(set, &Settings::filterFrequenciesChanged,
                this, [this](int rx, qreal low, qreal high) {
            if (rx == m_rx) setFilter(low, high);
        });
    }
}



long QWDSPEngine::centerFrequencyHz() const {

    return m_sliceModel ? m_sliceModel->centerFrequency() : set->getCtrFrequency(m_rx);
}

DSPMode QWDSPEngine::currentDspMode() const {

    return m_sliceModel ? m_sliceModel->dspMode() : set->getDSPMode(m_rx);
}

void QWDSPEngine::updateFreeDvSideband(qint64 frequency) {

    if (currentDspMode() != FDV) return;
    // Reselect USB/LSB when frequency crosses the 10 MHz boundary in FDV/FreeDV mode.
    DSPMode wdspMode = resolveWDSPMode(FDV, frequency);
    if (m_dspmode == wdspMode) return;
    m_dspmode = wdspMode;
    WDSP_ENGINE_DEBUG << "FreeDV sideband updated to" << wdspMode << "for freq" << frequency;
    SetRXAMode(m_rx, wdspMode);
    auto filter = getFilterFromDSPMode(set->getDefaultFilterList(), wdspMode);
    setFilter(filter.filterLo, filter.filterHi);
}



void QWDSPEngine::processDSP(CPX &in, CPX &out) {
    int error;
    fexchange0(m_rx, reinterpret_cast<double*>(in.data()),
               reinterpret_cast<double*>(out.data()), &error);
    if (error != 0) {
        if (error == -2) {
            return;
        }
        // Suppress the first-call transient (-20 = ring buffer not yet primed).
        // Log subsequent errors at full severity so real problems are visible.
        if (!m_firstExchangeDone) {
            WDSP_ENGINE_DEBUG << "[WDSP-DSP] rx=" << m_rx
                              << "first fexchange0 startup transient error=" << error << "(suppressed)";
        } else {
            WDSP_ENGINE_DEBUG << "[WDSP-DSP] rx=" << m_rx << "fexchange0 error=" << error;
        }
    } else {
        m_firstExchangeDone = true;
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

    m_volume = value;
    SetRXAPanelGain1(m_rx, static_cast<double>(value));
    WDSP_ENGINE_DEBUG << "WDSP volume set to" << value;
}

void QWDSPEngine::setQtDSPStatus(bool value) { 
	
	m_qtdspOn = value; 
}

void QWDSPEngine::setDSPMode(DSPMode mode) {

	DSPMode wdspMode = resolveWDSPMode(mode, centerFrequencyHz());
	m_dspmode = wdspMode;
	WDSP_ENGINE_DEBUG << "[RX" << m_rx << "] DSP mode set to" << mode << "(WDSP:" << wdspMode << ")";
	SetRXAMode(m_rx, wdspMode);

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
		emit set->agcLineLevelsChanged(m_rx,thresh,hang);
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


void QWDSPEngine::setSampleRate(int value) {
    setSampleRate(value, value);
}

void QWDSPEngine::setSampleRate(int inputRate, int dspRate) {
    if (m_samplerate == dspRate && m_inputSampleRate == inputRate) return;
    
    m_samplerate = dspRate;
    m_inputSampleRate = inputRate;

    reconfigure();
}

void QWDSPEngine::setInputSampleRate(int value) {
    if (value <= 0 || m_inputSampleRate == value)
        return;

    m_inputSampleRate = value;
    reconfigure();
}

void QWDSPEngine::reconfigure() {
    WDSP_ENGINE_DEBUG << "[WDSP-CFG] rx=" << m_rx << "reconfigure: input=" << m_inputSampleRate << "Hz dsp=" << m_samplerate << "Hz";

    QMutexLocker wdspLocker(&s_wdspMutex);

    // Stop and destroy everything related to this channel
    SetChannelState(m_rx, 0, 1);
    DestroyAnalyzer(m_rx);
    destroy_nobEXT(m_rx);
    destroy_anbEXT(m_rx);
    CloseChannel(m_rx);

    // Re-open and re-initialize
    WDSP_ENGINE_DEBUG << "[WDSP-CFG] rx=" << m_rx << "-> OpenChannel";
    OpenChannel(m_rx, m_size, 2048, m_inputSampleRate, m_samplerate, 48000, 0, 0, 0.010, 0.025, 0.0, 0.010, 0);
    
    create_anbEXT(m_rx, 1, m_size, m_inputSampleRate, 0.0001, 0.0001, 0.0001, 0.05, 20);
    create_nobEXT(m_rx, 1, 0, m_size, m_inputSampleRate, 0.0001, 0.0001, 0.0001, 0.05, 20);

    RXASetNC(m_rx, m_fftSize);
    SetRXAMode(m_rx, m_dspmode);
    setFilter(m_filterLo, m_filterHi);
    setFilterMode(m_rx);

    int analyzerResult;
    WDSP_ENGINE_DEBUG << "[WDSP-CFG] rx=" << m_rx << "-> XCreateAnalyzer";
    // Use a conservative but sufficient size for slice analyzers
    XCreateAnalyzer(m_rx, &analyzerResult, 32768, 1, 1, const_cast<char*>(""));
    if (analyzerResult != 0) {
        qWarning() << "[WDSP-CFG] XCreateAnalyzer id=" << m_rx << "failed:" << analyzerResult;
    }

    init_analyzer(m_refreshrate);
    calcDisplayAveraging();
    SetDisplayAvBackmult(m_rx, 0, m_display_avb);
    SetDisplayNumAverage(m_rx, 0, m_display_average);
    SetDisplayDetectorMode(m_rx, 0, m_PanDetMode);
    SetDisplayAverageMode(m_rx, 0, m_PanAvMode);
    SetRXAFMSQRun(m_rx, 1);
    SetRXAPanelGain1(m_rx, static_cast<double>(m_volume));
    SetChannelState(m_rx, 1, 0);

    WDSP_ENGINE_DEBUG << "[WDSP-CFG] rx=" << m_rx << "reconfigure complete";
}


void QWDSPEngine:: setFilter(double low,double high) {
    m_filterLo = low;
    m_filterHi = high;


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
    Q_UNUSED(in)
    Q_UNUSED(out)

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
        std::max(0.0, std::ceil(fft_size - static_cast<double>(m_inputSampleRate) / static_cast<double>(refreshrate)))
    );

    qDebug() << "SetAnalyzer id=" << m_rx << "buffer_size=" << m_size 
             << "overlap=" << overlap << "fft=" << m_fftSize;

    SetAnalyzer(m_rx, n_pixout, spur_elimination_ffts, data_type, 
                const_cast<int*>(flp), fft_size, 1024, window_type, kaiser_pi, 
                overlap, clip, span_clip_l, span_clip_h, pixels, stitches, 
                calibration_data_set, span_min_freq, span_max_freq, max_w);
}



void QWDSPEngine::setFramesPerSecond(int rx, int value){

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

void QWDSPEngine::setPanAdaptorAveragingCnt(int rx, int count){
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
    init_analyzer(m_refreshrate);
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

// TX WDSP channel state is managed by Transmitter::setRadioState().
// This function handles only the RX channel side of TX/RX switching.
void QWDSPEngine::set_txrx(RadioState state) {
    if (state == RadioState::RX) {
        SetChannelState(m_rx, 1, 1);
    }
}
