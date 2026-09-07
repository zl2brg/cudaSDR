#ifndef WDSP_RX_CHANNEL_H
#define WDSP_RX_CHANNEL_H

#include "WdspChannel.h"
#include "qtdsp_qComplex.h"
#include "cusdr_hamDatabase.h"
#include <QVector>

/**
 * @class WdspRxChannel
 * @brief RAII wrapper around a WDSP receiver channel (RX 0..N).
 *
 * Encapsulates OpenChannel, CloseChannel, fexchange0, AGC, noise reduction,
 * filtering, S-meter, and spectrum analyzer controls with channel-state validation.
 */
class WdspRxChannel : public WdspChannel {
public:
    explicit WdspRxChannel(int rxId);
    ~WdspRxChannel() override;

    /**
     * @brief Open and initialize the WDSP RX channel.
     * @param buffSize Number of complex samples per processing block
     * @param inRate Input sample rate from hardware/DDC (e.g. 48000, 96000, 192000, 384000)
     * @param dspRate Internal DSP rate (normally 48000 Hz)
     * @param outRate Demodulated audio output rate (normally 48000 Hz)
     * @param mode Demodulation mode
     * @return true if channel was successfully opened and started
     */
    bool open(int buffSize, int inRate, int dspRate, int outRate, DSPMode mode);

    /**
     * @brief Safely reconfigure channel rates and buffers (tears down and reopens).
     */
    bool reconfigure(int buffSize, int inRate, int dspRate, int outRate, DSPMode mode);

    /**
     * @brief Process one block of complex IQ samples through the DSP channel.
     * @param in  Input complex samples
     * @param out Output demodulated audio samples
     */
    void process(CPX &in, CPX &out);

    /**
     * @brief Close channel and release all WDSP resources.
     */
    void close() override;

    // Filter & Demodulation
    void setMode(DSPMode mode);
    DSPMode mode() const { return m_dspMode; }
    void setFilter(double low, double high);
    void setFilterSlope(int slope);
    void setNcoFrequency(long freqHz);

    // Audio & Gain
    void setVolume(float volume);
    void setFmSquelch(bool run, double level = 0.0);
    void setFmDeviation(double hz);

    // AGC
    void setAgcMode(AGCMode mode);
    AGCMode agcMode() const { return m_agcMode; }
    void setAgcAttack(int ms);
    void setAgcDecay(int ms);
    void setAgcHang(int ms);
    void setAgcHangThreshold(double thresh);
    void setAgcHangLevel(double level);
    void setAgcThreshold(double thresh);
    void setAgcSlope(int slope);
    void setAgcMaximumGain(double gainDb);
    void setAgcFixedGain(double gainDb);
    void getAgcLineLevels(double &thresh, double &hang);

    // Noise Blanking & Noise Reduction
    void setNoiseBlankerMode(int nbMode);
    void setNoiseFilterMode(int nrMode);
    void setNrAGC(int mode);
    void setNr2GainMethod(int method);
    void setNr2NpeMethod(int method);
    void setNr2Ae(bool enabled);
    void setAnf(bool enabled);
    void setSnb(bool enabled);
    void setEmnrPost2(bool run, double factor, double nlevel, int taper, int rate);

    // Equalizer
    void setRxEq(const QVector<int> &bands, int curveDeg, bool enabled);
    bool getEqDraw(double *x, double *y) const;
    static bool drawEq(int rxId, double *x, double *y);

    // S-Meter
    double getSMeterInstValue() const;
    double getSMeterPeakValue() const;

    // Panadapter / Spectrum Analyzer
    void initAnalyzer(int refreshRate);
    void setDisplayAveraging(double avb, int average, int detMode, int avMode);
    bool getSpectrumPixels(float *pixels, int &ready);
    static bool getSpectrumPixels(int rxId, float *pixels, int &ready);

    // Getters for configuration state
    int bufferSize() const { return m_bufferSize; }
    int inputSampleRate() const { return m_inRate; }
    int dspSampleRate() const { return m_dspRate; }
    int outputSampleRate() const { return m_outRate; }
    int fftSize() const { return m_fftSize; }
    void setFftSize(int fftSize);

private:
    void applyAgcParameters();
    void applyNcoFrequency();
    void applyNoiseParameters();

    int m_bufferSize = 0;
    int m_inRate = 48000;
    int m_dspRate = 48000;
    int m_outRate = 48000;
    int m_fftSize = 2048;
    int m_refreshRate = 15;

    DSPMode m_dspMode = USB;
    AGCMode m_agcMode = agcMED;

    double m_filterLow = 150.0;
    double m_filterHigh = 3050.0;
    int m_filterSlope = 0;
    long m_ncoFrequency = 0;
    float m_volume = 1.0f;

    // AGC internal state
    double m_agcThreshold = 0.0;
    double m_agcHangThreshold = 100.0;
    double m_agcHangLevel = 0.0;
    double m_agcMaximumGain = 100.0;
    int m_agcSlope = 25;
    int m_agcAttackTime = 2;
    int m_agcDecayTime = 250;
    int m_agcHangTime = 0;

    // Noise parameters
    int m_nbMode = 0;
    int m_nrMode = 0;
    int m_nrAgc = 0;
    int m_nr2GainMethod = 0;
    int m_nr2NpeMethod = 0;
    bool m_nr2Ae = false;
    bool m_anf = false;
    bool m_snb = false;

    // EMNR post2
    bool m_emnrPost2Run = false;
    double m_emnrPost2Factor = 0.0;
    double m_emnrPost2Nlevel = 0.0;
    int m_emnrPost2Taper = 0;
    int m_emnrPost2Rate = 0;

    // EQ
    QVector<int> m_eqBands;
    int m_eqCurveDeg = 0;
    bool m_eqEnabled = false;

    // Display
    double m_displayAvb = 0.0;
    int m_displayAverage = 2;
    int m_panDetMode = 0;
    int m_panAvMode = 0;

    bool m_firstExchangeDone = false;
};

#endif // WDSP_RX_CHANNEL_H
