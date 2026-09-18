#ifndef RTTYBAYESIANDECODER_H
#define RTTYBAYESIANDECODER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <cstdint>
#include <cmath>

#include "AudioEngine/RttyDemodulator.h"

/**
 * Result structure for an individual Bayesian decoded Baudot character.
 */
struct BayesianCharResult {
    QChar character;                ///< Decoded character ('\0' if non-printable or shift)
    quint8 baudotCode = 0;          ///< 5-bit Baudot code (0x00 - 0x1F)
    bool isFigs = false;            ///< Figure shift active during decode
    float confidence = 0.0f;        ///< Posterior probability P(C = c* | L) in [0.0, 1.0]
    float errorProbability = 1.0f;  ///< 1.0 - confidence
    float framingConfidence = 0.0f; ///< Framing log-odds Delta = V(STOP) - V(IDLE)
    float logLikelihoodMargin = 0.0f; ///< Score margin between top-1 and top-2 candidate codes
    float bitLlrs[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; ///< Soft LLRs of the 5 data bits
};

/**
 * Stage 2: Bayesian Trellis / HMM Viterbi RTTY Decoder.
 * 
 * Features:
 * - 8-State Viterbi Framing Trellis:
 *   Tracks concurrent framing hypotheses over soft symbol stream (IDLE, START, D0..D4, STOP).
 *   Effectively suppresses false start bits caused by noise pulses during idle mark tones.
 * - 32-State Bayesian Posterior Slicer:
 *   Computes full posterior distribution P(C = c | L) across all 32 Baudot codes using
 *   Hadamard bit correlations and prior character distributions.
 * - 2-State Hidden Markov Model (HMM) Shift Tracker:
 *   Maintains belief distribution for LTRS vs FIGS states, supporting standard amateur
 *   Unshift-On-Space (USOS) and automatic shift recovery.
 * - Smart Squelch:
 *   Suppresses random noise confetti when framing confidence or character confidence falls
 *   below configured statistical thresholds.
 */
class RttyBayesianDecoder : public QObject {
    Q_OBJECT

public:
    explicit RttyBayesianDecoder(int rxId = 0, QObject *parent = nullptr);
    ~RttyBayesianDecoder() override = default;

    int rxId() const { return m_rxId; }
    void setRxId(int id) { m_rxId = id; }

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

    bool isUsosEnabled() const { return m_usosEnabled; }
    void setUsosEnabled(bool enabled) { m_usosEnabled = enabled; }

    float squelchThreshold() const { return m_squelchThreshold; }
    void setSquelchThreshold(float thresh) { m_squelchThreshold = thresh; }

    float framingThreshold() const { return m_framingThreshold; }
    void setFramingThreshold(float thresh) { m_framingThreshold = thresh; }

    QString recentText() const { return m_recentText; }
    void clearText();
    void reset();

    bool isFramingLocked() const { return m_framingLocked; }
    float framingConfidence() const { return m_lastFramingConfidence; }
    float figsBelief() const { return m_figsBelief; }

    /**
     * Static helper to compute posterior probabilities and confidence for 5 soft data LLRs.
     * @param dataLlrs Array of 5 soft LLR values for bits b0..b4
     * @param figs Figure shift flag
     * @param customPriors Optional array of 32 prior probabilities (uniform if nullptr)
     */
    static BayesianCharResult evaluateBaudotPosterior(
        const float dataLlrs[5],
        bool figs,
        const float *customPriors = nullptr);

public slots:
    /**
     * Ingest a single soft symbol from RttyDemodulator::symbolSampled.
     */
    void processSymbol(const RttySoftSymbol &sym);

    /**
     * Ingest a batch of soft symbols.
     */
    void processSymbols(const QVector<RttySoftSymbol> &symbols);

signals:
    // Decoded character with confidence metrics
    void characterDecoded(int rx, const QString &character, float confidence, float errProb);
    void characterResultDecoded(int rx, const BayesianCharResult &result);
    void textUpdated(int rx, const QString &fullRecentText);

    // Framing synchronization diagnostics
    void framingStateChanged(int rx, bool locked, float framingConfidence);

private:
    void processSoftSymbol(float llr);
    void updateHMMShift(quint8 code);

    int m_rxId = 0;
    bool m_enabled = true;
    bool m_usosEnabled = true;          // Unshift On Space
    float m_squelchThreshold = 0.35f;   // Squelch if char confidence < threshold
    float m_framingThreshold = 0.0f;    // Minimum framing metric (-startLlr + stopLlr)

    struct FrameHypothesis {
        qint64 startSampleIdx = 0;
        int bitCount = 0; // 0..4: collecting data bits 0..4; 5: stop bit; 6: completed
        float startLlr = 0.0f;
        float dataLlrs[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        float stopLlr = 0.0f;
    };

    QVector<FrameHypothesis> m_activeHypotheses;
    qint64 m_totalSymbols = 0;
    qint64 m_lastAcceptedEndIdx = -100;

    // Shift state HMM
    float m_figsBelief = 0.0f;          // P(Shift = FIGS), 0.0 = LTRS, 1.0 = FIGS

    // Framing lock state
    bool m_framingLocked = false;
    float m_lastFramingConfidence = 0.0f;
    int m_lockCounter = 0;

    // Output text buffer
    QString m_recentText;
};

#endif // RTTYBAYESIANDECODER_H
