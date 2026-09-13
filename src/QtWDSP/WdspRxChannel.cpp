#include "WdspRxChannel.h"
#include <QDebug>
#include <cmath>
#include <algorithm>

extern "C" {
#include <wdsp.h>
}

namespace {
constexpr double DEFAULT_KEEP_TIME = 0.1;
constexpr double DEFAULT_KAISER_PI = 14.0;
constexpr int DEFAULT_PIXELS = 4096;
}

WdspRxChannel::WdspRxChannel(int rxId)
    : WdspChannel(rxId)
{
}

WdspRxChannel::~WdspRxChannel()
{
    close();
}

bool WdspRxChannel::open(int buffSize, int inRate, int dspRate, int outRate, DSPMode mode)
{
    close();

    QMutexLocker locker(&m_channelMutex);
    QMutexLocker wisdomLocker(&s_wdspWisdomMutex);

    m_bufferSize = buffSize > 0 ? buffSize : 1024;
    m_inRate = inRate > 0 ? inRate : 48000;
    m_dspRate = dspRate > 0 ? dspRate : 48000;
    m_outRate = outRate > 0 ? outRate : 48000;
    m_dspMode = mode;
    m_firstExchangeDone = false;

    OpenChannel(m_channelId, m_bufferSize, m_bufferSize, m_inRate, m_dspRate, m_outRate,
                0 /* RX */, 0 /* stopped initially */, 0.010, 0.025, 0.0, 0.010, 0);

    create_anbEXT(m_channelId, 1, m_bufferSize, m_inRate, 0.0001, 0.0001, 0.0001, 0.05, 20);
    create_nobEXT(m_channelId, 1, 0, m_bufferSize, m_inRate, 0.0001, 0.0001, 0.0001, 0.05, 20);

    RXASetNC(m_channelId, 4096);
    SetRXAFMDeviation(m_channelId, 8000.0);
    SetRXAMode(m_channelId, m_dspMode);
    RXASetPassband(m_channelId, m_filterLow, m_filterHigh);

    SetRXAPanelRun(m_channelId, 1);
    SetRXAPanelSelect(m_channelId, 3);
    SetRXAPanelGain1(m_channelId, static_cast<double>(m_volume));
    SetRXAFMSQRun(m_channelId, 1);

    int analyzerResult = 0;
    XCreateAnalyzer(m_channelId, &analyzerResult, 262144, 1, 1, const_cast<char*>(""));
    if (analyzerResult == 0) {
        m_analyzerCreated.store(true, std::memory_order_release);
    } else {
        qWarning() << "WdspRxChannel" << m_channelId << "XCreateAnalyzer failed:" << analyzerResult;
        m_analyzerCreated.store(false, std::memory_order_release);
    }

    applyNcoFrequency();
    applyAgcParameters();
    applyNoiseParameters();

    if (m_analyzerCreated.load(std::memory_order_acquire)) {
        initAnalyzer(m_refreshRate);
        SetDisplayAvBackmult(m_channelId, 0, m_displayAvb);
        SetDisplayNumAverage(m_channelId, 0, m_displayAverage);
        SetDisplayDetectorMode(m_channelId, 0, m_panDetMode);
        SetDisplayAverageMode(m_channelId, 0, m_panAvMode);
    }

    SetChannelState(m_channelId, 1, 0);
    m_isOpen.store(true, std::memory_order_release);
    m_isRunning.store(true, std::memory_order_release);
    return true;
}

bool WdspRxChannel::reconfigure(int buffSize, int inRate, int dspRate, int outRate, DSPMode mode)
{
    return open(buffSize, inRate, dspRate, outRate, mode);
}

void WdspRxChannel::close()
{
    QMutexLocker locker(&m_channelMutex);
    if (!m_isOpen.load(std::memory_order_acquire)) {
        return;
    }

    stopChannel();

    QMutexLocker wisdomLocker(&s_wdspWisdomMutex);
    if (m_analyzerCreated.load(std::memory_order_acquire)) {
        DestroyAnalyzer(m_channelId);
        m_analyzerCreated.store(false, std::memory_order_release);
    }

    SetRXAFMSQRun(m_channelId, 0);
    destroy_nobEXT(m_channelId);
    destroy_anbEXT(m_channelId);
    CloseChannel(m_channelId);

    m_isOpen.store(false, std::memory_order_release);
    m_isRunning.store(false, std::memory_order_release);
}

void WdspRxChannel::process(CPX &in, CPX &out)
{
    if (!m_isOpen.load(std::memory_order_acquire)) {
        std::fill(out.begin(), out.end(), cpx{0.0, 0.0});
        return;
    }

    int error = 0;
    fexchange0(m_channelId, reinterpret_cast<double*>(in.data()),
               reinterpret_cast<double*>(out.data()), &error);

    if (error != 0) {
        if (error != -2 && m_firstExchangeDone) {
            qDebug() << "WdspRxChannel" << m_channelId << "fexchange0 error=" << error;
        }
        return;
    }

    m_firstExchangeDone = true;
    if (m_analyzerCreated.load(std::memory_order_acquire)) {
        Spectrum0(1, m_channelId, 0, 0, reinterpret_cast<double*>(in.data()));
    }
}

void WdspRxChannel::setMode(DSPMode mode)
{
    m_dspMode = mode;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXAMode(m_channelId, m_dspMode);
        SetRXAPanelRun(m_channelId, 1);
        SetRXAFMSQRun(m_channelId, 1);
        applyNoiseParameters();
    }
}

void WdspRxChannel::setFilter(double low, double high)
{
    m_filterLow = low;
    m_filterHigh = high;
    if (m_isOpen.load(std::memory_order_acquire)) {
        if (m_dspMode == FMN) {
            SetRXAFMDeviation(m_channelId, 8000.0);
        }
        RXASetPassband(m_channelId, low, high);
    }
}

void WdspRxChannel::setFilterSlope(int slope)
{
    m_filterSlope = slope;
    if (m_isOpen.load(std::memory_order_acquire)) {
        setFilter(m_filterLow, m_filterHigh);
    }
}

void WdspRxChannel::setNcoFrequency(long freqHz)
{
    m_ncoFrequency = freqHz;
    if (m_isOpen.load(std::memory_order_acquire)) {
        applyNcoFrequency();
    }
}

void WdspRxChannel::applyNcoFrequency()
{
    if (m_ncoFrequency == 0) {
        SetRXAShiftFreq(m_channelId, 0.0);
        RXANBPSetShiftFrequency(m_channelId, 0.0);
        SetRXAShiftRun(m_channelId, 0);
    } else {
        SetRXAShiftFreq(m_channelId, static_cast<double>(m_ncoFrequency));
        RXANBPSetShiftFrequency(m_channelId, static_cast<double>(m_ncoFrequency));
        SetRXAShiftRun(m_channelId, 1);
    }
}

void WdspRxChannel::setVolume(float volume)
{
    m_volume = qBound(0.0f, volume, 100.0f);
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXAPanelGain1(m_channelId, static_cast<double>(m_volume));
    }
}

void WdspRxChannel::setFmSquelch(bool run, double level)
{
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXAFMSQRun(m_channelId, run ? 1 : 0);
        if (level > 0.0) {
            double threshold = pow(10.0, -2.0 * level / 100.0);
            SetRXAFMSQThreshold(m_channelId, threshold);
        }
    }
}

void WdspRxChannel::setFmDeviation(double hz)
{
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXAFMDeviation(m_channelId, hz);
    }
}

void WdspRxChannel::setAgcMode(AGCMode mode)
{
    m_agcMode = mode;
    if (m_isOpen.load(std::memory_order_acquire)) {
        applyAgcParameters();
    }
}

void WdspRxChannel::setAgcAttack(int ms)
{
    m_agcAttackTime = ms;
    if (m_isOpen.load(std::memory_order_acquire) && m_agcMode == agcUser) {
        SetRXAAGCAttack(m_channelId, ms);
    }
}

void WdspRxChannel::setAgcDecay(int ms)
{
    m_agcDecayTime = ms;
    if (m_isOpen.load(std::memory_order_acquire) && m_agcMode == agcUser) {
        SetRXAAGCDecay(m_channelId, ms);
    }
}

void WdspRxChannel::setAgcHang(int ms)
{
    m_agcHangTime = ms;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXAAGCHang(m_channelId, ms);
    }
}

void WdspRxChannel::setAgcHangThreshold(double thresh)
{
    m_agcHangThreshold = thresh;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXAAGCHangThreshold(m_channelId, thresh);
    }
}

void WdspRxChannel::setAgcHangLevel(double level)
{
    m_agcHangLevel = level;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXAAGCHangLevel(m_channelId, level);
    }
}

void WdspRxChannel::setAgcThreshold(double thresh)
{
    m_agcThreshold = thresh;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXAAGCThresh(m_channelId, thresh, 2048, static_cast<double>(m_dspRate));
    }
}

void WdspRxChannel::setAgcSlope(int slope)
{
    m_agcSlope = slope;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXAAGCSlope(m_channelId, slope);
    }
}

void WdspRxChannel::setAgcMaximumGain(double gainDb)
{
    m_agcMaximumGain = gainDb;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXAAGCTop(m_channelId, gainDb);
    }
}

void WdspRxChannel::setAgcFixedGain(double gainDb)
{
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXAAGCFixed(m_channelId, gainDb);
    }
}

void WdspRxChannel::getAgcLineLevels(double &thresh, double &hang)
{
    if (m_isOpen.load(std::memory_order_acquire)) {
        GetRXAAGCHangLevel(m_channelId, &hang);
        GetRXAAGCThresh(m_channelId, &thresh, 2048, static_cast<double>(m_dspRate));
    } else {
        thresh = m_agcThreshold;
        hang = m_agcHangLevel;
    }
}

void WdspRxChannel::applyAgcParameters()
{
    SetRXAAGCMode(m_channelId, m_agcMode);
    SetRXAAGCSlope(m_channelId, m_agcSlope);
    SetRXAAGCTop(m_channelId, m_agcMaximumGain);

    switch (m_agcMode) {
    case agcOFF:
        break;
    case agcLONG:
        SetRXAAGCAttack(m_channelId, 2);
        SetRXAAGCHang(m_channelId, 2000);
        SetRXAAGCDecay(m_channelId, 2000);
        SetRXAAGCHangThreshold(m_channelId, m_agcHangThreshold);
        break;
    case agcSLOW:
        SetRXAAGCAttack(m_channelId, 2);
        SetRXAAGCHang(m_channelId, 1000);
        SetRXAAGCDecay(m_channelId, 500);
        SetRXAAGCHangThreshold(m_channelId, m_agcHangThreshold);
        break;
    case agcMED:
        SetRXAAGCAttack(m_channelId, 2);
        SetRXAAGCHang(m_channelId, 0);
        SetRXAAGCDecay(m_channelId, 250);
        SetRXAAGCHangThreshold(m_channelId, 100);
        break;
    case agcFAST:
        SetRXAAGCAttack(m_channelId, 2);
        SetRXAAGCHang(m_channelId, 0);
        SetRXAAGCDecay(m_channelId, 50);
        SetRXAAGCHangThreshold(m_channelId, 100);
        break;
    case agcUser:
        SetRXAAGCAttack(m_channelId, m_agcAttackTime);
        SetRXAAGCHang(m_channelId, m_agcHangTime);
        SetRXAAGCDecay(m_channelId, m_agcDecayTime);
        SetRXAAGCHangThreshold(m_channelId, m_agcHangThreshold);
        break;
    }

    if (m_agcThreshold != 0.0) {
        SetRXAAGCThresh(m_channelId, m_agcThreshold, 2048, static_cast<double>(m_dspRate));
    }
    if (m_agcHangLevel != 0.0) {
        SetRXAAGCHangLevel(m_channelId, m_agcHangLevel);
    }
}

void WdspRxChannel::setNoiseBlankerMode(int nbMode)
{
    m_nbMode = nbMode;
    if (m_isOpen.load(std::memory_order_acquire)) {
        applyNoiseParameters();
    }
}

void WdspRxChannel::setNoiseFilterMode(int nrMode)
{
    m_nrMode = nrMode;
    if (m_isOpen.load(std::memory_order_acquire)) {
        applyNoiseParameters();
    }
}

void WdspRxChannel::setNrAGC(int mode)
{
    m_nrAgc = mode;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXAEMNRPosition(m_channelId, m_nrAgc);
    }
}

void WdspRxChannel::setNr2GainMethod(int method)
{
    m_nr2GainMethod = method;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXAEMNRgainMethod(m_channelId, m_nr2GainMethod);
    }
}

void WdspRxChannel::setNr2NpeMethod(int method)
{
    m_nr2NpeMethod = method;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXAEMNRnpeMethod(m_channelId, m_nr2NpeMethod);
    }
}

void WdspRxChannel::setNr2Ae(bool enabled)
{
    m_nr2Ae = enabled;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXAEMNRaeRun(m_channelId, m_nr2Ae);
    }
}

void WdspRxChannel::setAnf(bool enabled)
{
    m_anf = enabled;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXAANFRun(m_channelId, m_anf ? 1 : 0);
    }
}

void WdspRxChannel::setSnb(bool enabled)
{
    m_snb = enabled;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXASNBARun(m_channelId, m_snb ? 1 : 0);
    }
}

void WdspRxChannel::setEmnrPost2(bool run, double factor, double nlevel, int taper, int rate)
{
    m_emnrPost2Run = run;
    m_emnrPost2Factor = factor;
    m_emnrPost2Nlevel = nlevel;
    m_emnrPost2Taper = taper;
    m_emnrPost2Rate = rate;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetRXAEMNRpost2Factor(m_channelId, m_emnrPost2Factor);
        SetRXAEMNRpost2Nlevel(m_channelId, m_emnrPost2Nlevel);
        SetRXAEMNRpost2Taper(m_channelId, m_emnrPost2Taper);
        SetRXAEMNRpost2Rate(m_channelId, m_emnrPost2Rate);
        SetRXAEMNRpost2Run(m_channelId, m_emnrPost2Run ? 1 : 0);
    }
}

void WdspRxChannel::applyNoiseParameters()
{
    int nb = 0, nb2 = 0;
    switch (m_nbMode) {
    case 1: nb = 1; break;
    case 2: nb2 = 1; break;
    default: break;
    }

    int nr = 0, nr2 = 0, nr3 = 0, nr4 = 0;
    switch (m_nrMode) {
    case 1: nr = 1; break;
    case 2: nr2 = 1; break;
    case 3: nr3 = 1; break;
    case 4: nr4 = 1; break;
    default: break;
    }

    SetRXAEMNRPosition(m_channelId, m_nrAgc);
    SetRXAEMNRaeRun(m_channelId, m_nr2Ae ? 1 : 0);
    SetRXAEMNRnpeMethod(m_channelId, m_nr2NpeMethod);
    SetRXAEMNRgainMethod(m_channelId, m_nr2GainMethod);
    SetEXTANBRun(m_channelId, nb);
    SetEXTNOBRun(m_channelId, nb2);

    SetRXAANRRun(m_channelId, 0);
    SetRXAEMNRRun(m_channelId, 0);
    SetRXARNNRRun(m_channelId, 0);
    SetRXASBNRRun(m_channelId, 0);
    SetRXAANRRun(m_channelId, nr);
    SetRXAEMNRRun(m_channelId, nr2);
    SetRXARNNRRun(m_channelId, nr3);
    SetRXASBNRRun(m_channelId, nr4);

    SetRXAANFRun(m_channelId, m_anf ? 1 : 0);
    SetRXASNBARun(m_channelId, m_snb ? 1 : 0);

    setEmnrPost2(m_emnrPost2Run, m_emnrPost2Factor, m_emnrPost2Nlevel, m_emnrPost2Taper, m_emnrPost2Rate);
}

void WdspRxChannel::setRxEq(const QVector<int> &bands, int curveDeg, bool enabled)
{
    m_eqBands = bands;
    m_eqCurveDeg = curveDeg;
    m_eqEnabled = enabled;

    if (m_isOpen.load(std::memory_order_acquire)) {
        int rxeq[11];
        for (int i = 0; i < 11; ++i) {
            rxeq[i] = (i < bands.size()) ? bands.at(i) : 0;
        }
        SetRXAGrphEQ10(m_channelId, rxeq);
        SetRXAEQCurve(m_channelId, curveDeg, 0, 0);
        SetRXAEQRun(m_channelId, enabled ? 1 : 0);
    }
}

bool WdspRxChannel::getEqDraw(double *x, double *y) const
{
    if (!m_isOpen.load(std::memory_order_acquire)) {
        return false;
    }
    GetRXAEQDraw(m_channelId, x, y);
    return true;
}

bool WdspRxChannel::drawEq(int rxId, double *x, double *y)
{
    GetRXAEQDraw(rxId, x, y);
    return true;
}

double WdspRxChannel::getSMeterInstValue() const
{
    if (!m_isOpen.load(std::memory_order_acquire)) return -140.0;
    return GetRXAMeter(m_channelId, RXA_S_AV);
}

double WdspRxChannel::getSMeterPeakValue() const
{
    if (!m_isOpen.load(std::memory_order_acquire)) return -140.0;
    return GetRXAMeter(m_channelId, RXA_S_PK);
}

void WdspRxChannel::initAnalyzer(int refreshRate)
{
    m_refreshRate = refreshRate > 0 ? refreshRate : 15;
    if (!m_analyzerCreated.load(std::memory_order_acquire)) {
        return;
    }

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

    const int max_w = fft_size + std::max(4 * m_bufferSize, static_cast<int>(keep_time * static_cast<double>(m_inRate)));
    const int overlap = qBound(0, static_cast<int>(
        std::max(0.0, std::ceil(fft_size - static_cast<double>(m_inRate) / static_cast<double>(m_refreshRate)))
    ), fft_size - 1);

    SetAnalyzer(m_channelId, n_pixout, spur_elimination_ffts, data_type,
                const_cast<int*>(flp), fft_size, m_bufferSize, window_type, kaiser_pi,
                overlap, clip, span_clip_l, span_clip_h, pixels, stitches,
                calibration_data_set, span_min_freq, span_max_freq, max_w);
}

void WdspRxChannel::setDisplayAveraging(double avb, int average, int detMode, int avMode)
{
    m_displayAvb = avb;
    m_displayAverage = average;
    m_panDetMode = detMode;
    m_panAvMode = avMode;

    if (m_analyzerCreated.load(std::memory_order_acquire)) {
        SetDisplayAvBackmult(m_channelId, 0, avb);
        SetDisplayNumAverage(m_channelId, 0, average);
        SetDisplayDetectorMode(m_channelId, 0, detMode);
        SetDisplayAverageMode(m_channelId, 0, avMode);
    }
}

bool WdspRxChannel::getSpectrumPixels(float *pixels, int &ready)
{
    ready = 0;
    if (!m_analyzerCreated.load(std::memory_order_acquire)) {
        return false;
    }
    GetPixels(m_channelId, 0, pixels, &ready);
    return ready != 0;
}

bool WdspRxChannel::getSpectrumPixels(int rxId, float *pixels, int &ready)
{
    ready = 0;
    GetPixels(rxId, 0, pixels, &ready);
    return ready != 0;
}

void WdspRxChannel::setFftSize(int fftSize)
{
    m_fftSize = fftSize;
    if (m_analyzerCreated.load(std::memory_order_acquire)) {
        initAnalyzer(m_refreshRate);
        SetDisplayAvBackmult(m_channelId, 0, m_displayAvb);
        SetDisplayNumAverage(m_channelId, 0, m_displayAverage);
    }
}
