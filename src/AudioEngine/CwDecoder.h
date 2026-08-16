#ifndef CWDECODER_H
#define CWDECODER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <cmath>

/**
 * Real-time adaptive CW (Morse code) decoder.
 * 
 * Features:
 * - 2nd-order IIR bandpass filter tuned to CW pitch (e.g. 600 - 800 Hz)
 * - Low-pass filtered envelope detector
 * - Dynamic adaptive noise floor and peak signal tracker
 * - Schmitt-trigger hysteresis slicer
 * - Continuous automatic WPM (dit length) timing tracker (8 - 60 WPM)
 * - Prosign support (<AR>, <SK>, <BT>, <KN>, <AS>, <SOS>, etc.)
 * - Circular text buffer for UI HUD overlays
 */
class CwDecoder : public QObject {
    Q_OBJECT

public:
    explicit CwDecoder(int rxId = 0, QObject *parent = nullptr);
    ~CwDecoder() override = default;

    int rxId() const { return m_rxId; }
    void setRxId(int id) { m_rxId = id; }

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enable);

    int pitch() const { return m_pitchHz; }
    void setPitch(int pitchHz);

    int trackedPitch() const { return qRound(m_trackedPitchHz); }
    bool isAutoTrackEnabled() const { return m_autoTrack; }
    void setAutoTrackEnabled(bool enabled);

    int wpm() const { return m_lastReportedWpm; }
    float snrDb() const { return m_currentSnrDb; }
    bool isToneActive() const { return m_toneActive; }
    float minSnrDb() const { return m_minSnrDb; }
    void setMinSnrDb(float db) { m_minSnrDb = db; }

    QString recentText() const { return m_recentText; }
    void clearText();
    void reset();

    /**
     * Feed mono audio samples at specified sample rate.
     * @param samples Pointer to float mono audio samples (-1.0 to +1.0)
     * @param count Number of samples in buffer
     * @param sampleRate Input sample rate (typically 48000 Hz)
     */
    void processAudio(const float *samples, int count, int sampleRate = 48000);

signals:
    void characterDecoded(int rx, const QString &character, int currentWpm);
    void textUpdated(int rx, const QString &fullRecentText);
    void wpmChanged(int rx, int currentWpm);
    void toneStatusChanged(int rx, bool active, float snrDb);
    void trackedPitchChanged(int rx, int pitchHz);

private:
    void updateBiquadCoefficients();
    void estimateTonePitch();
    void processEnvelope(float env, float dtMs);
    void onMarkCompleted(float durationMs);
    void onSpaceCompleted(float durationMs);
    void decodeCurrentSymbol();
    static QString morseToChar(const QString &morse);

    int m_rxId = 0;
    bool m_enabled = true;
    int m_pitchHz = 700;
    int m_sampleRate = 48000;

    // Biquad Bandpass Filter State
    float m_b0 = 0.0f, m_b1 = 0.0f, m_b2 = 0.0f;
    float m_a1 = 0.0f, m_a2 = 0.0f;
    float m_x1 = 0.0f, m_x2 = 0.0f;
    float m_y1 = 0.0f, m_y2 = 0.0f;

    // Envelope Low-pass Filter State
    float m_envelope = 0.0f;
    float m_envAlpha = 0.05f;

    // Adaptive Slicer / Energy Tracker
    float m_noiseLevel = 0.005f;
    float m_peakLevel = 0.08f;
    bool m_toneActive = false;
    float m_currentSnrDb = 0.0f;
    float m_minSnrDb = 6.0f;

    // Timing Tracker (in milliseconds)
    float m_ditMs = 60.0f; // Initial 20 WPM (1200 / 60)
    int m_currentWpm = 20;
    int m_lastReportedWpm = 20;
    float m_markDurationMs = 0.0f;
    float m_spaceDurationMs = 0.0f;

    // Morse accumulator
    QString m_symbolAccumulator;
    QString m_recentText;
    bool m_charPending = false;
    bool m_wordSpacePending = false;

    // Decimation accumulator for 48kHz -> 8kHz
    int m_decimFactor = 6;
    int m_decimCounter = 0;

    // Auto Pitch Tracker State
    bool m_autoTrack = true;
    float m_trackedPitchHz = 700.0f;
    float m_lastFilterPitchHz = 700.0f;
    float m_wbB0 = 0.0f, m_wbB1 = 0.0f, m_wbB2 = 0.0f;
    float m_wbA1 = 0.0f, m_wbA2 = 0.0f;
    float m_wbX1 = 0.0f, m_wbX2 = 0.0f, m_wbY1 = 0.0f, m_wbY2 = 0.0f;
    float m_ringBuffer[64] = {0.0f};
    int m_ringIdx = 0;
    int m_pitchEstimCounter = 0;
};

#endif // CWDECODER_H
