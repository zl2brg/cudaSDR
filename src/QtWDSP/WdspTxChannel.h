#ifndef WDSP_TX_CHANNEL_H
#define WDSP_TX_CHANNEL_H

#include "WdspChannel.h"
#include "cusdr_hamDatabase.h"
#include <QVector>

/**
 * @class WdspTxChannel
 * @brief RAII wrapper around a WDSP transmitter channel (default TX_ID = 10).
 *
 * Encapsulates OpenChannel, CloseChannel, fexchange0, TX filtering, audio processing
 * (ALC, leveler, compressor, phase rotator, CFC, EQ), tone generation, and TX panadapter analyzer.
 */
class WdspTxChannel : public WdspChannel {
public:
    explicit WdspTxChannel(int txId = 10);
    ~WdspTxChannel() override;

    /**
     * @brief Open and initialize the WDSP TX channel.
     * @param bufferSize Samples per audio block (typically 1024)
     * @param fftSize FFT size (e.g. 2048)
     * @param micSampleRate Microphone capture rate (typically 48000)
     * @param micDspRate Internal TX DSP rate (typically 48000 or 96000)
     * @param iqOutputRate IQ output rate (48000, 192000, or device sample rate)
     * @param protocol Protocol ID (0 = Original, 1 = New Protocol, etc.)
     * @param lowLatency If true, enables minimum-phase low-latency filter mode
     * @return true if opened successfully
     */
    bool open(int bufferSize, int fftSize, int micSampleRate, int micDspRate,
              int iqOutputRate, int protocol = 0, bool lowLatency = false);

    /**
     * @brief Process microphone audio samples and produce interleaved TX IQ output.
     * @param audioIn Input microphone audio samples (bufferSize)
     * @param iqOut   Output interleaved I/Q samples (re, im alternating)
     * @param error   WDSP error code output
     */
    void process(const double *audioIn, double *iqOut, int &error);

    /**
     * @brief Close channel, release analyzer and channel memory.
     */
    void close() override;

    // Mode & Filter
    void setMode(DSPMode mode);
    DSPMode mode() const { return m_dspMode; }
    void setFilter(double low, double high);
    void setBandpassWindow(int window);

    // Audio & Processing Controls
    void setMicGain(double panelGain);
    void setAudioCompression(int compressionDb, bool run);
    void setLeveler(bool run, double attack = 1.0, double decay = 500.0, double top = 1.0);
    void setAlc(bool run, double attack = 2.0, double decay = 120.0);
    void setFmDeviation(double hz);
    void setAmCarrierLevel(double level);
    void setFmPreEmphasis(int position, bool run);
    void setCtcss(double freqHz, bool enabled);
    void setCfirRun(bool run);
    void setPhaseRotator(bool run, bool autoMode);
    void resetPhaseRotatorAuto();
    void setCfc(bool run, bool peq, const QVector<double> &freqs, const QVector<double> &levels,
                const QVector<double> &post, double precomp, double prePeq, int curveDeg);
    void setTxEq(const QVector<int> &bands, int curveDeg, bool run);

    // Tone & Test Signal Generation
    void setPostGen(int mode, double toneFreq, double toneMag, bool run);
    void setPreGen(int mode, double toneFreq, double toneMag, bool run);
    void setTwoTone(double freq1, double freq2, double mag1, double mag2, bool run);

    // Channel Run State
    void setTxRun(bool run);

    // Spectrum Analyzer & Panadapter
    void pushSpectrum(const double *iqData);
    bool getSpectrumPixels(float *pixels, int &ready);
    static bool getSpectrumPixels(int txId, float *pixels, int &ready);
    void initAnalyzer(int refreshRate);

    // Diagnostics / Curves
    bool getPhaseRotatorAsymmetry(double *in_pos, double *in_neg, double *in_ratio,
                                  double *out_pos, double *out_neg, double *out_ratio,
                                  double *current_fc, double *auto_step) const;
    bool getEqDraw(double *x, double *y) const;
    bool getCfcompCompDraw(double *x, double *y) const;
    bool getCfcompPeqDraw(double *x, double *y) const;
    static bool drawEq(int txId, double *x, double *y);
    static bool drawCfcompComp(int txId, double *x, double *y);
    static bool drawCfcompPeq(int txId, double *x, double *y);

    // Getters
    int bufferSize() const { return m_bufferSize; }
    int micSampleRate() const { return m_micSampleRate; }
    int micDspRate() const { return m_micDspRate; }
    int iqOutputRate() const { return m_iqOutputRate; }

private:
    int m_bufferSize = 1024;
    int m_fftSize = 2048;
    int m_micSampleRate = 48000;
    int m_micDspRate = 48000;
    int m_iqOutputRate = 48000;
    int m_protocol = 0;
    bool m_lowLatency = false;
    int m_refreshRate = 10;

    DSPMode m_dspMode = USB;
    double m_filterLow = 100.0;
    double m_filterHigh = 2900.0;

    double m_panelGain = 1.0;
    int m_compressionDb = 0;
    bool m_compressorRun = false;
    bool m_levelerRun = false;
    double m_fmDeviation = 2500.0;
    double m_amCarrierLevel = 0.5;
    bool m_fmPreEmphRun = false;
    int m_fmPreEmphPosition = 0;
    double m_ctcssFreq = 0.0;
    bool m_ctcssRun = false;
    bool m_cfirRun = false;
    bool m_phaseRotatorRun = false;
    bool m_phaseRotatorAuto = false;
};

#endif // WDSP_TX_CHANNEL_H
