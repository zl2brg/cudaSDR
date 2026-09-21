#ifndef RTTYAUTOCLASSIFIER_H
#define RTTYAUTOCLASSIFIER_H

#include <QObject>
#include <vector>
#include <complex>

/**
 * Real-time automatic RTTY signal classifier and parameter estimator.
 * - Detects dual-tone FSK carrier peaks in audio passband (500 - 2800 Hz)
 * - Classifies frequency shift (170, 200, 425, 450, 850 Hz) and center frequency
 * - Estimates baud rate from discriminator zero-crossing intervals (45.45, 50, 75, 100 baud)
 * - Evaluates stop-bit LLR to suggest polarity inversion
 */
class RttyAutoClassifier : public QObject {
    Q_OBJECT

public:
    explicit RttyAutoClassifier(int rxId = 0, QObject *parent = nullptr);
    ~RttyAutoClassifier() override = default;

    int rxId() const { return m_rxId; }
    void setRxId(int id) { m_rxId = id; }

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enable);

    void reset();

    // Input feeds
    void feedAudio(const float *samples, int count, int sampleRate = 48000);
    void feedTransition(int intervalSamples, float sampleRate = 2000.0f);
    void feedFramingResult(float stopBitLlr, float confidence);

    // Current best estimates
    float estimatedShiftHz() const { return m_lockedShiftHz; }
    float estimatedCenterFreqHz() const { return m_lockedCenterFreqHz; }
    float estimatedBaudRate() const { return m_lockedBaudRate; }
    bool isShiftLocked() const { return m_shiftLocked; }
    bool isBaudLocked() const { return m_baudLocked; }

    static constexpr int FFT_SIZE = 2048;

signals:
    void shiftDetected(float shiftHz, float centerFreqHz);
    void baudRateDetected(float baudRate);
    void polarityInversionSuggested();

private:
    void processFftBuffer();
    void analyzeBaudScores();

    int m_rxId = 0;
    bool m_enabled = true;

    std::vector<float> m_audioRing;
    int m_audioRingPos = 0;

    std::vector<float> m_fftWindow;
    std::vector<float> m_powerSpectrum;
    std::vector<float> m_smoothedSpectrum;
    int m_spectrumFrames = 0;

    float m_lockedShiftHz = 170.0f;
    float m_lockedCenterFreqHz = 2210.0f;
    float m_lockedBaudRate = 45.4545f;
    bool m_shiftLocked = false;
    bool m_baudLocked = false;

    float m_candidateShift = 0.0f;
    float m_candidateCenter = 0.0f;
    int m_shiftCandidateCount = 0;

    // Baud estimation state (candidates: 45.45, 50, 75, 100)
    float m_baudScores[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    int m_transitionCount = 0;

    // Polarity estimation state
    float m_stopBitLlrSum = 0.0f;
    int m_polarityFrameCount = 0;
    int m_polarityCooldown = 0;
};

#endif // RTTYAUTOCLASSIFIER_H
