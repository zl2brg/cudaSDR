#ifndef RTTYDEMODULATOR_H
#define RTTYDEMODULATOR_H

#include <QObject>
#include <QString>
#include <QVector>
#include <complex>
#include <cmath>

/**
 * Struct representing a soft-decision sampled symbol.
 * This is the fundamental input for the Stage 2 Bayesian Trellis / HMM decoder.
 */
struct RttySoftSymbol {
    float llr = 0.0f;        // Log-likelihood ratio (> 0: Mark / bit 1, < 0: Space / bit 0)
    float markMag = 0.0f;    // Matched filter Mark magnitude
    float spaceMag = 0.0f;   // Matched filter Space magnitude
    float snrDb = 0.0f;      // Instantaneous estimated symbol SNR in dB
    qint64 sampleIdx = 0;    // Sample timestamp / index
};

/**
 * Stage 1: Analytic Complex Baseband (I/Q at DC) RTTY Demodulator.
 * 
 * Takes 48 kHz mono audio from the receiver slice, downconverts it to complex
 * baseband centered at DC, decimates by 24 to 2000 Hz, runs orthogonal Mark/Space
 * matched filters at +/- (shift / 2), recovers symbol timing via a 2nd-order DPLL,
 * and computes continuous soft LLR metrics.
 * 
 * Also includes a standalone Baudot ITA2/US-TTY framing state machine for Stage 1
 * testing and verification.
 */
class RttyDemodulator : public QObject {
    Q_OBJECT

public:
    explicit RttyDemodulator(int rxId = 0, QObject *parent = nullptr);
    ~RttyDemodulator() override = default;

    int rxId() const { return m_rxId; }
    void setRxId(int id) { m_rxId = id; }

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enable);

    // Tone Configuration
    float baudRate() const { return m_baudRate; }
    void setBaudRate(float baud);

    float shiftHz() const { return m_shiftHz; }
    void setShiftHz(float shift);

    float centerFreqHz() const { return m_centerFreqHz; }
    void setCenterFreqHz(float centerFreq);

    bool isReversePolarity() const { return m_reversePolarity; }
    void setReversePolarity(bool reverse);

    float markFreqHz() const;
    float spaceFreqHz() const;

    // AFC
    bool isAfcEnabled() const { return m_afcEnabled; }
    void setAfcEnabled(bool enabled);
    float trackedOffsetHz() const { return m_trackedOffsetHz; }

    // Auto Detection & Classification
    bool isAutoDetectEnabled() const { return m_autoDetect; }
    void setAutoDetectEnabled(bool enabled);
    class RttyAutoClassifier* classifier() const { return m_classifier; }

    // Recent decoded text (Stage 1 direct slicer)
    QString recentText() const { return m_recentText; }
    void clearText();
    void reset();

    /**
     * Process audio samples (default input 48 kHz).
     * @param samples Pointer to mono float audio samples (-1.0 to +1.0)
     * @param count Number of samples in buffer
     * @param sampleRate Input sample rate (typically 48000 Hz)
     */
    void processAudio(const float *samples, int count, int sampleRate = 48000);

    static QChar decodeBaudot(quint8 code, bool figs);

signals:
    // Soft symbol stream (Primary interface for Stage 2 Bayesian Trellis)
    void symbolSampled(const RttySoftSymbol &sym);

    // Standalone decoded character stream (Stage 1 direct slicer)
    void characterDecoded(int rx, const QString &character);
    void textUpdated(int rx, const QString &fullRecentText);

    // Diagnostics / Tuning aid
    void toneStatusChanged(int rx, float markFreq, float spaceFreq, float snrDb, bool locked);
    void autoParametersDetected(float shiftHz, float centerFreqHz, float baudRate);
    void polarityInversionDetected(bool reverse);

private:
    void initDecimator();
    void initMatchedFilters();
    void processDecimatedComplexSample(const std::complex<float> &cpxSample);
    void onSymbolStrobe();
    void processBaudotBit(bool bit, float llr);

    int m_rxId = 0;
    bool m_enabled = true;
    float m_baudRate = 45.4545f;
    float m_shiftHz = 170.0f;
    float m_centerFreqHz = 2210.0f; // Standard High Tones (Mark 2125, Space 2295)
    bool m_reversePolarity = false;
    bool m_afcEnabled = true;
    float m_trackedOffsetHz = 0.0f;
    bool m_autoDetect = false;
    class RttyAutoClassifier* m_classifier = nullptr;
    int m_lastZeroCrossingSample = 0;

    // Direct Digital Downconversion (DDC to DC)
    float m_ddcPhase = 0.0f;

    // Decimation (48 kHz -> 2 kHz, decimate by 24)
    static constexpr int DECIM_FACTOR = 24;
    static constexpr int DECIM_RATE = 2000;
    QVector<float> m_firCoeffs;
    QVector<std::complex<float>> m_firState;
    int m_decimPhase = 0;

    // Matched Filters at 2000 Hz (N = 44 samples for 45.45 baud)
    int m_symSamples = 44;
    QVector<std::complex<float>> m_cpxHistory;
    int m_historyIdx = 0;
    QVector<std::complex<float>> m_markRef;
    QVector<std::complex<float>> m_spaceRef;

    // DPLL Clock Recovery
    float m_dpllPhase = 0.0f;
    float m_dpllFreq = 0.022727f; // 45.4545 / 2000
    float m_lastDiscriminator = 0.0f;
    int m_sampleCounter = 0;

    // Noise & Metric Estimation
    float m_noiseVariance = 0.001f;
    float m_markEnergySmooth = 0.0f;
    float m_spaceEnergySmooth = 0.0f;
    float m_currentSnrDb = 0.0f;
    bool m_locked = false;

    // Standalone Framing State Machine
    enum FramerState {
        WaitStart,
        DataBits,
        StopBit
    };
    FramerState m_framerState = WaitStart;
    int m_bitCount = 0;
    quint8 m_shiftReg = 0;
    bool m_figsMode = false;
    QString m_recentText;
    int m_consecutiveIdleCount = 0;
};

#endif // RTTYDEMODULATOR_H
