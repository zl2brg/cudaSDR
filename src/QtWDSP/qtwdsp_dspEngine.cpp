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
	, m_samplerate(48000)
    , m_inputSampleRate(set ? set->getSampleRate() : 48000)
	, m_ncoFrequency(0)
	, m_fftMultiplier(1)
	, m_volume(0.0f)
	, m_agcThreshold(0.0)
	, m_agcHangLevel(0.0)
	, m_nr(0)
	, m_nr2(0)
	, m_nr3(0)
	, m_nr4(0)
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
    m_agcMode = m_sliceModel ? m_sliceModel->agcMode() : set->getAGCMode(m_rx);
    m_agcSlope = set->getAGCSlope(m_rx);
    m_agcMaximumGain = set->getAGCMaximumGain_dB(m_rx);
    m_agcHangThreshold = set->getAGCHangThreshold(m_rx);
    m_agcAttackTime = static_cast<int>(set->getAGCAttackTime(m_rx));
    m_agcDecayTime = static_cast<int>(set->getAGCDecayTime(m_rx));
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

    if (m_inputSampleRate <= 0)
        m_inputSampleRate = 48000;
    const DSPMode startupMode = currentDspMode();
    const DSPMode startupWdspMode = resolveWDSPMode(startupMode, centerFrequencyHz());
    m_dspmode = startupWdspMode;
    m_samplerate = preferredDspRate(m_dspmode, m_inputSampleRate);
    if (m_sliceModel)
        m_ncoFrequency = static_cast<long>(m_sliceModel->frequency() - m_sliceModel->centerFrequency());
    else
        m_ncoFrequency = static_cast<long>(set->getVfoFrequency(m_rx) - set->getCtrFrequency(m_rx));

    WDSP_ENGINE_DEBUG << "init DSPEngine with size: " << m_size;

    setupConnections();

    WDSP_ENGINE_DEBUG << "[WDSP-INIT] rx=" << m_rx << "size=" << m_size << "inputRate=" << m_inputSampleRate << "dspRate=" << m_samplerate;
    m_channel = std::make_unique<WdspRxChannel>(m_rx);
    m_channel->setVolume(m_volume);
    m_channel->setFftSize(m_fftSize);
    m_channel->open(m_size, m_inputSampleRate, m_samplerate, 48000, startupWdspMode);

    setFilterMode(m_rx);
    applyRxEq();
    applyEmnrPost2();
    const double startupFilterLo = set->getFilterLo(m_rx);
    const double startupFilterHi = set->getFilterHi(m_rx);
    if (startupFilterLo < startupFilterHi) {
        setFilter(startupFilterLo, startupFilterHi);
    } else {
        const auto filter = getFilterFromDSPMode(set->getDefaultFilterList(), startupWdspMode);
        setFilter(filter.filterLo, filter.filterHi);
    }
    applyNco();
    applyAgc();
    calcDisplayAveraging();
    WDSP_ENGINE_DEBUG << "[WDSP-INIT] rx=" << m_rx << "WDSP channel fully initialised via WdspRxChannel.";
}

void QWDSPEngine::stopChannel() {
    WDSP_ENGINE_DEBUG << "[WDSP-STOP] rx=" << m_rx << "-> stopChannel";
    if (m_channel) {
        m_channel->stopChannel();
    }
}

QWDSPEngine::~QWDSPEngine() {
    WDSP_ENGINE_DEBUG << "[WDSP-DESTROY] rx=" << m_rx << "-> close channel";
    if (m_channel) {
        m_channel->close();
    }
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
    connect(m_sliceModel, &SliceModel::filterSlopeChanged, this, [this](int slope){ setFilterSlope(m_rx, slope); });
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

    connect(set, &Settings::rxEqChanged,
            this, [this]() { applyRxEq(); });
    connect(set, &Settings::emnrPost2Changed,
            this, [this]() { applyEmnrPost2(); });







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
    if (m_channel) {
        m_channel->process(in, out);
    }
}

bool QWDSPEngine::getSpectrumPixels(float *pixels, int &ready) {
    return m_channel ? m_channel->getSpectrumPixels(pixels, ready) : false;
}

double QWDSPEngine::getSMeterInstValue() {
    return m_channel ? m_channel->getSMeterInstValue() : -140.0;
}

double QWDSPEngine::getSMeterPeakValue() {
    return m_channel ? m_channel->getSMeterPeakValue() : -140.0;
}

void QWDSPEngine::setVolume(float value) {
    if (value < 0.0f || value > 100.0f) {
        qWarning() << "Invalid volume value:" << value << "valid range: 0.0-100.0";
        return;
    }

    m_volume = value;
    if (m_channel) {
        m_channel->setVolume(value);
    }
    WDSP_ENGINE_DEBUG << "WDSP volume set to" << value;
}

void QWDSPEngine::setQtDSPStatus(bool value) { 
	
	m_qtdspOn = value; 
}

void QWDSPEngine::setDSPMode(DSPMode mode) {

	DSPMode wdspMode = resolveWDSPMode(mode, centerFrequencyHz());
	m_dspmode = wdspMode;
	WDSP_ENGINE_DEBUG << "[RX" << m_rx << "] DSP mode set to" << mode << "(WDSP:" << wdspMode << ")";
	if (m_channel) {
		m_channel->setMode(wdspMode);
	}
	setFilterMode(m_rx);
	applyRxEq();

	const int inRate = m_inputSampleRate > 0 ? m_inputSampleRate : 48000;
	const int targetDsp = preferredDspRate(mode, inRate);
	if (m_samplerate != targetDsp && m_inputSampleRate > 0)
		setSampleRate(m_inputSampleRate, targetDsp);
}

void QWDSPEngine::applyRxEq()
{
	if (!set)
		return;
	if (m_channel)
		m_channel->setRxEq(set->getRxEqBands(), set->getRxEqCurveDeg(), set->getRxEqEnabled());
}

void QWDSPEngine::applyEmnrPost2()
{
	if (!set)
		return;
	if (m_channel)
		m_channel->setEmnrPost2(set->getEmnrPost2Enabled(), set->getEmnrPost2Factor(),
		                       set->getEmnrPost2Nlevel(), static_cast<int>(set->getEmnrPost2Taper()),
		                       set->getEmnrPost2Rate());
}

void QWDSPEngine::setAGCMode(AGCMode agc) {
	m_agcMode = agc;
	if (m_channel) {
		m_channel->setAgcSlope(m_agcSlope);
		m_channel->setAgcAttack(m_agcAttackTime);
		m_channel->setAgcDecay(m_agcDecayTime);
		m_channel->setAgcHang(m_agcMode == agcLONG ? 2000 : (m_agcMode == agcSLOW ? 1000 : 0));
		m_channel->setAgcHangThreshold(m_agcHangThreshold);
		m_channel->setAgcMode(agc);
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
	m_agcMaximumGain = value;
	if (m_channel) m_channel->setAgcMaximumGain(static_cast<double>(value));
	WDSP_ENGINE_DEBUG << "Set AGCMaximum gain " << value;
	emit setAGCLineValues(m_rx);
}

void QWDSPEngine::setAGCHangThreshold(int rx, double value) {
    if (m_rx != rx) return;
	m_agcHangThreshold = value;
	if (m_channel) m_channel->setAgcHangThreshold(value);
   	WDSP_ENGINE_DEBUG << "Set AGC Hang Threshold " << value;
}

void QWDSPEngine::setAGCLineValues(int rx) {
    if (m_rx != rx) return;
    double hang = 0.0;
    double thresh = 0.0;

    if (m_channel) {
        m_channel->getAgcLineLevels(thresh, hang);
    }

    if ((hang != m_agcHangLevel) || (thresh != m_agcHangThreshold))
	{
		m_agcHangLevel = hang;
		m_agcThreshold = thresh;
		emit set->agcLineLevelsChanged(m_rx,thresh,hang);
		WDSP_ENGINE_DEBUG << "Set AGC line value" << hang;

	}
}


void QWDSPEngine::setAGCHangLevel(double level) {

	m_agcHangLevel = level;
	if (m_channel) m_channel->setAgcHangLevel(level);
	WDSP_ENGINE_DEBUG << "Set AGC line value" << level;

}


void QWDSPEngine::setAGCThreshold(double threshold) {

	m_agcThreshold = threshold;
	if (m_channel) m_channel->setAgcThreshold(threshold);
	emit setAGCLineValues(m_rx);
	WDSP_ENGINE_DEBUG << "Set AGC threshold " << threshold;
}

void QWDSPEngine::setAGCHangTime(int value) {

	if (m_channel) m_channel->setAgcHang(value);
	WDSP_ENGINE_DEBUG << "Set AGC Hang time" << value;

}


void QWDSPEngine::setSampleRate(int value) {
    setSampleRate(value, preferredDspRate(m_dspmode, value));
}

int QWDSPEngine::preferredDspRate(DSPMode mode, int inputRate)
{
	Q_UNUSED(mode);
	Q_UNUSED(inputRate);
	// Dual-rate HB → 48 kHz DSP; audio out stays 48 kHz.
	return 48000;
}

void QWDSPEngine::setSampleRate(int inputRate, int dspRate) {
    if (m_samplerate == dspRate && m_inputSampleRate == inputRate) return;
    
    m_samplerate = dspRate;
    m_inputSampleRate = inputRate;

    reconfigure();
}

void QWDSPEngine::setInputSampleRate(int value) {
    if (value <= 0)
        return;

    const int targetDsp = preferredDspRate(m_dspmode, value);
    if (m_inputSampleRate == value && m_samplerate == targetDsp)
        return;

    setSampleRate(value, targetDsp);
}

void QWDSPEngine::reconfigure() {
    WDSP_ENGINE_DEBUG << "[WDSP-CFG] rx=" << m_rx << "reconfigure: input=" << m_inputSampleRate << "Hz dsp=" << m_samplerate << "Hz";

    if (m_channel) {
        m_channel->reconfigure(m_size, m_inputSampleRate, m_samplerate, 48000, m_dspmode);
        m_channel->setFilter(m_filterLo, m_filterHi);
        setFilterMode(m_rx);
        applyRxEq();
        applyEmnrPost2();
        applyNco();
        applyAgc();
        init_analyzer(m_refreshrate);
        calcDisplayAveraging();
        m_channel->setVolume(m_volume);
    }

    WDSP_ENGINE_DEBUG << "[WDSP-CFG] rx=" << m_rx << "reconfigure complete";
}


void QWDSPEngine::setFilter(double low,double high) {
    m_filterLo = low;
    m_filterHi = high;
    if (m_channel) {
        m_channel->setFilter(low, high);
    }
    emit setAGCLineValues(m_rx);
    WDSP_ENGINE_DEBUG << "Set Filter:Low  " <<  low << "High " << high;
}

void QWDSPEngine::setFilterSlope(int rx, int slope) {
    if (m_rx != rx) return;
    m_filterSlope = slope;
    if (m_channel) {
        m_channel->setFilterSlope(slope);
    }
    WDSP_ENGINE_DEBUG << "Set Filter Slope rx=" << rx << " slope=" << slope;
}


void QWDSPEngine::applyNco() {
    if (m_channel) {
        m_channel->setNcoFrequency(m_ncoFrequency);
    }
}

void QWDSPEngine::applyAgc() {
    if (m_channel) {
        m_channel->setAgcMode(m_agcMode);
        m_channel->setAgcMaximumGain(m_agcMaximumGain);
        if (m_agcThreshold != 0.0) m_channel->setAgcThreshold(m_agcThreshold);
        if (m_agcHangLevel != 0.0) m_channel->setAgcHangLevel(m_agcHangLevel);
    }
}

void QWDSPEngine::setNCOFrequency(int rx, long ncoFreq) {

	if (m_rx != rx) return;

	m_ncoFrequency = ncoFreq;
	if (getQtDSPStatus() == 0) return;

	applyNco();
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
    if (m_channel) {
        m_channel->setFftSize(m_fftSize);
        m_channel->initAnalyzer(refreshrate);
    }
}



void QWDSPEngine::setFramesPerSecond(int rx, int value){

	if (rx != m_rx) return;
    
    std::lock_guard<QMutex> lock(m_mutex);
    m_refreshrate = value;
    init_analyzer(value);
    calcDisplayAveraging();
    WDSP_ENGINE_DEBUG << "SetFramesPerSecond" << value;
}


void QWDSPEngine::setPanAdaptorAveragingMode(int rx, int mode) {
    if (rx != m_rx) return;
    m_PanAvMode = mode;
    calcDisplayAveraging();
    WDSP_ENGINE_DEBUG <<  "Setpan av mode" <<  mode;
}


void QWDSPEngine::setPanAdaptorDetectorMode(int rx, int mode) {
    if (rx != m_rx) return;
    m_PanDetMode = mode;
    calcDisplayAveraging();
    WDSP_ENGINE_DEBUG <<  "Setpan av det  mode" <<  mode;
}

void QWDSPEngine::setPanAdaptorAveragingCnt(int rx, int count){
    if (rx != m_rx) return;
    m_averageCount = count;
    calcDisplayAveraging();
    WDSP_ENGINE_DEBUG <<  "Setpan av count mode" <<  m_display_avb << " " << m_display_average;
}

void QWDSPEngine::calcDisplayAveraging() {
    const double t = 0.001 * m_averageCount;
    m_display_avb = std::exp(-1.0 / (static_cast<double>(m_refreshrate) * t));
    m_display_average = std::max(2, static_cast<int>(
        std::min(60.0, static_cast<double>(m_refreshrate) * t)
    ));
    if (m_channel) {
        m_channel->setDisplayAveraging(m_display_avb, m_display_average, m_PanDetMode, m_PanAvMode);
    }
}

int QWDSPEngine::getfftVal(int size) {
    static const std::map<int, int> fftSizeMap = {
        {0, 2048},
        {1, 4096},
        {2, 8192},
        {3, 16384},
        {4, 32768},
        {5, 65536},
        {6, 131072},
        {7, 262144}
    };
    
    auto it = fftSizeMap.find(size);
    if (it != fftSizeMap.end()) {
        return it->second;
    }
    
    WDSP_ENGINE_DEBUG << "invalid fft size set" << size << "using default 2048";
    return 2048;
}


void QWDSPEngine::setfftSize(int rx, int value) {
	if (rx != m_rx) return;
    
    m_fftSize = getfftVal(value);
    WDSP_ENGINE_DEBUG << "mfftsize set" << m_fftSize;
    
    std::lock_guard<QMutex> lock(m_mutex);
    if (m_channel) {
        m_channel->setFftSize(m_fftSize);
    }
    calcDisplayAveraging();
}


void QWDSPEngine::setfmsqLevel(int rx, int value) {
	if (rx != m_rx) return;
	WDSP_ENGINE_DEBUG <<  "fmSqLevel set" <<  value;
	if (m_channel) {
		m_channel->setFmSquelch(true, static_cast<double>(value));
	}
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

	m_nr = m_nr2 = m_nr3 = m_nr4 = 0;
	switch (m_nrMode) {
		case 0:
			break;
		case 1:
			m_nr = 1;
			break;
		case 2:
			m_nr2 = 1;
			break;
		case 3:
			m_nr3 = 1;
			break;
		case 4:
			m_nr4 = 1;
			break;
		default:
			WDSP_ENGINE_DEBUG <<  "invalid nr mode" <<  m_nrMode;
			break;
	}

	if (m_channel) {
		m_channel->setNoiseBlankerMode(m_nbMode);
		m_channel->setNoiseFilterMode(m_nrMode);
		m_channel->setNrAGC(m_nr_agc);
		m_channel->setNr2Ae(m_nr2_ae);
		m_channel->setNr2NpeMethod(m_nr2_npe_method);
		m_channel->setNr2GainMethod(m_nr2_gain_method);
		m_channel->setAnf(m_anf);
		m_channel->setSnb(m_snb);
	}
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
    if (m_channel) m_channel->setNr2Ae(value);
}

void QWDSPEngine::setNr2GainMethod(int rx, int value) {
    if (rx != m_rx) return;
    m_nr2_gain_method = value;
    if (m_channel) m_channel->setNr2GainMethod(value);
}

void QWDSPEngine::setNr2NpeMethod(int rx, int value) {
    if (rx != m_rx) return;
    m_nr2_npe_method = value;
    if (m_channel) m_channel->setNr2NpeMethod(value);
}

void QWDSPEngine::setNrAGC(int rx, int value) {
    if (rx != m_rx) return;
    m_nr_agc = value;
    if (m_channel) m_channel->setNrAGC(value);
}


void QWDSPEngine::setanf(int rx, bool value) {
	if (rx != m_rx) return;
	m_anf = value;
	WDSP_ENGINE_DEBUG <<  "anf mode" <<  value;
	if (m_channel) m_channel->setAnf(value);
}

void QWDSPEngine::setsnb(int rx, bool value) {
	m_snb = value;
	WDSP_ENGINE_DEBUG <<  "	snb mode" <<  value;
	if (m_channel) m_channel->setSnb(value);
}

// TX WDSP channel state is managed by Transmitter::setRadioState().
// This function handles only the RX channel side of TX/RX switching.
void QWDSPEngine::set_txrx(RadioState state) {
    if (state == RadioState::RX && m_channel) {
        m_channel->setChannelState(1, 1);
    }
}
