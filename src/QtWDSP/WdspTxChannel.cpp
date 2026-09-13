#include "WdspTxChannel.h"
#include <QDebug>
#include <cmath>
#include <algorithm>

extern "C" {
#include <wdsp.h>
}

WdspTxChannel::WdspTxChannel(int txId)
    : WdspChannel(txId)
{
}

WdspTxChannel::~WdspTxChannel()
{
    close();
}

bool WdspTxChannel::open(int bufferSize, int fftSize, int micSampleRate, int micDspRate,
                         int iqOutputRate, int protocol, bool lowLatency)
{
    close();

    QMutexLocker locker(&m_channelMutex);
    QMutexLocker wisdomLocker(&s_wdspWisdomMutex);

    m_bufferSize = bufferSize > 0 ? bufferSize : 1024;
    m_fftSize = fftSize > 0 ? fftSize : 2048;
    m_micSampleRate = micSampleRate > 0 ? micSampleRate : 48000;
    m_micDspRate = micDspRate > 0 ? micDspRate : 48000;
    m_iqOutputRate = iqOutputRate > 0 ? iqOutputRate : 48000;
    m_protocol = protocol;
    m_lowLatency = lowLatency;

    OpenChannel(m_channelId, m_bufferSize, 2048, m_micSampleRate, m_micDspRate, m_iqOutputRate,
                1 /* TX */, 0 /* stopped initially */, 0.010, 0.025, 0.0, 0.010, 0);

    if (m_fftSize > 2048) {
        TXASetNC(m_channelId, m_fftSize);
    }
    TXASetMP(m_channelId, m_lowLatency ? 1 : 0);
    SetTXABandpassWindow(m_channelId, 1);

    SetTXAFMEmphPosition(m_channelId, m_fmPreEmphPosition);
    SetTXAFMEmphRun(m_channelId, m_fmPreEmphRun ? 1 : 0);
    SetTXAPHROTRun(m_channelId, m_phaseRotatorRun ? 1 : 0);
    SetTXAPHROTAutoMode(m_channelId, m_phaseRotatorAuto ? 1 : 0);

    SetTXACFIRRun(m_channelId, m_protocol == 1 ? 1 : 0);
    SetTXAAMSQRun(m_channelId, 0);
    SetTXAosctrlRun(m_channelId, 0);

    SetTXAALCAttack(m_channelId, 2);
    SetTXAALCDecay(m_channelId, 120);
    SetTXAALCSt(m_channelId, 1);

    SetTXALevelerAttack(m_channelId, 1);
    SetTXALevelerDecay(m_channelId, 500);
    SetTXALevelerTop(m_channelId, 1.0);
    SetTXALevelerSt(m_channelId, m_levelerRun ? 1 : 0);

    SetTXAPostGenMode(m_channelId, 0);
    SetTXAPostGenToneMag(m_channelId, 1.0);
    SetTXAPostGenTTMag(m_channelId, 1.0, 1.0);
    SetTXAPostGenToneFreq(m_channelId, 1000.0);
    SetTXAPostGenRun(m_channelId, 0);

    SetTXAPreGenMode(m_channelId, 0);
    SetTXAPreGenToneMag(m_channelId, 0.0);
    SetTXAPreGenToneFreq(m_channelId, 0.0);
    SetTXAPreGenRun(m_channelId, 0);

    SetTXAPanelGain1(m_channelId, m_panelGain);
    SetTXAPanelRun(m_channelId, 1);

    SetTXAFMDeviation(m_channelId, m_fmDeviation);
    SetTXAAMCarrierLevel(m_channelId, m_amCarrierLevel);
    SetTXACompressorGain(m_channelId, m_compressionDb);
    SetTXACompressorRun(m_channelId, m_compressorRun ? 1 : 0);

    SetTXACTCSSFreq(m_channelId, m_ctcssFreq);
    SetTXACTCSSRun(m_channelId, m_ctcssRun ? 1 : 0);

    SetTXAMode(m_channelId, m_dspMode);
    SetTXABandpassFreqs(m_channelId, m_filterLow, m_filterHigh + 1.0);
    SetTXABandpassFreqs(m_channelId, m_filterLow, m_filterHigh);

    int rc = 0;
    XCreateAnalyzer(m_channelId, &rc, 262144, 1, 1, const_cast<char*>(""));
    if (rc == 0) {
        m_analyzerCreated.store(true, std::memory_order_release);
        initAnalyzer(m_refreshRate);
    } else {
        qWarning() << "WdspTxChannel" << m_channelId << "XCreateAnalyzer failed:" << rc;
        m_analyzerCreated.store(false, std::memory_order_release);
    }

    m_isOpen.store(true, std::memory_order_release);
    m_isRunning.store(false, std::memory_order_release);
    return true;
}

void WdspTxChannel::close()
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

    CloseChannel(m_channelId);
    m_isOpen.store(false, std::memory_order_release);
    m_isRunning.store(false, std::memory_order_release);
}

void WdspTxChannel::process(const double *audioIn, double *iqOut, int &error)
{
    error = 0;
    if (!m_isOpen.load(std::memory_order_acquire)) {
        error = -1;
        return;
    }

    fexchange0(m_channelId, const_cast<double*>(audioIn), iqOut, &error);
}

void WdspTxChannel::pushSpectrum(const double *iqData)
{
    if (m_analyzerCreated.load(std::memory_order_acquire)) {
        Spectrum0(1, m_channelId, 0, 0, const_cast<double*>(iqData));
    }
}

bool WdspTxChannel::getSpectrumPixels(float *pixels, int &ready)
{
    ready = 0;
    if (!m_analyzerCreated.load(std::memory_order_acquire)) {
        return false;
    }
    GetPixels(m_channelId, 0, pixels, &ready);
    return ready != 0;
}

bool WdspTxChannel::getSpectrumPixels(int txId, float *pixels, int &ready)
{
    ready = 0;
    GetPixels(txId, 0, pixels, &ready);
    return ready != 0;
}

void WdspTxChannel::initAnalyzer(int refreshRate)
{
    m_refreshRate = refreshRate > 0 ? refreshRate : 10;
    if (!m_analyzerCreated.load(std::memory_order_acquire)) {
        return;
    }

    int flp[] = {0};
    constexpr double keep_time = 0.1;
    constexpr int n_pixout = 1;
    constexpr int spur_elimination_ffts = 1;
    constexpr int data_type = 1;
    constexpr int fft_size = 2048;
    constexpr int window_type = 4;
    constexpr double kaiser_pi = 14.0;
    constexpr int clip = 0;
    constexpr int span_clip_l = 0;
    constexpr int span_clip_h = 0;
    constexpr int stitches = 1;
    constexpr int calibration_data_set = 0;
    constexpr double span_min_freq = 0.0;
    constexpr double span_max_freq = 0.0;

    int max_w = fft_size + static_cast<int>(std::min(keep_time * static_cast<double>(m_refreshRate),
                                                     keep_time * static_cast<double>(fft_size) * static_cast<double>(m_refreshRate)));
    int overlap = static_cast<int>(std::max(0.0, std::ceil(fft_size - static_cast<double>(m_micSampleRate) / static_cast<double>(m_refreshRate))));

    SetAnalyzer(m_channelId, n_pixout, spur_elimination_ffts, data_type,
                flp, fft_size, 1024, window_type, kaiser_pi,
                overlap, clip, span_clip_l, span_clip_h, 4096, stitches,
                calibration_data_set, span_min_freq, span_max_freq, max_w);
}

void WdspTxChannel::setMode(DSPMode mode)
{
    m_dspMode = mode;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetTXAMode(m_channelId, m_dspMode);
    }
}

void WdspTxChannel::setFilter(double low, double high)
{
    m_filterLow = low;
    m_filterHigh = high;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetTXABandpassFreqs(m_channelId, low, high + 1.0);
        SetTXABandpassFreqs(m_channelId, low, high);
    }
}

void WdspTxChannel::setBandpassWindow(int window)
{
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetTXABandpassWindow(m_channelId, window);
    }
}

void WdspTxChannel::setMicGain(double panelGain)
{
    m_panelGain = panelGain;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetTXAPanelGain1(m_channelId, m_panelGain);
        SetTXAPanelRun(m_channelId, 1);
    }
}

void WdspTxChannel::setAudioCompression(int compressionDb, bool run)
{
    m_compressionDb = compressionDb;
    m_compressorRun = run;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetTXACompressorGain(m_channelId, compressionDb);
        SetTXACompressorRun(m_channelId, run ? 1 : 0);
    }
}

void WdspTxChannel::setLeveler(bool run, double attack, double decay, double top)
{
    m_levelerRun = run;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetTXALevelerAttack(m_channelId, attack);
        SetTXALevelerDecay(m_channelId, decay);
        SetTXALevelerTop(m_channelId, top);
        SetTXALevelerSt(m_channelId, run ? 1 : 0);
    }
}

void WdspTxChannel::setAlc(bool run, double attack, double decay)
{
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetTXAALCAttack(m_channelId, attack);
        SetTXAALCDecay(m_channelId, decay);
        SetTXAALCSt(m_channelId, run ? 1 : 0);
    }
}

void WdspTxChannel::setFmDeviation(double hz)
{
    m_fmDeviation = hz;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetTXAFMDeviation(m_channelId, hz);
    }
}

void WdspTxChannel::setAmCarrierLevel(double level)
{
    m_amCarrierLevel = level;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetTXAAMCarrierLevel(m_channelId, level);
    }
}

void WdspTxChannel::setFmPreEmphasis(int position, bool run)
{
    m_fmPreEmphPosition = position;
    m_fmPreEmphRun = run;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetTXAFMEmphPosition(m_channelId, position);
        SetTXAFMEmphRun(m_channelId, run ? 1 : 0);
    }
}

void WdspTxChannel::setCtcss(double freqHz, bool enabled)
{
    m_ctcssFreq = freqHz;
    m_ctcssRun = enabled;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetTXACTCSSFreq(m_channelId, freqHz);
        SetTXACTCSSRun(m_channelId, enabled ? 1 : 0);
    }
}

void WdspTxChannel::setCfirRun(bool run)
{
    m_cfirRun = run;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetTXACFIRRun(m_channelId, run ? 1 : 0);
    }
}

void WdspTxChannel::setPhaseRotator(bool run, bool autoMode)
{
    m_phaseRotatorRun = run;
    m_phaseRotatorAuto = autoMode;
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetTXAPHROTRun(m_channelId, run ? 1 : 0);
        SetTXAPHROTAutoMode(m_channelId, (run && autoMode) ? 1 : 0);
    }
}

void WdspTxChannel::resetPhaseRotatorAuto()
{
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetTXAPHROTAutoReset(m_channelId);
    }
}

void WdspTxChannel::setCfc(bool run, bool peq, const QVector<double> &freqs, const QVector<double> &levels,
                           const QVector<double> &post, double precomp, double prePeq, int curveDeg)
{
    if (!m_isOpen.load(std::memory_order_acquire)) {
        return;
    }

    const int n = std::min({freqs.size(), levels.size(), post.size()});
    if (n > 0) {
        QVector<double> F = freqs.mid(0, n);
        QVector<double> G = levels.mid(0, n);
        QVector<double> E = post.mid(0, n);
        SetTXACFCOMPprofile(m_channelId, n, F.data(), G.data(), E.data());
    }
    SetTXACFCOMPPrecomp(m_channelId, precomp);
    SetTXACFCOMPPrePeq(m_channelId, prePeq);
    SetTXACFCOMPCompCurve(m_channelId, curveDeg, 0, 0);
    SetTXACFCOMPPeqCurve(m_channelId, curveDeg, 0, 0);
    SetTXACFCOMPRun(m_channelId, (run || peq) ? 1 : 0);
    SetTXACFCOMPPeqRun(m_channelId, peq ? 1 : 0);
}

void WdspTxChannel::setTxEq(const QVector<int> &bands, int curveDeg, bool run)
{
    if (!m_isOpen.load(std::memory_order_acquire)) {
        return;
    }

    if (run) {
        int txeq[11];
        for (int i = 0; i < 11; ++i) {
            txeq[i] = (i < bands.size()) ? bands.at(i) : 0;
        }
        SetTXAGrphEQ10(m_channelId, txeq);
        SetTXAEQCurve(m_channelId, curveDeg, 0, 0);
        SetTXAEQRun(m_channelId, 1);
    } else {
        SetTXAEQRun(m_channelId, 0);
    }
}

void WdspTxChannel::setPostGen(int mode, double toneFreq, double toneMag, bool run)
{
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetTXAPostGenMode(m_channelId, mode);
        SetTXAPostGenToneFreq(m_channelId, toneFreq);
        SetTXAPostGenToneMag(m_channelId, toneMag);
        SetTXAPostGenRun(m_channelId, run ? 1 : 0);
    }
}

void WdspTxChannel::setPreGen(int mode, double toneFreq, double toneMag, bool run)
{
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetTXAPreGenMode(m_channelId, mode);
        SetTXAPreGenToneFreq(m_channelId, toneFreq);
        SetTXAPreGenToneMag(m_channelId, toneMag);
        SetTXAPreGenRun(m_channelId, run ? 1 : 0);
    }
}

void WdspTxChannel::setTwoTone(double freq1, double freq2, double mag1, double mag2, bool run)
{
    Q_UNUSED(freq1)
    Q_UNUSED(freq2)
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetTXAPostGenTTMag(m_channelId, mag1, mag2);
        SetTXAPostGenRun(m_channelId, run ? 1 : 0);
    }
}

void WdspTxChannel::setTxRun(bool run)
{
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetChannelState(m_channelId, run ? 1 : 0, 1);
        m_isRunning.store(run, std::memory_order_release);
    }
}

bool WdspTxChannel::getPhaseRotatorAsymmetry(double *in_pos, double *in_neg, double *in_ratio,
                                             double *out_pos, double *out_neg, double *out_ratio,
                                             double *current_fc, double *auto_step) const
{
    if (!m_isOpen.load(std::memory_order_acquire)) {
        return false;
    }
    GetTXAPHROTAsymmetry(m_channelId, in_pos, in_neg, in_ratio,
                         out_pos, out_neg, out_ratio, current_fc, auto_step);
    return true;
}

bool WdspTxChannel::getEqDraw(double *x, double *y) const
{
    if (!m_isOpen.load(std::memory_order_acquire)) {
        return false;
    }
    GetTXAEQDraw(m_channelId, x, y);
    return true;
}

bool WdspTxChannel::getCfcompCompDraw(double *x, double *y) const
{
    if (!m_isOpen.load(std::memory_order_acquire)) {
        return false;
    }
    GetTXACFCOMPCompDraw(m_channelId, x, y);
    return true;
}

bool WdspTxChannel::getCfcompPeqDraw(double *x, double *y) const
{
    if (!m_isOpen.load(std::memory_order_acquire)) {
        return false;
    }
    GetTXACFCOMPPeqDraw(m_channelId, x, y);
    return true;
}

bool WdspTxChannel::drawEq(int txId, double *x, double *y)
{
    GetTXAEQDraw(txId, x, y);
    return true;
}

bool WdspTxChannel::drawCfcompComp(int txId, double *x, double *y)
{
    GetTXACFCOMPCompDraw(txId, x, y);
    return true;
}

bool WdspTxChannel::drawCfcompPeq(int txId, double *x, double *y)
{
    GetTXACFCOMPPeqDraw(txId, x, y);
    return true;
}
